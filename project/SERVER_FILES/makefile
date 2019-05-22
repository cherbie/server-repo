
# 	USAGE: \
	TO COMPILE SERVER FILES: "make server" \
	TO REMOVE ALL OBJECT FILES AND PRECOMPILED HEADER FILES: "make clear"


###########################################################################

SERVER_O = server.o queue.o connection.o gamelobby.o
SERVER = server.c server.h
CONNECTION = connection.c server.h
GAMELOBBY = gamelobby.c server.h
QUEUE = queue.c server.h
CC = cc -std=c99 -Wall -Werror -pedantic

###########################################################################

server : $(SERVER_O)
	$(CC) -o server $(SERVER_O)

server.o : $(SERVER)
	$(CC) -c $(SERVER)

queue.o : $(QUEUE)
	$(CC) -c $(QUEUE)

connection.o : $(CONNECTION)
	$(CC) -c $(CONNECTION)

gamelobby.o : $(GAMELOBBY)
	$(CC) -c $(GAMELOBBY)

clear : 
	rm -f *.o *.h.gch