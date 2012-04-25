

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
   //  This needs external linkage
   //

TableFlatFile GribTable (0);


////////////////////////////////////////////////////////////////////////


static const char table_data_dir   [] = "data/table_files";      //  relative to MET_BASE_DIR

static const char grib1_table_file [] = "nceptab_flat.txt";      //  relative to table_data_dir

static const char grib2_table_file [] = "grib2_vars_flat.txt";   //  relative to table_data_dir


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Grib1TableEntry
   //


////////////////////////////////////////////////////////////////////////


Grib1TableEntry::Grib1TableEntry()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Grib1TableEntry::~Grib1TableEntry()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Grib1TableEntry::Grib1TableEntry(const Grib1TableEntry & e)

{

init_from_scratch();

assign(e);

}


////////////////////////////////////////////////////////////////////////


Grib1TableEntry & Grib1TableEntry::operator=(const Grib1TableEntry & e)

{

if ( this == &e )  return ( * this );

assign(e);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Grib1TableEntry::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Grib1TableEntry::clear()

{

code = table_number = -1;

parm_name.clear();

full_name.clear();

units.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Grib1TableEntry::assign(const Grib1TableEntry & e)

{

clear();

code = e.code;
table_number = e.table_number;

parm_name = e.parm_name;

full_name = e.full_name;

units = e.units;

return;

}


////////////////////////////////////////////////////////////////////////


void Grib1TableEntry::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Index values = (" 
              << code << ", "
              << table_number << ")\n";

out << prefix << "parm_name = " << parm_name.contents() << "\n";

out << prefix << "full_name = " << full_name.contents() << "\n";

out << prefix << "units     = " << units.contents()     << "\n";

return;

}


////////////////////////////////////////////////////////////////////////


bool Grib1TableEntry::parse_line(const char * line)

{

clear();

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

memset(line2, 0, 1 + n);

strncpy(line2, line, n);

L = line2;

clear();

i[0] = &code;
i[1] = &table_number;

s[0] = &parm_name;
s[1] = &full_name;
s[2] = &units;

   //
   //  grab the first 2 ints
   //

for (j=0; j<2; ++j)  {

   c = strtok(L, i_delim);

   *(i[j]) = atoi(c);

   L = (char *) 0;

}   //  while

   //
   //  parm_name
   //

c = strtok(0, s_delim);

parm_name = c;

c = strtok(0, s_delim);

   //
   //  full_name
   //

c = strtok(0, s_delim);

full_name = c;

c = strtok(0, s_delim);

   //
   //  units (may be empty)
   //

c = strtok(0, s_delim);

if ( *c == ' ' )  units.clear();
else              units = c;

   //
   //  done
   //

delete [] line2;  line2 = (char *) 0;

return ( true );

}


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

out << prefix << "parm_name = " << parm_name.contents() << "\n";

out << prefix << "full_name = " << full_name.contents() << "\n";

out << prefix << "units     = " << units.contents()     << "\n";

return;

}


////////////////////////////////////////////////////////////////////////


bool Grib2TableEntry::parse_line(const char * line)

{

clear();

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


TableFlatFile::TableFlatFile(int)

{

init_from_scratch();

ConcatString path;

   //
   //  read the default grib1 table file
   //

path << cs_erase << MET_BASE_DIR << '/' << table_data_dir << '/' << grib1_table_file;

if ( ! read(path) )  {

   mlog << Error
        << "TableFlatFile::TableFlatFile(int) -> unable to read table file \"" << path << "\"\n\n";

   exit ( 1 );

}

   //
   //  read the default grib2 table file
   //

path << cs_erase << MET_BASE_DIR << '/' << table_data_dir << '/' << grib2_table_file;

if ( ! read(path) )  {

   mlog << Error
        << "TableFlatFile::TableFlatFile(int) -> unable to read table file \"" << path << "\"\n\n";

   exit ( 1 );

}


   //
   //  done
   //

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

g1e = (Grib1TableEntry *) 0;
g2e = (Grib2TableEntry *) 0;

clear();

}


////////////////////////////////////////////////////////////////////////


void TableFlatFile::clear()

{

if ( g1e )  { delete [] g1e; g1e = (Grib1TableEntry *) 0; }
if ( g2e )  { delete [] g2e; g2e = (Grib2TableEntry *) 0; }

N_grib1_elements = 0;
N_grib2_elements = 0;

return;

}

////////////////////////////////////////////////////////////////////////


void TableFlatFile::dump(ostream & out, int depth) const

{

int j;
Indent prefix(depth);


out << prefix << "N_grib1_elements = " << N_grib1_elements << "\n";

for (j=0; j<N_grib1_elements; ++j)  {

   out << prefix << "Grib1 Element # " << j << " ...\n";

   g1e[j].dump(out, depth + 1);

}

out << prefix << "N_grib2_elements = " << N_grib2_elements << "\n";

for (j=0; j<N_grib2_elements; ++j)  {

   out << prefix << "Grib2 Element # " << j << " ...\n";

   g2e[j].dump(out, depth + 1);

}


return;

}


////////////////////////////////////////////////////////////////////////


void TableFlatFile::assign(const TableFlatFile & f)

{

clear();


if ( f.N_grib1_elements != 0 )  {

   N_grib1_elements = f.N_grib1_elements;

   g1e = new Grib1TableEntry [N_grib1_elements];

   int j;

   for (j=0; j<N_grib1_elements; ++j)  {

      g1e[j] = f.g1e[j];

   }

}

if ( f.N_grib2_elements != 0 )  {

   N_grib2_elements = f.N_grib2_elements;

   g2e = new Grib2TableEntry [N_grib2_elements];

   int j;

   for (j=0; j<N_grib2_elements; ++j)  {

      g2e[j] = f.g2e[j];

   }

}

return;

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::read(const char * filename)

{

// clear();

ifstream in;
ConcatString line;
int n_lines;

if ( empty(filename) )  {

   mlog << Error 
        << "TableFlatFile::read(const char *) -> empty filename!\n\n";

   exit ( 1 );

}

n_lines = file_linecount(filename);

in.open(filename);

if ( !in )  {

   mlog << Error 
        << "TableFlatFile::read(const char *) -> unable to open input file \"" << filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  get first line for format
   //

line.read_line(in);

line.chomp('\n');

     if ( line == "GRIB1" )  { N_grib1_elements = n_lines - 1;  return ( read_grib1(in, filename) ); }
else if ( line == "GRIB2" )  { N_grib2_elements = n_lines - 1;  return ( read_grib2(in, filename) ); }
else {

   mlog << Error 
        << "TableFlatFile::read(const char *) -> unable unrecognized format spec \""
        << line << "\" in file \"" << filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  done
   //

in.close();

return ( false );

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::read_grib1(istream & in, const char * filename)

{

int j;
ConcatString line;
bool status = false;

g1e = new Grib1TableEntry [N_grib1_elements];

for (j=0; j<N_grib1_elements; ++j)  {

   status = line.read_line(in);

   if ( ! status )  {

      mlog << Error
           << "\n\n  TableFlatFile::read_grib1(istream &) -> trouble reading file \"" << filename << "\"\n\n";

      exit ( 1 );

   }

   status = g1e[j].parse_line(line);

   if ( ! status )  {

      mlog << Error
           << "\n\n  TableFlatFile::read_grib1(istream &) -> trouble parsing line \""
           << line << "\" from input file \"" << filename << "\"\n\n";

      exit ( 1 );

   }

}   //  for j


   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::read_grib2(istream & in, const char * filename)

{

int j;
ConcatString line;
bool status = false;

g2e = new Grib2TableEntry [N_grib2_elements];

for (j=0; j<N_grib2_elements; ++j)  {

   status = line.read_line(in);

   if ( ! status )  {

      mlog << Error
           << "\n\n  TableFlatFile::read_grib2(istream &) -> trouble reading file \"" << filename << "\"\n\n";

      exit ( 1 );

   }

   status = g2e[j].parse_line(line);

   if ( ! status )  {

      mlog << Error
           << "\n\n  TableFlatFile::read_grib2(istream &) -> trouble parsing line \""
           << line << "\" from input file \"" << filename << "\"\n\n";

      exit ( 1 );

   }

}   //  for j


   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::lookup_grib1(int code, int table_number, Grib1TableEntry & e)

{

int j;

e.clear();

for (j=0; j<N_grib1_elements; ++j)  {

   if ( (g1e[j].code == code) && (g1e[j].table_number == table_number) )  {

      e = g1e[j];

      return ( true );

   }

}

return ( false );

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::lookup_grib1(int code, Grib1TableEntry & e)   //  assumes table_number = 2;

{

const int table_number = 2;
bool status = false;

status = lookup_grib1(code, table_number, e);

return ( status );

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::lookup_grib1(int table_number, const char * parm_name, Grib1TableEntry & e, int & n_matches)

{

e.clear();

n_matches = 0;

int j;

for (j=0; j<N_grib1_elements; ++j)  {

   if ( g1e[j].table_number != table_number )  continue;

   if ( g1e[j].parm_name != parm_name )  continue;

   if ( n_matches == 0 )  e = g1e[j];

   ++n_matches;

}



return ( (n_matches > 0) );

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::lookup_grib1(const char * parm_name, Grib1TableEntry & e)   //  assumes table number is 2

{

const int table_number = 2;
bool status = false;
int n_matches;

status = lookup_grib1(table_number, parm_name, e, n_matches);

return ( status );

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::lookup_grib2(int a, int b, int c, Grib2TableEntry & e)

{

int j;

e.clear();

for (j=0; j<N_grib2_elements; ++j)  {

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

for (j=0; j<N_grib2_elements; ++j)  {

   if ( g2e[j].parm_name != parm_name )  continue;

   if ( n_matches == 0 )  e = g2e[j];

   ++n_matches;

}



return ( (n_matches > 0) );

}


////////////////////////////////////////////////////////////////////////






