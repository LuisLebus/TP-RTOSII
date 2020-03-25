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
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "uartManager.h"
#include "pool.h"

/*=====[Inclusions of private function dependencies]=========================*/

/*=====[Definition macros of private constants]==============================*/
#define UART_MANAGER_MAX_LEN			64
#define UART_MANAGER_MAX_QUEUE_ZISE		8

/*=====[Private function-like macros]========================================*/

/*=====[Definitions of private data types]===================================*/
typedef enum {
	WAITING_CH_START = 0,
	WAITING_CH_END,
}uartManagerState;

typedef struct {
	bool enabled;
	uartMap_t uart;
	uint32_t baudRate;
	char chStart;
	char chEnd;
	uint8_t msgMaxLen;
	uartManagerState state;
	uint8_t msgCount;
	pool_t poolRx;
	pool_t poolTx;
	uint8_t* poolBlockRx;
	uint8_t* poolBlockTx;
	xQueueHandle rxQueue;
	xQueueHandle txQueue;
	xTaskHandle txTaskHandle;
}uartManager_t;

/*=====[Definitions of external public global variables]=====================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/
static uartManager_t manager[UART_MAXNUM] = {0};

/*=====[Prototypes (declarations) of private functions]======================*/
static void txTask(void *pvParameters);
static void rxCallBack(void* param);

/*=====[Implementations of public functions]=================================*/
uartManagerError_t uartManagerInit(uartManagerHandle_t* handle, uartManagerConfig_t config)
{
	BaseType_t taskErr;
	poolError_t poolErr;

	if(config.uart >= UART_MAXNUM)
		return UART_MANAGER_ERROR;

	if(config.msgMaxLen > UART_MANAGER_MAX_LEN)
		return UART_MANAGER_ERROR;

	*handle = (uartManagerHandle_t)config.uart;

	if(manager[*handle].enabled == true)
		return UART_MANAGER_ERROR;

	manager[*handle].uart = config.uart;
	manager[*handle].baudRate = config.baudRate;
	manager[*handle].chStart = config.chStart;
	manager[*handle].chEnd = config.chEnd;
	manager[*handle].msgMaxLen = config.msgMaxLen;

	uartInit(manager[*handle].uart, manager[*handle].baudRate);
	uartCallbackSet(manager[*handle].uart, UART_RECEIVE, rxCallBack, handle);
	uartInterrupt(manager[*handle].uart, true);

	manager[*handle].state = WAITING_CH_START;
	manager[*handle].msgCount = 0;

	//Se inicializa el pool de memoria para RX
	poolErr = poolInit(&manager[*handle].poolRx, UART_MANAGER_MAX_LEN, UART_MANAGER_MAX_QUEUE_ZISE);
	if(poolErr == POOL_ERROR)
		return UART_MANAGER_ERROR;

	manager[*handle].poolBlockRx = NULL;

	//Se inicializa el pool de memoria para TX
	poolErr = poolInit(&manager[*handle].poolTx, UART_MANAGER_MAX_LEN, UART_MANAGER_MAX_QUEUE_ZISE);
	if(poolErr == POOL_ERROR)
		return UART_MANAGER_ERROR;

	manager[*handle].poolBlockTx = NULL;

	manager[*handle].rxQueue = xQueueCreate(UART_MANAGER_MAX_QUEUE_ZISE, sizeof(uint8_t*));
	if(manager[*handle].rxQueue == NULL)
		return UART_MANAGER_ERROR;

	manager[*handle].txQueue = xQueueCreate(UART_MANAGER_MAX_QUEUE_ZISE, sizeof(uint8_t*));
	if(manager[*handle].txQueue == NULL)
		return UART_MANAGER_ERROR;

	taskErr = xTaskCreate(
			txTask,
			(char*)"txTask",
			configMINIMAL_STACK_SIZE * 2,
			handle,
			(tskIDLE_PRIORITY + 1UL),
			&manager[*handle].txTaskHandle);

	if(taskErr != pdPASS)
		return UART_MANAGER_ERROR;

	manager[*handle].enabled = true;

	return UART_MANAGER_OK;
}


