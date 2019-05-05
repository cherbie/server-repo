/**
NOTE --> PREDOMINANTLY SERVER LOGIC
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

#include "server.h"

int main(int argc, char * argv[]) {
    
    if (argc < 2) {
        fprintf(stderr,"Usage: %s [port]\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[1]);
    set_server_socket(); //EXIT_FAILURE UPON ERROR
    printf("Server is listening on %d\n", port);

    int n;
    err = init_players(&n); //manages the introduction of players to the game
    if(err == -2) { //timeout for players joining the game
        fprintf(stderr, "Game timeout due to lack of players joining.\n");
        exit(EXIT_FAILURE);
    }
    else if(err == -1) {
        fprintf(stderr, "Failed to establish connection with requesting player.\n");
        exit(EXIT_FAILURE);
    }
    
    switch(fork()) {
        case 0 : { //child parent
            printf("child\n");
            start_game();
        }
        case -1 : { //error forking
            perror("UNEXPECTED APPLICATION ERROR: ");
        }
        default : { //parent
        printf("parent.\n");
            while(true)
                reject_connections();
        }
    }
}

/**
 * Set up connections with 4 players to the game
 * manages connection time outs and incorrect initialisation of connections.
 * returns 0 on success. On failure integer is returned based on following errors:
 * game_initialisation_timeout = -2;
 * failed_connection_establishment = -1;
 */
int init_players(int * n)
{
    *n = 1; //ensure starts at 1
    printf("%d", *n);
    while( *n < 5 ) {
        switch (*n) {
            case 1 : {
                conn_players(&player1, *n); //connect client socket
                listenForInit(n); //try to initiaise game entry for player #n
                break;
            }
            case 2 : {
                conn_players(&player2, *n); //connect client socket
                listenForInit(n); //try to initiaise game entry for player #n
                break;
            }
            case 3 : {
                conn_players(&player3, *n); //connect client socket
                listenForInit(n); //try to initiaise game entry for player #n
                break;
            }
            case 4 : {
                conn_players(&player4, *n); //connect client socket
                listenForInit(n); //try to initiaise game entry for player #n
                break;
            }
            default : break;
        }
        if( *n < 0) *n = 0;
        n++;
    }
    return 0;
}

/**
 * WAIT for client "INIT" message from 4 players
 * confirm game entry will "WELCOME".
 * Deny game entry with "REJECT".
 */
void listenForInit(int *n) {
    buf = calloc(MSG_SIZE, sizeof(char));
    switch(*n) {
        case 1 : {
            err = recv(player1.fd, buf, sizeof(buf), 0);
            if(err < 0) {
                fprintf(stderr, "Error reading buffer message");
                if(send_msg(&player1, "REJECT") < 0) {
                    fprintf(stderr, "Error sending message %s to %d\n", "REJECT", player1.fd);
                }
                //destroy connection
                n--; //reduce number of players connected
                return;
            }
            else if( strcmp(buf, "INIT") != 0) {
                if(send_msg(&player1, "REJECT") < 0) {
                    fprintf(stderr, "Error sending message %s to %d\n", "REJECT", player1.id);
                }
                //destroy connection
                n--; //reduce 
                return;
            }
            //"INIT" received
            player1.id = *n; //set player id
            if(send_msg(&player1, "WELCOME") < 0) {
                fprintf(stderr, "Error sending message %s to %d\n", "WELCOME", player1.id);
                //destroy connection
                n--; //reduce 
                return;
            }
            player1.id = *n;
            return;
        }
        case 2 : {
            break;
        }
        case 3 : {
            break;
        }
        case 4 : {
            break;
        }
        default: { // n < 1;
            return;
        }
    }
}

int send_msg(PLAYER* p, const char* s) {
    buf = calloc(MSG_SIZE, sizeof(char));
    sprintf(buf, "%s", s); //set buffer.
    return send(p->fd, buf, sizeof(char), 0);
}

void conn_players(PLAYER *player_n, int n) {
    socklen_t addr_len = sizeof(player_n->addr);
    player_n->fd = accept(server.fd, (struct sockaddr *) &player_n->addr, &addr_len);
}

/**
 * Initialise properties of server socket, bind to name and start listening.
 * On error -- exit program with EXIT_FAILURE of printing to stderr stream.
 */
void set_server_socket(void) {
    server.fd = socket(AF_INET, SOCK_STREAM, 0); //create an endpoint for communication
    if (server.fd < 0){
        fprintf(stderr,"Could not create socket\n");
        exit(EXIT_FAILURE);
    }

    //DEFINE PROPERTIES OF SERVER ADDRESS
    server.addr.sin_family = AF_INET;
    server.addr.sin_port = htons(port); //converts the unsigned short integer hostshort from host byte order to network byte order.
    server.addr.sin_addr.s_addr = htonl(INADDR_ANY); //converts the unsigned integer hostlong from host byte order to network byte order.

    opt_val = 1;

    setsockopt(server.fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val); //set socket

    err = bind(server.fd, (struct sockaddr *) &server.addr, sizeof(server.addr)); //bind name to socket
    if (err < 0)    {
        fprintf(stderr,"Could not bind socket\n");
        exit(EXIT_FAILURE);
    }

    //LISTEN FOR CONNECTIONS ON THE SOCKET
    err = listen(server.fd, 128);
    if (err < 0){
        fprintf(stderr,"Could not listen on socket\n");
        exit(EXIT_FAILURE);
    }
}

void reject_connections(void) {
    struct sockaddr_in client;
    socklen_t addr_len = sizeof(client);
    int client_fd = accept(server.fd, (struct sockaddr *) &client, &addr_len);
    if(client_fd < 0) {
        fprintf(stderr, "Error accepting expired connection.");
        return;
    }
    buf = calloc(BUFFER_SIZE, sizeof(char));
    if(recv(player1.fd, buf, sizeof(buf), 0) < 0) {
        fprintf(stderr, "Error reading buffer message");
        if(send_msg(&player1, "REJECT") < 0) {
            fprintf(stderr, "Error sending message %s to %d\n", "REJECT", client_fd);
        }
        //destroy connection
        return;
    }
    //Message received.
    if(send_msg(&player1, "REJECT") < 0) {
        fprintf(stderr, "Error sending message %s to %d\n", "REJECT", client_fd);
    }
    //destroy connection
    return;
}

void set_player_lives(void) {
    player1.lives = NUM_LIVES;
    player2.lives = NUM_LIVES;
    player3.lives = NUM_LIVES;
    player4.lives = NUM_LIVES;
}

void start_game(void) {
    printf("PLAYERS:\n\t%d\n\t%d\n\t%d\n\t%d\n", player1.id, player2.id, player3.id, player4.id);

    set_player_lives();

    while(true) {
        
    }
}
