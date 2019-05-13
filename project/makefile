SERVER = server.c server.h
SERVER_O = server.o queue.o
QUEUE = queue.c server.h
CLIENT = game.c client.h
CLIENT_O = game.o
CC = cc -std=c99 -Wall -Werror -pedantic

server : $(SERVER_O)
	$(CC) -o server $(SERVER_O)

server.o : $(SERVER)
	$(CC) -c $(SERVER)

queue.o : $(QUEUE)
	$(CC) -c $(QUEUE)

game : $(CLIENT_O)
	$(CC) -o game $(CLIENT_O)

game.o : $(CLIENT)
	$(CC) -c $(CLIENT)

clear : 
	rm -f *.o *.h.gch