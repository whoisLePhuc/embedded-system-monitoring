#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "cpuManager/cpuManager.h"
#include "logger/logger.h"

typedef struct {
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
} CpuStatus;

typedef struct {
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
} cpuCoreStatus;

typedef struct {
    pid_t pid;
    unsigned long long process_time;
} ProcessTimeSnapshot;

// Function to get CPU usage information
int readCpuStatus(CpuStatus *stats) {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        logMessage(LOG_ERROR, "Error opening /proc/stat");
        return -1;
    }
    int items_read = fscanf(fp, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
                        &stats->user, &stats->nice, &stats->system, &stats->idle,
                        &stats->iowait, &stats->irq, &stats->softirq, &stats->steal);
    if (items_read != 8) {
        logMessage(LOG_ERROR, "Could not read all CPU stats from /proc/stat");
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return 0;
}

float getTotalCpuUsage() {
    CpuStatus stats1, stats2;
    unsigned long long prev_idle, prev_total, curr_idle, curr_total;
    float cpu_usage;
    if (readCpuStatus(&stats1) == -1) {
        logMessage(LOG_ERROR, "Failed to read CPU status for usage calculation");
        return -1.0;
    } 
    prev_idle = stats1.idle + stats1.iowait;
    prev_total = stats1.user + stats1.nice + stats1.system + prev_idle +
                     stats1.irq + stats1.softirq + stats1.steal;
    sleep(1);
    if (readCpuStatus(&stats2) == -1) {
        logMessage(LOG_ERROR, "Failed to read CPU status for usage calculation");
        return -1.0;
    }
    curr_idle = stats2.idle + stats2.iowait;
    curr_total = stats2.user + stats2.nice + stats2.system + curr_idle +
                     stats2.irq + stats2.softirq + stats2.steal;

    if (curr_total - prev_total == 0) {
        logMessage(LOG_WARNING, "No change in total CPU time detected");
        return 0.0;
    }
    cpu_usage = (float)(curr_total - prev_total - (curr_idle - prev_idle)) /
                (curr_total - prev_total) * 100.0;
    return cpu_usage;
}

// ========= Function to get CPU core usage information =========
// Reads CPU core status from /proc/stat and fills the stats array
int readCoreStatus(cpuCoreStatus stats[], int *core_count) {
    FILE *fp;
    char buffer[256];
    int count = 0;

    fp = fopen("/proc/stat", "r");
    if (!fp) {
        logMessage(LOG_ERROR, "Error opening /proc/stat");
        return -1;
    }
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        logMessage(LOG_ERROR, "Could not read from /proc/stat");
        return -1;
    }
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (strncmp(buffer, "cpu", 3) == 0 && buffer[3] >= '0' && buffer[3] <= '9') {
            sscanf(buffer, "cpu%*d %llu %llu %llu %llu %llu %llu %llu %llu",
                    &stats[count].user, &stats[count].nice, &stats[count].system,
                    &stats[count].idle, &stats[count].iowait, &stats[count].irq,
                    &stats[count].softirq, &stats[count].steal);
            count++;
            if (count >= MAX_CORES) break;
        }
    }
    fclose(fp);
    *core_count = count;
    return 0;
}

// Gets per-core CPU usage as an array of floats
float* get_per_core_cpu_usage(int *core_count_out) {
    cpuCoreStatus prev[MAX_CORES], curr[MAX_CORES];
    int cores_prev = 0, cores_curr = 0;
    if (readCoreStatus(prev, &cores_prev) == -1) {
        logMessage(LOG_ERROR, "Failed to read initial core status");
        return NULL;
    }
    sleep(1);
    if (readCoreStatus(curr, &cores_curr) == -1) {
        logMessage(LOG_ERROR, "Failed to read current core status");
        return NULL;
    }    
    if (cores_prev != cores_curr) {
        logMessage(LOG_ERROR, "Core count changed between samples!");
        return NULL;
    }
    float* usage_array = (float*)malloc(cores_curr * sizeof(float));
    if (!usage_array) {
        logMessage(LOG_ERROR, "Failed to allocate memory for core usage array");
        return NULL;
    }
    for (int i = 0; i < cores_curr; i++) {
        unsigned long long prev_idle = prev[i].idle + prev[i].iowait;
        unsigned long long curr_idle = curr[i].idle + curr[i].iowait;
        unsigned long long prev_total = prev[i].user + prev[i].nice + prev[i].system +
                                        prev_idle + prev[i].irq + prev[i].softirq + prev[i].steal;
        unsigned long long curr_total = curr[i].user + curr[i].nice + curr[i].system +
                                        curr_idle + curr[i].irq + curr[i].softirq + curr[i].steal;
        unsigned long long total_diff = curr_total - prev_total;
        unsigned long long idle_diff  = curr_idle - prev_idle;
        if (total_diff != 0) {
            usage_array[i] = (float)(total_diff - idle_diff) / total_diff * 100.0f;
        } else {
            usage_array[i] = 0.0f;
        }
    }
    *core_count_out = cores_curr;
    return usage_array;
}

