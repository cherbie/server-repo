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

    err = init_players(); //manages the introduction of players to the game
    if(err == -2) { //timeout for players joining the game
        fprintf(stderr, "Game timeout due to lack of players joining.\n");
        exit(EXIT_FAILURE);
    }
    else if(err == -1) {
        fprintf(stderr, "Failed to establish connection with requesting player.\n");
        exit(EXIT_FAILURE);
    }

    enter_game();
}

/**
 * Set up connections with 4 players to the game
 * manages connection time outs and incorrect initialisation of connections.
 * returns 0 on success. On failure integer is returned based on following errors:
 * game_initialisation_timeout = -2;
 * failed_connection_establishment = -1;
 */
int init_players(void)
{
    int n = 1;
    switch (n) {
        case 1 : {
            conn_players(&player1_fd, &client1);
            if (player1_fd < 0) {
                fprintf(stderr,"Could not establish new connection with %d\n", player1_fd);
                return -1;
            }
        }
        case 2 : {
            conn_players(&player2_fd, &client2);
            if (player2_fd < 0) {
                fprintf(stderr,"Could not establish new connection with %d\n", player2_fd);
                return -1;
            }
        }
        case 3 : {
            conn_players(&player3_fd, &client3);
            if (player3_fd < 0) {
                fprintf(stderr,"Could not establish new connection with %d\n", player3_fd);
                return -1;
            }
        }
        case 4 : {
            conn_players(&player4_fd, &client4);
            if ( player4_fd < 0 ) {
                fprintf(stderr,"Could not establish new connection with %d\n", player4_fd);
                return -1;
            }
        }
        default : break;
    }
    return 0;
}

void conn_players(int * fd, struct sockaddr_in * client_add) {
    socklen_t client_len = sizeof(client_add);
    *fd = accept(server_fd, (struct sockaddr *) client_add, &client_len);
}

void enter_game(void) {
    printf("PLAYERS:\n\t%d\n\t%d\n\t%d\n\t%d\n", player1_fd, player2_fd, player3_fd, player4_fd);
    while(true);
}

/**
 * Initialise properties of server socket, bind to name and start listening.
 * On error -- exit program with EXIT_FAILURE of printing to stderr stream.
 */
void set_server_socket(void) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0); //create an endpoint for communication
    if (server_fd < 0){
        fprintf(stderr,"Could not create socket\n");
        exit(EXIT_FAILURE);
    }

    //DEFINE PROPERTIES OF SERVER ADDRESS
    server.sin_family = AF_INET;
    server.sin_port = htons(port); //converts the unsigned short integer hostshort from host byte order to network byte order.
    server.sin_addr.s_addr = htonl(INADDR_ANY); //converts the unsigned integer hostlong from host byte order to network byte order.

    opt_val = 1;

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val); //set socket

    err = bind(server_fd, (struct sockaddr *) &server, sizeof(server)); //bind name to socket
    if (err < 0)    {
        fprintf(stderr,"Could not bind socket\n");
        exit(EXIT_FAILURE);
    }

    //LISTEN FOR CONNECTIONS ON THE SOCKET
    err = listen(server_fd, 128);
    if (err < 0){
        fprintf(stderr,"Could not listen on socket\n");
        exit(EXIT_FAILURE);
    }
}

