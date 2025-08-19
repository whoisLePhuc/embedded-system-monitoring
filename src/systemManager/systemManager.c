#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "systemManager.h"

systemManager  *createSystemManager(){
    systemManager *manager = (systemManager *)malloc(sizeof(systemManager));
    if (manager == NULL) {
        fprintf(stderr, "Failed to allocate memory for systemManager\n");
        return NULL;
    }
    memset(manager, 0, sizeof(systemManager));
    manager->update = updateSystemInfo;
    manager->destroy = destroySystemManager;
    manager->display = displaySystemInfo;
    return manager;
}
void destroySystemManager(systemManager  *self){
    if (self != NULL) {
        free(self);
        self = NULL;
    } else {
        fprintf(stderr, "Attempted to free a NULL storageManager pointer\n");
    }
}

void updateSystemInfo(systemManager  *self){
    getSystemInfo(&self->systemInfo);
    getSystemDetails(&self->systemDetails);
    getActiveServices(self->serviceInfo, &self->serviceCount);
}

void displaySystemInfo(systemManager *self) {
    if (self == NULL) {
        fprintf(stderr, "Cannot display info from a NULL manager.\n");
        return;
    }

    system("clear");
    printf("==================== SYSTEM MONITOR ====================\n");
    
    // Hiển thị Uptime và Load Average
    printf("Uptime:         %.0f seconds\n", self-> systemInfo.uptime_seconds);
    printf("Load Average:   %.2f (1 min), %.2f (5 min), %.2f (15 min)\n",
           self-> systemInfo.load_avg[0], self-> systemInfo.load_avg[1], self-> systemInfo.load_avg[2]);
    printf("--------------------------------------------------------\n");
    
    // Hiển thị thời gian hệ thống và phiên bản kernel
    printf("System Time:    %s\n", self->systemDetails.system_time);
    printf("Kernel Version: %s\n", self->systemDetails.kernel_version);
    printf("--------------------------------------------------------\n");
    
    // Hiển thị thông tin service
    printf("Active Services:\n");
    for (int i = 0; i < self->serviceCount; i++) {
        // Kiểm tra để tránh in các phần tử rỗng
        if (strlen(self->serviceInfo[i].name) > 0) {
            printf("  - %s: %s\n",
                   self->serviceInfo[i].name,
                   self->serviceInfo[i].description);
        }
    }
    printf("========================================================\n");
}