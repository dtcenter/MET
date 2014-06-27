#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([MET], [4.1], [met_help@ucar.edu], [], [http://www.dtcenter.org/met/users])
AC_CONFIG_SRCDIR([src/tools/tc_utils/tc_dland/tc_dland.cc])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.9 foreign])

# Process netCDF library specified by user

AC_MSG_CHECKING([whether a location for the NETCDF include files was specified])
AC_ARG_WITH([netcdf_inc],
              [AS_HELP_STRING([--with-netcdf_inc=<directory>],
                              [Specify location of NETCDF include files..])],
            [NETCDFINCDIR=$with_netcdf_inc])
AC_MSG_RESULT([$NETCDFINDIR])
AC_SUBST(NETCDFINCDIR, [$NETCDFINCDIR])

if test ! "x$NETCDFINCDIR" = x; then
   CPPFLAGS="${CPPFLAGS} -I${NETCDFINCDIR}"
fi    

AC_MSG_CHECKING([whether a location for the NETCDF library files was specified])
AC_ARG_WITH([netcdf_lib],
              [AS_HELP_STRING([--with-netcdf_lib=<directory>],
                              [Specify location of NETCDF library files..])],
            [NETCDFLIBDIR=$with_netcdf_lib])
AC_MSG_RESULT([$NETCDFLIBDIR])
AC_SUBST(NETCDFLIBDIR, [$NETCDFLIBDIR])

if test ! "x$NETCDFLIBDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${NETCDFLIBDIR}"
fi    

# Process GRIB2C library specified by user

AC_MSG_CHECKING([whether a location for the GRIB2C include files was specified])
AC_ARG_WITH([grib2c_inc],
              [AS_HELP_STRING([--with-grib2c_inc=<directory>],
                              [Specify location of GRIB2C include files.])],
            [GRIB2CINCDIR=$with_grib2c_inc])
AC_MSG_RESULT([$GRIB2CINCDIR])
AC_SUBST(GRIB2CINCDIR, [$GRIB2CINCDIR])

if test ! "x$GRIB2CINCDIR" = x; then
   CPPFLAGS="${CPPFLAGS} -I${GRIB2CINCDIR}"
fi    

AC_MSG_CHECKING([whether a location for the GRIB2C library files was specified])
AC_ARG_WITH([grib2c_lib],
              [AS_HELP_STRING([--with-grib2c_lib=<directory>],
                              [Specify location of GRIB2C library files.])],
            [GRIB2CLIBDIR=$with_grib2c_lib])
AC_MSG_RESULT([$GRIB2CLIBDIR])
AC_SUBST(GRIB2CLIBDIR, [$GRIB2CLIBDIR])

if test ! "x$GRIB2CLIBDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${GRIB2CLIBDIR}"
fi    

# Process GSL library specified by user

AC_MSG_CHECKING([whether a location for the GSL include files was specified])
AC_ARG_WITH([gsl_inc],
              [AS_HELP_STRING([--with-gsl_inc=<directory>],
                              [Specify location of GSL include files.])],
            [GSLINCDIR=$with_gsl_inc])
AC_MSG_RESULT([$GSLINCDIR])
AC_SUBST(GSLINCDIR, [$GSLINCDIR])

if test ! "x$GSLINCDIR" = x; then
   CPPFLAGS="${CPPFLAGS} -I${GSLINCDIR}"
fi    

AC_MSG_CHECKING([whether a location for the GSL library files was specified])
AC_ARG_WITH([gsl_lib],
              [AS_HELP_STRING([--with-gsl_lib=<directory>],
                              [Specify location of GSL library files.])],
            [GSLLIBDIR=$with_gsl_lib])
AC_MSG_RESULT([$GSLLIBDIR])
AC_SUBST(GSLLIBDIR, [$GSLLIBDIR])

if test ! "x$GSLLIBDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${GSLLIBDIR}"
fi    

# Process BUFR library specified by user

AC_MSG_CHECKING([whether a location for the BUFR include files was specified])
AC_ARG_WITH([bufr_inc],
              [AS_HELP_STRING([--with-bufr_inc=<directory>],
                              [Specify location of BUFR include files.])],
            [BUFRINCDIR=$with_bufr_inc])
AC_MSG_RESULT([$BUFRINCDIR])
AC_SUBST(BUFRINCDIR, [$BUFRINCDIR])

if test ! "x$BUFRINCDIR" = x; then
   CPPFLAGS="${CPPFLAGS} -I${BUFRINCDIR}"
fi    

AC_MSG_CHECKING([whether a location for the BUFR library files was specified])
AC_ARG_WITH([bufr_lib],
              [AS_HELP_STRING([--with-bufr_lib=<directory>],
                              [Specify location of BUFR library files.])],
            [BUFRLIBDIR=$with_bufr_lib])
AC_MSG_RESULT([$BUFRLIBDIR])
AC_SUBST(BUFRLIBDIR, [$BUFRLIBDIR])

if test ! "x$BUFRLIBDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${BUFRLIBDIR}"
fi    

# Process HDF library specified by user

AC_MSG_CHECKING([whether a location for the HDF include files was specified])
AC_ARG_WITH([hdf_inc],
              [AS_HELP_STRING([--with-hdf_inc=<directory>],
                              [Specify location of HDF include files.])],
            [HDFINCDIR=$with_hdf_inc])
AC_MSG_RESULT([$HDFINCDIR])
AC_SUBST(HDFINCDIR, [$HDFINCDIR])

if test ! "x$HDFINCDIR" = x; then
   CPPFLAGS="${CPPFLAGS} -I${HDFINCDIR}"
