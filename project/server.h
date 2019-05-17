#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/select.h>

#define BUFFER_SIZE 1024
#define MSG_SIZE 14 //bytes
#define NUM_LIVES 3 
#define NUM_PLAYERS 3 //adjust number of players
#define NUM_SERVERS 1 //allow for scaling
#define NUM_DICE 2


//TYPES
typedef struct {
    int fd;
    struct sockaddr_in addr; //The sockaddr_in structure is used to store addresses for the internet address family.
    int id;
    int lives;
    char * move;
    int roll; //store %d value in "CON,%d" packet
    bool alive;
    bool correct;
} PLAYER;

typedef struct {
    int fd;
    struct sockaddr_in addr; //The sockaddr_in structure is used to store addresses for the internet address family.
    int num_players;
    int * dice;
} SERVER;

typedef struct {
    PLAYER * elements[NUM_PLAYERS];
    int front;
    int count;
    int length;
}QUEUE;

//GLOBAL VARIABLES
int port;
SERVER server; //socket integer descriptors
PLAYER * players; //define players
QUEUE queue, dead_queue; //queue managing players that are not alive and alive
int rec_err, send_err, conn_err, err, opt_val; //temporary variables
char *buf; //socket input and output stream buffer

struct fd_set active_fds;
struct fd_set rfds;
struct fd_set wfds;
struct timeval tv;

//FUNCTION DECLARATIONS
int start_game(void);
void set_server_socket(int);
int listenForInit(int);
int send_msg(PLAYER *, const char *);
void reject_connections(void);
void set_player_lives(void);
void send_start(void);
void send_cancel(void);
int send_welcome(PLAYER *);
int receive_move(PLAYER *);
void roll_dice(SERVER *);
int send_outcome(PLAYER *);
int construct_queue(QUEUE *, int);
int parse_move(PLAYER *, char *);
bool move_is_correct(PLAYER *);
int send_pass(PLAYER *);
int send_fail(PLAYER *);
int play_game_round(void);
int send_vict(PLAYER *);
int send_elim(PLAYER *);
int receive_init(QUEUE *, PLAYER *, int);

//QUEUE FUNCTION DECLARATIONS
bool isFull(QUEUE *);
bool isEmpty(QUEUE *);
PLAYER * examine(QUEUE *);
int size(QUEUE *);
int enqueue(QUEUE *, PLAYER *);
PLAYER * dequeue_front(QUEUE *);
PLAYER * dequeue_last(QUEUE *);

