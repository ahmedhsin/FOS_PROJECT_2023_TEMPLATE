#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"
/***/
//#define MAX_NUMBERS_OF_FRAMES 1048576
uint32 KheapPagesTracker[NUM_OF_KHEAP_PAGES];
//uint32 KheapFramesTracker[MAX_NUMBERS_OF_FRAMES];
uint32 isTrackerInitilized = 0;
uint32 *tmpVal;
#define KPAGENUMBER(va) ((va - KERNEL_HEAP_START) / PAGE_SIZE)
//#define KFRAMENUMBER(va) 0
//#define KFRAMENUMBER(va) (to_frame_number(get_frame_info(ptr_page_directory, va, &tmpVal)))
	void initilizeTracker(){
	memset(KheapPagesTracker, 0, sizeof(KheapPagesTracker));
	//memset(KheapFramesTracker, 0, sizeof(KheapFramesTracker));
	isTrackerInitilized = 1;
}
int mall(uint32 va, uint32 st){
	struct FrameInfo *fr = NULL;
	allocate_frame(&fr);
	if (fr == NULL) return E_NO_MEM;
	map_frame(ptr_page_directory, fr, va, PERM_WRITEABLE);
	//KheapFramesTracker[KFRAMENUMBER(kheap_physical_address(va))] = va;
	fr->va = va;
	//cprintf("fr->va : %u\n", fr->va);
	KheapPagesTracker[KPAGENUMBER(va)] = st;
	return 0;
}
void unmall(uint32 va){
	//KheapFramesTracker[KFRAMENUMBER(kheap_physical_address(va))] = 0;
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
	uint32 new_block = blockSbrk+increment;
	if(new_block>blockHardLimit||new_block<startBlock)
			panic("out of range : sbrk");


	uint32 alloced_page = blockSbrk - blockSbrk%PAGE_SIZE;
	if(increment>0){
	blockSbrk += increment;
	if(blockSbrk%PAGE_SIZE)
		blockSbrk += PAGE_SIZE - blockSbrk%PAGE_SIZE;
	for(;alloced_page!=blockSbrk;alloced_page+=PAGE_SIZE)
		if(mall(alloced_page, -1)==E_NO_MEM)
					return (void*)-1;
	}

	if(increment<0){
		blockSbrk+=increment;
		for(;alloced_page>blockSbrk;alloced_page-=PAGE_SIZE)
			unmall(alloced_page);
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
}

void* kPagesAllocate(uint32 start, uint32 requiredPages){
	int maxPages = 0;
	uint32 startPages = 0;
	uint32 isMapped;
	for (uint32 currentAddress = start;currentAddress < KERNEL_HEAP_MAX; currentAddress += PAGE_SIZE){
		isMapped = KheapPagesTracker[KPAGENUMBER(currentAddress)];
		if (isMapped){
			maxPages = 0;
			startPages = 0;
			currentAddress += (isMapped - 1) * PAGE_SIZE;
		}else{
			maxPages++;
			if (startPages == 0) startPages = currentAddress;
		}
		if (maxPages >= requiredPages) break;
	}
	if (maxPages >= requiredPages)
	{
		uint32 pagesHead = startPages;
		while (maxPages--)
		{
			if (mall(pagesHead, requiredPages))
				return NULL;
			pagesHead += PAGE_SIZE;
		}
		return (void *)startPages;
	}
	return NULL;
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

	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");
	return kPagesAllocate(KheapStart, requiredPages);
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
		uint32 totalPages = KheapPagesTracker[KPAGENUMBER(va)];
		while(totalPages--){
			unmall(va);
			va += PAGE_SIZE;
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
	struct FrameInfo *fr = to_frame_info(physical_address);
	if (!fr->references) return 0;
	return (unsigned int)PGADDR(PDX(fr->va), PTX(fr->va), PGOFF(physical_address));

}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #06] [1] KERNEL HEAP - kheap_physical_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	//change this "return" according to your answer
	struct FrameInfo *fr =  get_frame_info(ptr_page_directory, virtual_address, &tmpVal);
	if (fr == NULL) return 0;
	return to_physical_address(fr) | PGOFF(virtual_address);
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
	if (virtual_address == NULL) return kmalloc(new_size);
	if (new_size <= DYN_ALLOC_MAX_BLOCK_SIZE){
		kfree(virtual_address);
		if (new_size > 0) return kmalloc(new_size);
		return NULL;
	}
	int requiredPages = (new_size % PAGE_SIZE == 0 ? new_size / PAGE_SIZE : new_size / PAGE_SIZE + 1);
	int actualPages = KheapPagesTracker[KPAGENUMBER((uint32)virtual_address)];
	if (requiredPages == actualPages) return virtual_address;
	if (requiredPages < actualPages){
		kfree(virtual_address);
		return kPagesAllocate((uint32)virtual_address, requiredPages);
	}
	uint32 freePagesN = 0, freePagesP = 0;
	uint32 tmp_va = (uint32)virtual_address + actualPages * PAGE_SIZE;
	while(!KheapPagesTracker[KPAGENUMBER(tmp_va)]){
		freePagesN++; tmp_va+=PAGE_SIZE;
		if (freePagesN == requiredPages-actualPages) break;
	}
	tmp_va = (uint32)virtual_address - PAGE_SIZE;
	while(freePagesN != requiredPages-actualPages && tmp_va > KheapStart && !KheapPagesTracker[KPAGENUMBER(tmp_va)]){
		freePagesP++; tmp_va-=PAGE_SIZE;
		if (freePagesP == requiredPages-actualPages) break;
	}
	if (freePagesN + freePagesP < requiredPages-actualPages){
		void *relocate = kmalloc(new_size);
		if (relocate == NULL) return virtual_address;
		kfree(virtual_address);
		return relocate;
	}
	return kPagesAllocate(tmp_va, requiredPages);
	//panic("krealloc() is not implemented yet...!!");
}
