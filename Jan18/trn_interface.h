#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>

struct transmitter_input
{
    char *file_name;
    char *port_number;
    char *host_name;
};

struct transmitter 
{
    char *file_name;
    int port_number;
    int current_socket;
    struct hostent *server_name;
    struct sockaddr_in server_address;
    char buffer[2048];
    int (*fil_from_inp)(const struct transmitter_input*,
                      struct transmitter*);
    int (*sock_estb)(struct transmitter*);
    int (*file_tr)(struct transmitter*);

};

int fill_from_input(const struct transmitter_input *input,
                      struct transmitter *trmter);
int socket_establisher(struct transmitter *trmter);
int file_transmission(struct transmitter *trmter);