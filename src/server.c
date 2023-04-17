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

    //nos ubicamos en el final del archivo file
    fseek(file, 0, SEEK_END);

    //Se inicializa la lista reservando un espacio del tamaño del archivo
    list = (Index*)malloc(ftell(file));

    //Y se calcula la cantidad de indices dividiendo en tamaño del archivo 
    //en el tamaño de la estructura
    long size = ftell(file)/sizeof(Index);

    //Se vuelve al inicio del archivo
    fseek(file, 0, SEEK_SET);

    //Se recorrer el archivo y se guardan la información en la lista
    fread(list, sizeof(Index), size, file);

    for(long i = 0; i < size; i++){
        //Se recorre la lista y a cada elemento se agregar a la hashTable
        insertIndexHash(list[i].ID, list[i].ogOffset, table);
    }

    free(list);
}

int main(int argc, char* argv[]){

    //Se rectifica que los parametros de entrada sean menos de 3
    if(argc < 4){
        printf("usage: server {sharedMemName} {IndexTable} {IndexedInfo}");
        return -1;
    }

    FILE *tableFile, *infoFile;

    //Se abre correctamente el archivo donde estará guardada la hashtable
    tableFile = fopen(argv[2], "rb");
    if(!tableFile){
        printf("file {%s} failed to open", argv[2]);
        return -1;
    }

    //Se inicializa la hashtable y llena con la información del archivo
    IndexHTable table = createIndexHTable(1547);
    readAndHashTable(tableFile, &table);

    fclose(tableFile);

    //Se abre el archivo donde esta indexada toda la información de los viajes
    infoFile = fopen(argv[3], "rb");
    if(!infoFile){
        printf("file {%s} failed to open", argv[3]);
        return -1;
    }

    //Se esta abriendo una memoria nombrada compartida
    int memFd = shm_open(argv[1], O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if(memFd < 0){
        printf("failed to create shared mem file descriptor with name: %s\n", argv[1]);
        return -1;
    }

    //Se inicializa para que el server espere una nueva indicación de consulta
    SharedMSG *shared = mmap(NULL, 2*sizeof(SharedMSG), PROT_READ | PROT_WRITE, MAP_SHARED, memFd, 0);

    Index index;
    TravInfFID infoFID;

    return 0;
}
