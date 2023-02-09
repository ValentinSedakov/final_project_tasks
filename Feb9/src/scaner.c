#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/select.h>
#include <pthread.h>
#include <poll.h>
#include "recv_intf.h"
#include "recv_intf.c"


#define PORT 50000
#define TIMESEC 2000

struct scaner
{
    int i;
};

static struct user myip;
static struct user broadip;

void getIP();
void *host_scaner(void *arg);

struct user_list
{
    struct user user;
    struct user_list *next;
};

void new_unit_head(struct user_list **head, char *ipaddr);

static struct user_list *new_user_list(struct user user)
{
    struct user_list *res = (struct user_list *)malloc(sizeof(struct user_list));
    res->next = NULL;
    res->user = user;
    return res;
}

static void user_list_add(struct user_list **head, struct user user)
{

    if (*head == NULL)
    {

        *head = new_user_list(user);

        return;
    }
    struct user_list *temp = *head;

    while (temp->next != NULL)
    {
        temp = temp->next;
    }
    temp->next = new_user_list(user);
    return;
}

int size_user_list(struct user_list **head)
{

    if (*head == NULL)
    {
        return 0;
    }
    int i = 1;
    struct user_list *temp = *head;
    while (temp->next != NULL)
    {
        temp = temp->next;
        i++;
    }
    return i;
}

struct user *get_user(struct user_list **head, int num)
{
    if (*head == NULL)
    {
        return NULL;
    }

    int i = 0;
    struct user_list *temp = *head;
    while (temp != NULL)
    {
        if (num == i)
        {
            return &(temp->user);
        }
        temp = temp->next;
        i++;
    }
    return NULL;
}

void delete_user_list(struct user_list **head)
{
    if (*head == NULL)
    {
        return;
    }
    struct user_list *temp = *head;
    while (temp != NULL)
    {
        struct user_list *del = temp;
        temp = temp->next;
        free(del);
    }
    *head = NULL;
    return;
}

void stop_scan(struct scaner **scaner)
{
    if (*scaner == NULL)
    {
        return;
    }
    free(*scaner);
    *scaner = NULL;
    return;
}

void start_scan(struct scaner **scaner)
{
    if (*scaner != NULL)
    {
        stop_scan(scaner);
    }
    static int i = 0;
    *scaner = (struct scaner *)malloc(sizeof(struct scaner));
    (*scaner)->i = i++;
    getIP();

    return;
}

enum result get_user_list(struct scaner **scaner, struct user_list **userList)
{
   

    if ((*userList) != NULL)
    {
        delete_user_list(&(*userList));
    }

    if (*scaner != NULL)
    {

        pthread_t tid;
        pthread_create(&tid, NULL, host_scaner, &(*userList));
        pthread_join(tid, (void **)&(*userList));

    }

    return READY;
}

int size_file_name_list(struct file_name_list **head)
{
    if (*head == NULL)
    {
        return 0;
    }
    int i = 1;
    struct file_name_list *temp = *head;
    while (temp->next != NULL)
    {
        temp = temp->next;
        i++;
    }
    return i;
}

struct file_name *get_file_name(struct file_name_list **head, int num)
{
    if (*head == NULL)
    {
        return NULL;
    }
    int i = 0;
    struct file_name_list *temp = *head;
    while (temp != NULL)
    {
        if (num == i)
        {
            return &(temp->file_name);
        }
        temp = temp->next;
        i++;
    }
    return NULL;
}

struct queue_loaders
{
    struct loader *loader;
    struct queue_loaders *next;
};

static struct queue_loaders *new_queue_loaders(struct loader *loader)
{
    struct queue_loaders *queue_loaders = (struct queue_loaders *)malloc(sizeof(struct queue_loaders));
    queue_loaders->loader = loader;
    queue_loaders->next = NULL;
    return queue_loaders;
}

void push_queue_loaders(struct queue_loaders **head, struct loader *loader)
{
    if (*head == NULL)
    {
        *head = new_queue_loaders(loader);
        return;
    }
    struct queue_loaders *temp = *head;
    while (temp->next != NULL)
    {
        temp = temp->next;
    }
    temp->next = new_queue_loaders(loader);
    return;
}

