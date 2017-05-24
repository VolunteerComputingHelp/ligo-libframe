// saveproc.cc -- write data in Frame files from Octave.

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
// mkoctfile --verbose -L[path_to_Framelib] -lFrame saveproc.cc
//
// at the shell prompt.  The script mkoctfile should have been
// installed along with Octave.  Running it will create a file called
// saveproc.oct that can be loaded by Octave.
//
// Enter:
//
// help saveproc
//
// at the Octave prompt to have a description of how saveproc should be used.  
// 
// This file may be used under the terms defined by the GNU 
// General Public License [write to the Free Software Foundation, 
// 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA]
//
// WITHOUT ANY WARRANTY; without even the implied warranty 
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
//
// Author: Eric Chassande-Mottin [ecm@obs-nice.fr], Thu May  2 14:58:37 GMT 2002
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


DEFUN_DLD (saveproc, args, ,
  "SAVEPROC: Write an PROC signal from the Octave workspace to a given frame file.\n\
Usage:\n\
status=saveproc(fileName,signalName,data[,fs,[t0]])\n\
Input parameters:\n\
fileName: the name of the output frame file\n\
signalName: name of the PROC signal to be written\n\
data: input data (column vector of double)\n\
fs: sampling frequency\n\
t0: GPS time associated to the first bin of the first output frame\n\
Output parameters:\n\
status:  report about the writing operation.")
{
  octave_value_list retval;
  
  /*----------- get input parameters ----------------*/
  int nargin = args.length ();
  
  if (nargin < 3)
    {
      // print_usage("saveproc");
      error("Usage: status=saveproc(fileName,signalName,signal[,fs,[t0]])");
      return retval;
    }

  /* get the name of the output file */  
  string bufferfName=args(0).string_value();
  char* fileName;
  fileName = new char[ bufferfName.size() + 1 ];
  strcpy( fileName, bufferfName.c_str() );
  // printf("%s\n",fileName);

  /* get the name of the signal */
  string bufferprocName=args(1).string_value();
  char* procName;
  procName = new char[ bufferprocName.size() + 1 ];
  strcpy( procName, bufferprocName.c_str() );
  // printf("proc= %s\n",procName);

  /* get the sampling frequency */
  int fs;
  if (nargin<4)
    fs=20000;
  else  
    fs=(int) args(3).double_value();
  //    printf("fs= %d\n",fs);
  
  /* get the starting GPS time */
  int t0=0;
  if (nargin==5)  
    t0=(int) args(4).double_value();
  //    printf("t0= %d\n",t0);
  
  /* get the data */
  //ColumnVector data = args(2).vector_value ();
  ColumnVector data(args(2).vector_value ());

  /* get the size of the data */      
  int n=data.length();
  //  printf("n= %d\n",n);

  /* open file */
  
  struct FrFile *oFile;  
  oFile = FrFileONew(fileName,6);
  
  if (oFile == NULL)
    {
      error("saveproc: cannot open output frame file");
      return retval;
    }

  /* while data available */
  int i=0; /* running index in file */
  int j=0; /* running index in frame */
  int k=0; /* number of written frames */  
  int stat=0; /* status */

  while (i<n)
    {

      /* make new frame */
      struct FrameH *frame;
      frame = FrameHNew("frame created from Octave");
      frame->dt = 1.0; // 1 frame = 1 sec.  
      frame->run=1;
      frame->frame=k;
      frame->GTimeS=t0+k;

      /* make new proc */
      struct FrProcData *proc;
      proc = FrProcDataNew(frame,procName,fs,fs,-64);
      // proc = FrProcDataNew(frame,"Pr_B1p_ACq",fs,fs,-64);

      j=0;

      /* copy data to frame */
      while(j<fs)
	{
	  proc->data->dataD[j]=(double) data(i);
	  i++;j++;
	  if (i==n) break;
	}

      if (j<(fs-1))
	while (j<fs)
	  {
	    proc->data->dataD[j]=0.0;
	    j++;
	  }

      /* write frame */
      stat=FrameWrite(frame,oFile);
      k++;

      //        scanf("press a key",&dum);
      //        printf("\n end copy\n");
      // printf("frame k= %d\n",k);
	  
      if (stat!=FR_OK)
	{
	  error("saveproc: error occured during frame write\n"); 
	  double status = (double) stat;
	  retval(0)=status;
	  return retval;
	}

      /* free frame */      
      FrameFree(frame);
      
    }
  
  octave_stdout << "saveproc wrote " << k << " frames in " << fileName << endl;
  
  delete [] fileName;
  delete [] procName;

  /* ------------- Close file and release descriptor ----------*/
  
  FrFileOEnd(oFile);    

  /* -------------- set up and send return values -------------*/  

  double status = (double) stat;
  retval(0)=status;  
  return retval;
}




