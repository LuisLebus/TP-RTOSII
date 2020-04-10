/*=============================================================================
 * Author: Luis Lebus
 * Date: 2019/09/04
 * Version: 1.0
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/
#include "FreeRTOS.h"
#include "task.h"

#include "sapi.h"

#include "app.h"

/*=====[Definition macros of private constants]==============================*/

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/
static void ledTask(void *pvParameters)
{
	while(1)
	{
		gpioToggle(LED3);

		vTaskDelay( 100 / portTICK_RATE_MS);
	}
}


/*=====[Main function, program entry point after power on or reset]==========*/

int main( void )
{
	boardInit();

	appInit();

	xTaskCreate(
				ledTask,
				(char*)"ledTask",
				configMINIMAL_STACK_SIZE * 2,
				NULL,
				(tskIDLE_PRIORITY + 1UL),
				NULL);

	vTaskStartScheduler();

	return 0;
}
