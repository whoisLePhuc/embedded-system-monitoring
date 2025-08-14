#ifndef CPU_MANAGER_H
#define CPU_MANAGER_H

#include "parameters/CPUInfo.h"

typedef struct cpuManager cpuManager;

struct cpuManager{
    cpuInfo cpu_info; // CPU informations
    void (*update)(cpuManager *self); // Update CPU information
};

// Funtion to get CPU total usage 
cpuManager *createCpuManager(); // Create a cpuManager object
void destroyCpuManager(cpuManager *self); // Free memory of cpuManager object
void updateCpuInfo(cpuManager *self); // Update CPU information
void printCpuInfo(cpuManager *self); // Print CPU information

#endif // CPU_MANAGER_H