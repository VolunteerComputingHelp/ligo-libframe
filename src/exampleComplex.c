/*-----------------------------------------------------------*/
/*This program creates a complex vector and put it on a frame*/
/*-----------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "FrameL.h"
  
int main()     /*-------------------- main-----------*/
{FrFile *oFile;
 FrameH *frame;
 FrProcData *proc;
 FrVect *vect;
 double sampleRate, dt, omega, vr, vi;
 long nData,i;
 
 /*------- Create a vector of complex numbers (double) ----------------*/
 
 nData = 100;
 dt    = 1./nData;
 vect = FrVectNew1D("a complex vector",FR_VECT_16C, nData, dt, "time[s]","A.U.");
 
 /*------- fill the complex vector with exp(i*omega*t); So we just have
  a cosine for the real part and a sine for the imaginary part ---------*/
 
 omega = 100*6.14;
 for(i=0; i<vect->nData; i++)
  {vect->dataD[2*i  ] = cos(omega*i*dt);
   vect->dataD[2*i+1] = sin(omega*i*dt);}
   
 /*------------- some example of debug and data access ----------------*/
 
  FrVectDump(vect, stdout, 3);  /*---- to see the result on the screen--*/
  i = FrVectGetIndex(vect, .1); /* get index for sampling at t=0.1s-----*/
  vr = vect->dataD[2*i];        /* get real part */
  vi = vect->dataD[2*i+1];      /* get imaginary part */
  printf("The sampling %ld is at t=0.1s and has real part=%g imag. part=%g\n",
        i, vr, vi);
  /*----------------------- Create a new frame ------------------------*/
    
  frame = FrameNew("demo");
  frame->dt = 1.;
   
  /*----- attach the vector to a process channel; this is a little bit
     trick since the proc FrProcDataNew do not create directly complex
     vector. So a dummy vector is created and then replaced by the 
     complex vector----------------------------------------------------*/

  sampleRate = nData/frame->dt;
  proc = FrProcDataNew(frame,"fastProc",sampleRate,1,-64);
  FrVectFree(proc->data);
  proc->data = vect;
  
  /*--- store additional (optional) information in the procData struct*/
  
  proc->type   = 1;       /* declare it as a time serie---------------*/
  proc->fShift = 100.;    /* store the heterodyning frequency --------*/
  proc->phase  = 0.;      /* store the hetero. phase at vect. origin -*/
  FrProcDataAddParam  (proc, "gain", 1.5); /*- add one extra parameter*/
  printf(" ProcData gain=%f\n",FrProcDataGetParam(proc,"gain"));
  
  FrameDump(frame, stdout, 3);
    /*---------------------------------------- store frame on file --*/

  oFile = FrFileONewH("test.gwf", 8, "test program"); 
  if(oFile == NULL)
     {printf("Error during output file creation\n"
             "  Last errors are:\n%s",FrErrorGetHistory());
      return(0);}

  FrameWrite(frame, oFile);
 
  FrFileOEnd(oFile);
 
  return(0);
}
