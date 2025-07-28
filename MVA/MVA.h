#ifndef __MVA__
#define __MVA__

#include <malloc.h>
#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include <fstream>
// To control output precision when saving on file
#include <iomanip> 
#include <vector>
#include <math.h>

#define TP_FILENAME "tp.txt"
#define RP_FILENAME "rp.txt"
#define MAN_FILENAME "man.txt"


void mva(int, int, double *, double [6][6], char *);
void saveToFile(int, double *, const std::string&);

#endif