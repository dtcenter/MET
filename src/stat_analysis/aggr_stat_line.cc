// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   aggr_stat_line.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    12/17/08  Halley Gotway   New
//   001    05/24/10  Halley Gotway   Add aggregate_rhist_lines.
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cmath>
#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "aggr_stat_line.h"
#include "parse_stat_line.h"

////////////////////////////////////////////////////////////////////////
//
// The aggr_contable_lines routine should only be called when the
// -line_type option has been used exactly once.
//
////////////////////////////////////////////////////////////////////////

void aggr_contable_lines(const char *jobstring, LineDataFile &f,
                         STATAnalysisJob &j, CTSInfo &cts_info,
                         STATLineType lt, int &n_in, int &n_out,
                        int verbosity) {
   STATLine line;
   TTContingencyTable ct;
   int fy_oy, fy_on, fn_oy, fn_on;
   char line_type[max_str_len];

   //
   // Initialize the Contingency Table counts
   //
   fy_oy = fy_on = fn_oy = fn_on = 0;
   cts_info.clear();

   //
   // Process the STAT lines
   //
   while(f >> line) {

      n_in++;

      if(j.is_keeper(line)) {

         //
         // Zero out the contingecy table object
         //
         ct.zero_out();

         //
         // Switch on the line type looking only for contingency
         // table types of lines
         //
         switch(line.type()) {

            case(stat_fho):
               parse_fho_ctable(line, ct);
               break;

            case(stat_ctc):
               parse_ctc_ctable(line, ct);
               break;

            case(stat_nbrctc):
               parse_nbrctc_ctable(line, ct);
               break;

            default:
               statlinetype_to_string(line.type(), line_type);
               cerr << "\n\nERROR: aggr_contable_lines() -> "
                    << "line type value of " << line_type
                    << " not currently supported for the aggregation "
                    << "job!\n\n" << flush;
               throw(1);
         } // end switch

         //
         // Increment the ctable counts
         //
         fy_oy += ct.fy_oy();
         fy_on += ct.fy_on();
         fn_oy += ct.fn_oy();
         fn_on += ct.fn_on();

         if(j.dr_out) *(j.dr_out) << line;

         n_out++;
      }
   } // end while

   //
   // Store the ctable sums
   //
   cts_info.cts.set_fy_oy(fy_oy);
   cts_info.cts.set_fy_on(fy_on);
   cts_info.cts.set_fn_oy(fn_oy);
   cts_info.cts.set_fn_on(fn_on);

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_nx2_contable_lines(const char *jobstring, LineDataFile &f,
                             STATAnalysisJob &j, PCTInfo &pct_info,
                             STATLineType lt, int &n_in, int &n_out,
                             int verbosity) {
   STATLine line;
   Nx2ContingencyTable pct;
   char line_type[max_str_len];
   int i, oy, on;

   //
   // Initialize
   //
   pct_info.clear();

   //
   // Process the STAT lines
   //
   while(f >> line) {

      n_in++;

      if(j.is_keeper(line)) {

         //
         // Initialize
         //
         pct.clear();

         //
         // Switch on the line type looking only for contingency
         // table types of lines
         //
         switch(line.type()) {

            case(stat_pct):
               parse_nx2_ctable(line, pct);
               break;

            default:
               statlinetype_to_string(line.type(), line_type);
               cerr << "\n\nERROR: aggr_nx2_contable_lines() -> "
                    << "line type value of " << line_type
                    << " not currently supported for the aggregation "
                    << "job!\n\n" << flush;
               throw(1);
         } // end switch

         //
         // Store the first Nx2 Contingency Table
         //
         if(pct_info.pct.n() == 0) pct_info.pct = pct;
         //
         // Increment the Nx2 Contingency Table counts
         //
         else {

            //
            // The number of thresholds must remain the same
            //
            if(pct_info.pct.nrows() != pct.nrows()) {
               cerr << "\n\nERROR: aggr_nx2_contable_lines() -> "
                    << "when aggregating PCT lines the number of "
                    << "thresholds must remain the same for all lines, "
                    << pct_info.pct.nrows() << " != " << pct.nrows()
                    << "\n\n" << flush;
               throw(1);
            }

            for(i=0; i<pct_info.pct.nrows(); i++) {

               //
               // The threshold values must remain the same
               //
               if(pct_info.pct.threshold(i) != pct.threshold(i)) {
                  cerr << "\n\nERROR: aggr_nx2_contable_lines() -> "
                       << "when aggregating PCT lines the threshold "
                       << "values must remain the same for all lines, "
                       << pct_info.pct.threshold(i) << " != "
                       << pct.threshold(i)
                       << "\n\n" << flush;
                  throw(1);
               }

               //
               // Increment the counts
               //
               oy = pct_info.pct.event_count_by_row(i);
               on = pct_info.pct.nonevent_count_by_row(i);

               pct_info.pct.set_entry(i, nx2_event_column,
                                      oy+pct.event_count_by_row(i));
               pct_info.pct.set_entry(i, nx2_nonevent_column,
                                      on+pct.nonevent_count_by_row(i));
            } // end for i
         } // end else

         if(j.dr_out) *(j.dr_out) << line;

         n_out++;
      }
   } // end while

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_partial_sum_lines(const char *jobstring, LineDataFile &f,
                            STATAnalysisJob &j,
                            SL1L2Info &sl1l2_info, VL1L2Info &vl1l2_info,
                            STATLineType lt, int &n_in, int &n_out,
                           int verbosity) {
   STATLine line;
   SL1L2Info s;
   VL1L2Info v;

   //
   // Initialize the partial sums
   //
   sl1l2_info.clear();
   vl1l2_info.clear();

   //
   // Process the STAT lines
   //
   while(f >> line) {

      n_in++;

      if(j.is_keeper(line)) {

         //
         // Switch on the line type.
         // For each partial sum line type, clear out the object,
         // parse the new line, and add it to running sum.
         //
         switch(line.type()) {

            case(stat_sl1l2):
               s.clear();
               parse_sl1l2_line(line, s);
               sl1l2_info += s;
               break;

            case(stat_sal1l2):
               s.clear();
               parse_sal1l2_line(line, s);
               sl1l2_info += s;
               break;

            case(stat_vl1l2):
               v.clear();
               parse_vl1l2_line(line, v);
               vl1l2_info += v;
               break;

            case(stat_val1l2):
               v.clear();
               parse_val1l2_line(line, v);
               vl1l2_info += v;
               break;

            default:
               cerr << "\n\nERROR: aggr_partial_sum_lines() -> "
                    << "should only encounter partial sum line types!\n\n"
                    << flush;
               throw(1);
         } // end switch

         if(j.dr_out) *(j.dr_out) << line;

         n_out++;
      }
   } // end while

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_vl1l2_wdir(const char *jobstring, LineDataFile &f,
                     STATAnalysisJob &j,
                     VL1L2Info &vl1l2_info,
                     NumArray &uf_na, NumArray &vf_na,
                     NumArray &uo_na, NumArray &vo_na,
                     STATLineType lt, int &n_in, int &n_out,
                     int verbosity) {
   STATLine line;
   VL1L2Info v_info;
   double u, v;

   //
   // Initialize
   //
   vl1l2_info.clear();
   uf_na.clear();
   vf_na.clear();
   uo_na.clear();
   vo_na.clear();

   //
   // Process the STAT lines
   //
   while(f >> line) {

      n_in++;

      if(j.is_keeper(line)) {

         //
         // Switch on the line type.
         // For each partial sum line type, clear out the object,
         // parse the new line, and add it to running sum.
         //
         switch(line.type()) {

            case(stat_vl1l2):
               v_info.clear();
               parse_vl1l2_line(line, v_info);
               vl1l2_info += v_info;

               // Convert U and V component of winds to unit vectors,
               // and store the values.
               convert_u_v_to_unit(v_info.ufbar, v_info.vfbar, u, v);
               uf_na.add(u);
               vf_na.add(v);

               convert_u_v_to_unit(v_info.uobar, v_info.vobar, u, v);
               uo_na.add(u);
               vo_na.add(v);
               break;

            case(stat_val1l2):
               v_info.clear();
               parse_val1l2_line(line, v_info);
               vl1l2_info += v_info;

               // Convert U and V component of winds to unit vectors,
               // and store the values.
               convert_u_v_to_unit(v_info.ufabar, v_info.vfabar, u, v);
               uf_na.add(u);
               vf_na.add(v);

               convert_u_v_to_unit(v_info.uoabar, v_info.voabar, u, v);
               uo_na.add(u);
               vo_na.add(v);
               break;

            default:
               cerr << "\n\nERROR: aggr_vl1l2_wdir() -> "
                    << "should only encounter partial sum line types!\n\n"
                    << flush;
               throw(1);
         } // end switch

         if(j.dr_out) *(j.dr_out) << line;

         n_out++;
      }
   } // end while

   return;
}

////////////////////////////////////////////////////////////////////////

void read_mpr_lines(const char *jobstring, LineDataFile &f,
                    STATAnalysisJob &j,
                    int &fcst_gc, int &obs_gc,
                    NumArray &f_na, NumArray &o_na, NumArray &c_na,
                    int &n_in, int &n_out,
                    int verbosity) {
   STATLine line;
   MPRData m;

   //
   // Initialize the NumArray objects
   //
   f_na.clear();
   o_na.clear();
   c_na.clear();
   fcst_gc = obs_gc = 0;

   //
   // Process the STAT lines
   //
   while(f >> line) {

      n_in++;

      if(j.is_keeper(line)) {

         //
         // Switch on the line type.
         //
         switch(line.type()) {

            case(stat_mpr):

               parse_mpr_line(line, m);

               //
               // Check the grid and poly masks if specified.
               // Convert degrees_east to degrees_west.
               //
               if(!j.is_in_mask_grid(m.obs_lat, (-1.0)*m.obs_lon) ||
                  !j.is_in_mask_poly(m.obs_lat, (-1.0)*m.obs_lon))
                  continue;

               //
               // Check for bad data.
               //
               if(is_bad_data(m.fcst) || is_bad_data(m.obs)) continue;

               //
               // Store or check the GRIB codes
               //
               if(fcst_gc == 0 || obs_gc == 0) {
                  fcst_gc = m.fcst_gc;
                  obs_gc  = m.obs_gc;
               }
               else if(fcst_gc != m.fcst_gc ||
                       obs_gc  != m.obs_gc) {
                  cerr << "\n\nERROR: read_mpr_lines() -> "
                       << "the forecast variable type or observation "
                       << "variable type should remain the same!\n\n"
                       << flush;
                  throw(1);
               }

               //
               // Store the values
               //
               f_na.add(m.fcst);
               o_na.add(m.obs);
               c_na.add(m.climo);

               break;

            default:
               cerr << "\n\nERROR: read_mpr_lines() -> "
                    << "should only encounter MPR line types!\n\n"
                    << flush;
               throw(1);
         } // end switch

         if(j.dr_out) *(j.dr_out) << line;

         n_out++;
      }
   } // end while

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_mpr_lines_ct(STATAnalysisJob &j,
                       const NumArray &f_na,
                       const NumArray &o_na,
                       CTSInfo &cts_info) {
   int i;
   int n = f_na.n_elements();
   SingleThresh ft = j.out_fcst_thresh[0];

   //
   // Update the contingency table counts
   //
   for(i=0; i<n; i++) {

      if(      ft.check(f_na[i]) &&
               j.out_obs_thresh.check(o_na[i]))
         cts_info.cts.inc_fy_oy();
      else if( ft.check(f_na[i]) &&
              !j.out_obs_thresh.check(o_na[i]))
         cts_info.cts.inc_fy_on();
      else if(!ft.check(f_na[i]) &&
               j.out_obs_thresh.check(o_na[i]))
         cts_info.cts.inc_fn_oy();
      else if(!ft.check(f_na[i]) &&
              !j.out_obs_thresh.check(o_na[i]))
         cts_info.cts.inc_fn_on();
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_mpr_lines_cts(STATAnalysisJob &j,
                        const NumArray &f_na,
                        const NumArray &o_na,
                        CTSInfo &cts_info) {
   CTSInfo *cts_info_ptr;
   gsl_rng *rng_ptr = (gsl_rng *) 0;

   //
   // If there are no matched pairs to process, return
   //
   if(f_na.n_elements() == 0 || o_na.n_elements() == 0) return;

   //
   // Store the out_alpha value
   //
   cts_info.allocate_n_alpha(1);
   cts_info.alpha[0] = j.out_alpha;

   //
   // Store the thresholds to be applied.
   //
   if(j.out_fcst_thresh[0].type == thresh_na) {
      cerr << "\n\nERROR: aggr_mpr_lines_cts() -> "
           << "when computing CTS lines, -out_fcst_thresh must be "
           << "used.\n\n" << flush;
      throw(1);
   }
   else {
      cts_info.cts_fcst_thresh = j.out_fcst_thresh[0];
   }

   if(j.out_obs_thresh.type == thresh_na) {
      cerr << "\n\nERROR: aggr_mpr_lines_cts() -> "
           << "when computing CTS lines, -out_obs_thresh must be "
           << "used.\n\n" << flush;
      throw(1);
   }
   else {
      cts_info.cts_obs_thresh = j.out_obs_thresh;
   }

   //
   // Set up the random number generator and seed value
   //
   rng_set(rng_ptr, j.boot_rng, j.boot_seed);

   //
   // Compute the counts, stats, normal confidence intervals, and
   // bootstrap confidence intervals
   //
   cts_info_ptr = &cts_info;
   if(j.boot_interval == boot_bca_flag) {
      compute_cts_stats_ci_bca(rng_ptr, f_na, o_na,
         j.n_boot_rep,
         cts_info_ptr, 1, 1,
         j.rank_corr_flag, j.tmp_dir);
   }
   else {
      compute_cts_stats_ci_perc(rng_ptr, f_na, o_na,
         j.n_boot_rep, j.boot_rep_prop,
         cts_info_ptr, 1, 1,
         j.rank_corr_flag, j.tmp_dir);
   }

   //
   // Deallocate memory for the random number generator
   //
   rng_free(rng_ptr);

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_mpr_lines_cnt(STATAnalysisJob &j,
                        int fcst_gc, int obs_gc,
                        const NumArray &f_na, const NumArray &o_na,
                        CNTInfo &cnt_info) {
   gsl_rng *rng_ptr = (gsl_rng *) 0;

   //
   // If there are no matched pairs to process, return
   //
   if(f_na.n_elements() == 0 || o_na.n_elements() == 0) return;

   //
   // Store the out_alpha value
   //
   cnt_info.allocate_n_alpha(1);
   cnt_info.alpha[0] = j.out_alpha;

   //
   // Set up the random number generator and seed value
   //
   rng_set(rng_ptr, j.boot_rng, j.boot_seed);

   //
   // Compute the stats, normal confidence intervals, and
   // bootstrap confidence intervals
   //
   if(j.boot_interval == boot_bca_flag) {
      compute_cnt_stats_ci_bca(rng_ptr, f_na, o_na,
         fcst_gc, obs_gc,
         j.n_boot_rep,
         cnt_info, 1,
         j.rank_corr_flag, j.tmp_dir);
   }
   else {
      compute_cnt_stats_ci_perc(rng_ptr, f_na, o_na,
         fcst_gc, obs_gc,
         j.n_boot_rep, j.boot_rep_prop,
         cnt_info, 1,
         j.rank_corr_flag, j.tmp_dir);
   }

   //
   // Deallocate memory for the random number generator
   //
   rng_free(rng_ptr);

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_mpr_lines_psums(STATAnalysisJob &j,
               const NumArray &f_na,
               const NumArray &o_na,
               const NumArray &c_na,
               SL1L2Info &s_info) {
   int i;
   int n = f_na.n_elements();
   int scount, sacount;
   double f, o, c;
   double f_sum,  o_sum,  ff_sum,  oo_sum,  fo_sum;
   double fa_sum, oa_sum, ffa_sum, ooa_sum, foa_sum;

   //
   // Initialize the SL1L2Info object and counts
   //
   s_info.clear();
   scount = sacount = 0;
   f_sum  = o_sum  =  ff_sum  = oo_sum  = fo_sum  = 0.0;
   fa_sum = oa_sum =  ffa_sum = ooa_sum = foa_sum = 0.0;

   //
   // Update the partial sums
   //
   for(i=0; i<n; i++) {

      //
      // Update the counts for this matched pair
      //
      f = f_na[i];
      o = o_na[i];
      c = c_na[i];

      f_sum   += f;
      o_sum   += o;
      ff_sum  += f*f;
      oo_sum  += o*o;
      fo_sum  += f*o;
      scount  += 1;

      //
      // Check c for valid data
      //
      if(!is_bad_data(c)) {

         fa_sum  += f-c;
         oa_sum  += o-c;
         ffa_sum += (f-c)*(f-c);
         ooa_sum += (o-c)*(o-c);
         foa_sum += (f-c)*(o-c);
         sacount += 1;
      }
   } // end for

   if(scount != 0) {
      s_info.scount  = scount;
      s_info.fbar    = f_sum/scount;
      s_info.obar    = o_sum/scount;
      s_info.fobar   = fo_sum/scount;
      s_info.ffbar   = ff_sum/scount;
      s_info.oobar   = oo_sum/scount;
   }

   if(sacount != 0) {
      s_info.sacount = sacount;
      s_info.fabar   = fa_sum/sacount;
      s_info.oabar   = oa_sum/sacount;
      s_info.foabar  = foa_sum/sacount;
      s_info.ffabar  = ffa_sum/sacount;
      s_info.ooabar  = ooa_sum/sacount;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_mpr_lines_pct(STATAnalysisJob &j,
                        const NumArray &f_na,
                        const NumArray &o_na,
                        PCTInfo &pct_info) {
   int pstd_flag;

   //
   // If there are no matched pairs to process, return
   //
   if(f_na.n_elements() == 0 || o_na.n_elements() == 0) return;

   //
   // Set up the PCTInfo thresholds and alpha values
   //
   pct_info.pct_fcst_thresh = j.out_fcst_thresh;
   pct_info.pct_obs_thresh  = j.out_obs_thresh;
   pct_info.allocate_n_alpha(1);
   pct_info.alpha[0] = j.out_alpha;

   if(j.out_line_type == stat_pstd) pstd_flag = 1;
   else                             pstd_flag = 0;

   //
   // Compute the probabilistic counts and statistics
   //
   compute_pctinfo(f_na, o_na, pstd_flag, pct_info);

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_isc_lines(const char *jobstring, LineDataFile &ldf,
               STATAnalysisJob &j, ISCInfo &isc_aggr,
               int &n_in, int &n_out,
               int verbosity) {
   STATLine line;
   ISCInfo isc_info;

   int i, k, n_scale, iscale;
   double total, w, den, baser_fbias_sum;
   NumArray *total_na = (NumArray *) 0;
   NumArray *mse_na   = (NumArray *) 0;
   NumArray *fen_na   = (NumArray *) 0;
   NumArray *oen_na   = (NumArray *) 0;
   NumArray *baser_na = (NumArray *) 0;
   NumArray *fbias_na = (NumArray *) 0;

   //
   // Initialize the ISCInfo objects
   //
   isc_aggr.clear();

   //
   // Process the STAT lines
   //
   while(ldf >> line) {

      n_in++;

      if(j.is_keeper(line)) {

         //
         // Switch on the line type.
         //
         switch(line.type()) {

            case(stat_isc):

               parse_isc_line(line, isc_info, iscale);

               //
               // Check for bad data
               //
               if(is_bad_data(isc_info.total) ||
                  is_bad_data(isc_info.mse)   ||
                  is_bad_data(isc_info.fen)   ||
                  is_bad_data(isc_info.oen)   ||
                  is_bad_data(isc_info.baser) ||
                  is_bad_data(isc_info.fbias)) continue;

               //
               // After reading the first ISC line, setup the isc_info
               // object to store the data.  Also, store the number
               // of scales and make sure that it doesn't change.
               //
               if(isc_aggr.n_scale == 0) {

                  n_scale = isc_info.n_scale;

                  // Allocate room to store results for each scale
                  isc_aggr.allocate_n_scale(n_scale);
                  isc_aggr.zero_out();

                  //
                  // Initialize tile_dim, tile_xll, and tile_yll.
                  // If they stay the same over all the lines, write them out.
                  // Otherwise, write out bad data.
                  //
                  isc_aggr.tile_dim = isc_info.tile_dim;
                  isc_aggr.tile_xll = isc_info.tile_xll;
                  isc_aggr.tile_yll = isc_info.tile_yll;

                  // Allocate room to store values for each scale
                  total_na = new NumArray [n_scale + 2];
                  mse_na   = new NumArray [n_scale + 2];
                  fen_na   = new NumArray [n_scale + 2];
                  oen_na   = new NumArray [n_scale + 2];
                  baser_na = new NumArray [n_scale + 2];
                  fbias_na = new NumArray [n_scale + 2];
               }
               else {

                  if(isc_aggr.n_scale != isc_info.n_scale) {
                     cerr << "\n\nERROR: aggr_isc_lines() -> "
                          << "the number of scales must remain constant "
                          << "when aggregating ISC lines.  Use the "
                          << "\"-column_min NSCALE n\" and "
                          << "\"-column_max NSCALE n\" options to "
                          << "filter out only those lines you'd like "
                          << "to aggregate!\n\n"
                          << flush;
                     throw(1);
                  }

                  //
                  // Check to see if tile_dim, tile_xll, or tile_yll has changed.
                  // If so, write out bad data.
                  //
                  if(isc_aggr.tile_dim != bad_data_int &&
                     isc_aggr.tile_dim != isc_info.tile_dim) {
                     isc_aggr.tile_dim = bad_data_int;
                  }

                  if( (isc_aggr.tile_xll != bad_data_int &&
                       isc_aggr.tile_xll != isc_info.tile_xll) ||
                      (isc_aggr.tile_yll != bad_data_int &&
                       isc_aggr.tile_yll != isc_info.tile_yll) ) {
                     isc_aggr.tile_xll = bad_data_int;
                     isc_aggr.tile_yll = bad_data_int;
                  }
               }

               //
               // Store the data for this ISC line
               //
               total_na[iscale].add(isc_info.total);
               mse_na[iscale].add(isc_info.mse);
               fen_na[iscale].add(isc_info.fen);
               oen_na[iscale].add(isc_info.oen);
               baser_na[iscale].add(isc_info.baser);
               fbias_na[iscale].add(isc_info.fbias);

               break;

            default:
               cerr << "\n\nERROR: aggr_isc_lines() -> "
                    << "should only encounter ISC line types!\n\n"
                    << flush;
               throw(1);
         } // end switch

         if(j.dr_out) *(j.dr_out) << line;

         n_out++;
      }
   } // end while

   //
   // Return if no lines were read
   //
   if(n_out == 0) return;

   //
   // Get the sum of the totals, compute the weight, and sum the
   // weighted scores
   //
   for(i=0; i<n_scale+2; i++) {

      // Total number of points for this scale
      total = total_na[i].sum();

      // Initialize
      baser_fbias_sum = 0.0;

      // Loop through all scores for this scale
      for(k=0; k<total_na[i].n_elements(); k++) {

         // Compute the weight for each score to be aggregated
         // based on the number of points it represents
         w = total_na[i][k]/total;

         // Sum scores for the binary fields
         if(i == 0) {
            isc_aggr.mse    += w*mse_na[0][k];
            isc_aggr.fen    += w*fen_na[0][k];
            isc_aggr.oen    += w*oen_na[0][k];
            isc_aggr.baser  += w*baser_na[0][k];
            baser_fbias_sum += w*baser_na[0][k]*fbias_na[0][k];
         }
         // Weighted sum of scores for each scale
         else {
            isc_aggr.mse_scale[i-1] += w*mse_na[i][k];
            isc_aggr.fen_scale[i-1] += w*fen_na[i][k];
            isc_aggr.oen_scale[i-1] += w*oen_na[i][k];
         }
      }

      //
      // Compute the aggregated scores for the binary fields
      //
      if(i == 0) {

         // Total
         isc_aggr.total = nint(total_na[0].sum());

         // Aggregated FBIAS
         isc_aggr.fbias = baser_fbias_sum/isc_aggr.baser;

         // Compute the aggregated ISC score.  For the binary fields
         // do not divide by the number of scales.
         den = (isc_aggr.fbias*isc_aggr.baser*(1.0 - isc_aggr.baser) +
                isc_aggr.baser*(1.0 - isc_aggr.fbias*isc_aggr.baser));

         if(is_bad_data(isc_aggr.fbias) ||
            is_bad_data(isc_aggr.baser) ||
            is_eq(den, 0.0)) {
            isc_aggr.isc = bad_data_double;
         }
         else {
            isc_aggr.isc = 1.0 - isc_aggr.mse/den;
         }
      }
      //
      // Compute the aggregated scores for each scale
      //
      else {

         // Compute the aggregated ISC score.  For each scale, divide
         // by the number of scales.
         den = (isc_aggr.fbias*isc_aggr.baser*(1.0 - isc_aggr.baser) +
                isc_aggr.baser*(1.0 - isc_aggr.fbias*isc_aggr.baser))
               /(isc_aggr.n_scale+1);

         if(is_bad_data(isc_aggr.fbias) ||
            is_bad_data(isc_aggr.baser) ||
            is_eq(den, 0.0)) {
            isc_aggr.isc_scale[i-1] = bad_data_double;
         }
         else {
            isc_aggr.isc_scale[i-1] = 1.0 - isc_aggr.mse_scale[i-1]/den;
         }
      }
   } // end for i

   //
   // Deallocate memory
   //
   if(total_na) { delete [] total_na; total_na = (NumArray *) 0; }
   if(mse_na  ) { delete [] mse_na;   mse_na   = (NumArray *) 0; }
   if(fen_na  ) { delete [] fen_na;   fen_na   = (NumArray *) 0; }
   if(oen_na  ) { delete [] oen_na;   oen_na   = (NumArray *) 0; }
   if(baser_na) { delete [] baser_na; baser_na = (NumArray *) 0; }
   if(fbias_na) { delete [] fbias_na; fbias_na = (NumArray *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_rhist_lines(const char *jobstring, LineDataFile &f,
                      STATAnalysisJob &j, NumArray &rhist_na,
                      int &n_in, int &n_out, int verbosity) {
   STATLine line;
   RHISTData r_data;
   int i;

   //
   // Initialize the NumArray
   //
   rhist_na.clear();

   //
   // Process the STAT lines
   //
   while(f >> line) {

      n_in++;

      if(j.is_keeper(line)) {

         if(line.type() != stat_rhist) {
            cerr << "\n\nERROR: aggr_rhist_lines() -> "
                 << "should only encounter ranked histogram line types!\n\n"
                 << flush;
            throw(1);
         }

         //
         // Parse the current RHIST line
         //
         parse_rhist_line(line, r_data);

         //
         // Check for N_RANK remaining constant
         //
         if(rhist_na.n_elements() > 0 &&
            rhist_na.n_elements() != r_data.n_rank) {
            cerr << "\n\nERROR: aggr_rhist_lines() -> "
                 << "the \"N_RANK\" column must remain constant ("
                 << rhist_na.n_elements() << "!=" << r_data.n_rank
                 << ")!\n\n"
                 << flush;
            throw(1);
         }

         //
         // Aggregate the counts
         //
         if(rhist_na.n_elements() == 0) {
            rhist_na = r_data.rank_na;
         }
         else {
            for(i=0; i<rhist_na.n_elements(); i++) {
               rhist_na.set(i, rhist_na[i] + r_data.rank_na[i]);
            }
         }

         if(j.dr_out) *(j.dr_out) << line;

         n_out++;
      }
   } // end while

   return;
}

////////////////////////////////////////////////////////////////////////
