#include "priority_queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static inline void Swap(int* a, int* b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

static inline int Parent(int i)
{
    return (i - 1) / 2;
}

static inline int Right(int i)
{
    return 2 * i + 2;
}

static inline int Left(int i)
{
    return 2 * i + 1;
}

static void Heapify(PriorityQueue* pq, int index)
{
    int prevIndex = 0;

    while (1)
    {
        prevIndex = index;
        int leftIndex = Left(prevIndex);
        if (leftIndex < pq->size && pq->array[leftIndex] < pq->array[index])
        {
            index = leftIndex;
        }

        int rightIndex = Right(prevIndex);
        if (rightIndex < pq->size && pq->array[rightIndex] < pq->array[index])
        {
            index = rightIndex;
        }

        if (index == prevIndex) break; // no change, heap property restored

        Swap(pq->array + prevIndex, pq->array + index);
    }
}

void PriorityQueueCreate(PriorityQueue* pq, int* array, int capacity)
{
    pq->array = array;
    pq->size = 0;
    pq->capacity = capacity;
}

void PriorityQueueHeapify(PriorityQueue* pq, int* array, int size, int capacity)
{
    pq->array = array;
    pq->size = size;
    pq->capacity = capacity;

    int lastNonLeafNodeIndex = (size / 2) - 1;
    for (int index = lastNonLeafNodeIndex; index >= 0; index--)
    {
        Heapify(pq, index);
    }
}

void PriorityQueuePush(PriorityQueue* pq, int value)
{
    if (pq->size >= pq->capacity) exit(-1); // capacity reached, cannot push

    int index = pq->size;
    pq->array[index] = value;
    pq->size++;

    while (pq->array[Parent(index)] > value)
    {
        Swap(pq->array + Parent(index), pq->array + index);
        index = Parent(index);
        if (index < 0) break;
    }
}

int PriorityQueuePop(PriorityQueue* pq)
{
    int size = pq->size;
    if (size < 1) exit(-1); // no elements to pop

    int ret = pq->array[0];
    pq->array[0] = pq->array[size - 1];
    pq->size--;

    Heapify(pq, 0);

    return ret;
}