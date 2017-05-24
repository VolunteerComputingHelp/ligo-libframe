/*---------------------------------------------------------------------*/
/* File: exampleMultiTOC.c                      by B. Mours Jul 31, 01 */
/*                                                                     */
/*  This program reshape frame from one file.                          */
/* It run several time to check for memmory leaks                      */
/*  The program FrMultiW need to be run before to generate the file.   */
/*---------------------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "FrameL.h"
  
int main(int argc, char *argv[])   /*-------------- main ----*/
{FrFile *iFile;
 FrameH *frame, *frameOut;
 int iLoop;

 for(iLoop = 0; iLoop < 10000; iLoop ++) {
   if(iLoop < 10 || (iLoop%10) == 0) printf("start loop %d\n",iLoop);

             /*-----------------Open input file---------------*/

   iFile = FrFileINew("file6.gwf file7.gwf file8.gwf");
   if(iFile == NULL)
     {printf("Cannot open file\n");
      return(0);}

   frame = NULL;
   frameOut = NULL;
   while((frame = FrameRead(iFile)) != NULL) {
     if(iLoop == 0) FrameDump(frame, stdout, 1);

     if(frame->GTimeN/100000000 == 5 || frameOut == NULL) {
       FrameReshapeEnd(frameOut);
       if(iLoop < 3) FrameDump(frameOut, stdout, 2);
       FrameFree(frameOut);
       frameOut = FrameReshapeNew(frame, 10, (frame->GTimeN/100000000+5)%10);}
     else {
       FrameReshapeAdd(frameOut, frame);}}      

   FrameReshapeEnd(frameOut);
   if(iLoop < 3) FrameDump(frameOut, stdout, 2);
   FrameFree(frameOut);
    
   FrFileIEnd(iFile);

  }
 return(0);
}



