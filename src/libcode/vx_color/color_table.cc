// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>


#include "color.h"
#include "vx_log.h"
#include "vx_math.h"


using namespace std;


////////////////////////////////////////////////////////////////////////


   //
   //  declarations with external linkage
   //

extern int colordebug;

extern int colorparse();

extern FILE * colorin;

extern ColorList clist;

extern int color_file_line_number;

extern int color_file_column;



   //
   //  definitions with external linkage
   //

const char * input_filename = (const char *) nullptr;

// ColorList clist;

ColorTable * the_table = (ColorTable *) nullptr;


////////////////////////////////////////////////////////////////////////


   //
   //  static stuff
   //


static const int debug  = 0;

static const Color black(0, 0, 0);

static void init_clist(ColorList &);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class CtableEntry
   //


////////////////////////////////////////////////////////////////////////


CtableEntry::CtableEntry()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


CtableEntry::~CtableEntry()

{

clear();

}


////////////////////////////////////////////////////////////////////////


CtableEntry::CtableEntry(const CtableEntry & entry)

{

init_from_scratch();

assign(entry);

}


////////////////////////////////////////////////////////////////////////


CtableEntry & CtableEntry::operator=(const CtableEntry & entry)

{

if ( this == &entry )  return *this;

assign(entry);

return *this;

}


////////////////////////////////////////////////////////////////////////


void CtableEntry::init_from_scratch()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void CtableEntry::clear()

