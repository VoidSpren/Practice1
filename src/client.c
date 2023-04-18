#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

#include <headers/SharedMsg.h>
#include <headers/TravelInfo.h>

void flushStdIn(){
    char c;
    while ((c = getchar()) != '\n' && c != EOF);
}

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

    //Se revisa que se ingrese el nombre de la memoria compartida y nada mas
    if(argc != 2){
        printf("usage: client {sharedMemName}\n");
        return -1;
    }

    //Accede a la memoria compartida con el server
    int memFd = shm_open(argv[1], O_RDWR, S_IRUSR | S_IWUSR);
    if(memFd < 0){
        printf("failed to open shared mem file descriptor with name: %s\n", argv[1]);
        return -1;
    }  

    SharedMSG *shared = mmap(NULL, 2*sizeof(SharedMSG), PROT_READ | PROT_WRITE, MAP_SHARED, memFd, 0);

    //Crea un loop infinito para estar siempre mostrando el menu de inicio
    int opt;
    TravelInfo info;
    int erno;
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

        //Se lee el entero ingresado y se guarda en opt
        erno = scanf("%i", &opt);

        //Erno revisa que no hallan errores de lectura
        if(erno != 1) printf("error leyendo dato provisto\n");


        int status;
        char c;
        //hace un switch para cada caso seleccionado
        switch(opt){
            case 1:
                //pide, lee y guarda el id de origen en el info.srcId
                printf("\ningrese el id de origen (numero entero 1 - 1160): ");
                erno = scanf("%i", &(info.srcId));
                if(erno != 1) printf("error leyendo dato provisto\n");

                //Se comprueba que el dato de entrada sea valido
                if(info.srcId < 1 || info.srcId > 1160){

                    //De no ser valido imprime el error y guarda un 0 en info.srcId
                    printf("valor invalido (valores validos de 1 a 1160)\n");
                    info.srcId = 0;
                    flushStdIn();
                }
                break;
            case 2:
                //pide, lee y guarda el id de destino en el info.destID
                printf("\ningrese el id de destino (numero entero 1 - 1160): ");
                erno = scanf("%i", &(info.destId));
                if(erno != 1) printf("error leyendo dato provisto\n");

                //Se comprueba que el dato de entrada sea valido
                if(info.destId < 1 || info.destId > 1160){

                    //De no ser valido imprime el error y guarda un 0 en info.destId
                    printf("valor invalido (valores validos de 1 a 1160)\n");
                    info.destId = 0;
                    flushStdIn();
                }
                break;
            case 3:
                //pide, lee y guarda la hora en el info.hourOD
                printf("\ningrese el hora del dia (numero entero 0 - 23): ");
                erno = scanf("%i", &(info.hourOD));
                if(erno != 1) printf("error leyendo dato provisto\n");

                //Se comprueba que el dato de entrada sea valido
                if(info.hourOD < 0 || info.hourOD > 23){

                    //De no ser valido imprime el error y guarda un 0 en info.hourOD
                    printf("valor invalido (valores validos de 0 a 23)\n");
                    info.hourOD = 0;
                    flushStdIn();
                }
                break;
            case 4:
                //Se consulta el tiempo promedio y se guarda en status si fue exitoso
                status = consultMeanTime(&info, shared);

                //Comprueba que sea exitosa la consulta
                if(status == SHARED_SUCCESS){
                    //De ser exitosa imprime la información de viaje 
                    printf(
                        "\nconsulta exitosa\n"
                        "id de origen: %i\n"
                        "id de destino: %i\n"
                        "hora del dia: %i\n"
                        "media de tiempo del viaje: %f\n",
                        info.srcId, info.destId, info.hourOD, info.meanTime
                        );

                }else{
                    //Si status no es exitosos se imprime el error de no encontrado
                    printf("\ncoincidencia no encontrada\n");
                }

                //Se da la opción de continuar con la consulta de viajes
                printf("\nenter para continuar...\n");
                getchar();
                getchar();

                break;
            case 5:
                //Envia mensaje de confirmación de cerrado y termina de ejcutarse el programa
                printf("cerrando...\n");

                return 0;
                
            default:
                //Si se seleccina una opción distinta a las 5 envia error
                printf("\nopcion invalida, opciones validas solo de 1-5\n");
                flushStdIn();
                break;
        }
    }

    return 0;
}
