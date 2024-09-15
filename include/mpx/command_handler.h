#ifndef MPX_COMMANDS_H
#define MPX_COMMANDS_H

// Command Handler prototype for Kmain.c
// calls the prompt and menu
void command_handler(void);

// the menu
void comhand();

// Writes what each command does to the screen
void help_command(void);

// Shows what version user is currently working with
void get_version(void);

int sht_dwn(void);

// All of these are submenus for the main menu
void time_menu();

void proc_menu();

void proc_submenu();

void time_help();

void proc_help();

void proc_subhelp();

#endif