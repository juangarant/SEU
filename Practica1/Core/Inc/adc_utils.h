#ifndef __ADC_UTILS_H
#define __ADC_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/** Handle del ADC1, declarado en main.c / MX_ADC1_Init */
extern ADC_HandleTypeDef hadc1;

/**
 * @brief Lee un canal analógico del ADC1 en modo polling single-conversion.
 *        Reconfigura el canal antes de iniciar la conversión.
 * @param channel  Canal ADC (ej. ADC_CHANNEL_0, ADC_CHANNEL_1, ADC_CHANNEL_4).
 * @return         Valor raw 12 bits [0–4095], o 0 en caso de error.
 */
uint32_t ADC_Read_Channel(uint32_t channel);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_UTILS_H */
