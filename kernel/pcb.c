#include <memory.h>
#include <sys_req.h>
#include <pcb.h>
#include <string.h>
#include <stdlib.h>
#include <mpx/user_commands.h>

extern struct queue readyPtr;
extern struct queue blockedPtr;
extern struct pcb *cur_process;

// Allocates memory to a new PCB struct
struct pcb *pcb_allocate()
{
    // int pcbSize[20];
    //  Set the process equal to memory allocated
    // stack = data type * size of stack
    // needs to be pcb create
    struct pcb *newpcb = sys_alloc_mem(sizeof(pcb));

    if (newpcb == NULL)
    {
        char *errorHandler = "Allocation failed\n";
        sys_req(WRITE, COM1, errorHandler, sizeof(errorHandler));
    }

    unsigned char *stack = sys_alloc_mem(STACK_SIZE);

    if (stack == NULL)
    {
        char *errorHandler = "Stack allocation failed\n";
        sys_req(WRITE, COM1, errorHandler, sizeof(errorHandler));
    }

    newpcb->base = stack;
    newpcb->bottom = newpcb->base + STACK_SIZE;
    newpcb->top = newpcb->bottom - sizeof(Context);
    // Return the process with allocated memory
    return newpcb;
}

// Frees all memory from the PCB
int pcb_free(struct pcb *pcb)
{
    // Sets an integer equal to sys_free_mem of the PCB pointer
    int i = sys_free_mem(pcb);

    // Returns 0 on success
    if (i == 0)
    {
        return i;
    }

    // Returns an error message and the error number on error
    else
    {
        char *error = "Error: Could not free memory from pcb.\n";
        sys_req(WRITE, COM1, error, sizeof(error));
        return i;
    }

    return i;
}

// Sets up a new PCB using the name, class, and  queue priority
struct pcb *pcb_setup(char *name, int class, int priority)
{
    // Create new pcb struct through allocation
    struct pcb *newpcb = pcb_allocate();

    // Sets the new pcb name
    // name = malloc{sting copy / memcopy of string}
    newpcb->name = sys_alloc_mem((strlen(name) + 1) * sizeof(char));
    memcpy(newpcb->name, name, (strlen(name) + 1));

    newpcb->pcb_class = class;

    newpcb->priority = priority;

    newpcb->state = READY_STATE;

    return newpcb;
}

struct pcb *pcb_find(const char *desiredName)
{
    desiredName = upper_to_lower((char *)desiredName); 
    pcb *pcb_ptr = NULL;

    if (readyPtr.front != NULL)
    {
        pcb *curr_ptr = readyPtr.front;

        while (curr_ptr != NULL)
        {
            if (strcmp(curr_ptr->name, desiredName) == 0)
            {
                pcb_ptr = curr_ptr;
                break;
            }

            curr_ptr = curr_ptr->next;
        }
    }
    else if (blockedPtr.front != NULL)
    {
        pcb *curr_ptr = blockedPtr.front;

        while (curr_ptr != NULL)
        {
            if (strcmp(curr_ptr->name, desiredName) == 0)
            {
                pcb_ptr = curr_ptr;
                break;
            }

            curr_ptr = curr_ptr->next;
        }
    }

    return pcb_ptr;

    /*
    // If first in queue
    if ((readyPtr.front != NULL) && strcmp(readyPtr.front->name, desiredName) == 0)
    {
        return readyPtr.front;
    }
    pcb *newPtr = readyPtr.front;

    while (newPtr->next != NULL)
    {

        if (strcmp(newPtr->next->name, desiredName) == 0)
        {
            return newPtr->next;
        }
        newPtr = newPtr->next;
    }

    if ((blockedPtr.front != NULL) && strcmp(blockedPtr.front->name, desiredName) == 0)
    {
        return blockedPtr.front;
    }

    newPtr = blockedPtr.front;

    while (newPtr->next != NULL)
    {

        if (strcmp(newPtr->next->name, desiredName) == 0)
        {
            return newPtr;
        }
        newPtr = newPtr->next;
    }

    char *error = "No PCB Foud";
    sys_req(WRITE, COM1, error, strlen(error));

    // if (readyPtr.front != NULL)
    // {
    //     sys_req(WRITE, COM1, blockedPtr.front->name, strlen(blockedPtr.front->name));
    // }

    return newPtr;*/
}

int pcb_remove(struct pcb *p)
{
    // extern ready_queue *readyPtr;
    //THIS IS ESSENTIAL FOR SHUT DOWN TO WORK! Shut down passes a null pointer to remove and this checks to make sure that is what is hapening. - Jared
    if (p == NULL)
    {
        return 1;
    }

    // 0 for running, 1 for ready
    if (p->state == READY_STATE || p->state == READY_SUSPENDED_STATE)
    // Was not sure what these checks were doing so commented them out for now
    // p->state == RUNNING_STATE) && readyPtr.rear->next != NULL)
    {
        if (readyPtr.front == NULL)
        {
            // Nothing in queue
            return 1;
        }
        // One we are trying to delete is first in queue
        if (readyPtr.front == p)
        {
            readyPtr.front = readyPtr.front->next;
            readyPtr.count--;

            return 0;
        }
        pcb *prev_pcb = readyPtr.front;

        while (prev_pcb->next != p){
            prev_pcb = prev_pcb->next;
        }
        
        prev_pcb->next = prev_pcb->next->next;

        return 0;
    }
    else if (p->state == BLOCKED_STATE || p->state == BLOCKED_SUSPENDED_STATE)
    {
        if (blockedPtr.front == NULL)
        {
            // Nothing in queue
            return 1;
        }
        // One we are trying to delete is first in queue
        if (blockedPtr.front == p)
        {
            blockedPtr.front = blockedPtr.front->next;
            blockedPtr.count--;

            return 0;
        }
        pcb *prev_pcb = blockedPtr.front;

        while (prev_pcb->next != p){
            prev_pcb = prev_pcb->next;
        }
        
        prev_pcb->next = prev_pcb->next->next;

        return 0;
    }
    return 0;
}

// param is struct pcb *p
void pcb_insert(struct pcb *p)
{

    if (p->state == READY_STATE || p->state == READY_SUSPENDED_STATE)
    {

        if (readyPtr.front != NULL)
        {
            pcb *temp = readyPtr.front;
            while (temp->next != NULL && temp->priority <= p->priority)
            {
                temp = temp->next;
            }

            // Create new node for pcb insert

            pcb *tempPtr = temp->next;
            temp->next = p;
            p->next = tempPtr;
            tempPtr = NULL;
            readyPtr.count++;
        }
        else
        {

            readyPtr.front = p;
            readyPtr.count++;
        }
    }

    if (p->state == BLOCKED_STATE || p->state == BLOCKED_SUSPENDED_STATE)
    {

        if (blockedPtr.front != NULL)
        {

            pcb *temp = blockedPtr.front;
            while (temp->next != NULL)
            {
                temp = temp->next;
            }

            // Create new node for pcb insert
            pcb *newPcb = sys_alloc_mem(sizeof(newPcb));
            newPcb = p;

            pcb *tempPtr = temp->next;
            temp->next = newPcb;
            newPcb->next = tempPtr;
            tempPtr = NULL;
            blockedPtr.count++;
        }
        else
        {

            pcb *newFront = sys_alloc_mem(sizeof(newFront));
            newFront = p;
            blockedPtr.front = newFront;
            blockedPtr.count++;
        }
    }
}