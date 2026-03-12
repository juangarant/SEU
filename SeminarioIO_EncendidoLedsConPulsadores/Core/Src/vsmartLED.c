#include "vsmartLED.h"
#include "main.h"
#include <stdio.h>

#define DEBOUNCE_TIME 200  // ms

static uint8_t countL = 0;
static uint8_t countR = 0;

static uint8_t lastStateL = 1;
static uint8_t lastStateR = 1;

static uint32_t lastTimeL = 0;
static uint32_t lastTimeR = 0;


void statusLED(void) {
 uint32_t now = HAL_GetTick();

    uint8_t currentL = HAL_GPIO_ReadPin(btnL_GPIO_Port, btnL_Pin);
    uint8_t currentR = HAL_GPIO_ReadPin(btnR_GPIO_Port, btnR_Pin);

    // --- Botón Izquierdo ---
    if (currentL == GPIO_PIN_RESET && lastStateL == GPIO_PIN_SET)
    {
        if ((now - lastTimeL) > DEBOUNCE_TIME)
        {
            countL++;
            lastTimeL = now;

            if (countL >= 3)
            {
                HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
                countL = 0;
                printf("Encender");
            }
        }
    }
    lastStateL = currentL;

    // --- Botón Derecho ---
    if (currentR == GPIO_PIN_RESET && lastStateR == GPIO_PIN_SET)
    {
        if ((now - lastTimeR) > DEBOUNCE_TIME)
        {
            countR++;
            lastTimeR = now;

            if (countR >= 2)
            {
                lastStateL = 0;
                HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
                countR = 0;
                printf("Apagar");
            }
        }
    }
    lastStateR = currentR;
}