/*-----------------------------------------------------------*/
/* File: exampleDumpFile.c     by B. Mours (LAPP) Dec  7, 00 */
/*                                                           */
/*  This program produces of short summary for each frame    */
/*  present in a file. The file name is the program argument */
/*  An optional second argument is the debug level           */
/*-----------------------------------------------------------*/
#include <math.h>
#include "FrameL.h"
  
int main(int argc, char *argv[])   /*-------------- main ----*/
{FrFile *iFile;
 FrameH *frame;
   
    /*------------------------------------ file open---------*/

  iFile = FrFileINew(argv[1]);
  if(iFile == NULL)
     {printf("Cannot open file %s\n",argv[1]);
      return(0);}

  frame = NULL;
  while((frame = FrameReadRecycle(iFile, frame)) != NULL)
    {FrameDump(frame, stdout, 2);}

  return(0);
}
