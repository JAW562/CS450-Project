#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <mpx/device.h>
#include <sys_req.h>
#include <mpx/io.h>
#include <mpx/interrupts.h>
#include "mpx/command_handler.h"
#include <mpx/user_commands.h>
#include <pcb.h>
#include <processes.h>

extern struct queue readyPtr;
extern struct queue blockedPtr;
extern pcb *cur_process;

/*IMPORTANT!! There is no need to call Struct *pcb blah blah everytime you make use of a pcb pointer. It already knows that it's a struct when you call PCB and calling
it twice cannot be good. I think I got all mentions of it here but not 100% sure. -Jared
*/

alarm *alarms_glob[3] = {NULL};
// alarms_glob[0] = null;
// alarms_glob[1] = null;
// alarms_glob[2] = null;

// Reverse helper function for itoa
void reverse(char str[], int length)
{
    // front and back  of string declaration
    int f, b;

    // temp char
    char t;

    // reverses string by swapping
    for (f = 0, b = length - 1; f < b; f++, b--)
    {
        t = *(str + f);
        *(str + f) = *(str + b);
        *(str + b) = t;
    }
}

char *itoa(int n, char str[], int bottom)
{

    // var declaration for length
    int i = 0;

    // if number is 0, makes char 0
    if (n == 0)
    {
        str[i] = '0';
        str[i++] = '\0';
        return str;
    }

    // convert to specified bottom, then converts the numbers to ascii
    while (n != 0)
    {
        int r = n % bottom;
        str[i++] = (r > 9) ? (r - 10) + 'a' : r + '0';
        n = n / bottom;
    }

    // end with null
    str[i] = '\0';

    // reverse string
    reverse(str, i);

    // return string
    return str;
}

// Changes characters to lowercase and leaves digits in place
// It excludes the first letter so maybe need to modify
char *upper_to_lower(char *str)
{
    // convert capital letter string to small letter string
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] >= 'A' && str[i] <= 'Z')
        {
            str[i] = str[i] + ('a' - 'A');
        }
    }

    return str;
}

// Gets time from RTC
char get_time()
{

    char *clear = "\e[1;1H\e[2J";
    sys_req(WRITE, COM1, clear, strlen(clear));

    // Access Index Registries and Assign Seconds, Minutes, Hours to index pulls
    outb(0x70, 0x00);
    unsigned char seconds = inb(0x71);

    outb(0x70, 0x02);
    unsigned char minutes = inb(0x71);

    outb(0x70, 0x04);
    unsigned char hours = inb(0x71);

    // Bit shifting
    int sones = (int)(seconds & 0b00001111);
    int stens = (int)((seconds >> 4) & 0b00001111);
    seconds = sones + (stens * 10);

    int mones = (int)(minutes & 0b00001111);
    int mtens = (int)((minutes >> 4) & 0b00001111);
    minutes = mones + (mtens * 10);

    int hones = (int)(hours & 0b00001111);
    int htens = (int)((hours >> 4) & 0b00001111);
    hours = hones + (htens * 10);

    // Char arrays
    char h[3];
    char m[3];
    char s[3];
    char time[9];

    // itoa stuff
    itoa(hours, h, 10);
    itoa(minutes, m, 10);
    itoa(seconds, s, 10);

    // Forming the time
    sys_req(WRITE, COM1, "\n", 1);
    time[0] = h[0];
    time[1] = h[1];
    time[2] = ':';
    time[3] = m[0];
    time[4] = m[1];
    time[5] = '.';
    time[6] = s[0];
    time[7] = s[1];
    time[8] = '\0';

    // Write to comm handler
    sys_req(WRITE, COM1, time, sizeof(time));
    sys_req(WRITE, COM1, "\n", 1);

    return *time;
}

