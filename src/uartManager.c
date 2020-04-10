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
#include "../../tp/inc/uartManager.h"

#include <string.h>
#include <stdlib.h>

#include "../../tp/inc/crc8.h"
#include "../../tp/inc/pool.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"


/*=====[Inclusions of private function dependencies]=========================*/

/*=====[Definition macros of private constants]==============================*/
#define UART_MANAGER_MAX_LEN			64
#define UART_MANAGER_MAX_QUEUE_ZISE		8

#define UART_MANAGER_TIMEOUT			(50 / portTICK_RATE_MS)
#define UART_MANAGER_BLOCK_TIME			(50 / portTICK_RATE_MS)

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
    TimerHandle_t onTxTimeout;
    TimerHandle_t onRxTimeout;
}uartManager_t;

/*=====[Definitions of external public global variables]=====================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/
static uartManager_t manager[UART_MAXNUM] = {0};

/*=====[Prototypes (declarations) of private functions]======================*/
//static void txTask(void *pvParameters);
static void rxCallback(void* pvParameters);

static void onRxTimeoutCallback(TimerHandle_t xTimer);
static void onTxTimeoutCallback(TimerHandle_t xTimer);

static bool checkCrc8(uint8_t* data, uint8_t len);

/*=====[Implementations of public functions]=================================*/
uartManagerError_t uartManagerInit(uartManagerHandle_t* handle, uartManagerConfig_t config)
{
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
	uartCallbackSet(manager[*handle].uart, UART_RECEIVE, rxCallback, handle);
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

	manager[*handle].onRxTimeout = xTimerCreate(
			"onRxTimeout",
			UART_MANAGER_TIMEOUT,
			pdFALSE,
			handle,
			onRxTimeoutCallback);

	if(manager[*handle].onRxTimeout == NULL)
		return UART_MANAGER_ERROR;


	manager[*handle].onTxTimeout = xTimerCreate(
			"onTxTimeout",
			UART_MANAGER_TIMEOUT / portTICK_RATE_MS,
			pdFALSE,
			handle,
			onTxTimeoutCallback);

	if(manager[*handle].onTxTimeout == NULL)
		return UART_MANAGER_ERROR;


	manager[*handle].enabled = true;

	return UART_MANAGER_OK;
}


void uartManagerDeinit(uartManagerHandle_t handle)
{
	vQueueDelete(manager[handle].rxQueue);
	vQueueDelete(manager[handle].txQueue);

	uartCallbackClr(manager[handle].uart, UART_RECEIVE);
	uartInterrupt(manager[handle].uart, false);

	poolDeinit(&manager[handle].poolRx);
	poolDeinit(&manager[handle].poolTx);

	xTimerDelete(manager[handle].onRxTimeout, UART_MANAGER_BLOCK_TIME);
	xTimerDelete(manager[handle].onTxTimeout, UART_MANAGER_BLOCK_TIME);

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
			*size = (uint32_t)( strlen( (const char*)ptrMsg ) - 4 );

			err = UART_MANAGER_OK;
		}
	}
	else
	{
		if( xQueueReceive(manager[handle].rxQueue, &ptrMsg, timeout / portTICK_RATE_MS) == pdPASS)
		{
			*size = (uint32_t)( strlen( (const char*)ptrMsg ) - 4 );

			strncpy( (char*)msg, (const char*)(ptrMsg + 1), *size);

			msg[*size] = '\0';

			poolPut(&manager[handle].poolRx, ptrMsg);

			err = UART_MANAGER_OK;
		}
	}

	return err;
}


uartManagerError_t uartManagerPut(uartManagerHandle_t handle, uint8_t* msg, uint32_t timeout)
{
	uartManagerError_t err = UART_MANAGER_ERROR;
	uint8_t crcCalculated = 0;

	if( manager[handle].poolBlockTx == NULL )
	{
		manager[handle].poolBlockTx = (uint8_t*)poolGet(&manager[handle].poolTx);

		if( manager[handle].poolBlockTx == NULL )
			return err;
	}

	crcCalculated = crc8Calculate(0, msg, strlen( (const char*)msg ));

	sprintf((char*)manager[handle].poolBlockTx, "(%s%02X)", msg, crcCalculated);

	if( xQueueSend(manager[handle].txQueue, &manager[handle].poolBlockTx, timeout / portTICK_RATE_MS) == pdPASS)
	{
		manager[handle].poolBlockTx = NULL;

		err = UART_MANAGER_OK;

		if( xTimerIsTimerActive(manager[handle].onTxTimeout) == pdFALSE )
		{
			xTimerStart(manager[handle].onTxTimeout, UART_MANAGER_BLOCK_TIME);
		}
	}

	return err;
}

/*=====[Implementations of interrupt functions]==============================*/
static void rxCallback(void* pvParameters)
{
	uartManagerHandle_t handle = *(uartManagerHandle_t*)pvParameters;
	uint8_t ch;

	while( uartReadByte(manager[handle].uart, &ch) )
	{
		if( manager[handle].poolBlockRx == NULL )
		{
			manager[handle].poolBlockRx = (uint8_t*)poolGet(&manager[handle].poolRx);

			if( manager[handle].poolBlockRx == NULL )
				break;
		}

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
					manager[handle].msgCount++;

					manager[handle].poolBlockRx[manager[handle].msgCount] = '\0';

					if( checkCrc8(manager[handle].poolBlockRx, manager[handle].msgCount) )
					{
						xQueueSendFromISR(manager[handle].rxQueue, &manager[handle].poolBlockRx, NULL);

						manager[handle].poolBlockRx = NULL;
					}

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

	    xTimerStartFromISR(manager[handle].onRxTimeout, NULL);
	}
}

/*=====[Implementations of private functions]================================*/
static void onTxTimeoutCallback(TimerHandle_t xTimer)
{
	uartManagerHandle_t handle = *(uartManagerHandle_t*)pvTimerGetTimerID(xTimer);
	uint8_t* ptrMsg;

	if( xQueueReceive(manager[handle].txQueue, &ptrMsg, 0) == pdPASS)
	{
		uartWriteString(manager[handle].uart, ptrMsg);

		poolPut(&manager[handle].poolTx, ptrMsg);

		if( uxQueueMessagesWaiting(manager[handle].txQueue) )
		{
			xTimerStart(manager[handle].onTxTimeout, UART_MANAGER_BLOCK_TIME);
		}
	}
}

static void onRxTimeoutCallback(TimerHandle_t xTimer)
{
	uartManagerHandle_t handle = *(uartManagerHandle_t*)pvTimerGetTimerID(xTimer);

	manager[handle].state = WAITING_CH_START;
}

static bool checkCrc8(uint8_t* data, uint8_t len)
{
	bool retVal = false;
	char crcAux[3] = {0};
	uint8_t crcReceived = 0;
	uint8_t crcCalculated = 0;

	strncpy(crcAux, data + len - 3, 2);

	crcReceived = (uint8_t)strtoul(crcAux, NULL, 16);
	crcCalculated = crc8Calculate(0, data + 1, len - 4);

	if(crcReceived == crcCalculated)
		retVal = true;

	return retVal;
}
