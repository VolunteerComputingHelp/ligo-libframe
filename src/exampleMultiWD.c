/*----------------------------------------------------------------*/
/* File: exampleMulwritten in multiples directorie    Apr 16, 2009*/
/*                                                                */
/*   This  program creates a frame and writes it in               */  
/*   file for several files. There are several frames             */
/*   par files written in multiples directories.                  */
/*----------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "FrameL.h"
  
int main(int argc, char **argv)  /*-------------- main------------*/
{
  FrFile *oFile;
  FrameH *frame;
  double sampleRate;
  long nData, framePerFile, iFrame, dirPeriod;
  
  framePerFile = 100;
  dirPeriod = 1000;
  oFile = FrFileONewMD("./Test", 9,"test",framePerFile,"Test",dirPeriod); 
  if(oFile == NULL) {
    printf("Error during output file initialization\n"
	   "  Last errors are:\n%s",FrErrorGetHistory());
    return(0);}

  for(iFrame=0; iFrame<200; iFrame++) {

    frame = FrameHNew("new");
    frame->dt     = 16.;
    frame->GTimeS = 800000000 + iFrame * frame->dt;
 
    nData = 1000;
    sampleRate = nData/frame->dt;
    FrAdcDataNew(frame,"adc1", sampleRate, nData, 32);

    if(FrameWrite(frame, oFile) != FR_OK) {
      printf("Error during frame write\n"
	     "  Last errors are:\n%s",FrErrorGetHistory());
      return(0);}
          
    FrameFree(frame);}  /*--- end of the loop on frames */
	
  FrFileOEnd(oFile);

  return(0);
}
