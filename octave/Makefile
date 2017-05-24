FRAME_INC = ../src     
FRAME_LIB = $(FRROOT)/$(UNAME)
CC = mkoctfile
CFLAGS = --verbose 

LFRAME=Frame

INCLUDE= -I$(FRAME_INC)
LIBS = -L$(FRAME_LIB) -l$(LFRAME)

all: loadadc.oct loadproc.oct saveadc.oct saveproc.oct loadframe.oct

loadframe.oct: loadframe.cc
	env LDFLAGS="-shared -Wl,-rpath,$(FRAME_LIB)" $(VER)$(CC) $(INCLUDE) $(LIBS) loadframe.cc

loadadc.oct: loadadc.cc
	env LDFLAGS="-shared -Wl,-rpath,$(FRAME_LIB)" $(VER)$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) loadadc.cc

loadproc.oct: loadproc.cc
	env LDFLAGS="-shared -Wl,-rpath,$(FRAME_LIB)" $(VER)$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) loadproc.cc

saveadc.oct: saveadc.cc
	env LDFLAGS="-shared -Wl,-rpath,$(FRAME_LIB)" $(VER)$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) saveadc.cc

saveproc.oct: saveproc.cc
	env LDFLAGS="-shared -Wl,-rpath,$(FRAME_LIB)" $(VER)$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) saveproc.cc

clean:	
	/bin/rm -f *.o *.a *~

