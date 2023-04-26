AC_INIT([wrmilgateway], [1.0])
LT_INIT
AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX([11], [noext], [mandatory])

PKG_CHECK_MODULES([SAFTBUS], [saftbus])
PKG_CHECK_MODULES([SAFTLIB], [saftlib])
PKG_CHECK_MODULES([SIGCPP], [sigc++-2.0])
AC_CHECK_PROG( SAFTBUSGEN_CHECK,saftbus-gen,yes)
AS_IF([test x"$SAFTBUSGEN_CHECK" != x"yes"], [AC_MSG_ERROR([saftbus-gen not found. You can build and install it from saftbus/saftbus-gen directory])])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT