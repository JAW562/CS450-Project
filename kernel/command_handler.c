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
#include <heap_manager.h>
#include <memory_commands.h>
#include <serial.h>
#include <mpx/io_helpers.h>

#define size 200

// Function Calls
void time_menu();
void proc_menu();
void proc_submenu();
void help_command();
void time_help();
void proc_help();
void proc_subhelp();
void get_version();

// Declares a character array for entered prompt
char buf[size] = {0};
extern struct queue readyPtr;
extern struct queue blockedPtr;
extern struct dcb *device1;

void comhand()
{

    // Intro prompt
    char *intro =
        "\x1b[38;5;198m\x1b[1m\n  █████╗ ██╗  ██╗ ██╗████╗    ████████╗███████╗ █████╗ ███╗   ███╗"
        "\n ██╔══██╗██║  ██║ ███╔═══██╗  ╚══██╔══╝██╔════╝██╔══██╗████╗ ████║"
        "\n ██║  ██║██║  ██║ ██╔╝   ╚═╝     ██║   █████╗  ███████║██╔████╔██║"
        "\n ██║  ██║██║  ██║ ██║            ██║   ██╔══╝  ██╔══██║██║╚██╔╝██║"
        "\n  █████║  █████╔╝ ██║            ██║   ███████╗██║  ██║██║ ╚═╝ ██║"
        "\n  ╚════╝  ╚════╝  ╚═╝            ╚═╝   ╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝"
        "\n ╔═╗┬─┐┌─┐┬ ┬┌┬┐┬ ┬ ┬  ┌─┐┬─┐┌─┐┌─┐┌─┐┌┐┌┬┐┌─┐"
        "\n ╠═╝├┬┘│ ││ │ │││ └┬┘  ├─┘├┬┘├┤ └─┐├┤ ││││ └─┐"
        "\n ╩  ┴└─└─┘└─┘─┴┘┴─┘┴   ┴  ┴└─└─┘└─┘└─┘┘└┘┴ └─┘"
        "\n ┬─┐ ┌─"
        "\n ├┬┘ ├─┐"
        "\n ┴└─ └─┘\n\x1b[0m";

    sys_req(WRITE, COM1, intro, strlen(intro));

    int running = 1;

    while (running)
    {

        // Prompt to be displayed
        char *prompt = "\x1b[38;5;46m\n Please select one of the options below:\n[1]Help\n[2]Version\n[3]Date & Time\n[4]Processes [1]/[2]\n[5]Processes [2]/[2]\n[6]LoadR3PCBs\n[7]Allocate Memory\n[8]Free Memory\n[9]Show Allocated Memory\n[10]Show Free Memory\n[11]Show All Memory\n[12]Shutdown\n";

        // Writes the prompt to COM1 Serial Port
        sys_req(WRITE, COM1, prompt, strlen(prompt));

        // char letter = inb(COM1);

        // while (letter != '\r' || letter != '\n' || letter != 0x1C)
        // {
        //     serial_poll(COM1, buf, size);
        sys_req(READ, COM1, buf, 1);
        // }

        if (strcmp(buf, "1\0") == 0)
        {
            help_command();
        }

        else if (strcmp(buf, "2\0") == 0)
        {
            get_version();
        }

        else if (strcmp(buf, "3\0") == 0)
        {
            time_menu();
        }

        else if (strcmp(buf, "4\0") == 0)
        {
            // char *clear = "\e[1;1H\e[2J";
            // sys_req(WRITE, COM1, clear, strlen(clear));

            proc_menu();
        }

        else if (strcmp(buf, "5\0") == 0)
        {

            proc_submenu();
        }
        else if (strcmp(buf, "6\0") == 0)
        {
            // char *clear = "\e[1;1H\e[2J";
            // sys_req(WRITE, COM1, clear, strlen(clear));
            loadR3();
            const char *proc1 = (const char *)upper_to_lower("Proc1");
            const char *proc2 = (const char *)upper_to_lower("Proc2");
            const char *proc3 = (const char *)upper_to_lower("Proc3");
            const char *proc4 = (const char *)upper_to_lower("Proc4");
            const char *proc5 = (const char *)upper_to_lower("Proc5");

            if (pcb_find(proc1) != NULL && pcb_find(proc2) != NULL && pcb_find(proc3) != NULL && pcb_find(proc4) != NULL && pcb_find(proc5) != NULL)
            {
                char *success = "All R3 processes have been loaded in!";
                sys_req(WRITE, COM1, success, strlen(success));
            }
            else
            {
                char *error = "All R3 processes have NOT been loaded in!";
                sys_req(WRITE, COM1, error, strlen(error));
            }
        }
        else if (strcmp(buf, "7\0") == 0)
        {
            char *allocate = "\nHow much memory would you like to allocate\n";
            sys_req(WRITE, COM1, allocate, strlen(allocate));

            sys_req(READ, COM1, buf, size);
            int sizeAlloc = atoi(buf);
            void *startAddress = memory_alloc(sizeAlloc);

            char dest[9] = {'\0'};

            numtoa((int)startAddress, dest, 16);
            sys_req(WRITE, COM1, dest, strlen(dest));
        }
        else if (strcmp(buf, "8\0") == 0)
        {
            char *free = "\nPlease enter the address of the memory block you would you like to free\n";
            sys_req(WRITE, COM1, free, strlen(free));

            sys_req(READ, COM1, buf, size);

            // int sizeFree = atoi(free);
            uint32_t intToHex = atox(buf);

            //            sys_req(WRITE, COM1, (void *)intToHex, 25);

            memory_free((void *)intToHex);
        }

        else if (strcmp(buf, "9\0") == 0)
        {

            show_alloc_mem();
        }

        else if (strcmp(buf, "10\0") == 0)
        {

            show_free_mem();
        }

        else if (strcmp(buf, "11\0") == 0)
        {
            show_free_mem();
            show_alloc_mem();
        }

        else if (strcmp(buf, "12\0") == 0)
        {
            // shut_dwn = 0;
            running = sht_dwn();
        }

        else
        {
            char *i_prompt = "\nInput was incorrect\n";
            sys_req(WRITE, COM1, i_prompt, strlen(i_prompt));
        }
    }
}

