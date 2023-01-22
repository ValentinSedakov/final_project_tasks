#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

pthread_mutex_t file_mutex;

struct sock_attr_cl
{
	int sock_ds;
	int portno;
	struct sockaddr_in serv_addr;
	struct hostent *server;
};

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

void recv_file(int, char *);
struct sock_attr_cl sock_creator_cl(struct sock_attr_cl *, char *, char *);

int main(int argc, char *argv[])
{
	pthread_mutex_init(&file_mutex, NULL);
	printf("\n");
	printf("TCP receiver (client)\n");

	if (argc != 4)
	{
		printf("Usage: %s <hostname> <Port> <File name>\n", argv[0]);
		exit(1);
	}
	struct sock_attr_cl sock;
	sock = sock_creator_cl(&sock, argv[2], argv[1]);

	if (connect(sock.sock_ds, (struct sockaddr *)&sock.serv_addr, sizeof(sock.serv_addr)) < 0)
	{
		error("An error occurred while establishing connection!");
	}

	recv_file(sock.sock_ds, argv[3]);
	pthread_mutex_destroy(&file_mutex);
	return 0;
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
	pthread_mutex_lock(&file_mutex);

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
	pthread_mutex_unlock(&file_mutex);

	close(sock);
}