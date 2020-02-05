#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([MET], [m4_esyscmd_s(echo ${MET_BUILD_VERSION:-8.1})], [met_help@ucar.edu], [], [http://www.dtcenter.org/met/users])
AC_CONFIG_SRCDIR([src/tools/tc_utils/tc_dland/tc_dland.cc])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.9 foreign])

#
# Look for the NetCDF library
#

# Configure the variables.  The help text will appear if the user uses "configure --help".

AC_ARG_VAR([MET_NETCDF], [Where netcdf lib and include subdirectories are located.  If they are installed in /usr/local, you don't have to specify them.])
AC_ARG_VAR([MET_NETCDFINC], [Where netcdf include files are located. Use if the libraries and include files are not under the same main netcdf directory.  If used, MET_NETCDFLIB must also be defined and MET_NETCDF will be ignored.])
AC_ARG_VAR([MET_NETCDFLIB], [Where netcdf libraries are located. Use if the libraries and include files are not under the same main netcdf directory.  If used, MET_NETCDFINC must also be defined and MET_NETCDF will be ignored.])

# Ensure that both or neither are specified.

if (test -n "$MET_NETCDFLIB" && test -z "$MET_NETCDFINC") || \
   (test -n "$MET_NETCDFINC" && test -z "$MET_NETCDFLIB"); then
  if test -n "$MET_NETCDFLIB"; then
    AC_MSG_ERROR([MET_NETCDFLIB environment variable set, but MET_NETCDFINC not set.  If one of these variables is set, then both must be set.])
  else
    AC_MSG_ERROR([MET_NETCDFINC environment variable set, but MET_NETCDFLIB not set.  If one of these variables is set, then both must be set.])
  fi
fi

# If the individual locations weren't specified, then the main NetCDF directory
# must be specified.  If it isn't, let the user know they need to supply one.

if test -z "$MET_NETCDFLIB" && test -n "$MET_NETCDF"; then
   MET_NETCDFINC='${MET_NETCDF}/include'
   MET_NETCDFLIB='${MET_NETCDF}/lib'
fi

# We don't currently look for the NetCDF libraries in "known"
# locations.  If we want to, this example code might be
# helpful.
## Not specified? Check for FTK in standard places.
#if test -z "$FTKLIB"; then
#  # Check for flaim tool kit as a sub-project.
#  if test -d "$srcdir/ftk"; then
#    AC_CONFIG_SUBDIRS([ftk])
#    FTKINC='$(top_srcdir)/ftk/src'
#    FTKLIB='$(top_builddir)/ftk/src'
#  else
#    # Check for flaim tool kit as a super-project.
#    if test -d "$srcdir/../ftk"; then
#      FTKINC='$(top_srcdir)/../ftk/src'
#      FTKLIB='$(top_builddir)/../ftk/src'
#    fi
#  fi
#fi
#
## Still empty? Check for installed flaim tool kit.
#if test -z "$FTKLIB"; then
#  AC_CHECK_LIB([flaimtk], [ftkFastChecksum],
#    [AC_CHECK_HEADERS([flaimtk.h])
#     LIBS="-lflaimtk $LIBS"],
#    [AC_MSG_ERROR([No FLAIM Took Kit found.])])
#fi

# AC_SUBST command line variables.  If we get here, the NetCDF
# libraries were found so no need to check

if test -n "$MET_NETCDFLIB"; then
   CPPFLAGS="${CPPFLAGS} -I${MET_NETCDFINC}"
   LDFLAGS="${LDFLAGS} -L${MET_NETCDFLIB}"
fi

#
# Look for the HDF5 library
#

# Configure the variables.  The help text will appear if the user uses "configure --help".

AC_ARG_VAR([MET_HDF5], [Where hdf5 lib and include subdirectories are located.  If they are installed in /usr/local, you don't have to specify them.])
AC_ARG_VAR([MET_HDF5INC], [Where hdf5 include files are located. Use if the libraries and include files are not under the same main hdf5 directory.  If used, MET_HDF5LIB must also be defined and MET_HDF5 will be ignored.])
AC_ARG_VAR([MET_HDF5LIB], [Where hdf5 libraries are located. Use if the libraries and include files are not under the same main hdf5 directory.  If used, MET_HDF5INC must also be defined and MET_HDF5 will be ignored.])

# Ensure that both or neither are specified.

if (test -n "$MET_HDF5LIB" && test -z "$MET_HDF5INC") || \
   (test -n "$MET_HDF5INC" && test -z "$MET_HDF5LIB"); then
  if test -n "$MET_HDF5LIB"; then
    AC_MSG_ERROR([MET_HDF5LIB environment variable set, but MET_HDF5INC not set.  If one of these variables is set, then both must be set.])
  else
    AC_MSG_ERROR([MET_HDF5INC environment variable set, but MET_HDF5LIB not set.  If one of these variables is set, then both must be set.])
  fi
fi

# If the individual locations weren't specified, then the main HDF5 directory
# must be specified.  If it isn't, let the user know they need to supply one.

if test -z "$MET_HDF5LIB" && test -n "$MET_HDF5"; then
   MET_HDF5INC='${MET_HDF5}/include'
   MET_HDF5LIB='${MET_HDF5}/lib'
fi

# AC_SUBST command line variables.  If we get here, the HDF5
# libraries were found so no need to check

if test -n "$MET_HDF5LIB"; then
   CPPFLAGS="${CPPFLAGS} -I${MET_HDF5INC}"
   LDFLAGS="${LDFLAGS} -L${MET_HDF5LIB}"
fi

#
# Look for the GRIB2C library
#

# Configure the variables.  The help text will appear if the user uses "configure --help".

AC_ARG_VAR([MET_GRIB2C], [Where grib2c lib and include subdirectories are located.  If they are installed in /usr/local, you don't have to specify them.])
AC_ARG_VAR([MET_GRIB2CINC], [Where grib2c include files are located. Use if the libraries and include files are not under the same main grib2c directory.  If used, MET_GRIB2CLIB must also be defined and MET_GRIB2C will be ignored.])
AC_ARG_VAR([MET_GRIB2CLIB], [Where grib2c libraries are located. Use if the libraries and include files are not under the same main grib2c directory.  If used, MET_GRIB2CINC must also be defined and MET_GRIB2C will be ignored.])
AC_ARG_VAR([GRIB2CLIB_NAME], [Name of grib2c library to link with. For example, -lgrib2c_v1.6.0 or -lgrib2c, etc.  If it is -lgrib2c, you don't have to specify it.])

# Ensure that both or neither are specified.

if (test -n "$MET_GRIB2CLIB" && test -z "$MET_GRIB2CINC") || \
   (test -n "$MET_GRIB2CINC" && test -z "$MET_GRIB2CLIB"); then
  if test -n "$MET_GRIB2CLIB"; then
    AC_MSG_ERROR([MET_GRIB2CLIB environment variable set, but MET_GRIB2CINC not set.  If one of these variables is set, then both must be set.])
  else
    AC_MSG_ERROR([MET_GRIB2CINC environment variable set, but MET_GRIB2CLIB not set.  If one of these variables is set, then both must be set.])
  fi
fi

# If the individual locations weren't specified, then the main Grib2c directory
# must be specified.  If it isn't, let the user know they need to supply one.

if test -z "$MET_GRIB2CLIB" && test -n "$MET_GRIB2C"; then
   MET_GRIB2CINC='${MET_GRIB2C}/include'
   MET_GRIB2CLIB='${MET_GRIB2C}/lib'
fi

# AC_SUBST command line variables.  If we get here, the Grib2c
# libraries were found so no need to check

if test -n "$MET_GRIB2CLIB"; then
   CPPFLAGS="${CPPFLAGS} -I${MET_GRIB2CINC}"
   LDFLAGS="${LDFLAGS} -L${MET_GRIB2CLIB}"
fi

#
# Look for the PYTHON library
#

# Configure the variables.  The help text will appear if the user uses "configure --help".

AC_ARG_VAR([MET_PYTHON_CC], [Where the python header files are located, e.g. -I<include dir>. Set to the output of 'python-config --cflags'.])
AC_ARG_VAR([MET_PYTHON_LD], [Where the python library files are located, e.g. -L<lib dir>, followed by the python library names, e.g. -l<library>. Set to the output of 'python-config --ldflags'.])

if test -n "$MET_PYTHON_CC"; then
   CPPFLAGS="${CPPFLAGS} -I../../basic/vx_log -I../../basic/vx_util ${MET_PYTHON_CC}"
fi

if test -n "$MET_PYTHON_LD"; then
   LDFLAGS="${LDFLAGS} ${MET_PYTHON_LD}"
fi

#
# Look for the GSL library
#

# Configure the variables.  The help text will appear if the user uses "configure --help".

AC_ARG_VAR([MET_GSL], [Where gsl lib and include subdirectories are located.  If they are installed in /usr/local, you don't have to specify them.])
AC_ARG_VAR([MET_GSLINC], [Where gsl include files are located. Use if the libraries and include files are not under the same main gsl directory.  If used, MET_GSLLIB must also be defined and MET_GSL will be ignored.])
AC_ARG_VAR([MET_GSLLIB], [Where gsl libraries are located. Use if the libraries and include files are not under the same main gsl directory.  If used, MET_GSLINC must also be defined and MET_GSL will be ignored.])

# Ensure that both or neither are specified.

if (test -n "$MET_GSLLIB" && test -z "$MET_GSLINC") || \
   (test -n "$MET_GSLINC" && test -z "$MET_GSLLIB"); then
  if test -n "$MET_GSLLIB"; then
    AC_MSG_ERROR([MET_GSLLIB environment variable set, but MET_GSLINC not set.  If one of these variables is set, then both must be set.])
  else
    AC_MSG_ERROR([MET_GSLINC environment variable set, but MET_GSLLIB not set.  If one of these variables is set, then both must be set.])
  fi
fi

# If the individual locations weren't specified, then the main Gsl directory
# must be specified.  If it isn't, let the user know they need to supply one.

if test -z "$MET_GSLLIB" && test -n "$MET_GSL"; then
   MET_GSLINC='${MET_GSL}/include'
   MET_GSLLIB='${MET_GSL}/lib'
fi

# AC_SUBST command line variables.  If we get here, the Gsl
# libraries were found so no need to check

if test -n "$MET_GSLLIB"; then
   CPPFLAGS="${CPPFLAGS} -I${MET_GSLINC}"
   LDFLAGS="${LDFLAGS} -L${MET_GSLLIB}"
fi

#
# Look for the BUFR library
#

# Configure the variables.  The help text will appear if the user uses "configure --help".

AC_ARG_VAR([MET_BUFR], [Where bufr lib subdirectory is located.  If it is installed in /usr/local, you don't have to specify it.])
AC_ARG_VAR([MET_BUFRLIB], [Where bufr libraries are located. Use if the library files are not in a bin subdirectory.  If used MET_BUFR will be ignored.])

# If the individual locations weren't specified, then the main Bufr directory
# must be specified.  If it isn't, let the user know they need to supply one.

if test -z "$MET_BUFRLIB" && test -n "$MET_BUFR"; then
   MET_BUFRLIB='${MET_BUFR}/lib'
fi

# AC_SUBST command line variables.  If we get here, the Bufr
# libraries were found so no need to check

if test -n "$MET_BUFRLIB"; then
   LDFLAGS="${LDFLAGS} -L${MET_BUFRLIB}"
fi

# Configure the variables.  The help text will appear if the user uses "configure --help".

AC_ARG_VAR([BUFRLIB_NAME], [Name of bufr library to link with. For example, -lbufr_v11.3.0_4_64 or -lbufr, etc.  If it is -lbufr, you don't have to specify it.])

AC_SUBST([BUFRLIB_NAME])
AM_CONDITIONAL([BUFRLIB_NAME_SET], [test ! -z "$BUFRLIB_NAME"])
AC_MSG_NOTICE([BUFRLIB_NAME will be set])


#
# Look for the HDF library
#

# Configure the variables.  The help text will appear if the user uses "configure --help".

AC_ARG_VAR([MET_HDF], [Where hdf version 4 lib and include subdirectories are located.  If they are installed in /usr/local, you don't have to specify them. HDF version 4 is only needed if you are building the MODIS utility.])
AC_ARG_VAR([MET_HDFINC], [Where hdf version 4 include files are located. Use if the libraries and include files are not under the same main hdf directory.  If used, MET_HDFLIB must also be defined and MET_HDF will be ignored.])
AC_ARG_VAR([MET_HDFLIB], [Where hdf version 4 libraries are located. Use if the libraries and include files are not under the same main hdf directory.  If used, MET_HDFINC must also be defined and MET_HDF will be ignored.])

# Ensure that both or neither are specified.

if (test -n "$MET_HDFLIB" && test -z "$MET_HDFINC") || \
   (test -n "$MET_HDFINC" && test -z "$MET_HDFLIB"); then
  if test -n "$MET_HDFLIB"; then
    AC_MSG_ERROR([MET_HDFLIB environment variable set, but MET_HDFINC not set.  If one of these variables is set, then both must be set.])
  else
    AC_MSG_ERROR([MET_HDFINC environment variable set, but MET_HDFLIB not set.  If one of these variables is set, then both must be set.])
  fi
fi

# If the individual locations weren't specified, then the main HDF directory
# must be specified.  If it isn't, let the user know they need to supply one.

if test -z "$MET_HDFLIB" && test -n "$MET_HDF"; then
   MET_HDFINC='${MET_HDF}/include'
   MET_HDFLIB='${MET_HDF}/lib'
fi

# AC_SUBST command line variables.  If we get here, the HDF
# libraries were found so no need to check

if test -n "$MET_HDFLIB"; then
   CPPFLAGS="${CPPFLAGS} -I${MET_HDFINC}"
   LDFLAGS="${LDFLAGS} -L${MET_HDFLIB}"
fi

#
# Look for the HDFEOS library
#

# Configure the variables.  The help text will appear if the user uses "configure --help".

AC_ARG_VAR([MET_HDFEOS], [Where hdfeos version 2 lib and include subdirectories are located.  If they are installed in /usr/local, you don't have to specify them. HDF-EOS version 2 is only needed if you are building the MODIS utility.])
AC_ARG_VAR([MET_HDFEOSINC], [Where hdfeos version 2 include files are located. Use if the libraries and include files are not under the same main hdfeos directory.  If used, MET_HDFEOSLIB must also be defined and MET_HDFEOS will be ignored.])
AC_ARG_VAR([MET_HDFEOSLIB], [Where hdfeos version 2 libraries are located. Use if the libraries and include files are not under the same main hdfeos directory.  If used, MET_HDFEOSINC must also be defined and MET_HDFEOS will be ignored.])

# Ensure that both or neither are specified.

if (test -n "$MET_HDFEOSLIB" && test -z "$MET_HDFEOSINC") || \
   (test -n "$MET_HDFEOSINC" && test -z "$MET_HDFEOSLIB"); then
  if test -n "$MET_HDFEOSLIB"; then
    AC_MSG_ERROR([MET_HDFEOSLIB environment variable set, but MET_HDFEOSINC not set.  If one of these variables is set, then both must be set.])
  else
    AC_MSG_ERROR([MET_HDFEOSINC environment variable set, but MET_HDFEOSLIB not set.  If one of these variables is set, then both must be set.])
  fi
fi

# If the individual locations weren't specified, then the main HDFEOS directory
# must be specified.  If it isn't, let the user know they need to supply one.

if test -z "$MET_HDFEOSLIB" && test -n "$MET_HDFEOS"; then
   MET_HDFEOSINC='${MET_HDFEOS}/include'
   MET_HDFEOSLIB='${MET_HDFEOS}/lib'
fi

# AC_SUBST command line variables.  If we get here, the HDFEOS
# libraries were found so no need to check

if test -n "$MET_HDFEOSLIB"; then
   CPPFLAGS="${CPPFLAGS} -I${MET_HDFEOSINC}"
   LDFLAGS="${LDFLAGS} -L${MET_HDFEOSLIB}"
fi

#
# Look for the Cairo library
#

# Configure the variables.  The help text will appear if the user uses "configure --help".

AC_ARG_VAR([MET_CAIRO], [Where cairo lib and include subdirectories are located.  If they are installed in /usr/local, you don't have to specify them. Cairo is only needed if you are building the mode_graphics utility.])
AC_ARG_VAR([MET_CAIROINC], [Where cairo include files are located. Use if the libraries and include files are not under the same main cairo directory.  If used, MET_CAIROLIB must also be defined and MET_CAIRO will be ignored.])
AC_ARG_VAR([MET_CAIROLIB], [Where cairo libraries are located. Use if the libraries and include files are not under the same main cairo directory.  If used, MET_CAIROINC must also be defined and MET_CAIRO will be ignored.])

# Ensure that both or neither are specified.

if (test -n "$MET_CAIROLIB" && test -z "$MET_CAIROINC") || \
   (test -n "$MET_CAIROINC" && test -z "$MET_CAIROLIB"); then
  if test -n "$MET_CAIROLIB"; then
    AC_MSG_ERROR([MET_CAIROLIB environment variable set, but MET_CAIROINC not set.  If one of these variables is set, then both must be set.])
  else
    AC_MSG_ERROR([MET_CAIROINC environment variable set, but MET_CAIROLIB not set.  If one of these variables is set, then both must be set.])
  fi
fi

# If the individual locations weren't specified, then the main Cairo directory
# must be specified.  If it isn't, let the user know they need to supply one.

if test -z "$MET_CAIROLIB" && test -n "$MET_CAIRO"; then
   MET_CAIROINC='${MET_CAIRO}/include'
   MET_CAIROLIB='${MET_CAIRO}/lib'
fi

# AC_SUBST command line variables.  If we get here, the Cairo
# libraries were found so no need to check

if test -n "$MET_CAIROLIB"; then
   CPPFLAGS="${CPPFLAGS} -I${MET_CAIROINC}"
   LDFLAGS="${LDFLAGS} -L${MET_CAIROLIB}"
fi

#
# Look for the Freetype library
#

# Configure the variables.  The help text will appear if the user uses "configure --help".

AC_ARG_VAR([MET_FREETYPE], [Where freetype lib and include subdirectories are located.  If they are installed in /usr/local, you don't have to specify them. Freetype is only needed if you are building the mode_graphics utility.])
AC_ARG_VAR([MET_FREETYPEINC], [Where freetype include files are located. Use if the libraries and include files are not under the same main freetype directory.  If used, MET_FREETYPELIB must also be defined and MET_FREETYPE will be ignored.])
AC_ARG_VAR([MET_FREETYPELIB], [Where freetype libraries are located. Use if the libraries and include files are not under the same main freetype directory.  If used, MET_FREETYPEINC must also be defined and MET_FREETYPE will be ignored.])

# Ensure that both or neither are specified.

if (test -n "$MET_FREETYPELIB" && test -z "$MET_FREETYPEINC") || \
   (test -n "$MET_FREETYPEINC" && test -z "$MET_FREETYPELIB"); then
  if test -n "$MET_FREETYPELIB"; then
    AC_MSG_ERROR([MET_FREETYPELIB environment variable set, but MET_FREETYPEINC not set.  If one of these variables is set, then both must be set.])
  else
    AC_MSG_ERROR([MET_FREETYPEINC environment variable set, but MET_FREETYPELIB not set.  If one of these variables is set, then both must be set.])
  fi
fi

# If the individual locations weren't specified, then the main Freetype directory
# must be specified.  If it isn't, let the user know they need to supply one.

if test -z "$MET_FREETYPELIB" && test -n "$MET_FREETYPE"; then
   MET_FREETYPEINC='${MET_FREETYPE}/include'
   MET_FREETYPELIB='${MET_FREETYPE}/lib'
fi

# AC_SUBST command line variables.  If we get here, the Freetype
# libraries were found so no need to check

if test -n "$MET_FREETYPELIB"; then
   CPPFLAGS="${CPPFLAGS} -I${MET_FREETYPEINC}"
   LDFLAGS="${LDFLAGS} -L${MET_FREETYPELIB}"
fi

#
# Process any enable flags specified by the user
#

# BLOCK4

AC_ARG_ENABLE(block4,
	  [AS_HELP_STRING([--disable-block4], [Disable use of BLOCK4 in the compilation. Use this if you have trouble using PrepBufr files.])],
  [case "${enableval}" in
     yes | no ) ENABLE_BLOCK4="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-block4) ;;
   esac],
  [ENABLE_BLOCK4="yes"]
)

AM_CONDITIONAL([ENABLE_BLOCK4], [test "x$ENABLE_BLOCK4" = "xyes"])

if test "x$ENABLE_BLOCK4" = "xyes"; then
    CPPFLAGS="${CPPFLAGS} -DBLOCK4"
    AC_MSG_NOTICE([-DBLOCK4 will be used in the compiles])
else
    AC_MSG_NOTICE([-DBLOCK4 will not be used in the compiles])
fi

# static linking

AC_ARG_ENABLE(static_linking,
	  [AS_HELP_STRING([--enable-static_linking], [Enable static linking of the applications])],
  [case "${enableval}" in
     yes | no ) ENABLE_STATIC_LINKING="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --enable-static_linking) ;;
   esac],
  [ENABLE_STATIC_LINKING="no"]
)

AM_CONDITIONAL([ENABLE_STATIC_LINKING], [test "x$ENABLE_STATIC_LINKING" = "xyes"])

if test "x$ENABLE_STATIC_LINKING" = "xyes"; then
    LDFLAGS="${LDFLAGS} -static"
    AC_MSG_NOTICE([Static linking will be used in the compiles])
else
    AC_MSG_NOTICE([Dynamic linking will not be used in the compiles])
fi

# ascii2nc

AC_ARG_ENABLE(ascii2nc,
	  [AS_HELP_STRING([--disable-ascii2nc], [Disable compilation of ascii2nc])],
  [case "${enableval}" in
     yes | no ) ENABLE_ASCII2NC="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-ascii2nc) ;;
   esac],
  [ENABLE_ASCII2NC="yes"]
)

AM_CONDITIONAL([ENABLE_ASCII2NC], [test "x$ENABLE_ASCII2NC" = "xyes"])

if test "x$ENABLE_ASCII2NC" = "xyes"; then
    AC_DEFINE([ENABLE_ASCII2NC], [], ["build ascii2nc"])
    AC_MSG_NOTICE([ascii2nc will be compiled])
else
    AC_MSG_NOTICE([ascii2nc will not be compiled])
fi

# lidar2nc

AC_ARG_ENABLE(lidar2nc,
     [AS_HELP_STRING([--enable-lidar2nc], [Enable compilation of lidar2nc])],
  [case "${enableval}" in
     yes | no ) ENABLE_LIDAR2NC="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-lidar2nc) ;;
   esac],
  [ENABLE_LIDAR2NC="no"]
)

AM_CONDITIONAL([ENABLE_LIDAR2NC], [test "x$ENABLE_LIDAR2NC" = "xyes"])

if test "x$ENABLE_LIDAR2NC" = "xyes"; then
    AC_DEFINE([ENABLE_LIDAR2NC], [], ["build lidar2nc"])
    AC_MSG_NOTICE([lidar2nc will be compiled])
else
    AC_MSG_NOTICE([lidar2nc will not be compiled])
fi

# ensemble_stat

AC_ARG_ENABLE(ensemble_stat,
	  [AS_HELP_STRING([--disable-ensemble_stat], [Disable compilation of ensemble_stat])],
  [case "${enableval}" in
     yes | no ) ENABLE_ENSEMBLE_STAT="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-ensemble_stat) ;;
   esac],
  [ENABLE_ENSEMBLE_STAT="yes"]
)

AM_CONDITIONAL([ENABLE_ENSEMBLE_STAT], [test "x$ENABLE_ENSEMBLE_STAT" = "xyes"])

if test "x$ENABLE_ENSEMBLE_STAT" = "xyes"; then
    AC_DEFINE([ENABLE_ENSEMBLE_STAT], [], ["build ensemble_stat"])
    AC_MSG_NOTICE([ensemble_stat will be compiled])
else
    AC_MSG_NOTICE([ensemble_stat will not be compiled])
fi

# gen_vx_mask

AC_ARG_ENABLE(gen_vx_mask,
	  [AS_HELP_STRING([--disable-gen_vx_mask], [Disable compilation of gen_vx_mask])],
  [case "${enableval}" in
     yes | no ) ENABLE_GEN_VX_MASK="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-gen_vx_mask) ;;
   esac],
  [ENABLE_GEN_VX_MASK="yes"]
)

AM_CONDITIONAL([ENABLE_GEN_VX_MASK], [test "x$ENABLE_GEN_VX_MASK" = "xyes"])

if test "x$ENABLE_GEN_VX_MASK" = "xyes"; then
    AC_DEFINE([ENABLE_GEN_VX_MASK], [], ["build gen_vx_mask"])
    AC_MSG_NOTICE([gen_vx_mask will be compiled])
else
    AC_MSG_NOTICE([gen_vx_mask will not be compiled])
fi


# gis_utils

AC_ARG_ENABLE(gis_utils,
       [AS_HELP_STRING([--disable-gis_utils], [Disable compilation of gis_utils])],
  [case "${enableval}" in
     yes | no ) ENABLE_GIS_UTILS="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-gis_utils) ;;
   esac],
  [ENABLE_GIS_UTILS="yes"]
)

AM_CONDITIONAL([ENABLE_GIS_UTILS], [test "x$ENABLE_GIS_UTILS" = "xyes"])

if test "x$ENABLE_GIS_UTILS" = "xyes"; then
    AC_DEFINE([ENABLE_GIS_UTILS], [], ["build gis_utils"])
    AC_MSG_NOTICE([gis_utils will be compiled])
else
    AC_MSG_NOTICE([gis_utils will not be compiled])
fi


# grid_stat

AC_ARG_ENABLE(grid_stat,
	  [AS_HELP_STRING([--disable-grid_stat], [Disable compilation of grid_stat])],
  [case "${enableval}" in
     yes | no ) ENABLE_GRID_STAT="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-grid_stat) ;;
   esac],
  [ENABLE_GRID_STAT="yes"]
)

AM_CONDITIONAL([ENABLE_GRID_STAT], [test "x$ENABLE_GRID_STAT" = "xyes"])

if test "x$ENABLE_GRID_STAT" = "xyes"; then
    AC_DEFINE([ENABLE_GRID_STAT], [], ["build grid_stat"])
    AC_MSG_NOTICE([grid_stat will be compiled])
else
    AC_MSG_NOTICE([grid_stat will not be compiled])
fi

# madis2nc

AC_ARG_ENABLE(madis2nc,
	  [AS_HELP_STRING([--disable-madis2nc], [Disable compilation of madis2nc])],
  [case "${enableval}" in
     yes | no ) ENABLE_MADIS2NC="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-madis2nc) ;;
   esac],
  [ENABLE_MADIS2NC="yes"]
)

AM_CONDITIONAL([ENABLE_MADIS2NC], [test "x$ENABLE_MADIS2NC" = "xyes"])

if test "x$ENABLE_MADIS2NC" = "xyes"; then
    AC_DEFINE([ENABLE_MADIS2NC], [], ["build madis2nc"])
    AC_MSG_NOTICE([madis2nc will be compiled])
else
    AC_MSG_NOTICE([madis2nc will not be compiled])
fi

# mode

AC_ARG_ENABLE(mode,
	  [AS_HELP_STRING([--disable-mode], [Disable compilation of mode])],
  [case "${enableval}" in
     yes | no ) ENABLE_MODE="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-mode) ;;
   esac],
  [ENABLE_MODE="yes"]
)

AM_CONDITIONAL([ENABLE_MODE], [test "x$ENABLE_MODE" = "xyes"])

if test "x$ENABLE_MODE" = "xyes"; then
    AC_DEFINE([ENABLE_MODE], [], ["build mode"])
    AC_MSG_NOTICE([mode will be compiled])
else
    AC_MSG_NOTICE([mode will not be compiled])
fi

# mode_analysis

AC_ARG_ENABLE(mode_analysis,
	  [AS_HELP_STRING([--disable-mode_analysis], [Disable compilation of mode_analysis])],
  [case "${enableval}" in
     yes | no ) ENABLE_MODE_ANALYSIS="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-mode_analysis) ;;
   esac],
  [ENABLE_MODE_ANALYSIS="yes"]
)

AM_CONDITIONAL([ENABLE_MODE_ANALYSIS], [test "x$ENABLE_MODE_ANALYSIS" = "xyes"])

if test "x$ENABLE_MODE_ANALYSIS" = "xyes"; then
    AC_DEFINE([ENABLE_MODE_ANALYSIS], [], ["build mode_analysis"])
    AC_MSG_NOTICE([mode_analysis will be compiled])
else
    AC_MSG_NOTICE([mode_analysis will not be compiled])
fi

# pb2nc

AC_ARG_ENABLE(pb2nc,
	  [AS_HELP_STRING([--disable-pb2nc], [Disable compilation of pb2nc])],
  [case "${enableval}" in
     yes | no ) ENABLE_PB2NC="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-pb2nc) ;;
   esac],
  [ENABLE_PB2NC="yes"]
)

AM_CONDITIONAL([ENABLE_PB2NC], [test "x$ENABLE_PB2NC" = "xyes"])

if test "x$ENABLE_PB2NC" = "xyes"; then
    AC_DEFINE([ENABLE_PB2NC], [], ["build pb2nc"])
    AC_MSG_NOTICE([pb2nc will be compiled])
else
    AC_MSG_NOTICE([pb2nc will not be compiled])
fi

# pcp_combine

AC_ARG_ENABLE(pcp_combine,
	  [AS_HELP_STRING([--disable-pcp_combine], [Disable compilation of pcp_combine])],
  [case "${enableval}" in
     yes | no ) ENABLE_PCP_COMBINE="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-pcp_combine) ;;
   esac],
  [ENABLE_PCP_COMBINE="yes"]
)

AM_CONDITIONAL([ENABLE_PCP_COMBINE], [test "x$ENABLE_PCP_COMBINE" = "xyes"])

if test "x$ENABLE_PCP_COMBINE" = "xyes"; then
    AC_DEFINE([ENABLE_PCP_COMBINE], [], ["build pcp_combine"])
    AC_MSG_NOTICE([pcp_combine will be compiled])
else
    AC_MSG_NOTICE([pcp_combine will not be compiled])
fi

# plot_data_plane

AC_ARG_ENABLE(plot_data_plane,
	  [AS_HELP_STRING([--disable-plot_data_plane], [Disable compilation of plot_data_plane])],
  [case "${enableval}" in
     yes | no ) ENABLE_PLOT_DATA_PLANE="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-plot_data_plane) ;;
   esac],
  [ENABLE_PLOT_DATA_PLANE="yes"]
)

AM_CONDITIONAL([ENABLE_PLOT_DATA_PLANE], [test "x$ENABLE_PLOT_DATA_PLANE" = "xyes"])

if test "x$ENABLE_PLOT_DATA_PLANE" = "xyes"; then
    AC_DEFINE([ENABLE_PLOT_DATA_PLANE], [], ["build plot_data_plane"])
    AC_MSG_NOTICE([plot_data_plane will be compiled])
else
    AC_MSG_NOTICE([plot_data_plane will not be compiled])
fi

# regrid_data_plane

AC_ARG_ENABLE(regrid_data_plane,
          [AS_HELP_STRING([--disable-regrid_data_plane], [Disable compilation of regrid_data_plane])],
  [case "${enableval}" in
     yes | no ) ENABLE_REGRID_DATA_PLANE="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-regrid_data_plane) ;;
   esac],
  [ENABLE_REGRID_DATA_PLANE="yes"]
)

AM_CONDITIONAL([ENABLE_REGRID_DATA_PLANE], [test "x$ENABLE_REGRID_DATA_PLANE" = "xyes"])

if test "x$ENABLE_REGRID_DATA_PLANE" = "xyes"; then
    AC_DEFINE([ENABLE_REGRID_DATA_PLANE], [], ["build regrid_data_plane"])
    AC_MSG_NOTICE([regrid_data_plane will be compiled])
else
    AC_MSG_NOTICE([regrid_data_plane will not be compiled])
fi

# point2grid

AC_ARG_ENABLE(point2grid,
          [AS_HELP_STRING([--disable-point2grid], [Disable compilation of point2grid])],
  [case "${enableval}" in
     yes | no ) ENABLE_POINT2GRID="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-point2grid) ;;
   esac],
  [ENABLE_POINT2GRID="yes"]
)

AM_CONDITIONAL([ENABLE_POINT2GRID], [test "x$ENABLE_POINT2GRID" = "xyes"])

if test "x$ENABLE_POINT2GRID" = "xyes"; then
    AC_DEFINE([ENABLE_POINT2GRID], [], ["build point2grid"])
    AC_MSG_NOTICE([point2grid will be compiled])
else
    AC_MSG_NOTICE([point2grid will not be compiled])
fi

# shift_data_plane

AC_ARG_ENABLE(shift_data_plane,
          [AS_HELP_STRING([--disable-shift_data_plane], [Disable compilation of shift_data_plane])],
  [case "${enableval}" in
     yes | no ) ENABLE_SHIFT_DATA_PLANE="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-shift_data_plane) ;;
   esac],
  [ENABLE_SHIFT_DATA_PLANE="yes"]
)

AM_CONDITIONAL([ENABLE_SHIFT_DATA_PLANE], [test "x$ENABLE_SHIFT_DATA_PLANE" = "xyes"])

if test "x$ENABLE_SHIFT_DATA_PLANE" = "xyes"; then
    AC_DEFINE([ENABLE_SHIFT_DATA_PLANE], [], ["build shift_data_plane"])
    AC_MSG_NOTICE([shift_data_plane will be compiled])
else
    AC_MSG_NOTICE([shift_data_plane will not be compiled])
fi

# plot_point_obs

AC_ARG_ENABLE(plot_point_obs,
	  [AS_HELP_STRING([--disable-plot_point_obs], [Disable compilation of plot_point_obs])],
  [case "${enableval}" in
     yes | no ) ENABLE_PLOT_POINT_OBS="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-plot_point_obs) ;;
   esac],
  [ENABLE_PLOT_POINT_OBS="yes"]
)

AM_CONDITIONAL([ENABLE_PLOT_POINT_OBS], [test "x$ENABLE_PLOT_POINT_OBS" = "xyes"])

if test "x$ENABLE_PLOT_POINT_OBS" = "xyes"; then
    AC_DEFINE([ENABLE_PLOT_POINT_OBS], [], ["build plot_point_obs"])
    AC_MSG_NOTICE([plot_point_obs will be compiled])
else
    AC_MSG_NOTICE([plot_point_obs will not be compiled])
fi

# point_stat

AC_ARG_ENABLE(point_stat,
	  [AS_HELP_STRING([--disable-point_stat], [Disable compilation of point_stat])],
  [case "${enableval}" in
     yes | no ) ENABLE_POINT_STAT="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-point_stat) ;;
   esac],
  [ENABLE_POINT_STAT="yes"]
)

AM_CONDITIONAL([ENABLE_POINT_STAT], [test "x$ENABLE_POINT_STAT" = "xyes"])

if test "x$ENABLE_POINT_STAT" = "xyes"; then
    AC_DEFINE([ENABLE_POINT_STAT], [], ["build point_stat"])
    AC_MSG_NOTICE([point_stat will be compiled])
else
    AC_MSG_NOTICE([point_stat will not be compiled])
fi

# stat_analysis

AC_ARG_ENABLE(stat_analysis,
	  [AS_HELP_STRING([--disable-stat_analysis], [Disable compilation of stat_analysis])],
  [case "${enableval}" in
     yes | no ) ENABLE_STAT_ANALYSIS="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-stat_analysis) ;;
   esac],
  [ENABLE_STAT_ANALYSIS="yes"]
)

AM_CONDITIONAL([ENABLE_STAT_ANALYSIS], [test "x$ENABLE_STAT_ANALYSIS" = "xyes"])

if test "x$ENABLE_STAT_ANALYSIS" = "xyes"; then
    AC_DEFINE([ENABLE_STAT_ANALYSIS], [], ["build stat_analysis"])
    AC_MSG_NOTICE([stat_analysis will be compiled])
else
    AC_MSG_NOTICE([stat_analysis will not be compiled])
fi

# wavelet_stat

AC_ARG_ENABLE(wavelet_stat,
	  [AS_HELP_STRING([--disable-wavelet_stat], [Disable compilation of wavelet_stat])],
  [case "${enableval}" in
     yes | no ) ENABLE_WAVELET_STAT="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-wavelet_stat) ;;
   esac],
  [ENABLE_WAVELET_STAT="yes"]
)

AM_CONDITIONAL([ENABLE_WAVELET_STAT], [test "x$ENABLE_WAVELET_STAT" = "xyes"])

if test "x$ENABLE_WAVELET_STAT" = "xyes"; then
    AC_DEFINE([ENABLE_WAVELET_STAT], [], ["build wavelet_stat"])
    AC_MSG_NOTICE([wavelet_stat will be compiled])
else
    AC_MSG_NOTICE([wavelet_stat will not be compiled])
fi

# series_analysis

AC_ARG_ENABLE(series_analysis,
	  [AS_HELP_STRING([--disable-series_analysis], [Disable compilation of series_analysis])],
  [case "${enableval}" in
     yes | no ) ENABLE_SERIES_ANALYSIS="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-series_analysis) ;;
   esac],
  [ENABLE_SERIES_ANALYSIS="yes"]
)

