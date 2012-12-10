

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "mode_ps_file.h"


////////////////////////////////////////////////////////////////////////


static double Vtab, Htab_a, Htab_b, Htab_c;


////////////////////////////////////////////////////////////////////////


void ModePsFile::do_page_1(ModeFuzzyEngine & eng, EngineType eng_type, const char * title)

{

char junk[1024];
bool draw_line;
ConcatString label, thresh_str;
ConcatString tmp1_str, tmp2_str, tmp3_str;
int i, mon, day, yr, hr, minute, sec;

   ////////////////////////////////////////////////////////////////////
   //
   // First Page: create a 6 plot
   //
   ////////////////////////////////////////////////////////////////////

inc_pagenumber();

choose_font(31, 24.0);

write_centered_text(1, 1, 306.0, 752.0, 0.5, 0.5, title);

choose_font(31, 18.0);

write_centered_text(1, 1, Htab_1, 727.0, 0.5, 0.5, FcstString);
write_centered_text(1, 1, Htab_2, 727.0, 0.5, 0.5, ObsString);

   ////////////////////////////////////////////////////////////////////
   //
   // Draw raw forecast field
   //
   ////////////////////////////////////////////////////////////////////

set_view(Vtab_1, Vtab_1 + SmallPlotHeight, Htab_1);
render_image( eng, eng_type, *(eng.fcst_raw), 1, 0);
outline_view();
draw_map( &(eng.conf_info.conf) );

   ////////////////////////////////////////////////////////////////////
   //
   // Draw raw observation field
   //
   ////////////////////////////////////////////////////////////////////

set_view(Vtab_1, Vtab_1 + SmallPlotHeight, Htab_2);
render_image(eng, eng_type, *(eng.obs_raw), 0, 0);
outline_view();
draw_map( &(eng.conf_info.conf) );

   ////////////////////////////////////////////////////////////////////
   //
   // Draw fcst split field
   //
   ////////////////////////////////////////////////////////////////////

set_view(Vtab_2, Vtab_2 + SmallPlotHeight, Htab_1);
render_image(eng, eng_type, *(eng.fcst_split), 1, 1);
outline_view();
draw_map( &(eng.conf_info.conf) );
draw_convex_hulls(eng, 1, 0);

   ////////////////////////////////////////////////////////////////////
   //
   // Draw obs split field
   //
   ////////////////////////////////////////////////////////////////////

set_view(Vtab_2, Vtab_2 + SmallPlotHeight, Htab_2);
render_image(eng, eng_type, *eng.obs_split, 0, 1);
outline_view();
draw_map( &(eng.conf_info.conf) );
draw_convex_hulls(eng, 0, 0);

   ////////////////////////////////////////////////////////////////////
   //
   // Draw fcst simple ids
   //
   ////////////////////////////////////////////////////////////////////

set_view(Vtab_3, Vtab_3 + SmallPlotHeight, Htab_1);
outline_view();
draw_map( &(eng.conf_info.conf) );
plot_simple_ids(eng, 1);

   ////////////////////////////////////////////////////////////////////
   //
   // Draw obs simple ids
   //
   ////////////////////////////////////////////////////////////////////

set_view(Vtab_3, Vtab_3 + SmallPlotHeight, Htab_2);
outline_view();
draw_map( &(eng.conf_info.conf) );
plot_simple_ids(eng, 0);

   ////////////////////////////////////////////////////////////////////
   //
   // Plot interest values
   //
   ////////////////////////////////////////////////////////////////////

choose_font(31, 11.0);

write_centered_text(1, 1, Htab_3 + 1.5*TextSep, 727.0, 0.5, 0.5, FcstShortString);
write_centered_text(1, 1, Htab_3 + 4.5*TextSep, 727.0, 0.5, 0.5, ObsShortString);
write_centered_text(1, 1, Htab_3 + 7.5*TextSep, 727.0, 0.5, 0.5, "Interest");

Vtab = 727.0 - 1.5*TextSep;

draw_line = false;

for(i=0; i<(eng.n_fcst*eng.n_obs) && Vtab >= Vmargin; i++) {

   if ( (eng.info[i].interest_value < eng.conf_info.total_interest_thresh) && !draw_line )  {

      write_centered_text(1, 1, Htab_3 + 4.5*TextSep, Vtab, 0.5, 0.5, "----------------------------------");

      draw_line = true;

      Vtab -= TextSep;

   }

   label << cs_erase << eng.info[i].fcst_number;
   write_centered_text(1, 1, Htab_3 + 1.5*TextSep, Vtab, 0.5, 0.5, label);

   label << cs_erase << eng.info[i].obs_number;
   write_centered_text(1, 1, Htab_3 + 4.5*TextSep, Vtab, 0.5, 0.5, label);

   if ( eng.info[i].interest_value < 0 ) label = na_str;
   else {
      label.set_precision(4);
      label << cs_erase << eng.info[i].interest_value;
   }
   write_centered_text(1, 1, Htab_3 + 7.5*TextSep, Vtab, 0.5, 0.5, label);

   Vtab -= TextSep;

}   //  for i

   ////////////////////////////////////////////////////////////////////
   //
   // Model Name, Initialization, Valid, Lead, and Accumulation Times
   //
   ////////////////////////////////////////////////////////////////////

Vtab   = Vtab_3 - 1.0*TextSep;
Htab_a = Htab_1 - 0.5*View_box.width();
Htab_b = Htab_a + 4.0*TextSep;
Htab_c = Htab_a + 9.0*TextSep;

   //
   // Field name
   //
write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, FcstString);
write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, ObsString);
Vtab -= TextSep;

   //
   // Model Name
   //
