/*-----------------------------------------------------------*/
/* File: exampleEvent.c        by B. Mours(LAPP) Feb 06, 2003*/
/*                                                           */
/*  This program creates a complex frame and write it on file*/
/*-----------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "FrameL.h"
  
int main()     /*-------------------- main-----------*/
{FrFile *iFile, *oFile;
 FrameH *frame;
 FrEvent *event, *evt;
 FrSimEvent *sEvent, *se;
 FrProcData *proc;
 FrVect *vect;
 double sampleRate, amp, phase, evtTime, m1, m2, tBefore, tAfter;
 long nData,i,j, t0;
 
    /*----------------------- Create a new frame ---------*/
    
  frame = FrameNew("demo");
  frame->dt = 2.;
  frame->GTimeS = 728729557;
  nData = 2000;
  sampleRate = nData/frame->dt;  
  tBefore = .03;
  tAfter  = .05;
  phase   = 0.;
 
  proc = FrProcDataNew(frame,"fastProc",sampleRate,nData,-32);
  
    /*---------------- open the output file with  compression --*/

  oFile = FrFileONewH("testEvent.gwf", 8, "exampleEvent"); 
  if(oFile == NULL)
     {printf("Cannot open output file\n");
      return(0);}
                           /*-------------------output several frames--*/
  for(i=0; i<50; i++)
     {printf(" output frame %d\n",frame->frame);
      frame->frame++;
      frame->GTimeS += frame->dt;

      for(j=0; j < nData; j++)
         {phase += .04;
          proc->data->dataF[j] = ((200.*rand())/RAND_MAX + 200.*cos(phase));}

      if(frame->GTimeS %6 == 1)
        {m1 = sin(frame->GTimeS)+1.5; /*----- add simulated event -----*/
 	 m2 = cos(frame->GTimeS)+1.5;
         amp = 1.e-20*(m1+m2);
         evtTime = frame->GTimeS+.5;
         sEvent = FrSimEventNew(frame,"Sim_Inspiral","random",NULL,
                  evtTime, tBefore, tAfter,amp,1,"M1",m1);
         FrVectZoomIn(proc->data,evtTime - frame->GTimeS - tBefore, 
                                 tBefore+tAfter);
         sEvent->data = FrVectCopy(proc->data);
         sEvent->data->startX[0] = -tBefore;
         FrVectZoomOut(proc->data);
         FrSimEventAddParam(sEvent, "M2", m2);
        
         amp = amp * (1+.1*rand()/RAND_MAX); /*------ add first event ---*/
         m1  = m1  * (1+.1*rand()/RAND_MAX);
         m2  = m2  * (1+.1*rand()/RAND_MAX);
         evtTime = evtTime + .01*rand()/RAND_MAX;
         FrVectZoomIn(proc->data,evtTime - frame->GTimeS - tBefore, 
                                                    tBefore+tAfter);
         vect = FrVectCopy(proc->data);
         vect->startX[0] = -tBefore;
         FrVectZoomOut(proc->data);
         event = FrEventNew(frame,"trigger_1","Binary search",proc->name,
           evtTime, tBefore, tAfter ,amp, .5,
                NULL, vect, 2, "M1", m1, "M2", m2);

         evtTime = evtTime + .01*rand()/RAND_MAX;  /*---- add second event--*/
         FrVectZoomIn(proc->data,evtTime- frame->GTimeS -tBefore, 
                                                 tBefore+tAfter);
         vect = FrVectCopy(proc->data);
         vect->startX[0] = -tBefore;
         FrVectZoomOut(proc->data);
         event = FrEventNew(frame,"trigger_2","Binary search",proc->name,
           evtTime, tBefore, tAfter ,amp, .5,
                NULL, vect, 2, "M1", m1, "M2", m2);

         event = FrEventCopy(event);    /*----- duplicate the second event ---*/
         FrEventAddParam(event, "chisquare",frame->GTimeS%8);
         event->amplitude = event->amplitude * (1+.1*rand()/RAND_MAX);
         FrameAddEvent(frame, event);}
  
     if(i < 8) FrameDump(frame, stdout,2);

     FrameWrite(frame, oFile);

     FrEventFree(frame->event);
     FrSimEventFree(frame->simEvent);
     frame->event  = NULL;
     frame->simEvent = NULL;}
  
  FrFileOEnd(oFile); /*----- close output file ----------------*/
 
  FrameFree(frame);
                        /*----------------- Now test random access -----*/

  iFile = FrFileINew("testEvent.gwf"); 
  if(iFile == NULL)
     {printf("Cannot open input file\n");
      return(0);}

  t0 = FrFileITStart(iFile);
  printf("\n-----> now call:FrEventReadT(iFile,""tr*"",t0,10.,0, 1.e30)"
              " + call FrEventReadData----\n");
  event = FrEventReadT(iFile,"tr*",t0,10.,0., 1.e30);
  FrEventReadData(iFile, event);
  for(evt=event;evt!=NULL; evt=evt->next) {FrEventDump(evt, stdout, 3);}
  FrEventFree(event);

  printf("\n-----> now call:FrEventReadT(iFile,\"*2\",t0,20.,0, 1.e30)\n");
  event = FrEventReadT(iFile,"*2",t0,20.,0., 1.e30);
  for(evt=event;evt!=NULL; evt=evt->next) {FrEventDump(evt, stdout, 3);}
  FrEventFree(event);

  printf("\n-----> now call:FrEventReadT(iFile,\"*2\",t0,50.,0, 2, "
         " ""M1"", 2., 3., ""M2"", 1., 3.))\n");
  event = FrEventReadTF(iFile,"*2",t0,50.,1, 2, "M1", 2., 3., 
                                                "M2", 1., 3.);

  for(evt=event;evt!=NULL; evt=evt->next) {FrEventDump(evt, stdout, 1);}
  FrEventFree(event);

  printf("\n-----> now call:FrSimEventReadT(iFile,\"*\",t0,50.,0., 1.)\n");
  sEvent = FrSimEventReadT(iFile,"*",t0,50.,0., 1.);
  for(se=sEvent;se!=NULL; se=se->next) {FrSimEventDump(se, stdout, 1);}
  FrSimEventFree(sEvent);

  printf("\n-----> now call:"
   "FrSimEventReadTF2(iFile,\"*\",t0,50.,1,\"M1\",.9,2.,\"M2\",1,2.4)\n");
  sEvent = FrSimEventReadTF2(iFile,"*",t0,50.,1,"M1",.9,2.,"M2",1,2.4);
  for(se=sEvent;se!=NULL; se=se->next) {FrSimEventDump(se, stdout, 1);}
  FrSimEventFree(sEvent);

  return(0);
}
