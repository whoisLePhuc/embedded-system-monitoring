#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "subManager/cpuManager.h"

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

    self->cpu_info.total_usage = getTotalCpuUsage();
    if(self->cpu_info.total_usage < 0) {
        fprintf(stderr, "Failed to get CPU total usage\n");
        return;
    }

    // Here you can add more code to update other fields of cpu_info
    // such as core usage, frequency, temperature, etc.
}


// Cấu trúc để lưu trữ các thông số CPU
typedef struct {
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
} CpuStatus;

// Hàm đọc các thông số từ /proc/stat
int readCpuStatus(CpuStatus *stats) {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        return -1; // Lỗi khi mở file
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

