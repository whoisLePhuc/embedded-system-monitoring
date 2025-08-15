#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/statfs.h>
#include "storageManager/storageManager.h"
#include "storageManager/storageInfo.h"

storageManager *createStorageManager() {
    storageManager *manager = (storageManager *)malloc(sizeof(storageManager));
    if (manager == NULL) {
        fprintf(stderr, "Failed to allocate memory for storageManager\n");
        return NULL;
    }
    memset(manager, 0, sizeof(storageManager));
    manager->update = updateStorageInfo;
    manager->destroy = destroyStorageManager;
    manager->display = displayStorageInfo;
    return manager;
}

void destroyStorageManager(storageManager *self) {
    if (self != NULL) {
        free(self);
        self = NULL;
    } else {
        fprintf(stderr, "Attempted to free a NULL storageManager pointer\n");
    }
}

void updateStorageInfo(storageManager *self){
    int count = 0;
    getPartitionInfo(self->memory_info, &count);
    getDiskStats(self->disk_io_info, &count);
}

void displayStorageInfo(storageManager *self) {
    if (self == NULL) {
        fprintf(stderr, "Cannot display info from a NULL manager.\n");
        return;
    }
    PartitionInfo *partitions = self->memory_info;
    DiskIOInfo *io_stats = self->disk_io_info;
    int partition_count = MAX_PARTITIONS;
    int disk_count = MAX_DISKS;
    
    system("clear");
    printf("==================== STORAGE MONITOR ====================\n");
    
    printf("Partition Info:\n");
    for (int i = 0; i < partition_count; i++) {
        // You would need to check if the partition is valid, for example, by checking if the name is not empty
        if (strlen(partitions[i].partitionName) > 0) {
            printf("  Mount Point: %s\n", partitions[i].partitionName);
            printf("    Total Size: %lld MB\n", partitions[i].totalSize);
            printf("    Used Size:  %lld MB\n", partitions[i].usedSize);
            printf("    Free Size:  %lld MB\n", partitions[i].freeSize);
            printf("\n");
        }
    }
    
    printf("---------------------------------------------------------\n");
    printf("Disk I/O Stats:\n");
    for (int i = 0; i < disk_count; i++) {
        if (strlen(io_stats[i].name) > 0) {
            printf("  Device: %s\n", io_stats[i].name);
            printf("    Read Speed:   %.2f MB/s\n", io_stats[i].read_speed_mbps);
            printf("    Write Speed:  %.2f MB/s\n", io_stats[i].write_speed_mbps);
            printf("    Read IOPS:    %llu\n", io_stats[i].reads_per_second);
            printf("    Write IOPS:   %llu\n", io_stats[i].writes_per_second);
            printf("\n");
        }
    }
    printf("=========================================================\n");
}

