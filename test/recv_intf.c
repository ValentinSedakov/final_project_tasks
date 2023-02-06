#include "recv_intf.h"

const int trn_port = 51000;
const int name_port = 50500;


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

    //printf("File name transmission...\n");
    send(sock, loader->file_name, file_nm_sz, 0);
    //printf("File name has been transmitted.\n");
    //printf("\n");

    //printf("Receiving a file size...\n");
    bzero(buff, sizeof(buff));
    recv(sock, buff, sizeof(buff), 0);
    file_size = strtoul(buff, NULL, 10);

    if(0 >= file_size)
    {
        printf("File does not exist or size > 4Gb! Aborted!\n");
        return;
    }

    //printf("File size received: %lu\n", file_size);

    int num_of_segm = num_of_segs(file_size, sizeof(buff));
    //printf("%d segments (%zu bytes) in the file.\n", num_of_segm, sizeof(buff));
    //printf("\n");

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
		//printf("bytes_recv = %d", bytes_recv);
		//printf("buff = %s", buff);
		//sleep(3);
        if (bytes_recv < 0)
        {
            error("An error occurred while reading from socket!\n");
            return;
        }

        fwrite(buff, sizeof(char), bytes_recv, file);
        ++piece_ctr;
        loader->progress = (int)(100.0 * ((double)piece_ctr / (double)num_of_segm) - 1);

        if (bytes_recv < sizeof(buff))
        {
        	//fwrite(buff, sizeof(char), bytes_recv, file);
            break;
        }
    }
    //fwrite("Nigga", sizeof(char), 6, file);
    fclose(file);
    close(sock);
    loader->progress = (int)(100.0 * ((double)piece_ctr / (double)num_of_segm));
}

struct sock_attr_cl sock_creator_cl(struct sock_attr_cl *sk_at,
                                    char *str_host_nm)
{
    sk_at->portno = trn_port;
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

struct sock_attr_cl sock_creator_names(struct sock_attr_cl *sk_at, char *str_host_nm)
{
    sk_at->portno = name_port;
    sk_at->sock_ds = socket(AF_INET, SOCK_STREAM, 0);
    sk_at->server = gethostbyname(str_host_nm);

    if (sk_at->sock_ds < 0)
    {
        perror("An error occurred while creating a socket!");
    }

    if (sk_at->server == NULL)
    {
        perror("Error, such host does not exist!");
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
    loader->serv_name = fl_nm.owner.ip;
    loader->port_num = trn_port;
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

void *table_thr(void *arg)
{
    struct to_names_thread *in_thread = arg;

    in_thread->sock_for_th = sock_creator_names(&in_thread->sock_for_th,
                                            in_thread->table_for_th->serv_name);


    if(connect(in_thread->sock_for_th.sock_ds, (struct sockaddr *)&in_thread->sock_for_th.serv_addr, sizeof(in_thread->sock_for_th.serv_addr)) < 0)
    {
        perror("Error during connection establishment!\n");
        in_thread->table_for_th->status = FAIL;
        pthread_exit(0);
    }

    in_thread->table_for_th->status = WORK;
    recieve_filenames(in_thread->sock_for_th.sock_ds, in_thread->table_for_th);

    in_thread->table_for_th->status = READY;
    struct table *table_in_thread = in_thread->table_for_th;
    //pthread_exit(0);
    return table_in_thread;
}

void start_table(struct table **table, struct user user)
{
    struct table *temp_table = NULL;

    if (*table != NULL)
    {
        stop_table(table);
    }

    *table = (struct table *)malloc(sizeof(struct table));
    strcpy((*table)->serv_name, user.ip); // bzero?
    (*table)->port_num = name_port;

    struct sock_attr_cl *socket = &((*table)->sock_attrs);


    struct to_names_thread *to_thr = (struct to_names_thread *)malloc(sizeof(to_thr));
    to_thr->table_for_th = *table;
    to_thr->sock_for_th = *socket;
    to_thr->table_for_th->status = WORK;
    to_thr->table_for_th->list = NULL;

    if(pthread_create(&(*table)->loader_th, NULL, &table_thr, to_thr) != 0)
    {
        perror("An error during thread creation!\n");
        to_thr->table_for_th->status = FAIL;
    }

    if(pthread_join((*table)->loader_th, (void **)&temp_table) != 0)
    {
        perror("error during joining thread!");
        to_thr->table_for_th->status= FAIL;
    }

    to_thr->table_for_th = temp_table;
    *table = to_thr->table_for_th;
    return;
}

void stop_table(struct table **table)
{
    if(*table == NULL)
    {
        return;
    }

    if(pthread_cancel((*table)->loader_th) != 0)
    {
        perror("An error occured during thread canceling!\n");
    }

    free(*table);
    *table = NULL;
}

char *recieve_filenames(int socket, struct table *table)
{
    char filenames[2000];
    bzero(filenames, sizeof(filenames));

    send(socket, table->serv_name, sizeof(table->serv_name), 0);

    recv(socket, filenames, sizeof(filenames), 0);

    table->list = get_name_for_list(&table->list, table->serv_name, filenames);
}

void delete_file_name_list(struct file_name_list **head)
{
    if (*head == NULL)
    {
        return;
    }

    struct file_name_list *temp = *head;

    while (temp != NULL)
    {
        struct file_name_list *del = temp;
        temp = temp->next;
        free(del);
    }

    *head = NULL;

    return;
}

enum result get_file_name_list (struct table **table, struct file_name_list **file_namelist)
{
    delete_file_name_list(file_namelist);
    if(*table != NULL)
    {
        if((*table)->status == WORK)
        {
            return WORK;
        }
        else if((*table)->status == FAIL)
        {
            return FAIL;
        }
        else
        {
            *file_namelist = (*table)->list;
            return READY;
        }
    }
}

struct file_name_list *get_name_for_list(struct file_name_list **head, char *owner_ip, char *names)
{
    char buff[255];

	int i = 0;
	int j = 0;
	int file_number = 1;


	while (names[i] != '\0')
	{
		if(names[i] == '\n')
		{
            struct file_name temp;

            sprintf(temp.file_name ,"%s", buff);
			temp.i = file_number;
			sprintf(temp.owner.ip,"%s",owner_ip);

			table_list_add(head, temp);
            memset(&temp, 0, sizeof(struct file_name));

			i++;
			file_number++;
			
			bzero(buff, sizeof(buff));
			j = 0;

			continue;
		}

		buff[j] = names[i];

		i++;
		j++;
	}

    return *head;
}
                                             
struct file_name_list *new_file_name_list(struct file_name file_name)
{
    struct file_name_list *res = (struct file_name_list *)malloc(sizeof(struct file_name_list));
    res->next = NULL;
    res->file_name = file_name;
    return res;
}

void table_list_add(struct file_name_list **head, struct file_name file_name)
{
    if (*head == NULL)
    {
        *head = new_file_name_list(file_name);
        return;
    }

    struct file_name_list *temp = *head;

    while (temp->next != NULL)
    {
        temp = temp->next;
    }

    temp->next = new_file_name_list(file_name);
    return;
}

void show_file_name_list(struct file_name_list **head)
{
	int i = 1;

    if (*head == NULL)
    {
        return;
    }

    struct file_name_list *temp = *head;

    while (temp != NULL)
    {
		i++;
        temp = temp->next;
    }

    return;
}

/*void new_unit_head(struct file_name_list **head, char *ipaddr)
{

    struct file_name_list *tmp = (struct file_name_list *)malloc(sizeof(struct file_name_list));
    strcpy(tmp->user.ip, ipaddr);

    tmp->next = NULL;
    *head = tmp;
}*/