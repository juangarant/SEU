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

uint32_t global_orion_it;

static uint8_t orion_body[1024];
static uint8_t orion_request[1280];

void Task_ORION_init(void) {
    BaseType_t res;
    global_orion_it = 0;

    res = xTaskCreate(Task_ORION, "ORION", 2048, NULL, NORMAL_PRIORITY, NULL);
    if (res != pdPASS) {
        bprintf("PANIC: Error al crear Tarea ORION\r\n");
        while(1);
    }
}

void Task_ORION(void *pvParameters) {

    int signal;

    while (1) {

        // Solo publicar en modo conectado
        if (g_mode != 0) {
            vTaskDelay(2000 / portTICK_RATE_MS);
            continue;
        }

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
            sensor_ntc.valor, sensor_ntc.maximo,
            sensor_ntc.minimo, sensor_ntc.nivel_alarma,
            sensor_ldr.valor, sensor_ldr.maximo,
            sensor_ldr.minimo, sensor_ldr.nivel_alarma,
            (alarm_state == ALARM_ACTIVE) ? 'T' : 'F',
            alarma_src,
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
                COMM_request.dst_port    = 1026;
                COMM_request.dst_address = (uint8_t *)"158.42.180.130";
                COMM_request.HTTP_request = orion_request;
                signal = 0;
                xSemaphoreGive(COMM_xSem);
            } else {
                xSemaphoreGive(COMM_xSem);
                vTaskDelay(10 / portTICK_RATE_MS);
            }
        } while (signal);

        // Esperar respuesta
        while (COMM_request.result != 1)
            vTaskDelay(10 / portTICK_RATE_MS);

        bprintf("ORION: publicado it=%lu\r\n", global_orion_it);
        COMM_request.result  = 0;
        COMM_request.command = 0;

        global_orion_it++;

        // Publicar cada 10 segundos
        vTaskDelay(10000 / portTICK_RATE_MS);
    }
}