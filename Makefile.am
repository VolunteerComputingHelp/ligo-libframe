ACLOCAL_AMFLAGS = -I gnuscripts
EXTRA_DIST = LICENSE data debian libframe.spec mgr Python root octave cmt
SUBDIRS = doc src matlab

maintainer-clean-local:
	-rm -f Makefile.in
	-rm -f aclocal.m4
	-rm -f configure
	-rm -f doc/Makefile.in
	-rm -f src/Makefile.in
	-rm -f src/config.h.in
	-rm -f src/zlib/Makefile.in
	-rm -f matlab/Makefile.in

release:
	-$(MAKE) clean
	./configure --prefix=/opt/lscsoft/libframe
	$(MAKE) dist
