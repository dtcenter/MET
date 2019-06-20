// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
#include <string.h>
#include <cmath>
#include <vector>
#include <dirent.h>

#include "table_lookup.h"
#include "vx_util.h"
#include "vx_math.h"
#include <cerrno>
#include <sys/stat.h>


////////////////////////////////////////////////////////////////////////


   //
   //  This needs external linkage
   //

TableFlatFile GribTable (0);


////////////////////////////////////////////////////////////////////////


static const char table_data_dir  [] = "MET_BASE/table_files"; //  relative to MET_BASE
static const char met_grib_tables [] = "MET_GRIB_TABLES";      //  environment variable name
static const char user_grib_tables[] = "USER_GRIB_TABLES";     //  deprecated environment variable name


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

code = table_number = center = subcenter = -1;

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
center = e.center;
subcenter = e.subcenter;

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
              << table_number << ", "
              << center << ", "
              << subcenter <<  ")\n";

out << prefix << "parm_name = " << parm_name.contents() << "\n";

out << prefix << "full_name = " << full_name.contents() << "\n";

out << prefix << "units     = " << units.contents()     << "\n";

return;

}


////////////////////////////////////////////////////////////////////////


bool Grib1TableEntry::parse_line(const char * line)

{

clear();

int j;
StringArray tok;

   //
   //  grab the first 4 whitespace separated integers
   //

tok.parse_wsss(line);

if (tok.n_elements() < 4) return (false);

for (j=0; j<4; ++j) {
   if(!is_number(tok[j].c_str())) return (false);
}

code         = atoi(tok[0].c_str());
table_number = atoi(tok[1].c_str());
center       = atoi(tok[2].c_str());
subcenter    = atoi(tok[3].c_str());

   //
   //  grab the 3 strings separated by double quotes
   //

tok.parse_delim(line, "\"");

if (tok.n_elements() < 6) return (false);

parm_name = tok[1];
full_name = tok[3];
units     = tok[5];  // may be empty

if (units == "\n" || units == "\n\n") units.clear();

   //
   // if empty, set to the NA string to avoid a runtime error writing
   // units attribute to NetCDF output files
   //

if (units.empty()) units = na_str;

   //
   //  done
   //

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

index_a = index_b = index_c = mtab_set = mtab_low = mtab_high = cntr = ltab = -1;

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
mtab_high = e.mtab_high;
mtab_low = e.mtab_low;
mtab_set = e.mtab_set;
cntr = e.cntr;
ltab = e.ltab;

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
              << mtab_set << ", "
              << mtab_low << ", "
              << mtab_high << ", "
              << cntr << ", "
              << ltab << ", "
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

int j;
StringArray tok;

   //
   //  grab the first 8 whitespace separated integers
   //

tok.parse_wsss(line);

if (tok.n_elements() < 8) return (false);

for (j=0; j<8; ++j) {
   if(!is_number(tok[j].c_str())) return (false);
}

index_a   = atoi(tok[0].c_str());
mtab_set  = atoi(tok[1].c_str());
mtab_low  = atoi(tok[2].c_str());
mtab_high = atoi(tok[3].c_str());
cntr      = atoi(tok[4].c_str());
ltab      = atoi(tok[5].c_str());
index_b   = atoi(tok[6].c_str());
index_c   = atoi(tok[7].c_str());

   //
   //  grab the 3 strings separated by double quotes
   //

tok.parse_delim(line, "\"");

if (tok.n_elements() < 6) return (false);

parm_name = tok[1];
full_name = tok[3];
units     = tok[5];  // may be empty

if (units == "\n" || units == "\n\n") units.clear();

   //
   // if empty, set to the NA string to avoid a runtime error writing
   // units attribute to NetCDF output files
   //

if (units.empty()) units = na_str;

   //
   //  done
   //

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


TableFlatFile::TableFlatFile(int) {

   init_from_scratch();

   ConcatString path;
   StringArray filtered_file_names;

   //
   //  read user-specified GRIB1 tables followed by default tables
   //
   readUserGribTables("grib1");

   path = replace_path(table_data_dir);

   filtered_file_names = get_filenames(path, "grib1", ".txt", true);

   //
   //  read the default grib1 table file, expanding MET_BASE
   //
   for (int i = 0; i < filtered_file_names.n_elements(); i++) {
      if (!read(filtered_file_names[i].c_str())) {
         mlog << Error << "\nTableFlatFile::TableFlatFile(int) -> "
              << "unable to read table file \"" << filtered_file_names[i]
              << "\"\n\n";
         exit(1);
      }
   }

   //
   //  read user-specified GRIB2 tables followed by default tables
   //
   readUserGribTables("grib2");

   filtered_file_names.clear();

   filtered_file_names = get_filenames(path, "grib2", ".txt", true);

   for (int i = 0; i < filtered_file_names.n_elements(); i++)
   {
      if (!read(filtered_file_names[i].c_str())) {
         mlog << Error << "\nTableFlatFile::TableFlatFile(int) ->"
              << "unable to read table file \"" << filtered_file_names[i]
              << "\"\n\n";
         exit(1);
      }
   }

   //
   //  done
   //

}


////////////////////////////////////////////////////////////////////////


void TableFlatFile::readUserGribTables(const char * table_type) {
   ConcatString path;
   StringArray filtered_file_names;

   //
   // search for MET_GRIB_TABLES environment variable
   // if not defined, try the older USER_GRIB_TABLES ones
   //
   if(!get_env(met_grib_tables, path)) get_env(user_grib_tables, path);

   if(!path.empty()) {

      filtered_file_names = get_filenames(path, table_type, ".txt", true);

      for (int i = 0; i < filtered_file_names.n_elements(); i++) {

         //
         // write to cout since mlog may not have been constructed yet
         //
         cout << "DEBUG 1: Reading user-defined " << table_type << " "
              << met_grib_tables << " file: " << filtered_file_names[i]
              << "\n";

         if (!read(filtered_file_names[i].c_str())) {
            mlog << Error << "\nTableFlatFile::readUserGribTables() -> "
                 << "unable to read user-defined " << table_type
                 << " table file \"" << filtered_file_names[i]
                 << "\"\n\n";
            exit(1);
         }
      }
   }
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

g1e = (Grib1TableEntry **) 0;
g2e = (Grib2TableEntry **) 0;

clear();

}


////////////////////////////////////////////////////////////////////////


void TableFlatFile::clear()

{

int j;

if ( g1e )  {

   for (j=0; j<N_grib1_elements; ++j)  {

      if ( g1e[j] )  { delete g1e[j];  g1e[j] = (Grib1TableEntry *) 0; }

   }

   delete [] g1e; g1e = (Grib1TableEntry **) 0;

}


if ( g2e )  {

   for (j=0; j<N_grib2_elements; ++j)  {

      if ( g2e[j] )  { delete g2e[j];  g2e[j] = (Grib2TableEntry *) 0; }

   }

   delete [] g2e; g2e = (Grib2TableEntry* *) 0;

}

N_grib1_elements = 0;
N_grib2_elements = 0;

N_grib1_alloc = 0;
N_grib2_alloc = 0;

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

   g1e[j]->dump(out, depth + 1);

}

out << prefix << "N_grib2_elements = " << N_grib2_elements << "\n";

for (j=0; j<N_grib2_elements; ++j)  {

   out << prefix << "Grib2 Element # " << j << " ...\n";

   g2e[j]->dump(out, depth + 1);

}


return;

}


