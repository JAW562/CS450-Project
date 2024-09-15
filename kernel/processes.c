#include <memory.h>
#include <sys_req.h>
#include <string.h>
#include <stdlib.h>
#include <processes.h>

char *msg1 = "proc1 dispatched\n";
char *msg2 = "proc2 dispatched\n";
char *msg3 = "proc3 dispatched\n";
char *msg4 = "proc4 dispatched\n";
char *msg5 = "proc5 dispatched\n";
int msgSize = 17;

char *er1 = "proc1 ran after it was terminated";
char *er2 = "proc2 ran after it was terminated";
char *er3 = "proc3 ran after it was terminated";
char *er4 = "proc4 ran after it was terminated";
char *er5 = "proc5 ran after it was terminated";
int erSize = 34;

// A test process that prints a message then yields, exiting after 1 iteration.
void proc1()
{
    int i;

    while (1)
    {
        for (i = 0; i < ITERATION1; i++)
        {
            sys_req(WRITE, COM1, msg1, sizeof(msg1));
            sys_req(IDLE, COM1, NULL, NULL);
        }
        sys_req(EXIT, COM1, NULL, NULL);
        sys_req(WRITE, COM1, er1, sizeof(er1));
    }
}

void proc2()
{
    int i;

    while (1)
    {
        for (i = 0; i < ITERATION2; i++)
        {
            sys_req(WRITE, COM1, msg1, sizeof(msg1));
            sys_req(IDLE, COM1, NULL, NULL);
        }
        sys_req(EXIT, COM1, NULL, NULL);
        sys_req(WRITE, COM1, er1, sizeof(er1));
    }
}

void proc3()
{
    int i;

    while (1)
    {
        for (i = 0; i < ITERATION3; i++)
        {
            sys_req(WRITE, COM1, msg1, sizeof(msg1));
            sys_req(IDLE, COM1, NULL, NULL);
        }
        sys_req(EXIT, COM1, NULL, NULL);
        sys_req(WRITE, COM1, er1, sizeof(er1));
    }
}

void proc4()
{
    int i;

    while (1)
    {
        for (i = 0; i < ITERATION4; i++)
        {
            sys_req(WRITE, COM1, msg1, sizeof(msg1));
            sys_req(IDLE, COM1, NULL, NULL);
        }
        sys_req(EXIT, COM1, NULL, NULL);
        sys_req(WRITE, COM1, er1, sizeof(er1));
    }
}

void proc5()
{
    int i;

    while (1)
    {
        for (i = 0; i < ITERATION5; i++)
        {
            sys_req(WRITE, COM1, msg1, sizeof(msg1));
            sys_req(IDLE, COM1, NULL, NULL);
        }
        sys_req(EXIT, COM1, NULL, NULL);
        sys_req(WRITE, COM1, er1, sizeof(er1));
    }
}
