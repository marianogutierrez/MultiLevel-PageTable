
#include <stdio.h>
#include <stdlib.h>
#include <math.h> // for the pow function
#include "pagetableDS.h"
const int SEVERE_ERROR = 10;

//Mariano Gutierrez & Cameron Ozalatar
TABLE* tableConstructor(TABLE* table, int pages, unsigned int* bits) {
    table = malloc(sizeof(TABLE));
    table -> levelCount = pages;
    table -> bitmaskArr = malloc(sizeof(unsigned int) * pages); 
    table -> shiftArr = malloc(sizeof(unsigned int) * pages);
    table -> entryCount = malloc(sizeof(unsigned int) * pages);
    tableinit(table, bits); // init the previously malloc'd fields
    table -> pageZero = levelConstructor(table -> pageZero, table, 0); // starting with depth at zero
    // table struct, bitmask, shiftArr, and entryCount for below
    table -> byteTotal += sizeof(TABLE) + (sizeof(unsigned int) * pages) * 3; 
    return table;
}

//call table constructor first, lest you seg fault! could have leaf constructor
LEVEL* levelConstructor(LEVEL* level, TABLE* origTable, int depth) {
    level = malloc(sizeof(LEVEL));
    level -> currentDepth = depth;
    level -> tablePtr = origTable;
    if(origTable -> levelCount == 1) { // special case where only the map is init'd
        level -> nextLevel = NULL;
        level -> mapPtr = malloc(origTable -> entryCount[depth] * sizeof(MAP*)); 
        MapListConstructor(level, level->tablePtr); // fill all with invalids
        int mallocd = origTable -> entryCount[depth] * sizeof(MAP*);
        origTable -> byteTotal += sizeof(LEVEL) + mallocd; // add on bytes of map
    }
    else {
        level -> nextLevel = calloc(origTable -> entryCount[depth], sizeof(LEVEL*)); // nulled out with calloc
        level -> mapPtr = NULL; // to start off i.e. empty table
        int callocd = origTable -> entryCount[depth] * sizeof(LEVEL*);
        origTable -> byteTotal += sizeof(LEVEL) + callocd; 
    }
    return level;
}

void MapListConstructor(LEVEL* level,TABLE* origTable) {
    // the amt the leaf should alloc for its map Ptr
    int endLen = origTable ->entryCount[origTable ->levelCount - 1];
    for(int i = 0; i < endLen; i++) {
        MAP* entry = mapConstuctor(entry, false, -1); // supplant invalids
        level -> mapPtr[i] = entry;
        origTable -> byteTotal += sizeof(MAP); 
    }
}

MAP* mapConstuctor(MAP* map, bool validity, uint32_t frameNum) {
    map = malloc(sizeof(MAP)); // this is freed in the destructor 
    map -> valid = validity;
    map -> frame = frameNum;
    return map;
}

void tableDestructor(TABLE* table) {
    free(table -> bitmaskArr);
    free(table -> shiftArr);
    free(table ->numBits);
    int endLen = table -> entryCount[table -> levelCount - 1];
    destructorHelper(table -> pageZero, endLen);
    free(table -> entryCount);
}

void destructorHelper(LEVEL* level, int endLen) {
    if(level -> mapPtr != NULL) { // base case, at a leaf
        for(int i = 0; i < endLen; i++) {
            free(level -> mapPtr[i]); // free all memory from mapConstuctor
        }
        return; // will break end if level 1 only table smoothly.
    }

    if(level -> nextLevel != NULL) { // i.e. not a leaf
        for(int i = 0; i < level ->tablePtr ->entryCount[i]; i++) {
            if(level ->nextLevel[i] != NULL) { // check if null'd out
                destructorHelper(level ->nextLevel[i], endLen); // points to something else
                free(level ->nextLevel[i]); // once back free the pointer
            }
        } 
    } // done iterating thru the nextLevel arr
    free(level -> nextLevel); // the one at the very end i.e. from level zero
}

//Pre adjust the containers for top root. Note adddress is 32 bit
void tableinit(TABLE* table, unsigned int* bits) {
    int pages = table -> levelCount;
    int maxBits = 32; //init the shift array 
    int i; // counter for loop
    for(i = 0; i < pages; i++) {
        table -> shiftArr[i] = maxBits - bits[i]; 
        maxBits -= bits[i];
        if(maxBits < 1)  {
            printf("You entered more than 32 bits. Program terminating\n");
            // Severe error, i.e. the user provided so many bits s.t. there is no offset left!
            exit(SEVERE_ERROR); 
        }
    }
    //table -> offsetShift = maxBits; // what ever was left 
    for(int i = 0; i < pages; i++) { // init in the entry table
        table -> entryCount[i] = pow(2,bits[i]); // depends on each level e.g. level i = 2^entryCount[i]
    }
    for(int i = 0; i < pages; i++) { //init the bitMaskArray
        table -> bitmaskArr[i] = genMask(bits[i]) << table -> shiftArr[i];
    }
}

