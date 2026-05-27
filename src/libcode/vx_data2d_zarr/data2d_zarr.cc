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

bool MetZarrDataFile::open(const char * _filename) {
   // TODO: Open the input file
   return true;
}

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
   auto vinfo_zarr = (VarInfoZarr*)(&vinfo);

   return true;
}

////////////////////////////////////////////////////////////////////////

int MetZarrDataFile::data_plane_array(VarInfo &vinfo,
                                      DataPlaneArray &plane_array) {

   // Initialize
   plane_array.clear();

   //  narrow the vinfo pointer
   auto vinfo_zarr = (VarInfoZarr*)(&vinfo);

   return 0;
}

////////////////////////////////////////////////////////////////////////

int MetZarrDataFile::index(VarInfo &vinfo) {

   //  Is the file open?
   if(!Raw_Grid) return -1;
   //
   //  ok
   //

   return 0;
}

////////////////////////////////////////////////////////////////////////
