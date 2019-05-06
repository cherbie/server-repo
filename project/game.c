/** CLIENT
 * socket() : create socket and obtain client_fd
 * connect() : 
 * write() : send()
 * read)() : recv()
 * close() :
 * select() : ... when is it possible to send more data
 */

#include "client.h"

int main(int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr,"Usage: %s [port]\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[1]);

    //INITIALISE MATCH
    if( init_match() < 0 ) {
        close(client.fd);
        exit(EXIT_FAILURE);
    }
    
    while(true) {
        printf("WAITING FOR GAME.\n");
        gets(client.buf);
    }
}

/**
 * send message containing char * paramater
 * @return 0 to indicate success, -1 to indicate failure.
 */
int send_msg(char * s) {
    client.buf = calloc(MSG_SIZE, sizeof(char));
    if(client.buf == NULL) {
        fprintf(stderr, "send_msg(): error allocating memory to buffer. \n");
        return -1;
    }
    sprintf(client.buf, "%s", s);
    if( send(client.fd, client.buf, sizeof(client.buf), 0) < 0 ) {
        fprintf(stderr, "Error sending %s to server.", s);
        return -1;
    }
    return 0;
}

/**
 *
 *
 */
void connect_to_server(void) {
    client.fd = socket(AF_INET, SOCK_STREAM, 0); //create an endpoint for communication

    if (client.fd < 0){
        fprintf(stderr,"Could not create socket\n");
        exit(EXIT_FAILURE);
    }

    client.addr.sin_family = AF_INET;
    client.addr.sin_port = htons(port); //converts the unsigned short integer hostshort from host byte order to network byte order.
    client.addr.sin_addr.s_addr = htonl(INADDR_ANY); //converts the unsigned integer hostlong from host byte order to network byte order.

    opt_val = 1;
    setsockopt(client.fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

    err = connect(client.fd, (struct sockaddr *) &client.addr, sizeof(client.addr));
    if(err < 0) {
        fprintf(stderr, "Could not connect to server.\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * Handles funcition calls to initialise/enter game.
 * @return 0 to indicate game entry, -1 to indicate failure to enter game.
 */
int init_match(void) {
    connect_to_server();

    printf("Connected to server on port: %d\n\tclient:\t%d\n", port, client.fd);

    if( send_msg("INIT") < 0 ) {
        return -1;
    }

    client.buf = calloc(MSG_SIZE, sizeof(char));
    if( recv(client.fd, client.buf, sizeof(client.buf), 0) < 0) {
        fprintf(stderr, "Error receiving reply to %s message sent.\n", "INIT");
    }

    if( strcmp(client.buf, "REJECT") == 0) { //REJECT received
        fprintf(stderr, "Game entry rejected.\n");
        return -1;
    }
    if( strcmp(client.buf, "WELCOME") == 0) {
        printf("%s", client.buf);
        return 0;
    }
    fprintf(stderr, "Garbage message packet received.\n\tmessage: %s\n", client.buf);
    return -1;
}

