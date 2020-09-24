///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2019 Jim Skrentny
// Posting or sharing this file is prohibited, including any changes/additions.
//
///////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Main File:        heapAlloc.c
// This File:        heapAlloc.c
// Other Files:      (name of all other files if any)
// Semester:         CS 354 Fall 2019
//
// Author:           Sage Livingstone
// Email:            slivingstone@wisc.edu
// CS Login:         sage
//
/////////////////////////// OTHER SOURCES OF HELP //////////////////////////////
//                   fully acknowledge and credit all sources of help,
//                   other than Instructors and TAs.
//
// Persons:          Identify persons by name, relationship to you, and email.
//                   Describe in detail the the ideas and help they provided.
//
// Online sources:   avoid web searches to solve your problems, but if you do
//                   search, be sure to include Web URLs and description of 
//                   of any information you find.
//////////////////////////// 80 columns wide ///////////////////////////////////


#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include "heapAlloc.h"

/*
 * This structure serves as the header for each allocated and free block.
 * It also serves as the footer for each free block but only containing size.
 */
typedef struct blockHeader {
        int size_status;
    /*
    * Size of the block is always a multiple of 8.
    * Size is stored in all block headers and free block footers.
    *
    * Status is stored only in headers using the two least significant bits.
    *   Bit0 => least significant bit, last bit
    *   Bit0 == 0 => free block
    *   Bit0 == 1 => allocated block
    *
    *   Bit1 => second last bit 
    *   Bit1 == 0 => previous block is free
    *   Bit1 == 1 => previous block is allocated
    * 
    * End Mark: 
    *  The end of the available memory is indicated using a size_status of 1.
    * 
    * Examples:
    * 
    * 1. Allocated block of size 24 bytes:
    *    Header:
    *      If the previous block is allocated, size_status should be 27
    *      If the previous block is free, size_status should be 25
    * 
    * 2. Free block of size 24 bytes:
    *    Header:
    *      If the previous block is allocated, size_status should be 26
    *      If the previous block is free, size_status should be 24
    *    Footer:
    *      size_status should be 24
    */
} blockHeader;         

/* Global variable - DO NOT CHANGE. It should always point to the first block,
 * i.e., the block at the lowest address.
 */

blockHeader *heapStart = NULL;
blockHeader *lastAllocated = NULL;

/* 
 * Function for allocating 'size' bytes of heap memory.
 * Argument size: requested size for the payload
 * Returns address of allocated block on success.
 * Returns NULL on failure.
 * This function should:
 * - Check size - Return NULL if not positive or if larger than heap space.
 * - Determine block size rounding up to a multiple of 8 and possibly adding padding as a result.
 * - Use NEXT-FIT PLACEMENT POLICY to chose a free block
 * - Use SPLITTING to divide the chosen free block into two if it is too large.
 * - Update header(s) and footer as needed.
 * Tips: Be careful with pointer arithmetic and scale factors.
 */
void* allocHeap(int size) {
    // Pointer to keep track of last allocated block
    if (lastAllocated == NULL){
        lastAllocated = heapStart;
    }    

    // Error condition
    if (size <= 0) {
        return NULL;
    }

    // Add header size
    int blocksize = size + 4;

    // Calculate padding
    int remainder = blocksize % 8;
    if (remainder != 0) {
        remainder = 8 - remainder;
    }
    blocksize += remainder;

    // Find next correct-size free block
    blockHeader* searchPosition = lastAllocated;
    int currentSize = 0;
    int a = 0;
    int p = 0;
    int has_looped = 0;
    while (1)
    {
        currentSize = searchPosition->size_status;
        a = 0;
        p = 0;
        
        // Check if we are at the end of heap
        if (currentSize == 1)
        {
            searchPosition = heapStart;
            has_looped = 1;
            continue;
        }

        // Check a-bit
        if ((currentSize % 2) == 1)
        {
            a = 1;
            currentSize -= 1;
        }

        // Check p-bit
        if ((currentSize % 8) != 0)
        {
            p = 1;
            currentSize -= 2;
        }

        // Check if we can use this block
        if (!a)
        {
            // Check if this block is big enough
            if (currentSize >= blocksize)
            {
                // We found a suitable block, move on to allocation
                break;
            }
        }

        // Increment pointer since suitable block not yet found
        searchPosition = (blockHeader*) ((void*)searchPosition + currentSize);
        
        // Check if we're back at the start
        if (has_looped)
        {
            if (searchPosition >= lastAllocated)
            {
                // We searched the entire heap and didn't find anything :(
                return NULL;
            }
        }
    }
    
    // ** If we made it here we found a block to allocate **

    // Update header with new size
    int newSize = blocksize + (2*p) + 1;
    searchPosition->size_status = newSize;

    // Check if we need to split
    if ((currentSize - blocksize) != 0)
    {
        // We need to split
        int freesize = currentSize - blocksize;

        // Set free block header contents
        blockHeader* freeptr = (blockHeader*) ((void*)searchPosition + blocksize);
        freeptr->size_status = freesize + 2;
        
        // Set free block footer contents
        blockHeader* freeftr = (blockHeader*) ((void*)freeptr + freesize - 4);
        freeftr->size_status = freesize;
    }
    else
    {
        // Check next block and set its p-bit
        blockHeader* nextBlock = (blockHeader*) ((void*)searchPosition + blocksize);
        nextBlock->size_status += 2;
    }
    
    // Update last allocated position for next-fit logic
    lastAllocated = searchPosition;
    
    // Return pointer found block payload
    return ((blockHeader*) ((void*)searchPosition + 4));
}

