/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"
// for test
#include <inc/stdio.h>
// end test

//==============DECLARE LIST TO HOLD META BLOCKS=====================//

struct MemBlock_LIST linkedListMemoryBlocks;

//===================================================================//

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
uint32 get_block_size(void *va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1);
	return curBlkMetaData->size;
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
int8 is_free_block(void *va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1);
	return curBlkMetaData->is_free;
}

//===========================================
// 3) ALLOCATE BLOCK BASED ON GIVEN STRATEGY:
//===========================================
void *alloc_block(uint32 size, int ALLOC_STRATEGY)
{
	void *va = NULL;
	switch (ALLOC_STRATEGY)
	{
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_blocks_list(struct MemBlock_LIST list)
{
	cprintf("=========================================\n");
	struct BlockMetaData *blk;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &list)
	{
		cprintf("(size: %d, isFree: %d)\n", blk->size, blk->is_free);
	}
	cprintf("=========================================\n");
}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
//============================
// Helper functions
void *initializeMetaDataBlock(uint32 sAddress, uint32 size, uint32 free)
{
	struct BlockMetaData *intializeBlock = (struct BlockMetaData *)sAddress;
	intializeBlock->is_free = 1;
	intializeBlock->size = size;
	return intializeBlock;
}
void clearMetaDataBlock(void *sAddress)
{
	memset(sAddress, 0, sizeOfMetaData());
}
void printList(){
	struct BlockMetaData *currentBlock;
	LIST_FOREACH(currentBlock, &linkedListMemoryBlocks){
		cprintf("address : %x , isFree : %d , size : %d \n", (void *) currentBlock, currentBlock->is_free, currentBlock->size);
	}
}
void printBlock (void * Address){
	struct BlockMetaData *currentBlock = (struct BlockMetaData *)Address;
	cprintf("Block : address : %x , isFree : %d , size : %d \n", (void *) currentBlock, currentBlock->is_free, currentBlock->size);
}
//merges block at va with following one if it's free.
void coalesce(void* va){
	void* real_add = va-sizeOfMetaData();
	struct BlockMetaData *blk1 = (struct BlockMetaData*) real_add;
	if(!blk1->is_free)
			return;
	struct BlockMetaData *blk2 = (struct BlockMetaData *)LIST_NEXT(blk1);
	if(blk2==NULL)
		return;
	if(!blk2->is_free)
		return;
	blk1->size+=blk2->size;
	LIST_REMOVE(&linkedListMemoryBlocks,blk2);
	clearMetaDataBlock(blk2);
}

//============================
//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{
	//=========================================
	// DON'T CHANGE THESE LINES=================
	if (initSizeOfAllocatedSpace == 0)
		return;
	//=========================================
	//=========================================
	struct BlockMetaData *initializeDynamicBlock = (struct BlockMetaData *)daStart;
	initializeDynamicBlock->is_free = 1;
	initializeDynamicBlock->size = initSizeOfAllocatedSpace;

	// intialize the list
	printBlock(initializeDynamicBlock);
	LIST_INIT(&linkedListMemoryBlocks);
	LIST_INSERT_HEAD(&linkedListMemoryBlocks, initializeDynamicBlock);
	// TODO: [PROJECT'23.MS1 - #5] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator()
	// panic("initialize_dynamic_allocator is not implemented yet");
}

//=========================================
// [4] ALLOCATE BLOCK BY FIRST FIT:
//=========================================


