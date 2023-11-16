typedef struct Node{
    char **e;
    struct Node* next;
}Node;

typedef struct{
    Node* top;
    int stackSize;
}Stack;



Stack* createStack();
void push(Stack* stack, char **item);
char** pop(Stack* Stack);
int stack_size(Stack* stack);
char** get(Stack*, int index);

