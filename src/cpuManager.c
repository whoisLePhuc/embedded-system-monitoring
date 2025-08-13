#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "manager/cpuManager.h"

// Forward declaration
float getTotalCpuUsage(); 

cpuManager *createCpuManager(){
    cpuManager *manager = (cpuManager *)malloc(sizeof(cpuManager));
    if(manager == NULL) {
        fprintf(stderr, "Memory allocation failed for cpuManager\n");
        return NULL;
    }
    memset(manager, 0, sizeof(cpuManager));
    manager->update = updateCpuInfo;
    return manager;
}

void destroyCpuManager(cpuManager *self){
    if(self != NULL) {
        free(self);
        self = NULL;
    } else {
        fprintf(stderr, "Attempted to free a NULL cpuManager pointer\n");
    }
}


void updateCpuInfo(cpuManager *self){
    if(self == NULL) {
        fprintf(stderr, "cpuManager pointer is NULL\n");
        return;
    }
    // Get CPU total usage
    self->cpu_info.total_usage = getTotalCpuUsage();
    if(self->cpu_info.total_usage < 0) {
        fprintf(stderr, "Failed to get CPU total usage\n");
        return;
    }
    // Get per-core CPU usage 
    int core_count;
    float* usage_array = get_per_core_cpu_usage(&core_count);  
    if (usage_array != NULL) {
        for (int i = 0; i < core_count; i++) {
            self->cpu_info.core_usage[i] = usage_array[i];
        }
        free(usage_array); 
    }
    
    // Here you can add more code to update other fields of cpu_info
    // such as core usage, frequency, temperature, etc.
}

// ========= Function to get CPU usage information =========
typedef struct {
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
} CpuStatus;

int readCpuStatus(CpuStatus *stats) {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        return -1; 
    }
    fscanf(fp, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
           &stats->user, &stats->nice, &stats->system, &stats->idle,
           &stats->iowait, &stats->irq, &stats->softirq, &stats->steal);
    fclose(fp);
    return 0;
}

// Hàm tính phần trăm sử dụng CPU
float getTotalCpuUsage() {
    CpuStatus stats1, stats2;
    unsigned long long prev_idle, prev_total, curr_idle, curr_total;
    float cpu_usage;

    if (readCpuStatus(&stats1) == -1) return -1.0;
    prev_idle = stats1.idle + stats1.iowait;
    prev_total = stats1.user + stats1.nice + stats1.system + prev_idle +
                 stats1.irq + stats1.softirq + stats1.steal;
    sleep(1); // Chờ 1 giây
    if (readCpuStatus(&stats2) == -1) return -1.0;
    curr_idle = stats2.idle + stats2.iowait;
    curr_total = stats2.user + stats2.nice + stats2.system + curr_idle +
                 stats2.irq + stats2.softirq + stats2.steal;

    if (curr_total - prev_total == 0) return 0.0;
    
    cpu_usage = (float)(curr_total - prev_total - (curr_idle - prev_idle)) /
                (curr_total - prev_total) * 100.0;
    return cpu_usage;
}

// ========= Function to get CPU core usage information =========
typedef struct {
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
} cpuCoreStatus;

int readCoreStatus(cpuCoreStatus stats[], int *core_count) {
    FILE *fp;
    char buffer[256];
    int count = 0;

    fp = fopen("/proc/stat", "r");
    if (!fp) {
        perror("Error opening /proc/stat");
        return -1;
    }
    fgets(buffer, sizeof(buffer), fp); // Bỏ qua dòng tổng cpu
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (strncmp(buffer, "cpu", 3) == 0 && buffer[3] >= '0' && buffer[3] <= '9') {
            sscanf(buffer, "cpu%*d %llu %llu %llu %llu %llu %llu %llu %llu",
                   &stats[count].user, &stats[count].nice, &stats[count].system,
                   &stats[count].idle, &stats[count].iowait, &stats[count].irq,
                   &stats[count].softirq, &stats[count].steal);
            count++;
            if (count >= MAX_CORES) break; // tránh tràn mảng
        }
    }
    fclose(fp);
    *core_count = count;
    return 0;
}

float* get_per_core_cpu_usage(int *core_count_out) {
    cpuCoreStatus prev[MAX_CORES], curr[MAX_CORES];
    int cores_prev = 0, cores_curr = 0;
    if (read_per_core_stats(prev, &cores_prev) == -1) return NULL;
    sleep(1);
    if (read_per_core_stats(curr, &cores_curr) == -1) return NULL;
    if (cores_prev != cores_curr) {
        fprintf(stderr, "Core count changed between samples!\n");
        return NULL;
    }
    float* usage_array = (float*)malloc(cores_curr * sizeof(float));
    if (!usage_array) {
        perror("Failed to allocate memory");
        return NULL;
    }
    for (int i = 0; i < cores_curr; i++) {
        unsigned long long prev_idle = prev[i].idle + prev[i].iowait;
        unsigned long long curr_idle = curr[i].idle + curr[i].iowait;

        unsigned long long prev_total = prev[i].user + prev[i].nice + prev[i].system +
                                        prev_idle + prev[i].irq + prev[i].softirq + prev[i].steal;

        unsigned long long curr_total = curr[i].user + curr[i].nice + curr[i].system +
                                        curr_idle + curr[i].irq + curr[i].softirq + curr[i].steal;
        
        unsigned long long total_diff = curr_total - prev_total;
        unsigned long long idle_diff  = curr_idle - prev_idle;
        
        if (total_diff != 0) {
            usage_array[i] = (float)(total_diff - idle_diff) / total_diff * 100.0f;
        } else {
            usage_array[i] = 0.0f;
        }
    }
    *core_count_out = cores_curr;
    return usage_array;
}