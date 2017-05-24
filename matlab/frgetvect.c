/*---------------------------------------------------------------------*/
/* FrGetVect.c          by B. Mours Caltech&LAPP   May  2, 2002        */
/*   && Increase file name length - K. Thorne - Sept 19, 2006 &&       */ 
/*                                                                     */
/* This Matlab mex file extract from a frame file the data for one     */ 
/* vector (ADC or Proc or Sim data) (this is for Matlab version 5)     */
/*                                                                     */
/* The input arguments are:                                            */
/*   1) file name(s). This could be a single file, a list of file      */
/*                    separated by space or a frame file list (ffl)    */
/*   2) channel name (it could be an ADC, SIM or PROC channel)         */
/*   3) (optional) starting GPS time(default=0 first frame in the file)*/
/*   4) (optional) vector length in second (default = 1 second)        */
/*   5) (optional) debug level (default=0; -1=no output; > 1 more out.)*/
/*                                                                     */
/* Returned matlab data:                                               */
/*   1) ADC or SIM or PROC data stored as double                       */
/*   2) (optional) x axis values relative to the first data point.     */
/*            This is usual time but it could be frequency in the case */
/*            of a frequency serie (double)                            */
/*   3) (optional) frequency values in the case of time series (double)*/
/*   4) (optional) GPS starting time (in second, stored in double)     */
/*   5) (optional) starting time as a string                           */
/*   6) (optional) vector unitX as a string                            */
/*   7) (optional) vector unitY as a string                            */
/*      All values are stored as double                                */
/*                                                                     */
/* Warning types:                                                      */
/*   frgetvect:info    -- informational messages                       */
/*   frgetvect:dt==0   -- channel metadata dt == 0                     */
/*                                                                     */
/* Error types:                                                        */
/*   frgetvect:inputArgCount    -- too many or too few input arguments */
/*   frgetvect:outputArgCount   -- too many or too few output arguments*/
/*   frgetvect:inputArg         -- first two arguments are not strings */
/*   frgetvect:nameLength       -- file or channel name too long       */
/*   frgetvect:fileOpen         -- failure to open file                */
/*   frgetvect:channelNotFound  -- channel not in frame                */
/*   frgetvect:noMatlabType     -- no matlab type for channel data     */
/*---------------------------------------------------------------------*/
#include "mex.h"
#include "FrameL.h"

#define MAX_FILE_NAME 1024
#define MAX_VECT_NAME 100