AM_CONDITIONAL([ENABLE_SERIES_ANALYSIS], [test "x$ENABLE_SERIES_ANALYSIS" = "xyes"])

if test "x$ENABLE_SERIES_ANALYSIS" = "xyes"; then
    AC_DEFINE([ENABLE_SERIES_ANALYSIS], [], ["build series_analysis"])
    AC_MSG_NOTICE([series_analysis will be compiled])
else
    AC_MSG_NOTICE([series_analysis will not be compiled])
fi

# wwmca

AC_ARG_ENABLE(wwmca,
	  [AS_HELP_STRING([--disable-wwmca], [Disable compilation of wwmca])],
  [case "${enableval}" in
     yes | no ) ENABLE_WWMCA="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-wwmca) ;;
   esac],
  [ENABLE_WWMCA="yes"]
)

AM_CONDITIONAL([ENABLE_WWMCA], [test "x$ENABLE_WWMCA" = "xyes"])

if test "x$ENABLE_WWMCA" = "xyes"; then
    AC_DEFINE([ENABLE_WWMCA], [], ["build wwmca"])
    AC_MSG_NOTICE([wwmca will be compiled])
else
    AC_MSG_NOTICE([wwmca will not be compiled])
fi

# modis

AC_ARG_ENABLE(modis,
	  [AS_HELP_STRING([--enable-modis], [Enable compilation of modis])],
  [case "${enableval}" in
     yes | no ) ENABLE_MODIS="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-modis) ;;
   esac],
  [ENABLE_MODIS="no"]
)

