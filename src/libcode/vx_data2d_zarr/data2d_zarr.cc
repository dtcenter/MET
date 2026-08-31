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
#include "vx_util.h"
#include "vx_cal.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

//
// Path to the Python embedding script used to read a single 2D field
// from a Zarr store. Command line arguments:
//    <MET_PYTHON_INPUT_ARG> <init time, YYYYMMDD_HHMMSS>
//    <lead time, decimal hours> <variable name> <level name>
//

static const char zarr_read_script[] =
   "MET_BASE/python/pyembed/read_zarr_dataplane.py";

////////////////////////////////////////////////////////////////////////

static ConcatString build_zarr_python_command(VarInfo &vinfo);

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

   python_init_from_scratch();

   set_type(FileType_Zarr);

   return;
}

////////////////////////////////////////////////////////////////////////

void MetZarrDataFile::close() {

   MetPythonDataFile::close();

   return;
}

////////////////////////////////////////////////////////////////////////

bool MetZarrDataFile::open(const char * _path) {

   return MetPythonDataFile::open(_path);
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
   }
   else {
      out << prefix << "No raw Grid!\n";
   }

   if(Dest_Grid) {
      out << prefix << "Dest Grid:\n";
      Dest_Grid->dump(out, depth + 1);
   }
   else {
      out << prefix << "No dest Grid!\n";
   }

   out.flush();

   return;
}

////////////////////////////////////////////////////////////////////////

bool MetZarrDataFile::data_plane(VarInfo &vinfo, DataPlane &plane) {

   // Build the Python command for the requested field and let the
   // parent class run it, cache it, substitute MET_PYTHON_INPUT_ARG,
   // and populate Raw_Grid/Dest_Grid/Plane/VInfo

   vinfo.set_req_name(build_zarr_python_command(vinfo).c_str());

   return MetPythonDataFile::data_plane(vinfo, plane);
}

////////////////////////////////////////////////////////////////////////

int MetZarrDataFile::data_plane_array(VarInfo &vinfo,
                                      DataPlaneArray &plane_array) {

   vinfo.set_req_name(build_zarr_python_command(vinfo).c_str());

   return MetPythonDataFile::data_plane_array(vinfo, plane_array);
}

////////////////////////////////////////////////////////////////////////

int MetZarrDataFile::index(VarInfo &vinfo) {

   // Is the file open?
   if(!Raw_Grid) return -1;

   return 0;
}

////////////////////////////////////////////////////////////////////////

static ConcatString build_zarr_python_command(VarInfo &vinfo) {

   ConcatString cmd;

   //
   // Arguments for read_zarr_dataplane.py:
   //    input_file init_time lead_time var level
   //
   //  - input_file is left as the literal MET_PYTHON_INPUT_ARG token
   //    and MetPythonDataFile::data_plane() substitutes it with the
   //    real Zarr store path (from the environment variable of the
   //    name) before running the script
   //  - init_time in YYYYMMDD_HHMMSS format
   //  - lead_time in decimal hours
   //

   cmd << replace_path(zarr_read_script)
       << " " << met_python_input_arg
       << " " << unix_to_yyyymmdd_hhmmss(vinfo.init())
       << " " << str_format("%g", (double) vinfo.lead() / 3600.0)
       << " " << vinfo.name()
       << " " << vinfo.level_name();

   mlog << Debug(4) << "MetZarrDataFile: zarr command = " << cmd << "\n";

   return cmd;
}

////////////////////////////////////////////////////////////////////////