void  mexFunction( int nlhs,       mxArray *plhs[],
                   int nrhs, const mxArray *prhs[])
{
  struct FrFile *iFile;
  struct FrVect *vect;
  int   status;
  long  debugLvl, i, nData, utc;
  double *data, *datai, dt, df, t0, duration;
  char  *name, fileName[MAX_FILE_NAME],vectName[MAX_VECT_NAME];

    /*--------------- check arguments ---------------------*/
  
  if (nrhs < 2 || nrhs > 5) 
  {
    const char *msgId = "frgetvect:inputArgCount";
    mexErrMsgIdAndTxt(
      msgId,"%s:nRhs = %d, must be between 2 and 5",
      msgId,nrhs);
  }
  
  if (nlhs < 1 || nlhs > 7)
  {
    const char *msgId = "frgetvect:outputArgCount";
    mexErrMsgIdAndTxt(
      msgId,"%s: nLhs = %d, must be between 1 and 7",
      msgId,nlhs);
  }
  
  if(!mxIsChar(prhs[0]))
  {
    const char *msgId = "frgetvect:inputArg";
    mexErrMsgIdAndTxt(msgId,"%s: first arg must be filename",msgId);
  }
  
  if(!mxIsChar(prhs[1]))
  {
    const char *msgId = "frgetvect:inputArg";
    const char *msg = "%s: second arg must be channel name";
    mexErrMsgIdAndTxt(msgId,msg,msgId);
  }
  
  if(nrhs > 2)
  {
    data = mxGetPr(prhs[2]);
    t0 = data[0];
  }
  else
  {
    t0 = 0;
  }

  if(nrhs > 3)
  {
    data = mxGetPr(prhs[3]);
    duration = data[0];
  }
  else
  {
    duration = 1.;
  }

  if(nrhs > 4)
  {
    data = mxGetPr(prhs[4]);
    debugLvl = data[0];
    FrLibSetLvl(debugLvl);
  }
  else
  {
    debugLvl = 0.0;
    FrLibSetLvl(debugLvl);
  }
  

  /*
  {
    const char* msgId = "frgetvect:info";
    mexWarnMsgIdAndTxt(msgId,"%s:nrhs = %d",msgId,nrhs);
    mexWarnMsgIdAndTxt(msgId,"%s:nlhs = %d",msgId,nlhs);
  }
  */

    /*----------- open file --------------------------*/

  if (0 != mxGetString(prhs[0],fileName,MAX_FILE_NAME))
  {
    const char* msg = "frgetvect:nameLength";
    mexErrMsgIdAndTxt(msg,msg);
  }
  if (0 != mxGetString(prhs[1],vectName,MAX_VECT_NAME))
  {
    const char* msg = "frgetvect:nameLength";
    mexErrMsgIdAndTxt(msg,msg);
  }
  if(debugLvl > -1) 
  {
    const char* msg = "frgetvect: get %s t0=%.2f, duration=%g from %s\n";
    mexPrintf(msg, vectName, t0, duration, fileName);
  }

  iFile = FrFileINew(fileName);
  if(iFile == NULL)
  {
    const char *msgId = "frgetvect:fileOpen";
    const char *fmtStr = "%s:Last errors are %s";
    mexErrMsgIdAndTxt(msgId,fmtStr,msgId,FrErrorGetHistory());
    return;
  }

     /*--------- get the vector ------------------*/
     
  vect = FrFileIGetV(iFile, vectName, t0, duration);
  if(debugLvl > 0) FrVectDump(vect, stdout, debugLvl);
  if(vect == NULL)
  {
    const char *msgId = "frgetvect:channelNotFound";
    const char *fmtStr = "%s: %s";
    FrFileIEnd(iFile);
    mexErrMsgIdAndTxt(msgId,fmtStr,msgId,vectName);
    return;
  }

  nData = vect->nData;
  if ( vect->type == FR_VECT_8C || vect->type == FR_VECT_16C ) {
    plhs[0] = mxCreateDoubleMatrix(nData,1,mxCOMPLEX);
  } else {
    plhs[0] = mxCreateDoubleMatrix(nData,1,mxREAL);
  }
  dt = vect->dx[0];
  if (0.0 == dt)
  {
    const char *msgId = "frgetvect:dt==0";
    mexWarnMsgIdAndTxt(msgId,msgId);
    df = 0.0;
  } 
  else
  {
    df = 1.0/(dt*nData);
  }
 
    /*-------- copy data ------*/

  data = mxGetPr(plhs[0]);
  if(vect->type == FR_VECT_C)
  {for(i=0; i<nData; i++) {data[i] = vect->data[i];}}
  else if(vect->type == FR_VECT_2S)
  {for(i=0; i<nData; i++) {data[i] = vect->dataS[i];}}
  else if(vect->type == FR_VECT_4S)
  {for(i=0; i<nData; i++) {data[i] = vect->dataI[i];}}
  else if(vect->type == FR_VECT_8S)
  {for(i=0; i<nData; i++) {data[i] = vect->dataL[i];}}
  else if(vect->type == FR_VECT_1U)
  {for(i=0; i<nData; i++) {data[i] = vect->dataU[i];}}
  else if(vect->type == FR_VECT_2U)
  {for(i=0; i<nData; i++) {data[i] = vect->dataUS[i];}}
  else if(vect->type == FR_VECT_4U)
  {for(i=0; i<nData; i++) {data[i] = vect->dataUI[i];}}
  else if(vect->type == FR_VECT_8U)
  {for(i=0; i<nData; i++) {data[i] = vect->dataUL[i];}}
  else if(vect->type == FR_VECT_4R)
  {for(i=0; i<nData; i++) {data[i] = vect->dataF[i];}}
  else if(vect->type == FR_VECT_8R)
  {for(i=0; i<nData; i++) {data[i] = vect->dataD[i];}}
  else if(vect->type == FR_VECT_8C)
  {
    datai = mxGetPi(plhs[0]);
    for(i=0; i<nData; i++) {
      data[i] = vect->dataF[2*i];
      datai[i] = vect->dataF[2*i+1];
    }
  }
  else if(vect->type == FR_VECT_16C)
  {
    datai = mxGetPi(plhs[0]);
    for(i=0; i<nData; i++) {
      data[i] = vect->dataD[2*i];
      datai[i] = vect->dataD[2*i+1];
    }
  }
  else
  {
    const char* msgId = "frgetvect:noMatlabType";
    const char* fmtStr = "%s: no matlab type for this channel";
    FrVectFree(vect);
    FrFileIEnd(iFile);
    /* need to destroy matrices created for output */
    mxDestroyArray(plhs[0]);
    mexErrMsgIdAndTxt(msgId,fmtStr,msgId);
    return;
  }
    
/*------ fill time and frequency array ---------------*/

  if(nlhs > 1)
  {
    plhs[1] = mxCreateDoubleMatrix(nData,1,mxREAL);
    data = mxGetPr(plhs[1]);
    for(i=0; i<nData; i++)
    {
      data[i] = vect->startX[0]+(double)i*dt;
    }
  }

  if(nlhs > 2)
  {
    plhs[2] = mxCreateDoubleMatrix(nData/2,1,mxREAL);
    data = mxGetPr(plhs[2]);
    for(i=0; i<nData/2; i++)
    {
      data[i] = (double)i*df;
    }
  }

  if(nlhs > 3)
  {
    plhs[3] = mxCreateDoubleMatrix(1,1,mxREAL);
    data = mxGetPr(plhs[3]);
    data[0] = (double)vect->GTime;
  }

  if(nlhs > 4)
  {
    utc = vect->GTime - vect->ULeapS + FRGPSTAI;
    name = mxCalloc(200,sizeof(char));
    sprintf(name,"Starting GPS time:%.1f UTC=%s",vect->GTime,FrStrGTime(utc));
    plhs[4] = mxCreateString(name);
    mxFree(name);
  }

  if(nlhs > 5)
  {
    if(vect->unitX[0] != NULL)
    {
      name = mxCalloc(strlen(vect->unitX[0])+1,sizeof(char));
      strcpy(name, vect->unitX[0]);
      plhs[5] = mxCreateString(name);
      mxFree(name);
    }
    else
    {
      plhs[5] = mxCreateString("no unit");
    }
  }

  if(nlhs > 6)
  {
    if(vect->unitY != NULL)
    {
      name = mxCalloc(strlen(vect->unitY)+1,sizeof(char));
      strcpy(name, vect->unitY);
      plhs[6] = mxCreateString(name);
      mxFree(name);
    }
    else
    {
      plhs[6] = mxCreateString("no unit");
    }
  }

  FrVectFree(vect);
  FrFileIEnd(iFile);            /* Close file and release descriptor */

}
