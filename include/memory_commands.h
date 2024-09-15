#ifndef MEMORYCOMMANDS_H
#define MEMORYCOMMANDS_H

#include <heap_manager.h>

void* memory_alloc(int size);

void memory_free(void* address);

void show_mcb(mcb *mcb);

void show_alloc_mem();

void show_free_mem();

#endif