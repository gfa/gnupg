# ksba.m4 - autoconf macro to detect ksba
#       Copyright (C) 2002 g10 Code GmbH
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This file is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.


dnl AM_PATH_KSBA([MINIMUM-VERSION,
dnl              [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl Test for libksba and define KSBA_CFLAGS and KSBA_LIBS
dnl
AC_DEFUN(AM_PATH_KSBA,
[ AC_ARG_WITH(ksba-prefix,
            AC_HELP_STRING([--with-ksba-prefix=PFX],
                           [prefix where KSBA is installed (optional)]),
     ksba_config_prefix="$withval", ksba_config_prefix="")
  if test x$ksba_config_prefix != x ; then
     ksba_config_args="$ksba_config_args --prefix=$ksba_config_prefix"
     if test x${KSBA_CONFIG+set} != xset ; then
        KSBA_CONFIG=$ksba_config_prefix/bin/ksba-config
     fi
  fi

  AC_PATH_PROG(KSBA_CONFIG, ksba-config, no)
  min_ksba_version=ifelse([$1], ,0.4.4,$1)
  AC_MSG_CHECKING(for KSBA - version >= $min_ksba_version)
  ok=no
  if test "$KSBA_CONFIG" != "no" ; then
    req_major=`echo $min_ksba_version | \
               sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\)/\1/'`
    req_minor=`echo $min_ksba_version | \
               sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\)/\2/'`
    req_micro=`echo $min_ksba_version | \
               sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\)/\3/'`
    ksba_config_version=`$KSBA_CONFIG $ksba_config_args --version`
    major=`echo $ksba_config_version | \
               sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\1/'`
    minor=`echo $ksba_config_version | \
               sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\2/'`
    micro=`echo $ksba_config_version | \
               sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\3/'`
    if test "$major" -gt "$req_major"; then
        ok=yes
    else 
        if test "$major" -eq "$req_major"; then
            if test "$minor" -gt "$req_minor"; then
               ok=yes
            else
               if test "$minor" -eq "$req_minor"; then
                   if test "$micro" -ge "$req_micro"; then
                     ok=yes
                   fi
               fi
            fi
        fi
    fi
  fi
  if test $ok = yes; then
    KSBA_CFLAGS=`$KSBA_CONFIG $ksba_config_args --cflags`
    KSBA_LIBS=`$KSBA_CONFIG $ksba_config_args --libs`
    AC_MSG_RESULT(yes)
    ifelse([$2], , :, [$2])
  else
    KSBA_CFLAGS=""
    KSBA_LIBS=""
    AC_MSG_RESULT(no)
    ifelse([$3], , :, [$3])
  fi
  AC_SUBST(KSBA_CFLAGS)
  AC_SUBST(KSBA_LIBS)
])
