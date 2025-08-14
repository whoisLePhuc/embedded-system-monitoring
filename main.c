#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "manager/cpuManager.h"

int main(){
    // Create a CPU manager instance
    cpuManager *manager = createCpuManager();
    if(manager == NULL) {
        fprintf(stderr, "Failed to create CPU manager\n");
        return 1;
    }
    while(1){
        // Update CPU information
        manager->update(manager);
        
        // Display total CPU usage
        printf("Total CPU Usage:      %.2f%%\n", manager->cpu_info.total_usage);
        printf("Total CPU core Usage: %.2f%%\n", manager->cpu_info.core_usage[MAX_CORES]);
        sleep(1); // Sleep for 1 second before the next update
    }
    

    // Clean up
    destroyCpuManager(manager);
    
    return 0;
}
