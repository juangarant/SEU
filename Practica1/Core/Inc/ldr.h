#ifndef __LDR_H
#define __LDR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sensor.h"

/** Sensor LDR global accesible desde otras unidades de compilación */
extern volatile sensor_t ldr;

/**
 * @brief Inicializa el sensor LDR (rango 0–100 %).
 */
void LDR_Init(void);

/**
 * @brief Lee el ADC del LDR y actualiza ldr.valor en porcentaje [0.0–100.0].
 */
void LDR_Read(void);

#ifdef __cplusplus
}
#endif

#endif /* __LDR_H */