// Sets time using RTC
void set_time()
{

    char *clear = "\e[1;1H\e[2J";
    sys_req(WRITE, COM1, clear, strlen(clear));

    // Set a message size
    char m1[] = "\nPlease enter the hour: \n";
    char m2[] = "\nPlease enter the minutes: \n";
    char m3[] = "\nPlease enter the seconds: \n";

    // Initialize input values
    char *hourinput = "\0";
    char *mininput = "\0";
    char *secinput = "\0";

    // Hours
    sys_req(WRITE, COM1, "\n", 1);
    sys_req(WRITE, COM1, "\nPlease enter the hour: \n", strlen(m1));
    sys_req(READ, COM1, hourinput, 2);
    sys_req(WRITE, COM1, "\n", 1);

    // atoi stuff
    int hour = atoi(hourinput);

    // Error handling
    if (hour > 24 || hour <= 0)
    {
        char error[] = "\nError: The number of hours in a day ranges between 1 - 24.\n";
        sys_req(WRITE, COM1, error, strlen(error));
        return;
    }

    // Conversion to BCD
    int h = (((hour / 10) << 4) | (hour % 10));

    // Disable interrupts
    cli();

    // Open the "hours" registry and write to it
    outb(0x70, 0x04);

    outb(0x71, h);

    // Enable interrupts
    sti();

    // Minutes
    sys_req(WRITE, COM1, "\n", 1);
    sys_req(WRITE, COM1, "\nPlease enter the minutes: \n", strlen(m2));
    sys_req(READ, COM1, mininput, 2);
    sys_req(WRITE, COM1, "\n", 1);

    // atoi stuff
    int min = atoi(mininput);

    // Error Handling
    if (min > 59 || min < 0)
    {
        char error[] = "\nError: The number of minutes in an hour is between 00 - 59. \n";
        sys_req(WRITE, COM1, error, strlen(error));
    }

    // BCD conversion
    int m = (((min / 10) << 4) | (min % 10));

    // Disable interrupts
    cli();

    // Access "Minutes" registry and write to it
    outb(0x70, 0x02);

    outb(0x71, m);

    // Enable interrupts
    sti();

    sys_req(WRITE, COM1, "\n", 1);
    sys_req(WRITE, COM1, "\nPlease enter the seconds: \n", strlen(m3));
    sys_req(READ, COM1, secinput, 2);
    sys_req(WRITE, COM1, "\n", 1);

    // Atoi stuff
    int sec = atoi(secinput);

    // Error Handling
    if (sec > 59 || sec < 0)
    {
        char error[] = "\nError: Please input the number of seconds between 00 - 59.\n";
        sys_req(WRITE, COM1, error, strlen(error));
    }

    // BCD conversion
    int s = (((sec / 10) << 4) | (sec % 10));
    ;

    // Disable Interrupts
    cli();

    // Open "seconds" index registry and write to it
    outb(0x70, 0x00);

    outb(0x71, s);

    // Enable interrupts
    sti();

    // Confirmation Message
    char confirmation[] = "\nThe time has successfully been changed.\n";
    sys_req(WRITE, COM1, confirmation, strlen(confirmation));
}

// Gets date from RTC
void get_date()
{

    // Access Index Registries for Day Month Year & assign unsigned chars to the index pulls
    outb(0x70, 0x07);
    unsigned char day = inb(0x71);

    outb(0x70, 0x08);
    unsigned char month = inb(0x71);

    outb(0x70, 0x09);
    unsigned char year = inb(0x71);

    // Bit Shifting
    int dones = (int)(day & 0b00001111);
    int dtens = (int)((day >> 4) & 0b00001111);
    day = dones + (dtens * 10);

    int mones = (int)(month & 0b00001111);
    int mtens = (int)((month >> 4) & 0b00001111);
    month = mones + (mtens * 10);

    int yones = (int)(year & 0b00001111);
    int ytens = (int)((year >> 4) & 0b00001111);
    year = yones + (ytens * 10);

    // char arrays
    char d[3];
    char m[3];
    char y[3];
    char date[9];

    // itoa stuff
    itoa(day, d, 10);
    itoa(month, m, 10);
    itoa(year, y, 10);

    // date formatting
    sys_req(WRITE, COM1, "\n", 1);
    date[0] = d[0];
    date[1] = d[1];
    date[2] = '-';
    date[3] = m[0];
    date[4] = m[1];
    date[5] = '-';
    date[6] = y[0];
    date[7] = y[1];
    date[8] = '\0';

    // Use sys_req to write the date to the command interface
    sys_req(WRITE, COM1, date, sizeof(date));
    sys_req(WRITE, COM1, "\n", 1);
}

