#include <mpx/io.h>
#include <serial.h>
#include <sys_req.h>
#include <string.h>
#include <memory.h>
#include <mpx/user_commands.h>
#include <mpx/interrupts.h>
#include <mpx/io.h>
#include <stdlib.h>
#include <pcb.h>
#include <mpx/io_helpers.h>

extern struct dcb *device1;
extern pcb *cur_process;
extern void serial_isr(void *);

enum uart_registers
{
	RBR = 0, // Receive bufferer
	THR = 0, // Transmitter Holding
	DLL = 0, // Divisor Latch LSB
	IER = 1, // Interrupt Enable
	DLM = 1, // Divisor Latch MSB
	IIR = 2, // Interrupt Identification
	FCR = 2, // FIFO Control
	LCR = 3, // Line Control
	MCR = 4, // Modem Control
	LSR = 5, // Line Status
	MSR = 6, // Modem Status
	SCR = 7, // Scratch
};

static int initialized[4] = {0};

static int serial_devno(device dev)
{
	switch (dev)
	{
	case COM1:
		return 0;
	case COM2:
		return 1;
	case COM3:
		return 2;
	case COM4:
		return 3;
	}
	return -1;
}

int serial_init(device dev)
{
	int dno = serial_devno(dev);
	if (dno == -1)
	{
		return -1;
	}
	outb(dev + IER, 0x00);			// disable interrupts
	outb(dev + LCR, 0x80);			// set line control register
	outb(dev + DLL, 115200 / 9600); // set bsd least sig bit
	outb(dev + DLM, 0x00);			// brd most significant bit
	outb(dev + LCR, 0x03);			// lock divisor; 8bits, no parity, one stop
	outb(dev + FCR, 0xC7);			// enable fifo, clear, 14byte threshold
	outb(dev + MCR, 0x0B);			// enable interrupts, rts/dsr set
	(void)inb(dev);					// read bit to reset port
	initialized[dno] = 1;
	return 0;
}

int serial_out(device dev, const char *bufferer, size_t len)
{
	int dno = serial_devno(dev);
	if (dno == -1 || initialized[dno] == 0)
	{
		return -1;
	}
	for (size_t i = 0; i < len; i++)
	{
		outb(dev, bufferer[i]);
	}
	return (int)len;
}

void io_scheduler(device dev, int opCode, char *buf, int count)
{
	if (dev != COM1)
	{
		return;
	}

	if (device1 == NULL)
	{

		return;
	}

	if (device1->inputsize > 0 || device1->inputbuff != NULL)
	{

		if (opCode == READ || opCode == WRITE)
		{

			if (device1->status != IDLE)
			{
				iocb *new_iocb = create_iocb(device1->status, device1->inputbuff, device1->inputsize, cur_process);
				// Insert into iocb queue
				iocb_insert(new_iocb, device1);
			}
			else
			{

				if (opCode == READ)
				{
					serial_read(dev, buf, count);
					device1->event_flag = 1;
				}
				else if (opCode == WRITE)
				{
					serial_write(dev, buf, count);
					device1->event_flag = 1;
				}
			}
		}
	}
}

int serial_open(device dev, int speed)
{
	// ERROR OUTPUTS (NEED IMPLEMENTED)

	// INITIALIZE SERIAL PORT WITH THE DEV PARAMETER

	// Makes sure we are in COM1 and device is open
	if (dev != COM1 || device1 != NULL)
	{
		return 1;
	}

	cli();

	int baud_rate = 0;

	switch (speed)
	{

	case 110:
		baud_rate = 110;
		break;

	case 150:
		baud_rate = 150;
		break;

	case 300:
		baud_rate = 300;
		break;

	case 600:
		baud_rate = 600;
		break;

	case 1200:
		baud_rate = 1200;
		break;

	case 2400:
		baud_rate = 2400;
		break;

	case 4800:
		baud_rate = 4800;
		break;

	case 9600:
		baud_rate = 9600;
		break;

	case 19200:
		baud_rate = 19200;
		break;

	case 115200:
		baud_rate = 115200;
		break;

	default:
		return -1;
	}

	// Gets divisor for baud_rate
	int baud_rate_div = 115200 / baud_rate;

	device1 = create_dcb();

	// Gets baud_msb and lsb
	uint8_t baud_msb = (baud_rate_div >> 8) & 0xFF;

	uint8_t baud_lsb = baud_rate_div & 0xFF;

	// Turns to settings mode
	outb(dev + 3, 0x80);

	// Outb with each of these
	outb(BASE, baud_lsb);

	outb(BASE, baud_msb);

	// Back to normal mode
	outb(COM1 + 3, 0x03);

	// Mask to enable serial interrupts?
	int mask = inb(PIC_MASK);
	mask = mask & ~0x10;
	outb(PIC_MASK, mask);

	// Enalbe input interrupts
	outb(BASE + 1, 0x01);

	// Enable serial port interrupts
	outb(BASE + 4, 0x08);

	// Enables fifo
	outb(BASE + 2, 0xC7);

	// Renable interrupts

	idt_install(0x24, serial_isr);

	sti();

	return 0;
}

