#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([MET], [5.0], [met_help@ucar.edu], [], [http://www.dtcenter.org/met/users])
AC_CONFIG_SRCDIR([src/tools/tc_utils/tc_dland/tc_dland.cc])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.9 foreign])

#
# Look for the NetCDF library
#

# Configure the variables.  The help text will appear if the user uses "configure --help".

AC_ARG_VAR([NETCDF], [Where netcdf lib and include subdirectories are located.  If they are installed in /usr/local, you don't have to specify them.])
AC_ARG_VAR([NETCDFINC], [Where netcdf include files are located. Use if the libraries and include files are not under the same main netcdf directory.  If used, NETCDFLIB must also be defined and NETCDF will be ignored.])
AC_ARG_VAR([NETCDFLIB], [Where netcdf libraries are located. Use if the libraries and include files are not under the same main netcdf directory.  If used, NETCDFINC must also be defined and NETCDF will be ignored.])

# Ensure that both or neither are specified.

if (test -n "$NETCDFLIB" && test -z "$NETCDFINC") || \
   (test -n "$NETCDFINC" && test -z "$NETCDFLIB"); then
  AC_MSG_ERROR([both or neither NETCDF variables])
fi 

# If the individual locations weren't specified, then the main NetCDF directory
# must be specified.  If it isn't, let the user know they need to supply one.

if test -z "$NETCDFLIB" && test -n "$NETCDF"; then
   NETCDFINC='${NETCDF}/include'
   NETCDFLIB='${NETCDF}/lib'
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

if test -n "$NETCDFLIB"; then
   CPPFLAGS="${CPPFLAGS} -I${NETCDFINC}"
   LDFLAGS="${LDFLAGS} -L${NETCDFLIB}"
fi

#
# Look for the GRIB2C library
#

# Configure the variables.  The help text will appear if the user uses "configure --help".

AC_ARG_VAR([GRIB2C], [Where grib2c lib and include subdirectories are located.  If they are installed in /usr/local, you don't have to specify them.])
AC_ARG_VAR([GRIB2CINC], [Where grib2c include files are located. Use if the libraries and include files are not under the same main grib2c directory.  If used, GRIB2CLIB must also be defined and GRIB2C will be ignored.])
AC_ARG_VAR([GRIB2CLIB], [Where grib2c libraries are located. Use if the libraries and include files are not under the same main grib2c directory.  If used, GRIB2CINC must also be defined and GRIB2C will be ignored.])

# Ensure that both or neither are specified.

if (test -n "$GRIB2CLIB" && test -z "$GRIB2CINC") || \
   (test -n "$GRIB2CINC" && test -z "$GRIB2CLIB"); then
  AC_MSG_ERROR([both or neither GRIB2C variables])
fi 

# If the individual locations weren't specified, then the main Grib2c directory
# must be specified.  If it isn't, let the user know they need to supply one.

if test -z "$GRIB2CLIB" && test -n "$GRIB2C"; then
   GRIB2CINC='${GRIB2C}/include'
   GRIB2CLIB='${GRIB2C}/lib'
fi

# AC_SUBST command line variables.  If we get here, the Grib2c
# libraries were found so no need to check

if test -n "$GRIB2CLIB"; then
   CPPFLAGS="${CPPFLAGS} -I${GRIB2CINC}"
   LDFLAGS="${LDFLAGS} -L${GRIB2CLIB}"
fi

#
# Look for the GSL library
#

# Configure the variables.  The help text will appear if the user uses "configure --help".

AC_ARG_VAR([GSL], [Where gsl lib and include subdirectories are located.  If they are installed in /usr/local, you don't have to specify them.])
AC_ARG_VAR([GSLINC], [Where gsl include files are located. Use if the libraries and include files are not under the same main gsl directory.  If used, GSLLIB must also be defined and GSL will be ignored.])
AC_ARG_VAR([GSLLIB], [Where gsl libraries are located. Use if the libraries and include files are not under the same main gsl directory.  If used, GSLINC must also be defined and GSL will be ignored.])

# Ensure that both or neither are specified.

if (test -n "$GSLLIB" && test -z "$GSLINC") || \
   (test -n "$GSLINC" && test -z "$GSLLIB"); then
  AC_MSG_ERROR([both or neither GSL variables])
fi 

# If the individual locations weren't specified, then the main Gsl directory
# must be specified.  If it isn't, let the user know they need to supply one.

if test -z "$GSLLIB" && test -n "$GSL"; then
   GSLINC='${GSL}/include'
   GSLLIB='${GSL}/lib'
fi

# AC_SUBST command line variables.  If we get here, the Gsl
# libraries were found so no need to check

if test -n "$GSLLIB"; then
   CPPFLAGS="${CPPFLAGS} -I${GSLINC}"
   LDFLAGS="${LDFLAGS} -L${GSLLIB}"
fi

#
# Look for the BUFR library
#

# Configure the variables.  The help text will appear if the user uses "configure --help".

AC_ARG_VAR([BUFR], [Where bufr lib and include subdirectories are located.  If they are installed in /usr/local, you don't have to specify them.])
AC_ARG_VAR([BUFRINC], [Where bufr include files are located. Use if the libraries and include files are not under the same main bufr directory.  If used, BUFRLIB must also be defined and BUFR will be ignored.])
AC_ARG_VAR([BUFRLIB], [Where bufr libraries are located. Use if the libraries and include files are not under the same main bufr directory.  If used, BUFRINC must also be defined and BUFR will be ignored.])

# Ensure that both or neither are specified.

if (test -n "$BUFRLIB" && test -z "$BUFRINC") || \
   (test -n "$BUFRINC" && test -z "$BUFRLIB"); then
  AC_MSG_ERROR([both or neither BUFR variables])
fi 

# If the individual locations weren't specified, then the main Bufr directory
# must be specified.  If it isn't, let the user know they need to supply one.

if test -z "$BUFRLIB" && test -n "$BUFR"; then
   BUFRINC='${BUFR}/include'
   BUFRLIB='${BUFR}/lib'
fi

# AC_SUBST command line variables.  If we get here, the Bufr
# libraries were found so no need to check

if test -n "$BUFRLIB"; then
   CPPFLAGS="${CPPFLAGS} -I${BUFRINC}"
   LDFLAGS="${LDFLAGS} -L${BUFRLIB}"
fi

#
# Look for the HDF library
#

# Configure the variables.  The help text will appear if the user uses "configure --help".

AC_ARG_VAR([HDF], [Where hdf lib and include subdirectories are located.  If they are installed in /usr/local, you don't have to specify them.])
AC_ARG_VAR([HDFINC], [Where hdf include files are located. Use if the libraries and include files are not under the same main hdf directory.  If used, HDFLIB must also be defined and HDF will be ignored.])
AC_ARG_VAR([HDFLIB], [Where hdf libraries are located. Use if the libraries and include files are not under the same main hdf directory.  If used, HDFINC must also be defined and HDF will be ignored.])

# Ensure that both or neither are specified.

if (test -n "$HDFLIB" && test -z "$HDFINC") || \
   (test -n "$HDFINC" && test -z "$HDFLIB"); then
  AC_MSG_ERROR([both or neither HDF variables])
fi 

# If the individual locations weren't specified, then the main Hdf directory
# must be specified.  If it isn't, let the user know they need to supply one.

if test -z "$HDFLIB" && test -n "$HDF"; then
   HDFINC='${HDF}/include'
   HDFLIB='${HDF}/lib'
fi

# AC_SUBST command line variables.  If we get here, the Hdf
# libraries were found so no need to check

if test -n "$HDFLIB"; then
   CPPFLAGS="${CPPFLAGS} -I${HDFINC}"
   LDFLAGS="${LDFLAGS} -L${HDFLIB}"
fi

#
# Look for the Hdfeos library
#

# Configure the variables.  The help text will appear if the user uses "configure --help".

AC_ARG_VAR([HDFEOS], [Where hdfeos lib and include subdirectories are located.  If they are installed in /usr/local, you don't have to specify them.])
AC_ARG_VAR([HDFEOSINC], [Where hdfeos include files are located. Use if the libraries and include files are not under the same main hdfeos directory.  If used, HDFEOSLIB must also be defined and HDFEOS will be ignored.])
AC_ARG_VAR([HDFEOSLIB], [Where hdfeos libraries are located. Use if the libraries and include files are not under the same main hdfeos directory.  If used, HDFEOSINC must also be defined and HDFEOS will be ignored.])

# Ensure that both or neither are specified.

if (test -n "$HDFEOSLIB" && test -z "$HDFEOSINC") || \
   (test -n "$HDFEOSINC" && test -z "$HDFEOSLIB"); then
  AC_MSG_ERROR([both or neither HDFEOS variables])
fi 

# If the individual locations weren't specified, then the main Hdfeos directory
# must be specified.  If it isn't, let the user know they need to supply one.

if test -z "$HDFEOSLIB" && test -n "$HDFEOS"; then
   HDFEOSINC='${HDFEOS}/include'
   HDFEOSLIB='${HDFEOS}/lib'
fi

# AC_SUBST command line variables.  If we get here, the Hdfeos
# libraries were found so no need to check

if test -n "$HDFEOSLIB"; then
   CPPFLAGS="${CPPFLAGS} -I${HDFEOSINC}"
   LDFLAGS="${LDFLAGS} -L${HDFEOSLIB}"
fi

#
# Look for the Cairo library
#

# Configure the variables.  The help text will appear if the user uses "configure --help".

AC_ARG_VAR([CAIRO], [Where cairo lib and include subdirectories are located.  If they are installed in /usr/local, you don't have to specify them. Cairo is only needed if you are building the mode_graphics utility.])
AC_ARG_VAR([CAIROINC], [Where cairo include files are located. Use if the libraries and include files are not under the same main cairo directory.  If used, CAIROLIB must also be defined and CAIRO will be ignored.])
AC_ARG_VAR([CAIROLIB], [Where cairo libraries are located. Use if the libraries and include files are not under the same main cairo directory.  If used, CAIROINC must also be defined and CAIRO will be ignored.])

# Ensure that both or neither are specified.

if (test -n "$CAIROLIB" && test -z "$CAIROINC") || \
   (test -n "$CAIROINC" && test -z "$CAIROLIB"); then
  AC_MSG_ERROR([both or neither CAIRO variables])
fi 

# If the individual locations weren't specified, then the main Cairo directory
# must be specified.  If it isn't, let the user know they need to supply one.

if test -z "$CAIROLIB" && test -n "$CAIRO"; then
   CAIROINC='${CAIRO}/include'
   CAIROLIB='${CAIRO}/lib'
fi

# AC_SUBST command line variables.  If we get here, the Cairo
# libraries were found so no need to check

if test -n "$CAIROLIB"; then
   CPPFLAGS="${CPPFLAGS} -I${CAIROINC}"
   LDFLAGS="${LDFLAGS} -L${CAIROLIB}"
fi

#
# Look for the Freetype library
#

# Configure the variables.  The help text will appear if the user uses "configure --help".

AC_ARG_VAR([FREETYPE], [Where freetype lib and include subdirectories are located.  If they are installed in /usr/local, you don't have to specify them. Freetype is only needed if you are building the mode_graphics utility.])
AC_ARG_VAR([FREETYPEINC], [Where freetype include files are located. Use if the libraries and include files are not under the same main freetype directory.  If used, FREETYPELIB must also be defined and FREETYPE will be ignored.])
AC_ARG_VAR([FREETYPELIB], [Where freetype libraries are located. Use if the libraries and include files are not under the same main freetype directory.  If used, FREETYPEINC must also be defined and FREETYPE will be ignored.])

# Ensure that both or neither are specified.

if (test -n "$FREETYPELIB" && test -z "$FREETYPEINC") || \
   (test -n "$FREETYPEINC" && test -z "$FREETYPELIB"); then
  AC_MSG_ERROR([both or neither FREETYPE variables])
fi 

# If the individual locations weren't specified, then the main Freetype directory
# must be specified.  If it isn't, let the user know they need to supply one.

if test -z "$FREETYPELIB" && test -n "$FREETYPE"; then
   FREETYPEINC='${FREETYPE}/include'
   FREETYPELIB='${FREETYPE}/lib'
fi

# AC_SUBST command line variables.  If we get here, the Freetype
# libraries were found so no need to check

if test -n "$FREETYPELIB"; then
   CPPFLAGS="${CPPFLAGS} -I${FREETYPEINC}"
   LDFLAGS="${LDFLAGS} -L${FREETYPELIB}"
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

# gen_poly_mask

AC_ARG_ENABLE(gen_poly_mask,
	  [AS_HELP_STRING([--disable-gen_poly_mask], [Disable compilation of gen_poly_mask])],
  [case "${enableval}" in
     yes | no ) ENABLE_GEN_POLY_MASK="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-gen_poly_mask) ;;
   esac],
  [ENABLE_GEN_POLY_MASK="yes"]
)

AM_CONDITIONAL([ENABLE_GEN_POLY_MASK], [test "x$ENABLE_GEN_POLY_MASK" = "xyes"])

if test "x$ENABLE_GEN_POLY_MASK" = "xyes"; then
    AC_DEFINE([ENABLE_GEN_POLY_MASK], [], ["build gen_poly_mask"])
    AC_MSG_NOTICE([gen_poly_mask will be compiled])
else
    AC_MSG_NOTICE([gen_poly_mask will not be compiled])
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
	  [AS_HELP_STRING([--disable-modis], [Disable compilation of modis])],
  [case "${enableval}" in
     yes | no ) ENABLE_MODIS="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-modis) ;;
   esac],
  [ENABLE_MODIS="yes"]
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

# gen_circle_mask

AC_ARG_ENABLE(gen_circle_mask,
	  [AS_HELP_STRING([--disable-gen_circle_mask], [Disable compilation of gen_circle_mask])],
  [case "${enableval}" in
     yes | no ) ENABLE_GEN_CIRCLE_MASK="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-gen_circle_mask) ;;
   esac],
  [ENABLE_GEN_CIRCLE_MASK="yes"]
)

AM_CONDITIONAL([ENABLE_GEN_CIRCLE_MASK], [test "x$ENABLE_GEN_CIRCLE_MASK" = "xyes"])

if test "x$ENABLE_GEN_CIRCLE_MASK" = "xyes"; then
    AC_DEFINE([ENABLE_GEN_CIRCLE_MASK], [], ["build gen_circle_mask"])
    AC_MSG_NOTICE([gen_circle_mask will be compiled])
else
    AC_MSG_NOTICE([gen_circle_mask will not be compiled])
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

if test "x$ENABLE_GRIB2" = "xyes"; then
    AC_DEFINE([ENABLE_GRIB2], [], ["build GRIB2 utilities"])
    AC_MSG_NOTICE([GRIB2 utilites will be compiled])
    CPPFLAGS="${CPPFLAGS} -DWITH_GRIB2"
    GRIB2_LIBS="-lvx_data2d_grib2 -lgrib2c -ljasper -lpng -lz"
else
    AC_MSG_NOTICE([GRIB2 utilities will not be compiled])
    GRIB2_LIBS=
fi
AC_SUBST([GRIB2_LIBS])

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
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([dup2 floor gethostname memset mkdir pow regcomp select setenv sqrt strcasecmp strchr strcspn strerror strncasecmp strrchr strstr])

#AC_CONFIG_FILES([Makefile
#                 data/Makefile
#                 internal_tests/Makefile
#                 internal_tests/basic/Makefile
#                 internal_tests/basic/vx_config/Makefile
#                 internal_tests/basic/vx_log/Makefile
#                 internal_tests/basic/vx_util/Makefile
#                 internal_tests/libcode/Makefile
#                 internal_tests/libcode/vx_data2d/Makefile
#                 internal_tests/libcode/vx_data2d_factory/Makefile
#                 internal_tests/libcode/vx_data2d_grib/Makefile
#                 internal_tests/libcode/vx_data2d_nc_met/Makefile
#                 internal_tests/libcode/vx_data2d_nccf/Makefile
#                 internal_tests/libcode/vx_geodesy/Makefile
#                 internal_tests/libcode/vx_plot_util/Makefile
#                 internal_tests/libcode/vx_ps/Makefile
#                 internal_tests/libcode/vx_tc_util/Makefile
#                 src/Makefile
#                 src/basic/Makefile
#                 src/basic/enum_to_string/Makefile
#                 src/basic/vx_cal/Makefile
#                 src/basic/vx_config/Makefile
#                 src/basic/vx_log/Makefile
#                 src/basic/vx_math/Makefile
#                 src/basic/vx_util/Makefile
#                 src/libcode/Makefile
#                 src/libcode/vx_afm/Makefile
#                 src/libcode/vx_analysis_util/Makefile
#                 src/libcode/vx_color/Makefile
#                 src/libcode/vx_data2d/Makefile
#                 src/libcode/vx_data2d_factory/Makefile
#                 src/libcode/vx_data2d_grib/Makefile
#                 src/libcode/vx_data2d_grib2/Makefile
#                 src/libcode/vx_data2d_nc_met/Makefile
#                 src/libcode/vx_data2d_nc_pinterp/Makefile
#                 src/libcode/vx_data2d_nccf/Makefile
#                 src/libcode/vx_geodesy/Makefile
#                 src/libcode/vx_gnomon/Makefile
#                 src/libcode/vx_grid/Makefile
#                 src/libcode/vx_gsl_prob/Makefile
#                 src/libcode/vx_nav/Makefile
#                 src/libcode/vx_nc_util/Makefile
#                 src/libcode/vx_pb_util/Makefile
#                 src/libcode/vx_plot_util/Makefile
#                 src/libcode/vx_ps/Makefile
#                 src/libcode/vx_pxm/Makefile
#                 src/libcode/vx_render/Makefile
#                 src/libcode/vx_shapedata/Makefile
#                 src/libcode/vx_stat_out/Makefile
#                 src/libcode/vx_statistics/Makefile
#                 src/libcode/vx_tc_util/Makefile
#                 src/tools/Makefile
#                 src/tools/core/Makefile
#                 src/tools/core/ensemble_stat/Makefile
#                 src/tools/core/grid_stat/Makefile
#                 src/tools/core/mode/Makefile
#                 src/tools/core/mode_analysis/Makefile
#                 src/tools/core/pcp_combine/Makefile
#                 src/tools/core/point_stat/Makefile
#                 src/tools/core/series_analysis/Makefile
#                 src/tools/core/stat_analysis/Makefile
#                 src/tools/core/wavelet_stat/Makefile
#                 src/tools/dev_utils/Makefile
#                 src/tools/other/Makefile
#                 src/tools/other/ascii2nc/Makefile
#                 src/tools/other/gen_circle_mask/Makefile
#                 src/tools/other/gen_poly_mask/Makefile
#                 src/tools/other/madis2nc/Makefile
#                 src/tools/other/mode_graphics/Makefile
#                 src/tools/other/modis_regrid/Makefile
#                 src/tools/other/pb2nc/Makefile 
#                 src/tools/other/plot_data_plane/Makefile
#                 src/tools/other/plot_point_obs/Makefile
#                 src/tools/other/wwmca_tool/Makefile
#                 src/tools/tc_utils/Makefile
#                 src/tools/tc_utils/tc_dland/Makefile
#                 src/tools/tc_utils/tc_pairs/Makefile
#                 src/tools/tc_utils/tc_stat/Makefile])
AC_CONFIG_FILES([Makefile
                 data/Makefile
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
                 src/libcode/vx_gnomon/Makefile
                 src/libcode/vx_grid/Makefile
                 src/libcode/vx_gsl_prob/Makefile
                 src/libcode/vx_nav/Makefile
                 src/libcode/vx_nc_util/Makefile
                 src/libcode/vx_pb_util/Makefile
                 src/libcode/vx_plot_util/Makefile
                 src/libcode/vx_ps/Makefile
                 src/libcode/vx_pxm/Makefile
                 src/libcode/vx_render/Makefile
                 src/libcode/vx_shapedata/Makefile
                 src/libcode/vx_stat_out/Makefile
                 src/libcode/vx_statistics/Makefile
                 src/libcode/vx_tc_util/Makefile
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
                 src/tools/dev_utils/Makefile
                 src/tools/other/Makefile
                 src/tools/other/ascii2nc/Makefile
                 src/tools/other/gen_circle_mask/Makefile
                 src/tools/other/gen_poly_mask/Makefile
                 src/tools/other/madis2nc/Makefile
                 src/tools/other/mode_graphics/Makefile
                 src/tools/other/modis_regrid/Makefile
                 src/tools/other/pb2nc/Makefile 
                 src/tools/other/plot_data_plane/Makefile
                 src/tools/other/plot_point_obs/Makefile
                 src/tools/other/wwmca_tool/Makefile
                 src/tools/tc_utils/Makefile
                 src/tools/tc_utils/tc_dland/Makefile
                 src/tools/tc_utils/tc_pairs/Makefile
                 src/tools/tc_utils/tc_stat/Makefile])
#if test -n "$MET_DEVELOPMENT"; then
   AC_CONFIG_FILES([internal_tests/Makefile
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
                    internal_tests/libcode/vx_plot_util/Makefile
                    internal_tests/libcode/vx_ps/Makefile
                    internal_tests/libcode/vx_tc_util/Makefile])
#fi
AC_OUTPUT
