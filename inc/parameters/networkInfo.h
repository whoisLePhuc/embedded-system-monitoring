#ifndef NETWORK_INFO_H
#define NETWORK_INFO_H

#define MAX_INTERFACES 8
#define MAX_NAME_LEN 64

typedef struct {
    char interface_name[MAX_NAME_LEN];
    float upload_speed;    // MB/s
    float download_speed;  // MB/s
    float bandwidth_usage; // %
    unsigned long long connection_count;
    char ip_address[64];
    struct {
        unsigned long long tx_packets;
        unsigned long long rx_packets;
    } packet_stats;
} NetworkInterface;

typedef struct {
    NetworkInterface interfaces[MAX_INTERFACES];
    int interface_count;
} NetworkInfo;

#endif // NETWORK_INFO_H
