cmake_minimum_required(VERSION 3.26)
set(CMAKE_FORTRAN_COMPILER $ENV{FC})

project(MET LANGUAGES CXX Fortran)

set(CMAKE_CXX_STANDARD 17)

# ensure -std=c++xx instead of -std=g++xx
set(CMAKE_CXX_EXTENSIONS OFF)

# env vars that need to be set:
# MET_PYTHON_LD
# MET_PYTHON_BIN_EXE
# MET_GRIB2CINC
# PYTHON_INC

# if compiling with unstructured grid support
# TODO: make this optional with if
add_definitions(-DWITH_UGRID -DWITH_PYTHON -DWITH_GRIB2)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "Flags used by the C compiler")

# write executables into bin directory under build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")


# set variable to build development tools (src/tools/dev_utils and internal/test_util) if MET_DEVELOPMENT env var is set
# TODO: do something with BUILD_DEV_TOOLS variable
if(DEFINED ENV{MET_DEVELOPMENT})
  set(BUILD_DEV_TOOLS ON)
else()
  set(BUILD_DEV_TOOLS OFF)
endif()

# Set the BUFRLIB_NAME variable based on the environment variable or default value
if(DEFINED ENV{BUFRLIB_NAME})
    set(BUFRLIB_NAME $ENV{BUFRLIB_NAME})
else()
    set(BUFRLIB_NAME "-lbufr")
endif()

if(DEFINED ENV{GRIB2CLIB_NAME})
    set(GRIB2CLIB_NAME $ENV{GRIB2CLIB_NAME})
else()
    set(GRIB2CLIB_NAME "-lg2c")
endif()


set(PYTHON_DEP_LIBS $ENV{MET_PYTHON_LD})
set(GRIB2_MET_LIBS vx_data2d_grib2)
set(UGRID_MET_LIBS vx_data2d_ugrid)
set(PYTHON_MET_LIBS vx_data2d_python vx_pointdata_python vx_python3_utils)
set(GRIB2_DEP_LIBS ${GRIB2CLIB_NAME} -ljasper -lpng -lz)
set(UGRID_DEP_LIBS -latlas -leckit -leckit_mpi -leckit_geometry)

set(MET_BASE "${PROJECT_BINARY_DIR}/share/met")

add_definitions(-DMET_PYTHON_BIN_EXE="$ENV{MET_PYTHON_BIN_EXE}")
add_definitions(-DCOPYRIGHT_DIR="./data")
add_definitions(-DMET_BASE="${MET_BASE}")

# List of environment variables to check
set(dirs_to_link MET_GRIB2CLIB MET_BUFRLIB)

foreach(var IN LISTS dirs_to_link)
    if(DEFINED ENV{${var}})
        link_directories($ENV{${var}})
    else()
        message(DEBUG "Environment variable ${var} is not set. Skipping link_directories.")
    endif()
endforeach()


# TODO: may need to get python include directory to run python embedding -- PYTHON_INC isn't currently an env var used
# we could consider using MET_PYTHON_CC to pass the -I arguments or parse out the -I argument so it can be included in the CMake project

#include_directories(/Users/mccabe/met_install/12.0.0-beta2/external_libs/include)
#include_directories(/Users/mccabe/.conda/envs/met-python3-arm64/include/python3.10)
# TODO: add all other env vars that control include directories (MET_XXXINC and MET_XXX/include?)
include_directories($ENV{MET_GRIB2CINC})
include_directories($ENV{PYTHON_INC})
include_directories($ENV{FAKE_UNSET})
include_directories(.)
include_directories(src/basic/enum_to_string)
include_directories(src/basic/vx_cal)
include_directories(src/basic/vx_config)
include_directories(src/basic/vx_log)
include_directories(src/basic/vx_math)
include_directories(src/basic/vx_util)
include_directories(src/basic/vx_util_math)
include_directories(src/libcode/vx_afm)
include_directories(src/libcode/vx_analysis_util)
include_directories(src/libcode/vx_bool_calc)
include_directories(src/libcode/vx_color)
include_directories(src/libcode/vx_data2d)
include_directories(src/libcode/vx_data2d_factory)
include_directories(src/libcode/vx_data2d_grib)
include_directories(src/libcode/vx_data2d_grib2)
include_directories(src/libcode/vx_data2d_nc_cf)
include_directories(src/libcode/vx_data2d_nc_met)
include_directories(src/libcode/vx_data2d_nc_wrf)
include_directories(src/libcode/vx_data2d_python)
include_directories(src/libcode/vx_data2d_ugrid)
include_directories(src/libcode/vx_geodesy)
include_directories(src/libcode/vx_gis)
include_directories(src/libcode/vx_gnomon)
include_directories(src/libcode/vx_grid)
include_directories(src/libcode/vx_gsl_prob)
include_directories(src/libcode/vx_ioda)
include_directories(src/libcode/vx_nav)
include_directories(src/libcode/vx_nc_obs)
include_directories(src/libcode/vx_nc_util)
include_directories(src/libcode/vx_pb_util)
include_directories(src/libcode/vx_physics)
include_directories(src/libcode/vx_plot_util)
include_directories(src/libcode/vx_pointdata_python)
include_directories(src/libcode/vx_ps)
include_directories(src/libcode/vx_pxm)
include_directories(src/libcode/vx_python3_utils)
include_directories(src/libcode/vx_regrid)
include_directories(src/libcode/vx_render)
include_directories(src/libcode/vx_seeps)
include_directories(src/libcode/vx_series_data)
include_directories(src/libcode/vx_shapedata)
include_directories(src/libcode/vx_solar)
include_directories(src/libcode/vx_stat_out)
include_directories(src/libcode/vx_statistics)
include_directories(src/libcode/vx_summary)
include_directories(src/libcode/vx_tc_util)
include_directories(src/libcode/vx_time_series)
include_directories(src/tools/core/ensemble_stat)
include_directories(src/tools/core/grid_stat)
include_directories(src/tools/core/mode)
include_directories(src/tools/core/mode_analysis)
include_directories(src/tools/core/point_stat)
include_directories(src/tools/core/series_analysis)
include_directories(src/tools/core/stat_analysis)
include_directories(src/tools/core/wavelet_stat)
include_directories(src/tools/dev_utils)
include_directories(src/tools/other/ascii2nc)
include_directories(src/tools/other/gen_ens_prod)
include_directories(src/tools/other/gen_vx_mask)
include_directories(src/tools/other/grid_diag)
include_directories(src/tools/other/gsi_tools)
include_directories(src/tools/other/ioda2nc)
# TODO: control including directories based on if they are enabled
#include_directories(src/tools/other/lidar2nc)
include_directories(src/tools/other/madis2nc)
include_directories(src/tools/other/mode_graphics)
include_directories(src/tools/other/mode_time_domain)
#include_directories(src/tools/other/modis_regrid)
include_directories(src/tools/other/pb2nc)
include_directories(src/tools/other/plot_point_obs)
include_directories(src/tools/other/point2grid)
include_directories(src/tools/other/wwmca_tool)
include_directories(src/tools/tc_utils/rmw_analysis)
include_directories(src/tools/tc_utils/tc_diag)
include_directories(src/tools/tc_utils/tc_dland)
include_directories(src/tools/tc_utils/tc_gen)
include_directories(src/tools/tc_utils/tc_pairs)
include_directories(src/tools/tc_utils/tc_rmw)
include_directories(src/tools/tc_utils/tc_stat)

###
# install data files into share/met
###

file(MAKE_DIRECTORY ${MET_BASE}/colortables/NCL_colortables)
file(MAKE_DIRECTORY ${MET_BASE}/config)
file(MAKE_DIRECTORY ${MET_BASE}/map/admin_by_country)

# data/version.txt

add_custom_command(OUTPUT version.txt
        COMMAND ${CMAKE_COMMAND} -E copy
        ${PROJECT_SOURCE_DIR}/data/version.txt
        ${PROJECT_BINARY_DIR}/share/met/version.txt
        VERBATIM
)

# data/climo

add_custom_command(OUTPUT climo
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/data/climo/seeps
        ${PROJECT_BINARY_DIR}/share/met/climo/seeps/
)

# data/colortables

file(GLOB COLORTABLE_FILES "${PROJECT_SOURCE_DIR}/data/colortables/*")
list(FILTER COLORTABLE_FILES EXCLUDE REGEX "Makefile*")
add_custom_command(OUTPUT colortables
        COMMAND ${CMAKE_COMMAND} -E copy
        ${COLORTABLE_FILES}
        ${PROJECT_BINARY_DIR}/share/met/colortables/
        VERBATIM
)

file(GLOB NCLCOLORTABLE_FILES "${PROJECT_SOURCE_DIR}/data/colortables/NCL_colortables/*")
list(FILTER NCLCOLORTABLE_FILES EXCLUDE REGEX "Makefile*")
add_custom_command(OUTPUT nclcolortables
        COMMAND ${CMAKE_COMMAND} -E copy
        ${NCLCOLORTABLE_FILES}
        ${PROJECT_BINARY_DIR}/share/met/colortables/NCL_colortables/
        VERBATIM
)

# data/config
# TODO: if moving config file copy to another file, ensure MET_BASE is set properly if referencing it!

add_custom_command(OUTPUT config
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/data/config
        ${MET_BASE}/config/
        VERBATIM
)

# data/map - note: data/map/figure is not copied via make!

file(GLOB MAP_FILES "${PROJECT_SOURCE_DIR}/data/map/*_data")
add_custom_command(OUTPUT map
        COMMAND ${CMAKE_COMMAND} -E copy
        ${MAP_FILES}
        ${PROJECT_BINARY_DIR}/share/met/map/
        VERBATIM
)

file(GLOB MAP_ADMIN_FILES "${PROJECT_SOURCE_DIR}/data/map/admin_by_country/admin*")
add_custom_command(OUTPUT map_admin
        COMMAND ${CMAKE_COMMAND} -E copy
        ${MAP_ADMIN_FILES}
        ${PROJECT_BINARY_DIR}/share/met/map/admin_by_country/
        VERBATIM
)

# data/poly

add_custom_command(OUTPUT poly
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/data/poly
        ${PROJECT_BINARY_DIR}/share/met/poly/
        VERBATIM
)

# data/ps

add_custom_command(OUTPUT ps
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/data/ps
        ${PROJECT_BINARY_DIR}/share/met/ps/
        VERBATIM
)

# data/table_files

add_custom_command(OUTPUT table_files
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/data/table_files
        ${PROJECT_BINARY_DIR}/share/met/table_files/
        VERBATIM
)

# data/tc_data
add_custom_command(OUTPUT tc_data
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/data/tc_data
        ${PROJECT_BINARY_DIR}/share/met/tc_data/
        VERBATIM
)

# scripts/Rscripts
add_custom_command(OUTPUT rscript_data
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/scripts/RScripts
        ${PROJECT_BINARY_DIR}/share/met/Rscripts/
        VERBATIM
)

# scripts/python
add_custom_command(OUTPUT python_data
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/scripts/python
        ${PROJECT_BINARY_DIR}/share/met/python/
        VERBATIM
)

# add all data commands to data files target

add_custom_target(copy_data_files DEPENDS version.txt climo colortables nclcolortables config map map_admin poly ps table_files tc_data python_data)

# TODO: run ylwrap for lex/yacc invocations

#add_custom_target(OUTPUT color_parser_yacc
#  COMMAND ${PROJECT_SOURCE_DIR}/ylwrap
#)


###
# add libraries
###
# TODO: move these into CMakeLists.txt files in each directory and use add_subdirectory to traverse into them

