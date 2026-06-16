// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


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

using namespace std;


////////////////////////////////////////////////////////////////////////


   //
   //  This needs external linkage
   //

TableFlatFile GribTable (0);


////////////////////////////////////////////////////////////////////////


constexpr char table_data_dir  [] = "MET_BASE/table_files"; //  relative to MET_BASE
constexpr char met_grib_tables [] = "MET_GRIB_TABLES";      //  environment variable name
constexpr char user_grib_tables[] = "USER_GRIB_TABLES";     //  deprecated environment variable name


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

if ( this == &e )  return *this;

assign(e);

return *this;

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

StringArray tok;

   //
   //  grab the first 4 whitespace separated integers
   //

tok.parse_wsss(line);

if (tok.n_elements() < 4) return false;

for (int j=0; j<4; ++j) {
   if(!is_number(tok[j].c_str())) return false;
}

code         = atoi(tok[0].c_str());
table_number = atoi(tok[1].c_str());
center       = atoi(tok[2].c_str());
subcenter    = atoi(tok[3].c_str());

   //
   //  grab the 3 strings separated by double quotes
   //

tok.parse_delim(line, "\"");

if (tok.n_elements() < 6) return false;

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

return true;

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

if ( this == &e )  return *this;

assign(e);

return *this;

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

disc = pcat = pnum = mtab_set = mtab_low = mtab_high = cntr = ltab = -1;

parm_name.clear();

full_name.clear();

units.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Grib2TableEntry::assign(const Grib2TableEntry & e)

{

clear();

disc = e.disc;
pcat = e.pcat;
pnum = e.pnum;
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
              << disc << ", "
              << mtab_set << ", "
              << mtab_low << ", "
              << mtab_high << ", "
              << cntr << ", "
              << ltab << ", "
              << pcat << ", "
              << pnum << ")\n";

out << prefix << "parm_name = " << parm_name.contents() << "\n";

out << prefix << "full_name = " << full_name.contents() << "\n";

out << prefix << "units     = " << units.contents()     << "\n";

return;

}


////////////////////////////////////////////////////////////////////////

bool Grib2TableEntry::is_eq(const Grib2TableEntry &e) const

{
   return (disc == e.disc) &&
          (pcat == e.pcat) &&
          (pnum == e.pnum) &&
          (parm_name == e.parm_name) &&
          (mtab_set == e.mtab_set) &&
          (mtab_low == e.mtab_low) &&
          (mtab_high == e.mtab_high) &&
          (cntr == e.cntr) &&
          (ltab == e.ltab);
}


////////////////////////////////////////////////////////////////////////

GribEntryMatch Grib2TableEntry::match(const int &mtab, const int &_cntr, const int &_ltab) const

{

   GribEntryMatch status = GribEntryMatch::not_match;

   if (bad_data_int == mtab && bad_data_int == _cntr && bad_data_int == _ltab) {
      status = GribEntryMatch::match;
   }
   // Check master table
   else if (bad_data_int == mtab || mtab_low <= mtab && mtab_high >= mtab) {
      if (bad_data_int != _cntr && cntr == _cntr &&
          bad_data_int != _ltab && ltab == _ltab) status = GribEntryMatch::exact_match;
      else if (cntr == 0 && ltab == 0) status = GribEntryMatch::match;
   }

   return status;

}


////////////////////////////////////////////////////////////////////////


bool Grib2TableEntry::parse_line(const char * line)

