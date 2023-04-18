//Implementación de indices y una lista enlazada para indices

#pragma once

//Estructura index que se guardará en la lista enlazada
struct INDEX
{
    long ogOffset;
    int ID;
} typedef Index;

//Nodos de los cuales se compone la lista enlazada
struct INDEXNODE
{
    struct INDEXNODE *next;
    Index index;
}typedef IndexNode;

//agrega un nodo a la lista
IndexNode *insertIndex(IndexNode *node, Index val){
    IndexNode *newNode = (IndexNode*)malloc(sizeof(IndexNode));
    newNode->next = NULL;
    newNode->index = val;

    if(node == NULL) return newNode;

    IndexNode *child = node->next;
    node->next = newNode;
    newNode->next = child;

    return newNode;
}

//permite eliminar un nodo especifico teniendo al padre
IndexNode *deleteNode(IndexNode *parent, IndexNode *node){
    if(node == NULL) return parent;

    IndexNode *child = node->next;
    
    free(node);
    if(parent == NULL) return child;

    parent->next = child;
    return parent;
}

//Borra toda la estrucura de la lista enlazada
int deleteAll(IndexNode *head){
    int i = 0;
    
    for(;head != NULL; i++){
        head = deleteNode(NULL, head);
    }

    return i;
}
