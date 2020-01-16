// Cameron Ozatalar
// Mariano Gutierrez

//header file gaurds
#ifndef PAGETABLE_H
#define PAGETABLE_H
#include <stdbool.h>
#include <stdint.h>

typedef struct map {
  bool valid; // false == 0, true == 1
  unsigned int frame;
} MAP;

// arbitrary level of the page table
typedef struct Level {
   int currentDepth;
   struct PageTable* tablePtr; // useful to access information
   MAP** mapPtr; // mapPtr;
   struct Level** nextLevel; // level contains an array of pointers to the next level (Level*) or map entries.
} LEVEL;

//Main part of the table 
typedef struct PageTable {
  LEVEL* pageZero; // the root node ptr level 0
  int* numBits; // additonal member var to preconfig the other arrays below;
  unsigned int* bitmaskArr; //  due to variable number of bits, max is 32
  unsigned int* shiftArr; // # of bits to shift level "i" page bits
  unsigned int* entryCount;  // # of possible pages for arbitrary level "i"
  int levelCount; // number of levels in the table
  int byteTotal; // total bytes used by the entire tree.
} TABLE;

TABLE* tableConstructor(TABLE* table, int pages, unsigned int* bits);
LEVEL* levelConstructor(LEVEL* level, TABLE* origTable, int depth);
MAP* mapConstuctor(MAP* page, bool validity, unsigned int frameNum);
void tableinit(TABLE* table, unsigned int* bits); // init all the shifting configs
void MapListConstructor(LEVEL* level,TABLE* origTable); // used to fill in the map to all invalid entries
void tableDestructor(TABLE* table); //probably best if we have one grand deconstrut
void destructorHelper(LEVEL* level, int len); // recurse thru and delete all levels and their map and next level ptrs
unsigned int genMask(unsigned int num); //for generating bitmasks e.g. 8 -> 0xFF

uint32_t logicalToPage(uint32_t logicalAddress, unsigned int mask, unsigned int shift);
MAP* pageLookUp(TABLE* origTable, uint32_t logicalAddress);
void pageInsert(TABLE* pgTable, uint32_t logicalAddress, unsigned int frame);
void pageInsertHelper(LEVEL* levelPtr,uint32_t logicalAddress, unsigned int frame);

#endif