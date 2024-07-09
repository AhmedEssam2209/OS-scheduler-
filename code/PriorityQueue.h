#include <stdbool.h>
#include "process.h" 


#define MAX_SIZE 100

typedef struct {
    Process heap[MAX_SIZE]; 
    int size;
} PriorityQueue;

void swap(Process *a, Process *b) {
    Process temp = *a;
    *a = *b;
    *b = temp;
}

void push(PriorityQueue *pq, Process process, int priorityValue) {
    if (pq->size >= MAX_SIZE) {
        printf("Priority queue full\n");
        return;
    }

    int i = pq->size;
    pq->heap[i] = process;

    // Assign priority so you can use runt feh srtn
    pq->heap[i].priority = priorityValue;

    while (i > 0 && pq->heap[(i - 1) / 2].priority >= pq->heap[i].priority) {
        swap(&pq->heap[(i - 1) / 2], &pq->heap[i]);
        i = (i - 1) / 2;
    }

    pq->size++;
}



Process pop(PriorityQueue *pq) {
    if (pq->size <= 0) {
        printf("Priority queue underflow!\n");
        exit(1);
    }

    Process minProcess = pq->heap[0];
    pq->size--;

    pq->heap[0] = pq->heap[pq->size];
    int i = 0;
    while (2 * i + 1 < pq->size) {
        int leftChild = 2 * i + 1;
        int rightChild = 2 * i + 2;
        int minIndex = i;

        if (leftChild < pq->size && pq->heap[leftChild].priority < pq->heap[minIndex].priority) {
            minIndex = leftChild;
        }
        if (rightChild < pq->size && pq->heap[rightChild].priority < pq->heap[minIndex].priority) {
            minIndex = rightChild;
        }

        if (minIndex != i) {
            swap(&pq->heap[i], &pq->heap[minIndex]);
            i = minIndex;
        } else {
            break;
        }
    }

    return minProcess;
}

Process peek(PriorityQueue *pq) {
    if (pq->size <= 0) {
        printf("Priority queue is empty\n");
        exit(1);
    }

    return pq->heap[0];
}

bool isEmpty(PriorityQueue *pq) {
    return pq->size == 0;
}

bool isFull(PriorityQueue *pq) {
    return pq->size == MAX_SIZE;
}
