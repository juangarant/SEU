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
#include "main.h"
#include "task_EJER3.h"

#include <time.h>
#include "task_TIME.h"

uint32_t global_ejer3_it;

void Task_EJER3_init(void){
	BaseType_t res_task;

	global_ejer3_it=0;

	res_task=xTaskCreate(Task_EJER3,"EJER3",2048,NULL,	NORMAL_PRIORITY,NULL);
 	if( res_task != pdPASS ){
 	 				printf("PANIC: Error al crear Tarea Ejer3\r\n");
 	 				fflush(NULL);
 	 				while(1);
 	}
}

void Task_EJER3( void *pvParameters ){

	int contador = 0;
	//ALUMNO Rellenar Ejercicio 3
	bprintf("proyectoSEU at " __TIME__ "\r\n");

	while (1) {
		//ALUMNO Rellenar Ejercicio 3
		vTaskDelay(10000 / portTICK_RATE_MS);
        global_ejer3_it++;

        if (task_TIME_timeAvailable()) {
            time_t t = task_TIME_getTime();
            struct tm *tm_i = gmtime(&t);
            bprintf("[%02d:%02d:%02d] Cnt=%d\r\n",
                    tm_i->tm_hour,
                    tm_i->tm_min,
                    tm_i->tm_sec,
                    contador++);
        } else {
            bprintf("Hora no disponible. Cnt=%d\r\n", contador++);
		}
	}
}
