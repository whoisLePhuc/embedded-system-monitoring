#ifndef CPU_MANAGER_H
#define CPU_MANAGER_H

#include "cpuInfo.h"

typedef struct cpuManager cpuManager;

struct cpuManager{
    cpuInfo CpuInfo;
    void (*update)(cpuManager *self); 
    void (*destroy)(cpuManager *self);
    void (*display)(cpuManager *self);
};

// Funtion to get CPU total usage 
cpuManager *createCpuManager(); // Create a cpuManager object
void destroyCpuManager(cpuManager *self); // Free memory of cpuManager object
void updateCpuInfo(cpuManager *self); // Update CPU information
void displayCpuInfo(cpuManager *self); // Display CPU information

#endif // CPU_MANAGER_H