void uartManagerDeinit(uartManagerHandle_t handle)
{
	vQueueDelete(manager[handle].rxQueue);
	vQueueDelete(manager[handle].txQueue);

	vTaskDelete(manager[handle].txTaskHandle);

	uartCallbackClr(manager[handle].uart, UART_RECEIVE);
	uartInterrupt(manager[handle].uart, false);

	poolDeinit(&manager[handle].poolRx);
	poolDeinit(&manager[handle].poolTx);

	manager[handle].enabled = false;
}


uartManagerError_t uartManagerGet(uartManagerHandle_t handle, uint8_t* msg, uint32_t* size, uint32_t timeout)
{
	uartManagerError_t err = UART_MANAGER_ERROR;
	uint8_t* ptrMsg;

	if(msg == NULL)
	{
		if( xQueuePeek(manager[handle].rxQueue, &ptrMsg, timeout / portTICK_RATE_MS) == pdPASS)
		{
			*size = (uint32_t)strlen( (const char*)ptrMsg );

			err = UART_MANAGER_OK;
		}
	}
	else
	{
		if( xQueueReceive(manager[handle].rxQueue, &ptrMsg, timeout / portTICK_RATE_MS) == pdPASS)
		{
			strcpy( (char*)msg, (const char*)ptrMsg );

			poolPut(&manager[handle].poolRx, ptrMsg);

			*size = (uint32_t)strlen( (const char*)msg );

			err = UART_MANAGER_OK;
		}
	}

	return err;
}


uartManagerError_t uartManagerPut(uartManagerHandle_t handle, uint8_t* msg, uint32_t timeout)
{
	uartManagerError_t err = UART_MANAGER_ERROR;


	if( manager[handle].poolBlockTx == NULL )
	{
		manager[handle].poolBlockTx = (uint8_t*)poolGet(&manager[handle].poolTx);

		if( manager[handle].poolBlockTx == NULL )
			return err;
	}

	strcpy( (char*)manager[handle].poolBlockTx, (const char*)msg );


	if( xQueueSend(manager[handle].txQueue, manager[handle].poolBlockTx, timeout / portTICK_RATE_MS) == pdPASS)
	{
		manager[handle].poolBlockTx = NULL;

		err = UART_MANAGER_OK;
	}

	return err;
}

/*=====[Implementations of interrupt functions]==============================*/
static void rxCallBack(void* param)
{
	uartManagerHandle_t handle = *(uartManagerHandle_t*)param;
	bool_t uartErr;
	uint8_t ch;


	if( manager[handle].poolBlockRx == NULL )
	{
		manager[handle].poolBlockRx = (uint8_t*)poolGet(&manager[handle].poolRx);

		if( manager[handle].poolBlockRx == NULL )
			return;
	}


	while( uartReadByte(manager[handle].uart, &ch) )
	{
		switch(manager[handle].state)
		{
			case WAITING_CH_START:
				if( ch == manager[handle].chStart )
				{
					manager[handle].poolBlockRx[0] = ch;
					manager[handle].msgCount = 1;
					manager[handle].state = WAITING_CH_END;
				}
				break;
			case WAITING_CH_END:
				if( ch == manager[handle].chEnd )
				{
					manager[handle].poolBlockRx[manager[handle].msgCount] = ch;
					manager[handle].poolBlockRx[manager[handle].msgCount + 1] = '\0';

					xQueueSendFromISR(manager[handle].rxQueue, manager[handle].poolBlockRx, NULL);

					manager[handle].poolBlockRx = NULL;

					manager[handle].state = WAITING_CH_START;
				}
				else
				{
					if( manager[handle].msgCount == (manager[handle].msgMaxLen - 2) )
					{
						manager[handle].state = WAITING_CH_START;
					}
					else
					{
						manager[handle].poolBlockRx[manager[handle].msgCount] = ch;
						manager[handle].msgCount++;
					}
				}
				break;
			default:
				manager[handle].state = WAITING_CH_START;
		}
	}
}

/*=====[Implementations of private functions]================================*/
static void txTask(void *pvParameters)
{
	uartManagerHandle_t handle = *(uartManagerHandle_t*)pvParameters;
	uint8_t* ptrMsg;

	while(1)
	{
		if( xQueueReceive(manager[handle].txQueue, &ptrMsg, portMAX_DELAY) == pdPASS)
		{
			uartWriteString(manager[handle].uart, ptrMsg);

			poolPut(&manager[handle].poolTx, ptrMsg);
		}
	}
}
