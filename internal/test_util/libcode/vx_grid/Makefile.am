## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# Test programs

noinst_PROGRAMS = test_grid_area \
	search_3d_kdtree \
	search_3d_kdtree_api

test_grid_area_SOURCES = test_grid_area.cc
test_grid_area_CPPFLAGS = ${MET_CPPFLAGS}
test_grid_area_LDFLAGS = -L. ${MET_LDFLAGS}
test_grid_area_LDADD = \
	$(UGRID_MET_LIBS) \
	-lvx_grid \
	-lvx_geodesy \
	-lvx_nav \
	-lvx_util_math \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	$(UGRID_DEP_LIBS) \
	-lz -lm -lproj

search_3d_kdtree_SOURCES =  search_3d_kdtree.cc
search_3d_kdtree_CPPFLAGS = ${MET_CPPFLAGS}
search_3d_kdtree_LDFLAGS = -L. ${MET_LDFLAGS}
search_3d_kdtree_LDADD = \
	$(UGRID_MET_LIBS) \
	-lvx_nc_util \
	-lvx_geodesy \
	-lvx_nav \
	-lvx_util_math \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	$(UGRID_DEP_LIBS) \
	-lz -lm -lproj \
	-lnetcdf_c++4 -lnetcdf

search_3d_kdtree_api_SOURCES =  search_3d_kdtree_api.cc
search_3d_kdtree_api_CPPFLAGS = ${MET_CPPFLAGS}
search_3d_kdtree_api_LDFLAGS = -L. ${MET_LDFLAGS}
search_3d_kdtree_api_LDADD = \
	$(UGRID_MET_LIBS) \
	-lvx_nc_util \
	-lvx_grid \
	-lvx_geodesy \
	-lvx_nav \
	-lvx_util_math \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	$(UGRID_DEP_LIBS) \
	-lz -lm -lproj \
	-lnetcdf_c++4 -lnetcdf
