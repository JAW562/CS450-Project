#ifndef USER_COMMANDS_H
#define USER_COMMANDS_H

#include "pcb.h"
typedef struct Alarm
{
    char *message;
    struct
    {
        uint32_t hours;
        uint32_t minutes;
        uint32_t seconds;
    } time;
} alarm;

// Allows user to get time from system clock
char get_time(void);

// Allows user to set time for system clock
void set_time(void);

// Allows user to get date from system clock
void get_date(void);

// Allows user to set date from system clock
void set_date(void);

// Deletes a process control block
void delete_pcb(char *pcb_name);

// Moves a process to the blocked state
void block_pcb(char *pcb_name);

// Unblocks a blocked process
void unblock_pcb(char *pcb_name);

// Puts a process into the suspended state
void suspend_pcb(char *pcb_name);

// Unsuspends a process
void resume_pcb(char *pcb_name);

// Sets a new priority to the process
void set_pcb_prio(char *pcb_name, char priority);

// Finds a specific process
void show_pcb(char *pcb_name);

// Shows all processes in the ready state
void show_ready();

// Shows all processes in the blocked state
void show_blocked();

// Shows all processes
void show_all();

// Loads R3
void loadR3();

void yield();

void createProc(char *name, int function, int class, int priority);

char *upper_to_lower(char *str);

int sht_dwn(void);

void show_pcb_struct(pcb *pcbptr);

void createAlarm();

void initAlarm();
void setAlarm();
void checkAlarm();
void deleteAlarm();
void alarmCom();
void get_alarm();
void delete_alarm(alarm *alarm);
char *get_hours();
char *get_minutes();
char *get_seconds();

#endif