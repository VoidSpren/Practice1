#pragma once

struct INDEX
{
    long ogOffset;
    int ID;
} typedef Index;

struct INDEXNODE
{
    struct INDEXNODE *next;
    Index index;
}typedef IndexNode;

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

IndexNode *deleteNode(IndexNode *parent, IndexNode *node){
    if(node == NULL) return parent;

    IndexNode *child = node->next;
    
    free(node);
    if(parent == NULL) return child;

    parent->next = child;
    return parent;
}

int deleteAll(IndexNode *head){
    int i = 0;
    
    for(;head != NULL; i++){
        head = deleteNode(NULL, head);
    }

    return i;
}
