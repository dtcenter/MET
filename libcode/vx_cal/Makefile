# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the vx_cal library
#
###############################################################################

TARGETS  = libvx_cal.a

OBJECTS  =  date_to_mjd.o \
	day_dif.o \
	day_of_week.o \
	doyhms_to_unix.o \
	easter.o \
	holiday.o \
	is_dst.o \
	is_leap_year.o \
	mdyhms_to_unix.o \
	mjd_to_date.o \
	time_strings.o \
	time_array.o \
	unix_to_mdyhms.o

OPT_FLAGS=

FAKES    = vx_cal_all vx_cal_lib clean

LIB_DIR  = ..

vx_cal_all:
	@ echo
	@ echo "*** Making vx_cal Library ***"
	@ echo
	@ $(MAKE) vx_cal_lib  $(PRINT_OPTS)
	cp $(TARGETS) ../.

vx_cal_lib: libvx_cal.a
	@ echo

libvx_cal.a: $(OBJECTS)
	ar -rs libvx_cal.a $(OBJECTS)
	ranlib libvx_cal.a

date_to_mjd.o: date_to_mjd.cc
	$(CXX) date_to_mjd.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

day_dif.o: day_dif.cc
	$(CXX) day_dif.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

day_of_week.o: day_of_week.cc
	$(CXX) day_of_week.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

doyhms_to_unix.o: doyhms_to_unix.cc
	$(CXX) doyhms_to_unix.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

easter.o: easter.cc
	$(CXX) easter.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

holiday.o: holiday.cc
	$(CXX) holiday.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

is_dst.o: is_dst.cc
	$(CXX) is_dst.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

is_leap_year.o: is_leap_year.cc
	$(CXX) is_leap_year.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

mdyhms_to_unix.o: mdyhms_to_unix.cc
	$(CXX) mdyhms_to_unix.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

mjd_to_date.o: mjd_to_date.cc
	$(CXX) mjd_to_date.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

time_strings.o: time_strings.cc
	$(CXX) time_strings.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

time_array.o: time_array.cc
	$(CXX) time_array.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

unix_to_mdyhms.o: unix_to_mdyhms.cc
	$(CXX) unix_to_mdyhms.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

clean:
	rm -f *.a *.o temp junk core a.out $(TARGETS) ../$(TARGETS)

.PHONY: $(FAKES)


