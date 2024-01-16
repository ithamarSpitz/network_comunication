// RUDP_API.h

#ifndef RUDP_API_H
#define RUDP_API_H


#include <stddef.h>
#include <sys/socket.h>


int RUDP_socket();
int RUDP_send(int socket, const void *data, size_t length, const struct sockaddr *dest_addr, socklen_t addrlen);
int RUDP_receive(int socket, void *buffer, size_t length);
int RUDP_close(int socket);
int RUDP_init_sender(const char *receiver_ip, int port);
int RUDP_init_receiver(int port);

#endif

