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
    
    if (argc < 3) {
        fprintf(stderr,"Usage: %s [port] [number of players]\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[1]);
    if( atoi(argv[2]) < NUM_PLAYERS ) {
        fprintf(stderr, "INCORRECT NUMBER OF PLAYERS SPECIFIED. NEED %d\n", NUM_PLAYERS);
        return EXIT_FAILURE;
    }

    set_server_socket(NUM_SERVERS); //EXIT_FAILURE UPON ERROR

    //CONSTRUCT QUEUE LATER?
    if(construct_queue(&queue, NUM_PLAYERS) < 0) {
        fprintf(stderr, "UNABLE TO ALLOCATE MEMORY TO STACK\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Server is listening on %d\n", port);

    err = listenForInit(NUM_PLAYERS); //manages the introduction of players to the game
    if(err < 0) {
        fprintf(stderr, "Game connection establishment timeout. Failed to establish connection.\n");
        send_cancel();
        gets(buf);
        exit(EXIT_FAILURE);
    }
    int pid = fork();
    switch(pid) {
        case 0 : { //child
            srand(time(NULL));
            if( start_game() == 0 ) {
                close(server.fd);
                fprintf(stderr, "EXITED WITH EXIT_SUCCESS.\n");
                exit(EXIT_SUCCESS);
            }
            else {
                close(server.fd);
                fprintf(stderr, "EXITED WITH EXIT_FAILURE.\n");
                exit(EXIT_FAILURE);
            }
        }
        case -1 : { //error forking
            perror("UNEXPECTED APPLICATION ERROR: FORKING_ERROR.\n");
        }
        default : { //parent
            //HANDLE PLAYERS ATTEMPTING TO CONNECT AFTER GAME HAS STARTED
            int p = fork();
            if(p == 0) { //CHILD
                while(true)
                    reject_connections();
            }
            else if(p == -1) {
                perror(NULL);
                return EXIT_FAILURE;
            }
            else { //parent
                int wstatus;
                waitpid(pid, &wstatus, 0);
                close(server.fd);
                if(WIFEXITED(wstatus)) //TRUE -- has exited
                    fprintf(stderr, "RETURNED NORMALLY\n");
                    //exit(WEXITSTATUS(wstatus));
                fprintf(stderr, "DID NOT WAIT. EXIT STATUS IS %d\n", WEXITSTATUS(wstatus));
                gets(buf);
            }
        }
    }
}

/**
 * CONSTRUCTOR FOR QUEUE ADT
 * @return 0 to indicate success, -1 to indicate failure
 */
int construct_queue(QUEUE * q, int num) {
    q->front = 0;
    q->count = 0;
    q->length = num;
    return 0;
}
