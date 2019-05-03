/** CLIENT
 * socket() : create socket and obtain client_fd
 * connect() : 
 * write() : send()
 * read)() : recv()
 * close() :
 * select() : ... when is it possible to send more data
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFFER_SIZE 1024


/**
PREDOMINANTLY SERVER LOGIC
 1. INITIALISE MATCH
 2. PLAYERS LIVES > 0
    a. NO:
        - NOTIFY ALL PLAYERS OF THE CURRENT STANDING
        - EXIT GAME
    b. YES:
        - WAIT FOR PLAYERS MOVE
        - ROLL DICE
        - CALCULATE SCORES
        - NOTIFY ALL PLAYERS
        - REPEAT
**/

int main(int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr,"Usage: %s [port]\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int server_fd, err, opt_val;
    struct sockaddr_in server; //socket properties
    char *buf;

    //INITIALISE MATCH
    server_fd = socket(AF_INET, SOCK_STREAM, 0); //create an endpoint for communication

    if (server_fd < 0){
        fprintf(stderr,"Could not create socket\n");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port); //converts the unsigned short integer hostshort from host byte order to network byte order.
    server.sin_addr.s_addr = htonl(INADDR_ANY); //converts the unsigned integer hostlong from host byte order to network byte order.

    opt_val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

    err = connect(server_fd, (struct sockaddr *) &server, sizeof(server));
    if(err < 0) {
        fprintf(stderr, "Could not connect to server.\n");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server on port: %d", port);


    //send message
    buf = calloc(BUFFER_SIZE, sizeof(char));
    
    err = sprintf(buf,"Message from Client");
    if( err < 0) {
        fprintf(stderr, "Error setting chars in buffer.");
        exit(EXIT_FAILURE);
    }

    err = send(server_fd, buf, BUFFER_SIZE, 0);

    while(true) {
        buf = calloc(BUFFER_SIZE, sizeof(char));

        int read = recv(server_fd, buf, BUFFER_SIZE, 0);
        if(read < 0) {
            fprintf(stderr, "Error reading data from socket into buffer.");
            exit(EXIT_FAILURE);
        }

        printf("[+]SERVER -> CLIENT message: %s\n", buf);
        printf("waiting to receive more messages.\n");
    }
}
