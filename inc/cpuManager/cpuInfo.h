#ifndef CPU_INFO_H
#define CPU_INFO_H
#include <sys/types.h> // For pid_t
#define MAX_CORES 32
#define MAX_TOP_PROC 5
#define MAX_NAME_PROC 64

typedef struct {
    pid_t pid;              // Process ID
    char name[MAX_NAME_PROC]; // Process name
    float cpu_usage;         // %
} cpuProcessInfo;

typedef struct {
    float totalUsage;                   // %
    float coreUsage[MAX_CORES];         // %
    float frequency[MAX_CORES];          // MHz
    float temperature;                   // °C
    int coreCount;                     // Number of CPU cores
    cpuProcessInfo topProcesses[MAX_TOP_PROC];
} cpuInfo;

float getTotalCpuUsage();
float* get_per_core_cpu_usage(int *core_count_out);
float* get_cpu_frequency(int *core_count_out);
float get_cpu_temperature();
void update_top_processes(cpuInfo* cpu_data);

#endif // CPU_INFO_H