int serial_close(device dev)
{

	if (device1 == NULL || dev != COM1)
	{
		return 1;
	}

	else
	{

		// Disable interrupts

		cli();

		device1->event_flag = 0;

		int mask = PIC_MASK;
		mask = mask | 0x10;
		outb(COM1, PIC_MASK);

		// Disable all serial interrupts?
		outb(BASE + 1, 0x08);

		outb(BASE + 6, 0x08);

		sti();
	}
	return 0;
}
int add_byte_to_ring(dcb *dcb, char byte)
{
	// dcb = NULL;
	// cli();
	if (byte == '\b' || byte == 127)
	{
		if (dcb->ring_input_index != dcb->ring_output_index)
		{
			dcb->ring_input_index--;
		}
		// sti();
		return 1;
	}
	//(dcb->ring_output_index) = byte;

	dcb->ring_buffer[dcb->ring_input_index] = byte;

	dcb->ring_input_index++;

	// If index is outside of size of ring
	dcb->ring_input_index = dcb->ring_input_index % 20;

	if (dcb->ring_start_index == dcb->ring_input_index)
	{
		dcb->ring_start_index++;
		dcb->ring_output_index++;

		dcb->ring_start_index = dcb->ring_start_index % 20;
		dcb->ring_output_index = dcb->ring_output_index % 20;
	}
	// sti();
	return 1;
}

int read_from_ring(dcb *dcb, char *buffer, int count)
{
	// cli();
	int bytes_read = 0;

	while (count > 0)
	{
		if (dcb->ring_buffer[dcb->ring_input_index] == '\r' || dcb->ring_buffer[dcb->ring_input_index] == '\n')
		{
			// done reading, set event_flag
			device1->event_flag = 1;
			// sti();
			return bytes_read - count;
		}

		buffer[bytes_read] = dcb->ring_buffer[dcb->ring_input_index];
		dcb->ring_input_index = (dcb->ring_input_index + 1) % 20;

		bytes_read++;
		count--;

		if (dcb->ring_start_index == dcb->ring_input_index)
		{
			// /sti();
			return bytes_read;
		}
	}
	// sti();
	return bytes_read;
}

int serial_read(device dev, char *buffer, int len)
{

	// INVALID BUFFER ADDRESS
	if (buffer == NULL || dev != COM1)
	{
		return 1;
	}

	// INVALID COUNTER ADDRESS
	else if (len == 0)
	{
		return 1;
	}

	// IF THE DEVICE IS NOT BUSY
	else if (device1->event_flag == 1)
	{
		return 1;
	}

	// IF THE DEVICE IS NOT IDLE
	else if (device1->status != IDLE)
	{
		return 1;
	}
	else if (device1->port == CLOSED)
	{
		return 1;
	}
	else
	{

		// INITIALIZE INPUT BUFFER VARS
		device1->inputbuff = buffer;

		serial_poll(COM1, buffer, sizeof(buffer));

		device1->status = READ;
		device1->inputsize = len;

		// CLEAR FLAG
		device1->event_flag = 0;
		//  store in buff?
		//  outb(COM1, buffer[0]);

		// WHILE LOOP TO COPY FROM RING BUFFER TO INPUT BUFFER
		int bytes_read = read_from_ring(device1, buffer, len);

		if (bytes_read == len)
		{
			// /device1->status = IDLE;
			device1->event_flag = 1;
			device1->inputsize = len;
			return len;
		}
	}

	// RETURN 0 BY DEFAULT
	return 0;
}

void serial_write(device dev, char *buf, int len)
{

	// IF BUFFER ADDRESS IS INVALID
	if (buf == NULL || dev != COM1)
	{
		return;
	}

	// IF COUNT IS WRONG
	else if (len == 0)
	{
		return;
	}

	else
	{

		// IF PORT ISNT OPEN
		if (device1->event_flag != OPEN)
		{
			return;
		}

		// IF DEVICE IS BUSY
		else if (device1->status != IDLE)
		{
			return;
		}

		else
		{
			// CLEAR EVENT FLAG
			device1->event_flag = 0;

			// INSTALL THE OUTPUT BUFFER AND COUNTER
			device1->outputbuff = buf;
			device1->outputsize = len;
			device1->buff_index = 1;

			// SET STATUS TO WRITING
			device1->status = WRITE;

			// FOR REGISTERING THE CHARACTER

			// STORE CHARACTER TO THE OUTPUT REGISTER
			outb(COM1, buf[0]);

			// ENABLE WRITE INTERRUPT
			int mask = inb(BASE + 1);
			mask = mask | 0x02;
			outb(COM1 + 1, mask);
		}
	}
}

