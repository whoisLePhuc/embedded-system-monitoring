#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include "storageInfo.h"

typedef struct storageManager storageManager;
struct storageManager {
    PartitionInfo memory_info[MAX_PARTITIONS]; // Array of partition information
    DiskIOInfo disk_io_info[MAX_DISKS]; // Array of disk I/O information
    void (*update)(storageManager *self); 
    void (*destroy)(storageManager *self);
    void (*display)(storageManager *self);
};

storageManager *createStorageManager(); // Create a storageManageryManager object
void destroyStorageManager(storageManager *self); // Free memory of storageManager object
void updateStorageInfo(storageManager *self); // Update storageManager information
void displayStorageInfo(storageManager *self); // Display storageManager information

#endif // STORAGE_MANAGER_H