void *alloc_block_FF(uint32 size)
{
	//TODO: [PROJECT'23.MS1 - #6] [3] DYNAMIC ALLOCATOR - alloc_block_FF()
	if (size == 0)
		return NULL;
	struct BlockMetaData *currentBlock, *firstFitBlock;
	uint32 totalRequiredSize = size + sizeOfMetaData();
	int isFound = 0;
	LIST_FOREACH(currentBlock, &linkedListMemoryBlocks){
		if (isFound == 0 && currentBlock->size >= totalRequiredSize && currentBlock->is_free == 1){
			isFound = 1;
			firstFitBlock = currentBlock;
		}
	}
	if (isFound){

		uint32 remainFreeSize = firstFitBlock->size - totalRequiredSize;
		uint32 startOfFreeBlock = (uint32) firstFitBlock + totalRequiredSize;
		struct BlockMetaData *freeBlock;
		freeBlock = NULL;
		if (remainFreeSize >= sizeOfMetaData()){
			freeBlock = (struct BlockMetaData *) initializeMetaDataBlock(startOfFreeBlock, remainFreeSize, 1);
		}
		firstFitBlock->size = totalRequiredSize;
		firstFitBlock->is_free = 0;
		if(freeBlock != NULL)
			LIST_INSERT_AFTER(&linkedListMemoryBlocks, firstFitBlock, freeBlock);
		uint32 blockStart = (uint32)firstFitBlock + (uint32)sizeOfMetaData();
		return (void*) blockStart;
	}else{
		struct BlockMetaData *tail = LIST_LAST(&linkedListMemoryBlocks);
		if (tail->is_free){
			uint32 neededSize = totalRequiredSize - tail->size;
			if ((uint32)sbrk(neededSize) == -1) return NULL;
			tail->size = totalRequiredSize;
			tail->is_free = 0;
			return (void *) ((uint32) tail + (uint32) sizeOfMetaData());
		}else{
			void *oldSbrk = sbrk(0);
			if ((uint32)sbrk(totalRequiredSize) == -1) return NULL;
			struct BlockMetaData *addedBlock = (struct BlockMetaData *) initializeMetaDataBlock((uint32)oldSbrk, totalRequiredSize, 0);
			LIST_INSERT_TAIL(&linkedListMemoryBlocks, addedBlock);
			return (void *) ((uint32) addedBlock + (uint32) sizeOfMetaData());
		}
	}
	//panic("alloc_block_FF is not implemented yet");
	return NULL;
}
//=========================================
// [5] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size)
{
	// TODO: [PROJECT'23.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF()
	panic("alloc_block_BF is not implemented yet");
	return NULL;
}

//=========================================
// [6] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size)
{
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [7] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size)
{
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}

//===================================================
// [8] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va)
{
	if (va == NULL)
		return;
	struct BlockMetaData *deAllocatedBlock = (struct BlockMetaData *)((uint32) va - (uint32) sizeOfMetaData());
	struct BlockMetaData *prev = (struct BlockMetaData *)LIST_PREV(deAllocatedBlock);
	deAllocatedBlock->is_free= 1;
	coalesce(va);
	if(prev!=NULL)
		coalesce((void*)prev+sizeOfMetaData());
}

//=========================================
// [4] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size){
	if(va==NULL&&new_size==0)
		return NULL;

	if(va==NULL)
		return alloc_block_FF(new_size);

	if(new_size==0){
		free_block(va);
		return NULL;
	}

	uint32 real_size = new_size + sizeOfMetaData();

	void* blk_add = va-sizeOfMetaData();
	struct BlockMetaData *cur_block = (struct BlockMetaData*) blk_add;

	if(real_size == cur_block->size)
		return va;

	if(cur_block->size>real_size){
		if(cur_block->size-real_size>=sizeOfMetaData()){
			struct BlockMetaData* holder = (struct BlockMetaData*) (blk_add+real_size);
			holder->size = cur_block->size-real_size;
			holder->is_free = 1;
			LIST_INSERT_AFTER(&linkedListMemoryBlocks,cur_block,holder);
		}
		cur_block->size = real_size;
		return va;
	}

	struct BlockMetaData *next_block  =LIST_NEXT(cur_block);
	if(next_block->is_free&&next_block->size+cur_block->size>=real_size){
		cur_block->size+=next_block->size;
		LIST_REMOVE(&linkedListMemoryBlocks,next_block);
		clearMetaDataBlock(next_block);
		return realloc_block_FF(va,new_size);
	}
	void* new_address = alloc_block_FF(new_size);
	struct BlockMetaData *new_block = (struct BlockMetaData*)(new_address+sizeOfMetaData());
	uint32* new_data = new_address+sizeOfMetaData();
	uint32* old_data = va;
	for(;old_data!=(uint32*)next_block;old_data++,new_data++)
		*new_data = *old_data;

	free_block(va);
	return new_address+sizeOfMetaData();
}
