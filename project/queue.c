/** MANAGE QUEUE OF ACTIVE PLAYERS **/
#include "server.h"

PLAYER * examine(QUEUE * q) {
    return q->elements[q->front];
}

bool isEmpty(QUEUE* q) {
   return q->count <= 0;
}

bool isFull(QUEUE* q) {
   return q->count >= q->len;
}

int size(QUEUE * q) {
   return q->count;
}

/**
 * @return 0 to indicate success, -1 to indicate failure
 */
int enqueue(QUEUE * q, int data) {
    if(!isFull(q)) {
        int last = abs((q->front + q->count))%q->len; //cyclic property of queue
        q->count++; //increment count of elements
        q->elements[last] = data;
        return 0;
    }
    else 
        return -1;
}

/**
 * @return int id on success, -1 on failure;
 */
int dequeue(QUEUE * q) {
   if(!isEmpty(q)) {
        int id = q->elements[q->front];
        q->elements[q->front] = -1; //erase memory address
        q->front = (++q->front)%q->len;
        q->count--; //reduce number of elements in the queue
        return id;
    }
    else
        return -1;
}

int main(int argc, char * argv[]) {
    QUEUE queue;
    queue.front = 0;
    queue.count = 0;
    queue.len = NUM_PLAYERS;

    enqueue(&queue, 2);
    printf("%d\n", dequeue(&queue));
    enqueue(&queue, 4);
    enqueue(&queue, 6);
    printf("%d\n", enqueue(&queue, 9));
    printf("%d\n", examine(&queue));
    printf("%d\n", dequeue(&queue));
    printf("%d\n", examine(&queue));
    printf("%d\n", enqueue(&queue, 7));
    while(examine(&queue) != -1) {
        printf("-%d\t", dequeue(&queue));
    }
}