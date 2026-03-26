#ifndef __BOTONES_H
#define __BOTONES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/** Tiempo mínimo de estabilidad para considerar un flanco válido (ms) */
#define DEBOUNCE_MS   50U

/**
 * @brief Comprueba BtL (PC7) con debounce por polling.
 *        Detecta flanco de bajada (botón pulsado, pull-up activo).
 * @return 1 si se ha detectado pulsación nueva, 0 en caso contrario.
 */
int BtL_Pressed(void);

/**
 * @brief Comprueba BtR (PB6) con debounce por polling.
 *        Detecta flanco de bajada (botón pulsado, pull-up activo).
 * @return 1 si se ha detectado pulsación nueva, 0 en caso contrario.
 */
int BtR_Pressed(void);

#ifdef __cplusplus
}
#endif

#endif /* __BOTONES_H */