int sht_dwn(void)
{
    // This works now, should be good for the rest of the project. - Jared

    char shut[size] = {'\0'};

    char *clear = "\e[1;1H\e[2J";
    sys_req(WRITE, COM1, clear, strlen(clear));

    char *shut_dwn_prmpt = "\nAre you sure?\n1)Yes\n2)No\n";

    sys_req(WRITE, COM1, shut_dwn_prmpt, strlen(shut_dwn_prmpt));

    sys_req(READ, COM1, shut, size);

    while (1)
    {
        if (strcmp(shut, "1\0") == 0)
        {
            char *shut_dwn_execute = "\nShutting down...\n";
            sys_req(WRITE, COM1, shut_dwn_execute, strlen(shut_dwn_execute));
            readyPtr.front = NULL;
            sys_req(EXIT);
            return 0;
        }

        else if (strcmp(shut, "2\0") == 0)
        {
            return 1;
        }

        else
        {
            char *i_prompt = "\nInput was incorrect\n";
            sys_req(WRITE, COM1, i_prompt, strlen(i_prompt));
            break;
        }
    }
    return 0;
}

// Help command that tells user what each command does
void help_command()
{
    // Clear command to give space on screen
    char *clear = "\e[1;1H\e[2J";
    sys_req(WRITE, COM1, clear, strlen(clear));

    // New buffer for command
    char buf[size];

    int run = 1;

    // Makes statement run until user inputs correctly
    while (run)
    {

        // Prints prompt and gets user input

        char *helpmenu = "\nWhich command do you need help with?\n[1] Version \n[2] Date & Time Commands\n[3] Process commands[1]/[2]\n[4]Process commands[2]/[2]\n[5] Allocate Memory\n[6]Free Memory\n[7]Show Allocated Memory\n[8]Show Free Memory\n[9]Shutdown";

        sys_req(WRITE, COM1, helpmenu, strlen(helpmenu));
        memset(buf, '\0', size);
        sys_req(READ, COM1, buf, size);

        if (strcmp(buf, "1\0") == 0)
        {
            char *v_prompt = "\nThis command will get you the version of the OS you are currently using.\n";
            sys_req(WRITE, COM1, v_prompt, strlen(v_prompt));

            run = 0;
        }

        else if (strcmp(buf, "2\0") == 0)
        {

            time_help();
            run = 0;
        }

        else if (strcmp(buf, "3\0") == 0)
        {
            proc_help();

            run = 0;
        }

        else if (strcmp(buf, "4\0") == 0)
        {

            proc_subhelp();

            run = 0;
        }

        else if (strcmp(buf, "5\0") == 0)
        {
            char *s_prompt = "\nThis command will shut down the OS.\n";
            sys_req(WRITE, COM1, s_prompt, strlen(s_prompt));
            run = 0;
        }
        else if (strcmp(buf, "6\0") == 0)
        {
            char *free_prompt = "\nThis command will allow the user to free allocated memory in the system.\n";
            sys_req(WRITE, COM1, free_prompt, strlen(free_prompt));
            run = 0;
        }
        else if (strcmp(buf, "7\0") == 0)
        {
            char *show_alloc_prompt = "\nThis command will show the user all allocated memory addresses and their sizes on the system.\n";
            sys_req(WRITE, COM1, show_alloc_prompt, strlen(show_alloc_prompt));
            run = 0;
        }
        else if (strcmp(buf, "8\0") == 0)
        {
            char *show_free_prompt = "\nThis command will show the user all the free memory addresses and their sizes on the system.\n";
            sys_req(WRITE, COM1, show_free_prompt, strlen(show_free_prompt));
            run = 0;
        }
        else if (strcmp(buf, "9\0") == 0)
        {
            char *shutdown_prompt = "\nThis command will shut down the OS.\n";
            sys_req(WRITE, COM1, shutdown_prompt, strlen(shutdown_prompt));
            run = 0;
        }

        else
        {
            char *i_prompt = "\nInput was incorrect\n";
            sys_req(WRITE, COM1, i_prompt, strlen(i_prompt));
        }
    }
}

