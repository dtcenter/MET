# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_util library
#
########################################################################


EXECUTABLES =


OBJECTS     =  ascii_table.o \
		asciitablejust_to_string.o \
		bitarray.o \
		boundingbox.o \
		comma_string.o \
		concat_string.o \
		conversions.o \
		data_line.o \
		double_array.o \
		file_type.o \
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
		temp_file.o \
		thresh_array.o \
		threshold.o \
		vx_endian.o \
		fix_float.o \
		command_line.o \
		file_exists.o \
		two_to_one.o \

LIBRARIES   =  libvx_util.a


GEN_SOURCES = asciitablejust_to_string.h   \
		asciitablejust_to_string.cc  \


########################################################################


HEADERS     =  util_constants.h \
		ascii_table.h \
		asciitablejust_to_string.h \
		bitarray.h \
		boundingbox.h \
		comma_string.h \
		concat_string.h \
		conversions.h \
		data_line.h \
		double_array.h \
		file_type.h \
		indent.h \
		int_array.h \
		is_number.h \
		long_array.h \
		make_path.h \
		memory.h \
		num_array.h \
		ordinal.h \
		roman_numeral.h \
		string_array.h \
		string_fxns.h \
		substring.h \
		temp_file.h \
		thresh_array.h \
		threshold.h \
		vx_endian.h \
		fix_float.h \
		command_line.h \
		file_exists.h \
		two_to_one.h \
		vx_util.h \

########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished making vx_util Library ***"
	@ echo


########################################################################


gen_sources: $(GEN_SOURCES)
	@ echo


asciitablejust_to_string.h asciitablejust_to_string.cc: ascii_table.h
	@ echo
	@ rm -f asciitablejust_to_string.h asciitablejust_to_string.cc
	$(ENUM_TO_STRING) ascii_table.h


########################################################################


ascii_table.o: ascii_table.cc
	$(CXX) ascii_table.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

asciitablejust_to_string.o: asciitablejust_to_string.cc
	$(CXX) asciitablejust_to_string.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

bitarray.o: bitarray.cc
	$(CXX) bitarray.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

boundingbox.o: boundingbox.cc
	$(CXX) boundingbox.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

comma_string.o: comma_string.cc
	$(CXX) comma_string.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

concat_string.o: concat_string.cc
	$(CXX) concat_string.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

conversions.o: conversions.cc
	$(CXX) conversions.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

double_array.o: double_array.cc
	$(CXX) double_array.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

file_type.o: file_type.cc
	$(CXX) file_type.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

indent.o: indent.cc
	$(CXX) indent.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

int_array.o: int_array.cc
	$(CXX) int_array.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

is_number.o: is_number.cc
	$(CXX) is_number.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

long_array.o: long_array.cc
	$(CXX) long_array.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

make_path.o: make_path.cc
	$(CXX) make_path.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

memory.o: memory.cc
	$(CXX) memory.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

num_array.o: num_array.cc
	$(CXX) num_array.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

ordinal.o: ordinal.cc
	$(CXX) ordinal.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

roman_numeral.o: roman_numeral.cc
	$(CXX) roman_numeral.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

string_array.o: string_array.cc
	$(CXX) string_array.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

string_fxns.o: string_fxns.cc
	$(CXX) string_fxns.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

substring.o: substring.cc
	$(CXX) substring.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

temp_file.o: temp_file.cc
	$(CXX) temp_file.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

thresh_array.o: thresh_array.cc
	$(CXX) thresh_array.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

threshold.o: threshold.cc
	$(CXX) threshold.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(MET_INC_DIR)

vx_endian.o: vx_endian.cc
	$(CXX) vx_endian.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c

fix_float.o: fix_float.cc fix_float.h
	$(CXX) fix_float.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c

command_line.o: command_line.cc command_line.h
	$(CXX) command_line.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c

file_exists.o: file_exists.cc file_exists.h
	$(CXX) file_exists.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c

two_to_one.o: two_to_one.cc two_to_one.h
	$(CXX) two_to_one.cc $(CXX_FLAGS) $(OPT_FLAGS) $(ARCH_FLAGS) -c


########################################################################


   ##
   ##  libraries
   ##


libvx_util.a: $(OBJECTS)
	ar -rs libvx_util.a $(OBJECTS)
	ranlib libvx_util.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_util.a
	cp libvx_util.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_util.a
	rm -f asciitablejust_to_string.h asciitablejust_to_string.cc


########################################################################


.PHONY: all clean


########################################################################



