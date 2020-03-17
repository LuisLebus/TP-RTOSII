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
#include "FreeRTOS.h"
#include "task.h"

#include "sapi.h"

#include "lib.h"

/*=====[Inclusions of private function dependencies]=========================*/

/*=====[Definition macros of private constants]==============================*/
#define LIB_EV_BUTTON_PRESSED	0
#define LIB_EV_LED_ON			1

/*=====[Private function-like macros]========================================*/

/*=====[Definitions of private data types]===================================*/
typedef struct {
	uint8_t id;
	char* msg;
}libEvent_t;

typedef struct {
	libConfig_t config;
	xQueueHandle queueUart;
	xQueueHandle queueEcho;
}libControl_t;


typedef struct {
	libControl_t control;
	TaskHandle_t btnTaskHandle;
	TaskHandle_t ledTaskHandle;
	TaskHandle_t uartTaskHandle;
	TaskHandle_t echoTaskHandle;
}libBase_t;

typedef enum{
	btnUp = 0,
	btnFalling,
	btnDown,
	btnRising
}btnState_t;

/*=====[Definitions of external public global variables]=====================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Prototypes (declarations) of private functions]======================*/
static void libBtnTask(void *pvParameters);
static void libLedTask(void *pvParameters);
static void libUartTask(void *pvParameters);
static void libEchoTask(void *pvParameters);

/*=====[Implementations of public functions]=================================*/
void libInit(void)
{
	gpioInit(0, GPIO_ENABLE);

	uartInit(UART_USB, 115200);

	stdioPrintf(UART_USB, "¡¡ Inicio !!\r\n\r\n");
}

libHandle_t libCreate(libConfig_t config)
{
	BaseType_t err;

	libBase_t* base = (libBase_t*)pvPortMalloc( sizeof(libBase_t) );

	if(base == NULL)
		return NULL;

	base->control.config = config;

	base->control.queueUart = xQueueCreate(5, sizeof(libEvent_t));
	if(base->control.queueUart == NULL)
		return NULL;

	base->control.queueEcho = xQueueCreate(5, sizeof(libEvent_t));
	if(base->control.queueEcho == NULL)
		return NULL;


	err = xTaskCreate(
			libBtnTask,
			(char*)"libBtnTask",
			configMINIMAL_STACK_SIZE * 2,
			&base->control,
			(tskIDLE_PRIORITY + 1UL),
			&base->btnTaskHandle);

	if(err != pdPASS)
		return NULL;


	err = xTaskCreate(
			libLedTask,
			(char*)"libLedTask",
			configMINIMAL_STACK_SIZE * 2,
			&base->control,
			(tskIDLE_PRIORITY + 1UL),
			&base->ledTaskHandle);

	if(err != pdPASS)
		return NULL;


	xTaskCreate(
			libUartTask,
			(char*)"libUartTask",
			configMINIMAL_STACK_SIZE * 2,
			&base->control,
			(tskIDLE_PRIORITY + 2UL),
			&base->uartTaskHandle);

	if(err != pdPASS)
		return NULL;


	if(base->control.config.echo)
	{
		xTaskCreate(
				libEchoTask,
				(char*)"libEchoTask",
				configMINIMAL_STACK_SIZE * 2,
				&base->control.config,
				(tskIDLE_PRIORITY + 2UL),
				&base->echoTaskHandle);

		if(err != pdPASS)
			return NULL;
	}

	return (libHandle_t)&base;
}

void libDelete(libHandle_t handle)
{
	libBase_t* base = (libBase_t*)handle;

	libEvent_t event;

	if(base != NULL)
	{
		while( uxQueueMessagesWaiting(base->control.queueUart) )
		{
			 xQueueReceive(base->control.queueUart, &event, portMAX_DELAY);
			 vPortFree(event.msg);
		}

		vTaskDelete(base->uartTaskHandle);
		vQueueDelete(base->control.queueUart);

		while( uxQueueMessagesWaiting(base->control.queueEcho) )
		{
			 xQueueReceive(base->control.queueEcho, &event, portMAX_DELAY);
			 vPortFree(event.msg);
		}

		vTaskDelete(base->echoTaskHandle);
		vQueueDelete(base->control.queueEcho);

		vTaskDelete(base->btnTaskHandle);
		vTaskDelete(base->ledTaskHandle);

		vPortFree(base);
	}
}


/*=====[Implementations of interrupt functions]==============================*/

