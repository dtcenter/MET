# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 1992 - 2007
# ** University Corporation for Atmospheric Research (UCAR)
# ** National Center for Atmospheric Research (NCAR)
# ** Research Applications Lab (RAL)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

########################################################################
#
# Makefile for the vx_render library
#
########################################################################


EXECUTABLES = 


OBJECTS     = vx_ascii85_filter.o \
              vx_bit_filter.o     \
              vx_hex_filter.o     \
              vx_ps_filter.o      \
              vx_psout_filter.o   \
              vx_render_pbm.o     \
              vx_render_pcm.o     \
              vx_render_pgm.o     \
              vx_render_ppm.o     \
              vx_renderinfo.o     \
              vx_rle_filter.o     \
              vx_uc_queue.o       \


LIBRARIES   = libvx_render.a


########################################################################


HEADERS     = vx_ascii85_filter.h  \
              vx_bit_filter.h      \
              vx_hex_filter.h      \
              vx_ps_filter.h       \
              vx_psout_filter.h    \
              vx_render.h          \
              vx_renderinfo.h      \
              vx_rle_filter.h      \
              vx_uc_queue.h        \


########################################################################


all: $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	@ echo


########################################################################


   ##
   ##  objects
   ##


vx_ascii85_filter.o: vx_ascii85_filter.cc
	$(CXX) vx_ascii85_filter.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

vx_bit_filter.o: vx_bit_filter.cc
	$(CXX) vx_bit_filter.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

vx_hex_filter.o: vx_hex_filter.cc
	$(CXX) vx_hex_filter.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

vx_ps_filter.o: vx_ps_filter.cc
	$(CXX) vx_ps_filter.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

vx_psout_filter.o: vx_psout_filter.cc
	$(CXX) vx_psout_filter.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

vx_render_pbm.o: vx_render_pbm.cc
	$(CXX) vx_render_pbm.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

vx_render_pcm.o: vx_render_pcm.cc
	$(CXX) vx_render_pcm.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

vx_render_pgm.o: vx_render_pgm.cc
	$(CXX) vx_render_pgm.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

vx_render_ppm.o: vx_render_ppm.cc
	$(CXX) vx_render_ppm.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

vx_renderinfo.o: vx_renderinfo.cc
	$(CXX) vx_renderinfo.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

vx_rle_filter.o: vx_rle_filter.cc
	$(CXX) vx_rle_filter.cc $(CXX_FLAGS) $(OPT_FLAGS) -c

vx_uc_queue.o: vx_uc_queue.cc
	$(CXX) vx_uc_queue.cc $(CXX_FLAGS) $(OPT_FLAGS) -c


########################################################################


   ##
   ##  libraries
   ##


libvx_render.a: $(OBJECTS)
     ar -rs libvx_render.a $(OBJECTS)
     ranlib libvx_render.a
     cd $(MET_INC_DIR) ; rm -f $(HEADERS)
     cp $(HEADERS) $(MET_INC_DIR)
     rm -f $(MET_LIB_DIR)/libvx_render.a
     cp libvx_render.a $(MET_LIB_DIR)


########################################################################


   ##
   ##  executables
   ##


########################################################################


clean:
	rm -f *.a *.o temp junk core a.out $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)
	cd $(MET_INC_DIR) ; rm -f $(HEADERS)
	rm -f $(MET_LIB_DIR)/libvx_render.a


########################################################################


.PHONY: all clean


########################################################################



