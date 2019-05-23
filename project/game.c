/** CLIENT
 * socket() : create socket and obtain server_fd
 * connect() : 
 * write() : send()
 * read)() : recv()
 * close() :
 * select() : ... when is it possible to send more data
 */

#include "client.h"
char** mov;
char move_options[4][5] = {{"EVEN"}, {"ODD"}, {"DOUB"}, {"CON"}};

int main(int argc, char* argv[])
{
    if (argc < 3) {
        fprintf(stderr,"Usage: %s [port] [srand]\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[1]);
    srand(atoi(argv[2]));

    //INITIALISE MATCH
    if( init_match() < 0 ) {
        close(player.fd);
        exit(EXIT_FAILURE);
    }

    fcntl(player.fd, F_SETFL, O_NONBLOCK);

    mov = malloc(4 * sizeof(char*));
    for(int i = 0; i < 4; i++) {
        //mov[i] = calloc(5, sizeof(char));
        mov[i] = strdup(move_options[i]);
    }
    while(true) {
        //char *cp = calloc(MSG_SIZE, sizeof(char));
        printf("----------------------------------------------\n");
        printf("TYPE YOUR MOVE (ODD, EVEN, DOUB, CON, int):\n");
        //gets(cp);

        sleep(2);
        int i = rand()%4;

        if(strcmp(mov[i], "CON") == 0) {
            mov[i] = realloc(mov[i], 6*sizeof(char));
            int dice = rand()%6;
            sprintf(mov[i], "%s,%i", "CON", dice);
        }
        

        send_move(mov[i]); //or mov[i]
        receive_result();

        printf("PRESS ENTER TO CONTINUE\n");
        //gets(buf);
    }
}

/**
 * send message containing char * paramater
 * @return 0 to indicate success, -1 to indicate failure.
 */
int send_msg(char * s) {
    buf = calloc(MSG_SIZE, sizeof(char));
    if(buf == NULL) {
        fprintf(stderr, "send_msg(): error allocating memory to buffer. \n");
        return -1;
    }
    sprintf(buf, "%s", s);
    if( send(player.fd, buf, strlen(buf), 0) < 0 ) {
        fprintf(stderr, "Error sending %s to player.", s);
        return -1;
    }
    return 0;
}

/**
 *
 *
 */
void connect_to_server(void) {
    player.fd = socket(AF_INET, SOCK_STREAM, 0); //create an endpoint for communication

    if (player.fd < 0) {
        fprintf(stderr,"Could not create socket\n");
        exit(EXIT_FAILURE);
    }

    player.addr.sin_family = AF_INET;
    player.addr.sin_port = htons(port); //converts the unsigned short integer hostshort from host byte order to network byte order.
    player.addr.sin_addr.s_addr = htonl(INADDR_ANY); //converts the unsigned integer hostlong from host byte order to network byte order.

    opt_val = 1;

    setsockopt(player.fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

    err = connect(player.fd, (struct sockaddr *) &player.addr, sizeof(player.addr));
    if(err < 0) {
        fprintf(stderr, "Could not connect to server.\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * Tokenises char * paramater and sets
 * sets: player.lives
 * sets: player.players
 * @return 0 to indicate success, or -1 to indicate stop game involvement.
 */
int extract_start(char * s) {
    const char delim[2] = ",";
    int n = 0; //start at case 0

    char * tok = strtok(s, delim);
    while( tok != NULL ) {
        switch (n) {
            case 0 : { //START || CANCEL
                if(strcmp(tok, "START") == 0) {
                    printf("start = %s\n", tok);
                }
                else { //received "CANCEL"
                    printf("cancel = %s\n", tok);
                    return -1;
                }
                n++;
                tok = strtok(NULL, delim);
                break;
            }
            case 1 : { //number of players
                player.players = atoi(tok);
                n++;
                tok = strtok(NULL, delim);
                printf("num_players = %s\n", tok); //null
                break;
            }
            case 2 : { //number of lives
                player.lives = atoi(tok);
                n++;
                tok = strtok(NULL, delim);
                break;
            }
            default : 
                break;
        }
    }
    return 0;
}

/**
 * Handles funcition calls to initialise/enter game.
 * @return 0 to indicate game entry, -1 to indicate failure to enter game.
 */
int init_match(void) {
    connect_to_server();
    printf("Connected to server on port: %d\n\tclient:\t%d\n", port, player.fd);

    if( send_msg("INIT") < 0 ) {
        return -1;
    }
    
    buf = calloc(MSG_SIZE, sizeof(char));
    if( recv(player.fd, buf, MSG_SIZE, 0) < 0) {
        fprintf(stderr, "Error receiving reply to %s message sent.\n", "INIT");
    }

    //REJECT received
    if( strcmp(buf, "REJECT") == 0) { 
        fprintf(stderr, "Game entry rejected.\n");
        return -1;
    }

    //RECEIVE WELCOME
    if(receive_welcome(&player, buf) == 0)
        printf("%s\n", buf);
    else {
        fprintf(stderr, "Garbage message received.\n\tmessage: %s\n", buf);
        return -1;
    }

    buf = calloc(MSG_SIZE, sizeof(char));
    if( recv(player.fd, buf, MSG_SIZE, 0) <= 0) {
        fprintf(stderr, "Error receiving reply to %s message sent.\n", "INIT");
    }

    //REJECT received
    if( strcmp(buf, "CANCEL") == 0) { 
        fprintf(stderr, "Game entry rejected.\n");
        return -1;
    }

    //RECEIVE START

    //buf = calloc(MSG_SIZE, sizeof(char));
    //err = recv(player.fd, buf, MSG_SIZE, 0);
   // if( err < 0) 
        //fprintf(stderr, "Error receiving %s message sent.\n", "START");
    printf("RECEIVED: %s\n", buf);
    return extract_start(buf); //tokenise "START" message
}

/*
 * @return 0 to indicate success, -1 to indicate failure
 */
int receive_welcome(SERVER * p, char * s) {
    char delim[2] = ",";
    char * tok = strtok(s, delim);
    int n = 0;
    while(tok != NULL) {
        switch (n) {
            case 0 : { //check for "WELCOME"
                if( strcmp(tok, "WELCOME") != 0)
                    return -1;
                n++;
                tok = strtok(NULL, delim);
                continue;
            }
            case 1 : { //set player id
                p->id = atoi(tok);
                printf("\t tok:\t%s\tplayer id:\t%d\n", tok, p->id); //null
                tok = strtok(NULL, delim);
                return 0;
            }
            default : continue;
        }
    }
    return -1;
}

void send_move(char * str) {
    FD_ZERO(&active_fds);
    FD_SET(player.fd, &active_fds);
    //tv.tv_sec = 5;
    //tv.tv_usec = 0;

    int retval = select(player.fd+1, NULL, &active_fds, NULL, NULL);
    if(retval < 0) {
        fprintf(stderr, "select: error with file descriptor");
        close(player.fd);
        exit(EXIT_FAILURE);
    }
    else if( retval == 0 ) {
        fprintf(stderr, "select: could not write to fd.");
        return;
        //exit(EXIT_FAILURE);
    }
    else {
        buf = calloc(MSG_SIZE, sizeof(char));
        sprintf(buf, "%d,%s,%s", player.id, "MOV", str);

        if(send(player.fd, buf, strlen(buf), 0) < 0) {
            perror("send_move\n");
            return;
        }
        printf("SENT: %s\n", buf);
        return;
    }
}

void receive_result(void) {
    FD_ZERO(&active_fds);
    FD_SET(player.fd, &active_fds);

    int retval = select(player.fd+1, &active_fds, NULL, NULL, NULL); //BLOCKING

    fprintf(stderr, "RETURN VALUE:\t%d\n", retval);
    if(retval < 0) {
        fprintf(stderr, "select: error with file descriptor");
        close(player.fd);
        exit(EXIT_FAILURE);
    }
    else if( retval == 0 ) {
        fprintf(stderr, "select (RESULT): could not read from fd.\n");
        return;
        //exit(EXIT_FAILURE);
    }
    else {
        buf = calloc(MSG_SIZE, sizeof(char));
        recv(player.fd, buf, MSG_SIZE, 0);
        printf("RECEIVED: %s\n", buf);

        char delim[2] = ",";
        char * tok = strtok(buf, delim);
        while(tok != NULL) {
            int id = atoi(tok);
            if(id != player.id) {
                perror("MESSAGE RECEIVED IS NOT INTENDED FOR THIS CLIENT\n");
                return;
            } 
            tok = strtok(NULL, delim);
            if(strcmp(tok, "PASS") == 0) { //PASS
                return;
            }
            else if(strcmp(tok, "FAIL") == 0){ //FAIL
                player.lives -= 1;
                return;
            }
            else if(strcmp(tok, "ELIM") == 0) { //ELIM
                close(player.fd);
                printf("END OF GAME!\n");
                exit(EXIT_SUCCESS);
            }
            else if(strcmp(tok, "VICT") == 0) { //VICT
                close(player.fd);
                printf("END OF GAME!\n");
                exit(EXIT_SUCCESS);
            }
            else {
                fprintf(stderr, "RECEIVED ODD PACKET!\n");
                return;
            }
        }
    }
}


