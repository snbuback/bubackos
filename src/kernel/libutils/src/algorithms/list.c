#include <stdlib.h>
#include <algorithms/node.h>

Node* list_new(void* val)
{
    Node* new_node = (Node*)LIBK_ALLOC(sizeof(Node));
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
