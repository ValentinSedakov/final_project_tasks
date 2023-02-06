#include "serv_intf.h"
#include "serv_t.h"

struct user myip;
struct user otvetip;
struct user broadip;

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

            if (addr[0] == '1' && addr[1] == '9' && addr[2] == '2')
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

void *serv_file_send(void *arg)
{
    struct sock_attr_sv sock_sv;
    sock_sv = sock_creator_sv(&sock_sv);

    if (bind(sock_sv.sock_d, (struct sockaddr *)&sock_sv.serv_addr,
             sizeof(sock_sv.serv_addr)) < 0)
    {
        error("An error occurred while binding a socket!");
    }

    listen(sock_sv.sock_d, 5);
    sock_sv.clilen = sizeof(sock_sv.cli_addr);
    int work = 1;

    while (work)
    {
        int newsockfd = accept(sock_sv.sock_d, (struct sockaddr *)&sock_sv.cli_addr,
                               &sock_sv.clilen);
        if (newsockfd < 0)
        {
            error("An error occurred while accepting connection!");
        }

        struct hostent *hst;
        hst = gethostbyaddr((char *)&sock_sv.cli_addr.sin_addr, 4, AF_INET);

        printf("Added %s [%s] new connection\n", (hst) ? hst->h_name : "Unknown host",
               (char *)inet_ntoa(sock_sv.cli_addr.sin_addr));

        int pid = fork();
        if (pid < 0)
        {
            error("An error occurred while creating a new parallel connection!");
        }
        if (pid == 0)
        {
            close(sock_sv.sock_d);
            send_file(newsockfd);

            exit(0);
        }
        else
        {
            close(newsockfd);
        }
    }

    close(sock_sv.sock_d);
}

void *serv_send_addr(void *arg)
{
    struct sockaddr_in scan_server, scan_broadcast;
    int scan_socket_server, scan_socket_broad, port = 50000;
    char revsend[16];

    memset(&scan_broadcast, 0, sizeof(scan_broadcast));
    scan_broadcast.sin_family = AF_INET;
    scan_broadcast.sin_port = htons(port);
    inet_aton(broadip.ip, &scan_broadcast.sin_addr);

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

    int k = 1;
    while (k)
    {
        int client = sizeof(scan_broadcast);

        if (recvfrom(scan_socket_broad, revsend, 16, 0, (struct sockaddr *)&scan_broadcast, &client) < 0)
        {
            perror("Ошибка при приеме сообщения\n");
            close(scan_socket_server);
            exit(0);
        }
    

        strcpy(otvetip.ip, revsend);
        memset(&scan_server, 0, sizeof(scan_server));
        scan_server.sin_family = AF_INET;
        scan_server.sin_port = htons(PORT);
        inet_aton(otvetip.ip, &scan_server.sin_addr);

        if (sendto(scan_socket_server, myip.ip, strlen(myip.ip), 0, (struct sockaddr *)&scan_server, sizeof(scan_server)) < 0)
        {
            perror("Ошибка при отправке сообщения\n");
            exit(0);
        }

    }

    close(scan_socket_server);
    close(scan_socket_broad);
}

void *serv_send_flist(void *arg)
{

    struct sock_attr_sv sock_names;
    sock_names = sock_creator_names(&sock_names);

    if (bind(sock_names.sock_d, (struct sockaddr *)&sock_names.serv_addr,
             sizeof(sock_names.serv_addr)) < 0)
    {
        error("An error occurred while binding NAMES socket!");
    }

    listen(sock_names.sock_d, 2);
    int work = 1;

    while (work)
    {
        int names_sock = accept(sock_names.sock_d, (struct sockaddr *)&sock_names.cli_addr,
                                &sock_names.clilen);
        if (names_sock < 0)
        {
            error("An error occurred while accepting connection!");
        }

        send_names(names_sock);
        close(names_sock);
    }
    close(sock_names.sock_d);
}

int main(int argc, char *argv[])
{

    getIP();

    /* НА УДАЛЕНИЕ */
    printf("Server IP = %s", myip.ip);
    printf("\n");
    printf("TCP transmitter (server)\n");
    /* НА УДАЛЕНИЕ */

    pthread_t serv_fil_snd;
    pthread_t serv_sd_addr;
    pthread_t serv_sd_flist;

    if (pthread_create(&serv_sd_addr, NULL, &serv_send_addr, NULL) != 0)
    {
        error("An error occurred while clreating a server thread!");
    }

    if (pthread_create(&serv_sd_flist, NULL, &serv_send_flist, NULL) != 0)
    {
        error("An error occurred while clreating a server thread!");
    }

    if (pthread_create(&serv_fil_snd, NULL, &serv_file_send, NULL) != 0)
    {
        error("An error occurred while clreating a server thread!");
    }

    if (pthread_join(serv_sd_addr, NULL) != 0)
    {
        error("An error occurred while joining a loader thread!");
    }

    if (pthread_join(serv_sd_flist, NULL) != 0)
    {
        error("An error occurred while joining a loader thread!");
    }

    if (pthread_join(serv_fil_snd, NULL) != 0)
    {
        error("An error occurred while joining a loader thread!");
    }

    return 0;
}