// Adapted from:
// https://www.geeksforgeeks.org/queue-set-1introduction-and-array-implementation/
//
// C program for array implementation of queue
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// A queue of pointers.
// Implemented with a fixed length array.
struct Queue {
    int front, rear, size;
    unsigned capacity;
    void **array;
};

// Create a queue of given capacity.
// Initialize size of queue as 0.
// Return NULL on failure.
struct Queue* createQueue(unsigned capacity) {
    struct Queue* queue = malloc(sizeof(struct Queue));
    if (queue == NULL) {
      printf("Could not malloc. \n");
      return NULL;
    }
    queue->capacity = capacity;
    queue->front = queue->size = 0;

    // This is important, see the enqueue
    queue->rear = capacity - 1;
    queue->array = malloc(
        queue->capacity * sizeof(void *));
    return queue;
}

// Free the memory allocated for a queue.
void destroyQueue(struct Queue *queue) {
  free(queue->array);
  free(queue);
}

// Queue is full when size becomes
// equal to the capacity
int isFull(struct Queue* queue) {
    return (queue->size == queue->capacity);
}

// Queue is empty when size is 0
int isEmpty(struct Queue* queue) {
    return (queue->size == 0);
}

// Function to add an item to the queue.
// It changes rear and size.
// Retun 0 on succes,
// -1 on failure.
int enqueue(struct Queue* queue, void *item) {
    if (isFull(queue))
        return -1;
    queue->rear = (queue->rear + 1)
                  % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
    return 0;
}

// Function to remove an item from queue.
// It changes front and size.
// Return NULL on failure.
void *dequeue(struct Queue* queue) {
    if(isEmpty(queue)) {
        return NULL;
    }
    void *item = queue->array[queue->front];
    queue->front = (queue->front + 1)
                   % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

// Return queue number of contents.
int size(struct Queue* queue) {
    return queue->size;
}

