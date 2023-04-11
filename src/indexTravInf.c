#include <stdio.h>
#include <stdlib.h>
#include <headers/Index.h>
#include <headers/TravelInfo.h>
#include <headers/TravInfFileStruct.h>

IndexNode* IndexFile(FILE* input, FILE* output){

    /*---logging---*/
    long inEnd;
    fseek(input, 0, SEEK_END);
    inEnd = ftell(input);
    fseek(input, 0, SEEK_SET);
    /*-------------*/
    
    
    long prevIdOffset[1160];
    memset(prevIdOffset, -1, sizeof(prevIdOffset));

    long currOutputPos;
    int erno;
    
    IndexNode *head, *tail;
    TravelInfo travInf;
    TravInfFileStruct travInfFS;
    Index index;


    currOutputPos = ftell(output);
    fread(&travInf, sizeof(TravelInfo), 1, input);
    //erno = fileStrToTravInf(&travInf, input);

    travInfFS.info = travInf;
    travInfFS.nextOffset = -1;

    index.ID = travInf.sourceId;
    index.ogOffset = currOutputPos;

    head = insertIndex(head, index);
    tail = head;

    fwrite(&travInfFS, sizeof(TravInfFileStruct), 1, output);

    prevIdOffset[travInf.sourceId] = currOutputPos;

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

        int count = fread(&travInf, sizeof(TravelInfo), 1, input);
        if(count == 0) break;
        //erno = fileStrToTravInf(&travInf, input);

        travInfFS.info = travInf;
        travInfFS.nextOffset = -1;

        if(prevIdOffset[travInf.sourceId - 1] < 0){
            index.ID = travInf.sourceId;
            index.ogOffset = currOutputPos;

            tail = insertIndex(tail, index);

            fwrite(&travInfFS, sizeof(TravInfFileStruct), 1, output);
        }else{
            fseek(output, prevIdOffset[travInf.sourceId - 1], SEEK_SET);
            fwrite(&currOutputPos, sizeof(long), 1, output);

            fseek(output, currOutputPos, SEEK_SET);
            fwrite(&travInfFS, sizeof(TravInfFileStruct), 1, output);
        }

        prevIdOffset[travInf.sourceId - 1] = currOutputPos;

        /*---logging---*/
        cycles++;
        /*-------------*/
    }

    return head;
}

void saveIndexTable(FILE *file, IndexNode *node){

    while (node != NULL)
    {
        fwrite(&(node->index),sizeof(Index),1,file);
        node = node->next;
    }
}

int main(int argc, char *argv[]){
    if(argc != 4){
        printf("usage: indexTI {inputFile} {outputFile} {outputTableFile}");
        return -1;
    }

    FILE *input, *output, *table;
    long outStart;

    input = fopen(argv[1], "rb");
    if(!input){
        printf("file {%s} failed to open", argv[1]);
        return -1;
    }

    output = fopen(argv[2], "wb");
    if(!output){
        printf("file {%s} failed to open", argv[2]);
        return -1;
    }

    table = fopen(argv[3], "wb");
    if(!table){
        printf("file {%s} failed to open", argv[3]);
        return -1;
    }

    saveIndexTable(table, IndexFile(input, output));
    return 0;
}
