#include "transceiver_intf.h"

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

void* loader_thr(void *arg)
{
    struct to_thread in_thread = *(struct to_thread*)arg;

    in_thread.sock_for_th = sock_creator_cl(&in_thread.sock_for_th,
                                             in_thread.loader_for_th.port_num,
                                             in_thread.loader_for_th.serv_name);

	if (connect(in_thread.sock_for_th.sock_ds,
                (struct sockaddr *)&in_thread.sock_for_th.serv_addr,
                sizeof(in_thread.sock_for_th.serv_addr)) < 0)
	{
		error("An error occurred while establishing connection!");
	}

	recv_file(in_thread.sock_for_th.sock_ds, in_thread.loader_for_th.file_name);
	pthread_exit(0);

}
void recv_file(int sock, char *file_name)
{
	int piece_ctr = 0;
	int bytes_recv;
	char buff[1024];
	size_t file_nm_sz = strlen(file_name) * sizeof(char);

	printf("File name transmission...\n");
	send(sock, file_name, file_nm_sz, 0);
	printf("File name has been transmitted.\n");
	printf("\n");

	FILE *file;
	file = fopen(file_name, "wb");

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
		printf("bytes_recv: %d bytes, segment: %d\n", bytes_recv, piece_ctr);

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

	bzero((char *)&sk_at->serv_addr, sizeof(sk_at->serv_addr));
	sk_at->serv_addr.sin_family = AF_INET;
	bcopy((char *)sk_at->server->h_addr, (char *)&sk_at->serv_addr.sin_addr.s_addr, sk_at->server->h_length);
	sk_at->serv_addr.sin_port = htons(sk_at->portno);

	return *sk_at;
}

struct loader create_loader(const struct user *usr,
							char *file_name)
{
	struct loader loader_exp;
	loader_exp.serv_name = usr->host_nm;
	loader_exp.port_num = usr->port_no;
	loader_exp.file_name = file_name;

	struct sock_attr_cl sock = *loader_exp.sock_attrs;

    struct to_thread to_trd;
    to_trd.loader_for_th = loader_exp;
    to_trd.sock_for_th = sock;
    
    pthread_t loader_th;
    if(pthread_create(&loader_th, NULL, &loader_thr, &to_trd) != 0)
    {
        error("An error occurred while clreating a loader thread!");
    }

    if(pthread_join(loader_th, NULL) != 0)
    {
        error("An error occurred while joining a loader thread!");
    }

	return loader_exp;
}