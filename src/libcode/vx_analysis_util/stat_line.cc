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

#include "stat_columns.h"
#include "stat_line.h"
#include "stat_offsets.h"
#include "analysis_utils.h"
#include "stat_offsets.h"

#include "vx_log.h"


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

Indent prefix(depth);
ThreshArray ta;


DataLine::dump(out, depth);

out << prefix << "\n";

out << prefix << "Version        = "   << version()     << "\n";
out << prefix << "Model          = \"" << model()       << "\"\n";

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
out << prefix << "Fcst Level     = \"" << fcst_lev()   << "\"\n";

out << prefix << "Obs Var        = \"" << obs_var()    << "\"\n";
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

return ( DataLine::is_ok() );

}


////////////////////////////////////////////////////////////////////////


int STATLine::is_header() const

{

const char * c = line_type();

STATLineType t = string_to_statlinetype(c);

if ( t == stat_header ) return ( 1 );

return ( 0 );

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


int STATLine::fcst_valid_hour() const

{

return ( unix_to_sec_of_day(fcst_valid_beg()) );

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


int STATLine::obs_valid_hour() const

{

return ( unix_to_sec_of_day(obs_valid_beg()) );

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


ThreshArray STATLine::fcst_thresh() const

{

ThreshArray ta;

const char * c = get_item(fcst_thresh_offset);

ta.add_css(c);

return ( ta );

}


////////////////////////////////////////////////////////////////////////


ThreshArray STATLine::obs_thresh() const

{

ThreshArray ta;

const char * c = get_item(obs_thresh_offset);

ta.add_css(c);

return ( ta );

}


////////////////////////////////////////////////////////////////////////


SetLogic STATLine::thresh_logic() const

{

SetLogic t = SetLogic_None;

ConcatString cs = get_item(fcst_thresh_offset);

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

const char * c = get_item(cov_thresh_offset);

ta.add_css(c);

return ( ta );

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


int determine_column_offset(const STATLine &L, const char *c, bool error_out)

{

int offset = bad_data_int;

switch(L.type()) {

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
      offset = get_pstd_column_offset(c, L);
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
      offset = get_rhist_column_offset(c, L);
      break;

   case stat_phist:
      offset = get_phist_column_offset(c);
      break;

   case stat_orank:
      offset = get_orank_column_offset(c, L);
      break;

   case stat_ssvar:
      offset = get_column_offset(ssvar_columns, n_ssvar_columns, c);
      break;

   default:
      mlog << Error << "\ndetermine_column_offset() -> "
           << "unexpected line type value of " << L.type() << "\n\n";

      exit ( 1 );
      break;
};

// Check any extra header columns
if(is_bad_data(offset)) {
   if(!L.get_file()->header().has(c, offset)) offset = bad_data_int;
}

// Check for no match
if(error_out && is_bad_data(offset)) {
   mlog << Error << "\ndetermine_column_offset() -> "
        << "no match found for column named: \"" << c << "\"\n\n";
   throw(1);
}

return(offset);

}

////////////////////////////////////////////////////////////////////////

int get_column_offset(const char **arr, int n_cols, const char *col_name) {
   int i;
   int offset = bad_data_int;

   //
   // Search the STAT header columns first
   //
   for(i=0; i<n_header_columns; i++) {
      if(strcasecmp(hdr_columns[i], col_name) == 0) {
         offset = i;
         break;
      }
   }

   //
   // If not found, search the columns provided
   //
   if(is_bad_data(offset)) {
      for(i=0; i<n_cols; i++) {

         if(strcasecmp(arr[i], col_name) == 0) {
            offset = i+n_header_columns;
            break;
         }
      }
   }

   return(offset);
}

////////////////////////////////////////////////////////////////////////

int get_mctc_column_offset(const char *col_name, int n_cat) {
   int i, j;
   int offset = bad_data_int;

   //
   // Search the STAT header columns first
   //
   for(i=0; i<n_header_columns; i++) {
      if(strcasecmp(hdr_columns[i], col_name) == 0) {
         offset = i;
         break;
      }
   }

   //
   // If not found, search the PCT columns:
   //    TOTAL,       N_CAT,
   //    [Fi_Oj] (for of the NxN contingency table cells)
   //

   //
   // Check the static columns
   //
   if(is_bad_data(offset)) {
      for(i=0; i<2; i++) {

         if(strcasecmp(mctc_columns[i], col_name) == 0) {
            offset = i+n_header_columns;
            break;
         }
      }
   }

   //
   // Check the variable column
   //
   if(is_bad_data(offset)) {

      // Fi_Oj: Parse out i and j
      parse_row_col(col_name, i, j);
      offset = n_header_columns + 2 + (i-1)*n_cat + (j-1);
   }

   return(offset);
}

////////////////////////////////////////////////////////////////////////

int get_pct_column_offset(const char *col_name) {
   int i;
   int offset = bad_data_int;

   //
   // Search the STAT header columns first
   //
   for(i=0; i<n_header_columns; i++) {
      if(strcasecmp(hdr_columns[i], col_name) == 0) {
         offset = i;
         break;
      }
   }

   //
   // If not found, search the PCT columns:
   //    TOTAL,       N_THRESH,
   //   [THRESH,      OY,          ON,] (for each row of Nx2 Table)
   //    THRESH                         (last threshold)
   //

   //
   // Check the static columns
   //
   if(is_bad_data(offset)) {
      for(i=0; i<2; i++) {

         if(strcasecmp(pct_columns[i], col_name) == 0) {
            offset = i+n_header_columns;
            break;
         }
      }
   }

   //
   // Check the variable columns
   //
   if(is_bad_data(offset)) {

      // THRESH_i
      if(strncasecmp(pct_columns[2], col_name,
                     strlen(pct_columns[2])) == 0) {

         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 3*(i-1) + 0;
      }
      // OY_i
      else if(strncasecmp(pct_columns[3], col_name,
                          strlen(pct_columns[3])) == 0) {

         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 3*(i-1) + 1;
      }
      // ON_i
      else if(strncasecmp(pct_columns[4], col_name,
                          strlen(pct_columns[4])) == 0) {

         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 3*(i-1) + 2;
      }
   }

   return(offset);
}

////////////////////////////////////////////////////////////////////////

int get_pstd_column_offset(const char *col_name, const STATLine &L) {
   int i;
   int offset = bad_data_int;

   //
   // Search the STAT header columns first
   //
   for(i=0; i<n_header_columns; i++) {
      if(strcasecmp(hdr_columns[i], col_name) == 0) {
         offset = i;
         break;
      }
   }

   //
   // If not found, search the PSTD columns:
   //    TOTAL,       N_THRESH,    BASER,
   //    BASER_NCL,   BASER_NCU,   RELIABILTY,
   //    RESOLUTION,  UNCERTAINTY, ROC_AUC,
   //    BRIER,       BRIER_NCL,   BRIER_NCU,
   //    BRIERCL,     BRIERCL_NCL, BRIERCL_NCU,
   //    BSS,         [THRESH] (for each threshold),
   //

   //
   // Check the static columns
   //
   if(is_bad_data(offset)) {
      for(i=0; i<12; i++) {

         if(strcasecmp(pstd_columns[i], col_name) == 0) {
            offset = i+n_header_columns;
            break;
         }
      }
   }

   //
   // Check the variable columns
   //
   if(is_bad_data(offset)) {

      // THRESH_i
      if(strncasecmp(pstd_columns[12], col_name,
                     strlen(pstd_columns[12])) == 0) {

         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 12 + (i-1);
      }
   }

   return(offset);
}

////////////////////////////////////////////////////////////////////////

int get_pjc_column_offset(const char *col_name) {
   int i;
   int offset = bad_data_int;

   //
   // Search the STAT header columns first
   //
   for(i=0; i<n_header_columns; i++) {
      if(strcasecmp(hdr_columns[i], col_name) == 0) {
         offset = i;
         break;
      }
   }

   //
   // If not found, search the PJC columns:
   //    TOTAL,       N_THRESH,
   //   [THRESH,
   //    OY_TP,       ON_TP,      CALIBRATION,
   //    REFINEMENT,  LIKELIHOOD, BASER] (for each row of Nx2 Table)
   //    THRESH
   //

   //
   // Check the static columns
   //
   if(is_bad_data(offset)) {
      for(i=0; i<2; i++) {

         if(strcasecmp(pjc_columns[i], col_name) == 0) {
            offset = i+n_header_columns;
            break;
         }
      }
   }

   //
   // Check the variable columns
   //
   if(is_bad_data(offset)) {

      // THRESH_i
      if(strncasecmp(pjc_columns[2], col_name,
                     strlen(pjc_columns[2])) == 0) {

         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 7*(i-1) + 0;
      }
      // OY_TP_i
      else if(strncasecmp(pjc_columns[3], col_name,
                          strlen(pjc_columns[3])) == 0) {

         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 7*(i-1) + 1;
      }
      // ON_TP_i
      else if(strncasecmp(pjc_columns[4], col_name,
                          strlen(pjc_columns[4])) == 0) {

         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 7*(i-1) + 2;
      }
      // CALIBRATION_i
      else if(strncasecmp(pjc_columns[5], col_name,
                          strlen(pjc_columns[5])) == 0) {

         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 7*(i-1) + 3;
      }
      // REFINEMENT_i
      else if(strncasecmp(pjc_columns[6], col_name,
                          strlen(pjc_columns[6])) == 0) {

         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 7*(i-1) + 4;
      }
      // LIKELIHOOD_i
      else if(strncasecmp(pjc_columns[7], col_name,
                          strlen(pjc_columns[7])) == 0) {

         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 7*(i-1) + 5;
      }
      // BASER_i
      else if(strncasecmp(pjc_columns[8], col_name,
                          strlen(pjc_columns[8])) == 0) {

         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 7*(i-1) + 6;
      }
   }

   return(offset);
}

////////////////////////////////////////////////////////////////////////

int get_prc_column_offset(const char *col_name) {
   int i;
   int offset = bad_data_int;

   //
   // Search the STAT header columns first
   //
   for(i=0; i<n_header_columns; i++) {
      if(strcasecmp(hdr_columns[i], col_name) == 0) {
         offset = i;
         break;
      }
   }

   //
   // If not found, search the prc columns:
   //    TOTAL,       N_THRESH,
   //   [THRESH,      PODY,        POFD,] (for each row of Nx2 Table)
   //    THRESH                           (last threshold)
   //

   //
   // Check the static columns
   //
   if(is_bad_data(offset)) {
      for(i=0; i<2; i++) {

         if(strcasecmp(prc_columns[i], col_name) == 0) {
            offset = i+n_header_columns;
            break;
         }
      }
   }

   //
   // Check the variable columns
   //
   if(is_bad_data(offset)) {

      // THRESH_i
      if(strncasecmp(prc_columns[2], col_name,
                     strlen(prc_columns[2])) == 0) {

         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 3*(i-1) + 0;
      }
      // PODY_i
      else if(strncasecmp(prc_columns[3], col_name,
                          strlen(prc_columns[3])) == 0) {

         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 3*(i-1) + 1;
      }
      // POFD_i
      else if(strncasecmp(prc_columns[4], col_name,
                          strlen(prc_columns[4])) == 0) {

         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 3*(i-1) + 2;
      }
   }

   return(offset);
}

////////////////////////////////////////////////////////////////////////

int get_rhist_column_offset(const char *col_name, const STATLine &L) {
   int i;
   int offset = bad_data_int;

   //
   // Search the STAT header columns first
   //
   for(i=0; i<n_header_columns; i++) {
      if(strcasecmp(hdr_columns[i], col_name) == 0) {
         offset = i;
         break;
      }
   }

   //
   // If not found, search the rhist columns:
   //    TOTAL,  CRPS,  IGN,
   //    CRPSS,  N_RANK, [RANK_] (for possible ranks, n_ens+1),
   //

   //
   // Check the static columns
   //
   if(is_bad_data(offset)) {
      for(i=0; i<4; i++) {

         if(strcasecmp(rhist_columns[i], col_name) == 0) {
            offset = i+n_header_columns;
            break;
         }
      }
   }

   //
   // Check the variable column
   //
   if(is_bad_data(offset)) {

      // RANK_i
      if(strncasecmp(rhist_columns[4], col_name,
                     strlen(rhist_columns[4])) == 0) {
         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 4 + (i-1);
      }
   }

   return(offset);
}

////////////////////////////////////////////////////////////////////////

int get_phist_column_offset(const char *col_name) {
   int i;
   int offset = bad_data_int;

   //
   // Search the STAT header columns first
   //
   for(i=0; i<n_header_columns; i++) {
      if(strcasecmp(hdr_columns[i], col_name) == 0) {
         offset = i;
         break;
      }
   }

   //
   // If not found, search the phist columns:
   //    TOTAL,  BIN_SIZE, N_BIN, [BIN_] (for possible bins)
   //

   //
   // Check the static columns
   //
   if(is_bad_data(offset)) {
      for(i=0; i<3; i++) {

         if(strcasecmp(phist_columns[i], col_name) == 0) {
            offset = i+n_header_columns;
            break;
         }
      }
   }

   //
   // Check the variable column
   //
   if(is_bad_data(offset)) {

      // BIN_i
      if(strncasecmp(phist_columns[3], col_name,
                     strlen(phist_columns[3])) == 0) {
         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 3 + (i-1);
      }
   }

   return(offset);
}

////////////////////////////////////////////////////////////////////////

int get_orank_column_offset(const char *col_name, const STATLine &L) {
   int i, n;
   int offset = bad_data_int;

   //
   // Search the STAT header columns first
   //
   for(i=0; i<n_header_columns; i++) {
      if(strcasecmp(hdr_columns[i], col_name) == 0) {
         offset = i;
         break;
      }
   }

   //
   // If not found, search the orank columns:
   //    TOTAL,       INDEX,       OBS_SID,
   //    OBS_LAT,     OBS_LON,     OBS_LVL,
   //    OBS_ELV,     OBS,         PIT,
   //    RANK,        N_ENS_VLD,   N_ENS,
   //    [ENS_] (for each ensemble member)
   //    OBS_QC,      ENS_MEAN,    CLIMO
   //

   //
   // Check the static columns
   //
   if(is_bad_data(offset)) {
      for(i=0; i<12; i++) {

         if(strcasecmp(orank_columns[i], col_name) == 0) {
            offset = i+n_header_columns;
            break;
         }
      }
   }

   //
   // Check the variable column
   //
   if(is_bad_data(offset)) {

      // ENS_i
      if(strncasecmp(orank_columns[12], col_name,
                     strlen(orank_columns[12])) == 0) {
         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 12 + (i-1);
      }
   }

   //
   // Check for the OBS_QC special case after the variable columns
   //
   if(strcasecmp(col_name, "OBS_QC") == 0) {
      n      = atoi(L.get_item(get_orank_column_offset("N_ENS", L)));
      offset = orank_obs_qc_offset(n);
   }

   //
   // Check for the ENS_MEAN special case after the variable columns
   //
   if(strcasecmp(col_name, "ENS_MEAN") == 0) {
      n      = atoi(L.get_item(get_orank_column_offset("N_ENS", L)));
      offset = orank_ens_mean_offset(n);
   }

   //
   // Check for the CLIMO special case after the variable columns
   //
   if(strcasecmp(col_name, "CLIMO") == 0) {
      n      = atoi(L.get_item(get_orank_column_offset("N_ENS", L)));
      offset = orank_climo_offset(n);
   }

   return(offset);
}

////////////////////////////////////////////////////////////////////////
