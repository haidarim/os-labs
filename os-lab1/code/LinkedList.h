typedef struct Node{
    int e;
    struct Node* next;
}Node;


typedef struct{
    Node* head;
    int listSize;
}LinkedList;

LinkedList* createLinkedList();
int l_get(LinkedList* l, int i);
int set(LinkedList* l, int i , int e);
void add(LinkedList* l, int i, int e);
int remove_element(LinkedList* l, int i);
int l_size(LinkedList* l);


