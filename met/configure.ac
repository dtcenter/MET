#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([MET], [4.1], [met_help@ucar.edu], [], [http://www.dtcenter.org/met/users])
AC_CONFIG_SRCDIR([src/tools/tc_utils/tc_dland/tc_dland.cc])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.9 foreign])

# Process netCDF library specified by user

AC_MSG_CHECKING([whether a location for the NETCDF library was specified])
AC_ARG_WITH([netcdf],
              [AS_HELP_STRING([--with-netcdf=<directory>],
                              [Specify location of NETCDF library. Configure will \
                              expect to find subdirs include and lib.])],
            [NETCDFDIR=$with_netcdf])
AC_MSG_RESULT([$NETCDFDIR])
AC_SUBST(NETCDFDIR, [$NETCDFDIR])

if test ! "x$NETCDFDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${NETCDFDIR}/lib"
   CPPFLAGS="${CPPFLAGS} -I${NETCDFDIR}/include"
fi    

# Process GRIB2C library specified by user

AC_MSG_CHECKING([whether a location for the GRIB2C library was specified])
AC_ARG_WITH([grib2c],
              [AS_HELP_STRING([--with-grib2c=<directory>],
                              [Specify location of GRIB2C library. Configure will \
                              expect to find subdirs include and lib.])],
            [GRIB2CDIR=$with_grib2c])
AC_MSG_RESULT([$GRIB2CDIR])
AC_SUBST(GRIB2CDIR, [$GRIB2CDIR])

if test ! "x$GRIB2CDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${GRIB2CDIR}/lib"
   CPPFLAGS="${CPPFLAGS} -I${GRIB2CDIR}/include"
fi    

# Process GSL library specified by user

AC_MSG_CHECKING([whether a location for the GSL library was specified])
AC_ARG_WITH([gsl],
              [AS_HELP_STRING([--with-gsl=<directory>],
                              [Specify location of GSL library. Configure will \
                              expect to find subdirs include and lib.])],
            [GSLDIR=$with_gsl])
AC_MSG_RESULT([$GSLDIR])
AC_SUBST(GSLDIR, [$GSLDIR])

if test ! "x$GSLDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${GSLDIR}/lib"
   CPPFLAGS="${CPPFLAGS} -I${GSLDIR}/include"
fi    

# Process BUFR library specified by user

AC_MSG_CHECKING([whether a location for the BUFR library was specified])
AC_ARG_WITH([bufr],
              [AS_HELP_STRING([--with-bufr=<directory>],
                              [Specify location of BUFR library. Configure will \
                              expect to find lib and include in this dir.])],
            [BUFRDIR=$with_bufr])
AC_MSG_RESULT([$BUFRDIR])
AC_SUBST(BUFRDIR, [$BUFRDIR])

if test ! "x$BUFRDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${BUFRDIR}"
   CPPFLAGS="${CPPFLAGS} -I${BUFRDIR}"
fi    

# Process HDF library specified by user

AC_MSG_CHECKING([whether a location for the HDF library was specified])
AC_ARG_WITH([hdf],
              [AS_HELP_STRING([--with-hdf=<directory>],
                              [Specify location of HDF library. Configure will \
                              expect to find subdirs include and lib.])],
            [HDFDIR=$with_hdf])
AC_MSG_RESULT([$HDFDIR])
AC_SUBST(HDFDIR, [$HDFDIR])

if test ! "x$HDFDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${HDFDIR}/lib"
   CPPFLAGS="${CPPFLAGS} -I${HDFDIR}/include"
fi    

# Process HDF EOS library specified by user

AC_MSG_CHECKING([whether a location for the HDF EOS library was specified])
AC_ARG_WITH([hdfeos],
              [AS_HELP_STRING([--with-hdfeos=<directory>],
                              [Specify location of HDF EOS library. Configure will \
                              expect to find subdirs include and lib.])],
            [HDFEOSDIR=$with_hdfeos])
AC_MSG_RESULT([$HDFEOSDIR])
AC_SUBST(HDFDIR, [$HDFEOSDIR])

if test ! "x$HDFEOSDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${HDFEOSDIR}/lib"
   CPPFLAGS="${CPPFLAGS} -I${HDFEOSDIR}/include"
fi    

# Process any enable flags specified by the user

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

# Add MET_BASE to the CPPFLAGS

CPPFLAGS=$CPPFLAGS' -DMET_BASE="\"$(pkgdatadir)\""'
#CPPFLAGS="${CPPFLAGS} -DMET_BASE=\"${pkgdatadir}\""

# Checks for programs.

AC_PROG_YACC
AC_PROG_CXX
AC_PROG_CC
AC_PROG_F77
AC_PROG_LEX
AC_PROG_RANLIB

# Checks for libraries.

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
                 src/tools/other/gen_poly_mask/Makefile
                 src/tools/other/madis2nc/Makefile
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
