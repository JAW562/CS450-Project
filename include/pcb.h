#ifndef PCB_H
#define PCB_H

#define READY_STATE 0
#define READY_SUSPENDED_STATE 1
#define BLOCKED_STATE 2
#define BLOCKED_SUSPENDED_STATE 3
#define RUNNING_STATE 4

#define SYSTEM_PROCESS 0

#include <stdint.h>

#define USER_APPLICATION 1

#define MIN_NAME_LEN 1
#define MAX_NAME_LEN 256

#define MIN_PRIORITY 0
#define MAX_PRIORITY 9



typedef struct context{
    uint32_t GS, FS, ES, DS, SS;
    uint32_t EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX;
    uint32_t EIP, CS, EFLAGS;

}Context;


#define STACK_SIZE 1024
typedef struct pcb
{
    char *name;
    int pcb_class;
    char priority;
    int state;
    //unsigned char* stack;
    struct pcb *next;
    unsigned char* base;
    unsigned char* bottom;
    unsigned char* top;

} pcb;

struct queue
{
    int count;
    struct pcb *front;
    struct pcb *rear;
    
};


struct pcb *pcb_allocate(void);

struct pcb *pcb_find(const char *);

int pcb_free(struct pcb *);

void pcb_insert(struct pcb *p);

int pcb_remove(struct pcb *p);

struct pcb *pcb_setup( char *name, int class, int priority);


#endif