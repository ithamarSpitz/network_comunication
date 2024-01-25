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
// Function to receive the file using RUDP
double receiveFile(int sockfd, FILE *file, const struct sockaddr *addr) {
    printf("Receiving file...\n");
    // Receive file size first
    long file_size;
    if(RUDP_receive(sockfd, (char *)&file_size, sizeof(long),(struct sockaddr *)&addr) == -2)
        return 0;
    clock_t start_time = clock();
    // Receive file data in packets
    char buffer[MAX_PACKET_SIZE] = {0};
    size_t totalBytesReceived = 0;

    while (totalBytesReceived < file_size) {
        printf("totalBytesReceived: %ld\n file_size: %ld\n", totalBytesReceived, file_size);
        size_t bytesRead = RUDP_receive(sockfd, buffer, sizeof(buffer),(struct sockaddr *)&addr);
        printf("bytesRead: %ld", bytesRead);
        // Check if the received data is an acknowledgment
        if (bytesRead == 0) {
            printf("Received acknowledgment.\n");
        } else {
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
                if(bytesRead == -2){
                    printf("exit\n");
                    return 0;
                }else{
                    // Handle error or timeout, possibly request retransmission
                    printf("Error receiving data.\n");
                }
            }
        }
    }
    clock_t end_time = clock();

    printf("File received successfully.\n");
    return ((double)end_time - (double)start_time);
}


int main(int argc, char *argv[]) {

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_aton("127.0.0.1", &server_addr.sin_addr);     
    int dest_socket = RUDP_socket(&server_addr, 1);

    // Perform handshake
    char handshake_buffer[256];
    memset(handshake_buffer, 0, 256);
    size_t handshake_length = RUDP_receive(dest_socket, handshake_buffer, sizeof(handshake_buffer), (struct sockaddr *)&server_addr);
    printf("handshake_buffer: %s\n", handshake_buffer);
    // Check the handshake message
    if (handshake_length > 0 && strncmp(handshake_buffer, "RUDP_HANDSHAKE", strlen("RUDP_HANDSHAKE")) == 0) {
        printf("Handshake successful. Receiver ready.\n");
    } else {
        fprintf(stderr, "Error: Handshake failed.\n");
        // Handle the error, possibly close the socket and exit
    }

    printf("Receiver started, waiting for sender...\n");

    // Open a new file for writing
    FILE *received_file = fopen("received_file1.txt", "wb");
    if (received_file == NULL) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    // Receive the file using RUDP
    double file_time = 0.0;
    int counter = 1;
    double avrage_time = 0;
    do{
        file_time = receiveFile(dest_socket, received_file, (struct sockaddr *)&server_addr);
        if(file_time == 0.0)
            break;
        printf("file time is: %f", file_time);
        avrage_time = (((avrage_time*(double)counter-1)+file_time)/(double)counter);
        counter++;

    }while(file_time != 0.0);
    printf("avarage time is %f", avrage_time);

    // Clean up
    fclose(received_file);

    return 0;
}