unsigned int genMask(unsigned int num) { // creates the mask necessary based off num of bits.
    return (1 << num) - 1 ; //  e.g 2^8 = 256 so -1 for 255
}
/* ***************************** The main algos *********************************** */

// Practically the description from the .h file. 
unsigned int logicalToPage(uint32_t logicalAddress, unsigned int mask, unsigned int shift) {
    return (logicalAddress & mask) >> shift;
}

MAP* pageLookUp(TABLE* origTable,uint32_t logicalAddress) {
    int pages = origTable -> levelCount;
    LEVEL* current = origTable -> pageZero;
    for(int i = 0; i < pages; i++) {
        //Get the page number
        int pageIdx = logicalToPage(logicalAddress,origTable -> bitmaskArr[i], origTable -> shiftArr[i]);
            if(current != NULL && current -> mapPtr == NULL) { // i.e. not on a leaf 
                current = current -> nextLevel[pageIdx]; 
            }
            else if(current != NULL && current -> mapPtr[pageIdx] != NULL) {
                if (current -> mapPtr[pageIdx] -> valid) { // avoid seg faults at all costs
                    return current -> mapPtr[pageIdx];
                }
            }
            else { // not found
                return NULL;
            }
    }
    return NULL;
}

void pageInsert(TABLE* pgTable, uint32_t logicalAddress, unsigned int frame) {
     pageInsertHelper(pgTable -> pageZero,logicalAddress,frame);
}


void pageInsertHelper(LEVEL* levelPtr, uint32_t logicalAddress, unsigned int frame) {
     int currDepth = levelPtr -> currentDepth;
     unsigned int *entryCount = levelPtr -> tablePtr -> entryCount;
     int pageIdx;
     unsigned int* maskRef = levelPtr -> tablePtr -> bitmaskArr;
     unsigned int* shiftRef = levelPtr -> tablePtr -> shiftArr;
     int bytes = levelPtr -> tablePtr ->byteTotal;
    // end short cut vars

     if(levelPtr -> tablePtr -> levelCount == 1) { 
        pageIdx = logicalToPage(logicalAddress, maskRef[0],shiftRef[0]); 
        // do not want to mess with indices since always zero
     }
     else {
         pageIdx = logicalToPage(logicalAddress, maskRef[currDepth],shiftRef[currDepth]);
     }

    if(levelPtr -> mapPtr != NULL) { // BASE CASE: if the leaf, set appropriate pg idx to valid and store frame
        MAP* map = mapConstuctor(map,true,frame);
        levelPtr -> mapPtr[pageIdx] = map;
        return;
    }
    else {
        if(levelPtr -> nextLevel[pageIdx] == NULL) { // do not want to make dupes
            LEVEL* newLevel = levelConstructor(newLevel, levelPtr -> tablePtr, currDepth + 1); // update depth
            if(currDepth + 1 == (newLevel->tablePtr->levelCount) - 1)  {
                newLevel -> mapPtr = malloc(sizeof(MAP*) * entryCount[currDepth + 1]);
                MapListConstructor(newLevel, newLevel ->tablePtr); // fill all with invalids
                free(newLevel -> nextLevel); // remove the calloc from the constructor!
                newLevel ->nextLevel = NULL; // null it out
                levelPtr -> nextLevel[pageIdx] = newLevel; // make sure to connect it!
                //undo calloc bytes added from constructor
                bytes -= sizeof(LEVEL*) * entryCount[currDepth + 1]; 
                bytes += sizeof(MAP*) * entryCount[currDepth + 1]; // do malloc
            }
            else {
                // update the internal structure by adding a new page
                levelPtr ->nextLevel[pageIdx] = newLevel; 
            }
            pageInsertHelper(newLevel, logicalAddress,frame); // keep recursing regardless, since havent hit leaf
        }
        else { //e.g. 8 bit offse address 0x1212 34! 12 and then 0x1212 43! 12!
            pageInsertHelper(levelPtr->nextLevel[pageIdx],logicalAddress,frame); 
        }
    }
}
