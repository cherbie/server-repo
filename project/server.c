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
    set_server_socket(NUM_SERVERS); //EXIT_FAILURE UPON ERROR
    printf("Server is listening on %d\n", port);

    err = listenForInit(NUM_PLAYERS); //manages the introduction of players to the game
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
 *
 * WAIT for client "INIT" message from 4 players
 * confirm game entry will "WELCOME".
 * Deny game entry with "REJECT".
 */
int listenForInit(int n) {
    buf = calloc(MSG_SIZE, sizeof(char));
    for(int i = 0; i < n; i++) {
        int id = i +1;
        players = realloc( players, (i+1) * sizeof(PLAYER)); //add another player
        conn_players(&players[i]); //connect client socket
        err = recv(players[i].fd, buf, sizeof(buf), 0);
        printf("Player %i sent %s\n", id, buf);
        if(err < 0) {
            fprintf(stderr, "Error reading buffer message");
            if(send_msg(&players[i], "REJECT") < 0) {
                fprintf(stderr, "Error sending message %s to %d\n", "REJECT", id);
            }
            close(players[i].fd);
            i--; //reduce number of players connected
            continue;
        }
        else if( strcmp(buf, "INIT") != 0) {
            if(send_msg(&players[i], "REJECT") < 0) {
                fprintf(stderr, "Error sending message %s to %d\n", "REJECT", id);
            }
            close(players[i].fd);
            i--; //reduce number of players connected
            continue;
        }
        //"INIT" received
        if(send_msg(&players[i], "WELCOME") < 0) {
            fprintf(stderr, "Error sending message %s to %d\n", "WELCOME", id);
            close(players[i].fd);
            n--; //reduce number of players connected
            continue;
        }
        players[i].id = id;
        continue;
    }
    return 0;
}

int send_msg(PLAYER* p, const char* s) {
    buf = calloc(MSG_SIZE, sizeof(char));
    sprintf(buf, "%s", s); //set buffer.
    return send(p->fd, buf, sizeof(buf), 0);
}

void conn_players(PLAYER *player) {
    socklen_t addr_len = sizeof(player->addr);
    player->fd = accept(servers[0].fd, (struct sockaddr *) &player->addr, &addr_len);
}

/**
 * Initialise properties of server socket, bind to name and start listening.
 * On error -- exit program with EXIT_FAILURE of printing to stderr stream.
 */
void set_server_socket(int n) {
    for(int i = 0; i < n; i++) {
        int id = i+1;
        servers = realloc(servers, id * sizeof(SERVER));
        servers[i].fd = socket(AF_INET, SOCK_STREAM, 0); //create an endpoint for communication
        if (servers[i].fd < 0){
            fprintf(stderr,"Could not create socket\n");
            exit(EXIT_FAILURE);
        }

        //DEFINE PROPERTIES OF SERVER ADDRESS
        servers[i].addr.sin_family = AF_INET;
        servers[i].addr.sin_port = htons(port); //converts the unsigned short integer hostshort from host byte order to network byte order.
        servers[i].addr.sin_addr.s_addr = htonl(INADDR_ANY); //converts the unsigned integer hostlong from host byte order to network byte order.

        opt_val = 1;

        setsockopt(servers[i].fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val); //set socket

        err = bind(servers[i].fd, (struct sockaddr *) &servers[i].addr, sizeof(servers[i].addr)); //bind name to socket
        if (err < 0)    {
            fprintf(stderr,"Could not bind socket\n");
            exit(EXIT_FAILURE);
        }

        //LISTEN FOR CONNECTIONS ON THE SOCKET
        err = listen(servers[i].fd, 128);
        if (err < 0){
            fprintf(stderr,"Could not listen on socket\n");
            exit(EXIT_FAILURE);
        }
    }
}

void reject_connections(void) {
    struct sockaddr_in client;
    socklen_t addr_len = sizeof(client);
    int client_fd = accept(servers[0].fd, (struct sockaddr *) &client, &addr_len);
    if(client_fd < 0) {
        fprintf(stderr, "Error accepting expired connection.");
        return;
    }
    buf = calloc(BUFFER_SIZE, sizeof(char));
    if(recv(client_fd, buf, sizeof(buf), 0) < 0) {
        fprintf(stderr, "Error reading buffer message");
    }
    buf = calloc(MSG_SIZE, sizeof(char));
    sprintf(buf, "%s", "REJECT");
    if(send(client_fd, buf, sizeof(buf), 0) < 0) {
        fprintf(stderr, "Error sending message %s to %d\n", "REJECT", client_fd);
    }
    close(client_fd);
    return;
}

void set_player_lives(void) {
    for(int i = 0; i < NUM_PLAYERS; i++) {
        players[i].lives = NUM_LIVES;
    }
}

void start_game(void) {
    printf("GAME PLAYERS\n");
    for(int i = 0; i < NUM_PLAYERS; i++)
        printf("\t%d\n", players[i].id);

    set_player_lives();

    while(true) {
        printf("Enter !~ characters to quit program.\n");
        buf = calloc(MSG_SIZE, sizeof(char));
        gets(buf);
        if(strcmp(buf, "!~") == 0) {
            free(buf);
            free(players);
            exit(EXIT_SUCCESS);
        }
    }
}