write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "Model:");
write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, eng.conf_info.model);
Vtab -= TextSep;

   //
   // Variable Name
   //
write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "Field:");
write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, eng.conf_info.fcst_info->name());
write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, eng.conf_info.obs_info->name());
Vtab -= TextSep;

   //
   // Level Name
   //
write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "Level:");
write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, eng.conf_info.fcst_info->level_name());
write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, eng.conf_info.obs_info->level_name());
Vtab -= TextSep;

   //
   // Units
   //
write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "Units:");
write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, eng.conf_info.fcst_info->units());
write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, eng.conf_info.obs_info->units());
Vtab -= TextSep;

   //
   // Initialization Time
   //
write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "Initial:");
unix_to_mdyhms(eng.fcst_raw->data.valid() -
               eng.fcst_raw->data.lead(),
               mon, day, yr, hr, minute, sec);
snprintf(junk, sizeof(junk), "%.4i%.2i%.2i", yr, mon, day);
write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, junk);
snprintf(junk, sizeof(junk), "%.2i:%.2i:%.2i", hr, minute, sec);
write_centered_text(1, 1, Htab_b, Vtab-TextSep,
                      0.0, 0.5, junk);
unix_to_mdyhms(eng.obs_raw->data.valid() -
               eng.obs_raw->data.lead(),
               mon, day, yr, hr, minute, sec);
snprintf(junk, sizeof(junk), "%.4i%.2i%.2i", yr, mon, day);
write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, junk);
snprintf(junk, sizeof(junk), "%.2i:%.2i:%.2i", hr, minute, sec);
write_centered_text(1, 1, Htab_c, Vtab-TextSep,
                      0.0, 0.5, junk);
Vtab -= 2.0*TextSep;

   //
   // Valid time
   //
write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "Valid:");
unix_to_mdyhms(eng.fcst_raw->data.valid(),
               mon, day, yr, hr, minute, sec);
snprintf(junk, sizeof(junk), "%.4i%.2i%.2i", yr, mon, day);
write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, junk);
snprintf(junk, sizeof(junk), "%.2i:%.2i:%.2i", hr, minute, sec);
write_centered_text(1, 1, Htab_b, Vtab-TextSep,
                      0.0, 0.5, junk);
unix_to_mdyhms(eng.obs_raw->data.valid(),
               mon, day, yr, hr, minute, sec);
snprintf(junk, sizeof(junk), "%.4i%.2i%.2i", yr, mon, day);
write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, junk);
snprintf(junk, sizeof(junk), "%.2i:%.2i:%.2i", hr, minute, sec);
write_centered_text(1, 1, Htab_c, Vtab-TextSep,
                      0.0, 0.5, junk);
Vtab -= 2.0*TextSep;

   //
   // Accumulation time
   //
write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "Accum:");
sec_to_hms(eng.fcst_raw->data.accum(), hr, minute, sec);
snprintf(junk, sizeof(junk), "%.2i:%.2i:%.2i", hr, minute, sec);
write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, junk);
sec_to_hms(eng.obs_raw->data.accum(), hr, minute, sec);
snprintf(junk, sizeof(junk), "%.2i:%.2i:%.2i", hr, minute, sec);
write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, junk);
Vtab -= TextSep;

   ////////////////////////////////////////////////////////////////////
   //
   // ModeFuzzyEngine Weights
   //
   ////////////////////////////////////////////////////////////////////

