#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cpuManager/cpuManager.h"
#include "memoryManager/memoryManager.h"
#include "storageManager/storageManager.h"
#include "networkManager/networkManager.h"

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
