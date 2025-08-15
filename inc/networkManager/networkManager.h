#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "networkInfo.h"

typedef struct NetworkManager NetworkManager;
struct NetworkManager {
    NetIOInfo net_io_info[MAX_INTERFACES];
    IPAddressInfo ip_addresses[MAX_IP_ADDRESSES];
    int network_IO_count;
    int ip_address_count; 
    int connection_count;
    void (*update)(NetworkManager *self); 
    void (*destroy)(NetworkManager *self);
    void (*display)(NetworkManager *self);
};

NetworkManager *createNetworkManager(); // Create a NetworkManager object
void destroyNetworkManager(NetworkManager *self); // Free memory of NetworkManager object
void updateNetworkInfo(NetworkManager *self); // Update network information
void displayNetworkInfo(NetworkManager *self); // Display network information

#endif // NETWORK_MANAGER_H