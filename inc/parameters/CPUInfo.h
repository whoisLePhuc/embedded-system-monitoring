#ifndef CPU_INFO_H
#define CPU_INFO_H

#include "processInfo.h"

#define MAX_CORES 32
#define MAX_TOP_PROC 5

typedef struct {
    float total_usage;                   // %
    float core_usage[MAX_CORES];         // %
    float frequency[MAX_CORES];          // MHz
    float temperature;                   // °C
    ProcessInfo top_processes[MAX_TOP_PROC];
} CPUInfo;

#endif // CPU_INFO_H