fi    

AC_MSG_CHECKING([whether a location for the HDF library files was specified])
AC_ARG_WITH([hdf_lib],
              [AS_HELP_STRING([--with-hdf_lib=<directory>],
                              [Specify location of HDF library files.])],
            [HDFLIBDIR=$with_hdf_lib])
AC_MSG_RESULT([$HDFLIBDIR])
AC_SUBST(HDFLIBDIR, [$HDFLIBDIR])

if test ! "x$HDFLIBDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${HDFLIBDIR}"
fi    

# Process HDF EOS library specified by user

AC_MSG_CHECKING([whether a location for the HDF EOS include files was specified])
AC_ARG_WITH([hdfeos_inc],
              [AS_HELP_STRING([--with-hdfeos_inc=<directory>],
                              [Specify location of HDF EOS include files.])],
            [HDFEOSINCDIR=$with_hdfeos_inc])
AC_MSG_RESULT([$HDFEOSINCDIR])
AC_SUBST(HDFEOSINCDIR, [$HDFEOSINCDIR])

if test ! "x$HDFEOSINCDIR" = x; then
   CPPFLAGS="${CPPFLAGS} -I${HDFEOSINCDIR}"
fi    

AC_MSG_CHECKING([whether a location for the HDF EOS library files was specified])
AC_ARG_WITH([hdfeos_lib],
              [AS_HELP_STRING([--with-hdfeos_lib=<directory>],
                              [Specify location of HDF EOS library files.])],
            [HDFEOSLIBDIR=$with_hdfeos_lib])
AC_MSG_RESULT([$HDFEOSLIBDIR])
AC_SUBST(HDFEOSLIBDIR, [$HDFEOSLIBDIR])

if test ! "x$HDFEOSLIBDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${HDFEOSLIBDIR}"
fi    

# Process CAIRO include and library directories specified by user

AC_MSG_CHECKING([whether a location for the CAIRO include files was specified])
AC_ARG_WITH([cairo_inc],
              [AS_HELP_STRING([--with-cairo_inc=<directory>],
                              [Specify location of CAIRO includes directory. This must be supplied if you are compiling mode_graphics.])],
            [CAIROINCDIR=$with_cairo_inc])
AC_MSG_RESULT([$CAIROINCDIR])
AC_SUBST(CAIROINCDIR, [$CAIROINCDIR])

if test ! "x$CAIROINCDIR" = x; then
   CPPFLAGS="${CPPFLAGS} -I${CAIROINCDIR}"
fi    

AC_MSG_CHECKING([whether a location for the CAIRO library files was specified])
AC_ARG_WITH([cairo_lib],
              [AS_HELP_STRING([--with-cairo_lib=<directory>],
                              [Specify location of CAIRO library directory. This must be supplied if you are compiling mode_graphics.])],
            [CAIROLIBDIR=$with_cairo_lib])
AC_MSG_RESULT([$CAIROLIBDIR])
AC_SUBST(CAIROLIBDIR, [$CAIROLIBDIR])

if test ! "x$CAIROLIBDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${CAIROLIBDIR}"
fi    

# Process FREETYPE include and library directories specified by user

AC_MSG_CHECKING([whether a location for the FREETYPE include files was specified])
AC_ARG_WITH([freetype_inc],
              [AS_HELP_STRING([--with-freetype_inc=<directory>],
                              [Specify location of FREETYPE includes directory. This must be supplied if you are compiling mode_graphics.])],
            [FREETYPEINCDIR=$with_freetype_inc])
AC_MSG_RESULT([$FREETYPEINCDIR])
AC_SUBST(FREETYPEINCDIR, [$FREETYPEINCDIR])

if test ! "x$FREETYPEINCDIR" = x; then
   CPPFLAGS="${CPPFLAGS} -I${FREETYPEINCDIR}"
fi    

AC_MSG_CHECKING([whether a location for the FREETYPE library files was specified])
AC_ARG_WITH([freetype_lib],
              [AS_HELP_STRING([--with-freetype_lib=<directory>],
                              [Specify location of FREETYPE library directory. This must be supplied if you are compiling mode_graphics.])],
            [FREETYPELIBDIR=$with_freetype_lib])
AC_MSG_RESULT([$FREETYPELIBDIR])
AC_SUBST(FREETYPELIBDIR, [$FREETYPELIBDIR])

if test ! "x$FREETYPELIBDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${FREETYPELIBDIR}"
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
else
    AC_MSG_NOTICE([GRIB2 utilities will not be compiled])
fi

# Temporarily define GS_FRONT_DIR here.  Not sure how we should do this operationally.

AC_SUBST([GS_FONT_DIR], [/usr/share/groff/1.21.font])

# Add MET_BASE to the CPPFLAGS

CPPFLAGS=$CPPFLAGS' -DMET_BASE="\"$(pkgdatadir)\""'

# Define other variables for the makefiles

AC_SUBST(FC_LIBS, [-lgfortran])

# Checks for programs.

AC_PROG_YACC
AC_PROG_CXX
AC_PROG_CC
AC_PROG_F77
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

AC_CONFIG_FILES([Makefile
                 data/Makefile
                 data/colortables/Makefile
                 data/colortables/NCL_colortables/Makefile
                 data/config/Makefile
                 data/map/Makefile
                 data/map/legacy/Makefile
                 data/map/update_2012/Makefile
                 data/map/update_2012/admin_by_country/Makefile
                 data/poly/Makefile
                 data/poly/HMT_masks/Makefile
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
AC_OUTPUT
