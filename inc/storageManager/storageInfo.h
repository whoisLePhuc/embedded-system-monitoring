#ifndef STORAGE_INFO_H
#define STORAGE_INFO_H

#define MAX_NAME_LEN 64

typedef struct {
    char mount_point[MAX_NAME_LEN];
    float used_space;     // GB
    float free_space;     // GB
    float read_speed;     // MB/s
    float write_speed;    // MB/s
    unsigned long long io_read_ops;  // Số I/O đọc
    unsigned long long io_write_ops; // Số I/O ghi
    float iops;           // Input/Output Operations Per Second
} StoragePartition;

typedef struct {
    StoragePartition partitions[8];
    int partition_count;
} StorageInfo;

#endif // STORAGE_INFO_H
