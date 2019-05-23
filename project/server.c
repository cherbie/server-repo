/**
* AUTHORS: CLAYTON HERBST (22245091) & FRASER LONERAGAN (22243455)
* UNIT : CITS3002 "COMPUTER NETWORKS" -- THE UNIVERSITY OF WESTERN AUSTRALIA
* COORDINATOR : DR JIN HONG
*/

/**
* RESPONSIBLE FOR REPORTING PROCESS EXECUTION ERRORS, FORKING NEW PROCESSES AND HADNLING
* PROGRAM EXECUTION THROUGH THE MAIN() FUNCTION.
*/
#include "server.h"

int main(int argc, char * argv[]) {
    if (argc != 2) {
        fprintf(stderr,"Usage: %s [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[1]); //SET PORT

    set_server_socket(); //SET PROPERTIES OF SERVER SOCKET

    printf("Server is listening on %d\n", port);

    //CONSTRUCT QUEUE LATER?
    if(construct_queue(&queue) < 0) {
        fprintf(stderr, "UNABLE TO ALLOCATE MEMORY TO STACK\n");
        exit(EXIT_FAILURE);
    }
    
    err = listenForInit(); //MANAGES THE INITIALISATION OF PLAYERS IN THE GAME
    if(err < 0 || size(&queue) < NUM_PLAYERS) {
        fprintf(stderr, "Game connection establishment timeout. Failed to establish connection.\n");
        send_cancel(); //SEND CANCEL TO ALL WELCOMED GAME PLAYERS
        exit(EXIT_FAILURE);
    }

<<<<<<< HEAD

=======
    
>>>>>>> dev
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
            exit(EXIT_FAILURE);
        }
        default : { //PARENT
            int p = fork();
            if(p == 0) { //CHILD -- HANDLE PLAYERS ATTEMPTING TO CONNECT AFTER GAME HAS STARTED
                while(true)
                    reject_connections();
            }
            else if(p == -1) {
                perror("UNEXPECTED APPLICATION ERROR: FORKING_ERROR.\n");
                exit(EXIT_FAILURE);
            }
            else { //parent
                int wstatus;
                waitpid(pid, &wstatus, 0); //WAIT FOR GAMEPLAY CHILD TO FINISH EXECUTING
                close(server.fd);
                if(WIFEXITED(wstatus)) //TRUE -- has exited
                    fprintf(stderr, "GAME PLAY CHILD RETURNED NORMALLY WITH EXIT STATUS %d\n", WEXITSTATUS(wstatus));
                kill(p, SIGTERM); //KILL PROCESS REJECT CONNECTIONS
                exit(WEXITSTATUS(wstatus));
            }
        }
    }
}

