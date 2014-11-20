AC_INIT([Rsnn], [1.0])

AC_PREFIX_PROGRAM(snn)


#Set optional include path with option --with-snn-include or
#environment variable SNN_INCLUDE

AC_ARG_WITH([snn-include],
            AC_HELP_STRING([--with-snn-include=INCLUDE_PATH],
                           [the location of SNN header files]),
            [snn_include_path=$withval])
if test [ -n "$snn_include_path" ] ; then
   SNN_INCLUDE=${snn_include_path}
else
   if test [ -z "${SNN_INCLUDE}" ] ; then
      if test [ "$prefix" = "NONE" ]; then
         AC_MSG_ERROR("Location of SNN headers not defined. Use configure arg '--with-snn-include' or environment variable 'SNN_INCLUDE'")
      else
         SNN_INCLUDE=${prefix}/include/SNN
      fi
   fi
fi

#AC_LANG_PUSH(C++)
#OCPPFLAGS=${CPPFLAGS}
#CPPFLAGS=-I${SNN_INCLUDE}
#AC_CHECK_HEADER(Console.h, [],
#             [AC_MSG_FAILURE("Problem with header file ${SNN_INCLUDE}/Console.h ")])
#CPPFLAGS=${OCPPFLAGS}
#AC_LANG_POP()


#Set optional library path with option --with-snn-lib or
#environment variable SNN_LIBS

AC_ARG_WITH([snn-lib],
            AC_HELP_STRING([--with-snn-lib=LIB_PATH],
                           [the location of the SNN library]),
            [snn_lib_path=$withval])
if test [ -n "$snn_lib_path" ] ; then
   SNN_LIB=$snn_lib_path
else
   if test [ -z "${SNN_LIB}" ] ; then
      if test [ "${prefix}" = "NONE" ] ; then 
         AC_MSG_ERROR("Location of SNN library not defined. Use configure arg '--with-snn-lib' or environment variable 'SNN_LIB'")
      else
         SNN_LIB=${prefix}/${LIBnn}
      fi
   fi
fi

OLDFLAGS=${LDFLAGS}
AC_LANG_PUSH(C)
LDFLAGS=-L${SNN_LIB}
#AC_CHECK_LIB(snn, snn_version, [],
#             [AC_MSG_ERROR("Cannot load SNN library in ${SNN_LIB}")])

AC_LANG_POP()
LDFLAGS=${OLDFLAGS}

#Finally, set optional module directory using --with-snn-modules or
#environment variable SNN_MODULES

AC_SUBST(SNN_INCLUDE)
AC_SUBST(SNN_LIB)

AC_OUTPUT(src/Makevars)
