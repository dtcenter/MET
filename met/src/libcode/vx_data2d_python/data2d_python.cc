// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "data2d_python.h"
#include "vx_python_utils.h"
#include "data2d_utils.h"
#include "grdfiletype_to_string.h"

#include "vx_math.h"
#include "vx_log.h"


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

// python_init_from_scratch();
//
// assign(f);

}


////////////////////////////////////////////////////////////////////////


MetPythonDataFile & MetPythonDataFile::operator=(const MetPythonDataFile &)

{

mlog << Error << "\nMetPythonDataFile::operator=(const MetPythonDataFile &) -> "
     << "should never be called!\n\n";

exit ( 1 );

// if ( this == &f )  return ( * this );
//
// assign(f);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void MetPythonDataFile::python_init_from_scratch()

{

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


bool MetPythonDataFile::open(const char * script_filename)

{

close();

ConcatString full_path, path_name, file_name;

full_path = script_filename;
StringArray sa = full_path.split("/");

   //
   //  Build the path and store the file name
   //

if ( sa.n_elements() <= 1 )  {
   path_name = "./";
}
else {
   for ( int i=0; i<sa.n_elements()-1; i++ )  path_name << "/" << sa[i];
}

file_name = sa[sa.n_elements() - 1];

   //
   //  Set the PYTHONPATH
   //

setenv("PYTHONPATH", path_name, 1);

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
      break;

}   //  switch




Filename = file_name;

Raw_Grid = new Grid;


python_dataplane(file_name, use_xarray, Plane, *Raw_Grid, VInfo);

Dest_Grid = new Grid;

(*Dest_Grid) = (*Raw_Grid);

if ( ShiftRight != 0 )  Plane.shift_right(ShiftRight);


   //
   //  done
   //

return ( true );

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

return ( value );

}


////////////////////////////////////////////////////////////////////////


bool MetPythonDataFile::data_ok(int x, int y) const

{

const double value = get(x, y);

return ( !is_bad_data(value) );

}


////////////////////////////////////////////////////////////////////////


void MetPythonDataFile::data_minmax(double & data_min, double & data_max) const

{

Plane.data_range(data_min, data_max);

return;

}


////////////////////////////////////////////////////////////////////////


bool MetPythonDataFile::data_plane(VarInfo &vinfo, DataPlane &plane)

{

   //
   //  is the file even open?
   //

if ( ! Raw_Grid )  return ( false );

   //
   //  ok
   //

plane = Plane;

vinfo = VInfo;

return ( true );

}


////////////////////////////////////////////////////////////////////////


int MetPythonDataFile::data_plane_array(VarInfo &vinfo, DataPlaneArray &plane_array)

{

   //
   //  is the file even open?
   //

if ( ! Raw_Grid )  return ( false );

   //
   //  ok
   //

plane_array.clear();

plane_array.add(Plane, 0.0, 0.0);

vinfo = VInfo;

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool MetPythonDataFile::data_plane(DataPlane &plane)

{

   //
   //  is the file even open?
   //

if ( ! Raw_Grid )  return ( false );

   //
   //  ok
   //

plane = Plane;

return ( true );

}


////////////////////////////////////////////////////////////////////////


int MetPythonDataFile::index(VarInfo &vinfo)

{

   //
   //  is the file even open?
   //

if ( ! Raw_Grid )  return ( -1 );

   //
   //  ok
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////



