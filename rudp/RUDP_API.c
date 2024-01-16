// RUDP_API.c
#include "RUDP_API.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>


#define MAX_PACKET_SIZE 1500 // Maximum size of a UDP packet

typedef struct {
    uint16_t length;
    uint16_t checksum;
    uint8_t flags;
} RUDPPacketHeader;

typedef struct {
    RUDPPacketHeader header;
    char payload[MAX_PACKET_SIZE - sizeof(RUDPPacketHeader)];
} RUDPPacket;

// RUDP_API.c

int RUDP_init_receiver(int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the local address
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // You might want to perform additional initialization steps here

    return sockfd;
}

int RUDP_init_sender(const char *receiver_ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // Convert receiver IP address to binary form
    if (inet_pton(AF_INET, receiver_ip, &server_addr.sin_addr) <= 0) {
        perror("Error converting IP address");
        exit(EXIT_FAILURE);
    }

    // You might want to perform additional initialization steps here

    return sockfd;
}

int RUDP_socket() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

int RUDP_send(int socket, const void *data, size_t length, const struct sockaddr *dest_addr, socklen_t addrlen) {
    RUDPPacket packet;
    packet.header.length = htons(length);
    packet.header.checksum = 0; // You need to implement checksum calculation
    packet.header.flags = 0;   // Set appropriate flags for your application

    memcpy(packet.payload, data, length);

    ssize_t sent_bytes = sendto(socket, &packet, sizeof(packet), 0, dest_addr, addrlen);
    if (sent_bytes == -1) {
        perror("Error sending data");
        return -1;
    }
    return 0;
}

int RUDP_receive(int socket, void *buffer, size_t length) {
    RUDPPacket packet;
    ssize_t recv_bytes = recv(socket, &packet, sizeof(packet), 0);
    if (recv_bytes == -1) {
        perror("Error receiving data");
        return -1;
    }

    // Validate checksum and flags, handle errors if necessary

    memcpy(buffer, packet.payload, ntohs(packet.header.length));
    return ntohs(packet.header.length);
}

int RUDP_close(int socket) {
    close(socket);
    return 0;
}
