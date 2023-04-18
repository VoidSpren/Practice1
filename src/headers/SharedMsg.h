#pragma once

#include <semaphore.h>

#include <headers/TravelInfo.h>

//Los estados erroneos de memoria compartida 
enum{
    SHARED_FAILED = -3, SHARED_NOT_FOUND, SHARED_NOT_READY, SHARED_SUCCESS
};

//Estructura que se guarda en memoria compartida
struct SHAREDMSG
{
    sem_t serverSem;
    sem_t clientSem;
    int sharedStatus;
    TravelInfo info;
}typedef SharedMSG;
