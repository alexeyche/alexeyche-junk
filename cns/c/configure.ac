#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([snn_sim], [0.1], [alexey.chernushev@gmail.com])
AC_CONFIG_SRCDIR([snn_sim/])
AM_INIT_AUTOMAKE([subdir-objects])
#AC_CONFIG_HEADERS([config.h])
# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h netinet/in.h stddef.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([inet_ntoa socket])

LT_INIT

AC_OUTPUT(Makefile snn_sim/Makefile)
