#ifndef TRANSCEIVER_INTF_H
#define TRANSCEIVER_INTF_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>


struct sock_attr_cl
{
	int sock_ds;
	int portno;
	struct sockaddr_in serv_addr;
	struct hostent *server;
};

struct loader
{
	char *serv_name;
	char *port_num;
	char *file_name;
	int progress;

	struct sock_attr_cl sock_attrs;
};

struct user
{
	int i;
	char *host_nm;
	char *port_no;
};

struct to_thread
{
    struct sock_attr_cl sock_for_th;
    struct loader loader_for_th;
};

struct file_name
{
	struct user owner;
	int i;
	char *file_name;
};

void* loader_thr(void *);
void recv_file(int, struct loader *);
void error(const char *);
int num_of_segs(unsigned long, size_t);
int load_ready(struct loader **);
void stop_loader(struct loader **);
struct sock_attr_cl sock_creator_cl(struct sock_attr_cl *, char *, char *);
struct loader *start_load(struct file_name);


#endif