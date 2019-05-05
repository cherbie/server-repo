SERVER = server.c server.h
SERVER_O = server.o 
CLIENT = client_game.c client.h
CLIENT_O = client_game.o
CC = cc -std=c99 -Wall -Werror -pedantic

server : $(SERVER_O)
	$(CC) -o server $(SERVER_O)

server.o : $(SERVER)
	$(CC) -c $(SERVER)

client : $(CLIENT_O)
	$(CC) -o client $(CLIENT_O)

client.o : $(CLIENT)
	$(CC) -c $(CLIENT)

clear : 
	rm -f *.o *.h.gch