// Sets date to RTC
void set_date()
{

    char *clear = "\e[1;1H\e[2J";
    sys_req(WRITE, COM1, clear, strlen(clear));

    // New line
    sys_req(WRITE, COM1, "\n", 1);

    // Set message size
    char m1[] = "\nPlease input the day: \n";
    char m2[] = "\nPlease input the month: \n";
    char m3[] = "\nPlease input the year(Only last 2 digits): \n";

    // Char pointers
    char *dayinput = "\0";
    char *monthinput = "\0";
    char *yearinput = "\0";

    // Day Part
    sys_req(WRITE, COM1, "\nPlease input the day: \n", strlen(m1));
    sys_req(READ, COM1, dayinput, 2);
    sys_req(WRITE, COM1, "\n", 1);

    // atoi conversion
    int day = atoi(dayinput);

    // error handling
    if (day <= 0 || day >= 31)
    {
        char error[] = "\nError: Please input a day between 1 - 31. \n";
        sys_req(WRITE, COM1, error, strlen(error));
    }

    // Conversion to BCD
    int d = (((day / 10) << 4) | (day % 10));

    // Disable interrupts
    cli();

    // Open the "day" registry
    outb(0x70, 0x07);

    // Write to registry
    outb(0x71, d);

    // Enable interrupts
    sti();

    // Confirmation
    char con1[] = "\nDay successfully entered. \n";
    sys_req(WRITE, COM1, con1, strlen(con1));

    // Month Part
    sys_req(WRITE, COM1, "\n", 1);
    sys_req(WRITE, COM1, "\nPlease input the month: \n", strlen(m2));
    sys_req(READ, COM1, monthinput, 2);
    sys_req(WRITE, COM1, "\n", 1);

    // atoi conversion
    int month = atoi(monthinput);

    // error handling
    if (month <= 0 || month > 12)
    {
        char error[] = "\nError: Please input a month between 1 - 12.\n";
        sys_req(WRITE, COM1, error, strlen(error));
    }

    else if (month == 1 || month == 3 || month == 5 || month == 7 || month == 10 || month == 12)
    {
        if (day > 31 || day <= 0)
        {
            char error[] = "\nError: For the months of January, March, May, July, October, and Decemeber, the number of days ranges from 1 - 31.\n";
            sys_req(WRITE, COM1, error, strlen(error));
        }
    }

    else if (month == 4 || month == 6 || month == 9 || month == 11)
    {
        if (day > 30 || day <= 0)
        {
            char error[] = "\nError: For the months of April, June, September, and November, the number of days ranges from 1 - 30.\n";
            sys_req(WRITE, COM1, error, strlen(error));
        }
    }

    // BCD conversion
    int m = (((month / 10) << 4) | (month % 10));

    // Disable interrupts
    cli();

    // Access "Month" Index Registry and write to it
    outb(0x70, 0x08);

    outb(0x71, m);

    // Enable interrupts
    sti();

    // Confirmation message
    char con2[] = "\nMonth successfully entered. \n";
    sys_req(WRITE, COM1, con2, strlen(con2));

    // Year stuff
    sys_req(WRITE, COM1, "\n", 1);
    sys_req(WRITE, COM1, "\nPlease input the year (Only last 2 digits): \n", strlen(m3));
    sys_req(READ, COM1, yearinput, 2);
    sys_req(WRITE, COM1, "\n", 1);

    // atoi stuff
    int year = atoi(yearinput);

    // Error handling
    if (month == 2 && year % 4 == 0)
    {
        if (day > 29 || day < 0)
        {
            char error[] = "\nError: The number of days in February during a leap year range from 1 - 29.\n";
            sys_req(WRITE, COM1, error, strlen(error));
        }
    }

    else if (month == 2 && year % 4 != 0)
    {
        if (day > 28 || day < 0)
        {
            char error[] = "\nError: The number of days in February during a non-leap year range from 1 -28.\n";
            sys_req(WRITE, COM1, error, strlen(error));
        }
    }

    // Convert to Binary
    int y = (((year / 10) << 4) | (year % 10));

    // Disabled Interrupts
    cli();

    // Open the "Year" index registry and write to it
    outb(0x70, 0x09);

    outb(0x71, y);

    // Re-enable interrupts
    sti();

    // Confirmation Message
    char con3[] = "\nThe year has successfully been changed. \n";
    sys_req(WRITE, COM1, con3, strlen(con3));

    char final[] = "\nThe date has been successfully changed! \n";
    sys_req(WRITE, COM1, final, strlen(final));
}

