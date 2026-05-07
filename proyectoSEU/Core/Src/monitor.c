/*
 * monitor.c
 */

#include "monitor.h"
#include "stm32f4xx_hal.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

// constantes NTC
#define R25    10000.0f
#define T25    298.15f
#define BETA   3900.0f

// canales
#define CH_LDR ADC_CHANNEL_0
#define CH_NTC ADC_CHANNEL_1
#define CH_POT ADC_CHANNEL_4

extern ADC_HandleTypeDef hadc1;

// variables 
sensor_t sensor_ldr = {0.0f, 0.0f, 100.0f, 50.0f, 0, 0, 0, 0};
sensor_t sensor_ntc = {0.0f, 25.0f, 30.0f, 27.5f, 0, 0, 0, 0};

uint8_t selected_sensor = 0;      // 0 = LDR, 1 = NTC
uint32_t last_pot_value = 0xFFFF; // Para detectar cambios en el potenciómetro

typedef enum { ALARM_IDLE, ALARM_ACTIVE, ALARM_COOLDOWN } alarm_state_t;
alarm_state_t alarm_state = ALARM_IDLE;
uint32_t alarm_cooldown_start = 0;

uint8_t btn_izq_last = 1;
uint8_t btn_der_last = 1;

uint32_t bajadaIZQ;
uint32_t bajadaDER;
uint32_t subidaIZQ;
uint32_t subidaDER;
uint8_t g_mode;


//leds
/*GPIO_TypeDef* LED_PORT[8] = {LED1_GPIO_Port, LED2_GPIO_Port, LED3_GPIO_Port, LED4_GPIO_Port, 
                             LED5_GPIO_Port, LED6_GPIO_Port, LED7_GPIO_Port, LED8_GPIO_Port};
uint16_t LED_PIN[8]       = {LED1_Pin, LED2_Pin, LED3_Pin, LED4_Pin, 
                             LED5_Pin, LED6_Pin, LED7_Pin, LED8_Pin};*/

GPIO_TypeDef* LED_PORT[8] = {LED8_GPIO_Port, LED7_GPIO_Port, LED6_GPIO_Port, LED5_GPIO_Port, 
                             LED4_GPIO_Port, LED3_GPIO_Port, LED2_GPIO_Port, LED1_GPIO_Port};
uint16_t LED_PIN[8]       = {LED8_Pin, LED7_Pin, LED6_Pin, LED5_Pin, 
                             LED4_Pin, LED3_Pin, LED2_Pin, LED1_Pin};                      

//Funciones privadas

//Leer canal

static uint32_t ADC_ReadChannel(uint32_t channel) {
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = channel;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES; // Mayor tiempo para estabilidad
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    uint32_t val = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return val;
}

//Actualizar sensores y alarma

static void Update_Sensors(void) {
    // Muestreo LDR
    uint32_t adc_ldr = ADC_ReadChannel(CH_LDR);
    //sensor_ldr.valor = (adc_ldr / 4095.0f) * 100.0f;
    sensor_ldr.valor = 100.0f - ((adc_ldr / 4095.0f) * 100.0f);

    // Muestreo NTC 
    uint32_t adc_ntc = ADC_ReadChannel(CH_NTC);
    if (adc_ntc < 4095) {
        float temp = BETA / (logf((-10000.0f * 3.3f / (adc_ntc * 3.3f / 4095.9f - 3.3f) - 10000.0f) / R25) + BETA / T25) - 273.18f;
        sensor_ntc.valor = temp;
    }

    // ajustar el Nivel de Alarma
    uint32_t adc_pot = ADC_ReadChannel(CH_POT);
    if (abs((int)adc_pot - (int)last_pot_value) > 30) { // Histéresis de ruido
        last_pot_value = adc_pot;
        float pot_pct = adc_pot / 4095.0f;
        
        if (selected_sensor == 0) { 
            sensor_ldr.nivel_alarma = sensor_ldr.minimo + pot_pct * (sensor_ldr.maximo - sensor_ldr.minimo);
        } else { 
            sensor_ntc.nivel_alarma = sensor_ntc.minimo + pot_pct * (sensor_ntc.maximo - sensor_ntc.minimo);
        }
    }
}

