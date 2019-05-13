/** MANAGE QUEUE OF ACTIVE PLAYERS **/
#include "server.h"

PLAYER * examine(QUEUE * q) {
    return q->elements[q->front];
}

bool isEmpty(QUEUE* q) {
   return q->count <= 0;
}

bool isFull(QUEUE* q) {
   return q->count >= q->length;
}

int size(QUEUE * q) {
   return q->count;
}

/**
 * @return 0 to indicate success, -1 to indicate failure
 */
int enqueue(QUEUE * q, PLAYER * player) {
    if(!isFull(q)) {
        int last = abs((q->front + q->count))%q->length; //cyclic property of queue
        q->count++; //increment count of elements
        q->elements[last] = player;
        return 0;
    }
    else 
        return -1;
}

/**
 * @return int id on success, -1 on failure;
 */
PLAYER * dequeue_front(QUEUE * q) {
   if(!isEmpty(q)) {
        PLAYER * player = q->elements[q->front];
        q->elements[q->front] = NULL; //erase memory address
        q->front = (++q->front)%q->length;
        q->count--; //reduce number of elements in the queue
        return player;
    }
    else
        return NULL;
}

PLAYER * dequeue_last(QUEUE * q) {
    if(!isEmpty(q)) {
        int last = abs((q->front + q->count -1))%q->length; //cyclic property of queue
        PLAYER * player = q->elements[last];
        q->elements[last] = NULL;
        q->count -= 1;
        return player;
    }
    else
        return NULL; //empty
}
/*
int main(int argc, char * argv[]) {
    QUEUE queue;
    queue.front = 0;
    queue.count = 0;
    queue.length = NUM_PLAYERS;

    players = malloc(4 * sizeof(players));
    players[0].id = 2;
    players[1].id = 5;
    players[2].id = 9;
    players[3].id = 7;

    enqueue(&queue, &players[0]);
    printf("%d\n", dequeue_front(&queue)->id);
    enqueue(&queue, &players[1]);
    enqueue(&queue, &players[2]);
    printf("%d\n", enqueue(&queue, &players[3]));
    printf("%d\n", examine(&queue)->id);
    printf("%d\n", dequeue_last(&queue)->id);
    printf("%d\n", examine(&queue)->id);
    printf("%d\n", enqueue(&queue, &players[3]));
    while(examine(&queue) != NULL) {
        printf("-%d\t", dequeue_last(&queue)->id);
    }
}
*/
