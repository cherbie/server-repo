#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define MSG_SIZE 1000

typedef struct {
    int fd;
    struct sockaddr_in addr; //The sockaddr_in structure is used to store addresses for the internet address family.
    int players;
    int lives;
    int id;
} SERVER;

char * buf;
int port; //global variable
int err, opt_val;
SERVER server;

//FUNCITON DECLARATIONS
void connect_to_server(void);
int init_match(void);
int send_msg(char *);
int extract_start(char *);
int receive_welcome(SERVER *, char *);
void send_move(char *);
void receive_move(void);
