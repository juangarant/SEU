/**
 * @file    botones.c
 * @brief   Lectura de BtL (PC7) y BtR (PB6) con debounce por polling
 *          y detección de flanco descendente (pull-up, activo en LOW).
 */

#include "botones.h"
#include "main.h"

/* ------------------------------------------------------------------ */
/* Estado interno de cada botón                                        */
/* ------------------------------------------------------------------ */
typedef struct {
    GPIO_PinState last_raw;      /**< Última lectura sin filtrar */
    GPIO_PinState stable_state;  /**< Estado estable después del debounce */
    uint32_t      last_change;   /**< Tiempo en ms del último cambio de estado raw */
    int           reported;      /**< 1 = ya se notificó esta pulsación */
} btn_state_t;

static btn_state_t btl_state = { GPIO_PIN_SET, GPIO_PIN_SET, 0, 0 };
static btn_state_t btr_state = { GPIO_PIN_SET, GPIO_PIN_SET, 0, 0 };

/* ------------------------------------------------------------------ */
/**
 * @brief Lógica común de debounce + detección de flanco para un botón.
 * @param st    Puntero al estado interno del botón.
 * @param port  Puerto GPIO del botón.
 * @param pin   Pin del botón.
 * @return 1 si se detecta un nuevo flanco descendente estable, 0 si no.
 */
static int Button_Check(btn_state_t *st, GPIO_TypeDef *port, uint16_t pin)
{
    GPIO_PinState current = HAL_GPIO_ReadPin(port, pin);
    uint32_t now = HAL_GetTick();

    /* Detectar cambio en la lectura raw */
    if (current != st->last_raw)
    {
        st->last_raw    = current;
        st->last_change = now;
        st->reported    = 0;   /* Reiniciar flag al detectar cambio */
    }

    /* Esperar DEBOUNCE_MS antes de aceptar el nuevo estado */
    if (!st->reported &&
        ((now - st->last_change) >= DEBOUNCE_MS))
    {
        if (current != st->stable_state)
        {
            st->stable_state = current;

            /* Flanco descendente = botón pulsado (pull-up → LOW) */
            if (current == GPIO_PIN_RESET)
            {
                st->reported = 1;
                return 1;
            }
        }
        st->reported = 1;
    }

    return 0;
}

/* ------------------------------------------------------------------ */
int BtL_Pressed(void)
{
    return Button_Check(&btl_state, BtL_GPIO_Port, BtL_Pin);
}

/* ------------------------------------------------------------------ */
int BtR_Pressed(void)
{
    return Button_Check(&btr_state, BtR_GPIO_Port, BtR_Pin);
}
