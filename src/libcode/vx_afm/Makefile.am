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

# If we are in development mode, generate the "to_string" files and
# clean them up in a "make clean".  We don't want to do either of these
# in a distribution because the enum_to_string utility isn't available.

if ENABLE_DEVELOPMENT
    afmkeyword_to_string.h afmkeyword_to_string.cc: afm_keywords.h
	${ENUM_TO_STRING} -concat_string afm_keywords.h

    afmtokentype_to_string.h afmtokentype_to_string.cc: afm_token_types.h
	${ENUM_TO_STRING} -concat_string afm_token_types.h

   clean-local:
	-rm -f afmkeyword_to_string.cc
	-rm -f afmkeyword_to_string.h
	-rm -f afmtokentype_to_string.cc
	-rm -f afmtokentype_to_string.h
endif