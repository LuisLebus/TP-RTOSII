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

#include "pool.h"

/*=====[Inclusions of private function dependencies]=========================*/

/*=====[Definition macros of private constants]==============================*/

/*=====[Private function-like macros]========================================*/
#ifndef max
#define max(a,b) ((a)<(b)?(b):(a))
#endif

/*=====[Definitions of private data types]===================================*/

/*=====[Definitions of external public global variables]=====================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Prototypes (declarations) of private functions]======================*/

/*=====[Implementations of public functions]=================================*/
poolError_t poolInit(pool_t* me, uint32_t blocksSize, uint32_t blocksNum)
{
	me->blocksSize = max(blocksSize, sizeof(poolFree_t));
	me->blocksNum = blocksNum;

	me->blockMem = (uint8_t*)pvPortMalloc( me->blocksNum * me->blocksSize );
	if(me->blockMem == NULL)
		return POOL_ERROR;

	me->blocksUsed = 0;
	me->pFree = NULL;

	return POOL_OK;
}

void poolDeinit(pool_t* me)
{
	vPortFree(me->blockMem);
}

void* poolGet(pool_t* me)
{
	void* ret = NULL;

	if(me->pFree != NULL)
	{
		ret = me->pFree;
		me->pFree = me->pFree->next;
	}
	else
	{
		if(me->blocksUsed < me->blocksNum)
		{
			ret = me->blockMem + me->blocksUsed * me->blocksSize;
			me->blocksUsed++;
		}
	}

	return ret;
}

void poolPut(pool_t* me, void* ptr)
{
	poolFree_t* pFree = me->pFree;

	me->pFree = (poolFree_t*)ptr;
	me->pFree->next = pFree;
}

/*=====[Implementations of interrupt functions]==============================*/

/*=====[Implementations of private functions]================================*/

