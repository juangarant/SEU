#ifndef __SENSOR_H
#define __SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief Estructura genérica de sensor con gestión de alarma y parpadeo.
 */
typedef struct {
    float valor;                  /**< Valor actual del sensor */
    float minimo;                 /**< Rango mínimo de visualización */
    float maximo;                 /**< Rango máximo de visualización */
    float nivel_alarma;           /**< Nivel de alarma (establecido por POT) */
    int activated;                /**< 1 si alarma disparada, 0 si no */
    uint32_t time_activation;     /**< Instante de activación/desactivación del buzzer (ms HAL) */
    int value_flashing;           /**< Estado del LED de nivel (0=apagado, 1=encendido) */
    uint32_t flashing_last_time;  /**< Último instante en que se cambió el LED de parpadeo (ms HAL) */
} sensor_t;

/**
 * @brief Inicializa los campos de un sensor_t con valores por defecto.
 * @param s        Puntero al sensor a inicializar.
 * @param minimo   Valor mínimo del rango.
 * @param maximo   Valor máximo del rango.
 */
void Sensor_Init(sensor_t *s, float minimo, float maximo);

/**
 * @brief Calcula el porcentaje (0.0–1.0) del valor del sensor respecto a su rango.
 * @param s  Puntero al sensor.
 * @return   Porcentaje normalizado en [0.0, 1.0].
 */
float Sensor_GetPercent(const sensor_t *s);

/**
 * @brief Actualiza el nivel de alarma a partir de un porcentaje del potenciómetro.
 * @param s       Puntero al sensor.
 * @param pot_pct Porcentaje del potenciómetro en [0.0, 1.0].
 */
void Sensor_SetAlarmLevel(sensor_t *s, float pot_pct);

#ifdef __cplusplus
}
#endif

#endif /* __SENSOR_H */
