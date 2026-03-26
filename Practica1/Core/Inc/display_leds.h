#ifndef __DISPLAY_LEDS_H
#define __DISPLAY_LEDS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sensor.h"

/** Frecuencia mínima de parpadeo del LED de nivel de alarma (Hz) */
#define FLASH_FREQ_MIN_HZ   5U
/** Frecuencia máxima de parpadeo del LED de nivel de alarma (Hz) */
#define FLASH_FREQ_MAX_HZ   10U
/** Periodo de parpadeo en ms (usando frecuencia media = 7.5 Hz → ~133 ms) */
#define FLASH_PERIOD_MS     133U   /* 1000 / 7.5 ≈ 133 ms (mitad de período: 67 ms) */

/**
 * @brief Apaga todos los LEDs del display (LED1–LED8).
 */
void Display_Clear(void);

/**
 * @brief Actualiza los 8 LEDs mostrando el valor del sensor de forma gradual
 *        y gestiona el parpadeo del LED correspondiente al nivel de alarma.
 * @param s  Puntero al sensor activo (ldr o ntc).
 */
void Display_Update(volatile sensor_t *s);

#ifdef __cplusplus
}
#endif

#endif /* __DISPLAY_LEDS_H */
