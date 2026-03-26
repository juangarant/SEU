/**
 * @file    alarma.c
 * @brief   Lógica de disparo, apagado y rearme automático del buzzer.
 *          Buzzer en PA7 (etiqueta Buzzer_Pin / BUZZER_Pin).
 */

#include "alarma.h"
#include "main.h"

/* ------------------------------------------------------------------ */
/* Variables globales de estado de alarma                              */
/* ------------------------------------------------------------------ */
volatile uint32_t buzzer_off_time = 0;
volatile int        buzzer_active   = 0;
volatile int        alarm_armed     = 1;   /* Arranca armada */

/* ------------------------------------------------------------------ */
void Alarma_Init(void)
{
    buzzer_active = 0;
    alarm_armed   = 1;
    buzzer_off_time = 0;
    /* Asegurar buzzer apagado al inicio */
    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
}

/* ------------------------------------------------------------------ */
void Alarma_Check(volatile sensor_t *s)
{
    /* Si la alarma está desarmada, comprobar si ha pasado el tiempo de rearme */
    if (!alarm_armed)
    {
        uint32_t elapsed = HAL_GetTick() - buzzer_off_time;
        if (elapsed >= ALARM_REARM_MS)
        {
            alarm_armed = 1;
        }
        else
        {
            return;   /* Aún no rearmar */
        }
    }

    /* Evaluar si el sensor supera el nivel de alarma */
    if (s->valor > s->nivel_alarma)
    {
        if (!buzzer_active)
        {
            buzzer_active      = 1;
            s->activated       = 1;
            s->time_activation = HAL_GetTick();
            HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET);
        }
    }
    /* Nota: el buzzer NO se apaga aunque el sensor baje — queda activo hasta BtR */
}

/* ------------------------------------------------------------------ */
void Alarma_Off(void)
{
    buzzer_active   = 0;
    alarm_armed     = 0;   /* Iniciar cuenta de rearme */
    buzzer_off_time = HAL_GetTick();
    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
}
