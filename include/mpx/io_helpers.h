#include <serial.h>
#include <pcb.h>

struct iocb *create_iocb(int op_code, char *buffer, int buffer_size, pcb *process);

struct dcb *create_dcb();

int dcb_free(struct dcb *dcb);

int iocb_free(struct iocb *ioxb);

struct iocb *next_iocb(struct dcb *dcb);

void iocb_insert(struct iocb *iocb, struct dcb *dcb);

void delay(int number_of_seconds);