/* 
 * Function for freeing up a previously allocated block.
 * Argument ptr: address of the block to be freed up.
 * Returns 0 on success.
 * Returns -1 on failure.
 * This function should:
 * - Return -1 if ptr is NULL.
 * - Return -1 if ptr is not a multiple of 8.
 * - Return -1 if ptr is outside of the heap space.
 * - Return -1 if ptr block is already freed.
 * - USE IMMEDIATE COALESCING if one or both of the adjacent neighbors are free.
 * - Update header(s) and footer as needed.
 */                    
int freeHeap(void *ptr) {         
    // Check null ptr
    if (ptr == NULL) {
        return -1;
    }

    // Check pointer alignment
    if (((int)ptr % 8) != 0) {
        return -1;
    }

    // Check if ptr is before heap start
    if ((void*)ptr < (void*)heapStart) {
        return -1;
    }

    // Check if ptr is after or at heap end
    blockHeader* search = heapStart;
    while (1)
    {
        int status = search->size_status;
        if (status != 1)
        {
            if ((status % 2) == 1) {
                status--;
            }
            if ((status % 8) != 0) {
                status -= 2;
            }

            search = (blockHeader*) ((void*)search + status);
        }
        else
        {
            break;
        }
    }
    if ((void*)ptr >= (void*)search) {
        return -1;
    }
   
    // Get status of current block
    blockHeader* head = (blockHeader*) ((void*)ptr - 4);
    int blocksize = head->size_status;
    int a = 0;
    int p = 0;
    if ((blocksize % 2) == 1) {
        a = 1;
        blocksize -= 1;
    }

    if ((blocksize % 8) != 0) {
        p = 1;
        blocksize -= 1;
    }
    
    // Fail if block is already free
    if (a == 0) {
        return -1;
    }

    // ** If we made it here we need to free the block **

    // Check left direction for coalesce
    blockHeader* leftPtr = head;
    int currentsize = leftPtr->size_status;
    while (1)
    {
        currentsize = leftPtr->size_status;
        a = 0;
        p = 0;
        if ((currentsize % 2) == 1) {
            a = 1;
            currentsize -= 1;
        }
        if ((currentsize % 8) != 0) {
            p = 1;
            currentsize -= 2;
        }

        // Check if at heap start
        if ((void*)leftPtr == (void*)heapStart) {
            break;
        }
    
        // Continue if next (left) block is free
        if (p == 0) {
            blockHeader* prevFooter = (blockHeader*) ((void*)head - 4);
            int footerSize = prevFooter->size_status;
            leftPtr = (blockHeader*) ((void*)leftPtr - footerSize);
        }
        else {
            break;
        }
    }

    // Check right direction for coalesce 
    blockHeader* rightPtr = head;
    currentsize = rightPtr->size_status;
    while (1)
    {
        currentsize = rightPtr->size_status;
        a = 0;
        p = 0;

        // Check if at end marker of heap
        if (currentsize == 1) {
            break;
        }

        // Check p and a bits
        if ((currentsize % 2) == 1) {
            a = 1;
            currentsize -= 1;
        }

        if ((currentsize % 8) != 0) {
            p = 1;
            currentsize -= 2;
        }

        // Continue if this block is free or this is the block being freed
        if (a == 0 || ((void*)rightPtr == (void*)head) )
        {
            rightPtr = (blockHeader*) ((void*)rightPtr + currentsize);
        }
        else {
            break; // Result is a pointer to the next allocated block or end marker
        }
    }

    // Calc coalesce space size
    int totalSize = (void*)rightPtr - (void*)leftPtr;

    blockHeader* footer = (blockHeader*) ((void*)rightPtr - 4);

    // Set header
    leftPtr->size_status = totalSize + 2;
    
    // Set footer
    footer->size_status = totalSize;

    // Update next block header
    if (p == 1) {
        rightPtr->size_status -= 2;
    }
    lastAllocated = leftPtr;

    return 0;
}

