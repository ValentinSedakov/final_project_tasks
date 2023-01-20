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

void CopyFile(int, char*);

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

void printusers()
{
	if(nclients)
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
	char file_name[1024];
	int sockfd, newsockfd1;
	int portno;
	int pid;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;

	printf("TCP receiver\n");

	if(argc != 2)
	{
		printf("Usage: %s <Port>", argv[0]);
		exit(1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		error("An error occurred while creating a socket!");
	}

	bzero((char*) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		error("An error occurred while binding a socket!");
	}

	listen(sockfd, 5);
	clilen = sizeof(cli_addr);

	while(1)
	{
		newsockfd1 = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd1 < 0)
		{
			error("An error occurred while accepting connection!");
		}

		printf("\n");
		printf("Receiving a file name...\n");
		recv(newsockfd1, file_name, sizeof(file_name), 0);
		printf("File name is: %s\n", file_name);
		printf("\n");

		struct hostent *hst;
		hst = gethostbyaddr((char*)&cli_addr.sin_addr, 4, AF_INET);

		printf("Added %s [%s] new connection\n", (hst) ? hst->h_name : "Unknown host",
		 	   (char*)inet_ntoa(cli_addr.sin_addr));

		pid = fork();
		if(pid < 0)
		{
			error("An error occurred while creating a new parallel connection!");
		}
		if(pid == 0)
		{
			close(sockfd);
			CopyFile(newsockfd1, file_name);
			exit(0);
		}
		else
		{
			close(newsockfd1);
		}
	}

	close(sockfd);
	return 0;
}

void CopyFile (int sock, char *file_name)
{
	int piece_ctr = 0;
	int bytes_recv;
	char buff[1024];
	nclients++;
	printusers();

	FILE* file;
	file = fopen(file_name, "wb");
	if (file == NULL)
	{
		error("An error occurred while creating a file copy!");
		return;
	}

	while(1)
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

		if(bytes_recv < sizeof(buff))
		{
			break;
		}
	}


	nclients--;
	printf("\n");
	printf("-disconnecting\n");
	printusers();
	return;
}