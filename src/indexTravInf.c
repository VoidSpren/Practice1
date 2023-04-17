#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <headers/Index.h>
#include <headers/TravelInfo.h>
#include <headers/TravInfFID.h>

//Se crea una lista enlazada de 
IndexNode* IndexFile(FILE* input, FILE* output){
    /*---loggin--*/
    //Se calcula el tamaño del archivo y se guarda en inEnd
    long inEnd;
    fseek(input, 0, SEEK_END);
    inEnd = ftell(input);
    fseek(input, 0, SEEK_SET);
    /*-------------*/

    printf("----------Indexing and generating table------------\n");
    
    //Se crea un arreglo del tamaño 11160 y se llena con -1
    long prevIdOffset[1160];
    memset(prevIdOffset, -1, sizeof(prevIdOffset));

    long currOutputPos;
    int erno;
    
    IndexNode *head, *tail;
    TravelInfo travInf;
    TravInfFID infoFID;
    Index index;

    //Se guarda la posicion de desplazamiento en bytes del archivo
    //Luego se lee la información de un viaje y la guarda en la estrucutra travInf
    currOutputPos = ftell(output);
    fread(&travInf, sizeof(TravelInfo), 1, input);

    infoFID.info = travInf;
    infoFID.nextOffset = -1;

    index.ID = travInf.srcId;
    index.ogOffset = currOutputPos;

    head = insertIndex(NULL, index);
    tail = head;

    fwrite(&infoFID, sizeof(TravInfFID), 1, output);

    //En la posición correpondiente del numero de origen - 1
    //se guarda la posición donde leyó al archivo
    prevIdOffset[travInf.srcId - 1] = currOutputPos;

    /*---loggin---*/
    int cycles = 0;
    /*-------------*/
    while(!feof(input)){

        /*---logging---*/

        //Cada 100.000 ciclos muestra el porcentaje de avance
        if(cycles > 100000){
            cycles = 0;
            printf("%%%f\n", ((float)ftell(input))/inEnd *100.0);
        }
        /*-------------*/

        //Se guarda la posición del archivo donde se esta leyendo
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

        /*---loggin---*/
        cycles++;
        /*-------------*/
    }

    printf("----------Indexinf and table finished------------\n");

    return head;
}

//Recorre la lista enlazada y la guarda en un archivo binario
void saveIndexTable(FILE *file, IndexNode *node){

    while (node != NULL)
    {
        //Se escribe la información del nodo de la lista enlazada
        fwrite(&(node->index),sizeof(Index),1,file);
        node = node->next;
    }
}

//Pasa el csv a binario
void csvToBin(FILE *input, FILE *output){
    //Se calcula la el tamaño en bytes del csv
    long inEnd;
    fseek(input, 0, SEEK_END);
    inEnd = ftell(input);
    fseek(input, 0, SEEK_SET);
    /*-------------*/

    printf("----------conversion csv to bin------------\n");

    //Se lee hasta el salto de linea descartando los encabezados del csv
    char trash[200];
    fscanf(input, "%199s", trash);

    TravelInfo info;

    //Empieza la conversión y cuanta los ciclos
    int cycles = 0;
    while(!feof(input)){
        //Cada 100.000 ciclos muestra el porcentaje de avance 
        if(cycles > 100000){
            cycles = 0;
            printf("%%%f\n", ((float)ftell(input))/inEnd *100.0);
        }

        //Lee la información de un viaje y la guarda en un apuntador a la estructura info
        int erno = strFileToTravInf(&info, input);
        
        //Solo comprueba si ya termino de leer la información y evita leer la ultima vez
        if(feof(input)) break;

        //Escribe la escructura info binario en el archivo output
        fwrite(&info, sizeof(TravelInfo), 1, output);

        //loging
        cycles++;
    }

    printf("----------conversion finished------------\n");
}

int main(int argc, char *argv[]){
    //Se comprueba que no alla errores al ingresar los 4 archivos requeridos
    if(argc != 5){
        printf("usage: indexTI {inputFile} {binioFile} {outputFile} {outputTableFile}\n");
        return -1;
    }

    FILE *input, *binIO, *output, *table;
    long outStart;

    //Sa abre el csv para leerlo
    input = fopen(argv[1], "r");
    if(!input){
        printf("file {%s} failed to open", argv[1]);
        return -1;
    }

    //Se abre el archivo donde se guardara el csv en binario
    binIO = fopen(argv[2], "wb");
    if(!binIO){
        printf("file {%s} failed to open", argv[2]);
        return -1;
    }

    //Se pasa de csv a binario
    csvToBin(input, binIO);
    
    fclose(input); 
    fclose(binIO);
    binIO = NULL;

    //Se abre el archivo para leer el csv en binario
    binIO = fopen(argv[2], "rb");
    if(!binIO){
        printf("file {%s} failed to open", argv[2]);
        return -1;
    }
    
    //Se abre el archivo para escribir el indexado en binario
    output = fopen(argv[3], "wb");
    if(!output){
        printf("file {%s} failed to open", argv[3]);
        return -1;
    }

    //Se abre un archivo para guardar la lista enlazda de las primeras apariciones
    table = fopen(argv[4], "wb");
    if(!table){
        printf("file {%s} failed to open", argv[4]);
        return -1;
    }

    //Se crea una lista enlazada de las primeras apariciones para cada origen y su indice
    //Luego se guarda esta lista en un archivo binario
    saveIndexTable(table, IndexFile(binIO, output));   

    fclose(binIO);
    remove(argv[2]);
    
    return 0;
}
