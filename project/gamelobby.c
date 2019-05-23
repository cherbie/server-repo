/**
* AUTHORS: CLAYTON HERBST (22245091) & FRASER LONERAGAN (22243455)
* UNIT : CITS3002 "COMPUTER NETWORKS" -- THE UNIVERSITY OF WESTERN AUSTRALIA
* COORDINATOR : DR JIN HONG
*/

#include "server.h"

/** 
 * The "Lobby" simulating game play.
 * @return 0 to indicate successful execution. -1 to indicate failure.
 */
int start_game(void) {
    if(server.num_players == NUM_PLAYERS) { //ENOUGH PLAYERS IN THE GAME
        set_player_lives(); //set players initial lives
        send_start(); //send "START, %d, %d" packet
        construct_queue(&dead_queue);
    }
    else {
        send_cancel(); //send "CANCEL"
        return -1;
    }

    //CYCLE THROUGH ROUNDS UNTIL THERE IS A VICTOR.
    while(true) {
        int game_err = play_game_round();
        if(game_err == 0) return 0; //PLAYER VICTORY ... GAME OVER
        else if(game_err == -2) return -1; //
    }
    return 0;
}

/**
 * @return 0 to indicate player victory and -1 to indicate outstanding players. 
 * @return -2 to indicate immediate closure of game lobby
 *
 */
