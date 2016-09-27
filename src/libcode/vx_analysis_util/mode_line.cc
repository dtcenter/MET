// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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
#include "mode_analysis_columns.h"

#include "vx_math.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ModeLine
   //


////////////////////////////////////////////////////////////////////////


ModeLine::ModeLine()

{


}


////////////////////////////////////////////////////////////////////////


ModeLine::~ModeLine()

{


}


////////////////////////////////////////////////////////////////////////


ModeLine::ModeLine(const ModeLine & L)

{

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


void ModeLine::dump(ostream & out, int depth) const

{

Indent prefix(depth);
bool status = false;
int nf, no;

out << prefix << "ModeLine stuff ...\n";
out << prefix << "\n";

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

status = DataLine::read_line(ldf);

if ( !status )  {

   clear();

   return ( 0 );

}

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::version() const

{

const char * c = (const char *) 0;

c = get_item(version_column);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::model() const

{

const char * c = (const char *) 0;

c = get_item(model_column);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::desc() const

{

const char * c = (const char *) 0;

c = get_item(desc_column);

return ( c );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::fcst_lead() const

{

int s;
const char * c = (const char *) 0;

c = get_item(fcst_lead_column);

s = timestring_to_sec(c);

return ( s );

}


////////////////////////////////////////////////////////////////////////


unixtime ModeLine::fcst_valid() const

{

unixtime t;
const char * c = (const char *) 0;

c = get_item(fcst_valid_column);

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
const char * c = (const char *) 0;

c = get_item(fcst_valid_column);

v = timestring_to_unix(c);

c = get_item(fcst_lead_column);

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
const char * c = (const char *) 0;

c = get_item(fcst_accum_column);

s = timestring_to_sec(c);

return ( s );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::obs_lead() const

{

int s;
const char * c = (const char *) 0;

c = get_item(obs_lead_column);

s = timestring_to_sec(c);

return ( s );

}


////////////////////////////////////////////////////////////////////////


unixtime ModeLine::obs_valid() const

{

unixtime t;
const char * c = (const char *) 0;

c = get_item(obs_valid_column);

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
const char * c = (const char *) 0;

c = get_item(obs_valid_column);

v = timestring_to_unix(c);

c = get_item(obs_lead_column);

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
const char * c = (const char *) 0;

c = get_item(obs_accum_column);

s = timestring_to_sec(c);

return ( s );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::fcst_rad() const

{

int i;
const char * c = (const char *) 0;

c = get_item(fcst_rad_column);

i = atoi(c);

return ( i );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::fcst_thr() const

{

const char * c = (const char *) 0;

c = get_item(fcst_thr_column);

return ( c );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::obs_rad() const

{

int i;
const char * c = (const char *) 0;

c = get_item(obs_rad_column);

i = atoi(c);

return ( i );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::obs_thr() const

{

const char * c = (const char *) 0;

c = get_item(obs_thr_column);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::fcst_var() const

{

const char * c = (const char *) 0;

c = get_item(fcst_var_column);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::fcst_lev() const

{

const char * c = (const char *) 0;

c = get_item(fcst_lev_column);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::obs_var() const

{

const char * c = (const char *) 0;

c = get_item(obs_var_column);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::obs_lev() const

{

const char * c = (const char *) 0;

c = get_item(obs_lev_column);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::object_id() const

{

const char * c = (const char *) 0;

c = get_item(object_id_column);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::object_cat() const

{

const char * c = (const char *) 0;

c = get_item(object_cat_column);

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

const char * c = (const char *) 0;

c = object_id();

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

const char * c = (const char *) 0;

c = object_id();

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

const char * c = (const char *) 0;

c = object_id();

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

const char * c = (const char *) 0;

c = object_id();

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
const char * c = (const char *) 0;

c = object_cat();

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
const char * c = (const char *) 0;

c = object_id();

k = atoi(c + 1);   //  skip leading 'F' or 'O'

--k;  //  object numbers start at zero

return ( k );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::cluster_obj_number() const

{

if ( is_pair() )  return ( -1 );

int k;
const char * c = (const char *) 0;

c = object_cat();

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
const char * c = (const char *) 0;

c = get_item(centroid_x_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::centroid_y() const

{

double x;
const char * c = (const char *) 0;

c = get_item(centroid_y_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::centroid_lat() const

{

double x;
const char * c = (const char *) 0;

c = get_item(centroid_lat_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::centroid_lon() const

{

double x;
const char * c = (const char *) 0;

c = get_item(centroid_lon_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::axis_ang() const

{

double x;
const char * c = (const char *) 0;

c = get_item(axis_ang_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::length() const

{

double x;
const char * c = (const char *) 0;

c = get_item(length_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::width() const

{

double x;
const char * c = (const char *) 0;

c = get_item(width_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::aspect_ratio() const

{

double x;
const char * l = (const char *) 0;
const char * w = (const char *) 0;

l = get_item(length_column);
w = get_item(width_column);

x = atof(w)/atof(l);

return ( x );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::area() const

{

int a;
const char * c = (const char *) 0;

c = get_item(area_column);

a = atoi(c);

return ( a );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::area_filter() const

{

int a;
const char * c = (const char *) 0;

c = get_item(area_filter_column);

a = atoi(c);

return ( a );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::area_thresh() const

{

int a;
const char * c = (const char *) 0;

c = get_item(area_thresh_column);

a = atoi(c);

return ( a );

}

////////////////////////////////////////////////////////////////////////


double ModeLine::curvature() const

{

double x;
const char * c = (const char *) 0;

c = get_item(curvature_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::curvature_x() const

{

double x;
const char * c = (const char *) 0;

c = get_item(curvature_x_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::curvature_y() const

{

double x;
const char * c = (const char *) 0;

c = get_item(curvature_y_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::complexity() const

{

double x;
const char * c = (const char *) 0;

c = get_item(complexity_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::intensity_10() const

{

double x;
const char * c = (const char *) 0;

c = get_item(intensity_10_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::intensity_25() const

{

double x;
const char * c = (const char *) 0;

c = get_item(intensity_25_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::intensity_50() const

{

double x;
const char * c = (const char *) 0;

c = get_item(intensity_50_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::intensity_75() const

{

double x;
const char * c = (const char *) 0;

c = get_item(intensity_75_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::intensity_90() const

{

double x;
const char * c = (const char *) 0;

c = get_item(intensity_90_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::intensity_user() const

{

double x;
const char * c = (const char *) 0;

c = get_item(intensity_user_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::intensity_sum() const

{

double x;
const char * c = (const char *) 0;

c = get_item(intensity_sum_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::centroid_dist() const

{

double x;
const char * c = (const char *) 0;

c = get_item(centroid_dist_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::boundary_dist() const

{

double x;
const char * c = (const char *) 0;

c = get_item(boundary_dist_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::convex_hull_dist() const

{

double x;
const char * c = (const char *) 0;

c = get_item(convex_hull_dist_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::angle_diff() const

{

double x;
const char * c = (const char *) 0;

c = get_item(angle_diff_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::area_ratio() const

{

double x;
const char * c = (const char *) 0;

c = get_item(area_ratio_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::intersection_area() const

{

int i;
const char * c = (const char *) 0;

c = get_item(intersection_area_column);

i = atoi(c);

return ( i );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::union_area() const

{

int i;
const char * c = (const char *) 0;

c = get_item(union_area_column);

i = atoi(c);

return ( i );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::symmetric_diff() const

{

int i;
const char * c = (const char *) 0;

c = get_item(symmetric_diff_column);

i = atoi(c);

return ( i );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::intersection_over_area() const

{

double x;
const char * c = (const char *) 0;

c = get_item(intersection_over_area_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::complexity_ratio() const

{

double x;
const char * c = (const char *) 0;

c = get_item(complexity_ratio_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::percentile_intensity_ratio() const

{

double x;
const char * c = (const char *) 0;

c = get_item(percentile_intensity_ratio_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


double ModeLine::interest() const

{

double x;
const char * c = (const char *) 0;

c = get_item(interest_column);

x = atof(c);

return ( x );

}


////////////////////////////////////////////////////////////////////////


int ModeLine::is_ok() const

{

if ( !(DataLine::is_ok()) )  return ( 0 );

const char * c = get_item(0);

if ( strcmp(c, "VERSION") == 0 )  return ( 0 );

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


const char * ModeLine::get_item(int k) const

{

const char * c = DataLine::get_item(k);

   //
   // Check for the NA string and interpret it as bad data
   //

if ( strcmp(c, na_str) == 0 ) return ( bad_data_str );
else                          return ( c );

}


////////////////////////////////////////////////////////////////////////

