/**
 * @file    display_leds.c
 * @brief   Visualización gradual del valor del sensor en el array de 8 LEDs
 *          y parpadeo del LED correspondiente al nivel de alarma.
 *
 *  LEDs ordenados de menor (LED1) a mayor (LED8):
 *    LED1 → PB4    LED2 → PB10   LED3 → PA8    LED4 → PB5
 *    LED5 → PB3    LED6 → PA6    LED7 → PB0    LED8 → PA5
 */

#include "display_leds.h"
#include "main.h"

/* ------------------------------------------------------------------ */
/* Tabla de descriptores de los 8 LEDs en orden ascendente (LED1→LED8) */
/* ------------------------------------------------------------------ */
typedef struct {
    GPIO_TypeDef *port;
    uint16_t      pin;
} led_desc_t;

static const led_desc_t leds[8] = {
    { LED1_GPIO_Port, LED1_Pin },   /* LED1 – PB4 */
    { LED2_GPIO_Port, LED2_Pin },   /* LED2 – PB10 */
    { LED3_GPIO_Port, LED3_Pin },   /* LED3 – PA8 */
    { LED4_GPIO_Port, LED4_Pin },   /* LED4 – PB5 */
    { LED5_GPIO_Port, LED5_Pin },   /* LED5 – PB3 */
    { LED6_GPIO_Port, LED6_Pin },   /* LED6 – PA6 */
    { LED7_GPIO_Port, LED7_Pin },   /* LED7 – PB0 */
    { LED8_GPIO_Port, LED8_Pin },   /* LED8 – PA5 */
};

/* ------------------------------------------------------------------ */
void Display_Clear(void)
{
    for (int i = 0; i < 8; i++)
    {
        HAL_GPIO_WritePin(leds[i].port, leds[i].pin, GPIO_PIN_RESET);
    }
}

/* ------------------------------------------------------------------ */
/**
 * @brief Calcula el índice del LED (0–7) que corresponde al nivel de alarma.
 *        El LED de nivel es el último encendido según el porcentaje del nivel.
 * @param s  Puntero al sensor activo.
 * @return   Índice del LED de nivel [0–7].
 */
static int Display_AlarmLedIndex(const volatile sensor_t *s)
{
    float rango = s->maximo - s->minimo;
    if (rango <= 0.0f) return 0;

    float pct = (s->nivel_alarma - s->minimo) / rango;
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 1.0f) pct = 1.0f;

    int idx = (int)(pct * 8.0f);
    if (idx > 7) idx = 7;
    if (idx < 0) idx = 0;
    return idx;
}

/* ------------------------------------------------------------------ */
void Display_Update(volatile sensor_t *s)
{
    /* ---- Calcular cuántos LEDs deben estar encendidos ---- */
    float pct = Sensor_GetPercent((const sensor_t *)s);
    int leds_on = (int)(pct * 8.0f + 0.5f);   /* Redondeo */
    if (leds_on > 8) leds_on = 8;
    if (leds_on < 0) leds_on = 0;

    /* ---- Índice del LED de nivel de alarma ---- */
    int alarm_idx = Display_AlarmLedIndex(s);

    /* ---- Gestión del parpadeo del LED de nivel ---- */
    uint32_t now     = HAL_GetTick();
    uint32_t elapsed = now - s->flashing_last_time;

    if (elapsed >= (FLASH_PERIOD_MS / 2U))
    {
        s->value_flashing     = !s->value_flashing;
        s->flashing_last_time = now;
    }

    /* ---- Escribir los 8 LEDs ---- */
    for (int i = 0; i < 8; i++)
    {
        if (i == alarm_idx)
        {
            /* LED de nivel: parpadea siempre independientemente del valor */
            HAL_GPIO_WritePin(leds[i].port, leds[i].pin,
                              s->value_flashing ? GPIO_PIN_SET : GPIO_PIN_RESET);
        }
        else
        {
            /* LEDs de valor: encendidos según el porcentaje del sensor */
            HAL_GPIO_WritePin(leds[i].port, leds[i].pin,
                              (i < leds_on) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        }
    }
}
