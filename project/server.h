#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

//GLOBAL VARIABLES
int port;
int server_fd, player1_fd, player2_fd, player3_fd, player4_fd; //socket integer descriptors
int err, opt_val; //temporary variables
struct sockaddr_in server, client1, client2, client3, client4; //The sockaddr_in structure is used to store addresses for the internet address family. 
char *buf; //socket input and output stream buffer

//FUNCTION DECLARATIONS
void conn_players(int *, struct sockaddr_in *);
int init_players(void);
void enter_game(void);
void set_server_socket(void);



