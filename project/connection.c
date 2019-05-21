#include "server.h"

/**
 * Set up connections with 4 players to the game
 * manages connection time outs and incorrect initialisation of connections.
 * returns 0 on success. On failure integer is returned based on following errors:
 * game_initialisation_timeout = -2;
 * failed_connection_establishment = -1;
 *
 * WAIT for client "INIT" message from 4 players
 * confirm game entry will "WELCOME".
 * Deny game entry with "REJECT".
 */
int listenForInit(int x) {

    //WAIT CERTAIN AMOUNT OF TIME TO CONNECT TO GAME
    sleep(WAIT_TIME_CONN);

    tv.tv_sec = 1;
    tv.tv_usec = 0;
    
    FD_ZERO(&rfds);
    FD_SET(server.fd, &rfds); 

    int retval = select(server.fd+1, &rfds, NULL, NULL, &tv);
    if( retval < 0 ) {
        perror("select() error.\n");
        gets(buf);
        return -1;
    }
    else if(retval == 0) {
        fprintf(stderr, "NO CLIENTS ATTEMPTING TO CONNECT.\n");
        gets(buf);
        return -1;
    }
    else {
        //FD_ZERO(&active_fds); NOT NEEDED AS THIS IS HANDLED AT THE START OF THE GAME
        players = malloc( NUM_PLAYERS * sizeof(PLAYER)); //allocate for all players
        int tempfd = 0;
        int i = 0;
        while(true) {
            socklen_t addr_len = sizeof(players[i].addr);
            tempfd = accept(server.fd, (struct sockaddr *) &players[i].addr, &addr_len);
            if(tempfd < 0) { //error -- not enough clients connected / in buffer
                perror("accept()\n");
                return -1;
            }
            players[i].fd = tempfd;
            fcntl(players[i].fd, F_SETFL, O_NONBLOCK); //SET TO NON-BLOCKING

            rec_err = receive_init(&queue, &players[i], i+1);
            if(rec_err < 0) {
                continue;
            }

            //FD_SET(players[i].fd, &active_fds); //add to working set
            if(server.num_players == NUM_PLAYERS) return 0;
            i++;
        }
    }      
    return -1;
}

/**
 * ENQUEUE: initialised game players.
 * SEND: welcome
 * @return: 0 to indicate succesful connection
 * @return: -1 to indicate connection error
 */
int receive_init(QUEUE * q, PLAYER * p, int count) {
    tv.tv_sec = WAIT_TIME_INIT;
    tv.tv_usec = 0;

    FD_ZERO(&rfds);
    FD_SET(p->fd, &rfds); 

    int retval = select(p->fd + 1, &rfds, NULL, NULL, &tv);
    if( retval < 0 ) {
        perror("select() error.\n");
        gets(buf);
        close(p->fd);
        return -1;
    }
    else if(retval == 0) {
        fprintf(stderr, "CLIENT: NOT ATTEMPTING TO CONNECT.\n");
        gets(buf);
        close(p->fd);
        return -1;
    }
    else {
        if(FD_ISSET(p->fd, &rfds) == 0) { //not in set
            return -1;
        }
    
        p->id = count;
        buf = calloc(MSG_SIZE, sizeof(char));
        rec_err = recv(p->fd, buf, MSG_SIZE, 0);
        
        printf("\tPlayer %i sent %s\n", p->id, buf);
        if(rec_err < 0) {
            perror("Error reading buffer message");
            if(send_msg(p, "REJECT") < 0) {
                fprintf(stderr, "Error sending message %s to %d\n", "REJECT", p->id);
            }
            close(p->fd);
            return -1;
        }
        else if(strcmp(buf, "INIT") != 0) {
            if(send_msg(p, "REJECT") < 0) {
                fprintf(stderr, "Error sending message %s to %d\n", "REJECT", p->id);
            }
            close(p->fd);
            return -1;
        }
        //"INIT" received
        if(send_welcome(p) < 0) {
            fprintf(stderr, "Error sending message %s to %d\n", "WELCOME", p->id);
            close(p->fd); //terminate connection
            return -1;
        }
        enqueue(q, p); //add to queue of players in lobby
    }
    server.num_players = size(q); //set number of players
    return 0;
}
/**
 * Send welcome message to client.
 * @return >=0 to indicate success, -1 to indicate error
 */