// Displays Version Number & Compilation Date
void get_version()
{

    char *prompt = "\nCurrent Version is: R6\n";
    sys_req(WRITE, COM1, prompt, strlen(prompt));

    char *compilation = "\nCompiled on: ";

    sys_req(WRITE, COM1, compilation, strlen(prompt));
    sys_req(WRITE, COM1, __DATE__, strlen(__DATE__));
    sys_req(WRITE, COM1, "\n", 1);
}

void time_menu()
{
    char *prompt = "Please choose a command.\n[1]Get Time\n[2]Set Time\n[3]Get Date\n[4]Set Date\n[5]Set Alarm\n[6]Go Back";

    int back = 1;

    while (back)
    {
        sys_req(WRITE, COM1, prompt, strlen(prompt));
        memset(buf, '\0', size);
        sys_req(READ, COM1, buf, size);
        if (strcmp(buf, "1\0") == 0)
        {
            get_time();
            break;
        }

        else if (strcmp(buf, "2\0") == 0)
        {
            set_time();
            break;
        }

        else if (strcmp(buf, "3\0") == 0)
        {
            get_date();
            break;
        }

        else if (strcmp(buf, "4\0") == 0)
        {
            set_date();
            break;
        }

        else if (strcmp(buf, "5\0") == 0)
        {
            setAlarm();

            break;
        }

        else if (strcmp(buf, "6\0") == 0)
        {
            back = 0;
        }

        else
        {

            char *p = "\nInput was incorrect.\n";
            sys_req(WRITE, COM1, p, strlen(p));
        }
    }
}

