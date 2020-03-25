/*=============================================================================
 * Author: Luis Lebus
 * Date: 2019/09/04
 * Version: 1.0
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/
#include "FreeRTOS.h"
#include "task.h"

#include "app.h"
#include "sapi.h"


/*=====[Definition macros of private constants]==============================*/

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Main function, program entry point after power on or reset]==========*/

int main( void )
{
	boardInit();

	appInit();

	vTaskStartScheduler();

	return 0;
}
