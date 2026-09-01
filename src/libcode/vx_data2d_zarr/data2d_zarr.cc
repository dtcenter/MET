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
// Python embedding script used to read a single 2D field from a Zarr
// store, run via the MET_PYTHON_INPUT_ARG substitution mechanism (see
// data2d_factory.cc). Command line:
//    <MET_PYTHON_INPUT_ARG> <init_time> <lead_time> <var> <level>
//    <set_attr_grid_flag>
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

   //
   // Met2dDataFileFactory does not call set_type() for FileType_Zarr
   // like it does for the other python types.
   //
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

   //
   // _path is always the python command built by
   // build_zarr_python_command() below, not a bare Zarr store path.
   //
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

bool MetZarrDataFile::data_plane(VarInfo &vinfo, DataPlane &plane,
                                 bool do_winds) {

   vinfo.set_req_name(build_zarr_python_command(vinfo).c_str());

   return MetPythonDataFile::data_plane(vinfo, plane, do_winds);
}

////////////////////////////////////////////////////////////////////////

int MetZarrDataFile::data_plane_array(VarInfo &vinfo,
                                      DataPlaneArray &plane_array,
                                      bool do_winds) {

   vinfo.set_req_name(build_zarr_python_command(vinfo).c_str());

   return MetPythonDataFile::data_plane_array(vinfo, plane_array,
                                              do_winds);
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
   // Build the command line expected by read_zarr_dataplane.py:
   //    input_file init_time lead_time var level set_attr_grid_flag
   // input_file is left as the MET_PYTHON_INPUT_ARG token, substituted
   // with the real store path by MetPythonDataFile::data_plane().
   //
   // Use name()/level_name(), not req_name()/req_level_name(), since
   // data_plane() overwrites req_name() with the command built here.
   //

   // Set when this field's set_attr_grid config option is set, so the
   // script can skip its own grid detection (data_class.cc applies
   // the set_attr_grid override generically for every file type).
   const bool has_set_attr_grid = (vinfo.grid_attr().nxy() > 0);

   cmd << replace_path(zarr_read_script)
       << " " << met_python_input_arg
       << " " << unix_to_yyyymmdd_hhmmss(vinfo.init())
       << " " << str_format("%g", (double) vinfo.lead() / 3600.0)
       << " " << vinfo.name()
       << " " << vinfo.level_name()
       << " " << (has_set_attr_grid ? 1 : 0);

   mlog << Debug(4) << "MetZarrDataFile: zarr command = " << cmd << "\n";

   return cmd;
}

////////////////////////////////////////////////////////////////////////
