#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

#include <headers/SharedMsg.h>
#include <headers/TravelInfo.h>

int consultMeanTime(TravelInfo *info, SharedMSG *shared){
    if(shared->sharedStatus != SHARED_NOT_READY){
        shared->info = (*info);
        sem_post(&(shared->clientSem));
        sem_wait(&(shared->serverSem));

        (*info) = shared->info;
    }
    
    return shared->sharedStatus;
}

int main(int argc, char* argv[]){

    if(argc != 2){
        printf("usage: client {sharedMemName}\n");
        return -1;
    }

    int memFd = shm_open(argv[1], O_RDWR, S_IRUSR | S_IWUSR);
    if(memFd < 0){
        printf("failed to open shared mem file descriptor with name: %s\n", argv[1]);
        return -1;
    }  

    SharedMSG *shared = mmap(NULL, 2*sizeof(SharedMSG), PROT_READ | PROT_WRITE, MAP_SHARED, memFd, 0);

    int opt;
    TravelInfo info;
    while(1){
        printf(
            "\nBienvenido\n\n"
            "1. Ingresar origen\n"
            "2. Ingresar destino\n"
            "3. Ingresar hora\n"
            "4. Buscar tiempo de viaje medio\n"
            "5. Salir\n"
            "\nopcion: "
        );
        scanf("%i", &opt);

        int status;
        switch(opt){
            case 1:
                printf("\ningrese el id de origen (numero entero 1 - 1160): ");
                scanf("%i", &(info.srcId));

                if(info.srcId < 1 || info.srcId > 1160){
                    printf("valor invalido (valores validos de 1 a 1160)\n");
                    info.srcId = 0;
                }
                break;
            case 2:
                printf("\ningrese el id de destino (numero entero 1 - 1160): ");
                scanf("%i", &(info.destId));

                if(info.destId < 1 || info.destId > 1160){
                    printf("valor invalido (valores validos de 1 a 1160)\n");
                    info.destId = 0;
                }
                break;
            case 3:
                printf("\ningrese el hora del dia (numero entero 0 - 23): ");
                scanf("%i", &(info.hourOD));

                if(info.hourOD < 0 || info.hourOD > 23){
                    printf("valor invalido (valores validos de 0 a 23)\n");
                    info.hourOD = 0;
                }
                break;
            case 4:
                status = consultMeanTime(&info, shared);
                if(status == SHARED_SUCCESS){
                    printf(
                        "\nconsulta exitosa\n"
                        "id de origen: %i\n"
                        "id de destino: %i\n"
                        "hora del dia: %i\n"
                        "media de tiempo del viaje: %f\n",
                        info.srcId, info.destId, info.hourOD, info.meanTime
                        );
                }else{
                    printf("status: %i\n", status);
                    printTravI(info);
                }

                break;
            case 5:
                printf("cerrando...\n");

                return 0;
                
            default:
                printf("\nopcion invalida, opciones validas solo de 1-5\n");
                break;
        }
    }

    return 0;
}