AM_CONDITIONAL([ENABLE_MODIS], [test "x$ENABLE_MODIS" = "xyes"])

if test "x$ENABLE_MODIS" = "xyes"; then
    AC_DEFINE([ENABLE_MODIS], [], ["build modis"])
    AC_MSG_NOTICE([modis will be compiled])
else
    AC_MSG_NOTICE([modis will not be compiled])
fi

# mode_graphics

AC_ARG_ENABLE(mode_graphics,
	  [AS_HELP_STRING([--enable-mode_graphics], [Enable compilation of mode_graphics])],
  [case "${enableval}" in
     yes | no ) ENABLE_MODE_GRAPHICS="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --enable-mode_graphics) ;;
   esac],
  [ENABLE_MODE_GRAPHICS="no"]
)

AM_CONDITIONAL([ENABLE_MODE_GRAPHICS], [test "x$ENABLE_MODE_GRAPHICS" = "xyes"])

if test "x$ENABLE_MODE_GRAPHICS" = "xyes"; then
    AC_DEFINE([ENABLE_MODE_GRAPHICS], [], ["build mode_graphics"])
    AC_MSG_NOTICE([mode_graphics will be compiled])
else
    AC_MSG_NOTICE([mode_graphics will not be compiled])
fi

# mode_time_domain

