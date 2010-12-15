# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2011
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


OBJECTS     = date_to_mjd.o     \
              day_dif.o         \
              day_of_week.o     \
              doyhms_to_unix.o  \
              easter.o          \
              holiday.o         \
              is_dst.o          \
              is_leap_year.o    \
              mdyhms_to_unix.o  \
              mjd_to_date.o     \
              time_array.o      \
              time_strings.o    \
              unix_to_mdyhms.o


LIBRARIES   = libvx_cal.a


########################################################################


HEADERS     = holiday.h         \
              time_array.h      \
              vx_cal.h


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)


########################################################################


   ##
   ##  objects
   ##


date_to_mjd.o: date_to_mjd.cc
	$(CXX) date_to_mjd.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

day_dif.o: day_dif.cc
	$(CXX) day_dif.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

day_of_week.o: day_of_week.cc
	$(CXX) day_of_week.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

doyhms_to_unix.o: doyhms_to_unix.cc
	$(CXX) doyhms_to_unix.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

easter.o: easter.cc
	$(CXX) easter.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

holiday.o: holiday.cc
	$(CXX) holiday.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

is_dst.o: is_dst.cc
	$(CXX) is_dst.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

is_leap_year.o: is_leap_year.cc
	$(CXX) is_leap_year.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

mdyhms_to_unix.o: mdyhms_to_unix.cc
	$(CXX) mdyhms_to_unix.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

mjd_to_date.o: mjd_to_date.cc
	$(CXX) mjd_to_date.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

time_array.o: time_array.cc
	$(CXX) time_array.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

time_strings.o: time_strings.cc
	$(CXX) time_strings.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

unix_to_mdyhms.o: unix_to_mdyhms.cc
	$(CXX) unix_to_mdyhms.cc $(CXX_FLAGS) $(OPT_FLAGS) -c


########################################################################


   ##
   ##  libraries
   ##


libvx_cal.a: $(OBJECTS)
	ar -rs libvx_cal.a $(OBJECTS)
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


.PHONY: all clean


########################################################################


