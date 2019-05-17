#include "server.h"

/** 
 * @return 0 to indicate successful execution. -1 to indicate failure.
 * The "Lobby" simulating game play.
 */
int start_game(void) {
    printf("GAME PLAYERS\n");
    for(int i = 0; i < server.num_players; i++)
        printf("\t%d\n", players[i].id);

    if(server.num_players == NUM_PLAYERS) { //ENOUGH PLAYERS IN THE GAME
        set_player_lives(); //set players initial lives
        send_start(); //send "START, %d, %d" packet
        construct_queue(&dead_queue, NUM_PLAYERS);
    }
    else {
        send_cancel(); //send "CANCEL"
        return -1;
    }

    //CYCLE THROUGH EACH PLAYER
    while(true) {
        printf("ENTER STRING TO CONTINUE.\n");
        gets(buf);
        if(play_game_round() == 0) break; //player victory game over
    }
    return 0;
}

/**
 * @return 0 to indicate player victory and -1 to indicate outstanding players
 *
 */
int play_game_round( void ) {
    PLAYER * p;

    memcpy( &rfds, &active_fds, sizeof(active_fds));
    tv.tv_sec = 10;
    tv.tv_usec = 0;

    conn_err = select(FD_SETSIZE, &rfds, NULL, NULL, &tv); //5 seconds to make move
    if(conn_err < 0) {
        perror("SELECT\n");
        exit(EXIT_FAILURE);
    }
    else if(conn_err == 0) {
        fprintf(stderr, "NO GAME PLAYERS MADE A MOVE\n");
        return -1;
    }
    //MORE THAN 0 PLAYERS HAVE SENT A MOVE
    int active_players = size(&queue); //iterate through all players once.
    printf("------------------\n");
    for(int i = 0; i < active_players; i++) {
        p = dequeue_front(&queue); //get player
        if(!FD_ISSET(p->fd, &rfds)) { //MOVE NOT RECEIVED
            printf("FAILED TO RECEIVE MOVE: %d\n", p->id);
            p->move = NULL; //player->move is equal to?
            enqueue(&queue, p);
            continue;
        }
        else { //CONNECTION FD ready for reading
            err = receive_move(p);
            if(err == -3) { //CLIENT HAS LEFT THE GAME
                printf("CLIENT: %d has left the game.\n", p->id);
                p->alive = false;
                close(p->fd);
                continue;
            }
            else if(err < 0) { //error receiving move (timeout or otherwise)
                printf("FAILED TO RECEIVE MOVE: %d\n", p->id);
                p->move = NULL; //player->move is equal to?
                enqueue(&queue, p);
                continue;
            }
            else { // SUCCESS
                printf("RECEIVED: %s / %d\n", p->move, p->id);
                enqueue(&queue, p);
                continue;
            }
        }
    }

    //ROLL
    roll_dice(&server);

    //OUTCOME
    active_players = size(&queue);
    for(int i = 0; i < active_players; i++) {
        p = dequeue_front(&queue);
        if(!p->alive || p == NULL) continue; //cautionary measure
        printf("CHECKING IF MOVE CORRRECT\n");
        if(!move_is_correct(p)) { //handle comparison & lives //sending message
            //NOT CORRECT
            p->correct = false;
            if(p->lives <= 0) { //FAILED AND NO LIVES REMAINING
                p->alive = false; //set player to not alive
                enqueue(&dead_queue, p); //add player to dead_queue
                continue;
            }
            //LIVES REMAINING
            //send_fail(p);
            enqueue(&queue, p); //add back to active queue
            continue;
        }
        else { //CORRECT MOVE DECISION
            p->correct = true;
            enqueue(&queue, p);
            continue;
        }
    }

    //SEND STATUS
    if(size(&queue) <= 0 && size(&dead_queue) > 0) { //no more players in game
        //SEND VICTORY to all people in dead_queue;
        active_players = size(&dead_queue);
        for(int i = 0; i < active_players; i++) {
            p = dequeue_front(&dead_queue);
            send_vict(p);
            close(p->fd);
            continue;
        }
        return 0; //victory
    }
    else if(size(&queue) == 1) { //one winner.
        //SEND VICTORY to person in queue
        p = dequeue_front(&queue);
        send_vict(p); //handle failure to send?
        close(p->fd);

        //SEND ELIMINATION TO ALL PEOPLE IN dead_queue;
        active_players = size(&dead_queue);
        if(active_players < 0) return 0;
        for(int i = 0; i < active_players; i++) {
            p = dequeue_front(&dead_queue);
            send_elim(p); //handle failure to send?
            FD_CLR(p->fd, &active_fds);
            close(p->fd);
        }
        return 0; //victory
    }
    else { //MULTIPLE GAME PLAYERS IN GAME
        //HANDLE DEAD_QUEUE
        active_players = size(&dead_queue);
        if(active_players > 0) {
            for(int i = 0; i < active_players; i++) { //SEND ELIM message & close file descriptor
                p = dequeue_front(&dead_queue);
                send_elim(p);
                FD_CLR(p->fd, &active_fds);
                close(p->fd);
            }
        }

        //HANDLE ACTIVE PLAYERS
        active_players = size(&queue);
        if(active_players > 0) {
            for(int i = 0; i < active_players; i++) {
                p = dequeue_front(&queue);
                if(p->correct) { //CORRECT MOVE
                    send_pass(p);
                    enqueue(&queue, p);
                    continue;
                }
                else {
                    send_fail(p);
                    enqueue(&queue, p);
                    continue;
                }
            }
        }
    }
    //play next round
    printf("PLAY NEXT ROUND\n");
    return -1;
}

