# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the vx_util library
#
###############################################################################

TARGETS  = libvx_util.a

OBJECTS  =  ascii_table.o \
	asciitablejust_to_string.o \
	bitarray.o \
	boundingbox.o \
	comma_string.o \
	concat_string.o \
	double_array.o \
	indent.o \
	int_array.o \
	is_number.o \
	long_array.o \
	make_path.o \
	memory.o \
	num_array.o \
	ordinal.o \
	roman_numeral.o \
	string_array.o \
	string_fxns.o \
	substring.o \
	thresh_array.o \
	threshold.o \
	vx_endian.o  \
	fix_float.o  \
	command_line.o  \


OPT_FLAGS=

FAKES    = vx_util_all vx_util_lib clean

LIB_DIR  = ..


vx_util_all:
	@ echo
	@ echo "*** Making vx_util Library ***"
	@ echo
	@ $(MAKE) vx_util_lib  $(PRINT_OPTS)
	cp $(TARGETS) ../.

vx_util_lib: libvx_util.a
	@ echo

libvx_util.a: $(OBJECTS)
	ar -rs libvx_util.a $(OBJECTS)
	ranlib libvx_util.a

ascii_table.o: ascii_table.cc
	$(CXX) ascii_table.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

asciitablejust_to_string.o: asciitablejust_to_string.cc
	$(CXX) asciitablejust_to_string.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

bitarray.o: bitarray.cc
	$(CXX) bitarray.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

boundingbox.o: boundingbox.cc
	$(CXX) boundingbox.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

comma_string.o: comma_string.cc
	$(CXX) comma_string.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

concat_string.o: concat_string.cc
	$(CXX) concat_string.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

double_array.o: double_array.cc
	$(CXX) double_array.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

indent.o: indent.cc
	$(CXX) indent.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

int_array.o: int_array.cc
	$(CXX) int_array.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

is_number.o: is_number.cc
	$(CXX) is_number.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

long_array.o: long_array.cc
	$(CXX) long_array.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

make_path.o: make_path.cc
	$(CXX) make_path.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

memory.o: memory.cc
	$(CXX) memory.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

num_array.o: num_array.cc
	$(CXX) num_array.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

ordinal.o: ordinal.cc
	$(CXX) ordinal.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

roman_numeral.o: roman_numeral.cc
	$(CXX) roman_numeral.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

string_array.o: string_array.cc
	$(CXX) string_array.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

string_fxns.o: string_fxns.cc
	$(CXX) string_fxns.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

substring.o: substring.cc
	$(CXX) substring.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

thresh_array.o: thresh_array.cc
	$(CXX) thresh_array.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

threshold.o: threshold.cc
	$(CXX) threshold.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

vx_endian.o: vx_endian.cc
	$(CXX) vx_endian.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -I$(LIB_DIR)

fix_float.o: fix_float.cc fix_float.h
	$(CXX) fix_float.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -I$(LIB_DIR)

command_line.o: command_line.cc command_line.h
	$(CXX) command_line.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c -I$(LIB_DIR)

clean:
	rm -f *.a *.o temp junk core a.out $(TARGETS) ../$(TARGETS)

.PHONY: $(FAKES)
