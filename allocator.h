/**
 * @file allocator.h
 * @brief Custom memory allocator implementation
 *
 * @details This allocator:
 * - Keeps a list of free memory locations inside the heap (free list)
 * - Tracks all active allocations
 * - Returns free memory blocks on demand; if no block is large enough,
 *   requests the OS to grow the heap via sbrk()
 * - Frees chosen blocks on demand; may shrink the heap if possible
 * - Is thread-safe (safe to use from multiple threads simultaneously)
 */



#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H 

#include <unistd.h>

/**
*@brief prevets the header for being processed *more than once
*/

#include <stdbool.h>
#include <stdint.h>

#define PAGE_SIZE 4096

typedef uint8_t  u8;   
typedef uint16_t u16;  
typedef uint32_t u32; 
typedef uint64_t u64;  

typedef int8_t   i8;   
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef struct free_area{ 
    int marker;
    struct free_area *prev;
    bool InUse;
    u32 length;
    struct free_area *next;    
} area;

/**
 * @brief this is basically the structure of each of the blocks in the linked list.
 * @details - marker: is the value that all of the blocks must match in their marker, if a have a block stepping on other because of some error in which there are more bytes in the previous block that is suppoused to be, it will tell in which block the error is making trouble.
 * 
*/ 

typedef struct stast{
    int magical_bytes;
    bool my_simple_lock;
    u32 amount_of_blocks;
    u32 amount_of_pages;
}my_stats;

/**
 * @details 
 * - magical_byte: ensures the allocator was properly initialized before any allocation is attempted. If my_malloc is called without initializing the allocator first, this value will differ from MAGICAL_BYTES (0x55) and the operation will fail.
 * - my_simple_lock: this is for preventing some kind of error like initiating a double free on the same block of memory (examples in my README)
 * 
*/ 

void *heap_start = NULL;
void *heap_end = NULL;

#endif