int play_game_round( void ) {
    PLAYER * p;

    FD_ZERO(&rfds);
    FD_COPY(&active_fds, &rfds);
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    sleep(WAIT_TIME_MOVE); //PLAYERS MOVES ACCUMULATE IN CONNECTION BUFFER

    conn_err = select(FD_SETSIZE, &rfds, NULL, NULL, &tv);
    if(conn_err < 0) { //ERROR
        if(errno == EBADF) {
            fprintf(stderr, "ERRNO = %d:\n\tThere was a bad file descriptor.\n", errno);
        }
        else {
            fprintf(stderr, "ERRNO = %d:", errno);
            perror("\t");
        }
    }
    else if(conn_err == 0) { //NO GAME PLAYERS MADE A MOVE HENCE DON'T DEDUCT A LIFE.
        fprintf(stderr, "NO GAME PLAYERS MADE A MOVE\n");
        return -1;
    }
    //MORE THAN 0 PLAYERS HAVE SENT A MOVE
    int active_players = size(&queue); //iterate through all players once.
    for(int i = 0; i < active_players; i++) {
        p = dequeue_front(&queue); //get player
        if(!FD_ISSET(p->fd, &rfds)) { //MOVE NOT RECEIVED
            fprintf(stderr, "FAILED TO RECEIVE MOVE: %d\n", p->id);
            p->move = NULL; //register no player move
            enqueue(&queue, p);
            continue;
        }
        else { //CONNECTION FD ready for reading
            err = receive_move(p);
            if(err < 0) { //CLIENT HAS LEFT THE GAME. ERRNO 54 or 60 is set if client has left
                fprintf(stderr, "CLIENT: %d has left the game.\n", p->id);
                p->alive = false;
                FD_CLR(p->fd, &active_fds); //CLEAR PLAYER FD FROM SET OF ACTIVE FD's
                close(p->fd); //SOCKET STILL REMAINS IN TIME-WAIT STATE HOWEVER FOR 1 MINUTE AS DEFINED IN tcp.h
                continue;
            }
            else { // SUCCESS
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
        if(!move_is_correct(p)) { //handle comparison & lives
            //NOT CORRECT
            p->correct = false;
            if(p->lives <= 0) { //FAILED AND NO LIVES REMAINING
                p->alive = false; //set player to not alive
                enqueue(&dead_queue, p); //add player to dead_queue
                continue;
            }
            //LIVES REMAINING
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
    active_players = size(&dead_queue);
    if(size(&queue) <= 0 && size(&dead_queue) > 0) { //NO MORE PLAYERS IN THE GAME
        //SEND VICTORY to all people in dead_queue;
        active_players = size(&dead_queue);
        for(int i = 0; i < active_players; i++) {
            p = dequeue_front(&dead_queue);
            if(send_vict(p) < 0) perror("send victory error.\n");
            FD_CLR(p->fd, &active_fds);
            close(p->fd);
            continue;
        }
        return 0; //victory
    }
    else if(size(&queue) == 1) { //ONE WINNER
        //SEND VICTORY to person in queue
        p = dequeue_front(&queue);
        if(send_vict(p) < 0) //handle failure to send?
            perror("send victory error.\n");
        FD_CLR(p->fd, &active_fds);
        p->alive = false;
        close(p->fd);

        //SEND ELIMINATION TO ALL PEOPLE IN dead_queue;
        active_players = size(&dead_queue);
        if(active_players < 0) return 0;
        for(int i = 0; i < active_players; i++) {
            p = dequeue_front(&dead_queue);
            if(send_elim(p) < 0) perror("send elimination error.\n"); //handle failure to send?
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
                if(send_elim(p) < 0) perror("send elimination error.\n");
                FD_CLR(p->fd, &active_fds);
                close(p->fd);
            }
        }

        //HANDLE ACTIVE PLAYERS
        active_players = size(&queue);
        if(active_players > 0) {
            for(int i = 0; i < active_players; i++) {
                p = dequeue_front(&queue);
                if(p == NULL) continue;
                if(p->correct) { //CORRECT MOVE
                    if(send_pass(p) < 0) { //error sending. assume socket has been closed
                        perror("send pass error.\n");
                        FD_CLR(p->fd, &active_fds);
                        p->alive = false;
                        close(p->fd);
                        continue;
                    }
                    else enqueue(&queue, p);
                    continue;
                }
                else {
                    if(send_fail(p) < 0) { //error sending. Assume socket has been closed.
                        perror("send fail error.\n");
                        FD_CLR(p->fd, &active_fds);
                        p->alive = false;
                        close(p->fd);
                        continue;
                    }
                    else enqueue(&queue, p);
                    continue;
                }
            }
        }
    }
    //PLAY NEXT ROUND
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
 * @return 0 to indicate success, -1 to indicate error, -3 to indicate EBADF error
 */
int receive_move(PLAYER * p) {
    char * cp = calloc(MSG_SIZE, sizeof(char));
    int suc = recv(p->fd, cp, MSG_SIZE, 0);
    if( suc <= 0 ) { //FAIL
        fprintf(stderr, "receive_move: returned = %d\t errno = %d\n", suc, errno);
        perror(NULL);
        free(cp);
        return -1;
    }
    
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
    return;
}

/**
 * BREAK CLIENTS PACKET INTO READABLE SEGMENTS
 * @return 0 to indicate successful interpretation, -1 to indicate error.
 */
int parse_move(PLAYER * p, char * cp) {
    char delim[2] = ",";
    char * tok = strtok(cp, delim);
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
                    fprintf(stderr, "PLAYER HAS NOT SENT A 'MOV' packet: %d\n", p->id);
                    return -1;
                }
                n++;
                tok = strtok(NULL, delim);
                continue;
            }
            case 2 : { //MOVE -- EVEN || ODD || DOUB || CON
                upper_string(tok);
                if(tok == NULL) return -1;
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
                    n++; //go to next packet element
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
 * convert all characters before null-byte to uppercase.
 */
void upper_string(char * cp) {
   int i = 0;
   while (cp[i] != '\0') {
      if (cp[i] >= 'a' && cp[i] <= 'z') {
         cp[i] = cp[i] - 32;
      }
      i++;
   }
}

/**
 * Dice value vs player move comparison logic.
 * UPDATE player lives.
 * @return true if correct, false otherwise.
**/
bool move_is_correct(PLAYER * p) {
    //PLAYER MOVE IS in p->move & p->roll
    //DICE roll is in server.dice[0] & server.dice[1]
    int dice1 = server.dice[0];
    int dice2 = server.dice[1];
    if(p->lives > 0) {
        if(p->move == NULL) {
            p->lives -= 1;
            return false;
        }
        else if(strcmp(p->move, "CON") == 0 && p->roll >= 1 && p->roll <= 6) { //"CON,%d" player move
            if(p->roll == dice1 || p->roll == dice2) { //player passed
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
    tv.tv_sec = WAIT_TIME_SEND;
    tv.tv_usec = 0;

    if(select(p->fd+1, NULL, &wfds, NULL, &tv) <= 0) {//BLOCKING
        perror("select (send_pass): error\n");
        return -1;
    }
    buf = calloc(MSG_SIZE, sizeof(char));
    if(sprintf(buf, "%d,%s", p->id, "PASS") <= 0) return -1;
    err = send(p->fd, buf, strlen(buf), 0);
    if(err <= 0) {
        return -1;
    }
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

    tv.tv_sec = WAIT_TIME_SEND;
    tv.tv_usec = 0;

    if(select(p->fd+1, NULL, &wfds, NULL, &tv) <= 0) { //BLOCKING
        fprintf(stderr, "send_fail: select returned < 0\n");
        return -1;
    }
    
    buf = calloc(MSG_SIZE, sizeof(char));
    if(sprintf(buf, "%d,%s", p->id, "FAIL") <= 0) return -1;
    err = send(p->fd, buf, strlen(buf), 0);
    if(err <= 0) {
        return -1;
    }
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

    tv.tv_sec = WAIT_TIME_SEND;
    tv.tv_usec = 0;

    if(select(p->fd+1, NULL, &wfds, NULL, &tv) <= 0) //BLOCKING
        return -1;
    
    buf = calloc(MSG_SIZE, sizeof(char));
    if(sprintf(buf, "%d,%s", p->id, "VICT") <= 0) return -1;
    err = send(p->fd, buf, strlen(buf), 0);
    if(err <= 0)
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

    tv.tv_sec = WAIT_TIME_SEND;
    tv.tv_usec = 0;

    if(select(p->fd+1, NULL, &wfds, NULL, &tv) <= 0) //BLOCKING
        return -1;
    
    buf = calloc(MSG_SIZE, sizeof(char));
    if(sprintf(buf, "%d,%s", p->id, "ELIM") <= 0) return -1;
    err = send(p->fd, buf, strlen(buf), 0);
    if(err < 0)
        return -1;
    else 
        return 0;
}

