/*--------------------------------------------------------------------*/
/* File: exampleSelectFrame.c            by B. Mours (LAPP) Feb 11, 05*/
/*                                                                    */
/*  This program copy selected frames from one file to a second file  */
/*--------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "FrameL.h"
  
int main(int argc, char *argv[])   /*-------------- main ----*/
{FrFile *iFile, *oFile;
 FrameH *frame;
 int ierr;

    /*----- open the input file ---------------------------*/

  iFile = FrFileINew("file1.gwf");
  if(iFile == NULL)
     {printf("Input File open error\n %s",FrErrorGetHistory());
      return(0);}

    /*------ open the output file --------------------------*/

  oFile = FrFileONewM("file2", -1, "Select Frames", 3000); 
  if(oFile == NULL)
     {printf("Cannot open output file\n");
      return(0);}

    /*------------------------- copy all selected frames ------*/

  while((frame = FrameRead(iFile)) != NULL)
    {if(frame->GTimeS % 100 == 0) printf("Copy frame %d\n",frame->GTimeS);
    
     if(frame->dataQuality == 12)
       {ierr = FrameWrite(frame, oFile);
        if(ierr != FR_OK) 
           {printf("Write error\n Last errors:\n%s",FrErrorGetHistory());
           return(0);}}
 
     FrameFree(frame);}
       
    /*------------------------- close files --------------*/

  FrFileIEnd(iFile);
  FrFileOEnd(oFile);

  return(0);
}
