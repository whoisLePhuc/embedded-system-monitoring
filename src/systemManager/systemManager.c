#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "systemManager.h"
#include "logger/logger.h"

// Function to create and initialize a systemManager object
systemManager  *createSystemManager(){
    systemManager *manager = (systemManager *)malloc(sizeof(systemManager));
    if (manager == NULL) {
        logMessage(LOG_ERROR, "Failed to allocate memory for systemManager");
        return NULL;
    }
    memset(manager, 0, sizeof(systemManager));
    manager->update = updateSystemInfo;
    manager->destroy = destroySystemManager;
    manager->display = displaySystemInfo;
    return manager;
}
// Function to free a systemManager object
void destroySystemManager(systemManager  *self){
    if (self != NULL) {
        free(self);
        self = NULL;
    } else {
        logMessage(LOG_WARNING, "Attempted to free a NULL systemManager pointer");
    }
}
// Function to update system information
void updateSystemInfo(systemManager  *self){
    getSystemInfo(&self->systemInfo);
    getSystemDetails(&self->systemDetails);
    getActiveServices(self->serviceInfo, &self->serviceCount);
}
// Function to display system information
void displaySystemInfo(systemManager *self) {
    if (self == NULL) {
        logMessage(LOG_ERROR, "Cannot display info from a NULL manager.");
        return;
    }
    printf("==================== SYSTEM MONITOR ====================\n");
    printf("Uptime:         %.0f seconds\n", self-> systemInfo.uptime_seconds);
    printf("Load Average:   %.2f (1 min), %.2f (5 min), %.2f (15 min)\n",
           self-> systemInfo.load_avg[0], self-> systemInfo.load_avg[1], self-> systemInfo.load_avg[2]);
    printf("--------------------------------------------------------\n");
    printf("System Time:    %s\n", self->systemDetails.system_time);
    printf("Kernel Version: %s\n", self->systemDetails.kernel_version);
    printf("--------------------------------------------------------\n");
    printf("Active Services:\n");
    for (int i = 0; i < self->serviceCount; i++) {
        if (strlen(self->serviceInfo[i].name) > 0) {
            printf("  - %s: %s\n",
                   self->serviceInfo[i].name,
                   self->serviceInfo[i].description);
        }
    }
    printf("========================================================\n");
}