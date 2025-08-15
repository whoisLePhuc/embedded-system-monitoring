#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#define MAX_SYSTEM_TIME_LEN 30
#define MAX_KERNEL_VERSION_LEN 256
#define MAX_SERVICE_NAME_LEN 128
#define MAX_SERVICE_DESC_LEN 256
#define MAX_ACTIVE_SERVICES 20

typedef struct {
    double uptime_seconds; // system uptime
    double idle_seconds; // idle time
    double load_avg[3]; // load average in 1, 5, 15 minutes
} SystemInfo;

typedef struct {
    char system_time[MAX_SYSTEM_TIME_LEN];
    char kernel_version[MAX_KERNEL_VERSION_LEN];
} SystemDetails;

typedef struct {
    char name[MAX_SERVICE_NAME_LEN];
    char description[MAX_SERVICE_DESC_LEN];
} ServiceInfo;

void getSystemInfo(SystemInfo *info);
void getSystemDetails(SystemDetails *details);
void getActiveServices(ServiceInfo services[], int *count);

#endif // SYSTEM_INFO_H
