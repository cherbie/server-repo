#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define MSG_SIZE 14
#define NUM_LIVES 3

//TYPES
typedef struct {
    int fd;
    struct sockaddr_in addr; //The sockaddr_in structure is used to store addresses for the internet address family.
    int id;
    int lives;
} PLAYER;

typedef struct {
    int fd;
    struct sockaddr_in addr; //The sockaddr_in structure is used to store addresses for the internet address family.
} SERVER;

//GLOBAL VARIABLES
int port;
SERVER server; //socket integer descriptors
PLAYER player1, player2, player3, player4; //define players
int err, opt_val; //temporary variables
char *buf; //socket input and output stream buffer

//FUNCTION DECLARATIONS
void conn_players(PLAYER *, int);
void start_game(void);
void set_server_socket(void);
int listenForInit(int *);
int send_msg(PLAYER *, const char *);
void reject_connections(void);
void set_player_lives(void);


