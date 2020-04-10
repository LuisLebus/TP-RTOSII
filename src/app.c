/*=====[Module Name]===========================================================
 * Copyright YYYY Author Compelte Name <author@mail.com>
 * All rights reserved.
 * License: license text or at least name and link 
         (example: BSD-3-Clause <https://opensource.org/licenses/BSD-3-Clause>)
 *
 * Version: 0.0.0
 * Creation Date: YYYY/MM/DD
 */

/*=====[Inclusion of own header]=============================================*/
#include "../../tp/inc/app.h"

#include <string.h>
#include <ctype.h>

#include "../../tp/inc/sep.h"
#include "../../tp/inc/uartManager.h"
#include "FreeRTOS.h"
#include "task.h"


/*=====[Inclusions of private function dependencies]=========================*/

/*=====[Definition macros of private constants]==============================*/

/*=====[Private function-like macros]========================================*/

/*=====[Definitions of private data types]===================================*/

/*=====[Definitions of external public global variables]=====================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Prototypes (declarations) of private functions]======================*/
static void rxTask(void *pvParameters);

/*=====[Implementations of public functions]=================================*/
void appInit(void)
{
	uartManagerConfig_t uartConfig;
	static uartManagerHandle_t uartHandle;

	static sepHandle_t sepHandle;

	uartConfig.baudRate = 115200;
	uartConfig.uart = UART_USB;
	uartConfig.chStart = '(';
	uartConfig.chEnd = ')';
	uartConfig.msgMaxLen = 32;

	uartManagerInit(&uartHandle, uartConfig);

	sepInit(&sepHandle, uartHandle);

	xTaskCreate(
			rxTask,
			(char*)"rxTask",
			configMINIMAL_STACK_SIZE * 2,
			(void*)&sepHandle,
			(tskIDLE_PRIORITY + 1UL),
			NULL);
}

/*=====[Implementations of interrupt functions]==============================*/

/*=====[Implementations of private functions]================================*/
static void rxTask(void *pvParameters)
{
	sepHandle_t handle = *(sepHandle_t*)pvParameters;
	uint32_t size;
	sepData_t data;

	uint8_t* ptr;

	while(1)
	{
		if( sepGet(&handle, &data, UINT32_MAX) )
		{
			ptr = data.msg;

			switch(data.event)
			{
				case TO_LOWER:
					while( *ptr != '\0' )
					{
						if( (*ptr >= 'A') && (*ptr <= 'Z') )
						{
							*ptr = tolower( *ptr );
							ptr++;
						}
						else
						{
							strcpy(data.msg, "ERROR\0");
							data.event = TO_ERROR;
							break;
						}
					}
					break;
				case TO_UPPER:
					while( *ptr != '\0' )
					{
						if( (*ptr >= 'a') && (*ptr <= 'z') )
						{
							*ptr = toupper( *ptr );
							ptr++;
						}
						else
						{
							strcpy(data.msg, "ERROR\0");
							data.event = TO_ERROR;
							break;
						}
					}

					break;
			}

			sepPut(&handle, &data, 0);
		}
	}
}
