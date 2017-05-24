/*-----------------------------------------------------------*/
/* File: exampleStat.c      by B. Mours(LAPP) Oct 6, 2000    */
/*                                                           */
/*  This program produces frames with static data.           */
/*-----------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "FrameL.h"
  
int main()     /*-------------------- main-------------------*/
{FrFile *oFile;
 FrameH *frame;
 FrAdcData *adc1;
 FrVect *gData;
 FrStatData *gain;
 double sampleRate, phase;
 long nData,i,j,k;
 unsigned int t0,dtS,dtN,sec;
 
    /*----- create one Frame header and detector structure--*/
    
  frame = FrameNew("Test data");
  if(frame == NULL)
     {printf("FrameNew failed (%s)",FrErrorGetHistory());
      return(0);}

  FrHistoryAdd(frame,"created by a test program");

   /*----- add 1 fast ADC with data provided by a 
           random generator or a sine wave --*/

  sampleRate = 1000.;
  nData = 1000;
  frame->dt = nData/sampleRate;  
  dtS = (unsigned int) (frame->dt);
  dtN = ((unsigned int) (1.e+9*frame->dt)) % 1000000000;

  adc1 = FrAdcDataNew(frame,"adc_1",sampleRate,nData,16);
  phase = 0.1532;

   /*------- add static data -----------------------------*/
   /*--(in this case the ADC gain for each channel)-------*/

  t0 = frame->GTimeS;
  gData = FrVectNew(FR_VECT_4R,1,nData,1.,"relatif_gain");
  gain  = FrStatDataNew("gain","ADC 1 gain","calibration",
                               t0, t0+20, 0, gData, NULL);
  FrStatDataAdd(frame->detectProc, gain);

  for(i=0; i<2; i++) 
       {gData->dataF[i] = 1.1;}

    /*----- open the output file -------------------------*/
                              
  oFile = FrFileONew("test.gwf", 0); 

    /*-------generate and output 10 frames  --------------*/
    
  for(i=0; i<100; i++)
     {
       /*---- we first update the frame data -------------*/

      frame->frame++;
      frame->GTimeS += dtS;
      frame->GTimeN += dtN;
      sec = frame->GTimeN/1000000000;
      frame->GTimeS += sec;
      frame->GTimeN -= sec*1000000000;

      for(j=0; j < nData; j++)
        {phase = phase + 1.934e-3;
         adc1->data->dataS[j] = (short)(rand()+2000.*sin(phase));}

        /*---- we simulate de generation of new calibration data:
               we change the calibration constant every 10 seconds.  --*/

      if(frame->GTimeS > gain->timeEnd)
         {gain->timeStart = gain->timeEnd;
          gain->timeEnd   = gain->timeStart + 20;
          FrStatDataTouch(gain);
          for(k=0; k<nData; k++)
             {gData->dataF[k] += .0002;}}

    /*----- dump on stdout the current frame  -----------*/

     if(i < 5) FrameDump(frame, stdout,2);

        /*--- then we write the data on file ------------*/

      printf(" output frame %d\n",frame->frame);

      if(FrameWrite(frame, oFile) != FR_OK)
          {printf("Error during frame write\n"
             "  Last errors are:\n%s",FrErrorGetHistory());
           return(0);}
      }

    /*----- close everything ----------------------------*/
     
  FrFileOEnd(oFile);

  return(0);
}
