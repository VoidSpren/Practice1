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
#include <headers/SharedMsg.h>


//Metodo que lee el archivo entrante y lo vuelve una hashtable
void readAndHashTable(FILE *file, IndexHTable *table){
    Index *list;

    
    fseek(file, 0, SEEK_END);

    list = (Index*)malloc(ftell(file));
    long size = ftell(file)/sizeof(Index);

    fseek(file, 0, SEEK_SET);

    fread(list, sizeof(Index), size, file);

    for(long i = 0; i < size; i++){

        insertIndexHash(list[i].ID, list[i].ogOffset, table);
    }

    free(list);
}

//Iniciar el server y espera mensajes entrantes
int main(int argc, char* argv[]){

    if(argc < 4){
        printf("usage: server {sharedMemName} {IndexTable} {IndexedInfo}");
        return -1;
    }

    FILE *tableFile, *infoFile;

    tableFile = fopen(argv[2], "rb");
    if(!tableFile){
        printf("file {%s} failed to open", argv[2]);
        return -1;
    }

    IndexHTable table = createIndexHTable(1547);
    readAndHashTable(tableFile, &table);

    fclose(tableFile);


    infoFile = fopen(argv[3], "rb");
    if(!infoFile){
        printf("file {%s} failed to open", argv[3]);
        return -1;
    }

    int memFd = shm_open(argv[1], O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if(memFd < 0){
        printf("failed to create shared mem file descriptor with name: %s\n", argv[1]);
        return -1;
    }

    SharedMSG *shared = mmap(NULL, 2*sizeof(SharedMSG), PROT_READ | PROT_WRITE, MAP_SHARED, memFd, 0);

    Index index;
    TravInfFID infoFID;

    return 0;
}
