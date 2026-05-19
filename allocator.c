#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "allocator.h"


const int MAGICAL_BYTES = 0x55;
const int BLOCK_MARKER = 0xDD; 

/**
 * @details 
 * - MAGICAL_BYTES: the value for ensuring the allocator is properly initialized
 * - BLOCK_MARKER: the value that each block's marker must match to ensure it has not been corrupted
 * 
 */


int *an_malloc(ssize_t size){
    if (heap_start == NULL){
        heap_start = sbrk(0); //the heap position now
        sbrk(4096); //makes 4096 new bytes for the heap
    }
    heap_end = sbrk(0);
    long int length = heap_end - heap_start;

    // First, we check if the magical bytes are at the beggining of the heap

    if (heap_start != MAGICAL_BYTES){
    // first execution of malloc
    
    *heap_start = MAGICAL_BYTES;
    my_stats *malloc_header = (my_stats *)heap_start;
    malloc_header->amount_of_blocks = 1;
    malloc_header->amount_of_pages = 1;
    area *fst_block = (area *)((char *)heap_start + sizeof(my_stats));  // the first block occupies all available heap space and will later be split into smaller blocks
    fst_block->marker = BLOCK_MARKER;
    fst_block->prev = NULL;
    fst_block->InUse = false; // false because this is the first block and no one has requested memory yet
    fst_block->length = length - sizeof(my_stats) - sizeof(area); // we subtract the size of the stats header and the area header from the total heap length 
    fst_block->next = NULL;
    }
}

//[ my_stats | first_block              ]
//    ^           ^
// heap_start  heap_start + sizeof(my_stats)



/**
 * @brief 
 * - *an_malloc: starts the heap and the header (personal doc memory allocator)
 * 
 * why heap_end is a char? its because the char occupies 1 byte 
 * why heap_end ends at the sbrk(0)? because we are writing from the end sbrk(4096) to the beginning sbrk(0)
 * 
 * [  heap_start ——————————→ heap_end  ]
 * 
 * 
 */ 


int main(){
    
    
    printf("hola");
    
    return 0;
}