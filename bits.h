#ifndef BITS_H
#define BITS_H

#include <stdio.h>
#include <stdlib.h>



int getBit(unsigned long bitStream, int position);
unsigned long setBit(unsigned long bitStream, int position);
unsigned long clearBit(unsigned long bitStream, int position);
unsigned long toggleBit(unsigned long bitStream, int position);
char* showBits(unsigned long bitStream, int startPosition, int endPosition);

#endif
