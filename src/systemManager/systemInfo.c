#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/utsname.h>
#include "systemManager/systemInfo.h"

void getSystemInfo(SystemInfo *info) {
    FILE *uptime_fp = fopen("/proc/uptime", "r");
    if (uptime_fp) {
        fscanf(uptime_fp, "%lf %lf", &info->uptime_seconds, &info->idle_seconds);
        fclose(uptime_fp);
    } else {
        perror("Error opening /proc/uptime");
        info->uptime_seconds = -1.0;
        info->idle_seconds = -1.0;
    }

    FILE *load_fp = fopen("/proc/loadavg", "r");
    if (load_fp) {
        fscanf(load_fp, "%lf %lf %lf", &info->load_avg[0], &info->load_avg[1], &info->load_avg[2]);
        fclose(load_fp);
    } else {
        perror("Error opening /proc/loadavg");
        info->load_avg[0] = -1.0;
        info->load_avg[1] = -1.0;
        info->load_avg[2] = -1.0;
    }
}

void getSystemDetails(SystemDetails *details) {
    // Get system time
    time_t raw_time;
    struct tm *info;
    time(&raw_time);
    info = localtime(&raw_time);
    strftime(details->system_time, sizeof(details->system_time), "%Y-%m-%d %H:%M:%S", info);

    // Get kernel version
    FILE *fp = fopen("/proc/version", "r");
    if (fp) {
        fgets(details->kernel_version, sizeof(details->kernel_version), fp);
        fclose(fp);
        // Remove trailing newline
        details->kernel_version[strcspn(details->kernel_version, "\n")] = 0;
    } else {
        perror("Error opening /proc/version");
        strncpy(details->kernel_version, "N/A", sizeof(details->kernel_version));
    }
}

void getActiveServices(ServiceInfo services[], int *count) {
    FILE *fp;
    char line[512];
    *count = 0;

    // Use popen to run the systemctl command and read its output
    fp = popen("systemctl list-units --type=service --state=active --no-pager", "r");
    if (fp == NULL) {
        perror("Failed to run systemctl command");
        return;
    }

    // Skip header lines
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp)) {
        if (*count >= MAX_ACTIVE_SERVICES) break;
        char name[128], description[256], unit_file[128];
        if (sscanf(line, "%s %s active running %255[^\n]", name, unit_file, description) == 3) {
            strncpy(services[*count].name, name, sizeof(services[*count].name));
            strncpy(services[*count].description, description, sizeof(services[*count].description));
            (*count)++;
        }
    }
    pclose(fp);
}