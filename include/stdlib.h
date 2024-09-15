#ifndef MPX_STDLIB_H
#define MPX_STDLIB_H

#include<stdint.h>
#include <stdarg.h>

/**
 @file stdlib.h
 @brief A subset of standard C library functions.
*/

/**
 Convert an ASCII string to an integer
 @param s A NUL-terminated string
 @return The value of the string converted to an integer
*/
int atoi(const char *s);

/* 
 Convert an integer to a hex number
*/
int atox(const char *s);

char* numtoa(uint32_t i, char* dest, int base);

int printf(char* format, ...);

void puts(char* str);

#endif
