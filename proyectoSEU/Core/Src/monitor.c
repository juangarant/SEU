/*
 * monitor.c
 */

#include "monitor.h"
#include "stm32f4xx_hal.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include <string.h>
#include "task_CONSOLE.h"
#include "task_CLONE.h"
#include "task_COMM.h"

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
sensor_t sensor_ldr = {0.0f, 0.0f, 100.0f, 75.0f, 0, 0, 0, 0};
sensor_t sensor_ntc = {0.0f, 25.0f, 30.0f, 27.5f, 0, 0, 0, 0};

uint8_t selected_sensor = 0;      // 0 = LDR, 1 = NTC
uint32_t last_pot_value = 0xFFFF; // Para detectar cambios en el potenciómetro

alarm_state_t alarm_state = ALARM_IDLE;
uint32_t alarm_cooldown_start = 0;

uint8_t btn_izq_last = 1;
uint8_t btn_der_last = 1;

uint32_t bajadaIZQ;
uint32_t bajadaDER;
uint32_t subidaIZQ;
uint32_t subidaDER;
uint8_t g_mode;
char alarma_src[24] = "SensorSEU_05";
SemaphoreHandle_t monitor_xMutex = NULL;
static uint32_t mode_show_until = 0;   /* Fase 4: tick hasta el que se muestra el modo */
static int clon_alarm_silenced = 0;   /* Fase 5: buzzer del clon silenciado por boton 2 */


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
    static int      combo_pressed = 0;   /* ambos botones pulsados a la vez */
    static int      combo_fired   = 0;   /* el cambio de modo ya se hizo en este gesto */
    static uint32_t combo_start   = 0;
    uint32_t now = HAL_GetTick();

    uint8_t btn_izq = HAL_GPIO_ReadPin(BTN_IZQ_GPIO_Port, BTN_IZQ_Pin);
    uint8_t btn_der = HAL_GPIO_ReadPin(BTN_DER_GPIO_Port, BTN_DER_Pin);
    /* botones activos a nivel bajo: 0 = pulsado, 1 = soltado */

    /* Combo: ambos botones pulsados mas de 1 s -> cambia de modo */
    if (btn_izq == 0 && btn_der == 0) {
        if (!combo_pressed) {
            combo_pressed = 1;
            combo_fired   = 0;
            combo_start   = now;
        } else if (!combo_fired && (now - combo_start >= 1000)) {
            g_mode      = (g_mode + 1) % 3;   /* 0=conectado 1=clon 2=test */
            combo_fired = 1;
        }
    }

    /* Boton 1 (izquierdo): al soltar, cambia el sensor seleccionado */
    if (btn_izq == 1 && btn_izq_last == 0) {
        if (!combo_pressed)
            selected_sensor = !selected_sensor;
    }

    /* Boton 2 (derecho): al soltar */
    if (btn_der == 1 && btn_der_last == 0) {
        if (!combo_pressed) {
            if (g_mode == MODE_CLON && clone_alarma_activa && !clon_alarm_silenced) {
                /* modo clon: silenciar el buzzer del clon */
                clon_alarm_silenced = 1;
                HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
                CLONE_request_silence();   /* Fase 5: avisar al nodo real via Alarma_src */
            } else if (alarm_state == ALARM_ACTIVE) {
                /* la alarma suena -> apagarla (igual que en el entregable 1) */
                alarm_state = ALARM_COOLDOWN;
                alarm_cooldown_start = now;
                HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
            } else {
                /* la alarma no suena -> mostrar el modo en los LEDs */
                mode_show_until = now + 2000;
            }
        }
    }

    /* el gesto de combo se cierra al soltar AMBOS botones */
    if (btn_izq == 1 && btn_der == 1) {
        combo_pressed = 0;
        combo_fired   = 0;
    }

    btn_izq_last = btn_izq;
    btn_der_last = btn_der;
}

//Funciones públicas