////////////////////////////////////////////////////////////////////////


void TableFlatFile::assign(const TableFlatFile & f)

{

clear();

int j;


if ( f.N_grib1_elements != 0 )  {

   N_grib1_elements = N_grib1_alloc = f.N_grib1_elements;

   g1e = new Grib1TableEntry * [N_grib1_elements];

   for (j=0; j<N_grib1_elements; ++j)  {

      g1e[j] = new Grib1TableEntry;

      *(g1e[j]) = *(f.g1e[j]);

   }

}

if ( f.N_grib2_elements != 0 )  {

   N_grib2_elements = N_grib2_alloc = f.N_grib2_elements;

   g2e = new Grib2TableEntry * [N_grib2_elements];

   for (j=0; j<N_grib2_elements; ++j)  {

      g2e[j] = new Grib2TableEntry;

      *(g2e[j]) = *(f.g2e[j]);

   }

}

return;

}


////////////////////////////////////////////////////////////////////////


void TableFlatFile::extend_grib1(int n)

{

if ( n <= N_grib1_alloc )  return;

int j;
Grib1TableEntry ** u = (Grib1TableEntry **) 0;

u = new Grib1TableEntry * [n];

for (j=0; j<n; ++j)  u[j] = (Grib1TableEntry *) 0;

if ( N_grib1_elements > 0 )  {

   for (j=0; j<N_grib1_elements; ++j)  u[j] = g1e[j];

   delete [] g1e;  g1e = (Grib1TableEntry **) 0;

}

g1e = u;

u = (Grib1TableEntry **) 0;

   //
   //  done
   //

N_grib1_alloc = n;

return;

}


