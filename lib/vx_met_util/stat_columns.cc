// // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
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

#include "vx_met_util/constants.h"
#include "vx_met_util/stat_columns.h"

////////////////////////////////////////////////////////////////////////

int get_column_offset(const char **arr, int n_cols, const char *col_name) {
   int i, offset, found;

   found = 0;

   //
   // Search the STAT header columns first
   //
   for(i=0; i<n_header_columns; i++) {
      if(strcasecmp(hdr_columns[i], col_name) == 0) {
         found  = 1;
         offset = i;
         break;
      }
   }

   //
   // If not found, search the columns provided
   //
   if(!found) {
      for(i=0; i<n_cols; i++) {

         if(strcasecmp(arr[i], col_name) == 0) {
            found  = 1;
            offset = i+n_header_columns;
            break;
         }
      }
   }

   if(!found) {
      cerr << "\n\nERROR: get_column_offset() -> "
           << "no match found in the indicated array for the column name "
           << "specified: \"" << col_name << "\"\n\n" 
           << flush;

      throw(1);
   }

   return(offset);
}

////////////////////////////////////////////////////////////////////////

int get_mctc_column_offset(const char *col_name, int n_cat) {
   int i, j, offset, found;

   found = 0;

   //
   // Search the STAT header columns first
   //
   for(i=0; i<n_header_columns; i++) {
      if(strcasecmp(hdr_columns[i], col_name) == 0) {
         found  = 1;
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
   if(!found) {
      for(i=0; i<2; i++) {

         if(strcasecmp(mctc_columns[i], col_name) == 0) {
            found  = 1;
            offset = i+n_header_columns;
            break;
         }
      }
   }

   //
   // Check the variable colum
   //
   if(!found) {

      // Fi_Oj: Parse out i and j
      parse_row_col(col_name, i, j);
      offset = n_header_columns + 2 + (i-1)*n_cat + (j-1);
      found  = 1;
   }

   if(!found) {
      cerr << "\n\nERROR: get_mctc_column_offset() -> "
           << "no match found for the column name specified: \""
           << col_name << "\"\n\n" << flush;
      throw(1);
   }

   return(offset);
}

////////////////////////////////////////////////////////////////////////

int get_pct_column_offset(const char *col_name) {
   int i, offset, found;

   found = 0;

   //
   // Search the STAT header columns first
   //
   for(i=0; i<n_header_columns; i++) {
      if(strcasecmp(hdr_columns[i], col_name) == 0) {
         found  = 1;
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
   if(!found) {
      for(i=0; i<2; i++) {

         if(strcasecmp(pct_columns[i], col_name) == 0) {
            found  = 1;
            offset = i+n_header_columns;
            break;
         }
      }
   }

   //
   // Check the variable columns
   //
   if(!found) {

      // THRESH_i
      if(strncasecmp(pct_columns[2], col_name,
                     strlen(pct_columns[2])) == 0) {

         found  = 1;
         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 3*(i-1) + 0;
      }
      // OY_i
      else if(strncasecmp(pct_columns[3], col_name,
                          strlen(pct_columns[3])) == 0) {

         found  = 1;
         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 3*(i-1) + 1;
      }
      // ON_i
      else if(strncasecmp(pct_columns[4], col_name,
                          strlen(pct_columns[4])) == 0) {

         found  = 1;
         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 3*(i-1) + 2;
      }
   }

   if(!found) {
      cerr << "\n\nERROR: get_pct_column_offset() -> "
           << "no match found for the column name specified: \""
           << col_name << "\"\n\n" << flush;
      throw(1);
   }

   return(offset);
}

////////////////////////////////////////////////////////////////////////

int get_pstd_column_offset(const char *col_name) {
   int i, offset, found;

   found = 0;

   //
   // Search the STAT header columns first
   //
   for(i=0; i<n_header_columns; i++) {
      if(strcasecmp(hdr_columns[i], col_name) == 0) {
         found  = 1;
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
   //    [THRESH] (for each threshold)
   //

   //
   // Check the static columns
   //
   if(!found) {
      for(i=0; i<12; i++) {

         if(strcasecmp(pstd_columns[i], col_name) == 0) {
            found  = 1;
            offset = i+n_header_columns;
            break;
         }
      }
   }

   //
   // Check the variable columns
   //
   if(!found) {

      // THRESH_i
      if(strncasecmp(pstd_columns[12], col_name,
                     strlen(pstd_columns[12])) == 0) {

         found  = 1;
         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 12 + (i-1);
      }
   }

   if(!found) {
      cerr << "\n\nERROR: get_pstd_column_offset() -> "
           << "no match found for the column name specified: \""
           << col_name << "\"\n\n" << flush;
      throw(1);
   }

   return(offset);
}

////////////////////////////////////////////////////////////////////////

int get_pjc_column_offset(const char *col_name) {
   int i, offset, found;

   found = 0;

   //
   // Search the STAT header columns first
   //
   for(i=0; i<n_header_columns; i++) {
      if(strcasecmp(hdr_columns[i], col_name) == 0) {
         found  = 1;
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
   if(!found) {
      for(i=0; i<2; i++) {

         if(strcasecmp(pjc_columns[i], col_name) == 0) {
            found  = 1;
            offset = i+n_header_columns;
            break;
         }
      }
   }

   //
   // Check the variable columns
   //
   if(!found) {

      // THRESH_i
      if(strncasecmp(pjc_columns[2], col_name,
                     strlen(pjc_columns[2])) == 0) {

         found  = 1;
         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 7*(i-1) + 0;
      }
      // OY_TP_i
      else if(strncasecmp(pjc_columns[3], col_name,
                          strlen(pjc_columns[3])) == 0) {

         found  = 1;
         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 7*(i-1) + 1;
      }
      // ON_TP_i
      else if(strncasecmp(pjc_columns[4], col_name,
                          strlen(pjc_columns[4])) == 0) {

         found  = 1;
         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 7*(i-1) + 2;
      }
      // CALIBRATION_i
      else if(strncasecmp(pjc_columns[5], col_name,
                          strlen(pjc_columns[5])) == 0) {

         found  = 1;
         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 7*(i-1) + 3;
      }
      // REFINEMENT_i
      else if(strncasecmp(pjc_columns[6], col_name,
                          strlen(pjc_columns[6])) == 0) {

         found  = 1;
         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 7*(i-1) + 4;
      }
      // LIKELIHOOD_i
      else if(strncasecmp(pjc_columns[7], col_name,
                          strlen(pjc_columns[7])) == 0) {

         found  = 1;
         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 7*(i-1) + 5;
      }
      // BASER_i
      else if(strncasecmp(pjc_columns[8], col_name,
                          strlen(pjc_columns[8])) == 0) {

         found  = 1;
         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 7*(i-1) + 6;
      }
   }

   if(!found) {
      cerr << "\n\nERROR: get_pjc_column_offset() -> "
           << "no match found for the column name specified: \""
           << col_name << "\"\n\n" << flush;
      throw(1);
   }

   return(offset);
}

////////////////////////////////////////////////////////////////////////

int get_prc_column_offset(const char *col_name) {
   int i, offset, found;

   found = 0;

   //
   // Search the STAT header columns first
   //
   for(i=0; i<n_header_columns; i++) {
      if(strcasecmp(hdr_columns[i], col_name) == 0) {
         found  = 1;
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
   if(!found) {
      for(i=0; i<2; i++) {

         if(strcasecmp(prc_columns[i], col_name) == 0) {
            found  = 1;
            offset = i+n_header_columns;
            break;
         }
      }
   }

   //
   // Check the variable columns
   //
   if(!found) {

      // THRESH_i
      if(strncasecmp(prc_columns[2], col_name,
                     strlen(prc_columns[2])) == 0) {

         found  = 1;
         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 3*(i-1) + 0;
      }
      // PODY_i
      else if(strncasecmp(prc_columns[3], col_name,
                          strlen(prc_columns[3])) == 0) {

         found  = 1;
         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 3*(i-1) + 1;
      }
      // POFD_i
      else if(strncasecmp(prc_columns[4], col_name,
                          strlen(prc_columns[4])) == 0) {

         found  = 1;
         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 2 + 3*(i-1) + 2;
      }
   }

   if(!found) {
      cerr << "\n\nERROR: get_prc_column_offset() -> "
           << "no match found for the column name specified: \""
           << col_name << "\"\n\n" << flush;
      throw(1);
   }

   return(offset);
}

////////////////////////////////////////////////////////////////////////

int get_rhist_column_offset(const char *col_name) {
   int i, offset, found;

   found = 0;

   //
   // Search the STAT header columns first
   //
   for(i=0; i<n_header_columns; i++) {
      if(strcasecmp(hdr_columns[i], col_name) == 0) {
         found  = 1;
         offset = i;
         break;
      }
   }

   //
   // If not found, search the rhist columns:
   //    TOTAL,  CRPS,  IGN,
   //    N_RANK, [RANK_] (for possible ranks, n_ens+1)
   //

   //
   // Check the static columns
   //
   if(!found) {
      for(i=0; i<4; i++) {

         if(strcasecmp(rhist_columns[i], col_name) == 0) {
            found  = 1;
            offset = i+n_header_columns;
            break;
         }
      }
   }

   //
   // Check the variable column
   //
   if(!found) {

      // RANK_i
      if(strncasecmp(rhist_columns[4], col_name,
                     strlen(rhist_columns[4])) == 0) {
         found  = 1;
         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 4 + (i-1);
      }
   }

   if(!found) {
      cerr << "\n\nERROR: get_rhist_column_offset() -> "
           << "no match found for the column name specified: \""
           << col_name << "\"\n\n" << flush;
      throw(1);
   }

   return(offset);
}

////////////////////////////////////////////////////////////////////////

int get_orank_column_offset(const char *col_name) {
   int i, offset, found;

   found = 0;

   //
   // Search the STAT header columns first
   //
   for(i=0; i<n_header_columns; i++) {
      if(strcasecmp(hdr_columns[i], col_name) == 0) {
         found  = 1;
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
   //

   //
   // Check the static columns
   //
   if(!found) {
      for(i=0; i<12; i++) {

         if(strcasecmp(orank_columns[i], col_name) == 0) {
            found  = 1;
            offset = i+n_header_columns;
            break;
         }
      }
   }

   //
   // Check the variable column
   //
   if(!found) {

      // ENS_i
      if(strncasecmp(orank_columns[13], col_name,
                     strlen(orank_columns[13])) == 0) {
         found  = 1;
         i      = parse_thresh_index(col_name);
         offset = n_header_columns + 13 + (i-1);
      }
   }

   if(!found) {
      cerr << "\n\nERROR: get_orank_column_offset() -> "
           << "no match found for the column name specified: \""
           << col_name << "\"\n\n" << flush;
      throw(1);
   }

   return(offset);
}

////////////////////////////////////////////////////////////////////////

int parse_thresh_index(const char *col_name) {
   int i;
   const char *ptr;

   if((ptr = strrchr(col_name, '_')) != NULL) i = atoi(++ptr);
   else {
      cerr << "\n\nERROR: parse_thresh_index() -> "
           << "unexpected column name specified: \""
           << col_name << "\"\n\n" << flush;
      throw(1);
   }

   return(i);
}

////////////////////////////////////////////////////////////////////////

void parse_row_col(const char *col_name, int &r, int &c) {
   const char *ptr;

   // Parse Fi_Oj strings
   r = atoi(++col_name);

   if((ptr = strrchr(col_name, '_')) != NULL) c = atoi(++ptr);
   else {
      cerr << "\n\nERROR: parse_row_col() -> "
           << "unexpected column name specified: \""
           << col_name << "\"\n\n" << flush;
      throw(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void open_txt_file(ofstream *&out, const char *file_name,
                   int verbosity) {

   // Create and open the output file stream
   out = new ofstream;
   out->open(file_name);

   if(!(*out)) {
      cerr << "\n\nERROR: open_txt_file()-> "
           << "can't open the output file \"" << file_name
           << "\" for writing!\n\n" << flush;
      exit(1);
   }

   out->setf(ios::fixed);

   return;
}

////////////////////////////////////////////////////////////////////////

void close_txt_file(ofstream *&out, const char *file_name,
                    int verbosity) {

   // List the file being closed
   if(verbosity > 0) {
      cout << "Output file: "
           << file_name << "\n" << flush;
   }

   // Close the output file
   out->close();
   delete out;
   out = (ofstream *) 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_header_row(const char **cols, int n_cols, int hdr_flag,
                      AsciiTable &at, int r, int c) {
   int i;

   // Write the header column names if requested
   if(hdr_flag) {
      for(i=0; i<n_header_columns; i++)
         at.set_entry(r, i+c, hdr_columns[i]);

      c += n_header_columns;
   }

   // Write the columns names specific to this line type
   for(i=0; i<n_cols; i++)
      at.set_entry(r, i+c, cols[i]);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_pct_header_row(int hdr_flag, int n_thresh, AsciiTable &at,
                          int r, int c) {
   int i, col;
   char tmp_str[max_str_len];

   // Write the header column names if requested
   if(hdr_flag) {
      for(i=0; i<n_header_columns; i++)
         at.set_entry(r, i+c, hdr_columns[i]);

      c += n_header_columns;
   }

   // Write the columns names specific to the PCT line type
   at.set_entry(r, c+0, pct_columns[0]);
   at.set_entry(r, c+1, pct_columns[1]);

   // Write THRESH_i, OY_i, ON_i for each row of the Nx2 table
   for(i=0, col=c+2; i<n_thresh-1; i++) {

      sprintf(tmp_str, "%s%i", pct_columns[2], i+1);
      at.set_entry(r, col, tmp_str); // Threshold
      col++;

      sprintf(tmp_str, "%s%i", pct_columns[3], i+1);
      at.set_entry(r, col, tmp_str); // Event Count (OY)
      col++;

      sprintf(tmp_str, "%s%i", pct_columns[4], i+1);
      at.set_entry(r, col, tmp_str); // Non-Event Count (ON)
      col++;
   }

   // Write out the last threshold
   if(n_thresh >= 1) {
      sprintf(tmp_str, "%s%i", pct_columns[2], n_thresh);
      at.set_entry(r, col, tmp_str);    // Threshold
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_pstd_header_row(int hdr_flag, int n_thresh, AsciiTable &at,
                           int r, int c) {
   int i, col;
   char tmp_str[max_str_len];

   // Write the header column names if requested
   if(hdr_flag) {
      for(i=0; i<n_header_columns; i++)
         at.set_entry(r, i+c, hdr_columns[i]);

      c += n_header_columns;
   }

   // Write the columns names specific to the pstd line type
   at.set_entry(r, c+0,  pstd_columns[0]);
   at.set_entry(r, c+1,  pstd_columns[1]);
   at.set_entry(r, c+2,  pstd_columns[2]);
   at.set_entry(r, c+3,  pstd_columns[3]);
   at.set_entry(r, c+4,  pstd_columns[4]);
   at.set_entry(r, c+5,  pstd_columns[5]);
   at.set_entry(r, c+6,  pstd_columns[6]);
   at.set_entry(r, c+7,  pstd_columns[7]);
   at.set_entry(r, c+8,  pstd_columns[8]);
   at.set_entry(r, c+9,  pstd_columns[9]);
   at.set_entry(r, c+10, pstd_columns[10]);
   at.set_entry(r, c+11, pstd_columns[11]);

   // Write THRESH_i for each threshold
   for(i=0, col=c+12; i<n_thresh; i++) {

      sprintf(tmp_str, "%s%i", pstd_columns[12], i+1);
      at.set_entry(r, col, tmp_str); // Threshold
      col++;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_pjc_header_row(int hdr_flag, int n_thresh, AsciiTable &at,
                          int r, int c) {
   int i, col;
   char tmp_str[max_str_len];

   // Write the header column names if requested
   if(hdr_flag) {
      for(i=0; i<n_header_columns; i++)
         at.set_entry(r, i+c, hdr_columns[i]);

      c += n_header_columns;
   }

   // Write the columns names specific to the PJC line type
   at.set_entry(r, c+0, pjc_columns[0]);
   at.set_entry(r, c+1, pjc_columns[1]);

   // Write THRESH_i, OY_TP_i, ON_TP_i, CALIBRATION_i, REFINEMENT_i,
   // LIKELIHOOD_i, and BASER_i for each row of the Nx2 table
   for(i=0, col=c+2; i<n_thresh-1; i++) {

      sprintf(tmp_str, "%s%i", pjc_columns[2], i+1);
      at.set_entry(r, col, tmp_str); // Threshold
      col++;

      sprintf(tmp_str, "%s%i", pjc_columns[3], i+1);
      at.set_entry(r, col, tmp_str); // Event Count/N (OY_TP)
      col++;

      sprintf(tmp_str, "%s%i", pjc_columns[4], i+1);
      at.set_entry(r, col, tmp_str); // Non-Event Count/N (ON_TP)
      col++;

      sprintf(tmp_str, "%s%i", pjc_columns[5], i+1);
      at.set_entry(r, col, tmp_str); // Calibration
      col++;

      sprintf(tmp_str, "%s%i", pjc_columns[6], i+1);
      at.set_entry(r, col, tmp_str); // Refinement
      col++;

      sprintf(tmp_str, "%s%i", pjc_columns[7], i+1);
      at.set_entry(r, col, tmp_str); // Likelihood
      col++;

      sprintf(tmp_str, "%s%i", pjc_columns[8], i+1);
      at.set_entry(r, col, tmp_str); // Base Rate
      col++;
   }

   // Write out the last threshold
   if(n_thresh >= 1) {
      sprintf(tmp_str, "%s%i", pct_columns[2], n_thresh);
      at.set_entry(r, col, tmp_str);    // Threshold
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_prc_header_row(int hdr_flag, int n_thresh, AsciiTable &at,
                          int r, int c) {
   int i, col;
   char tmp_str[max_str_len];

   // Write the header column names if requested
   if(hdr_flag) {
      for(i=0; i<n_header_columns; i++)
         at.set_entry(r, i+c, hdr_columns[i]);

      c += n_header_columns;
   }

   // Write the columns names specific to the PRC line type
   at.set_entry(r, c+0, prc_columns[0]);
   at.set_entry(r, c+1, prc_columns[1]);

   // Write THRESH_i, PODY_i, POFD_i for each row of the Nx2 table
   for(i=0, col=c+2; i<n_thresh-1; i++) {

      sprintf(tmp_str, "%s%i", prc_columns[2], i+1);
      at.set_entry(r, col, tmp_str); // Threshold
      col++;

      sprintf(tmp_str, "%s%i", prc_columns[3], i+1);
      at.set_entry(r, col, tmp_str); // PODY
      col++;

      sprintf(tmp_str, "%s%i", prc_columns[4], i+1);
      at.set_entry(r, col, tmp_str); // POFD
      col++;
   }

   // Write out the last threshold
   if(n_thresh >= 1) {
      sprintf(tmp_str, "%s%i", prc_columns[2], n_thresh);
      at.set_entry(r, col, tmp_str);    // Threshold
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_rhist_header_row(int hdr_flag, int n_rank, AsciiTable &at,
                            int r, int c) {
   int i, col;
   char tmp_str[max_str_len];

   // Write the header column names if requested
   if(hdr_flag) {
      for(i=0; i<n_header_columns; i++)
         at.set_entry(r, i+c, hdr_columns[i]);

      c += n_header_columns;
   }

   // Write the columns names specific to the RHIST line type
   at.set_entry(r, c+0, rhist_columns[0]);
   at.set_entry(r, c+1, rhist_columns[1]);
   at.set_entry(r, c+2, rhist_columns[2]);
   at.set_entry(r, c+3, rhist_columns[3]);

   // Write N_i for each ensemble member
   for(i=0, col=c+4; i<n_rank; i++) {

      sprintf(tmp_str, "%s%i", rhist_columns[4], i+1);
      at.set_entry(r, col, tmp_str); // Counts for each rank
      col++;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_orank_header_row(int hdr_flag, int n_ens, AsciiTable &at,
                            int r, int c) {
   int i, col;
   char tmp_str[max_str_len];

   // Write the header column names if requested
   if(hdr_flag) {
      for(i=0; i<n_header_columns; i++)
         at.set_entry(r, i+c, hdr_columns[i]);

      c += n_header_columns;
   }

   // Write the columns names specific to the orank line type
   at.set_entry(r, c+0,  orank_columns[0]);
   at.set_entry(r, c+1,  orank_columns[1]);
   at.set_entry(r, c+2,  orank_columns[2]);
   at.set_entry(r, c+3,  orank_columns[3]);
   at.set_entry(r, c+4,  orank_columns[4]);
   at.set_entry(r, c+5,  orank_columns[5]);
   at.set_entry(r, c+6,  orank_columns[6]);
   at.set_entry(r, c+7,  orank_columns[7]);
   at.set_entry(r, c+8,  orank_columns[8]);
   at.set_entry(r, c+9,  orank_columns[9]);
   at.set_entry(r, c+10, orank_columns[10]);
   at.set_entry(r, c+11, orank_columns[11]);

   // Write ENS_i for each ensemble member
   for(i=0, col=c+12; i<n_ens; i++) {

      sprintf(tmp_str, "%s%i", orank_columns[12], i+1);
      at.set_entry(r, col, tmp_str); // Ensemble member value
      col++;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_fho_row(StatHdrColumns &shc, const CTSInfo &cts_info,
                   int out_flag,
                   AsciiTable &stat_at, int &stat_row,
                   AsciiTable &txt_at, int &txt_row) {

   // FHO line type
   shc.set_line_type("FHO");

   // Thresholds
   shc.set_fcst_thresh(cts_info.cts_fcst_thresh);
   shc.set_obs_thresh(cts_info.cts_obs_thresh);

   // Not Applicable
   shc.set_alpha(bad_data_double);
   shc.clear_cov_thresh();

   // Write the header columns
   write_header_cols(shc, stat_at, stat_row);

   // Write the data columns
   write_fho_cols(cts_info, stat_at, stat_row, n_header_columns);

   // If requested, copy row to the text file
   if(out_flag >= 2) {
      copy_ascii_table_row(stat_at, stat_row, txt_at, txt_row);

      // Increment the text row counter
      txt_row++;
   }

   // Increment the STAT row counter
   stat_row++;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_ctc_row(StatHdrColumns &shc, const CTSInfo &cts_info,
                   int out_flag,
                   AsciiTable &stat_at, int &stat_row,
                   AsciiTable &txt_at, int &txt_row) {

   // CTC line type
   shc.set_line_type("CTC");

   // Thresholds
   shc.set_fcst_thresh(cts_info.cts_fcst_thresh);
   shc.set_obs_thresh(cts_info.cts_obs_thresh);

   // Not Applicable
   shc.set_alpha(bad_data_double);
   shc.clear_cov_thresh();

   // Write the header columns
   write_header_cols(shc, stat_at, stat_row);

   // Write the data columns
   write_ctc_cols(cts_info, stat_at, stat_row, n_header_columns);

   // If requested, copy row to the text file
   if(out_flag >= 2) {
      copy_ascii_table_row(stat_at, stat_row, txt_at, txt_row);

      // Increment the text row counter
      txt_row++;
   }

   // Increment the STAT row counter
   stat_row++;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_cts_row(StatHdrColumns &shc, const CTSInfo &cts_info,
                   int out_flag,
                   AsciiTable &stat_at, int &stat_row,
                   AsciiTable &txt_at, int &txt_row) {
   int i;

   // CTS line type
   shc.set_line_type("CTS");

   // Thresholds
   shc.set_fcst_thresh(cts_info.cts_fcst_thresh);
   shc.set_obs_thresh(cts_info.cts_obs_thresh);

   // Not Applicable
   shc.clear_cov_thresh();

   // Write a line for each alpha value
   for(i=0; i<cts_info.n_alpha; i++) {

      // Alpha value
      shc.set_alpha(cts_info.alpha[i]);

      // Write the header columns
      write_header_cols(shc, stat_at, stat_row);

      // Write the data columns
      write_cts_cols(cts_info, i, stat_at, stat_row, n_header_columns);

      // If requested, copy row to the text file
      if(out_flag >= 2) {
         copy_ascii_table_row(stat_at, stat_row, txt_at, txt_row);

         // Increment the text row counter
         txt_row++;
      }

      // Increment the STAT row counter
      stat_row++;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_mctc_row(StatHdrColumns &shc, const MCTSInfo &mcts_info,
                    int out_flag,
                    AsciiTable &stat_at, int &stat_row,
                    AsciiTable &txt_at, int &txt_row) {

   // MCTC line type
   shc.set_line_type("MCTC");

   // Thresholds
   shc.set_fcst_thresh(mcts_info.cts_fcst_ta);
   shc.set_obs_thresh(mcts_info.cts_obs_ta);

   // Not Applicable
   shc.set_alpha(bad_data_double);
   shc.clear_cov_thresh();

   // Write the header columns
   write_header_cols(shc, stat_at, stat_row);

   // Write the data columns
   write_mctc_cols(mcts_info, stat_at, stat_row, n_header_columns);

   // If requested, copy row to the text file
   if(out_flag >= 2) {
      copy_ascii_table_row(stat_at, stat_row, txt_at, txt_row);

      // Increment the text row counter
      txt_row++;
   }

   // Increment the STAT row counter
   stat_row++;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_mcts_row(StatHdrColumns &shc, const MCTSInfo &mcts_info,
                    int out_flag,
                    AsciiTable &stat_at, int &stat_row,
                    AsciiTable &txt_at, int &txt_row) {
   int i;

   // MCTS line type
   shc.set_line_type("MCTS");

   // Thresholds
   shc.set_fcst_thresh(mcts_info.cts_fcst_ta);
   shc.set_obs_thresh(mcts_info.cts_obs_ta);

   // Not Applicable
   shc.clear_cov_thresh();

   // Write a line for each alpha value
   for(i=0; i<mcts_info.n_alpha; i++) {

      // Alpha value
      shc.set_alpha(mcts_info.alpha[i]);

      // Write the header columns
      write_header_cols(shc, stat_at, stat_row);

      // Write the data columns
      write_mcts_cols(mcts_info, i, stat_at, stat_row, n_header_columns);

      // If requested, copy row to the text file
      if(out_flag >= 2) {
         copy_ascii_table_row(stat_at, stat_row, txt_at, txt_row);

         // Increment the text row counter
         txt_row++;
      }

      // Increment the STAT row counter
      stat_row++;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_cnt_row(StatHdrColumns &shc, const CNTInfo &cnt_info,
                   int out_flag,
                   AsciiTable &stat_at, int &stat_row,
                   AsciiTable &txt_at, int &txt_row) {
   int i;

   // CNT line type
   shc.set_line_type("CNT");

   // Not Applicable
   shc.clear_fcst_thresh();
   shc.clear_obs_thresh();
   shc.clear_cov_thresh();

   // Write a line for each alpha value
   for(i=0; i<cnt_info.n_alpha; i++) {

      // Alpha value
      shc.set_alpha(cnt_info.alpha[i]);

      // Write the header columns
      write_header_cols(shc, stat_at, stat_row);

      // Write the data columns
      write_cnt_cols(cnt_info, i, stat_at, stat_row, n_header_columns);

      // If requested, copy row to the text file
      if(out_flag >= 2) {
         copy_ascii_table_row(stat_at, stat_row, txt_at, txt_row);

         // Increment the text row counter
         txt_row++;
      }

      // Increment the STAT row counter
      stat_row++;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_sl1l2_row(StatHdrColumns &shc, const CNTInfo &cnt_info,
                     int out_flag,
                     AsciiTable &stat_at, int &stat_row,
                     AsciiTable &txt_at, int &txt_row) {

   // SL1L2 line type
   shc.set_line_type("SL1L2");

   // Not Applicable
   shc.clear_fcst_thresh();
   shc.clear_obs_thresh();
   shc.clear_cov_thresh();
   shc.set_alpha(bad_data_double);

   // Write the header columns
   write_header_cols(shc, stat_at, stat_row);

   // Write the data columns
   write_sl1l2_cols(cnt_info, stat_at, stat_row, n_header_columns);

   // If requested, copy row to the text file
   if(out_flag >= 2) {
      copy_ascii_table_row(stat_at, stat_row, txt_at, txt_row);

      // Increment the text row counter
      txt_row++;
   }

   // Increment the STAT row counter
   stat_row++;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_sl1l2_row(StatHdrColumns &shc, const SL1L2Info &sl1l2_info,
                     int out_flag,
                     AsciiTable &stat_at, int &stat_row,
                     AsciiTable &txt_at, int &txt_row) {

   // SL1L2 line type
   shc.set_line_type("SL1L2");

   // Not Applicable
   shc.clear_fcst_thresh();
   shc.clear_obs_thresh();
   shc.clear_cov_thresh();
   shc.set_alpha(bad_data_double);

   // Write the header columns
   write_header_cols(shc, stat_at, stat_row);

   // Write the data columns
   write_sl1l2_cols(sl1l2_info, stat_at, stat_row, n_header_columns);

   // If requested, copy row to the text file
   if(out_flag >= 2) {
      copy_ascii_table_row(stat_at, stat_row, txt_at, txt_row);

      // Increment the text row counter
      txt_row++;
   }

   // Increment the STAT row counter
   stat_row++;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_sal1l2_row(StatHdrColumns &shc, const SL1L2Info &sl1l2_info,
                      int out_flag,
                      AsciiTable &stat_at, int &stat_row,
                      AsciiTable &txt_at, int &txt_row) {

   // SAL1L2 line type
   shc.set_line_type("SAL1L2");

   // Not Applicable
   shc.clear_fcst_thresh();
   shc.clear_obs_thresh();
   shc.clear_cov_thresh();
   shc.set_alpha(bad_data_double);

   // Write the header columns
   write_header_cols(shc, stat_at, stat_row);

   // Write the data columns
   write_sal1l2_cols(sl1l2_info, stat_at, stat_row, n_header_columns);

   // If requested, copy row to the text file
   if(out_flag >= 2) {
      copy_ascii_table_row(stat_at, stat_row, txt_at, txt_row);

      // Increment the text row counter
      txt_row++;
   }

   // Increment the STAT row counter
   stat_row++;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_vl1l2_row(StatHdrColumns &shc, const VL1L2Info &vl1l2_info,
                     int out_flag,
                     AsciiTable &stat_at, int &stat_row,
                     AsciiTable &txt_at, int &txt_row) {

   // VL1L2 line type
   shc.set_line_type("VL1L2");

   // Thresholds
   shc.set_fcst_thresh(vl1l2_info.wind_fcst_thresh);
   shc.set_obs_thresh(vl1l2_info.wind_obs_thresh);

   // Not Applicable
   shc.clear_cov_thresh();
   shc.set_alpha(bad_data_double);

   // Write the header columns
   write_header_cols(shc, stat_at, stat_row);

   // Write the data columns
   write_vl1l2_cols(vl1l2_info, stat_at, stat_row, n_header_columns);

   // If requested, copy row to the text file
   if(out_flag >= 2) {
      copy_ascii_table_row(stat_at, stat_row, txt_at, txt_row);

      // Increment the text row counter
      txt_row++;
   }

   // Increment the STAT row counter
   stat_row++;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_val1l2_row(StatHdrColumns &shc, const VL1L2Info &vl1l2_info,
                      int out_flag,
                      AsciiTable &stat_at, int &stat_row,
                      AsciiTable &txt_at, int &txt_row) {

   // VAL1L2 line type
   shc.set_line_type("VAL1L2");

   // Not Applicable
   shc.clear_fcst_thresh();
   shc.clear_obs_thresh();
   shc.clear_cov_thresh();
   shc.set_alpha(bad_data_double);

   // Write the header columns
   write_header_cols(shc, stat_at, stat_row);

   // Write the data columns
   write_val1l2_cols(vl1l2_info, stat_at, stat_row, n_header_columns);

   // If requested, copy row to the text file
   if(out_flag >= 2) {
      copy_ascii_table_row(stat_at, stat_row, txt_at, txt_row);

      // Increment the text row counter
      txt_row++;
   }

   // Increment the STAT row counter
   stat_row++;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_pct_row(StatHdrColumns &shc, const PCTInfo &pct_info,
                   int out_flag,
                   AsciiTable &stat_at, int &stat_row,
                   AsciiTable &txt_at, int &txt_row) {

   // PCT line type
   shc.set_line_type("PCT");

   // Thresholds
   shc.set_fcst_thresh(pct_info.pct_fcst_thresh);
   shc.set_obs_thresh(pct_info.pct_obs_thresh);

   // Not Applicable
   shc.set_alpha(bad_data_double);
   shc.clear_cov_thresh();

   // Write the header columns
   write_header_cols(shc, stat_at, stat_row);

   // Write the data columns
   write_pct_cols(pct_info, stat_at, stat_row, n_header_columns);

   // If requested, copy row to the text file
   if(out_flag >= 2) {
      copy_ascii_table_row(stat_at, stat_row, txt_at, txt_row);

      // Increment the text row counter
      txt_row++;
   }

   // Increment the STAT row counter
   stat_row++;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_pstd_row(StatHdrColumns &shc, const PCTInfo &pct_info,
                    int out_flag,
                    AsciiTable &stat_at, int &stat_row,
                    AsciiTable &txt_at, int &txt_row) {
   int i;

   // PSTD line type
   shc.set_line_type("PSTD");

   // Thresholds
   shc.set_fcst_thresh(pct_info.pct_fcst_thresh);
   shc.set_obs_thresh(pct_info.pct_obs_thresh);

   // Not Applicable
   shc.clear_cov_thresh();

   // Write a line for each alpha value
   for(i=0; i<pct_info.n_alpha; i++) {

      // Alpha value
      shc.set_alpha(pct_info.alpha[i]);

      // Write the header columns
      write_header_cols(shc, stat_at, stat_row);

      // Write the data columns
      write_pstd_cols(pct_info, i, stat_at, stat_row, n_header_columns);

      // If requested, copy row to the text file
      if(out_flag >= 2) {
         copy_ascii_table_row(stat_at, stat_row, txt_at, txt_row);

         // Increment the text row counter
         txt_row++;
      }

      // Increment the STAT row counter
      stat_row++;
   }

   return;
}


////////////////////////////////////////////////////////////////////////

void write_pjc_row(StatHdrColumns &shc, const PCTInfo &pct_info,
                   int out_flag,
                   AsciiTable &stat_at, int &stat_row,
                   AsciiTable &txt_at, int &txt_row) {

   // PJC line type
   shc.set_line_type("PJC");

   // Thresholds
   shc.set_fcst_thresh(pct_info.pct_fcst_thresh);
   shc.set_obs_thresh(pct_info.pct_obs_thresh);

   // Not Applicable
   shc.set_alpha(bad_data_double);
   shc.clear_cov_thresh();

   // Write the header columns
   write_header_cols(shc, stat_at, stat_row);

   // Write the data columns
   write_pjc_cols(pct_info, stat_at, stat_row, n_header_columns);

   // If requested, copy row to the text file
   if(out_flag >= 2) {
      copy_ascii_table_row(stat_at, stat_row, txt_at, txt_row);

      // Increment the text row counter
      txt_row++;
   }

   // Increment the STAT row counter
   stat_row++;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_prc_row(StatHdrColumns &shc, const PCTInfo &pct_info,
                   int out_flag,
                   AsciiTable &stat_at, int &stat_row,
                   AsciiTable &txt_at, int &txt_row) {

   // PRC line type
   shc.set_line_type("PRC");

   // Thresholds
   shc.set_fcst_thresh(pct_info.pct_fcst_thresh);
   shc.set_obs_thresh(pct_info.pct_obs_thresh);

   // Not Applicable
   shc.set_alpha(bad_data_double);
   shc.clear_cov_thresh();

   // Write the header columns
   write_header_cols(shc, stat_at, stat_row);

   // Write the data columns
   write_prc_cols(pct_info, stat_at, stat_row, n_header_columns);

   // If requested, copy row to the text file
   if(out_flag >= 2) {
      copy_ascii_table_row(stat_at, stat_row, txt_at, txt_row);

      // Increment the text row counter
      txt_row++;
   }

   // Increment the STAT row counter
   stat_row++;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_nbrctc_row(StatHdrColumns &shc, const NBRCTSInfo &nbrcts_info,
                   int out_flag,
                   AsciiTable &stat_at, int &stat_row,
                   AsciiTable &txt_at, int &txt_row) {

   // NBRCTC line type
   shc.set_line_type("NBRCTC");

   // Raw thresholds
   shc.set_fcst_thresh(nbrcts_info.raw_fcst_thresh);
   shc.set_obs_thresh(nbrcts_info.raw_obs_thresh);

   // Fractional coverage threshold
   shc.set_cov_thresh(nbrcts_info.frac_thresh);

   // Not Applicable
   shc.set_alpha(bad_data_double);

   // Write the header columns
   write_header_cols(shc, stat_at, stat_row);

   // Write the data columns
   write_nbrctc_cols(nbrcts_info, stat_at, stat_row, n_header_columns);

   // If requested, copy row to the text file
   if(out_flag >= 2) {
      copy_ascii_table_row(stat_at, stat_row, txt_at, txt_row);

      // Increment the text row counter
      txt_row++;
   }

   // Increment the STAT row counter
   stat_row++;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_nbrcts_row(StatHdrColumns &shc, const NBRCTSInfo &nbrcts_info,
                   int out_flag,
                   AsciiTable &stat_at, int &stat_row,
                   AsciiTable &txt_at, int &txt_row) {
   int i;

   // NBRCTS line type
   shc.set_line_type("NBRCTS");

   // Raw thresholds
   shc.set_fcst_thresh(nbrcts_info.raw_fcst_thresh);
   shc.set_obs_thresh(nbrcts_info.raw_obs_thresh);

   // Fractional coverage threshold
   shc.set_cov_thresh(nbrcts_info.frac_thresh);

   // Write a line for each alpha value
   for(i=0; i<nbrcts_info.cts_info.n_alpha; i++) {

      // Alpha value
      shc.set_alpha(nbrcts_info.cts_info.alpha[i]);

      // Write the header columns
      write_header_cols(shc, stat_at, stat_row);

      // Write the data columns
      write_nbrcts_cols(nbrcts_info, i, stat_at, stat_row,
                        n_header_columns);

      // If requested, copy row to the text file
      if(out_flag >= 2) {
         copy_ascii_table_row(stat_at, stat_row, txt_at, txt_row);

         // Increment the text row counter
         txt_row++;
      }

      // Increment the STAT row counter
      stat_row++;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_nbrcnt_row(StatHdrColumns &shc, const NBRCNTInfo &nbrcnt_info,
                      int out_flag,
                      AsciiTable &stat_at, int &stat_row,
                      AsciiTable &txt_at, int &txt_row) {
   int i;

   // NBRCNT line type
   shc.set_line_type("NBRCNT");

   // Raw thresholds
   shc.set_fcst_thresh(nbrcnt_info.raw_fcst_thresh);
   shc.set_obs_thresh(nbrcnt_info.raw_obs_thresh);

   // Not applicable
   shc.clear_cov_thresh();

   // Write a line for each alpha value
   for(i=0; i<nbrcnt_info.cnt_info.n_alpha; i++) {

      // Alpha value
      shc.set_alpha(nbrcnt_info.cnt_info.alpha[i]);

      // Write the header columns
      write_header_cols(shc, stat_at, stat_row);

      // Write the data columns
      write_nbrcnt_cols(nbrcnt_info, i, stat_at, stat_row,
                        n_header_columns);

      // If requested, copy row to the text file
      if(out_flag >= 2) {
         copy_ascii_table_row(stat_at, stat_row, txt_at, txt_row);

         // Increment the text row counter
         txt_row++;
      }

      // Increment the STAT row counter
      stat_row++;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_mpr_row(StatHdrColumns &shc, const PairData *pd_ptr,
                   int out_flag,
                   AsciiTable &stat_at, int &stat_row,
                   AsciiTable &txt_at, int &txt_row) {
   int i;

   // MPR line type
   shc.set_line_type("MPR");

   // Not Applicable
   shc.clear_fcst_thresh();
   shc.clear_obs_thresh();
   shc.clear_cov_thresh();
   shc.set_alpha(bad_data_double);

   // Write a line for each matched pair
   for(i=0; i<pd_ptr->n_pair; i++) {

      // Set the observation valid time
      shc.set_obs_valid_beg(pd_ptr->vld_ta[i]);
      shc.set_obs_valid_end(pd_ptr->vld_ta[i]);

      // Write the header columns
      write_header_cols(shc, stat_at, stat_row);

      // Write the data columns
      write_mpr_cols(pd_ptr, i, stat_at, stat_row, n_header_columns);

      // If requested, copy row to the text file
      if(out_flag >= 2) {
         copy_ascii_table_row(stat_at, stat_row, txt_at, txt_row);

         // Increment the text row counter
         txt_row++;
      }

      // Increment the STAT row counter
      stat_row++;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_isc_row(StatHdrColumns &shc, const ISCInfo &isc_info,
                   int out_flag,
                   AsciiTable &stat_at, int &stat_row,
                   AsciiTable &txt_at, int &txt_row) {
   int i;

   // ISC line type
   shc.set_line_type("ISC");

   // Not Applicable
   shc.set_interp_mthd(im_na);
   shc.set_interp_wdth(bad_data_int);
   shc.clear_cov_thresh();
   shc.set_alpha(bad_data_double);

   // Write a line for each scale plus one for the thresholded binary
   // field and one for the father wavelet
   for(i=-1; i<=isc_info.n_scale; i++) {

      // Write the header columns
      write_header_cols(shc, stat_at, stat_row);

      // Write the data columns
      write_isc_cols(isc_info, i, stat_at, stat_row, n_header_columns);

      // If requested, copy row to the text file
      if(out_flag >= 2) {
         copy_ascii_table_row(stat_at, stat_row, txt_at, txt_row);

         // Increment the text row counter
         txt_row++;
      }

      // Increment the STAT row counter
      stat_row++;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_rhist_row(StatHdrColumns &shc, const EnsPairData *pd_ptr,
                     int out_flag,
                     AsciiTable &stat_at, int &stat_row,
                     AsciiTable &txt_at, int &txt_row) {

   // RHIST line type
   shc.set_line_type("RHIST");

   // Not Applicable
   shc.clear_fcst_thresh();
   shc.clear_obs_thresh();
   shc.clear_cov_thresh();
   shc.set_alpha(bad_data_double);

   // Write the header columns
   write_header_cols(shc, stat_at, stat_row);

   // Write the data columns
   write_rhist_cols(pd_ptr, stat_at, stat_row, n_header_columns);

   // If requested, copy row to the text file
   if(out_flag >= 2) {
      copy_ascii_table_row(stat_at, stat_row, txt_at, txt_row);

      // Increment the text row counter
      txt_row++;
   }

   // Increment the STAT row counter
   stat_row++;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_orank_row(StatHdrColumns &shc, const EnsPairData *pd_ptr,
                     int out_flag,
                     AsciiTable &stat_at, int &stat_row,
                     AsciiTable &txt_at, int &txt_row) {
   int i;

   // Observation Rank line type
   shc.set_line_type("ORANK");

   // Not Applicable
   shc.clear_fcst_thresh();
   shc.clear_obs_thresh();
   shc.clear_cov_thresh();
   shc.set_alpha(bad_data_double);

   // Write a line for each ensemble pair
   for(i=0; i<pd_ptr->n_pair; i++) {

      // Set the observation valid time
      shc.set_obs_valid_beg(pd_ptr->vld_ta[i]);
      shc.set_obs_valid_end(pd_ptr->vld_ta[i]);

      // Write the header columns
      write_header_cols(shc, stat_at, stat_row);

      // Write the data columns
      write_orank_cols(pd_ptr, i, stat_at, stat_row, n_header_columns);

      // If requested, copy row to the text file
      if(out_flag >= 2) {
         copy_ascii_table_row(stat_at, stat_row, txt_at, txt_row);

         // Increment the text row counter
         txt_row++;
      }

      // Increment the STAT row counter
      stat_row++;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_header_cols(const StatHdrColumns &shc,
                       AsciiTable &at, int r) {

   //
   // Header columns:
   //    VERSION,        MODEL,
   //    FCST_LEAD,      FCST_VALID_BEG,
   //    FCST_VALID_END, OBS_LEAD,
   //    OBS_VALID_BEG,  OBS_VALID_END,
   //    FCST_VAR,       FCST_LEV,
   //    OBS_VAR,        OBS_LEV,
   //    OBTYPE,         VX_MASK,
   //    INTERP_MTHD,    INTERP_PNTS,
   //    FCST_THRESH,    OBS_THRESH,
   //    COV_THRESH,     ALPHA,
   //    LINE_TYPE
   //
   at.set_entry(r,  0, met_version);                  // MET version
   at.set_entry(r,  1, shc.get_model());              // Model name
   at.set_entry(r,  2, shc.get_fcst_lead_str());      // Fcst lead time
   at.set_entry(r,  3, shc.get_fcst_valid_beg_str()); // Fcst valid time
   at.set_entry(r,  4, shc.get_fcst_valid_end_str()); // Fcst valid time
   at.set_entry(r,  5, shc.get_obs_lead_str());       // Obs lead time
   at.set_entry(r,  6, shc.get_obs_valid_beg_str());  // Obs valid time
   at.set_entry(r,  7, shc.get_obs_valid_end_str());  // Obs valid time
   at.set_entry(r,  8, shc.get_fcst_var());           // Fcst variable
   at.set_entry(r,  9, shc.get_fcst_lev());           // Fcst level
   at.set_entry(r, 10, shc.get_obs_var());            // Obs variable
   at.set_entry(r, 11, shc.get_obs_lev());            // Obs level
   at.set_entry(r, 12, shc.get_msg_typ());            // Verifying observation type
   at.set_entry(r, 13, shc.get_mask());               // Verification masking region
   at.set_entry(r, 14, shc.get_interp_mthd_str());    // Interpolation method
   at.set_entry(r, 15, shc.get_interp_pnts_str());    // Interpolation points
   at.set_entry(r, 16, shc.get_fcst_thresh_str());    // Fcst threshold
   at.set_entry(r, 17, shc.get_obs_thresh_str());     // Obs threshold
   at.set_entry(r, 18, shc.get_cov_thresh_str());     // Coverage threshold
   at.set_entry(r, 19, shc.get_alpha());              // Alpha value
   at.set_entry(r, 20, shc.get_line_type());          // Line type

   return;
}

////////////////////////////////////////////////////////////////////////

void write_fho_cols(const CTSInfo &cts_info,
                    AsciiTable &at, int r, int c) {

   //
   // FHO
   // Dump out the FHO line:
   //    TOTAL,       F_RATE,      H_RATE,
   //    O_RATE
   //
   at.set_entry(r, c+0,  // Total Count
      cts_info.cts.n());

   at.set_entry(r, c+1,  // Forecast Rate = FY/N
      cts_info.cts.f_rate());

   at.set_entry(r, c+2,  // Hit Rate = FY_OY/N
      cts_info.cts.h_rate());

   at.set_entry(r, c+3,  // Observation = OY/N
      cts_info.cts.o_rate());

   return;
}

////////////////////////////////////////////////////////////////////////

void write_ctc_cols(const CTSInfo &cts_info,
                    AsciiTable &at, int r, int c) {

   //
   // Contingency Table Counts
   // Dump out the CTC line:
   //    TOTAL,       FY_OY,       FY_ON,
   //    FN_OY,       FN_ON
   //
   at.set_entry(r, c+0,  // Total Count
      cts_info.cts.n());

   at.set_entry(r, c+1,  // FY_OY
      cts_info.cts.fy_oy());

   at.set_entry(r, c+2,  // FY_ON
      cts_info.cts.fy_on());

   at.set_entry(r, c+3,  // FN_OY
      cts_info.cts.fn_oy());

   at.set_entry(r, c+4,  // FN_ON
      cts_info.cts.fn_on());

   return;
}

////////////////////////////////////////////////////////////////////////

void write_cts_cols(const CTSInfo &cts_info, int i,
                    AsciiTable &at, int r, int c) {

   //
   // Contingency Table Stats
   // Dump out the CTS line:
   //    TOTAL,
   //    BASER,       BASER_NCL,   BASER_NCU,   BASER_BCL,   BASER_BCU,
   //    FMEAN,       FMEAN_NCL,   FMEAN_NCU,   FMEAN_BCL,   FMEAN_BCU,
   //    ACC,         ACC_NCL,     ACC_NCU,     ACC_BCL,     ACC_BCU,
   //    FBIAS,       FBIAS_BCL,   FBIAS_BCU,
   //    PODY,        PODY_NCL,    PODY_NCU,    PODY_BCL,    PODY_BCU,
   //    PODN,        PODN_NCL,    PODN_NCU,    PODN_BCL,    PODN_BCU,
   //    POFD,        POFD_NCL,    POFD_NCU,    POFD_BCL,    POFD_BCU,
   //    FAR,         FAR_NCL,     FAR_NCU,     FAR_BCL,     FAR_BCU,
   //    CSI,         CSI_NCL,     CSI_NCU,     CSI_BCL,     CSI_BCU,
   //    GSS,         GSS_BCL,     GSS_BCU,
   //    HK,          HK_NCL,      HK_NCU,      HK_BCL,      HK_BCU,
   //    HSS,         HSS_BCL,     HSS_BCU,
   //    ODDS,        ODDS_NCL,    ODDS_NCU,    ODDS_BCL,    ODDS_BCU
   //
   at.set_entry(r, c+0,  // Total count
      cts_info.cts.n());

   at.set_entry(r, c+1,  // Base Rate (oy_tp)
      cts_info.baser.v);

   at.set_entry(r, c+2,  // Base Rate (oy_tp) NCL
      cts_info.baser.v_ncl[i]);

   at.set_entry(r, c+3,  // Base Rate (oy_tp) NCU
      cts_info.baser.v_ncu[i]);

   at.set_entry(r, c+4,  // Base Rate (oy_tp) BCL
      cts_info.baser.v_bcl[i]);

   at.set_entry(r, c+5,  // Base Rate (oy_tp) BCU
      cts_info.baser.v_bcu[i]);

   at.set_entry(r, c+6,  // Forecast Mean (fy_tp)
      cts_info.fmean.v);

   at.set_entry(r, c+7,  // Forecast Mean (fy_tp) NCL
      cts_info.fmean.v_ncl[i]);

   at.set_entry(r, c+8,  // Forecast Mean (fy_tp) NCU
      cts_info.fmean.v_ncu[i]);

   at.set_entry(r, c+9,  // Forecast Mean (fy_tp) BCL
      cts_info.fmean.v_bcl[i]);

   at.set_entry(r, c+10, // Forecast Mean (fy_tp) BCU
      cts_info.fmean.v_bcu[i]);

   at.set_entry(r, c+11, // Accuracy
      cts_info.acc.v);

   at.set_entry(r, c+12, // Accuracy NCL
      cts_info.acc.v_ncl[i]);

   at.set_entry(r, c+13, // Accuracy NCU
      cts_info.acc.v_ncu[i]);

   at.set_entry(r, c+14, // Accuracy BCL
      cts_info.acc.v_bcl[i]);

   at.set_entry(r, c+15, // Accuracy BCU
      cts_info.acc.v_bcu[i]);

   at.set_entry(r, c+16, // Frequency Bias
      cts_info.fbias.v);

   at.set_entry(r, c+17, // Frequency Bias BCL
      cts_info.fbias.v_bcl[i]);

   at.set_entry(r, c+18, // Frequency Bias BCU
      cts_info.fbias.v_bcu[i]);

   at.set_entry(r, c+19, // POD Yes
      cts_info.pody.v);

   at.set_entry(r, c+20, // POD Yes NCL
      cts_info.pody.v_ncl[i]);

   at.set_entry(r, c+21, // POD Yes NCU
      cts_info.pody.v_ncu[i]);

   at.set_entry(r, c+22, // POD Yes BCL
      cts_info.pody.v_bcl[i]);

   at.set_entry(r, c+23, // POD Yes BCU
      cts_info.pody.v_bcu[i]);

   at.set_entry(r, c+24, // POD No
      cts_info.podn.v);

   at.set_entry(r, c+25, // POD No NCL
      cts_info.podn.v_ncl[i]);

   at.set_entry(r, c+26, // POD No NCU
      cts_info.podn.v_ncu[i]);

   at.set_entry(r, c+27, // POD No BCL
      cts_info.podn.v_bcl[i]);

   at.set_entry(r, c+28, // POD No BCU
      cts_info.podn.v_bcu[i]);

   at.set_entry(r, c+29, // POFD
      cts_info.pofd.v);

   at.set_entry(r, c+30, // POFD NCL
      cts_info.pofd.v_ncl[i]);

   at.set_entry(r, c+31, // POFD NCU
      cts_info.pofd.v_ncu[i]);

   at.set_entry(r, c+32, // POFD BCL
      cts_info.pofd.v_bcl[i]);

   at.set_entry(r, c+33, // POFD BCU
      cts_info.pofd.v_bcu[i]);

   at.set_entry(r, c+34, // FAR
      cts_info.far.v);

   at.set_entry(r, c+35, // FAR NCL
      cts_info.far.v_ncl[i]);

   at.set_entry(r, c+36, // FAR NCU
      cts_info.far.v_ncu[i]);

   at.set_entry(r, c+37, // FAR BCL
      cts_info.far.v_bcl[i]);

   at.set_entry(r, c+38, // FAR BCU
      cts_info.far.v_bcu[i]);

   at.set_entry(r, c+39, // CSI (Threat Score)
      cts_info.csi.v);

   at.set_entry(r, c+40, // CSI (Threat Score) NCL
      cts_info.csi.v_ncl[i]);

   at.set_entry(r, c+41, // CSI (Threat Score) NCU
      cts_info.csi.v_ncu[i]);

   at.set_entry(r, c+42, // CSI (Threat Score) BCL
      cts_info.csi.v_bcl[i]);

   at.set_entry(r, c+43, // CSI (Threat Score) BCU
      cts_info.csi.v_bcu[i]);

   at.set_entry(r, c+44, // Gilbert Skill Score (ETS)
      cts_info.gss.v);

   at.set_entry(r, c+45, // Gilbert Skill Score (ETS) BCL
      cts_info.gss.v_bcl[i]);

   at.set_entry(r, c+46, // Gilbert Skill Score (ETS) BCU
      cts_info.gss.v_bcu[i]);

   at.set_entry(r, c+47, // Hanssen-Kuipers Discriminant (TSS)
      cts_info.hk.v);

   at.set_entry(r, c+48, // Hanssen-Kuipers Discriminant (TSS) NCL
      cts_info.hk.v_ncl[i]);

   at.set_entry(r, c+49, // Hanssen-Kuipers Discriminant (TSS) NCU
      cts_info.hk.v_ncu[i]);

   at.set_entry(r, c+50, // Hanssen-Kuipers Discriminant (TSS) BCL
      cts_info.hk.v_bcl[i]);

   at.set_entry(r, c+51, // Hanssen-Kuipers Discriminant (TSS) BCU
      cts_info.hk.v_bcu[i]);

   at.set_entry(r, c+52, // Heidke Skill Score
      cts_info.hss.v);

   at.set_entry(r, c+53, // Heidke Skill Score BCL
      cts_info.hss.v_bcl[i]);

   at.set_entry(r, c+54, // Heidke Skill Score BCU
      cts_info.hss.v_bcu[i]);

   at.set_entry(r, c+55, // Odds Ratio
      cts_info.odds.v);

   at.set_entry(r, c+56, // Odds Ratio NCL
      cts_info.odds.v_ncl[i]);

   at.set_entry(r, c+57, // Odds Ratio NCU
      cts_info.odds.v_ncu[i]);

   at.set_entry(r, c+58, // Odds Ratio BCL
      cts_info.odds.v_bcl[i]);

   at.set_entry(r, c+59, // Odds Ratio BCU
      cts_info.odds.v_bcu[i]);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_cnt_cols(const CNTInfo &cnt_info, int i,
                    AsciiTable &at, int r, int c) {

   //
   // Continuous Variable Stats
   // Dump out the CNT line:
   //    TOTAL,
   //    FBAR,        FBAR_NCL,    FBAR_NCU,    FBAR_BCL,    FBAR_BCU,
   //    FSTDEV,      FSTDEV_NCL,  FSTDEV_NCU,  FSTDEV_BCL,  FSTDEV_BCU,
   //    OBAR,        OBAR_NCL,    OBAR_NCU,    OBAR_BCL,    OBAR_BCU,
   //    OSTDEV,      OSTDEV_NCL,  OSTDEV_NCU,  OSTDEV_BCL,  OSTDEV_BCU,
   //    PR_CORR,     PR_CORR_NCL, PR_CORR_NCU, PR_CORR_BCL, PR_CORR_BCU,
   //    SP_CORR,
   //    KT_CORR,
   //    RANKS,
   //    FRANK_TIES,
   //    ORANK_TIES,
   //    ME,          ME_NCL,      ME_NCU,      ME_BCL,      ME_BCU,
   //    ESTDEV,      ESTDEV_NCL,  ESTDEV_NCU,  ESTDEV_BCL,  ESTDEV_BCU,
   //    MBIAS,       MBIAS_BCL,   MBIAS_BCU,
   //    MAE,         MAE_BCL,     MAE_BCU,
   //    MSE,         MSE_BCL,     MSE_BCU,
   //    BCMSE,       BCMSE_BCL,   BCMSE_BCU,
   //    RMSE,        RMSE_BCL,    RMSE_BCU,
   //    E10,         E10_BCL,     E10_BCU,
   //    E25,         E25_BCL,     E25_BCU,
   //    E50,         E50_BCL,     E50_BCU,
   //    E75,         E75_BCL,     E75_BCU,
   //    E90,         E90_BCL,     E90_BCU
   //
   at.set_entry(r, c+0,  // Total Number of Grid Points
      cnt_info.n);

   at.set_entry(r, c+1,  // Forecast Mean
      cnt_info.fbar.v);

   at.set_entry(r, c+2,  // Forecast Mean NCL
      cnt_info.fbar.v_ncl[i]);

   at.set_entry(r, c+3,  // Forecast Mean NCU
      cnt_info.fbar.v_ncu[i]);

   at.set_entry(r, c+4,  // Forecast Mean BCL
      cnt_info.fbar.v_bcl[i]);

   at.set_entry(r, c+5,  // Forecast Mean BCU
      cnt_info.fbar.v_bcu[i]);

   at.set_entry(r, c+6,  // Forecast Standard Deviation
      cnt_info.fstdev.v);

   at.set_entry(r, c+7,  // Forecast Standard Deviation NCL
      cnt_info.fstdev.v_ncl[i]);

   at.set_entry(r, c+8,  // Forecast Standard Deviation NCU
      cnt_info.fstdev.v_ncu[i]);

   at.set_entry(r, c+9,  // Forecast Standard Deviation BCL
      cnt_info.fstdev.v_bcl[i]);

   at.set_entry(r, c+10, // Forecast Standard Deviation BCU
      cnt_info.fstdev.v_bcu[i]);

   at.set_entry(r, c+11, // Observation Mean
      cnt_info.obar.v);

   at.set_entry(r, c+12, // Observation Mean NCL
      cnt_info.obar.v_ncl[i]);

   at.set_entry(r, c+13, // Observation Mean NCU
      cnt_info.obar.v_ncu[i]);

   at.set_entry(r, c+14, // Observation Mean BCL
      cnt_info.obar.v_bcl[i]);

   at.set_entry(r, c+15, // Observation Mean BCU
      cnt_info.obar.v_bcu[i]);

   at.set_entry(r, c+16, // Obsevation Standard Deviation
      cnt_info.ostdev.v);

   at.set_entry(r, c+17, // Obsevation Standard Deviation NCL
      cnt_info.ostdev.v_ncl[i]);

   at.set_entry(r, c+18, // Obsevation Standard Deviation NCU
      cnt_info.ostdev.v_ncu[i]);

   at.set_entry(r, c+19, // Obsevation Standard Deviation BCL
      cnt_info.ostdev.v_bcl[i]);

   at.set_entry(r, c+20, // Obsevation Standard Deviation BCU
      cnt_info.ostdev.v_bcu[i]);

   at.set_entry(r, c+21, // Pearson's Correlation Coefficient
      cnt_info.pr_corr.v);

   at.set_entry(r, c+22, // Pearson's Correlation Coefficient NCL
      cnt_info.pr_corr.v_ncl[i]);

   at.set_entry(r, c+23, // Pearson's Correlation Coefficient NCU
      cnt_info.pr_corr.v_ncu[i]);

   at.set_entry(r, c+24, // Pearson's Correlation Coefficient BCL
      cnt_info.pr_corr.v_bcl[i]);

   at.set_entry(r, c+25, // Pearson's Correlation Coefficient BCU
      cnt_info.pr_corr.v_bcu[i]);

   at.set_entry(r, c+26, // Spearman's Rank Correlation Coefficient
      cnt_info.sp_corr.v);

   at.set_entry(r, c+27, // Kendall Tau Rank Correlation Coefficient
      cnt_info.kt_corr.v);

   at.set_entry(r, c+28, // Number of ranks compared
      cnt_info.n_ranks);

   at.set_entry(r, c+29, // Number of tied forecast ranks
      cnt_info.frank_ties);

   at.set_entry(r, c+30, // Number of tied observation ranks
      cnt_info.orank_ties);

   at.set_entry(r, c+31, // Mean Error
      cnt_info.me.v);

   at.set_entry(r, c+32, // Mean Error NCL
      cnt_info.me.v_ncl[i]);

   at.set_entry(r, c+33, // Mean Error NCU
      cnt_info.me.v_ncu[i]);

   at.set_entry(r, c+34, // Mean Error BCL
      cnt_info.me.v_bcl[i]);

   at.set_entry(r, c+35, // Mean Error BCU
      cnt_info.me.v_bcu[i]);

   at.set_entry(r, c+36, // Error Standard Deviation
      cnt_info.estdev.v);

   at.set_entry(r, c+37, // Error Standard Deviation NCL
      cnt_info.estdev.v_ncl[i]);

   at.set_entry(r, c+38, // Error Standard Deviation NCU
      cnt_info.estdev.v_ncu[i]);

   at.set_entry(r, c+39, // Error Standard Deviation BCL
      cnt_info.estdev.v_bcl[i]);

   at.set_entry(r, c+40, // Error Standard Deviation BCU
      cnt_info.estdev.v_bcu[i]);

   at.set_entry(r, c+41, // Multiplicative Bias
      cnt_info.mbias.v);

   at.set_entry(r, c+42, // Multiplicative Bias BCL
      cnt_info.mbias.v_bcl[i]);

   at.set_entry(r, c+43, // Multiplicative Bias BCU
      cnt_info.mbias.v_bcu[i]);

   at.set_entry(r, c+44, // Mean Absolute Error
      cnt_info.mae.v);

   at.set_entry(r, c+45, // Mean Absolute Error BCL
      cnt_info.mae.v_bcl[i]);

   at.set_entry(r, c+46, // Mean Absolute Error BCU
      cnt_info.mae.v_bcu[i]);

   at.set_entry(r, c+47, // Mean Squared Error
      cnt_info.mse.v);

   at.set_entry(r, c+48, // Mean Squared Error BCL
      cnt_info.mse.v_bcl[i]);

   at.set_entry(r, c+49, // Mean Squared Error BCU
      cnt_info.mse.v_bcu[i]);

   at.set_entry(r, c+50, // Bias-Corrected Mean Squared Error
      cnt_info.bcmse.v);

   at.set_entry(r, c+51, // Bias-Corrected Mean Squared Error BCL
      cnt_info.bcmse.v_bcl[i]);

   at.set_entry(r, c+52, // Bias-Corrected Mean Squared Error BCU
      cnt_info.bcmse.v_bcu[i]);

   at.set_entry(r, c+53, // Root Mean Squared Error
      cnt_info.rmse.v);

   at.set_entry(r, c+54, // Root Mean Squared Error BCL
      cnt_info.rmse.v_bcl[i]);

   at.set_entry(r, c+55, // Root Mean Squared Error BCU
      cnt_info.rmse.v_bcu[i]);

   at.set_entry(r, c+56, // 10th Percentile of the Error
      cnt_info.e10.v);

   at.set_entry(r, c+57, // 10th Percentile of the Error BCL
      cnt_info.e10.v_bcl[i]);

   at.set_entry(r, c+58, // 10th Percentile of the Error BCU
      cnt_info.e10.v_bcu[i]);

   at.set_entry(r, c+59, // 25th Percentile of the Error
      cnt_info.e25.v);

   at.set_entry(r, c+60, // 25th Percentile of the Error BCL
      cnt_info.e25.v_bcl[i]);

   at.set_entry(r, c+61, // 25th Percentile of the Error BCU
      cnt_info.e25.v_bcu[i]);

   at.set_entry(r, c+62, // 50th Percentile of the Error
      cnt_info.e50.v);

   at.set_entry(r, c+63, // 50th Percentile of the Error BCL
      cnt_info.e50.v_bcl[i]);

   at.set_entry(r, c+64, // 50th Percentile of the Error BCU
      cnt_info.e50.v_bcu[i]);

   at.set_entry(r, c+65, // 75th Percentile of the Error
      cnt_info.e75.v);

   at.set_entry(r, c+66, // 75th Percentile of the Error BCL
      cnt_info.e75.v_bcl[i]);

   at.set_entry(r, c+67, // 75th Percentile of the Error BCU
      cnt_info.e75.v_bcu[i]);

   at.set_entry(r, c+68, // 90th Percentile of the Error
      cnt_info.e90.v);

   at.set_entry(r, c+69, // 90th Percentile of the Error BCL
      cnt_info.e90.v_bcl[i]);

   at.set_entry(r, c+70, // 90th Percentile of the Error BCU
      cnt_info.e90.v_bcu[i]);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_sl1l2_cols(const CNTInfo &cnt_info,
                      AsciiTable &at, int r, int c) {

   //
   // Scalar L1L2 Line Type (SL1L2)
   // Dump out the SL1L2 line:
   //    TOTAL,       FBAR,        OBAR,
   //    FOBAR,       FFBAR,       OOBAR
   //
   at.set_entry(r, c+0,  // Total Count
      cnt_info.n);

   at.set_entry(r, c+1,  // FBAR
      cnt_info.fbar.v);

   at.set_entry(r, c+2,  // OBAR
      cnt_info.obar.v);

   at.set_entry(r, c+3,  // FOBAR
      cnt_info.fobar);

   at.set_entry(r, c+4,  // FFBAR
      cnt_info.ffbar);

   at.set_entry(r, c+5,  // OOBAR
      cnt_info.oobar);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_mctc_cols(const MCTSInfo &mcts_info,
                     AsciiTable &at, int r, int c) {
   int i, j, col;

   //
   // Multi-Category Contingency Table Counts
   // Dump out the MCTC line:
   //    TOTAL,       N_CAT,     Fi_Oj
   //
   at.set_entry(r, c+0,  // Total Count
      mcts_info.cts.total());

   at.set_entry(r, c+1,  // N_CAT
      mcts_info.cts.nrows());

   //
   // Loop through the contingency table rows and columns
   //
   for(i=0, col=c+2; i<mcts_info.cts.nrows(); i++) {
      for(j=0; j<mcts_info.cts.ncols(); j++) {

         at.set_entry(r, col,      // Fi_Oj
            mcts_info.cts.entry(i, j));
         col++;
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_mcts_cols(const MCTSInfo &mcts_info, int i,
                     AsciiTable &at, int r, int c) {

   //
   // Multi-Category Contingency Table Stats
   // Dump out the MCTS line:
   //    TOTAL,       N_CAT,
   //    ACC,         ACC_NCL,     ACC_NCU,     ACC_BCL,     ACC_BCU,
   //    HK,          HK_BCL,      HK_BCU,
   //    HSS,         HSS_BCL,     HSS_BCU,
   //    GER,         GER_BCL,     GER_BCU
   //
   at.set_entry(r, c+0,  // Total count
      mcts_info.cts.total());

   at.set_entry(r, c+1,  // Number of categories
      mcts_info.cts.nrows());

   at.set_entry(r, c+2, // Accuracy
      mcts_info.acc.v);

   at.set_entry(r, c+3, // Accuracy NCL
      mcts_info.acc.v_ncl[i]);

   at.set_entry(r, c+4, // Accuracy NCU
      mcts_info.acc.v_ncu[i]);

   at.set_entry(r, c+5, // Accuracy BCL
      mcts_info.acc.v_bcl[i]);

   at.set_entry(r, c+6, // Accuracy BCU
      mcts_info.acc.v_bcu[i]);

   at.set_entry(r, c+7, // Hanssen-Kuipers Discriminant (TSS)
      mcts_info.hk.v);

   at.set_entry(r, c+8, // Hanssen-Kuipers Discriminant (TSS) BCL
      mcts_info.hk.v_bcl[i]);

   at.set_entry(r, c+9, // Hanssen-Kuipers Discriminant (TSS) BCU
      mcts_info.hk.v_bcu[i]);

   at.set_entry(r, c+10, // Heidke Skill Score
      mcts_info.hss.v);

   at.set_entry(r, c+11, // Heidke Skill Score BCL
      mcts_info.hss.v_bcl[i]);

   at.set_entry(r, c+12, // Heidke Skill Score BCU
      mcts_info.hss.v_bcu[i]);

   at.set_entry(r, c+13, // Gerrity Score
      mcts_info.ger.v);

   at.set_entry(r, c+14, // Gerrity Score BCL
      mcts_info.ger.v_bcl[i]);

   at.set_entry(r, c+15, // Gerrity Score BCU
      mcts_info.ger.v_bcu[i]);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_sl1l2_cols(const SL1L2Info &sl1l2_info,
                      AsciiTable &at, int r, int c) {

   //
   // Scalar L1L2 Line Type (SL1L2)
   // Dump out the SL1L2 line:
   //    TOTAL,       FBAR,        OBAR,
   //    FOBAR,       FFBAR,       OOBAR
   //
   at.set_entry(r, c+0,  // Total Count
      sl1l2_info.scount);

   at.set_entry(r, c+1,  // FBAR
      sl1l2_info.fbar);

   at.set_entry(r, c+2,  // OBAR
      sl1l2_info.obar);

   at.set_entry(r, c+3,  // FOBAR
      sl1l2_info.fobar);

   at.set_entry(r, c+4,  // FFBAR
      sl1l2_info.ffbar);

   at.set_entry(r, c+5,  // OOBAR
      sl1l2_info.oobar);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_sal1l2_cols(const SL1L2Info &sl1l2_info,
                       AsciiTable &at, int r, int c) {

   //
   // Scalar Anomaly L1L2 Line Type (SAL1L2)
   // Dump out the SAL1L2 line:
   //    TOTAL,       FABAR,       OABAR,
   //    FOABAR,      FFABAR,      OOABAR
   //
   at.set_entry(r, c+0,  // Total Anomaly Count
      sl1l2_info.sacount);

   at.set_entry(r, c+1,  // FABAR
      sl1l2_info.fabar);

   at.set_entry(r, c+2,  // OABAR
      sl1l2_info.oabar);

   at.set_entry(r, c+3,  // FOABAR
      sl1l2_info.foabar);

   at.set_entry(r, c+4,  // FFABAR
      sl1l2_info.ffabar);

   at.set_entry(r, c+5,  // OOABAR
      sl1l2_info.ooabar);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_vl1l2_cols(const VL1L2Info &vl1l2_info,
                      AsciiTable &at, int r, int c) {

   //
   // Vector L1L2 Line Type (VL1L2)
   // Dump out the VL1L2 line:
   //    TOTAL,       UFBAR,       VFBAR,
   //    UOBAR,       VOBAR,       UVFOBAR,
   //    UVFFBAR,     UVOOBAR
   //
   at.set_entry(r, c+0,  // Total Count
      vl1l2_info.vcount);

   at.set_entry(r, c+1,  // UFBAR
      vl1l2_info.ufbar);

   at.set_entry(r, c+2,  // VFBAR
      vl1l2_info.vfbar);

   at.set_entry(r, c+3,  // UOBAR
      vl1l2_info.uobar);

   at.set_entry(r, c+4,  // VOBAR
      vl1l2_info.vobar);

   at.set_entry(r, c+5,  // UVFOBAR
      vl1l2_info.uvfobar);

   at.set_entry(r, c+6,  // UVFFBAR
      vl1l2_info.uvffbar);

   at.set_entry(r, c+7,  // UVOOBAR
      vl1l2_info.uvoobar);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_val1l2_cols(const VL1L2Info &vl1l2_info,
                       AsciiTable &at, int r, int c) {

   //
   // Vector Anomaly L1L2 Line Type (VAL1L2)
   // Dump out the VAL1L2 line:
   //    TOTAL,       UFABAR,      VFABAR,
   //    UOABAR,      VOABAR,      UVFOABAR,
   //    UVFFABAR,    UVOOABAR
   //
   at.set_entry(r, c+0,  // Total Anomaly Count
      vl1l2_info.vacount);

   at.set_entry(r, c+1,  // UFABAR
      vl1l2_info.ufabar);

   at.set_entry(r, c+2,  // VFABAR
      vl1l2_info.vfabar);

   at.set_entry(r, c+3,  // UOABAR
      vl1l2_info.uoabar);

   at.set_entry(r, c+4,  // VOABAR
      vl1l2_info.voabar);

   at.set_entry(r, c+5,  // UVFOABAR
      vl1l2_info.uvfoabar);

   at.set_entry(r, c+6,  // UVFFABAR
      vl1l2_info.uvffabar);

   at.set_entry(r, c+7,  // UVOOABAR
      vl1l2_info.uvooabar);
   return;
}

////////////////////////////////////////////////////////////////////////

void write_pct_cols(const PCTInfo &pct_info,
                    AsciiTable &at, int r, int c) {
   int i, col;

   //
   // Nx2 Contingency Table Counts for Probability Forecast
   // Dump out the PCT line:
   //    TOTAL,       N_THRESH,
   //   [THRESH,      OY,          ON,] (for each row of Nx2 Table)
   //    THRESH                         (last threshold)
   //
   at.set_entry(r, c+0,    // Total Count
      pct_info.pct.n());

   at.set_entry(r, c+1,    // N_THRESH
      pct_info.pct.nrows() + 1);

   //
   // Write THRESH_i, OY_i, ON_i for each row of the Nx2 table
   //
   for(i=0, col=c+2; i<pct_info.pct.nrows(); i++) {

      at.set_entry(r, col, // THRESH
         pct_info.pct.threshold(i));
      col++;

      at.set_entry(r, col, // Event Count (OY)
         pct_info.pct.event_count_by_row(i));
      col++;

      at.set_entry(r, col, // Non-Event Count (ON)
         pct_info.pct.nonevent_count_by_row(i));
      col++;
   }

   //
   // Write out the last threshold
   //
   at.set_entry(r, col,    // THRESH
      pct_info.pct.threshold(pct_info.pct.nrows()));

   return;
}

////////////////////////////////////////////////////////////////////////

void write_pstd_cols(const PCTInfo &pct_info, int alpha_i,
                     AsciiTable &at, int r, int c) {
   int i, col;

   //
   // Nx2 Contingency Table Statistics for Probability Forecast
   // Dump out the PSTD line:
   //    TOTAL,       N_THRESH,    BASER,
   //    BASER_NCL,   BASER_NCU,   RELIABILTY,
   //    RESOLUTION,  UNCERTAINTY, ROC_AUC,
   //    BRIER,       BRIER_NCL,   BRIER_NCU,
   //    [THRESH] (for each threshold)
   //
   at.set_entry(r, c+0,  // Total count
      pct_info.pct.n());

   at.set_entry(r, c+1,  // N_THRESH
      pct_info.pct.nrows() + 1);

   at.set_entry(r, c+2,  // BASER
      pct_info.baser.v);

   at.set_entry(r, c+3,  // BASER_NCL
      pct_info.baser.v_ncl[alpha_i]);

   at.set_entry(r, c+4,  // BASER_NCU
      pct_info.baser.v_ncu[alpha_i]);

   at.set_entry(r, c+5,  // RELIABILITY
      pct_info.pct.reliability());

   at.set_entry(r, c+6,  // RESOLUTION
      pct_info.pct.resolution());

   at.set_entry(r, c+7,  // UNCERTAINTY
      pct_info.pct.uncertainty());

   at.set_entry(r, c+8,  // ROC_AUC
      pct_info.pct.roc_auc());

   at.set_entry(r, c+9,  // BRIER
      pct_info.brier.v);

   at.set_entry(r, c+10, // BRIER_NCL
      pct_info.brier.v_ncl[alpha_i]);

   at.set_entry(r, c+11, // BRIER_NCU
      pct_info.brier.v_ncu[alpha_i]);

   //
   // Write THRESH_i for each probability threshold
   //
   for(i=0, col=c+12; i<=pct_info.pct.nrows(); i++) {

      at.set_entry(r, col, // THRESH
         pct_info.pct.threshold(i));
      col++;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_pjc_cols(const PCTInfo &pct_info,
                    AsciiTable &at, int r, int c) {
   int i, col, n;

   //
   // Nx2 Contingency Table Joint/Continuous Probability
   // Dump out the PJC line:
   //    TOTAL,       N_THRESH,
   //   [THRESH,
   //    OY_TP,       ON_TP,      CALIBRATION,
   //    REFINEMENT,  LIKELIHOOD, BASER] (for each row of Nx2 Table)
   //    THRESH                          (last threshold)
   //
   at.set_entry(r, c+0,    // Total Count
      pct_info.pct.n());

   at.set_entry(r, c+1,    // N_THRESH
      pct_info.pct.nrows() + 1);

   //
   // Write THRESH, OY, ON for each row of the Nx2 table
   //
   n = pct_info.pct.n();
   for(i=0, col=c+2; i<pct_info.pct.nrows(); i++) {

      at.set_entry(r, col, // THRESH
         pct_info.pct.threshold(i));
      col++;

      at.set_entry(r, col, // OY_TP
         pct_info.pct.event_count_by_row(i)/(double) n);
      col++;

      at.set_entry(r, col, // ON_TP
         pct_info.pct.nonevent_count_by_row(i)/(double) n);
      col++;

      at.set_entry(r, col, // CALIBRATION
         pct_info.pct.row_calibration(i));
      col++;

      at.set_entry(r, col, // REFINEMENT
         pct_info.pct.row_refinement(i));
      col++;

      at.set_entry(r, col, // LIKELIHOOD
         pct_info.pct.row_event_likelihood(i));
      col++;

      at.set_entry(r, col, // BASER
         pct_info.pct.row_nonevent_likelihood(i));
      col++;
   }

   //
   // Write out the last threshold
   //
   at.set_entry(r, col,    // THRESH
      pct_info.pct.threshold(pct_info.pct.nrows()));

   return;
}

////////////////////////////////////////////////////////////////////////

void write_prc_cols(const PCTInfo &pct_info,
                    AsciiTable &at, int r, int c) {
   int i, col;
   TTContingencyTable ct;

   //
   // Nx2 Contingency Table Points for the ROC Curve
   // Dump out the PRC line:
   //    TOTAL,       N_THRESH,
   //   [THRESH,      PODY,        POFD,] (for each row of Nx2 Table)
   //    THRESH                           (last threshold)
   //
   at.set_entry(r, c+0,    // Total Count
      pct_info.pct.n());

   at.set_entry(r, c+1,    // N_THRESH
      pct_info.pct.nrows() + 1);

   //
   // Write THRESH, OY, ON for each row of the Nx2 table
   //
   for(i=0, col=c+2; i<pct_info.pct.nrows(); i++) {

      //
      // Get the 2x2 contingency table for this row
      //
      ct = pct_info.pct.roc_point_by_row(i);

      at.set_entry(r, col, // THRESH
         pct_info.pct.threshold(i));
      col++;

      at.set_entry(r, col, // PODY
         ct.pod_yes());
      col++;

      at.set_entry(r, col, // POFD
         ct.pofd());
      col++;
   }

   //
   // Write out the last threshold
   //
   at.set_entry(r, col,    // THRESH
      pct_info.pct.threshold(pct_info.pct.nrows()));

   return;
}

////////////////////////////////////////////////////////////////////////

void write_nbrctc_cols(const NBRCTSInfo &nbrcts_info,
                       AsciiTable &at, int r, int c) {

   //
   // Neighborhood Method Contingency Table Counts Line Type (NBRCTC)
   // Dump out the NBRCTC line:
   //    TOTAL,       FY_OY,       FY_ON,
   //    FN_OY,       FN_ON
   //
   at.set_entry(r, c+0,  // Total Count
      nbrcts_info.cts_info.cts.n());

   at.set_entry(r, c+1,  // FY_OY
      nbrcts_info.cts_info.cts.fy_oy());

   at.set_entry(r, c+2,  // FY_ON
      nbrcts_info.cts_info.cts.fy_on());

   at.set_entry(r, c+3,  // FN_OY
      nbrcts_info.cts_info.cts.fn_oy());

   at.set_entry(r, c+4,  // FN_ON
      nbrcts_info.cts_info.cts.fn_on());

   return;
}

////////////////////////////////////////////////////////////////////////

void write_nbrcts_cols(const NBRCTSInfo &nbrcts_info, int i,
                       AsciiTable &at, int r, int c) {

   //
   // Neighborhood Method Contingency Table Statistics Line Type (NBRCTS)
   // Dump out the NBRCTS line:
   //    TOTAL,
   //    BASER,       BASER_NCL,   BASER_NCU,   BASER_BCL,   BASER_BCU,
   //    FMEAN,       FMEAN_NCL,   FMEAN_NCU,   FMEAN_BCL,   FMEAN_BCU,
   //    ACC,         ACC_NCL,     ACC_NCU,     ACC_BCL,     ACC_BCU,
   //    FBIAS,       FBIAS_BCL,   FBIAS_BCU,
   //    PODY,        PODY_NCL,    PODY_NCU,    PODY_BCL,    PODY_BCU,
   //    PODN,        PODN_NCL,    PODN_NCU,    PODN_BCL,    PODN_BCU,
   //    POFD,        POFD_NCL,    POFD_NCU,    POFD_BCL,    POFD_BCU,
   //    FAR,         FAR_NCL,     FAR_NCU,     FAR_BCL,     FAR_BCU,
   //    CSI,         CSI_NCL,     CSI_NCU,     CSI_BCL,     CSI_BCU,
   //    GSS,         GSS_BCL,     GSS_BCU,
   //    HK,          HK_NCL,      HK_NCU,      HK_BCL,      HK_BCU,
   //    HSS,         HSS_BCL,     HSS_BCU,
   //    ODDS,        ODDS_NCL,    ODDS_NCU,    ODDS_BCL,    ODDS_BCU
   //
   at.set_entry(r, c+0,  // Total count
      nbrcts_info.cts_info.cts.n());

   at.set_entry(r, c+1,  // Base Rate (oy_tp)
      nbrcts_info.cts_info.baser.v);

   at.set_entry(r, c+2,  // Base Rate (oy_tp) NCL
      nbrcts_info.cts_info.baser.v_ncl[i]);

   at.set_entry(r, c+3,  // Base Rate (oy_tp) NCU
      nbrcts_info.cts_info.baser.v_ncu[i]);

   at.set_entry(r, c+4,  // Base Rate (oy_tp) BCL
      nbrcts_info.cts_info.baser.v_bcl[i]);

   at.set_entry(r, c+5,  // Base Rate (oy_tp) BCU
      nbrcts_info.cts_info.baser.v_bcu[i]);

   at.set_entry(r, c+6,  // Forecast Mean (fy_tp)
      nbrcts_info.cts_info.fmean.v);

   at.set_entry(r, c+7,  // Forecast Mean (fy_tp) NCL
      nbrcts_info.cts_info.fmean.v_ncl[i]);

   at.set_entry(r, c+8,  // Forecast Mean (fy_tp) NCU
      nbrcts_info.cts_info.fmean.v_ncu[i]);

   at.set_entry(r, c+9,  // Forecast Mean (fy_tp) BCL
      nbrcts_info.cts_info.fmean.v_bcl[i]);

   at.set_entry(r, c+10, // Forecast Mean (fy_tp) BCU
      nbrcts_info.cts_info.fmean.v_bcu[i]);

   at.set_entry(r, c+11, // Accuracy
      nbrcts_info.cts_info.acc.v);

   at.set_entry(r, c+12, // Accuracy NCL
      nbrcts_info.cts_info.acc.v_ncl[i]);

   at.set_entry(r, c+13, // Accuracy NCU
      nbrcts_info.cts_info.acc.v_ncu[i]);

   at.set_entry(r, c+14, // Accuracy BCL
      nbrcts_info.cts_info.acc.v_bcl[i]);

   at.set_entry(r, c+15, // Accuracy BCU
      nbrcts_info.cts_info.acc.v_bcu[i]);

   at.set_entry(r, c+16, // Frequency Bias
      nbrcts_info.cts_info.fbias.v);

   at.set_entry(r, c+17, // Frequency Bias BCL
      nbrcts_info.cts_info.fbias.v_bcl[i]);

   at.set_entry(r, c+18, // Frequency Bias BCU
      nbrcts_info.cts_info.fbias.v_bcu[i]);

   at.set_entry(r, c+19, // POD Yes
      nbrcts_info.cts_info.pody.v);

   at.set_entry(r, c+20, // POD Yes NCL
      nbrcts_info.cts_info.pody.v_ncl[i]);

   at.set_entry(r, c+21, // POD Yes NCU
      nbrcts_info.cts_info.pody.v_ncu[i]);

   at.set_entry(r, c+22, // POD Yes BCL
      nbrcts_info.cts_info.pody.v_bcl[i]);

   at.set_entry(r, c+23, // POD Yes BCU
      nbrcts_info.cts_info.pody.v_bcu[i]);

   at.set_entry(r, c+24, // POD No
      nbrcts_info.cts_info.podn.v);

   at.set_entry(r, c+25, // POD No NCL
      nbrcts_info.cts_info.podn.v_ncl[i]);

   at.set_entry(r, c+26, // POD No NCU
      nbrcts_info.cts_info.podn.v_ncu[i]);

   at.set_entry(r, c+27, // POD No BCL
      nbrcts_info.cts_info.podn.v_bcl[i]);

   at.set_entry(r, c+28, // POD No BCU
      nbrcts_info.cts_info.podn.v_bcu[i]);

   at.set_entry(r, c+29, // POFD
      nbrcts_info.cts_info.pofd.v);

   at.set_entry(r, c+30, // POFD NCL
      nbrcts_info.cts_info.pofd.v_ncl[i]);

   at.set_entry(r, c+31, // POFD NCU
      nbrcts_info.cts_info.pofd.v_ncu[i]);

   at.set_entry(r, c+32, // POFD BCL
      nbrcts_info.cts_info.pofd.v_bcl[i]);

   at.set_entry(r, c+33, // POFD BCU
      nbrcts_info.cts_info.pofd.v_bcu[i]);

   at.set_entry(r, c+34, // FAR
      nbrcts_info.cts_info.far.v);

   at.set_entry(r, c+35, // FAR NCL
      nbrcts_info.cts_info.far.v_ncl[i]);

   at.set_entry(r, c+36, // FAR NCU
      nbrcts_info.cts_info.far.v_ncu[i]);

   at.set_entry(r, c+37, // FAR BCL
      nbrcts_info.cts_info.far.v_bcl[i]);

   at.set_entry(r, c+38, // FAR BCU
      nbrcts_info.cts_info.far.v_bcu[i]);

   at.set_entry(r, c+39, // CSI (Threat Score)
      nbrcts_info.cts_info.csi.v);

   at.set_entry(r, c+40, // CSI (Threat Score) NCL
      nbrcts_info.cts_info.csi.v_ncl[i]);

   at.set_entry(r, c+41, // CSI (Threat Score) NCU
      nbrcts_info.cts_info.csi.v_ncu[i]);

   at.set_entry(r, c+42, // CSI (Threat Score) BCL
      nbrcts_info.cts_info.csi.v_bcl[i]);

   at.set_entry(r, c+43, // CSI (Threat Score) BCU
      nbrcts_info.cts_info.csi.v_bcu[i]);

   at.set_entry(r, c+44, // Gilbert Skill Score (ETS)
      nbrcts_info.cts_info.gss.v);

   at.set_entry(r, c+45, // Gilbert Skill Score (ETS) BCL
      nbrcts_info.cts_info.gss.v_bcl[i]);

   at.set_entry(r, c+46, // Gilbert Skill Score (ETS) BCU
      nbrcts_info.cts_info.gss.v_bcu[i]);

   at.set_entry(r, c+47, // Hanssen-Kuipers Discriminant (TSS)
      nbrcts_info.cts_info.hk.v);

   at.set_entry(r, c+48, // Hanssen-Kuipers Discriminant (TSS) NCL
      nbrcts_info.cts_info.hk.v_ncl[i]);

   at.set_entry(r, c+49, // Hanssen-Kuipers Discriminant (TSS) NCU
      nbrcts_info.cts_info.hk.v_ncu[i]);

   at.set_entry(r, c+50, // Hanssen-Kuipers Discriminant (TSS) BCL
      nbrcts_info.cts_info.hk.v_bcl[i]);

   at.set_entry(r, c+51, // Hanssen-Kuipers Discriminant (TSS) BCU
      nbrcts_info.cts_info.hk.v_bcu[i]);

   at.set_entry(r, c+52, // Heidke Skill Score
      nbrcts_info.cts_info.hss.v);

   at.set_entry(r, c+53, // Heidke Skill Score BCL
      nbrcts_info.cts_info.hss.v_bcl[i]);

   at.set_entry(r, c+54, // Heidke Skill Score BCU
      nbrcts_info.cts_info.hss.v_bcu[i]);

   at.set_entry(r, c+55, // Odds Ratio
      nbrcts_info.cts_info.odds.v);

   at.set_entry(r, c+56, // Odds Ratio NCL
      nbrcts_info.cts_info.odds.v_ncl[i]);

   at.set_entry(r, c+57, // Odds Ratio NCU
      nbrcts_info.cts_info.odds.v_ncu[i]);

   at.set_entry(r, c+58, // Odds Ratio BCL
      nbrcts_info.cts_info.odds.v_bcl[i]);

   at.set_entry(r, c+59, // Odds Ratio BCU
      nbrcts_info.cts_info.odds.v_bcu[i]);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_nbrcnt_cols(const NBRCNTInfo &nbrcnt_info, int i,
                       AsciiTable &at, int r, int c) {

   //
   // Neighborhood Method Continuous Statistics (NBRCNT)
   // Dump out the NBRCNT line:
   //    TOTAL,
   //    FBS,         FBS_BCL,     FBS_BCU,
   //    FSS,         FSS_BCL,     FSS_BCU,
   //
   at.set_entry(r, c+0,  // Total Count
      nbrcnt_info.cnt_info.n);

   at.set_entry(r, c+1,  // Fractions Brier Score
      nbrcnt_info.fbs.v);

   at.set_entry(r, c+2,  // Fractions Brier Score BCL
      nbrcnt_info.fbs.v_bcl[i]);

   at.set_entry(r, c+3,  // Fractions Brier Score BCU
      nbrcnt_info.fbs.v_bcu[i]);

   at.set_entry(r, c+4,  // Fractions Skill score
      nbrcnt_info.fss.v);

   at.set_entry(r, c+5,  // Fractions Skill score BCL
      nbrcnt_info.fss.v_bcl[i]);

   at.set_entry(r, c+6,  // Fractions Skill score BCU
      nbrcnt_info.fss.v_bcu[i]);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_mpr_cols(const PairData *pd_ptr, int i,
                    AsciiTable &at, int r, int c) {

   //
   // Matched Pairs (MPR)
   // Dump out the MPR line:
   //    TOTAL,       INDEX,       OBS_SID,
   //    OBS_LAT,     OBS_LON,     OBS_LVL,
   //    OBS_ELV,     FCST,        OBS,
   //    CLIMO
   //
   at.set_entry(r, c+0,  // Total Number of Pairs
      pd_ptr->n_pair);

   at.set_entry(r, c+1,  // Index of Current Pair
      i+1);

   at.set_entry(r, c+2,  // Station ID
      pd_ptr->sid_sa[i]);

   at.set_entry(r, c+3,  // Latitude
      pd_ptr->lat_na[i]);

   at.set_entry(r, c+4,  // Longitude
      pd_ptr->lon_na[i]);

   at.set_entry(r, c+5,  // Level
      pd_ptr->lvl_na[i]);

   at.set_entry(r, c+6,  // Elevation
      pd_ptr->elv_na[i]);

   at.set_entry(r, c+7,  // Forecast Value
      pd_ptr->f_na[i]);

   at.set_entry(r, c+8,  // Observation Value
      pd_ptr->o_na[i]);

   at.set_entry(r, c+9,  // Climatology Value
      pd_ptr->c_na[i]);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_isc_cols(const ISCInfo &isc_info, int i,
                    AsciiTable &at, int r, int c) {

   //
   // Intensity Scale Statistics (ISC)
   // Dump out the ISC line:
   //    TOTAL,
   //    NSCALE,      ISCALE,      MSE,
   //    ISC,         FENERGY2,    OENERGY2,
   //    BASER,       FBIAS
   //
   at.set_entry(r, c+0,     // Total Number of Points
      isc_info.total);

   at.set_entry(r, c+1,     // Tile dimension
      isc_info.tile_dim);

   at.set_entry(r, c+2,     // Tile x-lower left point
      isc_info.tile_xll);

   at.set_entry(r, c+3,     // Tile y-lower left point
      isc_info.tile_yll);

   at.set_entry(r, c+4,     // Total Number of Scales
      isc_info.n_scale);

   at.set_entry(r, c+5,     // Index of Current Scale
      i+1);

   if(i < 0) {
      at.set_entry(r, c+6,  // Mean Squared Error
         isc_info.mse);

      at.set_entry(r, c+7,  // Intensity Scale Score
         isc_info.isc);

      at.set_entry(r, c+8,  // Forecast Energy Squared
         isc_info.fen);

      at.set_entry(r, c+9,  // Observation Energy Squared
         isc_info.oen);
   }
   else {
      at.set_entry(r, c+6,  // Mean Squared Error
         isc_info.mse_scale[i]);

      at.set_entry(r, c+7,  // Intensity Scale Score
         isc_info.isc_scale[i]);

      at.set_entry(r, c+8,  // Forecast Energy Squared
         isc_info.fen_scale[i]);

      at.set_entry(r, c+9,  // Observation Energy Squared
         isc_info.oen_scale[i]);
   }

   at.set_entry(r, c+10,    // Base Rate
      isc_info.baser);

   at.set_entry(r, c+11,    // Forecast Bias
      isc_info.fbias);

   return;
}
////////////////////////////////////////////////////////////////////////

void write_rhist_cols(const EnsPairData *pd_ptr,
                      AsciiTable &at, int r, int c) {
   int i, col;

   //
   // Ensemble Ranked Histogram
   // Dump out the RHIST line:
   //    TOTAL,   CRPS,   IGN,
   //    N_RANKS, [RANK_] (for each bin)
   //
   at.set_entry(r, c+0,  // Total Number of Ranked Observations
      nint(pd_ptr->rhist_na.sum()));

   at.set_entry(r, c+1,  // Continuous Ranked Probability Score
      pd_ptr->crps_na.mean());

   at.set_entry(r, c+2,  // Ignorance Score
      pd_ptr->ign_na.mean());

   at.set_entry(r, c+3,  // Total Number of Ranks
      pd_ptr->rhist_na.n_elements());

   //
   // Write RANK_i count for each bin
   //
   for(i=0, col=c+4; i<pd_ptr->rhist_na.n_elements(); i++) {

      at.set_entry(r, col, // RANK_i
         nint(pd_ptr->rhist_na[i]));
      col++;
   }

   return;
};

////////////////////////////////////////////////////////////////////////

void write_orank_cols(const EnsPairData *pd_ptr, int i,
                      AsciiTable &at, int r, int c) {
   int j, col;

   //
   // Ensemble Observation Rank Matched Pairs
   // Dump out the ORANK line:
   //    TOTAL,       INDEX,       OBS_SID,
   //    OBS_LAT,     OBS_LON,     OBS_LVL,
   //    OBS_ELV,     OBS,         PIT,
   //    RANK,        N_ENS_VLD,   N_ENS,
   //    [ENS_] (for each ensemble member)
   //
   at.set_entry(r, c+0,  // Total Number of Pairs
      pd_ptr->n_pair);

   at.set_entry(r, c+1,  // Index of Current Pair
      i+1);

   at.set_entry(r, c+2,  // Station ID
      pd_ptr->sid_sa[i]);

   at.set_entry(r, c+3,  // Latitude
      pd_ptr->lat_na[i]);

   at.set_entry(r, c+4,  // Longitude
      pd_ptr->lon_na[i]);

   at.set_entry(r, c+5,  // Level
      pd_ptr->lvl_na[i]);

   at.set_entry(r, c+6,  // Elevation
      pd_ptr->elv_na[i]);

   at.set_entry(r, c+7,  // Observation Value
      pd_ptr->o_na[i]);

   at.set_entry(r, c+8,  // Probability Integral Transform
      pd_ptr->pit_na[i]);

   at.set_entry(r, c+9,  // Observation Rank
      nint(pd_ptr->r_na[i]));

   at.set_entry(r, c+10, // Number of valid ensembles
      nint(pd_ptr->v_na[i]));

   at.set_entry(r, c+11, // Number of ensembles
      pd_ptr->e_na[i].n_elements());

   //
   // Write ENS_j for each ensemble member
   //
   for(j=0, col=c+12; j<pd_ptr->e_na[i].n_elements(); j++) {

      at.set_entry(r, col, // ENS_j
         pd_ptr->e_na[i][j]);
      col++;
   }

   return;
};

////////////////////////////////////////////////////////////////////////
