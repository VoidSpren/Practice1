#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <headers/Index.h>
#include <headers/TravelInfo.h>
#include <headers/TravInfFID.h>

//souce id maximo
#define MAXSRCID 1160

//crea un archivo indexado con la informacionde viaje del archivp de entrada
IndexNode* IndexFile(FILE* input, FILE* output){
    /*---logging--*/
    //Se calcula el tamaño del archivo y se guarda en inEnd
    long inEnd;
    fseek(input, 0, SEEK_END);
    inEnd = ftell(input);
    fseek(input, 0, SEEK_SET);
    /*-------------*/

    printf("----------Indexing and generating table------------\n");
    
    //Se crea un arreglo del tamaño 1160 y se llena con -1
    long prevIdOffset[MAXSRCID];
    memset(prevIdOffset, -1, sizeof(prevIdOffset));

    long currOutputPos;
    int erno;
    
    IndexNode *head, *tail;
    TravelInfo travInf;
    TravInfFID infoFID;
    Index index;

    //Se guarda la posicion de desplazamiento en bytes del archivo de salida
    //Luego se lee la información de un viaje y la guarda en la estrucutra travInf
    currOutputPos = ftell(output);
    erno = fread(&travInf, sizeof(TravelInfo), 1, input);
    if(erno < 1){
        printf("error reading input file\n");
    }

    infoFID.info = travInf;
    infoFID.nextOffset = -1;

    //se crea un indice con el id de origen y la posicion de desplazamiento del archivo de salida leida
    index.ID = travInf.srcId;
    index.ogOffset = currOutputPos;

    //se crea un lista enlazada de indices que guardara la posicion de desplazamiento del archivo de salida
    //donde esta por primera vez el id de origen guardado en el indice
    head = insertIndex(NULL, index);
    tail = head;

    fwrite(&infoFID, sizeof(TravInfFID), 1, output);

    //En la posición correpondiente del numero de origen - 1
    //se guarda la posición donde se escribio al archivo
    prevIdOffset[travInf.srcId - 1] = currOutputPos;

    /*---logging---*/
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

        //Se guarda la posición del archivo donde se esta escribiendos
        currOutputPos = ftell(output);

        //Se lee la información del siguiente viaje y verifica si ya se llego al final del archivo de entrada
        size_t count = fread(&travInf, sizeof(TravelInfo), 1, input);
        if(feof(input) || count != 1) break;

        //Se guarda la info de vaije y un apuntador a -1 (invalido) en la estructura infoFID
        infoFID.info = travInf;
        infoFID.nextOffset = -1;

        if(prevIdOffset[travInf.srcId - 1] < 0){
            //En este caso es la primera vez que se encuentra este id de origen
            
            //Se crea un index con el desplazamiento actual en el archivo de salida
            index.ID = travInf.srcId;
            index.ogOffset = currOutputPos;

            //dicha estructura se agrega a la lista enlazada
            tail = insertIndex(tail, index);

            //Se escribe esta estructura en el archivo
            fwrite(&infoFID, sizeof(TravInfFID), 1, output);
        }else{
            //De prevIdOffset[travInf.srcId - 1] ser diferente de -1 se usa la
            //posicion guardada en ese punto y se remplaza el desplazamiento de
            //la estructura guardada en el archivo por el valor de la posición currOutputPos
            fseek(output, prevIdOffset[travInf.srcId - 1], SEEK_SET);
            fwrite(&currOutputPos, sizeof(long), 1, output);

            //Luego se vuelve a la posición donde no se ha escrito aún
            //y se escribe la estructura del nuevo viaje con el mismo origen
            fseek(output, currOutputPos, SEEK_SET);
            fwrite(&infoFID, sizeof(TravInfFID), 1, output);
        }

        // en el array en la posición del id de origen - 1 se 
        //coloca el desplazamiento de la ultima estructura de viaje escrita
        prevIdOffset[travInf.srcId - 1] = currOutputPos;

        /*---logging---*/
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
    //Se calcula el tamaño del csv
    long inEnd;
    fseek(input, 0, SEEK_END);
    inEnd = ftell(input);
    fseek(input, 0, SEEK_SET);
    /*-------------*/

    printf("----------conversion csv to bin------------\n");

    //Se lee hasta el salto de linea descartando los encabezados del csv
    char trash[200];
    int erno = fscanf(input, "%199s", trash);
    if(erno == EOF) printf("failed to read csv input\n");

    TravelInfo info;

    //Empieza la conversión y cuenta de los ciclos
    int cycles = 0;
    while(!feof(input)){
        //Cada 100.000 ciclos muestra el porcentaje de avance 
        if(cycles > 100000){
            cycles = 0;
            printf("%%%f\n", ((float)ftell(input))/inEnd *100.0);
        }

        //Lee la información de un viaje y la guarda en un apuntador a la estructura info
        int erno = strFileToTravInf(&info, input);
        
        //Se comprueba si se termino de leer la información
        if(feof(input)) break;

        //Escribe la escructura info binario en el archivo output
        fwrite(&info, sizeof(TravelInfo), 1, output);

        cycles++;
    }

    printf("----------conversion finished------------\n");
}

int main(int argc, char *argv[]){
    //Se comprueba que no haya errores al ingresar los 4 archivos requeridos
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

    //Se abre un archivo para guardar la lista enlazada de las primeras apariciones
    table = fopen(argv[4], "wb");
    if(!table){
        printf("file {%s} failed to open", argv[4]);
        return -1;
    }

    //Se crea una lista enlazada de las primeras apariciones para cada origen y su indice
    //Luego se guarda esta lista en un archivo binario
    saveIndexTable(table, IndexFile(binIO, output));   

    fclose(binIO);

    //see borra el archivo csv en binario
    remove(argv[2]);
    
    return 0;
}
