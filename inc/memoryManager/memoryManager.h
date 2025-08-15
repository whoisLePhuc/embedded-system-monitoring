#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "memoryInfo.h"

typedef struct memoryManger memoryManager;
struct memoryManger {
    MemoryInfo memory_info; 
    void (*update)(memoryManager *self); 
    void (*destroy)(memoryManager *self);
    void (*display)(memoryManager *self);
};

memoryManager *createMemoryManager(); // Create a memoryManager object
void destroyMemoryManager(memoryManager *self); // Free memory of memoryManager object
void updateMemoryInfo(memoryManager *self); // Update memory information
void displayMemoryInfo(memoryManager *self); // Display memory information

#endif // MEMORY_MANAGER_H