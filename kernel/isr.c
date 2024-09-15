#include <pcb.h>
#include <sys_req.h>
#include <stddef.h>
#include "mpx/user_commands.h"
#include <mpx/io.h>
#include <serial.h>
#include <mpx/io_helpers.h>

extern struct queue readyPtr;
extern struct queue blockedPtr;
extern struct dcb *device1;
extern Context *initial_context;
extern pcb *cur_process;

// Context *stack_Ptr = NULL;

Context *sys_call(Context *process)
{

    if (device1 != NULL && device1->event_flag == 1)
    {
        device1->event_flag = 0;
        // return (Context *)new_iocb;
        // io_scheduler(COM1, IDLE, (char *)process->ECX, process->EDX);
        // cur_process->top = (unsigned char *)process;

        pcb *block_pcb = blockedPtr.front;

        // finds next ready pcb
        while (block_pcb != NULL)
        {
            if (block_pcb->state == BLOCKED_STATE)
            {
                break;
            }

            block_pcb = block_pcb->next;
        }

        pcb_remove(block_pcb);
        block_pcb->state = READY_STATE;
        pcb_insert(block_pcb);
        cur_process = NULL;

        // device1->process= cur_process;

        // pcb_remove(device1->process);

        // device1->process->state= READY_STATE;

        // pcb_insert(device1->process);

        device1->event_flag = 0;

        // Make device idle

        device1->status = IDLE;

        iocb *new_iocb = next_iocb(device1);

        if (new_iocb != NULL)
        {

            if (new_iocb->typeOP == READ)
            {

                // set dev1 proc to next icob
                // device1->process = (pcb*)new_iocb;
                // com read kik start, use iocb buffer and count
                serial_read(COM1, new_iocb->buffer, new_iocb->buffer_size);

                pcb_insert(new_iocb->process);
            }
            else
            {
                // set dev1 proc to next icob
                //  device1->process = (pcb*)new_iocb;
                // com read kik start, use iocb buffer and count
                serial_write(COM1, new_iocb->buffer, new_iocb->buffer_size);

                pcb_insert(new_iocb->process);
            }

            iocb_free(new_iocb);
        }
    }

    //     if (process->EAX == READ)
    //     {
    //         serial_read(process->EBX, (char *)process->ECX, process->EDX);
    //     }
    //     else if (process->EAX == WRITE)
    //     {
    //         serial_write(process->EBX, (char *)process->ECX, process->EDX);
    //     }

    // Check to see whether the requested device is currently in use

    pcb *next_pcb = readyPtr.front;

    // finds next ready pcb
    while (next_pcb != NULL)
    {
        if (next_pcb->state == READY_STATE)
        {
            break;
        }

        next_pcb = next_pcb->next;
    }

    pcb_remove(next_pcb);

    // either saves current context or processes op code
    if (cur_process == NULL)
    {
        initial_context = process;
        // return initial_context;
    }
    else
    {
        if (process->EAX == IDLE)
        {
            cur_process->top = (unsigned char *)process;
            cur_process->state = READY_STATE;
            pcb_insert(cur_process);
            cur_process = NULL;
        }
        else if (process->EAX == EXIT)
        {
            pcb_remove(cur_process);
            cur_process = NULL;
        }

        else if (process->EAX == READ)
        {
            io_scheduler(COM1, READ, (char *)process->ECX, process->EDX);

            cur_process->top = (unsigned char *)process;
            cur_process->state = BLOCKED_STATE;
            process->EDX = device1->inputsize;
            // insert cur pcb
            pcb_insert(cur_process);
            // cur pcb == null
            cur_process = NULL;
            return process;
        }

        else if (process->EAX == WRITE)
        {
            io_scheduler(COM1, WRITE, (char *)process->ECX, process->EDX);

            cur_process->top = (unsigned char *)process;
            cur_process->state = BLOCKED_STATE;

            // insert cur pcb;
            pcb_insert(cur_process);
            // cur pcb == null
            cur_process = NULL;
        }
        else
        {
            process->EAX = -1;
            return (Context *)process;
        }
    }

    if (next_pcb == NULL)
    {
        while (next_pcb != NULL)
        {
            if (next_pcb->state == READY_STATE)
            {
                break;
            }

            next_pcb = next_pcb->next;
        }

        pcb_remove(next_pcb);
    }

    if (next_pcb == NULL)
    {

        return initial_context;
    }
    else
    {
        cur_process = next_pcb;
        cur_process->state = RUNNING_STATE;
        return (Context *)cur_process->top;
    }

    return NULL;
}