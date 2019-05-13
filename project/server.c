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
    if(construct_queue(NUM_PLAYERS) < 0) {
        fprintf(stderr, "UNABLE TO ALLOCATE MEMORY TO STACK\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Server is listening on %d\n", port);

    err = listenForInit(NUM_PLAYERS); //manages the introduction of players to the game
    if(err == -2) { //timeout for players joining the game
        fprintf(stderr, "Game timeout due to lack of players joining.\n");
        send_cancel();
        exit(EXIT_FAILURE);
    }
    else if(err == -1) {
        fprintf(stderr, "Failed to establish connection with requesting player.\n");
        exit(EXIT_FAILURE);
    }
    
    switch(fork()) {
        case 0 : { //child
            srand(time(NULL));
            start_game();
        }
        case -1 : { //error forking
            perror("UNEXPECTED APPLICATION ERROR: FORKING_ERROR.\n");
        }
        default : { //parent
            while(true)
                reject_connections();
        }
    }
}

/**
 * CONSTRUCTOR FOR QUEUE ADT
 * @return 0 to indicate success, -1 to indicate failure
 */
int construct_queue(int num) {
    queue.front = 0;
    queue.count = 0;
    queue.length = num;
    return 0;
}
