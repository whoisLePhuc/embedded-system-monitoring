#ifndef SYSTEM_METRICS_H
#define SYSTEM_METRICS_H

#include "CPUInfo.h"
#include "memoryInfo.h"
#include "storageInfo.h"
#include "networkInfo.h"
#include "systemInfo.h"

// Structures to hold system metrics
typedef struct {
    CPUInfo cpu;
    MemoryInfo memory;
    StorageInfo storage;
    NetworkInfo network;
    SystemInfo system;
} systemMetrics;

// Function prototypes for systemManager
typedef struct systemManager systemManager;
struct systemManager{
    systemMetrics metrics; // Thông tin hệ thống
    void (*update)(systemManager *self); // Cập nhật thông tin hệ thống
    void (*display)(const systemManager *self); // Hiển thị thông tin hệ thống
    void (*reset)(systemManager *self); // Đặt lại thông tin hệ thống
};

systemManager *createSystemManager(systemMetrics *system); // Tạo một đối tượng systemManager
systemMetrics *createSystemMetrics(); // Tạo một đối tượng systemMetrics

static void destroySystemManager(systemManager *self); // Giải phóng bộ nhớ của đối tượng
static void updateSystemMetrics(systemManager *self); // Cập nhật thông tin hệ thống
static void displaySystemMetrics(const systemManager *self); // Hiển thị thông tin hệ thống
static void resetSystemMetrics(systemManager *self); // Đặt lại thông tin hệ thống

#endif // SYSTEM_METRICS_H
