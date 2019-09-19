## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES = Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# Test programs

noinst_PROGRAMS = test_atmos_thermo

test_atmos_thermo_SOURCES = test_atmos_thermo
test_atmos_thermo_CPPFLAGS = ${MET_CPPFLAGS}
test_atmos_thermo_LDFLAGS = -L. ${MET_LDFLAGS}
test_atmos_thermo_LDADD = -lvx_atmos_thermo
