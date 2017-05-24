/*---------------------------------------------------------------------*/
/* FrExtract.c             by B. Mours LAPP   Oct 22, 2002             */
/*   && Increase file name length - K. Thorne - Sept 19, 2006 &&       */ 
/*                                                                     */
/* This Matlab mex file extract from a frame file the data for         */ 
/*   one ADC channel (this is for Matlab version 5)                    */
/*                                                                     */
/* The input arguments are:                                            */
/*   1) file name(s). This could be a single file, a list of file      */
/*                    separated by space or a frame file list (ffl)    */
/*   2) ADC or PROC name                                               */
/*   3) (optional) first frame (default=0 first frame in the file)     */
/*   4) (optional) number of frames to read (default = Frame)          */
/*   5) (optional) debug level (default = 0 = no output)               */
/*                                                                     */
/* Returned matlab data:                                               */
/*   1) ADC or PROC data (time series)                                 */
/*   2) (optional) x axis values relative to the first data point.     */
/*            This is usual time but it could be frequency in the case */
/*            of a frequency series (double)                           */
/*   3) (optional) frequency values in the case of time series (double)*/
/*   4) (optional) GPS starting time (in second.nanosec)               */
/*   5) (optional) starting time as a string                           */
/*   6) (optional) ADC comment as a string                             */
/*   7) (optional) ADC unit as a string                                */
/*   8) (optional) additional info: it is a 9 words vector which       */
/*       content:crate #, channel#, nBits#, biais, slope,              */
/*               sampleRate, timeOffset(S.N), fShift, overRange        */
/*      All this values are stored as double                           */
/*---------------------------------------------------------------------*/
#include "mex.h"
#include "FrameL.h"

#define MAX_FILE_NAME 1024
#define MAX_VECT_NAME 100

