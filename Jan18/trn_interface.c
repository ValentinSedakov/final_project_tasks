#include "trn_interface.h"


int fill_from_input(const struct transmitter_input *input,
                      struct transmitter *trmter)
{
    if(!input->file_name || !input->port_number || !input->host_name)
    {
        perror("Input error: check input! NO file name or port num, or server name!");
        exit(EXIT_FAILURE);
    }

    trmter->file_name = input->file_name;
    trmter->port_number = strtol(input->port_number, NULL, 10);
    trmter->server_name = gethostbyname(input->host_name);

    exit(EXIT_SUCCESS);
}

int socket_establisher(struct transmitter *trmter)
{
    trmter->current_socket = socket(AF_INET, SOCK_STREAM, 0);

    if(trmter->current_socket < 0)
    {
        perror("An error occurred while creating a socket!");
        exit(EXIT_FAILURE);
    }

    if(!trmter->server_name)
    {
        perror("An error occurred while obtaining a server name!");
        exit(EXIT_FAILURE);
    }

    bzero((char*) &(trmter->server_address), sizeof(trmter->server_address));
    trmter->server_address.sin_family = AF_INET;

    bcopy((char*)(trmter->server_name->h_addr), 
          (char*) &(trmter->server_address.sin_addr.s_addr),
          trmter->server_name->h_length);

    trmter->server_address.sin_port = htons(trmter->port_number);
    
    if(connect(trmter->current_socket, (struct sockaddr*)&(trmter->server_address),
        sizeof(trmter->server_address)) < 0)
    {
        perror("An error occurred while establishing connection!");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

int file_transmission(struct transmitter *trmter)
{
    size_t read_piece;
    long  sz_marker;
    int piece = 1;
    FILE *to_tran = fopen(trmter->file_name, "rb");

    if(!to_tran)
    {
        perror("An error occurred while opening file!");
        exit(EXIT_FAILURE);
    }

    while(!feof(to_tran))
    {
        read_piece = fread(trmter->buffer, sizeof(char),
                                  sizeof(trmter->buffer), to_tran);
        if(read_piece < 0)
        {
            perror("An error occurred while reading a file fragment!");
            exit(EXIT_FAILURE);
        }
        sz_marker = ftell(to_tran);

        if(-1L == sz_marker)
        {
            perror("An error occurred during the file transmission!");
            exit(EXIT_FAILURE);
        }

        printf("Bytes read: %zu, piece:%d, already read: %ld \n", read_piece,
                piece, sz_marker);

        if(read_piece)
        {
            send(trmter->current_socket, trmter->buffer, read_piece, 0);
            ++piece;
        }
    }
    printf("File has been succesfully sent.\n");
    close(trmter->current_socket);
    exit(EXIT_SUCCESS);
}

int main()
{
    return 0;
}