#pragma once

#include <headers/TravelInfo.h>

struct TRAVINFFID
{
    long nextOffset;
    TravelInfo info;
} __attribute__((packed)) typedef TravInfFID;


