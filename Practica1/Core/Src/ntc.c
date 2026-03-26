/**
 * @file    ntc.c
 * @brief   Lectura y conversión del sensor NTC a temperatura (°C).
 *          Canal ADC1_IN1 → PA1 (etiqueta NTC).
 *          Fórmula: ecuación Beta con R25=10kΩ, T25=298.15K, Beta=3900.
 */

#include "ntc.h"
#include "adc_utils.h"
#include "stm32f4xx_hal.h"
#include <math.h>

/* Parámetros del termistor NTC */
#define NTC_R25    10000.0f   /**< Resistencia a 25 °C (Ω) */
#define NTC_T25    298.15f    /**< Temperatura de referencia (K) */
#define NTC_BETA   3900.0f    /**< Coeficiente Beta del termistor */

/* Variable global del sensor NTC */
volatile sensor_t ntc;

/* ------------------------------------------------------------------ */
void NTC_Init(void)
{
    /* Rango de visualización: 25 °C (mínimo) – 30 °C (máximo) */
    Sensor_Init((sensor_t *)&ntc, 25.0f, 30.0f);
}

/* ------------------------------------------------------------------ */
void NTC_Read(void)
{
    uint32_t raw = ADC_Read_Channel(ADC_CHANNEL_1);   /* PA1 = ADC1_IN1 */

    /* Evitar división por cero o logaritmo de valor negativo */
    if (raw == 0U || raw >= 4095U)
    {
        return;
    }

    /* Tensión medida: Vadc = raw * 3.3 / 4095 */
    float vadc = (float)raw * 3.3f / 4095.0f;

    /*
     * Divisor resistivo: Vcc=3.3V, R_serie=10kΩ conectado a GND,
     * NTC conectado a Vcc.
     *   V_adc = Vcc * R_serie / (R_ntc + R_serie)
     *   R_ntc = R_serie * (Vcc / V_adc - 1)
     *
     * Expresión equivalente al enunciado:
     *   R_ntc = -10000 * 3.3 / (vadc - 3.3) - 10000
     *         = 10000 * 3.3 / (3.3 - vadc) - 10000
     */
    float r_ntc = -10000.0f * 3.3f / (vadc - 3.3f) - 10000.0f;

    if (r_ntc <= 0.0f)
    {
        return;
    }

    /* Ecuación Beta: T (K) = Beta / (ln(R_ntc / R25) + Beta / T25) */
    float tmp = NTC_BETA /
                (logf(r_ntc / NTC_R25) + NTC_BETA / NTC_T25)
                - 273.15f;

    ntc.valor = tmp;
}