add_library(vx_log
        src/basic/vx_log/concat_string.cc
        src/basic/vx_log/concat_string.h
        src/basic/vx_log/file_fxns.cc
        src/basic/vx_log/file_fxns.h
        src/basic/vx_log/indent.cc
        src/basic/vx_log/indent.h
        src/basic/vx_log/logger.cc
        src/basic/vx_log/logger.h
        src/basic/vx_log/str_wrappers.cc
        src/basic/vx_log/str_wrappers.h
        src/basic/vx_log/string_array.cc
        src/basic/vx_log/string_array.h
        src/basic/vx_log/vx_log.h
)
# TODO: this makes sure the data files are copied before building library - figure out where this should go
add_dependencies(vx_log copy_data_files)

add_library(vx_cal
        src/basic/vx_cal/date_to_mjd.cc
        src/basic/vx_cal/day_dif.cc
        src/basic/vx_cal/day_of_week.cc
        src/basic/vx_cal/doyhms_to_unix.cc
        src/basic/vx_cal/is_dst.cc
        src/basic/vx_cal/is_leap_year.cc
        src/basic/vx_cal/mdyhms_to_unix.cc
        src/basic/vx_cal/mjd_to_date.cc
        src/basic/vx_cal/time_array.cc
        src/basic/vx_cal/time_array.h
        src/basic/vx_cal/time_strings.cc
        src/basic/vx_cal/unix_string.cc
        src/basic/vx_cal/unix_to_mdyhms.cc
        src/basic/vx_cal/vx_cal.h
)

add_library(enum_to_string
        src/basic/enum_to_string/code.cc
        src/basic/enum_to_string/code.h
        src/basic/enum_to_string/enum.tab.h
        src/basic/enum_to_string/enum_to_string.cc
        src/basic/enum_to_string/enum_to_string.h
        src/basic/enum_to_string/info.cc
        src/basic/enum_to_string/info.h
        src/basic/enum_to_string/my_enum_scanner.cc
        src/basic/enum_to_string/scope.cc
        src/basic/enum_to_string/scope.h
)

add_library(vx_math
        src/basic/vx_math/affine.cc
        src/basic/vx_math/affine.h
        src/basic/vx_math/angles.cc
        src/basic/vx_math/angles.h
        src/basic/vx_math/hist.cc
        src/basic/vx_math/hist.h
        src/basic/vx_math/is_bad_data.h
        src/basic/vx_math/legendre.cc
        src/basic/vx_math/legendre.h
        src/basic/vx_math/math_constants.h
        src/basic/vx_math/nint.cc
        src/basic/vx_math/nint.h
        src/basic/vx_math/nti.cc
        src/basic/vx_math/nti.h
        src/basic/vx_math/ptile.cc
        src/basic/vx_math/ptile.h
        src/basic/vx_math/pwl.cc
        src/basic/vx_math/pwl.h
        src/basic/vx_math/trig.h
        src/basic/vx_math/viewgravity_to_string.cc
        src/basic/vx_math/viewgravity_to_string.h
        src/basic/vx_math/vx_math.h
        src/basic/vx_math/vx_vector.cc
        src/basic/vx_math/vx_vector.h
)

add_library(vx_util
        src/basic/vx_util/ascii_header.cc
        src/basic/vx_util/ascii_header.h
        src/basic/vx_util/ascii_table.cc
        src/basic/vx_util/ascii_table.h
        src/basic/vx_util/asciitablejust_to_string.cc
        src/basic/vx_util/asciitablejust_to_string.h
        src/basic/vx_util/bool_array.h
        src/basic/vx_util/bool_to_string.h
        src/basic/vx_util/check_endian.cc
        src/basic/vx_util/check_endian.h
        src/basic/vx_util/CircularTemplate.cc
        src/basic/vx_util/CircularTemplate.h
        src/basic/vx_util/comma_string.cc
        src/basic/vx_util/comma_string.h
        src/basic/vx_util/command_line.cc
        src/basic/vx_util/command_line.h
        src/basic/vx_util/conversions.cc
        src/basic/vx_util/conversions.h
        src/basic/vx_util/crc_array.h
        src/basic/vx_util/data_cube.cc
        src/basic/vx_util/data_cube.h
        src/basic/vx_util/data_line.cc
        src/basic/vx_util/data_line.h
        src/basic/vx_util/data_plane.cc
        src/basic/vx_util/data_plane.h
        src/basic/vx_util/data_plane_util.cc
        src/basic/vx_util/data_plane_util.h
        src/basic/vx_util/empty_string.h
        src/basic/vx_util/file_exists.cc
        src/basic/vx_util/file_exists.h
        src/basic/vx_util/file_linecount.cc
        src/basic/vx_util/file_linecount.h
        src/basic/vx_util/file_size.cc
        src/basic/vx_util/file_size.h
        src/basic/vx_util/filename_suffix.cc
        src/basic/vx_util/filename_suffix.h
        src/basic/vx_util/fix_float.cc
        src/basic/vx_util/fix_float.h
        src/basic/vx_util/get_filenames.cc
        src/basic/vx_util/get_filenames.h
        src/basic/vx_util/grib_constants.cc
        src/basic/vx_util/grib_constants.h
        src/basic/vx_util/GridOffset.cc
        src/basic/vx_util/GridOffset.h
        src/basic/vx_util/GridPoint.cc
        src/basic/vx_util/GridPoint.h
        src/basic/vx_util/GridTemplate.cc
        src/basic/vx_util/GridTemplate.h
        src/basic/vx_util/handle_openmp.cc
        src/basic/vx_util/handle_openmp.h
        src/basic/vx_util/int_array.h
        src/basic/vx_util/interp_mthd.cc
        src/basic/vx_util/interp_mthd.h
        src/basic/vx_util/interp_util.cc
        src/basic/vx_util/interp_util.h
        src/basic/vx_util/is_number.cc
        src/basic/vx_util/is_number.h
        src/basic/vx_util/long_array.cc
        src/basic/vx_util/long_array.h
        src/basic/vx_util/main.cc
        src/basic/vx_util/main.h
        src/basic/vx_util/make_path.cc
        src/basic/vx_util/make_path.h
        src/basic/vx_util/mask_poly.cc
        src/basic/vx_util/mask_poly.h
        src/basic/vx_util/mask_sid.cc
        src/basic/vx_util/mask_poly.h
        src/basic/vx_util/memory.cc
        src/basic/vx_util/memory.h
        src/basic/vx_util/met_buffer.cc
        src/basic/vx_util/met_buffer.h
        src/basic/vx_util/ncrr_array.h
        src/basic/vx_util/normalize.cc
        src/basic/vx_util/normalize.h
        src/basic/vx_util/num_array.cc
        src/basic/vx_util/num_array.h
        src/basic/vx_util/observation.cc
        src/basic/vx_util/observation.h
        src/basic/vx_util/ordinal.cc
        src/basic/vx_util/ordinal.h
        src/basic/vx_util/polyline.cc
        src/basic/vx_util/polyline.h
        src/basic/vx_util/python_line.cc
        src/basic/vx_util/python_line.h
        src/basic/vx_util/read_fortran_binary.cc
        src/basic/vx_util/read_fortran_binary.h
        src/basic/vx_util/RectangularTemplate.cc
        src/basic/vx_util/RectangularTemplate.h
        src/basic/vx_util/roman_numeral.cc
        src/basic/vx_util/roman_numeral.h
        src/basic/vx_util/smart_buffer.cc
        src/basic/vx_util/smart_buffer.h
        src/basic/vx_util/stat_column_defs.h
        src/basic/vx_util/string_fxns.cc
        src/basic/vx_util/string_fxns.h
        src/basic/vx_util/substring.cc
        src/basic/vx_util/substring.h
        src/basic/vx_util/thresh_array.cc
        src/basic/vx_util/thresh_array.h
        src/basic/vx_util/two_d_array.h
        src/basic/vx_util/two_to_one.cc
        src/basic/vx_util/two_to_one.h
        src/basic/vx_util/util_constants.h
        src/basic/vx_util/vx_util.h
)

add_library(vx_util_math
        src/basic/vx_util_math/so3.cc
        src/basic/vx_util_math/so3.h
)

add_library(vx_gsl_prob
        src/libcode/vx_gsl_prob/gsl_bvn.cc
        src/libcode/vx_gsl_prob/gsl_bvn.h
        src/libcode/vx_gsl_prob/gsl_cdf.cc
        src/libcode/vx_gsl_prob/gsl_cdf.h
        src/libcode/vx_gsl_prob/gsl_randist.cc
        src/libcode/vx_gsl_prob/gsl_randist.h
        src/libcode/vx_gsl_prob/gsl_statistics.cc
        src/libcode/vx_gsl_prob/gsl_statistics.h
        src/libcode/vx_gsl_prob/gsl_wavelet2d.cc
        src/libcode/vx_gsl_prob/gsl_wavelet2d.h
        src/libcode/vx_gsl_prob/vx_gsl_prob.h
)

add_library(vx_ioda
    src/libcode/vx_ioda/ioda.cc
    src/libcode/vx_ioda/ioda.h
    src/libcode/vx_ioda/ioda_data_conf_info.cc
    src/libcode/vx_ioda/ioda_data_conf_info.h
    src/libcode/vx_ioda/vx_ioda.h
)

add_library(vx_config
        src/basic/vx_config/builtin.cc
        src/basic/vx_config/builtin.h
        src/basic/vx_config/calculator.cc
        src/basic/vx_config/calculator.h
        src/basic/vx_config/celltype_to_string.cc
        src/basic/vx_config/celltype_to_string.h
        src/basic/vx_config/config.tab.cc
        src/basic/vx_config/config.tab.h
        src/basic/vx_config/config_constants.h
        src/basic/vx_config/config_file.cc
        src/basic/vx_config/config_file.h
        src/basic/vx_config/config_funcs.cc
        src/basic/vx_config/config_funcs.h
        src/basic/vx_config/config_gaussian.h
        src/basic/vx_config/config_util.cc
        src/basic/vx_config/config_util.h
        src/basic/vx_config/configobjecttype_to_string.cc
        src/basic/vx_config/configobjecttype_to_string.h
        src/basic/vx_config/data_file_type.h
        src/basic/vx_config/dictionary.cc
        src/basic/vx_config/dictionary.h
        src/basic/vx_config/grdfiletype_to_string.cc
        src/basic/vx_config/grdfiletype_to_string.h
        src/basic/vx_config/icode.cc
        src/basic/vx_config/icode.h
        src/basic/vx_config/idstack.cc
        src/basic/vx_config/idstack.h
        src/basic/vx_config/my_config_scanner.cc
        src/basic/vx_config/number_stack.cc
        src/basic/vx_config/number_stack.h
        src/basic/vx_config/object_types.h
        src/basic/vx_config/scanner_stuff.h
        src/basic/vx_config/temp_file.cc
        src/basic/vx_config/temp_file.h
        src/basic/vx_config/threshold.cc
        src/basic/vx_config/threshold.h
        src/basic/vx_config/vx_config.h
)

add_library(vx_geodesy
        src/libcode/vx_geodesy/spheroid.cc
        src/libcode/vx_geodesy/spheroid.h
        src/libcode/vx_geodesy/vx_geodesy.h
)

