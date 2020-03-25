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

#ifndef _SEP_H_
#define _SEP_H_

/*=====[Inclusions of public function dependencies]==========================*/
#include <stdint.h>

#include "uartManager.h"

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/

/*=====[Public function-like macros]=========================================*/

/*=====[Definitions of public data types]====================================*/
typedef enum {
	TO_LOWER = 0,
	TO_UPPER
}sepEvent_t;

typedef struct {
	sepEvent_t event;
	uint8_t* msg;
}sepData_t;

typedef void* sepHandle_t;

typedef enum {
	SEP_OK = 0,
	SEP_ERROR,
}sepError_t;

/*=====[Prototypes (declarations) of public functions]=======================*/
sepHandle_t sepInit(uartManagerConfig_t config);
void sepDeinit(sepHandle_t handle);

sepError_t sepGet(sepHandle_t handle, sepData_t* data, uint32_t* size, uint32_t timeout);
sepError_t sepPut(sepHandle_t handle, sepData_t* data, uint32_t timeout);

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* _SEP_H_ */
