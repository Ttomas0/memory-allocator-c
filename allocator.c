#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#include "allocator.h"


const int MAGICAL_BYTES = 0x55;
const int BLOCK_MARKER = 0xDD; 

/**
 * @details 
 * - MAGICAL_BYTES: the value for ensuring the allocator is properly initialized
 * - BLOCK_MARKER: the value that each block's marker must match to ensure it has not been corrupted
 * 
 */



my_stats *get_malloc_headear(){
    return (my_stats *)heap_start; 
}

area *find_last_block(){

    area *block = (area *)((char *)heap_start + sizeof(my_stats));
    while (block->next != NULL ){
        block = block->next;
    }
    return block;
} 

/**
 * @brief 
 * - find_last_block(): serch the blocks until there are no more blocks next
 */ 


/**
 * @brief 
 * - get_malloc_headear(): the return is straight form the my_stats *malloc_header = (my_stats *)heap_start in the an_malloc() function
 */ 

 int *add_used_block(ssize_t size){
      // format the heap_start with the shape of stats
      my_stats *malloc_header = get_malloc_headear();
      while (malloc_header->my_simple_lock){
        sleep(1); //its include in unistd.h and pauses 1 second so two thread couldnt get call at the same time 
      }
      malloc_header->my_simple_lock = true;
    // find smallest space in the free blocks and add there.
    area *block = (area*)((char *)heap_start + sizeof(my_stats));
    area *smallest_block = NULL;
    area *last_block = block;
    // best fit
    while(block != NULL){
        assert(block->marker == BLOCK_MARKER);
        if((block->length + sizeof(area)) >= size && block->InUse == false){  //is the block enough to save the data and is the block free?
            if(smallest_block == NULL || smallest_block->length > block->length){
                smallest_block = block; //rename te smallest block
            }
        }
        last_block = block;
        block = block->next;

         // no big enough blocks.
        if(smallest_block == NULL){
            area *last_block = find_last_block();
            while(last_block->length < size){
                sbrk(4096); //ask for more memory to the OS
                last_block->length += 4096;
                malloc_header->amount_of_pages += 1;
            }
            smallest_block = last_block;
        }
    }
    smallest_block->InUse = true;
    int must_have_size = smallest_block->length - size - sizeof(area) - 1; //sizeof(area) its the header of the new block just created the -1 guarantee that the new block have at least 1 byte

    if(must_have_size <= 0){
        sbrk(4096); //because the lack of memory for future blocks
        malloc_header->amount_of_pages += 1;
        smallest_block->length += 4096;
        must_have_size = smallest_block->length - size - sizeof(area) - 1; //this fix a bug from the author of the article, we just give size to the block that needs it
    }
    int remaining_sizes = must_have_size + 1;
    malloc_header->amount_of_blocks += 1; //we are gonna create a new block for the remaining_size
    area *new_block = (area *)((char *)last_block + sizeof(area) + size); //the direction of the new block
    new_block->marker = BLOCK_MARKER;
    new_block->InUse = false;
    new_block->length = remaining_sizes;
    new_block->prev = smallest_block;
    new_block->next = smallest_block->next; //it points to whatever was after the smallest_block after the next block
    if(new_block->next != NULL){
        (new_block->next)->prev = new_block; //this means that the last block is the new block
    }
    smallest_block->next = new_block;
    new_block->length = remaining_sizes;
    smallest_block->length = size; //now the rest of the spaces is given to the new block
    malloc_header->my_simple_lock = false;
    return (int *)((char *)smallest_block + sizeof(area)); //how we chosee to allocate the memory in the samllest block we return the malloc of the smallest one
 
 }


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
    
    heap_start = MAGICAL_BYTES;
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
    return add_used_block(size);
}

//[ my_stats | first_block              ]
//    ^           ^
// heap_start  heap_start + sizeof(my_stats)


//Now we gonna free the memory

area *find_previous_used_block(area *ptr) {
    area *last_block = ptr->prev;
    while (last_block != NULL) {
        if (last_block->InUse == true) {
            return last_block; // returns the closest
        }
        last_block = last_block->prev;
    }
    return NULL;
}


void reduce_heap_size_if_possible() {
    area *last_block = find_last_block();
    area *prev_used_block = find_previous_used_block(last_block);

    if (prev_used_block == NULL){
        if(last_block->length > PAGE_SIZE){
        last_block->length = PAGE_SIZE;
    }
    prev_used_block = last_block;
    }
    
    void *new_end = (void *)prev_used_block + sizeof(area) + prev_used_block->length; //prev_used_block where the prev used block begin 

    void *heap_end = sbrk(0); //tells you the current memory adress
    while(new_end < heap_end - PAGE_SIZE){
        sbrk(-PAGE_SIZE); //reduce the heap until its less than the new end -- this is why in most cases we have a gap, we are cuting by page size 
        heap_end = sbrk(0);
        my_stats *malloc_header = get_malloc_headear();
        malloc_header->amount_of_pages -= 1; //we cut the page
    }
     // Now, in most cases, there is a gap between the last used block and the end
     // of the heap. If there is enough space, we can inyect there an empty block.
     // At least, we write 1 element in the block

     //[ last_block_used ][ leftover space ][ end of heap ]
    if(heap_end - new_end > sizeof(area) + 1){ //we left a space for a new block with is header + 1
        area *new_not_used_block = (area *)new_end;
        new_not_used_block->marker = BLOCK_MARKER;
        new_not_used_block->InUse = false;
        new_not_used_block->prev = prev_used_block;
        new_not_used_block->next = NULL;
        new_not_used_block->length = heap_end - new_end - sizeof(area);
        prev_used_block->next = new_not_used_block;
    }
}


bool an_free(void *ptr){
    bool marker_state; //we use a var so we can avoid double returns
    my_stats *malloc_header = get_malloc_headear();
    while(malloc_header->my_simple_lock){
        sleep(1);
    }
    malloc_header->my_simple_lock = true;

    //we need access to the area (the header) to know what data to free

    area *block = ptr - sizeof(area); //each block have a header the objetive with this is to find where it starts (it goes from back to front)

    if(block->marker != BLOCK_MARKER){
        return marker_state = false;
    }else{
        block->InUse = false;
        memset(ptr, 0, block->length); //this put all the bytes in 0s so basically it delete all the information
        if(block->next != NULL && (block->next)->InUse == false){
            // Next block is not used, we can merge them
            area *not_used_next_block = block->next;
            if(not_used_next_block != NULL){
                block->next = not_used_next_block->next; //block <-> not_used_next_block <-> [next] so now we made block <-> [next]
                if(not_used_next_block->next != NULL){
                    (not_used_next_block->next)->prev = block; //we make the pointer to point to the block that sould be pointing
                }else{
                    block->next = NULL;
                }

                block->length += not_used_next_block->length + sizeof(area); //we determine the length of the updated block

                memset((void *)not_used_next_block, 0, sizeof(area) + not_used_next_block->length); //puts the direction of not_used_next_block in all 0s bytes
                
                malloc_header->amount_of_blocks -= 1;


            }

            if(block->prev != NULL && (block->prev)->InUse == false){
                area *to_delete_block = block;
                block = block->prev;

                block->length += to_delete_block->length + sizeof(area);
                block->next = to_delete_block->next; //update where the pointer is pointing at
            }
            if(block->next != NULL){
                block->next->prev = block;
            }
            malloc_header->amount_of_blocks -= 1;
        }
        reduce_heap_size_if_possible();
    }
    malloc_header->my_simple_lock = false;
    marker_state = true;
    return marker_state;

}











int main(){
    
    
    printf("hola");
    
    return 0;
}