#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"
/***/
#define MAX_NUMBERS_OF_FRAMES 1048576
uint32 KheapPagesTracker[NUM_OF_KHEAP_PAGES];
uint32 KheapFramesTracker[MAX_NUMBERS_OF_FRAMES];
uint32 isTrackerInitilized = 0;
uint32 *tmpVal;
#define KPAGENUMBER(va) ((va - KERNEL_HEAP_START) / PAGE_SIZE)
#define KFRAMENUMBER(va) 0
//#define KFRAMENUMBER(va) (to_frame_number(get_frame_info(ptr_page_directory, va, &tmpVal)))
	void initilizeTracker(){
	memset(KheapPagesTracker, 0, sizeof(KheapPagesTracker));
	memset(KheapFramesTracker, 0, sizeof(KheapFramesTracker));
	isTrackerInitilized = 1;
}
int mall(uint32 va, uint32 st){
	struct FrameInfo *fr = NULL;
	allocate_frame(&fr);
	if (fr == NULL) return E_NO_MEM;
	map_frame(ptr_page_directory, fr, va, PERM_WRITEABLE);
	KheapFramesTracker[KFRAMENUMBER(kheap_physical_address(va))] = va;
	KheapPagesTracker[KPAGENUMBER(va)] = st;
	return 0;
}
void unmall(uint32 va){
	KheapFramesTracker[KFRAMENUMBER(kheap_physical_address(va))] = 0;
	KheapPagesTracker[KPAGENUMBER(va)] = 0;
	unmap_frame(ptr_page_directory,va);
}

/***/


int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
	//TODO: [PROJECT'23.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator()
	//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
	//All pages in the given range should be allocated
	//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
	//Return:
	//	On success: 0
	//	Otherwise (if no memory OR initial size exceed the given limit): E_NO_MEM

	startBlock = ROUNDDOWN(daStart, PAGE_SIZE);
	blockSbrk = ROUNDUP(initSizeToAllocate + daStart, PAGE_SIZE);
	blockHardLimit = ROUNDUP(daLimit, PAGE_SIZE);
	KheapStart = blockHardLimit + PAGE_SIZE;
	if (!isTrackerInitilized) initilizeTracker();
	if (blockSbrk > blockHardLimit) return E_NO_MEM;
	for (uint32 current = startBlock; current < blockSbrk; current += PAGE_SIZE){
		if (mall(current, -1)) return E_NO_MEM;
	}
	initialize_dynamic_allocator(startBlock, blockSbrk - startBlock);

	//Comment the following line(s) before start coding...
	//panic("not implemented yet");
	return 0;
}

void* sbrk(int increment)
{
	if(!increment)
		return (void*)blockSbrk;

	if(increment%PAGE_SIZE){
		if(increment>0)
			increment += PAGE_SIZE-increment%PAGE_SIZE;
		else increment -= increment%PAGE_SIZE;
	}
	uint32 new_block = blockSbrk+increment;
	if(new_block>blockHardLimit||new_block<startBlock)
		panic("out of range : sbrk");
	int s = increment>0?1:-1;
	for(;blockSbrk != new_block;blockSbrk+=s*PAGE_SIZE){
		if(increment>0){
			if(mall(blockSbrk, -1)==E_NO_MEM)
				return (void*)-1;
		}
		else{
			unmall(blockSbrk - PAGE_SIZE);
		}

	}

	return (void*)blockSbrk;

	//TODO: [PROJECT'23.MS2 - #02] [1] KERNEL HEAP - sbrk()
	/* increment > 0: move the segment break of the kernel to increase the size of its heap,
	 * 				you should allocate pages and map them into the kernel virtual address space as necessary,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * increment = 0: just return the current position of the segment break
	 * increment < 0: move the segment break of the kernel to decrease the size of its heap,
	 * 				you should deallocate pages that no longer contain part of the heap as necessary.
	 * 				and returns the address of the new break (i.e. the end of the current heap space).
	 *
	 * NOTES:
	 * 	1) You should only have to allocate or deallocate pages if the segment break crosses a page boundary
	 * 	2) New segment break should be aligned on page-boundary to avoid "No Man's Land" problem
	 * 	3) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, kernel should panic(...)
	 */

	//MS2: COMMENT THIS LINE BEFORE START CODING====
	//return (void*)-1 ;
	//panic("not implemented yet");
}


void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'23.MS2 - #03] [1] KERNEL HEAP - kmalloc()
	//refer to the project presentation and documentation for details
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

	//change this "return" according to your answer

	if (!isTrackerInitilized) initilizeTracker();
	if (size <= DYN_ALLOC_MAX_BLOCK_SIZE)
		return alloc_block_FF(size);
	int requiredPages = (size % PAGE_SIZE == 0 ? size / PAGE_SIZE : size / PAGE_SIZE + 1);
	int maxPages = 0;
	uint32 startPages = 0;
	bool isMapped;
	for (uint32 currentAddress = KheapStart;currentAddress < KERNEL_HEAP_MAX; currentAddress += PAGE_SIZE){
		isMapped = (KheapPagesTracker[KPAGENUMBER(currentAddress)]);
		if (isMapped){
			maxPages = 0;
			startPages = 0;
		}else{
			maxPages++;
			if (startPages == 0) startPages = currentAddress;
		}
		if (maxPages >= requiredPages) break;
	}
	if (maxPages >= requiredPages)
	{
		uint32 pagesHead = startPages;
		while (requiredPages--)
		{
			if (mall(pagesHead, startPages))
				return NULL;
			pagesHead += PAGE_SIZE;
		}
		return (void *)startPages;
	}
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");
	return NULL;
}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #04] [1] KERNEL HEAP - kfree()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	uint32 va = (uint32)virtual_address;
	if (va >= startBlock && va < blockSbrk){
		free_block(virtual_address);
		return;
	}
	if (va >= KheapStart && va < KERNEL_HEAP_MAX){
		for (uint32 current = va;KheapPagesTracker[KPAGENUMBER(current)] == va; current += PAGE_SIZE){
			unmall(current);
		}
		return;
	}
	panic("invalid address : kfree");
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'23.MS2 - #05] [1] KERNEL HEAP - kheap_virtual_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================

	//change this "return" according to your answer
	return 0;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #06] [1] KERNEL HEAP - kheap_physical_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	//change this "return" according to your answer
	return 0;
}


void kfreeall()
{
	panic("Not implemented!");

}

void kshrink(uint32 newSize)
{
	panic("Not implemented!");
}

void kexpand(uint32 newSize)
{
	panic("Not implemented!");
}




//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'23.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc()
	// Write your code here, remove the panic and write your code
	return NULL;
	panic("krealloc() is not implemented yet...!!");
}