{

clear();

StringArray tok;

   //
   //  grab the first 8 whitespace separated integers
   //

tok.parse_wsss(line);

if (tok.n_elements() < 8) return false;

for (int j=0; j<8; ++j) {
   if(!is_number(tok[j].c_str())) return false;
}

disc      = atoi(tok[0].c_str());
mtab_set  = atoi(tok[1].c_str());
mtab_low  = atoi(tok[2].c_str());
mtab_high = atoi(tok[3].c_str());
cntr      = atoi(tok[4].c_str());
ltab      = atoi(tok[5].c_str());
pcat      = atoi(tok[6].c_str());
pnum      = atoi(tok[7].c_str());

   //
   //  grab the 3 strings separated by double quotes
   //

tok.parse_delim(line, "\"");

if (tok.n_elements() < 6) return false;

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

return true;

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

   filtered_file_names = get_filenames(path, "^grib1", ".txt$", true);

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

   filtered_file_names = get_filenames(path, "^grib2", ".txt$", true);

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

      ConcatString prefix_reg_exp;
      prefix_reg_exp << "^" << table_type;

      filtered_file_names = get_filenames(path, prefix_reg_exp.c_str(),
                                          ".txt$", true);

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

clear();

}


////////////////////////////////////////////////////////////////////////


void TableFlatFile::clear()

{

g1e.clear();
g2e.clear();

N_grib1_elements = 0;
N_grib2_elements = 0;

N_grib1_alloc = 0;
N_grib2_alloc = 0;

return;

}

////////////////////////////////////////////////////////////////////////


void TableFlatFile::dump(ostream & out, int depth) const

{

Indent prefix(depth);


out << prefix << "N_grib1_elements = " << N_grib1_elements << "\n";

for (int j=0; j<N_grib1_elements; ++j)  {

   out << prefix << "Grib1 Element # " << j << " ...\n";

   g1e[j].dump(out, depth + 1);

}

out << prefix << "N_grib2_elements = " << N_grib2_elements << "\n";

for (int j=0; j<N_grib2_elements; ++j)  {

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

   N_grib1_elements = N_grib1_alloc = f.N_grib1_elements;

   g1e = f.g1e;

}

if ( f.N_grib2_elements != 0 )  {

   N_grib2_elements = N_grib2_alloc = f.N_grib2_elements;

   g2e = f.g2e;

}

return;

}


////////////////////////////////////////////////////////////////////////


void TableFlatFile::extend_grib1(int n)

{

if ( n <= N_grib1_alloc )  return;

g1e.reserve(n);

N_grib1_alloc = n;

return;

}


////////////////////////////////////////////////////////////////////////


void TableFlatFile::extend_grib2(int n)

