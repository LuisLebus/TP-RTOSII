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

#ifndef _UART_MANAGER_H_
#define _UART_MANAGER_H_

/*=====[Inclusions of public function dependencies]==========================*/
#include "sapi.h"

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/

/*=====[Public function-like macros]=========================================*/

/*=====[Definitions of public data types]====================================*/
typedef struct {
	uartMap_t uart;
	uint32_t baudRate;
	uint8_t chStart;
	uint8_t chEnd;
	uint8_t msgMaxLen;
}uartManagerConfig_t;

typedef uint32_t uartManagerHandle_t;

typedef enum {
	UART_MANAGER_OK = 0,
	UART_MANAGER_ERROR,
}uartManagerError_t;

/*=====[Prototypes (declarations) of public functions]=======================*/
uartManagerError_t uartManagerInit(uartManagerHandle_t* handler, uartManagerConfig_t config);
void uartManagerDeinit(uartManagerHandle_t handle);

uartManagerError_t uartManagerGet(uartManagerHandle_t handle, uint8_t* msg, uint32_t* size, uint32_t timeout);
uartManagerError_t uartManagerPut(uartManagerHandle_t handle, uint8_t* msg, uint32_t timeout);

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* _UART_MANAGER_H_ */
