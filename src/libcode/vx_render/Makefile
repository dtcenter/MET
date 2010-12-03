# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

###############################################################################
#
# Makefile for the vx_render library
#
###############################################################################

TARGETS  = libvx_render.a

OBJECTS  =  vx_ascii85_filter.o \
	vx_bit_filter.o \
	vx_hex_filter.o \
	vx_ps_filter.o \
	vx_psout_filter.o \
	vx_render_pbm.o \
	vx_render_pcm.o \
	vx_render_pgm.o \
	vx_render_ppm.o \
	vx_renderinfo.o \
	vx_rle_filter.o \
	vx_uc_queue.o

OPT_FLAGS=

FAKES    = vx_render_all vx_render_lib clean

LIB_DIR  = ..


vx_render_all:
	@ echo
	@ echo "*** Making vx_render Library ***"
	@ echo
	@ $(MAKE) vx_render_lib  $(PRINT_OPTS)
	cp $(TARGETS) ../.

vx_render_lib: libvx_render.a
	@ echo

libvx_render.a: $(OBJECTS)
	ar -rs libvx_render.a $(OBJECTS)
	ranlib libvx_render.a

vx_ascii85_filter.o: vx_ascii85_filter.cc
	$(CXX) vx_ascii85_filter.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

vx_bit_filter.o: vx_bit_filter.cc
	$(CXX) vx_bit_filter.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

vx_hex_filter.o: vx_hex_filter.cc
	$(CXX) vx_hex_filter.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

vx_ps_filter.o: vx_ps_filter.cc
	$(CXX) vx_ps_filter.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

vx_psout_filter.o: vx_psout_filter.cc
	$(CXX) vx_psout_filter.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

vx_render_pbm.o: vx_render_pbm.cc
	$(CXX) vx_render_pbm.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

vx_render_pcm.o: vx_render_pcm.cc
	$(CXX) vx_render_pcm.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

vx_render_pgm.o: vx_render_pgm.cc
	$(CXX) vx_render_pgm.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

vx_render_ppm.o: vx_render_ppm.cc
	$(CXX) vx_render_ppm.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

vx_renderinfo.o: vx_renderinfo.cc
	$(CXX) vx_renderinfo.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

vx_rle_filter.o: vx_rle_filter.cc
	$(CXX) vx_rle_filter.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

vx_uc_queue.o: vx_uc_queue.cc
	$(CXX) vx_uc_queue.cc $(CXX_FLAGS) $(OPT_FLAGS) -c -I$(LIB_DIR)

clean:
	rm -f *.a *.o temp junk core a.out $(TARGETS) ../$(TARGETS)

.PHONY: $(FAKES)


