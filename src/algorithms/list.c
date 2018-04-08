#include <algorithms/node.h>
#include <stdlib.h>

Node* list_new(void* val)
{
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->val = val;
    new_node->next = NULL;
    return new_node;
}

Node* list_append(Node* node, Node* new_node)
{
    new_node->next = node->next;
    node->next = new_node;
    return new_node;
}
