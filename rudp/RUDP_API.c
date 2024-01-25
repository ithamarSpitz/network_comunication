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

#define MAX_PACKET_SIZE 540 // Maximum size of a UDP packet

typedef struct {
    uint16_t length;
    uint16_t checksum;
    uint8_t flags;
    uint16_t sender_port;   // Include sender port
    uint32_t sender_ip;     // Include sender IP address
    uint16_t ack_number;  // New field for acknowledgment number

} RUDPPacketHeader;

typedef struct {
    RUDPPacketHeader header;
    char payload[MAX_PACKET_SIZE - sizeof(RUDPPacketHeader)];
} RUDPPacket;

void print_sockaddr(const struct sockaddr *addr) {
    char ip_str[INET6_ADDRSTRLEN];  // Buffer for the IP address string

    if (addr->sa_family == AF_INET) {
        const struct sockaddr_in *ipv4_addr = (const struct sockaddr_in *)addr;

        // Convert the binary IPv4 address to a human-readable string
        if (inet_ntop(AF_INET, &(ipv4_addr->sin_addr), ip_str, INET_ADDRSTRLEN) == NULL) {
            perror("Error converting IPv4 address to string");
            return;
        }

        // Print the IPv4 address and port
        printf("IPv4: %s, Port: %d\n", ip_str, ntohs(ipv4_addr->sin_port));
    } else if (addr->sa_family == AF_INET6) {
        const struct sockaddr_in6 *ipv6_addr = (const struct sockaddr_in6 *)addr;

        // Convert the binary IPv6 address to a human-readable string
        if (inet_ntop(AF_INET6, &(ipv6_addr->sin6_addr), ip_str, INET6_ADDRSTRLEN) == NULL) {
            perror("Error converting IPv6 address to string");
            return;
        }

        // Print the IPv6 address and port
        printf("IPv6: %s, Port: %d\n", ip_str, ntohs(ipv6_addr->sin6_port));
    } else {
        printf("Unsupported address family\n");
    }
}

RUDPPacket create_packet(void *data, size_t length, const struct sockaddr *dest_addr, socklen_t addrlen, uint16_t ack_number){
    print_sockaddr(dest_addr);
    
    RUDPPacket packet;
    memset(&packet, 0, sizeof(packet));
    packet.header.length = htons(length);
    packet.header.checksum = 0; // You don't care about checksum
    packet.header.flags = 0;   // Set appropriate flags for your application
    packet.header.ack_number = htons(ack_number);
    // Extract and copy sender information
    if (addrlen >= sizeof(struct sockaddr_in)) {
        const struct sockaddr_in *ipv4_addr = (const struct sockaddr_in *)dest_addr;
        packet.header.sender_port = htons(ipv4_addr->sin_port);
        packet.header.sender_ip = ipv4_addr->sin_addr.s_addr;
    } else {
        // Handle IPv6 or other cases if needed
        // You may need to adjust this part based on your actual use case
        perror("Unsupported address family");
    }
    size_t copy_length = (length < sizeof(packet.payload)) ? length : sizeof(packet.payload);
    memcpy(packet.payload, data, copy_length);
    return packet;
}

int RUDP_send_ACK(int socket, struct sockaddr_in *dest_addr, const  struct sockaddr *src_addr, socklen_t addrlen, uint16_t ack_number){
    printf("RUDP_send_ACK\n");

    const char *ack_message = "RUDP_ACK";
    RUDPPacket ack_packet = create_packet(&ack_message, strlen(ack_message), (const struct sockaddr *)src_addr, addrlen, ack_number);
    printf("addrlen: %d",addrlen);
    print_sockaddr((const struct sockaddr *)dest_addr);
    ssize_t sent_bytes = sendto(socket, &ack_packet, sizeof(ack_packet), 0, (const struct sockaddr *)dest_addr, addrlen);
    if (sent_bytes == -1) {
        perror("Error sending data");
        return -1;
    }    
    printf("ack sent\n");
    return 0;
}