/* ---- Fase 3: display del nodo clonado (modo clon) ---- */
static void Update_Display_Clone(void) {
    static uint32_t flash_t = 0;
    static int      flash_on = 0;
    float value, trip, mn, mx, range, rel, frel;
    uint8_t num_leds, flash_idx;
    int i;
    uint32_t now = HAL_GetTick();

    if (selected_sensor == 0) {        /* LDR del nodo clonado */
        value = clone_ldr;
        trip  = clone_alarma_ldr;
        mn = sensor_ldr.minimo;
        mx = sensor_ldr.maximo;
    } else {                           /* NTC del nodo clonado */
        value = clone_temperatura;
        trip  = clone_alarma_ntc;
        mn = sensor_ntc.minimo;
        mx = sensor_ntc.maximo;
    }

    range = mx - mn;
    if (range <= 0.0f) range = 1.0f;

    rel = value - mn;
    if (rel < 0.0f) rel = 0.0f;
    num_leds = (uint8_t)((rel / range) * 8.0f);
    if (num_leds > 8) num_leds = 8;

    frel = (trip - mn) / range;
    if (frel < 0.0f) frel = 0.0f;
    if (frel > 1.0f) frel = 1.0f;
    flash_idx = (uint8_t)(frel * 7.99f);

    if (now - flash_t > 50) {
        flash_t = now;
        flash_on = !flash_on;
    }

    for (i = 0; i < 8; i++) {
        GPIO_PinState st = GPIO_PIN_RESET;
        if (i == (int)flash_idx)
            st = flash_on ? GPIO_PIN_SET : GPIO_PIN_RESET;
        else if (i < (int)num_leds)
            st = GPIO_PIN_SET;
        HAL_GPIO_WritePin(LED_PORT[i], LED_PIN[i], st);
    }
}

/* ---- Fase 3: secuencia de test (modo test) ---- */
static void Update_Test(void) {
    static int      phase   = 0;
    static int      sweep_i = 0;
    static uint32_t t0      = 0;
    uint32_t now = HAL_GetTick();
    int i;

    switch (phase) {
        case 0:  /* inicio de la secuencia */
            bprintf("\r\n===== MODO TEST =====\r\n");
            for (i = 0; i < 8; i++)
                HAL_GPIO_WritePin(LED_PORT[i], LED_PIN[i], GPIO_PIN_RESET);
            HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
            sweep_i = 0;
            t0 = now;
            phase = 1;
            break;

        case 1:  /* 1) barrido de LEDs */
            if (now - t0 >= 120) {
                t0 = now;
                for (i = 0; i < 8; i++)
                    HAL_GPIO_WritePin(LED_PORT[i], LED_PIN[i],
                                      (i == sweep_i) ? GPIO_PIN_SET : GPIO_PIN_RESET);
                sweep_i++;
                if (sweep_i >= 8) {
                    for (i = 0; i < 8; i++)
                        HAL_GPIO_WritePin(LED_PORT[i], LED_PIN[i], GPIO_PIN_RESET);
                    bprintf("TEST 1: barrido de LEDs OK\r\n");
                    t0 = now;
                    phase = 2;
                }
            }
            break;

        case 2:  /* 2) buzzer */
            HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
            if (now - t0 >= 500) {
                HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
                bprintf("TEST 2: buzzer OK\r\n");
                phase = 3;
            }
            break;

        case 3:  /* 3) canales analogicos: temp, ldr %, pot V */
        {
            uint32_t adc_ldr = ADC_ReadChannel(CH_LDR);
            uint32_t adc_ntc = ADC_ReadChannel(CH_NTC);
            uint32_t adc_pot = ADC_ReadChannel(CH_POT);
            int ldr_pct = (int)(100.0f - (adc_ldr / 4095.0f) * 100.0f);
            int pot_mv  = (int)((adc_pot / 4095.0f) * 3300.0f);
            int temp_c  = 0;
            if (adc_ntc < 4095) {
                float tc = BETA / (logf((-10000.0f * 3.3f /
                           (adc_ntc * 3.3f / 4095.9f - 3.3f) - 10000.0f) / R25)
                           + BETA / T25) - 273.18f;
                temp_c = (int)tc;
            }
            bprintf("TEST 3: Temp=%d C  LDR=%d %%  POT=%d mV\r\n",
                    temp_c, ldr_pct, pot_mv);
            phase = 4;
            break;
        }

        case 4:  /* 4) estado de los botones */
        {
            int bi = HAL_GPIO_ReadPin(BTN_IZQ_GPIO_Port, BTN_IZQ_Pin);
            int bd = HAL_GPIO_ReadPin(BTN_DER_GPIO_Port, BTN_DER_Pin);
            bprintf("TEST 4: BTN_IZQ=%d  BTN_DER=%d (0=pulsado)\r\n", bi, bd);
            phase = 5;
            break;
        }

        case 5:  /* 5) orden AT y su respuesta */
            if (Test_SendAT() == 0)
                bprintf("TEST 5: recurso COMM ocupado\r\n");
            t0 = now;
            phase = 6;
            break;

        case 6:  /* pausa y repeticion de la secuencia */
            if (now - t0 >= 3000)
                phase = 0;
            break;

        default:
            phase = 0;
            break;
    }
}

