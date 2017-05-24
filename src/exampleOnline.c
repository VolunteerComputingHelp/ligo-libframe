/*-----------------------------------------------------------*/
/* File: exampleOnline.c      by B.Mours (LAPP)   Aug 23, 99 */
/*                                                           */
/*  This program creates and write in memory several frames  */
/*  The compression level is the parameter. This program     */
/*  could be used to measure the frame output speed.         */
/*-----------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "FrameL.h"
  
int main(int argc, char *argv[])     /*-------- main---------*/
{struct FrameH *frame;
 struct FrAdcData *adc;
 double sampleRate, rate;
 long nData,buffSize, nAdc, i, j, size, rawSize, t1, t0 ,compType;
 char *buff,name[100];

 if(argc > 1)
      {sscanf(argv[1],"%ld",&compType);}
 else {compType = 6;}
 printf(" Compression level: %ld\n",compType);

 FrLibSetLvl(0);

    /*----- create one Frame header ------------------------*/
    
 frame = FrameHNew("new");
 if(frame == NULL)
     {printf("Error during frame creation\n"
             "  Last errors are:\n%s",FrErrorGetHistory());
      return(0);}

   /*-- add 40 fast ADC with random data of various rms ---*/

 nAdc  = 40;
 nData = 65536; 
 sampleRate = 1.e+4;

 for(i=0; i<nAdc; i++)
     {sprintf(name,"fastAdc%ld",i);
      adc = FrAdcDataNew(frame,name,sampleRate,nData,16);
      if(adc == NULL)
         {printf("Error during adc creation\n"
             "  Last errors are:\n%s",FrErrorGetHistory());
          return(0);}

      for(j=0; j < nData; j++)
         {adc->data->dataS[j] = ((short)rand())/((i+1)*200.);}}

 FrameDump(frame,stdout,2);   
  
    /*--reserve space for the output internal buffer ------*/

 buffSize = nAdc*nData*2 + 500000;
 buff = malloc(buffSize);

    /*-- output frame in the internal buffer whitout 
                  compression to get the full size ---------*/

 rawSize = FrameWriteToBuf(frame, 0, buff, buffSize, 0);
 printf(" Raw frame size = %5f MBytes\n", rawSize/1024./1024.);
 
   /*---- output 100000 frames in the internal buffer (they are 
       identical except for the frame number)---------------*/
  
 t0 = time(NULL);
 for(i=0; i<100000; i++)
    {frame->frame++;
     size = FrameWriteToBuf(frame, compType, buff, buffSize, 0);

     if(i%5==0) 
       {t1 = time(NULL);
        if(t1 != t0) 
             {rate = 1.e-6*rawSize*(i+1)/(t1-t0);}
        else {rate = 0.;}
        printf(" frame %ld, frame size = %5f MBytes; rate = %f MBytes/s\n",
             i, size/1024./1024.,rate);}}
                                
  return(0);
}
