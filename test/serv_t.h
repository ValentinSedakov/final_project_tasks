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

void getIP(void);