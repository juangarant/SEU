#ifndef __NTC_H
#define __NTC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sensor.h"

/** Sensor NTC global accesible desde otras unidades de compilación */
extern volatile sensor_t ntc;

/**
 * @brief Inicializa el sensor NTC (rango 25–30 °C).
 */
void NTC_Init(void);

/**
 * @brief Lee el ADC del NTC, convierte a temperatura (°C) mediante fórmula Beta
 *        y actualiza ntc.valor.
 */
void NTC_Read(void);

#ifdef __cplusplus
}
#endif

#endif /* __NTC_H */
