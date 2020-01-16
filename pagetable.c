#include "pagetableDS.h"
#include "byutr.h"
#include <getopt.h> // getopt
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
const int fileOpenFail = 5;

//Mariano Gutierrez & Cameron Ozalatar
void printToFile(TABLE * table, FILE* file); // recursive method for -p
void printToFileHelper(LEVEL* level, int endLen, FILE* file);
uint32_t logi2Phys(uint32_t logiAddr,int offBits,int pagesize,TABLE* table);

int main(int argc, char const *argv[]) {

     if(argc < 3) { // usage Error
         printf("Not enough arguments entered\n");
         exit(1);
     }

    char flag = '\0'; // flag in command line
    int pages = 0; // number of pages the table will contain 
    char* fileName = NULL; // the name of the trace file
    char* outFile = NULL; //for -p flag
    int memRefs = 0; // the number of memory references desired to be processed if specified -n option
    int logToPhys = 0; // a flage to determine if logical to physical printing is desired. -t option
    int hits = 0; // number of hits in the page table
    int misses = 0; // number of page faults i.e. not in the page table
    int totalBytes = 0;
    unsigned int* bits = malloc(sizeof(uint32_t) * argc); // the bits to pass in to the table constructor i.e. the bits per level

    // Parse cmd line args with get opts
    while((flag = getopt(argc, argv, "n:p:t")) != -1){
        switch(flag){
            case 'n':
                memRefs = atoi(optarg);
                break;
            case 'p':
                outFile = optarg;
                break;
            case 't':
                logToPhys = true;
                break;
            default:
                printf("Illegal option supplemented or missing arguements: %c\n", optopt);
        }
    }

    int restOfArgs = 1;
    if(memRefs != 0){
        restOfArgs += 2;
    }

    if(outFile != NULL){
        restOfArgs += 2;
    }

    if(logToPhys){
        restOfArgs += 1;
    }

    fileName = argv[restOfArgs++]; // it was either the first entry or pushed back
    while(restOfArgs < argc){ // load bits
        bits[pages++] = atoi(argv[restOfArgs++]);
    }

    //At this point all data has been retrieved from the command line
    TABLE* pageTable = tableConstructor(pageTable, pages, bits);  
    totalBytes = sizeof(TABLE) + (sizeof(unsigned int) * pages) * 3;
    FILE* redirectFile = NULL;
    p2AddrTr trace;
    FILE* traceFile; // the trace file
    int frameNum = 0; // data to input that in incrmemented sequentially.
    int endIdx = pageTable -> levelCount - 1;
    int addrOffset = pageTable -> shiftArr[endIdx];// offset for addresses
    int pageSize = pow(2,pageTable ->shiftArr[endIdx]);

    if ((traceFile = fopen(fileName,"rb")) == NULL) {
        fprintf(stderr,"cannot open the file: %s \n",fileName);
        exit(fileOpenFail); // error code for failure to open file
    }

    if(outFile != NULL)  {
        if((redirectFile = fopen(outFile,"w")) == NULL) {
            fprintf(stderr,"cannot open the file: %s \n",outFile);
            exit(fileOpenFail); // error code for failure to open file
        }
    }

   if(memRefs != 0) { // Process first N addresses i.e. -n option was selected.
      for(int i = 0; i < memRefs; i++)
      {
         if(NextAddress(traceFile, &trace)) { 
            MAP* entry; 
            if((entry = pageLookUp(pageTable,trace.addr)) != NULL) {
                hits++;
            }
            else {
            pageInsert(pageTable,trace.addr,frameNum);
            misses++;
            frameNum++;
            }
          }
          if(logToPhys) {
            uint32_t phys=logi2Phys(trace.addr,addrOffset,pageSize,pageTable);
            printf("%08x -> %08x\n",trace.addr,phys);
          }
      }  
    }
    else {  //proess the entire file
        while (!feof(traceFile)) {
        if (NextAddress(traceFile, &trace)) { 
            MAP* entry; 
            if((entry = pageLookUp(pageTable, (unsigned int) trace.addr)) != NULL) {
                hits++;
            }
            else {
            pageInsert(pageTable, (unsigned int)trace.addr,frameNum);
            misses++;
            frameNum++;
            }
          }
          if(logToPhys) {
            uint32_t phys=logi2Phys(trace.addr,addrOffset,pageSize,pageTable);
            printf("%08x -> %08x\n",trace.addr,phys);
          }
       }
    }

    // calculations
    int totalAddresses = hits + misses;
    // C is not nice like Java, so I must cast
    double percentHit = ( (double) hits / (double) totalAddresses * 100.0);
    double percentMiss = ((double) misses / (double) totalAddresses * 100.0); 
    printf("Page size: %d\n",pageSize);
    printf("Hits %d (%.2lf%), Misses %d (%.2lf%) # of Addresses %d\n",hits,percentHit,misses,percentMiss,totalAddresses);
    printf("Bytes used: %d\n", pageTable -> byteTotal);

    if(outFile != NULL) { // after addreses have been processed.
        printToFile(pageTable,redirectFile);
    }

    // clean up
    fclose(traceFile);
    tableDestructor(pageTable);
    free(pageTable);
    free(bits);
    if(outFile != NULL) fclose(redirectFile);
    return 0;
}
uint32_t logi2Phys(uint32_t logiAddr,int offBits,int pgSize, TABLE* table) {
    unsigned int offSetMask = genMask(offBits);
    uint32_t offset = offSetMask & logiAddr;
    int frame = pageLookUp(table,logiAddr) -> frame;
    uint32_t base = frame * pgSize;
    return offset + base;
}
// -p option
void printToFile(TABLE* table, FILE* file) {
    int endLen = table -> entryCount[table -> levelCount - 1];
    printToFileHelper(table -> pageZero, endLen,file);
}

void printToFileHelper(LEVEL* level, int endLen, FILE* file) {
if(level -> mapPtr != NULL) { // base case, at a leaf
    for(int i = 0; i < endLen; i++) {
        if(level -> mapPtr[i] != NULL) {
            bool validity = level -> mapPtr[i] -> valid;
            if(validity) {
                fprintf(file,"%08x -> %08x\n",i,level -> mapPtr[i] -> frame);
            }
        }
    }
    return; // will break end if level 1 only table smoothly.
}

if(level -> nextLevel != NULL) { // i.e. not a leaf
    for(int i = 0; i < level ->tablePtr ->entryCount[i]; i++) {
        if(level ->nextLevel[i] != NULL) { // check  if null'd out
            printToFileHelper(level ->nextLevel[i], endLen, file); // points to something else
        }
    } 
  } 
}

