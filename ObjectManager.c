/*
 * ObjectManager.c
 * COMP 2160 Programming Practices
 * Assignment 4
 * @author Saif Mahmud,
 * ID: 7808507
 * @version 04/12/2020
 * Implementing Garbage Collector and make use of it.
 */
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "ObjectManager.h"
// You can add other header files if needed

//----function prototypes---//
void compact();
//
// tracks the next reference (ID) to use, we start at 1 so we can use 0 as the NULL reference
static Ref nextRef = 1;

// A Memblock holds the relevent information associated with an allocated block of memory by our memory manager
typedef struct MEMBLOCK MemBlock;


// information needed to track our objects in memory
struct MEMBLOCK
{
  int numBytes;    // how big is this object?
  int startAddr;   // where the object starts
  Ref ref;         // the reference used to identify the object
  int count;       // the number of references to this object
  MemBlock *next;  // pointer to next block.  Blocks stored in a linked list.
};


// The blocks are stored in a linked list where the start of the list is pointed to by memBlockStart.
static MemBlock *memBlockStart; // start of linked list of blocks allocated
static MemBlock *memBlockEnd;   // end of linked list of blocks allocated
static int numBlocks;            // number of blocks allocated

// our buffers.  This is where we allocate memory from.  One of these is always the current buffer.  The other is used for swapping
//during compaction stage.

static unsigned char buffer1[MEMORY_SIZE];
static unsigned char buffer2[MEMORY_SIZE];

// points to the current buffer containing our data
static unsigned char *currBuffer = buffer1;

// points to the location of the next available memory location
static int freeIndex = 0;


// performs required setup
void initPool()
{
    numBlocks = 0;
    memBlockStart = NULL;
    memBlockEnd = NULL;
}

// performs required clean up
void destroyPool()
{
    assert(memBlockStart != NULL && memBlockEnd != NULL);
    if (memBlockStart != NULL && memBlockEnd != NULL)
    {
        MemBlock *prev = memBlockStart;
        MemBlock *curr = prev;
        while(curr != NULL)
        {
            curr = curr->next;
	    free(prev);
            prev = curr;
        }
        memBlockStart = NULL;
        memBlockEnd = NULL;
        numBlocks = 0;
    }
}

// Adds the given object into our buffer. It will fire the garbage collector as required.
// We always assume that an insert always creates a new object...
// On success it returns the reference number for the block allocated.
// On failure it returns NULL_REF (0)
Ref insertObject( const int size )
{
    Ref out = NULL_REF;
    assert(size > 0);
    if(MEMORY_SIZE-freeIndex < size)
    {
        compact();
    } else {
        if (size > 0)
        {
            out = nextRef++;
            MemBlock *newObj = (MemBlock *) malloc(sizeof(MemBlock));
            assert(newObj != NULL);

            newObj->numBytes = size;
            newObj->ref = out;
            newObj->count = 1;
            newObj->startAddr = freeIndex;
            newObj->next = NULL;
            numBlocks++;

            if (numBlocks == 1) {
                memBlockStart = newObj;
                memBlockEnd = newObj;
            } else {
                memBlockEnd->next = newObj;
                memBlockEnd = newObj;
            }
            freeIndex = freeIndex + size;
        }
    }
    return out;
}

// returns a pointer to the object being requested
void *retrieveObject( const Ref ref )
{
    void *ptr = NULL_REF;
    assert( ref > 0 && ref < nextRef);
    bool isFound = false;
    if (ref > 0 && ref < nextRef)
    {
        MemBlock *curr = memBlockStart;
        while (curr != NULL && !isFound)
        {
            if(curr->ref == ref)
            {
                ptr = &currBuffer[curr->startAddr];
                isFound = true;
            }
            curr = curr->next;
        }
    }
    return ptr;
}

// update our index to indicate that we have another reference to the given object
void addReference( const Ref ref )
{
    assert( ref > 0 && ref < nextRef);
    if (ref > 0 && ref < nextRef)
    {
        MemBlock *curr = memBlockStart;
        while (curr != memBlockEnd)
        {
            if(curr->ref == ref)
            {
                curr->count++;
            }
            curr = curr->next;
        }
    }
}

// update our index to indicate that a reference is gone
void dropReference( const Ref ref )
{
    assert( ref > 0 && ref < nextRef);
    if (ref > 0 && ref < nextRef)
    {
        MemBlock *prev = memBlockStart;
        MemBlock *curr = memBlockStart->next;
        bool dropped = false;

        //---------- if its the first node -----------//
        if(prev->ref == ref)
        {
            memBlockStart = prev->next;
            prev->count--;
            free(prev);
            dropped = true;
        } else if(memBlockEnd->ref == ref) { //---------- if its the last node -----------//
            while (curr->next != memBlockEnd)
            {
                curr = curr->next;
            }
            curr->count--;
            free(curr->next);
            curr->next = NULL;
            memBlockEnd = curr;
            dropped = true;
        } else { //---------- if its in the middle -----------//
            while (curr != NULL && dropped == false)
            {
                if (curr->ref == ref)
                {
                    curr->count--;
                    free(curr);
                    dropped = true;
                }
                prev = prev->next;
                curr = prev;
                curr = curr->next;
            }
        }
    }
}

// performs our garbage collection
void compact()
{
    assert(memBlockStart != NULL);
    assert(memBlockEnd != NULL);
    if(memBlockStart != NULL && memBlockEnd != NULL)
    {
        int totalObj = 0;
        int bytesInUse = freeIndex;
        int collectedBytes;
        unsigned char *nextBuffer;
        MemBlock *curr = memBlockStart;
        freeIndex = 0;

        if(currBuffer == buffer1)
        {
            nextBuffer = buffer2;
        } else {
            nextBuffer = buffer1;
        }
        while(curr != NULL) // -------------- coping the old buffer to the new buffer -------------
        {
            memcpy(&nextBuffer[freeIndex], &currBuffer[curr->startAddr], curr->numBytes);
            freeIndex = freeIndex + curr->numBytes;
            totalObj++;
            curr = curr->next;
        }
        currBuffer = nextBuffer;
        collectedBytes = bytesInUse-freeIndex;
    #ifndef DNDEBUG
        printf("-------------// Garbage collector //-------------\n");
        printf("Existing Objects: %d, Bytes in use: %d Bytes Collected: %d\n", totalObj, freeIndex, collectedBytes);
        printf("Data Structures:\n");
        dumpPool();
    #endif
    }//if
}//compact()

void dumpPool()
{
    assert(memBlockStart != NULL);
    assert(memBlockEnd != NULL);
    if(memBlockStart != NULL && memBlockEnd != NULL)
    {
        MemBlock *curr = memBlockStart;
        while (curr != NULL) {
            printf("\nID: %lu, Start Address: %d, Size: %d bytes.", curr->ref, curr->startAddr, curr->numBytes);
            curr = curr->next;
        }
	printf("\n");
    }
}
