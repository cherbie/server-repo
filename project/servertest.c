#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define SERVER "./server"
#define CLIENT "./game"


int port;
int num_players;
int status;

int main(int argc, char * argv[]) {
    if(argc != 3) {
        fprintf(stderr, "USAGE: ./prog_name [port] [players]\n");
        return EXIT_FAILURE;
    }
    
    port = atoi(argv[1]);
    num_players = atoi(argv[2]);

    int pid = fork();
    if(pid == 0) {
        pid = fork();
        if(pid == 0) {
            char * prog = calloc(50, sizeof(char));
            sprintf(prog, "xterm -e %s %d %d", SERVER, port, num_players);

            if(system(prog) < 0) {
                fprintf(stderr, "SERVER SHUTDOWN WITH ERROR\n");
                exit(EXIT_FAILURE);
            }
            printf("server shutdown\n");
            exit(EXIT_SUCCESS);
        }
        else if(pid == -1) {
            perror(NULL);
            exit(EXIT_FAILURE);
        }

        for(int i = 0; i <= num_players; i++) {
            pid = fork();
            if(pid == 0) { //child
                pid = getpid();
                char * prog = calloc(50, sizeof(char));
                sprintf(prog, "xterm -e %s %d %d", CLIENT, port, pid);

                if( system(prog) < 0 ) {
                    perror(NULL);
                    exit(EXIT_FAILURE);
                }
                exit(EXIT_SUCCESS);
            }
            else if(pid == -1) {
                perror(NULL);
                exit(EXIT_FAILURE);
            }
        }
        int i = wait(&status);
        printf("wait status pid: %i\n", i);
        if(WIFEXITED(status)){
            return EXIT_SUCCESS;
        }
    }
    else if(pid == -1) {
        perror(NULL);
        return EXIT_FAILURE;
    }
    else {
        char * cp = malloc(2 * sizeof(char));
        gets(cp);
        wait(&status);
        if(WIFEXITED(status)) {
            printf("EXITED\n");
            exit(EXIT_SUCCESS);
        }
        exit(EXIT_FAILURE);
    }
}

