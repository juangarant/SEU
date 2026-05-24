#ifndef INC_TASK_CLONE_H_
#define INC_TASK_CLONE_H_

#include <stdint.h>

extern float   clone_temperatura;
extern float   clone_ldr;
extern float   clone_alarma_ntc;
extern float   clone_alarma_ldr;
extern uint8_t clone_alarma_activa;
extern uint32_t global_clone_it;

void Task_CLONE_init(void);
void Task_CLONE(void *pvParameters);
void CLONE_select_node(uint8_t node_index);
void CLONE_request_silence(void);

#endif