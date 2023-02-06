#include "serv_intf.h"

int nclients = 0;
const int trn_serv_port = 51000;
const int name_serv_port = 50500;

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
        printf("Noone is on-line\n");
    }
}

struct sock_attr_sv sock_creator_sv(struct sock_attr_sv *sk_at)
{
    sk_at->sock_d = socket(AF_INET, SOCK_STREAM, 0);
    if (sk_at->sock_d < 0)
    {
        error("An error occurred while creating a socket!");
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
    long long file_size = 0;
    static char fl_sz_str[1024];
    struct stat file_stat_buff;
    int fd = open(file_name, O_RDONLY);

    if (fd == -1)
    {
        printf("File does not exist! Aborted!\n");
        return NULL;
    }
    else
    {
        if ((fstat(fd, &file_stat_buff) != 0) || (!S_ISREG(file_stat_buff.st_mode)))
        {
            printf("An error occurred while reading a file size! Aborted!\n");
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
        printf("Forbidden file size! Max = 4Gb! Aborted!\n");
        return NULL;
    }
    sprintf(fl_sz_str, "%lli", file_size);
    return fl_sz_str;
}

void send_names(int sock_send, char *path)
{
    char *filenames;
    char recv_line[16];

    printf("\n Recieved a line: ");
    recv(sock_send, recv_line, sizeof(recv_line), 0);
    printf("<%s>\n\n", recv_line);

    printf("sending filenames...\n");
    filenames = get_filenames(path);
    printf("filenames to send:\n%s", filenames);
    send(sock_send, filenames, strlen(filenames), 0);
    printf("filenames sent!\n");

    return;
}

char *get_filenames(char *path)
{
    char filenames[2000];
    bzero(filenames, sizeof(filenames));

    DIR *directory = opendir(path);
	
    char file_name_buff[257];
    int length = 0;

    if(directory==0)
    {
        perror("there was an error during opening\n");
    }

    struct dirent *dir;

    while ((dir = readdir(directory)) != NULL)
    {
        if(dir->d_type != DT_DIR)
        {
            sprintf(file_name_buff, "%s\n", dir->d_name);
            length +=(1 + strlen(dir->d_name));
            strcat(filenames, file_name_buff);
        }
    }

    printf("Length of names is: %d\n", length);
    
    char *to_send = (char *)malloc(length);
    sprintf(to_send, "%s", filenames);
    printf ("<function will return:\n%s>\n", to_send);
    closedir(directory);
    return to_send;
}

