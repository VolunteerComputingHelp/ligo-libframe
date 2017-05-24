/*-----------------------------------------------------------*/
/* File: exampleSpeed.c      by B.Mours.          Aug 09, 02 */
/*                                                           */
/*  This program read in memory one frame and then measure   */
/*  the in memory write/read speed.                          */
/*  The first parameter is the frame file name.              */
/*  The second parameters is the compression level.          */
/*  The third (optinal) parameter is the time for the loop   */
/*  Usage:  ./FrSpeed test.dat 6                             */
/*-----------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "FrameL.h"
  
int main(int argc, char *argv[])     /*-------- main---------*/
{FrameH *frame;
 FrFile *iFile;
 long  buffSize, i, j, size, size0, t0 ,compType,dt;
 char *buff;
 int tn, tprev, nread, nfree;
 double speedR, speedW;

 if(argc < 3)
      {printf(" You should provide a file name and compress level\n");
       exit(0);}

 sscanf(argv[2],"%ld",&compType);
 printf(" Compression level: %ld\n",compType);

 if(argc >3) 
      dt = atoi(argv[3]);
 else dt = 10;

    /*----- Read one from file -----------------------------*/
    
 iFile = FrFileINew(argv[1]);
 iFile->compress = -1;
 frame = FrameRead(iFile);
 if(frame == NULL) exit(0);
 FrFileIEnd(iFile);

 FrameDump(frame,stdout,1);   
 buffSize = 1.1*FrameStat(frame,stdout) + 10000;
 printf("  Estimated buffer size=%ld\n",buffSize);

 printf("\n Please wait %ld seconds\n",2*dt+1);
  
    /*--reserve space for the output internal buffer ------*/

 buff = malloc(buffSize);
 
   /*---- output many frames in the internal buffer --------*/
 
 while((size0 =  FrameWriteToBuf(frame, compType, buff, buffSize, 0)) == 0)
   {buffSize = 1.2*buffSize;
    buff = realloc(buff, buffSize);
    if(buff == NULL) return(0);}

 t0 = time(NULL);
 while(time(NULL) == t0) {;}

 i = 0;
 size = 0;
 while(time(NULL) < t0+dt+1)
   {i++;
    size = FrameWriteToBuf(frame, compType, buff, buffSize, 0);}

   /*---- input many frames from the internal buffer --------*/

 j = 0;
 tprev = time(NULL); 
 nread = nfree = 0;
 while(time(NULL) < t0+2*dt+1)
   {j++;
    tn = time(NULL);
    nread += tn-tprev;
    tprev = tn;
    FrameFree(frame);
    tn = time(NULL);
    nfree += tn-tprev;
    tprev = tn;
    frame = FrameReadFromBuf(buff, buffSize, 0);}

 printf(" Now a second FrameStat to check the frame\n");
 FrameStat(frame,stdout);

 speedR = (double)i*(double)size0/1024./1024./dt;
 speedW = (double)j*(double)size0/1024./1024./dt;
 printf(" Frame size =%ld Bytes (%ld uncompressed)\n",size,size0);
 printf("%8ld write  in memory. Speed=%.2fMB/s\n", i,speedR);
 printf("%8ld read from memory. Speed=%.2fMB/s", j,speedW);
 printf("    Time spend to read = %.0f%%, to free=%.0f%%\n",
           (100.*nread)/dt, (100.*nfree)/dt);
 printf("  (1 MB= 1024kB = 1024 Bytes)\n");

 return(0);
}
