// // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
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

#include "vx_util.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

int get_tc_col_offset(const char **arr, int n_cols, const char *col_name) {
   int i, offset;
   bool found = false;

   // Search the TC header columns first
   for(i=0; i<n_tc_header_cols; i++) {
      if(strcasecmp(tc_header_cols[i], col_name) == 0) {
         found  = true;
         offset = i;
         break;
      }
   }

   // If not found, search the columns provided
   if(!found) {
      for(i=0; i<n_cols; i++) {

         if(strcasecmp(arr[i], col_name) == 0) {
            found  = true;
            offset = i+n_tc_header_cols;
            break;
         }
      }
   }

   if(!found) {
      mlog << Error
           << "\nget_tc_col_offset() -> "
           << "no match found in the indicated array for the column name "
           << "specified: \"" << col_name << "\"\n\n";
      exit(1);
   }

   return(offset);
}

////////////////////////////////////////////////////////////////////////

int get_tc_mpr_col_offset(const char *col_name) {
   int i, j, wind, offset;
   bool found = false;
   ConcatString s;

   // Search the TC header columns first
   for(i=0; i<n_tc_header_cols; i++) {
      if(strcasecmp(tc_header_cols[i], col_name) == 0) {
         found  = true;
         offset = i;
         break;
      }
   }

   // If not found, search the TC MPR columns:
   //    TOTAL,       INDEX,      LEVEL,
   //    ALAT,        ALON,
   //    BLAT,        BLON,
   //    TK_ERR,      X_ERR,      Y_ERR,
   //    ALTK_ERR,    CRTK_ERR,
   //    ADLAND,      BDLAND,
   //    AMSLP,       BMSLP,
   //    AMAX_WIND,   BMAX_WIND,
   //   [AQUAD_WIND_, BQUAD_WIND_,
   //    ARAD1_WIND_, BRAD1_WIND_,
   //    ARAD2_WIND_, BRAD2_WIND_,
   //    ARAD3_WIND_, BRAD3_WIND_,
   //    ARAD4_WIND_, BRAD4_WIND_]
   //    (for each wind intensity value)

   // Check the static columns
   if(!found) {

      // Loop through the static columns looking for a match
      for(i=0; i<n_tc_mpr_static; i++) {

         // Check for a match
         if(strcasecmp(tc_mpr_cols[i], col_name) == 0) {
            found  = true;
            offset = n_tc_header_cols + i;
            break;
         }
      }
   }

   // Check the variable columns
   if(!found) {

      // Loop through the variable columns looking for a match
      for(i=0; i<n_tc_mpr_var; i++) {

         if(strncasecmp(tc_mpr_cols[n_tc_mpr_static + i], col_name,
                        strlen(tc_mpr_cols[n_tc_mpr_static + i])) == 0) {

            // Parse the wind intensity from col_name
            wind = parse_wind_intensity(col_name);

            // Loop through the wind intensities
            for(j=0; j<NWinds; j++) {
               if(WindIntensity[j] == wind) {
                  found  = true;
                  offset = n_tc_header_cols + n_tc_mpr_static + j*n_tc_mpr_var + i;
                  break;
               }
            } // end for j

            // Check if it's been found
            if(found) break;
         }
      } // end for i
   }
   
   if(!found) {
      mlog << Error
           << "\nget_tc_mpr_col_offset() -> "
           << "no match found for the column name specified: \""
           << col_name << "\"\n\n";
      exit(1);
   }

   return(offset);
}

////////////////////////////////////////////////////////////////////////

