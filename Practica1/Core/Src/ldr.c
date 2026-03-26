/**
 * @file    ldr.c
 * @brief   Lectura y conversión del sensor LDR (porcentaje de luz 0–100 %).
 *          Canal ADC1_IN0 → PA0 (etiqueta LDR).
 */

#include "ldr.h"
#include "adc_utils.h"
#include "stm32f4xx_hal.h"

/* Variable global del sensor LDR */
volatile sensor_t ldr;

/* ------------------------------------------------------------------ */
void LDR_Init(void)
{
    Sensor_Init((sensor_t *)&ldr, 0.0f, 100.0f);
}

/* ------------------------------------------------------------------ */
void LDR_Read(void)
{
    uint32_t raw = ADC_Read_Channel(ADC_CHANNEL_0);   /* PA0 = ADC1_IN0 */
    /* Conversión lineal: raw [0–4095] → porcentaje [0.0–100.0] */
    ldr.valor = (float)raw * 100.0f / 4095.0f;
}