{

ValueLo = ValueHi = 0.0;

C.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void CtableEntry::assign(const CtableEntry & entry)

{

clear();

ValueLo = entry.ValueLo;
ValueHi = entry.ValueHi;

C = entry.C;

return;

}


////////////////////////////////////////////////////////////////////////


void CtableEntry::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "ValueLo  = " << ValueLo << "\n";
out << prefix << "ValueHi  = " << ValueHi << "\n";
out << prefix << "Color ... \n";

C.dump(out, depth + 1);

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void CtableEntry::set_values(double v1, double v2)

{

ValueLo = ( (v1 < v2) ? v1 : v2 );
ValueHi = ( (v1 < v2) ? v2 : v1 );

return;

}


////////////////////////////////////////////////////////////////////////


void CtableEntry::set_value(double v)

{

ValueLo = ValueHi = v;

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ColorTable
   //


////////////////////////////////////////////////////////////////////////


ColorTable::ColorTable()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ColorTable::~ColorTable()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ColorTable::ColorTable(const ColorTable & c)

{

init_from_scratch();

assign(c);

}


////////////////////////////////////////////////////////////////////////


ColorTable & ColorTable::operator=(const ColorTable & c)

{

if ( this == &c )  return *this;

assign(c);

return *this;

}


////////////////////////////////////////////////////////////////////////


void ColorTable::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void ColorTable::clear()

{

Entry.clear();

Gamma = 1.0;

for (int j=0; j<fudge_size; ++j)  {

   fudge[j] = (unsigned char) j;

}

RescaleFlag = false;

return;

}


////////////////////////////////////////////////////////////////////////


void ColorTable::assign(const ColorTable & c)

{

clear();

extend(c.n_entries());

Entry = c.Entry;

Gamma = c.Gamma;

for (int j=0; j<fudge_size; ++j)  {

   fudge[j] = c.fudge[j];

}

RescaleFlag = c.RescaleFlag;

return;

}


////////////////////////////////////////////////////////////////////////


Color ColorTable::nearest(double t_in) const

{

Color color;

double t = ( is_bad_data(t_in) ? bad_data_double : t_in );

if ( Entry.empty() )  return black;

if ( t <= Entry[0].value_low() )  return Entry[0].color();

if ( t >= Entry.back().value_high() )  return Entry.back().color();

double min_dist = fabs(t - Entry[0].value_low());

color = Entry[0].color();

for (const auto &e : Entry) {

   double vlo = e.value_low();
   double vhi = e.value_high();

   if ( (t >= vlo) && (t <= vhi) )  {   //  direct hit

      color = e.color();

      fudge_color(color);

      return color;

   }

   double d1 = fabs(t - vlo);
   double d2 = fabs(t - vhi);

   double dist = ( (d1 < d2) ? d1 : d2 );

   if ( dist < min_dist )  {

      color = e.color();

      min_dist = dist;

   }

}

fudge_color(color);

return color;

}


////////////////////////////////////////////////////////////////////////


Color ColorTable::interp(double z_in) const

{

Color color;

double z = ( is_bad_data(z_in) ? bad_data_double : z_in );

   //
   //  check for empty colortable
   //

if ( Entry.empty() )  return color;

   //
   //  check if value outside range
   //

if ( z <= Entry[0].value_low() )  return Entry[0].color();

if ( z >= Entry.back().value_high() )  return Entry.back().color();

   //
   //  check for direct hits
   //

for (const auto &e : Entry) {

   double vlo = e.value_low();
   double vhi = e.value_high();

   if ( (z >= vlo) && (z <= vhi) )  {   //  direct hit

      color = e.color();

      fudge_color(color);

      return color;

   }

}

   //
   //  must be in-between
   //

for (int j=0; j<(Entry.size() - 1); ++j)  {

   int k = j + 1;

   double vlo = Entry[j].value_high();
   double vhi = Entry[k].value_low();

   if ( (z >= vlo) && (z <= vhi) )  {

      double t = (z - vlo)/(vhi - vlo);

      color = blend_colors(Entry[j].color(), Entry[k].color(), t);

      fudge_color(color);

      return color;

   }

}   //  for j


   //
   //  should never get here
   //

mlog << Error << "\nColorTable::interp(double) const -> confused!\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


int ColorTable::read(const char * filename)

{

int status;

clear();

input_filename = filename;

if ( (colorin = fopen(input_filename, "r")) == nullptr )  {

   colorin = (FILE *) nullptr;

   mlog << Error << "\nColorTable::read(const char *) -> failed to read colortable file \""
        << input_filename << "\"\n\n";

   exit ( 1 );

}

colordebug = debug;

color_file_line_number = 1;

color_file_column = 1;

clist.clear();

init_clist(clist);

the_table = this;

status = colorparse();

if ( status )  {

   mlog << Error << "\nColorTable::read(const char *) -> failed to read colortable file \""
        << input_filename << "\"\n\n";

   exit ( 1 );

}

sort();


   //
   //  done
   //

fclose(colorin);   colorin = (FILE *) nullptr;

input_filename = (const char *) nullptr;

the_table = (ColorTable *) nullptr;

clist.clear();

return 1;

}


////////////////////////////////////////////////////////////////////////


int ColorTable::write(const char * filename)

{

int j;
ofstream out;

out.open(filename);

if ( !out )  {

   mlog << Error << "\nColorTable::write() -> unable to open output file \"" << filename << "\"\n\n";

   // exit ( 1 );

   return 0;

}

for (const auto &e : Entry) out << e;

out.close();

return 1;

}


////////////////////////////////////////////////////////////////////////


void ColorTable::fudge_color(Color & c) const

{

unsigned char r = fudge[c.red()];
unsigned char g = fudge[c.green()];
unsigned char b = fudge[c.blue()];

c.set_rgb(r, g, b);

return;

}


////////////////////////////////////////////////////////////////////////


void ColorTable::set_gamma(double g)

{

if ( g <= 0.0 )  {

   mlog << Error << "\nvoid ColorTable::set_gamma(double) -> "
        << "bad gamma value ... " << g << "\n\n";

   exit ( 1 );

}

Gamma = g;

const double exponent = 1.0/Gamma;

for (int j=0; j<fudge_size; ++j)  {

   double x = ((double) j)/255.0;

   double y = pow(x, exponent);

   int k = nint(255.0*y);

   fudge[j] = (unsigned char) k;

}

return;

}


////////////////////////////////////////////////////////////////////////


void ColorTable::set_gray()

{

for (auto &e : Entry) {

   Color c = e.color();

   c.to_gray();

   e.set_color(c);

}

return;

}


////////////////////////////////////////////////////////////////////////


void ColorTable::tint(const Color & c, double t)

{

for (auto &e : Entry) {

   Color cc = blend_colors(e.color(), c, t);

   e.set_color(cc);

}

return;

}


////////////////////////////////////////////////////////////////////////


void ColorTable::dump(ostream & out, int indent_depth) const

{

Indent prefix (indent_depth);


out << prefix << "Colortable dump ... \n";
out << prefix << "Gamma     = " << Gamma    << "\n";
out << prefix << "# entries = " << Entry.size() << "\n";

for (int j=0; j<Entry.size(); ++j)  {

   out << prefix << "Entry # " << j << " ...\n";

   Entry[j].dump(out, indent_depth + 1);

}

return;

}


////////////////////////////////////////////////////////////////////////


void ColorTable::extend(int n)

{

if ( Entry.capacity() >= n )  return;

int j = n/ctable_alloc_inc;

if ( n%ctable_alloc_inc )  ++j;

n = j*ctable_alloc_inc;

Entry.reserve(n);

return;

}


////////////////////////////////////////////////////////////////////////


void ColorTable::add_entry(const CtableEntry & ce)

{

extend((int) Entry.size() + 1);

Entry.emplace_back(ce);

return;

}


////////////////////////////////////////////////////////////////////////


void ColorTable::sort()

{

if ( Entry.size() <= 1 )  return;

   //
   //  simple bubble sort - colortables
   //
   //     don't usually have very many entries
   //

for (int j=0; j<(Entry.size() - 1); ++j)  {

   for (int k=(j + 1); k<Entry.size(); ++k)  {

      if ( Entry[j].value_low() > Entry[k].value_low() )  {

         CtableEntry ce = Entry[j];

         Entry[j] = Entry[k];

         Entry[k] = ce;

      }

   }   //  for k

}   //  for j

return;

}


////////////////////////////////////////////////////////////////////////


CtableEntry ColorTable::operator[](int n) const

{

if ( (n < 0) || (n >= Entry.size()) )  {

   mlog << Error << "\nCtableEntry ColorTable::operator[](int) const -> "
        << "range check error\n\n";

   exit ( 1 );

}

return Entry[n];

}


////////////////////////////////////////////////////////////////////////


double ColorTable::data_min() const

{

if ( Entry.empty() == 0 )  return 0.0;

return Entry[0].value_low();

}


////////////////////////////////////////////////////////////////////////
//
// This function computes the min value in the colortable ignoring
// any instances of the bad_data_value.
//
////////////////////////////////////////////////////////////////////////


double ColorTable::data_min(double bad_data_value) const

{

if ( Entry.empty() )  return 0.0;

double v_low = 1.0e30;

for(const auto &e : Entry) {

   if(e.value_low() < v_low &&
      !is_eq(e.value_low(), bad_data_value))
      v_low = e.value_low();
}

return v_low;

}


////////////////////////////////////////////////////////////////////////


double ColorTable::data_max() const

{

if ( Entry.empty() )  return 0.0;

return Entry.back().value_high();

}


////////////////////////////////////////////////////////////////////////
//
// This function computes the max value in the colortable ignoring
// any instances of the bad_data_value.
//
////////////////////////////////////////////////////////////////////////


double ColorTable::data_max(double bad_data_value) const

{

if ( Entry.empty() )  return 0.0;

double v_high = -1.0e30;

for(const auto &e : Entry) {

   if(e.value_high() > v_high &&
      !is_eq(e.value_high(), bad_data_value))
      v_high = e.value_high();
}

return v_high;

}


////////////////////////////////////////////////////////////////////////
//
// This function provides the capability of rescaling the color table
// to a new range of values based on the min/max values.
//
////////////////////////////////////////////////////////////////////////


void ColorTable::rescale(double new_min, double new_max, double bad_data_value)

{

if(Entry.size() <= 0) {

   mlog << Error << "\nCtableEntry ColorTable::rescale(double, double, double) -> "
        << "the colortable must be non-empty to call rescale.\n\n";

   exit ( 1 );

}

double old_b = data_min(bad_data_value);
double old_m = data_max(bad_data_value) - data_min(bad_data_value);

double new_b = new_min;
double new_m = new_max - new_min;

for(auto &e : Entry) {

   double old_v_lo = e.value_low();
   double old_v_hi = e.value_high();

   //
   // Check old low value for bad data or scale the old value to a
   // [0, 1] range and convert to the new range
   //
   double new_v_lo;
   if(is_eq(old_v_lo, bad_data_value)) {
      new_v_lo = bad_data_value;
   }
   else {
      double v_lo = (old_v_lo - old_b)/old_m;
      new_v_lo = v_lo*new_m + new_b;
   }

   //
   // Check old hi value for bad data or scale the old value to a
   // [0, 1] range and convert to the new range
   //
   double new_v_hi;
   if(is_eq(old_v_hi, bad_data_value)) {
      new_v_hi = bad_data_value;
   }
   else {
      double v_hi = (old_v_hi - old_b)/old_m;
      new_v_hi = v_hi*new_m + new_b;
   }

   e.set_values(new_v_lo, new_v_hi);
}

//
// Set flag to indicate that the colortable was rescaled
//
RescaleFlag = true;

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


istream & operator>>(istream & s, CtableEntry & entry)

{

int j;
int r, g, b;
double x;
char line[256];
Color c;


s.getline(line, sizeof(line));

if ( !s )  return s;

j = sscanf(line, "%lf%d%d%d", &x, &r, &g, &b);

if ( j != 4 )  {

   mlog << Error << "\noperator>>(ostream &, CtableEntry &) -> bad line (j = " << j << " ... \""
        << line << "\"\n\n";

   exit ( 1 );

}

entry.set_value(x);

c.set_rgb(r, g, b);

entry.set_color(c);


return s;

}


////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream & s, const CtableEntry & entry)

{

if ( fabs(entry.value_high() - entry.value_low()) < 1.0e-5 )  {

   s << (entry.value_low())   << "       { ";

} else {

   s << (entry.value_low())    << "       "
     << (entry.value_high())   << "       { ";

}

int R, G, B;

R = (int) (entry.color().red());
G = (int) (entry.color().green());
B = (int) (entry.color().blue());

s << R << ", ";
s << G << ", ";
s << B << "}\n";



return s;

}


////////////////////////////////////////////////////////////////////////


void init_clist(ColorList & L)

{

ClistEntry ce;
Dcolor d;

   //
   //  pre-define a few colors in the list
   //

   //
   //  red
   //

d.r = 255.0;  d.g = 0.0;  d.b = 0.0;

ce.set_color(d);
ce.set_name("red");

clist.add(ce);

   //
   //  green
   //

d.r = 0.0;  d.g = 255.0;  d.b = 0.0;

ce.set_color(d);
ce.set_name("green");

clist.add(ce);

   //
   //  blue
   //

d.r = 0.0;  d.g = 0.0;  d.b = 255.0;

ce.set_color(d);
ce.set_name("blue");

clist.add(ce);

   //
   //  yellow
   //

d.r = 255.0;  d.g = 255.0;  d.b = 0.0;

ce.set_color(d);
ce.set_name("yellow");

clist.add(ce);

   //
   //  purple
   //

d.r = 255.0;  d.g = 0.0;  d.b = 255.0;

ce.set_color(d);
ce.set_name("purple");

clist.add(ce);

   //
   //  cyan
   //

d.r = 0.0;  d.g = 255.0;  d.b = 255.0;

ce.set_color(d);
ce.set_name("cyan");

clist.add(ce);

   //
   //  white
   //

d.r = 255.0;  d.g = 255.0;  d.b = 255.0;

ce.set_color(d);
ce.set_name("white");

clist.add(ce);

   //
   //  black
   //

d.r = 0.0;  d.g = 0.0;  d.b = 0.0;

ce.set_color(d);
ce.set_name("black");

clist.add(ce);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////






