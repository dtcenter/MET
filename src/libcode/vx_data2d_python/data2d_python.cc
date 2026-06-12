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
#include <cmath>

#include "data2d_python.h"
#include "vx_python3_utils.h"
#include "data2d_utils.h"
#include "grdfiletype_to_string.h"

#include "vx_math.h"
#include "vx_log.h"

using namespace std;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class MetPythonDataFile
   //


////////////////////////////////////////////////////////////////////////


MetPythonDataFile::MetPythonDataFile()

{

python_init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


MetPythonDataFile::~MetPythonDataFile()

{

close();

}


////////////////////////////////////////////////////////////////////////


MetPythonDataFile::MetPythonDataFile(const MetPythonDataFile &)

{

mlog << Error << "\nMetPythonDataFile::MetPythonDataFile(const MetPythonDataFile &) -> "
     << "should never be called!\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


MetPythonDataFile & MetPythonDataFile::operator=(const MetPythonDataFile &)

{

mlog << Error << "\nMetPythonDataFile::operator=(const MetPythonDataFile &) -> "
     << "should never be called!\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


void MetPythonDataFile::python_init_from_scratch()

{

PythonCommand.clear();
Plane.clear();
VInfo.clear();

close();

return;

}


////////////////////////////////////////////////////////////////////////


void MetPythonDataFile::close()

{

Plane.clear();
VInfo.clear();

mtddf_clear();   //   base class

   //
   //  Don't reset the Type field
   //  Don't reset the PythonCommand
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MetPythonDataFile::set_type(const GrdFileType t)

{

Type = t;

return;

}


////////////////////////////////////////////////////////////////////////


bool MetPythonDataFile::open(const char * cur_command)

{

close();

const char *method_name = "MetPythonDataFile::open() ";

   //
   //  Store the PythonCommand that is being run
   //

PythonCommand = cur_command;

   //
   //  Build the path and store the file name
   //

StringArray sa = PythonCommand.split(" ");

ConcatString full_path(sa[0]);

ConcatString file_name(full_path);

file_name.chomp(".py");   //  remove possible ".py" suffix from script filename

bool use_xarray = false;

switch ( Type )  {   //  assumes Type is already set

   case FileType_Python_Xarray:
      use_xarray = true;
      break;

   case FileType_Python_Numpy:
      use_xarray = false;
      break;

   default:
      mlog << Error
           << "MetPythonDataFile::open(const char * script_filename) -> bad file type: "
           << grdfiletype_to_string(Type) << "\n\n";
      exit ( 1 );

}   //  switch

Filename = file_name;

Raw_Grid = new Grid;

bool status = python_dataplane(file_name.c_str(), sa,
                               use_xarray, Plane, *Raw_Grid, VInfo);

Dest_Grid = new Grid;

(*Dest_Grid) = (*Raw_Grid);

if ( ShiftRight != 0 )  Plane.shift_right(ShiftRight);

   //
   //  done
   //

return status;

}


////////////////////////////////////////////////////////////////////////


void MetPythonDataFile::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "File = ";

if ( Filename.empty() )  out << "(nul)\n";
else                     out << '\"' << Filename << "\"\n";

if ( Raw_Grid )  {

   out << prefix << "Grid:\n";

   Raw_Grid->dump(out, depth + 1);

} else {

   out << prefix << "No Grid!\n";

}

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


double MetPythonDataFile::get(int x, int y) const

{

double value = Plane.get(x, y);

return value;

}


////////////////////////////////////////////////////////////////////////


bool MetPythonDataFile::data_ok(int x, int y) const

{

const double value = get(x, y);

return !is_bad_data(value);

}


////////////////////////////////////////////////////////////////////////


void MetPythonDataFile::data_minmax(double & data_min, double & data_max) const

{

Plane.data_range(data_min, data_max);

return;

}


////////////////////////////////////////////////////////////////////////


bool MetPythonDataFile::data_plane(VarInfo &vinfo, DataPlane &plane,
                                   bool do_winds)

{

bool status = true;

   //
   //  the python command is specified by VarInfo::Name
   //  only open if the python command is empty or has changed
   //

if ( PythonCommand.empty() || PythonCommand != vinfo.req_name() ) {

   close();

   ConcatString cur_req_name = vinfo.req_name();

   //
   //  replace MET_PYTHON_INPUT_ARG constant with the current environment
   //  variable value
   //

   ConcatString cur_env_val;
   if ( get_env(met_python_input_arg, cur_env_val) )  {

      cur_req_name.replace(met_python_input_arg, cur_env_val.c_str(), false);

   }

   status = open(cur_req_name.c_str());

}

if ( status ) {

   plane = Plane;

   status = process_data_plane(&vinfo, plane, do_winds);

}

if ( !status )  return false;

   //
   //  ok
   //

   //
   //  store the VarInfo metadata without completely overwritting it
   //

vinfo.set_name(VInfo.name());
vinfo.set_long_name(VInfo.long_name().c_str());
vinfo.set_level_name(VInfo.level_name().c_str());
vinfo.set_units(VInfo.units().c_str());
vinfo.set_magic(VInfo.name(), VInfo.level_name());

return true;

}


////////////////////////////////////////////////////////////////////////


int MetPythonDataFile::data_plane_array(VarInfo &vinfo,
                                        DataPlaneArray &plane_array,
                                        bool do_winds)

{

bool status = true;
DataPlane plane;

   //
   //  the python command is specified by VarInfo::Name
   //  only open if the python command is empty or has changed
   //

if ( PythonCommand.empty() || PythonCommand != vinfo.req_name() ) {

   close();

   ConcatString cur_req_name = vinfo.req_name();

   //
   //  replace MET_PYTHON_INPUT_ARG constant with the current environment
   //  variable value
   //

   ConcatString cur_env_val;
   if ( get_env(met_python_input_arg, cur_env_val) )  {

      cur_req_name.replace(met_python_input_arg, cur_env_val.c_str(), false);

   }

   status = open(cur_req_name.c_str());

}

if ( status ) {

   plane = Plane;

   status = process_data_plane(&vinfo, plane, do_winds);

}

if ( !status )  return 0;

   //
   //  ok
   //

plane_array.clear();

plane_array.add(plane, 0.0, 0.0);

   //
   //  store the VarInfo metadata without completely overwritting it
   //

vinfo.set_name(VInfo.name());
vinfo.set_long_name(VInfo.long_name().c_str());
vinfo.set_level_name(VInfo.level_name().c_str());
vinfo.set_units(VInfo.units().c_str());
vinfo.set_magic(VInfo.name(), VInfo.level_name());

return 1;

}


////////////////////////////////////////////////////////////////////////


bool MetPythonDataFile::data_plane(DataPlane &plane)

{

   //
   //  is the file even open?
   //

if ( ! Raw_Grid )  return false;

   //
   //  ok
   //

plane = Plane;

return true;

}


////////////////////////////////////////////////////////////////////////


int MetPythonDataFile::index(VarInfo &vinfo)

{

   //
   //  is the file even open?
   //

if ( ! Raw_Grid )  return -1;

   //
   //  ok
   //

return 0;

}


////////////////////////////////////////////////////////////////////////
