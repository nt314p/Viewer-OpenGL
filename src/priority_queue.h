#pragma once
#include "physics.h"

// Represents a min priority queue 
typedef struct PriorityQueue
{
    Interaction* array;
    int size;
    int capacity;
} PriorityQueue;

// `array` points to a buffer with size `capacity` to serve as the backing array
void PriorityQueueCreate(PriorityQueue* pq, Interaction* array, int capacity);

// Creates a priority queue that uses the buffer pointed to by `array` as the backing
// array and keeps the `size` elements in that array
void PriorityQueueHeapify(PriorityQueue* pq, Interaction* array, int size, int capacity);

void PriorityQueuePush(PriorityQueue* pq, Interaction value);
Interaction PriorityQueuePop(PriorityQueue* pq);
Interaction PriorityQueuePeek(PriorityQueue* pq);
