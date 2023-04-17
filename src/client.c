#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

#include <headers/SharedMsg.h>
#include <headers/TravelInfo.h>

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

        int opt;
        scanf("%i", &opt);

        if(opt < 1 || opt > 5){
            printf("\nopcion invalida, opciones validas solo de 1-5\n");
            continue;
        }
    }

    return 0;
}
