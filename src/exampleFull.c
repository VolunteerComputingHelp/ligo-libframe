/*-----------------------------------------------------------*/
/* File: exampleFull.c         by B. Mours(LAPP) May 04, 2009*/
/*                                                           */
/*  This program creates a complex frame and write it on file*/
/*-----------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "FrameL.h"
  
int main()     /*-------------------- main-----------*/
{FrFile *oFile;
 FrameH *frame;
 FrAdcData *adc;
 FrSimData *sim;
 FrTable *table;
 FrEvent *event;
 FrSimEvent *evt;
 FrFilter *filter;
 FrProcData *proc;
 FrVect *col, *gData;
 FrStatData *gain;
 double sampleRate, phase, slope;
 long nData,i,j,nADC,sec;
 char name[100];
 
    /*------ initialisation -------------------------------*/
 
  if(FrLibIni("full.lis",NULL, 2) == NULL)
     {printf("Error during initialisation\n"
             "  Last errors are:\n%s",FrErrorGetHistory());
      return(0);}

    /*----------------------- Create a new frame ---------*/
    
  frame = FrameNew("demo");
  if(frame == NULL)
     {printf("FrameNew failed (%s)",FrErrorGetHistory());
      return(0);}

   /*----- add fasts (2kHz) ADC with data provided 
                          by a random generator ------------*/

  sampleRate = 2000.;
  nData = 2000;
  frame->dt = nData/sampleRate;  
  nADC = 2;
 
  phase = 0;
  for(i=0; i<nADC; i++)
     {sprintf(name,"fastAdc%ld",i);
      slope =  20./65536;
      adc = FrAdcDataNewF(frame,name,"test",0,0,16,0.,slope,"Volts",sampleRate,nData);
      if(adc == NULL) return(0);
      for(j=0; j < nData; j++)
         {phase += .04;
          adc->data->dataS[j] = ((200.*rand())/RAND_MAX + 200.*cos(phase));}}

  /*----- add one process channel -----------*/

  proc = FrProcDataNew(frame,"fastProc",sampleRate,nData,-32);
  FrProcDataAddHistory(proc, "fastProc = 2 *fastAdc",0);
  FrProcDataAddParam  (proc, "gain", 1.5);
  printf(" ProcData gain=%f\n",FrProcDataGetParam(proc,"gain"));

  /*---- reserve space for slower (200 Hz) ADC's ------*/

  nData = nData / 10;
  sampleRate = sampleRate/10.;
  nADC = 4;
  for(i=0; i<nADC; i++)
     {sprintf(name,"Adc%ld",i);
      adc = FrAdcDataNew(frame,name,sampleRate,nData,16);

      for(j=0; j < adc->data->nData; j++)
         {adc->data->dataS[j] = (short) (rand());}}

  /*----- add one table -----------------------*/
 
  table = FrTableNew("table_1","slow monitoring",5,2,
               FR_VECT_STRING, "channel", FR_VECT_4R,"value");
  frame->rawData->firstTable = table;
     
  col = FrTableGetCol(table,"value");
  for(j=0; j < col->nData; j++) {col->dataF[j] = .2*cos(.1*j);}

    /*---------------- reserve space for various type of data --*/
  
  adc = FrAdcDataNew(frame,"Bad_data_(data[0]=nan)",sampleRate,nData,-32);
  for(j=0; j < nData; j++) {adc->data->dataF[j] = .1*j;}
  adc->data->dataUI[0] = 0x7f800000;            /*-- this is INF*/
  adc->data->dataUI[1] = 0x7f801000;            /*-- this is NAN*/
  adc->dataValid = 1;

  adc = FrAdcDataNew(frame,"D1",sampleRate,nData,-32);
  for(j=0; j < nData; j++) {adc->data->dataF[j] = sin(.01*j);}
  
  /*-- We build two sim1 channel to check the errors messages --*/

  sim = FrSimDataNew(frame,"sim1",sampleRate,nData,32);
  sim = FrSimDataNew(frame,"sim1",sampleRate,nData,32);
  for(j=0; j < nData; j++) {sim->data->dataI[j] = j;}
  
  FrSerDataNew(frame,"sms1",0,"sms data are here", 1.);
  FrMsgAdd(frame,"Test","Test message",0);
  FrSummaryNew(frame,"Quality_1","main quality"," ",NULL, NULL);
  FrSummaryNew(frame,"Quality_2","aux"," ",NULL, NULL);
  
  event =FrEventNew(frame,"trigger_1","Binary search","V0:Pr_B1_ACq",
           0.,1.,.2,1.e-20,.01,NULL, NULL, 2, "M1", 1.4, "M2", 1.3);
  FrEventAddParam(event, "chisquare",2.3);
  printf(" for    event:%s M2=%f\n",event->name,FrEventGetParam(event,"M2"));

  FrEventNew(frame,"trigger_1","Binary search","V0:Pr_B1_ACq",
           0.,1.,.2,2.e-20,.01,NULL,NULL, 2,"M1",234.,"M2", 0.123);
  FrEventNew(frame,"Burst_1","Burst algorithm 1234","V0:Pr_B1_ACp",
           frame->GTimeS+.2,5.,.3,2.e-20,.02,NULL,NULL,0);

  evt = FrSimEventNew(frame,"Sim_Inspiral_2",NULL,NULL,frame->GTimeS+.4,.2,1,1.e-21,0); 
  evt = FrSimEventNew(frame,"Sim_Inspiral",NULL,NULL,frame->GTimeS+3.,.2,1,1.e-21,0); 
  FrSimEventAddParam(evt, "M1", 1.4);
  FrSimEventAddParam(evt, "M2", 1.333);
  printf(" for simEvent:%s M2=%f\n",evt->name,FrSimEventGetParam(evt,"M2"));

  /*------- add some static data -----------*/

  gData = FrVectNew(FR_VECT_4R,1,10,1.,"relatif_gain");
  gain  = FrStatDataNew("gain","ADC 1 gain","calibration",
                        frame->GTimeS, frame->GTimeS+20, 0, gData, NULL);
  FrStatDataAdd(frame->detectProc, gain);

                     /*-------------------------------------create one filter*/

 filter = FrFilterNew("f1",1000.,1.,2, 1., 0., 2.34, 2.);
 FrFilterDump(filter, stdout, 2);
 FrameAddStatFilter(frame, frame->detectProc->name, "filter",
                    frame->GTimeS, 1999999999, 0, filter);

    /*---------------- open the output file with  compression --*/

  oFile = FrFileONewH("test.gwf", 5, "FrFull"); 
  if(oFile == NULL)
     {printf("Error during output file creation\n"
             "  Last errors are:\n%s",FrErrorGetHistory());
      return(0);}
  //  oFile->chkSumFrFlag = FR_NO;

    /*------ ---------------------------------output 10 frames--*/
    
  for(i=0; i<10; i++)
     {printf(" output frame %d\n",frame->frame);
 
      event = NULL;
      while((event = FrEventFind(frame,"trigger_1", event)) != NULL)
       {event->GTimeS = frame->GTimeS;
        event->GTimeN = 500000000+1000000*i;
        event->amplitude = 1.1*event->amplitude;}

      evt = FrSimEventFind(frame,"Sim_Inspiral", NULL);
      evt->GTimeS = frame->GTimeS;  
      evt->GTimeN = 6000000*i;  

      adc = FrAdcDataFind(frame,"fastAdc1");
      if(adc != NULL)
        {for(j=0; j < adc->data->nData; j++)
          {phase += .04;
           adc->data->dataS[j] = ((200.*rand())/RAND_MAX + 200.*cos(phase));
           proc->data->dataF[j] = 2.*adc->data->dataS[j];}}

      /*---------------------- dump on stdout the current frame--*/

      if(i < 2) FrameDump(frame, stdout,2);

      if(FrameWrite(frame, oFile) != FR_OK)
          {printf("Error during frame write\n"
             "  Last errors are:\n%s",FrErrorGetHistory());
           return(0);}

      frame->frame++;
      frame->GTimeS += (unsigned int) (frame->dt);
      frame->GTimeN += ((unsigned int) (1.e+9*frame->dt)) % 1000000000;
      sec = frame->GTimeN/1000000000;
      frame->GTimeS += sec;
      frame->GTimeN -= sec*1000000000;}
  
   /*------ try the tag function ---------------------*/
  
  printf("\n>>>>>>>>> Now dump frame with tag=sim* *1\n");
  FrameTag(frame, "sim* *1");
  FrameDump(frame, stdout, 2);

  printf("\n>>>>>>>>> Now all channels except trig channels \n");
  FrameTag(frame, "-trig*");
  FrameDump(frame, stdout, 2);

    /*----- close everything -------------------------*/

  FrFileOEnd(oFile);
 
  FrameFree(frame);

  return(0);
}
