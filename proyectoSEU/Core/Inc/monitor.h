/*
 * monitor.h
 *
 */

#ifndef INC_MONITOR_H_
#define INC_MONITOR_H_

#include "main.h"

// Estructura sugerida para manejar los sensores
typedef struct {
    float valor;
    float minimo;
    float maximo;
    float nivel_alarma;
    int activated;
    uint32_t time_activation;
    int value_flashing;
    uint32_t flashing_last_time;
} sensor_t;

// Prototipos de funciones públicas
void Monitor_Init(void);
void Monitor_Loop(void);

#endif /* INC_MONITOR_H_ */