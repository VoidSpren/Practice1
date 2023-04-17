#pragma once

#include <semaphore.h>

#include <headers/TravelInfo.h>

enum{
    SHARED_FAILED = -2, SHARED_NOT_FOUND, SHARED_WAITING, SHARED_SUCCESS
};

struct SHAREDMSG
{
    sem_t serverSem;
    sem_t clientSem;
    int SHARED_STATUS;
    TravelInfo info;
}typedef SharedMSG;
