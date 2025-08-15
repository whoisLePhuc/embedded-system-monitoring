#ifndef NETWORK_INFO_H
#define NETWORK_INFO_H

#define MAX_INTERFACES 8
#define MAX_NAME_LEN 16
#define MAX_IP_ADDRESSES 5

#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct {
    char name[MAX_NAME_LEN];
    float upload_speed_kbps;
    float download_speed_kbps;
    unsigned long long rx_packets_per_sec;
    unsigned long long tx_packets_per_sec;
} NetIOInfo;

typedef struct {
    char name[MAX_NAME_LEN];
    unsigned long long rx_bytes_pre;
    unsigned long long tx_bytes_pre;
    unsigned long long rx_packets_pre;
    unsigned long long tx_packets_pre;
} NetStatsSnapshot;

typedef struct {
    char name[MAX_NAME_LEN];
    char ip_address[INET_ADDRSTRLEN];
} IPAddressInfo;

void getNetStats(NetIOInfo io_info[], int *count);
void getIpAddresses(IPAddressInfo ip_info[], int *count);
int get_connection_count();

#endif // NETWORK_INFO_H
