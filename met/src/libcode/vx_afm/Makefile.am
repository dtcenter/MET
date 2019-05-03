## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_afm.a
libvx_afm_a_SOURCES = afm.cc afm.h \
              afm_keywords.cc afm_keywords.h \
              afm_line.cc afm_line.h \
              afm_token.cc afm_token.h \
              afmkeyword_to_string.cc afmkeyword_to_string.h \
              afmtokentype_to_string.cc afmtokentype_to_string.h \
              afm_token_types.h
libvx_afm_a_CPPFLAGS = ${MET_CPPFLAGS}