void proc_menu()
{
    char *prompt = "\nProcess Commands\n[1]Delete Process\n[2]Block Process\n[3]Unblock Process\n[4]Suspend Process\n[5]Unsuspend Process\n[6]Set Process Priority\n[7]Go Back\n";

    int back = 1;

    while (back == 1)
    {

        sys_req(WRITE, COM1, prompt, strlen(prompt));
        memset(buf, '\0', size);
        sys_req(READ, COM1, buf, size);

        if (strcmp(buf, "1\0") == 0)
        {
            char name[9] = {'\0'};

            char *p = "\nPlease enter the name of the process to delete\n";

            sys_req(WRITE, COM1, p, strlen(p));

            sys_req(READ, COM1, name, 8);

            delete_pcb(name);

            back = 0;
        }

        else if (strcmp(buf, "2\0") == 0)
        {
            char name[9] = {'\0'};

            char *p = "\nPlease enter the name of the process to block.\n";

            sys_req(WRITE, COM1, p, strlen(p));

            sys_req(READ, COM1, name, 8);

            block_pcb(name);

            back = 0;
        }

        else if (strcmp(buf, "3\0") == 0)
        {
            char name[9] = {'\0'};

            char *p = "Please enter the name of the process to unblock.\n";

            sys_req(WRITE, COM1, p, strlen(p));
            sys_req(READ, COM1, name, 8);

            unblock_pcb(name);

            back = 0;
        }

        else if (strcmp(buf, "4\0") == 0)
        {
            char name[9] = {'\0'};

            char *p = "\nPlease enter the name of the process to suspend.\n";

            sys_req(WRITE, COM1, p, strlen(p));

            sys_req(READ, COM1, name, 8);

            suspend_pcb(name);

            back = 0;
        }

        else if (strcmp(buf, "5\0") == 0)
        {

            char name[9] = {'\0'};

            char *p = "\nPlease enter the name of the process to unsuspend.\n";

            sys_req(WRITE, COM1, p, strlen(p));

            sys_req(READ, COM1, name, 8);

            resume_pcb(name);

            back = 0;
        }

        else if (strcmp(buf, "6\0") == 0)
        {
            int priority = 0;

            char name[9] = {'\0'};

            char *p = "\nPlease enter the name of the process to change priority.\n";

            sys_req(WRITE, COM1, p, strlen(p));

            sys_req(READ, COM1, name, 8);

            char prioString[2] = {'\0'};
            char *s = "\nPlease enter an integer for the new priority. NOTE: MUST BE IN THE RANGE OF 1 TO 9!\n";

            sys_req(WRITE, COM1, s, strlen(s));

            sys_req(READ, COM1, prioString, 1);

            priority = atoi(prioString);

            set_pcb_prio(name, priority);

            back = 0;
        }

        else if (strcmp(buf, "7\0") == 0)
        {
            back = 0;
        }

        else
        {

            char *p = "\nInput was incorrect.\n";

            sys_req(WRITE, COM1, p, strlen(p));
        }
    }
}

void proc_submenu()

{

    char *prompt = "\nProcess commands continued\n[1]Show Specific Process\n[2]Show Ready Processes\n[3]Show Blocked Processes\n[4]Show All Processes\n[5]Main Menu\npage[2]/[2]\n";

    char *error = "Invalid entry, try again.";

    int back = 1;

    while (back)
    {
        sys_req(WRITE, COM1, prompt, strlen(prompt));
        memset(buf, '\0', size);
        sys_req(READ, COM1, buf, size);

        if (strcmp(buf, "1\0") == 0)
        {
            char name[9] = {'\0'};

            char *p = "\nPlease enter the name of the process you want to find.\n";

            sys_req(WRITE, COM1, p, strlen(p));

            sys_req(READ, COM1, name, 8);

            show_pcb(name);

            break;
        }

        else if (strcmp(buf, "2\0") == 0)
        {

            show_ready();

            break;
        }

        else if (strcmp(buf, "3\0") == 0)
        {

            show_blocked();

            break;
        }

        else if (strcmp(buf, "4\0") == 0)
        {

            show_all();

            break;
        }

        else if (strcmp(buf, "5\0") == 0)
        {
            break;
        }

        else
        {

            sys_req(WRITE, COM1, error, strlen(error));
        }
    }
}

void time_help()
{

    char *prompt = "\nWhich Date & Time command do you need help with.\n[1]Get Time\n[2]Set Time\n[3]Get Date\n[4]Set Date\n[5]Set Alarm\n[6]Go Back\n";

    int back = 1;

    while (back)
    {
        sys_req(WRITE, COM1, prompt, strlen(prompt));
        memset(buf, '\0', size);
        sys_req(READ, COM1, buf, size);

        if (strcmp(buf, "1\0") == 0)
        {
            char *p = "\nTHis command will get you the time (in UTC).";

            sys_req(WRITE, COM1, p, strlen(prompt));

            break;
        }

        else if (strcmp(buf, "2\0") == 0)
        {

            char *p = "This command will allow you to set the time (in UTC).";

            sys_req(WRITE, COM1, p, strlen(p));

            break;
        }

        else if (strcmp(buf, "3\0") == 0)
        {

            char *p = "\nThis command will display the date (Month / Day / Year)";

            sys_req(WRITE, COM1, p, strlen(p));

            break;
        }

        else if (strcmp(buf, "4\0") == 0)
        {

            char *p = "\nThis command will allow you to set the date (Month / Dat/ Year)";

            sys_req(WRITE, COM1, p, strlen(p));

            break;
        }

        else if (strcmp(buf, "5\0") == 0)
        {

            char *p = "\nThis command will allow you to set an alarm.";

            sys_req(WRITE, COM1, p, strlen(p));

            break;
        }

        else if (strcmp(buf, "6\0") == 0)
        {

            break;
        }
    }
}

