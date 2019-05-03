## Makefile.am -- Process this file with automake to produce Makefile.in
## Copyright (C) 2000, 2006 Gary V. Vaughan
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
## MA 02110-1301, USA.

SUBDIRS	= vx_grid \
	vx_nc_util \
	vx_data2d \
	vx_data2d_grib \
	vx_data2d_nc_met \
	vx_data2d_nccf \
	vx_data2d_nc_pinterp

if ENABLE_PYTHON
  SUBDIRS += vx_python_utils
  SUBDIRS += vx_data2d_python
endif

if ENABLE_GRIB2
  SUBDIRS += vx_data2d_grib2
endif

SUBDIRS += vx_data2d_factory \
	vx_gsl_prob \
	vx_gnomon \
	vx_geodesy \
	vx_gis \
	vx_nav \
	vx_pb_util \
	vx_afm \
	vx_color \
	vx_pxm \
	vx_render \
	vx_ps \
	vx_plot_util \
	vx_shapedata \
	vx_statistics \
	vx_stat_out \
	vx_analysis_util \
	vx_tc_util \
	vx_summary \
	vx_time_series \
	vx_regrid \
	vx_nc_obs \
	vx_solar


MAINTAINERCLEANFILES 	= Makefile.in
