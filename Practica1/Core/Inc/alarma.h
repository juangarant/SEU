#ifndef __ALARMA_H
#define __ALARMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sensor.h"

/** Tiempo de rearme del buzzer tras apagarlo manualmente (ms) */
#define ALARM_REARM_MS    10000U

/** Instante en que se apagó el buzzer (para calcular el rearme) */
extern volatile uint32_t buzzer_off_time;

/** 1 = buzzer en marcha, 0 = buzzer apagado */
extern volatile int buzzer_active;

/** 1 = alarma armada (puede dispararse), 0 = desarmada */
extern volatile int alarm_armed;

/**
 * @brief Evalúa si el sensor supera su nivel de alarma y activa el buzzer.
 *        Si la alarma está desarmada, comprueba si ya han pasado los 10 s de rearme.
 * @param s  Puntero al sensor activo.
 */
void Alarma_Check(volatile sensor_t *s);

/**
 * @brief Apaga el buzzer y guarda el instante de apagado para el rearme.
 */
void Alarma_Off(void);

/**
 * @brief Inicializa las variables de alarma.
 */
void Alarma_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __ALARMA_H */
