/*
 * stack.c
 * Implementasi Stack dengan linked list.
 * push menaruh node baru di atas (top), pop mengambil node paling atas.
 */
#include <stdlib.h>
#include "stack.h"

void initStack(Stack *stack)
{
    stack->top = NULL;
    stack->size = 0;
}

int isStackEmpty(const Stack *stack)
{
    return stack->top == NULL;
}

int push(Stack *stack, const Activity *activity)
{
    StackNode *node = (StackNode *) malloc(sizeof(StackNode));
    if (node == NULL) {
        return 0;
    }
    node->data = *activity;
    node->next = stack->top;   /* node baru menunjuk ke top lama */
    stack->top = node;         /* node baru menjadi top          */
    stack->size++;
    return 1;
}

int pop(Stack *stack, Activity *activityOut)
{
    StackNode *removed;

    if (isStackEmpty(stack)) {
        return 0;
    }
    removed = stack->top;
    if (activityOut != NULL) {
        *activityOut = removed->data;
    }
    stack->top = removed->next;
    free(removed);
    stack->size--;
    return 1;
}

const Activity *peekStack(const Stack *stack)
{
    if (isStackEmpty(stack)) {
        return NULL;
    }
    return &stack->top->data;
}

void freeStack(Stack *stack)
{
    while (pop(stack, NULL)) {
        /* pop sudah membebaskan setiap node */
    }
}
