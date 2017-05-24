/*---------------------------------------------------------------------*/
/* File: exampleMultiTOC.c                      by B. Mours Aug 09, 02 */
/*                                                                     */
/*  This program test all the random access function.                  */
/*  This program loop several times to check for memory leak.          */
/*  The program FrMultiW need to be run before to generate the file.   */
/*---------------------------------------------------------------------*/
#include <math.h>
#include "FrameL.h"
  
int main(int argc, char *argv[])   /*-------------- main ----*/
{FrFile *iFile;
 FrameH *frame;
 FrEvent *event;
 FrAdcData *adc;
 FrVect *vect;
 FrSimEvent *sEvt;
 FrSerData *ser;
 FrProcData *proc;
 FrSimData *sim;
 FrSummary *sum;
 double tStart, len, tEnd, t;
 int iLoop;
 FILE *fp;

 for(iLoop = 0; iLoop < 10000; iLoop ++) {
   if(iLoop < 10 || (iLoop%10) == 0) printf("start loop %d\n",iLoop);

             /*-----------------Open input file---------------*/

   iFile = FrFileINew("file4.gwf file5.gwf file7.gwf file8.gwf");
   if(iFile == NULL)
     {printf("Cannot open file\n");
      return(0);}

   if(iLoop < 3) printf("---------------------- File Dump level 1\n");
   if(iLoop < 3) FrFileIDump(iFile, stdout, 1, NULL);

   if(iLoop < 3) printf("---------------------- File Dump level 2\n");
   if(iLoop < 3) FrFileIDump(iFile, stdout, 2, NULL);

   if(iLoop <3) printf("---------------------- Create Frame File List\n");
   fp = fopen("test.ffl","w");
   FrFileIDump(iFile, fp, 1, NULL);
   fclose(fp);
   FrFileIEnd(iFile);

   iFile = FrFileINew("test.ffl");
   if(iLoop < 3)FrFileIDump(iFile, stdout, 1, NULL);

   if(iLoop == 0) printf("---------------------- Extract one sim vector\n");
   tStart = 666000048;
   len = 11;
   vect = FrFileIGetV    (iFile, "sim1", tStart, len);
   if(iLoop == 0) FrVectDump(vect, stdout, 2);
   FrVectFree(vect);

   if(iLoop == 0) printf("----------- Extract and decimate one adc vector\n");
   vect = FrFileIGetVAdc (iFile, "adc1", tStart, len, 2);
   if(iLoop == 0) FrVectDump(vect, stdout, 3);
   FrVectFree(vect);

   if(iLoop == 0) printf("----------- Extract and decimate one sim vector\n");
   vect = FrFileIGetVSim (iFile, "sim1", tStart, len, 5);
   if(iLoop == 0) FrVectDump(vect, stdout, 3);
   FrVectFree(vect);

   if(iLoop == 0) printf("----------- Try to Extract a non existing vector\n");
   vect = FrFileIGetVSim (iFile, "proc1", tStart, len, 5);
   if(iLoop == 0) FrVectDump(vect, stdout, 3);
   FrVectFree(vect);

   if(iLoop == 0) printf("----------- Extract and decimate one proc vector\n");
   vect = FrFileIGetVProc(iFile, "proc1", tStart, len, 5);
   if(iLoop == 0) FrVectDump(vect, stdout, 3);
   FrVectFree(vect);

   if(iLoop == 0) printf("---------------------- Extract all event "
                        "tStart=%.2f len=%.2f\n",tStart, len);
   event = FrEventReadT(iFile,"trigger_1", tStart, len, 0., 1.e20);
   if(iLoop == 0) 
     {for(;event != NULL; event=event->next) {FrEventDump(event, stdout, 3);}}
   FrEventFree(event);

   if(iLoop == 0) printf("--------Extract all event (amplitude [0;47.5e-20])"
                        "tStart=%.2f len=%.2f\n",tStart, len);
   event = FrEventReadT(iFile,"*", tStart, len, 0., 47.5e-20);
   if(iLoop == 0) 
     {for(;event != NULL; event=event->next) {FrEventDump(event, stdout, 3);}}
   FrEventFree(event);

   tStart -= 10;
   if(iLoop == 0) printf("----------- Extract simulated events "
                        "tStart=%.2f len-=%.2f\n",tStart, len);
   sEvt = FrSimEventReadT(iFile,"simEvent", tStart, len, 0., 1.e10);
   if(iLoop == 0) 
     {for(;sEvt != NULL; sEvt=sEvt->next) {FrSimEventDump(sEvt, stdout, 3);}}
   FrSimEventFree(sEvt);

   tStart = FrFileITStart(iFile);
   tEnd   = FrFileITEnd(iFile);
   if(iLoop < 3) printf("------------ Start/End time=%.2f %.2f\n",tStart,tEnd);

   t = tStart + .9*(tEnd-tStart);
   if(iLoop == 0) printf("--------------Extract full frame for t=%.2f\n",t);
   frame = FrameReadT(iFile, t);
   if(iLoop == 0) FrameDump(frame, stdout, 2);
   FrameFree(frame);

   t = tStart + .85*(tEnd-tStart);
   if(iLoop == 0) printf("-------Extract frame with adc1 only for t=%.2f\n",t);
   frame = FrameReadTAdc(iFile, t, "adc1");
   if(iLoop == 0) FrameDump(frame, stdout, 2);
   FrameFree(frame);

   t = tStart + .75*(tEnd-tStart);
   if(iLoop == 0) printf("-------Extract frame with adc1 only for t=%.2f\n",t);
   frame = FrameReadTAdc(iFile, t, "adc1");
   if(iLoop == 0) FrameDump(frame, stdout, 2);
   FrameFree(frame);

   t = tStart + .7*(tEnd-tStart);
   if(iLoop == 0) printf("--------------Extract adc channel for t=%.2f\n",t);
   adc = FrAdcDataReadT(iFile, "adc*",t);
   if(iLoop == 0) FrAdcDataDump(adc, stdout, 2);
   FrAdcDataFree(adc);

   t = tStart + .7*(tEnd-tStart);
   if(iLoop == 0) printf("--------------Extract ser channel for t=%.2f\n",t);
   ser = FrSerDataReadT(iFile, "t*",t);
   if(iLoop == 0) FrSerDataDump(ser, stdout, 2);
   FrSerDataFree(ser);

   t = tStart + .65*(tEnd-tStart);
   if(iLoop == 0) printf("--------------Extract proc channel for t=%.2f\n",t);
   proc = FrProcDataReadT(iFile, "p*",t);
   if(iLoop == 0) FrProcDataDump(proc, stdout, 2);
   FrProcDataFree(proc);

   t = tStart + .65*(tEnd-tStart);
   if(iLoop == 0) printf("--------------Extract sim channel for t=%.2f\n",t);
   sim = FrSimDataReadT(iFile, "s*",t);
   if(iLoop == 0) FrSimDataDump(sim, stdout, 2);
   FrSimDataFree(sim);

   t = tStart + .6*(tEnd-tStart);
   if(iLoop == 0) printf("--------------Extract frame header for t=%.2f\n",t);
   frame = FrameHReadT(iFile, t);
   if(iLoop == 0) FrameDump(frame, stdout, 2);
   FrameFree(frame);

   t = tStart + .2*(tEnd-tStart);
   if(iLoop == 0) printf("--------------Extract summary info for t=%.2f\n",t);
   sum = FrSummaryReadT(iFile, "*", t);
   if(iLoop == 0) FrSummaryDump(sum, stdout, 2);
   FrSummaryFree(sum);

   FrFileIEnd(iFile);

  }
 return(0);
}



