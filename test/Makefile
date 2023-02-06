all:
	gcc -o main main.c -lncurses
	gcc -o serv -g -pthread uni_server.c serv_intf.c

clean:
	rm -f main
	rm -f serv
