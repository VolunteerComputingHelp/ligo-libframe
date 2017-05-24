// loadproc.cc -- load data from Frame files in Octave.

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
// mkoctfile --verbose -L[path_to_Framelib] -lFrame loadproc.cc
//
// at the shell prompt.  The script mkoctfile should have been
// installed along with Octave.  Running it will create a file called
// loadproc.oct that can be loaded by Octave.
//
// Enter:
//
// help loadproc
//
// at the Octave prompt to have a description of how loadproc should be used.  
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


DEFUN_DLD (loadproc, args, ,
  "LOADPROC: Download an PROC signal in the Octave workspace from a given frame file.\n\
Usage:\n\
[proc,fs,t0,timegps] = loadproc (fileName,[procName[,nFrames[,first]]])\n\
Input parameters:\n\
fileName: the name of the frame file\n\
procName:  [opt] the name of the PROC signal to be extr. [if missing: send a dump of fileName]\n\
nFrames:  [opt] the number of frames to be extr. [if missing: send a dump of procName frames]\n\
first:    [opt] number of the first frame to be extr. [default=first frame avail.]\n\
Output parameters:\n\
proc:      the PROC signal\n\
fs:       the sampling frequency\n\
t0:       the GPS time associated to the first bin in the first extracted frame\n\
timegps:  [string] same thing but human readable format")
{
  octave_value_list retval;
  
  /*----------- get input parameters ----------------*/
  int nargin = args.length ();
  
  if (nargin < 1)
    {
      // print_usage("loadproc");
      error("Usage: [proc,fs,valid,t0,timegps,unit,slope,bias] = loadproc(fileName,procName[,nFrames[,first]])");
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
      error("loadproc: cannot open frame file");
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
      error("loadproc: frame file corrupted\n"); 
      return retval;
    }
  
  if (nargin==4)
    {
      /*--------- fast forward to the first frame if requested --------------*/
      
      unsigned int first=args(3).int_value();
      octave_stdout << "skeeping data from ";
      octave_stdout << "Run #" << frame->run;
      octave_stdout << ", frame #" << frame->frame;
      octave_stdout << " @ GPStime=" << frame->GTimeS << " [...]" << endl;
      
      if (frame->frame > first)
	{
	  error("loadproc: first frame requested not available in given frame file"); 
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
		  error("loadproc: EOF reached and first frame not found"); 
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
  octave_stdout << " @ GPStime=" << frame->GTimeS ;
  
  /*------ find PROC and read context infos  ---------*/
  
  struct FrProcData *proc;
  
  string bufferprocName=args(1).string_value();
  char* procName; 
  procName = new char[ bufferprocName.size() + 1 ];
  strcpy( procName, bufferprocName.c_str() );
  // printf("%s\n",procName);

  proc = FrProcDataFind(frame,procName);
  
  if (proc == NULL) 
    {
      error("loadproc: PROC signal not found in frame");
      return retval;
    }
  
  if (nargin==2)
    {
      /*---------- make a PROC dump ------------------*/
      octave_stdout << endl;
      fprintf(stdout,"Dump of PROC contents in the first frame of input file:\n");
      FrProcDataDump(proc,stdout,2);
      return retval;
    }
  else
    {
      octave_stdout << " [...]" << endl;
    }
  
  /* JMT- double fs = proc->sampleRate; */
  /* JMT( */
  double fs = 1./(proc->data->dx[0]);
  /* JMT) */
  double t0=(double)frame->GTimeS + 1.e-9*(double)frame->GTimeN;
  
  const time_t tp = frame->GTimeS + 315961200;
  std::string bufferTime (ctime(&tp),24);
  std::ostringstream microsec;
  microsec << frame->GTimeN/1000 << " usec" << ends;
  std::string timegps = "Starting GPS time: " + bufferTime 
    +  " (and " + microsec.str() + ")"; 
    
  retval(1)=fs;
  retval(2)=t0;
  retval(3)=timegps;
  
  /* ----------------  read PROC data ----------------*/
  
  int index=0, i=0;
  struct FrVect *vect;
  
  int nFrames=(int) args(2).double_value();
  if (nFrames<=0)
    {
      error("loadproc: nFrames must be a positive integer"); 
      return retval;
    }
    
  // nFrames=iFile->nFrames;
  int nData = proc->data->nData;
  int nTot = nData*nFrames;
  unsigned int last_frame=0;
  
  ColumnVector data(nTot);

  while (1)
    {
      // uncomment to be verbose
      //        octave_stdout << "reading Run #" << frame->run;
      //        octave_stdout << ", frame #" << frame->frame;
      //        octave_stdout << " @ GPStime=" << frame->GTimeS << endl;
      
      vect = proc->data;
      if (vect == NULL) 
	{
	  error("loadproc: corrupted frame in file");
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
	    error("loadproc: unknown data type");
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
	  error("loadproc: EOF reached before expected");
	  return retval;
	}
      
      /* ---- check for missing frames ---- */
      
      if (frame->frame!=(last_frame+1))
	octave_stdout << "loadproc: warning! missing data between frame #" << last_frame << "and frame #" << frame->frame << endl;
      
      /* ---- check if PROC are OK ---- */
      
      proc = FrProcDataFind(frame,procName);
      if (proc == NULL)
	{ 
	  error("loadproc: PROC signal not found in frame");
	  return retval;
	}
            
    } // while (1)

  octave_stdout << "          [...] to Run #" << frame->run;
  octave_stdout << ", frame #" << frame->frame;
  octave_stdout << " @ GPStime=" << frame->GTimeS << endl;

  FrameFree(frame);
  
  delete [] fileName;
  delete [] procName;

  /* ------------- Close file and release descriptor ----------*/
  
  FrFileIEnd(iFile);    
  
  /* ------------- set up and send return values -------------*/
  
  retval(0)=data;
  
  return retval;
}


