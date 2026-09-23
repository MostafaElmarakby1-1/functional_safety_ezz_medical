


#ifndef INC_SAFETY_H_
#define INC_SAFETY_H_

#include "main.h"
#include <stdint.h>

/*----------------------------------------------------------
 * Configuration
 *----------------------------------------------------------*/

/*
 * Maximum number of MPU-fault-induced resets allowed
 * before entering the safe state.
 *
 * With value = 3:
 *   1st fault -> reset
 *   2nd fault -> reset
 *   3rd fault -> reset
 *   4th fault -> safe state
 */
#define SAFETY_MAX_MPU_RESETS       3U

/* Test indication: LED ON time */
#define SAFETY_LED_ON_TIME_MS       2000U

/* Test indication: LED OFF time between pulses */
#define SAFETY_LED_OFF_TIME_MS       500U

/* Period of safety alarm callback */
#define SAFETY_ALARM_PERIOD_MS      1000U


/*----------------------------------------------------------
 * Safety state
 *----------------------------------------------------------*/

typedef enum
{
	SAFETY_STATE_NORMAL = 0,
	SAFETY_STATE_SAFE

} Safety_StateTypeDef;


/*----------------------------------------------------------
 * Alarm callback
 *----------------------------------------------------------*/

/*
 * Function that the Safety module can call periodically
 * while the system is in the safe state.
 *
 * Later this can be connected to your UART alarm frame.
 */
typedef void (*Safety_AlarmCallbackTypeDef)(void);


/*----------------------------------------------------------
 * Public functions
 *----------------------------------------------------------*/

/* Initialize Safety Layer */
void Safety_Init(void);

/* Request reset because of MPU fault */
void Safety_RequestMPUReset(void);

/* Execute periodic safety-state processing */
void Safety_Process(void);

/* Enter latched safe state */
void Safety_EnterSafeState(void);

/* Check whether system is in safe state */
uint8_t Safety_IsSafeState(void);

/* Get stored MPU reset counter */
uint32_t Safety_GetResetCounter(void);

/* Test indication using PC13 */
void Safety_RunTestIndication(void);

/* Register periodic alarm callback */
void Safety_SetAlarmCallback(Safety_AlarmCallbackTypeDef callback);

/* Clear stored safety history */
void Safety_ClearResetHistory(void);

#endif /* INC_SAFETY_H_ */
