/*-----------------------------------------------------------*/
/* File: exampleCopyFile.c      by B. Mours (LAPP) Oct  9, 00*/
/*                                                           */
/*  This program copy frames from one file to a second file  */
/*   the input file name is the first program argument       */
/*   the output file name is the second program arguement    */
/*-----------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "FrameL.h"
  
int main(int argc, char *argv[])   /*-------------- main ----*/
{FrFile *iFile, *oFile;
 FrameH *frame;
 int level;

  if(argc != 4)
    {printf(" you need to provide two names: input and output file names"
             " and the compression level\n");
     return(0);}

    /*----- open the input file ---------------------------*/

  iFile = FrFileINew(argv[1]);
  if(iFile == NULL)
     {printf("Input File open error\n %s",FrErrorGetHistory());
      return(0);}

    /*------ open the output file --------------------------*/

  sscanf(argv[3],"%d",&level);
  oFile = FrFileONew(argv[2], level); 
  if(oFile == NULL)
     {printf("Cannot open output file %s\n",argv[2]);
      return(0);}

  printf(" OK I will copy all frames from file %s to file %s level=%d\n",
    argv[1],argv[2],level);

    /*------------------------- read all the frames ------*/

  while((frame = FrameRead(iFile)) != NULL)
      {printf(" copy frame %d run:%d\n",frame->frame, frame->run);
 
       if(FrameWrite(frame, oFile) != FR_OK)
          {printf("Error during frame write\n"
             "  Last errors are:\n%s",FrErrorGetHistory());
           return(0);}
 
       FrameFree(frame);}
       
    /*------------------------- close files --------------*/

  FrFileIEnd(iFile);
  FrFileOEnd(oFile);

  return(0);
}
