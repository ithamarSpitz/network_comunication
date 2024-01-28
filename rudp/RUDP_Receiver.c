//RUDP_Receiver.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "RUDP_API.h"
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h> 
#include <time.h>

#define MAX_PACKET_SIZE 524
#define FILE_PATH "received_file.txt"

// Function to receive the file using RUDP
long receiveFile(int sockfd, FILE *file, const struct sockaddr *addr) {
    // Receive file size first
    long file_size;
    if(RUDP_receive(sockfd, (char *)&file_size, (struct sockaddr *)&addr) == -2)
        return 0;
    // Receive file data in packets
    char buffer[MAX_PACKET_SIZE] = {0};
    size_t totalBytesReceived = 0;

    while (totalBytesReceived < (long unsigned int)file_size) {
        size_t bytesRead = RUDP_receive(sockfd, buffer, (struct sockaddr *)&addr);
        if (bytesRead > 0) {
            // Normal data, write to file
            fwrite(buffer, 1, bytesRead, file);
            if (ferror(file)) {
                perror("Error writing to file");
                exit(EXIT_FAILURE);
            }
            totalBytesReceived += bytesRead;
            fflush(file);
        } else {
            if(bytesRead == (long unsigned int)-2){
                printf("exit\n");
                return 0;
            }else{
                // Handle error or timeout, possibly request retransmission
                printf("Error receiving data.\n");
            }
        }
    }
    return totalBytesReceived;
}


int main(int argc, char *argv[]) {

    if (argc != 3) {
        fprintf(stderr, "Usage: %s -p <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, "127.0.0.1", &(server_addr.sin_addr));
    int dest_socket = RUDP_socket(&server_addr, 1);

    // Perform handshake
    char handshake_buffer[256];
    memset(handshake_buffer, 0, 256);
    size_t handshake_length = RUDP_receive(dest_socket, handshake_buffer, (struct sockaddr *)&server_addr);
    // Check the handshake message
    if (!(handshake_length > 0 && strncmp(handshake_buffer, "RUDP_HANDSHAKE", strlen("RUDP_HANDSHAKE")) == 0)) {
        fprintf(stderr, "Error: Handshake failed.\n");
    }
    printf("Receiver started\n");

    // Open a new file for writing
    FILE *received_file = fopen(FILE_PATH, "wb");
    if (received_file == NULL) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    // Receive the file using RUDP
    double file_time = 0.0;
    int counter = 0;
    double avrage_time = 0;
    double avrage_speed = 0;
    do{    
        clock_t start_time = clock();
        long bytes_per_file = receiveFile(dest_socket, received_file, (struct sockaddr *)&server_addr);
        clock_t end_time = clock();
        file_time = (end_time - start_time)/1000;
        double speed = (bytes_per_file/(1024.0*1024.0))/(file_time/1000);
;
        if(file_time == 0.0)
            break;
        printf("duration: %f ms\n", file_time);
        printf("speed: %f mb/s\n", speed);
        
        avrage_time = (((avrage_time*(double)counter)+file_time)/((double)counter+1));
        avrage_speed = (((avrage_speed*(double)counter)+speed)/((double)counter+1));
        counter++;

    }while(file_time != 0.0);
    printf("avarage time: %f\n", avrage_time);
    printf("avarage speed: %f\n", avrage_speed);
    

    // Clean up
    fclose(received_file);
    remove(FILE_PATH);


    return 0;
}
