# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2012
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_cal library
#
########################################################################


EXECUTABLES =


OBJECTS     = date_to_mjd.o \
              day_dif.o \
              day_of_week.o \
              doyhms_to_unix.o \
              is_dst.o \
              is_leap_year.o \
              mdyhms_to_unix.o \
              mjd_to_date.o \
              time_array.o \
              time_strings.o \
              unix_to_mdyhms.o


LIBRARIES   = libvx_cal.a


GEN_SOURCES =


########################################################################


HEADERS     = time_array.h \
              vx_cal.h


########################################################################


all: $(GEN_SOURCES) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo
	@ echo "*** Finished Making the vx_cal Library ***"
	@ echo


########################################################################


   ##
   ##  generated sources
   ##  call make all since this library is used by source code
   ##  generation tools
   ##


gen_sources: $(GEN_SOURCES)
	@ make all


########################################################################


   ##
   ##  objects
   ##


date_to_mjd.o: date_to_mjd.cc
	$(CXX) date_to_mjd.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

day_dif.o: day_dif.cc
	$(CXX) day_dif.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

day_of_week.o: day_of_week.cc
	$(CXX) day_of_week.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

doyhms_to_unix.o: doyhms_to_unix.cc
	$(CXX) doyhms_to_unix.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

is_dst.o: is_dst.cc
	$(CXX) is_dst.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

is_leap_year.o: is_leap_year.cc
	$(CXX) is_leap_year.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

mdyhms_to_unix.o: mdyhms_to_unix.cc
	$(CXX) mdyhms_to_unix.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

mjd_to_date.o: mjd_to_date.cc
	$(CXX) mjd_to_date.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

time_array.o: time_array.cc
	$(CXX) time_array.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

time_strings.o: time_strings.cc
	$(CXX) time_strings.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)

unix_to_mdyhms.o: unix_to_mdyhms.cc
	$(CXX) unix_to_mdyhms.cc $(CXX_FLAGS) -c -I$(MET_INC_DIR)


########################################################################


   ##
   ##  libraries
   ##


libvx_cal.a: $(OBJECTS)
	ar -rs libvx_cal.a $(OBJECTS) 2>&1
	ranlib libvx_cal.a
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	cp $(HEADERS) $(MET_INC_DIR)
	rm -f $(MET_LIB_DIR)/libvx_cal.a
	cp libvx_cal.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_cal.a


########################################################################


.PHONY: all clean gen_sources


########################################################################