//leds y parpadeo alarma
static void Update_Display(void) {
    uint8_t num_leds_on = 0;
    sensor_t* active = (selected_sensor == 0) ? &sensor_ldr : &sensor_ntc;

    //cuantos LEDs encender
    float range = active->maximo - active->minimo;
    float relative_val = active->valor - active->minimo;
    if (relative_val < 0) relative_val = 0;
    num_leds_on = (uint8_t)((relative_val / range) * 8.0f);
    if (num_leds_on > 8) num_leds_on = 8;

    // calcular LED debe parpadear
    float relative_alarm = active->nivel_alarma - active->minimo;
    uint8_t flash_index = (uint8_t)((relative_alarm / range) * 7.99f);

    //parpadeo a 10Hz
    uint32_t current_tick = HAL_GetTick();
    if (current_tick - active->flashing_last_time > 50) {
        active->flashing_last_time = current_tick;
        active->value_flashing = !active->value_flashing;
    }

    // salida de los LEDs
    for (int i = 0; i < 8; i++) {
        GPIO_PinState state = GPIO_PIN_RESET;
        
        if (i == flash_index) {
            state = active->value_flashing ? GPIO_PIN_SET : GPIO_PIN_RESET;
        } else if (i < num_leds_on) {
            state = GPIO_PIN_SET;
        }
        HAL_GPIO_WritePin(LED_PORT[i], LED_PIN[i], state);
    }
}

//alarma

static void Update_Alarm(void) {
    uint32_t current_tick = HAL_GetTick();

    switch (alarm_state) {
        case ALARM_IDLE:
            if ((sensor_ldr.valor > sensor_ldr.nivel_alarma) || (sensor_ntc.valor > sensor_ntc.nivel_alarma)) {
                alarm_state = ALARM_ACTIVE;
                HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
            }
            break;

        case ALARM_ACTIVE:
            // Se mantiene en este estado hasta pulsar el botón derecho
            break;

        case ALARM_COOLDOWN:
            if ((current_tick - alarm_cooldown_start) > 10000) { // Rearme tras 10s
                alarm_state = ALARM_IDLE;
            }
            break;
    }
}

//pulsar de botones

static void Process_Buttons(void) {
    

    uint8_t btn_izq = HAL_GPIO_ReadPin(BTN_IZQ_GPIO_Port, BTN_IZQ_Pin);
    uint8_t btn_der = HAL_GPIO_ReadPin(BTN_DER_GPIO_Port, BTN_DER_Pin);

    //Bajada
    if (btn_izq == 0 && btn_izq_last == 1) {
        bajadaIZQ = HAL_GetTick();
    }

    if (btn_der == 0 && btn_der_last == 1) {
        bajadaDER = HAL_GetTick();
    }

    // Cambiar sensor
    if (btn_izq == 1 && btn_izq_last == 0) {
        uint32_t subidaIZQ =HAL_GetTick();
        if (subidaIZQ - bajadaIZQ < 2000) {
            selected_sensor = !selected_sensor;
        }
    }
    
    

    //Apagar alarma y entrar en cooldown
    if (btn_der == 1 && btn_der_last == 0) {
        subidaDER = HAL_GetTick();
        if (subidaDER - bajadaDER < 2000) {
            if (alarm_state == ALARM_ACTIVE) {
            alarm_state = ALARM_COOLDOWN;
            alarm_cooldown_start = HAL_GetTick();
            HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
            }
        }
    }

    if (btn_izq == 1 && btn_izq_last == 0 && btn_der == 1 && btn_der_last == 0) {
        if (subidaIZQ - bajadaIZQ >= 2000 && subidaDER - bajadaDER >= 2000) {
            g_mode = (g_mode + 1) % 3; // Cambia entre modos 0, 1 y 2
        }
    }   

    btn_izq_last = btn_izq;
    btn_der_last = btn_der;
}

//Funciones públicas

void Monitor_Init(void) {
    // Asegurar que todo inicie apagado
    for (int i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(LED_PORT[i], LED_PIN[i], GPIO_PIN_RESET);
    }
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);

    g_mode = 0; // Modo inicial
}

void Monitor_Loop(void) {
    static uint32_t last_cycle = 0;
    uint32_t current = HAL_GetTick();

    // Ejecutar cada 20 ms
    if (current - last_cycle >= 20) {
        last_cycle = current;

        Process_Buttons();
        Update_Sensors();
        Update_Alarm();
        Update_Display();
    }
}