void delete_pcb(char *pcb_name)
{
    pcb_name = upper_to_lower(pcb_name);

    pcb *newpcb = pcb_find(pcb_name);

    if (newpcb == NULL)
    {
        char *error = "\nError finding process, please make sure the name is correct.\n";

        sys_req(WRITE, COM1, error, strlen(error));
    }

    if (newpcb->pcb_class != 0)
    {

        pcb_remove(newpcb);

        // pcb_free(newpcb);
    }
    else
    {

        char *error = "\nYou cannot delete a system process, please input a user process.\n";

        sys_req(WRITE, COM1, error, strlen(error));
    }
}

void block_pcb(char *pcb_name)
{
    pcb_name = upper_to_lower(pcb_name);
    pcb *pcbptr = pcb_find(pcb_name);

    if (pcbptr == NULL)
    {

        char *error = "\nCould not find process, please make sure the name is correct.\n";

        sys_req(WRITE, COM1, error, strlen(error));
    }

    if (pcbptr->state == 0)
    {
        pcbptr->state = 2;
        pcb_insert(pcbptr);
    }

    else if (pcbptr->state == 1)
    {
        pcbptr->state = 3;
        pcb_insert(pcbptr);
    }
}

void unblock_pcb(char *pcb_name)
{
    pcb_name = upper_to_lower(pcb_name);
    pcb *pcbptr = pcb_find(pcb_name);

    if (pcbptr == NULL)
    {

        char *error = "\nCould not find process, please make sure the name is correct.\n";

        sys_req(WRITE, COM1, error, strlen(error));
    }

    // move process pointer to unblocked state (0)
    if (pcbptr->state == 2)
    {
        pcbptr->state = 0;
        pcb_insert(pcbptr);
    }
    else if (pcbptr->state == 3)
    {
        pcbptr->state = 1;
        pcb_insert(pcbptr);
    }
}

void suspend_pcb(char *pcb_name)
{
    pcb_name = upper_to_lower(pcb_name);
    pcb *pcbptr = pcb_find(pcb_name);

    if (pcbptr == NULL)
    {

        char *error = "\nCould not find process, please make sure the name is correct.\n";

        sys_req(WRITE, COM1, error, strlen(error));
    }

    // move process pointer to suspended state (3)

    if (pcbptr->pcb_class != 0)
    {
        if (pcbptr->state == 0)
        {
            pcbptr->state = 1;
        }

        else if (pcbptr->state == 2)
        {
            pcbptr->state = 3;
        }
    }
    else
    {

        char *error = "\nCannot delete system process\n";

        sys_req(WRITE, COM1, error, strlen(error));
    }
}

void resume_pcb(char *pcb_name)
{
    pcb_name = upper_to_lower(pcb_name);
    pcb *pcbptr = pcb_find(pcb_name);

    if (pcbptr == NULL)
    {

        char *error = "\nCould not find process, please make sure the name is correct.\n";

        sys_req(WRITE, COM1, error, strlen(error));
    }

    // move process pointer to unsuspended state
    if (pcbptr->state == 1)
    {
        pcbptr->state = 0;
    }

    else if (pcbptr->state == 3)
    {
        pcbptr->state = 2;
    }
}

void set_pcb_prio(char *pcb_name, char prio)
{

    pcb_name = upper_to_lower(pcb_name);

    if (prio < 0)
    {

        char *error = "\nInvalid parameter, please make sure the priority is between 0 and 9.\n";

        sys_req(WRITE, COM1, error, strlen(error));
    }

    pcb *pcbptr = pcb_find(pcb_name);

    if (pcbptr == NULL)
    {

        char *error = "\nError finding process, please make sure the correct name is entered.\n";

        sys_req(WRITE, COM1, error, strlen(error));
    }
    else if (pcbptr != NULL && pcbptr->priority >= 0)
    {
        char *success = "\nPriority successfully changed!\n";
        sys_req(WRITE, COM1, success, strlen(success));
    }

    pcbptr->priority = prio;
    pcb_insert(pcbptr);
}