struct loader *front_queue_loaders(struct queue_loaders **head)
{
    if (*head == NULL)
    {
        return NULL;
    }
    return (*head)->loader;
}
void pop_queue_loaders(struct queue_loaders **head)
{
    if (*head == NULL)
    {
        return;
    }
    struct queue_loaders *temp = (*head)->next;
    stop_loader(&((*head)->loader));
    free(*head);
    *head = temp;
    return;
}

void delete_queue_loaders(struct queue_loaders **head)
{
    while (*head != NULL)
    {
        pop_queue_loaders(head);
    }
    return;
}

void getIP()
{
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    int len;
    int j = 0;
    static char *addr;

    getifaddrs(&ifap);
    for (ifa = ifap; ifa; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET)
        {
            sa = (struct sockaddr_in *)ifa->ifa_addr;
            addr = inet_ntoa(sa->sin_addr);

            if ((addr[0] == '1' && addr[1] == '9' && addr[2] == '2') ||
                (addr[0] == '1' && addr[1] == '7' && addr[2] == '2' && addr[4] == '1' && addr[5] == '6'))
            {
                strcpy(myip.ip, addr);
                len = strlen(myip.ip);
                strcpy(broadip.ip, addr);
                for (int i = 0; i < len; i++)
                {
                    if (addr[i] == '.')
                    {
                        j = i;
                    }
                }
                broadip.ip[j + 1] = '2';
                broadip.ip[j + 2] = '5';
                broadip.ip[j + 3] = '5';
            }
        }
    }
    freeifaddrs(ifap);
}

void *host_scaner(void *arg)
{
    static struct user user;
    struct sockaddr_in scan_client, scan_broadcast;
    int scan_socket_client;
    char otvet[16];

    arg = (struct user_list *)malloc(sizeof(struct user_list));
    struct user_list *test = arg;

    memset(&scan_client, 0, sizeof(scan_client));
    scan_client.sin_family = AF_INET;
    scan_client.sin_port = htons(PORT);
    if (!inet_aton(myip.ip, &scan_client.sin_addr))
    {
        perror("inet_aton");
    }

    /**********BROADCAST***************/
    memset(&scan_broadcast, 0, sizeof(scan_broadcast));
    scan_broadcast.sin_family = AF_INET;
    scan_broadcast.sin_port = htons(PORT);
    if (!inet_aton(broadip.ip, &scan_broadcast.sin_addr))
    {
        perror("inet_aton");
    }

    if ((scan_socket_client = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Не удалось создать сокет!\n");
        exit(0);
    }

    int yes = 1;

    if (setsockopt(scan_socket_client, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes)) < 0)
    {
        perror("setsockopt failed\n");
        exit(0);
    }

    if (bind(scan_socket_client, (struct sockaddr *)&scan_client, sizeof(scan_client)) < 0)
    {
        perror("bind error\n");
        close(scan_socket_client);
        exit(1);
    }

    if (sendto(scan_socket_client, myip.ip, strlen(myip.ip) + 1, 0, (struct sockaddr *)&scan_broadcast, sizeof(scan_broadcast)) < 0)
    {
        perror("Ошибка отправки\n");
        exit(1);
    }

    int size_broad = sizeof(scan_broadcast);
    int flag = 1;
    struct pollfd fds;
    fds.fd = scan_socket_client;
    fds.events = POLLIN;

    for (;;)
    {
        int ret = poll(&fds, 1, TIMESEC);

        if (ret == -1)
        {
            break;
        }
        else if (ret == 0)
        {
            break;
        }
        else
        {
            if (fds.revents)
            {

                if (recvfrom(scan_socket_client, otvet, 16, 0, (struct sockaddr *)&scan_broadcast, &size_broad) < 0)
                {
                    perror("Ошибка при приеме сообщения\n");
                    exit(0);
                }
                else
                {
                    strcpy(user.ip, otvet);
                }

                if (flag == 0)
                {
                    user_list_add(&test, user);
                }

                if (flag == 1)
                {
                    new_unit_head(&test, otvet);
                    flag = 0;
                }
                fds.revents = 0;
            }
        }
    }
    close(scan_socket_client);
    return test;
}

void new_unit_head(struct user_list **head, char *ipaddr)
{

    struct user_list *tmp = (struct user_list *)malloc(sizeof(struct user_list));
    strcpy(tmp->user.ip, ipaddr);

    tmp->next = NULL;
    *head = tmp;
}