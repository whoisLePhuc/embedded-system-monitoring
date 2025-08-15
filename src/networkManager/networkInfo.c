#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "networkManager/networkInfo.h"

void getNetStats(NetIOInfo io_info[], int *count) {
    FILE *fp;
    char line[256];
    NetStatsSnapshot snapshots[MAX_INTERFACES];
    int pre_count = 0;

    // --- Chụp ảnh lần 1 ---
    fp = fopen("/proc/net/dev", "r");
    if (!fp) {
        perror("Error opening /proc/net/dev");
        return;
    }
    // Bỏ qua 2 dòng đầu
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);
    while (fgets(line, sizeof(line), fp)) {
        if (pre_count >= MAX_INTERFACES) break;
        unsigned long long rx_bytes, rx_packets, tx_bytes, tx_packets, discard;
        char device[16];
        sscanf(line, "%s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
               device, &rx_bytes, &rx_packets, &discard, &discard, &discard, &discard, &discard, &discard,
               &tx_bytes, &tx_packets, &discard, &discard, &discard, &discard, &discard, &discard);
        
        device[strlen(device) - 1] = '\0'; // Bỏ dấu ':'
        
        snapshots[pre_count].rx_bytes_pre = rx_bytes;
        snapshots[pre_count].tx_bytes_pre = tx_bytes;
        snapshots[pre_count].rx_packets_pre = rx_packets;
        snapshots[pre_count].tx_packets_pre = tx_packets;
        strncpy(snapshots[pre_count].name, device, sizeof(snapshots[pre_count].name));
        pre_count++;
    }
    fclose(fp);

    // --- Chờ 1 giây ---
    sleep(1);

    // --- Chụp ảnh lần 2 và tính toán ---
    fp = fopen("/proc/net/dev", "r");
    if (!fp) {
        perror("Error opening /proc/net/dev");
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

        // Tìm thiết bị tương ứng từ lần chụp đầu
        for (int i = 0; i < pre_count; i++) {
            if (strcmp(snapshots[i].name, device) == 0) {
                // Tốc độ (tính bằng KB/s)
                io_info[*count].download_speed_kbps = (float)(rx_bytes - snapshots[i].rx_bytes_pre) / 1024.0;
                io_info[*count].upload_speed_kbps = (float)(tx_bytes - snapshots[i].tx_bytes_pre) / 1024.0;
                // Thống kê gói tin
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

void getIpAddresses(IPAddressInfo ip_info[], int *count) {
    struct ifaddrs *ifaddr, *ifa;
    *count = 0;

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return;
    }
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (*count >= MAX_IP_ADDRESSES) break;
        if (ifa->ifa_addr == NULL) continue;
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

int get_connection_count() {
    FILE *fp;
    char line[256];
    int count = 0;

    // Đếm số dòng trong file chứa thông tin kết nối TCP
    fp = popen("ss -t -a | wc -l", "r");
    if (fp == NULL) {
        perror("Failed to run ss command");
        return -1;
    }
    fgets(line, sizeof(line), fp);
    sscanf(line, "%d", &count);
    pclose(fp);
    
    // Trừ đi 1 cho dòng tiêu đề
    return count > 0 ? count - 1 : 0;
}