void show_pcb(char *pcb_name)
{

    pcb *pcbptr = pcb_find(pcb_name);

    if (pcbptr == NULL)
    {
        char *error = "\nError finding pcb, please make sure the name is correct.\n";

        sys_req(WRITE, COM1, error, strlen(error));
    }
    else
    {
        show_pcb_struct(pcbptr);
    }
}

void show_pcb_struct(struct pcb *pcbptr)
{
    // char *name = pcb->name;

    // char *class = pcb->class;

    // char priority = pcb->priority;

    // char ascii_val = '0' + priority;

    sys_req(WRITE, COM1, pcbptr->name, strlen(pcbptr->name));
    sys_req(WRITE, COM1, ", ", 2);

    if (pcbptr->pcb_class == 0)
    {
        sys_req(WRITE, COM1, "SYSTEM PROCESS, ", strlen("SYSTEM PROCESS, "));
    }
    else if (pcbptr->pcb_class == 1)
    {
        sys_req(WRITE, COM1, "USER PROCESS, ", strlen("USER PROCESS, "));
    }

    if (pcbptr->state == 0)
    {
        sys_req(WRITE, COM1, "STATE: UNSUSPENDED READY, ", strlen("STATE: UNSUSPENDED READY, "));
    }

    else if (pcbptr->state == 1)
    {
        sys_req(WRITE, COM1, "STATE: SUSPENDED READY, ", strlen("STATE: SUSPENDED READY, "));
    }

    else if (pcbptr->state == 2)
    {
        sys_req(WRITE, COM1, "STATE: UNSUSPENDED BLOCKED, ", strlen("STATE: UNSUSPENDED BLOCKED, "));
    }

    else if (pcbptr->state == 3)
    {
        sys_req(WRITE, COM1, "STATE: SUSPENDED BLOCKED, ", strlen("STATE: SUSPENDED BLOCKED, "));
    }

    else if (pcbptr->state == 4)
    {
        sys_req(WRITE, COM1, "STATE: UNSUSPENDED RUNNING, ", strlen("STATE: UNSUSPENDED RUNNING, "));
    }

    char ascii_val = '0' + pcbptr->priority;
    sys_req(WRITE, COM1, "PRIORITY: ", strlen("PRIORITY: "));
    sys_req(WRITE, COM1, &ascii_val, 1);
    sys_req(WRITE, COM1, "\n", 1);
}

void show_ready()
{

    // Both of these were showing the exact same thing so it looked like duplicate processe even though they were not

    pcb *pcbPtr = readyPtr.front;
    sys_req(WRITE, COM1, "\nREADY QUEUE:\n", strlen("\nREADY QUEUE:\n"));

    if (pcbPtr == NULL)
    {
        sys_req(WRITE, COM1, "No PCBs\n", strlen("No PCBs\n"));
    }
    else
    {
        while (pcbPtr != NULL && (pcbPtr->state == READY_STATE || pcbPtr->state == READY_SUSPENDED_STATE))
        {
            show_pcb_struct(pcbPtr);
            pcbPtr = pcbPtr->next;
        }
    }
}

void show_blocked()
{
    // Both of these were showing the exact same thing so it looked like duplicate processe even though they were not
    pcb *pcbPtr = blockedPtr.front;
    sys_req(WRITE, COM1, "\nBLOCKED QUEUE:\n", strlen("\nBLOCKED QUEUE:\n"));

    if (pcbPtr == NULL)
    {
        sys_req(WRITE, COM1, "No PCBs\n", strlen("No PCBs\n"));
    }
    else
    {
        while (pcbPtr != NULL && (pcbPtr->state == BLOCKED_STATE || pcbPtr->state == BLOCKED_SUSPENDED_STATE))
        {
            show_pcb_struct(pcbPtr);
            pcbPtr = pcbPtr->next;
        }
    }
}

void show_all()
{
    show_ready();

    show_blocked();
}

