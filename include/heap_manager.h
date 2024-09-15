#ifndef HEAPMANAGER_H
#define HEAPMANAGER_H

#define FREE_LIST 0
#define ALLOCATED_LIST 1

typedef struct mcb
{
    void* start;
    size_t size;
    struct mcb* next;
    struct mcb* prev;
    int indicator;
}mcb;


void initialize_heap(size_t size);

void *allocate_memory(size_t size);

int free_memory(void *block);

// Use for allocate
mcb *split(mcb *blockToBeSplit, int size);

// Use for free
int merge(mcb *blockToBeMerged);

int mcb_remove(struct mcb *m);

static void * (*malloc_function)(size_t);
static int (*free_function)(void *);




#endif
