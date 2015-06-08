## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The programs

bin_PROGRAMS    = reformat_conv \
                  reformat_rad  \
                  reformat_gsi


reformat_conv_SOURCES = gsi_record.h \
	gsi_record.cc \
	conv_record.h \
	conv_record.cc \
	reformat_conv.cc \
	conv_offsets.h \
	ftto.h
reformat_conv_CPPFLAGS = ${MET_CPPFLAGS}
reformat_conv_LDFLAGS = ${MET_LDFLAGS}
reformat_conv_LDADD = -lvx_stat_out \
	-lvx_color \
	-lvx_config \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_log \
	-lm


reformat_rad_SOURCES = gsi_record.h \
	gsi_record.cc \
	rad_record.h \
	rad_record.cc \
	reformat_rad.cc \
	rad_offsets.h \
	rad_config.h  \
	rad_config.cc \
	ftto.h
reformat_rad_CPPFLAGS = ${MET_CPPFLAGS}
reformat_rad_LDFLAGS = ${MET_LDFLAGS}
reformat_rad_LDADD = -lvx_stat_out \
	-lvx_color \
	-lvx_config \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_log \
	-lm
	
reformat_gsi_SOURCES = gsi_record.h \
	gsi_record.cc \
	conv_record.h \
	conv_record.cc \
	rad_record.h \
	rad_record.cc \
	reformat_gsi.cc \
	conv_offsets.h \
	rad_offsets.h \
	rad_config.h  \
	rad_config.cc \
	ftto.h
reformat_gsi_CPPFLAGS = ${MET_CPPFLAGS}
reformat_gsi_LDFLAGS = ${MET_LDFLAGS}
reformat_gsi_LDADD = -lvx_stat_out \
	-lvx_color \
	-lvx_config \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_log \
	-lm
