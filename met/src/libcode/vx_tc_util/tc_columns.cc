////////////////////////////////////////////////////////////////////////
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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "tc_columns.h"
#include "track_point.h"
#include "track_info.h"

#include "prob_rirw_pair_info.h"
#include "prob_rirw_info.h"

#include "vx_util.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

void open_tc_txt_file(ofstream *&out, const char *file_name) {

   // Create and open the output file stream
   out = new ofstream;
   out->open(file_name);

   if(!(*out)) {
      mlog << Error
           << "\nopen_tc_txt_file()-> "
           << "can't open the output file \"" << file_name
           << "\" for writing!\n\n";
      exit(1);
   }

   out->setf(ios::fixed);

   return;
}

////////////////////////////////////////////////////////////////////////

void close_tc_txt_file(ofstream *&out, const char *file_name) {

   // List the file being closed
   mlog << Debug(1)
        << "Output file: " << file_name << "\n";

   // Close the output file
   out->close();
   delete out;
   out = (ofstream *) 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_tc_header_row(const char **cols, int n_cols, int hdr_flag,
                         AsciiTable &at, int r, int c) {
   int i;

   // Write the header column names if requested
   if(hdr_flag) {
      for(i=0; i<n_tc_header_cols; i++)
         at.set_entry(r, i+c, tc_header_cols[i]);

      c += n_tc_header_cols;
   }

   // Write the columns names specific to this line type
   for(i=0; i<n_cols; i++)
      at.set_entry(r, i+c, cols[i]);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_tc_mpr_header_row(int hdr_flag, AsciiTable &at,
                             int r, int c) {
   int i;
   ConcatString s;

   // Write the header column names if requested
   if(hdr_flag) {
      for(i=0; i<n_tc_header_cols; i++)
         at.set_entry(r, c++, tc_header_cols[i]);
   }

   // Write the tc_mpr header columns
   for(i=0; i<n_tc_mpr_cols; i++) {
      at.set_entry(r, c++, tc_mpr_cols[i]);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_prob_rirw_header_row(int hdr_flag, int n_thresh, AsciiTable &at,
                              int r, int c) {
   int i;
   ConcatString s;
   char tmp_str[max_str_len];

   // Write the header column names if requested
   if(hdr_flag) {
      for(i=0; i<n_tc_header_cols; i++)
         at.set_entry(r, c++, tc_header_cols[i]);
   }

   // Write the static PROBRIRW header columns
   for(i=0; i<n_prob_rirw_cols-2; i++) {
      at.set_entry(r, c++, prob_rirw_cols[i]);
   }

   // Write the variable PROBRIRW header columns
   for(i=0; i<n_thresh; i++) {
      snprintf(tmp_str, sizeof(tmp_str), "%s%i", prob_rirw_cols[21], i+1);
      at.set_entry(r, c++, tmp_str); // THRESH_i

      snprintf(tmp_str, sizeof(tmp_str), "%s%i", prob_rirw_cols[22], i+1);
      at.set_entry(r, c++, tmp_str); // PROB_i
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_tc_mpr_row(TcHdrColumns &hdr, const TrackPairInfo &p,
                      AsciiTable &at, int &i_row) {
   int i;

   // TCMPR line type
   hdr.set_line_type((string)TCStatLineType_TCMPR_Str);

   // Loop through the TrackPairInfo points
   for(i=0; i<p.n_points(); i++) {

      // Timing information

      // Initialization and lead time for the ADECK
      hdr.set_init(p.adeck().init());
      hdr.set_lead(p.adeck()[i].lead());

      // Valid time for the ADECK and/or BDECK
      hdr.set_valid(p.valid(i));

      // Set the description
      if(p.n_lines() > i) {
	hdr.set_desc((string)p.line(i)->get_item("DESC", false));
      }

      // Write the header columns
      write_tc_header_cols(hdr, at, i_row);

      // Write the data columns
      write_tc_mpr_cols(p, i, at, i_row, n_tc_header_cols);

      // Increment the row counter
      i_row++;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_prob_rirw_row(TcHdrColumns &hdr, const ProbRIRWPairInfo &p,
                         AsciiTable &at, int &i_row) {

   // PROBRIRW line type
  hdr.set_line_type((string)"PROBRIRW");

   // Timing information
   hdr.set_init (p.prob_rirw().init());
   hdr.set_lead (p.prob_rirw().valid() - p.prob_rirw().init());
   hdr.set_valid(p.prob_rirw().valid());

   // Pass the description from the input line to the output
   if(p.line().n_items() > 0) {
     hdr.set_desc((string)p.line().get_item("DESC", false));
   }

   // Write one line for all the probabilities
   write_tc_header_cols(hdr, at, i_row);
   write_prob_rirw_cols(p, -1, at, i_row, n_tc_header_cols);
   i_row++;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_tc_header_cols(const TcHdrColumns &hdr,
                          AsciiTable &at, int r) {
   int c = 0;

   // Header columns:
   //    VERSION,    AMODEL,     BMODEL,
   //    DESC,       STORM_ID,   BASIN,
   //    CYCLONE,    STORM_NAME, INIT,
   //    LEAD,       VALID,      INIT_MASK,
   //    VALID_MASK, LINE_TYPE

   at.set_entry(r, c++, met_version);
   at.set_entry(r, c++, hdr.adeck_model());
   at.set_entry(r, c++, hdr.bdeck_model());
   at.set_entry(r, c++, hdr.desc());
   at.set_entry(r, c++, hdr.storm_id());
   at.set_entry(r, c++, hdr.basin());
   at.set_entry(r, c++, hdr.cyclone());
   if(!hdr.storm_name().empty()) at.set_entry(r, c++, hdr.storm_name());
   else                          at.set_entry(r, c++, na_str);
   if(hdr.init() > 0)            at.set_entry(r, c++, unix_to_yyyymmdd_hhmmss(hdr.init()));
   else                          at.set_entry(r, c++, na_str);
   if(!is_bad_data(hdr.lead()))  at.set_entry(r, c++, sec_to_hhmmss(hdr.lead()));
   else                          at.set_entry(r, c++, na_str);
   if(hdr.valid() > 0)           at.set_entry(r, c++, unix_to_yyyymmdd_hhmmss(hdr.valid()));
   else                          at.set_entry(r, c++, na_str);
   at.set_entry(r, c++, hdr.init_mask());
   at.set_entry(r, c++, hdr.valid_mask());
   at.set_entry(r, c++, hdr.line_type());

   return;
}

////////////////////////////////////////////////////////////////////////

void write_tc_mpr_cols(const TrackPairInfo &p, int i,
                       AsciiTable &at, int r, int c) {
   int j;

   // Write tc_mpr columns
   at.set_entry(r, c++, p.n_points());
   at.set_entry(r, c++, i+1);
   at.set_entry(r, c++, cyclonelevel_to_string(p.bdeck()[i].level()));
   at.set_entry(r, c++, watchwarntype_to_string(p.bdeck()[i].watch_warn()));
   if(p.adeck().initials().empty()) at.set_entry(r, c++, na_str);
   else                             at.set_entry(r, c++, p.adeck().initials());
   at.set_entry(r, c++, p.adeck()[i].lat());
   at.set_entry(r, c++, p.adeck()[i].lon());
   at.set_entry(r, c++, p.bdeck()[i].lat());
   at.set_entry(r, c++, p.bdeck()[i].lon());
   at.set_entry(r, c++, p.track_err(i));
   at.set_entry(r, c++, p.x_err(i));
   at.set_entry(r, c++, p.y_err(i));
   at.set_entry(r, c++, p.along_track_err(i));
   at.set_entry(r, c++, p.cross_track_err(i));
   at.set_entry(r, c++, p.adeck_dland(i));
   at.set_entry(r, c++, p.bdeck_dland(i));
   at.set_entry(r, c++, p.adeck()[i].mslp());
   at.set_entry(r, c++, p.bdeck()[i].mslp());
   at.set_entry(r, c++, p.adeck()[i].v_max());
   at.set_entry(r, c++, p.bdeck()[i].v_max());

   // Write the wind columns
   for(j=0; j<NWinds; j++) {
      at.set_entry(r, c++, p.adeck()[i][j].al_val());
      at.set_entry(r, c++, p.bdeck()[i][j].al_val());
      at.set_entry(r, c++, p.adeck()[i][j].ne_val());
      at.set_entry(r, c++, p.bdeck()[i][j].ne_val());
      at.set_entry(r, c++, p.adeck()[i][j].se_val());
      at.set_entry(r, c++, p.bdeck()[i][j].se_val());
      at.set_entry(r, c++, p.adeck()[i][j].sw_val());
      at.set_entry(r, c++, p.bdeck()[i][j].sw_val());
      at.set_entry(r, c++, p.adeck()[i][j].nw_val());
      at.set_entry(r, c++, p.bdeck()[i][j].nw_val());
   } // end for j

   // Write remaining columns
   at.set_entry(r, c++, p.adeck()[i].radp());
   at.set_entry(r, c++, p.bdeck()[i].radp());
   at.set_entry(r, c++, p.adeck()[i].rrp());
   at.set_entry(r, c++, p.bdeck()[i].rrp());
   at.set_entry(r, c++, p.adeck()[i].mrd());
   at.set_entry(r, c++, p.bdeck()[i].mrd());
   at.set_entry(r, c++, p.adeck()[i].gusts());
   at.set_entry(r, c++, p.bdeck()[i].gusts());
   at.set_entry(r, c++, p.adeck()[i].eye());
   at.set_entry(r, c++, p.bdeck()[i].eye());
   at.set_entry(r, c++, p.adeck()[i].direction());
   at.set_entry(r, c++, p.bdeck()[i].direction());
   at.set_entry(r, c++, p.adeck()[i].speed());
   at.set_entry(r, c++, p.bdeck()[i].speed());
   at.set_entry(r, c++, systemsdepth_to_string(p.adeck()[i].depth()));
   at.set_entry(r, c++, systemsdepth_to_string(p.bdeck()[i].depth()));

   return;
}

////////////////////////////////////////////////////////////////////////

void write_prob_rirw_cols(const ProbRIRWPairInfo &p, int i,
                          AsciiTable &at, int r, int c) {
   int j;
   double v;

   // Write PROBRIRW columns
   at.set_entry(r, c++, p.prob_rirw().lat());
   at.set_entry(r, c++, p.prob_rirw().lon());
   at.set_entry(r, c++, p.blat());
   at.set_entry(r, c++, p.blon());
   if(p.prob_rirw().initials() != "") at.set_entry(r, c++, na_str);
   else                         at.set_entry(r, c++, p.prob_rirw().initials());
   at.set_entry(r, c++, p.track_err());
   at.set_entry(r, c++, p.x_err());
   at.set_entry(r, c++, p.y_err());
   at.set_entry(r, c++, p.adland());
   at.set_entry(r, c++, p.bdland());
   at.set_entry(r, c++, p.prob_rirw().rirw_beg());
   at.set_entry(r, c++, p.prob_rirw().rirw_end());
   at.set_entry(r, c++, p.prob_rirw().rirw_window());
   at.set_entry(r, c++, p.prob_rirw().value());
   at.set_entry(r, c++, p.bbegv());
   at.set_entry(r, c++, p.bendv());

   // BDELTA is end minus begin
   v = (is_bad_data(p.bbegv()) || is_bad_data(p.bendv()) ?
        bad_data_double : p.bendv() - p.bbegv());
   at.set_entry(r, c++, v);

   // BDELTA_MAX
   if(is_bad_data(p.bbegv()) || is_bad_data(p.bendv()) ||
      is_bad_data(p.bminv()) || is_bad_data(p.bmaxv())) {
      v = bad_data_double;
   }
   else if(p.bbegv() <= p.bendv()) v = p.bendv() - p.bminv();
   else                            v = p.bendv() - p.bmaxv();
   at.set_entry(r, c++, v);

   at.set_entry(r, c++, cyclonelevel_to_string(p.bbeglev()));
   at.set_entry(r, c++, cyclonelevel_to_string(p.bendlev()));

   // Write all the probabilities (i == -1)
   if(i<0) {
      at.set_entry(r, c++, p.prob_rirw().n_prob());
      for(j=0; j<p.prob_rirw().n_prob(); j++) {
         at.set_entry(r, c++, p.prob_rirw().prob_item(j));
         at.set_entry(r, c++, p.prob_rirw().prob(j));
      }
   }
   // Write the i-th probability
   else {
      at.set_entry(r, c++, 1);
      at.set_entry(r, c++, p.prob_rirw().prob_item(i));
      at.set_entry(r, c++, p.prob_rirw().prob(i));
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void justify_tc_stat_cols(AsciiTable &at) {

   justify_met_at(at, n_tc_header_cols);

   return;
}

////////////////////////////////////////////////////////////////////////
