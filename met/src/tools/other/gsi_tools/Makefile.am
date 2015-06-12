## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The programs

bin_PROGRAMS    = reformat_gsi_diag

reformat_gsi_diag_SOURCES = reformat_gsi_diag.h \
        gsi_record.h \
	gsi_record.cc \
	conv_record.h \
	conv_record.cc \
	rad_record.h \
	rad_record.cc \
	reformat_gsi_diag.cc \
	conv_offsets.h \
	rad_offsets.h \
	rad_config.h  \
	rad_config.cc \
	ftto.h
reformat_gsi_diag_CPPFLAGS = ${MET_CPPFLAGS}
reformat_gsi_diag_LDFLAGS = ${MET_LDFLAGS}
reformat_gsi_diag_LDADD = -lvx_stat_out \
	-lvx_statistics \
	-lvx_gsl_prob \
	-lvx_config \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	-lm -lgsl -lgslcblas