/**
 * INITIALISE: player lives, alive status to true & active_fds
 */
void set_player_lives(void) {
    FD_ZERO(&active_fds);
    for(int i = 0; i < server.num_players; i++) {
        players[i].lives = NUM_LIVES;
        players[i].alive = true;
        FD_SET(players[i].fd, &active_fds);
    }
}

/**
 * TEMPORARY BLOCKING HANDLED BEFORE FUNCTION CALL
 * player to send move. EVEN || ODD || DOUB || CON,%d
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
    for(int i = 0; i < NUM_DICE; i++) s->dice[i] = (rand()%6 + 1);
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
        return send_pass(p); //if lost connection to player will send 0 (success) ??
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
    int n = 0; //handle switch ordering
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
    //DICE roll is in server.dice[0] & server.dice[1]
    int dice1 = server.dice[0];
    int dice2 = server.dice[1];
    if(p->lives > 0) {
        if( p->move == NULL) {
            p->lives -= 1;
            return false;
        }
        else if(strcmp(p->move, "CON") && p->roll >= 1 && p->roll <= 6) { //"CON,%d" player move
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
        return false;
    }
    else return false;
}

/**
 * SEND "%d,PASS"
 * @return 0 to indicate success, -1 otherwise.
 */
int send_pass(PLAYER * p) {
    FD_ZERO(&wfds);
    FD_SET(p->fd, &wfds);
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    if(select(p->fd+1, NULL, &wfds, NULL, &tv) <= 0) {//BLOCKING
        perror("select (send_pass): error\n");
        return -1;
    }

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
    FD_ZERO(&wfds);
    FD_SET(p->fd, &wfds);

    tv.tv_sec = 5;
    tv.tv_usec = 0;

    if(select(p->fd+1, NULL, &wfds, NULL, &tv) < 0) //BLOCKING
        return -1;
    
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
    FD_ZERO(&wfds);
    FD_SET(p->fd, &wfds);

    tv.tv_sec = 5;
    tv.tv_usec = 0;

    if(select(p->fd+1, NULL, &wfds, NULL, &tv) <= 0) //BLOCKING
        return -1;
    
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
    FD_ZERO(&wfds);
    FD_SET(p->fd, &wfds);

    tv.tv_sec = 5;
    tv.tv_usec = 0;

    if(select(p->fd+1, NULL, &wfds, NULL, &tv) <= 0) //BLOCKING
        return -1;
    
    buf = calloc(MSG_SIZE, sizeof(char));
    sprintf(buf, "%d,%s", p->id, "ELIM");
    err = send(p->fd, buf, strlen(buf), 0);
    printf("%s\n", buf);
    if(err < 0)
        return -1;
    else 
        return 0;
}

