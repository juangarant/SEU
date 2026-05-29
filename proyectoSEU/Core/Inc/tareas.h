/*
 * tareas.h
 *
 *  Created on: Apr 29, 2021
 *      Author: pperez
 */

#ifndef __TAREAS_H__
#define __TAREAS_H__

#include <stdint.h>
#include "main.h"
#include <tareas.h>
#include <main.h>
#include "FreeRTOS.h"
#include <stdio.h>
#include "cmsis_os.h"
#include <stdlib.h>

#include "semphr.h"
#include <string.h>

#include <task.h>
#include <math.h>




//operativa
#define FREERTOS_5SEG		(5000/portTICK_PERIOD_MS)

// Orion Context IoT name
#define IoT_NAME   	 	  "SensorSEU_70"
#define IoT_NAME_CLONE    "SensorSEU_ZL"
#define SERVER_IP  "192.168.1.166"
#define ORION_PORT  1026
#define TIME_PORT   5000

/* ---- Modos de funcionamiento (valores que toma g_mode) ---- */
#define MODE_CONECTADO  0   /* publica sensores en el broker Orion        */
#define MODE_CLON       1   /* clona los datos de otro nodo SensorSEU_XX  */
#define MODE_TEST       2   /* secuencia de auto-test del hardware        */

// seminario 6

#define HIGH_PRIORITY (( configMAX_PRIORITIES - 1 )>>1)+1
#define NORMAL_PRIORITY (( configMAX_PRIORITIES - 1 )>>1)
#define LOW_PRIORITY (( configMAX_PRIORITIES - 1 )>>1)-1

void CONFIGURACION_INICIAL(void);

#endif
