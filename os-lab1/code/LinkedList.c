#include <stdlib.h>
#include <stdio.h>
#include "LinkedList.h"

LinkedList* createLinkedList(){
    LinkedList* list = (LinkedList*)malloc(sizeof(LinkedList));
    list->head = NULL;
    list->listSize = 0;
    return list;
}

int l_get(LinkedList* list, int index){
    if(!(index<list->listSize && index>=0)){
        printf("list index out of range");
        return -1;
    }
    Node* current = list->head;
    for(int j = 0; j<index ; j++){
        current = current->next;
    }
    return current->e;
}

int set(LinkedList* list , int index, int item){
    if(index> list->listSize && index>=0){
        printf("list index out of range");
        return -1;
    }
    Node* current  = list->head;
    for(int j = 0; j<index; j++){
        current = current->next;
    }
    int old = current->e;
    current->e = item;
    return old;
}

void add(LinkedList* list, int index, int item){
    if(index> list->listSize || index<0){
        printf("list index out of range");
        return;
    }
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->e = item;
    if(index == 0){
        newNode->next = list->head;
        list->head = newNode;
    }else{
        Node* prev = list->head;
        for(int j = 0; j<index-1; j++){
            prev = prev->next;
        }
        newNode->next = prev->next;
        prev->next = newNode;
    }
    list->listSize += 1;
}

int remove_element(LinkedList* list, int index){
    if(index> list->listSize && index>=0){
        printf("list index out of range");
        return -1;
    }
    Node* removed;
    if(index == 0){
        removed = list->head;
        list->head = removed->next; 
    }else{
        Node* prev = list->head;
        for (int j = 0; j < index-1; j++){
            prev = prev->next;
        }
        removed = prev->next;
        prev->next = removed->next;        
    }
    int removed_item = removed->e; 
    free(removed);
    list->listSize -=1;
    return removed_item;
}
int l_size(LinkedList* list){
    return list->listSize;
}


