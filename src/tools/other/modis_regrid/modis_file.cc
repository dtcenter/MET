

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_util.h"
#include "vx_math.h"

#include "modis_file.h"
#include "sat_utils.h"


////////////////////////////////////////////////////////////////////////


static const char dim0_name [] = "Cell_Along_Swath_5km";
static const char dim1_name [] = "Cell_Across_Swath_5km";

static int32 edge_2[2] = { 1, 1 };   //  can't declare this "const" or SWreadfield will complain

static const bool do_shuffle = false;

static const int buf_size = 4096;

static unsigned char buf[buf_size];

static const int max_dims = 100;

static const unixtime ut_modis_start = mdyhms_to_unix(1, 1, 1993, 0, 0, 0);


////////////////////////////////////////////////////////////////////////


static void clear_buf();


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ModisFile
   //


////////////////////////////////////////////////////////////////////////


ModisFile::ModisFile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ModisFile::~ModisFile()

{

close();

}


////////////////////////////////////////////////////////////////////////


ModisFile::ModisFile(const ModisFile &)

{

// init_from_scratch();
// 
// assign(f);

cerr << "\n\n  ModisFile::ModisFile(const ModisFile &) -> should never be called!\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


ModisFile & ModisFile::operator=(const ModisFile &)

{

// if ( this == &f )  return ( * this );
// 
// assign(f);


cerr << "\n\n  ModisFile::operator=(const ModisFile &) -> should never be called!\n\n";

exit ( 1 );

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void ModisFile::init_from_scratch()

{

FileId = -1;

Swath  = 0;

Latitude = 0;

Longitude = 0;

SurfaceTemperature = 0;

CloudFraction = 0;


close();

return;

}


////////////////////////////////////////////////////////////////////////


void ModisFile::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Filename = ";

if ( Filename.empty() )  out << "(nul)\n";
else {

   out << '\"' << short_name() << "\"\n";

}

out << prefix << "\n";

Swath->dump(out, depth + 1);

out << prefix << "Dim0 = " << Dim0 << "\n";
out << prefix << "Dim1 = " << Dim1 << "\n";


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


bool ModisFile::open(const char * _filename)

{

int k;
int n_swaths;
int32 size;
int n0, n1;
double dt;
StringArray a;
SwathDataField * sst = 0;   //  scan start time


close();

Filename = _filename;

   //
   //  open file
   //

if ( (FileId = SWopen((char *) _filename, DFACC_READ)) < 0 )  {

   cerr << "\n\n  ModisFile::open(const char *) -> unable to open input file \"" << _filename << "\"\n\n";

   close();

   return ( false );

}

   //
   //  get swath
   //

clear_buf();

if ( SWinqswath((char *) _filename, (char *) buf, &size) < 0 )  {

   cerr << "\n\n  ModisFile::open(const char *) -> unable to get number of swaths from file \"" << _filename << "\"\n\n";

   close();

   return ( false );

}

parse_csl(buf, a);

n_swaths = a.n_elements();

if ( n_swaths > 1 )  {

   cerr << "\n\n  ModisFile::open(const char *) -> too many swaths ("
        << n_swaths << ") in file \"" << _filename << "\"\n\n";

   close();

   return ( false );

}

Swath = new CloudsatSwath;

Swath->set_name(a[0]);

if ( (k = SWattach(FileId, (char *) (a[0]))) < 0 )  {

   cerr << "\n\n  ModisFile::open(const char *) -> unable to attach swath in file \"" << _filename << "\"\n\n";

   close();

   return ( false );

}

Swath->set_swath_id(k);

Swath->get_dimensions();

Swath->get_data_fields();

Swath->get_attributes();

Swath->get_geo_fields();

SatDimension * dim = 0;

dim = Swath->dimension(dim0_name);

if ( ! dim )  {

   cerr << "\n\n  ModisFile::open(const char *) -> can't get dimension \"" << dim0_name << "\"\n\n";

   close();

   return ( false );

}

Dim0 = dim->size();

dim = Swath->dimension(dim1_name);

if ( ! dim )  {

   cerr << "\n\n  ModisFile::open(const char *) -> can't get dimension \"" << dim1_name << "\"\n\n";

   close();

   return ( false );

}

Dim1 = dim->size();

   //
   //  get the geolocation fields
   //

get_geo_field(Latitude,  "Latitude");
get_geo_field(Longitude, "Longitude");

   //
   //  get the data fields
   //

get_data_field(SurfaceTemperature, "Surface_Temperature");
get_data_field(CloudFraction,      "Cloud_Fraction");

   //
   //  get the scan start time
   //

get_data_field(sst, "Scan_Start_Time");

n0 = 0;
n1 = 0;

dt = get_double_data(sst, n0, n1);

if ( dt < 0.0 )  {

   cerr << "\n\n  ModisFile::open(const char *) -> bad scan start time ("
        << dt << ") in file \"" << _filename << "\"\n\n";

   close();

   return ( false );

}

ScanStartTime = ut_modis_start + nint(dt);


   //
   //  done
   //

return ( true );

}



////////////////////////////////////////////////////////////////////////


void ModisFile::get_geo_field(SwathDataField * & field, const char * name)

{

field = Swath->get_geo_field(name);

if ( !field )  {

   cerr << "\n\n  ModisFile::get_geo_field() -> unable to find geolocation field \"" << name << "\"\n\n";

   exit ( 1 );

}

if ( field->rank() != 2 )  {

   cerr << "\n\n  ModisFile::get_geo_field() -> bad rank ("
        << (field->rank()) << ") for geolocation field \"" << name << "\"\n\n";

   exit ( 1 );

}

int k;

k = field->dimension_size(0);

if ( k != Dim0 )  {

   cerr << "\n\n  ModisFile::get_geo_field() -> bad dimension 0 ("
        << k << ") for geolocation field \"" << name << "\"\n\n";

   exit ( 1 );

}

k = field->dimension_size(1);

if ( k != Dim1 )  {

   cerr << "\n\n  ModisFile::get_geo_field() -> bad dimension 1 ("
        << k << ") for geolocation field \"" << name << "\"\n\n";

   exit ( 1 );

}


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ModisFile::get_data_field(SwathDataField * & field, const char * name)

{

field = Swath->get_data_field(name);

if ( !field )  {

   cerr << "\n\n  ModisFile::get_data_field() -> unable to find data field \"" << name << "\"\n\n";

   exit ( 1 );

}

if ( field->rank() != 2 )  {

   cerr << "\n\n  ModisFile::get_data_field() -> bad rank ("
        << (field->rank()) << ") for data field \"" << name << "\"\n\n";

   exit ( 1 );

}

int k;

k = field->dimension_size(0);

if ( k != Dim0 )  {

   cerr << "\n\n  ModisFile::get_data_field() -> bad dimension 0 ("
        << k << ") for data field \"" << name << "\"\n\n";

   exit ( 1 );

}

k = field->dimension_size(1);

if ( k != Dim1 )  {

   cerr << "\n\n  ModisFile::get_data_field() -> bad dimension 1 ("
        << k << ") for data field \"" << name << "\"\n\n";

   exit ( 1 );

}


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ModisFile::close()

{

if ( (FileId >= 0) && (SWclose(FileId) < 0) )  {

   cerr << "\n\n  ModisFile::close() -> trouble closing file\n\n";

   exit ( 1 );

}

FileId = -1;

Filename.clear();

if ( Swath )  { delete Swath;  Swath = (CloudsatSwath *) 0; }

Latitude = 0;

Longitude = 0;

SurfaceTemperature = 0;

CloudFraction = 0;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


double ModisFile::get_double_data(SwathDataField * field, int n0, int n1) const

{

int32 start[2];
intn status;
double * d = (double *) buf;

if ( (n0 < 0) || (n0 >= Dim0) || (n1 < 0) || (n1 >= Dim1) )  {

   cerr << "\n\n  ModisFile::get_float_data() -> range check error ... " << n0 << ", " << n1 << "\n\n";

   exit ( 1 );

}

start[0] = n0;
start[1] = n1;

ConcatString _field_name = field->name();

char * field_name = (char *) (const char *) _field_name;

// cout << "field_name = " << field_name << "\n" << flush;

status = SWreadfield(Swath->swath_id(), field_name, start, 0, edge_2, buf);

if ( status < 0 )  {

   cerr << "\n\n  CloudsatSwath::get_float_data(int, int) const -> bad SWreadfield status\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return ( d[0] );

}


////////////////////////////////////////////////////////////////////////


float ModisFile::get_float_data(SwathDataField * field, int n0, int n1) const

{

int32 start[2];
intn status;
float * f = (float *) buf;

if ( (n0 < 0) || (n0 >= Dim0) || (n1 < 0) || (n1 >= Dim1) )  {

   cerr << "\n\n  ModisFile::get_float_data() -> range check error ... " << n0 << ", " << n1 << "\n\n";

   exit ( 1 );

}

start[0] = n0;
start[1] = n1;

ConcatString _field_name = field->name();

char * field_name = (char *) (const char *) _field_name;

// cout << "field_name = " << field_name << "\n" << flush;

status = SWreadfield(Swath->swath_id(), field_name, start, 0, edge_2, buf);

if ( status < 0 )  {

   cerr << "\n\n  CloudsatSwath::get_float_data(int, int) const -> bad SWreadfield status\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return ( f[0] );

}


////////////////////////////////////////////////////////////////////////


short ModisFile::get_int16_data(SwathDataField * field, int n0, int n1) const

{

int32 start[2];
intn status;
short * s = (short *) buf;

if ( (n0 < 0) || (n0 >= Dim0) || (n1 < 0) || (n1 >= Dim1) )  {

   cerr << "\n\n  ModisFile::get_int16_data() -> range check error\n\n";

   exit ( 1 );

}

start[0] = n0;
start[1] = n1;

ConcatString _field_name = field->name();

char * field_name = (char *) (const char *) _field_name;

// cout << "field_name = " << field_name << "\n" << flush;

status = SWreadfield(Swath->swath_id(), field_name, start, 0, edge_2, buf);

if ( status < 0 )  {

   cerr << "\n\n  CloudsatSwath::get_float_data(int, int) const -> bad SWreadfield status\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return ( s[0] );

}


////////////////////////////////////////////////////////////////////////


char ModisFile::get_int8_data(SwathDataField * field, int n0, int n1) const

{

int32 start[2];
intn status;
char * c = (char *) buf;

if ( (n0 < 0) || (n0 >= Dim0) || (n1 < 0) || (n1 >= Dim1) )  {

   cerr << "\n\n  ModisFile::get_int16_data() -> range check error\n\n";

   exit ( 1 );

}

start[0] = n0;
start[1] = n1;

ConcatString _field_name = field->name();

char * field_name = (char *) (const char *) _field_name;

// cout << "field_name = " << field_name << "\n" << flush;

status = SWreadfield(Swath->swath_id(), field_name, start, 0, edge_2, buf);

if ( status < 0 )  {

   cerr << "\n\n  CloudsatSwath::get_float_data(int, int) const -> bad SWreadfield status\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return ( c[0] );

}


////////////////////////////////////////////////////////////////////////


double ModisFile::lat(int n0, int n1) const

{

double v;

v = get_float_data(Latitude, n0, n1);

return ( v );

}


////////////////////////////////////////////////////////////////////////


double ModisFile::lon(int n0, int n1) const

{

double v;

v = get_float_data(Longitude, n0, n1);

v = -v;   //  west longitude positive

return ( v );

}


////////////////////////////////////////////////////////////////////////


void ModisFile::latlon_range(double & lat_min, double & lat_max, double & lon_min, double & lon_max) const

{

int n0, n1;
double lat_point, lon_point;

lat_min = lat_max = lat(0, 0);
lon_min = lon_max = lon(0, 0);

for (n0=0; n0<Dim0; ++n0)  {

   for (n1=0; n1<Dim1; ++n1)  {

      lat_point = lat(n0, n1);
      lon_point = lon(n0, n1);

      if ( lat_point < lat_min )  lat_min = lat_point;
      if ( lat_point > lat_max )  lat_max = lat_point;

      if ( lon_point < lon_min )  lon_min = lon_point;
      if ( lon_point > lon_max )  lon_max = lon_point;

   }

}


return;

}


////////////////////////////////////////////////////////////////////////


bool ModisFile::surface_temperature(int n0, int n1, double & value) const

{

value = 0.0;

const short s = get_int16_data(SurfaceTemperature, n0, n1);

if ( s == -32768 )  return ( false );

// value = 0.01*s - 15000.0;

// value = 0.01*(s - 15000);

// value = 0.01*s + 15000.0;

value = 0.01*(s + 15000.0);


   //
   //   done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool ModisFile::cloud_fraction(int n0, int n1, double & value) const

{

value = 0.0;
int k;

const char c = get_int8_data(CloudFraction, n0, n1);

if ( c < 0 )  return ( false );

k = (int) c;

value = 0.01*k;


   //
   //   done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


ConcatString ModisFile::short_name() const

{

ConcatString s;

if ( Filename.nonempty() )  s = get_short_name(Filename);

return ( s );

}


////////////////////////////////////////////////////////////////////////


CloudsatSwath * ModisFile::swath() const

{

return ( Swath );

}


////////////////////////////////////////////////////////////////////////


unixtime ModisFile::scan_start_time() const

{

return ( ScanStartTime );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


void clear_buf()

{

memset(buf, 0, buf_size);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////