{

if ( n <= N_grib2_alloc )  return;

g2e.reserve(n);

N_grib2_alloc = n;

return;

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::is_new_entry(const vector<Grib2TableEntry> &matches,
                                 const Grib2TableEntry & e) const

{

bool status = true;
for (auto &e_tmp : matches) {
  if( e.is_eq(e_tmp) ) {
     status = false;
     break;
  }
}
return status;

}


////////////////////////////////////////////////////////////////////////

ConcatString TableFlatFile::log_arguments(const char * parm_name,
                                          int disc, int pcat, int pnum,
                                          int mtab, int cntr, int ltab) const
{

ConcatString msg;
msg << "parm_name = " << parm_name;
if( bad_data_int != disc ) msg << ", disc = " << disc;
if( bad_data_int != mtab ) msg << ", grib2_mtab = " << mtab;
if( bad_data_int != cntr ) msg << ", grib2_cntr = " << cntr;
if( bad_data_int != ltab ) msg << ", grib2_ltab = " << ltab;
if( bad_data_int != pcat ) msg << ", pcat = " << pcat;
if( bad_data_int != pnum ) msg << ", pnum = " << pnum;

return msg;

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::read(const char * filename)

{

ifstream in;
ConcatString line;
int n_lines;
bool status = false;
const char *method_name = "TableFlatFile::read(const char *) -> ";

if ( empty(filename) )  {

   mlog << Error << "\n" << method_name
        << "empty filename!\n\n";

   exit ( 1 );

}

   //
   //  add one in case there is no trailing new line
   //

n_lines = file_linecount(filename) + 1;

met_open(in, filename);

if ( !in )  {

   mlog << Error << "\n" << method_name
        << "unable to open input file \"" << filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  get first line for format
   //

line.read_line(in);

line.chomp('\n');

line.ws_strip();

     if ( line == "GRIB1" ) status = read_grib1(in, filename, n_lines - 1);
else if ( line == "GRIB2" ) status = read_grib2(in, filename, n_lines - 1);
else {

   mlog << Error << "\n" << method_name
        << "unable unrecognized format spec \""
        << line << "\" in file \"" << filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  done
   //

in.close();

return status;

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::read_grib1(istream & in, const char * filename, const int n)

{

int j;
ConcatString line;
Grib1TableEntry e;
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

   //
   //  add newline in case it is missing from the last line of the file
   //

   line << "\n";

   status = e.parse_line(line.c_str());

   if ( ! status )  {

      mlog << Error << "\nTableFlatFile::read_grib1(istream &) -> "
           << "trouble parsing line number " << j+2 << " from input file \""
           << filename << "\"\n\n";

      exit ( 1 );

   }

   //
   //  store entry and increment counter
   //

   g1e.emplace_back(e);

   j++;

}  //  while

   //
   //  increment the number of elements
   //

N_grib1_elements += j;

   //
   //  done
   //

return true;

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::read_grib2(istream & in, const char * filename, const int n)

{

int j;
ConcatString line;
Grib2TableEntry e;
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

   status = e.parse_line(line.c_str());

   if ( ! status )  {

      mlog << Error << "\nTableFlatFile::read_grib2(istream &) -> "
           << "trouble parsing line number " << j+2 << " from input file \""
           << filename << "\"\n\n";

      exit ( 1 );

   }

   //
   //  store entry and increment counter
   //

   g2e.emplace_back(e);

   j++;

}  //  while

   //
   //  done
   //

N_grib2_elements += j;

return true;

}

////////////////////////////////////////////////////////////////////////

int TableFlatFile::lookup_grib1(int code, int table_number,
                                vector<Grib1TableEntry> &matches) {
   matches.clear();

   for(const auto &e : g1e) {
      if((e.code         == code        ) &&
         (e.table_number == table_number)) {
         matches.emplace_back(e);
      }
   }

   return (int) matches.size();
}

////////////////////////////////////////////////////////////////////////

int TableFlatFile::lookup_grib1(int code, int table_number, int center, int subcenter,
                                vector<Grib1TableEntry> &matches) {
   matches.clear();

   for(const auto &e : g1e) {
      if((e.code         == code        ) &&
         (e.table_number == table_number) &&
         (e.center       == center      ) &&
         (e.subcenter    == -1 ||
	  e.subcenter    == subcenter)) {
         matches.emplace_back(e);
      }
   }

   return (int) matches.size();
}

////////////////////////////////////////////////////////////////////////

int TableFlatFile::lookup_grib1(int code,
                                vector<Grib1TableEntry> &matches) {

   // Assume default table_number = 2
   return lookup_grib1(code, 2, matches);

}

////////////////////////////////////////////////////////////////////////

int TableFlatFile::lookup_grib1(const char * parm_name, int table_number, int code,
                                vector<Grib1TableEntry> &matches) {
   matches.clear();

   for(const auto &e : g1e) {
      if((e.parm_name == parm_name                                   ) &&
         (is_bad_data(table_number) || e.table_number == table_number) &&
         (is_bad_data(code)         || e.code         == code        )) {
         matches.emplace_back(e);
      }
   }

   return (int) matches.size();
}

////////////////////////////////////////////////////////////////////////

int TableFlatFile::lookup_grib1(const char *parm_name, int table_number,
                                int code,int center, int subcenter,
                                vector<Grib1TableEntry> &matches) {
   matches.clear();

   for(const auto &e : g1e) {
      if((e.parm_name == parm_name                                   ) &&
         (is_bad_data(table_number) || e.table_number == table_number) &&
         (is_bad_data(code)         || e.code         == code        ) &&
         (is_bad_data(center)       || e.center       == center      ) &&
         (e.subcenter == -1         ||
          is_bad_data(subcenter)    || e.subcenter    == subcenter   )) {
         matches.emplace_back(e);
      }
   }

   return (int) matches.size();
}

////////////////////////////////////////////////////////////////////////

int TableFlatFile::lookup_grib1(const char * parm_name,
                                vector<Grib1TableEntry> &matches) {

   // Assume default table_number = 2
   return lookup_grib1(parm_name, 2, bad_data_int, matches);
}

////////////////////////////////////////////////////////////////////////

int TableFlatFile::lookup_grib2(int disc, int pcat, int pnum,
                                vector<Grib2TableEntry> &matches) {
   matches.clear();

   for(const auto &e : g2e) {
      if((e.disc == disc) &&
         (e.pcat == pcat) &&
         (e.pnum == pnum)) {
         matches.emplace_back(e);
      }
   }

   return (int) matches.size();
}

////////////////////////////////////////////////////////////////////////

int TableFlatFile::lookup_grib2(int disc, int pcat, int pnum,
                                int mtab, int cntr, int ltab,
                                vector<Grib2TableEntry> &matches) {
   vector<Grib2TableEntry> exact_matches;
   vector<Grib2TableEntry> partial_matches;

   for(const auto &e : g2e) {

      // Check discipline, parm_cat, and cat
      if((e.disc != disc) ||
         (e.pcat != pcat) ||
         (e.pnum != pnum)) continue;

      GribEntryMatch status = e.match(mtab, cntr, ltab);

      if(status == GribEntryMatch::exact_match) {
         exact_matches.emplace_back(e);
      }
      else if(status == GribEntryMatch::match) {
         partial_matches.emplace_back(e);
      }
   }

   // Prefer exact matches
   if(!exact_matches.empty()) matches = exact_matches;
   else                       matches = partial_matches;

   return (int) matches.size();
}

////////////////////////////////////////////////////////////////////////

int TableFlatFile::lookup_grib2(const char * parm_name,
                                int disc, int pcat, int pnum,
                                vector<Grib2TableEntry> &matches) {
   matches.clear();

   for(const auto &e : g2e) {

      if((e.parm_name == parm_name           ) &&
         (is_bad_data(disc) || e.disc == disc) &&
         (is_bad_data(pcat) || e.pcat == pcat) &&
         (is_bad_data(pnum) || e.pnum == pnum) &&
         is_new_entry(matches, e)) {
         matches.emplace_back(e);
      }
   }

   return (int) matches.size();
}

////////////////////////////////////////////////////////////////////////

int TableFlatFile::lookup_grib2(const char * parm_name,
                                int disc, int pcat, int pnum,
                                int mtab, int cntr, int ltab,
                                vector<Grib2TableEntry> &matches) {
   vector<Grib2TableEntry> exact_matches;
   vector<Grib2TableEntry> partial_matches;

   for(const auto &e : g2e) {

      if((e.parm_name == parm_name           ) &&
         (is_bad_data(disc) || e.disc == disc) &&
         (is_bad_data(pcat) || e.pcat == pcat) &&
         (is_bad_data(pnum) || e.pnum == pnum)) {

         GribEntryMatch status = e.match(mtab, cntr, ltab);

         if(status == GribEntryMatch::exact_match &&
            is_new_entry(matches, e)) {
            exact_matches.emplace_back(e);
         }

         if(status == GribEntryMatch::match &&
            is_new_entry(partial_matches, e)) {
            partial_matches.emplace_back(e);
         }
      }
   }

   // Prefer exact matches
   if(!exact_matches.empty()) matches = exact_matches;
   else                       matches = partial_matches;

   return (int) matches.size();
}

////////////////////////////////////////////////////////////////////////

int TableFlatFile::lookup_grib2(const char * parm_name,
                                vector<Grib2TableEntry> &matches) {
   return lookup_grib2(parm_name, bad_data_int, bad_data_int, bad_data_int,
                       matches);
}

////////////////////////////////////////////////////////////////////////
