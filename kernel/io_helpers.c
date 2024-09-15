#include <serial.h>
#include <pcb.h>
#include <memory.h>
#include <sys_req.h>
#include <mpx/io_helpers.h>
#include <mpx/user_commands.h>
int max_length_buffer = 50;

iocb *create_iocb(int op_code, char *buffer, int buffer_size, pcb *process)
{
    iocb *new_iocb = (iocb *)sys_alloc_mem(sizeof(iocb));

    if (new_iocb == NULL)
    {
        return NULL;
    }
    else
    {
        new_iocb->typeOP = op_code;
        new_iocb->buffer = buffer;
        new_iocb->buffer_size = buffer_size;
        new_iocb->process = process;
        new_iocb->next = NULL;

        return new_iocb;
    }
    return NULL;
}

dcb *create_dcb()
{
    dcb *new_dcb = (dcb *)sys_alloc_mem(sizeof(dcb));

    if (new_dcb == NULL)
    {
        return NULL;
    }
    else
    {
        new_dcb->port = COM1;
        new_dcb->event_flag = 0;
        new_dcb->status = IDLE;
        new_dcb->inputbuff = NULL;
        new_dcb->inputsize = max_length_buffer;
        new_dcb->outputbuff = NULL;
        new_dcb->outputsize = 0;
        new_dcb->buff_index = 0;
        new_dcb->ring_input_index = 0;
        new_dcb->ring_output_index = 19;
        new_dcb->ring_start_index = 0;
        new_dcb->iocb_queue = NULL;
    }
    return new_dcb;
}

int dcb_free(struct dcb *dcb)
{
    int i = sys_free_mem(dcb);

    // Returns 0 on success
    if (i == 0)
    {
        return i;
    }

    // Returns an error message and the error number on error
    else
    {
        char *error = "Error: Could not free memory from dcb.\n";
        sys_req(WRITE, COM1, error, sizeof(error));
        return i;
    }

    return i;
}

int iocb_free(struct iocb *iocb)
{

    int i = sys_free_mem(iocb);

    // Returns 0 on success
    if (i == 0)
    {
        return i;
    }

    // Returns an error message and the error number on error
    else
    {
        char *error = "Error: Could not free memory from dcb.\n";
        sys_req(WRITE, COM1, error, sizeof(error));
        return i;
    }

    return i;
}

iocb *next_iocb(struct dcb *dcb)
{

    if (dcb->iocb_queue == NULL)
    {
        return NULL;
    }

    iocb *new_iocb = create_iocb(dcb->status, dcb->outputbuff, dcb->outputsize, dcb->iocb_queue->process);

    return new_iocb;
}

void iocb_insert(struct iocb *iocb, struct dcb *dcb)
{

    if (dcb->iocb_queue != NULL)
    {
        struct iocb *temp = dcb->iocb_queue;
        while (temp->next != NULL)
        {
            temp = temp->next;
        }

        // Create new node for pcb insert

        temp->next = iocb;
    }
    else
    {

        dcb->iocb_queue = iocb;
    }
}

void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;

    // Storing start time
    int start_time = (int)get_time();

    // looping till required time is not achieved
    while (get_time() < start_time + milli_seconds)
        ;
}
