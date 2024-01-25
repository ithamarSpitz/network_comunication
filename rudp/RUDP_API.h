// RUDP_API.h

#ifndef RUDP_API_H
#define RUDP_API_H


#include <stddef.h>
#include <sys/socket.h>
#include <stdint.h>
#include <netinet/in.h>
#include <netinet/ip.h> 

int RUDP_socket(struct sockaddr_in *address, int sender);
int RUDP_send(int socket, char *data, size_t length, const struct sockaddr *dest_addr,const struct sockaddr *src_addr, socklen_t addrlen, uint16_t ack_number);
int RUDP_receive(int socket, void *buffer, size_t length, const struct sockaddr *addr);
int RUDP_close(int socket, struct sockaddr_in *dest_addr,const struct sockaddr *src_addr, socklen_t addrlen);
#endif
