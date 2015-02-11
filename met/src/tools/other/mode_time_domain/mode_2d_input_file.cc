

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "nc_utils.h"
#include "nc_grid.h"
#include "mode_2d_input_file.h"


////////////////////////////////////////////////////////////////////////


static ConcatString timestring(const Unixtime);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Mode2DInputFile
   //


////////////////////////////////////////////////////////////////////////


Mode2DInputFile::Mode2DInputFile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Mode2DInputFile::~Mode2DInputFile()

{

close();

}


////////////////////////////////////////////////////////////////////////


Mode2DInputFile::Mode2DInputFile(const Mode2DInputFile &)

{

cerr << "\n\n  Mode2DInputFile::Mode2DInputFile(const Mode2DInputFile &) -> should never be called!\n\n";

exit ( 1 );

// init_from_scratch();
// 
// assign(f);

}


////////////////////////////////////////////////////////////////////////


Mode2DInputFile & Mode2DInputFile::operator=(const Mode2DInputFile &)

{

cerr << "\n\n  Mode2DInputFile::operator=(const Mode2DInputFile &) -> should never be called!\n\n";

exit ( 1 );

// if ( this == &f )  return ( * this );
// 
// assign(f);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Mode2DInputFile::init_from_scratch()

{

Nc = (NcFile *) 0;

Var = (NcVar *) 0;

G = (Grid *) 0;

close();

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Mode2DInputFile::close()

{

if ( Nc )  { delete Nc;  Nc = (NcFile *) 0; }

Var = (NcVar *) 0;

if ( G )  { delete G;  G = (Grid *) 0; }

ValidTime = (Unixtime) 0;

LeadTime = 0;

FileName.clear();

FieldName.clear();

DataMin = DataMax = 0.0;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


int Mode2DInputFile::nx() const

{

if ( !G )  {

   cerr << "\n\n  Mode2DInputFile::nx() const -> no grid!\n\n";

   exit ( 1 );

}

return ( G->nx() );

}


////////////////////////////////////////////////////////////////////////


int Mode2DInputFile::ny() const

{

if ( !G )  {

   cerr << "\n\n  Mode2DInputFile::ny() const -> no grid!\n\n";

   exit ( 1 );

}

return ( G->ny() );

}


////////////////////////////////////////////////////////////////////////


void Mode2DInputFile::latlon_to_xy(double lat, double lon, double & x, double & y) const

{

if ( !G )  {

   cerr << "\n\n  Mode2DInputFile::latlon_to_xy() const -> no grid!\n\n";

   exit ( 1 );

}

G->latlon_to_xy(lat, lon, x, y);

return;

}


////////////////////////////////////////////////////////////////////////


void Mode2DInputFile::xy_to_latlon(double x, double y, double & lat, double & lon) const

{

if ( !G )  {

   cerr << "\n\n  Mode2DInputFile::xy_to_latlon() const -> no grid!\n\n";

   exit ( 1 );

}

G->xy_to_latlon(x, y, lat, lon);

return;

}


////////////////////////////////////////////////////////////////////////


bool Mode2DInputFile::open(const char * _filename, const char * _fieldname)

{

close();

   //
   //  open the file
   //

Nc = new NcFile(_filename);

if ( Nc->is_valid() == 0 )  {

   cerr << "\n\n  Mode2DInputFile::open() -> unable to open netcdf file \"" << _filename << "\"\n\n";

   close();

   return ( false );

}

FileName = _filename;

   //
   //  see if the fieldname is there
   //

int j;
const int nvars = Nc->num_vars();
NcAtt * att = (NcAtt *) 0;
bool found = false;
Unixtime init;


for (j=0; j<nvars; ++j)  {

   Var = Nc->get_var(j);

   if ( strcmp(Var->name(), _fieldname) == 0 )  { found = true;  break; }

}   //  for j

if ( !found )  {

   cerr << "\n\n  Mode2DInputFile::open() -> field \"" << _fieldname << "\" not found in file \"" << _filename << "\"\n\n";

   close();

   return ( false );

}

FieldName = _fieldname;

   //
   //  get time info
   //

att = Var->get_att("valid_time_ut");

ValidTime = (Unixtime) (att->as_int(0));

att = Var->get_att("init_time_ut");

init = (Unixtime) (att->as_int(0));

LeadTime = (int) (ValidTime - init);


   //
   //  parse the grid info
   //

G = new Grid;

if ( ! read_nc_grid(*Nc, *G) )  {

   cerr << "\n\n  Mode2DInputFile::open() -> unable to parse grid information in file \"" << FileName << "\"\n\n";

   return ( false );

}

   //
   //  get data range
   //

find_data_range();

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


double Mode2DInputFile::data_value(int x, int y) const

{

if ( (x < 0) || (x >= nx()) || (y < 0) || (y >= ny()) )  {

   cerr << "\n\n  Mode2DInputFile::data_value(int x, int y) const -> range check error\n\n";

   exit ( 1 );

}

if ( !Nc || !Var )  {

   cerr << "\n\n  Mode2DInputFile::data_value(int x, int y) const -> no input file!\n\n";

   exit ( 1 );

}

double value = 0.0;
float f[2];


if ( ! (Var->set_cur(y, x)) )  {   //  NOTE reversal of x and y

   cerr << "\n\n  Mode2DInputFile::data_value(int x, int y) const -> trouble setting corner\n\n";

   exit ( 1 );

}

if ( ! (Var->get(f, 1, 1)) )  {

   cerr << "\n\n  Mode2DInputFile::data_value(int x, int y) const -> trouble getting data\n\n";

   exit ( 1 );

}

value = (double) (f[0]);

if ( value < 0.0 )  value = 0.0;   //  screen out "-9999" flag values

   //
   //  done
   //

return ( value );

}


////////////////////////////////////////////////////////////////////////


void Mode2DInputFile::dump(ostream & out, int depth) const

{

Indent prefix(depth);
char junk[256];



out << prefix << "File  Name = ";

if ( FileName.nonempty() )   out << '\"' << FileName << "\"\n";
else                         out << "(nul)\n";


out << prefix << "Field Name = ";

if ( FieldName.nonempty() )  out << '\"' << FieldName << "\"\n";
else                         out << "(nul)\n";

out << prefix << '\n';

comma_string(ValidTime, junk);

out << prefix << "Valid Time = " << junk;

if ( ValidTime != 0 )  {

   out << "   (" << timestring(ValidTime) << ')';

}

out << '\n';

out << prefix << "Lead  Time = " << LeadTime  << "\n";

out << prefix << '\n';

out << prefix << "Data  Min  = " << DataMin << "\n";
out << prefix << "Data  Max  = " << DataMax << "\n";


out << prefix << '\n';

out << prefix << "Grid ...\n";

if ( G )  G->dump(out, depth + 1);
else      out << "(nul)";

out << '\n';



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Mode2DInputFile::find_data_range()

{

int x, y;
double value;
const int Nx = nx();
const int Ny = ny();


DataMin = DataMax = data_value(0, 0);

for (x=0; x<Nx; ++x)  {

   for (y=0; y<Ny; ++y)  {

      value = data_value(x, y);
      // value = operator()(x, y);

      if ( value < DataMin )  DataMin = value;
      if ( value > DataMax )  DataMax = value;

   }

}


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


Grid Mode2DInputFile::grid() const

{

if ( !G )  {

   cerr << "\n\n  Mode2DInputFile::grid() -> no current grid!\n\n";

   exit ( 1 );

}


return ( *G );

}


////////////////////////////////////////////////////////////////////////


bool Mode2DInputFile::same_grid(const Grid & g) const

{

if ( !G )  return ( false );

bool status = ( (*G) == g );

return ( status );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


ConcatString timestring(const Unixtime t)

{

char junk[256];
int month, day, year, hour, minute, second;


unix_to_mdyhms(t, month, day, year, hour, minute, second);

sprintf(junk, "%s %d, %d  %d:%02d:%02d", short_month_name[month], day, year, hour, minute, second);


   //
   //  done
   //

return ( ConcatString (junk) );

}


////////////////////////////////////////////////////////////////////////




