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
#include "recv_intf.h"
#include "recv_intf.c"


#define PORT 50000 // ПОРТ по которому опрашиваем доступные сервера
#define TIMESEC 1  // кол-во секунд для опроса


//сканер хранит всё для связи с потоком который занимается поиском пользователей
struct scaner
{
    int i;
};
//юзер хранит всю инфу о пользователе, его ip и ник
/*struct user
{
    int i;
    char ip[16];
};*/

/*мои изменения*/
static struct user myip; // структура которая хранит наш ip
static struct user broadip; // структура которая харнит наш бродкаст

void getIP(); // функция через которую  получаем  ip-пользователя и ip-широковещательный
void *host_scaner(void *arg); //функция для потока

/*конец*/

struct user_list
{
    struct user user;
    struct user_list *next;
};

/*dddddddddddddddddddddddddddddd*/
void new_unit_head(struct user_list **head, char *ipaddr);
/*dddddddddddddddddddddddddddddd*/



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
//убивает поток на который ссылается сканер, и 
void stop_scan(struct scaner **scaner)
{
    if (*scaner == NULL){
        return;
    }
    free(*scaner);
    *scaner = NULL;
    return;
}
//запускает поток, сохраняет в сканер всю инфу для связи с ним, если сканер уже ссылается на какой-то поток,
// то необходимо завершить этот поток
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

//возвращает текущий результат работы сканера, если всё готово то сохраняет пользователй в списке,
//если список не пустой на начало запроса, то его следует очистить
enum result get_user_list(struct scaner **scaner, struct user_list **userList)
{
   static struct user_list *test = NULL;
   static struct user_list *test2 = NULL;

if (test != NULL){
    delete_user_list(&test);
    delete_user_list(&test2);
}
    if (*scaner != NULL){
    
/*
    for (int i = 0; i < 10; i++)
    {
        user.i++;
        user_list_add(userList, user);
    }
*/
    pthread_t tid; /* идентификатор потока */
    pthread_create(&tid, NULL, host_scaner, &test); /* создаем новый поток */ // проблема может быть тут?
    pthread_join(tid, (void **)&test2); /* ждем завершения исполнения потока */

   

    *userList = test2;
    }

   

    return READY;

}
//тайбл хранит всё для связи с потоком который занимается получением списка доступный файлов 
//и пользователя у которого этот спискок был запрошен
//хранит имя файла и пользователя от которого это название пришло
/*struct file_name
{
    struct user owner;
    int i;
    char file_name[255];
};*/
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

//лоадер хранит всё что нужно для связи с потоком-загрузчиком 
/*struct loader
{
    char *serv_name;
    int port_num;
    char *file_name;
    int progress;
    pthread_t loader_th;
    pthread_mutex_t hide_from_serv; // закрываем доступ сервера к загружаемому в текущий момент клиентом файлу
    struct sock_attr_cl sock_attrs;
};
//запускает поток загрузчик, принимает имя файла для скачки и в нём есть и сам пользователь
//возващает лоадер который хранит всё что нужно для связи с его потоком
struct loader *start_load(struct file_name file_name)
{
    static int i = 0;
    struct loader *loader = (struct loader *)malloc(sizeof(struct loader));
    return loader;
}
//возвращает готовность загрузски 0-100, -1 - ошибка
//должна срабатывать сразу, ничего не ждать
int load_ready(struct loader **loader)
{
    if(*loader == NULL){
        return -1;
    }
    return 100;
}
//убийство лоадера
void stop_loader(struct loader **loader)
{
    if(*loader == NULL){
        return;
    }
    free(*loader);
    *loader = NULL;
}*/

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

struct loader * front_queue_loaders(struct queue_loaders **head){
    if(*head == NULL){
        return NULL;
    }
    return (*head)->loader;
}
void pop_queue_loaders(struct queue_loaders **head){
    if(*head == NULL){
        return;
    }
    struct queue_loaders *temp = (*head)->next;
    stop_loader(&((*head)->loader));
    free(*head);
    *head = temp;
    return;
}

void delete_queue_loaders(struct queue_loaders **head){
    while(*head != NULL){
        pop_queue_loaders(head);
    }
    return;
}


/*МОИ ДОБАВЛЕНИЯ*/

void getIP()
{
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    int len;        // длина ip-адреса
    int j = 0;      // адрес 3 точки в ip-адресе
    static char *addr;

    getifaddrs(&ifap);
    for (ifa = ifap; ifa; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET)
        {
            sa = (struct sockaddr_in *)ifa->ifa_addr;
            addr = inet_ntoa(sa->sin_addr);

            if (addr[0] == '1' && addr[1] == '9' && addr[2] == '2')
            {
                strcpy(myip.ip, addr); // ipstr заменить на struct user my_ip-> ip
                len = strlen(myip.ip);
                strcpy(broadip.ip, addr); // так же заменить  на переменную в struct user
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
   
   struct user_list *test = arg;

    /***********CLIENT**************/
    memset(&scan_client, 0, sizeof(scan_client));
    scan_client.sin_family = AF_INET;   // Используем протокол IPv4
    scan_client.sin_port = htons(PORT); // используем свободный порт 50000
    if (!inet_aton(myip.ip, &scan_client.sin_addr))
    {
        perror("inet_aton");
    }

    /**********BROADCAST***************/
    memset(&scan_broadcast, 0, sizeof(scan_broadcast));
    scan_broadcast.sin_family = AF_INET;   // используем протокол IPv4
    scan_broadcast.sin_port = htons(PORT); // Используем свободный порт 50000
    if (!inet_aton(broadip.ip, &scan_broadcast.sin_addr))
    {
        perror("inet_aton");
    }
    

    // ************Генерируем socket-client********************
    // AF_INET - используем протокол IPv4
    // SOCK_DGRAM - используем UDP
    if ((scan_socket_client = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Не удалось создать сокет!\n");
        exit(0);
    }

    // Настраиваем возможность броадкаст вещения
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


    // Отправляем серверам сообщение
    if (sendto(scan_socket_client, myip.ip, strlen(myip.ip) + 1, 0, (struct sockaddr *)&scan_broadcast, sizeof(scan_broadcast)) < 0)
    {
        perror("Ошибка отправки\n");
        exit(1);
    }

    // получаем ответ
    int size_broad = sizeof(scan_broadcast);

    // чето делаем
    const ulong timeout = TIMESEC;

    struct timeval tv;
    tv.tv_sec = timeout;

    int flag = 1;
    
    for (;;)
    {
        fd_set rfd;
        FD_ZERO(&rfd);
        FD_SET(scan_socket_client, &rfd);

        // блокируемся пока не получим данные или не истечет время
        int n = select(scan_socket_client + 1, &rfd, 0, 0, &tv);

        if (n < 0)
        {
            break;
        } // произошла ошибка

        if (n == 0)
        {
            break;
        } // выходим из цикла по таймауту

        if (FD_ISSET(scan_socket_client, &rfd))
        {
            if (recvfrom(scan_socket_client, otvet, 16, 0, (struct sockaddr *)&scan_broadcast, &size_broad) < 0)
            {
                perror("Ошибка при приеме сообщения\n");
                exit(0);
            }

            strcpy(user.ip, otvet);

             if (flag == 0){
            user_list_add(&test, user);
            }
            
            if (flag == 1){
            new_unit_head(&test, otvet);// добавляем по его схеме
            flag = 0;
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