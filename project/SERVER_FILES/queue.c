/** MANAGE QUEUE OF ACTIVE PLAYERS **/
#include "server.h"

/**
 * CONSTRUCTOR FOR QUEUE ADT
 * @param q QUEUE struct
 * @return 0 to indicate success, -1 to indicate failure
 */
int construct_queue(QUEUE * q) {
    q->front = 0;
    q->count = 0;
    q->length = NUM_PLAYERS;
    return 0;
}

/**
* Examine the element at the front of the queue.
* @param q QUEUE to be examined
* @return (PLAYER *) element at the front of the queue
*/
PLAYER * examine(QUEUE * q) {
    return q->elements[q->front];
}

/**
* Verifies whether the queue is empty.
* @return true if the queue is empty, false otherwise.
*/
bool isEmpty(QUEUE* q) {
   return q->count <= 0;
}

/**
* Verifies whether the queue is full.
* @return true if the queue is full, false otherwise.
*/
bool isFull(QUEUE* q) {
   return q->count >= q->length;
}

/**
* Accessor method for the number of elements in the queue
* @return the number of elements in the queue 
*/
int size(QUEUE * q) {
   return q->count;
}

/**
 * Enqueue the specified (PLAYER *) element into the queue.
 * Cyclical-Block implementation of a queue ADT.
 * @param q QUEUE struct referenced
 * @param player PLAYER struct to be added to the queue
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
 * Deqeueue the element at the front of the queue.
 * Cyclical-Block implementation of a queue ADT.
 * @param q QUEUE struct referenced
 * @return PLAYER * removed from queue or NULL if the queue is empty.
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

/**
* Dequeue the element at the back of the queue.
* Cyclical-Block implementation of a queue ADT.
* @param q QUEUE struct referenced.
* @return PLAYER * removed from queue or NULL if the queue is empty.
*/
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


