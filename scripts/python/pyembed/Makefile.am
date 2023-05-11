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

SUBDIRS =

pyembeddir = $(pkgdatadir)/python/pyembed

pyembed_DATA = \
     python_embedding.py \
     read_tmp_dataplane.py \
     read_tmp_ascii.py \
     read_tmp_point_nc.py \
     set_python_env.py \
     write_tmp_dataplane.py \
     write_tmp_point.py \
     write_tmp_point_nc.py \
     write_tmp_mpr.py

EXTRA_DIST = ${pyembed_DATA}

MAINTAINERCLEANFILES 	= Makefile.in 
