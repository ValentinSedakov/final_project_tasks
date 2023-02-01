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

#define PORT 50000

static char ipstr[16];   // локальный ip-адрес компьютера
static char ipbrc[16];   // широковещательный адресс
static char ipotvet[16]; // записываем ip-адрес того кто нас опрашивает

void getIP();

int main()
{
    struct sockaddr_in scan_server, scan_client, scan_broadcast;
    int scan_socket_client, scan_socket_server, scan_socket_broad, port = 50000;
    char revsend[100];

    getIP(); // получаем Ip адрес.

    //***********CLIENT**************
    memset(&scan_client, 0, sizeof(scan_client));
    scan_client.sin_family = AF_INET;   // Используем протокол IPv4
    scan_client.sin_port = htons(port); // используем свободный порт 50000
    // scan_server.sin_addr.s_addr = inet_addr(ipstr); // Вроде используем наш Ip-адресс
    inet_aton(ipstr, &scan_client.sin_addr);

    //**********BROADCAST***************
    memset(&scan_broadcast, 0, sizeof(scan_broadcast));
    scan_broadcast.sin_family = AF_INET;   // используем протокол IPv4
    scan_broadcast.sin_port = htons(port); // Используем свободный порт 50000
    // scan_broadcast.sin_addr.s_addr = inet_addr(ipbrc); // используем широковещательный адрес
    inet_aton(ipbrc, &scan_broadcast.sin_addr);

    printf("Ip-adres: %s\n", ipstr);
    printf("Broadcast: %s\n", ipbrc);

    // add scan_socket_broad
    // SOCK_DGRAM - используем UDP
    if ((scan_socket_broad = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Не удалось создать сокет!\n");
        exit(0);
    }

    if (bind(scan_socket_broad, (struct sockaddr *)&scan_broadcast, sizeof(scan_broadcast)) < 0)
    {
        perror("bind error\n");
        close(scan_socket_broad);
        exit(1);
    }
    // add scan_socket_server

    if ((scan_socket_server = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Не удалось создать сокет!\n");
        exit(0);
    }

    if (bind(scan_socket_server, (struct sockaddr *)&scan_server, sizeof(scan_server)) < 0)
    {
        perror("bind error\n");
        close(scan_socket_broad);
        exit(1);
    }

    listen(scan_socket_server, 4);
    listen(scan_socket_broad, 4);

    int j = 0;
    int k = 1;
    while (k)
    {
        int client = sizeof(scan_broadcast);

        if (recvfrom(scan_socket_broad, revsend, 100, 0, (struct sockaddr *)&scan_broadcast, &client) < 0)
        {
            perror("Ошибка при приеме сообщения\n");
            close(scan_socket_server);
            exit(0);
        }
        else
        {
            j++;
            printf("%d: %s\n", j, revsend);
            //k = 0;
        }

 

        strcpy(ipotvet, revsend);
        printf("НУЖНО ОТВЕТИТЬ %s\n", ipotvet);
        //************SERVER*************
        memset(&scan_server, 0, sizeof(scan_server));
        scan_server.sin_family = AF_INET;   // используем протокол IPv4
        scan_server.sin_port = htons(PORT); // Используем свободный порт 50000
        // scan_server.sin_addr.s_addr = inet_addr(ipstr); // используем собственный адрес
        inet_aton(ipotvet, &scan_server.sin_addr);

        if (sendto(scan_socket_server, ipstr, strlen(ipstr), 0, (struct sockaddr *)&scan_server, sizeof(scan_server)) < 0)
        {
            printf("ОШИБКА\n");
            // perror("Ошибка при отправке сообщения\n");
            // close(scan_socket);
            // exit(0);
        }

        printf("Ovet send! \n");
    }

    close(scan_socket_server);
    close(scan_socket_client);
    close(scan_socket_broad);
}



// работает только для частынх сетей класса С 192.168.0.0 с маской 24

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
                strcpy(ipstr, addr); // ipstr заменить на struct user my_ip-> ip
                len = strlen(ipstr);
                strcpy(ipbrc, addr); // так же заменить  на переменную в struct user
                for (int i = 0; i < len; i++)
                {
                    if (addr[i] == '.')
                    {
                        j = i;
                    }
                }

                ipbrc[j + 1] = '2';
                ipbrc[j + 2] = '5';
                ipbrc[j + 3] = '5';
            }
        }
    }
    freeifaddrs(ifap);
}