add_library(vx_grid
        src/libcode/vx_grid/earth_rotation.cc
        src/libcode/vx_grid/earth_rotation.h
        src/libcode/vx_grid/find_grid_by_name.cc
        src/libcode/vx_grid/find_grid_by_name.h
        src/libcode/vx_grid/grid_base.cc
        src/libcode/vx_grid/grid_base.h
        src/libcode/vx_grid/gaussian_grid.cc
        src/libcode/vx_grid/gaussian_grid.h
        src/libcode/vx_grid/gaussian_grid_defs.h
        src/libcode/vx_grid/goes_grid.cc
        src/libcode/vx_grid/goes_grid.h
        src/libcode/vx_grid/goes_grid_defs.h
        src/libcode/vx_grid/laea_grid.cc
        src/libcode/vx_grid/laea_grid.h
        src/libcode/vx_grid/laea_grid_defs.h
        src/libcode/vx_grid/latlon_grid.cc
        src/libcode/vx_grid/latlon_grid.h
        src/libcode/vx_grid/latlon_grid_defs.h
        src/libcode/vx_grid/latlon_xyz.cc
        src/libcode/vx_grid/latlon_xyz.h
        src/libcode/vx_grid/lc_grid.cc
        src/libcode/vx_grid/lc_grid.h
        src/libcode/vx_grid/lc_grid_defs.h
        src/libcode/vx_grid/merc_grid.cc
        src/libcode/vx_grid/merc_grid.h
        src/libcode/vx_grid/merc_grid_defs.h
        src/libcode/vx_grid/rng_azi_grid.cc
        src/libcode/vx_grid/rng_azi_grid.h
        src/libcode/vx_grid/rot_latlon_grid.cc
        src/libcode/vx_grid/rot_latlon_grid.h
        src/libcode/vx_grid/semilatlon_grid.cc
        src/libcode/vx_grid/semilatlon_grid.h
        src/libcode/vx_grid/semilatlon_grid_defs.h
        src/libcode/vx_grid/st_grid.cc
        src/libcode/vx_grid/st_grid.h
        src/libcode/vx_grid/st_grid_defs.h
        src/libcode/vx_grid/unstructured_grid_defs.h
        src/libcode/vx_grid/unstructured_grid.h
        src/libcode/vx_grid/unstructured_grid.cc
        src/libcode/vx_grid/vx_grid.h
)

add_library(vx_regrid
        src/libcode/vx_regrid/vx_regrid.cc
        src/libcode/vx_regrid/vx_regrid.h
        src/libcode/vx_regrid/vx_regrid_budget.cc
)

add_library(vx_gnomon
        src/libcode/vx_gnomon/gnomon.cc
        src/libcode/vx_gnomon/gnomon.h
)

add_library(vx_nav
        src/libcode/vx_nav/nav.cc
        src/libcode/vx_nav/nav.h
)

add_library(vx_afm
        src/libcode/vx_afm/afm.cc
        src/libcode/vx_afm/afm.h
        src/libcode/vx_afm/afm_keywords.cc
        src/libcode/vx_afm/afm_keywords.h
        src/libcode/vx_afm/afm_line.cc
        src/libcode/vx_afm/afm_line.h
        src/libcode/vx_afm/afm_token.cc
        src/libcode/vx_afm/afm_token.h
        src/libcode/vx_afm/afm_token_types.h
        src/libcode/vx_afm/afmkeyword_to_string.cc
        src/libcode/vx_afm/afmkeyword_to_string.h
        src/libcode/vx_afm/afmtokentype_to_string.cc
        src/libcode/vx_afm/afmtokentype_to_string.h
)

add_library(vx_ps
        src/libcode/vx_ps/documentmedia_to_string.cc
        src/libcode/vx_ps/documentmedia_to_string.h
        src/libcode/vx_ps/documentorientation_to_string.cc
        src/libcode/vx_ps/documentorientation_to_string.h
        src/libcode/vx_ps/fontfamily_to_string.cc
        src/libcode/vx_ps/fontfamily_to_string.h
        src/libcode/vx_ps/ps_text.cc
        src/libcode/vx_ps/ps_text.h
        src/libcode/vx_ps/table_helper.cc
        src/libcode/vx_ps/table_helper.h
        src/libcode/vx_ps/vx_ps.cc
        src/libcode/vx_ps/vx_ps.h
)

# TODO: call ylwrap to generate color_parser_yacc.cc!
add_library(vx_color
        src/libcode/vx_color/color.cc
        src/libcode/vx_color/color.h
        src/libcode/vx_color/color_list.cc
        src/libcode/vx_color/color_list.h
        src/libcode/vx_color/color_parser.h
        src/libcode/vx_color/color_parser_yacc.cc
        src/libcode/vx_color/color_parser_yacc.h
        src/libcode/vx_color/color_table.cc
        src/libcode/vx_color/my_color_scanner.cc
        src/libcode/vx_color/vx_color.h
)

add_library(vx_pxm
        src/libcode/vx_pxm/pbm.cc
        src/libcode/vx_pxm/pbm.h
        src/libcode/vx_pxm/pcm.cc
        src/libcode/vx_pxm/pcm.h
        src/libcode/vx_pxm/pgm.cc
        src/libcode/vx_pxm/pgm.h
        src/libcode/vx_pxm/ppm.cc
        src/libcode/vx_pxm/ppm.h
        src/libcode/vx_pxm/pxm_base.cc
        src/libcode/vx_pxm/pxm_base.h
        src/libcode/vx_pxm/pxm_utils.cc
        src/libcode/vx_pxm/pxm_utils.h
        src/libcode/vx_pxm/vx_pxm.h
)

add_library(vx_render
        src/libcode/vx_render/ascii85_filter.cc
        src/libcode/vx_render/ascii85_filter.h
        src/libcode/vx_render/bit_filter.cc
        src/libcode/vx_render/bit_filter.h
        src/libcode/vx_render/flate_filter.cc
        src/libcode/vx_render/flate_filter.h
        src/libcode/vx_render/hex_filter.cc
        src/libcode/vx_render/hex_filter.h
        src/libcode/vx_render/ps_filter.cc
        src/libcode/vx_render/ps_filter.h
        src/libcode/vx_render/psout_filter.cc
        src/libcode/vx_render/psout_filter.h
        src/libcode/vx_render/render_pbm.cc
        src/libcode/vx_render/render_pcm.cc
        src/libcode/vx_render/render_pgm.cc
        src/libcode/vx_render/render_ppm.cc
        src/libcode/vx_render/renderinfo.cc
        src/libcode/vx_render/renderinfo.h
        src/libcode/vx_render/rle_filter.cc
        src/libcode/vx_render/rle_filter.h
        src/libcode/vx_render/uc_queue.cc
        src/libcode/vx_render/uc_queue.h
        src/libcode/vx_render/vx_render.h
)

add_library(vx_plot_util
        src/libcode/vx_plot_util/data_plane_plot.cc
        src/libcode/vx_plot_util/data_plane_plot.h
        src/libcode/vx_plot_util/map_region.cc
        src/libcode/vx_plot_util/map_region.h
        src/libcode/vx_plot_util/vx_plot_util.cc
        src/libcode/vx_plot_util/vx_plot_util.h
)

add_library(vx_data2d
        src/libcode/vx_data2d/data2d_utils.cc
        src/libcode/vx_data2d/data2d_utils.h
        src/libcode/vx_data2d/data_class.cc
        src/libcode/vx_data2d/data_class.h
        src/libcode/vx_data2d/level_info.cc
        src/libcode/vx_data2d/level_info.h
        src/libcode/vx_data2d/leveltype_to_string.cc
        src/libcode/vx_data2d/leveltype_to_string.h
        src/libcode/vx_data2d/mask_filters.cc
        src/libcode/vx_data2d/mask_filters.h
        src/libcode/vx_data2d/table_lookup.cc
        src/libcode/vx_data2d/table_lookup.h
        src/libcode/vx_data2d/var_info.cc
        src/libcode/vx_data2d/var_info.h
        src/libcode/vx_data2d/var_info_pairs.cc
        src/libcode/vx_data2d/var_info_pairs.h
        src/libcode/vx_data2d/vx_data2d.h
)

add_library(vx_nc_util
        src/libcode/vx_nc_util/grid_output.cc
        src/libcode/vx_nc_util/grid_output.h
        src/libcode/vx_nc_util/load_tc_data.cc
        src/libcode/vx_nc_util/load_tc_data.h
        src/libcode/vx_nc_util/nc_constants.h
        src/libcode/vx_nc_util/nc_utils.cc
        src/libcode/vx_nc_util/nc_utils.h
        src/libcode/vx_nc_util/nc_utils.hpp
        src/libcode/vx_nc_util/nc_var_info.cc
        src/libcode/vx_nc_util/nc_var_info.h
        src/libcode/vx_nc_util/vx_nc_util.h
        src/libcode/vx_nc_util/write_netcdf.cc
        src/libcode/vx_nc_util/write_netcdf.h
)

add_library(vx_statistics
        src/libcode/vx_statistics/apply_mask.cc
        src/libcode/vx_statistics/apply_mask.h
        src/libcode/vx_statistics/compute_ci.cc
        src/libcode/vx_statistics/compute_ci.h
        src/libcode/vx_statistics/compute_stats.cc
        src/libcode/vx_statistics/compute_stats.h
        src/libcode/vx_statistics/contable.cc
        src/libcode/vx_statistics/contable.h
        src/libcode/vx_statistics/contable_nx2.cc
        src/libcode/vx_statistics/contable_stats.cc
        src/libcode/vx_statistics/ens_stats.cc
        src/libcode/vx_statistics/ens_stats.h
        src/libcode/vx_statistics/grid_closed_poly.cc
        src/libcode/vx_statistics/grid_closed_poly.h
        src/libcode/vx_statistics/met_stats.cc
        src/libcode/vx_statistics/met_stats.h
        src/libcode/vx_statistics/obs_error.cc
        src/libcode/vx_statistics/obs_error.h
        src/libcode/vx_statistics/pair_base.cc
        src/libcode/vx_statistics/pair_base.h
        src/libcode/vx_statistics/pair_data_ensemble.cc
        src/libcode/vx_statistics/pair_data_ensemble.h
        src/libcode/vx_statistics/pair_data_point.cc
        src/libcode/vx_statistics/pair_data_point.h
        src/libcode/vx_statistics/read_climo.cc
        src/libcode/vx_statistics/read_climo.h
        src/libcode/vx_statistics/vx_statistics.h
)

add_library(vx_python3_utils
        src/libcode/vx_python3_utils/global_python.h
        src/libcode/vx_python3_utils/python3_dict.cc
        src/libcode/vx_python3_utils/python3_dict.h
        src/libcode/vx_python3_utils/python3_list.cc
        src/libcode/vx_python3_utils/python3_list.h
        src/libcode/vx_python3_utils/python3_numpy.cc
        src/libcode/vx_python3_utils/python3_numpy.h
        src/libcode/vx_python3_utils/python3_script.cc
        src/libcode/vx_python3_utils/python3_script.h
        src/libcode/vx_python3_utils/python3_util.cc
        src/libcode/vx_python3_utils/python3_util.h
        src/libcode/vx_python3_utils/vx_python3_utils.h
        src/libcode/vx_python3_utils/wchar_argv.cc
        src/libcode/vx_python3_utils/wchar_argv.h
)

add_library(vx_pointdata_python
        src/libcode/vx_pointdata_python/pointdata_from_array.cc
        src/libcode/vx_pointdata_python/pointdata_from_array.h
        src/libcode/vx_pointdata_python/pointdata_from_array.hpp
        src/libcode/vx_pointdata_python/pointdata_python.cc
        src/libcode/vx_pointdata_python/pointdata_python.h
        src/libcode/vx_pointdata_python/python_pointdata.cc
        src/libcode/vx_pointdata_python/python_pointdata.h
        src/libcode/vx_pointdata_python/python_pointdata.hpp
)

