/** CLIENT
 * socket() : create socket and obtain server_fd
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
        close(server.fd);
        exit(EXIT_FAILURE);
    }
    
    while(true) {
        printf("WAITING FOR GAME.\n");
        gets(buf);
    }
}

/**
 * send message containing char * paramater
 * @return 0 to indicate success, -1 to indicate failure.
 */
int send_msg(char * s) {
    buf = calloc(MSG_SIZE, sizeof(char));
    if(buf == NULL) {
        fprintf(stderr, "send_msg(): error allocating memory to buffer. \n");
        return -1;
    }
    sprintf(buf, "%s", s);
    if( send(server.fd, buf, strlen(buf), 0) < 0 ) {
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
    server.fd = socket(AF_INET, SOCK_STREAM, 0); //create an endpoint for communication

    if (server.fd < 0){
        fprintf(stderr,"Could not create socket\n");
        exit(EXIT_FAILURE);
    }

    server.addr.sin_family = AF_INET;
    server.addr.sin_port = htons(port); //converts the unsigned short integer hostshort from host byte order to network byte order.
    server.addr.sin_addr.s_addr = htonl(INADDR_ANY); //converts the unsigned integer hostlong from host byte order to network byte order.

    opt_val = 1;
    setsockopt(server.fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

    err = connect(server.fd, (struct sockaddr *) &server.addr, sizeof(server.addr));
    if(err < 0) {
        fprintf(stderr, "Could not connect to server.\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * Tokenises char * paramater and sets
 * sets: server.lives
 * sets: server.players
 * @return 0 to indicate success, or -1 to indicate stop game involvement.
 */
int extract_start(char * s) {
    const char delim[2] = ",";
    int n = 0; //start at case 0

    char * tok = strtok(s, delim);
    while( tok != NULL ) {
        printf("token: %s\n", tok);
        switch (n) {
            case 0 : { //START || CANCEL
                if(strcmp(tok, "START") == 0) {
                    printf("%s\n", tok);
                }
                else { //received "CANCEL"
                    printf("%s\n", tok);
                    return -1;
                }
                n++;
                tok = strtok(NULL, delim);
                break;
            }
            case 1 : { //number of players
                server.players = atoi(tok);
                n++;
                tok = strtok(NULL, delim);
                printf("--> %s\n", tok); //null
                break;
            }
            case 2 : { //number of lives
                printf("REACHED REACHED REACHED.\n");
                server.lives = atoi(tok);
                n++;
                tok = strtok(NULL, delim);
                break;
            }
            default : 
                printf("default\n");
                return 0;
        }
    }
    printf("EXIT EXIT EXIT\n");
    return 0;
}

/**
 * Handles funcition calls to initialise/enter game.
 * @return 0 to indicate game entry, -1 to indicate failure to enter game.
 */
int init_match(void) {
    connect_to_server();

    printf("Connected to server on port: %d\n\tclient:\t%d\n", port, server.fd);

    if( send_msg("INIT") < 0 ) {
        return -1;
    }

    buf = calloc(MSG_SIZE, sizeof(char));
    if( recv(server.fd, buf, MSG_SIZE, 0) < 0) {
        fprintf(stderr, "Error receiving reply to %s message sent.\n", "INIT");
    }

    //REJECT received
    if( strcmp(buf, "REJECT") == 0) { 
        fprintf(stderr, "Game entry rejected.\n");
        return -1;
    }
    //RECEIVE WELCOME
    if( strcmp(buf, "WELCOME") == 0) {
        printf("%s\n", buf);
    }
    else {
        fprintf(stderr, "Garbage message packet received.\n\tmessage: %s\n", buf);
        return -1;
    }

    //RECEIVE START
    buf = calloc(MSG_SIZE, sizeof(char));
    err = recv(server.fd, buf, MSG_SIZE, 0);
    printf("%i\n", err);
    if( err < 0) 
        fprintf(stderr, "Error receiving %s message sent.\n", "START");
    printf("RECEIVED: %s\n", buf);
    if(strcmp(buf, "CANCEL") == 0) {
        printf("%s\n", buf);
        return -1;
    }
    return extract_start(buf); //tokenise "START" message
}

