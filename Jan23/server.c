#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

int nclients = 0;

struct sock_attr_sv
{
	int sock_d;
	int portno;
	struct sockaddr_in serv_addr;
	socklen_t clilen;
	struct sockaddr_in cli_addr;
};

void send_file(int);
struct sock_attr_sv sock_creator_sv(struct sock_attr_sv *, char *);

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

int main(int argc, char *argv[])
{

	printf("\n");
	printf("TCP transmitter (server)\n");

	if (argc != 2)
	{
		printf("Usage: %s <Port>", argv[0]);
		exit(1);
	}
	struct sock_attr_sv sock_sv;
	sock_sv = sock_creator_sv(&sock_sv, argv[1]);

	if (bind(sock_sv.sock_d, (struct sockaddr *)&sock_sv.serv_addr,
			 sizeof(sock_sv.serv_addr)) < 0)
	{
		error("An error occurred while binding a socket!");
	}

	listen(sock_sv.sock_d, 5);
	sock_sv.clilen = sizeof(sock_sv.cli_addr);

	while (1)
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
	return 0;
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
	char buff[1024];
	bzero(buff, sizeof(buff));
	nclients++;
	printusers();

	printf("\n");
	printf("Receiving a file name...\n");
	recv(sock_d, buff, sizeof(buff), 0);
	printf("File name is: %s\n", buff);
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