/*
 * ObjectManager.h
 * COMP 2160 Programming Practices
 * Assignment 4
 * @author Saif Mahmud,
 * ID: 7808507
 * @version 04/12/2
 */
#ifndef _OBJECT_MANAGER_H
#define _OBJECT_MANAGER_H

// The number of bytes of memory we have access to -- put here so everyone's consistent.
#ifndef MEMORY_SIZE
#define MEMORY_SIZE 1024*512
#endif

#define NULL_REF 0

typedef unsigned long Ref;

// Note that we provide our entire interface via this object module and completely hide our index (see course notes).
// This allows us to change indexing strategies without affecting the interface to everyone else.

// This function trys to allocate a block of given size from our buffer.
// It will fire the garbage collector as required.
// We always assume that an insert always creates a new object...
// On success it returns the reference number for the block of memory allocated for the object.
// On failure it returns NULL_REF (0)
Ref insertObject( const int size );

// returns a pointer to the object being requested given by the reference id
void *retrieveObject( const Ref ref );

// update our index to indicate that we have another reference to the given object
void addReference( const Ref ref );

// update our index to indicate that a reference is gone
void dropReference( const Ref ref );

// initialize the object manager
void initPool();

// clean up the object manager (before exiting)
void destroyPool();

// This function traverses the index and prints the info in each entry corresponding to a block of allocated memory.
// You should print the block's reference id, it's starting address, and it's size (in bytes).
void dumpPool();

#endif
