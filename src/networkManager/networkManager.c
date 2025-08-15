#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "networkManager/networkManager.h"

NetworkManager *createNetworkManager(){
    NetworkManager *manager = (NetworkManager *)malloc(sizeof(NetworkManager));
    if (!manager) {
        perror("Failed to create NetworkManager");
        return NULL;
    }
    memset(manager, 0, sizeof(NetworkManager));
    manager->update = updateNetworkInfo;
    manager->destroy = destroyNetworkManager;
    manager->display = displayNetworkInfo;
    return manager;
}

void destroyNetworkManager(NetworkManager *self){   
    if (self) {
        free(self);
    } else {
        fprintf(stderr, "Attempted to destroy a NULL NetworkManager\n");
    }
}

void updateNetworkInfo(NetworkManager *self){
    if (!self) {
        fprintf(stderr, "NetworkManager is NULL, cannot update network info\n");
        return;
    }
    getNetStats(self->net_io_info, &self->network_IO_count);
    // Update IP addresses
    getIpAddresses(self->ip_addresses, &self->ip_address_count);
    
    self->connection_count = get_connection_count();
}

void displayNetworkInfo(NetworkManager *self) {
    if (self == NULL) {
        fprintf(stderr, "Cannot display info from a NULL manager.\n");
        return;
    }

    system("clear");
    printf("==================== NETWORK MONITOR ====================\n");
    
    printf("Network I/O Stats:\n");
    // Giả định bạn có một biến để lưu số lượng giao diện mạng thực tế
    for (int i = 0; i < self->network_IO_count; i++) {
        if (strlen(self->net_io_info[i].name) > 0) {
            printf("  Interface: %s\n", self->net_io_info[i].name);
            printf("    Download Speed: %.2f KB/s\n", self->net_io_info[i].download_speed_kbps);
            printf("    Upload Speed:   %.2f KB/s\n", self->net_io_info[i].upload_speed_kbps);
            printf("    Rx Packets:     %llu/s\n", self->net_io_info[i].rx_packets_per_sec);
            printf("    Tx Packets:     %llu/s\n", self->net_io_info[i].tx_packets_per_sec);
            printf("\n");
        }
    }
    
    printf("---------------------------------------------------------\n");
    printf("IP Addresses:\n");
    // Giả định bạn có một biến để lưu số lượng địa chỉ IP thực tế
    for (int i = 0; i < self->ip_address_count; i++) {
        if (strlen(self->ip_addresses[i].name) > 0) {
            printf("  Interface: %s\n", self->ip_addresses[i].name);
            printf("    IP:        %s\n", self->ip_addresses[i].ip_address);
            printf("\n");
        }
    }

    printf("---------------------------------------------------------\n");
    printf("Total Connections: %d\n", self->connection_count);
    printf("=========================================================\n");
}