AC_ARG_ENABLE(mode_time_domain,
          [AS_HELP_STRING([--disable-mode_time_domain], [Disable compilation of mode_time_domain])],
  [case "${enableval}" in
     yes | no ) ENABLE_MODE_TIME_DOMAIN="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-mode_time_domain) ;;
   esac],
  [ENABLE_MODE_TIME_DOMAIN="yes"]
)

AM_CONDITIONAL([ENABLE_MODE_TIME_DOMAIN], [test "x$ENABLE_MODE_TIME_DOMAIN" = "xyes"])

if test "x$ENABLE_MODE_TIME_DOMAIN" = "xyes"; then
    AC_DEFINE([ENABLE_MODE_TIME_DOMAIN], [], ["build mode_time_domain"])
    AC_MSG_NOTICE([mode_time_domain will be compiled])
else
    AC_MSG_NOTICE([mode_time_domain will not be compiled])
fi

# tc_utils

AC_ARG_ENABLE(tc_utils,
	  [AS_HELP_STRING([--disable-tc_utils], [Disable compilation of tc_utils])],
  [case "${enableval}" in
     yes | no ) ENABLE_TC_UTILS="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-tc_utils) ;;
   esac],
  [ENABLE_TC_UTILS="yes"]
)

AM_CONDITIONAL([ENABLE_TC_UTILS], [test "x$ENABLE_TC_UTILS" = "xyes"])

