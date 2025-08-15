#ifndef CPU_MANAGER_H
#define CPU_MANAGER_H

#include "cpuInfo.h"

typedef struct cpuManager cpuManager;

struct cpuManager{
    cpuInfo CpuInfo; // CPU informations
    void (*update)(cpuManager *self); // Update CPU information
    void (*destroy)(cpuManager *self); // Free memory of cpuManager object
    void (*display)(cpuManager *self); // Display CPU information
};

// Funtion to get CPU total usage 
cpuManager *createCpuManager(); // Create a cpuManager object
void destroyCpuManager(cpuManager *self); // Free memory of cpuManager object
void updateCpuInfo(cpuManager *self); // Update CPU information

#endif // CPU_MANAGER_H