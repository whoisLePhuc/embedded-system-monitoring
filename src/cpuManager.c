#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include "manager/cpuManager.h"

// Forward declaration
float getTotalCpuUsage(); 
float* get_per_core_cpu_usage(int *core_count_out);
float* get_cpu_frequency(int *core_count_out);
void update_per_core_usage(cpuInfo* cpu_data);
void update_cpu_frequencies(cpuInfo* cpu_data);
float get_cpu_temperature();
void update_top_processes(cpuInfo* cpu_data);

// Function to create a cpuManager object
cpuManager *createCpuManager(){
    cpuManager *manager = (cpuManager *)malloc(sizeof(cpuManager));
    if(manager == NULL) {
        fprintf(stderr, "Memory allocation failed for cpuManager\n");
        return NULL;
    }
    memset(manager, 0, sizeof(cpuManager));
    manager->update = updateCpuInfo;
    return manager;
}

// Function to destroy a cpuManager object
void destroyCpuManager(cpuManager *self){
    if(self != NULL) {
        free(self);
        self = NULL;
    } else {
        fprintf(stderr, "Attempted to free a NULL cpuManager pointer\n");
    }
}

// Function to update CPU information
void updateCpuInfo(cpuManager *self){
    if(self == NULL) {
        fprintf(stderr, "cpuManager pointer is NULL\n");
        return;
    }
    
    // Get CPU total usage
    self->cpu_info.total_usage = getTotalCpuUsage();
    if(self->cpu_info.total_usage < 0) {
        fprintf(stderr, "Failed to get CPU total usage\n");
    }

    // Get per-core CPU usage
    update_per_core_usage(&self->cpu_info);

    // Get CPU core frequency  
    update_cpu_frequencies(&self->cpu_info);

    // Get CPU temperature
    self->cpu_info.temperature = get_cpu_temperature();
    if(self->cpu_info.temperature < 0) {
        fprintf(stderr, "Failed to get CPU temperature\n");
    }

    // Get top processes
    update_top_processes(&self->cpu_info);     
}

// ========= Function to get CPU usage information =========
typedef struct {
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
} CpuStatus;

int readCpuStatus(CpuStatus *stats) {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        return -1; 
    }
    int items_read = fscanf(fp, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
                        &stats->user, &stats->nice, &stats->system, &stats->idle,
                        &stats->iowait, &stats->irq, &stats->softirq, &stats->steal);
    if (items_read != 8) {
        // Xử lý lỗi, ví dụ: in ra thông báo lỗi hoặc return -1;
        fprintf(stderr, "Error: Could not read all CPU stats from /proc/stat\n");
    }
    fclose(fp);
    return 0;
}

// Hàm tính phần trăm sử dụng CPU
float getTotalCpuUsage() {
    CpuStatus stats1, stats2;
    unsigned long long prev_idle, prev_total, curr_idle, curr_total;
    float cpu_usage;

    if (readCpuStatus(&stats1) == -1) return -1.0;
    prev_idle = stats1.idle + stats1.iowait;
    prev_total = stats1.user + stats1.nice + stats1.system + prev_idle +
                 stats1.irq + stats1.softirq + stats1.steal;
    sleep(1); // Chờ 1 giây
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
typedef struct {
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
} cpuCoreStatus;

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
        // Xử lý lỗi hoặc trường hợp file rỗng
        fprintf(stderr, "Error: Could not read from /proc/stat\n");
        return -1;
    }
// Nếu không có lỗi, tiếp tục xử lý buffer
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (strncmp(buffer, "cpu", 3) == 0 && buffer[3] >= '0' && buffer[3] <= '9') {
            sscanf(buffer, "cpu%*d %llu %llu %llu %llu %llu %llu %llu %llu",
                   &stats[count].user, &stats[count].nice, &stats[count].system,
                   &stats[count].idle, &stats[count].iowait, &stats[count].irq,
                   &stats[count].softirq, &stats[count].steal);
            count++;
            if (count >= MAX_CORES) break; // tránh tràn mảng
        }
    }
    fclose(fp);
    *core_count = count;
    return 0;
}

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

void update_per_core_usage(cpuInfo* cpu_data) {
    int core_count;

    float* usage_array = get_per_core_cpu_usage(&core_count);
    
    if (usage_array != NULL) {
        for (int i = 0; i < core_count; i++) {
            cpu_data->core_usage[i] = usage_array[i];
        }
        free(usage_array);
        // Lưu ý: nên có một cách thống nhất để lưu core_count
    } else {
        fprintf(stderr, "Failed to get per-core CPU usage\n");
    }
}
// ========= Function to get CPU core frequency =========
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
            // Không tìm thấy core tiếp theo, kết thúc vòng lặp
            break;
        }

        unsigned int freq_khz;
        fscanf(fp, "%u", &freq_khz);
        fclose(fp);
        
        freq_array[core_count] = (float)freq_khz / 1000.0; // Lưu tần số dưới dạng MHz
        core_count++;
    }

    *core_count_out = core_count;
    // Cắt bớt mảng nếu số lượng core thực tế ít hơn MAX_CORES
    if (core_count < MAX_CORES) {
        freq_array = (float*)realloc(freq_array, core_count * sizeof(float));
    }

    return freq_array;
}

