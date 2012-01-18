// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
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
#include "stat_offsets.h"
#include "analysis_utils.h"
#include "stat_offsets.h"

#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


static const char * suffix_list [] = {

   ".stat"

};

static const int n_suffixes = sizeof(suffix_list)/sizeof(*suffix_list);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class STATLine
   //


////////////////////////////////////////////////////////////////////////


STATLine::STATLine()

{


}


////////////////////////////////////////////////////////////////////////


STATLine::~STATLine()

{


}


////////////////////////////////////////////////////////////////////////


STATLine::STATLine(const STATLine & L)

{

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


void STATLine::dump(ostream & out, int depth) const

{

int month, day, year, hour, minute, second;
char junk[256];
Indent prefix(depth);
SingleThresh st;


DataLine::dump(out, depth);

out << prefix << "\n";

out << prefix << "Version        = "   << version()     << "\n";
out << prefix << "Model          = \"" << model()       << "\"\n";

sec_to_hms(fcst_lead(), hour, minute, second);
sprintf(junk, "%02d:%02d:%02d", hour, minute, second);
out << prefix << "Fcst Lead      = "   << fcst_lead()
    << "  ( " << junk << " )\n";

unix_to_mdyhms(fcst_valid_beg(),
               month, day, year, hour, minute, second);
sprintf(junk, "%s %d, %d  %02d:%02d:%02d",
        short_month_name[month], day, year, hour, minute, second);
out << prefix << "Fcst Valid Beg = "   << fcst_valid_beg()
    << "  ( " << junk << " )\n";

unix_to_mdyhms(fcst_valid_end(),
               month, day, year, hour, minute, second);
sprintf(junk, "%s %d, %d  %02d:%02d:%02d",
        short_month_name[month], day, year, hour, minute, second);
out << prefix << "Fcst Valid End = "   << fcst_valid_end()
    << "  ( " << junk << " )\n";

sec_to_hms(obs_lead(), hour, minute, second);
sprintf(junk, "%02d:%02d:%02d", hour, minute, second);
out << prefix << "Obs Lead       = "   << obs_lead()
    << "  ( " << junk << " )\n";

unix_to_mdyhms(obs_valid_beg(),
               month, day, year, hour, minute, second);
sprintf(junk, "%s %d, %d  %02d:%02d:%02d",
        short_month_name[month], day, year, hour, minute, second);
out << prefix << "Obs Valid Beg  = "   << obs_valid_beg()
    << "  ( " << junk << " )\n";

unix_to_mdyhms(obs_valid_end(),
               month, day, year, hour, minute, second);
sprintf(junk, "%s %d, %d  %02d:%02d:%02d",
        short_month_name[month], day, year, hour, minute, second);
out << prefix << "Obs Valid End  = "   << obs_valid_end()
    << "  ( " << junk << " )\n";

out << prefix << "Fcst Var       = \"" << fcst_var()   << "\"\n";
out << prefix << "Fcst Level     = \"" << fcst_lev()   << "\"\n";

out << prefix << "Obs Var        = \"" << obs_var()    << "\"\n";
out << prefix << "Obs Level      = \"" << obs_lev()    << "\"\n";

out << prefix << "Obs Type       = \"" << obtype()     << "\"\n";
out << prefix << "Vx Mask        = \"" << vx_mask()    << "\"\n";

out << prefix << "Interp Mthd    = \"" << interp_mthd()<< "\"\n";
out << prefix << "Interp Pnts    = \"" << interp_pnts()<< "\"\n";

st = fcst_thresh();
st.get_str(junk);
out << prefix << "Fcst Thresh    = \"" << junk << "\"\n";

st = obs_thresh();
st.get_str(junk);
out << prefix << "Obs Thresh     = \"" << junk << "\"\n";

st = cov_thresh();
st.get_str(junk);
out << prefix << "Cov Thresh     = \"" << junk << "\"\n";

out << prefix << "Alpha          = \"" << alpha() << "\"\n";

statlinetype_to_string(Type, junk);
out << prefix << "Line Type      = \"" << junk << "\"\n";

unix_to_mdyhms(fcst_init_beg(),
               month, day, year, hour, minute, second);
sprintf(junk, "%s %d, %d  %02d:%02d:%02d",
        short_month_name[month], day, year, hour, minute, second);
out << prefix << "Fcst Init Beg  = "   << fcst_init_beg()
    << "  ( " << junk << " )\n";

unix_to_mdyhms(fcst_init_end(),
               month, day, year, hour, minute, second);
sprintf(junk, "%s %d, %d  %02d:%02d:%02d",
        short_month_name[month], day, year, hour, minute, second);
out << prefix << "Fcst Init End  = "   << fcst_init_end()
    << "  ( " << junk << " )\n";

unix_to_mdyhms(obs_init_beg(),
               month, day, year, hour, minute, second);
sprintf(junk, "%s %d, %d  %02d:%02d:%02d",
        short_month_name[month], day, year, hour, minute, second);
out << prefix << "Obs Init Beg   = "   << obs_init_beg()
    << "  ( " << junk << " )\n";

unix_to_mdyhms(obs_init_end(),
               month, day, year, hour, minute, second);
sprintf(junk, "%s %d, %d  %02d:%02d:%02d",
        short_month_name[month], day, year, hour, minute, second);
out << prefix << "Obs Init End   = "   << obs_init_end()
    << "  ( " << junk << " )\n";

out << prefix << "Fcst Init Hour = \"" << fcst_init_hour() << "\"\n";
out << prefix << "Obs Init Hour  = \"" << obs_init_hour() << "\"\n";

return;

}


////////////////////////////////////////////////////////////////////////


int STATLine::read_line(LineDataFile * ldf)

{

int status;

status = DataLine::read_line(ldf);

if ( !status )  {

   clear();

   Type = no_stat_line_type;

   return ( 0 );

}

determine_line_type();

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int STATLine::is_ok() const

{

if ( !(DataLine::is_ok()) )  return ( 0 );

const char * c = get_item(0);

// Check is this is a header line

if ( strcmp(c, "VERSION") == 0 )  return ( 0 );

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::get_item(int k) const

{

const char * c = DataLine::get_item(k);

   //
   // Check for the NA string and interpret it as bad data
   //

if ( strcmp(c, na_str) == 0 ) return ( bad_data_str );
else                          return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::version() const

{

const char * c = get_item(version_offset);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::model() const

{

const char * c = get_item(model_offset);

return ( c );

}


////////////////////////////////////////////////////////////////////////


int STATLine::fcst_lead() const

{

int j;
const char * c = get_item(fcst_lead_offset);

j = timestring_to_sec(c);

return ( j );

}


////////////////////////////////////////////////////////////////////////


unixtime STATLine::fcst_valid_beg() const

{

unixtime t;
const char * c = get_item(fcst_valid_beg_offset);

t = timestring_to_unix(c);

return ( t );

}


////////////////////////////////////////////////////////////////////////


unixtime STATLine::fcst_valid_end() const

{

unixtime t;
const char * c = get_item(fcst_valid_end_offset);

t = timestring_to_unix(c);

return ( t );

}


////////////////////////////////////////////////////////////////////////


int STATLine::obs_lead() const

{

int j;
const char * c = get_item(obs_lead_offset);

j = timestring_to_sec(c);

return ( j );

}


////////////////////////////////////////////////////////////////////////


unixtime STATLine::obs_valid_beg() const

{

unixtime t;
const char * c = get_item(obs_valid_beg_offset);

t = timestring_to_unix(c);

return ( t );

}


////////////////////////////////////////////////////////////////////////


unixtime STATLine::obs_valid_end() const

{

unixtime t;
const char * c = get_item(obs_valid_end_offset);

t = timestring_to_unix(c);

return ( t );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::fcst_var() const

{

const char * c = get_item(fcst_var_offset);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::fcst_lev() const

{

const char * c = get_item(fcst_lev_offset);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::obs_var() const

{

const char * c = get_item(obs_var_offset);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::obs_lev() const

{

const char * c = get_item(obs_lev_offset);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::obtype() const

{

const char * c = get_item(obtype_offset);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::vx_mask() const

{

const char * c = get_item(vx_mask_offset);

return ( c );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::interp_mthd() const

{

const char * c = get_item(interp_mthd_offset);

return ( c );

}


////////////////////////////////////////////////////////////////////////


int STATLine::interp_pnts() const

{

int k;
const char * c = get_item(interp_pnts_offset);

k = atoi(c);

return ( k );

}


////////////////////////////////////////////////////////////////////////


SingleThresh STATLine::fcst_thresh() const

{

SingleThresh st;

const char * c = get_item(fcst_thresh_offset);

st.set(c);

return ( st );

}


////////////////////////////////////////////////////////////////////////


SingleThresh STATLine::obs_thresh() const

{

SingleThresh st;

const char * c = get_item(obs_thresh_offset);

st.set(c);

return ( st );

}


////////////////////////////////////////////////////////////////////////


SingleThresh STATLine::cov_thresh() const

{

SingleThresh st;

const char * c = get_item(cov_thresh_offset);

st.set(c);

return ( st );

}


////////////////////////////////////////////////////////////////////////


double STATLine::alpha() const

{

double a;
const char * c = get_item(alpha_offset);

a = atof(c);

return ( a );

}


////////////////////////////////////////////////////////////////////////


const char * STATLine::line_type() const

{

const char * c = get_item(line_type_offset);

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


int STATLine::fcst_init_hour() const

{

int s, mon, day, yr, hr, min, sec;
unixtime t;

t = fcst_init_beg();

unix_to_mdyhms(t, mon, day, yr, hr, min, sec);

s = hms_to_sec(hr, min, sec);

return ( s );

}


////////////////////////////////////////////////////////////////////////


int STATLine::obs_init_hour() const

{

int s, mon, day, yr, hr, min, sec;
unixtime t;

t = obs_init_beg();

unix_to_mdyhms(t, mon, day, yr, hr, min, sec);

s = hms_to_sec(hr, min, sec);

return ( s );

}


////////////////////////////////////////////////////////////////////////


void STATLine::determine_line_type()

{

//
// If there aren't enough columns present to determine the line type
// just return.
//
if( n_items() < (line_type_offset + 1) )  {

   Type = no_stat_line_type;
   return;
}

const char * const c = line_type();

Type = string_to_statlinetype(c);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////
//
// Code for misc functions
//
////////////////////////////////////////////////////////////////////////


const char * statlinetype_to_string(const STATLineType t) {
   return(statlinetype_str[t]);
}


////////////////////////////////////////////////////////////////////////


void statlinetype_to_string(const STATLineType t, char *out) {

   strcpy(out, statlinetype_to_string(t));

   return;
}


////////////////////////////////////////////////////////////////////////


STATLineType string_to_statlinetype(const char *str) {
   STATLineType t;

   if(     strcasecmp(str, statlinetype_str[0]) == 0)
      t = stat_sl1l2;
   else if(strcasecmp(str, statlinetype_str[1]) == 0)
      t = stat_sal1l2;
   else if(strcasecmp(str, statlinetype_str[2]) == 0)
      t = stat_vl1l2;
   else if(strcasecmp(str, statlinetype_str[3]) == 0)
      t = stat_val1l2;
   else if(strcasecmp(str, statlinetype_str[4]) == 0)
      t = stat_fho;
   else if(strcasecmp(str, statlinetype_str[5]) == 0)
      t = stat_ctc;
   else if(strcasecmp(str, statlinetype_str[6]) == 0)
      t = stat_cts;
   else if(strcasecmp(str, statlinetype_str[7]) == 0)
      t = stat_mctc;
   else if(strcasecmp(str, statlinetype_str[8]) == 0)
      t = stat_mcts;
   else if(strcasecmp(str, statlinetype_str[9]) == 0)
      t = stat_cnt;
   else if(strcasecmp(str, statlinetype_str[10]) == 0)
      t = stat_pct;
   else if(strcasecmp(str, statlinetype_str[11]) == 0)
      t = stat_pstd;
   else if(strcasecmp(str, statlinetype_str[12]) == 0)
      t = stat_pjc;
   else if(strcasecmp(str, statlinetype_str[13]) == 0)
      t = stat_prc;
   else if(strcasecmp(str, statlinetype_str[14]) == 0)
      t = stat_mpr;
   else if(strcasecmp(str, statlinetype_str[15]) == 0)
      t = stat_nbrctc;
   else if(strcasecmp(str, statlinetype_str[16]) == 0)
      t = stat_nbrcts;
   else if(strcasecmp(str, statlinetype_str[17]) == 0)
      t = stat_nbrcnt;
   else if(strcasecmp(str, statlinetype_str[18]) == 0)
      t = stat_isc;
   else if(strcasecmp(str, statlinetype_str[19]) == 0)
      t = stat_wdir;
   else if(strcasecmp(str, statlinetype_str[20]) == 0)
      t = stat_rhist;
   else if(strcasecmp(str, statlinetype_str[21]) == 0)
      t = stat_orank;
   else
      t = no_stat_line_type;

   return(t);
}


////////////////////////////////////////////////////////////////////////


StringArray get_stat_filenames(const StringArray & search_dirs)

{

int j;
const int N = search_dirs.n_elements();
StringArray a, b;
struct stat sbuf;


for (j=0; j<N; ++j)  {

   if ( stat(search_dirs[j], &sbuf) < 0 )  {

      mlog << Error << "\n  get_stat_filenames() -> "
           << "can't stat \"" << search_dirs[j] << "\"\n\n";

      exit ( 1 );

   }

   if ( S_ISDIR(sbuf.st_mode) )  {

      b = get_stat_filenames_from_dir(search_dirs[j]);

      a.add(b);

      b.clear();

   } else if ( S_ISREG(sbuf.st_mode) )  {

      if ( is_stat_filename(search_dirs[j]) )  a.add(search_dirs[j]);

   }

}


return ( a );

}


////////////////////////////////////////////////////////////////////////


StringArray get_stat_filenames_from_dir(const char * directory_path)

{

DIR * directory = (DIR *) 0;
struct dirent * entry = (struct dirent *) 0;
StringArray a, b;
char entry_path[PATH_MAX];
struct stat sbuf;



directory = opendir(directory_path);

if ( !directory )  {

   mlog << Error << "\n  get_stat_filenames_from_dir() -> "
        << "can't open directory path \"" << directory_path << "\"\n\n";

   exit ( 1 );

}

while ( (entry = readdir(directory)) != NULL )  {

   if ( strcmp(entry->d_name, "." ) == 0 )  continue;
   if ( strcmp(entry->d_name, "..") == 0 )  continue;

   sprintf(entry_path, "%s/%s", directory_path, entry->d_name);

   if ( stat(entry_path, &sbuf) < 0 )  {

      mlog << Error << "\n  get_stat_filenames_from_dir() -> "
           << "can't stat \"" << entry_path << "\"\n\n";

      exit ( 1 );

   }

   if ( S_ISDIR(sbuf.st_mode) )  {

      b = get_stat_filenames_from_dir(entry_path);

      a.add(b);

      b.clear();

   } else if ( S_ISREG(sbuf.st_mode) )  {

      if ( is_stat_filename(entry_path) )  a.add(entry_path);

   }

}   //  while

   //
   //  done
   //

closedir(directory);  directory = (DIR *) 0;

return ( a );

}


////////////////////////////////////////////////////////////////////////


int is_stat_filename(const char * path)

{

int j, k, n;
int match;
const char * short_name = (const char *) 0;

   //
   //  get short name
   //

j = strlen(path) - 1;

while ( (j >= 0) && (path[j] != '/') )  --j;

++j;

short_name = path + j;

   //
   //  does the filename end in a proper suffix?
   //

n = strlen(short_name);

match = 0;

for (j=0; j<n_suffixes; ++j)  {

   k = strlen(suffix_list[j]);

   if ( strncmp(short_name + (n - k), suffix_list[j], k) == 0 ) {

      match = 1;

      break;

   }

}

if ( !match )  return ( 0 );

   //
   //  done
   //

return ( 1 );

}


////////////////////////////////////////////////////////////////////////

int determine_column_offset(STATLineType type, const char *c)

{

int offset;

switch(type) {

   case stat_sl1l2:
      offset = get_column_offset(sl1l2_columns, n_sl1l2_columns, c);
      break;

   case stat_sal1l2:
      offset = get_column_offset(sal1l2_columns, n_sal1l2_columns, c);
      break;

   case stat_vl1l2:
      offset = get_column_offset(vl1l2_columns, n_vl1l2_columns, c);
      break;

   case stat_val1l2:
      offset = get_column_offset(val1l2_columns, n_val1l2_columns, c);
      break;

   case stat_fho:
      offset = get_column_offset(fho_columns, n_fho_columns, c);
      break;

   case stat_ctc:
      offset = get_column_offset(ctc_columns, n_ctc_columns, c);
      break;

   case stat_cts:
      offset = get_column_offset(cts_columns, n_cts_columns, c);
      break;

   case stat_mctc:
      offset = get_column_offset(mctc_columns, n_mctc_columns, c);
      break;

   case stat_mcts:
      offset = get_column_offset(mcts_columns, n_mcts_columns, c);
      break;

   case stat_cnt:
      offset = get_column_offset(cnt_columns, n_cnt_columns, c);
      break;

   case stat_pct:
      offset = get_pct_column_offset(c);
      break;

   case stat_pstd:
      offset = get_pstd_column_offset(c);
      break;

   case stat_pjc:
      offset = get_pjc_column_offset(c);
      break;

   case stat_prc:
      offset = get_prc_column_offset(c);
      break;

   case stat_mpr:
      offset = get_column_offset(mpr_columns, n_mpr_columns, c);
      break;

   case stat_nbrctc:
      offset = get_column_offset(nbrctc_columns, n_nbrctc_columns, c);
      break;

   case stat_nbrcts:
      offset = get_column_offset(nbrcts_columns, n_nbrcts_columns, c);
      break;

   case stat_nbrcnt:
      offset = get_column_offset(nbrcnt_columns, n_nbrcnt_columns, c);
      break;

   case stat_isc:
      offset = get_column_offset(isc_columns, n_isc_columns, c);
      break;

   case stat_rhist:
      offset = get_rhist_column_offset(c);
      break;

   case stat_orank:
      offset = get_orank_column_offset(c);
      break;

   default:
      mlog << Error << "\n  determine_column_offset() -> "
           << "unexpected line type value of " << type << "\n\n";

      exit ( 1 );
      break;
};

return(offset);

}


////////////////////////////////////////////////////////////////////////