void proc_help()
{

    char *processhelp1 = "\nWhich Process command do you need help with?\n[1]Delete Process\n[2]Block Process\n[3]Unblock Process\n[4]Suspend Process\n[5]Resume Process\n[6]Set Process Priority\n[7]Go Back\n[8]Go to next Page\npage [1]/[2]\n";

    int back = 1;

    while (back)
    {
        sys_req(WRITE, COM1, processhelp1, strlen(processhelp1));
        memset(buf, '\0', size);
        sys_req(READ, COM1, buf, size);

        if (strcmp(buf, "1\0") == 0)
        {

            char *prompt = "This command will delete a process.\nNote: Cannot delete a system process!\n";

            sys_req(WRITE, COM1, prompt, strlen(prompt));

            break;
        }

        else if (strcmp(buf, "2\0") == 0)
        {
            char *prompt = "\nThis command will put the specified process in the blocked state.\n";

            sys_req(WRITE, COM1, prompt, strlen(prompt));

            break;
        }

        else if (strcmp(buf, "3\0") == 0)
        {

            char *prompt = "\nThis command move a process from the blocked state to the unblocked state.\n";

            sys_req(WRITE, COM1, prompt, strlen(prompt));

            break;
        }

        else if (strcmp(buf, "4\0") == 0)
        {

            char *prompt = "\nThis command will put a process in the suspended state.\n";

            sys_req(WRITE, COM1, prompt, strlen(prompt));

            break;
        }

        else if (strcmp(buf, "5\0") == 0)
        {

            char *prompt = "\nThis command will move a suspended process into the unsuspended state.\n";

            sys_req(WRITE, COM1, prompt, strlen(prompt));

            break;
        }

        else if (strcmp(buf, "6\0") == 0)
        {

            char *prompt = "\nThis command will allow you to set a new priority for the process.\n";

            sys_req(WRITE, COM1, prompt, strlen(prompt));

            break;
        }

        else if (strcmp(buf, "7\0") == 0)
        {

            break;
        }

        else if (strcmp(buf, "8\0") == 0)
        {

            break;
        }
    }
}

void proc_subhelp()
{

    char *prompt = "Page 2\n[1]Show Specific Process\n[2]Show All Ready Processes\n[3]Show All Blocked Processes\n[4]Show All Processes\n[5]Go Back\npage[2]/[2]\n";

    int back = 1;

    while (back)
    {
        sys_req(WRITE, COM1, prompt, strlen(prompt));
        memset(buf, '\0', size);
        sys_req(READ, COM1, buf, size);

        if (strcmp(buf, "1\0") == 0)
        {

            char *p = "This command will show a specific process when given the name.";

            sys_req(WRITE, COM1, p, strlen(p));

            break;
        }

        else if (strcmp(buf, "2\0") == 0)
        {

            char *p = "This command will show all the processes currently in the ready queue.";

            sys_req(WRITE, COM1, p, strlen(p));

            break;
        }

        else if (strcmp(buf, "3\0") == 0)
        {

            char *p = "This command will show all the processes currently in this blocked queue.";

            sys_req(WRITE, COM1, p, strlen(p));

            break;
        }

        else if (strcmp(buf, "4\0") == 0)
        {

            char *p = "This command will show all processes.";

            sys_req(WRITE, COM1, p, strlen(p));

            break;
        }

        else if (strcmp(buf, "5\0") == 0)
        {

            break;
        }
    }
}

// void command_handler()
// {
//     // printf("\n%s", intro());
//     // printf("%p\n", *intro);
//     // printf("%p\n", *comhand);
//     intro();
//     comhand();
// }
