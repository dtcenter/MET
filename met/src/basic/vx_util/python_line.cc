// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "vx_log.h"
#include "vx_math.h"
#include "stat_column_defs.h"
#include "util_constants.h"
#include "vx_python3_utils.h"
#include "python3_util.h"
#include "global_python.h"

#include "python_line.h"


////////////////////////////////////////////////////////////////////////


   //
   //  user-supplied columns are
   //
   //     "FCST_VALID"
   //      "OBS_VALID"
   //
   //  plus the mpr_columns from stat_columns.h
   //


////////////////////////////////////////////////////////////////////////


static const char generic_python_wrapper [] = "generic_python";
static const char generic_pickle_wrapper [] = "generic_pickle";

static const char write_pickle_wrapper   [] = "MET_BASE/wrappers/point_write_pickle.py";

static const char list_name              [] = "point_data";

static const char pickle_output_filename [] = "out.pickle";


////////////////////////////////////////////////////////////////////////


static void get_cs     (PyObject * in, ConcatString & out);

static void get_int    (PyObject * in, int & out);

static void get_double (PyObject * in, double & out);

static bool is_na      (PyObject *);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class PyLineDataFile
   //


////////////////////////////////////////////////////////////////////////


PyLineDataFile::PyLineDataFile()

{

close();

}


////////////////////////////////////////////////////////////////////////


PyLineDataFile::~PyLineDataFile()

{

close();

}


////////////////////////////////////////////////////////////////////////


void PyLineDataFile::close()

{

first_call = false;

index = -1;

N = -1;

main_list = 0;   //  don't delete

UserScriptPath.clear();


return;

}


////////////////////////////////////////////////////////////////////////


bool PyLineDataFile::open(const char * user_script_path)

{

close();

first_call = true;

UserScriptPath = user_script_path;

if ( getenv (user_python_path_env) )  do_pickle   ();
else                                  do_straight ();

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


ConcatString PyLineDataFile::make_header_line() const

{

int j;
bool last = false;
ConcatString line;

line << "VERSION"    << ' ';
line << "FCST_VALID" << ' ';
line << "OBS_VALID"  << ' ';

for (j=0; j<n_mpr_columns; ++j)  {

   last = (j == (n_mpr_columns - 1));

   line << mpr_columns[j];

   if ( ! last )  line << ' ';

}



   //
   //  done
   //

return ( line );

}


////////////////////////////////////////////////////////////////////////


ConcatString PyLineDataFile::make_data_line()

{

int j, k;
double v;
ConcatString line;
ConcatString a;
PyObject * sublist = 0;


sublist = PyList_GetItem(main_list, index);

if ( ! PyList_Check(sublist) )  {

   mlog << Error
        << "\n\n  PyLineDataFile::make_data_line() -> python object is not a list!\n\n";

   exit ( 1 );

}

j = 0;   //  index into the sublist from the user

   //
   //  version
   //

line << met_version;

   //
   //  fcst and obs valid time
   //

get_cs     (PyList_GetItem(sublist, j++), a);   //  fcst valid

line << ' ' << a;

get_cs     (PyList_GetItem(sublist, j++), a);   //  obs valid

line << ' ' << a;

   //
   //  total
   //

get_int    (PyList_GetItem(sublist, j++), k);

line << ' ' << k;
 
   //
   //  index
   //

get_int    (PyList_GetItem(sublist, j++), k);

line << ' ' << k;

   //
   //  obs_sid
   //

get_cs     (PyList_GetItem(sublist, j++), a);

line << ' ' << a;

   //
   //  obs_lat
   //

get_double (PyList_GetItem(sublist, j++), v);

line << ' ' << v;

   //
   //  obs_lon
   //

get_double (PyList_GetItem(sublist, j++), v);

line << ' ' << v;

   //
   //  obs_lvl
   //

get_double (PyList_GetItem(sublist, j++), v);

line << ' ' << v;

   //
   //  obs_elev
   //

get_double (PyList_GetItem(sublist, j++), v);

line << ' ' << v;

   //
   //  fcst
   //

get_double (PyList_GetItem(sublist, j++), v);

line << ' ' << v;

   //
   //  obs
   //

get_double (PyList_GetItem(sublist, j++), v);

line << ' ' << v;

   //
   //  obs_qc
   //

get_cs     (PyList_GetItem(sublist, j++), a);

line << ' ' << a;

   //
   //  climo_mean
   //

get_double (PyList_GetItem(sublist, j++), v);

line << ' ' << v;

   //
   //  climo_stdev
   //

get_double (PyList_GetItem(sublist, j++), v);

line << ' ' << v;

   //
   //  climo_cdf
   //

get_double (PyList_GetItem(sublist, j++), v);

line << ' ' << v;




   //
   //  done
   //

return ( line );

}


////////////////////////////////////////////////////////////////////////


void PyLineDataFile::do_straight()

{

cerr << "\n\n  PyLineDataFile::do_straight() -> not yet implemented!\n\n" << flush;

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void PyLineDataFile::do_pickle()

{

cerr << "\n\n  PyLineDataFile::do_pickle() -> not yet implemented!\n\n" << flush;

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


void get_cs(PyObject * in, ConcatString & out)

{

   //
   //  ok if the string is "NA", no separate check needed
   //

out = pyobject_as_concat_string (in);

return;

}


////////////////////////////////////////////////////////////////////////


void get_int    (PyObject * in, int & out)

{

if ( is_na(in) )  {

   out = bad_data_int;

   return;

}

out = pyobject_as_int(in);

return;

}


////////////////////////////////////////////////////////////////////////


void get_double (PyObject * in, double & out)

{

if ( is_na(in) )  {

   out = bad_data_double;

   return;

}

out = pyobject_as_double(in);


return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  check that it's a string and that the string contents are "NA"
   //


bool is_na (PyObject * obj)

{

if ( ! PyUnicode_Check(obj) )  return ( false );

   //
   //  now we know it's a string, the value had better be "NA"
   //

ConcatString s = pyobject_as_concat_string(obj);

if ( strcmp(s.text(), na_string.c_str()) == 0 )  return ( true );

   //
   //  if it's a string but not NA, then error out
   //

mlog << Error
     << "\n\n  is_na (PyObject *) -> bad string value ... \"" << s.text() << "\"\n\n";

exit ( 1 );

   //
   //  done
   //

return ( false );   //  control flow should never get here

}


////////////////////////////////////////////////////////////////////////



