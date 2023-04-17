#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <headers/Index.h>
#include <headers/TravelInfo.h>
#include <headers/TravInfFID.h>

IndexNode* IndexFile(FILE* input, FILE* output){

    /*---logging---*/
    long inEnd;
    fseek(input, 0, SEEK_END);
    inEnd = ftell(input);
    fseek(input, 0, SEEK_SET);
    /*-------------*/

    printf("----------Indexing and generating table------------\n");
    
    
    long prevIdOffset[1160];
    memset(prevIdOffset, -1, sizeof(prevIdOffset));

    long currOutputPos;
    int erno;
    
    IndexNode *head, *tail;
    TravelInfo travInf;
    TravInfFID infoFID;
    Index index;


    currOutputPos = ftell(output);
    fread(&travInf, sizeof(TravelInfo), 1, input);
    //erno = fileStrToTravInf(&travInf, input);

    infoFID.info = travInf;
    infoFID.nextOffset = -1;

    index.ID = travInf.srcId;
    index.ogOffset = currOutputPos;

    head = insertIndex(NULL, index);
    tail = head;

    fwrite(&infoFID, sizeof(TravInfFID), 1, output);

    prevIdOffset[travInf.srcId - 1] = currOutputPos;

    /*---logging---*/
    int cycles = 0;
    /*-------------*/
    while(!feof(input)){

        /*---logging---*/

        if(cycles > 100000){
            cycles = 0;
            printf("%%%f\n", ((float)ftell(input))/inEnd *100.0);
        }
        /*-------------*/

        currOutputPos = ftell(output);

        size_t count = fread(&travInf, sizeof(TravelInfo), 1, input);
        if(feof(input)) break;

        infoFID.info = travInf;
        infoFID.nextOffset = -1;

        if(prevIdOffset[travInf.srcId - 1] < 0){
            index.ID = travInf.srcId;
            index.ogOffset = currOutputPos;

            tail = insertIndex(tail, index);

            fwrite(&infoFID, sizeof(TravInfFID), 1, output);
        }else{
            fseek(output, prevIdOffset[travInf.srcId - 1], SEEK_SET);
            fwrite(&currOutputPos, sizeof(long), 1, output);

            fseek(output, currOutputPos, SEEK_SET);
            fwrite(&infoFID, sizeof(TravInfFID), 1, output);
        }

        prevIdOffset[travInf.srcId - 1] = currOutputPos;

        /*---logging---*/
        cycles++;
        /*-------------*/
    }

    printf("----------Indexinf and table finished------------\n");

    return head;
}

void saveIndexTable(FILE *file, IndexNode *node){

    while (node != NULL)
    {
        fwrite(&(node->index),sizeof(Index),1,file);
        node = node->next;
    }
}

void csvToBin(FILE *input, FILE *output){
    /*---logging---*/
    long inEnd;
    fseek(input, 0, SEEK_END);
    inEnd = ftell(input);
    fseek(input, 0, SEEK_SET);
    /*-------------*/

    printf("----------conversion csv to bin------------\n");

    char trash[200];
    fscanf(input, "%199s", trash);

    TravelInfo info;

    /*---logging---*/
    int cycles = 0;
    /*-------------*/
    while(!feof(input)){
        /*---logging---*/
        if(cycles > 100000){
            cycles = 0;
            printf("%%%f\n", ((float)ftell(input))/inEnd *100.0);
            //printTravI(info);
        }
        /*-------------*/

        int erno = strFileToTravInf(&info, input);
        if(feof(input)) break;


        fwrite(&info, sizeof(TravelInfo), 1, output);

        /*---logging---*/
        cycles++;
        /*-------------*/
    }

    printf("----------conversion finished------------\n");
}

int main(int argc, char *argv[]){
    if(argc != 5){
        printf("usage: indexTI {inputFile} {binioFile} {outputFile} {outputTableFile}\n");
        return -1;
    }

    FILE *input, *binIO, *output, *table;
    long outStart;

    input = fopen(argv[1], "r");
    if(!input){
        printf("file {%s} failed to open", argv[1]);
        return -1;
    }

    binIO = fopen(argv[2], "wb");
    if(!binIO){
        printf("file {%s} failed to open", argv[2]);
        return -1;
    }

    csvToBin(input, binIO);
    fclose(input); fclose(binIO);
    binIO = NULL;

    binIO = fopen(argv[2], "rb");
    if(!binIO){
        printf("file {%s} failed to open", argv[2]);
        return -1;
    }
    

    output = fopen(argv[3], "wb");
    if(!output){
        printf("file {%s} failed to open", argv[3]);
        return -1;
    }

    table = fopen(argv[4], "wb");
    if(!table){
        printf("file {%s} failed to open", argv[4]);
        return -1;
    }

    saveIndexTable(table, IndexFile(binIO, output));   

    fclose(binIO);
    remove(argv[2]);
    
    return 0;
}
