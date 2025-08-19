#include <stdio.h>
#include <stdlib.h>
#include "monitorManager.h"

MonitorManager *createMonitorManager() {
    MonitorManager *manager = (MonitorManager *)malloc(sizeof(MonitorManager));
    if (manager == NULL) {
        return NULL;
    }
    manager->cpu = createCpuManager();
    manager->memory = createMemoryManager();
    manager->storage = createStorageManager();
    manager->network = createNetworkManager();
    manager->system = createSystemManager();
    return manager;
}

void destroyMonitorManager(MonitorManager *self) {
    if (self == NULL) {
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
    self->system->update(self->system);
    self->cpu->update(self->cpu);
    self->memory->update(self->memory);
    self->storage->update(self->storage);
    self->network->update(self->network);
}

void displayInfo(MonitorManager *self){
    self->system->display(self->system);
    self->cpu->display(self->cpu);
    self->memory->display(self->memory);
    self->storage->display(self->storage);
    self->network->display(self->network);
}