add_library(vx_data2d_python
        src/libcode/vx_data2d_python/data2d_python.cc
        src/libcode/vx_data2d_python/data2d_python.h
        src/libcode/vx_data2d_python/dataplane_from_numpy_array.cc
        src/libcode/vx_data2d_python/dataplane_from_numpy_array.h
        src/libcode/vx_data2d_python/dataplane_from_numpy_array.hpp
        src/libcode/vx_data2d_python/dataplane_from_xarray.cc
        src/libcode/vx_data2d_python/dataplane_from_xarray.h
        src/libcode/vx_data2d_python/grid_from_python_dict.cc
        src/libcode/vx_data2d_python/grid_from_python_dict.h
        src/libcode/vx_data2d_python/python_dataplane.cc
        src/libcode/vx_data2d_python/python_dataplane.h
        src/libcode/vx_data2d_python/var_info_python.cc
        src/libcode/vx_data2d_python/var_info_python.h
)

add_library(vx_data2d_ugrid
        src/libcode/vx_data2d_ugrid/ugrid_file.cc
        src/libcode/vx_data2d_ugrid/ugrid_file.h
        src/libcode/vx_data2d_ugrid/var_info_ugrid.cc
        src/libcode/vx_data2d_ugrid/var_info_ugrid.h
        src/libcode/vx_data2d_ugrid/data2d_ugrid.cc
        src/libcode/vx_data2d_ugrid/data2d_ugrid.h
        src/libcode/vx_data2d_ugrid/vx_data2d_ugrid.h
)

add_library(vx_data2d_nc_cf
        src/libcode/vx_data2d_nc_cf/data2d_nc_cf.cc
        src/libcode/vx_data2d_nc_cf/data2d_nc_cf.h
        src/libcode/vx_data2d_nc_cf/nc_cf_file.cc
        src/libcode/vx_data2d_nc_cf/nc_cf_file.h
        src/libcode/vx_data2d_nc_cf/var_info_nc_cf.cc
        src/libcode/vx_data2d_nc_cf/var_info_nc_cf.h
        src/libcode/vx_data2d_nc_cf/vx_data2d_nc_cf.h
)

add_library(vx_data2d_nc_wrf
        src/libcode/vx_data2d_nc_wrf/data2d_nc_wrf.cc
        src/libcode/vx_data2d_nc_wrf/data2d_nc_wrf.h
        src/libcode/vx_data2d_nc_wrf/get_wrf_grid.cc
        src/libcode/vx_data2d_nc_wrf/get_wrf_grid.h
        src/libcode/vx_data2d_nc_wrf/var_info_nc_wrf.cc
        src/libcode/vx_data2d_nc_wrf/var_info_nc_wrf.h
        src/libcode/vx_data2d_nc_wrf/vx_data2d_nc_wrf.h
        src/libcode/vx_data2d_nc_wrf/wrf_file.cc
        src/libcode/vx_data2d_nc_wrf/wrf_file.h
)

add_library(vx_data2d_grib2
        src/libcode/vx_data2d_grib2/data2d_grib2.cc
        src/libcode/vx_data2d_grib2/data2d_grib2.h
        src/libcode/vx_data2d_grib2/var_info_grib2.cc
        src/libcode/vx_data2d_grib2/var_info_grib2.h
)

add_library(vx_data2d_grib
        src/libcode/vx_data2d_grib/data2d_grib.cc
        src/libcode/vx_data2d_grib/data2d_grib.h
        src/libcode/vx_data2d_grib/data2d_grib_utils.cc
        src/libcode/vx_data2d_grib/data2d_grib_utils.h
        src/libcode/vx_data2d_grib/grib_classes.cc
        src/libcode/vx_data2d_grib/grib_classes.h
        src/libcode/vx_data2d_grib/grib_strings.cc
        src/libcode/vx_data2d_grib/grib_strings.h
        src/libcode/vx_data2d_grib/grib_utils.cc
        src/libcode/vx_data2d_grib/grib_utils.h
        src/libcode/vx_data2d_grib/var_info_grib.cc
        src/libcode/vx_data2d_grib/var_info_grib.h
        src/libcode/vx_data2d_grib/vx_data2d_grib.h
        src/libcode/vx_data2d_grib/vx_grib_classes.h
)

add_library(vx_data2d_nc_met
        src/libcode/vx_data2d_nc_met/data2d_nc_met.cc
        src/libcode/vx_data2d_nc_met/data2d_nc_met.h
        src/libcode/vx_data2d_nc_met/get_met_grid.cc
        src/libcode/vx_data2d_nc_met/get_met_grid.h
        src/libcode/vx_data2d_nc_met/met_file.cc
        src/libcode/vx_data2d_nc_met/met_file.h
        src/libcode/vx_data2d_nc_met/met_file.hpp
        src/libcode/vx_data2d_nc_met/var_info_nc_met.cc
        src/libcode/vx_data2d_nc_met/var_info_nc_met.h
        src/libcode/vx_data2d_nc_met/vx_data2d_nc_met.h
)

add_library(vx_data2d_factory
        src/libcode/vx_data2d_factory/data2d_factory.cc
        src/libcode/vx_data2d_factory/data2d_factory.h
        src/libcode/vx_data2d_factory/data2d_factory_utils.cc
        src/libcode/vx_data2d_factory/data2d_factory_utils.h
        src/libcode/vx_data2d_factory/is_bufr_file.cc
        src/libcode/vx_data2d_factory/is_bufr_file.h
        src/libcode/vx_data2d_factory/is_grib_file.cc
        src/libcode/vx_data2d_factory/is_grib_file.h
        src/libcode/vx_data2d_factory/is_netcdf_file.cc
        src/libcode/vx_data2d_factory/is_netcdf_file.h
        src/libcode/vx_data2d_factory/parse_file_list.cc
        src/libcode/vx_data2d_factory/parse_file_list.h
        src/libcode/vx_data2d_factory/var_info_factory.cc
        src/libcode/vx_data2d_factory/var_info_factory.h
        src/libcode/vx_data2d_factory/vx_data2d_factory.h
)

add_library(vx_stat_out
        src/libcode/vx_stat_out/stat_columns.cc
        src/libcode/vx_stat_out/stat_columns.h
        src/libcode/vx_stat_out/stat_hdr_columns.cc
        src/libcode/vx_stat_out/stat_hdr_columns.h
        src/libcode/vx_stat_out/stat_hdr_info.cc
        src/libcode/vx_stat_out/stat_hdr_info.h
        src/libcode/vx_stat_out/vx_stat_out.h
)

add_library(vx_shapedata
        src/libcode/vx_shapedata/engine.cc
        src/libcode/vx_shapedata/engine.h
        src/libcode/vx_shapedata/ihull.cc
        src/libcode/vx_shapedata/ihull.h
        src/libcode/vx_shapedata/interest.cc
        src/libcode/vx_shapedata/interest.h
        src/libcode/vx_shapedata/mode_columns.h
        src/libcode/vx_shapedata/mode_conf_info.cc
        src/libcode/vx_shapedata/mode_conf_info.h
        src/libcode/vx_shapedata/mode_data_field.h
        src/libcode/vx_shapedata/mode_data_type.h
        src/libcode/vx_shapedata/mode_field_info.cc
        src/libcode/vx_shapedata/mode_field_info.h
        src/libcode/vx_shapedata/moments.cc
        src/libcode/vx_shapedata/moments.h
        src/libcode/vx_shapedata/node.cc
        src/libcode/vx_shapedata/node.h
        src/libcode/vx_shapedata/set.cc
        src/libcode/vx_shapedata/set.h
        src/libcode/vx_shapedata/shape.h
        src/libcode/vx_shapedata/shapedata.cc
        src/libcode/vx_shapedata/shapedata.h
        src/libcode/vx_shapedata/vx_shapedata.h
)

add_library(vx_analysis_util
        src/libcode/vx_analysis_util/analysis_utils.cc
        src/libcode/vx_analysis_util/analysis_utils.h
        src/libcode/vx_analysis_util/by_case_info.cc
        src/libcode/vx_analysis_util/by_case_info.h
        src/libcode/vx_analysis_util/mode_atts.cc
        src/libcode/vx_analysis_util/mode_atts.h
        src/libcode/vx_analysis_util/mode_job.cc
        src/libcode/vx_analysis_util/mode_job.h
        src/libcode/vx_analysis_util/mode_line.cc
        src/libcode/vx_analysis_util/mode_line.h
        src/libcode/vx_analysis_util/stat_job.cc
        src/libcode/vx_analysis_util/stat_job.h
        src/libcode/vx_analysis_util/stat_line.cc
        src/libcode/vx_analysis_util/stat_line.h
        src/libcode/vx_analysis_util/time_series.cc
        src/libcode/vx_analysis_util/time_series.h
        src/libcode/vx_analysis_util/vx_analysis_util.h
)

add_library(vx_seeps
        src/libcode/vx_seeps/seeps.cc
        src/libcode/vx_seeps/seeps.h
)

add_library(vx_nc_obs
        src/libcode/vx_nc_obs/met_point_data.cc
        src/libcode/vx_nc_obs/met_point_data.h
        src/libcode/vx_nc_obs/nc_obs_util.cc
        src/libcode/vx_nc_obs/nc_obs_util.h
        src/libcode/vx_nc_obs/nc_point_obs.cc
        src/libcode/vx_nc_obs/nc_point_obs.h
        src/libcode/vx_nc_obs/nc_point_obs_in.cc
        src/libcode/vx_nc_obs/nc_point_obs_in.h
        src/libcode/vx_nc_obs/nc_point_obs_out.cc
        src/libcode/vx_nc_obs/nc_point_obs_out.h
        src/libcode/vx_nc_obs/nc_summary.cc
        src/libcode/vx_nc_obs/nc_summary.h
)

add_library(vx_summary
        src/libcode/vx_summary/summary_calc.cc
        src/libcode/vx_summary/summary_calc.h
        src/libcode/vx_summary/summary_calc_max.cc
        src/libcode/vx_summary/summary_calc_max.h
        src/libcode/vx_summary/summary_calc_mean.cc
        src/libcode/vx_summary/summary_calc_mean.h
        src/libcode/vx_summary/summary_calc_median.cc
        src/libcode/vx_summary/summary_calc_median.h
        src/libcode/vx_summary/summary_calc_min.cc
        src/libcode/vx_summary/summary_calc_min.h
        src/libcode/vx_summary/summary_calc_percentile.cc
        src/libcode/vx_summary/summary_calc_percentile.h
        src/libcode/vx_summary/summary_calc_range.cc
        src/libcode/vx_summary/summary_calc_range.h
        src/libcode/vx_summary/summary_calc_stdev.cc
        src/libcode/vx_summary/summary_calc_stdev.h
        src/libcode/vx_summary/summary_calc_sum.cc
        src/libcode/vx_summary/summary_calc_sum.h
        src/libcode/vx_summary/summary_key.cc
        src/libcode/vx_summary/summary_key.h
        src/libcode/vx_summary/summary_obs.cc
        src/libcode/vx_summary/summary_obs.h
        src/libcode/vx_summary/time_summary_interval.cc
        src/libcode/vx_summary/time_summary_interval.h
        src/libcode/vx_summary/vx_summary.h
)

add_library(vx_pb_util
    src/libcode/vx_pb_util/copy_bytes.cc
    src/libcode/vx_pb_util/copy_bytes.h
    src/libcode/vx_pb_util/do_blocking.cc
    src/libcode/vx_pb_util/do_blocking.h
    src/libcode/vx_pb_util/do_unblocking.cc
    src/libcode/vx_pb_util/do_unblocking.h
    src/libcode/vx_pb_util/pblock.cc
    src/libcode/vx_pb_util/pblock.h
    src/libcode/vx_pb_util/vx_pb_util.h
)