Vtab -= TextSep;

Htab_a = Htab_1 - 0.5*View_box.width();
Htab_b = Htab_a + 8.0*TextSep;
Htab_c = Htab_a + 11.0*TextSep;

   //
   // Centroid and Boundary Distance Weights
   //
write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5,
                      "Centroid/Boundary:");
snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.centroid_dist_wt);
write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, junk);
snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.boundary_dist_wt);
write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, junk);
Vtab -= TextSep;

   //
   // Convex Hull Distance and Angle Difference Weights
   //
write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5,
                      "Convex Hull/Angle:");
snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.convex_hull_dist_wt);
write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, junk);
snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.angle_diff_wt);
write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, junk);
Vtab -= TextSep;

   //
   // Area Ratio and Intesection Over Minimum Area Weights
   //
write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5,
                      "Area/Intersection Area:");
snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.area_ratio_wt);
write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, junk);
snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.int_area_ratio_wt);
write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, junk);
Vtab -= TextSep;

   //
   // Complexity Ratio and Intensity Ratio Weights
   //
write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5,
                      "Complexity/Intensity:");
snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.complexity_ratio_wt);
write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, junk);
snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.inten_perc_ratio_wt);
write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, junk);
Vtab -= TextSep;

   //
   // Total Interest Threshold
   //
write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5,
                      "Total Interest Thresh:");
snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.total_interest_thresh);
write_centered_text(1, 1, (Htab_b+Htab_c)/2.0, Vtab, 0.0, 0.5,
                      junk);
Vtab -= TextSep;

   ////////////////////////////////////////////////////////////////////
   //
   // ModeFuzzyEngine configuration
   //
   ////////////////////////////////////////////////////////////////////

Vtab   = Vtab_3 - 1.0*TextSep;
Htab_a = Htab_2 - 0.5*View_box.width();
Htab_b = Htab_a + 5.0*TextSep;
Htab_c = Htab_a + 10.0*TextSep;

if ( eng_type == FOEng )  do_page_1_FOEng (eng, eng_type, title);
else                      do_page_1_other (eng, eng_type, title);

showpage();

return;

}


////////////////////////////////////////////////////////////////////////

void ModePsFile::do_page_1_FOEng(ModeFuzzyEngine & eng, EngineType eng_type, const char * title)

