## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# Test programs

noinst_PROGRAMS = test_table_read \
              dump_default_table

test_table_read_SOURCES = test_table_read.cc
test_table_read_CPPFLAGS = ${MET_CPPFLAGS}
test_table_read_LDFLAGS = -L. ${MET_LDFLAGS}
test_table_read_LDADD = -lvx_config \
	-lvx_gsl_prob \
	-lvx_log \
	-lvx_data2d \
	-lvx_grid  \
	-lvx_util \
	-lvx_cal \
	-lvx_math \
	-lvx_log \
	-lgsl -lgslcblas

dump_default_table_SOURCES = dump_default_table.cc
dump_default_table_CPPFLAGS = ${MET_CPPFLAGS}
dump_default_table_LDFLAGS = -L. ${MET_LDFLAGS}
dump_default_table_LDADD = -lvx_config \
	-lvx_gsl_prob \
	-lvx_log \
	-lvx_data2d \
	-lvx_grid \
	-lvx_util \
	-lvx_cal \
	-lvx_math \
	-lvx_log \
	-lgsl -lgslcblas
