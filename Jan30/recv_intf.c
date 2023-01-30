#include "recv_intf.h"

void error(const char *msg)
{
    perror(msg);
    exit(-1);
}

void *loader_thr(void *arg)
{
    struct to_thread in_thread = *(struct to_thread *)arg;

    in_thread.sock_for_th =
        sock_creator_cl(&in_thread.sock_for_th,
                        in_thread.loader_for_th.port_num,
                        in_thread.loader_for_th.serv_name);

    if (connect(in_thread.sock_for_th.sock_ds,
                (struct sockaddr *)&in_thread.sock_for_th.serv_addr,
                sizeof(in_thread.sock_for_th.serv_addr)) < 0)
    {
        error("An error occurred while establishing connection!");
    }

    pthread_mutex_lock(&(in_thread.loader_for_th.hide_from_serv));
    recv_file(in_thread.sock_for_th.sock_ds, &in_thread.loader_for_th); // Во время получения файла, сервер не может
    pthread_mutex_unlock(&(in_thread.loader_for_th.hide_from_serv));	// получить к нему доступ и, тем
                                                                        // самым, помешать записи файла
    pthread_exit(0);
}
void recv_file(int sock, struct loader *loader)
{
    int piece_ctr = 0;
    int bytes_recv;
    unsigned long file_size;
    char buff[4096];
    size_t file_nm_sz = strlen(loader->file_name) * sizeof(char);

    printf("File name transmission...\n");
    send(sock, loader->file_name, file_nm_sz, 0);
    printf("File name has been transmitted.\n");
    printf("\n");

    printf("Receiving a file size...\n");
    bzero(buff, sizeof(buff));
    recv(sock, buff, sizeof(buff), 0);
    file_size = strtoul(buff, NULL, 10);
    printf("File size received: %lu\n", file_size);

    int num_of_segm = num_of_segs(file_size, sizeof(buff));
    printf("%d segments (%zu bytes) in the file.\n", num_of_segm, sizeof(buff));
    printf("\n");

    FILE *file;
    file = fopen(loader->file_name, "wb");

    if (file == NULL)
    {
        error("An error occurred while creating a file copy!");
        return;
    }

    while (1)
    {
        bytes_recv = recv(sock, buff, sizeof(buff), 0);

        if (bytes_recv < 0)
        {
            error("An error occurred while reading from socket!\n");
            return;
        }

        fwrite(buff, sizeof(char), bytes_recv, file);
        ++piece_ctr;
        loader->progress = (int)(100.0 * ((double)piece_ctr / (double)num_of_segm));
        printf("bytes_recv: %d bytes, segment: %d/%d (%d %%)\n", bytes_recv,
               piece_ctr, num_of_segm, loader->progress);

        printf("---------------------------------------------------------------\n");
        printf("Test of load ready func:              %d\n", load_ready(&loader));
        printf("---------------------------------------------------------------\n");

        if (bytes_recv < sizeof(buff))
        {
            break;
        }
    }
    close(sock);
}

struct sock_attr_cl sock_creator_cl(struct sock_attr_cl *sk_at,
                                    char *str_portno,
                                    char *str_host_nm)
{
    sk_at->portno = atoi(str_portno);
    sk_at->sock_ds = socket(AF_INET, SOCK_STREAM, 0);
    sk_at->server = gethostbyname(str_host_nm);

    if (sk_at->sock_ds < 0)
    {
        error("An error occurred while creating a socket!");
    }

    if (sk_at->server == NULL)
    {
        error("Error, such host does not exist!");
    }

	bzero((char *)&sk_at->serv_addr,
          sizeof(sk_at->serv_addr));

    sk_at->serv_addr.sin_family = AF_INET;

    bcopy((char *)sk_at->server->h_addr,
          (char *)&sk_at->serv_addr.sin_addr.s_addr,
          sk_at->server->h_length);

    sk_at->serv_addr.sin_port = htons(sk_at->portno);

    return *sk_at;
}

struct loader *start_load(struct file_name fl_nm)
{
    static int i = 0;
    struct loader *loader = (struct loader *)malloc(sizeof(struct loader));
    loader->serv_name = fl_nm.owner.host_nm;
    loader->port_num = fl_nm.owner.port_no;
    loader->file_name = fl_nm.file_name;

    struct sock_attr_cl *sock = &(loader->sock_attrs);

    pthread_mutex_init(&(loader->hide_from_serv), NULL);
    struct to_thread to_trd;
    to_trd.loader_for_th = *loader;
    to_trd.sock_for_th = *sock;

    if (pthread_create(&loader->loader_th, NULL, &loader_thr, &to_trd) != 0)
    {
        error("An error occurred while clreating a loader thread!");
    }

    if (pthread_join(loader->loader_th, NULL) != 0)
    {
        error("An error occurred while joining a loader thread!");
    }

    pthread_mutex_destroy(&(loader->hide_from_serv));

    return loader;
}

int num_of_segs(unsigned long file_sz, size_t seg_val)
{
    int num_of_segs;

    if (file_sz % seg_val)
    {
        num_of_segs = ((file_sz / seg_val) + 1);
    }
    else
    {
        num_of_segs = (file_sz / seg_val);
    }
    return num_of_segs;
}

int load_ready(struct loader **loader)
{
    if (*loader == NULL)
    {
        return -1;
    }
    return (*loader)->progress;
}

void stop_loader(struct loader **loader)
{
    if (*loader == NULL)
    {
        return;
    }

    if (pthread_cancel((*loader)->loader_th) != 0)
    {
        error("An error occurred while canceling a thread!");
    }
    else
    {
        pthread_exit(0);
    }

    pthread_mutex_destroy(&((*loader)->hide_from_serv));
    free(*loader);
    *loader = NULL;
}
