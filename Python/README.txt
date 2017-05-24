Python frgetvect
----------------
This module is intended to allow access to GWF frame files in Python.  It
interfaces the FrameL library.

Written by Nick Fotopoulos (nvf@ligo.mit.edu)


Installation instructions 
-------------------------

0. Go in the Python subdirectory

1. Compile and install FrameL:
   ../mgr/makegcc.

2. Include FrameL in LD_LIBRARY_PATH:
   setenv LD_LIBRARY_PATH ../${UNAME}:${LD_LIBRARY_PATH}

3. Compile the python code after checking FrameLlibdir in the setup files:
   python setup.py  build   # for the old frgetvect.
   python setupFr.py  build   # for the new Fr.c

4. Include the library in PYTHONPATH:
   setenv PYTHONPATH build/lib.linux-x86_64-2.5/:$PYTHONPATH

5. Run the test script:
   python test.py 
