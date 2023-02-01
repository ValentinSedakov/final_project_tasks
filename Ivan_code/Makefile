all:
	gcc -o main main.c -lncurses
	gcc -o serv server.c serv_intf.c
	gcc -o servtest Servertest.c

clean:
	rm -f main
	rm -f serv
	rm -f servtest
