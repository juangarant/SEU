/*
 * Task_COMM.h
 *
 *  Created on: 24 may. 2023
 *      Author: pperez
 */

#ifndef INC_TASK_COMM_H_
#define INC_TASK_COMM_H_


#include "FreeRTOS.h"
#include <stdio.h>
#include "cmsis_os.h"
#include <stdlib.h>

#include "semphr.h"

// internal

void cleanResponse(uint8_t * data,int maxlen);

typedef struct REQUEST_DUMMY {
								int32_t  command;
								int32_t  result;
								int32_t  dst_port;
								uint8_t * 	 dst_address;
								uint8_t * 	 HTTP_request;
								uint8_t *   HTTP_response;
} scomm_request_t;



extern scomm_request_t COMM_request;
extern SemaphoreHandle_t COMM_xSem;
extern volatile int global_wifi_ready;   /* 1 = ESP unido a la red WiFi */

/* Espera ACOTADA a que Task_COMM marque result==1. Evita cuelgues permanentes
   (p.ej. si COMM esta reconectando el WiFi): tras ~30 s deja de esperar y el
   codigo posterior libera command/result como de costumbre. */
#define COMM_WAIT_RESULT()                                              \
	do {                                                               \
		int _g = 0;                                                    \
		while (COMM_request.result != 1) {                             \
			vTaskDelay(10 / portTICK_RATE_MS);                         \
			if (++_g > 3000) break;                                    \
		}                                                              \
	} while (0)

#include "main.h"
#include <stdint.h>
#include "FreeRTOS.h"



extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart1;

extern uint8_t buff_recv[2048];


// WIFI
#define SSID			  "Tus Queridos Vecinos_EXT"
#define SSID_PASS		  "dame el wifi"

void WIFI_RESET(void);
void WIFI_Boot(void);
void WIFI_Boot_TEST(void);

uint8_t * ESP_Send_Request(uint8_t * dst_address, uint32_t dst_port, uint8_t * request);

extern uint32_t global_comm_it;
void Task_COMM_init(void);
void Task_COMM( void *pvParameters );
int Test_SendAT(void);


#endif /* INC_TASK_COMM_H_ */
