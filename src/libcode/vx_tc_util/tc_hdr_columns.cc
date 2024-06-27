// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <string.h>

#include "tc_hdr_columns.h"

using namespace std;

////////////////////////////////////////////////////////////////////////
//
//  Code for class TcHdrColumns
//
////////////////////////////////////////////////////////////////////////

TcHdrColumns::TcHdrColumns() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TcHdrColumns::~TcHdrColumns() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void TcHdrColumns::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TcHdrColumns::clear() {

   ADeckModel.clear();
   BDeckModel.clear();
   Desc.clear();
   StormId.clear();
   Basin.clear();
   Cyclone.clear();
   StormName.clear();
   InitTime = (unixtime) 0;
   LeadTime = 0;
   ValidTime = (unixtime) 0;
   InitMask.clear();
   ValidMask.clear();
   LineType.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TcHdrColumns::apply_set_hdr_opts(
        const StringArray &hdr_cols, const StringArray &hdr_vals) {

   // No updates needed
   if(hdr_cols.n() == 0) return;

   int index;

   // Sanity check lengths
   if(hdr_cols.n() != hdr_vals.n()) {
      mlog << Error << "\nTcHdrColumns::apply_set_hdr_opts() -> "
           << "the number of -set_hdr columns names (" << hdr_cols.n()
           << " and values (" << hdr_vals.n() << " must match!\n\n";
      exit(1);
   }

   // AMODEL
   if(hdr_cols.has("AMODEL", index)) {
      set_adeck_model(hdr_vals[index]);
   }

   // BMODEL
   if(hdr_cols.has("BMODEL", index)) {
      set_bdeck_model(hdr_vals[index]);
   }

   // DESC
   if(hdr_cols.has("DESC", index)) {
      set_desc(hdr_vals[index]);
   }

   // STORM_ID
   if(hdr_cols.has("STORM_ID", index)) {
      set_storm_id(hdr_vals[index]);
   }

   // BASIN
   if(hdr_cols.has("BASIN", index)) {
      set_basin(hdr_vals[index]);
   }

   // CYCLONE
   if(hdr_cols.has("CYCLONE", index)) {
      set_cyclone(hdr_vals[index]);
   }

   // STORM_NAME
   if(hdr_cols.has("STORM_NAME", index)) {
      set_storm_name(hdr_vals[index]);
   }

   // INIT
   if(hdr_cols.has("INIT", index)) {
      set_init(timestring_to_sec(hdr_vals[index].c_str()));
   }

   // LEAD
   if(hdr_cols.has("LEAD", index)) {
      set_lead(timestring_to_sec(hdr_vals[index].c_str()));
   }

   // VALID
   if(hdr_cols.has("VALID", index)) {
      set_valid(timestring_to_sec(hdr_vals[index].c_str()));
   }

   // INIT_MASK
   if(hdr_cols.has("INIT_MASK", index)) {
      set_init_mask(hdr_vals[index]);
   }

   // VALID_MASK
   if(hdr_cols.has("VALID_MASK", index)) {
      set_valid_mask(hdr_vals[index]);
   }

   // LINE_TYPE
   if(hdr_cols.has("LINE_TYPE", index)) {
      set_line_type(hdr_vals[index]);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