/* ---- Fase 4: muestra el modo actual en los LEDs (boton 2) ---- */
static void Show_Mode_Leds(void) {
    int i;
    uint8_t n = (uint8_t)(g_mode + 1);   /* conectado=1, clon=2, test=3 */
    for (i = 0; i < 8; i++)
        HAL_GPIO_WritePin(LED_PORT[i], LED_PIN[i],
                          (i < n) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* ---- Fase 5: seleccion del nodo a clonar con el potenciometro ---- */
static void Clone_Select_From_Pot(void) {
    uint32_t pot = ADC_ReadChannel(CH_POT);
    uint8_t  idx = (uint8_t)((pot * 27) / 4096);   /* mapea 0..4095 a 0..26 */
    if (idx > 26) idx = 26;
    CLONE_select_node(idx);
}

/* ---- Fase 5: el buzzer del clon refleja la alarma del nodo clonado ---- */
static void Update_Clone_Alarm(void) {
    if (clone_alarma_activa && !clon_alarm_silenced)
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);

    /* cuando el nodo remoto deja de tener alarma, el clon se rearma */
    if (!clone_alarma_activa)
        clon_alarm_silenced = 0;
}

void Monitor_Init(void) {
    // Asegurar que todo inicie apagado
    for (int i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(LED_PORT[i], LED_PIN[i], GPIO_PIN_RESET);
    }
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);

    g_mode = 0; // Modo inicial

    // Fase 2: mutex que protege el modelo compartido
    monitor_xMutex = xSemaphoreCreateMutex();
}

void Monitor_Loop(void) {
    static uint32_t last_cycle = 0;
    uint32_t current = HAL_GetTick();
    uint8_t  mode;

    /* Ejecutar cada 20 ms */
    if (current - last_cycle >= 20) {
        last_cycle = current;

        /* Botones y lectura del modo: siempre, protegido por mutex */
        Monitor_LockModel();
        Process_Buttons();
        mode = g_mode;
        Monitor_UnlockModel();

        /* Maquina de estados de los 3 modos de funcionamiento */
        switch (mode) {
            case MODE_CONECTADO:
                Monitor_LockModel();
                Update_Sensors();
                Update_Alarm();
                if (HAL_GetTick() < mode_show_until)
                    Show_Mode_Leds();
                else
                    Update_Display();
                Monitor_UnlockModel();
                break;

            case MODE_CLON:
                Clone_Select_From_Pot();
                Monitor_LockModel();
                if (HAL_GetTick() < mode_show_until)
                    Show_Mode_Leds();
                else
                    Update_Display_Clone();
                Update_Clone_Alarm();
                Monitor_UnlockModel();
                break;

            case MODE_TEST:
                Update_Test();
                break;
        }
    }
}

/* ---- Fase 2: acceso protegido al modelo compartido ---- */
void Monitor_LockModel(void) {
    if (monitor_xMutex != NULL)
        xSemaphoreTake(monitor_xMutex, portMAX_DELAY);
}

void Monitor_UnlockModel(void) {
    if (monitor_xMutex != NULL)
        xSemaphoreGive(monitor_xMutex);
}

void Monitor_GetSnapshot(monitor_snapshot_t *snap) {
    if (snap == NULL) return;
    Monitor_LockModel();
    snap->ntc   = sensor_ntc;
    snap->ldr   = sensor_ldr;
    snap->mode  = g_mode;
    snap->alarm = alarm_state;
    memcpy(snap->alarma_src, alarma_src, sizeof(snap->alarma_src));
    Monitor_UnlockModel();
}

uint8_t Monitor_GetMode(void) {
    uint8_t m;
    Monitor_LockModel();
    m = g_mode;
    Monitor_UnlockModel();
    return m;
}

/* ---- Fase 5: el nodo conectado honra una orden de apagado de un nodo clon ---- */
void Monitor_ProcessRemoteAlarmaSrc(const char *value) {
    int changed = 0;
    if (value == NULL || value[0] == 0) return;
    Monitor_LockModel();
    if (strncmp(value, alarma_src, sizeof(alarma_src)) != 0) {
        /* Alarma_src ha cambiado: un nodo clon pide apagar la alarma */
        strncpy(alarma_src, value, sizeof(alarma_src) - 1);
        alarma_src[sizeof(alarma_src) - 1] = 0;
        if (alarm_state == ALARM_ACTIVE) {
            alarm_state = ALARM_COOLDOWN;
            alarm_cooldown_start = HAL_GetTick();
            HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
        }
        changed = 1;
    }
    Monitor_UnlockModel();
    if (changed)
        bprintf("ORION: orden de apagado recibida de un nodo clon (%s)\r\n", value);
}
