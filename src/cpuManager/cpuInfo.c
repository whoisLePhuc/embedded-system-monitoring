#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "cpuManager/cpuManager.h"

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


// ========= Function to get CPU usage information =========
int readCpuStatus(CpuStatus *stats) {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        return -1;
    }
    int items_read = fscanf(fp, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
                        &stats->user, &stats->nice, &stats->system, &stats->idle,
                        &stats->iowait, &stats->irq, &stats->softirq, &stats->steal);
    if (items_read != 8) {
        fprintf(stderr, "Error: Could not read all CPU stats from /proc/stat\n");
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

    if (readCpuStatus(&stats1) == -1) return -1.0;
    prev_idle = stats1.idle + stats1.iowait;
    prev_total = stats1.user + stats1.nice + stats1.system + prev_idle +
                     stats1.irq + stats1.softirq + stats1.steal;
    sleep(1);
    if (readCpuStatus(&stats2) == -1) return -1.0;
    curr_idle = stats2.idle + stats2.iowait;
    curr_total = stats2.user + stats2.nice + stats2.system + curr_idle +
                     stats2.irq + stats2.softirq + stats2.steal;

    if (curr_total - prev_total == 0) return 0.0;
    
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
        perror("Error opening /proc/stat");
        return -1;
    }
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        fprintf(stderr, "Error: Could not read from /proc/stat\n");
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
    if (readCoreStatus(prev, &cores_prev) == -1) return NULL;
    sleep(1);
    if (readCoreStatus(curr, &cores_curr) == -1) return NULL;
    if (cores_prev != cores_curr) {
        fprintf(stderr, "Core count changed between samples!\n");
        return NULL;
    }
    float* usage_array = (float*)malloc(cores_curr * sizeof(float));
    if (!usage_array) {
        perror("Failed to allocate memory");
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

// ========= Function to get CPU core usage information =========
// Hàm lấy tần số của từng core
float* get_cpu_frequency(int *core_count_out) {
    FILE *fp;
    char path[128];
    int core_count = 0;
    float* freq_array = (float*)malloc(MAX_CORES * sizeof(float));
    if (!freq_array) {
        perror("Failed to allocate memory");
        *core_count_out = 0;
        return NULL;
    }
    while (core_count < MAX_CORES) {
        snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", core_count);
        fp = fopen(path, "r");
        if (!fp) {
            break;
        }
        unsigned int freq_khz;
        fscanf(fp, "%u", &freq_khz);
        fclose(fp);
        freq_array[core_count] = (float)freq_khz / 1000.0;
        core_count++;
    }
    *core_count_out = core_count;
    if (core_count < MAX_CORES) {
        freq_array = (float*)realloc(freq_array, core_count * sizeof(float));
    }
    return freq_array;
}

// ========= Function to get CPU temperature information =========
float get_cpu_temperature() {
    FILE *fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (!fp) {
        perror("Error opening /sys/class/thermal/thermal_zone0/temp");
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
    if (p1->cpu_usage < p2->cpu_usage) return 1;
    if (p1->cpu_usage > p2->cpu_usage) return -1;
    return 0;
}

unsigned long long get_total_cpu_time() {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) return 0;

    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal, total_time = 0;
    fscanf(fp, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
    fclose(fp);

    total_time = user + nice + system + idle + iowait + irq + softirq + steal;
    return total_time;
}

cpuProcessInfo* get_top_cpu_processes(int* process_count_out) {
    DIR *proc_dir;
    struct dirent *entry;
    
    // --- LẤY ẢNH CHỤP LẦN 1 ---
    unsigned long long total_time1 = get_total_cpu_time();
    
    ProcessTimeSnapshot *snapshots1 = NULL;
    int count1 = 0;

    proc_dir = opendir("/proc");
    if (!proc_dir) {
        perror("Could not open /proc");
        return NULL;
    }

    while ((entry = readdir(proc_dir)) != NULL) {
        if (isdigit(entry->d_name[0])) {
            char path[256];
            snprintf(path, sizeof(path), "/proc/%s/stat", entry->d_name);
            
            FILE *fp = fopen(path, "r");
            if (fp) {
                unsigned long long utime, stime;
                fscanf(fp, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %llu %llu", &utime, &stime);
                fclose(fp);
                
                count1++;
                snapshots1 = realloc(snapshots1, count1 * sizeof(ProcessTimeSnapshot));
                snapshots1[count1 - 1].pid = atoi(entry->d_name);
                snapshots1[count1 - 1].process_time = utime + stime;
            }
        }
    }
    closedir(proc_dir);

    // --- CHỜ 1 GIÂY ---
    sleep(1);

    // --- LẤY ẢNH CHỤP LẦN 2 VÀ TÍNH TOÁN ---
    unsigned long long total_time2 = get_total_cpu_time();
    unsigned long long total_time_delta = total_time2 - total_time1;

    cpuProcessInfo *final_results = NULL;
    int final_count = 0;

    for (int i = 0; i < count1; i++) {
        char path[256];
        snprintf(path, sizeof(path), "/proc/%d/stat", snapshots1[i].pid);

        FILE *fp = fopen(path, "r");
        if (fp) {
            unsigned long long utime, stime;
            fscanf(fp, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %llu %llu", &utime, &stime);
            fclose(fp);

            unsigned long long process_time2 = utime + stime;
            unsigned long long process_time_delta = process_time2 - snapshots1[i].process_time;

            float cpu_usage = 0.0f;
            if (total_time_delta > 0) {
                cpu_usage = 100.0f * (float)process_time_delta / (float)total_time_delta;
            }

            final_count++;
            final_results = realloc(final_results, final_count * sizeof(cpuProcessInfo));
            
            cpuProcessInfo *current_result = &final_results[final_count - 1];
            current_result->pid = snapshots1[i].pid;
            current_result->cpu_usage = cpu_usage;

            snprintf(path, sizeof(path), "/proc/%d/comm", snapshots1[i].pid);
            fp = fopen(path, "r");
            if (fp) {
                fgets(current_result->name, sizeof(current_result->name), fp);
                current_result->name[strcspn(current_result->name, "\n")] = 0;
                fclose(fp);
            }
        }
    }
    free(snapshots1);
    qsort(final_results, final_count, sizeof(cpuProcessInfo), compare_processes);
    *process_count_out = final_count;
    return final_results;
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
        fprintf(stderr, "Error: Failed to get top processes.\n");
        memset(cpu_data->topProcesses, 0, sizeof(cpu_data->topProcesses));
    }
}