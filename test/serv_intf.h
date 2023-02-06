#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <pthread.h>

#define MAX_FILE_SZ 4294967295

struct sock_attr_sv
{
    int sock_d;
    int portno;
    struct sockaddr_in serv_addr;
    socklen_t clilen;
    struct sockaddr_in cli_addr;
};



void error(const char *);
void send_file(int);
char *get_file_size(const char *);
struct sock_attr_sv sock_creator_sv(struct sock_attr_sv *);
struct sock_attr_sv sock_creator_names(struct sock_attr_sv *);

void send_names(int);
char *get_filenames(void);