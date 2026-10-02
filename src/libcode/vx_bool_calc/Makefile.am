## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_bool_calc.a
libvx_bool_calc_a_SOURCES = \
              tokentype_to_string.cc tokentype_to_string.h \
              make_program.cc make_program.h \
              bool_calc.cc bool_calc.h \
              token.cc token.h \
              tokenizer.cc tokenizer.h \
              token_stack.cc token_stack.h
libvx_bool_calc_a_CPPFLAGS = ${MET_CPPFLAGS}

if ENABLE_DEVELOPMENT

# BUILT_SOURCES so these are generated before anything that
# includes them is compiled.  Without it, a development build
# whose .deps are empty - as after the make clean that removes
# these very files - compiles sources in _SOURCES order and can
# reach a consumer before its header has been generated.

BUILT_SOURCES = tokentype_to_string.cc \
                tokentype_to_string.h

tokentype_to_string.cc tokentype_to_string.h: token.h
	${ENUM_TO_STRING} -concat_string -reverse token.h
 
clean-local:
	-rm -f tokentype_to_string.cc
	-rm -f tokentype_to_string.h
endif

