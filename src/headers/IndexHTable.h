//Implementación de la hashtable

#pragma once

#include <headers/Index.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//Estructura de la hashtable que es una lista dinamica
struct INDEXHTABLE
{
    IndexNode **list;
    long size;
    long reserved;
    float loadFac;
}typedef IndexHTable;

//Función hash que requiere el tamaño total y valor del nodo a agregar
long hash(int key, long size){
    long k = 268382559;
    long p = 1163;
    return ((k*key)%p)%size;
}

//Crea la hashTable y la inicializa 
IndexHTable createIndexHTable(long reserved){
    IndexHTable table;
    table.reserved = (reserved)? reserved: 8;
    table.list = (IndexNode**)malloc(table.reserved * sizeof(IndexNode*));
    table.size = 0;
    table.loadFac = 0;

    memset(table.list, 0, 8 * sizeof(IndexNode*));
    
    return table;
}

//Elimina la hashtable
void closeIndexHTable(IndexHTable *table){
    for(long i = 0; i < table->reserved; i++){
        deleteAll(table->list[i]);
    }

    free(table->list);
    table->size = 0;
    table->reserved = 0;
    table->loadFac = 0;
}

//Inserta un valor a la hashtable que no requiere redimensionarla
void insertIndexHashNoResize(IndexHTable *table, Index index){
    long i = hash(index.ID, table->reserved);

    if(table->list[i] == NULL){
        table->list[i] = insertIndex(NULL, index);
    }else{
        insertIndex(table->list[i], index);
    }

    table->size++;
}

//Expande el tamaño de la hashtable y vuelve a ingresar cada dato
void expandRehash(IndexHTable *table){
    long nSize = table->reserved * 2;
    long prevSize = table->reserved;
    
    IndexNode **auxList = table->list;
    table->list = (IndexNode**)malloc(nSize * sizeof(IndexNode*));

    table->size = 0;
    table->reserved = nSize;
    table->loadFac = 0;
    
    for(long i = 0; i < prevSize; i++){
        IndexNode *head = auxList[i];
        IndexNode *node = head;

        while(node != NULL){
            insertIndexHashNoResize(table, node->index);
            node = node->next;
        }

        deleteAll(head);
    }
    free(auxList);
}

//Inserta cualquier nuevo valor a la hashtable determinanado si requiere o no redimensionarla
void insertIndexHash(int key, long value, IndexHTable *table){
    float loadFactor = ((float)table->size + 1.f)/table->reserved;

    if(loadFactor >= 0.75){
        Index index;
        index.ID = key;
        index.ogOffset = value;

        expandRehash(table);
        insertIndexHashNoResize(table, index);
    }else{
        Index index;
        index.ID = key;
        index.ogOffset = value;

        insertIndexHashNoResize(table, index);
        table->loadFac = loadFactor;
    }
}

//Función que determina si existe un valor especifico
int getIfExists(int key, Index *index, IndexHTable table){
    long i = hash(key, table.reserved);

    IndexNode *node = table.list[i];

    while(node != NULL){
        if(node->index.ID == key){
            if(index != NULL) (*index) = node->index;
            return 0;
        }

        node = node->next;
    }

    return -1;
}

//Recorre e imprime la hashtable 
void printHTable(IndexHTable table){
    printf("{\n");
    for(long i = 0; i < table.reserved; i++){
        IndexNode *node = table.list[i];

        if(node != NULL){
            printf("| {");

            while(node != NULL){
                printf(" { %i, %li }", node->index.ID, node->index.ogOffset);
                node = node->next;
            }
            printf(" } |");
        }
    }
    printf("}\n");
}

