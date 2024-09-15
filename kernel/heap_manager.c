#include <mpx/vm.h>
#include <heap_manager.h>
#include <sys_req.h>
#include <pcb.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <mpx/io.h>

extern struct mcb *freeList;

void initialize_heap(size_t size)
{
    struct mcb *newMCB;

    newMCB = kmalloc(size + sizeof(mcb), 0, NULL);

    // Start always 1 byte after mcb
    newMCB->start = (void *)((uint8_t *)newMCB + sizeof(mcb));

    newMCB->size = size;

    newMCB->next = NULL;

    newMCB->prev = NULL;

    newMCB->indicator = FREE_LIST;

    freeList = newMCB;
}

// Alloc Mem is WIP -Jacob
void *allocate_memory(size_t size)
{

    // Initialize a memory Block from the free list
    mcb *curBlock = freeList;

    // If bytes requested larger than size of mcb return an error
    while (curBlock != NULL)
    {
        if(curBlock->size >= size && curBlock->indicator == FREE_LIST){
            break;
        }

        curBlock = curBlock->next;
    }

    if (curBlock == NULL)
    {
        return NULL;
    }

    // Figure out what form of memory allocation we will do (easiest is first fit)
    // Memory Location will not change for initial allocated mcb
    // Memory location WILL CHANGE for the rest of the free list (point arithmatic)
    split(curBlock, size);
    // curBlock = curBlock->next;

    curBlock->indicator = ALLOCATED_LIST;

    return curBlock->start;
}
// Free_mem is a WIP -Jacob
int free_memory(void *block)
{
    mcb *mcb = freeList;

    while (mcb != NULL)
    {
        if (mcb->start == (void *)block)
        {
            break;
        }

        mcb = mcb->next;
    }

    if (mcb == NULL)
    {
        return 1;
    }

    mcb->indicator =FREE_LIST;
    merge(mcb);

    return 0;
}

// returns null if block is not split
// returns the mcb after the original block
mcb *split(mcb *blockToBeSplit, int size)
{
    if ((int)blockToBeSplit->size < size)
    {
        return NULL;
    }

    if (blockToBeSplit->size < (size + sizeof(mcb)))
    {
        return NULL;
    }

    mcb *new_mcb = (mcb *)((uint8_t *)blockToBeSplit->start + size);

    new_mcb->size = blockToBeSplit->size - size - sizeof(mcb);
    new_mcb->next = blockToBeSplit->next;
    new_mcb->start = (void *)((uint8_t *)new_mcb + sizeof(mcb));
    new_mcb->prev = blockToBeSplit;
    new_mcb->indicator = FREE_LIST;

    blockToBeSplit->next = new_mcb;
    blockToBeSplit->size = size;

    return new_mcb;

}

int merge(mcb *blockToBeMerged)
{
    mcb *next_mcb = blockToBeMerged->next;
    mcb *prev_mcb = blockToBeMerged->prev;

    if (next_mcb != NULL)
    {
        if (next_mcb->indicator == FREE_LIST)
        {
            blockToBeMerged->size = blockToBeMerged->size + sizeof(mcb) + next_mcb->size;
            mcb_remove(next_mcb);
        }
    }

    if (prev_mcb != NULL)
    {
        if (prev_mcb->indicator == FREE_LIST)
        {
            prev_mcb->size = prev_mcb->size + sizeof(mcb) + blockToBeMerged->size;
            mcb_remove(blockToBeMerged);
        }
    }

    if (next_mcb == NULL && prev_mcb == NULL)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int mcb_remove(struct mcb *m)
{
    // Doing this to not have to repeat code, curr list can be free or alloc
    mcb *curr_list = freeList;


    // If nothing after or before, empty
    if (m->next == NULL && m->prev == NULL)
    {
        curr_list->next = NULL;
        curr_list->next->prev = NULL;

        return 0;
        // Checks bottom first, if bottom and current are null, set tail to previous
        // essentially moving its view "up"
    }
    else if (m->next == NULL)
    {
        m->prev->next = NULL;
        m->prev = NULL;

        return 0;
        // Does the same thing as above, but iterates from top to bottom
    }
    else if (m->prev == NULL)
    {
        m->next->prev = NULL;
        m->next = NULL;

        return 0;
        // Last check to see if there might be one in the middle somewhere
        // Sets first and last node to point to each other and sets the middle node to point to both nulls
    }
    else
    {
        m->prev->next = m->next;
        m->next->prev = m->prev;

        m->prev = NULL;
        m->next = NULL;

        return 0;
    }

    return 1;
}
