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

enum result
{
    READY = 0,
    WORK = 1,
    FAIL = -1
};

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
    int port_num;
    char *file_name;
    int progress;
    pthread_t loader_th;
    pthread_mutex_t hide_from_serv; // закрываем доступ сервера к загружаемому в текущий момент клиентом файлу

    struct sock_attr_cl sock_attrs;
};

struct user
{
    int i;
    char ip[16];
};

struct to_thread
{
    struct sock_attr_cl sock_for_th;
    struct loader loader_for_th;
};

struct table
{
    enum result status;
    char serv_name[16];
    int port_num;
    pthread_t loader_th;

    struct sock_attr_cl sock_attrs;
    struct file_name_list *list;
};

struct to_names_thread
{
    struct sock_attr_cl sock_for_th;
    struct table *table_for_th;
};

struct file_name
{
    struct user owner;
    int i;
    char file_name[255];
};

struct file_name_list {
	struct file_name file_name;
	struct file_name_list* next;
};

//void new_unit_head(struct file_name_list **head, char *ipaddr);

void *loader_thr(void *);
void recv_file(int, struct loader *);
void error(const char *);
int num_of_segs(unsigned long, size_t);
int load_ready(struct loader **);
void stop_loader(struct loader **);
struct sock_attr_cl sock_creator_cl(struct sock_attr_cl *, char *);
struct loader *start_load(struct file_name);

void delete_file_name_list(struct file_name_list **);
struct file_name_list *get_name_for_list(struct file_name_list **, char *, char *);                                        
struct file_name_list *new_file_name_list(struct file_name );
void table_list_add(struct file_name_list **, struct file_name );
void show_file_name_list(struct file_name_list **);
char *recieve_filenames(int , struct table *);
struct sock_attr_cl sock_creator_names(struct sock_attr_cl *, char *);

enum result get_file_name_list (struct table **, struct file_name_list **);

void *table_thr(void *);
void start_table(struct table **, struct user);
void stop_table(struct table **);

#endif