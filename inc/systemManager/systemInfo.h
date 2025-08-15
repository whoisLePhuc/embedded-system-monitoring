#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#define MAX_NAME_LEN 64

typedef struct {
    unsigned long long uptime_seconds;
    unsigned long long system_time;  // timestamp
    char kernel_version[MAX_NAME_LEN];
    char os_version[MAX_NAME_LEN];
    unsigned long long running_processes;
    float load_avg[3]; // 1, 5, 15 phút
} SystemInfo;

#endif // SYSTEM_INFO_H
