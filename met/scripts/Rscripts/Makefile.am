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

SUBDIRS = include

## NOTE: some Rscripts use the path MET_BASE. ie. plot_tcmpr.R
## MET_BASE = $(prefix)/share/met, which is $(pkddatadir)
## If you want to install outside of MET_BASE you
## will need to update the R scripts.

## Example of how to Install outside of $(pkgdatadir)
## scriptsrootdir = $(prefix)/share/scripts
## rscriptsdir = ${scriptsrootdir}/Rscripts

rscriptsdir = $(pkgdatadir)/Rscripts

rscripts_DATA = \
	gspairs2mpr.R \
	mode_nc_inventory.R \
	mode_quilt_plot.R \
	mode_summary.R \
	plot_cnt.R \
	plot_mpr.R \
	plot_ensemble.R \
	plot_probri.R \
	plot_tcmpr.R \
	pntnc2ascii.R \
	trmm2nc.R \
	trmmbin2nc.R

EXTRA_DIST = ${rscripts_DATA}

MAINTAINERCLEANFILES 	= Makefile.in