// Get each of CPU core frequency information from sysfs
float* get_cpu_frequency(int *core_count_out) {
    DIR *cpu_dir;
    struct dirent *entry;
    int core_count = 0;
    float *freq_array = malloc(MAX_CORES * sizeof(float));
    if (!freq_array) {
        logMessage(LOG_ERROR, "Failed to allocate memory for frequency array");
        *core_count_out = 0;
        return NULL;
    }
    cpu_dir = opendir("/sys/devices/system/cpu/");
    if (!cpu_dir) {
        logMessage(LOG_ERROR, "Cannot open /sys/devices/system/cpu/");
        free(freq_array);
        *core_count_out = 0;        
        return NULL;
    }
    while ((entry = readdir(cpu_dir)) != NULL) {
        if (strncmp(entry->d_name, "cpu", 3) == 0 && isdigit(entry->d_name[3])) {
            // Lấy số core từ tên thư mục
            int cpu_id = atoi(entry->d_name + 3);
            char path[256];
            snprintf(path, sizeof(path),
                     "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", cpu_id);
            FILE *fp = fopen(path, "r");
            if (!fp) {
                logMessage(LOG_DEBUG, "Cannot open frequency file for %s", entry->d_name);
                continue;
            }
            unsigned int freq_khz = 0;
            if (fscanf(fp, "%u", &freq_khz) == 1) {
                if (core_count < MAX_CORES) {
                    freq_array[core_count++] = (float)freq_khz / 1000.0f; // MHz
                }
            } else {
                logMessage(LOG_DEBUG, "Cannot read frequency for %s", entry->d_name);
            }
            fclose(fp);
        }
    }
    closedir(cpu_dir);
    if (core_count == 0) {
        free(freq_array);
        *core_count_out = 0;
        return NULL;
    }
    float *tmp = realloc(freq_array, core_count * sizeof(float));
    if (tmp) freq_array = tmp;
    *core_count_out = core_count;
    return freq_array;
}

// ========= Function to get CPU temperature information =========
float get_cpu_temperature() {
    FILE *fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (!fp) {
        logMessage(LOG_ERROR, "Error opening /sys/class/thermal/thermal_zone0/temp");
        return -1.0;
    }
    int temp_raw;
    fscanf(fp, "%d", &temp_raw);
    fclose(fp);
    return (float)temp_raw / 1000.0;
}

// ========= Function to get top 5 CPU process =========
int compare_processes(const void *a, const void *b) {
    cpuProcessInfo *p1 = (cpuProcessInfo *)a;
    cpuProcessInfo *p2 = (cpuProcessInfo *)b;
    if (p1->cpu_usage < p2->cpu_usage) {
        logMessage(LOG_DEBUG, "Comparing processes: %s (%.2f%%) < %s (%.2f%%)", p1->name, p1->cpu_usage, p2->name, p2->cpu_usage);
        return 1;
    }
    if (p1->cpu_usage > p2->cpu_usage) {
        logMessage(LOG_DEBUG, "Comparing processes: %s (%.2f%%) > %s (%.2f%%)", p1->name, p1->cpu_usage, p2->name, p2->cpu_usage);
        return -1;
    }
    return 0;
}

// Helper function to get total CPU time from /proc/stat
unsigned long long get_total_cpu_time() {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        logMessage(LOG_ERROR, "Error opening /proc/stat");
        return 0;
    }
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal, total_time = 0;
    fscanf(fp, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
    fclose(fp);

    total_time = user + nice + system + idle + iowait + irq + softirq + steal;
    return total_time;
}

static int read_proc_times(int pid, unsigned long long *utime_out, unsigned long long *stime_out) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    FILE *fp = fopen(path, "r");
    if (!fp) return -1;

    // Bỏ qua pid, bỏ qua (comm) an toàn với khoảng trắng: %*[^)] đọc đến ')', %*c ăn dấu ')'
    unsigned long long ut=0, st=0;
    int scanned = fscanf(fp,
        "%*d "          // pid
        "%*[^)]%*c "    // comm trong ngoặc ) và ăn dấu ')'
        "%*c "          // state
        "%*d %*d %*d %*d %*d "  // ppid, pgrp, session, tty_nr, tpgid
        "%*u %*u %*u %*u %*u "  // flags, minflt, cminflt, majflt, cmajflt
        "%llu %llu",    // utime, stime (fields 14,15)
        &ut, &st);
    fclose(fp);
    if (scanned != 2) return -2;

    *utime_out = ut;
    *stime_out = st;
    return 0;
}

