## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The programs

bin_PROGRAMS    = reformat_conv \
                  reformat_rad


reformat_conv_SOURCES = reformat_conv.cc conv_offsets.h \
reformat_conv_CPPFLAGS = ${MET_CPPFLAGS} \
            -I../../../../include
reformat_conv_LDFLAGS = ${MET_LDFLAGS}
reformat_conv_LDADD = -lvx_stat_out \
	-lvx_color \
	-lvx_config \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_log \
	-lm


reformat_rad_SOURCES = reformat_rad.cc rad_offsets.h \
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



# If we are in development mode, generate the "to_string" files and
# clean them up in a "make clean".  We don't want to do either of these
# in a distribution because the enum_to_string utility isn't available.

if ENABLE_DEVELOPMENT

   clean-local:
endif
