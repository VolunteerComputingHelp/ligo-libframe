/*-----------------------------------------------------------*/
/* File: exampleMark.c         by B. Mours (LAPP) Oct  9, 00 */
/*                                                           */
/*  This program do a few random access in a file            */
/*-----------------------------------------------------------*/
#include <math.h>
#include "FrameL.h"
  
int main(int argc, char *argv[])   /*-------------- main ----*/
{FrFile *iFile;
 FrameH *frame;
 FrEvent *event;
 FrAdcData *adc;
 FrVect *vect;
 FrSimEvent *simEvt;
 double tstart, tend, t;
 int i;
 char *adcName = "fastAdc1";
 
             /*-----------------Open input file---------------*/

 iFile = FrFileINew("test.gwf");
 if(iFile == NULL)
     {printf("Cannot open file\n");
      return(0);}

             /*----- Get the start/end time for this file----*/

 tstart = FrFileITStart(iFile);
 tend   = FrFileITEnd(iFile);
 t = .5*(tstart + tend);
 printf(" Start time= %.1f\n End   time=%.1f used time=%.1f\n",
          tstart, tend, t);

             /*--------------- test event random access -----*/
  
 event = FrEventReadT(iFile,"*",t,10., 0., 1.e20);
 for(;event != NULL; event = event->next)
      {FrEventDump(event, stdout,2);}

             /*------------ test simEvent random access -----*/
  
 simEvt = FrSimEventReadT(iFile,"*",t,2., 0. , 1.e30);
 for(; simEvt != NULL; simEvt = simEvt->next)
      {FrSimEventDump(simEvt, stdout,2);}

             /*--- we loop many time to check for memory leak -*/

 FrLibSetLvl(0);
 for(i=0; i<99999999; i++)
   {
             /*---------- get a frame according it's time ----*/
        
    frame = FrameReadT(iFile, t);
    if(i<1) FrameDump(frame, stdout, 2);
    FrameFree(frame);
     
             /*------------ Read only one ADC -------*/

    if(i<1) printf("\nSearch for an ADC: %s\n",adcName);
    adc = FrAdcDataReadT(iFile, adcName, t);
    if((adc != NULL) && (i<1))
       {FrVectDump( adc->data, stdout, 3);}
    if(adc != NULL) FrAdcDataFree(adc);

             /*---- Get on ADC vector across several frames--------*/
 
    if(i<1) printf("\nSearch for the ADC vector: %s\n",adcName);
    vect = FrFileIGetV(iFile, adcName, tstart+1.1,5.);
    if(vect != NULL)
       {if(i < 1) FrVectDump(vect, stdout, 3);
        FrVectFree(vect);}
 
             /*-------- read only the frame header ------------*/
 
    if(i<1) printf("\nSearch for the frame header\n");
    frame = FrameHReadT(iFile,  t-1);
    if(i<1) FrameDump(frame, stdout, 2);
    FrameFree(frame);
 
    if(i%100 == 0) printf("%6d Read preformed \n",i+1);}
    
 return(0);
}