if test "x$ENABLE_TC_UTILS" = "xyes"; then
    AC_DEFINE([ENABLE_TC_UTILS], [], ["build tc_utils"])
    AC_MSG_NOTICE([tc_utils will be compiled])
else
    AC_MSG_NOTICE([tc_utils will not be compiled])
fi

# gsi_tools

AC_ARG_ENABLE(gsi_tools,
	  [AS_HELP_STRING([--disable-gsi_tools], [Disable compilation of gsi_tools])],
  [case "${enableval}" in
     yes | no ) ENABLE_GSI_TOOLS="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-gsi_tools) ;;
   esac],
  [ENABLE_GSI_TOOLS="yes"]
)

AM_CONDITIONAL([ENABLE_GSI_TOOLS], [test "x$ENABLE_GSI_TOOLS" = "xyes"])

if test "x$ENABLE_GSI_TOOLS" = "xyes"; then
    AC_DEFINE([ENABLE_GSI_TOOLS], [], ["build gsi_tools"])
    AC_MSG_NOTICE([gsi_tools will be compiled])
else
    AC_MSG_NOTICE([gsi_tools will not be compiled])
fi

# GRIB2

AC_ARG_ENABLE(grib2,
	  [AS_HELP_STRING([--enable-grib2], [Enable compilation of utilities using GRIB2])],
  [case "${enableval}" in
     yes | no ) ENABLE_GRIB2="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --enable-grib2) ;;
   esac],
  [ENABLE_GRIB2="no"]
)

