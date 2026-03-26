/**
 * @file    adc_utils.c
 * @brief   Función genérica de lectura ADC1 por canal (polling, single-conversion).
 */

#include "adc_utils.h"

/* ------------------------------------------------------------------ */
uint32_t ADC_Read_Channel(uint32_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel      = channel;
    sConfig.Rank         = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        return 0U;
    }

    if (HAL_ADC_Start(&hadc1) != HAL_OK)
    {
        return 0U;
    }

    if (HAL_ADC_PollForConversion(&hadc1, 10U) != HAL_OK)
    {
        HAL_ADC_Stop(&hadc1);
        return 0U;
    }

    uint32_t value = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return value;
}
