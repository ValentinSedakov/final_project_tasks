#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <limits.h>

int nclients = 0;


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