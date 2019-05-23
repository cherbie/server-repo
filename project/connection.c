/**
* AUTHORS: CLAYTON HERBST (22245091) & FRASER LONERAGAN (22243455)
* UNIT : CITS3002 "COMPUTER NETWORKS" -- THE UNIVERSITY OF WESTERN AUSTRALIA
* COORDINATOR : DR JIN HONG
*/

#include "server.h"

/**
 * ACCEPT AND RECEIVE INIT WITH GAME PLAYERS.
 * Manages connection time outs and incorrect initialisation of connections.
 * confirm game entry will "WELCOME".
 * Deny game entry with "REJECT".
 * @return 0 on success,  return -1 on failure to initialise players connection in game play.
 */
int listenForInit(void) {
    //SLEEP SERVER TO ALLOW CLIENTS/PLAYERS TO CONNECT TO THE GAME
    sleep(WAIT_TIME_CONN);

    tv.tv_sec = 1;
    tv.tv_usec = 0;
    
    FD_ZERO(&rfds);
    FD_SET(server.fd, &rfds); 

    //CHECK IF THERE ARE ANY CONNECTION REQUESTS IN SERVER SOCKET BUFFER
    int retval = select(server.fd+1, &rfds, NULL, NULL, &tv);
    if( retval < 0 ) {
        fprintf(stderr,"ERRNO=%d: \n\tSELECT SYS-CALL ERROR: UNABLE TO ESTABLISH TCP CONNECTION WITH CLIENTS.\n", errno);
        gets(buf);
        return -1;
    }
    else if(retval == 0) {
        fprintf(stderr, "SELECT SYS-CALL: NO CLIENTS ATTEMPTING TO CONNECT.\n");
        gets(buf);
        return -1;
    }
    else {
        players = malloc(NUM_PLAYERS * sizeof(PLAYER)); //ALLOCATE MEMORY FOR ALL POSSIBLE PLAYERS
        if(players == NULL) {
            fprintf(stderr, "UNABLE TO ALLOCATE MEMORY FOR PLAYERS\n");
            return -1;
        }
        int tempfd = 0;
        int i = 0;
        while(true) {
            socklen_t addr_len = sizeof(players[i].addr);
            tempfd = accept(server.fd, (struct sockaddr *) &players[i].addr, &addr_len);
            if(tempfd < 0) { //ERROR -- UNABLE TO ACCEPT CONNECTION REQUESTS
                perror("ACCEPT() SYS-CALL ERROR: UNABLE TO ACCEPT CONNECTION REQUESTS.\n");
                free(players);
                gets(buf);
                return -1;
            }
            players[i].fd = tempfd; //ALLOCATE CONNECTED CLIENT TO PLAYER MEMORY

            fcntl(players[i].fd, F_SETFL, O_NONBLOCK); //SET TO NON-BLOCKING

            rec_err = receive_init(&queue, &players[i], i+1);
            if(rec_err < 0) {
                fprintf(stderr, "DID NOT RECEIVE INIT PACKET FROM CLIENT.\n");
                continue; //client
            }

            if(server.num_players == NUM_PLAYERS) return 0; //ENOUGH PLAYERS CONNECTED TO SERVER
            i++;
        }
    }
    gets(buf); 
    return -1;
}

/**
 * Initialise game player entry into game play lobby by sending "Welcome,%d" message.
 * Otherwise send the "REJECT" message packet.
 * @param q QUEUE containing all active game players
 * @param p PLAYER to receive INIT from
 * @param count int used to assign player with id
 * @return 0 to indicate successful tcp connection established, -1 otherwise.
 */
int receive_init(QUEUE * q, PLAYER * p, int count) {
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    FD_ZERO(&rfds);
    FD_SET(p->fd, &rfds); 

    int retval = select(p->fd + 1, &rfds, NULL, NULL, &tv);
    if(retval < 0) {
        perror("SELECT: ERROR READING FILE DESCRIPTOR\n");
        close(p->fd);
        return -1;
    }
    else if(retval == 0) {
        fprintf(stderr, "CLIENT: NOT ATTEMPTING TO SEND INIT\n");
        close(p->fd);
        return -1;
    }
    else {
        if(FD_ISSET(p->fd, &rfds) == 0) { //NOT IN FILE DESCRIPTOR SET READ FROM
            close(p->fd);
            return -1;
        }

        p->id = count;
        buf = calloc(MSG_SIZE, sizeof(char));
        rec_err = recv(p->fd, buf, MSG_SIZE, 0);
        
        upper_string(buf); //SETS ALL ALPHABETICAL CHARACTERS TO UPPERCASE
        if(rec_err < 0) { //RECEIVE ERROR ... ERRNO SET
            fprintf(stderr, "ERRNO = %d:\n", errno);
            perror(NULL);
            close(p->fd);
            return -1;
        }
        else if(rec_err == 0) { //CLIENT GRACEFULLY CLOSED 
            fprintf(stderr, "ERRNO = %d:\n\tCLIENT HAS CLOSED THE SOCKET", errno);
            close(p->fd);
            return -1;
        }
        if(strcmp(buf, "INIT") != 0) { //BUF DOES NOT CONTAIN "INIT" MESSAGE
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
        enqueue(q, p); //ADD PLAYER TO QUEUE OF PLAYERS IN GAME PLAY LOBBY
    }
    server.num_players = size(q); //REVISE NUMBER OF PLAYERS MANAGED BY THE SERVER
    return 0;
}
/**
 * Send welcome message to player/client.
 * @return >=0 to indicate success, -1 to indicate error
 */
int send_welcome(PLAYER * p) {
    char * str = calloc(MSG_SIZE, sizeof(char));
    if(sprintf(str, "%s,%d", "WELCOME", p->id) <= 0) return -1;
    int ret = send_msg(p, str); //OUTCOME RETURNED TO CALLING FUNCTION
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
        if( sprintf(buf, "%s", s) <= 0) return -1; //SET SENDING BUFFER
        return send(p->fd, buf, strlen(buf), 0);
    }
}

/**
 * Initialise properties of server socket, bind to name and start listening.
 * On error -- exit program with EXIT_FAILURE of printing to stderr stream.
 */
void set_server_socket(void) {
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
    server.num_players = 0; //INTIALISE FIELD

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
 * @return 0 to indicate success, -1 to indicate fialure.
 */
int send_start(void) {
    for(int i = 0; i < server.num_players; i++) {
        buf = calloc(MSG_SIZE, sizeof(char));
        if(sprintf(buf, "%s,%d,%d", "START", server.num_players, players[i].lives) <= 0) return -1;
        if(send_msg(&players[i], buf) <= 0) {
            perror("ERROR SENDING START\n");
            return -1;
        }
    }
    return 0;
}

/**
 * Send to all active game players "CANCEL"
 * @return 0 to indicate success, -1 to indicate fialure.
 */
int send_cancel(void) {
    for(int i = 0; i < server.num_players; i++) {
        buf = calloc(MSG_SIZE, sizeof(char));
        if(sprintf(buf, "%s", "CANCEL") <= 0) return -1;
        if(send_msg(&players[i], buf) < 0) {
            perror("ERROR SENDING CANCEL\n");
            return -1;
        }
    }
    return 0;
}

