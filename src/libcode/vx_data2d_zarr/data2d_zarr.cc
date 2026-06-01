// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <cmath>

#include <string>
#include <map>
#include <utility>
#include <limits>
#include <list>

#include "data2d_zarr.h"
#include "vx_data2d.h"
#include "vx_math.h"
#include "vx_log.h"

#include "is_zarr_store.h"

using namespace std;

////////////////////////////////////////////////////////////////////////
//
// Code for class MetZarrDataFile
//
////////////////////////////////////////////////////////////////////////

MetZarrDataFile::MetZarrDataFile() {
   zarr_init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

MetZarrDataFile::~MetZarrDataFile() {
   close();
}

////////////////////////////////////////////////////////////////////////

MetZarrDataFile::MetZarrDataFile(const MetZarrDataFile &) {
   mlog << Error << "\nMetZarrDataFile::MetZarrDataFile(const MetZarrDataFile &) -> "
        << "should never be called!\n\n";
   exit(1);
}

////////////////////////////////////////////////////////////////////////

MetZarrDataFile & MetZarrDataFile::operator=(const MetZarrDataFile &) {
   mlog << Error << "\nMetZarrDataFile::operator=(const MetZarrDataFile &) -> "
        << "should never be called!\n\n";
   exit(1);
}

////////////////////////////////////////////////////////////////////////

void MetZarrDataFile::zarr_init_from_scratch() {
   return;
}

////////////////////////////////////////////////////////////////////////

void MetZarrDataFile::close() {
   // TODO: Close the input file
   return;
}

////////////////////////////////////////////////////////////////////////

bool MetZarrDataFile::open(const char * _path) {
   const char *method_name = "MetZarrDataFile::open(const char *) -> ";

   // Check for a valid Zarr store
   if(!is_zarr_store(_path)) {
      mlog << Error << "\n" << method_name
           << _path << " is not a valid Zarr store.\n\n";
      exit(1);
   }

   Filename = _path;

   return true;
}

// JHG: We could consider having the open function inventory the
// list of dataset names and/or inspect the metadata, but there's
// no clear and obvious need for that here since the data_plane
// function will need to handle it. Note that a single Zarr store can
// contain multiple grids. 

////////////////////////////////////////////////////////////////////////

void MetZarrDataFile::dump(ostream & out, int depth) const {
   Indent prefix(depth);

   out << prefix << "File = ";
   if(Filename.empty()) out << "(nul)\n";
   else                 out << '\"' << Filename << "\"\n";

   if(Raw_Grid) {
      out << prefix << "Raw Grid:\n";
      Raw_Grid->dump(out, depth + 1);
   } else {
      out << prefix << "No raw Grid!\n";
   }

   if(Dest_Grid) {
      out << prefix << "Dest Grid:\n";
      Dest_Grid->dump(out, depth + 1);
   } else {
      out << prefix << "No dest Grid!\n";
   }

   out.flush();

   return;
}

////////////////////////////////////////////////////////////////////////

bool MetZarrDataFile::data_plane(VarInfo &vinfo, DataPlane &plane) {

   // Narrow the vinfo pointer
   auto vinfo_python = (VarInfoPython*)(&vinfo);

   // JHG: call python function to read the data

   return true;
}

////////////////////////////////////////////////////////////////////////

int MetZarrDataFile::data_plane_array(VarInfo &vinfo,
                                      DataPlaneArray &plane_array) {

   // Initialize
   plane_array.clear();

   // Narrow the vinfo pointer
   auto vinfo_python = (VarInfoPython*)(&vinfo);

   // JHG: call python function to read multiple data planes

   return 0;
}

////////////////////////////////////////////////////////////////////////

int MetZarrDataFile::index(VarInfo &vinfo) {

   // Is the file open?
   if(!Raw_Grid) return -1;

   return 0;
}

////////////////////////////////////////////////////////////////////////

