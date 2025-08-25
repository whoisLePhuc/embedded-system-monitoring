#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include "cpuManager/cpuManager.h"
#include "cpuManager/cpuInfo.h"
#include "logger/logger.h"

// Function to free memory of cpuManager object
void destroyCpuManager(cpuManager *self){
    if(self != NULL) {
        free(self);
        self = NULL;
    } else {
        logMessage(LOG_WARNING, "Attempted to free a NULL cpuManager pointer");
    }
}

void displayCpuInfo(cpuManager* manager) {
    if (manager == NULL) {
        logMessage(LOG_ERROR, "Cannot display info from a NULL manager.");
        return;
    }
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
void updateCpuInfo(cpuManager *self) {
    if (self == NULL) {
        logMessage(LOG_ERROR, "cpuManager pointer is NULL in updateCpuInfo");
        return;
    }
    // --- Total CPU Usage ---
    self->CpuInfo.totalUsage = getTotalCpuUsage();
    if (self->CpuInfo.totalUsage < 0.0f) {  // giả sử hàm trả về <0 nếu lỗi
        logMessage(LOG_ERROR, "Failed to get CPU total usage");
        self->CpuInfo.totalUsage = 0.0f;
    }
    // --- Per-core CPU usage ---
    int usage_cores = 0;
    float *usage_array = get_per_core_cpu_usage(&usage_cores);
    if (usage_array != NULL && usage_cores > 0) {
        int count = (usage_cores <= MAX_CORES) ? usage_cores : MAX_CORES;
        for (int i = 0; i < count; i++) {
            self->CpuInfo.coreUsage[i] = usage_array[i];
        }
        // Nếu core_count > MAX_CORES, bỏ qua phần dư và log cảnh báo
        if (usage_cores > MAX_CORES) {
            logMessage(LOG_WARNING, 
                       "Core count exceeds MAX_CORES: %d > %d (truncating)", 
                       usage_cores, MAX_CORES);
        }
        self->CpuInfo.coreCount = count;
        free(usage_array);
    } else {
        logMessage(LOG_ERROR, "Failed to get per-core CPU usage");
        self->CpuInfo.coreCount = 0;
        memset(self->CpuInfo.coreUsage, 0, sizeof(self->CpuInfo.coreUsage));
    }
    // --- Per-core CPU frequency ---
    int freq_cores = 0;
    float *frequencies = get_cpu_frequency(&freq_cores);
    int n = (freq_cores < self->CpuInfo.coreCount) ? freq_cores : self->CpuInfo.coreCount;
    for (int i = 0; i < n; i++) {
        self->CpuInfo.frequency[i] = frequencies[i];
    }
    if (n < self->CpuInfo.coreCount) {
        memset(&self->CpuInfo.frequency[n], 0,
        (self->CpuInfo.coreCount - n) * sizeof(float));
    }
    free(frequencies);
    // --- CPU Temperature ---
    self->CpuInfo.temperature = get_cpu_temperature();
    if (self->CpuInfo.temperature < 0.0f) {
        logMessage(LOG_ERROR, "Failed to get CPU temperature");
        self->CpuInfo.temperature = 0.0f;
    }
    // --- Update top processes ---
    update_top_processes(&self->CpuInfo);
}

// Function to create a cpuManager object
cpuManager *createCpuManager() {
    cpuManager *manager = (cpuManager *)malloc(sizeof(cpuManager));
    if (manager == NULL) {
        logMessage(LOG_ERROR, "Failed to allocate memory for cpuManager");
        return NULL;
    }
    memset(manager, 0, sizeof(cpuManager));
    manager->update = updateCpuInfo;
    manager->destroy = destroyCpuManager;
    manager->display = displayCpuInfo;
    return manager;
}

