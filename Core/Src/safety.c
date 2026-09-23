#include "Safety.h"


/*----------------------------------------------------------
 * Backup-register definitions
 *----------------------------------------------------------*/

/*
 * RTC backup register 0:
 * Persistent marker indicating that the previous reset
 * was intentionally requested because of an MPU fault.
 */
#define SAFETY_BKP_FAULT_MARKER      RTC->BKP0R

/*
 * RTC backup register 1:
 * Persistent MPU reset counter.
 */
#define SAFETY_BKP_RESET_COUNTER     RTC->BKP1R


/*
 * Magic value used to identify a valid MPU-fault marker.
 */
#define SAFETY_MPU_FAULT_MAGIC       0x4D505546UL


/*----------------------------------------------------------
 * Internal variables
 *----------------------------------------------------------*/

static volatile Safety_StateTypeDef Safety_State =
		SAFETY_STATE_NORMAL;

static volatile uint8_t Safety_MPUFaultFlag = 0U;

static Safety_AlarmCallbackTypeDef Safety_AlarmCallback = NULL;

static uint32_t Safety_LastAlarmTick = 0U;


/*----------------------------------------------------------
 * Internal backup-register functions
 *----------------------------------------------------------*/

static uint32_t Safety_BackupRead(volatile uint32_t *register_address)
{
	uint32_t value;

	/*
	 * Enable PWR peripheral clock.
	 */
	__HAL_RCC_PWR_CLK_ENABLE();

	/*
	 * Enable write/read access to backup domain.
	 */
	HAL_PWR_EnableBkUpAccess();

	/*
	 * Read backup register.
	 */
	value = *register_address;

	/*
	 * Protect backup domain again.
	 */
	HAL_PWR_DisableBkUpAccess();

	return value;
}


static void Safety_BackupWrite(volatile uint32_t *register_address,
		uint32_t value)
{
	/*
	 * Enable PWR peripheral clock.
	 */
	__HAL_RCC_PWR_CLK_ENABLE();

	/*
	 * Enable access to backup domain.
	 */
	HAL_PWR_EnableBkUpAccess();

	/*
	 * Write value.
	 */
	*register_address = value;

	/*
	 * Ensure write completes.
	 */
	__DSB();

	/*
	 * Protect backup domain again.
	 */
	HAL_PWR_DisableBkUpAccess();
}


/*----------------------------------------------------------
 * Safety Initialization
 *----------------------------------------------------------*/

void Safety_Init(void)
{
	uint32_t fault_marker;
	uint32_t reset_counter;


	/*
	 * Read persistent MPU fault marker.
	 */
	fault_marker = Safety_BackupRead(
			&SAFETY_BKP_FAULT_MARKER);


	/*
	 * Read persistent reset counter.
	 */
	reset_counter = Safety_BackupRead(
			&SAFETY_BKP_RESET_COUNTER);


	/*
	 * Check whether the previous reset was caused by
	 * our MPU safety reset request.
	 */
	if (fault_marker == SAFETY_MPU_FAULT_MAGIC)
	{
		/*
		 * Record that an MPU fault occurred.
		 */
		Safety_MPUFaultFlag = 1U;


		/*
		 * Increment counter.
		 */
		if (reset_counter < UINT32_MAX)
		{
			reset_counter++;
		}


		/*
		 * Store updated counter.
		 */
		Safety_BackupWrite(
				&SAFETY_BKP_RESET_COUNTER,
				reset_counter);


		/*
		 * Clear fault marker.
		 *
		 * This means the marker only represents the
		 * immediately previous MPU-triggered reset.
		 */
		Safety_BackupWrite(
				&SAFETY_BKP_FAULT_MARKER,
				0U);
	}


	/*
	 * Check reset threshold.
	 *
	 * ">" means safe state starts AFTER 3 resets.
	 */
	if (reset_counter > SAFETY_MAX_MPU_RESETS)
	{
		Safety_EnterSafeState();
	}
}


/*----------------------------------------------------------
 * Request MPU reset
 *----------------------------------------------------------*/