AM_CONDITIONAL([ENABLE_GRIB2], [test "x$ENABLE_GRIB2" = "xyes"])
AM_CONDITIONAL([GRIB2CLIB_NAME_SET], [test ! -z "$GRIB2CLIB_NAME"])

if test "x$ENABLE_GRIB2" = "xyes"; then
    AC_DEFINE([ENABLE_GRIB2], [], ["build GRIB2 utilities"])
    AC_MSG_NOTICE([GRIB2 utilites will be compiled])
    CPPFLAGS="${CPPFLAGS} -DWITH_GRIB2"
    AC_SUBST([GRIB2CLIB_NAME])
    AC_MSG_NOTICE([GRIB2CLIB_NAME will be set])
    if test -z "$GRIB2CLIB_NAME"; then
       GRIB2_LIBS="-lvx_data2d_grib2 -lgrib2c -ljasper -lpng -lz"
    else
       GRIB2_LIBS="-lvx_data2d_grib2 ${GRIB2CLIB_NAME} -ljasper -lpng -lz"
    fi
else
    AC_MSG_NOTICE([GRIB2 utilities will not be compiled])
    GRIB2_LIBS=
fi
AC_SUBST([GRIB2_LIBS])

# Python

AC_ARG_ENABLE(python,
	  [AS_HELP_STRING([--enable-python], [Enable python embedding])],
  [case "${enableval}" in
     yes | no ) ENABLE_PYTHON="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --enable-python) ;;
   esac],
  [ENABLE_PYTHON="no"]
)

