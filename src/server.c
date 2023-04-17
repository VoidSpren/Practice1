#include <stdio.h>
#include <unistd.h>
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

int searchInFile(TravelInfo *info, long offset, FILE *input){
    TravInfFID readedInfFID;

    while(!feof(input) && offset >= 0){
        fseek(input, offset, SEEK_SET);
        fread(&readedInfFID, sizeof(TravInfFID), 1,input);

        if(readedInfFID.info.destId == info->destId && readedInfFID.info.hourOD == info->hourOD){
            (*info) = readedInfFID.info;
            return 0;
        }

        offset = readedInfFID.nextOffset;
    }

    return -1;
}

int main(int argc, char* argv[]){

    //Se rectifica que los parametros de entrada sean menos de 3
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

    //Se verifica que el nombre de la memoria compartida no este usado
    shm_unlink(argv[1]);

    //Se esta abriendo una memoria nombrada compartida
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

    erno = sem_init(&(shared->serverSem), 1, 0);
    if(erno < 0){
        printf("failed to initialize server ready semaphore\n");
        return -1;
    }
    erno = sem_init(&(shared->clientSem), 1, 0);
    if(erno < 0){
        printf("failed to initialize client ready semaphore\n");
        return -1;
    }

    Index index;
    TravelInfo info;

    shared->sharedStatus = SHARED_NOT_FOUND;
    while(1){
        printf("server ready\n");

        sem_post(&(shared->serverSem));
        sem_wait(&(shared->clientSem));

        printf("working... \n");

        shared->sharedStatus = SHARED_NOT_READY;

        info = shared->info;

        printTravI(info);

        int res = getIfExists(info.srcId, &index, &table);

        printf("found?: %i", res);

        if(res >= 0){
            res = searchInFile(&info, index.ogOffset, infoFile);
            if(res >= 0){
                shared->info = info;
                shared->sharedStatus = SHARED_SUCCESS;
            }else{
                shared->sharedStatus = SHARED_NOT_FOUND;
            }
        }else{
            shared->sharedStatus = SHARED_NOT_FOUND;
        }
    }

    return 0;
}
