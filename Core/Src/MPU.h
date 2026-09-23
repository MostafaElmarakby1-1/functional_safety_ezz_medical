



#ifndef SRC_MPU_H_
#define SRC_MPU_H_


#include "main.h"
#include <stdint.h>

/*----------------------------------------------------------
 * MPU Fault Type
 *----------------------------------------------------------*/
typedef enum
{
	MPU_FAULT_NONE = 0,
	MPU_FAULT_DATA_ACCESS,
	MPU_FAULT_INSTRUCTION_ACCESS,
	MPU_FAULT_UNKNOWN

} MPU_FaultType;


/*----------------------------------------------------------
 * MPU Fault Information
 *----------------------------------------------------------*/
typedef struct
{
	uint8_t       detected;
	MPU_FaultType type;

	uint8_t       address_valid;
	uint32_t      address;

	uint32_t      cfsr;

} MPU_FaultInfoTypeDef;


/*----------------------------------------------------------
 * Functions
 *----------------------------------------------------------*/

/* Configure and enable MPU */
void MPU_Init(void);

/* Called by MemManage_Handler() */
void MPU_HandleMemManageFault(void);

/* Get the latest MPU fault information */
void MPU_GetFaultInfo(MPU_FaultInfoTypeDef *info);

/* Clear stored MPU fault information */
void MPU_ClearFault(void);



#endif /* SRC_MPU_H_ */
