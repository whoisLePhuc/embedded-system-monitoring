#ifndef CPU_INFO_H
#define CPU_INFO_H

#include <stdio.h>
#include <sys/types.h> // For pid_t

#define MAX_CORES 32
#define MAX_TOP_PROC 5
#define MAX_NAME_PROC 64

typedef struct {
    pid_t pid;              // Process ID
    char name[MAX_NAME_PROC]; // Tên tiến trình
    float cpu_usage;         // %
} ProcessInfo;

typedef struct {
    float totalUsage;                   // %
    float coreUsage[MAX_CORES];         // %
    float frequency[MAX_CORES];          // MHz
    float temperature;                   // °C
    int coreCount;                     // Number of CPU cores
    ProcessInfo topProcesses[MAX_TOP_PROC];
} cpuInfo;

typedef struct {
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
} CpuStatus;

typedef struct {
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
} cpuCoreStatus;

typedef struct {
    pid_t pid;
    unsigned long long process_time;
} ProcessTimeSnapshot;

float getTotalCpuUsage();
float* get_per_core_cpu_usage(int *core_count_out);
float* get_cpu_frequency(int *core_count_out);
float get_cpu_temperature();
void update_top_processes(cpuInfo* cpu_data);

#endif // CPU_INFO_H
