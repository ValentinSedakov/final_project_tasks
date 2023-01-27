#include "serv_intf.h"

int nclients = 0;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void printusers()
{
    if (nclients)
    {
        printf("Users on-line: %d\n", nclients);
    }
    else
    {
        printf("Noone is on-line");
    }
}

struct sock_attr_sv sock_creator_sv(struct sock_attr_sv *sk_at,
                                    char *portno_str)
{
    sk_at->sock_d = socket(AF_INET, SOCK_STREAM, 0);
    if (sk_at->sock_d < 0)
    {
        error("An error occurred while creating a socket!");
    }

    bzero((char *)&sk_at->serv_addr, sizeof(sk_at->serv_addr));
    sk_at->portno = atoi(portno_str);
    sk_at->serv_addr.sin_family = AF_INET;
    sk_at->serv_addr.sin_addr.s_addr = INADDR_ANY;
    sk_at->serv_addr.sin_port = htons(sk_at->portno);

    return *sk_at;
}

void send_file(int sock_d)
{
    char buff[4096];
    char *file_size;
    bzero(buff, sizeof(buff));
    nclients++;
    printusers();

    printf("\n");
    printf("Receiving a file name...\n");
    recv(sock_d, buff, sizeof(buff), 0);
    printf("File name is: %s\n", buff);
    printf("\n");

    printf("Sending a file size...\n");
    file_size = get_file_size(buff);
    if (file_size == NULL) // проверка размера файла, не более 4 Гб!
    {
        return;
    }
    send(sock_d, file_size, sizeof(file_size), 0);
    printf("File size sent. %s bytes\n", file_size);
    printf("\n");

    FILE *tran_file = fopen(buff, "rb");
    if (tran_file == NULL)
    {
        error("An error occurred while opening file!\n");
        return;
    }

    size_t read_piece;
    long size_marker;
    int piece_cnt = 1;

    while (!feof(tran_file))
    {
        read_piece = fread(buff, sizeof(char), sizeof(buff), tran_file);
        buff[read_piece] = 0;

        if (read_piece < 0)
        {
            error("An error occurred while reading a file segment!\n");
            break;
        }

        size_marker = ftell(tran_file);
        printf("bytes read: %zu bytes, segment: %d, read already: %ld bytes\n", read_piece,
               piece_cnt, size_marker);

        if (read_piece != 0)
        {
            send(sock_d, buff, read_piece, 0);
            ++piece_cnt;
        }
    }

    printf("\n");
    printf("The file has been successfully sent!  Disconnecting...\n");
    nclients--;
    printf("\n");
    printf("-Disconnected\n");
    printusers();
    return;
}

char *get_file_size(const char *file_name)
{
    int file_size = 0;
    static char fl_sz_str[1024];
    struct stat file_stat_buff;
    int fd = open(file_name, O_RDONLY);

    if (fd == -1)
    {
        file_size = -1;
    }
    else
    {
        if ((fstat(fd, &file_stat_buff) != 0) || (!S_ISREG(file_stat_buff.st_mode)))
        {
            file_size = -1;
        }
        else
        {
            file_size = file_stat_buff.st_size;
        }
        close(fd);
    }
    if (file_size > ULONG_MAX)
    {
        printf("Forbidden file size! Max = 4Gb! Aborted!\n");
        return NULL;
    }
    sprintf(fl_sz_str, "%d", file_size);
    return fl_sz_str;
}