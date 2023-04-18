#pragma once

#include <headers/TravelInfo.h>

//Estructura que guarda la información de viaje y un indice a un estuctura siguiente en un archivo
struct TRAVINFFID
{
    long nextOffset;
    TravelInfo info;
} __attribute__((packed)) typedef TravInfFID;


