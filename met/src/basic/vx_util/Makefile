# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2013
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


EXECUTABLES =  test_command_line \
               test_data_plane   \
               test_add_rows     \


OBJECTS     =  ascii_table.o \
               asciitablejust_to_string.o \
               check_endian.o \
               comma_string.o \
               conversions.o \
               data_line.o \
               grib_constants.o \
               int_array.o \
               is_number.o \
               long_array.o \
               make_path.o \
               memory.o \
               num_array.o \
               ordinal.o \
               roman_numeral.o \
               string_fxns.o \
               substring.o \
               temp_file.o \
               thresh_array.o \
               threshold.o \
               fix_float.o \
               command_line.o \
               file_exists.o \
               file_linecount.o \
               filename_suffix.o \
               data_plane.o \
               data_plane_util.o \
               interp_mthd.o \
               interp_util.o \
               two_to_one.o \
               get_filenames.o


LIBRARIES   =  libvx_util.a


GEN_SOURCES = asciitablejust_to_string.h \
              asciitablejust_to_string.cc


########################################################################


HEADERS     =  util_constants.h \
               ascii_table.h \
               asciitablejust_to_string.h \
               check_endian.h \
               comma_string.h \
               conversions.h \
               data_line.h \
               grib_constants.h \
               int_array.h \
               is_number.h \
               long_array.h \
               make_path.h \
               memory.h \
               num_array.h \
               ordinal.h \
               roman_numeral.h \
               string_fxns.h \
               substring.h \
               temp_file.h \
               thresh_array.h \
               threshold.h \
               fix_float.h \
               command_line.h \
               file_exists.h \
               file_linecount.h \
               filename_suffix.h \
               data_plane.h \
               data_plane_util.h \
               interp_mthd.h \
               two_to_one.h \
               interp_util.h \
               bool_to_string.h \
               empty_string.h \
               get_filenames.h \
               vx_util.h  \


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_util Library ***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##  call make all since this library is used by source code
   ##  generation tools
   ##


gen_sources: $(GEN_SOURCES)
	@ $(MAKE) all


asciitablejust_to_string.h asciitablejust_to_string.cc: ascii_table.h
	@ echo
	@ rm -f asciitablejust_to_string.h asciitablejust_to_string.cc
	$(ENUM_TO_STRING) -concat_string ascii_table.h


########################################################################


   ##
   ##  objects
   ##


ascii_table.o: ascii_table.cc
	$(CXX) ascii_table.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

asciitablejust_to_string.o: asciitablejust_to_string.cc
	$(CXX) asciitablejust_to_string.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

check_endian.o: check_endian.cc
	$(CXX) check_endian.cc $(CXX_FLAGS) -c

comma_string.o: comma_string.cc
	$(CXX) comma_string.cc $(CXX_FLAGS) -c

conversions.o: conversions.cc
	$(CXX) conversions.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

data_line.o: data_line.cc
	$(CXX) data_line.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

data_plane.o: data_plane.cc
	$(CXX) data_plane.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

data_plane_util.o: data_plane_util.cc
	$(CXX) data_plane_util.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

grib_constants.o: grib_constants.cc
	$(CXX) grib_constants.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

int_array.o: int_array.cc
	$(CXX) int_array.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

interp_mthd.o: interp_mthd.cc
	$(CXX) interp_mthd.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

interp_util.o: interp_util.cc
	$(CXX) interp_util.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

is_number.o: is_number.cc
	$(CXX) is_number.cc $(CXX_FLAGS) -c

long_array.o: long_array.cc
	$(CXX) long_array.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

make_path.o: make_path.cc
	$(CXX) make_path.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

memory.o: memory.cc
	$(CXX) memory.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

num_array.o: num_array.cc
	$(CXX) num_array.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

ordinal.o: ordinal.cc
	$(CXX) ordinal.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

roman_numeral.o: roman_numeral.cc
	$(CXX) roman_numeral.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

string_fxns.o: string_fxns.cc
	$(CXX) string_fxns.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR) -DMET_BASE=\"$(MET_BASE_DIR)\"

substring.o: substring.cc
	$(CXX) substring.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

temp_file.o: temp_file.cc
	$(CXX) temp_file.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

thresh_array.o: thresh_array.cc
	$(CXX) thresh_array.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

threshold.o: threshold.cc
	$(CXX) threshold.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

fix_float.o: fix_float.cc fix_float.h
	$(CXX) fix_float.cc $(CXX_FLAGS) -c

command_line.o: command_line.cc command_line.h
	$(CXX) command_line.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

file_exists.o: file_exists.cc file_exists.h
	$(CXX) file_exists.cc $(CXX_FLAGS) -c

file_linecount.o: file_linecount.cc file_linecount.h
	$(CXX) file_linecount.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

filename_suffix.o: filename_suffix.cc filename_suffix.h
	$(CXX) filename_suffix.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

two_to_one.o: two_to_one.cc two_to_one.h
	$(CXX) two_to_one.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

get_filenames.o: get_filenames.cc get_filenames.h
	$(CXX) get_filenames.cc $(CXX_FLAGS) $(ARCH_FLAGS) -c -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_util.a: $(OBJECTS)
	ar -rs libvx_util.a $(OBJECTS) 2>&1
	ranlib libvx_util.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_util.a
	cp libvx_util.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##

test_command_line: test_command_line.cc $(LIBRARIES)
	@ echo
	$(CXX) test_command_line.cc -o test_command_line $(CXX_FLAGS) -I$(MET_INC_DIR) -L$(MET_LIB_DIR) \
	-lvx_util \
	-lvx_log


test_data_plane: test_data_plane.cc $(LIBRARIES)
	@ echo
	$(CXX) test_data_plane.cc -o test_data_plane $(CXX_FLAGS) -I$(MET_INC_DIR) -L$(MET_LIB_DIR) \
	-lvx_util \
	-lvx_cal \
	-lvx_log


test_add_rows: test_add_rows.cc $(LIBRARIES)
	@ echo
	$(CXX) test_add_rows.cc -o test_add_rows $(CXX_FLAGS) -I$(MET_INC_DIR) -L$(MET_LIB_DIR) \
	-lvx_util \
	-lvx_cal \
	-lvx_log


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) $(GEN_SOURCES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_util.a


########################################################################


.PHONY: all clean gen_sources


########################################################################

