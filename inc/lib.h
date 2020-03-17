/*=====[Module Name]===========================================================
 * Copyright YYYY Author Compelte Name <author@mail.com>
 * All rights reserved.
 * License: license text or at least name and link 
         (example: BSD-3-Clause <https://opensource.org/licenses/BSD-3-Clause>)
 *
 * Version: 0.0.0
 * Creation Date: YYYY/MM/DD
 */

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef _LIB_H_
#define _LIB_H_

/*=====[Inclusions of public function dependencies]==========================*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/

/*=====[Public function-like macros]=========================================*/

/*=====[Definitions of public data types]====================================*/
typedef struct {
	struct {
		gpioMap_t gpio;
		char name;
	}btnA;
	struct {
		gpioMap_t gpio;
		char name;
	}btnB;
	struct {
		gpioMap_t gpio;
		char name;
	}led;
	bool echo;
}libConfig_t;

typedef void * libHandle_t;

/*=====[Prototypes (declarations) of public functions]=======================*/
void libInit(void);

libHandle_t libCreate(libConfig_t config);
void libDelete(libHandle_t handle);

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* _LIB_H_ */
