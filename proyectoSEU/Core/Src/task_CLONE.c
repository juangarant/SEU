#include <stdint.h>
#include "FreeRTOS.h"
#include <stdio.h>
#include "cmsis_os.h"
#include <stdlib.h>
#include "semphr.h"
#include "tareas.h"
#include <string.h>
#include <task.h>

#include "task_CONSOLE.h"
#include "task_COMM.h"
#include "task_CLONE.h"
#include "monitor.h"
#include "cJSON.h"

uint32_t global_clone_it;

// Datos leídos del nodo clonado, accesibles desde monitor.c
float clone_temperatura   = 0.0f;
float clone_ldr           = 0.0f;
float clone_alarma_ntc    = 0.0f;
float clone_alarma_ldr    = 0.0f;
uint8_t clone_alarma_activa = 0;

static uint8_t clone_request[256];
static volatile uint8_t clone_node_index = 0;   /* nodo a clonar 0..26 (lo fija el potenciometro) */
static volatile uint8_t clone_silence_request = 0;   /* el boton 2 ha pedido silenciar */
static uint8_t          clone_seq = 0;               /* numero de secuencia de Alarma_src */
static uint8_t          clone_post_body[96];

void Task_CLONE_init(void) {
    BaseType_t res;
    global_clone_it = 0;

    res = xTaskCreate(Task_CLONE, "CLONE", 768, NULL, NORMAL_PRIORITY, NULL);
    if (res != pdPASS) {
        bprintf("PANIC: Error al crear Tarea CLONE\r\n");
        while(1);
    }
}

// Selecciona el nodo a clonar según el potenciómetro (0–26)
void CLONE_select_node(uint8_t node_index) {
    if (node_index > 26) node_index = 26;
    clone_node_index = node_index;   /* escritura atomica de 1 byte */
}

// Pide enviar Alarma_src al nodo clonado (lo llama monitor.c desde el boton 2)
void CLONE_request_silence(void) {
    clone_silence_request = 1;
}

void Task_CLONE(void *pvParameters) {

    int signal;
    uint8_t last_idx = 0xFF;   /* para imprimir el nodo clonado solo al cambiar */
    cJSON *json;

    while (1) {

        // Solo activo en modo clon
        if (Monitor_GetMode() != MODE_CLON) {
            vTaskDelay(2000 / portTICK_RATE_MS);
            continue;
        }

        // Sin WiFi: esperar sin saturar a Task_COMM
        if (!global_wifi_ready) {
            vTaskDelay(3000 / portTICK_RATE_MS);
            continue;
        }

        // Construir la petición GET al Context Broker
        if (clone_node_index != last_idx) {
            last_idx = clone_node_index;
            bprintf("CLONE: clonando SensorSEU_%02d\r\n", clone_node_index);
        }

        snprintf((char *)clone_request, sizeof(clone_request),
            "GET /v2/entities/SensorSEU_%02d HTTP/1.1\r\n"
            "Host: %s\r\n"
            "Accept: application/json\r\n"
            "\r\n",
            clone_node_index,
            SERVER_IP
        );

        // Esperar acceso exclusivo a COMM_request
        signal = 1;
        do {
            if (xSemaphoreTake(COMM_xSem, 20000 / portTICK_RATE_MS) != pdTRUE) {
                bprintf("CLONE: HARAKIRI\r\n");
                HAL_NVIC_SystemReset();
            }
            if (COMM_request.command == 0) {
                COMM_request.command      = 1;
                COMM_request.result       = 0;
                COMM_request.dst_port     = ORION_PORT;
                COMM_request.dst_address  = (uint8_t *)SERVER_IP;
                COMM_request.HTTP_request = clone_request;
                signal = 0;
                xSemaphoreGive(COMM_xSem);
            } else {
                xSemaphoreGive(COMM_xSem);
                vTaskDelay(10 / portTICK_RATE_MS);
            }
        } while (signal);

        // Esperar respuesta
        COMM_WAIT_RESULT();

        // Parsear el JSON de respuesta
        json = cJSON_Parse((const char *)COMM_request.HTTP_response);
        if (json) {
            // En v2 los atributos son objetos directos, no un array
            cJSON *temp = cJSON_GetObjectItem(json, "Temperatura");
            cJSON *lux  = cJSON_GetObjectItem(json, "IntensidadLuz");
            cJSON *alarm = cJSON_GetObjectItem(json, "Alarma");

            Monitor_LockModel(); /* Fase 2: proteger variables clone_* */
            if (temp) {
                cJSON *val = cJSON_GetObjectItem(temp, "value");
                if (val && val->valuestring) {
                    float dmax = 0.0f, dmin = 0.0f;
                    sscanf(val->valuestring, "%f,%f,%f,%f",
                        &clone_temperatura, &dmax, &dmin, &clone_alarma_ntc);
                }
            }
            if (lux) {
                cJSON *val = cJSON_GetObjectItem(lux, "value");
                if (val && val->valuestring) {
                    float dmax = 0.0f, dmin = 0.0f;
                    sscanf(val->valuestring, "%f,%f,%f,%f",
                        &clone_ldr, &dmax, &dmin, &clone_alarma_ldr);
                }
            }
            if (alarm) {
                cJSON *val = cJSON_GetObjectItem(alarm, "value");
                if (val && val->valuestring)
                    clone_alarma_activa = (val->valuestring[0] == 'T') ? 1 : 0;
            }
            Monitor_UnlockModel();
            cJSON_Delete(json);
        }

        COMM_request.result  = 0;
        COMM_request.command = 0;

        /* Fase 5: si el boton 2 pidio silenciar, escribir Alarma_src en la entidad clonada */
        if (clone_silence_request) {
            clone_silence_request = 0;
            clone_seq++;
            snprintf((char *)clone_post_body, sizeof(clone_post_body),
                "{\"Alarma_src\":{\"value\":\"%s_%u\",\"type\":\"string\"}}",
                IoT_NAME, (unsigned)clone_seq);
            snprintf((char *)clone_request, sizeof(clone_request),
                "POST /v2/entities/SensorSEU_%02d/attrs HTTP/1.1\r\n"
                "Host: %s\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: %d\r\n"
                "\r\n"
                "%s",
                clone_node_index, SERVER_IP,
                (int)strlen((char *)clone_post_body), clone_post_body);

            signal = 1;
            do {
                if (xSemaphoreTake(COMM_xSem, 20000 / portTICK_RATE_MS) != pdTRUE) {
                    bprintf("CLONE: HARAKIRI\r\n");
                    HAL_NVIC_SystemReset();
                }
                if (COMM_request.command == 0) {
                    COMM_request.command      = 1;
                    COMM_request.result       = 0;
                    COMM_request.dst_port     = ORION_PORT;
                    COMM_request.dst_address  = (uint8_t *)SERVER_IP;
                    COMM_request.HTTP_request = clone_request;
                    signal = 0;
                    xSemaphoreGive(COMM_xSem);
                } else {
                    xSemaphoreGive(COMM_xSem);
                    vTaskDelay(10 / portTICK_RATE_MS);
                }
            } while (signal);
            COMM_WAIT_RESULT();
            COMM_request.result  = 0;
            COMM_request.command = 0;
            bprintf("CLONE: Alarma_src enviado a SensorSEU_%02d (seq=%u)\r\n",
                    clone_node_index, (unsigned)clone_seq);
        }

        global_clone_it++;

        // Consultar cada 3 segundos (mejor latencia de la alarma del nodo clonado)
        vTaskDelay(3000 / portTICK_RATE_MS);
    }
}