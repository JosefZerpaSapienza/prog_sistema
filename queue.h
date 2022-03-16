// Adapted from:
// https://www.geeksforgeeks.org/queue-set-1introduction-and-array-implementation/
//
// C program for array implementation of queue
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "connection.h"

// A structure to represent a queue
struct Queue {
    int front, rear, size;
    unsigned capacity;
    struct Connection** array;
};

// Create a queue of given capacity.
// Initialize size of queue as 0
struct Queue* createQueue(unsigned capacity)
{
    struct Queue* queue = malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;

    // This is important, see the enqueue
    queue->rear = capacity - 1;
    queue->array = malloc(
        queue->capacity * sizeof(struct Connection *));
    return queue;
}

// Free the memory allocated for a queue.
void destroyQueue(struct Queue *queue) {
  free(queue->array);
  free(queue);
}

// Queue is full when size becomes
// equal to the capacity
int isFull(struct Queue* queue)
{
    return (queue->size == queue->capacity);
}

// Queue is empty when size is 0
int isEmpty(struct Queue* queue)
{
    return (queue->size == 0);
}

// Function to add an item to the queue.
// It changes rear and size
int enqueue(struct Queue* queue, struct Connection* item)
{
    if (isFull(queue))
        return -1;
    queue->rear = (queue->rear + 1)
                  % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
    return 0;
}

// Function to remove an item from queue.
// It changes front and size
struct Connection *dequeue(struct Queue* queue)
{
    struct Connection *item = queue->array[queue->front];
    queue->front = (queue->front + 1)
                   % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

// Return queue number of contents.
int size(struct Queue* queue) 
{
    return queue->size;
}