AM_CONDITIONAL([ENABLE_PYTHON], [test "x$ENABLE_PYTHON" = "xyes"])

if test "x$ENABLE_PYTHON" = "xyes"; then
    AC_DEFINE([ENABLE_PYTHON], [], ["build python embedding"])
    AC_MSG_NOTICE([python embedding will be compiled])
    CPPFLAGS="${CPPFLAGS} -DWITH_PYTHON"
    PYTHON_LIBS="-lvx_data2d_python -lvx_python3_utils ${MET_PYTHON_LD}"
else
    AC_MSG_NOTICE([python embedding will not be compiled])
    PYTHON_LIBS=
fi
AC_SUBST([PYTHON_LIBS])

########################################################################




########################################################################

# Check for the MET_DEVELOPMENT environment variable

AM_CONDITIONAL([ENABLE_DEVELOPMENT], [test -n "$MET_DEVELOPMENT"])

# Add MET_BASE to the CPPFLAGS

CPPFLAGS=$CPPFLAGS' -DMET_BASE="\"$(pkgdatadir)\""'

# Define other variables for the makefiles

AC_SUBST(FC_LIBS, [-lgfortran])

# The CXXFLAGS default to "-O2 -g".  The optimization is causing
# problems so just set it to "-g" if the user hasn't overridden it
# themselves.

: ${CXXFLAGS="-g"}

# Checks for programs.

AC_PROG_YACC
AC_PROG_CXX
AC_PROG_CC
AC_PROG_F77(gfortran g77 xlf f77 frt pgf77 cf77 fort77 fl32 af77 xlf90 f90 pgf90 pghpf epcf90)
AC_PROG_LEX
AC_PROG_RANLIB

# Checks for libraries.

AC_F77_LIBRARY_LDFLAGS

# Checks for header files.

