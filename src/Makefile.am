SUBDIRS = zlib
AM_CPPFLAGS = -Izlib
AM_CFLAGS = $(FR_LONG_LONG_FLAG)
include_HEADERS = FrameL.h FrFilter.h FrIO.h FrVect.h
noinst_LTLIBRARIES = libFrame0.la
lib_LTLIBRARIES = libFrame.la
bin_PROGRAMS = FrCheck FrCopy FrDump FrDiff FrChannels FrameDataDump FrTrend
noinst_PROGRAMS = \
	FrComplex \
	FrCompress \
	FrCopyFile \
	FrCopyFrame \
	FrEvent \
	FrFilter \
	FrFileDump \
	FrFull \
	FrMark \
	FrMultiR \
	FrMultiTOC \
	FrMultiW \
	FrMultiWD \
	FrOnline \
	FrReshape \
	FrSelectFrame \
	FrSpeed \
	FrStat \
	FrDiff \
	FrChannels \
	FrameDataDump \
	FrTrend
libFrame0_la_SOURCES = FrameL.c FrIO.c FrFilter.c
libFrame_la_SOURCES =
libFrame_la_DEPENDENCIES = libFrame0.la
libFrame_la_LIBADD = libFrame0.la zlib/libzlib.la
libFrame_la_LDFLAGS = -version-info $(LIBVERSION)

FrCheck_SOURCES = FrCheck.c
FrCopy_SOURCES = FrCopy.c
FrDump_SOURCES = FrDump.c

FrComplex_SOURCES = exampleComplex.c
FrCompress_SOURCES = exampleCompress.c
FrCopyFile_SOURCES = exampleCopyFile.c
FrCopyFrame_SOURCES = exampleCopyFrame.c
FrEvent_SOURCES = exampleEvent.c
FrFileDump_SOURCES = exampleFileDump.c
FrFilter_SOURCES = exampleFilter.c
FrFull_SOURCES = exampleFull.c
FrMark_SOURCES = exampleMark.c
FrMultiR_SOURCES = exampleMultiR.c
FrMultiTOC_SOURCES = exampleMultiTOC.c
FrMultiW_SOURCES = exampleMultiW.c
FrMultiWD_SOURCES = exampleMultiWD.c
FrOnline_SOURCES = exampleOnline.c
FrReshape_SOURCES = exampleReshape.c
FrSelectFrame_SOURCES = exampleSelectFrame.c
FrSpeed_SOURCES = exampleSpeed.c
FrStat_SOURCES = exampleStat.c

FrDiff_SOURCES = FrDiff.c
FrChannels_SOURCES = FrChannels.c
FrameDataDump_SOURCES = FrameDataDump.c
FrTrend_SOURCES = FrTrend.c

LDADD = libFrame.la

pkgconfig_DATA = libframe.pc
