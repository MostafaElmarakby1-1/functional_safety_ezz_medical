/*----------------------------------------------------------
 * Internal Fault Information
 *----------------------------------------------------------*/
#include "MPU.h"
static volatile MPU_FaultInfoTypeDef MPU_FaultInfo =
{
		0,
		MPU_FAULT_NONE,
		0,
		0,
		0
};

/*----------------------------------------------------------
 * MPU Initialization
 *----------------------------------------------------------*/

void MPU_Init(void)
{
	MPU_Region_InitTypeDef MPU_InitStruct = {0};


	/* Disable MPU before configuration */
	HAL_MPU_Disable();


	/*======================================================
	 * REGION 0 : FLASH
	 *
	 * STM32F401RC:
	 * FLASH = 256 KB
	 *
	 * Read     -> Allowed
	 * Write    -> Not allowed
	 * Execute  -> Allowed
	 *======================================================*/

	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
	MPU_InitStruct.BaseAddress      = 0x08000000U;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_256KB;
	MPU_InitStruct.SubRegionDisable = 0x00U;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;

	/* Privileged + unprivileged Read Only */
	MPU_InitStruct.AccessPermission = MPU_REGION_PRIV_RO_URO;

	/* Instruction execution allowed */
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;

	MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);


	/*======================================================
	 * REGION 1 : SRAM
	 *
	 * SRAM = 64 KB
	 *
	 * Read     -> Allowed
	 * Write    -> Allowed
	 * Execute  -> Not allowed
	 *======================================================*/

	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER1;
	MPU_InitStruct.BaseAddress      = 0x20000000U;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_64KB;
	MPU_InitStruct.SubRegionDisable = 0x00U;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;

	/* Full Read + Write */
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;

	/* Execute Never */
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;

	MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);


	/*======================================================
	 * REGION 2 : PERIPHERALS
	 *======================================================*/

	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER2;
	MPU_InitStruct.BaseAddress      = 0x40000000U;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_512MB;
	MPU_InitStruct.SubRegionDisable = 0x00U;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;

	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;

	/* Execute Never */
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;

	MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);


	/* Enable MPU */
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);


	/* Enable MemManage Fault */
	SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;


	/* Synchronization barriers */
	__DSB();
	__ISB();
}
/*----------------------------------------------------------
 * Handle MemManage Fault
 *----------------------------------------------------------*/

void MPU_HandleMemManageFault(void)
{
	uint32_t cfsr;

	/* Read fault status */
	cfsr = SCB->CFSR;

	/* Fault detected */
	MPU_FaultInfo.detected = 1;

	/* Save CFSR */
	MPU_FaultInfo.cfsr = cfsr;


	/*------------------------------------------------------
	 * Determine fault type
	 *------------------------------------------------------*/

	if ((cfsr & SCB_CFSR_DACCVIOL_Msk) != 0U)
	{
		MPU_FaultInfo.type = MPU_FAULT_DATA_ACCESS;
	}
	else if ((cfsr & SCB_CFSR_IACCVIOL_Msk) != 0U)
	{
		MPU_FaultInfo.type = MPU_FAULT_INSTRUCTION_ACCESS;
	}
	else
	{
		MPU_FaultInfo.type = MPU_FAULT_UNKNOWN;
	}


	/*------------------------------------------------------
	 * Check if MMFAR contains a valid address
	 *------------------------------------------------------*/

	if ((cfsr & SCB_CFSR_MMARVALID_Msk) != 0U)
	{
		MPU_FaultInfo.address_valid = 1;
		MPU_FaultInfo.address = SCB->MMFAR;
	}
	else
	{
		MPU_FaultInfo.address_valid = 0;
		MPU_FaultInfo.address = 0;
	}
}


/*----------------------------------------------------------
 * Get MPU Fault Information
 *----------------------------------------------------------*/

void MPU_GetFaultInfo(MPU_FaultInfoTypeDef *info)
{
	if (info != NULL)
	{
		info->detected       = MPU_FaultInfo.detected;
		info->type           = MPU_FaultInfo.type;
		info->address_valid  = MPU_FaultInfo.address_valid;
		info->address        = MPU_FaultInfo.address;
		info->cfsr            = MPU_FaultInfo.cfsr;
	}
}


/*----------------------------------------------------------
 * Clear MPU Fault Information
 *----------------------------------------------------------*/

void MPU_ClearFault(void)
{
	MPU_FaultInfo.detected      = 0;
	MPU_FaultInfo.type          = MPU_FAULT_NONE;
	MPU_FaultInfo.address_valid = 0;
	MPU_FaultInfo.address       = 0;
	MPU_FaultInfo.cfsr           = 0;

	/* Clear MemManage fault status bits */
	SCB->CFSR = 0xFFU;
}
