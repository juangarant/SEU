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
#include "task_ORION.h"
#include "monitor.h"
#include "cJSON.h"

uint32_t global_orion_it;

static uint8_t orion_body[1024];
static uint8_t orion_request[1280];

void Task_ORION_init(void) {
    BaseType_t res;
    global_orion_it = 0;

    res = xTaskCreate(Task_ORION, "ORION", 1024, NULL, NORMAL_PRIORITY, NULL);
    if (res != pdPASS) {
        bprintf("PANIC: Error al crear Tarea ORION\r\n");
        while(1);
    }
}

void Task_ORION(void *pvParameters) {

    int signal;
    monitor_snapshot_t snap;

    while (1) {

        // Fase 2: copia consistente del modelo bajo mutex
        Monitor_GetSnapshot(&snap);

        // Solo publicar en modo conectado
        if (snap.mode != MODE_CONECTADO) {
            vTaskDelay(2000 / portTICK_RATE_MS);
            continue;
        }

        // Sin WiFi: esperar sin saturar a Task_COMM
        if (!global_wifi_ready) {
            vTaskDelay(3000 / portTICK_RATE_MS);
            continue;
        }

        /* Fase 5: leer el propio Alarma_src del broker para honrar a un nodo clon */
        snprintf((char *)orion_request, sizeof(orion_request),
            "GET /v2/entities/%s/attrs/Alarma_src HTTP/1.1\r\n"
            "Host: %s\r\n"
            "Accept: application/json\r\n"
            "\r\n",
            IoT_NAME, SERVER_IP);

        signal = 1;
        do {
            if (xSemaphoreTake(COMM_xSem, 20000 / portTICK_RATE_MS) != pdTRUE) {
                bprintf("ORION: HARAKIRI\r\n");
                HAL_NVIC_SystemReset();
            }
            if (COMM_request.command == 0) {
                COMM_request.command     = 1;
                COMM_request.result      = 0;
                COMM_request.dst_port    = ORION_PORT;
                COMM_request.dst_address = (uint8_t *)SERVER_IP;
                COMM_request.HTTP_request = orion_request;
                signal = 0;
                xSemaphoreGive(COMM_xSem);
            } else {
                xSemaphoreGive(COMM_xSem);
                vTaskDelay(10 / portTICK_RATE_MS);
            }
        } while (signal);
        COMM_WAIT_RESULT();
        {
            cJSON *j = cJSON_Parse((const char *)COMM_request.HTTP_response);
            if (j) {
                cJSON *v = cJSON_GetObjectItem(j, "value");
                if (v && v->valuestring)
                    Monitor_ProcessRemoteAlarmaSrc(v->valuestring);
                cJSON_Delete(j);
            }
        }
        COMM_request.result  = 0;
        COMM_request.command = 0;

        /* releer el modelo: Alarma_src puede haber cambiado */
        Monitor_GetSnapshot(&snap);

        // Construir el cuerpo JSON con los valores actuales
        snprintf((char *)orion_body, sizeof(orion_body),
            "{"
            "\"id\":\"%s\","
            "\"type\":\"Sensor\","
            "\"Temperatura\":{"
                "\"value\":\"%.2f,%.2f,%.2f,%.2f\","
                "\"type\":\"floatarray\"},"
            "\"IntensidadLuz\":{"
                "\"value\":\"%.2f,%.2f,%.2f,%.2f\","
                "\"type\":\"floatarray\"},"
            "\"Alarma\":{"
                "\"value\":\"%c\","
                "\"type\":\"boolean\"},"
            "\"Alarma_src\":{"
                "\"value\":\"%s\","
                "\"type\":\"string\"},"
            "\"modo\":{"
                "\"value\":\"%s\","
                "\"type\":\"string\"}"
            "}",
            IoT_NAME,
            snap.ntc.valor, snap.ntc.maximo,
            snap.ntc.minimo, snap.ntc.nivel_alarma,
            snap.ldr.valor, snap.ldr.maximo,
            snap.ldr.minimo, snap.ldr.nivel_alarma,
            (snap.alarm == ALARM_ACTIVE) ? 'T' : 'F',
            snap.alarma_src,
            IoT_NAME
        );

        // Construir la petición HTTP completa
        snprintf((char *)orion_request, sizeof(orion_request),
            "POST /v2/entities?options=upsert HTTP/1.1\r\n"
            "Host: %s\r\n"
            "Content-Type: application/json\r\n"
            "Accept: application/json\r\n"
            "Content-Length: %d\r\n"
            "\r\n"
            "%s",
            SERVER_IP,
            strlen((char *)orion_body),
            orion_body
        );

        // Esperar acceso exclusivo a COMM_request
        signal = 1;
        do {
            if (xSemaphoreTake(COMM_xSem, 20000 / portTICK_RATE_MS) != pdTRUE) {
                bprintf("ORION: HARAKIRI\r\n");
                HAL_NVIC_SystemReset();
            }
            if (COMM_request.command == 0) {
                COMM_request.command     = 1;
                COMM_request.result      = 0;
                COMM_request.dst_port    = ORION_PORT;
                COMM_request.dst_address = (uint8_t *)SERVER_IP;
                COMM_request.HTTP_request = orion_request;
                signal = 0;
                xSemaphoreGive(COMM_xSem);
            } else {
                xSemaphoreGive(COMM_xSem);
                vTaskDelay(10 / portTICK_RATE_MS);
            }
        } while (signal);

        // Esperar respuesta
        COMM_WAIT_RESULT();

        bprintf("ORION: publicado it=%lu\r\n", global_orion_it);
        COMM_request.result  = 0;
        COMM_request.command = 0;

        global_orion_it++;

        // Publicar cada 3 segundos (mejor latencia de silenciado remoto via Alarma_src)
        vTaskDelay(3000 / portTICK_RATE_MS);
    }
}