#ifndef MEMORY_INFO_H
#define MEMORY_INFO_H

#include "processInfo.h"

#define MAX_TOP_PROC 5

typedef struct {
    float ram_used;       // MB
    float ram_free;       // MB
    float swap_used;      // MB
    float swap_free;      // MB
    ProcessInfo top_processes[MAX_TOP_PROC];
    struct {
        float l1; // KB
        float l2; // KB
        float l3; // KB
    } cache;
} MemoryInfo;

#endif // MEMORY_INFO_H
