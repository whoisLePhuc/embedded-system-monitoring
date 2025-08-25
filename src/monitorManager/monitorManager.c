#include <stdio.h>
#include <stdlib.h>
#include "monitorManager.h"
#include "logger/logger.h"

MonitorManager *createMonitorManager() {
    MonitorManager *manager = (MonitorManager *)malloc(sizeof(MonitorManager));
    if (manager == NULL) {
        return NULL;
    }
    manager->cpu = createCpuManager();
    if(manager->cpu == NULL) {
        logMessage(LOG_ERROR, "Failed to create cpuManager");
        free(manager);
        return NULL;
    }
    manager->memory = createMemoryManager();
    if(manager->memory == NULL) {
        logMessage(LOG_ERROR, "Failed to create memoryManager");
        manager->cpu->destroy(manager->cpu);
        free(manager);
        return NULL;
    }
    manager->storage = createStorageManager();
    if(manager->storage == NULL) {
        logMessage(LOG_ERROR, "Failed to create storageManager");
        manager->cpu->destroy(manager->cpu);
        manager->memory->destroy(manager->memory);
        free(manager);
        return NULL;
    }
    manager->network = createNetworkManager();
    if(manager->network == NULL) {
        logMessage(LOG_ERROR, "Failed to create NetworkManager");
        manager->cpu->destroy(manager->cpu);
        manager->memory->destroy(manager->memory);
        manager->storage->destroy(manager->storage);
        free(manager);
        return NULL;
    }
    manager->system = createSystemManager();
    if(manager->system == NULL) {
        logMessage(LOG_ERROR, "Failed to create systemManager");
        manager->cpu->destroy(manager->cpu);
        manager->memory->destroy(manager->memory);
        manager->storage->destroy(manager->storage);
        manager->network->destroy(manager->network);
        free(manager);
        return NULL;
    }
    manager->updateInfo = updateInfo;
    manager->displayInfo = displayInfo;
    manager->destroyMonitorManager = destroyMonitorManager;
    return manager;
}

void destroyMonitorManager(MonitorManager *self) {
    if (self == NULL) {
        logMessage(LOG_WARNING, "Attempted to destroy a NULL MonitorManager pointer");
        return;
    }
    self->cpu->destroy(self->cpu);
    self->memory->destroy(self->memory);
    self->storage->destroy(self->storage);
    self->network->destroy(self->network);
    self->system->destroy(self->system);
    free(self);
}

void updateInfo(MonitorManager *self){
    if (self == NULL) {
        logMessage(LOG_WARNING, "Attempted to update a NULL MonitorManager pointer");
        return;
    }
    self->system->update(self->system);
    self->cpu->update(self->cpu);
    self->memory->update(self->memory);
    self->storage->update(self->storage);
    self->network->update(self->network);
}

void displayInfo(MonitorManager *self){
    if (self == NULL) {
        logMessage(LOG_WARNING, "Attempted to display a NULL MonitorManager pointer");
        return;
    }
    self->system->display(self->system);
    self->cpu->display(self->cpu);
    self->memory->display(self->memory);
    self->storage->display(self->storage);
    self->network->display(self->network);
}