#ifndef __POTENCIOMETRO_H
#define __POTENCIOMETRO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sensor.h"

/**
 * @brief Lee el ADC del potenciómetro y actualiza el nivel de alarma
 *        del sensor actualmente seleccionado.
 * @param active_sensor  Puntero al sensor activo (ldr o ntc).
 */
void POT_UpdateAlarmLevel(volatile sensor_t *active_sensor);

#ifdef __cplusplus
}
#endif

#endif /* __POTENCIOMETRO_H */
