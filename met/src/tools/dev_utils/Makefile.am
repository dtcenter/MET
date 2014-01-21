## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

bin_PROGRAMS    = chk4copyright \
	reformat_map_data \
	reformat_county_data \
	pbtime  \
	gribtab_dat_to_flat  \
	nceptab_to_flat  \
	insitu_nc_to_ascii

chk4copyright_SOURCES = chk4copyright.cc
chk4copyright_CPPFLAGS = -I${includedir}
chk4copyright_LDFLAGS = -L${libdir}
chk4copyright_LDADD = -lvx_util \
	-lvx_log

reformat_map_data_SOURCES = reformat_map_data.cc
reformat_map_data_CPPFLAGS = -I${includedir}
reformat_map_data_LDFLAGS = -L${libdir}
reformat_map_data_LDADD = -lvx_util \
	-lvx_log

reformat_county_data_SOURCES = reformat_county_data.cc
reformat_county_data_CPPFLAGS = -I${includedir}
reformat_county_data_LDFLAGS = -L${libdir}
reformat_county_data_LDADD = -lvx_util \
	-lvx_log

pbtime_SOURCES = pbtime.cc \
	../other/pb2nc/openpb.f \
	../other/pb2nc/readpb.f
pbtime_CPPFLAGS = -I${includedir}
pbtime_LDFLAGS = -L${libdir}
pbtime_LDADD = -lvx_pb_util \
	-lvx_cal \
	-lvx_util \
	-lvx_log \
	-lbufr \
	$(FLIBS)

gribtab_dat_to_flat_SOURCES = gribtab.dat_to_flat.cc \
	../other/pb2nc/openpb.f \
	../other/pb2nc/readpb.f
gribtab_dat_to_flat_CPPFLAGS = -I${includedir}
gribtab_dat_to_flat_LDFLAGS = -L${libdir}
gribtab_dat_to_flat_LDADD = -lvx_pb_util \
	-lvx_cal \
	-lvx_util \
	-lvx_log \
	-lbufr \
	$(FLIBS)

nceptab_to_flat_SOURCES = nceptab_to_flat.cc \
	../other/pb2nc/openpb.f \
	../other/pb2nc/readpb.f
nceptab_to_flat_CPPFLAGS = -I${includedir}
nceptab_to_flat_LDFLAGS = -L${libdir}
nceptab_to_flat_LDADD = -lvx_pb_util \
	-lvx_cal \
	-lvx_util \
	-lvx_log \
	-lbufr \
	$(FLIBS)

insitu_nc_to_ascii_SOURCES = insitu_nc_to_ascii.cc \
	insitu_nc_file.cc
insitu_nc_to_ascii_CPPFLAGS = -I${includedir}
insitu_nc_to_ascii_LDFLAGS = -L${libdir}
insitu_nc_to_ascii_LDADD = -lvx_util \
	-lvx_log \
	-lm -lnetcdf_c++ -lnetcdf
