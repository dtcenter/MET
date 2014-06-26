## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

lib_LIBRARIES = libvx_afm.a
libvx_afm_a_SOURCES = afm.cc \
              afm_keywords.cc \
              afm_line.cc \
              afm_token.cc \
              afmkeyword_to_string.cc \
              afmtokentype_to_string.cc
libvx_afm_a_CPPFLAGS = -I${includedir}

include_HEADERS = afm.h \
              afm_keywords.h \
              afmkeyword_to_string.h \
              afm_line.h \
              afm_token.h \
              afm_token_types.h \
              afmtokentype_to_string.h

EXTRA_DIST = afmkeyword_to_string.h \
	afmtokentype_to_string.h