#pragma once

#include <headers/TravelInfo.h>

//Estructura que guarda la información de viaje y un indice para escribirlo la tabla indexada
struct TRAVINFFID
{
    long nextOffset;
    TravelInfo info;
} __attribute__((packed)) typedef TravInfFID;


