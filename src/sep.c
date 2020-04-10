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
#include "../../tp/inc/sep.h"

#include <string.h>

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

/*=====[Implementations of public functions]=================================*/
void sepInit(sepHandle_t* me, uartManagerHandle_t uartHandle)
{
	me->uartHandle = uartHandle;
}

sepError_t sepGet(sepHandle_t* me, sepData_t* data, uint32_t timeout)
{
	sepError_t ret = SEP_ERROR;
	uint32_t size;
	uint8_t* msg;

	if (uartManagerGet( me->uartHandle, NULL, &size, timeout))
	{
		msg = (uint8_t *)pvPortMalloc(size);

		uartManagerGet( me->uartHandle, msg, &size, timeout);

		if (msg[0] == 'm' || msg[0] == 'M' )
		{
			switch (msg[0])
			{
				case 'm':
					data->event = TO_LOWER;
					break;

				case 'M':
					data->event = TO_UPPER;
					break;
			}

			data->msg = (uint8_t*)pvPortMalloc(size);
			strcpy(data->msg, msg + 1);
			data->msg[size] = '\0';

			ret = SEP_OK;
		}

		vPortFree(msg);
	}

	return ret;
}


sepError_t sepPut(sepHandle_t* me, sepData_t* data, uint32_t timeout)
{
	sepError_t ret = SEP_ERROR;
	uint8_t* msg;

	msg = (uint8_t *)pvPortMalloc(strlen(data->msg) + 1);

	switch(data->event)
	{
		case TO_UPPER:
			*msg = 'M';
			strcpy(msg + 1, data->msg);
			break;
		case TO_LOWER:
			*msg = 'm';
			strcpy(msg + 1, data->msg);
			break;
		case TO_ERROR:
			strcpy(msg, data->msg);
			break;
	}

	if(uartManagerPut(me->uartHandle, msg, timeout))
	{
		ret = SEP_OK;
	}

	vPortFree(data->msg);
	vPortFree(msg);

	return ret;
}


/*=====[Implementations of interrupt functions]==============================*/

/*=====[Implementations of private functions]================================*/

