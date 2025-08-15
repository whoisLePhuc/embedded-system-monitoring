#ifndef MEMORY_INFO_H
#define MEMORY_INFO_H
#define MAX_TOP_PROC 5
#define MAX_NAME_PROC 64
#include <sys/types.h> // For pid_t

typedef struct {
    pid_t pid;
    char name[MAX_NAME_PROC];
    long vm_rss; // Resident Set Size (real usage Ram = kB) 
} ramProcessInfo;

typedef struct {
    long totalRam;    // kB
    long freeRam;     // kB
    long usedRam;     // kB
    long totalSwap;   // kB
    long freeSwap;    // kB
    long usedSwap;    // kB
    long cachedMem;   // kB
    ramProcessInfo top_processes[MAX_TOP_PROC];
} MemoryInfo;

void getMemoryInfo(MemoryInfo *mem_info);
void getTopMemoryProcesses(ramProcessInfo top_processes[], int top_n);

#endif // MEMORY_INFO_H