{

char junk[1024];
ConcatString label, thresh_str;
ConcatString tmp1_str, tmp2_str, tmp3_str;
double v;

      //
      // Field name
      //
      write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, FcstString);
      write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, ObsString);
      Vtab -= TextSep;

      //
      // Mask missing, grid, and polyline Flags
      //
      write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "Mask M/G/P:");
      if(eng.conf_info.mask_missing_flag == FieldType_Both ||
         eng.conf_info.mask_missing_flag == FieldType_Fcst) tmp1_str = "on";
      else                                                  tmp1_str = "off";
      if(eng.conf_info.mask_grid_flag == FieldType_Both ||
         eng.conf_info.mask_grid_flag == FieldType_Fcst)    tmp2_str = "on";
      else                                                  tmp2_str = "off";
      if(eng.conf_info.mask_grid_flag == FieldType_Both ||
         eng.conf_info.mask_grid_flag == FieldType_Fcst)    tmp3_str = "on";
      else                                                  tmp3_str = "off";
      label << cs_erase << tmp1_str << '/' << tmp2_str << '/' << tmp3_str;
      write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, label);

      if(eng.conf_info.mask_missing_flag == FieldType_Both ||
         eng.conf_info.mask_missing_flag == FieldType_Obs)  tmp1_str = "on";
      else                                                  tmp1_str = "off";
      if(eng.conf_info.mask_grid_flag == FieldType_Both ||
         eng.conf_info.mask_grid_flag == FieldType_Obs)     tmp2_str = "on";
      else                                                  tmp2_str = "off";
      if(eng.conf_info.mask_grid_flag == FieldType_Both ||
         eng.conf_info.mask_grid_flag == FieldType_Obs)     tmp3_str = "on";
      else                                                  tmp3_str = "off";
      label << cs_erase << tmp1_str << '/' << tmp2_str << '/' << tmp3_str;
      write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, label);

      Vtab -= TextSep;

      //
      // Raw threshold
      //
      write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "Raw Thresh:");
      thresh_str = eng.conf_info.fcst_raw_thresh.get_str(2);
      write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, thresh_str);
      thresh_str = eng.conf_info.obs_raw_thresh.get_str(2);
      write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, thresh_str);
      Vtab -= TextSep;

      //
      // Convolution Radius
      //
      write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "Conv Radius:");
      snprintf(junk, sizeof(junk), "%.0i gs", eng.conf_info.fcst_conv_radius);
      write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, junk);
      snprintf(junk, sizeof(junk), "%.0i gs", eng.conf_info.obs_conv_radius);
      write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, junk);
      Vtab -= TextSep;

      //
      // Convolution Threshold
      //
      write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "Conv Thresh:");
      thresh_str = eng.conf_info.fcst_conv_thresh.get_str(2);
      write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, thresh_str);
      thresh_str = eng.conf_info.obs_conv_thresh.get_str(2);
      write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, thresh_str);
      Vtab -= TextSep;

      //
      // Area Threshold
      //
      write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "Area Thresh:");
      thresh_str = eng.conf_info.fcst_area_thresh.get_str(0);
      snprintf(junk, sizeof(junk), "%s gs", thresh_str.text());
      write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, junk);
      thresh_str = eng.conf_info.obs_area_thresh.get_str(0);
      snprintf(junk, sizeof(junk), "%s gs", thresh_str.text());
      write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, junk);
      Vtab -= TextSep;

      //
      // Intensity Percentile and Threshold
      //
      write_centered_text(1, 1, Htab_a, Vtab,
                           0.0, 0.5, "Inten Thresh:");

           if(nint(eng.conf_info.fcst_inten_perc_value) == 101) tmp1_str = "mean";
      else if(nint(eng.conf_info.fcst_inten_perc_value) == 102) tmp1_str = "sum";
      else {
         snprintf(junk, sizeof(junk), "p%.0i", eng.conf_info.fcst_inten_perc_value);
         tmp1_str = junk;
      }
      thresh_str = eng.conf_info.fcst_inten_perc_thresh.get_str(2);
      label << cs_erase << tmp1_str << thresh_str;
      write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, label);

           if(nint(eng.conf_info.obs_inten_perc_value) == 101) tmp1_str = "mean";
      else if(nint(eng.conf_info.obs_inten_perc_value) == 102) tmp1_str = "sum";
      else {
         snprintf(junk, sizeof(junk), "p%.0i", eng.conf_info.obs_inten_perc_value);
         tmp1_str = junk;
      }
      thresh_str = eng.conf_info.obs_inten_perc_thresh.get_str(2);
      label << cs_erase << tmp1_str << thresh_str;
      write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, label);
      Vtab -= TextSep;

      //
      // Merge Threshold
      //
      write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "Merge Thresh:");
      thresh_str = eng.conf_info.fcst_merge_thresh.get_str(2);
      write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, thresh_str);
      thresh_str = eng.conf_info.obs_merge_thresh.get_str(2);
      write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, thresh_str);
      Vtab -= TextSep;

      /////////////////////////////////////////////////////////////////
      //
      // Matching/Merging Criteria
      //
      /////////////////////////////////////////////////////////////////

      //
      // Merging flag
      //
      write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "Merging:");
           if(eng.conf_info.fcst_merge_flag == MergeType_Thresh) label = "thresh";
      else if(eng.conf_info.fcst_merge_flag == MergeType_Engine) label = "engine";
      else if(eng.conf_info.fcst_merge_flag == MergeType_Both)   label = "thresh/engine";
      else                                                       label = "none";
      write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, label);

           if(eng.conf_info.obs_merge_flag == MergeType_Thresh) label = "thresh";
      else if(eng.conf_info.obs_merge_flag == MergeType_Engine) label = "engine";
      else if(eng.conf_info.obs_merge_flag == MergeType_Both)   label = "thresh/engine";
      else                                                      label = "none";
      write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, label);
      Vtab -= TextSep;

      //
      // Matching scheme
      //
      write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "Matching:");
           if(eng.conf_info.match_flag == MatchType_MergeBoth) label = "match/merge";
      else if(eng.conf_info.match_flag == MatchType_MergeFcst) label = "match/fcst merge";
      else if(eng.conf_info.match_flag == MatchType_NoMerge)   label = "match/no merge";
      else                                                     label = "none";
      write_centered_text(1, 1, (Htab_b+Htab_c)/2.0, Vtab, 0.0, 0.5, label);
      Vtab -= TextSep;

      /////////////////////////////////////////////////////////////////
      //
      // Object Counts and Areas
      //
      /////////////////////////////////////////////////////////////////

      //
      // Simple objects counts (Matched Simples/Unmatched Simples)
      //
      write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "Simple/M/U:");
      snprintf(junk, sizeof(junk), "%i/%i/%i", eng.n_fcst,
              eng.get_matched_fcst(0), eng.get_unmatched_fcst(0));
      write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, junk);
      snprintf(junk, sizeof(junk), "%i/%i/%i", eng.n_obs,
              eng.get_matched_obs(0), eng.get_unmatched_obs(0));
      write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, junk);
      Vtab -= TextSep;

      //
      // Area counts
      //
      write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "Area:");
      snprintf(junk, sizeof(junk), "%i gs",
              eng.get_matched_fcst(1) + eng.get_unmatched_fcst(1));
      write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, junk);
      snprintf(junk, sizeof(junk), "%i gs",
              eng.get_matched_obs(1) + eng.get_unmatched_obs(1));
      write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, junk);
      Vtab -= TextSep;

      //
      // Area counts (Matched Simple/Unmatched Simples)
      //
      write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "Area M/U:");
      snprintf(junk, sizeof(junk), "%i/%i",
              eng.get_matched_fcst(1),  eng.get_unmatched_fcst(1));
      write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, junk);
      snprintf(junk, sizeof(junk), "%i/%i",
              eng.get_matched_obs(1),  eng.get_unmatched_obs(1));
      write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, junk);
      Vtab -= TextSep;

      //
      // Cluster object counts
      //
      write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "Cluster:");
      snprintf(junk, sizeof(junk), "%i", eng.collection.n_sets);
      write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, junk);
      snprintf(junk, sizeof(junk), "%i", eng.collection.n_sets);
      write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, junk);
      Vtab -= TextSep;

      /////////////////////////////////////////////////////////////////
      //
      // Interest Values
      //
      /////////////////////////////////////////////////////////////////

      //
      // Median of Maximum Interest Values
      //
      write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "MMI:");
      v = interest_percentile(eng, 50.0, 1);
      snprintf(junk, sizeof(junk), "%.4f", v);
      write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, junk);
      v = interest_percentile(eng, 50.0, 2);
      snprintf(junk, sizeof(junk), "%.4f", v);
      write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, junk);
      Vtab -= TextSep;

      //
      // Median of Maximum Interest Values
      //
      write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "MMI (F+O):");
      v = interest_percentile(eng, 50.0, 3);
      snprintf(junk, sizeof(junk), "%.4f", v);
      write_centered_text(1, 1, (Htab_b+Htab_c)/2.0, Vtab, 0.0, 0.5, junk);
      Vtab -= TextSep;

   //
   //  done
   //