void  mexFunction( int nlhs,       mxArray *plhs[],
                   int nrhs, const mxArray *prhs[])
{FrFile *iFile;
 FrameH *frame;
 FrAdcData *adc;
 FrVect *vect;
 FrProcData *proc;
 int   status, first, nFrames;
 long buffSize, debugLvl, i, nBin, nData, index, utc;
 double *data, *datai, dt, df, sampleRate;
 char  fileName[MAX_FILE_NAME];
 char  adcName[MAX_VECT_NAME];
 char *buff, *name, *comment, *units;

    /*--------------- check arguments ---------------------*/

 if(nrhs<2)
    {mexErrMsgTxt("Please provide the file name(s) and ADC or PROC name as argument\n");
     return;}

 if(!mxIsChar(prhs[0]))
    {mexErrMsgTxt("The first arguement is not a filename\n");
     return;}
  
 if(!mxIsChar(prhs[1]))
    {mexErrMsgTxt("The second arguement is not the adc name \n");
     return;}
  
 if(nlhs==0)
    {mexErrMsgTxt("Please provide at least one output argument\n");
     return;}

 if(nrhs > 2)
     {data = mxGetPr(prhs[2]);
      first = data[0];}
 else{first = 0;}

 if(nrhs > 3)
     {data = mxGetPr(prhs[3]);
      nFrames = data[0];}
 else{nFrames = 1;}

 if(nrhs > 4)
     {data = mxGetPr(prhs[4]);
      debugLvl = data[0];
      FrLibSetLvl(debugLvl);}
 else{debugLvl = 0;}

    /*----------- open file --------------------------*/

  if (0 != mxGetString(prhs[0],fileName,MAX_FILE_NAME))
  {
    const char* msg = "frextract:nameLength";
    mexErrMsgIdAndTxt(msg,msg);
  }
  if (0 != mxGetString(prhs[1],adcName,MAX_VECT_NAME))
  {
    const char* msg = "frextract:nameLength";
    mexErrMsgIdAndTxt(msg,msg);
  }
  if(debugLvl > 0)
   {mexPrintf(" Open file:%s for channel:%s\n",fileName,adcName);
    mexPrintf(" first frame:%d nFrames=%d\n",first,nFrames);}

  iFile = FrFileINew(fileName);
  if(iFile == NULL)
     {mexPrintf("Error during file opening\n"
             "  Last errors are:\n%s",FrErrorGetHistory());
      return;}

     /*--------- read the first frame ------------------*/
   
  while((frame = FrameRead(iFile)) != NULL)
      {
       if(debugLvl > 0) mexPrintf(" Frame %d run %d GPS : %d\n", 
                       frame->frame, frame->run,frame->GTimeS);
       if(first == 0) break;
       if(frame->frame >= first) break; 
       FrameFree(frame);}

  if(frame == NULL) 
     {FrFileIEnd(iFile);  
      return;}

     /*------ find ADC and create output matrix ---------*/

  adc = FrAdcDataFind(frame,adcName);
  if(adc != NULL) 
    {vect       = adc->data;
     nBin       = adc->data->nData;
     sampleRate = adc->sampleRate;
     comment    = adc->comment;
     units      = adc->units;}
  else if((proc = FrProcDataFind(frame, adcName)) != NULL)
    {vect       = proc->data;
     nBin       = proc->data->nData;
     dt         = vect->dx[0];
     sampleRate = 1./vect->dx[0];
     comment    = proc->comment;
     units      = proc->data->unitY;}
   else 
     {FrFileIEnd(iFile);  
      FrameFree(frame);
      return;}

  nData = nBin*nFrames;
  dt = 1./sampleRate;
  df = sampleRate/((double)nData);
  if ( vect->type == FR_VECT_8C || vect->type == FR_VECT_16C ) {
    plhs[0] = mxCreateDoubleMatrix(nData,1,mxCOMPLEX);
  } else {
    plhs[0] = mxCreateDoubleMatrix(nData,1,mxREAL);
  }
 
    /*------ fill time and frequency array ---------------*/

  if(nlhs > 1)
    {plhs[1] = mxCreateDoubleMatrix(nData,1,mxREAL);
     data = mxGetPr(plhs[1]);
     for(i=0; i<nData; i++)
        {data[i] = (double)i*dt;}}

  if(nlhs > 2)
    {plhs[2] = mxCreateDoubleMatrix(nData/2,1,mxREAL);
     data = mxGetPr(plhs[2]);
     for(i=0; i<nData/2; i++)
        {data[i] = (double)i*df;}}

  if(nlhs > 3)
    {plhs[3] = mxCreateDoubleMatrix(1,1,mxREAL);
     data = mxGetPr(plhs[3]);
     data[0] = (double)frame->GTimeS + 1.e-9*(double)frame->GTimeN;}

  if(nlhs > 4)
    {utc = frame->GTimeS - frame->ULeapS + FRGPSTAI;
     name = mxCalloc(200,sizeof(char));
     sprintf(name,"Starting GPS time:%d UTC=%s",frame->GTimeS,FrStrGTime(utc));
     plhs[4] = mxCreateString(name);
     mxFree(name);}	

  if(nlhs > 5)
    {if(comment != NULL)
         {name = mxCalloc(strlen(comment)+1,sizeof(char));
          strcpy(name, comment);
          plhs[5] = mxCreateString(name);
          mxFree(name);}		
     else{plhs[5] = mxCreateString("no comment");}}

  if(nlhs > 6)
    {if(units != NULL)
         {name = mxCalloc(strlen(units)+1,sizeof(char));
          strcpy(name, units);
          plhs[6] = mxCreateString(name);
          mxFree(name);}
     else{plhs[6] = mxCreateString("no unit");}}

  if(nlhs > 7)
    {plhs[7] = mxCreateDoubleMatrix(1,9,mxREAL);
     data = mxGetPr(plhs[7]);
     if(adc != NULL)
       {data[0] = (double) adc->channelGroup;
        data[1] = (double) adc->channelNumber;
        data[2] = (double) adc->nBits;
        data[3] = (double) adc->bias;
        data[4] = (double) adc->slope;
        data[5] = (double) sampleRate;
        data[6] = (double) adc->timeOffset;
        data[7] = (double) adc->fShift;
        data[8] = (double) adc->dataValid;
	vect = adc->data;}
     else
       {data[0] = (double) 0;
        data[1] = (double) 0;
        data[2] = (double) proc->data->wSize;
        data[3] = (double) 0;
        data[4] = (double) 0;
        data[5] = (double) sampleRate;
        data[6] = (double) proc->timeOffset;
        data[7] = (double) proc->fShift;
        data[8] = (double) 0;
	vect = proc->data;}}

    /*-------- copy data and read the next frames -------*/

  data = mxGetPr(plhs[0]);
  datai = mxGetPi(plhs[0]);
  index = 0;
  while(1)
    {if(vect == NULL) 
        {mexPrintf(" no data for frame %ld\n",frame->frame);}
     else if(vect->type == FR_VECT_C)
        {for(i=0; i<nBin; i++) {data[i+index] = vect->data[i];}}
     else if(vect->type == FR_VECT_2S)
        {for(i=0; i<nBin; i++) {data[i+index] = vect->dataS[i];}}
     else if(vect->type == FR_VECT_4S)
        {for(i=0; i<nBin; i++) {data[i+index] = vect->dataI[i];}}
     else if(vect->type == FR_VECT_8S)
        {for(i=0; i<nBin; i++) {data[i+index] = vect->dataL[i];}}
     else if(vect->type == FR_VECT_1U)
        {for(i=0; i<nBin; i++) {data[i+index] = vect->dataU[i];}}
     else if(vect->type == FR_VECT_2U)
        {for(i=0; i<nBin; i++) {data[i+index] = vect->dataUS[i];}}
     else if(vect->type == FR_VECT_4U)
        {for(i=0; i<nBin; i++) {data[i+index] = vect->dataUI[i];}}
     else if(vect->type == FR_VECT_8U)
        {for(i=0; i<nBin; i++) {data[i+index] = vect->dataUL[i];}}
     else if(vect->type == FR_VECT_4R)
        {for(i=0; i<nBin; i++) {data[i+index] = vect->dataF[i];}}
     else if(vect->type == FR_VECT_8R)
        {for(i=0; i<nBin; i++) {data[i+index] = vect->dataD[i];}}
     else if(vect->type == FR_VECT_8C)
        {
	  for(i=0; i<nBin; i++) {
	    data[i+index] = vect->dataF[2*i];
	    datai[i+index] = vect->dataF[2*i+1];
	  }
	}
     else if(vect->type == FR_VECT_16C)
        {
	  for(i=0; i<nBin; i++) {
	    data[i+index] = vect->dataD[2*i];
	    datai[i+index] = vect->dataD[2*i+1];
	  }
	}
     else 
        {mexPrintf("Error: cannot handle the data type of this vector\n");
         return;}

     index += nBin;

     FrameFree(frame);
     frame = FrameRead(iFile);
     if(frame == NULL) break; 
     if(index >= nFrames*nBin) break; 

     vect = FrameFindVect(frame,adcName);}
    
  if(frame != NULL) FrameFree(frame);

  FrFileIEnd(iFile);            /* Close file and release descriptor */

}
