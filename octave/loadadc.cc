// loadadc.cc -- load data from Frame files in Octave.

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
// mkoctfile --verbose -L[path_to_Framelib] -lFrame loadadc.cc
//
// at the shell prompt.  The script mkoctfile should have been
// installed along with Octave.  Running it will create a file called
// loadadc.oct that can be loaded by Octave.
//
// Enter:
//
// help loadadc
//
// at the Octave prompt to have a description of how loadadc should be used.  
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


DEFUN_DLD (loadadc, args, ,
  "LOADADC: Download an ADC signal in the Octave workspace from a given frame file.\n\
Usage:\n\
[adc,fs,valid,t0,timegps,unit,slope,bias] = loadadc (fileName,[adcName[,nFrames[,first]]])\n\
Input parameters:\n\
fileName: the name of the frame file\n\
adcName:  [opt] the name of the ADC signal to be extr. [if missing: send a dump of fileName]\n\
nFrames:  [opt] the number of frames to be extr. [if missing: send a dump of adcName frames]\n\
first:    [opt] number of the first frame to be extr. [default=first frame avail.]\n\
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
  
  if (nargin < 1)
    {
      // print_usage("loadadc");
      error("Usage: [adc,fs,valid,t0,timegps,unit,slope,bias] = loadadc(fileName,adcName[,nFrames[,first]])");
      return retval;
    }
  
  string bufferfName=args(0).string_value();
  char* fileName;
  fileName = new char[ bufferfName.size() + 1 ];
  strcpy( fileName, bufferfName.c_str() );
  // printf("%s\n",fileName);
  
  /*----------- open file --------------------------*/
  
  struct FrFile *iFile;  
  iFile = FrFileINew(fileName);
  
  if (iFile == NULL)
    {
      error("loadadc: cannot open frame file");
      return retval;
    }
  
  if (nargin==1)
    {
      /*---------- make a file dump ------------------*/
      FrFileIDump(iFile,stdout,4,"*");
      return retval;
    }

  /*--------- read the first frame ------------------*/
  
  struct FrameH *frame;
  frame = FrameRead(iFile);
  
  if (frame == NULL)
    {
      error("loadadc: frame file corrupted\n"); 
      return retval;
    }
  
  if (nargin==4)
    {
      /*--------- fast forward to the first frame if requested --------------*/
      
      unsigned int first= args(3).int_value();
      octave_stdout << "skeeping data from ";
      octave_stdout << "Run #" << frame->run;
      octave_stdout << ", frame #" << frame->frame;
      octave_stdout << " @ GPStime=" << frame->GTimeS << " [...]" << endl;
      
      if (frame->frame > first)
	{
	  error("loadadc: first frame requested not available in given frame file"); 
	  return retval;
	}
      else
	{
	  while(frame->frame!=first)
	    {
	      FrameFree(frame);
	      frame = FrameRead(iFile);
	      if (frame == NULL)
		{
		  error("loadadc: EOF reached and first frame not found"); 
		  return retval;
		}
	    }
	}

      octave_stdout << "          [...] to Run #" << frame->run;
      octave_stdout << ", frame #" << frame->frame;
      octave_stdout << " @ GPStime=" << frame->GTimeS << endl << endl;
    } 

  octave_stdout << " reading data from Run #" << frame->run;
  octave_stdout << ", frame #" << frame->frame;
  octave_stdout << " @ GPStime=" << frame->GTimeS;  
  
  /*------ find ADC and read context infos  ---------*/
  
  struct FrAdcData *adc;
  
  string bufferadcName=args(1).string_value();
  char* adcName; 
  adcName = new char[ bufferadcName.size() + 1 ];
  strcpy( adcName, bufferadcName.c_str() );
  // printf("%s\n",adcName);

  adc = FrAdcDataFind(frame,adcName);
  
  if (adc == NULL) 
    {
      error("loadadc: ADC signal not found in frame");
      return retval;
    }
  
  if (nargin==2)
    {
      /*---------- make a ADC dump ------------------*/
      octave_stdout << endl;
      fprintf(stdout,"Dump of ADC contents in the first frame of input file:\n");
      FrAdcDataDump(adc,stdout,2);
      return retval;
    }
  else
    {
      octave_stdout << " [...]" << endl;
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
  
  int nFrames=(int) args(2).double_value();
  if (nFrames<=0)
    {
      error("loadadc: nFrames must be a positive integer"); 
      return retval;
    }
    
  // nFrames=iFile->nFrames;
  int nData = adc->data->nData;
  int nTot = nData*nFrames;
  unsigned int last_frame=0;
  
  ColumnVector data(nTot);
  
  while (1)
    {
      // uncomment to be verbose
      // octave_stdout << "reading Run #" << frame->run;
      // octave_stdout << ", frame #" << frame->frame;
      // octave_stdout << " @ GPStime=" << frame->GTimeS << endl;
      
      vect = adc->data;
      if (vect == NULL) 
	{
	  error("loadadc: corrupted frame in file");
	  return retval;
	}
      
      switch (vect->type)
	{
	case FR_VECT_C:
	  {for(i=0; i<nData; i++) {data(i+index) = (double) vect->data[i];}} break;
	case FR_VECT_2S:
	  {for(i=0; i<nData; i++) {data(i+index) = (double) vect->dataS[i];}} break;
	case FR_VECT_4S:
	  {for(i=0; i<nData; i++) {data(i+index) = (double) vect->dataI[i];}} break;
	case FR_VECT_1U:
	  {for(i=0; i<nData; i++) {data(i+index) = (double) vect->dataU[i];}} break;
	case FR_VECT_2U:
	  {for(i=0; i<nData; i++) {data(i+index) = (double) vect->dataUS[i];}} break;
	case FR_VECT_4U:
	  {for(i=0; i<nData; i++) {data(i+index) = (double) vect->dataUI[i];}} break;
	case FR_VECT_4R:
	  {for(i=0; i<nData; i++) {data(i+index) = (double) vect->dataF[i];}} break;
	case FR_VECT_8R:
	  {for(i=0; i<nData; i++) {data(i+index) = (double) vect->dataD[i];}} break;
	default:
	  {		      
	    error("loadadc: unknown data type");
	    return retval;
	      }
	}
      index += nData;
      
      if (index>=nTot)
	break;

      last_frame=frame->frame;
      FrameFree(frame);
      
      /* ----   read next frame ----- */
      
      frame = FrameRead(iFile);
      
      /* ----   check if data are OK  ----- */
      
      if (frame == NULL)
	{		  
	  error("loadadc: EOF reached before expected");
	  return retval;
	}
      
      /* ---- check for missing frames ---- */
      
      if (frame->frame!=(last_frame+1))
	octave_stdout << "loadadc: warning! missing data between frame #" << last_frame << "and frame #" << frame->frame << endl;
      
      /* ---- check if ADC are OK ---- */
      
      adc = FrAdcDataFind(frame,adcName);
      if (adc == NULL)
	{ 
	  error("loadadc: ADC signal not found in frame");
	  return retval;
	}
      
      /* ---- check for non valid data ---- */
      
      if (adc->dataValid)
	octave_stdout << "loadadc: warning! bad data in frame #" << frame->frame << endl;

    } // while (1)
  
  octave_stdout << "          [...] to Run #" << frame->run;
  octave_stdout << ", frame #" << frame->frame;
  octave_stdout << " @ GPStime=" << frame->GTimeS << endl;

  FrameFree(frame);

  delete [] fileName;
  delete [] adcName;

  /* ------------- Close file and release descriptor ----------*/
  
  FrFileIEnd(iFile);    
  
  /* ------------- set up and send return values -------------*/
  
  retval(0)=data;
      
  return retval;
}

