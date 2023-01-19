#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

int main (int argc, char *argv[])
{
	int my_sock, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buff[1024];

	printf("TCP transmitter\n");

	if (argc != 4)
	{
		printf("Usage: %s <hostname> <Port> <FileNameToCopy>\n", argv[0]);
		exit(1);
	}

	portno = atoi(argv[2]);

	my_sock = socket(AF_INET, SOCK_STREAM, 0);

	if(my_sock < 0)
	{
		error("Error opening socket");
	}

	server = gethostbyname(argv[1]);

	if(server == NULL)
	{
		error("Error, no such host");
	}

	bzero((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;

	bcopy((char*)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

	serv_addr.sin_port = htons(portno);

	if(connect(my_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		error("Error connecting");
	}

    printf("File name transmission...\n");
    send(my_sock, argv[3], sizeof(argv[3]), 0);
    printf("File name has been transmitted.\n");

	FILE* tran_file = fopen(argv[3], "rb");;
	if(tran_file == NULL)
	{
		error("An error occurred while opening file!\n");
		return 1;
	}

	size_t read_piece;
    long size_marker;
    int piece_cnt = 1;

 	while(!feof(tran_file))
 	{
 		read_piece = fread(buff, sizeof(char), sizeof(buff), tran_file);
        buff[read_piece] = 0;

 		if(read_piece < 0)
 		{
 			error("An error occurred while reading!\n");
 			break;
 		}

        size_marker = ftell(tran_file);
        printf("bytes read: %zu bytes, segment: %d, read already: %ld bytes\n", read_piece,
                piece_cnt, size_marker);

 		if(read_piece != 0)
        {
            send(my_sock, buff, read_piece, 0);
            ++piece_cnt;
        }

 	}

 	printf("Done sending file...Disconnect...\n");
 	close(my_sock);
 	return 0;
}