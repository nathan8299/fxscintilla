AC_INIT(fox/ScintillaFOX.cxx)
dnl AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR([m4])


dnl Check Cygwin and MinGW32
AC_CANONICAL_HOST
case $host_os in
  *cygwin* )  HAVE_CYGWIN=yes;;
  *mingw32* ) HAVE_MINGW32=yes;;
  * ) ;;
esac

AM_CONDITIONAL(HAVE_CYGWIN, test x"$HAVE_CYGWIN" = xyes)
AM_CONDITIONAL(HAVE_MINGW32, test x"$HAVE_MINGW32" = xyes)

dnl Using libtool
if test x"$HAVE_CYGWIN" = xyes -o x"$HAVE_MINGW32" = xyes; then
  AC_DISABLE_SHARED
fi
AM_PROG_LIBTOOL

dnl Must have a C++ compiler
AC_PROG_CXX

dnl Versioning
dnl - FXScintilla versioning scheme
MAJOR_VERSION=3
MINOR_VERSION=2
PATCH_LEVEL=3
AC_SUBST(MAJOR_VERSION)
AC_SUBST(MINOR_VERSION)
AC_SUBST(PATCH_LEVEL)
AM_INIT_AUTOMAKE(fxscintilla, "$MAJOR_VERSION.$MINOR_VERSION.$PATCH_LEVEL")

dnl - Libtool versioning scheme
dnl   current:revision:age
dnl   . start with 0:0:0
dnl   . if the source code changed, then increment revision
dnl   . if any interface has been added, removed or changed, increment
dnl     current and set revision to 0
dnl   . if backward compatibility has been preserved, then increment age
dnl   . if backward compatibility has been broken, then set age to 0
LT_FXSCINTILLA_CURRENT=24
LT_FXSCINTILLA_REVISION=3
LT_FXSCINTILLA_AGE=2
AC_SUBST(LT_FXSCINTILLA_CURRENT)
AC_SUBST(LT_FXSCINTILLA_REVISION)
AC_SUBST(LT_FXSCINTILLA_AGE)

LT_FXSCINTILLANOLEXER_CURRENT=21
LT_FXSCINTILLANOLEXER_REVISION=0
LT_FXSCINTILLANOLEXER_AGE=1
AC_SUBST(LT_FXSCINTILLANOLEXER_CURRENT)
AC_SUBST(LT_FXSCINTILLANOLEXER_REVISION)
AC_SUBST(LT_FXSCINTILLANOLEXER_AGE)

dnl Enabling libfxscintillanolexer
AC_MSG_CHECKING(for nolexer library build)
AC_ARG_ENABLE(nolexer, [  --enable-nolexer        enable libfxscintillanolexer [default=no]],,
              enable_nolexer=no)
AC_MSG_RESULT([$enable_nolexer])
AM_CONDITIONAL(NOLEXER, test x"$enable_nolexer" = xyes)

dnl Check FOX library
PKG_CHECK_MODULES([FOX], [fox >= 1.7], [AC_DEFINE(FOX_1_7)], [AC_DEFINE(FOX_1_6)])
PKG_CHECK_MODULES([FOX], [fox >= 1.6])
AC_SUBST(FOX_CFLAGS)
AC_SUBST(FOX_LIBS)


dnl Generated makefiles
AC_OUTPUT( \
  lexers/Makefile \
  lexlib/Makefile \
  src/Makefile \
  include/Makefile \
  fox/Makefile \
  fox/version.h \
  tests/Makefile \
  Makefile \
  fxscintilla.pc)