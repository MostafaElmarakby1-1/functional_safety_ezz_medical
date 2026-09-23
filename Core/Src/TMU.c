#include "TMU.h"

typedef struct
{
	uint32_t period_ms;
	uint32_t counter_ms;
	uint8_t active;
	uint8_t ready;
} TMU_TaskControl_t;

static TMU_TaskControl_t tasks[TMU_TASK_COUNT];

void TMU_Init(void)
{
	for (uint8_t i = 0; i < TMU_TASK_COUNT; i++)
	{
		tasks[i].period_ms = 0;
		tasks[i].counter_ms = 0;
		tasks[i].active = 0;
		tasks[i].ready = 0;
	}
}

void TMU_StartTask(TMU_Task_t task, uint32_t period_ms)
{
	if (task >= TMU_TASK_COUNT || period_ms == 0)
	{
		return;
	}

	tasks[task].period_ms = period_ms;
	tasks[task].counter_ms = 0;
	tasks[task].active = 1;
	tasks[task].ready = 0;
}

void TMU_Tick(void)
{
	for (uint8_t i = 0; i < TMU_TASK_COUNT; i++)
	{
		if (tasks[i].active)
		{
			tasks[i].counter_ms++;

			if (tasks[i].counter_ms >= tasks[i].period_ms)
			{
				tasks[i].counter_ms = 0;
				tasks[i].ready = 1;
			}
		}
	}
}

uint8_t TMU_TaskReady(TMU_Task_t task)
{
	if (task >= TMU_TASK_COUNT)
	{
		return 0;
	}

	if (tasks[task].ready)
	{
		tasks[task].ready = 0;
		return 1;
	}

	return 0;
}