# TODO: call ylwrap to generate files!
add_library(vx_bool_calc
    src/libcode/vx_bool_calc/bool_calc.cc
    src/libcode/vx_bool_calc/bool_calc.h
    src/libcode/vx_bool_calc/make_program.cc
    src/libcode/vx_bool_calc/make_program.h
    src/libcode/vx_bool_calc/token.cc
    src/libcode/vx_bool_calc/token.h
    src/libcode/vx_bool_calc/token_stack.cc
    src/libcode/vx_bool_calc/token_stack.h
    src/libcode/vx_bool_calc/tokenizer.cc
    src/libcode/vx_bool_calc/tokenizer.h
    src/libcode/vx_bool_calc/tokentype_to_string.cc
    src/libcode/vx_bool_calc/tokentype_to_string.h
)

add_library(vx_gis
    src/libcode/vx_gis/dbf_file.cc
    src/libcode/vx_gis/dbf_file.h
    src/libcode/vx_gis/shapetype_to_string.cc
    src/libcode/vx_gis/shapetype_to_string.h
    src/libcode/vx_gis/shp_array.h
    src/libcode/vx_gis/shp_file.cc
    src/libcode/vx_gis/shp_file.h
    src/libcode/vx_gis/shp_point_record.cc
    src/libcode/vx_gis/shp_point_record.h
    src/libcode/vx_gis/shp_poly_record.cc
    src/libcode/vx_gis/shp_poly_record.h
    src/libcode/vx_gis/shp_types.h
    src/libcode/vx_gis/shx_file.cc
    src/libcode/vx_gis/shx_file.h
    src/libcode/vx_gis/vx_gis.h
)

add_library(vx_physics
    src/libcode/vx_physics/thermo.cc
    src/libcode/vx_physics/thermo.h
)

add_library(vx_series_data
    src/libcode/vx_series_data/series_data.cc
    src/libcode/vx_series_data/series_data.h
    src/libcode/vx_series_data/series_pdf.cc
    src/libcode/vx_series_data/series_pdf.h
)

add_library(vx_solar
    src/libcode/vx_solar/astro_constants.h
    src/libcode/vx_solar/siderial.cc
    src/libcode/vx_solar/siderial.h
    src/libcode/vx_solar/solar.cc
    src/libcode/vx_solar/solar.h
)
add_library(vx_tc_util
        src/libcode/vx_tc_util/atcf_line_base.cc
        src/libcode/vx_tc_util/atcf_line_base.h
        src/libcode/vx_tc_util/atcf_offsets.h
        src/libcode/vx_tc_util/atcf_prob_line.cc
        src/libcode/vx_tc_util/atcf_prob_line.h
        src/libcode/vx_tc_util/atcf_track_line.cc
        src/libcode/vx_tc_util/atcf_track_line.h
        src/libcode/vx_tc_util/diag_file.cc
        src/libcode/vx_tc_util/diag_file.h
        src/libcode/vx_tc_util/gen_shape_info.cc
        src/libcode/vx_tc_util/gen_shape_info.h
        src/libcode/vx_tc_util/genesis_info.cc
        src/libcode/vx_tc_util/genesis_info.h
        src/libcode/vx_tc_util/pair_data_genesis.cc
        src/libcode/vx_tc_util/pair_data_genesis.h
        src/libcode/vx_tc_util/prob_gen_info.cc
        src/libcode/vx_tc_util/prob_gen_info.h
        src/libcode/vx_tc_util/prob_info_array.cc
        src/libcode/vx_tc_util/prob_info_array.h
        src/libcode/vx_tc_util/prob_info_base.cc
        src/libcode/vx_tc_util/prob_info_base.h
#        src/libcode/vx_tc_util/prob_pair_info.cc
#        src/libcode/vx_tc_util/prob_pair_info.h
        src/libcode/vx_tc_util/prob_rirw_info.cc
        src/libcode/vx_tc_util/prob_rirw_info.h
        src/libcode/vx_tc_util/prob_rirw_pair_info.cc
        src/libcode/vx_tc_util/prob_rirw_pair_info.h
        src/libcode/vx_tc_util/tc_columns.cc
        src/libcode/vx_tc_util/tc_columns.h
        src/libcode/vx_tc_util/tc_hdr_columns.cc
        src/libcode/vx_tc_util/tc_hdr_columns.h
        src/libcode/vx_tc_util/tc_stat_line.cc
        src/libcode/vx_tc_util/tc_stat_line.h
        src/libcode/vx_tc_util/track_info.cc
        src/libcode/vx_tc_util/track_info.h
        src/libcode/vx_tc_util/track_pair_info.cc
        src/libcode/vx_tc_util/track_pair_info.h
        src/libcode/vx_tc_util/track_point.cc
        src/libcode/vx_tc_util/track_point.h
        src/libcode/vx_tc_util/vx_tc_nc_util.cc
        src/libcode/vx_tc_util/vx_tc_nc_util.h
        src/libcode/vx_tc_util/vx_tc_util.h
)

add_library(vx_time_series
    src/libcode/vx_time_series/compute_swinging_door.cc
    src/libcode/vx_time_series/compute_swinging_door.h
    src/libcode/vx_time_series/time_series_util.cc
    src/libcode/vx_time_series/time_series_util.h
    src/libcode/vx_time_series/vx_time_series.h
)

###
# add applications
###
# TODO: move these into CMakeLists.txt files in each directory and use add_subdirectory to traverse into them

add_executable(plot_data_plane
        src/tools/other/plot_data_plane/plot_data_plane.cc
)
target_link_libraries(plot_data_plane
        vx_data2d_factory
        vx_data2d_nc_met
        vx_data2d_grib
        ${GRIB2_MET_LIBS}
        vx_data2d_nc_wrf
        vx_data2d_nc_cf
        ${UGRID_MET_LIBS}
        ${PYTHON_MET_LIBS}
        vx_statistics
        vx_nc_util
        vx_data2d
        vx_plot_util
        vx_render
        vx_pxm
        vx_color
        vx_ps
        vx_afm
        vx_nav
        vx_gnomon
        vx_regrid
        vx_grid
        vx_geodesy
        vx_config
        vx_gsl_prob
        vx_util_math
        vx_util
        vx_math
        vx_cal
        vx_log
        ${GRIB2_DEP_LIBS}
        ${UGRID_DEP_LIBS}
        ${PYTHON_DEP_LIBS}
        -lm -lz -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas
)
#target_include_directories(plot_data_plane PUBLIC ${PROJECT_SOURCE_DIR}/include)


add_executable(grid_stat
        src/tools/core/grid_stat/grid_stat.cc
        src/tools/core/grid_stat/grid_stat.h
        src/tools/core/grid_stat/grid_stat_conf_info.cc
        src/tools/core/grid_stat/grid_stat_conf_info.h
)
target_link_libraries(grid_stat
        vx_stat_out
        vx_statistics
        vx_shapedata
        vx_gsl_prob
        vx_analysis_util
        vx_data2d_factory
        vx_data2d_nc_met
        vx_data2d_grib
        ${GRIB2_MET_LIBS}
        vx_data2d_nc_wrf
        vx_data2d_nc_cf
        ${UGRID_MET_LIBS}
        ${PYTHON_MET_LIBS}
        vx_data2d
        vx_seeps
        vx_nc_util
        vx_regrid
        vx_grid
        vx_geodesy
        vx_config
        vx_cal
        vx_util_math
        vx_util
        vx_math
        vx_color
        vx_log
        ${GRIB2_DEP_LIBS}
        ${UGRID_DEP_LIBS}
        ${PYTHON_DEP_LIBS}
        -lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas
)


add_executable(point_stat
        src/tools/core/point_stat/point_stat.cc
        src/tools/core/point_stat/point_stat.h
        src/tools/core/point_stat/point_stat_conf_info.cc
        src/tools/core/point_stat/point_stat_conf_info.h
)
target_link_libraries(point_stat
        vx_stat_out
        vx_statistics
        vx_shapedata
        vx_analysis_util
        vx_data2d_factory
        vx_data2d_nc_met
        vx_data2d_grib
        ${GRIB2_MET_LIBS}
        vx_data2d_nc_wrf
        vx_data2d_nc_cf
        ${UGRID_MET_LIBS}
        ${PYTHON_MET_LIBS}
        vx_data2d
        vx_nc_obs
        vx_seeps
        vx_nc_util
        vx_regrid
        vx_grid
        vx_geodesy
        vx_config
        vx_gsl_prob
        vx_color
        vx_util_math
        vx_util
        vx_math
        vx_cal
        vx_log
        ${GRIB2_DEP_LIBS}
        ${UGRID_DEP_LIBS}
        ${PYTHON_DEP_LIBS}
        -lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas
)

add_executable(ensemble_stat
    src/tools/core/ensemble_stat/ensemble_stat.cc
    src/tools/core/ensemble_stat/ensemble_stat.h
    src/tools/core/ensemble_stat/ensemble_stat_conf_info.cc
    src/tools/core/ensemble_stat/ensemble_stat_conf_info.h
)
target_link_libraries(ensemble_stat
        vx_stat_out
        vx_statistics
        vx_shapedata
        vx_gsl_prob
        vx_analysis_util
        vx_data2d_factory
        vx_data2d_nc_met
        vx_data2d_grib
        ${GRIB2_MET_LIBS}
        vx_data2d_nc_wrf
        vx_data2d_nc_cf
        ${UGRID_MET_LIBS}
        ${PYTHON_MET_LIBS}
        vx_data2d
        vx_nc_obs
        vx_seeps
        vx_nc_util
        vx_regrid
        vx_grid
        vx_geodesy
        vx_config
        vx_color
        vx_util_math
        vx_util
        vx_math
        vx_cal
        vx_log
        ${GRIB2_DEP_LIBS}
        ${UGRID_DEP_LIBS}
        ${PYTHON_DEP_LIBS}
        -lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas
)


add_executable(wavelet_stat
        src/tools/core/wavelet_stat/wavelet_stat.cc
        src/tools/core/wavelet_stat/wavelet_stat.h
        src/tools/core/wavelet_stat/wavelet_stat_conf_info.cc
        src/tools/core/wavelet_stat/wavelet_stat_conf_info.h
)
target_link_libraries(wavelet_stat
vx_pxm
vx_plot_util
vx_nav
vx_render
vx_ps
vx_afm
        vx_stat_out
        vx_statistics
        vx_shapedata
        vx_gsl_prob
        vx_analysis_util
        vx_data2d_factory
        vx_data2d_nc_met
        vx_data2d_grib
        ${GRIB2_MET_LIBS}
        vx_data2d_nc_wrf
        vx_data2d_nc_cf
        ${UGRID_MET_LIBS}
        ${PYTHON_MET_LIBS}
        vx_data2d
        vx_seeps
        vx_nc_util
        vx_regrid
        vx_grid
        vx_geodesy
        vx_config
        vx_cal
        vx_util_math
        vx_util
        vx_math
        vx_color
        vx_log
        ${GRIB2_DEP_LIBS}
        ${UGRID_DEP_LIBS}
        ${PYTHON_DEP_LIBS}
        -lm -lproj -lz -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas
)

add_executable(series_analysis
        src/tools/core/series_analysis/series_analysis.cc
        src/tools/core/series_analysis/series_analysis.h
        src/tools/core/series_analysis/series_analysis_conf_info.cc
        src/tools/core/series_analysis/series_analysis_conf_info.h
)
target_link_libraries(series_analysis
        vx_stat_out
        vx_statistics
        vx_shapedata
        vx_analysis_util
        vx_data2d_factory
        vx_data2d_nc_met
        vx_data2d_grib
        ${GRIB2_MET_LIBS}
        vx_data2d_nc_wrf
        vx_data2d_nc_cf
        ${UGRID_MET_LIBS}
        ${PYTHON_MET_LIBS}
        vx_data2d
        vx_nc_obs
        vx_seeps
        vx_nc_util
        vx_regrid
        vx_grid
        vx_geodesy
        vx_config
        vx_gsl_prob
        vx_color
        vx_util_math
        vx_util
        vx_math
        vx_cal
        vx_log
        ${GRIB2_DEP_LIBS}
        ${UGRID_DEP_LIBS}
        ${PYTHON_DEP_LIBS}
        -lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas
)

