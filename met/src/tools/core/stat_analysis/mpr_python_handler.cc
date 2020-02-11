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

#include "vx_python3_utils.h"
#include "mpr_python_handler.h"


////////////////////////////////////////////////////////////////////////


static const char generic_python_wrapper [] = "generic_python";
static const char generic_pickle_wrapper [] = "generic_pickle";

static const char write_pickle_wrapper   [] = "MET_BASE/wrappers/point_write_pickle.py";

static const char list_name              [] = "point_data";

static const char pickle_output_filename [] = "out.pickle";


////////////////////////////////////////////////////////////////////////


   //
   //     from parse_stat_line.h:
   //
   // // Matched Pair (MPR) data structure
   // struct MPRData {
   //    ConcatString fcst_var;
   //    ConcatString obs_var;
   //    int total, index;
   //    ConcatString obs_sid, obs_qc;
   //    double obs_lat, obs_lon, obs_lvl, obs_elv;
   //    double fcst, obs, climo_mean, climo_stdev, climo_cdf;
   // };
   //


static const int n_mpr = 15;   //  how many "things" in an mpr struct
                               //    hope I counted right


////////////////////////////////////////////////////////////////////////


static const char sq = '\'';   //  single quote


////////////////////////////////////////////////////////////////////////


static void get_cs     (PyObject * in, ConcatString & out);

static void get_int    (PyObject * in, int & out);

static void get_double (PyObject * in, double & out);

static bool is_na      (PyObject *);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


void mpr_populate(PyObject * list_in, MPRData & mpr_out)

{

   //
   //  make sure the given python object is a list
   //

if ( ! PyList_Check(list_in) )  {

   mlog << Error
        << "\n\n   mpr_populate(PyObject * list_in, MPRData & mpr_out) -> python object is not a list!\n\n";

   exit ( 1 );

}

   //
   //  make sure the list is the correct size
   //

const int N = (int) PyList_Size(list_in);

if ( N != n_mpr )  {

   mlog << Error
        << "\n\n   mpr_populate(PyObject * list_in, MPRData & mpr_out) -> bad list size ... (" << N << ")\n\n";

   exit ( 1 );

}

   //
   //  Grab the elements of the list one at a time
   //    and populate the MPRData struct
   //
   //  Note that we always have to be prepared
   //    for some element to be the "NA" string,
   //    even when we were expecting some other
   //    data type such as int or double
   //

int j = 0;


get_cs     (PyList_GetItem(list_in, j++), mpr_out.fcst_var);
get_cs     (PyList_GetItem(list_in, j++), mpr_out.obs_var);

get_int    (PyList_GetItem(list_in, j++), mpr_out.total);
get_int    (PyList_GetItem(list_in, j++), mpr_out.index);
   
get_cs     (PyList_GetItem(list_in, j++), mpr_out.obs_sid);
get_cs     (PyList_GetItem(list_in, j++), mpr_out.obs_qc);

get_double (PyList_GetItem(list_in, j++), mpr_out.obs_lat);
get_double (PyList_GetItem(list_in, j++), mpr_out.obs_lon);
get_double (PyList_GetItem(list_in, j++), mpr_out.obs_lvl);
get_double (PyList_GetItem(list_in, j++), mpr_out.obs_elv);

get_double (PyList_GetItem(list_in, j++), mpr_out.fcst);
get_double (PyList_GetItem(list_in, j++), mpr_out.obs);
get_double (PyList_GetItem(list_in, j++), mpr_out.climo_mean);
get_double (PyList_GetItem(list_in, j++), mpr_out.climo_stdev);
get_double (PyList_GetItem(list_in, j++), mpr_out.climo_cdf);



   //
   //  done
   //

return;

}


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
   //  check that is a string and that the string contents are "NA"
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




