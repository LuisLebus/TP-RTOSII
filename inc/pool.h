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

#ifndef _POOL_H_
#define _POOL_H_

/*=====[Inclusions of public function dependencies]==========================*/
#include <stdint.h>

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/

/*=====[Public function-like macros]=========================================*/

/*=====[Definitions of public data types]====================================*/
typedef struct poolFree_t {
	struct poolFree_t* next;
}poolFree_t;

typedef struct {
	uint32_t blocksNum;
	uint32_t blocksSize;
	uint32_t blocksUsed;
	poolFree_t* pFree;
	uint8_t* blockMem;
}pool_t;

typedef enum {
	POOL_OK = 0,
	POOL_ERROR
}poolError_t;

/*=====[Prototypes (declarations) of public functions]=======================*/
poolError_t poolInit(pool_t* me, uint32_t blocksSize, uint32_t blocksNum);
void poolDeinit(pool_t* me);

void* poolGet(pool_t* p);
void poolPut(pool_t* p, void* ptr);

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* _POOL_H_ */
