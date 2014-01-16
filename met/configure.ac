#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([MET], [4.1], [met_help@ucar.edu], [], [http://www.dtcenter.org/met/users])
AC_CONFIG_SRCDIR([src/tools/tc_utils/tc_dland/tc_dland.cc])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.9 foreign])

# Did the user specify a location for the NETCDF library?
AC_MSG_CHECKING([whether a location for the NETCDF library was specified])
AC_ARG_WITH([netcdf],
              [AS_HELP_STRING([--with-netcdf=<directory>],
                              [Specify location of NETCDF library. Configure will \
                              expect to find subdirs include and lib.])],
            [NETCDFDIR=$with_netcdf])
AC_MSG_RESULT([$NETCDFDIR])
AC_SUBST(NETCDFDIR, [$NETCDFDIR])

# Add the NETCDF directory to includes and link flags
if test ! "x$NETCDFDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${NETCDFDIR}/lib"
   CPPFLAGS="${CPPFLAGS} -I${NETCDFDIR}/include"
fi    

# Did the user specify a location for the GRIB2C library?
AC_MSG_CHECKING([whether a location for the GRIB2C library was specified])
AC_ARG_WITH([grib2c],
              [AS_HELP_STRING([--with-grib2c=<directory>],
                              [Specify location of GRIB2C library. Configure will \
                              expect to find subdirs include and lib.])],
            [GRIB2CDIR=$with_grib2c])
AC_MSG_RESULT([$GRIB2CDIR])
AC_SUBST(GRIB2CDIR, [$GRIB2CDIR])

# Add the GRIB2C directory to includes and link flags
if test ! "x$GRIB2CDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${GRIB2CDIR}/lib"
   CPPFLAGS="${CPPFLAGS} -I${GRIB2CDIR}/include"
fi    

# Did the user specify a location for the GSL library?
AC_MSG_CHECKING([whether a location for the GSL library was specified])
AC_ARG_WITH([gsl],
              [AS_HELP_STRING([--with-gsl=<directory>],
                              [Specify location of GSL library. Configure will \
                              expect to find subdirs include and lib.])],
            [GSLDIR=$with_gsl])
AC_MSG_RESULT([$GSLDIR])
AC_SUBST(GSLDIR, [$GSLDIR])

# Add the GSL directory to includes and link flags
if test ! "x$GSLDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${GSLDIR}/lib"
   CPPFLAGS="${CPPFLAGS} -I${GSLDIR}/include"
fi    

# Did the user specify a location for the BUFR library?
AC_MSG_CHECKING([whether a location for the BUFR library was specified])
AC_ARG_WITH([bufr],
              [AS_HELP_STRING([--with-bufr=<directory>],
                              [Specify location of BUFR library. Configure will \
                              expect to find lib and include in this dir.])],
            [BUFRDIR=$with_bufr])
AC_MSG_RESULT([$BUFRDIR])
AC_SUBST(BUFRDIR, [$BUFRDIR])

# Add the BUFR directory to includes and link flags
if test ! "x$BUFRDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${BUFRDIR}"
   CPPFLAGS="${CPPFLAGS} -I${BUFRDIR}"
fi    

# Did the user specify a location for the HDF library?
AC_MSG_CHECKING([whether a location for the HDF library was specified])
AC_ARG_WITH([hdf],
              [AS_HELP_STRING([--with-hdf=<directory>],
                              [Specify location of HDF library. Configure will \
                              expect to find subdirs include and lib.])],
            [HDFDIR=$with_hdf])
AC_MSG_RESULT([$HDFDIR])
AC_SUBST(HDFDIR, [$HDFDIR])

# Add the HDF directory to includes and link flags
if test ! "x$HDFDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${HDFDIR}/lib"
   CPPFLAGS="${CPPFLAGS} -I${HDFDIR}/include"
fi    

# Did the user specify a location for the HDF EOS library?
AC_MSG_CHECKING([whether a location for the HDF EOS library was specified])
AC_ARG_WITH([hdfeos],
              [AS_HELP_STRING([--with-hdfeos=<directory>],
                              [Specify location of HDF EOS library. Configure will \
                              expect to find subdirs include and lib.])],
            [HDFEOSDIR=$with_hdfeos])
AC_MSG_RESULT([$HDFEOSDIR])
AC_SUBST(HDFDIR, [$HDFEOSDIR])

# Add the HDF EOS directory to includes and link flags
if test ! "x$HDFEOSDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${HDFEOSDIR}/lib"
   CPPFLAGS="${CPPFLAGS} -I${HDFEOSDIR}/include"
fi    

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
