// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   parse_stat_line.h
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    12/17/08  Halley Gotway   New
//
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////

#ifndef  __PARSE_STAT_LINE_H__
#define  __PARSE_STAT_LINE_H__

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cmath>
#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "vx_analysis_util/vx_analysis_util.h"
#include "vx_util/vx_util.h"
#include "vx_met_util/vx_met_util.h"
#include "vx_contable/vx_contable.h"

////////////////////////////////////////////////////////////////////////

// Matched Pair (MPR) data structure
struct MPRData {
   int    fcst_gc, obs_gc;
   int    total,   index;
   double obs_lat, obs_lon, obs_lvl, obs_elv;
   double fcst,    obs,     climo;
};

////////////////////////////////////////////////////////////////////////

extern void parse_fho_ctable   (STATLine &, TTContingencyTable &);
extern void parse_ctc_ctable   (STATLine &, TTContingencyTable &);
extern void parse_nbrctc_ctable(STATLine &, TTContingencyTable &);
extern void parse_nx2_ctable   (STATLine &, Nx2ContingencyTable &);

extern void parse_sl1l2_line   (STATLine &, SL1L2Info &);
extern void parse_sal1l2_line  (STATLine &, SL1L2Info &);
extern void parse_vl1l2_line   (STATLine &, VL1L2Info &);
extern void parse_val1l2_line  (STATLine &, VL1L2Info &);

extern void parse_mpr_line     (STATLine &, MPRData &);
extern void parse_isc_line     (STATLine &, ISCInfo &, int &);

////////////////////////////////////////////////////////////////////////

#endif   //  __PARSE_STAT_LINE_H__

////////////////////////////////////////////////////////////////////////
