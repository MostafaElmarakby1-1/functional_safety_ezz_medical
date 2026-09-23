#ifndef TMU_H
#define TMU_H

#include <stdint.h>

typedef enum
{
    TMU_LED,
    TMU_PWM,
    TMU_UART,
    TMU_TASK_COUNT
} TMU_Task_t;

void TMU_Init(void);
void TMU_Tick(void);

void TMU_StartTask(TMU_Task_t task, uint32_t period_ms);
uint8_t TMU_TaskReady(TMU_Task_t task);

#endif
