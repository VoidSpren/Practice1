#pragma once

#include <semaphore.h>

#include <headers/TravelInfo.h>

enum{
    SHARED_FAILED = -3, SHARED_NOT_FOUND, SHARED_NOT_READY, SHARED_SUCCESS
};

struct SHAREDMSG
{
    sem_t serverSem;
    sem_t clientSem;
    int sharedStatus;
    TravelInfo info;
}typedef SharedMSG;
