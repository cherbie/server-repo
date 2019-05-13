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


