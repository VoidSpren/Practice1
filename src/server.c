#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

#include <headers/TravelInfo.h>
#include <headers/TravInfFID.h>
#include <headers/Index.h>
#include <headers/IndexHTable.h>

int main(int argc, char* argv[]){
    FILE *file = fopen(argv[1], "rb");
    if(!file){
        printf("file {%s} failed to open", argv[1]);
        return -1;
    }

    IndexHTable table = createIndexHTable(1547);

    Index *list;

    fseek(file, 0, SEEK_END);

    list = (Index*)malloc(ftell(file));
    long size = ftell(file)/sizeof(Index);

    fseek(file, 0, SEEK_SET);

    fread(list, sizeof(Index), size, file);

    for(long i = 0; i < size; i++){

        insertIndexHash(list[i].ID, list[i].ogOffset, &table);
    }

    printHTable(table);
}
