#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "memoryManager/memoryManager.h"
#include "logger/logger.h"

void getMemoryInfo(MemoryInfo *mem_info) {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) {
        logMessage(LOG_ERROR, "Error opening /proc/meminfo");
        memset(mem_info, 0, sizeof(MemoryInfo));
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "MemTotal:")) {
            sscanf(line, "MemTotal: %ld kB", &mem_info->totalRam);
        } else if (strstr(line, "MemFree:")) {
            sscanf(line, "MemFree: %ld kB", &mem_info->freeRam);
        } else if (strstr(line, "SwapTotal:")) {
            sscanf(line, "SwapTotal: %ld kB", &mem_info->totalSwap);
        } else if (strstr(line, "SwapFree:")) {
            sscanf(line, "SwapFree: %ld kB", &mem_info->freeSwap);
        } else if (strstr(line, "Cached:")) {
            sscanf(line, "Cached: %ld kB", &mem_info->cachedMem);
        }
    }
    fclose(fp);
    mem_info->usedRam = mem_info->totalRam - mem_info->freeRam;
    mem_info->usedSwap = mem_info->totalSwap - mem_info->freeSwap;
}

// qsort comparator for memory usage
// This function sorts processes by their Resident Set Size (vm_rss)
int compareMemUsage(const void *a, const void *b) {
    ramProcessInfo *p1 = (ramProcessInfo *)a;
    ramProcessInfo *p2 = (ramProcessInfo *)b;
    if (p1->vm_rss < p2->vm_rss) return 1;
    if (p1->vm_rss > p2->vm_rss) return -1;
    return 0;
}

void getTopMemoryProcesses(ramProcessInfo topProcesses[], int top_n) {
    DIR *proc_dir;
    struct dirent *entry;
    ramProcessInfo *allProcesses = NULL;
    int processCount = 0;
    proc_dir = opendir("/proc");
    if (!proc_dir) {
        logMessage(LOG_ERROR, "Error opening /proc");
        return;
    }
    while ((entry = readdir(proc_dir)) != NULL) {
        if (isdigit(entry->d_name[0])) {
            char path[512];
            char name[MAX_NAME_PROC] = "N/A";
            long vm_rss = 0;
            logMessage(LOG_DEBUG, "Reading process info for PID: /proc/%s/status", entry->d_name);
            FILE *fp = fopen(path, "r");
            if (fp) {
                char line[256];
                while (fgets(line, sizeof(line), fp)) {
                    if (strstr(line, "Name:")) {
                        sscanf(line, "Name: %s", name);
                    }
                    if (strstr(line, "VmRSS:")) {
                        sscanf(line, "VmRSS: %ld kB", &vm_rss);
                        break;
                    }
                }
                fclose(fp);
                if (vm_rss > 0) {
                    processCount++;
                    allProcesses = realloc(allProcesses, processCount * sizeof(ramProcessInfo));
                    allProcesses[processCount - 1].pid = atoi(entry->d_name);
                    strncpy(allProcesses[processCount - 1].name, name, sizeof(allProcesses[processCount - 1].name));
                    allProcesses[processCount - 1].vm_rss = vm_rss;
                }
            }
        }
    }
    closedir(proc_dir);
    if (processCount > 0) {
        qsort(allProcesses, processCount, sizeof(ramProcessInfo), compareMemUsage);
        int limit = (processCount < top_n) ? processCount : top_n;
        for (int i = 0; i < limit; i++) {
            topProcesses[i] = allProcesses[i];
        }
        free(allProcesses);
    }
}

