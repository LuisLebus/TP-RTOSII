/*=============================================================================
 * Author: Luis Lebus
 * Date: 2019/09/04
 * Version: 1.0
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/
#include "sapi.h"

#include "lib.h"

/*=====[Definition macros of private constants]==============================*/

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Main function, program entry point after power on or reset]==========*/

int main( void )
{
	libConfig_t config;
	libHandle_t handle;

	boardInit();




	libInit();

	config.btnA.gpio = TEC1;
	config.btnA.name = '1';

	config.btnB.gpio = TEC2;
	config.btnB.name = '2';

	config.led.gpio = LED1;
	config.led.name = '1';

	config.echo = false;


	handle = libCreate(config);


	config.btnA.gpio = TEC3;
	config.btnA.name = '3';

	config.btnB.gpio = TEC4;
	config.btnB.name = '4';

	config.led.gpio = LED2;
	config.led.name = '2';

	config.echo = false;


	handle = libCreate(config);



	vTaskStartScheduler();


	// YOU NEVER REACH HERE, because this program runs directly or on a
	// microcontroller and is not called by any Operating System, as in the
	// case of a PC program.
	return 0;
}
