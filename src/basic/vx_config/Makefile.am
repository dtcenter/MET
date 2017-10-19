## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# Yacc/lex flags

AM_YFLAGS	= --defines=config.tab.h -p config
AM_LFLAGS	= --prefix=config --outfile=lex.yy.c

# The library

noinst_LIBRARIES = libvx_config.a

if ENABLE_DEVELOPMENT
    libvx_config_a_SOURCES = config.tab.yy config_scanner.ll
else
    libvx_config_a_SOURCES = config.tab.cc config.tab.h config_scanner.cc
endif

libvx_config_a_SOURCES += \
              configobjecttype_to_string.cc configobjecttype_to_string.h \
              grdfiletype_to_string.cc grdfiletype_to_string.h \
              dictionary.cc dictionary.h \
              config_file.cc config_file.h \
              config_util.cc config_util.h \
              temp_file.cc temp_file.h \
              threshold.cc threshold.h \
              data_file_type.h \
              object_types.h \
              scanner_stuff.h \
              config_constants.h \
              vx_config.h
libvx_config_a_CPPFLAGS = ${MET_CPPFLAGS}

# If we are in development mode, generate the "to_string" files and
# clean them up in a "make clean".  We don't want to do either of these
# in a distribution because the enum_to_string utility isn't available.

if ENABLE_DEVELOPMENT
    configobjecttype_to_string.cc configobjecttype_to_string.h: object_types.h
	${ENUM_TO_STRING} -concat_string object_types.h

    grdfiletype_to_string.cc grdfiletype_to_string.h: data_file_type.h
	${ENUM_TO_STRING} -concat_string -reverse data_file_type.h

    clean-local:
	-rm -r configobjecttype_to_string.cc
	-rm -r configobjecttype_to_string.h
	-rm -r grdfiletype_to_string.cc
	-rm -r grdfiletype_to_string.h
endif

# If we are in development mode, clean up extra local files in a "make clean".

if ENABLE_DEVELOPMENT
    CLEANFILES = config_scanner.cc \
        	config.tab.cc \
        	config.tab.h \
        	lex.config.cc
endif

# Extra distribution files

EXTRA_DIST = config.tab.cc \
	config.tab.h \
	config_scanner.cc