int RUDP_receive_ACK(int socket, uint16_t ack_number){
    RUDPPacket packet;
    printf("waiting for ack\n");
    ssize_t recv_bytes = recv(socket, &packet, sizeof(packet), 0);
    printf("ack received\n");
    if (recv_bytes == -1) {
        perror("Error receiving data");
        return -1;
    }
    uint16_t length_field = packet.header.length;
    uint16_t ack_field = ntohs(packet.header.ack_number);
    if ((ack_field == ack_number)
        &&(memcmp(packet.payload, "RUDP_ACK", strlen("RUDP_ACK"))) ) {
        return 0;
    } 
    else {
        return -1;
    }      
}

int RUDP_socket(struct sockaddr_in *address, int bind_sock) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    if(bind_sock == 1){
        if (bind(sockfd, (struct sockaddr*)address, sizeof(*address)) < 0) {
            perror("Error binding socket");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    }
    return sockfd;
}

int RUDP_send(int socket, char *data, size_t length, const struct sockaddr *dest_addr,const struct sockaddr *src_addr, socklen_t addrlen, uint16_t ack_number) {
    printf("RUDP_send -> data: %s\n",(char *)data);
    RUDPPacket packet = create_packet(data, length, (const struct sockaddr *)src_addr, addrlen, ack_number);
    do{
        ssize_t sent_bytes = sendto(socket, &packet, sizeof(packet), 0, dest_addr, addrlen);
        if (sent_bytes == -1) {
            perror("Error sending data");
            return -1;
        }    
    } while (RUDP_receive_ACK(socket, ack_number));
    return 0;
}

int RUDP_receive(int socket, void *buffer, size_t length,const  struct sockaddr *addr) {
    printf("RUDP_receive\n");
    RUDPPacket packet;
    struct sockaddr_in client_address;
    socklen_t addr_len = sizeof(client_address);

    ssize_t recv_bytes = recvfrom(socket, &packet, sizeof(packet), 0, (struct sockaddr*)&client_address, &addr_len);
    printf("recv_bytes: %zd\n", recv_bytes);

    if (recv_bytes == -1) {
        perror("Error receiving data");
        return -1;
    }

    //extruct the sender adress
    struct sockaddr_in sender_addr; 
    memset(&sender_addr, 0, sizeof(sender_addr));
    sender_addr.sin_family = AF_INET;
    sender_addr.sin_port = htons(packet.header.sender_port);
    printf("port: %d\n", packet.header.sender_port);
    struct in_addr ip_addr;
    ip_addr.s_addr = packet.header.sender_ip;
    printf("packet.header.sender_ip: %d\n", packet.header.sender_ip);
    inet_aton(inet_ntoa(ip_addr), &sender_addr.sin_addr); 
    socklen_t addrlen = sizeof(sender_addr);  // Set the addrlen
    
    uint16_t length_field = packet.header.length;
    uint16_t ack_field = ntohs(packet.header.ack_number);
    // Validate checksum and flags, handle errors if necessary
    printf("Received packet with payload length: %d\n", ntohs(packet.header.length));
    printf("Buffer size: %ld\n", length);
    printf("packet.payload: %s\n",packet.payload);
    memcpy(buffer, packet.payload, ntohs(packet.header.length));
    printf("buffer: %s\n",(char *)buffer);
    printf("Buffer alright\n");
    print_sockaddr((struct sockaddr *)&sender_addr);
    // Send acknowledgment
    if ((length_field >= strlen("RUDP_CLOSE") && strstr(buffer, "RUDP_CLOSE") != NULL)) {
        printf("RUDP_CLOSE\n\n\n");
        RUDP_send_ACK(socket, &sender_addr, addr, addrlen, ack_field);
        return -2;
    }
    else{
        if (RUDP_send_ACK(socket, &sender_addr, addr, addrlen, ack_field))
            return -1;
        return ntohs(packet.header.length);
    }
}

int RUDP_close(int socket, struct sockaddr_in *dest_addr,const struct sockaddr *src_addr, socklen_t addrlen) {
    print_sockaddr((const struct sockaddr *)src_addr);
    char *close_message = "RUDP_CLOSE";
    RUDP_send(socket, close_message, strlen(close_message), (const struct sockaddr *)dest_addr,(struct sockaddr *)src_addr, addrlen, 0);
    close(socket);
    return 0;
}
