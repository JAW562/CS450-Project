#ifndef MPX_SERIAL_H
#define MPX_SERIAL_H

#include <stddef.h>
#include <mpx/device.h>

#include <pcb.h>

#define PIC_MASK 0x21
#define ON 1
#define OFF 0

// COM1 Addr
#define BASE 0x03F8

// Event flag codes
#define CLEAR 0
#define SET 1

// Port status codes
#define OPEN 0
#define CLOSED 1

// Status codes
#define IO_IDLE 0
#define IO_READ 1
#define IO_WRITE 2

// Open error codes
#define INVAL_FLAG_PTR -101 //-101 INVALID/NULL EVENT FLAG POINTER
#define INVAL_BAUD_DIV -102 //-102 INVALID BAUD RATE DIVISOR
#define PORT_OPEN -103		//-103 PORT ALREADY OPEN

// Close error codes
#define SERIAL_PORT_NOT_OPEN -201

// COM_READ ERROR CODES
#define READ_PORT_NOT_OPEN -301
#define READ_INVALID_BUFFER_ADDRESS -302
#define READ_INVALID_COUNT -303
#define READ_DEVICE_BUSY -304

// COM_WRITE ERROR CODES
#define WRITE_PORT_NOT_OPEN -401
#define WRITE_INVALID_BUFFER_ADDRESS -402
#define WRITE_INVALID_COUNT -403
#define WRITE_DEVICE_BUSY -404

/**
 @file mpx/serial.h
 @brief Kernel functions and constants for handling serial I/O
*/

/**
 Initializes devices for user input and output
 @param device A serial port to initialize (COM1, COM2, COM3, or COM4)
 @return 0 on success, non-zero on failure
*/
int serial_init(device dev);

/**
 Writes a buffer to a serial port
 @param device The serial port to output to
 @param buffer A pointer to an array of characters to output
 @param len The number of bytes to write
 @return The number of bytes written
*/
int serial_out(device dev, const char *buffer, size_t len);

/**
 Reads a string from a serial port
 @param device The serial port to read data from
 @param buffer A buffer to write data into as it is read from the serial port
 @param count The maximum number of bytes to read
 @return The number of bytes read on success, a negative number on failure
*/

// int serial_poll(device dev, char *buffer, size_t len);

typedef struct iocb
{
	struct pcb *process;
	int typeOP;
	char *buffer;
	int buffer_size;
	struct iocb *next;
} iocb;

typedef struct dcb
{

	// struct pcb *process;
	// INDICATES IF THE PORT IS OPEN OR CLOSED (0 FOR OPEN, 1 FOR CLOSED)
	int port;

	// EVENT FLAG, 0 AT THE BEGINNING OF AN OPERATION, 1 WHEN OP IS COMPLETE
	int event_flag;

	// STATUS CODE VALUES ARE IDLE READING AND WRITING
	int status;

	// INPUT BUFFER ADDRESS
	char *inputbuff;

	// COUNTER OF INPUT BUFFER This is count
	uint32_t inputsize;

	// Last character read in
	uint32_t buff_index;

	// OUTPUT BUFFER ADDRESS
	char *outputbuff;

	// COUNTER OF OUTPUT BUFFER
	uint32_t outputsize;

	// CHAR ARRAY TO SET UP THE RING BUFFER
	char ring_buffer[20];

	// INDEX OF INPUT FOR RING
	int ring_input_index;

	// INDEX OF OUTPUT FOR RING
	int ring_output_index;

	// INDEX OF START FOR RING
	int ring_start_index;

	struct iocb *iocb_queue;

	int ring_size;
	int *input_count;
	int *output_count;
	int request_input_transfer;
	int request_output_transfer;

} dcb;

// Should a separate queue of IOCBs for each device
// Records informaiton about a specific transfer or event

int add_byte_to_ring(dcb *dcb, char byte);

int read_from_ring(dcb *dcb, char *buffer, int count);

int serial_open(device dev, int speed);

int serial_close(device dev);

int serial_read(device dev, char *buffer, int len);

void serial_write(device dev, char *buffer, int len);

void serial_interrupt(void);

void serial_input_interrupt();

void serial_output_interrupt();

void call_to_assm();

void io_scheduler(device dev, int opCode, char *buf, int count);

int serial_poll(device dev, char *buffer, size_t len);

void set_int(int on);
#endif