add_executable(stat_analysis
    src/tools/core/stat_analysis/aggr_stat_line.cc
    src/tools/core/stat_analysis/aggr_stat_line.h
    src/tools/core/stat_analysis/parse_stat_line.cc
    src/tools/core/stat_analysis/parse_stat_line.h
    src/tools/core/stat_analysis/skill_score_index_job.cc
    src/tools/core/stat_analysis/skill_score_index_job.h
    src/tools/core/stat_analysis/stat_analysis.cc
    src/tools/core/stat_analysis/stat_analysis.h
    src/tools/core/stat_analysis/stat_analysis_job.cc
    src/tools/core/stat_analysis/stat_analysis_job.h
)

target_link_libraries(stat_analysis
        vx_stat_out
        vx_analysis_util
        vx_statistics
        vx_shapedata
        vx_gsl_prob
        vx_data2d_factory
        vx_data2d_nc_met
        vx_data2d_grib
        ${GRIB2_MET_LIBS}
        vx_data2d_nc_wrf
        vx_data2d_nc_cf
        ${UGRID_MET_LIBS}
        ${PYTHON_MET_LIBS}
        vx_data2d
        vx_seeps
        vx_nc_util
        vx_regrid
        vx_grid
        vx_geodesy
        vx_config
        vx_time_series
        vx_pb_util
        vx_cal
        vx_util_math
        vx_util
        vx_math
        vx_color
        vx_log
        vx_summary
        ${GRIB2_DEP_LIBS}
        ${UGRID_DEP_LIBS}
        ${PYTHON_DEP_LIBS}
        -lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas
)

add_executable(pb2nc
        src/tools/other/pb2nc/calcape.f
        src/tools/other/pb2nc/calpbl.f
        src/tools/other/pb2nc/numpbmsg.f
        src/tools/other/pb2nc/openpb.f
        src/tools/other/pb2nc/closepb.f
        src/tools/other/pb2nc/readpb.f
        src/tools/other/pb2nc/dumppb.f
        src/tools/other/pb2nc/pb2nc.cc
        src/tools/other/pb2nc/pb2nc_conf_info.cc
        src/tools/other/pb2nc/pb2nc_conf_info.h
)
target_link_libraries(pb2nc
	#$(FLIBS)
        vx_shapedata
        vx_analysis_util
        vx_data2d_factory
        vx_data2d_nc_met
        vx_data2d_grib
        ${GRIB2_MET_LIBS}
        vx_data2d_nc_wrf
        vx_data2d_nc_cf
        ${UGRID_MET_LIBS}
        ${PYTHON_MET_LIBS}
        vx_data2d
        vx_nc_obs
        vx_nc_util
        vx_regrid
        vx_grid
        vx_geodesy
        vx_pb_util
        vx_summary
        vx_statistics
        vx_config
        vx_gsl_prob
        vx_color
        vx_util_math
        vx_util
        vx_math
        vx_cal
        vx_log
        ${GRIB2_DEP_LIBS}
        ${UGRID_DEP_LIBS}
        ${PYTHON_DEP_LIBS}
        -lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas -lgfortran
        ${BUFRLIB_NAME}
)
target_compile_options(pb2nc PRIVATE "${CMAKE_C_FLAGS}")


add_executable(pair_stat
    src/tools/core/pair_stat/pair_stat.cc
    src/tools/core/pair_stat/pair_stat.h
    src/tools/core/pair_stat/pair_stat_conf_info.cc
    src/tools/core/pair_stat/pair_stat_conf_info.h
)
target_link_libraries(pair_stat
    vx_stat_out
    vx_statistics
    vx_shapedata
    vx_analysis_util
    vx_data2d_factory
    vx_data2d_nc_met
    vx_data2d_grib
    ${GRIB2_MET_LIBS}
    vx_data2d_nc_wrf
    vx_data2d_nc_cf
    ${UGRID_MET_LIBS}
    ${PYTHON_MET_LIBS}
    vx_ioda
    vx_data2d
    vx_nc_obs
    vx_seeps
    vx_nc_util
    vx_regrid
    vx_grid
    vx_geodesy
    vx_config
    vx_gsl_prob
    vx_color
    vx_util_math
    vx_util
    vx_math
    vx_cal
    vx_log
    ${GRIB2_DEP_LIBS}
    ${UGRID_DEP_LIBS}
    ${PYTHON_DEP_LIBS}
    -lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas
)

add_executable(ascii2nc
        src/tools/other/ascii2nc/aeronet_handler.cc
        src/tools/other/ascii2nc/aeronet_handler.h
        src/tools/other/ascii2nc/airnow_handler.cc
        src/tools/other/ascii2nc/airnow_handler.h
        src/tools/other/ascii2nc/airnow_locations.cc
        src/tools/other/ascii2nc/airnow_locations.h
        src/tools/other/ascii2nc/ascii2nc.cc
        src/tools/other/ascii2nc/ascii2nc_conf_info.cc
        src/tools/other/ascii2nc/ascii2nc_conf_info.h
        src/tools/other/ascii2nc/file_handler.cc
        src/tools/other/ascii2nc/file_handler.h
        src/tools/other/ascii2nc/iabp_handler.cc
        src/tools/other/ascii2nc/iabp_handler.h
        src/tools/other/ascii2nc/ismn_handler.cc
        src/tools/other/ascii2nc/ismn_handler.h
        src/tools/other/ascii2nc/little_r_handler.cc
        src/tools/other/ascii2nc/little_r_handler.h
        src/tools/other/ascii2nc/met_handler.cc
        src/tools/other/ascii2nc/met_handler.h
        src/tools/other/ascii2nc/ndbc_handler.cc
        src/tools/other/ascii2nc/ndbc_handler.h
        src/tools/other/ascii2nc/ndbc_locations.cc
        src/tools/other/ascii2nc/ndbc_locations.h
        src/tools/other/ascii2nc/python_handler.cc
        src/tools/other/ascii2nc/python_handler.h
        src/tools/other/ascii2nc/surfrad_handler.cc
        src/tools/other/ascii2nc/surfrad_handler.h
        src/tools/other/ascii2nc/uscrn_handler.cc
        src/tools/other/ascii2nc/uscrn_handler.h
        src/tools/other/ascii2nc/wwsis_handler.cc
        src/tools/other/ascii2nc/wwsis_handler.h
)
target_link_libraries(ascii2nc
    vx_stat_out
    vx_statistics
    vx_shapedata
    vx_analysis_util
    vx_data2d_factory
    vx_data2d_nc_met
    vx_data2d_grib
    ${GRIB2_MET_LIBS}
    vx_data2d_nc_wrf
    vx_data2d_nc_cf
    ${UGRID_MET_LIBS}
    ${PYTHON_MET_LIBS}
    vx_data2d
    vx_nc_obs
    vx_nc_util
    vx_regrid
    vx_grid
    vx_geodesy
    vx_summary
    vx_pb_util
    vx_config
    vx_gsl_prob
    vx_color
    vx_util_math
    vx_util
    vx_cal
    vx_math
    vx_log
    ${GRIB2_DEP_LIBS}
    ${UGRID_DEP_LIBS}
    ${PYTHON_DEP_LIBS}
    -lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas
)

add_executable(plot_point_obs
    src/tools/other/plot_point_obs/plot_point_obs.cc
    src/tools/other/plot_point_obs/plot_point_obs.h
    src/tools/other/plot_point_obs/plot_point_obs_conf_info.cc
    src/tools/other/plot_point_obs/plot_point_obs_conf_info.h
)
target_link_libraries(plot_point_obs
    vx_statistics
    vx_data2d_factory
    vx_data2d_nc_met
    vx_data2d_grib
    ${GRIB2_MET_LIBS}
    vx_data2d_nc_wrf
    vx_data2d_nc_cf
    ${UGRID_MET_LIBS}
    ${PYTHON_MET_LIBS}
    vx_data2d
    vx_nc_util
    vx_gnomon
    vx_regrid
    vx_plot_util
    vx_render
    vx_pxm
    vx_ps
    vx_afm
    vx_nav
    vx_grid
    vx_geodesy
    vx_config
    vx_gsl_prob
    vx_color
    vx_util_math
    vx_util
    vx_cal
    vx_math
    vx_log
    ${GRIB2_DEP_LIBS}
    ${UGRID_DEP_LIBS}
    ${PYTHON_DEP_LIBS}
    -lm -lproj -lz -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas
)

add_executable(pcp_combine
    src/tools/core/pcp_combine/pcp_combine.cc
)
target_link_libraries(pcp_combine
    vx_data2d_factory
    vx_data2d_nc_met
    vx_data2d_grib
    ${GRIB2_MET_LIBS}
    vx_data2d_nc_wrf
    vx_data2d_nc_cf
    ${UGRID_MET_LIBS}
    ${PYTHON_MET_LIBS}
    vx_statistics
    vx_data2d
    vx_nc_util
    vx_regrid
    vx_grid
    vx_geodesy
    vx_config
    vx_cal
    vx_util_math
    vx_util
    vx_gsl_prob
    vx_math
    vx_color
    vx_log
    ${GRIB2_DEP_LIBS}
    ${UGRID_DEP_LIBS}
    ${PYTHON_DEP_LIBS}
    -lm -lproj -lz -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas
)

add_executable(grid_diag
    src/tools/other/grid_diag/grid_diag.cc
    src/tools/other/grid_diag/grid_diag.h
    src/tools/other/grid_diag/grid_diag_conf_info.cc
    src/tools/other/grid_diag/grid_diag_conf_info.h
)
target_link_libraries(grid_diag
    vx_stat_out
    vx_statistics
    vx_shapedata
    vx_analysis_util
    vx_series_data
    vx_data2d_factory
    vx_data2d_nc_met
    vx_data2d_grib
    ${GRIB2_MET_LIBS}
    vx_data2d_nc_wrf
    vx_data2d_nc_cf
    ${UGRID_MET_LIBS}
    ${PYTHON_MET_LIBS}
    vx_data2d
    vx_nc_util
    vx_regrid
    vx_grid
    vx_geodesy
    vx_config
    vx_cal
    vx_util_math
    vx_util
    vx_gsl_prob
    vx_math
    vx_color
    vx_log
    ${GRIB2_DEP_LIBS}
    ${UGRID_DEP_LIBS}
    ${PYTHON_DEP_LIBS}
    -lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas
)
add_executable(gen_vx_mask
    src/tools/other/gen_vx_mask/gen_vx_mask.cc
    src/tools/other/gen_vx_mask/gen_vx_mask.h
)
target_link_libraries(gen_vx_mask
    vx_shapedata
    vx_analysis_util
    vx_data2d_factory
    vx_data2d_nc_met
    vx_data2d_grib
    ${GRIB2_MET_LIBS}
    vx_data2d_nc_wrf
    vx_data2d_nc_cf
    ${UGRID_MET_LIBS}
    ${PYTHON_MET_LIBS}
    vx_statistics
    vx_gis
    vx_nc_util
    vx_data2d
    vx_render
    vx_plot_util
    vx_pxm
    vx_color
    vx_ps
    vx_afm
    vx_pb_util
	vx_nav
	vx_gnomon
	vx_stat_out
	vx_gsl_prob
	vx_regrid
	vx_grid
	vx_geodesy
	vx_config
	vx_gsl_prob
	vx_util_math
	vx_util
	vx_math
	vx_solar
	vx_cal
	vx_log
    ${GRIB2_DEP_LIBS}
    ${UGRID_DEP_LIBS}
    ${PYTHON_DEP_LIBS}
    -lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas
)

