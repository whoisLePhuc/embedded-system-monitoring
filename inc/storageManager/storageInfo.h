#ifndef STORAGE_INFO_H
#define STORAGE_INFO_H

#define MAX_PARTITIONS 4
#define MAX_DISKS 4

typedef struct {
    char partitionName[256];
    long long totalBlocks;
    long long freeBlocks;
    long long usedBlocks;
    long long totalSize;  //in MB
    long long freeSize;//in MB
    long long usedSize;//in MB
} PartitionInfo;

typedef struct {
    char name[128];
    float read_speed_mbps;
    float write_speed_mbps;
    unsigned long long reads_per_second;
    unsigned long long writes_per_second;
} DiskIOInfo;

typedef struct {
    char name[128];
    unsigned long long reads_completed_pre;
    unsigned long long sectors_read_pre;
    unsigned long long writes_completed_pre;
    unsigned long long sectors_written_pre;
} DiskStatsSnapshot;

void getPartitionInfo(PartitionInfo partitions[], int *count);
void getDiskStats(DiskIOInfo io_info[], int *count);

#endif // STORAGE_INFO_H
