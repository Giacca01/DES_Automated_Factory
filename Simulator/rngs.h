/* $Id: rngs.h 55 2005-09-13 22:29:52Z asminer $ */
/* ----------------------------------------------------------------------- 
 * Name            : rngs.h  (header file for the library file rngs.c) 
 * Author          : Steve Park & Dave Geyer
 * Language        : ANSI C
 * Latest Revision : 09-22-98
 * ----------------------------------------------------------------------- 
 */

#if !defined( _RNGS_ )
#define _RNGS_

// generate a random number between 0 and 1
double Random(void);

// sets the first number of the sequence
void   PlantSeeds(long x);

// retrives the first value of the sequence
void   GetSeed(long *x);
void   PutSeed(long x);

// Selects a sequence of random numbers
void   SelectStream(int index);

void   TestRandom(void);

#endif