void Safety_RequestMPUReset(void)
{
	/*
	 * Set runtime MPU fault flag.
	 */
	Safety_MPUFaultFlag = 1U;


	/*
	 * Store a persistent marker BEFORE reset.
	 *
	 * After reboot Safety_Init() will detect this marker
	 * and increment the reset counter.
	 */
	Safety_BackupWrite(
			&SAFETY_BKP_FAULT_MARKER,
			SAFETY_MPU_FAULT_MAGIC);


	/*
	 * Reset the MCU.
	 */
	NVIC_SystemReset();


	/*
	 * Should never be reached.
	 */
	while (1)
	{
	}
}


/*----------------------------------------------------------
 * Enter Safe State
 *----------------------------------------------------------*/

void Safety_EnterSafeState(void)
{
	/*
	 * Set safety state.
	 */
	Safety_State = SAFETY_STATE_SAFE;


	/*
	 * PC13 LED is ACTIVE LOW on your board.
	 *
	 * RESET = LED ON
	 */
	HAL_GPIO_WritePin(
			GPIOC,
			GPIO_PIN_13,
			GPIO_PIN_RESET);
}


/*----------------------------------------------------------
 * Safety processing
 *----------------------------------------------------------*/

void Safety_Process(void)
{
	uint32_t current_tick;


	/*
	 * Nothing special to do in normal state.
	 */
	if (Safety_State != SAFETY_STATE_SAFE)
	{
		return;
	}


	/*
	 * Keep PC13 ON while in safe state.
	 */
	HAL_GPIO_WritePin(
			GPIOC,
			GPIO_PIN_13,
			GPIO_PIN_RESET);


	/*
	 * Get current system time.
	 */
	current_tick = HAL_GetTick();


	/*
	 * Check alarm period.
	 */
	if ((current_tick - Safety_LastAlarmTick)
			>= SAFETY_ALARM_PERIOD_MS)
	{
		Safety_LastAlarmTick = current_tick;


		/*
		 * Call alarm callback if one has been registered.
		 *
		 * Later this can send your UART alarm frame.
		 */
		if (Safety_AlarmCallback != NULL)
		{
			Safety_AlarmCallback();
		}
	}
}


/*----------------------------------------------------------
 * Check Safe State
 *----------------------------------------------------------*/

uint8_t Safety_IsSafeState(void)
{
	if (Safety_State == SAFETY_STATE_SAFE)
	{
		return 1U;
	}

	return 0U;
}


/*----------------------------------------------------------
 * Get Reset Counter
 *----------------------------------------------------------*/

uint32_t Safety_GetResetCounter(void)
{
	return Safety_BackupRead(
			&SAFETY_BKP_RESET_COUNTER);
}


/*----------------------------------------------------------
 * Test LED indication
 *----------------------------------------------------------*/

void Safety_RunTestIndication(void)
{
	uint32_t count;
	uint32_t i;


	count = Safety_GetResetCounter();


	/*
	 * Test only:
	 *
	 * One LED pulse = one stored MPU fault reset.
	 *
	 * LED ON  = 2 seconds
	 * LED OFF = 500 ms
	 */
	for (i = 0U; i < count; i++)
	{
		/* LED ON */
		HAL_GPIO_WritePin(
				GPIOC,
				GPIO_PIN_13,
				GPIO_PIN_RESET);

		HAL_Delay(SAFETY_LED_ON_TIME_MS);


		/* LED OFF */
		HAL_GPIO_WritePin(
				GPIOC,
				GPIO_PIN_13,
				GPIO_PIN_SET);

		HAL_Delay(SAFETY_LED_OFF_TIME_MS);
	}
}


/*----------------------------------------------------------
 * Register alarm callback
 *----------------------------------------------------------*/

void Safety_SetAlarmCallback(
		Safety_AlarmCallbackTypeDef callback)
{
	Safety_AlarmCallback = callback;
}


/*----------------------------------------------------------
 * Clear Safety History
 *----------------------------------------------------------*/

void Safety_ClearResetHistory(void)
{
	/*
	 * Clear persistent MPU fault marker.
	 */
	Safety_BackupWrite(
			&SAFETY_BKP_FAULT_MARKER,
			0U);


	/*
	 * Clear persistent reset counter.
	 */
	Safety_BackupWrite(
			&SAFETY_BKP_RESET_COUNTER,
			0U);


	/*
	 * Clear runtime information.
	 */
	Safety_MPUFaultFlag = 0U;

	Safety_State = SAFETY_STATE_NORMAL;
}
