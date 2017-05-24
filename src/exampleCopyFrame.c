/*-----------------------------------------------------------*/
/* File: exampleCopyFrame.c     by B. Mours (LAPP) Sep 14,98 */
/*                                                           */
/*  This program read frames from one file, it copy them     */
/*  and then keep only 3 adc's. The selection is done in two */
/*  differant ways for demo purpose. The input file (ran.dat)*/
/*  is a file produce by the example exampleFull.c program.  */
/*   The result is written in a file name out.gwf            */
/*-----------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "FrameL.h"
  
int main()   /*------------------------ main ------------------*/
{struct FrFile *iFile, *oFile;
 struct FrameH *frame, *copy;
 struct FrAdcData *adc;

     /*---------- open the input file --------------------------*/

  iFile = FrFileINew("ran.gwf");
  if(iFile == NULL)
     {printf("Cannot open the input file\n");
      return(0);}

     /*----------- open the output file ------------------------*/

  oFile = FrFileONew("out.gwf", 3); 
  if(oFile == NULL)
     {printf("Cannot open the output file\n");
      return(0);}

    /*---------------- read all the frames ----------------------*/

  while((frame = FrameRead(iFile)) != NULL)
      {printf(" copy frame %d run:%d\n",frame->frame, frame->run);

    /*--- we just keep 2 adc channels in the new frame ----------*/

       FrameTagAdc(frame,"Adc4 Adc5");

    /*--- we copy the frame to test the frame duplication--------*/
       
       copy = FrameCopy(frame);

    /*--- we copy one more channelto the output frame ----------*/

       adc = FrAdcDataFind(frame,"Adc8");
       FrAdcDataCopy(adc,copy);
       FrameDump(copy,stdout,3);

    /*--- we write the stripped frame ---------------------------*/
 
       if(FrameWrite(copy, oFile) != FR_OK)
          {printf("Error during frame write\n"
             "  Last errors are:\n%s",FrErrorGetHistory());
           return(0);}
 
       FrameFree(copy);
       FrameFree(frame);}
       
    /*------------------------- close files --------------------*/

  FrFileIEnd(iFile);
  FrFileOEnd(oFile);

  return(0);
}
