## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# Check for BUFRLIB_NAME

if BUFRLIB_NAME_SET
BLIB_NAME = ${BUFRLIB_NAME}
else
BLIB_NAME = -lbufr
endif

SUBDIRS	= shapefiles

# The programs

bin_PROGRAMS = chk4copyright \
	reformat_map_data \
	reformat_county_data \
	pbtime  \
	gribtab_dat_to_flat  \
	nceptab_to_flat  \
	insitu_nc_to_ascii \
	swinging_door \
	gen_climo_bin

chk4copyright_SOURCES = chk4copyright.cc
chk4copyright_CPPFLAGS = ${MET_CPPFLAGS}
chk4copyright_LDFLAGS = ${MET_LDFLAGS}
chk4copyright_LDADD = -lvx_util \
	-lvx_log

reformat_map_data_SOURCES = reformat_map_data.cc
reformat_map_data_CPPFLAGS = ${MET_CPPFLAGS}
reformat_map_data_LDFLAGS = ${MET_LDFLAGS}
reformat_map_data_LDADD = -lvx_util \
	-lvx_log

reformat_county_data_SOURCES = reformat_county_data.cc
reformat_county_data_CPPFLAGS = ${MET_CPPFLAGS}
reformat_county_data_LDFLAGS = ${MET_LDFLAGS}
reformat_county_data_LDADD = -lvx_util \
	-lvx_log

pbtime_SOURCES = pbtime.cc \
	../other/pb2nc/openpb.f \
	../other/pb2nc/readpb.f
pbtime_CPPFLAGS = ${MET_CPPFLAGS}
pbtime_LDFLAGS = ${MET_LDFLAGS}
pbtime_LDADD = -lvx_pb_util \
	-lvx_config \
	-lvx_util \
	-lvx_cal \
	-lvx_log \
	$(BLIB_NAME) \
	$(FLIBS)

gribtab_dat_to_flat_SOURCES = gribtab.dat_to_flat.cc \
	../other/pb2nc/openpb.f \
	../other/pb2nc/readpb.f
gribtab_dat_to_flat_CPPFLAGS = ${MET_CPPFLAGS}
gribtab_dat_to_flat_LDFLAGS = ${MET_LDFLAGS}
gribtab_dat_to_flat_LDADD = -lvx_pb_util \
	-lvx_config \
	-lvx_util \
	-lvx_cal \
	-lvx_log \
	$(BLIB_NAME) \
	$(FLIBS)

nceptab_to_flat_SOURCES = nceptab_to_flat.cc \
	../other/pb2nc/openpb.f \
	../other/pb2nc/readpb.f
nceptab_to_flat_CPPFLAGS = ${MET_CPPFLAGS}
nceptab_to_flat_LDFLAGS = ${MET_LDFLAGS}
nceptab_to_flat_LDADD = -lvx_pb_util \
	-lvx_config \
	-lvx_util \
	-lvx_cal \
	-lvx_log \
	$(BLIB_NAME) \
	$(FLIBS)

insitu_nc_to_ascii_SOURCES = insitu_nc_to_ascii.cc \
	insitu_nc_file.cc insitu_nc_file.h
insitu_nc_to_ascii_CPPFLAGS = ${MET_CPPFLAGS}
insitu_nc_to_ascii_LDFLAGS = ${MET_LDFLAGS}
insitu_nc_to_ascii_LDADD = -lvx_nc_util \
	-lvx_config \
	-lvx_util \
	-lvx_cal \
	-lvx_math \
	-lvx_log \
	-lm -lnetcdf_c++4 -lnetcdf

swinging_door_SOURCES = swinging_door.cc \
	met_nc_file.cc met_nc_file.h
swinging_door_CPPFLAGS = ${MET_CPPFLAGS}
swinging_door_LDFLAGS = ${MET_LDFLAGS}
swinging_door_LDADD = -lvx_time_series \
	-lvx_nc_util \
	-lvx_config \
	-lvx_util \
	-lvx_cal \
	-lvx_math \
	-lvx_log \
	-lm -lnetcdf_c++4 -lnetcdf

gen_climo_bin_SOURCES = gen_climo_bin.cc
gen_climo_bin_CPPFLAGS = ${MET_CPPFLAGS}
gen_climo_bin_LDFLAGS = ${MET_LDFLAGS}
gen_climo_bin_LDADD = -lvx_stat_out \
	-lvx_statistics \
	-lvx_shapedata \
	-lvx_gsl_prob \
	-lvx_analysis_util \
	-lvx_data2d_factory \
	-lvx_data2d_nc_met \
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_data2d_nc_pinterp \
	$(PYTHON_LIBS) \
	-lvx_data2d_nccf \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_regrid \
	-lvx_grid \
	-lvx_config \
	-lvx_util \
	-lvx_cal \
	-lvx_gsl_prob \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	-lm -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas

# Build chk4copyright when making a distribution so that we can
#  add missing copyrights to the source files

dist-hook: chk4copyright

