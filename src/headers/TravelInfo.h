#pragma once

#include <stdio.h>

struct TRAVELINFO
{
    int srcId;
    int destId;
    int hourOD;
    float meanTime;
    float stdDev;
    float geomMeanTime;
    float geomstdDev;
}typedef TravelInfo;

int strFileToTravInf(TravelInfo *info, FILE *file){
    return fscanf(
        file, "%i,%i,%i,%f,%f,%f,%f",
        info->srcId, info->destId, info->hourOD,
        info->meanTime, info->stdDev, info->geomMeanTime, info->geomstdDev
    );
}
