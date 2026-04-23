/**
 * @file    ldr.c
 * @brief   Lectura y conversión del sensor LDR (porcentaje de luz 0–100 %).
 *          Canal ADC1_IN0 → PA0 (etiqueta LDR).
 */

#include "ldr.h"
#include "adc_utils.h"
#include "stm32f4xx_hal.h"

/* Calibración empírica del montaje (ajustable a mano) */
#define LDR_LIGHT_MIN_PCT   50.0f   /* valor con LDR tapada */
#define LDR_LIGHT_MAX_PCT   97.0f   /* valor con linterna */

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
    /*
     * Conversión invertida: raw [0–4095] → porcentaje [100.0–0.0]
     * Así, con este montaje, más luz => valor más alto => más LEDs encendidos.
     */
    float light_pct = 100.0f - ((float)raw * 100.0f / 4095.0f);

    /* Reescalar [50..97] => [0..100] y saturar */
    float norm = (light_pct - LDR_LIGHT_MIN_PCT) /
                 (LDR_LIGHT_MAX_PCT - LDR_LIGHT_MIN_PCT);

    if (norm < 0.0f) norm = 0.0f;
    if (norm > 1.0f) norm = 1.0f;

    ldr.valor = norm * 100.0f;
}
