#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include "cpuManager/cpuManager.h"
#include "cpuManager/cpuInfo.h"

// Function to create a cpuManager object
cpuManager *createCpuManager() {
    cpuManager *manager = (cpuManager *)malloc(sizeof(cpuManager));
    if (manager == NULL) {
        perror("Failed to allocate memory for cpuManager");
        return NULL;
    }
    memset(manager, 0, sizeof(cpuManager));
    manager->update = updateCpuInfo;
    manager->destroy = destroyCpuManager;
    manager->display = displayCpuInfo;
    return manager;
}

// Function to free memory of cpuManager object
void destroyCpuManager(cpuManager *self){
    if(self != NULL) {
        free(self);
        self = NULL;
    } else {
        fprintf(stderr, "Attempted to free a NULL cpuManager pointer\n");
    }
}

void displayCpuInfo(cpuManager* manager) {
    if (manager == NULL) {
        fprintf(stderr, "Cannot display info from a NULL manager.\n");
        return;
    }

    printf("\033[2J\033[1;1H"); // Xóa màn hình và di chuyển con trỏ lên đầu
    
    printf("==================== CPU MONITOR ====================\n");
    printf("Total Usage: %.2f%%\n", manager->CpuInfo.totalUsage);
    printf("Temperature: %.2f C\n", manager->CpuInfo.temperature);
    
    printf("-----------------------------------------------------\n");
    printf("Per-Core Usage & Frequency:\n");
    for (int i = 0; i < manager->CpuInfo.coreCount; i++) {
        printf("  Core %d: Usage: %.2f%% | Freq: %.2f MHz\n",
               i, 
               manager->CpuInfo.coreUsage[i], 
               manager->CpuInfo.frequency[i]);
    }
    
    printf("-----------------------------------------------------\n");
    printf("Top %d Processes:\n", MAX_TOP_PROC);
    printf("%-10s %-20s %-10s\n", "PID", "NAME", "CPU %");
    for (int i = 0; i < MAX_TOP_PROC; i++) {
        if (manager->CpuInfo.topProcesses[i].pid != 0) {
            printf("%-10d %-20s %.2f\n",
                   manager->CpuInfo.topProcesses[i].pid,
                   manager->CpuInfo.topProcesses[i].name,
                   manager->CpuInfo.topProcesses[i].cpu_usage);
        }
    }
    printf("=====================================================\n");
}

// Function to update CPU information
void updateCpuInfocp(cpuManager *self){
    if(self == NULL) {
        fprintf(stderr, "cpuManager pointer is NULL\n");
        return;
    }
    self->CpuInfo.totalUsage = getTotalCpuUsage();
    if(self->CpuInfo.totalUsage < 0) {
        fprintf(stderr, "Failed to get CPU total usage\n");
    }

    int core_count;
    float* usage_array = get_per_core_cpu_usage(&core_count);
    if (usage_array != NULL) {
        // Sao chép dữ liệu và lưu số lượng core thực tế
        if(core_count <= MAX_CORES) {
            for (int i = 0; i < core_count; i++) {
                self->CpuInfo.coreUsage[i] = usage_array[i];
            }
            self->CpuInfo.coreCount = core_count;
        } else {
            fprintf(stderr, "Core count exceeds MAX_CORES\n");
            self->CpuInfo.coreCount = MAX_CORES;
        }
        free(usage_array);
    } else {
        fprintf(stderr, "Failed to get per-core CPU usage\n");
        self->CpuInfo.coreCount = 0;
    }

    float* frequencies = get_cpu_frequency(&core_count);
    // Kiểm tra số lượng core có khớp không để đảm bảo dữ liệu hợp lệ
    if (frequencies != NULL && core_count == self->CpuInfo.coreCount) {
        for (int i = 0; i < core_count; i++) {
            self->CpuInfo.frequency[i] = frequencies[i];
        }
        free(frequencies);
    } else {
        fprintf(stderr, "Failed to get CPU core frequencies or core count mismatch\n");
        if(frequencies) free(frequencies);
    }

    self->CpuInfo.temperature = get_cpu_temperature();
    if(self->CpuInfo.temperature < 0) {
        fprintf(stderr, "Failed to get CPU temperature\n");
    }

    update_top_processes(&self->CpuInfo);
}

