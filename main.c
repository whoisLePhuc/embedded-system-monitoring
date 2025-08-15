#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cpuManager/cpuManager.h"

int main() {
    cpuManager *CpuManager = createCpuManager();
    if (CpuManager == NULL) {
        return 1;
    }
    while (1) {
        CpuManager->update(CpuManager);
        CpuManager->display(CpuManager);
        sleep(1);
    }
    CpuManager->destroy(CpuManager);
    return 0;
}