return;

} 

////////////////////////////////////////////////////////////////////////

void ModePsFile::do_page_1_other(ModeFuzzyEngine & eng, EngineType eng_type, const char * title)

{

char junk[1024];

      Vtab -= 12.0*TextSep;

      //
      // Field name
      //
      write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, FcstString);
      write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, ObsString);
      Vtab -= TextSep;

      /////////////////////////////////////////////////////////////////
      //
      // Object Counts and Areas
      //
      /////////////////////////////////////////////////////////////////

      //
      // Simple objects counts
      //
      write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "Simple:");
      snprintf(junk, sizeof(junk), "%i", eng.n_fcst);
      write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, junk);
      snprintf(junk, sizeof(junk), "%i", eng.n_obs);
      write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, junk);
      Vtab -= TextSep;

      //
      // Area counts
      //
      write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "Area:");
      snprintf(junk, sizeof(junk), "%i gs",
              eng.get_matched_fcst(1) + eng.get_unmatched_fcst(1));
      write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, junk);
      snprintf(junk, sizeof(junk), "%i gs",
              eng.get_matched_obs(1) + eng.get_unmatched_obs(1));
      write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, junk);
      Vtab -= TextSep;

      //
      // Cluster object counts
      //
      write_centered_text(1, 1, Htab_a, Vtab, 0.0, 0.5, "Cluster:");
      snprintf(junk, sizeof(junk), "%i", eng.collection.n_sets);
      write_centered_text(1, 1, Htab_b, Vtab, 0.0, 0.5, junk);
      snprintf(junk, sizeof(junk), "%i", eng.collection.n_sets);
      write_centered_text(1, 1, Htab_c, Vtab, 0.0, 0.5, junk);
      Vtab -= TextSep;

   //
   // done
   //

return;

}


////////////////////////////////////////////////////////////////////////



