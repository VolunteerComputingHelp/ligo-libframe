# preamble
AC_INIT([libframe],[8.30],[daswg@ligo.org])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_AUX_DIR([gnuscripts])
AC_CONFIG_MACRO_DIR([gnuscripts])
AC_CONFIG_FILES([Makefile \
                 libframe.spec \
                 doc/Makefile \
                 src/Makefile \
                 src/libframe.pc \
                 src/zlib/Makefile \
                 matlab/Makefile])
AM_INIT_AUTOMAKE([foreign])
AC_CANONICAL_HOST

# set library api info
# 1. start with version information of "0:0:0" for each libtool library
# 2. update the version information only immediately before a public
#    release of your software. More frequent updates are unnecessary,
#    and only guarantee that the current interface number gets larger
#    faster.
# 3. if the library source code has changed at all since the last
#    update, then increment revision ("c:r:a" becomes "c:r+1:a").
# 4. if any interfaces have been added, removed, or changed since the
#    last update, increment current, and set revision to 0.
# 5. if any interfaces have been added since the last public release,
#    then increment age.
# 6. if any interfaces have been removed since the last public release,
#    then set age to 0.
AC_SUBST([LIBCURRENT], [6])
AC_SUBST([LIBREVISION], [0])
AC_SUBST([LIBAGE], [5])
AC_SUBST([LIBVERSION], [${LIBCURRENT}:${LIBREVISION}:${LIBAGE}])

# pkg-config module path
AC_SUBST([pkgconfigdir], [${libdir}/pkgconfig])

# program checks
AC_PROG_CC_STDC

# initialise libtool
LT_INIT

# is -DFR_LONG_LONG needed
AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([long long])
if test $ac_cv_sizeof_long == "8" ; then
  FR_LONG_LONG_FLAG=
  AC_MSG_RESULT([using long as 8-byte integer type])
elif test $ac_cv_sizeof_long_long == "8" ; then
  FR_LONG_LONG_FLAG=-DFR_LONG_LONG
  AC_MSG_RESULT([using long long as 8-byte integer type])
else
  AC_MSG_ERROR([libframe requires a platform on which either long or long long is 8 bytes])
fi
AC_SUBST([FR_LONG_LONG_FLAG])

# add -fno-strict-aliasing for gcc compilers
if test x$GCC = xyes ; then
  CFLAGS="$CFLAGS -fno-strict-aliasing"
fi
AC_SUBST(CFLAGS)

# check for required libraries
AC_CHECK_LIB([m], [main], , [AC_MSG_ERROR([cannot find the math library])])

# generate output
AC_PROG_MAKE_SET
AC_OUTPUT
