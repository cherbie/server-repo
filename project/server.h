#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define MSG_SIZE 14 //bytes
#define NUM_LIVES 3 
#define NUM_PLAYERS 2 //adjust number of players
#define NUM_SERVERS 1 //allow for scaling
#define NUM_DICE 2


//TYPES
typedef struct {
    int fd;
    struct sockaddr_in addr; //The sockaddr_in structure is used to store addresses for the internet address family.
    int id;
    int lives;
    char * move;
    bool alive;
} PLAYER;

typedef struct {
    int fd;
    struct sockaddr_in addr; //The sockaddr_in structure is used to store addresses for the internet address family.
    int num_players;
    int * dice;
} SERVER;

typedef struct {
    int elements[NUM_PLAYERS];
    int front;
    int count;
    int len;;
}QUEUE;

//GLOBAL VARIABLES
int port;
SERVER * servers; //socket integer descriptors
PLAYER * players; //define players
int err, opt_val; //temporary variables
char *buf; //socket input and output stream buffer

//FUNCTION DECLARATIONS
void conn_players(PLAYER *);
int start_game(void);
void set_server_socket(int);
int listenForInit(int);
int send_msg(PLAYER *, const char *);
void reject_connections(void);
void set_player_lives(void);
void send_start(void);
void send_cancel(void);
int send_welcome(PLAYER *);
void receive_move(PLAYER *);
void roll_dice(SERVER *);
void send_success(PLAYER *);
