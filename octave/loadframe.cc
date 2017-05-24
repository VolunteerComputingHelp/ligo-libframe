
// loadframe.cc -- load data from Frame files in Octave.
// Fast loading provided the Frame file is complete and correctly set up.
// Use fast loading functions of the frame lib.

// To use this file, your version of Octave must support dynamic
// linking.  To find out if it does, type the command
//
//   x = octave_config_info; x.DEFS
//
// at the Octave prompt.  Support for dynamic linking is included if
// the output contains the string -DWITH_DYNAMIC_LINKING=1.
//
// To compile this file, type the command
//
// mkoctfile --verbose -L[path_to_Framelib] -lFrame loadframe.cc
//
// at the shell prompt.  The script mkoctfile should have been
// installed along with Octave.  Running it will create a file called
// loadframe.oct that can be loaded by Octave.
//
// Enter:
//
// help loadframe
//
// at the Octave prompt to have a description of how loadframe should be used.  
// 
// This file may be used under the terms defined by the GNU 
// General Public License [write to the Free Software Foundation, 
// 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA]
//
// WITHOUT ANY WARRANTY; without even the implied warranty 
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
//
// Author: Eric Chassande-Mottin [ecm@obs-nice.fr], Sun Oct 14 00:48:45 CEST 2001
//

#include <octave/config.h>

#include <iostream>
#include <sstream>
using namespace std;
#include <string>

#include <octave/defun-dld.h>
#include <octave/error.h>
#include <octave/oct-obj.h>
#include <octave/pager.h>
#include <octave/symtab.h>
#include <octave/variables.h>

#include "FrameL.h"


