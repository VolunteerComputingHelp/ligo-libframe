/*-----------------------------------------------------------*/
/* File: exampleMultiW.c      by B. Mours (LAPP) Jul  6, 01  */
/*                                                           */
/*   This  program creates a frame and writes it in          */  
/*   file for several files. There are several frames        */
/*   par files (see the value of framePerFile).              */ 
/*   This is usefull to check the memory leaks....           */
/* The argument is the number of loop (default 1)            */
/*-----------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "FrameL.h"
  
int main(int argc, char **argv)  /*--------- main------------*/
{FrFile *oFile;
 FrameH *frame;
 double sampleRate;
 FrAdcData *adc;
 FrSimData *sim;
 FrProcData *proc1, *proc2, *proc3;
 long nData, framePerFile, i, iFile, iFrame, nFrame, compress;
 char fileName[100];
 
    /*------ initialisation --------------------------------*/
 
 framePerFile = 10;
 iFile = 0;
 oFile = NULL;

 if(argc == 1) 
      nFrame = framePerFile*10;
 else nFrame = framePerFile*10*atoi(argv[1]);

 for(iFrame=0; iFrame<nFrame; iFrame++)
   {

    /*---frame 72 is missing just for random access test ---*/

    if(iFrame == 72) continue;

    /*----- create one Frame header (frame size = 1.1s)-----*/
    
    frame = FrameHNew("new");
    if(frame == NULL)
       {printf("Error during frame creation\n"
             "  Last errors are:\n%s",FrErrorGetHistory());
        return(0);}
    frame->frame     = iFrame;
    frame->GTimeS    = 666000000+iFrame+iFrame/10;
    frame->GTimeN    = iFrame%10*100000000;
    frame->dt        = 1.1;
    frame->ULeapS    = 32;
 
     /*----- add ADC, sim and proc channels ---------------*/

    nData = 110;
    sampleRate = nData/frame->dt;

    adc = FrAdcDataNewF(frame,"adc1","a comment",
           10,222,16,.1,.00305,"Volts",sampleRate,nData);  
    for(i=0; i < nData; i++) {adc->data->dataS[i] = (short) (i+nData*iFrame);}

    adc = FrAdcDataNewF(frame,"adc2","a comment",
           10,223,16,.1,.00305,"Volts",sampleRate,nData);  
    for(i=0; i < nData; i++) {adc->data->dataS[i] = (short)(i+nData*iFrame)/2;}

    sim = FrSimDataNew(frame,"sim1",sampleRate,nData,32);
    for(i=0; i < nData; i++) {sim->data->dataI[i] = (short) (i+nData*iFrame);}

    proc1 = FrProcDataNew(frame,"proc1",sampleRate,nData,-32);
    for(i=0; i < nData; i++) {proc1->data->dataF[i] = (short)(i+nData*iFrame);}
    FrProcDataAddHistory(proc1, "Just a test channel",0);

    proc2 = FrProcDataNew(frame,"proc2",sampleRate,nData,-32);
    for(i=0; i < nData; i++) {proc2->data->dataF[i] = 2*proc1->data->dataF[i];}
    FrProcDataAddHistory(proc2, "proc2=2*proc1",1, proc1->history);
    proc2->type = 1; /* this is a time serie */

    proc3 = FrProcDataNew(frame,"FFT",sampleRate,nData,-32);
    for(i=0; i < nData; i++) {proc3->data->dataF[i] = 1./(1.+i);}
    proc3->type = 2; /* this is an frequency domain */

    FrSerDataNew (frame, "tower", frame->GTimeS+1, "P 3.63", 1.);
    FrSummaryNew(frame,"Quality_1","main quality"," ",NULL, NULL);

    /*----- add triger data ----------------------------*/

    FrEventNew(frame,"trigger_1","Binary search","V0:Pr_B1_ACq",
        frame->GTimeS+1.e-9*frame->GTimeN+.2,1.,.2,
             iFrame*1.e-20, .01,"value/rms",NULL, 2, "m1", 1.4, "m2", 1.33);

    FrSimEventNew(frame,"simEvent","A burst event",NULL,
		  frame->GTimeS+1.e-9*frame->GTimeN + .2, .1, .2, 
                    1.e-22, 1,"distance", 1.23);

   /*----- dump one frame on stdout---------------------*/

    if(iFrame < 2) FrameDump(frame, stdout, 2);
    
    /*--open the output file (we use only 10 file names)-*/

    if(iFrame%framePerFile == 0)
      {iFile ++;
       sprintf(fileName,"file%ld.gwf",iFile%10);
       if(iFrame%500 == 0) 
           printf("open file %s at frame %ld \n",fileName,iFrame);

       compress = 0; /*------ mix files with different compression values---*/
       if(iFrame%2 == 0) compress = 5;

       oFile = FrFileONew(fileName, compress); 
       if(oFile == NULL)
          {printf("Error during output file initialization\n"
               "  Last errors are:\n%s",FrErrorGetHistory());
	  return(0);}}

    if(FrameWrite(frame, oFile) != FR_OK)
       {printf("Error during frame write\n"
             "  Last errors are:\n%s",FrErrorGetHistory());
        return(0);}

     /*----- close everything -------------------------*/
     
    if(iFrame%framePerFile == framePerFile-1)
      {FrFileOEnd(oFile);}
                                
    FrameFree(frame);

   }  /*--- end of the loop on frames */
  return(0);
}
