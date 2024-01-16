#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define FILE_PATH "random_file.txt"

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void createRandomFile() {
    // Use dd command to create a random file of approximately 2 MB
    system("dd if=/dev/urandom of=random_file.txt bs=1024 count=2000");
}

int main(int argc, char *argv[]) {
    if (argc != 7) {
        fprintf(stderr, "Usage: %s -ip <IP> -p <Port> -algo <ALGO>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *receiver_ip = NULL;
    int receiver_port = -1;
    const char *algo = NULL;

    // cut the given values to pices:, strcmp is string comparison

    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-ip") == 0) {
            receiver_ip = argv[i + 1];
        } else if (strcmp(argv[i], "-p") == 0) {
            receiver_port = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-algo") == 0) {
            algo = argv[i + 1];
        }
    }

    // exit if given values are incorrect

    if (receiver_ip == NULL || receiver_port == -1 || algo == NULL) {
        fprintf(stderr, "Receiver IP, port number, or algorithm is missing or invalid.\n");
        exit(EXIT_FAILURE);
    }

    printf("Creating a random file...\n");
    createRandomFile();
    printf("Random file created.\n");

    printf("Sender started. Connecting to %s on port %d with algorithm %s.\n", receiver_ip, receiver_port, algo);
    
    // Create a socket using the AF_INET address family (IPv4),
    // SOCK_STREAM socket type (TCP), and protocol 0.
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        // Print an error message and exit the program if the socket creation fails.
        error("Error opening socket");





    // Set the TCP congestion control algorithm for this socket
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_CONGESTION, algo, strlen(algo)) < 0)
        error("Error setting congestion control algorithm");

    // Declare a structure to hold information about the receiver's address 
    struct sockaddr_in receiver_addr;
    // Initialize the structure, setting all its members to zero.
    memset(&receiver_addr, 0, sizeof(receiver_addr));
    // Set the address family to AF_INET (IPv4).
    receiver_addr.sin_family = AF_INET;
    // Set the port number for the receiver's address, 
    //converting it to network byte order using htons.
    receiver_addr.sin_port = htons(receiver_port);

    // Convert the human-readable IPv4 address stored in receiver_ip to binary form 
    // and store it in the sin_addr member of the receiver_addr structure.
    if (inet_pton(AF_INET, receiver_ip, &receiver_addr.sin_addr) <= 0)
        error("Error converting IP address");

    printf("Connecting to the Receiver...\n");

    // Attempt to establish a connection to the receiver using the socket file descriptor,
    // the receiver_addr structure containing destination address information, and its size.
    if (connect(sockfd, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr)) < 0)
        error("Error connecting to receiver");

    printf("Connection established with the Receiver.\n");

    while (1) {
        FILE *file = fopen(FILE_PATH, "rb");
        if (!file)
            error("Error opening file");

        char buffer[BUFFER_SIZE];
        size_t bytesRead;

        printf("Sending file...\n");

        //prepare the receiver  
        send(sockfd, "prpr", 4, 0);
        
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            send(sockfd, buffer, bytesRead, 0);
        }

        fclose(file);

        printf("File sent successfully.\n");

        //tell the receiver that the file sent successfully
        send(sockfd, "sent", 4, 0);

        // Ask the user if they want to send the file again
        char response;
        printf("Do you want to send the file again? (y/n): ");
        scanf(" %c", &response);

        if (response != 'y') {
            if (response == 'n') {
                // Send exit message to the receiver
                send(sockfd, "exit", 4, 0);
                break; // Exit the loop if the user doesn't want to send the file again
            }
            else{
                error("incorrect char, bye!");
            }
        }
    }

    close(sockfd);

    return 0;
}