void serial_interrupt(void)
{

	if (device1 != NULL)
	{
		int type = inb(COM1 + 2); // most likely wrong

		// Checks if bit is zero to make sure port is the source of the interrupt
		if ((type & 0x01) != 0)
		{
			outb(0x20, 0x20);
			return;
		}

		// skip bit0, grab bit 1 and 2
		int bit1 = (type >> 1) & 1;
		int bit2 = (type >> 2) & 1;

		if (!bit1 && !bit2)
		{
			inb(COM1 + 6); // modem status
		}
		else if (bit1 && !bit2)
		{
			serial_output_interrupt();
		}
		else if (!bit1 && bit2)
		{
			// device1->status = READ;
			serial_input_interrupt();
		}
		else if (bit1 && bit2)
		{
			inb(COM1 + 5); // line status
		}
	}

	outb(0x20, 0x20); // Issue EOI to the PIC
					  // OBTAIN DCB

	// READ REGISTRY TO DETERMINE INTERRUPT

	// IF ELSE for interrupt handling

	// Issue EOI to the PIC
}

void set_int(int on)
{
	if (on)
	{
		outb(BASE + 1, inb(BASE + 1) | (0x02)); // on
	}
	else
	{
		outb(BASE + 1, inb(BASE + 1) | ~(0x02)); // off
	}
}

void serial_input_interrupt()
{
	// char *currPtr;
	//  inb from base register to get input
	char input = inb(BASE);

	if (device1->status != READ) // if not read ignore the interrupt and return
	{
		if (device1->inputsize >= 20)
		{
			return;
		}

		add_byte_to_ring(device1, input);
		// device1->ring_buffer[device1->buff_index++] = input; // return first-level handler

		// maybe add arrow key checks here
	}
	else
	{
		// device1->buffer[com1_index++] = input;
		// outb(com1_base, input);
		// polling goes here

		if (device1->buff_index > 20)
		{
			device1->status = IO_IDLE;
			device1->event_flag = 1;
			outb(COM1, '\n');
		}
	}
}

//   dbc->ring_buffer[dcb->input_buffer++] = input;//store request in input  buffer
//   if(dcb->input_buffer < 16 && input != '/r'){//if not complete (not full) and not new line
//       return 1;
//   } else{//otherwise transfer complete set it to idle,set event flag, and return requested count
//       dbc->currentOP = IO_IDLE;
//       dbc->event_flag = SET;
//       return dbc->input_buffer;
//   }

// IF THE DCB STATE OS READING, STORE THE CHARACTER IN THE
// APPROPRIATE IOCB BUFFER
// IF BUFFER IS NOW FULL OR INPUT WAS NEW LINE, SIGNAL COMPLETION
// ELSE
// ATTEMPT TO STORE CHARACTER IN THE RING BUFFER

void serial_output_interrupt()
{

	// if current status is not writing, return
	if (device1->status != WRITE)
	{
		return;
		// i the count has not been exhausted, get the next character from the requestor’s output
		// buffer and store it in the output register
	}
	outb(BASE, device1->outputbuff[device1->buff_index]);
	device1->buff_index++;

	if (device1->buff_index == device1->outputsize || device1->outputbuff[device1->buff_index] == '\0')
	{
		device1->event_flag = 1;

		int IER = inb(BASE + 1);
		outb(BASE + 1, IER & ~0x2);
	}
}

int serial_poll(device dev, char *buffer, size_t len)
{
	// command to show ascii
	// sudo showkey -s
	// size pointer
	int index = 0;
	// not sure yet
	int col = 0;
	// int count = 0;
	// sizeof will always return 4 which is the size of an address
	// /int buf_len = strlen( buffer);
	while ((size_t)index < len)
	{
		// if LSB of LSR is 1 (if something is available)
		// +5 is used to go to right most bit & checks to see if 1
		if (inb(dev + 5) & 1)
		{
			char letter = inb(COM1);
			if (letter == '\r' || letter == '\n')
			{
				buffer[index] = '\0';
				return index;
			}

			else if (letter == '\b' || letter == 127)
			{
				if (index != 0)
				{
					buffer[index] = '\0';
					index--;
					outb(COM1, '\b');
					outb(COM1, ' ');
					outb(COM1, '\b');
				}
			}
			else if (letter == '\033')
			{
				letter = inb(COM1);
				if (letter == '[')
				{
					letter = inb(COM1);
					if (letter == 'D')
					{ // left key
						if (col > 0)
						{
							outb(COM1, '\033'); // arrow key are three bytes which is why its written like this
							outb(COM1, '[');
							outb(COM1, 'D');
							col--;
						}
					}
					else if (letter == 'C')
					{ // right key
						if (col < index)
						{
							outb(COM1, '\033');
							outb(COM1, '[');
							outb(COM1, 'C');
							col++;
						}
					}
				}
			}
			else if (letter >= ' ' && letter <= 'Z')
			{ // 0-9 and A to Z input
				buffer[index] = letter;
				outb(COM1, buffer[index]);
				col++;
				index++;
			}
			else if (letter >= 'a' && letter <= 'z')
			{
				buffer[index] = letter;
				outb(COM1, buffer[index]);
				col++;
				index++;
			}
			if ((size_t)index == len)
			{
				letter = '\r';
				buffer = NULL;
				// sys_free_mem(buffer);
				return 0;
			}
		}
	}
	return len;
}