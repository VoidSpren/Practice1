#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <time.h>

#include <headers/TravelInfo.h>
#include <headers/TravInfFID.h>
#include <headers/Index.h>
#include <headers/IndexHTable.h>
#include <headers/SharedMsg.h>

//numero escogido de manera que el hastable de indices no necesite cambiar su tamaño reservado
#define HASHRESERVE 1547


//Metodo que lee el archivo con la tabla de indices y lo vuelve una hashtable
void readAndHashTable(FILE *file, IndexHTable *table){
    Index *list;

    //nos ubicamos en el final del archivo file
    fseek(file, 0, SEEK_END);

    //Se inicializa la lista reservando un espacio del tamaño del archivo
    list = (Index*)malloc(ftell(file));

    //se calcula la cantidad de indices dividiendo el tamaño del archivo 
    //en el tamaño de la estructura Index
    long size = ftell(file)/sizeof(Index);

    //Se vuelve al inicio del archivo
    fseek(file, 0, SEEK_SET);

    //Se lee el archivo y se guardan la información en la lista
    size_t count = fread(list, sizeof(Index), size, file);
    if(count != size){
        printf("failed reading table file\n");
    }

    for(long i = 0; i < size; i++){
        //Se recorre la lista y a cada elemento se agregar a la hashTable
        insertIndexHash(list[i].ID, list[i].ogOffset, table);
    }

    free(list);
}

int searchInFile(TravelInfo *info, long offset, FILE *input){
    TravInfFID readedInfFID;

    while(!feof(input) && offset >= 0){
        fseek(input, offset, SEEK_SET);
        size_t count = fread(&readedInfFID, sizeof(TravInfFID), 1,input);
        if(count < 1){
            printf("failed reading indexed file\n");
        }

        if(readedInfFID.info.destId == info->destId && readedInfFID.info.hourOD == info->hourOD){
            (*info) = readedInfFID.info;

            return 0;
        }

        offset = readedInfFID.nextOffset;
    }

    return -1;
}

int main(int argc, char* argv[]){

    //Se rverifica que los parametros de entrada sean 4
    if(argc != 4){
        printf("usage: server {sharedMemName} {IndexTable} {IndexedInfo}\n");
        return -1;
    }

    FILE *tableFile, *infoFile;

    //Se abre correctamente el archivo donde estará guardada la hashtable
    tableFile = fopen(argv[2], "rb");
    if(!tableFile){
        printf("file {%s} failed to open", argv[2]);
        return -1;
    }

    //Se inicializa la hashtable 1547 y llena con la información del archivo
    IndexHTable table = createIndexHTable(HASHRESERVE);
    readAndHashTable(tableFile, &table);

    fclose(tableFile);

    //Se abre el archivo donde esta indexada toda la información de los viajes
    infoFile = fopen(argv[3], "rb");
    if(!infoFile){
        printf("file {%s} failed to open", argv[3]);
        return -1;
    }

    //Se verifica que el nombre de la memoria compartida no este usado 
    shm_unlink(argv[1]);

    //Se abre memoria compartida nombrada
    int memFd = shm_open(argv[1], O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if(memFd < 0){
        printf("failed to create shared mem file descriptor with name: %s\n", argv[1]);
        return -1;
    }
    
    int erno;
    erno = ftruncate(memFd, 2*sizeof(SharedMSG));
    if(erno < 0){
        printf("failed to truncate to size\n");
        return -1;
    }   
    SharedMSG *shared = mmap(NULL, 2*sizeof(SharedMSG), PROT_READ | PROT_WRITE, MAP_SHARED, memFd, 0);
    shared->sharedStatus = SHARED_NOT_READY;

    //Inicializa el semaforo para el server
    erno = sem_init(&(shared->serverSem), 1, 0);
    if(erno < 0){
        printf("failed to initialize server ready semaphore\n");
        return -1;
    }

    //Inicializa el semaforo para el client
    erno = sem_init(&(shared->clientSem), 1, 0);
    if(erno < 0){
        printf("failed to initialize client ready semaphore\n");
        return -1;
    }

    Index index;
    TravelInfo info;

    clock_t start, diff;

    //Se pone en status SHARED_NOT_FOUND
    shared->sharedStatus = SHARED_NOT_FOUND;
    while(1){
        printf("server ready\n");

        //Se espera indicaciones del client
        sem_wait(&(shared->clientSem));
        
        printf("working... \n");

        start = clock();

        //Se declara el status en SHARED_NOT_READY 
        shared->sharedStatus = SHARED_NOT_READY;

        //Se guarda la informacion de la memoria compartida en info
        info = shared->info;

        //obtiene el indice para el id de origen si existe
        int res = getIfExists(info.srcId, &index, table);
    
        //comprueba que el indice anterior existe
        if(res >= 0){

            //Si existe se busca en el archivo y se guarda la información de viaje en info
            res = searchInFile(&info, index.ogOffset, infoFile);
            if(res >= 0){
                //guarda en la memoria compartida el resultado info
                shared->info = info;
                //actualiza el status de la memoria
                shared->sharedStatus = SHARED_SUCCESS;
            }else{
                //Si no se encuentra en el archivo el status es SHARED_NOT_FOUND
                shared->sharedStatus = SHARED_NOT_FOUND;
            }
        }else{
            //Si no se existe el indice, el status es SHARED_NOT_FOUND
            shared->sharedStatus = SHARED_NOT_FOUND;
        }
        
        //Envía la señal de que se subio la información a la memoria compartida
        sem_post(&(shared->serverSem));

        //calcula e imprime el tiempo utilizado para la consulta
        diff = clock() - start;
        int msec = diff * 1000000 / CLOCKS_PER_SEC;
        printf("time in micros: %i\n", msec);
    }

    return 0;
}