add_executable(tc_stat
    src/tools/tc_utils/tc_stat/tc_stat.cc
    src/tools/tc_utils/tc_stat/tc_stat.h
    src/tools/tc_utils/tc_stat/tc_stat_conf_info.cc
    src/tools/tc_utils/tc_stat/tc_stat_conf_info.h
    src/tools/tc_utils/tc_stat/tc_stat_files.cc
    src/tools/tc_utils/tc_stat/tc_stat_files.h
    src/tools/tc_utils/tc_stat/tc_stat_job.cc
    src/tools/tc_utils/tc_stat/tc_stat_job.h
)
target_link_libraries(tc_stat
	vx_stat_out
    vx_statistics
    vx_shapedata
	vx_gsl_prob
    vx_analysis_util
    vx_tc_util
    vx_data2d_factory
    vx_data2d_nc_met
    vx_data2d_grib
    ${GRIB2_MET_LIBS}
    vx_data2d_nc_wrf
    vx_data2d_nc_cf
    ${UGRID_MET_LIBS}
    ${PYTHON_MET_LIBS}
    vx_data2d
    vx_seeps
    vx_nc_util
    vx_regrid
    vx_grid
    vx_geodesy
    vx_config
    vx_pb_util
    vx_cal
    vx_nav
    vx_util_math
    vx_util
    vx_math
    vx_color
    vx_log
    ${GRIB2_DEP_LIBS}
    ${UGRID_DEP_LIBS}
    ${PYTHON_DEP_LIBS}
    -lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas
)

add_executable(mode
    src/tools/core/mode/cluster_page.cc
    src/tools/core/mode/combine_boolplanes.cc
    src/tools/core/mode/combine_boolplanes.h
    src/tools/core/mode/fcst_enlarge_page.cc
    src/tools/core/mode/mode.cc
    src/tools/core/mode/mode_exec.cc
    src/tools/core/mode/mode_exec.h
    src/tools/core/mode/mode_frontend.cc
    src/tools/core/mode/mode_frontend.h
    src/tools/core/mode/mode_ps_file.cc
    src/tools/core/mode/mode_ps_file.h
    src/tools/core/mode/mode_ps_table_defs.h
    src/tools/core/mode/mode_superobject.cc
    src/tools/core/mode/mode_superobject.h
    src/tools/core/mode/mode_usage.cc
    src/tools/core/mode/mode_usage.h
    src/tools/core/mode/multivar_data.cc
    src/tools/core/mode/multivar_data.h
    src/tools/core/mode/multivar_frontend.cc
    src/tools/core/mode/objects_from_netcdf.cc
    src/tools/core/mode/objects_from_netcdf.h
    src/tools/core/mode/obs_enlarge_page.cc
    src/tools/core/mode/overlap_page.cc
    src/tools/core/mode/page_1.cc
    src/tools/core/mode/plot_engine.cc
    src/tools/core/mode/simple_objects.cc
)

target_link_libraries(mode
    vx_pxm
    vx_plot_util
    vx_nav
	vx_render
	vx_ps
	vx_afm
	vx_stat_out
	vx_statistics
	vx_shapedata
	vx_gsl_prob
	vx_analysis_util
	vx_data2d_factory
	vx_data2d_nc_met
	vx_data2d_grib
	${GRIB2_MET_LIBS}
	vx_data2d_nc_wrf
	vx_data2d_nc_cf
    ${UGRID_MET_LIBS}
    ${PYTHON_MET_LIBS}
	vx_statistics
	vx_data2d
	vx_seeps
	vx_nc_util
	vx_regrid
	vx_grid
	vx_geodesy
	vx_config
	vx_bool_calc
	vx_cal
	vx_util_math
	vx_util
	vx_math
	vx_color
	vx_log
    ${GRIB2_DEP_LIBS}
    ${UGRID_DEP_LIBS}
    ${PYTHON_DEP_LIBS}
	-lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas -lz
)

add_executable(tc_pairs
    src/tools/tc_utils/tc_pairs/acerr.f
    src/tools/tc_utils/tc_pairs/clip_shif.f
    src/tools/tc_utils/tc_pairs/tc_pairs.cc
    src/tools/tc_utils/tc_pairs/tc_pairs.h
    src/tools/tc_utils/tc_pairs/tc_pairs_conf_info.cc
    src/tools/tc_utils/tc_pairs/tc_pairs_conf_info.h
)

target_link_libraries(tc_pairs
	vx_stat_out
	vx_statistics
	vx_shapedata
	vx_gsl_prob
	vx_analysis_util
	vx_tc_util
	vx_data2d_factory
	vx_data2d_nc_met
	vx_data2d_grib
	${GRIB2_MET_LIBS}
	vx_data2d_nc_wrf
	vx_data2d_nc_cf
    ${UGRID_MET_LIBS}
    ${PYTHON_MET_LIBS}
	vx_data2d
	vx_nc_util
	vx_regrid
	vx_grid
	vx_geodesy
	vx_config
	vx_pb_util
	vx_cal
	vx_nav
	vx_util_math
	vx_util
	vx_math
	vx_color
	vx_log
    ${GRIB2_DEP_LIBS}
    ${UGRID_DEP_LIBS}
    ${PYTHON_DEP_LIBS}
	-lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas -lz
)

add_executable(regrid_data_plane
    src/tools/other/regrid_data_plane/regrid_data_plane.cc
)

target_link_libraries(regrid_data_plane
    vx_statistics
	vx_data2d_factory
	vx_data2d_nc_met
	vx_data2d_grib
	${GRIB2_MET_LIBS}
	vx_data2d_nc_wrf
	vx_data2d_nc_cf
	${UGRID_MET_LIBS}
	${PYTHON_MET_LIBS}
	vx_statistics
	vx_nc_util
	vx_data2d
	vx_gnomon
	vx_regrid
	vx_grid
	vx_geodesy
	vx_config
	vx_gsl_prob
	vx_cal
	vx_util_math
	vx_util
	vx_math
	vx_color
	vx_log
	${GRIB2_DEP_LIBS} ${UGRID_DEP_LIBS} ${PYTHON_DEP_LIBS}
	-lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas
)

add_executable(mode_analysis
    src/tools/core/mode_analysis/config_to_att.cc
    src/tools/core/mode_analysis/config_to_att.h
    src/tools/core/mode_analysis/mode_analysis.cc
)

target_link_libraries(mode_analysis
    vx_stat_out
    vx_statistics
    vx_shapedata
    vx_gsl_prob
    vx_analysis_util
    vx_util
    vx_data2d_factory
	vx_data2d_nc_met
	vx_data2d_grib
	${GRIB2_MET_LIBS}
	vx_data2d_nc_wrf
	vx_data2d_nc_cf
	${UGRID_MET_LIBS}
	${PYTHON_MET_LIBS}
	vx_data2d
	vx_nc_util
	vx_regrid
	vx_grid
	vx_geodesy
	vx_config
	vx_cal
	vx_util_math
	vx_math
	vx_color
	vx_log
	${GRIB2_DEP_LIBS} ${UGRID_DEP_LIBS} ${PYTHON_DEP_LIBS}
	-lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas
)

add_executable(tc_gen
    src/tools/tc_utils/tc_gen/tc_gen.cc
    src/tools/tc_utils/tc_gen/tc_gen.h
    src/tools/tc_utils/tc_gen/tc_gen_conf_info.cc
    src/tools/tc_utils/tc_gen/tc_gen_conf_info.h
)


target_link_libraries(tc_gen
	vx_stat_out
	vx_statistics
	vx_shapedata
	vx_gsl_prob
	vx_analysis_util
	vx_tc_util
	vx_data2d_factory
	vx_data2d_nc_met
	vx_data2d_grib
	${GRIB2_MET_LIBS}
	vx_data2d_nc_wrf
	vx_data2d_nc_cf
    ${UGRID_MET_LIBS}
    ${PYTHON_MET_LIBS}
    vx_gis
	vx_data2d
	vx_seeps
	vx_nc_util
	vx_regrid
	vx_grid
	vx_geodesy
	vx_config
	vx_pb_util
	vx_cal
	vx_nav
	vx_util_math
	vx_util
	vx_math
	vx_color
	vx_log
    ${GRIB2_DEP_LIBS}
    ${UGRID_DEP_LIBS}
    ${PYTHON_DEP_LIBS}
	-lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas -lz
	#$(FLIBS)
)

add_executable(gen_ens_prod
    src/tools/other/gen_ens_prod/gen_ens_prod.cc
    src/tools/other/gen_ens_prod/gen_ens_prod.h
    src/tools/other/gen_ens_prod/gen_ens_prod_conf_info.cc
    src/tools/other/gen_ens_prod/gen_ens_prod_conf_info.h
    src/tools/other/gen_ens_prod/compute_eas.cc
    src/tools/other/gen_ens_prod/compute_eas.h
)


target_link_libraries(gen_ens_prod
	vx_stat_out
	vx_statistics
	vx_shapedata
	vx_gsl_prob
	vx_analysis_util
	vx_data2d_factory
	vx_data2d_nc_met
	vx_data2d_grib
	${GRIB2_MET_LIBS}
	vx_data2d_nc_wrf
	vx_data2d_nc_cf
    ${UGRID_MET_LIBS}
    ${PYTHON_MET_LIBS}
	vx_data2d
	vx_nc_obs
	vx_seeps
	vx_nc_util
	vx_regrid
	vx_grid
	vx_geodesy
	vx_config
	vx_color
	vx_util_math
	vx_util
	vx_math
	vx_cal
	vx_log
    ${GRIB2_DEP_LIBS}
    ${UGRID_DEP_LIBS}
    ${PYTHON_DEP_LIBS}
	-lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas -lz
)