cpuProcessInfo* get_top_cpu_processes(int* process_count_out) {
    if (process_count_out) *process_count_out = 0;

    DIR *proc_dir = opendir("/proc");
    if (!proc_dir) {
        logMessage(LOG_ERROR, "Error opening /proc");
        return NULL;
    }
    // --- Lấy snapshot 1 ---
    unsigned long long total_time1 = get_total_cpu_time();
    ProcessTimeSnapshot *snapshots1 = NULL;
    int count1 = 0;

    struct dirent *entry;
    while ((entry = readdir(proc_dir)) != NULL) {
        if (!isdigit((unsigned char)entry->d_name[0])) continue; // chỉ nhận mục có tên bắt đầu bằng số (PID)
        int pid = atoi(entry->d_name);
        unsigned long long ut=0, st=0;
        if (read_proc_times(pid, &ut, &st) == 0) {
            // safe-realloc pattern
            void *tmp = realloc(snapshots1, (count1 + 1) * sizeof(*snapshots1));
            if (!tmp) {
                logMessage(LOG_ERROR, "realloc snapshots1 failed");
                free(snapshots1);
                closedir(proc_dir);
                return NULL;
            }
            snapshots1 = (ProcessTimeSnapshot*)tmp;
            snapshots1[count1].pid = pid;
            snapshots1[count1].process_time = ut + st;
            count1++;
        }
    }
    closedir(proc_dir);
    sleep(1);
    unsigned long long total_time2 = get_total_cpu_time();
    if (total_time2 < total_time1) {
        total_time1 = total_time2;
    }
    unsigned long long total_time_delta = total_time2 - total_time1;
    cpuProcessInfo *final_results = NULL;
    int final_count = 0;
    for (int i = 0; i < count1; i++) {
        int pid = snapshots1[i].pid;
        unsigned long long ut=0, st=0;
        if (read_proc_times(pid, &ut, &st) != 0) {
            // Process có thể đã thoát; bỏ qua
            continue;
        }
        unsigned long long process_time2 = ut + st;
        unsigned long long process_time_delta =
            (process_time2 >= snapshots1[i].process_time)
            ? (process_time2 - snapshots1[i].process_time)
            : 0ULL; // đề phòng counter lùi/overflow
        float cpu_usage = 0.0f;
        if (total_time_delta > 0ULL) {
            cpu_usage = 100.0f * (float)process_time_delta / (float)total_time_delta;
        }
        void *tmp = realloc(final_results, (final_count + 1) * sizeof(*final_results));
        if (!tmp) {
            logMessage(LOG_ERROR, "realloc final_results failed");
            free(snapshots1);
            free(final_results);
            return NULL;
        }
        final_results = (cpuProcessInfo*)tmp;
        cpuProcessInfo *cur = &final_results[final_count++];
        cur->pid = pid;
        cur->cpu_usage = cpu_usage;
        cur->name[0] = '\0';
        // Đọc tên tiến trình từ /proc/<pid>/comm
        char path[256];
        snprintf(path, sizeof(path), "/proc/%d/comm", pid);
        FILE *fp = fopen(path, "r");
        if (fp) {
            if (fgets(cur->name, (int)sizeof(cur->name), fp)) {
                cur->name[strcspn(cur->name, "\n")] = '\0';
            } else {
                cur->name[0] = '\0';
            }
            fclose(fp);
        }
    }
    free(snapshots1);
    if (final_count > 0) {
        qsort(final_results, final_count, sizeof(cpuProcessInfo), compare_processes);
    }
    if (process_count_out) *process_count_out = final_count;
    return final_results; // caller chịu trách nhiệm free()
}

void update_top_processes(cpuInfo* cpu_data) {
    int process_count;
    cpuProcessInfo* processes = get_top_cpu_processes(&process_count);
    if (processes != NULL) {
        int count_to_copy = (process_count < MAX_TOP_PROC) ? process_count : MAX_TOP_PROC;
        for (int i = 0; i < count_to_copy; i++) {
            cpu_data->topProcesses[i] = processes[i];
        }
        for (int i = count_to_copy; i < MAX_TOP_PROC; i++) {
            memset(&cpu_data->topProcesses[i], 0, sizeof(cpuProcessInfo));
        }
        free(processes);
    } else {
        logMessage(LOG_ERROR, "Error: Failed to get top processes.");
        memset(cpu_data->topProcesses, 0, sizeof(cpu_data->topProcesses));
    }
}