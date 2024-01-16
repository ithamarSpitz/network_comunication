#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "RUDP_API.h"

int main(int argc, char *argv[]) {
    FILE *stderr = stderr; // Add this line for stderr

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    printf("Receiving file...\n");

    // Initialize RUDP and establish connection
    int sockfd = RUDP_init_receiver(port);

    // Receive file size
    long file_size;
    RUDP_receive(sockfd, (char *)&file_size, sizeof(long));

    // Receive file data
    char *file_buffer = malloc(file_size);
    RUDP_receive(sockfd, file_buffer, file_size);

    // Your file-saving logic goes here
    // For example, you can save the received file to disk
    FILE *received_file = fopen("received_file.txt", "wb");
    if (received_file == NULL) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    fwrite(file_buffer, 1, file_size, received_file);

    // Clean up
    RUDP_close(sockfd);
    fclose(received_file);
    free(file_buffer);

    return 0;
}