////////////////////////////////////////////////////////////////////////


void TableFlatFile::extend_grib2(int n)

{

if ( n <= N_grib2_alloc )  return;

int j;
Grib2TableEntry ** u = (Grib2TableEntry **) 0;

u = new Grib2TableEntry * [n];

for (j=0; j<n; ++j)  u[j] = (Grib2TableEntry *) 0;

if ( N_grib2_elements > 0 )  {

   for (j=0; j<N_grib2_elements; ++j)  u[j] = g2e[j];

   delete [] g2e;  g2e = (Grib2TableEntry **) 0;

}

g2e = u;

u = (Grib2TableEntry **) 0;

   //
   //  done
   //

N_grib2_alloc = n;

return;

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::read(const char * filename)

{

ifstream in;
ConcatString line;
int n_lines;

if ( empty(filename) )  {

   mlog << Error << "\nTableFlatFile::read(const char *) ->"
        << "empty filename!\n\n";

   exit ( 1 );

}

   //
   //  add one in case there is no trailing new line
   //

n_lines = file_linecount(filename) + 1;

met_open(in, filename);

if ( !in )  {

   mlog << Error << "\nTableFlatFile::read(const char *) -> "
        << "unable to open input file \"" << filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  get first line for format
   //

line.read_line(in);

line.chomp('\n');

line.ws_strip();

     if ( line == "GRIB1" )  { return ( read_grib1(in, filename, n_lines - 1) ); }
else if ( line == "GRIB2" )  { return ( read_grib2(in, filename, n_lines - 1) ); }
else {

   mlog << Error << "\nTableFlatFile::read(const char *) -> "
        << "unable unrecognized format spec \""
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


bool TableFlatFile::read_grib1(istream & in, const char * filename, const int n)

{

int j;
ConcatString line;
bool status = false;

   //
   //  make room for the new elements
   //

extend_grib1(N_grib1_elements + n);

   //
   //  read the new elements
   //

j = 0;

while ( line.read_line(in) )  {

   //
   //  skip blank lines
   //

   if ( line.empty() )  continue;

   g1e[N_grib1_elements + j] = new Grib1TableEntry;

   //
   //  add newline in case it is missing from the last line of the file
   //

   line << "\n";

   status = g1e[N_grib1_elements + j]->parse_line(line.c_str());

   if ( ! status )  {

      mlog << Error << "\nTableFlatFile::read_grib1(istream &) -> "
           << "trouble parsing line number " << j+2 << " from input file \""
           << filename << "\"\n\n";

      exit ( 1 );

   }

   //
   //  increment counter
   //

   j++;

}  //  while

   //
   //  increment the number of elements
   //

N_grib1_elements += j;

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::read_grib2(istream & in, const char * filename, const int n)

{

int j;
ConcatString line;
bool status = false;

   //
   //  make room for the new elements
   //

extend_grib2(N_grib2_elements + n);

   //
   //  read the new elements
   //

j = 0;

while ( line.read_line(in) )  {

   //
   //  skip blank lines
   //

   if ( line.empty() )  continue;

   //
   //  add newline in case it is missing from the last line of the file
   //

   line << "\n";

   g2e[N_grib2_elements + j] = new Grib2TableEntry;

   status = g2e[N_grib2_elements + j]->parse_line(line.c_str());

   if ( ! status )  {

      mlog << Error << "\nTableFlatFile::read_grib2(istream &) -> "
           << "trouble parsing line number " << j+2 << " from input file \""
           << filename << "\"\n\n";

      exit ( 1 );

   }

   //
   //  increment counter
   //

   j++;

}  //  while

   //
   //  done
   //

N_grib2_elements += j;

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::lookup_grib1(int code, int table_number, Grib1TableEntry & e)

{

int j;

e.clear();

for (j=0; j<N_grib1_elements; ++j)  {

   if ( (g1e[j]->code == code) && (g1e[j]->table_number == table_number) )  {

      e = *(g1e[j]);

      return ( true );

   }

}

return ( false );

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::lookup_grib1(int code, int table_number, int center, int subcenter, Grib1TableEntry & e)

{
   int j;

   e.clear();
   int matching_subsenter;

   for (j=0; j<N_grib1_elements; ++j)  {

      matching_subsenter = subcenter;
      if( g1e[j]->subcenter == -1){
         matching_subsenter = -1;
      }

      if ( (g1e[j]->code == code) && (g1e[j]->table_number == table_number)
              && (g1e[j]->center == center)  && (g1e[j]->subcenter == matching_subsenter))  {

         e = *(g1e[j]);

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


bool TableFlatFile::lookup_grib1(const char * parm_name, int table_number, int code,
                                 Grib1TableEntry & e, int & n_matches)

{

   //  clear the by-reference arguments
   e.clear();
   n_matches = 0;

   //  build a list of matches
   vector<Grib1TableEntry*> matches;
   for(int j=0; j < N_grib1_elements; j++){

      if( g1e[j]->parm_name != parm_name ||
          (bad_data_int != table_number && g1e[j]->table_number != table_number) ||
          (bad_data_int != code         && g1e[j]->code         != code        ) )
         continue;

      if( n_matches++ == 0 ) e = *(g1e[j]);
      matches.push_back( g1e[j] );

   }

   //  if there are multiple matches, print a descriptive message
   if( 1 < n_matches ){

      ConcatString msg;
      msg << "Multiple GRIB1 table entries match lookup criteria ("
          << "parm_name = " << parm_name;
      if( bad_data_int != table_number ) msg << ", table_number = " << table_number;
      if( bad_data_int != code         ) msg << ", code = "         << code;
      msg << "):\n";
      mlog << Debug(3) << "\n" << msg;

      for(vector<Grib1TableEntry*>::iterator it = matches.begin();
          it < matches.end(); it++)
         mlog << Debug(3) << "  parm_name: "     << (*it)->parm_name
                          << ", table_number = " << (*it)->table_number
                          << ", code = "         << (*it)->code << "\n";

      mlog << Debug(3) << "Using the first match found: "
                       << "  parm_name: "     << e.parm_name
                       << ", table_number = " << e.table_number
                       << ", code = "         << e.code << "\n\n";

   }

   return (n_matches > 0);

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::lookup_grib1(const char * parm_name, int table_number, int code,int center, int subcenter,
                  Grib1TableEntry & e, int & n_matches)

{

   //  clear the by-reference arguments
   e.clear();
   n_matches = 0;

   //  build a list of matches
   vector<Grib1TableEntry*> matches;
   int matching_subsenter;
   for(int j=0; j < N_grib1_elements; j++){
      matching_subsenter = subcenter;
      if( g1e[j]->subcenter == -1){
         matching_subsenter = -1;
      }

      if( g1e[j]->parm_name != parm_name ||
          (bad_data_int != table_number && g1e[j]->table_number != table_number) ||
          (bad_data_int != code         && g1e[j]->code         != code        ) ||
          (bad_data_int != center         && g1e[j]->center != center        )   ||
          (bad_data_int != matching_subsenter         && g1e[j]->subcenter != matching_subsenter)   )
         continue;

      if( n_matches++ == 0 ) e = *(g1e[j]);
      matches.push_back( g1e[j] );

   }

   //  if there are multiple matches, print a descriptive message
   if( 1 < n_matches ){

      ConcatString msg;
      msg << "Multiple GRIB1 table entries match lookup criteria ("
      << "parm_name = " << parm_name;
      if( bad_data_int != table_number ) msg << ", table_number = " << table_number;
      if( bad_data_int != code         ) msg << ", code = "         << code;
      msg << "):\n";
      mlog << Debug(3) << "\n" << msg;

      for(vector<Grib1TableEntry*>::iterator it = matches.begin();
          it < matches.end(); it++)
      {
         mlog << Debug(3) << "  parm_name: "     << (*it)->parm_name
                          << ", table_number = " << (*it)->table_number
                          << ", code = "         << (*it)->code
                          << ", center = "       << (*it)->center
                          << ", subcenter = "    << (*it)->subcenter << "\n";
      }

      mlog << Debug(3) << "Using the first match found: "
                       << "  parm_name: "     << e.parm_name
                       << ", table_number = " << e.table_number
                       << ", code = "         << e.code << "\n\n";

   }

   return (n_matches > 0);

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::lookup_grib1(const char * parm_name, Grib1TableEntry & e)   //  assumes table number is 2

{

   int n_matches = -1;
   return lookup_grib1(parm_name, 2, bad_data_int, e, n_matches);

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::lookup_grib2(int a, int b, int c, Grib2TableEntry & e)

{

int j;

e.clear();

for (j=0; j<N_grib2_elements; ++j)  {

   if ( (g2e[j]->index_a == a) && (g2e[j]->index_b == b) && (g2e[j]->index_c == c) )  {

      e = *(g2e[j]);

      return ( true );

   }

}

return ( false );

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::lookup_grib2(int a, int b, int c,
                                 int mtab, int cntr, int ltab,
                                 Grib2TableEntry & e)

{
   int j;

   e.clear();

   for (j=0; j<N_grib2_elements; ++j)  {

      // Check discipline, parm_cat, and cat
      if ( g2e[j]->index_a != a ||
           g2e[j]->index_b != b ||
           g2e[j]->index_c != c ) continue;

      // Check master table, center, and local table
      if ( (bad_data_int != mtab && g2e[j]->mtab_low  > mtab) ||
           (bad_data_int != mtab && g2e[j]->mtab_high < mtab) ||
           (bad_data_int != cntr && g2e[j]->cntr > 0 && g2e[j]->cntr != cntr) ||
           (bad_data_int != ltab && g2e[j]->ltab > 0 && g2e[j]->ltab != ltab) ) continue;

      e = *(g2e[j]);

      return ( true );

   }

   return ( false );
}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::lookup_grib2(const char * parm_name, int a, int b, int c,
                                 Grib2TableEntry & e, int & n_matches)

{

   //  clear the by-reference arguments
   e.clear();
   n_matches = 0;

   //  build a list of matches
   vector<Grib2TableEntry*> matches;
   for(int j=0; j<N_grib2_elements; ++j){

      if( g2e[j]->parm_name != parm_name ||
          (bad_data_int != a && g2e[j]->index_a != a) ||
          (bad_data_int != b && g2e[j]->index_b != b) ||
          (bad_data_int != c && g2e[j]->index_c != c) )
         continue;

      if( n_matches++ == 0 ) e = *(g2e[j]);
      matches.push_back( g2e[j] );

   }

   //  if there are multiple matches, print a descriptive message
   if( 1 < n_matches ){

      ConcatString msg;
      msg << "Multiple GRIB2 table entries match lookup criteria ("
          << "parm_name = " << parm_name;
      if( bad_data_int != a ) msg << ", index_a = " << a;
      if( bad_data_int != b ) msg << ", index_b = " << b;
      if( bad_data_int != c ) msg << ", index_c = " << c;
      msg << "):\n";
      mlog << Debug(3) << "\n" << msg;

      for(vector<Grib2TableEntry*>::iterator it = matches.begin();
          it < matches.end(); it++)
         mlog << Debug(3) << "  parm_name: " << (*it)->parm_name
                          << ", index_a = "  << (*it)->index_a
                          << ", index_b = "  << (*it)->index_b
                          << ", index_c = "  << (*it)->index_c << "\n";

      mlog << Debug(3) << "Using the first match found: "
                       << "  parm_name: " << e.parm_name
                       << ", index_a = "  << e.index_a
                       << ", index_b = "  << e.index_b
                       << ", index_c = "  << e.index_c << "\n\n";

   }

   return (n_matches > 0);

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::lookup_grib2(const char * parm_name,
                                 int a, int b, int c,
                                 int mtab, int cntr, int ltab,
                                 Grib2TableEntry & e, int & n_matches)

{
   //  clear the by-reference arguments
   e.clear();
   n_matches = 0;

   //  build a list of matches
   vector<Grib2TableEntry*> matches;
   for(int j=0; j<N_grib2_elements; ++j){

      if( g2e[j]->parm_name != parm_name ||
          (bad_data_int != a    && g2e[j]->index_a != a) ||
          (bad_data_int != b    && g2e[j]->index_b != b) ||
          (bad_data_int != c    && g2e[j]->index_c != c) ||
          (bad_data_int != mtab && g2e[j]->mtab_low  > mtab) ||
          (bad_data_int != mtab && g2e[j]->mtab_high < mtab) ||
          (bad_data_int != cntr && g2e[j]->cntr > 0 && g2e[j]->cntr != cntr) ||
          (bad_data_int != ltab && g2e[j]->ltab > 0 && g2e[j]->ltab != ltab) )
         continue;

      if( n_matches++ == 0 ) e = *(g2e[j]);
      matches.push_back( g2e[j] );

   }

   //  if there are multiple matches, print a descriptive message
   if( 1 < n_matches ){

      ConcatString msg;
      msg << "Multiple GRIB2 table entries match lookup criteria ("
      << "parm_name = " << parm_name;
      if( bad_data_int != a ) msg << ", index_a = " << a;
      if( bad_data_int != mtab ) msg << ", grib2_mtab = " << mtab;
      if( bad_data_int != cntr ) msg << ", grib2_cntr = " << cntr;
      if( bad_data_int != ltab ) msg << ", grib2_ltab = " << ltab;
      if( bad_data_int != b ) msg << ", index_b = " << b;
      if( bad_data_int != c ) msg << ", index_c = " << c;
      msg << "):\n";
      mlog << Debug(3) << "\n" << msg;

      for(vector<Grib2TableEntry*>::iterator it = matches.begin();
          it < matches.end(); it++)
         mlog << Debug(3) << "  parm_name: "   << (*it)->parm_name
                          << ", index_a = "    << (*it)->index_a
                          << ", grib2_mtab = " << (*it)->mtab_set
                          << ", grib2_cntr = " << (*it)->cntr
                          << ", grib2_ltab = " << (*it)->ltab
                          << ", index_b = "    << (*it)->index_b
                          << ", index_c = "    << (*it)->index_c
                          << "\n";

      mlog << Debug(3) << "Using the first match found: "
                       << "  parm_name: "   << e.parm_name
                       << ", index_a = "    << e.index_a
                       << ", grib2_mtab = " << e.mtab_set
                       << ", grib2_cntr = " << e.cntr
                       << ", grib2_ltab = " << e.ltab
                       << ", index_b = "    << e.index_b
                       << ", index_c = "    << e.index_c
                       << "\n\n";

   }

   return (n_matches > 0);
}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::lookup_grib2(const char * parm_name, Grib2TableEntry & e, int & n_matches)

{

   return lookup_grib2(parm_name, bad_data_int, bad_data_int, bad_data_int, e, n_matches);

}


////////////////////////////////////////////////////////////////////////
