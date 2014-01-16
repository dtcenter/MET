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

ARCH_FLAGS = $(ARCH_FLAGS) -DWITH_GRIB2
G2SUP_LIBS = -lgrib2c -ljasper -lpng -lz
GRIB2_LIBS = -lvx_data2d_grib2 $(G2SUP_LIBS)

F2C_LIBNAME =
FC_LIBS = -lgfortran

CXX_FLAGS = $(CXX_FLAGS) $(ARCH_FLAGS)

ACLOCAL_AMFLAGS		= -I m4

SUBDIRS 		= basic libcode tools

MAINTAINERCLEANFILES 	= Makefile.in 