/*=====[Implementations of private functions]================================*/
static void libBtnTask(void *pvParameters)
{
	libControl_t* control = (libControl_t*)pvParameters;

	libEvent_t event;

	btnState_t btnAState = btnUp;
	portTickType btnAStart = 0;
	uint32_t btnADuration = 0;

	btnState_t btnBState  = btnUp;
	portTickType btnBStart = 0;
	uint32_t btnBDuration = 0;


	while(1)
	{
		//MEF para btnA
		switch(btnAState)
		{
			case btnUp:
				if( gpioRead(control->config.btnA.gpio) == false )
				{
					btnAState = btnFalling;
				}
				break;
			case btnFalling:
				if( gpioRead(control->config.btnA.gpio) == false )
				{
					btnAState = btnDown;

					btnAStart = xTaskGetTickCount();
				}
				else
				{
					btnAState = btnUp;
				}
				break;
			case btnDown:
				if( gpioRead(control->config.btnA.gpio) == true )
				{
					btnAState = btnRising;
				}
				break;
			case btnRising:
				if( gpioRead(control->config.btnA.gpio) == true )
				{
					btnAState = btnUp;

					btnADuration = ( (xTaskGetTickCount() - btnAStart) * portTICK_RATE_MS );

					event.id = LIB_EV_BUTTON_PRESSED;

					event.msg = (char*)pvPortMalloc(14 * sizeof(char));

					if(event.msg != NULL)
					{
						stdioSprintf(event.msg, "TEC%c %06u\r\n", control->config.btnA.name, btnADuration);

						xQueueSend(control->queueUart, &event, 0);
					}
				}
				else
				{
					btnAState = btnDown;
				}
				break;
			default:
				btnAState = btnUp;
		}


		//MEF para btnB
		switch(btnBState)
		{
			case btnUp:
				if( gpioRead(control->config.btnB.gpio) == false )
				{
					btnBState = btnFalling;
				}
				break;
			case btnFalling:
				if( gpioRead(control->config.btnB.gpio) == false )
				{
					btnBState = btnDown;

					btnBStart = xTaskGetTickCount();
				}
				else
				{
					btnBState = btnUp;
				}
				break;
			case btnDown:
				if( gpioRead(control->config.btnB.gpio) == true )
				{
					btnBState = btnRising;
				}
				break;
			case btnRising:
				if( gpioRead(control->config.btnB.gpio) == true )
				{
					btnBState = btnUp;

					btnBDuration = ( (xTaskGetTickCount() - btnBStart) * portTICK_RATE_MS );

					event.id = LIB_EV_BUTTON_PRESSED;

					event.msg = (char*)pvPortMalloc(14 * sizeof(char));

					if(event.msg != NULL)
					{
						stdioSprintf(event.msg, "TEC%c %06u\r\n", control->config.btnB.name, btnBDuration);

						xQueueSend(control->queueUart, &event, 0);
					}
				}
				else
				{
					btnBState = btnDown;
				}
				break;
			default:
				btnBState = btnUp;
		}

		vTaskDelay( 50 / portTICK_RATE_MS);
	}
}


static void libLedTask(void *pvParameters)
{
	libControl_t* control = (libControl_t*)pvParameters;

	libEvent_t event;

	bool ledState = OFF;

	while(1)
	{
		if(ledState == ON)
		{
			ledState = OFF;
			gpioWrite(control->config.led.gpio, ledState);
		}
		else
		{
			ledState = ON;
			gpioWrite(control->config.led.gpio, ledState);

			event.id = LIB_EV_LED_ON;

			event.msg = (char*)pvPortMalloc(10 * sizeof(char));

			if(event.msg != NULL)
			{
				stdioSprintf(event.msg, "LED%c ON\r\n", control->config.led.name);

				xQueueSend(control->queueUart, &event, 0);
			}
		}

		vTaskDelay( 1000 / portTICK_RATE_MS);
	}
}


static void libUartTask(void *pvParameters)
{
	libControl_t* control = (libControl_t*)pvParameters;

	libEvent_t event;

	while(1)
	{
		if( xQueueReceive(control->queueUart, &event, portMAX_DELAY) == pdTRUE)
		{
			switch(event.id)
			{
				case LIB_EV_BUTTON_PRESSED:
				case LIB_EV_LED_ON:
					stdioPrintf(UART_USB, "%s", event.msg);
					break;
				default:
					stdioPrintf(UART_USB, "ERROR!");
			}

			if(control->config.echo)
			{
				xQueueSend(control->queueEcho, &event, 0);
			}
			else
			{
				vPortFree(event.msg);
			}
		}
	}
}


static void libEchoTask(void *pvParameters)
{
	libControl_t* control = (libControl_t*)pvParameters;

	libEvent_t event;

	while(1)
	{
		if( xQueueReceive(control->queueEcho, &event, portMAX_DELAY) == pdTRUE )
		{
			stdioPrintf(UART_USB, "ECHO %s", event.msg);

			vPortFree(event.msg);
		}
	}
}
