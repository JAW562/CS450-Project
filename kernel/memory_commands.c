#include <mpx/vm.h>
#include <heap_manager.h>
#include <sys_req.h>
#include <pcb.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>

extern struct mcb *freeList;

void *memory_alloc(int size)
{
    char *error = "\nERROR: COULD NOT ALLOCATE MEMORY.\n";
    char *success = "\nMemory allocated successfully!";
    void *address;
    address = allocate_memory(size);

    if (address != NULL)
    {
        char *addressLocation = "\nThe address of the previously allocated memory is: ";
        sys_req(WRITE, COM1, success, strlen(success));
        sys_req(WRITE, COM1, addressLocation, strlen(addressLocation));
        return address;
    }
    else
    {
        sys_req(WRITE, COM1, error, strlen(error));
        return NULL;
    }
}

void memory_free(void *address)
{
    char *error = "\nError: Could not free memory!";
    char *success = "\nMemory freed successfully!";
    int check;

    check = free_memory(address);

    if (check != 0)
    {

        sys_req(WRITE, COM1, error, strlen(error));
    }

    else
    {

        sys_req(WRITE, COM1, success, strlen(success));
    }
}

void show_mcb(mcb *mcb)
{

    char dest[9] = {'\0'};
    numtoa((int)mcb->start, dest, 16);
    printf("\n\tMEMORY ADDRESS: %s", dest);
    printf("\n\tMEMORY SIZE: %d\n", mcb->size);
}

void show_alloc_mem()
{

    mcb *mcbPtr = freeList;

    sys_req(WRITE, COM1, "\nALLOCATED MEMORY: \n", strlen("\nALLOCATED MEMORY: \n"));

    while (mcbPtr != NULL)
    {
        if (mcbPtr->indicator == ALLOCATED_LIST)
        {
            show_mcb(mcbPtr);
            puts("\n");
        }
        mcbPtr = mcbPtr->next;
    }
}

void show_free_mem()
{

    mcb *mcbPtr = freeList;

    sys_req(WRITE, COM1, "\nFREE MEMORY: \n", strlen("\nFREE MEMORY: \n"));

    while (mcbPtr != NULL)
    {
        if (mcbPtr->indicator == FREE_LIST)
        {
            show_mcb(mcbPtr);
            puts("\n");
        }
        mcbPtr = mcbPtr->next;
    }
}
