#include "transceiver_intf.h"


int main(int argc, char *argv[])
{
	printf("\n");
	printf("TCP receiver (client)\n");

	if (argc != 4)
	{
		printf("Usage: %s <hostname> <Port> <File name>\n", argv[0]);
		exit(1);
	}

	struct user usr1 = {argv[1], argv[2]};
	struct loader loader_1 = create_loader(&usr1, argv[3]);

	exit(EXIT_SUCCESS);
}

