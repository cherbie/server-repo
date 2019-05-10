    
#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include "stdbool.h"
#include "string.h"

bool singlewinner;


typedef struct {
    int id;
    char* input;
    int lives;
} PLAYER;


int diceRoll(){
    
    return (rand()%6 +1);
}

void play(PLAYER players[]){
    printf("Play - round 1\n");
    int dice1 = diceRoll();
    int dice2 = diceRoll();
    printf("dice: %d,%d \n",dice1, dice2);
    players[0].input = "even"; //to test have each player have same input each turn, no matter what.
    players[1].input = "odd";
    players[2].input = "3";
    
    for(int i = 0; i<3; i++){ //loop through number of players
        if(players[i].lives != 0){
        if(players[i].input[0] >= '1' && players[i].input[0] <= '6'){
            int input = atoi(players[i].input);
            if(input == dice1 || input == dice2){
				//player passed
                continue;
            }
             players[i].lives -=1;
        }else{
            if(strncmp(players[i].input, "even", 8)==0 && (dice1+dice2) %2 == 0 && dice1 != dice2){
                //player passed
				continue;
            }
            if(strncmp(players[i].input, "odd", 8)==0 && (dice1+dice2)%2 ==1 && (dice1+dice2)>5){
                //player passed
				continue;
            }
            if(strncmp(players[i].input, "doubles", 8)==0 && dice1==dice2){
                //player passed
				continue;
            }
			//player failed
            players[i].lives -=1;
        }
        }
    }
    for(int i = 0; i<3; i++){
        printf("id = %d, lives = %d\n", players[i].id, players[i].lives);
    }
    
}

int main()
{
    srand(time(NULL));
    PLAYER players[3];
    for(int i = 0; i<3; i++){
        players[i].id = i+1;
        players[i].input = NULL;
        players[i].lives = 3;
    }
    printf("INITIALIZATION \n");
    for(int i = 0; i<3; i++){
        printf("id = %d, lives = %d\n", players[i].id, players[i].lives);
    }
    int playersalive = 3;
    while(playersalive>1){
        play(players);
        playersalive = 0;
        for(int i = 0; i<3; i++){
            if(players[i].lives == 0){
                //send message to client informing them they are dead.
            }else{
                playersalive+=1;
            }
        }
    }
    singlewinner = false;
    for(int i = 0; i<3; i++){
        if(players[i].lives !=0){
            //declare winner/ VICT
            printf("Player %d wins! \n", i+1);
            singlewinner = true;
            break;
        }
    }
    if(!singlewinner){
        printf("Everyone loses \n");
    }
    return 0;
}