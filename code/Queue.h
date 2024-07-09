#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    Process *arr;
    int size;
} Queue;

Queue* InstQueue() {
    Queue *array = (Queue*)malloc(sizeof(Queue));
    array->size = 0;
    array->arr = NULL;
    return array;
}

void pushq(Queue *array, Process data) {
    array->size++;
    array->arr = (Process*)realloc(array->arr, array->size * sizeof(Process));
    array->arr[array->size - 1] = data;
}

void removeAtIndex(Queue *array, int index) {
    if (index < 0 || index >= array->size) {
        printf("Invalid index\n");
        return;
    }
    for (int i = index; i < array->size - 1; ++i) {
        array->arr[i] = array->arr[i + 1];
    }
    array->size--;
    array->arr = (Process*)realloc(array->arr, array->size * sizeof(Process));
}

Process getElementAtIndex(Queue *array, int index) {
    if (index < 0 || index >= array->size) {
        printf("Invalid index\n");
    }
    return array->arr[index];
}