int send_welcome(PLAYER * p) {
    char * str = calloc(MSG_SIZE, sizeof(char));
    if(sprintf(str, "%s,%d", "WELCOME", p->id) <= 0) return -1;
    int ret = send_msg(p, str);
    free(str);
    return ret;
}
/**
 * @return the return value of socket send() function.
  @return: -1 indicates failure
 */
int send_msg(PLAYER* p, const char* s) {
    FD_ZERO(&rfds);
    FD_SET(p->fd, &rfds); 

    int retval = select(p->fd + 1, NULL, &rfds, NULL, NULL); //BLOCKING
    if( retval < 0 ) {
        perror("select() error.\n");
        return -1;
    }
    else {
        buf = calloc(MSG_SIZE, sizeof(char));
        if( sprintf(buf, "%s", s) <= 0) return -1; //set buffer.
        send_err = send(p->fd, buf, strlen(buf), 0);
        printf("sent: %s to player id: %d\n", buf, p->id);
        return send_err;
    }
}

/**
 * Initialise properties of server socket, bind to name and start listening.
 * On error -- exit program with EXIT_FAILURE of printing to stderr stream.
 */
void set_server_socket(int n) {
    server.fd = socket(AF_INET, SOCK_STREAM, 0); //create an endpoint for communication
    if (server.fd < 0){
        fprintf(stderr,"Could not create socket\n");
        exit(EXIT_FAILURE);
    }

    //DEFINE PROPERTIES OF SERVER ADDRESS
    server.addr.sin_family = AF_INET;
    server.addr.sin_port = htons(port); //converts the unsigned short integer hostshort from host byte order to network byte order.
    server.addr.sin_addr.s_addr = htonl(INADDR_ANY); //converts the unsigned integer hostlong from host byte order to network byte order.

    opt_val = 1;
    setsockopt(server.fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val); //set socket

    err = bind(server.fd, (struct sockaddr *) &server.addr, sizeof(server.addr)); //bind name to socket
    if (err < 0)    {
        fprintf(stderr,"Could not bind socket\n");
        exit(EXIT_FAILURE);
    }

    fcntl(server.fd, F_SETFL, O_NONBLOCK); //SET AS NON_BLOCKING
    server.num_players = 0; //no players connected

    //LISTEN FOR CONNECTIONS ON THE SOCKET
    err = listen(server.fd, 128);
    if (err < 0){
        perror("Could not listen on socket\n");
        exit(EXIT_FAILURE);
    }
}

void reject_connections(void) {
    struct sockaddr_in client;
    socklen_t addr_len = sizeof(client);

    //BLOCKING OPERATION
    struct fd_set fds;
    FD_ZERO(&fds);
    FD_SET(server.fd, &fds);
    int sel_err = select(server.fd + 1, &fds, NULL, NULL, NULL); //BLOCKING CONNECTION
    if(sel_err < 0) { //error
        perror(NULL);
        return;
    }
    else {
        int client_fd = accept(server.fd, (struct sockaddr *) &client, &addr_len);
        if(client_fd < 0) {
            fprintf(stderr, "Error accepting expired connection.");
            return;
        }
        FD_ZERO(&fds);
        FD_SET(client_fd, &fds);
        if(select(client_fd+1, NULL, &fds, NULL, NULL) > 0) { //BLOCKING OPERATION
            buf = calloc(MSG_SIZE, sizeof(char));
            sprintf(buf, "%s", "REJECT");
            if(send(client_fd, buf, strlen(buf), 0) < 0)
                perror("Error sending message 'REJECT'\n");
        }
        close(client_fd);
        return;
    }
}

/**
 * Send to all active game players "START,%d,%d"
 * with number of players & lives respectively
 * On failure to send, message will attempt to send another time.
 */
void send_start(void) {
    for(int i = 0; i < server.num_players; i++) {
        buf = calloc(MSG_SIZE, sizeof(char));
        sprintf(buf, "%s,%d,%d", "START", server.num_players, players[i].lives);
        if(send_msg(&players[i], buf) < 0) {
            send_msg(&players[i], buf); //try again
        }
        printf("\tsent %s\n", buf);
    }
}

/**
 * Send to all active game players "CANCEL"
 * On failure message will attempt to send one more time.
 */
void send_cancel(void) {
    for(int i = 0; i < server.num_players; i++) {
        buf = calloc(MSG_SIZE, sizeof(char));
        sprintf(buf, "%s", "CANCEL");
        if(send_msg(&players[i], buf) < 0) {
            send_msg(&players[i], buf); //try again
        }
        printf("\tsent -> %s\n", buf);
    }
}

