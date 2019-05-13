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

        for(int i = 0; i < NUM_PLAYERS; i++) {
            if(!players[i].alive) continue;
            receive_move(&players[i]);
        }
        roll_dice(&servers[0]);
        for(int i = 0; i < NUM_PLAYERS; i++) {
            if(!players[i].alive) continue;
            send_success(&players[i]); //handle comparison and sending
        }

        printf("Enter !~ characters to quit program.\n");
        buf = calloc(MSG_SIZE, sizeof(char));
        gets(buf);
        if(strcmp(buf, "!~") == 0) {
            free(buf);
            free(players);
            exit(EXIT_SUCCESS);
        }
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
 */
void receive_move(PLAYER * p) {

}

/**
 * Roll server dice and set server.dice1 & server.dice2 to respective values
 */
void roll_dice(SERVER * s) {
    s->dice = malloc(NUM_DICE * sizeof(int));
    for(int i = 0; i < NUM_DICE; i++)
        s->dice[i] = (rand()%6 + 1);
    printf("dice1:\t%d\ndice2:\t%d\n", s->dice[0], s->dice[1]);
}

void send_success(PLAYER * p) {

}
