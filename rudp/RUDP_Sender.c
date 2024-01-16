#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "RUDP_API.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>  // Include this for sockaddr_in
#include <arpa/inet.h>   // Include this for inet_pton


int main(int argc, char *argv[]) {
    FILE *stderr = stderr; // Add this line for stderr
    socklen_t client_addr_len; // Add this line for socklen_t

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <receiver_ip> <port> <file_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[2]);

    // Define server address structure
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    FILE *file = fopen(argv[3], "rb");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_buffer = malloc(file_size);
    if (file_buffer == NULL) {
        perror("Error allocating memory for file");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fread(file_buffer, 1, file_size, file);
    fclose(file);

    printf("Sending file...\n");

    // Initialize RUDP and establish connection
    int sockfd = RUDP_init_sender(argv[1], port);

    // Send file size first
    RUDP_send(sockfd, (char *)&file_size, sizeof(long), (struct sockaddr*)&server_addr, sizeof(server_addr));
    printf("RUDP_send()\n");

    // Send file data
    RUDP_send(sockfd, file_buffer, file_size, (struct sockaddr*)&server_addr, sizeof(server_addr));
    printf("RUDP_send()\n");

    // Clean up
    RUDP_close(sockfd);
        printf("RUDP_close(sockfd)\n");

    free(file_buffer);
        printf("free(file_buffer)");


    return 0;
}