AC_CHECK_HEADERS([errno.h fcntl.h inttypes.h limits.h memory.h stddef.h stdlib.h string.h strings.h sys/file.h sys/param.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.

AC_FUNC_ERROR_AT_LINE
# AC_FUNC_MALLOC
# AC_FUNC_REALLOC
AC_CHECK_FUNCS([dup2 floor gethostname memset mkdir pow regcomp select setenv sqrt strcasecmp strchr strcspn strerror strncasecmp strrchr strstr isatty])


#AC_CHECK_LIB(netcdf, nc_inq_libvers, [
#        echo "NetCDF library found"
#        ], [
#        echo "Error! Couldn't find the NetCDF library."
#        echo "Check your MET_NETCDF environment variable settings and try again."
#        exit -1
#        ])
#
#if test "x$ENABLE_GRIB2" = "xyes"; then
#  AC_CHECK_LIB(grib2c, seekgb, [
#          echo "GRIB2C library found"
#          ], [
#          echo "Error! Couldn't find the GRIB2C library."
#          echo "Check your MET_GRIB2C environment variable settings and try again."
#          exit -1
#          ])
#
#AC_CHECK_LIB([m],[cos])
#AC_CHECK_LIB([gslcblas],[cblas_dgemm])
#AC_CHECK_LIB(gsl, gsl_blas_dgemm, [
#        echo "GSL library found"
#        ], [
#        echo "Error! Couldn't find the GSL library."
#        echo "Check your MET_GSL environment variable settings and try again."
#        exit -1
#        ], [-lgslcblas -lm])

###########

#AC_CHECK_LIB(bufr, OPENBF, [], [
#        echo "Error! Couldn't find the BUFR library."
#        echo "Check your MET_BUFR environment variable settings and try again."
#        exit -1
#        ], [-lgslcblas -lm])

#if test "x$ENABLE_MODIS" = "xyes"; then
#  AC_CHECK_LIB(mfhdf, hdf_close, [], [
#          echo "Error! Couldn't find the HDF library."
#          echo "Check your MET_HDF environment variable settings and try again."
#          exit -1
#          ])
#  AC_CHECK_LIB(hdfeos, hdf_close, [], [
#          echo "Error! Couldn't find the HDF EOS library."
#          echo "Check your MET_HDFEOS environment variable settings and try again."
#          exit -1
#          ])
#fi

#if test "x$ENABLE_MODE_GRAPHICS" = "xyes"; then
#  AC_CHECK_LIB(cairo, hdf_close, [], [
#          echo "Error! Couldn't find the Cairo library."
#          echo "Check your MET_CAIRO environment variable settings and try again."
#          exit -1
#          ])
#  AC_CHECK_LIB(freetype, hdf_close, [], [
#          echo "Error! Couldn't find the FreeType library."
#          echo "Check your MET_FREETYPE environment variable settings and try again."
#          exit -1
#          ])
#fi

# Create configured files

AC_CONFIG_FILES([Makefile
                 scripts/Rscripts/Makefile
                 scripts/Rscripts/include/Makefile
                 scripts/python/Makefile
                 data/Makefile
                 data/colortables/Makefile
                 data/colortables/NCL_colortables/Makefile
                 data/config/Makefile
                 data/map/Makefile
                 data/map/admin_by_country/Makefile
                 data/poly/Makefile
                 data/poly/HMT_masks/Makefile
                 data/poly/NCEP_masks/Makefile
                 data/wrappers/Makefile
                 data/ps/Makefile
                 data/table_files/Makefile
                 data/tc_data/Makefile
                 src/Makefile
                 src/basic/Makefile
                 src/basic/enum_to_string/Makefile
                 src/basic/vx_cal/Makefile
                 src/basic/vx_config/Makefile
                 src/basic/vx_log/Makefile
                 src/basic/vx_math/Makefile
                 src/basic/vx_util/Makefile
                 src/libcode/Makefile
                 src/libcode/vx_afm/Makefile
                 src/libcode/vx_analysis_util/Makefile
                 src/libcode/vx_color/Makefile
                 src/libcode/vx_data2d/Makefile
                 src/libcode/vx_data2d_factory/Makefile
                 src/libcode/vx_data2d_grib/Makefile
                 src/libcode/vx_data2d_grib2/Makefile
                 src/libcode/vx_data2d_nc_met/Makefile
                 src/libcode/vx_data2d_nc_pinterp/Makefile
                 src/libcode/vx_data2d_nccf/Makefile
                 src/libcode/vx_geodesy/Makefile
                 src/libcode/vx_gis/Makefile
                 src/libcode/vx_gnomon/Makefile
                 src/libcode/vx_grid/Makefile
                 src/libcode/vx_gsl_prob/Makefile
                 src/libcode/vx_nav/Makefile
                 src/libcode/vx_solar/Makefile
                 src/libcode/vx_nc_obs/Makefile
                 src/libcode/vx_nc_util/Makefile
                 src/libcode/vx_pb_util/Makefile
                 src/libcode/vx_plot_util/Makefile
                 src/libcode/vx_ps/Makefile
                 src/libcode/vx_pxm/Makefile
                 src/libcode/vx_render/Makefile
                 src/libcode/vx_shapedata/Makefile
                 src/libcode/vx_stat_out/Makefile
                 src/libcode/vx_statistics/Makefile
                 src/libcode/vx_time_series/Makefile
                 src/libcode/vx_physics/Makefile
                 src/libcode/vx_series_data/Makefile
                 src/libcode/vx_regrid/Makefile
                 src/libcode/vx_tc_util/Makefile
                 src/libcode/vx_summary/Makefile
                 src/libcode/vx_python3_utils/Makefile
                 src/libcode/vx_data2d_python/Makefile
                 src/tools/Makefile
                 src/tools/core/Makefile
                 src/tools/core/ensemble_stat/Makefile
                 src/tools/core/grid_stat/Makefile
                 src/tools/core/mode/Makefile
                 src/tools/core/mode_analysis/Makefile
                 src/tools/core/pcp_combine/Makefile
                 src/tools/core/point_stat/Makefile
                 src/tools/core/series_analysis/Makefile
                 src/tools/core/stat_analysis/Makefile
                 src/tools/core/wavelet_stat/Makefile
                 src/tools/other/Makefile
                 src/tools/other/ascii2nc/Makefile
                 src/tools/other/lidar2nc/Makefile
                 src/tools/other/gen_vx_mask/Makefile
                 src/tools/other/gis_utils/Makefile
                 src/tools/other/madis2nc/Makefile
                 src/tools/other/mode_graphics/Makefile
                 src/tools/other/modis_regrid/Makefile
                 src/tools/other/pb2nc/Makefile
                 src/tools/other/plot_data_plane/Makefile
                 src/tools/other/plot_point_obs/Makefile
                 src/tools/other/wwmca_tool/Makefile
                 src/tools/other/gsi_tools/Makefile
                 src/tools/other/regrid_data_plane/Makefile
                 src/tools/other/point2grid/Makefile
                 src/tools/other/shift_data_plane/Makefile
                 src/tools/other/mode_time_domain/Makefile
                 src/tools/other/grid_diag/Makefile
                 src/tools/tc_utils/Makefile
                 src/tools/tc_utils/tc_dland/Makefile
                 src/tools/tc_utils/tc_pairs/Makefile
                 src/tools/tc_utils/tc_stat/Makefile
                 src/tools/tc_utils/tc_gen/Makefile
                 src/tools/tc_utils/rmw_analysis/Makefile
                 src/tools/tc_utils/tc_rmw/Makefile])

if test -n "$MET_DEVELOPMENT"; then
   AC_CONFIG_FILES([src/tools/dev_utils/Makefile
                    src/tools/dev_utils/shapefiles/Makefile
                    internal_tests/Makefile
                    internal_tests/basic/Makefile
                    internal_tests/basic/vx_config/Makefile
                    internal_tests/basic/vx_log/Makefile
                    internal_tests/basic/vx_util/Makefile
                    internal_tests/libcode/Makefile
                    internal_tests/libcode/vx_data2d/Makefile
                    internal_tests/libcode/vx_data2d_factory/Makefile
                    internal_tests/libcode/vx_data2d_grib/Makefile
                    internal_tests/libcode/vx_data2d_nc_met/Makefile
                    internal_tests/libcode/vx_data2d_nccf/Makefile
                    internal_tests/libcode/vx_geodesy/Makefile
                    internal_tests/libcode/vx_grid/Makefile
                    internal_tests/libcode/vx_plot_util/Makefile
                    internal_tests/libcode/vx_ps/Makefile
                    internal_tests/libcode/vx_tc_util/Makefile
                    internal_tests/libcode/vx_nc_util/Makefile
                    internal_tests/libcode/vx_physics/Makefile
                    internal_tests/libcode/vx_series_data/Makefile
                    internal_tests/libcode/vx_solar/Makefile
                    internal_tests/tools/Makefile
                    internal_tests/tools/other/Makefile
                    internal_tests/tools/other/mode_time_domain/Makefile])
fi

AC_OUTPUT
