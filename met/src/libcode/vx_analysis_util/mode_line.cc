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
#include <fstream>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <cstdio>
#include <cmath>

#include "mode_line.h"
#include "analysis_utils.h"

#include "vx_math.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ModeLine
   //


////////////////////////////////////////////////////////////////////////


ModeLine::ModeLine()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ModeLine::~ModeLine()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ModeLine::ModeLine(const ModeLine & L)

{

init_from_scratch();

assign(L);

return;

}


////////////////////////////////////////////////////////////////////////


ModeLine & ModeLine::operator=(const ModeLine & L)

{

if ( this == &L )  return ( * this );

assign(L);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void ModeLine::init_from_scratch()

{

DataLine::init_from_scratch();

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void ModeLine::assign(const ModeLine &l)

{

DataLine::assign(l);

HdrFlag = l.HdrFlag;
HdrLine = l.HdrLine;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeLine::clear()

{

DataLine::clear();

HdrFlag = false;
HdrLine = (AsciiHeaderLine *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeLine::dump(ostream & out, int depth) const

{

Indent prefix(depth);
bool status = false;
int nf, no;

out << prefix << "ModeLine stuff ...\n";
out << prefix << "\n";

out << prefix << "header                  = " << HdrLine << "\n";

out << prefix << "is_fcst                 = " << (is_fcst () ? "True" : "False") << "\n";
out << prefix << "is_obs                  = " << (is_obs  () ? "True" : "False") << "\n";
out << prefix << "\n";

out << prefix << "is_single               = " << (is_single () ? "True" : "False") << "\n";
out << prefix << "is_pair                 = " << (is_pair   () ? "True" : "False") << "\n";
out << prefix << "\n";

out << prefix << "is_simple               = " << (is_simple    () ? "True" : "False") << "\n";
out << prefix << "is_cluster              = " << (is_cluster   () ? "True" : "False") << "\n";
out << prefix << "\n";

out << prefix << "is_matched              = " << (is_matched   () ? "True" : "False") << "\n";
out << prefix << "is_unmatched            = " << (is_unmatched () ? "True" : "False") << "\n";
out << prefix << "\n";

out << prefix << "simple_obj_number       = " << simple_obj_number    () << "\n";
out << prefix << "cluster_obj_number      = " << cluster_obj_number () << "\n";
out << prefix << "\n";

status = pair_obj_numbers(nf, no);

out << prefix << "pair_obj_numbers        = (" << nf << ", " << no << ")\n";
out << prefix << "pair_obj_numbers_status = " << (status ? "True" : "False") << "\n";
out << prefix << "\n";

out << prefix << "DataLine stuff ...\n";
out << prefix << "\n";

DataLine::dump(out, depth);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


int ModeLine::read_line(LineDataFile * ldf)

{

int status;

clear();

status = DataLine::read_line(ldf);

if ( !status || n_items() == 0 )  {

   clear();

   return ( 0 );

}

//
// Check for a header line
//

if ( strcmp(get_item(0), "VERSION") == 0 ) {

   HdrFlag = true;

   return ( 1 );
}

//
// Load the matching header line
//

HdrLine = METHdrTable.header(get_item(0), "MODE", "OBJ");

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


bool ModeLine::is_ok() const

{

return ( DataLine::is_ok() );

}


////////////////////////////////////////////////////////////////////////


bool ModeLine::is_header() const

{

return ( HdrFlag );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::get_item(const char *col_str, bool check_na) const

{

int offset = bad_data_int;

   //
   // Search for matching header column
   //

offset = HdrLine->col_offset(col_str, 0);

   //
   // If not found, check extra header columns
   //

if ( is_bad_data(offset) ) {
   if ( !get_file()->header().has(col_str, offset) ) offset = bad_data_int;
}

   //
   // Return bad data string for no match
   //

if ( is_bad_data(offset) ) return ( bad_data_str );
else                       return ( get_item(offset, check_na) );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::get_item(int offset, bool check_na) const

{

   //
   // Range check
   //

if ( offset < 0 || offset >= N_items ) return ( bad_data_str );

const char * c = DataLine::get_item(offset);

   //
   // Check for the NA string and interpret it as bad data
   //

if ( check_na && strcmp(c, na_str) == 0 ) return ( bad_data_str );
else                                      return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::version() const

{

const char * c = get_item("VERSION", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::model() const

{

const char * c = get_item("MODEL", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::desc() const

{

const char * c = get_item("DESC", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::fcst_lead() const

{

int s;

const char * c = get_item("FCST_LEAD");

s = timestring_to_sec(c);

return ( s );

}


////////////////////////////////////////////////////////////////////////


unixtime ModeLine::fcst_valid() const

{

unixtime t;

const char * c = get_item("FCST_VALID");

t = timestring_to_unix(c);

return ( t );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::fcst_valid_hour() const

{

return ( unix_to_sec_of_day(fcst_valid()) );

}


////////////////////////////////////////////////////////////////////////


unixtime ModeLine::fcst_init() const

{

unixtime v, l;

const char * c = (char *) 0;

c = get_item("FCST_VALID");

v = timestring_to_unix(c);

c = get_item("FCST_LEAD");

l = timestring_to_sec(c);

return ( v - l );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::fcst_init_hour() const

{

return ( unix_to_sec_of_day(fcst_init()) );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::fcst_accum() const

{

int s;

const char * c = get_item("FCST_ACCUM");

s = timestring_to_sec(c);

return ( s );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::obs_lead() const

{

int s;

const char * c = get_item("OBS_LEAD");

s = timestring_to_sec(c);

return ( s );

}


////////////////////////////////////////////////////////////////////////


unixtime ModeLine::obs_valid() const

{

unixtime t;

const char * c = get_item("OBS_VALID");

t = timestring_to_unix(c);

return ( t );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::obs_valid_hour() const

{

return ( unix_to_sec_of_day(obs_valid()) );

}


////////////////////////////////////////////////////////////////////////


unixtime ModeLine::obs_init() const

{

unixtime v, l;

const char * c = (char *) 0;

c = get_item("OBS_VALID");

v = timestring_to_unix(c);

c = get_item("OBS_LEAD");

l = timestring_to_sec(c);

return ( v - l );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::obs_init_hour() const

{

return ( unix_to_sec_of_day(obs_init()) );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::obs_accum() const

{

int s;

const char * c = get_item("OBS_ACCUM");

s = timestring_to_sec(c);

return ( s );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::fcst_rad() const

{

int i;

const char * c = get_item("FCST_RAD");

i = atoi(c);

return ( i );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::fcst_thr() const

{

const char * c = get_item("FCST_THR", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::obs_rad() const

{

int i;

const char * c = get_item("OBS_RAD");

i = atoi(c);

return ( i );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::obs_thr() const

{

const char * c = get_item("OBS_THR", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::fcst_var() const

{

const char * c = get_item("FCST_VAR", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::fcst_units() const

{

const char * c = get_item("FCST_UNITS", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::fcst_lev() const

{

const char * c = get_item("FCST_LEV", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::obs_var() const

{

const char * c = get_item("OBS_VAR", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::obs_units() const

{

const char * c = get_item("OBS_UNITS", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::obs_lev() const

{

const char * c = get_item("OBS_LEV", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::object_id() const

{



const char * c = get_item("OBJECT_ID", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::object_cat() const

{



const char * c = get_item("OBJECT_CAT", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::is_fcst() const

{

   //
   //  has to be a single object, not a pair
   //

if ( is_pair() )  return ( 0 );

   //
   //  look for an "F" or "CF" at the start of the OBJECT_ID field
   //



const char * c = object_id();

if ( c[0] == 'F' )  return ( 1 );

if ( strncmp(c, "CF", 2) == 0 )  return ( 1 );

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::is_obs() const

{

   //
   //  has to be a single object, not a pair
   //

if ( is_pair() )  return ( 0 );

   //
   //  look for an "O" or "CO" at the start of the OBJECT_ID field
   //



const char * c = object_id();

if ( c[0] == 'O' )  return ( 1 );

if ( strncmp(c, "CO", 2) == 0 )  return ( 1 );

   //
   //  nope
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::is_single() const

{

   //
   //  look for an underscore in the OBJECT_ID value
   //



const char * c = object_id();

if ( strchr(c, '_') )  return ( 0 );

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::is_pair() const

{

int status;

status = is_single();

return ( !status );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::is_simple() const

{

   //
   //  look for a 'C' in the first character of the OBJECT_ID
   //



const char * c = object_id();

if ( c[0] == 'C' )  return ( 0 );

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::is_cluster() const

{

int status;

status = is_simple();

return ( !status );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::is_matched() const

{

   //
   //  has to be a single object (not a pair)
   //

if ( is_pair() )  return ( 0 );

   //
   //  look for a number > 0 in the OBJECT_CAT field
   //

int k;


const char * c = object_cat();

k = atoi(c + 2);  //  skip the leading "CF" or "CO" in the OBJECT_CAT field

if ( k > 0 )  return ( 1 );

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::is_unmatched() const

{

int status;

status = is_matched();

return ( !status );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::simple_obj_number() const

{

if ( !is_single() )  return ( -1 );
if ( !is_simple() )  return ( -1 );

int k;


const char * c = object_id();

k = atoi(c + 1);   //  skip leading 'F' or 'O'

--k;  //  object numbers start at zero

return ( k );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::cluster_obj_number() const

{

if ( is_pair() )  return ( -1 );

int k;


const char * c = object_cat();

k = atoi(c + 2);   //  skip leading "CF" or "CO"

--k;  // object numbers start at zero

return ( k );

}


////////////////////////////////////////////////////////////////////////


bool ModeLine::pair_obj_numbers(int & nfcst, int & nobs) const

{

nfcst = nobs = -1;

if ( is_single() )  return ( false );

const char * c = object_id();
const char * u = (const char *) 0;

   //
   //  skip leading F, O, CF or CO
   //

if ( c[0] == 'C' )  nfcst = atoi(c + 2);
else                nfcst = atoi(c + 1);

--nfcst;   //  object numbers start at zero

u = strchr(c, '_');

if ( !u )  {

   mlog << Error << "\nModeLine::pair_obj_numbers() const -> "
        << "underscore char not found in object id: \"" << c << "\"\n\n";

   exit ( 1 );

}

++u;  //  skip underscore

   //
   //  again, skip leading F, O, CF or CO
   //

if ( u[0] == 'C' )  nobs = atoi(u + 2);
else                nobs = atoi(u + 1);

--nobs;   //  object numbers start at zero

   //
   //  ok
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::centroid_x() const

{

double x;

const char * c = get_item("CENTROID_X");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::centroid_y() const

{

double x;

const char * c = get_item("CENTROID_Y");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::centroid_lat() const

{

double x;

const char * c = get_item("CENTROID_LAT");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::centroid_lon() const

{

double x;

const char * c = get_item("CENTROID_LON");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::axis_ang() const

{

double x;

const char * c = get_item("AXIS_ANG");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::length() const

{

double x;

const char * c = get_item("LENGTH");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::width() const

{

double x;

const char * c = get_item("WIDTH");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::aspect_ratio() const

{

double x;

const char * l = get_item("LENGTH");
const char * w = get_item("WIDTH");

x = atof(w)/atof(l);

return ( x );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::area() const

{

int a;

const char * c = get_item("AREA");

a = atoi(c);

return ( a );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::area_thresh() const

{

int a;

const char * c = get_item("AREA_THRESH");

a = atoi(c);

return ( a );

}

////////////////////////////////////////////////////////////////////////


double ModeLine::curvature() const

{

double x;

const char * c = get_item("CURVATURE");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::curvature_x() const

{

double x;

const char * c = get_item("CURVATURE_X");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::curvature_y() const

{

double x;

const char * c = get_item("CURVATURE_Y");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::complexity() const

{

double x;

const char * c = get_item("COMPLEXITY");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::intensity_10() const

{

double x;

const char * c = get_item("INTENSITY_10");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::intensity_25() const

{

double x;

const char * c = get_item("INTENSITY_25");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::intensity_50() const

{

double x;

const char * c = get_item("INTENSITY_50");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::intensity_75() const

{

double x;

const char * c = get_item("INTENSITY_75");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::intensity_90() const

{

double x;

const char * c = get_item("INTENSITY_90");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::intensity_user() const

{

int i;
double x;

   //
   // Get the INTENSITY_USER column immediately after INTENSITY_90
   //

i = HdrLine->col_offset("INTENSITY_90", 0);
const char * c = get_item(i+1);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::intensity_sum() const

{

double x;

const char * c = get_item("INTENSITY_SUM");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::centroid_dist() const

{

double x;

const char * c = get_item("CENTROID_DIST");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::boundary_dist() const

{

double x;

const char * c = get_item("BOUNDARY_DIST");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::convex_hull_dist() const

{

double x;

const char * c = get_item("CONVEX_HULL_DIST");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::angle_diff() const

{

double x;

const char * c = get_item("ANGLE_DIFF");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::aspect_diff() const

{

double x;

const char * c = get_item("ASPECT_DIFF");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::area_ratio() const

{

double x;

const char * c = get_item("AREA_RATIO");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::intersection_area() const

{

int i;

const char * c = get_item("INTERSECTION_AREA");

i = atoi(c);

return ( i );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::union_area() const

{

int i;

const char * c = get_item("UNION_AREA");

i = atoi(c);

return ( i );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::symmetric_diff() const

{

int i;

const char * c = get_item("SYMMETRIC_DIFF");

i = atoi(c);

return ( i );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::intersection_over_area() const

{

double x;

const char * c = get_item("INTERSECTION_OVER_AREA");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::curvature_ratio() const

{

double x;

const char * c = get_item("CURVATURE_RATIO");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::complexity_ratio() const

{

double x;

const char * c = get_item("COMPLEXITY_RATIO");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::percentile_intensity_ratio() const

{

double x;

const char * c = get_item("PERCENTILE_INTENSITY_RATIO");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::interest() const

{

double x;

const char * c = get_item("INTEREST");

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////
