/**
 * @file    sensor.c
 * @brief   Implementación de funciones comunes para sensor_t.
 */

#include "sensor.h"

/* ------------------------------------------------------------------ */
void Sensor_Init(sensor_t *s, float minimo, float maximo)
{
    s->valor              = minimo;
    s->minimo             = minimo;
    s->maximo             = maximo;
    s->nivel_alarma       = maximo;   /* alarma inicialmente al máximo (no dispara) */
    s->activated          = 0;
    s->time_activation    = 0;
    s->value_flashing     = 0;
    s->flashing_last_time = 0;
}

/* ------------------------------------------------------------------ */
float Sensor_GetPercent(const sensor_t *s)
{
    float rango = s->maximo - s->minimo;
    if (rango <= 0.0f) return 0.0f;

    float pct = (s->valor - s->minimo) / rango;
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 1.0f) pct = 1.0f;
    return pct;
}

/* ------------------------------------------------------------------ */
void Sensor_SetAlarmLevel(sensor_t *s, float pot_pct)
{
    if (pot_pct < 0.0f) pot_pct = 0.0f;
    if (pot_pct > 1.0f) pot_pct = 1.0f;
    s->nivel_alarma = s->minimo + pot_pct * (s->maximo - s->minimo);
}
