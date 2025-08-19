#ifndef SYSTEM_METRICS_H
#define SYSTEM_METRICS_H

#include"systemInfo.h"

typedef struct systemManager systemManager;
struct systemManager {
    SystemInfo systemInfo;
    SystemDetails systemDetails;
    ServiceInfo serviceInfo[MAX_ACTIVE_SERVICES];
    int serviceCount;
    void (*update)(systemManager *self); 
    void (*destroy)(systemManager *self);
    void (*display)(systemManager *self);
};

systemManager  *createSystemManager(); 
void destroySystemManager(systemManager  *self); 
void updateSystemInfo(systemManager  *self); 
void displaySystemInfo(systemManager  *self);

#endif // SYSTEM_METRICS_H
