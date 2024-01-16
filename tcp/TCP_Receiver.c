#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    // Check if the correct number and format of command-line arguments are provided.
    if (argc != 5 || strcmp(argv[1], "-p") != 0 || strcmp(argv[3], "-algo") != 0) {
        fprintf(stderr, "Usage: %s -p <Port> -algo <ALGO>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = -1;
    const char *algo = NULL;

    // Parse command-line arguments.
    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-p") == 0) {
            port = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-algo") == 0) {
            algo = argv[i + 1];
        }
    }

    // Validate that both the port and algorithm are provided and valid.
    if (port == -1 || algo == NULL) {
        fprintf(stderr, "Port number or algorithm is missing or invalid.\n");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "Receiver started. Listening on port %d with algorithm %s.\n", port, algo);

    // Create a socket for communication.
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("Error opening socket");

    // Set the TCP congestion control algorithm for this socket
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_CONGESTION, algo, strlen(algo)) < 0)
        error("Error setting congestion control algorithm");

    // Set up the receiver's address structure.
    struct sockaddr_in receiver_addr, sender_addr;
    socklen_t sender_len = sizeof(sender_addr);

    memset(&receiver_addr, 0, sizeof(receiver_addr));
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_addr.s_addr = INADDR_ANY;
    receiver_addr.sin_port = htons(port);

    // Bind the socket to the specified address and port.
    if (bind(sockfd, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr)) < 0)
        error("Error on binding");

    listen(sockfd, 1);

    fprintf(stderr, "Waiting for a connection...\n");

    // Accept a connection from a sender.
    int newsockfd = accept(sockfd, (struct sockaddr *)&sender_addr, &sender_len);
    if (newsockfd < 0)
        error("Error on accept");

    fprintf(stderr, "Connection established with the Sender.\n");

    FILE *file = fopen("received_file.txt", "wb");
    if (!file)
        error("Error creating file");

    char buffer[BUFFER_SIZE];
    size_t bytesRead;
    clock_t prpr_time = 0;
    double total_time = 0;
    int count = 0;
    size_t bytes_per_file = 0;
    size_t total_bytes = 0;

    // Loop until an error occurs
    while ((bytesRead = recv(newsockfd, buffer, sizeof(buffer), 0)) > 0) {
        // Check if the received data contains "prpr"
        if (strstr(buffer, "prpr") != NULL) {
            prpr_time = clock();
        } else if (strstr(buffer, "sent") != NULL && 0 != bytes_per_file ) {
            clock_t sent_time = clock();
            double time_taken = ((double)(sent_time - prpr_time)) / CLOCKS_PER_SEC * 1000;
            double speed = (bytes_per_file/(1024.0*1024.0))/(time_taken/1000);
            fprintf(stderr, "file arrived, duration: %.2f ms speed: %.2f mb/s\n", time_taken, speed);
            total_bytes += bytes_per_file;
            bytes_per_file = 0;
            // Accumulate total time and increment count for averaging
            total_time += time_taken;
            count++;
        } else {
            // Write to the file only if the data doesn't contain "prpr" or "sent"
            fwrite(buffer, 1, bytesRead, file);
            bytes_per_file += bytesRead;
        }
    }
    // Calculate and print average time
    double average_time = (count > 0) ? total_time / count : 0;
    fprintf(stderr, "Average time taken: %.2fms\n", average_time);
    // Calculate and print the total average speed
    double avg_speed = (total_bytes/(1024.0*1024.0))/(total_time/1000);  // Variable to accumulate total speed
    fprintf(stderr, "Average Speed: %.2f MB/s\n", avg_speed);
    fprintf(stderr, "Files transfer completed.\n");

    fclose(file);
    close(newsockfd);
    close(sockfd);

    return 0;
}