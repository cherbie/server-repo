/**
* AUTHORS: CLAYTON HERBST (22245091) & FRASER LONERAGAN (22243455)
* UNIT : CITS3002 "COMPUTER NETWORKS" -- THE UNIVERSITY OF WESTERN AUSTRALIA
* COORDINATOR : DR JIN HONG
*/

#define NUM_LIVES 10
#define NUM_PLAYERS 10 //adjust number of players
#define MSG_SIZE 14 //bytes
#define WAIT_TIME_MOVE 10 //SECONDS SERVERS WAITS FOR TO RECEIVE CLIENT "MOV" MESSAGE
#define WAIT_TIME_INIT 10 //SECONDS NEEDED TO WAIT FOR CLIENT TO SEND "INIT" MESSAGE
#define WAIT_TIME_CONN 30 //SECONDS NEEDED TO WAIT FOR CLIENTS TO SEND CONNECTION REQUEST
#define WAIT_TIME_SEND 5 //SECONDS SERVER MUST WAIT FOR CLIENT TCP SOCKET TO BE READY FOR WRITING
#define NUM_DICE 2 //NUMBER OF DICE INVOLVED IN THE GAME

/***********************************************************/

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
#include <sys/wait.h>
#include <sys/errno.h>
#include <signal.h>

/** DATA STRUCTURES **/
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

/** GLOBAL VARIABLES **/
int                     port;
SERVER                  server; //SINGLE SERVER
PLAYER *                players; //GAME PLAYERS
QUEUE                   queue, dead_queue; //QUEUE ADT THAT MANAGES ACTIVE/ALIVE PLAYERS AND ELIMINATED PLAYERS
int                     rec_err, send_err, conn_err, err, opt_val; //VARIABLES USED FOR ERROR REPORTING
char *                  buf; //SOCKET READ AND WRITE BUFFER
struct fd_set           active_fds, rfds, wfds; //FILE DESCRIPTOR SETS FOR SELECT() SYS-CALL
struct timeval          tv;

/** FUNCTION DECLARATIONS **/
int start_game(void);
void set_server_socket(void);
int listenForInit(void);
void close_all_connections(int);
int receive_connections(void);
int send_msg(PLAYER *, const char *);
void reject_connections(void);
void set_player_lives(void);
int send_start(void);
int send_cancel(void);
int send_welcome(void);
int receive_move(PLAYER *);
void roll_dice(SERVER *);
int send_outcome(PLAYER *);
int parse_move(PLAYER *, char *);
bool move_is_correct(PLAYER *);
int send_pass(PLAYER *);
int send_fail(PLAYER *);
int play_game_round(void);
int send_vict(PLAYER *);
int send_elim(PLAYER *);
int receive_init(QUEUE *, PLAYER *, int);
void upper_string(char *);

/** QUEUE FUNCTION DECLARATIONS **/
int construct_queue(QUEUE *);
bool isFull(QUEUE *);
bool isEmpty(QUEUE *);
PLAYER * examine(QUEUE *);
int size(QUEUE *);
int enqueue(QUEUE *, PLAYER *);
PLAYER * dequeue_front(QUEUE *);
PLAYER * dequeue_last(QUEUE *);

