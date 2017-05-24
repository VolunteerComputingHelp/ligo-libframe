/*-----------------------------------------------------------*/
/* File: exampleMultiR.c        by B.Mours (LAPP) Oct  9, 00 */
/*                                                           */
/*  This program reads the sevreral files produced by the    */
/*   program exampleMultiW.c. It open the files by group of  */
/*   two files.                                              */
/*-----------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "FrameL.h"
  
int main()   /*-------------- main ------*/
{FrFile *iFile;
 FrameH *frame;
 long fileIndex;
 char fileName[100];
 int i;

    /*------------------------------------- file open--------*/

  fileIndex = 0;

  for(i=0; i<5888; i++)
   {fileIndex = (fileIndex+2)%10;
    sprintf(fileName,"file%ld.dat file%ld.dat",fileIndex,fileIndex+1);
    printf(" Try to open file %s\n",fileName);
 
    iFile = FrFileINew(fileName);
    if(iFile == NULL)
       {printf("Error during file opening: %s\n"
               "  Last errors are:\n%s",
                fileName,FrErrorGetHistory());
        return(0);}

    /*-------------------------- read all the frames ------*/

    while((frame = FrameRead(iFile)) != NULL)
        {if(i < 2) FrameDump(frame, stdout, 2);
         FrameFree(frame);}
       
    /*-------------------------- close files --------------*/

    FrFileIEnd(iFile);}

  return(0);
}


