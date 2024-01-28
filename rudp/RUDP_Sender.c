//RUDP_Sender.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "RUDP_API.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PACKET_SIZE 524
#define FILE_PATH "random_file.txt"

void createRandomFile() {
    // Use dd command to create a random file of approximately 2 MB
    system("dd if=/dev/urandom of=random_file.txt bs=1024 count=2000");
}

void sendFile(int socket, FILE *file, const struct sockaddr_in *receiver_addr,const struct sockaddr_in *sender_addr, long file_size) {
    // Send file size first    
    RUDP_send(socket, (char *)&file_size, sizeof(long), (struct sockaddr *)receiver_addr, (struct sockaddr *)sender_addr, sizeof(*receiver_addr), (uint16_t)0);

    // Send file data in packets
    char buffer[PACKET_SIZE];
    size_t bytesRead;
    int ack_counter = 1;

    while ((bytesRead = fread(buffer, 1, PACKET_SIZE, file)) > 0) {
        if (ferror(file)) {
            perror("Error reading file");
            exit(EXIT_FAILURE);
        }
        // Send the packet
        RUDP_send(socket, buffer, bytesRead, (struct sockaddr *)receiver_addr, (struct sockaddr *)sender_addr, sizeof(*receiver_addr), (uint16_t)ack_counter);
        ack_counter++;            
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s -ip <receiver_ip> -p <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("Creating a random file...\n");
    createRandomFile();
    printf("Random file created.\n");

    struct sockaddr_in receiver_addr;
    memset(&receiver_addr, 0, sizeof(receiver_addr));
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(atoi(argv[4]));
    inet_pton(AF_INET, argv[2], &receiver_addr.sin_addr);
    struct sockaddr_in sender_addr;
    memset(&sender_addr, 0, sizeof(sender_addr));
    sender_addr.sin_family = AF_INET;
    sender_addr.sin_port = htons(atoi("9997"));
    inet_pton(AF_INET, "127.0.0.1", &sender_addr.sin_addr);

    char *handshake_message = "RUDP_HANDSHAKE";

    printf("Sender started. sending a handshake.\n");


    int socket = RUDP_socket(&sender_addr, 1);
    int send_result = RUDP_send(socket, handshake_message, (size_t)strlen(handshake_message), (struct sockaddr *)&receiver_addr,(struct sockaddr *)&sender_addr, sizeof(receiver_addr), (uint16_t)0);

    if (send_result == -1) {
        perror("Error sending data");
        exit(EXIT_FAILURE);
    }
    
    FILE *file = fopen(FILE_PATH, "rb");

    // Send the file using RUDP
    while(1){
        if (file == NULL) {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }

        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        sendFile(socket, file, &receiver_addr, &sender_addr, file_size);

        // Ask the user if they want to send the file again
        char response;
        printf("Do you want to send the file again? (y/n): ");
        scanf(" %c", &response);

        if (response != 'y') {
            if (response == 'n') {
                // Send exit message to the receiver
                break; // Exit the loop if the user doesn't want to send the file again
            }
            else{
                perror("incorrect char, bye!");
            }
        }
    }
    // Clean up
    RUDP_close(socket, &receiver_addr, (struct sockaddr *)&sender_addr, sizeof(receiver_addr));

    fclose(file);

    remove(FILE_PATH);

    return 0;
}
