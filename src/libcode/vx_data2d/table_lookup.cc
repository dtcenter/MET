

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "table_lookup.h"
#include "vx_util.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Grib2TableEntry
   //


////////////////////////////////////////////////////////////////////////


Grib2TableEntry::Grib2TableEntry()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Grib2TableEntry::~Grib2TableEntry()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Grib2TableEntry::Grib2TableEntry(const Grib2TableEntry & e)

{

init_from_scratch();

assign(e);

}


////////////////////////////////////////////////////////////////////////


Grib2TableEntry & Grib2TableEntry::operator=(const Grib2TableEntry & e)

{

if ( this == &e )  return ( * this );

assign(e);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Grib2TableEntry::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Grib2TableEntry::clear()

{

index_a = index_b = index_c = -1;

parm_name.clear();

full_name.clear();

units.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Grib2TableEntry::assign(const Grib2TableEntry & e)

{

clear();

index_a = e.index_a;
index_b = e.index_b;
index_c = e.index_c;

parm_name = e.parm_name;

full_name = e.full_name;

units = e.units;

return;

}


////////////////////////////////////////////////////////////////////////


void Grib2TableEntry::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Index values = (" 
              << index_a << ", "
              << index_b << ", "
              << index_c << ")\n";

out << prefix << 

out << prefix << "parm_name = " << parm_name.contents() << "\n";

out << prefix << "full_name = " << full_name.contents() << "\n";

out << prefix << "units     = " << units.contents()     << "\n";

return;

}


////////////////////////////////////////////////////////////////////////


bool Grib2TableEntry::parse_line(const char * line)

{

int j, n;
int * i[3];
ConcatString * s[3];
const char i_delim [] = "{} \"";
const char s_delim [] = "\"";
const char * c = (const char *) 0;
char * line2 = (char *) 0;
char * L = (char *) 0;

n = strlen(line);

line2 = new char [1 + n];

strncpy(line2, line, n);

L = line2;

clear();

i[0] = &index_a;
i[1] = &index_b;
i[2] = &index_c;

s[0] = &parm_name;
s[1] = &full_name;
s[2] = &units;

   //
   //  grab the first 3 ints
   //

for (j=0; j<3; ++j)  {

   c = strtok(L, i_delim);

   *(i[j]) = atoi(c);

   L = (char *) 0;

}   //  while

   //
   //  grab the strings
   //

for (j=0; j<3; ++j)  {

   c = strtok(L, s_delim);
   c = strtok(L, s_delim);

   *(s[j]) = c;

   L = (char *) 0;

}   //  while

   //
   //  done
   //

delete [] line2;  line2 = (char *) 0;

return ( true );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class TableFlatFile
   //


////////////////////////////////////////////////////////////////////////


TableFlatFile::TableFlatFile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


TableFlatFile::~TableFlatFile()

{

clear();

}


////////////////////////////////////////////////////////////////////////


TableFlatFile::TableFlatFile(const TableFlatFile & f)

{

init_from_scratch();

assign(f);

}


////////////////////////////////////////////////////////////////////////


void TableFlatFile::init_from_scratch()

{

g2e = (Grib2TableEntry *) 0;

clear();

}


////////////////////////////////////////////////////////////////////////


void TableFlatFile::clear()

{

if ( g2e )  { delete [] g2e; g2e = (Grib2TableEntry *) 0; }

Filename.clear();

Nelements = 0;

return;

}

////////////////////////////////////////////////////////////////////////


void TableFlatFile::dump(ostream & out, int depth) const

{

int j;
Indent prefix(depth);

out << prefix << "Filename  = " << Filename.contents() << "\n";
out << prefix << "Nelements = " << Nelements << "\n";

for (j=0; j<Nelements; ++j)  {

   out << prefix << "Element # " << j << " ...\n";

   g2e[j].dump(out, depth + 1);

}


return;

}


////////////////////////////////////////////////////////////////////////


void TableFlatFile::assign(const TableFlatFile & f)

{

clear();

if ( f.Nelements == 0 )  return;

Nelements = f.Nelements;

Filename = f.Filename;

g2e = new Grib2TableEntry [Nelements];

int j;

for (j=0; j<Nelements; ++j)  {

   g2e[j] = f.g2e[j];

}

return;

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::read(const char * _filename)

{

clear();
ifstream in;
ConcatString line;
int n_lines;

if ( empty(_filename) )  {

   mlog << Error 
        << "TableFlatFile::read(const char *) -> empty filename!\n\n";

   exit ( 1 );

}

n_lines = file_linecount(_filename);

Filename = _filename;

Nelements = n_lines - 1;

in.open(_filename);

if ( !in )  {

   mlog << Error 
        << "TableFlatFile::read(const char *) -> unable to open input file \"" << _filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  get first line for format
   //

line.read_line(in);

line.chomp('\n');

if ( line == "GRIB2" )  return ( read_grib2(in) );
else {

   mlog << Error 
        << "TableFlatFile::read(const char *) -> unable unrecognized format spec \""
        << line << "\" in file \"" << _filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  done
   //

in.close();

return ( false );

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::read_grib2(istream & in)

{

int j;
ConcatString line;
bool status = false;

g2e = new Grib2TableEntry [Nelements];

for (j=0; j<Nelements; ++j)  {

   status = line.read_line(in);

   if ( ! status )  {

      mlog << Error
           << "\n\n  TableFlatFile::read_grib2(istream &) -> trouble reading file \"" << filename() << "\"\n\n";

      exit ( 1 );

   }

   if ( j == (Nelements - 1) )  {

      cout << "Hello!\n\n" << flush;

   }

   status = g2e[j].parse_line(line);

   if ( ! status )  {

      mlog << Error
           << "\n\n  TableFlatFile::read_grib2(istream &) -> trouble parsing line \""
           << line << "\" from input file \"" << filename() << "\"\n\n";

      exit ( 1 );

   }

}   //  for j


   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::lookup_grib2(int a, int b, int c, Grib2TableEntry & e)

{

int j;

e.clear();

for (j=0; j<Nelements; ++j)  {

   if ( (g2e[j].index_a == a) && (g2e[j].index_b == b) && (g2e[j].index_c == c) )  {

      e = g2e[j];

      return ( true );

   }

}

return ( false );

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::lookup_grib2(const char * parm_name, Grib2TableEntry & e, int & n_matches)

{

e.clear();

n_matches = 0;

int j;

for (j=0; j<Nelements; ++j)  {

   if ( g2e[j].parm_name != parm_name )  continue;

   if ( n_matches == 0 )  e = g2e[j];

   ++n_matches;

}



return ( (n_matches > 0) );

}


////////////////////////////////////////////////////////////////////////






