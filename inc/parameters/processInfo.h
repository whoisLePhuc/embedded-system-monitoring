#ifndef PROCESS_INFO_H
#define PROCESS_INFO_H

#define MAX_NAME_LEN 64

typedef struct {
    char name[MAX_NAME_LEN]; // Tên tiến trình
    float cpu_usage;         // %
    float ram_usage;         // MB
} ProcessInfo;

#endif // PROCESS_INFO_H
