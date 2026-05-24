/*
 * monitor.h
 *
 */

#ifndef INC_MONITOR_H_
#define INC_MONITOR_H_

#include "main.h"

// Estructura sugerida para manejar los sensores
typedef struct {
    float valor;
    float minimo;
    float maximo;
    float nivel_alarma;
    int activated;
    uint32_t time_activation;
    int value_flashing;
    uint32_t flashing_last_time;
} sensor_t;

// Prototipos de funciones públicas
void Monitor_Init(void);
void Monitor_Loop(void);

/* ---- Fase 3: modos de funcionamiento (valores de g_mode) ---- */
#define MODE_CONECTADO  0
#define MODE_CLON       1
#define MODE_TEST       2

extern uint8_t g_mode;
typedef enum { ALARM_IDLE, ALARM_ACTIVE, ALARM_COOLDOWN } alarm_state_t;
extern sensor_t sensor_ldr;
extern sensor_t sensor_ntc;
extern alarm_state_t alarm_state;
extern char alarma_src[24];


/* ---- Fase 2: sincronizacion del modelo compartido ---- */
typedef struct {
    sensor_t      ntc;
    sensor_t      ldr;
    uint8_t       mode;
    alarm_state_t alarm;
    char          alarma_src[24];
} monitor_snapshot_t;

void    Monitor_GetSnapshot(monitor_snapshot_t *snap);
uint8_t Monitor_GetMode(void);
void    Monitor_LockModel(void);
void    Monitor_UnlockModel(void);
void    Monitor_ProcessRemoteAlarmaSrc(const char *value);


#endif /* INC_MONITOR_H_ */