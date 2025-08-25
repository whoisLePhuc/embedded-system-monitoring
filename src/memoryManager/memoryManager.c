#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include "memoryManager/memoryManager.h"
#include "logger/logger.h"

void destroyMemoryManager(memoryManager *self){
    if (self != NULL) {
        free(self);
        self = NULL;
    } else {
        logMessage(LOG_WARNING, "Attempted to free a NULL memoryManager pointer");
    }
}

void updateMemoryInfo(memoryManager *self){
    getMemoryInfo(&self->memory_info);
    getTopMemoryProcesses(self->memory_info.top_processes, MAX_TOP_PROC);
}

void displayMemoryInfo(memoryManager *self) {
    if (self == NULL) {
        logMessage(LOG_ERROR, "Cannot display info from a NULL manager.");
        return;
    }
    MemoryInfo *mem_info = &self->memory_info;
    printf("==================== MEMORY MONITOR ====================\n");
    printf("RAM Total: %ld MB\n", mem_info->totalRam / 1024);
    printf("RAM Used:  %ld MB\n", mem_info->usedRam / 1024);
    printf("RAM Free:  %ld MB\n", mem_info->freeRam / 1024);
    printf("Cached:    %ld MB\n", mem_info->cachedMem / 1024);
    printf("--------------------------------------------------------\n");
    printf("Swap Total: %ld MB\n", mem_info->totalSwap / 1024);
    printf("Swap Used:  %ld MB\n", mem_info->usedSwap / 1024);
    printf("Swap Free:  %ld MB\n", mem_info->freeSwap / 1024);
    printf("--------------------------------------------------------\n");
    printf("Top %d Processes by RAM Usage:\n", MAX_TOP_PROC);
    printf("%-10s %-20s %-10s\n", "PID", "NAME", "RAM (MB)");
    for (int i = 0; i < MAX_TOP_PROC; i++) {
        if (mem_info->top_processes[i].pid != 0) {
            printf("%-10d %-20s %.2f\n",
                   mem_info->top_processes[i].pid,
                   mem_info->top_processes[i].name,
                   (float)mem_info->top_processes[i].vm_rss / 1024.0);
        }
    }
    printf("========================================================\n");
}

memoryManager *createMemoryManager(){
    memoryManager *manager = (memoryManager *)malloc(sizeof(memoryManager));
    if (manager == NULL) {
        logMessage(LOG_ERROR, "Failed to allocate memory for memoryManager");
        return NULL;
    }
    memset(manager, 0, sizeof(memoryManager));
    manager->update = updateMemoryInfo;
    manager->destroy = destroyMemoryManager;
    manager->display = displayMemoryInfo;
    return manager;
}