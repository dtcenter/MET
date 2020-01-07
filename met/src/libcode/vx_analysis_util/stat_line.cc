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

#include "stat_columns.h"
#include "stat_line.h"
#include "analysis_utils.h"

#include "vx_util.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class STATLine
   //


////////////////////////////////////////////////////////////////////////


STATLine::STATLine()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


STATLine::~STATLine()

{

clear();

}


////////////////////////////////////////////////////////////////////////


STATLine::STATLine(const STATLine & L)

{

init_from_scratch();

assign(L);

return;

}


////////////////////////////////////////////////////////////////////////


STATLine & STATLine::operator=(const STATLine & L)

{

if ( this == &L )  return ( * this );

assign(L);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void STATLine::init_from_scratch()

{

DataLine::init_from_scratch();

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void STATLine::assign(const STATLine &l)

{

DataLine::assign(l);

Type    = l.Type;
HdrLine = l.HdrLine;

return;

}


////////////////////////////////////////////////////////////////////////


void STATLine::clear()

{

DataLine::clear();

Type    = no_stat_line_type;
HdrLine = (AsciiHeaderLine *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void STATLine::dump(ostream & out, int depth) const

{

Indent prefix(depth);
ThreshArray ta;


DataLine::dump(out, depth);

out << prefix << "\n";

out << prefix << "HdrLine        = "   << HdrLine       << "\n";

out << prefix << "Version        = "   << version()     << "\n";
out << prefix << "Model          = \"" << model()       << "\"\n";
out << prefix << "Description    = \"" << desc()        << "\"\n";

out << prefix << "Fcst Lead      = "   << fcst_lead()
    << "  ( " << sec_to_hhmmss(fcst_lead()) << " )\n";

out << prefix << "Fcst Valid Beg = "   << fcst_valid_beg()
    << "  ( " << unix_to_yyyymmdd_hhmmss(fcst_valid_beg()) << " )\n";

out << prefix << "Fcst Valid End = "   << fcst_valid_end()
    << "  ( " << unix_to_yyyymmdd_hhmmss(fcst_valid_end()) << " )\n";

out << prefix << "Obs Lead       = "   << obs_lead()
    << "  ( " << sec_to_hhmmss(obs_lead()) << " )\n";

out << prefix << "Obs Valid Beg  = "   << obs_valid_beg()
    << "  ( " << unix_to_yyyymmdd_hhmmss(obs_valid_beg()) << " )\n";

out << prefix << "Obs Valid End  = "   << obs_valid_end()
    << "  ( " << unix_to_yyyymmdd_hhmmss(obs_valid_end()) << " )\n";

out << prefix << "Fcst Var       = \"" << fcst_var()   << "\"\n";
out << prefix << "Fcst Units     = \"" << fcst_units() << "\"\n";
out << prefix << "Fcst Level     = \"" << fcst_lev()   << "\"\n";

out << prefix << "Obs Var        = \"" << obs_var()    << "\"\n";
out << prefix << "Obs Units      = \"" << obs_units()  << "\"\n";
out << prefix << "Obs Level      = \"" << obs_lev()    << "\"\n";

out << prefix << "Obs Type       = \"" << obtype()     << "\"\n";
out << prefix << "Vx Mask        = \"" << vx_mask()    << "\"\n";

out << prefix << "Interp Mthd    = \"" << interp_mthd()<< "\"\n";
out << prefix << "Interp Pnts    = \"" << interp_pnts()<< "\"\n";

ta = fcst_thresh();
out << prefix << "Fcst Thresh    = \"" << ta.get_str() << "\"\n";

ta = obs_thresh();
out << prefix << "Obs Thresh     = \"" << ta.get_str() << "\"\n";

ta = cov_thresh();
out << prefix << "Cov Thresh     = \"" << ta.get_str() << "\"\n";

out << prefix << "Alpha          = \"" << alpha() << "\"\n";

out << prefix << "Line Type      = \"" << statlinetype_to_string(Type) << "\"\n";

out << prefix << "Fcst Init Beg  = "   << fcst_init_beg()
    << "  ( " << unix_to_yyyymmdd_hhmmss(fcst_init_beg()) << " )\n";

out << prefix << "Fcst Init End  = "   << fcst_init_end()
    << "  ( " << unix_to_yyyymmdd_hhmmss(fcst_init_end()) << " )\n";

out << prefix << "Obs Init Beg   = "   << obs_init_beg()
    << "  ( " << unix_to_yyyymmdd_hhmmss(obs_init_beg()) << " )\n";

out << prefix << "Obs Init End   = "   << obs_init_end()
    << "  ( " << unix_to_yyyymmdd_hhmmss(obs_init_end()) << " )\n";

out << prefix << "Fcst Init Hour = \"" << fcst_init_hour() << "\"\n";
out << prefix << "Obs Init Hour  = \"" << obs_init_hour() << "\"\n";

return;

}


////////////////////////////////////////////////////////////////////////


int STATLine::read_line(LineDataFile * ldf)

{

int status, offset;

clear();

status = DataLine::read_line(ldf);

//
// Check for bad read status or zero length
//

if ( !status || n_items() == 0 )  {

   clear();

   return ( 0 );

}

//
// Check for a header line
//

if ( strcmp(get_item(0), "VERSION") == 0 ) {

   Type = stat_header;

   return ( 1 );
}

//
// Determine the LINE_TYPE column offset
//

offset = METHdrTable.col_offset(get_item(0), "STAT", na_str, "LINE_TYPE");

if( is_bad_data(offset) || n_items() < (offset + 1) )  {

   Type = no_stat_line_type;

   return ( 0 );
}

//
// Load the matching header line and store the line type
//

HdrLine = METHdrTable.header(get_item(0), "STAT", get_item(offset));

Type = string_to_statlinetype(get_item(offset));

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


bool STATLine::is_ok() const

{

return ( DataLine::is_ok() );

}


////////////////////////////////////////////////////////////////////////


bool STATLine::is_header() const

{

return ( Type == stat_header );

}


////////////////////////////////////////////////////////////////////////


bool STATLine::has(const char *col_str) const

{

int offset = bad_data_int;
int dim = bad_data_int;

   //
   // Parse the variable length dimension
   //

if ( HdrLine->is_var_length() ) {
   dim = atoi( get_item(HdrLine->var_index_offset()) );
}

   //
   // Search for matching header column
   //

offset = HdrLine->col_offset(col_str, dim);

   //
   // If not found, check extra header columns
   //

if ( is_bad_data(offset) ) {
   if ( !get_file()->header().has(col_str, offset) ) offset = bad_data_int;
}

   //
   // Return whether a valid offset value was found
   //

return ( !is_bad_data(offset) );

}


////////////////////////////////////////////////////////////////////////


ConcatString STATLine::get(const char *col_str, bool check_na) const

{

  ConcatString cs = (string)get_item(col_str, check_na);

   //
   // If not found, check derivable timing columns
   //

if ( cs == bad_data_str )  {

   if ( strcasecmp(col_str, conf_key_fcst_init_beg) == 0 )
      cs = unix_to_yyyymmdd_hhmmss(fcst_init_beg());
   else if ( strcasecmp(col_str, conf_key_fcst_init_end) == 0 )
      cs = unix_to_yyyymmdd_hhmmss(fcst_init_end());
   else if ( strcasecmp(col_str, conf_key_fcst_init_hour) == 0 )
      cs = sec_to_hhmmss(fcst_init_hour());
   else if ( strcasecmp(col_str, conf_key_fcst_valid_hour) == 0 )
      cs = sec_to_hhmmss(fcst_valid_hour());
   else if ( strcasecmp(col_str, conf_key_obs_init_beg) == 0 )
      cs = unix_to_yyyymmdd_hhmmss(obs_init_beg());
   else if ( strcasecmp(col_str, conf_key_obs_init_end) == 0 )
      cs = unix_to_yyyymmdd_hhmmss(obs_init_beg());
   else if ( strcasecmp(col_str, conf_key_obs_init_hour) == 0 )
      cs = sec_to_hhmmss(obs_init_hour());
   else if ( strcasecmp(col_str, conf_key_obs_valid_hour) == 0 )
      cs = sec_to_hhmmss(obs_valid_hour());

}

return ( cs );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::get_item(const char *col_str, bool check_na) const

{

int offset = bad_data_int;
int dim = bad_data_int;

   //
   // Parse the variable length dimension
   //

if ( HdrLine->is_var_length() ) {
   dim = atoi( get_item(HdrLine->var_index_offset()) );
}

   //
   // Search for matching header column
   //

offset = HdrLine->col_offset(col_str, dim);

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


const char * STATLine::get_item(int offset, bool check_na) const

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


const char * STATLine::version() const

{

const char * c = get_item("VERSION", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::model() const

{

const char * c = get_item("MODEL", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::desc() const

{

const char * c = get_item("DESC", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


int STATLine::fcst_lead() const

{

int j;
const char * c = get_item("FCST_LEAD");

j = timestring_to_sec(c);

return ( j );

}


////////////////////////////////////////////////////////////////////////


unixtime STATLine::fcst_valid_beg() const

{

unixtime t;
const char * c = get_item("FCST_VALID_BEG");
t = timestring_to_unix(c);

return ( t );

}


////////////////////////////////////////////////////////////////////////


unixtime STATLine::fcst_valid_end() const

{

unixtime t;
const char * c = get_item("FCST_VALID_END");
t = timestring_to_unix(c);

return ( t );

}


////////////////////////////////////////////////////////////////////////


int STATLine::fcst_valid_hour() const

{

return ( unix_to_sec_of_day(fcst_valid_beg()) );

}


////////////////////////////////////////////////////////////////////////


int STATLine::obs_lead() const

{

int j;
const char * c = get_item("OBS_LEAD");

j = timestring_to_sec(c);

return ( j );

}


////////////////////////////////////////////////////////////////////////


unixtime STATLine::obs_valid_beg() const

{

unixtime t;
const char * c = get_item("OBS_VALID_BEG");
t = timestring_to_unix(c);

return ( t );

}


////////////////////////////////////////////////////////////////////////


unixtime STATLine::obs_valid_end() const

{

unixtime t;
const char * c = get_item("OBS_VALID_END");
t = timestring_to_unix(c);

return ( t );

}


////////////////////////////////////////////////////////////////////////


int STATLine::obs_valid_hour() const

{

return ( unix_to_sec_of_day(obs_valid_beg()) );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::fcst_var() const

{

const char * c = get_item("FCST_VAR", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::fcst_units() const

{

const char * c = get_item("FCST_UNITS", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::fcst_lev() const

{

const char * c = get_item("FCST_LEV", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::obs_var() const

{

const char * c = get_item("OBS_VAR", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::obs_units() const

{

const char * c = get_item("OBS_UNITS", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::obs_lev() const

{

const char * c = get_item("OBS_LEV", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::obtype() const

{

const char * c = get_item("OBTYPE", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::vx_mask() const

{

const char * c = get_item("VX_MASK", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::interp_mthd() const

{

const char * c = get_item("INTERP_MTHD", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


int STATLine::interp_pnts() const

{

int k;
const char * c = get_item("INTERP_PNTS", false);

k = atoi(c);

return ( k );

}


////////////////////////////////////////////////////////////////////////


ThreshArray STATLine::fcst_thresh() const

{

ThreshArray ta;

const char * c = get_item("FCST_THRESH", false);

ta.add_css(c);

return ( ta );

}


////////////////////////////////////////////////////////////////////////


ThreshArray STATLine::obs_thresh() const

{

ThreshArray ta;

const char * c = get_item("OBS_THRESH", false);

ta.add_css(c);

return ( ta );

}


////////////////////////////////////////////////////////////////////////


SetLogic STATLine::thresh_logic() const

{

SetLogic t = SetLogic_None;

ConcatString cs = (string)get_item("FCST_THRESH", false);

     if(cs.endswith(setlogic_symbol_union))        t = SetLogic_Union;
else if(cs.endswith(setlogic_symbol_intersection)) t = SetLogic_Intersection;
else if(cs.endswith(setlogic_symbol_symdiff))      t = SetLogic_SymDiff;
else                                               t = SetLogic_None;

return ( t );

}


////////////////////////////////////////////////////////////////////////


ThreshArray STATLine::cov_thresh() const

{

ThreshArray ta;

const char * c = get_item("COV_THRESH", false);

ta.add_css(c);

return ( ta );

}


////////////////////////////////////////////////////////////////////////


double STATLine::alpha() const

{

double a;

const char * c = get_item("ALPHA");

a = atof(c);

return ( a );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::line_type() const

{

const char * c = get_item("LINE_TYPE", false);

return ( c );

}


////////////////////////////////////////////////////////////////////////


unixtime STATLine::fcst_init_beg() const

{

int s;
unixtime t;

t = fcst_valid_beg();

s = fcst_lead();

t -= s;

return ( t );

}


////////////////////////////////////////////////////////////////////////


unixtime STATLine::fcst_init_end() const

{

int s;
unixtime t;

t = fcst_valid_end();

s = fcst_lead();

t -= s;

return ( t );

}


////////////////////////////////////////////////////////////////////////


int STATLine::fcst_init_hour() const

{

return ( unix_to_sec_of_day(fcst_init_beg()) );

}


////////////////////////////////////////////////////////////////////////


unixtime STATLine::obs_init_beg() const

{

int s;
unixtime t;

t = obs_valid_beg();

s = obs_lead();

t -= s;

return ( t );

}


////////////////////////////////////////////////////////////////////////


unixtime STATLine::obs_init_end() const

{

int s;
unixtime t;

t = obs_valid_end();

s = obs_lead();

t -= s;

return ( t );

}


////////////////////////////////////////////////////////////////////////


int STATLine::obs_init_hour() const

{

return ( unix_to_sec_of_day(obs_init_beg()) );

}


////////////////////////////////////////////////////////////////////////
