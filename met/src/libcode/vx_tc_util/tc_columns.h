// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __TC_COLUMNS_H__
#define  __TC_COLUMNS_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>

#include "vx_util.h"

#include "tc_hdr_columns.h"
#include "track_pair_info.h"
#include "prob_rirw_pair_info.h"

////////////////////////////////////////////////////////////////////////

static const char * tc_header_cols [] = {
   "VERSION",    "AMODEL",     "BMODEL",
   "DESC",       "STORM_ID",   "BASIN",
   "CYCLONE",    "STORM_NAME", "INIT",
   "LEAD",       "VALID",      "INIT_MASK",
   "VALID_MASK", "LINE_TYPE"
};

static const int n_tc_header_cols = sizeof(tc_header_cols)/sizeof(*tc_header_cols);

////////////////////////////////////////////////////////////////////////

static const char * tc_mpr_cols [] = {
   "TOTAL",       "INDEX",
   "LEVEL",       "WATCH_WARN", "INITIALS",
   "ALAT",        "ALON",
   "BLAT",        "BLON",
   "TK_ERR",      "X_ERR",      "Y_ERR",
   "ALTK_ERR",    "CRTK_ERR",
   "ADLAND",      "BDLAND",
   "AMSLP",       "BMSLP",
   "AMAX_WIND",   "BMAX_WIND",
   "AAL_WIND_34", "BAL_WIND_34",
   "ANE_WIND_34", "BNE_WIND_34",
   "ASE_WIND_34", "BSE_WIND_34",
   "ASW_WIND_34", "BSW_WIND_34",
   "ANW_WIND_34", "BNW_WIND_34",
   "AAL_WIND_50", "BAL_WIND_50",
   "ANE_WIND_50", "BNE_WIND_50",
   "ASE_WIND_50", "BSE_WIND_50",
   "ASW_WIND_50", "BSW_WIND_50",
   "ANW_WIND_50", "BNW_WIND_50",
   "AAL_WIND_64", "BAL_WIND_64",
   "ANE_WIND_64", "BNE_WIND_64",
   "ASE_WIND_64", "BSE_WIND_64",
   "ASW_WIND_64", "BSW_WIND_64",
   "ANW_WIND_64", "BNW_WIND_64",
   "ARADP",       "BRADP",
   "ARRP",        "BRRP",
   "AMRD",        "BMRD",
   "AGUSTS",      "BGUSTS",
   "AEYE",        "BEYE",
   "ADIR",        "BDIR",
   "ASPEED",      "BSPEED",
   "ADEPTH",      "BDEPTH"
};

static const int n_tc_mpr_cols = sizeof(tc_mpr_cols)/sizeof(*tc_mpr_cols);

////////////////////////////////////////////////////////////////////////

static const char * tc_cols_track [] = {
  "TK_ERR", "ALTK_ERR", "CRTK_ERR"
};
static const int n_tc_cols_track = sizeof(tc_cols_track)/sizeof(*tc_cols_track);

static const char * tc_cols_wind [] = {
   "ABS(AAL_WIND_34-BAL_WIND_34)",
   "ABS(ANE_WIND_34-BNE_WIND_34)",
   "ABS(ASE_WIND_34-BSE_WIND_34)",
   "ABS(ASW_WIND_34-BSW_WIND_34)",
   "ABS(ANW_WIND_34-BNW_WIND_34)",
   "ABS(AAL_WIND_50-BAL_WIND_50)",
   "ABS(ANE_WIND_50-BNE_WIND_50)",
   "ABS(ASE_WIND_50-BSE_WIND_50)",
   "ABS(ASW_WIND_50-BSW_WIND_50)",
   "ABS(ANW_WIND_50-BNW_WIND_50)",
   "ABS(AAL_WIND_64-BAL_WIND_64)",
   "ABS(ANE_WIND_64-BNE_WIND_64)",
   "ABS(ASE_WIND_64-BSE_WIND_64)",
   "ABS(ASW_WIND_64-BSW_WIND_64)",
   "ABS(ANW_WIND_64-BNW_WIND_64)"
};
static const int n_tc_cols_wind = sizeof(tc_cols_wind)/sizeof(*tc_cols_wind);

static const char * tc_cols_ti [] = {
  "ABS(TK_ERR)", "ABS(AMAX_WIND-BMAX_WIND)"
};
static const int n_tc_cols_ti = sizeof(tc_cols_ti)/sizeof(*tc_cols_ti);

static const char * tc_cols_ac [] = {
  "ABS(ALTK_ERR)", "ABS(CRTK_ERR)"
};
static const int n_tc_cols_ac = sizeof(tc_cols_ac)/sizeof(*tc_cols_ac);

static const char * tc_cols_xy [] = {
  "ABS(X_ERR)", "ABS(Y_ERR)"
};
static const int n_tc_cols_xy = sizeof(tc_cols_xy)/sizeof(*tc_cols_xy);

////////////////////////////////////////////////////////////////////////

static const char * prob_rirw_cols [] = {
   "ALAT",        "ALON",
   "BLAT",        "BLON",
   "INITIALS",
   "TK_ERR",      "X_ERR",      "Y_ERR",
   "ADLAND",      "BDLAND",
   "RIRW_BEG",    "RIRW_END",   "RIRW_WINDOW",
   "AWIND_END",
   "BWIND_BEG",   "BWIND_END",
   "BDELTA",      "BDELTA_MAX",
   "BLEVEL_BEG",  "BLEVEL_END",
   "N_THRESH",    "THRESH_",    "PROB_"
};

static int n_prob_rirw_cols = sizeof(prob_rirw_cols)/sizeof(*prob_rirw_cols);

inline int get_n_prob_rirw_cols (int n) { return(n_prob_rirw_cols + (2*n)); } // n = NThresh

////////////////////////////////////////////////////////////////////////

extern void open_tc_txt_file (ofstream *&,  const char *);
extern void close_tc_txt_file(ofstream *&,  const char *);

////////////////////////////////////////////////////////////////////////

// Write out the header row for fixed length line types
extern void write_tc_header_row(const char **, int, int, AsciiTable &, int, int);

// Write out the header row
extern void write_tc_mpr_header_row   (int,      AsciiTable &, int, int);
extern void write_prob_rirw_header_row(int, int, AsciiTable &, int, int);

// Write out the data lines
extern void write_tc_mpr_row   (TcHdrColumns &, const TrackPairInfo &,    AsciiTable &, int &);
extern void write_prob_rirw_row(TcHdrColumns &, const ProbRIRWPairInfo &, AsciiTable &, int &);

// Write out the header entries
extern void write_tc_header_cols(const TcHdrColumns &, AsciiTable &, int);

// Write out the data columns
extern void write_tc_mpr_cols   (const TrackPairInfo  &,   int, AsciiTable &, int, int);
extern void write_prob_rirw_cols(const ProbRIRWPairInfo &, int, AsciiTable &, int, int);

// Setup column justification for TC-STAT AsciiTable objects
extern void justify_tc_stat_cols(AsciiTable &);

////////////////////////////////////////////////////////////////////////

#endif   /*  __TC_COLUMNS_H__  */

////////////////////////////////////////////////////////////////////////