DEFUN_DLD (loadframe, args, ,
  "LOADFRAME: Download an ADC signal in the Octave workspace from a given frame file.\n\n\
LOADFRAME is much faster than LOADADC but requires the frame file to be complete and correctly\n\
written (i.e., LOADFRAME is less robust than LOADADC).\n\n\
Usage:\n\n\
[adc,fs,valid,t0,timegps,unit,slope,bias] = loadframe (fileName,adcName[,nFrames[,GPSfirst]])\n\n\
Input parameters:\n\
fileName: the name of the frame file\n\
adcName:  the name of the ADC signal to be extr.\n\
nFrames:  [opt] the number of frames to be extr. [default=read data as much as possible]\n\
GPSfirst: [opt] GPS time of the first frame to be extr. [default=first frame of file]\n\n\
Output parameters:\n\
adc:      the ADC signal\n\
fs:       the sampling frequency\n\
valid:    an index specifying whether the data are OK or not\n\
t0:       the GPS time associated to the first bin in the first extracted frame\n\
timegps:  [string] same thing but human readable format\n\
unit:     physical units of the signal\n\
slope:    slope coef. used to calibrate the signal X\n\
bias:     bias coef. used to calibrate the signal X")
{
  octave_value_list retval;
 
  /*----------- get input parameters ----------------*/
  
  int nargin = args.length ();
  
  if (nargin < 2)
    {
      error("Usage: [adc,fs,valid,t0,timegps,unit,slope,bias] = loadframe(fileName,adcName[,nFrames[,GPSfirst]])");
      return retval;
    }
  
  string bufferfName=args(0).string_value();
  char* fileName;
  fileName = new char[ bufferfName.size() + 1 ];
  strcpy( fileName, bufferfName.c_str() );
  
  string bufferadcName=args(1).string_value();
  char* adcName; 
  adcName = new char[ bufferadcName.size() + 1 ];
  strcpy( adcName, bufferadcName.c_str() );
  
  /*----------- open file --------------------------*/
  
  struct FrFile *iFile;  
  iFile = FrFileINew(fileName);
  
  if (iFile == NULL)
    {
      error("loadframe: cannot open frame file");
      return retval;
    }
  
  /*----------------- get the TOC ------------------*/
  struct FrTOC *toc; 
  toc = FrTOCReadFull(iFile);
  if (toc==NULL)
    {
      FrFileIEnd(iFile);
      error("loadframe: cannot read TOC");
      return retval;
    }
  int nFrameFile = toc->nFrame;
  int gpsTime = *toc->GTimeS;

  /* ------------ get number of frames -------------*/
  int nFrames=nFrameFile;
  if (nargin>2)
    nFrames=args(2).int_value();

  /* ------------ get starting GPS time ------------*/  
  int GPSfirst=gpsTime;
  if (nargin==4)
    GPSfirst=args(3).int_value();
  
  if ((GPSfirst<gpsTime)||(GPSfirst>(gpsTime+nFrameFile)))
    {
      error("loadframe: cannot access required GPS time in this file");
      return retval;
    }
  
  if (nFrames>(nFrameFile-gpsTime+GPSfirst))
    {
      error("loadframe: number of required frames too large");
      return retval;
    }

  struct FrameH *frame=FrameReadTAdc(iFile,GPSfirst,adcName);
  int countframe=1;

  /*------ find ADC and read context infos  ---------*/
  
  struct FrAdcData *adc;
  adc = FrAdcDataFind(frame,adcName);
  
  if (adc == NULL) 
    {
      error("loadframe: ADC signal not found in frame");
      return retval;
    }

  double fs = adc->sampleRate;
  double t0=(double)frame->GTimeS + 1.e-9*(double)frame->GTimeN;
 
  const time_t tp = frame->GTimeS + 315961200;
  std::string bufferTime (ctime(&tp),24);
  std::ostringstream microsec;
  microsec << frame->GTimeN/1000 << " usec" << ends;
  std::string timegps = "Starting GPS time: " + bufferTime 
    +  " (and " + microsec.str() + ")"; 

  std::string unit; 
  if(adc->units != NULL)
    {
      std::string bufferUnit(adc->units);
      unit=bufferUnit;
    }
  else
    {
      std::string bufferUnit("no unit");
      unit=bufferUnit;
    }
  
  double bias = (double) adc->bias;
  double slope = (double) adc->slope;
  double valid = (double) adc->dataValid; 

  retval(1)=fs;
  retval(2)=valid;
  retval(3)=t0;
  retval(4)=timegps;
  retval(5)= unit; 
  retval(6)= slope; 
  retval(7)= bias; 
  
  /* ----------------  read ADC data ----------------*/
  
  int index=0, i=0;
  struct FrVect *vect;
  
  int nData = adc->data->nData;
  int nTot = nData*nFrames;
  
  ColumnVector data(nTot);
  
  while (index<nTot)
    {
      vect = adc->data;
      if (vect == NULL) 
	{
	  error("loadframe: corrupted frame in file");
	  return retval;
	}
      else if(vect->type == FR_VECT_C)
	{for(i=0; i<nData; i++) {data(i+index) = (double) vect->data[i];}}
      else if(vect->type == FR_VECT_2S)
	{for(i=0; i<nData; i++) {data(i+index) = (double) vect->dataS[i];}}
      else if(vect->type == FR_VECT_4S)
	{for(i=0; i<nData; i++) {data(i+index) = (double) vect->dataI[i];}}
      else if(vect->type == FR_VECT_1U)
	{for(i=0; i<nData; i++) {data(i+index) = (double) vect->dataU[i];}}
      else if(vect->type == FR_VECT_2U)
	{for(i=0; i<nData; i++) {data(i+index) = (double) vect->dataUS[i];}}
      else if(vect->type == FR_VECT_4U)
	{for(i=0; i<nData; i++) {data(i+index) = (double) vect->dataUI[i];}}
      else if(vect->type == FR_VECT_4R)
	{for(i=0; i<nData; i++) {data(i+index) = (double) vect->dataF[i];}}
      else if(vect->type == FR_VECT_8R)
	{for(i=0; i<nData; i++) {data(i+index) = (double) vect->dataD[i];}}
      index += nData;

      FrameFree(frame);
      
      if (index<nTot)
	{
	  /* ----   read next frame ----- */
	  frame = FrameReadTAdc(iFile,GPSfirst+countframe,adcName);
	  if (frame == NULL)
	    {		  
	      error("loadframe: cannot read frame");
	      return retval;
	    }
	  countframe++;
	  adc = FrAdcDataFind(frame,adcName);
	  if (adc == NULL)
	    { 
	      error("loadframe: ADC signal not found in frame");
	      return retval;
	    }
	}
      
    }
  
  /* ------------- Close file and release descriptor ----------*/
  
  FrFileIEnd(iFile); 
  
  delete [] fileName;
  delete [] adcName;

  /* ------------- set up and send return values -------------*/
  
  retval(0)=data;
  
  return retval;
}









