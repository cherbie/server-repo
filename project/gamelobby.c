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
        if(play_game_round() == 0) break;
        printf("ENTER STRING TO CONTINUE.\n");
        buf = calloc(MSG_SIZE, sizeof(char));
        gets(buf);
    }
    return 0;
}

/**
 * @return 0 to indicate player victory and -1 to indicate outstanding players
 *
 */
int play_game_round( void ) {
    PLAYER * p;
    int active_players = queue.count; //iterate through all players once.
    printf("------------------\n");
    for(int i = 0; i < active_players; i++) {
        p = dequeue_front(&queue); //get player
        err = receive_move(p);
        switch (err) {
            case 0 : { //success
                printf("RECEIVED: %s / %d\n", p->move, p->id);
                enqueue(&queue, p);
                break;
            }
            case -1 : { //failure
                printf("FAILED TO RECEIVE MOVE: %d\n", p->id);
                p->move = NULL; //player->move is equal to?
                enqueue(&queue, p);
                break;
            }
            case -2 : { //timeout failure
                printf("TIMEOUT to receive move from: %d\n", p->id);
                p->move = NULL; //what is player->move equal to
                enqueue(&queue, p);
                break;
            }
            case -3 : { // CLIENT LEFT
                printf("CLIENT: %d has left the game.\n", p->id);
                p->alive = false;
                close(p->fd);
                //close player & set alive to false;
                continue;
            }
            default : continue;
        }
    }

    //ROLL
    roll_dice(&servers[0]);

    //OUTCOME
    QUEUE dead_queue; //queue managing players that are not alive
    construct_queue(&dead_queue, NUM_PLAYERS);
    active_players = queue.count;
    for(int i = 0; i < active_players; i++) {
        p = dequeue_front(&queue);
        if(!p->alive) continue;
        err = send_outcome(p); //handle comparison, lives and sending message
        if(err < 0) 
            fprintf(stderr, "ERROR sending outcome to client: %d\n", p->id);
        if(p->lives <= 0) { //failed and NO lives
            p->alive = false; //set player to not alive
            enqueue(&dead_queue, p); //add player to dead_queue
            continue;
        }
        else //lives is > 0
            enqueue(&queue, p);
        continue;
    }

    //SEND STATUS
    if(queue.count <= 0 && dead_queue.count > 0) { //no more players in game
        //SEND VICTORY to all people in dead_queue;
        active_players = dead_queue.count;
        for(int i = 0; i < active_players; i++) {
            p = dequeue_front(&dead_queue);
            send_vict(p);
            continue;
        }
        return 0; //victory
    }
    else if(queue.count == 1) { //one winner.
        //SEND VICTORY to people in queue
        p = dequeue_front(&queue);
        send_vict(p); //handle failure to send?
        //SEND ELIMINATION TO ALL PEOPLE IN dead_queue;
        active_players = dead_queue.count;
        for(int i = 0; i < active_players; i++) {
            p = dequeue_front(&dead_queue);
            send_elim(p); //handle failrue to send?
            continue;
        }
        return 0; //victory
    }
    else {
        //DEAD_QUEUE
        active_players = dead_queue.count;
        if(active_players > 0) {
            for(int i = 0; i < active_players; i++) {
                p = dequeue_front(&dead_queue);
                send_elim(p); //handle failure to send?
                continue;
            }
        }
    }
    //play next round
    printf("PLAY NEXT ROUND\n");
    return -1;
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
 * @return -2 to indicate timeout
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
    return;
}

/**
 * SEND to player if successful or failure. ATTEMPT at most twice to send notification
 * @return 0 on succesful player notification, -1 otherwise.
 **/
int send_outcome(PLAYER * p) {
    //ANALYSE MOVE & SEND PASS OR FAIL
    if(move_is_correct(p))
        return send_pass(p); //if lost connection to player will send 0 (success) .. not goog
    else 
        return send_fail(p); //attempt to send "FAIL"
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
                else {
                    p->move = NULL;
                    return -1;
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
 * UPDATE player lives
 * @return true if correct, false otherwise.
**/
bool move_is_correct(PLAYER * p) {
    //PLAYER MOVE IS in p->move & p->roll
    //DICE roll is in servers[0].dice[0] & servers[0].dice[1]
    int dice1 = servers[0].dice[0];
    int dice2 = servers[0].dice[1];
    if(p->lives > 0) {
        if(strcmp(p->move, "CON") && p->roll >= 1 && p->roll <= 6) { //"CON,%d" player move
            if(p->roll == dice1|| p->roll == dice2) { //player passed
                return true; // do not change lives
            }
            else p->lives -= 1;
            return false;
        }
        else if(strcmp(p->move, "EVEN") == 0 && (dice1+dice2)%2 == 0 && dice1 != dice2) { //player passed
            return true;
        }
        else if(strcmp(p->move, "ODD") == 0 && (dice1+dice2)%2 == 1 && (dice1+dice2)>5) { //player passed
            return true;
        }
        else if(strcmp(p->move, "DOUB") == 0 && dice1 == dice2) { //player passed
            return true;
        }
        p->lives -= 1; //player failed
    }    
    return false;
}

/**
 * SEND "%d,PASS"
 * @return 0 to indicate success, -1 otherwise.
 */
int send_pass(PLAYER * p) {
    buf = calloc(MSG_SIZE, sizeof(char));
    sprintf(buf, "%d,%s", p->id, "PASS");
    err = send(p->fd, buf, strlen(buf), 0);
    printf("%s\n", buf);
    if(err < 0)
        return -1;
    else 
        return 0;
}

/**
 * SEND "%d,FAIL"
 * @return 0 to indicate success, -1 otherwise.
 */
int send_fail(PLAYER * p) {
    buf = calloc(MSG_SIZE, sizeof(char));
    sprintf(buf, "%d,%s", p->id, "FAIL");
    err = send(p->fd, buf, strlen(buf), 0);
    printf("%s\n", buf);
    if(err < 0)
        return -1;
    else 
        return 0;
}

/**
 * SEND "%d,VICT"
 * @return 0 to indicate success and -1 otherwise
 */
int send_vict(PLAYER * p) {
    buf = calloc(MSG_SIZE, sizeof(char));
    sprintf(buf, "%d,%s", p->id, "VICT");
    err = send(p->fd, buf, strlen(buf), 0);
    printf("%s\n", buf);
    if(err < 0)
        return -1;
    else 
        return 0;
}

/**
 * SEND "%d,ELIM"
 * @return 0 to indicate success and -1 otherwise
 */
int send_elim(PLAYER * p) {
    buf = calloc(MSG_SIZE, sizeof(char));
    sprintf(buf, "%d,%s", p->id, "ELIM");
    err = send(p->fd, buf, strlen(buf), 0);
    printf("%s\n", buf);
    if(err < 0)
        return -1;
    else 
        return 0;
}

