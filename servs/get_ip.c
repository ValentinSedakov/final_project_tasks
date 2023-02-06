#include "serv_t.h"

void getIP(void)
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
