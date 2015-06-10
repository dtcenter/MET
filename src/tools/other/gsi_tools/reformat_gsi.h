// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   reformat_gsi.h
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    06/09/15  Bullock         New
//
////////////////////////////////////////////////////////////////////////

#ifndef  __REFORMAT_GSI_H__
#define  __REFORMAT_GSI_H__

////////////////////////////////////////////////////////////////////////

using namespace std;

#include "vx_util.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

static const char *program_name = "reformat_gsi";
static const int   rec_pad_length = 4;
static const bool  swap_endian = true;

// Defaults for output header
static const char  *default_model       = "GSI";
static const int    default_lead        = 0;
static const char  *default_lev         = na_str;
static const char  *default_obtype      = na_str;
static const char  *default_vx_mask     = na_str;
static const char  *default_interp_mthd = na_str;
static const int    default_interp_wdth = 0;
static const char  *default_thresh      = na_str;
static const double default_alpha       = bad_data_double;
static const char  *default_line_type   = "MPR";

////////////////////////////////////////////////////////////////////////

static const char *conv_extra_columns [] = {
   "OBS_STYPE",    //  observation subtype                 (1)
   "OBERR_IN",     //  prepbufr inverse observation error  (14)
   "FIN_INV_ERR",  //  final inverse observation error     (16)

   "OBS_HGT",      //  observation height                  (7)
   "OBS_TIME",     //  observation time                    (8)
   "INPUT_QC",     //  input prepbufr qc                   (9)

   "SETUP_QC",     //  setup qc                            (10)
   "PREP_USE",     //  read_prepbufr usage                 (11)
   "ANLY_USE",     //  analysis usage                      (12)

   "RWGT",         //  non-linear qc rel weight            (13)
   "OBERR_ADJ",    //  read_prepbufr inverse obs error     (15)
};

static const int n_conv_extra_cols = sizeof(conv_extra_columns)/sizeof(*conv_extra_columns);

////////////////////////////////////////////////////////////////////////

static const char * rad_extra_columns [] = {
   "INV_OBS_ERR",   //  inverse observation error
   "SURF_EMIS",     //  surface emissivity
};

static const int n_rad_extra_cols = sizeof(rad_extra_columns)/sizeof(*rad_extra_columns);

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

static NumArray       channel;
static StringArray    hdr_name;
static StringArray    hdr_value;
static ConcatString   suffix = ".stat";
static ConcatString   output_directory = ".";
static StatHdrColumns shc;

////////////////////////////////////////////////////////////////////////

#endif   /*  __REFORMAT_GSI_H__  */

////////////////////////////////////////////////////////////////////////
