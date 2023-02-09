MAIN = main
SERV = serv
SRC = ./src
INC = ./inc
BIN = ./bin
MAINFLG = -lncurses
SERVFLG = -g -pthread

all:
	mkdir $(BIN)
	gcc -I$(INC) -o $(BIN)/$(MAIN) $(SRC)/main.c $(MAINFLG)
	gcc -I$(INC) -o $(BIN)/$(SERV) $(SERVFLG) $(SRC)/uni_server.c $(SRC)/serv_intf.c

clean:
	rm -f $(BIN)/$(MAIN)
	rm -f $(BIN)/$(SERV)
	rmdir $(BIN)