/*
    Each process is loaded (one per function -> parameter)
    and is queued in a non-suspended ready state, with a name and prio
    of your choosing
*/
void loadR3PCB(char *name, int(function))
{
    // I added in this line to the function so that it would be easier to search for the process
    // Will also add to suspend, block, etc. so that searching for pcbs is easier

    // Changed this to just call create proc as they both did the exact same thing. Will change back if causes errors - Jared

    createProc(name, function, USER_APPLICATION, 0);
}

void loadR3()
{
    if (pcb_find("Proc1") == NULL)
    {
        /*
        the name of the function, without
        parenthesis, is a pointer to that function
        */
        loadR3PCB("Proc1", (int)proc1);
    }

    if (pcb_find("Proc2") == NULL)
    {
        loadR3PCB("Proc2", (int)proc2);
    }

    if (pcb_find("Proc3") == NULL)
    {
        loadR3PCB("Proc3", (int)proc3);
    }

    if (pcb_find("Proc4") == NULL)
    {
        loadR3PCB("Proc4", (int)proc4);
    }

    if (pcb_find("Proc5") == NULL)
    {
        loadR3PCB("Proc5", (int)proc5);
    }

    else
    {
        char *alreadyIn = {"\nR3 FUNCTIONS HAVE ALREADY BEEN LOADED"};
        sys_req(WRITE, COM1, alreadyIn, strlen(alreadyIn));
    }
}

void createProc(char *name, int function, int class, int priority)
{
    if (pcb_find(name) == NULL)
    {
        pcb *newPCB = pcb_setup(name, class, priority);
        Context *con = (Context *)(newPCB->top);
        con->ESP = (int)(newPCB->top);
        con->CS = 0x08;
        con->DS = 0x10;
        con->ES = 0x10;
        con->FS = 0x10;
        con->GS = 0x10;
        con->SS = 0x10;
        con->EBP = (int)(newPCB->bottom);

        con->EIP = (int)function;
        con->EFLAGS = 0x0202;
        con->EAX = 1;
        con->EBX = 0;
        con->ECX = 0;
        con->EDX = 0;
        con->EDI = 0;
        con->ESI = 0;
        pcb_insert(newPCB);
    }
}
struct Alarm alarm1;
struct Alarm alarm2;
struct Alarm alarm3;
int times[10][3];
char messages;
int count = 0;

char *get_hours()
{
    outb(0x70, 0x04);

    unsigned char hours = '\0';

    int hones = (int)(hours & 0b00001111);
    int htens = (int)((hours >> 4) & 0b00001111);

    hours = hones + (htens * 10);

    static char h[3];
    itoa(hours, h, 10);

    return h;
}

char *get_minutes()
{

    outb(0x70, 0x02);

    unsigned char minutes = '\0';

    int mones = (int)(minutes & 0b00001111);
    int mtens = (int)((minutes >> 4) & 0b00001111);

    minutes = mones + (mtens * 10);

    static char m[3];
    itoa(minutes, m, 10);

    return m;
}

char *get_seconds()
{

    outb(0x70, 0x00);

    unsigned char seconds = '\0';

    int sones = (int)(seconds & 0b00001111);
    int stens = (int)((seconds >> 4) & 0b00001111);

    seconds = sones + (stens * 10);

    static char s[3];
    itoa(seconds, s, 10);

    return s;
}

