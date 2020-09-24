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

SUBDIRS	= 

if ENABLE_GIS_UTILS
  SUBDIRS	+= gis_utils
endif

if ENABLE_GEN_VX_MASK
  SUBDIRS += gen_vx_mask
endif

if ENABLE_PLOT_DATA_PLANE
  SUBDIRS += plot_data_plane
endif

if ENABLE_PLOT_POINT_OBS
  SUBDIRS += plot_point_obs
endif

if ENABLE_ASCII2NC
  SUBDIRS += ascii2nc
endif

if ENABLE_LIDAR2NC
  SUBDIRS += lidar2nc
endif

if ENABLE_MADIS2NC
  SUBDIRS += madis2nc
endif

if ENABLE_PB2NC
  SUBDIRS += pb2nc
endif

if ENABLE_WWMCA
  SUBDIRS += wwmca_tool
endif

if ENABLE_MODIS
  SUBDIRS += modis_regrid
endif

if ENABLE_MODE_GRAPHICS
  SUBDIRS += mode_graphics
endif

if ENABLE_MODE_TIME_DOMAIN
  SUBDIRS += mode_time_domain
endif

if ENABLE_GSI_TOOLS
  SUBDIRS += gsi_tools
endif

if ENABLE_REGRID_DATA_PLANE
  SUBDIRS += regrid_data_plane
endif

if ENABLE_POINT2GRID
  SUBDIRS += point2grid
endif

if ENABLE_SHIFT_DATA_PLANE
  SUBDIRS += shift_data_plane
endif

SUBDIRS += grid_diag

MAINTAINERCLEANFILES 	= Makefile.in
