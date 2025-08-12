#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "subManager/cpuManager.h"

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
        printf("Total CPU Usage: %.2f%%\n", manager->cpu_info.total_usage);
        sleep(1); // Sleep for 1 second before the next update
    }
    

    // Clean up
    destroyCpuManager(manager);
    
    return 0;
}
