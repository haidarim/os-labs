#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "Stack.h"


Stack* createStack(){
    Stack* stack = (Stack*) malloc(sizeof(Stack));
    stack->top = NULL;
    stack->stackSize = 0;
    return stack;
}

void push(Stack* stack,char **item){
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->e = item;
    newNode->next = stack->top;
    stack->top = newNode;
    stack->stackSize++;
}

char** pop(Stack* stack){
    if(!(stack->stackSize>0)){
        printf("pop from empty stack? why? tell me why you did that?");
    }
    Node* removed = stack->top;
    stack->top = removed->next;
    char **removedItem = removed->e;
    free(removed);
    stack->stackSize--;
    return removedItem;
}

bool isEmpty(Stack* stack){
    return stack->stackSize ==0;
}

int stack_size(Stack* stack){
    return stack->stackSize;
}

char** get(Stack* stack, int index){
    if(!(index<stack->stackSize && index>=0)){
        printf("list index out of range");
        return -1;
    }
    Node* current = stack->top;
    for(int j = 0; j<index ; j++){
        current = current->next;
    }
    return current->e;
}





