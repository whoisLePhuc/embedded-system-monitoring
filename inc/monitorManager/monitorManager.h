#include "cpuManager/cpuManager.h"
#include "memoryManager/memoryManager.h"
#include "storageManager/storageManager.h"
#include "networkManager/networkManager.h"
#include "systemManager/systemManager.h"

typedef struct MonitorManager MonitorManager;
struct MonitorManager {
    cpuManager *cpu;
    memoryManager *memory;
    storageManager *storage;
    NetworkManager *network;
    systemManager *system;
    void(*destroyMonitorManager)(MonitorManager *self);
    void(*updateInfo)(MonitorManager *self);
    void(*displayInfo)(MonitorManager *self);
};

MonitorManager *createMonitorManager();
void destroyMonitorManager(MonitorManager *self);
void updateInfo(MonitorManager *self);
void displayInfo(MonitorManager *self);