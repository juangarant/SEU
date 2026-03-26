/**
 * @file    potenciometro.c
 * @brief   Lectura del potenciómetro y actualización del nivel de alarma
 *          del sensor actualmente seleccionado.
 *          Canal ADC1_IN4 → PA4 (etiqueta POT).
 */

#include "potenciometro.h"
#include "adc_utils.h"
#include "stm32f4xx_hal.h"

/* ------------------------------------------------------------------ */
void POT_UpdateAlarmLevel(volatile sensor_t *active_sensor)
{
    uint32_t raw = ADC_Read_Channel(ADC_CHANNEL_4);   /* PA4 = ADC1_IN4 */
    float pot_pct = (float)raw / 4095.0f;             /* Normalizado [0.0–1.0] */
    Sensor_SetAlarmLevel((sensor_t *)active_sensor, pot_pct);
}
