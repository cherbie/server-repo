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

typedef struct {
    int fd;
    struct sockaddr_in addr; //The sockaddr_in structure is used to store addresses for the internet address family.
    int lives;
    char * buf;
} CLIENT;

int port; //global variable
int err, opt_val;
CLIENT client;

//FUNCITON DECLARATIONS
void connect_to_server(void);
int init_match(void);
int send_msg(char *);

