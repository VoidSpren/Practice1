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

IndexNode *insertIndex(IndexNode *head, Index val){
    IndexNode *node = (IndexNode*)malloc(sizeof(IndexNode));
    node->next = NULL;
    node->index = val;

    if(head == NULL) return node;

    IndexNode *child = head->next;
    head->next = node;
    node->next = child;

    return node;
}