void procAlarm()
{
    struct Alarm alarmPtr;

    checkAlarm(&alarmPtr);

    if (pcb_find("alarm2") != NULL || pcb_find("alarm3") != NULL || pcb_find("alarm1") != NULL)

    {
        sys_req(IDLE);
    }

    else
    {
        sys_req(EXIT);
    }
}
void setAlarm()
{

    char *prompt = "\nPlease input the message for the alarm.";
    sys_req(WRITE, COM1, prompt, strlen(prompt));

    char *input = sys_alloc_mem(15 * sizeof(char));
    if (input == NULL)
    {
        sys_req(WRITE, COM1, "could not allocate memory\n", strlen("could not allocate memory\n"));
        return;
    }
    sys_req(READ, COM1, input, 15);

    char hoursinput[3];
    char minutesinput[3];
    char secondsinput[3];

    char *hours = "\nPlease input the hour(s) value.\n";
    sys_req(WRITE, COM1, hours, strlen(hours));
    sys_req(READ, COM1, hoursinput, sizeof(hoursinput));

    char *minutes = "\nPlease input the minute(s) value.\n";
    sys_req(WRITE, COM1, minutes, strlen(minutes));
    sys_req(READ, COM1, minutesinput, sizeof(minutesinput));

    char *seconds = "\nPlease input the second(s) value.\n";
    sys_req(WRITE, COM1, seconds, strlen(seconds));
    sys_req(READ, COM1, secondsinput, sizeof(secondsinput));

    struct Alarm *new_alarm = sys_alloc_mem(sizeof(alarm));

    if (new_alarm == NULL)
    {
        sys_req(WRITE, COM1, "could not allocate memory\n", strlen("could not allocate memory\n"));
        sys_free_mem(input);
        return;
    }

    new_alarm->message = input;

    int hour = atoi(hoursinput);

    if (hour > 24 || hour < 0)
    {
        delete_alarm(new_alarm);
        return;
    }
    else
    {
        new_alarm->time.hours = hour;
    }

    int minute = atoi(minutesinput);

    if (minute > 59 || minute < 0)
    {
        delete_alarm(new_alarm);
        return;
    }
    else
    {
        new_alarm->time.minutes = minute;
    }

    int second = atoi(secondsinput);

    if (second > 59 || second < 0)
    {
        delete_alarm(new_alarm);
        return;
    }
    else
    {
        new_alarm->time.seconds = second;
    }

    if (alarms_glob[0] != NULL)
    {
        if (alarms_glob[1] != NULL)
        {
            if (alarms_glob[2] != NULL)
            {
                sys_req(WRITE, COM1, "ERROR: Maximum amount of alarms set.\n", strlen("ERROR: Maximum amount of alarms set.\n"));
                delete_alarm(new_alarm);
                return;
            }
            else
            {
                alarms_glob[2] = new_alarm;
            }
        }
        else
        {
            alarms_glob[1] = new_alarm;
        }
    }
    else
    {
        alarms_glob[0] = new_alarm;
    }

    if (pcb_find("ALARMS") == NULL)
    {
        createProc("ALARMS", (int)checkAlarm, USER_APPLICATION, 7);
    }
}

void delete_alarm(alarm *alarm)
{
    sys_free_mem(alarm->message);
    sys_free_mem(alarm);
}

void checkAlarm()
{
    uint32_t remaining = 0;

    for (int i = 0; i < 3; i++)
    {
        if (alarms_glob[i] != NULL)
        {
            uint32_t curr_hours = atoi(get_hours());

            if (alarms_glob[i]->time.hours < curr_hours)
            {
                sys_req(WRITE, COM1, alarms_glob[i]->message, strlen(alarms_glob[i]->message));
                delete_alarm(alarms_glob[i]);
                alarms_glob[i] = NULL;
            }
            else if (alarms_glob[i]->time.hours > curr_hours)
            {
                remaining++;
                continue;
            }
            else
            {
                uint32_t curr_minutes = atoi(get_minutes());

                if (alarms_glob[i]->time.minutes < curr_minutes)
                {
                    sys_req(WRITE, COM1, alarms_glob[i]->message, strlen(alarms_glob[i]->message));
                    delete_alarm(alarms_glob[i]);
                    alarms_glob[i] = NULL;
                }
                else if (alarms_glob[i]->time.minutes > curr_minutes)
                {
                    remaining++;
                    continue;
                }
                else
                {
                    uint32_t curr_seconds = atoi(get_seconds());

                    if (alarms_glob[i]->time.seconds < curr_seconds)
                    {
                        sys_req(WRITE, COM1, alarms_glob[i]->message, strlen(alarms_glob[i]->message));
                        delete_alarm(alarms_glob[i]);
                        alarms_glob[i] = NULL;
                    }
                    else
                    {
                        remaining++;
                        continue;
                    }
                }
            }
        }
    }

    if (remaining != 0)
    {
        sys_req(IDLE);
    }
    else
    {
        sys_req(EXIT);
    }
}