#        src/tools/dev_utils/shapefiles/make_mapfiles.cc
#        src/tools/dev_utils/chk4copyright.cc
#        src/tools/dev_utils/gen_climo_bin.cc
#        src/tools/dev_utils/gribtab.dat_to_flat.cc
#        src/tools/dev_utils/insitu_nc_file.cc
#        src/tools/dev_utils/insitu_nc_file.h
#        src/tools/dev_utils/insitu_nc_to_ascii.cc
#        src/tools/dev_utils/met_nc_file.cc
#        src/tools/dev_utils/met_nc_file.h
#        src/tools/dev_utils/nceptab_to_flat.cc
#        src/tools/dev_utils/pbtime.cc
#        src/tools/dev_utils/reformat_county_data.cc
#        src/tools/dev_utils/reformat_map_data.cc
#        src/tools/dev_utils/swinging_door.cc
#        src/tools/other/gis_utils/gis_dump_dbf.cc
#        src/tools/other/gis_utils/gis_dump_shp.cc
#        src/tools/other/gis_utils/gis_dump_shx.cc
#        src/tools/other/gsi_tools/conv_offsets.h
#        src/tools/other/gsi_tools/conv_record.cc
#        src/tools/other/gsi_tools/conv_record.h
#        src/tools/other/gsi_tools/ftto.h
#        src/tools/other/gsi_tools/gsi_record.cc
#        src/tools/other/gsi_tools/gsi_record.h
#        src/tools/other/gsi_tools/gsi_util.cc
#        src/tools/other/gsi_tools/gsi_util.h
#        src/tools/other/gsi_tools/gsid2mpr.cc
#        src/tools/other/gsi_tools/gsid2mpr.h
#        src/tools/other/gsi_tools/gsidens2orank.cc
#        src/tools/other/gsi_tools/gsidens2orank.h
#        src/tools/other/gsi_tools/rad_config.cc
#        src/tools/other/gsi_tools/rad_config.h
#        src/tools/other/gsi_tools/rad_offsets.h
#        src/tools/other/gsi_tools/rad_record.cc
#        src/tools/other/gsi_tools/rad_record.h
#        src/tools/other/ioda2nc/ioda2nc.cc
#        src/tools/other/ioda2nc/ioda2nc_conf_info.cc
#        src/tools/other/ioda2nc/ioda2nc_conf_info.h
##        src/tools/other/lidar2nc/calipso_5km.cc
##        src/tools/other/lidar2nc/calipso_5km.h
##        src/tools/other/lidar2nc/hdf_utils.cc
##        src/tools/other/lidar2nc/hdf_utils.h
##        src/tools/other/lidar2nc/lidar2nc.cc
##        src/tools/other/madis2nc/madis2nc.cc
##        src/tools/other/madis2nc/madis2nc.h
##        src/tools/other/madis2nc/madis2nc_conf_info.cc
##        src/tools/other/madis2nc/madis2nc_conf_info.h
#        src/tools/other/mode_time_domain/2d_att.cc
#        src/tools/other/mode_time_domain/2d_att.h
#        src/tools/other/mode_time_domain/2d_att_array.cc
#        src/tools/other/mode_time_domain/2d_att_array.h
#        src/tools/other/mode_time_domain/2d_columns.h
#        src/tools/other/mode_time_domain/2d_moments.cc
#        src/tools/other/mode_time_domain/2d_moments.h
#        src/tools/other/mode_time_domain/3d_att.cc
#        src/tools/other/mode_time_domain/3d_att.h
#        src/tools/other/mode_time_domain/3d_att_pair_array.cc
#        src/tools/other/mode_time_domain/3d_att_pair_array.h
#        src/tools/other/mode_time_domain/3d_att_single_array.cc
#        src/tools/other/mode_time_domain/3d_att_single_array.h
#        src/tools/other/mode_time_domain/3d_conv.cc
#        src/tools/other/mode_time_domain/3d_moments.cc
#        src/tools/other/mode_time_domain/3d_moments.h
#        src/tools/other/mode_time_domain/3d_pair_columns.h
#        src/tools/other/mode_time_domain/3d_single_columns.h
#        src/tools/other/mode_time_domain/3d_txt_header.h
#        src/tools/other/mode_time_domain/fo_graph.cc
#        src/tools/other/mode_time_domain/fo_graph.h
#        src/tools/other/mode_time_domain/fo_node.cc
#        src/tools/other/mode_time_domain/fo_node.h
#        src/tools/other/mode_time_domain/fo_node_array.cc
#        src/tools/other/mode_time_domain/fo_node_array.h
#        src/tools/other/mode_time_domain/interest_calc.cc
#        src/tools/other/mode_time_domain/interest_calc.h
#        src/tools/other/mode_time_domain/mm_engine.cc
#        src/tools/other/mode_time_domain/mm_engine.h
#        src/tools/other/mode_time_domain/mtd.cc
#        src/tools/other/mode_time_domain/mtd_config_info.cc
#        src/tools/other/mode_time_domain/mtd_config_info.h
#        src/tools/other/mode_time_domain/mtd_file.h
#        src/tools/other/mode_time_domain/mtd_file_base.cc
#        src/tools/other/mode_time_domain/mtd_file_base.h
#        src/tools/other/mode_time_domain/mtd_file_float.cc
#        src/tools/other/mode_time_domain/mtd_file_float.h
#        src/tools/other/mode_time_domain/mtd_file_int.cc
#        src/tools/other/mode_time_domain/mtd_file_int.h
#        src/tools/other/mode_time_domain/mtd_nc_defs.h
#        src/tools/other/mode_time_domain/mtd_nc_output.cc
#        src/tools/other/mode_time_domain/mtd_nc_output.h
#        src/tools/other/mode_time_domain/mtd_partition.cc
#        src/tools/other/mode_time_domain/mtd_partition.h
#        src/tools/other/mode_time_domain/mtd_read_data.cc
#        src/tools/other/mode_time_domain/mtd_read_data.h
#        src/tools/other/mode_time_domain/mtd_txt_output.cc
#        src/tools/other/mode_time_domain/mtd_txt_output.h
#        src/tools/other/mode_time_domain/mtdfiletype_to_string.cc
#        src/tools/other/mode_time_domain/mtdfiletype_to_string.h
#        src/tools/other/mode_time_domain/nc_grid.cc
#        src/tools/other/mode_time_domain/nc_grid.h
#        src/tools/other/mode_time_domain/nc_utils_local.cc
#        src/tools/other/mode_time_domain/nc_utils_local.h
##        src/tools/other/modis_regrid/cloudsat_swath_file.cc
##        src/tools/other/modis_regrid/cloudsat_swath_file.h
##        src/tools/other/modis_regrid/data_averager.cc
##        src/tools/other/modis_regrid/data_averager.h
##        src/tools/other/modis_regrid/data_plane_to_netcdf.cc
##        src/tools/other/modis_regrid/data_plane_to_netcdf.h
##        src/tools/other/modis_regrid/modis_file.cc
##        src/tools/other/modis_regrid/modis_file.h
##        src/tools/other/modis_regrid/modis_regrid.cc
##        src/tools/other/modis_regrid/sat_utils.cc
##        src/tools/other/modis_regrid/sat_utils.h
#        src/tools/other/point2grid/point2grid.cc
#        src/tools/other/point2grid/point2grid_conf_info.cc
#        src/tools/other/point2grid/point2grid_conf_info.h

#        src/tools/other/shift_data_plane/shift_data_plane.cc
#        src/tools/other/wwmca_tool/af_cp_file.cc
#        src/tools/other/wwmca_tool/af_cp_file.h
#        src/tools/other/wwmca_tool/af_file.cc
#        src/tools/other/wwmca_tool/af_file.h
#        src/tools/other/wwmca_tool/af_pt_file.cc
#        src/tools/other/wwmca_tool/af_pt_file.h
#        src/tools/other/wwmca_tool/ave_interp.cc
#        src/tools/other/wwmca_tool/ave_interp.h
#        src/tools/other/wwmca_tool/gridhemisphere_to_string.cc
#        src/tools/other/wwmca_tool/gridhemisphere_to_string.h
#        src/tools/other/wwmca_tool/interp_base.cc
#        src/tools/other/wwmca_tool/interp_base.h
#        src/tools/other/wwmca_tool/max_interp.cc
#        src/tools/other/wwmca_tool/max_interp.h
#        src/tools/other/wwmca_tool/min_interp.cc
#        src/tools/other/wwmca_tool/min_interp.h
#        src/tools/other/wwmca_tool/nc_output.cc
#        src/tools/other/wwmca_tool/nearest_interp.cc
#        src/tools/other/wwmca_tool/nearest_interp.h
#        src/tools/other/wwmca_tool/wwmca_plot.cc
#        src/tools/other/wwmca_tool/wwmca_ref.cc
#        src/tools/other/wwmca_tool/wwmca_ref.h
#        src/tools/other/wwmca_tool/wwmca_regrid.cc
##        src/tools/other/wwmca_tool/wwmca_utils.cc
##        src/tools/other/wwmca_tool/wwmca_utils.h
#        src/tools/tc_utils/rmw_analysis/rmw_analysis.cc
#        src/tools/tc_utils/rmw_analysis/rmw_analysis.h
#        src/tools/tc_utils/rmw_analysis/rmw_analysis_conf_info.cc
#        src/tools/tc_utils/rmw_analysis/rmw_analysis_conf_info.h
#        src/tools/tc_utils/rmw_analysis/rmw_analysis_utils.cc
#        src/tools/tc_utils/rmw_analysis/rmw_analysis_utils.h
#        src/tools/tc_utils/tc_diag/python_tc_diag.cc
#        src/tools/tc_utils/tc_diag/python_tc_diag.h
#        src/tools/tc_utils/tc_diag/tc_diag.cc
#        src/tools/tc_utils/tc_diag/tc_diag.h
#        src/tools/tc_utils/tc_diag/tc_diag_conf_info.cc
#        src/tools/tc_utils/tc_diag/tc_diag_conf_info.h
#        src/tools/tc_utils/tc_dland/tc_dland.cc
#        src/tools/tc_utils/tc_dland/tc_poly.cc
#        src/tools/tc_utils/tc_dland/tc_poly.h

#        src/tools/tc_utils/tc_rmw/tc_rmw.cc
#        src/tools/tc_utils/tc_rmw/tc_rmw.h
#        src/tools/tc_utils/tc_rmw/tc_rmw_conf_info.cc
#        src/tools/tc_utils/tc_rmw/tc_rmw_conf_info.h
#        src/tools/tc_utils/tc_rmw/tc_rmw_wind_converter.cc
#        src/tools/tc_utils/tc_rmw/tc_rmw_wind_converter.h
#        internal/test_util/basic/vx_config/test_config.cc
#        internal/test_util/basic/vx_config/test_lookup.cc
#        internal/test_util/basic/vx_config/test_lookup2.cc
#        internal/test_util/basic/vx_config/test_lookup3.cc
#        internal/test_util/basic/vx_config/test_met_478.cc
#        internal/test_util/basic/vx_config/test_string.cc
#        internal/test_util/basic/vx_config/test_string_then_config.cc
#        internal/test_util/basic/vx_config/test_thresh.cc
#        internal/test_util/basic/vx_config/test_user_func.cc
#        internal/test_util/basic/vx_log/test_logger.cc
#        internal/test_util/basic/vx_log/test_reg_exp.cc
#        internal/test_util/basic/vx_util/test_add_rows.cc
#        internal/test_util/basic/vx_util/test_ascii_header.cc
#        internal/test_util/basic/vx_util/test_command_line.cc
#        internal/test_util/basic/vx_util/test_data_plane.cc
#        internal/test_util/basic/vx_util/test_table_float.cc
#        internal/test_util/libcode/vx_data2d/dump_default_table.cc
#        internal/test_util/libcode/vx_data2d/test_table_read.cc
#        internal/test_util/libcode/vx_data2d_factory/test_factory.cc
#        internal/test_util/libcode/vx_data2d_factory/test_is_grib.cc
#        internal/test_util/libcode/vx_data2d_grib/test_read_grib1.cc
#        internal/test_util/libcode/vx_data2d_nc_cf/test_read_nccf.cc
#        internal/test_util/libcode/vx_data2d_nc_met/test_read_nc_met.cc
#        internal/test_util/libcode/vx_geodesy/test_spheroid.cc
#        internal/test_util/libcode/vx_grid/test_grid_area.cc
#        internal/test_util/libcode/vx_nc_util/test_pressure_levels.cc
#        internal/test_util/libcode/vx_physics/test_thermo.cc
#        internal/test_util/libcode/vx_plot_util/test_map_region.cc
#        internal/test_util/libcode/vx_ps/test_ps.cc
##        internal/test_util/libcode/vx_python3_utils/test_get.cc
#        internal/test_util/libcode/vx_python3_utils/test_numpy.cc
#        internal/test_util/libcode/vx_series_data/test_series_data.cc
#        internal/test_util/libcode/vx_solar/test_ra_dec.cc
#        internal/test_util/libcode/vx_tc_util/test_read.cc
#        internal/test_util/libcode/vx_tc_util/test_read_prob.cc
##        internal/test_util/libcode/vx_tc_util/test_read_rmw.cc
#        internal/test_util/tools/other/mode_time_domain/test_velocity.cc
#        config.h)
