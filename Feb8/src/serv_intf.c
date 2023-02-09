#include "serv_intf.h"

const int trn_serv_port = 51000;
const int name_serv_port = 50500;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

struct sock_attr_sv sock_creator_sv(struct sock_attr_sv *sk_at)
{
    sk_at->sock_d = socket(AF_INET, SOCK_STREAM, 0);
    if (sk_at->sock_d < 0)
    {
        error("An error occurred while creating a socket!");
    }

    int yes = 1;

    if (setsockopt(sk_at->sock_d, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR | SO_REUSEPORT, &yes, sizeof(yes)) < 0)
    {
        perror("setsockopt failed\n");
        exit(0);
    }

    bzero((char *)&sk_at->serv_addr, sizeof(sk_at->serv_addr));
    sk_at->portno = trn_serv_port;
    sk_at->serv_addr.sin_family = AF_INET;
    sk_at->serv_addr.sin_addr.s_addr = INADDR_ANY;
    sk_at->serv_addr.sin_port = htons(sk_at->portno);

    return *sk_at;
}

struct sock_attr_sv sock_creator_names(struct sock_attr_sv *sk_at)
{
    sk_at->sock_d = socket(AF_INET, SOCK_STREAM, 0);
    if (sk_at->sock_d < 0)
    {
        error("An error occurred while creating a socket!");
    }

    int yes = 1;
    if (setsockopt(sk_at->sock_d, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR | SO_REUSEPORT, &yes, sizeof(yes)) < 0)
    {
        perror("setsockopt failed\n");
        exit(0);
    }

    bzero((char *)&sk_at->serv_addr, sizeof(sk_at->serv_addr));
    sk_at->portno = name_serv_port;
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

    recv(sock_d, buff, sizeof(buff), 0);

    file_size = get_file_size(buff);
    if (file_size == NULL) // проверка размера файла, не более 4 Гб!
    {
        return;
    }
    send(sock_d, file_size, sizeof(file_size), 0);

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

        if (read_piece != 0)
        {
            send(sock_d, buff, read_piece, 0);
            ++piece_cnt;
        }
    }

    return;
}

char *get_file_size(const char *file_name)
{
    long long file_size = 0;
    static char fl_sz_str[1024];
    struct stat file_stat_buff;
    int fd = open(file_name, O_RDONLY);

    if (fd == -1)
    {
        return NULL;
    }
    else
    {
        if ((fstat(fd, &file_stat_buff) != 0) || (!S_ISREG(file_stat_buff.st_mode)))
        {
            // printf("An error occurred while reading a file size! Aborted!\n");
            return NULL;
        }
        else
        {
            file_size = file_stat_buff.st_size;
        }
        close(fd);
    }
    if (file_size > MAX_FILE_SZ)
    {
        // printf("Forbidden file size! Max = 4Gb! Aborted!\n");
        return NULL;
    }
    sprintf(fl_sz_str, "%lli", file_size);
    return fl_sz_str;
}

void send_names(int sock_send)
{
    char *filenames;
    char recv_line[16];

    recv(sock_send, recv_line, sizeof(recv_line), 0);
    filenames = get_filenames();
    send(sock_send, filenames, strlen(filenames), 0);

    return;
}

char *get_filenames(void)
{
    char filenames[2000];
    char path[2] = "./";
    bzero(filenames, sizeof(filenames));

    DIR *directory = opendir(path);

    char file_name_buff[257];
    int length = 0;

    if (directory == 0)
    {
        perror("there was an error during opening\n");
    }

    struct dirent *dir;

    while ((dir = readdir(directory)) != NULL)
    {
        if (dir->d_type != DT_DIR)
        {
            sprintf(file_name_buff, "%s\n", dir->d_name);
            length += (1 + strlen(dir->d_name));
            strcat(filenames, file_name_buff);
        }
    }


    char *to_send = (char *)malloc(length);
    sprintf(to_send, "%s", filenames);
    closedir(directory);
    return to_send;
}
