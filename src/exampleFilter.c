/*---------------------------------------------------------------------------*/
/* Test program for FrFilter:                                exampleFilter.c */
/*---------------------------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "FrameL.h"
#include <stdarg.h>
#include "FrameL.h"
/*---------------------------------------------------------------------------*/
int main()     /*-------------------- main-----------*/
/*---------------------------------------------------------------------------*/
{FrFilter *filter;
 FrVect *vect;
 FrProcData *proc;
                     /*-------------------------------------create one filter*/

 filter = FrFilterNew("f1",1000.,1.,2, 1., 0., 2.34, 2.);
 FrFilterDump(filter, stdout, 2);

 vect = FrFilterPackToVect(filter);    /*----- convert the filter to a vector-*/
 FrVectDump(vect, stdout, 2);

 filter = FrFilterGetFromVect(vect);   /*-- create a filter from a vector ---*/
 FrFilterDump(filter, stdout, 2);
 
 proc = FrProcDataNew (NULL,"proc data", 1000., 1000, -64);
 FrProcDataAddFilter(proc,filter);

 filter = FrFilterNew("f2",1000.,1.,1, 1., 2.); /*--create a second filter*/
 FrProcDataAddFilter(proc,filter);
 FrProcDataDump(proc, stdout, 2);
 
 filter = FrProcDataGetFilter(proc, "f1");  /*----- get the two filters ---*/
 FrFilterDump(filter, stdout, 3);
 filter = FrProcDataGetFilter(proc, "f2");
 FrFilterDump(filter, stdout, 3);

 return(0);
}
