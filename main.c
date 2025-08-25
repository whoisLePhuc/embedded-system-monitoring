#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cpuManager/cpuManager.h"
#include "memoryManager/memoryManager.h"
#include "storageManager/storageManager.h"
#include "networkManager/networkManager.h"
#include "systemManager/systemManager.h"
#include "monitorManager/monitorManager.h"
#include "logger/logger.h"


int main(){
    loggerInit(LOG_INFO, "log_file.txt");
    MonitorManager *manager = createMonitorManager();
    if (manager == NULL) {
        logMessage(LOG_ERROR, "Failed to create MonitorManager");
    }
    while(1){
        manager->updateInfo(manager);
        manager->displayInfo(manager);
    }
    destroyMonitorManager(manager);
    loggerClose();
    return 0;
}


/*
int main(){
    systemManager *SystemManager = createSystemManager();
    // Main loop
    while (1) {
        SystemManager->update(SystemManager);
        SystemManager->display(SystemManager);
        sleep(1); // Sleep for a second before the next update
    }

    // Cleanup
    SystemManager->destroy(SystemManager);
    return 0;
}
*/

/*
int main(){
    NetworkManager *NetworkManager = createNetworkManager();
    // Main loop
    while (1) {
        NetworkManager->update(NetworkManager);
        NetworkManager->display(NetworkManager);
        sleep(1); // Sleep for a second before the next update
    }

    // Cleanup
    NetworkManager->destroy(NetworkManager);
    return 0;
}
*/

/*
int main() {
    storageManager *StorageManager = createStorageManager();
    if (StorageManager == NULL) {
        return 1;
    }
    while (1) {
        StorageManager->update(StorageManager);
        StorageManager->display(StorageManager);
        sleep(1);
    }
    StorageManager->destroy(StorageManager);
    return 0;
}
*/

/*
int main() {
    memoryManager *MemoryManager = createMemoryManager();
    if (MemoryManager == NULL) {
        return 1;
    }
    while (1) {
        MemoryManager->update(MemoryManager);
        MemoryManager->display(MemoryManager);
        sleep(1);
    }
    MemoryManager->destroy(MemoryManager);
    return 0;
}*/

/*
int main() {
    loggerInit(LOG_INFO, "log_file.txt");
    cpuManager *CpuManager = createCpuManager();
    if (CpuManager == NULL) {
        return 1;
    }
    while (1) {
        CpuManager->update(CpuManager);
        CpuManager->display(CpuManager);
        sleep(1);
    }
    CpuManager->destroy(CpuManager);
    return 0;
}
*/


