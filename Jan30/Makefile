all:
	gcc -o cli client.c -g -pthread recv_intf.c
	gcc -o serv server.c serv_intf.c

clean:
	rm -f cli
	rm -f serv