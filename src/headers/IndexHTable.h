#pragma once

#include <headers/Index.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>


struct INDEXHTABLE
{
    IndexNode **list;
    long size;
    long reserved;
    float loadFac;
}typedef IndexHTable;

long hash(int key, long size){
    long k = 268382559;
    long p = 1160;
    return ((k*key)%p)%size;
}

IndexHTable createIndexHTable(){
    IndexHTable table;
    table.list = (IndexNode**)malloc(8 * sizeof(IndexNode*));
    table.size = 0;
    table.reserved = 8;
    table.loadFac = 0;

    memset(table.list, 0, 8 * sizeof(IndexNode*));
    
    return table;
}

void closeIndexHTable(IndexHTable *table){
    free(table->list);
    table->size = 0;
    table->reserved = 0;
    table->loadFac = 0;
}

void expandRehash(IndexHTable *table){
    IndexNode **nList = (IndexNode**)malloc(table->reserved * sizeof(IndexNode*));
    
}

void insertIndexHash(int key, long value, IndexHTable *table){
    float loadFactor = ((float)table->size + 1.f)/table->reserved;

    if(loadFactor >= 0.6){

        

    }else{

        long i = hash(key, table->reserved);
        Index index;
        index.ID = key;
        index.ogOffset = value;

        if(table->list[i] == NULL){
            table->list[i] = insertIndex(NULL, index);
        }else{
            insertIndex(table->list[i], index);
        }

        table->size++;
        table->loadFac = loadFactor;
    }
}