/*
 * Function used to initialize the memory allocator.
 * Intended to be called ONLY once by a program.
 * Argument sizeOfRegion: the size of the heap space to be allocated.
 * Returns 0 on success.
 * Returns -1 on failure.
 */                    
int initHeap(int sizeOfRegion) {         

    static int allocated_once = 0; //prevent multiple initHeap calls

    int pagesize;  // page size
    int padsize;   // size of padding when heap size not a multiple of page size
    int allocsize; // size of requested allocation including padding
    void* mmap_ptr; // pointer to memory mapped area
    int fd;

    blockHeader* endMark;
  
    if (0 != allocated_once) {
        fprintf(stderr, 
        "Error:mem.c: InitHeap has allocated space during a previous call\n");
        return -1;
    }
    if (sizeOfRegion <= 0) {
        fprintf(stderr, "Error:mem.c: Requested block size is not positive\n");
        return -1;
    }

    // Get the pagesize
    pagesize = getpagesize();

    // Calculate padsize as the padding required to round up sizeOfRegion 
    // to a multiple of pagesize
    padsize = sizeOfRegion % pagesize;
    padsize = (pagesize - padsize) % pagesize;

    allocsize = sizeOfRegion + padsize;

    // Using mmap to allocate memory
    fd = open("/dev/zero", O_RDWR);
    if (-1 == fd) {
        fprintf(stderr, "Error:mem.c: Cannot open /dev/zero\n");
        return -1;
    }
    mmap_ptr = mmap(NULL, allocsize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (MAP_FAILED == mmap_ptr) {
        fprintf(stderr, "Error:mem.c: mmap cannot allocate space\n");
        allocated_once = 0;
        return -1;
    }
  
    allocated_once = 1;

    // for double word alignment and end mark
    allocsize -= 8;

    // Initially there is only one big free block in the heap.
    // Skip first 4 bytes for double word alignment requirement.
    heapStart = (blockHeader*) mmap_ptr + 1;

    // Set the end mark
    endMark = (blockHeader*)((void*)heapStart + allocsize);
    endMark->size_status = 1;

    // Set size in header
    heapStart->size_status = allocsize;

    // Set p-bit as allocated in header
    // note a-bit left at 0 for free
    heapStart->size_status += 2;

    // Set the footer
    blockHeader *footer = (blockHeader*) ((char*)heapStart + allocsize - 4);
    footer->size_status = allocsize;
  
    return 0;
}         
                 
/* 
 * Function to be used for DEBUGGING to help you visualize your heap structure.
 * Prints out a list of all the blocks including this information:
 * No.      : serial number of the block 
 * Status   : free/used (allocated)
 * Prev     : status of previous block free/used (allocated)
 * t_Begin  : address of the first byte in the block (where the header starts) 
 * t_End    : address of the last byte in the block 
 * t_Size   : size of the block as stored in the block header
 */                     
void dumpMem() {  

    int counter;
    char status[5];
    char p_status[5];
    char *t_begin = NULL;
    char *t_end   = NULL;
    int t_size;

    blockHeader *current = heapStart;
    counter = 1;

    int used_size = 0;
    int free_size = 0;
    int is_used   = -1;

    fprintf(stdout, "************************************Block list***\
                    ********************************\n");
    fprintf(stdout, "No.\tStatus\tPrev\tt_Begin\t\tt_End\t\tt_Size\n");
    fprintf(stdout, "-------------------------------------------------\
                    --------------------------------\n");
  
    while (current->size_status != 1) {
        t_begin = (char*)current;
        t_size = current->size_status;
    
        if (t_size & 1) {
            // LSB = 1 => used block
            strcpy(status, "used");
            is_used = 1;
            t_size = t_size - 1;
        } else {
            strcpy(status, "Free");
            is_used = 0;
        }

        if (t_size & 2) {
            strcpy(p_status, "used");
            t_size = t_size - 2;
        } else {
            strcpy(p_status, "Free");
        }

        if (is_used) 
            used_size += t_size;
        else 
            free_size += t_size;

        t_end = t_begin + t_size - 1;
    
        fprintf(stdout, "%d\t%s\t%s\t0x%08lx\t0x%08lx\t%d\n", counter, status, 
        p_status, (unsigned long int)t_begin, (unsigned long int)t_end, t_size);
    
        current = (blockHeader*)((char*)current + t_size);
        counter = counter + 1;
    }

    fprintf(stdout, "---------------------------------------------------\
                    ------------------------------\n");
    fprintf(stdout, "***************************************************\
                    ******************************\n");
    fprintf(stdout, "Total used size = %d\n", used_size);
    fprintf(stdout, "Total free size = %d\n", free_size);
    fprintf(stdout, "Total size = %d\n", used_size + free_size);
    fprintf(stdout, "***************************************************\
                    ******************************\n");
    fflush(stdout);

    return;  
}  
