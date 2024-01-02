#pragma once

// Represents a min priority queue 
typedef struct PriorityQueue
{
    int* array;
    int size;
    int capacity;
} PriorityQueue;

// `array` points to a buffer with size `capacity` to serve as the backing array
void PriorityQueueCreate(PriorityQueue* pq, int* array, int capacity);

// Creates a priority queue that uses the buffer pointed to by `array` as the backing
// array and keeps the `size` elements in that array
void PriorityQueueHeapify(PriorityQueue* pq, int* array, int size, int capacity);

void PriorityQueuePush(PriorityQueue* pq, int value);
int PriorityQueuePop(PriorityQueue* pq);