void update_cpu_frequencies(cpuInfo* cpu_data) {
    int core_count;
    
    float* frequencies = get_cpu_frequency(&core_count);
    
    if (frequencies != NULL) {
        for (int i = 0; i < core_count; i++) {
            cpu_data->frequency[i] = frequencies[i];
        }
        cpu_data->core_count = core_count;
        free(frequencies);
    } else {
        fprintf(stderr, "Failed to get CPU core frequencies\n");
        // Nếu thất bại, có thể đặt core_count về 0
        cpu_data->core_count = 0;
    }
}

// ========= Function to get CPU temperature =========
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
// ========= Function to get top process =========
// Cấu trúc để lưu thông tin cuối cùng của tiến trình
typedef struct {
    pid_t pid;
    char name[256];
    float cpu_usage;
} cpuProcessInfo;

// Cấu trúc tạm thời để lưu thời gian của tiến trình tại một thời điểm
typedef struct {
    pid_t pid;
    unsigned long long process_time; // Tổng utime + stime
} ProcessTimeSnapshot;

// Hàm so sánh cho qsort để sắp xếp theo mức sử dụng CPU giảm dần
int compare_processes(const void *a, const void *b) {
    cpuProcessInfo *p1 = (cpuProcessInfo *)a;
    cpuProcessInfo *p2 = (cpuProcessInfo *)b;
    if (p1->cpu_usage < p2->cpu_usage) return 1;
    if (p1->cpu_usage > p2->cpu_usage) return -1;
    return 0;
}

// Hàm đọc tổng thời gian CPU của hệ thống từ /proc/stat
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

// Hàm chính để lấy danh sách các tiến trình và mức sử dụng CPU
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

    // Duyệt qua thư mục /proc
    while ((entry = readdir(proc_dir)) != NULL) {
        // Nếu tên thư mục là một số (PID)
        if (isdigit(entry->d_name[0])) {
            char path[256];
            snprintf(path, sizeof(path), "/proc/%s/stat", entry->d_name);
            
            FILE *fp = fopen(path, "r");
            if (fp) {
                unsigned long long utime, stime;
                // Bỏ qua 13 trường đầu tiên để lấy utime (14) và stime (15)
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

    // Duyệt qua snapshot 1 để tính toán cho từng tiến trình
    for (int i = 0; i < count1; i++) {
        char path[256];
        snprintf(path, sizeof(path), "/proc/%d/stat", snapshots1[i].pid);

        FILE *fp = fopen(path, "r");
        if (fp) { // Nếu tiến trình vẫn còn tồn tại
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

            // Lấy tên tiến trình
            snprintf(path, sizeof(path), "/proc/%d/comm", snapshots1[i].pid);
            fp = fopen(path, "r");
            if (fp) {
                fgets(current_result->name, sizeof(current_result->name), fp);
                current_result->name[strcspn(current_result->name, "\n")] = 0; // Xóa ký tự newline
                fclose(fp);
            }
        }
    }
    free(snapshots1); // Giải phóng bộ nhớ của snapshot 1

    // Sắp xếp danh sách kết quả
    qsort(final_results, final_count, sizeof(cpuProcessInfo), compare_processes);
    
    *process_count_out = final_count;
    return final_results;
}
// Function to update top processes in cpuInfo
void update_top_processes(cpuInfo* cpu_data) {
    int process_count;
    
    // 1 & 2. Gọi hàm và lưu con trỏ
    cpuProcessInfo* processes = get_top_cpu_processes(&process_count);

    // 3. Kiểm tra và sao chép dữ liệu
    if (processes != NULL) {
        // Xác định số lượng tiến trình cần sao chép, không vượt quá MAX_TOP_PROCCESS
        int count_to_copy = (process_count < MAX_TOP_PROCCESS) ? process_count : MAX_TOP_PROCCESS;

        // Sao chép dữ liệu từ mảng động vào struct
        for (int i = 0; i < count_to_copy; i++) {
            memcpy(&cpu_data->top_processes[i], &processes[i], sizeof(cpuProcessInfo));
        }

        // Điền các phần tử còn lại bằng 0 nếu cần (tùy chọn)
        for (int i = count_to_copy; i < MAX_TOP_PROCCESS; i++) {
            memset(&cpu_data->top_processes[i], 0, sizeof(cpuProcessInfo));
        }

        // 4. Giải phóng bộ nhớ đã cấp phát
        free(processes);
    } else {
        fprintf(stderr, "Error: Failed to get top processes.\n");
        // Có thể điền toàn bộ struct bằng 0 để tránh dữ liệu rác
        memset(cpu_data->top_processes, 0, sizeof(cpu_data->top_processes));
    }
}