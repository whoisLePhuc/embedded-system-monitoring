#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "networkManager/networkInfo.h"
#include "logger/logger.h"

void getNetStats(NetIOInfo io_info[], int *count) {
    FILE *fp;
    char line[256];
    NetStatsSnapshot snapshots[MAX_INTERFACES];
    int pre_count = 0;

    // --- Get initial net information ---
    fp = fopen("/proc/net/dev", "r");
    if (!fp) {
        logMessage(LOG_ERROR, "Error opening /proc/net/dev");
        return;
    }
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);
    while (fgets(line, sizeof(line), fp)) {
        if (pre_count >= MAX_INTERFACES) break;
        unsigned long long rx_bytes, rx_packets, tx_bytes, tx_packets, discard;
        char device[16];
        sscanf(line, "%s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
               device, &rx_bytes, &rx_packets, &discard, &discard, &discard, &discard, &discard, &discard,
               &tx_bytes, &tx_packets, &discard, &discard, &discard, &discard, &discard, &discard);
        device[strlen(device) - 1] = '\0'; // remove ':'
        snapshots[pre_count].rx_bytes_pre = rx_bytes;
        snapshots[pre_count].tx_bytes_pre = tx_bytes;
        snapshots[pre_count].rx_packets_pre = rx_packets;
        snapshots[pre_count].tx_packets_pre = tx_packets;
        strncpy(snapshots[pre_count].name, device, sizeof(snapshots[pre_count].name));
        pre_count++;
    }
    fclose(fp);
    sleep(1);
    // --- Take second information and calculate ---
    fp = fopen("/proc/net/dev", "r");
    if (!fp) {
        logMessage(LOG_ERROR, "Error opening /proc/net/dev");
        return;
    }
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);
    *count = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (*count >= MAX_INTERFACES) break;
        unsigned long long rx_bytes, rx_packets, tx_bytes, tx_packets, discard;
        char device[16];
        sscanf(line, "%s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
               device, &rx_bytes, &rx_packets, &discard, &discard, &discard, &discard, &discard, &discard,
               &tx_bytes, &tx_packets, &discard, &discard, &discard, &discard, &discard, &discard);
        device[strlen(device) - 1] = '\0';

        // Find matching previous snapshot
        for (int i = 0; i < pre_count; i++) {
            if (strcmp(snapshots[i].name, device) == 0) {
                // speed in KB/s
                io_info[*count].download_speed_kbps = (float)(rx_bytes - snapshots[i].rx_bytes_pre) / 1024.0;
                io_info[*count].upload_speed_kbps = (float)(tx_bytes - snapshots[i].tx_bytes_pre) / 1024.0;
                // packet per sec
                io_info[*count].rx_packets_per_sec = rx_packets - snapshots[i].rx_packets_pre;
                io_info[*count].tx_packets_per_sec = tx_packets - snapshots[i].tx_packets_pre;
                strncpy(io_info[*count].name, device, sizeof(io_info[*count].name));
                (*count)++;
                break;
            }
        }
    }
    fclose(fp);
}

// Function to get IP addresses of all network interfaces
void getIpAddresses(IPAddressInfo ip_info[], int *count) {
    struct ifaddrs *ifaddr, *ifa;
    *count = 0;
    if (getifaddrs(&ifaddr) == -1) {
        logMessage(LOG_ERROR, "Error getting network interfaces");
        return;
    }
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (*count >= MAX_IP_ADDRESSES){
            logMessage(LOG_WARNING, "Maximum IP address count reached: %d", MAX_IP_ADDRESSES);
            break;
        }
        if (ifa->ifa_addr == NULL) {
            continue;
        }
        int family = ifa->ifa_addr->sa_family;
        if (family == AF_INET) {
            char ip[INET_ADDRSTRLEN];
            struct sockaddr_in *sa = (struct sockaddr_in *) ifa->ifa_addr;
            inet_ntop(AF_INET, &(sa->sin_addr), ip, INET_ADDRSTRLEN);
            strncpy(ip_info[*count].name, ifa->ifa_name, sizeof(ip_info[*count].name));
            strncpy(ip_info[*count].ip_address, ip, sizeof(ip_info[*count].ip_address));
            (*count)++;
        }
    }
    freeifaddrs(ifaddr);
}

// Function to get the number of active TCP connections
int get_connection_count() {
    FILE *fp;
    char line[256];
    int count = 0;
    // Call the ss command to get TCP connections
    fp = popen("ss -t -a | wc -l", "r");
    if (fp == NULL) {
        logMessage(LOG_ERROR, "Failed to run ss command");
        return -1;
    }
    fgets(line, sizeof(line), fp);
    sscanf(line, "%d", &count);
    pclose(fp);
    return count > 0 ? count - 1 : 0;
}