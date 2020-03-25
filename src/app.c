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
#include <ctype.h>

#include "FreeRTOS.h"
#include "task.h"

#include "app.h"
#include "uartManager.h"
#include "sep.h"

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
	uartManagerHandle_t uartHandle;

	sepHandle_t sepHandle;

	uartConfig.baudRate = 115200;
	uartConfig.uart = UART_USB;
	uartConfig.chStart = '(';
	uartConfig.chEnd = ')';
	uartConfig.msgMaxLen = 32;

	uartManagerInit(&uartHandle, uartConfig);

	sepHandle = sepInit(uartConfig);

	xTaskCreate(
			rxTask,
			(char*)"rxTask",
			configMINIMAL_STACK_SIZE * 2,
			sepHandle,
			(tskIDLE_PRIORITY + 1UL),
			NULL);
}

/*=====[Implementations of interrupt functions]==============================*/

/*=====[Implementations of private functions]================================*/
static void rxTask(void *pvParameters)
{
	sepHandle_t handle = (sepHandle_t)pvParameters;
	uint32_t size;
	sepData_t data;


	while(1)
	{
		if( sepGet(handle, NULL, &size, portMAX_DELAY) )
		{
			//TODO: asignar memoria para size, algo asi
//			data.msg = malloc(size)

			sepGet(handle, &data, NULL, portMAX_DELAY);

			switch(data.event)
			{
				case TO_LOWER:
					while( *data.msg != '\0' )
					{
						*data.msg = tolower( *data.msg );
						data.msg++;
					}
					break;
				case TO_UPPER:
					while( *data.msg != '\0' )
					{
						*data.msg = toupper( *data.msg );
						data.msg++;
					}
					break;
			}

			sepPut(handle, &data, 0);

			//TODO: liberar la memoria adignada
//			free(data.msg)
		}
	}
}
