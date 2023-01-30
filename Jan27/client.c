#include "recv_intf.h"

int main(int argc, char *argv[])
{
    printf("\n");
    printf("TCP receiver (client)\n");

    if (argc != 4)
    {
        printf("Usage: %s <hostname> <Port> <File name>\n", argv[0]);
        exit(1);
    }

    // struct user usr1 = {"192.168.0.105", "51000"};
    struct user usr1 = {1, argv[1], argv[2]};
    struct file_name fl_nm = {usr1, 1, argv[3]};
    struct loader *loader_1 = start_load(fl_nm);
    stop_loader(&loader_1);
    // struct loader loader_2 = create_loader(&usr1, "Unit.pdf");

    exit(EXIT_SUCCESS);
}
