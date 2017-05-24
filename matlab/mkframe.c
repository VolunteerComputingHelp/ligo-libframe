/* mkframe.c - MEX file implementation of mkframe by Sam Finn

% MKFRAME - write frame files (according with LIGO standards)
%   mkframe(file, data, mode[,frameLength,gpsStart,parameters,comment])
%
% file = frame file name string
% data = array of structures of data to be written to frame file
%        (can be empty if parameters are input)
% mode   = 'n' to write new frame file
%          'a' to add to existing frame file
% frameLength = [optional] length of frame in seconds (default = 16 sec)
% gpsStart = [optional] GPS time of frame (default = current time)
% parameters = [optional] a structure of parameters and desired channel name 
%                 (default = NULL)
% comment = [optional] Comment to be added to the frame (default = NULL)
%
% Each element of the data array must be structures with these fields
%
% data.channel = channel name string
% data.data    = time-series data
% data.type    = type of data ('s','us','i','ui','l','ul','d','f' or 'dc')
% data.mode    = mode( 'a' for ADC, 'p' for proc, 's' for static )
%
% The parameters input must be a structure with the fields
%
% parameters.name = parameters channel name string
% parameters.parameters = 2-D array of parameter names and values
%
%  Error message IDs
%   mkframe:badInput - error in input parameters
%   mkframe:badField - error in data array
%   mkframe:frameFail - error in reading/writing frame file
%
*/

#include "mex.h"
#include "FrameL.h"

