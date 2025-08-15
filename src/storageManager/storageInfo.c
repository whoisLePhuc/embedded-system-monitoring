#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/statfs.h>
#include <string.h>
#include "storageManager/storageInfo.h"

void getPartitionInfo(PartitionInfo partitions[], int *count) {
    FILE *fp;
    char line[256];
    *count = 0;

    // Đọc file /proc/mounts để lấy danh sách các phân vùng
    fp = fopen("/proc/mounts", "r");
    if (!fp) {
        perror("Error opening /proc/mounts");
        return;
    }

    while (fgets(line, sizeof(line), fp)) {
        char dev[256], mount_point[256], fs_type[256];
        struct statfs fs_stats;
        sscanf(line, "%s %s %s", dev, mount_point, fs_type);

        if (*count >= MAX_PARTITIONS) break;

        // Bỏ qua các phân vùng ảo
        if (strncmp(dev, "/dev/", 5) == 0) {
            if (statfs(mount_point, &fs_stats) == 0) {
                partitions[*count].totalBlocks = fs_stats.f_blocks;
                partitions[*count].freeBlocks = fs_stats.f_bfree;
                partitions[*count].usedBlocks = fs_stats.f_blocks - fs_stats.f_bfree;

                long long block_size = fs_stats.f_bsize;
                partitions[*count].totalSize = (partitions[*count].totalBlocks * block_size) / (1024 * 1024);
                partitions[*count].freeSize = (partitions[*count].freeBlocks * block_size) / (1024 * 1024);
                partitions[*count].usedSize = (partitions[*count].usedBlocks * block_size) / (1024 * 1024);
                
                strncpy(partitions[*count].partitionName, mount_point, sizeof(partitions[*count].partitionName));
                (*count)++;
            }
        }
    }
    fclose(fp);
}

void getDiskStats(DiskIOInfo io_info[], int *count) {
    FILE *fp;
    char line[256];
    DiskStatsSnapshot snapshots[MAX_DISKS];
    *count = 0;
    fp = fopen("/proc/diskstats", "r");
    if (!fp) {
        perror("Error opening /proc/diskstats");
        return;
    }
    while (fgets(line, sizeof(line), fp)) {
        if (*count >= MAX_DISKS) break;
        unsigned long long discard, reads_completed, sectors_read, writes_completed, sectors_written;
        char device[128];
        sscanf(line, "%*u %*u %s %llu %llu %llu %llu %llu %llu",
               device, &reads_completed, &discard, &sectors_read, &writes_completed, &discard, &sectors_written);
        snapshots[*count].reads_completed_pre = reads_completed;
        snapshots[*count].sectors_read_pre = sectors_read;
        snapshots[*count].writes_completed_pre = writes_completed;
        snapshots[*count].sectors_written_pre = sectors_written;
        strncpy(snapshots[*count].name, device, sizeof(snapshots[*count].name));
        (*count)++;
    }
    fclose(fp);
    sleep(1);
    fp = fopen("/proc/diskstats", "r");
    if (!fp) {
        perror("Error opening /proc/diskstats");
        return;
    }
    for (int i = 0; i < *count; i++) {
        unsigned long long reads_completed, sectors_read, writes_completed, sectors_written, discard;
        char device[128];
        
        while (fgets(line, sizeof(line), fp)) {
            sscanf(line, "%*u %*u %s %llu %llu %llu %llu %llu %llu",
                   device, &reads_completed, &discard, &sectors_read, &writes_completed, &discard, &sectors_written);
            if (strcmp(snapshots[i].name, device) == 0) {
                float read_delta_sectors = (float)(sectors_read - snapshots[i].sectors_read_pre);
                float write_delta_sectors = (float)(sectors_written - snapshots[i].sectors_written_pre);
                io_info[i].read_speed_mbps = read_delta_sectors * 512 / (1024 * 1024);
                io_info[i].write_speed_mbps = write_delta_sectors * 512 / (1024 * 1024);

                unsigned long long read_delta_count = reads_completed - snapshots[i].reads_completed_pre;
                unsigned long long write_delta_count = writes_completed - snapshots[i].writes_completed_pre;
                io_info[i].reads_per_second = read_delta_count;
                io_info[i].writes_per_second = write_delta_count;
                
                strncpy(io_info[i].name, snapshots[i].name, sizeof(io_info[i].name));
                
                break;
            }
        }
        rewind(fp);
    }
    fclose(fp);
}