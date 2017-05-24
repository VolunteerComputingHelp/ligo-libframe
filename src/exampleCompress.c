/*-------------------------------------------------------------*/
/* File: exampleCompress.c       by B. Mours(LAPP) Mar 26, 10  */
/*  This program creates a frame, write and read it with       */ 
/*  various compression level                                  */
/*                                                             */
/* In order to check compression/decompression across platforms*/
/* this program check the files comp*.dat available in the     */
/* current directory if one argument is provided.              */
/*-------------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "FrameL.h"
  
int main(int narg, char **arcv)     /*-------------------- main-----------*/
{FrFile *oFile, *iFile;
 FrameH *frame;
 FrAdcData *adc0,*adc1,*adc2,*adc3,*adc4,*adc5,*adc6,*adc7,*adc8, *adc9;
 FrProcData *proc1, *proc2;
 FrVect *vect;
 double sampleRate;
 int i, compType, nBytes;
#define NDATA 2000
 short data[NDATA];
 float dataF1[NDATA],dataF2[NDATA];
 char dataC1[NDATA], dataC2[NDATA];
 char fname[64];

 FrLibSetLvl(2);

    /*----- create one Frame header with 8 adc channels-------*/
    
  frame = FrameHNew("demo");

  sampleRate = 2000.;
  frame->dt = NDATA/sampleRate;  

  adc0 = FrAdcDataNew(frame,"adc0",sampleRate,1,-32);
  adc0->data->dataF[0] = 1.32/71.;

  adc1 = FrAdcDataNew(frame,"adc1",sampleRate,NDATA,8);
  adc2 = FrAdcDataNew(frame,"adc2",sampleRate,NDATA,8);
  adc3 = FrAdcDataNew(frame,"adc3",sampleRate,NDATA,16);
  adc4 = FrAdcDataNew(frame,"adc4",sampleRate,NDATA,16);
  adc5 = FrAdcDataNew(frame,"adc5",sampleRate,NDATA,24);
  adc6 = FrAdcDataNew(frame,"adc6",sampleRate,NDATA,24);
  adc7 = FrAdcDataNew(frame,"adc7",sampleRate,NDATA,-32);
  adc8 = FrAdcDataNew(frame,"adc8",sampleRate,NDATA,-32);
  adc9 = FrAdcDataNew(frame,"adc9",sampleRate,NDATA,-64);

  vect = FrVectNewL(FR_VECT_8C, 1,NDATA, 1./sampleRate, "time");
  FrVectSetName(vect,"proc1");
  proc1 = FrProcDataNewV(frame, vect);
  vect  =FrVectNewL(FR_VECT_16C,1,NDATA, 1./sampleRate, "time");
  FrVectSetName(vect,"proc2");
  proc2 = FrProcDataNewV(frame, vect);

  for(i=0; i < NDATA; i++)
    {data  [i] = rand()>>17;
     dataC1[i] = data[i]/1000;
     dataC2[i] = i;
     dataF1[i] = data[i] * 0.001;
     dataF2[i] = i * 0.001;

     adc1->data->data [i] = dataC1[i];
     adc2->data->data [i] = dataC2[i];
     adc3->data->dataS[i] = data[i];
     adc4->data->dataS[i] = i;
     adc5->data->dataI[i] = data[i];
     adc6->data->dataI[i] = i;
     adc7->data->dataF[i] = dataF1[i];
     adc8->data->dataF[i] = dataF2[i];
     adc9->data->dataD[i] = dataF2[i];
     proc1->data->dataF[2*i]   = dataF1[i];
     proc1->data->dataF[2*i+1] = dataF2[i];
     proc2->data->dataD[2*i]   = dataF1[i];
     proc2->data->dataD[2*i+1] = dataF2[i];}

    /*--------- special test case (bug in version 4.10)-----*/

  data[0] = -32768;
  adc3->data->dataS[0] = data[0];
  adc5->data->dataI[0] = data[0];


    /*----- dump on stdout the current frame  ----------*/

  FrameDump(frame, stdout,2);

    /*----- open the output file in binary format 
            with differentiation and compression ------*/

  for(compType=0; compType<10; compType++)
    {if(compType == 2) continue;
     if(compType == 6) continue;
     if(compType == 7) continue;
     printf("\n ----------------------- compression = %d\n",compType);
     sprintf(fname,"comp%d.dat",compType);

     if(narg == 1)
       {printf("write file %s\n",fname);
        oFile = FrFileONew(fname, compType); 
        if(oFile == NULL)
            {printf("Error during output file creation\n"
                "  Last errors are:\n%s",FrErrorGetHistory());
             return(0);}

         if(FrameWrite(frame, oFile) != FR_OK)
             {printf("Error during frame write\n"
                "  Last errors are:\n%s",FrErrorGetHistory());
              return(0);}
     
	 nBytes= FrFileOEnd(oFile);
         printf(" nBytes=%d\n",nBytes);}
     else
       {printf("do not write file; read existing file\n");}

     FrameFree(frame);
  
     printf(" -------------------------now read the file ...\n");

     iFile = FrFileINew(fname);
     if(iFile == NULL)
        {printf("Cannot open file");
         return(0);}

     frame = FrameRead(iFile);
     if(frame == NULL)
       {printf(" FrameRead failed\n");
        exit(0);}

     /*FrameDump(frame, stdout, 2);*/

           /*----------- check data------------------*/

     adc1 = FrAdcDataFind(frame, "adc1");
     adc2 = FrAdcDataFind(frame, "adc2");
     adc3 = FrAdcDataFind(frame, "adc3");
     adc4 = FrAdcDataFind(frame, "adc4");
     adc5 = FrAdcDataFind(frame, "adc5");
     adc6 = FrAdcDataFind(frame, "adc6");
     adc7 = FrAdcDataFind(frame, "adc7");
     adc8 = FrAdcDataFind(frame, "adc8");
     adc9 = FrAdcDataFind(frame, "adc9");
     proc1 = FrProcDataFind(frame, "proc1");
     proc2 = FrProcDataFind(frame, "proc2");

     for(i=0; i < NDATA; i++)
       {if(adc1->data->data [i] != dataC1[i]) {printf("\n>>>Error:adc1"
           " i=%d adc=%d ran=%d\n",i,adc1->data->data [i],dataC1[i]);
	    exit(1);}
        if(adc2->data->data [i] != dataC2[i]) {printf("\n>>>Error:adc2"
           " i=%d adc=%d dataC2=%d\n",i,adc2->data->data[i],dataC2[i]);
	    exit(1);}
        if(adc3->data->dataS[i] != data[i]) {printf("\n>>>Error:adc3"
           " i=%d adcS=%d ran=%d\n",i,adc3->data->dataS[i],data[i]);
            exit(1);}
        if(adc4->data->dataS[i] != i) {printf("\n>>>Error:adc4"
           " i=%d adcS=%d \n",i,adc4->data->dataS[i]);
	    exit(1);}
        if(adc5->data->dataI[i] != data[i]) {printf("\n>>>Error:adc5"
           " i=%d adcI=%d ran=%d\n",i,adc5->data->dataI[i],data[i]);
            exit(1);}
        if(adc6->data->dataI[i] != i) {printf("\n>>>Error:adc6"
           " i=%d adcI=%d \n",i,adc6->data->dataI[i]);
            exit(1);}
        if(adc7->data->dataF[i] != dataF1[i]) {printf("\n>>>Error:adc7"
           " i=%d adcF=%e ran=%e\n",i,adc7->data->dataF[i],dataF1[i]);
            exit(1);}
        if(adc8->data->dataF[i] != dataF2[i]) {printf("\n>>>Error:adc8"
           " i=%d adcF=%e dataF2=%e\n",i,adc8->data->dataF[i],dataF2[i]);
            exit(1);}
        if(adc9->data->dataD[i] != dataF2[i]) {printf("\n>>>Error:adc9"
           " i=%d adcF=%le dataF2=%e\n",i,adc9->data->dataD[i],dataF2[i]);
            exit(1);}
        if(proc1->data->dataF[2*i] != dataF1[i]) {
          printf("\n>>>Error:proc1 real part i=%d proc=%e dataF1=%e\n",
		 i,proc1->data->dataF[2*i],dataF2[i]);
 	  exit(1);}
	        if(proc1->data->dataF[2*i+1] != dataF2[i]) {
          printf("\n>>>Error:proc1 imag. part i=%d proc=%e dataF2=%e\n",
		 i,proc1->data->dataF[2*i+1],dataF2[i]);
 	  exit(1);}
        if(proc2->data->dataD[2*i] != dataF1[i]) {
          printf("\n>>>Error:proc1 real part i=%d proc=%e dataF1=%e\n",
		 i,proc2->data->dataD[2*i],dataF2[i]);
 	  exit(1);}
        if(proc2->data->dataD[2*i+1] != dataF2[i]) {
          printf("\n>>>Error:proc1 imag. part i=%d proc=%e dataF2=%e\n",
		 i,proc2->data->dataD[2*i+1],dataF2[i]);
		 exit(1);}}
      
     FrFileIEnd(iFile);}

  printf("\n>>>>>>>>>>>> Compression test OK <<<<<<<<<<<<<<\n");

  return(0);
}
