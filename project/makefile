SERVER = basic_server.c
CLIENT = client_game.c
CC = cc -std=c99 -Wall -Werror -pedantic

server : $(SERVER)
	$(CC) -o server $(SERVER)

client : $(CLIENT)
	$(CC) -o client $(CLIENT)

clear : 
	rm -f *.o