int parse_wind_intensity(const char *col_name) {
   int i;
   const char *ptr;

   if((ptr = strrchr(col_name, '_')) != NULL) i = atoi(++ptr);
   else {
      mlog << Error
           << "\nparse_wind_intensity() -> "
           << "unexpected column name specified: \""
           << col_name << "\"\n\n";
      exit(1);
   }

   return(i);
}

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
   int i, j;
   ConcatString s;

   // Write the header column names if requested
   if(hdr_flag) {
      for(i=0; i<n_tc_header_cols; i++)
         at.set_entry(r, c++, tc_header_cols[i]);
   }

   // Write the static columns names specific to the TCMPR line type
   for(i=0; i<n_tc_mpr_static; i++) {
      at.set_entry(r, c++, tc_mpr_cols[i]);
   }

   // Write the variable columns names specific to the TCMPR line type
   for(i=0; i<NWinds; i++) {
      for(j=0; j<n_tc_mpr_var; j++) {
         s.format("%s%i", tc_mpr_cols[n_tc_mpr_static + j],
                  WindIntensity[i]);
         at.set_entry(r, c++, s);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_tc_mpr_row(TcHdrColumns &hdr, const TrackPairInfo &p,
                      AsciiTable &at, int &i_row) {
   int i;

   // TCMPR line type
   hdr.set_line_type("TCMPR");
   
   // Loop through the TrackPairInfo points
   for(i=0; i<p.n_points(); i++) {

      // Timing information

      // Initialization and lead time for the ADECK
      hdr.set_init(p.adeck().init());
      hdr.set_lead(p.adeck()[i].lead());

      // Valid time for the ADECK and/or BDECK
      if(p.adeck()[i].valid() > 0) hdr.set_valid(p.adeck()[i].valid());
      else                         hdr.set_valid(p.bdeck()[i].valid());
     
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

void write_tc_header_cols(const TcHdrColumns &hdr,
                          AsciiTable &at, int r) {
   int c = 0;

   // Header columns:
   //    VERSION,      AMODEL,     BDECK_MODEL,
   //    BASIN,        CYCLONE,    STORM_NAME,
   //    INIT,         LEAD,       VALID,
   //    INIT_MASK,    VALID_MASK, LINE_TYPE

   at.set_entry(r, c++, met_version);
   at.set_entry(r, c++, hdr.adeck_model());
   at.set_entry(r, c++, hdr.bdeck_model());
   at.set_entry(r, c++, hdr.basin());
   at.set_entry(r, c++, hdr.cyclone());
   at.set_entry(r, c++, hdr.storm_name());
   if(hdr.init() > 0)           at.set_entry(r, c++, unix_to_yyyymmdd_hhmmss(hdr.init()));
   else                         at.set_entry(r, c++, na_str);
   if(!is_bad_data(hdr.lead())) at.set_entry(r, c++, sec_to_hhmmss(hdr.lead()));
   else                         at.set_entry(r, c++, na_str);
   if(hdr.valid() > 0)          at.set_entry(r, c++, unix_to_yyyymmdd_hhmmss(hdr.valid()));
   else                         at.set_entry(r, c++, na_str);
   at.set_entry(r, c++, hdr.init_mask());
   at.set_entry(r, c++, hdr.valid_mask());
   at.set_entry(r, c++, hdr.line_type());

   return;
}

////////////////////////////////////////////////////////////////////////

void write_tc_mpr_cols(const TrackPairInfo &p, int i,
                       AsciiTable &at, int r, int c) {
   int j;
   WatchWarnType ww_type;

   // Get the most severe watch/warning type
   ww_type = ww_max(p.adeck()[i].watch_warn(), p.bdeck()[i].watch_warn());

   // Write static columns
   at.set_entry(r, c++, p.n_points());
   at.set_entry(r, c++, i+1);
   at.set_entry(r, c++, cyclonelevel_to_string(p.bdeck()[i].level()));
   at.set_entry(r, c++, watchwarntype_to_string(p.bdeck()[i].watch_warn()));
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

   // Write variable columns
   for(j=0; j<NWinds; j++) {
      at.set_entry(r, c++, quadranttype_to_string(p.adeck()[i][j].quadrant()));
      at.set_entry(r, c++, quadranttype_to_string(p.bdeck()[i][j].quadrant()));
      at.set_entry(r, c++, p.adeck()[i][j][0]);
      at.set_entry(r, c++, p.bdeck()[i][j][0]);
      at.set_entry(r, c++, p.adeck()[i][j][1]);
      at.set_entry(r, c++, p.bdeck()[i][j][1]);
      at.set_entry(r, c++, p.adeck()[i][j][2]);
      at.set_entry(r, c++, p.bdeck()[i][j][2]);
      at.set_entry(r, c++, p.adeck()[i][j][3]);
      at.set_entry(r, c++, p.bdeck()[i][j][3]);
   } // end for j

   return;
}

////////////////////////////////////////////////////////////////////////
