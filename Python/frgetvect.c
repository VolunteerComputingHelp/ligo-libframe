/*
    Nick Fotopoulos (nvf@ligo.mit.edu)
    Python port of frgetvect (based on Matlab frgetvect by Benoit Mours)
    $Id: frgetvect.c,v 1.1.1.24 2013-05-24 14:59:10 mours Exp $
    
    Requires: numpy, FrameL
    To do:  Check inputs, allow multiple files, raise appropriate exceptions,
            test on complex-valued frames, make more Pythonic (use keyword
            arguments, etc)
*/

const char docstring[] = "Nick Fotopoulos (nvf@mit.edu)\n"
"Python port of frgetvect (based on Matlab frgetvect)\n"
"\n"
"The input arguments are:\n"
"   1) file name\n"
"   2) channel name (it could be an ADC, SIM or PROC channel)\n"
"   3) (optional) starting GPS time (default = first frame in the file)\n"
"   4) (optional) vector length in second (default = 1 second)\n"
"   5) (optional) debug level (default = 0 = no output)\n"
"\n"
"Returned data (in a tuple):\n"
"   1) ADC or SIM or PROC data stored as double\n"
"   2) x axis values relative to the first data point.  This is usually\n"
"       time, but it could be frequency in the case of a frequency series\n"
"   3) frequency values, in the case of time series\n"
"   4) GPS starting time (in seconds)\n"
"   5) starting time as a string\n"
"   6) vector unitX as a string\n"
"   7) vector unitY as a string\n";

#include "Python.h"
#include "numpy/arrayobject.h"
#include "FrameL.h"


static PyObject *frgetvect(PyObject *self, PyObject *args)
{
    import_array();
    
    int nrhs = 5;  // I don't actually use this at present.
    PyArrayObject *out1, *out2, *out3;
    double out4;
    char *out5, *out6, *out7;
    
    struct FrFile *iFile;
    struct FrVect *vect;	
    long  debugLvl, i, nData, utc;
    double *data, dt, df, t0, duration;
    char  *fileName, *vectName, *tempstr;
    
    npy_intp shape[1];
    int ok;
    
    /*--------------- unpack arguments --------------------*/
    /* Give bad default arguments as a way of counting how many
       arguments were passed to this function. */
    t0 = -1.;
    duration = -1.;
    debugLvl = -1;

    /* The | in the format string indicates the next arguments are
       optional.  They are simply not assigned anything. */
    ok = PyArg_ParseTuple(args, "ss|ddi", &fileName, &vectName,
                           &t0, &duration, &debugLvl);

    // Handle defaults
    if (debugLvl < 0)
    {
        debugLvl = 0;
        nrhs = 4;
    }
    if (duration < 0.)
    {
        duration = 1.;
        nrhs = 3;
    }
    if (t0 < 0.)
    {
        t0 = 0.;
        nrhs = 2;
    }
    
    FrLibSetLvl(debugLvl);
    
    /*-------------- open file --------------------------*/
    
    if (debugLvl > 0)
    {
        const char *msg = "Opening %s for channel %s (t0=%.2f, duration=%.2f).\n";
        printf(msg, fileName, vectName, t0, duration);
    }
    
    iFile = FrFileINew(fileName);
    if (iFile == NULL)
    {
        printf("%s\n", FrErrorGetHistory());
        return;
    }
    
    if (debugLvl > 0)
    {
        const char *msg = "Opened %s for channel %s!\n";
        printf(msg, fileName, vectName);
    }
    
    /*-------------- get vector --------------------------*/
    
    vect = FrFileIGetV(iFile, vectName, t0, duration);
    
    if(debugLvl > 0) FrVectDump(vect, stdout, debugLvl);
    if(vect == NULL)
    {
        printf("In file %s, vector not found: %s\n",fileName,vectName);
        FrFileIEnd(iFile);
        return;
    }
    
    if (debugLvl > 0)
    {
        const char *msg = "Extracted channel %s successfully!\n";
        printf(msg, vectName);
    }
    
    nData = vect->nData;
    dt = vect->dx[0];
    if (0.0 == dt)
    {
        printf("dt==0\n");
        df = 0.0;
    }
    else
    {
        df = 1.0/(dt*nData);
    }
    
    /*-------- copy data ------*/
    
    shape[0] = (npy_intp)nData;
    if (vect->type == FR_VECT_8C || vect->type == FR_VECT_16C) {
        out1 = (PyArrayObject *)PyArray_SimpleNew(1, shape, PyArray_CDOUBLE);
    } else {
        out1 = (PyArrayObject *)PyArray_SimpleNew(1, shape, PyArray_DOUBLE);
    }
    
    data = (double *)PyArray_DATA(out1);
    
    if (data==NULL) {
        printf("Unable to allocate space for data.\n");
        return;
    }
    
    if(vect->type == FR_VECT_2S)
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
    // Note the 2*nData in the for loop for complex types
    else if(vect->type == FR_VECT_8C)
    {for(i=0; i<2*nData; i++) {data[i] = vect->dataF[i];}}
    else if(vect->type == FR_VECT_16C)
    {for(i=0; i<2*nData; i++) {data[i] = vect->dataD[i];}}
    else
    {
        printf("No numpy type for this channel");
        FrVectFree(vect);
        FrFileIEnd(iFile);
        return;
    }
    
    /*------------- fill time and frequency array --------*/
    
    // output2 = x-axis values relative to first data point.
    // Usually time, but could be frequency in the case of a frequency
    // series
    out2 = (PyArrayObject *)PyArray_SimpleNew(1, shape, PyArray_DOUBLE);
    data = (double *)PyArray_DATA(out2);
    for(i=0; i<nData; i++) {
      data[i] = vect->startX[0]+(double)i*dt;
    }
    
    // output3 = frequency values in the case of time series
    shape[0] = nData/2;
    out3 = (PyArrayObject *)PyArray_SimpleNew(1, shape, PyArray_DOUBLE);
    data = (double *)PyArray_DATA(out2);
    for (i=0;i<nData/2;i++) {
        data[i] = (double)i*df;
    }
    
    // output4 = gps start time
    out4 = (double)vect->GTime;
    
    // output5 = gps start time as a string
    utc = vect->GTime - vect->ULeapS + FRGPSTAI;
    tempstr = (char *)malloc(200*sizeof(char));
    sprintf(tempstr,"Starting GPS time:%.1f UTC=%s",
        vect->GTime,FrStrGTime(utc));
    out5 = (PyStringObject *)PyString_FromString(tempstr);
    free(tempstr);
    
    // output6 = unitX as a string
    out6 = (PyStringObject *)PyString_FromString(vect->unitX[0]);
    
    // output7 = unitY as a string
    out7 = (PyStringObject *)PyString_FromString(vect->unitY);
    
    /*------------- clean up -----------------------------*/
    
    FrVectFree(vect);
    FrFileIEnd(iFile);
    
    return Py_BuildValue("(NNNdNNN)",out1,out2,out3,out4,out5,out6,out7);
}

static PyMethodDef frgetvectMethods[] = {
  {"frgetvect", frgetvect, METH_VARARGS, docstring},
        {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initfrgetvect(void){
    (void) Py_InitModule("frgetvect", frgetvectMethods);
}
