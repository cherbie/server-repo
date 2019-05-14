#include "server.h"

/** 
 * @return 0 to indicate successful execution. -1 to indicate failure.
 * The "Lobby" simulating game play.
 */
int start_game(void) {
    printf("GAME PLAYERS\n");
    for(int i = 0; i < servers[0].num_players; i++)
        printf("\t%d\n", players[i].id);

    if(servers[0].num_players == NUM_PLAYERS) { //ENOUGH PLAYERS IN THE GAME
        set_player_lives(); //set players initial lives
        send_start(); //send "START, %d, %d" packet
    }
    else {
        send_cancel(); //send "CANCEL"
        return -1;
    }

    //CYCLE THROUGH EACH PLAYER
    while(true) {
        play_game_round();
        printf("ENTER STRING TO CONTINUE.\n");
        buf = calloc(MSG_SIZE, sizeof(char));
        gets(buf);
    }
}

void play_game_round( void ) {
    PLAYER * p;
    int active_players = queue.count; //iterate through all players once.
    printf("------------------\n");
    for(int i = 0; i < active_players; i++) {
        p = dequeue_front(&queue); //get player
        err = receive_move(p);
        switch (err) {
            case 0 : { //success
                printf("RECEIVED: %s / %d\n", p->move, p->id);
            }
            case -1 : { //failure

            }
            case -2 : { //timeout failure
            
            }
            case -3 : { // CLIENT LEFT

            }
        }
    }
    roll_dice(&servers[0]);
    for(int i = 0; i < NUM_PLAYERS; i++) {
        if(!players[i].alive) continue;
        send_success(&players[i]); //handle comparison and sending
    }
}

void set_player_lives(void) {
    for(int i = 0; i < servers[0].num_players; i++) {
        players[i].lives = NUM_LIVES;
        players[i].alive = true;
    }
}

/**
 * Wait for game player to send move. EVEN || ODD || DOUB || CON,%d
 * On player timeout, set player.alive status to FALSE
 * @return 0 to indicate success, -1 to indicate error
 * @return -3 to indicate proper client file descriptor shutdown
 */
int receive_move(PLAYER * p) {
    char * cp = calloc(MSG_SIZE, sizeof(char));
    int suc = recv(p->fd, cp, MSG_SIZE, 0);
    if( suc < 0 ) //FAIL
        return -1;
    else if( suc == 0) //proper client server exit
        return -3;

    //RECIEVE MOVE
    if(parse_move(p, cp) < 0 ) { //FAIL
        return -1;
    }
    return 0;
}

/**
 * Roll server dice and set server.dice1 & server.dice2 to respective values
 */
void roll_dice(SERVER * s) {
    s->dice = malloc(NUM_DICE * sizeof(int));
    for(int i = 0; i < NUM_DICE; i++)
        s->dice[i] = (rand()%6 + 1);
    printf("dice1:\t%d\tdice2:\t%d\n", s->dice[0], s->dice[1]);
}

/**
 * SEND to player if successful or failure. ATTEMPT at most twice to send notification
 * UPDATE player lives
 * @return 0 on succesful player notification, -1 otherwise.
 **/
int send_success(PLAYER * p) {
    //ANALYSE MOVE & SEND PASS OR FAIL
    if(move_is_correct(p)) {
        if(send_pass(p) < 0)
            return send_pass(p);
    }
    if(send_fail(p) < 0) { //attempt to send "FAIL"
        p->lives -= 1; //reduce player lives
        return send_fail(p);
    }
    return 0; //indicate success
}

/**
 * BREAK CLIENTS PACKET INTO READABLE SEGMENTS
 * @return 0 to indicate successful interpretation, -1 to indicate error.
 */
int parse_move(PLAYER * p, char * move) {
    char delim[2] = ",";
    char * tok = strtok(move, delim);
    int n = 0;
    while(tok != NULL) {
        switch(n) {
            case 0 : { //player id verification
                int id = atoi(tok);
                if(id != p->id) {
                    fprintf(stderr, "CLIENT HAS CLAIMED TO BE ANOTHER PLAYER: %d\n", p->id);
                    return -1;
                }
                n++;
                tok = strtok(NULL, delim);
                continue;
            }
            case 1 : { //MOV
                if(strcmp(tok, "MOV") != 0) {
                    fprintf(stderr, "PLAYER HAS NOT SENT AN 'MOV' packet: %d\n", p->id);
                    return -1;
                }
                n++;
                tok = strtok(NULL, delim);
                continue;
            }
            case 2 : { //MOVE -- EVEN || ODD || DOUB || CON
                if(strcmp(tok, "EVEN") == 0) { //EVEN
                    p->move = calloc(MSG_SIZE, sizeof(char));
                    sprintf(p->move, "%s", "EVEN");
                    return 0;
                }
                else if(strcmp(tok, "ODD") == 0) { //ODD
                    p->move = calloc(MSG_SIZE, sizeof(char));
                    sprintf(p->move, "%s", "ODD");
                    return 0;
                }
                else if(strcmp(tok, "DOUB") == 0) { //DOUB
                    p->move = calloc(MSG_SIZE, sizeof(char));
                    sprintf(p->move, "%s", "DOUB");
                    return 0;
                }
                else if(strcmp(tok, "CON") == 0) { //CON
                    p->move = calloc(MSG_SIZE, sizeof(char));
                    sprintf(p->move, "%s", "CON");
                    tok = strtok(NULL, delim);
                    continue;
                }
            }
            case 3 : { //%d
                p->roll = atoi(tok);
                return 0;
            }
            default : p->move = NULL;
                break;
        }
    }
    return -1; //not meant to reach this stage
}

/**
 * Dice value comparison logic.
 * @return true if correct, false otherwise.
**/
bool move_is_correct(PLAYER * p) {
    return false;
}

/**
 * SEND "%d,PASS"
 * @return 0 to indicate success, -1 otherwise.
 */
int send_pass(PLAYER * p) {
    return -1;
}

/**
 * SEND "%d,FAIL"
 * @return 0 to indicate success, -1 otherwise.
 */
int send_fail(PLAYER * p) {
    return -1;
}