void mexFunction( int nlhs,       mxArray *plhs[],
		  int nrhs, const mxArray *prhs[])
{
    struct FrameH *frame;
    struct FrAdcData *adc, *ptrPossAdc;
    struct FrProcData *proc, *ptrPossProc;
    struct FrFile *oFile, *iFile;
    struct FrVect *b, *paramV;
    struct FrStatData *StatData, *StatParams;
    mxArray *ptrType, *dataVector, *ptrMode, *channelName;
    mxArray *thisCell;
    char *thisName, *thisParam, *pNameStr;
    mxArray *ptrName;
    const mxArray *ptrParamArray;
    char *mode, *fileName, *name, *type, *ADCvsStat, *comment;
    double rate, *frameLength, *RealdataPtr, *ImagdataPtr; 
    void *dataPtr;
    int numData, numFields, numElems, elem, elemSize, numCharParm;
    size_t dataBytes;
    mwSize paramElems, numParamCol, numParamRow, numParamCell;
    mwSize lenBuf,lenComment,lenChan, numSub;
    mwIndex *subVal;
    double *frameGPS;
    time_t utc;
    unsigned int t0;
    int i, cIdx;
    mwIndex j, k, thisIdx;
    const char *channelField = "channel";
    const char *dataField = "data";
    const char *typeField = "type";
    const char *modeField = "mode";
    const char *errBadInput = "mkframe:badParam";
    const char *errFrameFail = "mkframe:frameFail";
    const char *errBadField = "mkframe:badField";
    const char *errBadData = "mkframe:badData";
    const double DEFAULT_FRAME_LENGTH = 16;
    int compress = 6;
    

/*  -----check input arguments for correct type----------- */
if(!mxIsChar(prhs[0]) || !mxIsChar(prhs[2]))
{
    mexErrMsgIdAndTxt(errBadInput,"%sInput must include: file name(string), write mode(char)",errBadInput);
}
		
/* -----check is there is anything to be written	*/
if ( nrhs > 5 )
{
	if( mxIsEmpty(prhs[1]) && mxIsEmpty(prhs[5]) )
    { 
     mexErrMsgTxt("Neither data nor parameters will be written to frame. Why write it? \n");
    }
}	
/*  GET string for frame file name      */
/*  GET write mode                      */
/*  IF write mode is invalid            */
/*      CREATE error and exit           */
/*  ENDIF                               */
/*  IF framelength was input            */
/*      IF framelength is not numeric   */
/*          CREATE error and exit       */
/*      ELSE                            */
/*          SET framelength from input  */
/*          IF framelength is invalid   */
/*              CREATE error and exit   */
/*          ENDIF                       */
/*      ENDIF                           */
/*  ELSE                                */
/*      SET framelength to default      */
/*  ENDIF                               */
/*  IF frameGPS was input               */
/*      IF frameGPS is not numeric      */
/*          CREATE error and exit       */
/*      ELSE                            */
/*          SET frameGPS from input     */
/*      ENDIF                           */
/*  ELSE                                */
/*      SET frameGPS to current time    */
/*  ENDIF                               */
lenBuf = mxGetN(prhs[0])*sizeof(mxChar)+1;
fileName = mxMalloc(lenBuf);
mxGetString(prhs[0], fileName, lenBuf);
lenBuf = mxGetN(prhs[2])*sizeof(mxChar)*1;
mode = mxMalloc(lenBuf);
mxGetString(prhs[2], mode, lenBuf);
if((mode[0] != 'n') && mode[0] != 'a')
{
    mexErrMsgIdAndTxt(errBadInput,"%s The mode for writing must be 'n' or 'a'",errBadInput);
}
    
if( nrhs > 3 )
{
	if( mxIsEmpty(prhs[3]))
	{
        frameLength = mxMalloc(sizeof(double));
        *frameLength = DEFAULT_FRAME_LENGTH;
        mexPrintf("Input Comment: No frame length provided, setting it to default %f sec\n", DEFAULT_FRAME_LENGTH);
	}
	else
    {
        if(!mxIsNumeric(prhs[3]))
        {
            mexErrMsgIdAndTxt(errBadInput,"%s The frame length must be numeric",errBadInput);
        }
        else
        {
            frameLength = mxGetPr(prhs[3]);
            if(*frameLength <= 0)
            {
                mexErrMsgIdAndTxt(errBadInput,"%s The frame length must be > 0",errBadInput);
            }    
        }
    }
}
else
{
    frameLength = mxMalloc(sizeof(double));
    *frameLength = DEFAULT_FRAME_LENGTH;
    mexPrintf("Input Comment: No frame length provided, setting it to default %f sec\n", DEFAULT_FRAME_LENGTH);
}

if( nrhs > 4)
{
	if( mxIsEmpty(prhs[4]) )
	{ 
        utc = time(NULL);
        /*-- localTime no longer exists as of frame format version 6 --*/
        /*frame->localTime = mktime(localtime(&utc)) - mktime(gmtime(&utc));*/
        frameGPS = mxMalloc(sizeof(double));
        *frameGPS = utc - FRGPSOFF + FRGPSLEAPS;
        mexPrintf("Input Comment: No frame GPS provided, setting it to current time \n");
	}
	else
    {
		if(!mxIsNumeric(prhs[4]))
        {
            mexErrMsgIdAndTxt(errBadInput,"%s The frame GPS must be numeric",errBadInput);
        }
        else
        {
            frameGPS = mxGetPr(prhs[4]);
        }
    }
}	
else
{
    utc = time(NULL);
    /*-- localTime no longer exists as of frame format version 6 --*/
    /*frame->localTime = mktime(localtime(&utc)) - mktime(gmtime(&utc));*/
	frameGPS = mxMalloc(sizeof(double));
	*frameGPS = utc - FRGPSOFF + FRGPSLEAPS;
    mexPrintf("Input Comment: No frame GPS provided, setting it to current time \n");
}	

  /*---------create frame if the file is new-------*/
if(mode[0] == 'n')
{
    frame = FrameNew("LIGO");
    if (frame == NULL)
    {
        mexErrMsgIdAndTxt(errFrameFail,"%s Error during frame creation\n"
		 "Last errors are: \n%s", errFrameFail,FrErrorGetHistory() );
    }
	/*-- set history in frame --*/
	FrHistoryAdd(frame, "Created by mkframe");
		
   /*----------add time to frame---------*/
    frame->dt = *frameLength;
    frame->ULeapS    = FRGPSLEAPS;
    frame->GTimeS    = *frameGPS;
    t0 = frame->GTimeS;
    frame->run = -1;
    mexPrintf("MKFRAME: Create new frame file\n");
}
  /*------open existing frame file if appending data--------*/
if(mode[0] == 'a')
{
    iFile = FrFileINew(fileName);
    if(iFile == NULL)
    {
        mexErrMsgIdAndTxt(errFrameFail,"%s Cannot open existing frame file",
        errFrameFail);
    }

/* Set Compression */
    iFile->compress = 6;
    frame = FrameRead(iFile);
    if(frame == NULL)
    {
        mexErrMsgIdAndTxt(errFrameFail,"%s Cannot read existing frame file",
        errFrameFail);
    }
    FrFileIEnd(iFile);
}
/*  GET number of elements----                */
/*  LOOP over elements                        */
/*      GET channel name from field             */
/*      IF channel field is not present         */
/*          CREATE error and exit               */
/*      ELSE                                    */
/*          IF channel is not a string          */
/*              CREATE error and exit           */
/*          ENDIF                               */
/*      ENDIF                                   */
/*      GET time-series data                    */
/*      IF data field is not present            */
/*          CREATE error and exit               */
/*      ELSE                                    */
/*          IF data field is not numeric        */
/*              CREATE error and exit           */
/*          ENDIF                               */
/*      ENDIF                                   */

if(mxIsEmpty(prhs[1]))
{ 
	mexPrintf("Input Comment: No data will be written to frame, only parameters will be\n");
}
else if(!mxIsStruct(prhs[1]))
{
	mexErrMsgIdAndTxt(errBadInput,"%s The second input must be a structure",errBadInput);
}
else
{
	numElems = mxGetNumberOfElements(prhs[1]);
    if(numElems < 1)
    {
        mexErrMsgIdAndTxt(errBadField,"%s no elements in data structure array",errBadField);
    }
    for(elem=0; elem < numElems; elem++)
    {        
        channelName = mxGetField(prhs[1], elem, channelField);
        if(channelName == NULL)
        {
            mexErrMsgIdAndTxt(errBadField,"%s entry %d field %s missing",
            errBadField,(elem+1),channelField);
        }
        else
        {
            if(!mxIsChar(channelName))
            {
                mexErrMsgIdAndTxt(errBadField,"%s entry %d channel must be string",
                errBadField,elem+1);
            }
            lenChan = mxGetN(channelName)*sizeof(mxChar)+1;
            name = mxMalloc(lenChan);
            mxGetString(channelName, name, lenChan);
        }
        if(mode[0] == 'a')
        {
            mexPrintf("MKFRAME: Append Channel %s to existing frame\n",name);
        }
        else
        {
            mexPrintf("MKFRAME: Add Channel %s to new frame\n",name);
        }
        dataVector = mxGetField(prhs[1], elem, dataField);
        if(dataVector == NULL)
        {
            mexErrMsgIdAndTxt(errBadField,"%s entry %d field %s missing",
            errBadField,(elem+1),dataField);
        }
        else
        {
            if(!mxIsNumeric(dataVector))
            {
                mexErrMsgIdAndTxt(errBadField,"%s entry %d data must be numeric",
                errBadField,elem+1);
            }
            dataPtr = mxGetPr(dataVector);

			numData = mxGetNumberOfElements(dataVector);	
			elemSize = mxGetElementSize(dataVector);
            dataBytes = numData * elemSize;
            rate = numData/(*frameLength);
        }
/*      GET data type from type field           */
/*      IF type field is not present            */
/*          CREATE error and exit               */
/*      ELSE                                    */
/*          IF type is not valid                */
/*              CREATE error and exit           */
/*          ENDIF                               */
/*      ENDIF                                   */
/*      GET data mode from mode field           */
/*      IF mode field is not present            */
/*          CREATE error and exit               */
/*      ELSE                                    */
/*          IF data mode is not valid           */
/*              CREATE error and exit           */
/*          ENDIF                               */
/*      ENDIF                                   */
        ptrType = mxGetField(prhs[1], elem, typeField);
        if(ptrType == NULL)
        {
            mexErrMsgIdAndTxt(errBadField,"%s entry %d field %s missing",
            errBadField,(elem+1),typeField);
        }
        else
        {
            lenBuf = mxGetN(ptrType) + 1;            
            type = mxCalloc(lenBuf,sizeof(mxChar));
            mxGetString(ptrType, type, lenBuf);
            if((type[0] != 'd') && (type[0] != 'f') && (type[0] != 's') &&
               (type[0] != 'l') && (type[0] != 'i') && (type[0] != 'u')) 
            {
                mexErrMsgIdAndTxt(errBadField,
                "%s entry %d type must be one of d,f,s,us,i,ui,l,ul,dc", 
                errBadField,elem+1);
            }
            if(type[0] == 'u')
            {
                if((type[1] != 's') && (type[1] != 'l') && (type[1] != 'i'))
                {
                    mexErrMsgIdAndTxt(errBadField,
                    "%s entry %d type must be one of d,f,s,us,i,ui,l,ul,dc",
                    errBadField,elem+1);
                }
            }
        }
        ptrMode = mxGetField(prhs[1], elem, modeField);
        if(ptrMode == NULL)
        {
            mexErrMsgIdAndTxt(errBadField,"%s entry %d field %s missing",
            errBadField,(elem+1),modeField);
        }
        else
        {
            ADCvsStat = mxCalloc(6,sizeof(short));
            mxGetString(ptrMode, ADCvsStat, 6);
            if((ADCvsStat[0] != 'a') && (ADCvsStat[0] != 'p') && (ADCvsStat[0] != 's'))
            {
                mexErrMsgIdAndTxt(errBadField,
                "%s entry %d mode must be 'a', 'p' or 's'",
                errBadField,elem+1);
            }
        }
/*      IF ADC data                                                 */
/*          GET pointer to channel 'name' in frame                  */
/*          IF pointer is valid (which means it exists)             */
/*              CREATE error and exit                               */
/*          ENDIF                                                   */
/*          IF a particular data type                               */
/*              GET new ADC data structure                          */
/*              IF data structure is bad                            */
/*                  CREATE error and exit                           */
/*              ENDIF                                               */
/*              FILL appropriate field in structure with data       */
/*          ENDIF                                                   */
        if(ADCvsStat[0] == 'a')
        {
            ptrPossAdc = FrAdcDataFind(frame,name);
            if(ptrPossAdc != NULL)
            {
                mexErrMsgIdAndTxt(errBadField,
                    "%s ADC for %s already exists!",
                  errFrameFail,name);
            }
			if(type[0] == 'd')
            {
				if(!mxIsDouble(dataVector))
                {
                    mexErrMsgIdAndTxt(errBadData,
					"%s ADC data for %s must be MATLAB double\n Last errors are are:\n%s",
					errBadData,name,FrErrorGetHistory() ); 
				}
					
			    if(type[1] == 'c')
				{ 
                    if(!mxIsComplex(dataVector)) 
		      {
			mexErrMsgIdAndTxt(errBadData,
					  "%s ADC data for %s must be MATLAB complex\n Last errors are are:\n%s",
					  errBadData,name,FrErrorGetHistory() ); 
		      }
		    
                    adc = FrAdcDataNew(frame, name, rate, numData, -64); 
                    if(adc == NULL)
					{
						mexErrMsgIdAndTxt(errFrameFail,
						"%s ADC for %s is null\n Last errors are:\n%s",
					    errFrameFail,name,FrErrorGetHistory() );
					}
					
                    adc->data->type = FR_VECT_16C;

                    RealdataPtr = mxGetPr(dataVector);  /* since no mxGetComplex exists */
                    ImagdataPtr = mxGetPi(dataVector);  /* real and imaginary data are passed separately */

		    /*cet: complex adc file gets here, complex static no*/
		    /*
		    mexPrintf("cet: right after real and imaginary\n");
		    mexPrintf("%f %f \n",RealdataPtr[0],RealdataPtr[1]);
		    mexPrintf("%f %f \n",ImagdataPtr[0],ImagdataPtr[1]);
		    */
		    
                    for(i=0; i<numData; i++) 
					{
                        adc->data->data[2*i] = RealdataPtr[i];  /* data->data not data->dataD is populated */ 
                        adc->data->data[2*i+1] = ImagdataPtr[i];
					}	
				}
				else
                {
					adc = FrAdcDataNew(frame, name, rate, numData, -64);
					if(adc == NULL)
					{
						mexErrMsgIdAndTxt(errFrameFail,
						"%s ADC for %s is null\n Last errors are:\n%s",
                        errFrameFail,name,FrErrorGetHistory() );
					}
					dataBytes = numData * elemSize;
					memcpy(adc->data->dataD,dataPtr,dataBytes);
				}
				
			}
            else if(type[0] == 'f')
            {
                if(!mxIsSingle(dataVector))
                {
                    mexErrMsgIdAndTxt(errBadData,
                    "%s ADC data for %s must be MATLAB single\n Last errors are are:\n%s",
                    errBadData,name,FrErrorGetHistory() ); 
                }
	            adc = FrAdcDataNew(frame, name, rate, numData, -32);
                if(adc == NULL)
                {
                    mexErrMsgIdAndTxt(errFrameFail,
                    "%s ADC for %s is null\n Last errors are:\n%s",
                    errFrameFail,name,FrErrorGetHistory() );
                }
	            memcpy(adc->data->dataF,dataPtr,dataBytes);
            }
            else if(type[0] == 's')
            {
                if(!mxIsInt16(dataVector))
                {
                    mexErrMsgIdAndTxt(errBadData,
                    "%s ADC data for %s must be MATLAB int16\n Last errors are are:\n%s",
                    errBadData,name,FrErrorGetHistory() ); 
                }
	            adc = FrAdcDataNew(frame, name, rate, numData, 16);
                if(adc == NULL)
                {
                    mexErrMsgIdAndTxt(errFrameFail,
                    "%s ADC for %s is null\n Last errors are:\n%s",
                    errFrameFail,name,FrErrorGetHistory() );
                }
	            memcpy(adc->data->dataS,dataPtr,dataBytes);
            }
            else if(type[0] == 'i')
            {
                if(!mxIsInt32(dataVector))
                {
                    mexErrMsgIdAndTxt(errBadData,
                    "%s ADC data for %s must be MATLAB int32\n Last errors are are:\n%s",
                    errBadData,name,FrErrorGetHistory() ); 
                }
	            adc = FrAdcDataNew(frame, name, rate, numData, 32);
                if(adc == NULL)
                {
                    mexErrMsgIdAndTxt(errFrameFail,
                    "%s ADC for %s is null\n Last errors are:\n%s",
                    errFrameFail,name,FrErrorGetHistory() );
                }
	            memcpy(adc->data->dataI,dataPtr,dataBytes);
            }
            else if(type[0] == 'l')
            {
                if(!mxIsInt64(dataVector))
                {
                    mexErrMsgIdAndTxt(errBadData,
                    "%s ADC data for %s must be MATLAB int64\n Last errors are are:\n%s",
                    errBadData,name,FrErrorGetHistory() ); 
                }
	            adc = FrAdcDataNew(frame, name, rate, numData, 64);
                if(adc == NULL)
                {
                    mexErrMsgIdAndTxt(errFrameFail,
                    "%s ADC for %s is null\n Last errors are:\n%s",
                    errFrameFail,name,FrErrorGetHistory() );
                }
	            memcpy(adc->data->dataL,dataPtr,dataBytes);
            }
            else if(type[0] == 'u')
            {
	            if(type[1] == 's')
                {
                    if(!mxIsUint16(dataVector))
                    {
                        mexErrMsgIdAndTxt(errBadData,
                        "%s ADC data for %s must be MATLAB uint16\n Last errors are are:\n%s",
                        errBadData,name,FrErrorGetHistory() ); 
                    }
	                adc = FrAdcDataNew(frame, name, rate, numData, 16);
                    if(adc == NULL)
                    {
                        mexErrMsgIdAndTxt(errFrameFail,
                        "%s ADC for %s is null\n Last errors are:\n%s",
                        errFrameFail,name,FrErrorGetHistory() );
                    }
	                memcpy(adc->data->dataUS,dataPtr,dataBytes);
	            }
	            else if(type[1] == 'i')
                {
                    if(!mxIsUint32(dataVector))
                    {
                        mexErrMsgIdAndTxt(errBadData,
                        "%s ADC data for %s must be MATLAB uint32\n Last errors are are:\n%s",
                        errBadData,name,FrErrorGetHistory() ); 
                    }
	                adc = FrAdcDataNew(frame, name, rate, numData, 32);
                    if(adc == NULL)
                    {
                        mexErrMsgIdAndTxt(errFrameFail,
                        "%s ADC for %s is null\n Last errors are:\n%s",
                        errFrameFail,name,FrErrorGetHistory() );
                    }
	                memcpy(adc->data->dataUI,dataPtr,dataBytes);
	            }
	            else if(type[1] == 'l')
                {
	                if(!mxIsUint64(dataVector))
                    {
                        mexErrMsgIdAndTxt(errBadData,
                        "%s ADC data for %s must be MATLAB uint64\n Last errors are are:\n%s",
                        errBadData,name,FrErrorGetHistory() ); 
                    }
                    adc = FrAdcDataNew(frame, name, rate, numData, 64);
                    if(adc == NULL)
                    {
                        mexErrMsgIdAndTxt(errFrameFail,
                        "%s ADC for %s is null\n Last errors are:\n%s",
                        errFrameFail,name,FrErrorGetHistory() );
                    }
	                memcpy(adc->data->dataUL,dataPtr,dataBytes);
	            }
            }
        }

    /*-------if data is proc------------ */

/*      IF proc data                                                 */
/*          GET pointer to channel 'name' in frame                  */
/*          IF pointer is valid (which means it exists)             */
/*              CREATE error and exit                               */
/*          ENDIF                                                   */
/*          IF a particular data type                               */
/*              GET new proc data structure                          */
/*              IF data structure is bad                            */
/*                  CREATE error and exit                           */
/*              ENDIF                                               */
/*              FILL appropriate field in structure with data       */
/*          ENDIF                                                   */
        if(ADCvsStat[0] == 'p')
        {
            ptrPossProc = FrProcDataFind(frame,name);
            if(ptrPossProc != NULL)
            {
                mexErrMsgIdAndTxt(errBadField,
                    "%s proc for %s already exists!",
                  errFrameFail,name);
            }
			if(type[0] == 'd')
            {
				if(!mxIsDouble(dataVector))
                {
                    mexErrMsgIdAndTxt(errBadData,
					"%s proc data for %s must be MATLAB double\n Last errors are are:\n%s",
					errBadData,name,FrErrorGetHistory() ); 
				}
					
			    if(type[1] == 'c')
				{ 
                    if(!mxIsComplex(dataVector)) 
		      {
			mexErrMsgIdAndTxt(errBadData,
					  "%s proc data for %s must be MATLAB complex\n Last errors are are:\n%s",
					  errBadData,name,FrErrorGetHistory() ); 
		      }
		    
                    proc = FrProcDataNew(frame, name, rate, numData, -64); 
                    if(proc == NULL)
					{
						mexErrMsgIdAndTxt(errFrameFail,
						"%s proc for %s is null\n Last errors are:\n%s",
					    errFrameFail,name,FrErrorGetHistory() );
					}
					
                    proc->data->type = FR_VECT_16C;

                    RealdataPtr = mxGetPr(dataVector);  /* since no mxGetComplex exists */
                    ImagdataPtr = mxGetPi(dataVector);  /* real and imaginary data are passed separately */

		    /*cet: complex proc file gets here, complex static no*/
		    /*
		    mexPrintf("cet: right after real and imaginary\n");
		    mexPrintf("%f %f \n",RealdataPtr[0],RealdataPtr[1]);
		    mexPrintf("%f %f \n",ImagdataPtr[0],ImagdataPtr[1]);
		    */
		    
                    for(i=0; i<numData; i++) 
					{
                        proc->data->data[2*i] = RealdataPtr[i];  /* data->data not data->dataD is populated */ 
                        proc->data->data[2*i+1] = ImagdataPtr[i];
					}	
				}
				else
                {
					proc = FrProcDataNew(frame, name, rate, numData, -64);
					if(proc == NULL)
					{
						mexErrMsgIdAndTxt(errFrameFail,
						"%s proc for %s is null\n Last errors are:\n%s",
                        errFrameFail,name,FrErrorGetHistory() );
					}
					dataBytes = numData * elemSize;
					memcpy(proc->data->dataD,dataPtr,dataBytes);
				}
				
			}
            else if(type[0] == 'f')
            {
                if(!mxIsSingle(dataVector))
                {
                    mexErrMsgIdAndTxt(errBadData,
                    "%s proc data for %s must be MATLAB single\n Last errors are are:\n%s",
                    errBadData,name,FrErrorGetHistory() ); 
                }
	            proc = FrProcDataNew(frame, name, rate, numData, -32);
                if(proc == NULL)
                {
                    mexErrMsgIdAndTxt(errFrameFail,
                    "%s proc for %s is null\n Last errors are:\n%s",
                    errFrameFail,name,FrErrorGetHistory() );
                }
	            memcpy(proc->data->dataF,dataPtr,dataBytes);
            }
            else if(type[0] == 's')
            {
                if(!mxIsInt16(dataVector))
                {
                    mexErrMsgIdAndTxt(errBadData,
                    "%s proc data for %s must be MATLAB int16\n Last errors are are:\n%s",
                    errBadData,name,FrErrorGetHistory() ); 
                }
	            proc = FrProcDataNew(frame, name, rate, numData, 16);
                if(proc == NULL)
                {
                    mexErrMsgIdAndTxt(errFrameFail,
                    "%s proc for %s is null\n Last errors are:\n%s",
                    errFrameFail,name,FrErrorGetHistory() );
                }
	            memcpy(proc->data->dataS,dataPtr,dataBytes);
            }
            else if(type[0] == 'i')
            {
                if(!mxIsInt32(dataVector))
                {
                    mexErrMsgIdAndTxt(errBadData,
                    "%s proc data for %s must be MATLAB int32\n Last errors are are:\n%s",
                    errBadData,name,FrErrorGetHistory() ); 
                }
	            proc = FrProcDataNew(frame, name, rate, numData, 32);
                if(proc == NULL)
                {
                    mexErrMsgIdAndTxt(errFrameFail,
                    "%s proc for %s is null\n Last errors are:\n%s",
                    errFrameFail,name,FrErrorGetHistory() );
                }
	            memcpy(proc->data->dataI,dataPtr,dataBytes);
            }
            else if(type[0] == 'l')
            {
                if(!mxIsInt64(dataVector))
                {
                    mexErrMsgIdAndTxt(errBadData,
                    "%s proc data for %s must be MATLAB int64\n Last errors are are:\n%s",
                    errBadData,name,FrErrorGetHistory() ); 
                }
	            proc = FrProcDataNew(frame, name, rate, numData, 64);
                if(proc == NULL)
                {
                    mexErrMsgIdAndTxt(errFrameFail,
                    "%s proc for %s is null\n Last errors are:\n%s",
                    errFrameFail,name,FrErrorGetHistory() );
                }
	            memcpy(proc->data->dataL,dataPtr,dataBytes);
            }
            else if(type[0] == 'u')
            {
	            if(type[1] == 's')
                {
                    if(!mxIsUint16(dataVector))
                    {
                        mexErrMsgIdAndTxt(errBadData,
                        "%s proc data for %s must be MATLAB uint16\n Last errors are are:\n%s",
                        errBadData,name,FrErrorGetHistory() ); 
                    }
	                proc = FrProcDataNew(frame, name, rate, numData, 16);
                    if(proc == NULL)
                    {
                        mexErrMsgIdAndTxt(errFrameFail,
                        "%s proc for %s is null\n Last errors are:\n%s",
                        errFrameFail,name,FrErrorGetHistory() );
                    }
	                memcpy(proc->data->dataUS,dataPtr,dataBytes);
	            }
	            else if(type[1] == 'i')
                {
                    if(!mxIsUint32(dataVector))
                    {
                        mexErrMsgIdAndTxt(errBadData,
                        "%s proc data for %s must be MATLAB uint32\n Last errors are are:\n%s",
                        errBadData,name,FrErrorGetHistory() ); 
                    }
	                proc = FrProcDataNew(frame, name, rate, numData, 32);
                    if(proc == NULL)
                    {
                        mexErrMsgIdAndTxt(errFrameFail,
                        "%s proc for %s is null\n Last errors are:\n%s",
                        errFrameFail,name,FrErrorGetHistory() );
                    }
	                memcpy(proc->data->dataUI,dataPtr,dataBytes);
	            }
	            else if(type[1] == 'l')
                {
	                if(!mxIsUint64(dataVector))
                    {
                        mexErrMsgIdAndTxt(errBadData,
                        "%s proc data for %s must be MATLAB uint64\n Last errors are are:\n%s",
                        errBadData,name,FrErrorGetHistory() ); 
                    }
                    proc = FrProcDataNew(frame, name, rate, numData, 64);
                    if(proc == NULL)
                    {
                        mexErrMsgIdAndTxt(errFrameFail,
                        "%s proc for %s is null\n Last errors are:\n%s",
                        errFrameFail,name,FrErrorGetHistory() );
                    }
	                memcpy(proc->data->dataUL,dataPtr,dataBytes);
	            }
            }
        }


    /*-------if data is Static------------*/
    /*----cast data as appropriate type into a frame vector-------*/
    /*----store in static data channel----------------------------*/
        else if(ADCvsStat[0] == 's')
        {
            if(type[0] == 'd') /* New */
            {
                if(!mxIsDouble(dataVector)) 
                {
                    mexErrMsgIdAndTxt(errBadData,
                    "%s ADC data for %s must be MATLAB double\n Last errors are are:\n%s",
                    errBadData,name,FrErrorGetHistory() ); 
                }
				
				if(type[1] == 'c')
				{
					if(!mxIsComplex(dataVector)) 
					{
						mexErrMsgIdAndTxt(errBadData,
						"%s ADC data for %s must be MATLAB double and complex\n Last errors are are:\n%s",
						errBadData,name,FrErrorGetHistory() ); 
					}
					b = FrVectNew(FR_VECT_16C, 1, numData, 1., "units"); 
					if(b == NULL)
					{
						mexErrMsgIdAndTxt(errFrameFail,
						"%s Vector for %s is null\n Last errors are:\n%s",
						errFrameFail,name,FrErrorGetHistory() );
					}
					RealdataPtr = mxGetPr(dataVector);   
					ImagdataPtr = mxGetPi(dataVector);  
													
					for(i=0; i<numData; i++) 
					{
                        b->data[2*i] = RealdataPtr[i];  /* data->data not data->dataD is populated */
                        b->data[2*i+1] = ImagdataPtr[i];
					}
				}
				else
				{
					b = FrVectNew(FR_VECT_8R, 1, numData, 1., "units");
					if(b == NULL)
					{
						mexErrMsgIdAndTxt(errFrameFail,
                        "%s Vector for %s is null\n Last errors are:\n%s",
                        errFrameFail,name,FrErrorGetHistory() );
					}
					memcpy(b->dataD,dataPtr,dataBytes);
				}
            }
            else if(type[0] == 'f')
            {
                if(!mxIsSingle(dataVector))
                {
                    mexErrMsgIdAndTxt(errBadData,
                    "%s ADC data for %s must be MATLAB single\n Last errors are are:\n%s",
                    errBadData,name,FrErrorGetHistory() ); 
                }
	            b = FrVectNew(FR_VECT_4R, 1, numData, NULL, "units");
                if(b == NULL)
                {
                    mexErrMsgIdAndTxt(errFrameFail,
                    "%s Vector for %s is null\n Last errors are:\n%s",
                    errFrameFail,name,FrErrorGetHistory() );
                }
	            memcpy(b->dataF,dataPtr,dataBytes);
            }
            else if(type[0] == 's')
            {
                if(!mxIsInt16(dataVector))
                {
                    mexErrMsgIdAndTxt(errBadData,
                    "%s ADC data for %s must be MATLAB int16\n Last errors are are:\n%s",
                    errBadData,name,FrErrorGetHistory() ); 
                }
	            b = FrVectNew(FR_VECT_2S, 1, numData, NULL, "units");
                if(b == NULL)
                {
                    mexErrMsgIdAndTxt(errFrameFail,
                    "%s Vector for %s is null\n Last errors are:\n%s", 
                    errFrameFail,name,FrErrorGetHistory() );
                }
	            memcpy(b->dataS,dataPtr,dataBytes);
            }
            else if(type[0] == 'i')
            {
                if(!mxIsInt32(dataVector))
                {
                    mexErrMsgIdAndTxt(errBadData,
                    "%s ADC data for %s must be MATLAB int32\n Last errors are are:\n%s",
                    errBadData,name,FrErrorGetHistory() ); 
                }
	            b = FrVectNew(FR_VECT_4S, 1, numData, NULL, "units");
                if(b == NULL)
                {
                    mexErrMsgIdAndTxt(errFrameFail,
                    "%s Vector for %s is null\n Last errors are:\n%s", 
                    errFrameFail,name,FrErrorGetHistory() );
                }
	            memcpy(b->dataI,dataPtr,dataBytes);
            }
            else if(type[0] == 'l')
            {
                if(!mxIsInt64(dataVector))
                {
                    mexErrMsgIdAndTxt(errBadData,
                    "%s ADC data for %s must be MATLAB int64\n Last errors are are:\n%s",
                    errBadData,name,FrErrorGetHistory() ); 
                }
	            b = FrVectNew(FR_VECT_8S, 1, numData, NULL, "units");
                if(b == NULL)
                {
                    mexErrMsgIdAndTxt(errFrameFail,
                    "%s Vector for %s is null\n Last errors are:\n%s", 
                    errFrameFail,name,FrErrorGetHistory() );
                }
	            memcpy(b->dataL,dataPtr,dataBytes);
            }
            else if(type[0] == 'u')
            {
	            if(type[1] == 's')
                {
                    if(!mxIsUint16(dataVector))
                    {
                        mexErrMsgIdAndTxt(errBadData,
                        "%s ADC data for %s must be MATLAB uint16\n Last errors are are:\n%s",
                        errBadData,name,FrErrorGetHistory() ); 
                    }
	                b = FrVectNew(FR_VECT_2U, 1, numData, NULL, "units");
                    if(b == NULL)
                    {
                        mexErrMsgIdAndTxt(errFrameFail,
                        "%s Vector for %s is null\n Last errors are:\n%s", 
                        errFrameFail,name,FrErrorGetHistory() );
                    }
	                memcpy(b->dataUS,dataPtr,dataBytes);
	            }
	            else if(type[1] == 'i')
                {
                    if(!mxIsUint32(dataVector))
                    {
                        mexErrMsgIdAndTxt(errBadData,
                        "%s ADC data for %s must be MATLAB uint32\n Last errors are are:\n%s",
                        errBadData,name,FrErrorGetHistory() ); 
                    }
	                b = FrVectNew(FR_VECT_4U, 1, numData, NULL, "units");
                    if(b == NULL)
                    {
                        mexErrMsgIdAndTxt(errFrameFail,
                        "%s Vector for %s is null\n Last errors are:\n%s", 
                        errFrameFail,name,FrErrorGetHistory() );
                    }
	                memcpy(b->dataUI,dataPtr,dataBytes);
	            }
	            else if(type[1] == 'l')
                {
	                if(!mxIsUint64(dataVector))
                    {
                        mexErrMsgIdAndTxt(errBadData,
                        "%s ADC data for %s must be MATLAB uint64\n Last errors are are:\n%s",
                        errBadData,name,FrErrorGetHistory() ); 
                    }
	                b = FrVectNew(FR_VECT_8U, 1, numData, NULL, "units");
                    if(b == NULL)
                    {
                        mexErrMsgIdAndTxt(errFrameFail,
                        "%s Vector for %s is null\n Last errors are:\n%s", 
                        errFrameFail,name,FrErrorGetHistory() );
                    }
	                memcpy(b->dataUL, dataPtr,dataBytes);
	            }
			}
			/* Add Static Data */
			StatData = FrStatDataNew("DataChannel","Created by mkframe","data",t0, t0+1, 0, b, NULL); 
            FrStatDataAdd(frame->detectProc, StatData);
       }
    }	
}
	
/*  IF params was input                      */
/*      GET number of elements               */
/*      LOOP over elements                   */
/*          GET name of element              */
/*          GET parameters array of element  */
/*          CREATE string vector             */
/*          COPY parameters to string vector */
/*          ADD name,string vector to frame  */
/*      ENDLOOP                              */
/*  ENDIF                                    */

if( nrhs > 5 )
{
    if( mxIsEmpty(prhs[5]) )
    { 
        mexPrintf("Input Comment: No parameters will be written to frame \n");
    }
    else if(!mxIsStruct(prhs[5]))
    {
        mexErrMsgIdAndTxt(errBadInput,"%s The parameters input must be a structure",errBadInput);
    }
    else
    {
	    paramElems = mxGetNumberOfElements(prhs[5]);
        if(paramElems < 1)
        {
            mexErrMsgIdAndTxt(errBadField,"%s no elements in parameters structure array",errBadField);
		}
    
    /* loop over elements */
    /* GET channel name  */
    /* GET parameters */
    /* GET number of keyword-value pairs */
    /* CREATE string vector to hold parameters */
    /* LOOP over # of parameter elements */
    /*  GET STRING for parameter element */
    /*  ALLOCATE memory for string in array */
    /*  COPY string to allocated memory */
	    for(i=0; i<paramElems; i++)
        {
            ptrName = mxGetField(prhs[5], i, "name");
		    thisName = mxArrayToString(ptrName);
            pNameStr = mxMalloc((strlen(thisName)+1)*sizeof(mxChar));
            strcpy(pNameStr,thisName);
            mxFree(thisName);

            ptrParamArray = mxGetField(prhs[5], i, "parameters");
            numParamCell = mxGetNumberOfElements(ptrParamArray);
		    numParamCol = mxGetN(ptrParamArray);
            numParamRow = mxGetM(ptrParamArray);
	 	    if( numParamCol != 2 ) 
	 	    {
                mexErrMsgIdAndTxt(errBadField,"%s params must be a 2 column array",errBadField);
		    }
		    else 
		    {
        /* note the ordering of columns and rows..MATLAB order is opposite that in C~ */
                paramV = FrVectNew(FR_VECT_STRING, 2, numParamRow, 1., "pairs", numParamCol, 1., "values");
		        if( paramV == NULL)
			    {
                    mexErrMsgIdAndTxt(errBadData,"%s Vector for %s is null\n Last errors are:\n%s",
                    errFrameFail,"params",FrErrorGetHistory() );
			    }
        /* We do this by row and column to keep the ordering straight.  MATLAB pointer 
             ordering is not like C */
                numSub = 2;
                subVal = mxCalloc(numSub,sizeof(mwSize));                
                for(j=0; j<numParamRow; j++)
			    {	
                    for(k=0; k<numParamCol; k++)
                    {
                        subVal[0] = j;
                        subVal[1] = k;                        
                        thisIdx =  mxCalcSingleSubscript(ptrParamArray, numSub, subVal);
                        thisCell = mxGetCell(ptrParamArray, thisIdx);
                        thisParam = mxArrayToString(thisCell);
                        numCharParm = strlen(thisParam);
       /* Here we use a native C malloc as we are addressing a structure created in C, not MATLAB */
                        cIdx = (int) (j*numParamCol + k);
                        paramV->dataQ[cIdx] = malloc((strlen(thisParam)*sizeof(mxChar))+1);
                        strcpy(paramV->dataQ[cIdx],thisParam);
                        mxFree(thisParam);
                    }
			    }
                mxFree(subVal);
                StatParams = FrStatDataNew(pNameStr, NULL, "names and values", t0, t0+1, 0, paramV, NULL); 
                FrStatDataAdd(frame->detectProc, StatParams);
		    }
        }
	}
}   	 

/*  IF comment was input                 */
/*      IF comment is not a string       */
/*          CREATE error and exit        */
/*      ELSE                             */
/*      Get comment string               */
/*  ENDIF                                */

if( nrhs > 6 )
{
    if( mxIsEmpty(prhs[6]) )
    { 
        comment = NULL;
        mexPrintf("Input Comment: No comment will be added to frame \n");
    }  
    else
    {
        if(!mxIsChar(prhs[6]))
        {
            mexErrMsgIdAndTxt(errBadInput,"%s The comment must be a string",errBadInput);
        }
        lenComment = (mxGetN(prhs[6])*sizeof(mxChar)) + 1;
        comment = mxMalloc(lenComment);
        mxGetString(prhs[6], comment, lenComment);  /* written to frame using FrFileOSetMsg */
        mexPrintf("MKFRAME: Add to frame the comment: %s \n",comment);
    }
}
else
{
    comment = NULL;
}
  /*-----------write frame to the output file-----------------------*/
							  
oFile = FrFileONew(fileName, compress);
	
if(oFile == NULL)
{
    mexErrMsgIdAndTxt(errFrameFail,"%s Error during output file creation.\n"
	      "Last errors are:\n%s", errFrameFail,FrErrorGetHistory() );
}
	
if(comment != NULL)
{
    FrFileOSetMsg(oFile, comment);
    mxFree(comment);
}
	
if(mode[0] == 'a')
{
    mexPrintf("MKFRAME: Write out updated frame file %s\n",fileName);
}
else
{
    mexPrintf("MKFRAME: Write out new frame file %s\n",fileName);
}
if(FrameWrite(frame, oFile) != FR_OK)
{
    mexErrMsgIdAndTxt(errFrameFail,"%s Error during frame write.\n"
	"Last errors are:\n%s", errFrameFail,FrErrorGetHistory() );
}
/*-----------close down------------------------*/
mexPrintf("MKFRAME: Write EOF \n");
FrFileOEnd(oFile);
FrameFree(frame);
mexPrintf("MKFRAME: Complete frame file writing\n");	
}
