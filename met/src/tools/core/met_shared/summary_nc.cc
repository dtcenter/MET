// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   summary_util.cc
//
//   Description:
//      Common routines for time summary (into NetCDF).
//

using namespace std;

//#include <algorithm>
#include <iostream>

//#include "vx_math.h"
//#include "vx_nc_util.h"

#include "summary_nc.h"

string _secsToTimeString(const int secs)
{
  // Get the different fields from the number of seconds

  int remaining_secs = secs;
  int hour = remaining_secs / 3600;
  remaining_secs -= hour * 3600;
  int minute = remaining_secs / 60;
  remaining_secs -= minute * 60;
  int second = remaining_secs;

  // Create the string

  char string_buffer[20];

  sprintf(string_buffer, "%02d%02d%02d", hour, minute, second);

  return string(string_buffer);
}

//  void write_summary_hdr_data(bool do_summary, bool is_prepbufr, SummaryObs summary_obs) {
//     long dim_count, pb_hdr_count;
//     static const string method_name = "\nwrite_summary_hdr_data()";
//  
//     pb_hdr_count = (long) nc_data_buffer.cur_hdr_idx;
//     dim_count = pb_hdr_count;
//     if (doSummary) {
//        int summmary_hdr_cnt = summaryObs->countSummaryHeaders();
//        if (save_summary_only)
//           dim_count = summmary_hdr_cnt;
//        else
//           dim_count += summmary_hdr_cnt;
//     }
//  
//     // Check for no messages retained
//     if(dim_count <= 0) {
//        mlog << Error << method_name << " -> "
//             << "No PrepBufr messages retained.  Nothing to write.\n\n";
//  
//        // Delete the NetCDF file
//        if(remove(ncfile) != 0) {
//           mlog << Error << method_name << " -> "
//                << "can't remove output NetCDF file \"" << ncfile
//                << "\"\n\n";
//        }
//        exit(1);
//     }
//  
//     // Make sure all obs data is processed before handling header
//     if (doSummary) {
//        // Write out the summary data
//        //write_nc_observations(obs_vars, summaryObs->getSummaries(), false);
//        if (save_summary_only) reset_header_buffer(pb_hdr_count, true);
//        write_nc_observations(obs_vars, summaryObs->getSummaries(), save_summary_only);
//        mlog << Debug(4) << "write_summary_hdr_data obs count: "
//             << (int)summaryObs->getObservations().size()
//             << "  summary count: " << (int)summaryObs->getSummaries().size()
//             << " header count: " << dim_count
//             << " summary header count: " << (dim_count-pb_hdr_count) << "\n";
//     }
//  
//     int deflate_level = compress_level;
//     if (deflate_level < 0) deflate_level = conf_info.conf.nc_compression();
//     create_nc_hdr_vars(obs_vars, f_out, dim_count, deflate_level);
//     if (is_prepbufr) create_nc_pb_hdrs(obs_vars, f_out, pb_hdr_count, deflate_level);
//  
//     dim_count = bufr_obs_name_arr.n_elements();
//     create_nc_other_vars (obs_vars, f_out, nc_data_buffer, hdr_data,
//           dim_count, dim_count, deflate_level);
//  
//     // Write out the header data
//     if (header_to_vector) {
//        // Write out the saved header data at vector
//        write_nc_headers(obs_vars);
//     }
//     if (nc_data_buffer.hdr_data_idx > 0) {
//        // Write out the remaining header data
//        write_nc_header(obs_vars);
//     }
//  
//     int hdr_str_len;
//     long offsets[2] = { 0, 0 };
//     long lengths[2] = { 1, strl_len } ;
//     StringArray nc_var_name_arr;
//     StringArray nc_var_unit_arr;
//     StringArray nc_var_desc_arr;
//     map<ConcatString, ConcatString> obs_var_map = conf_info.getObsVarMap();
//     for(int i=0; i<bufr_obs_name_arr.n_elements(); i++) {
//        int var_index;
//        const char *var_name;
//        const char *unit_str;
//        const char *desc_str;
//  
//        unit_str = "";
//        var_name = bufr_obs_name_arr[i];
//        if (var_names.has(var_name, var_index)) {
//           unit_str = var_units[var_index];
//           if (0 == strcmp("DEG C", unit_str)) {
//              unit_str = "KELVIN";
//           }
//           else if (0 == strcmp("MB", unit_str)) {
//              unit_str = "PASCALS";
//           }
//           else if (0 == strcmp("MG/KG", unit_str)) {
//              unit_str = "KG/KG";
//           }
//        }
//        nc_var_unit_arr.add(unit_str);
//  
//        desc_str = (tableB_vars.has(var_name, var_index))
//              ? tableB_descs[var_index]
//              : "";
//        nc_var_desc_arr.add(desc_str);
//  
//        ConcatString grib_name = obs_var_map[var_name];
//        if (0 < grib_name.length()) {
//           var_name = grib_name.text();
//        }
//        nc_var_name_arr.add(var_name);
//     } // end for i
//     write_nc_string_array (&obs_vars.obs_var,  nc_var_name_arr, HEADER_STR_LEN);
//     write_nc_string_array (&obs_vars.unit_var, nc_var_unit_arr, HEADER_STR_LEN2);
//     write_nc_string_array (&obs_vars.desc_var, nc_var_desc_arr, HEADER_STR_LEN3);
//  
//     write_nc_other_vars(obs_vars);
//  
//     TimeSummaryInfo summaryInfo = conf_info.getSummaryInfo();
//     if (summaryInfo.flag) write_summary_attributes(f_out, summaryInfo);
//  //   {
//  //
//  //      add_att(f_out, "time_summary_beg", SummaryObs::secsToTimeString(summaryInfo.beg));
//  //      add_att(f_out, "time_summary_end", SummaryObs::secsToTimeString(summaryInfo.end));
//  //
//  //      char att_string[1024];
//  //
//  //      sprintf(att_string, "%d", summaryInfo.step);
//  //      add_att(f_out, "time_summary_step", att_string);
//  //
//  //      sprintf(att_string, "%d", summaryInfo.width_beg);
//  //      add_att(f_out, "time_summary_width_beg", att_string);
//  //
//  //      sprintf(att_string, "%d", summaryInfo.width_end);
//  //      add_att(f_out, "time_summary_width_end", att_string);
//  //
//  //      string grib_code_string;
//  //      for (int i = 0; i < summaryInfo.grib_code.n_elements(); ++i) {
//  //        sprintf(att_string, "%d", summaryInfo.grib_code[i]);
//  //        if (i == 0)
//  //          grib_code_string = string(att_string);
//  //        else
//  //          grib_code_string += string(" ") + att_string;
//  //      }
//  //      add_att(f_out, "time_summary_grib_code", grib_code_string.c_str());
//  //
//  //      string type_string;
//  //      for (int i = 0; i < summaryInfo.type.n_elements(); ++i) {
//  //        if (i == 0)
//  //          type_string = summaryInfo.type[i];
//  //        else
//  //          type_string += string(" ") + summaryInfo.type[i];
//  //      }
//  //      add_att(f_out, "time_summary_type", type_string.c_str());
//  //   }
//  
//     return;
//  }
//  
void write_summary_attributes(NcFile *nc_file, TimeSummaryInfo summary_info) {
   add_att(nc_file, "time_summary_beg",
                    _secsToTimeString(summary_info.beg));
   add_att(nc_file, "time_summary_end",
                    _secsToTimeString(summary_info.end));

   char att_string[1024];

   sprintf(att_string, "%d", summary_info.step);
   add_att(nc_file, "time_summary_step", att_string);

   sprintf(att_string, "%d", summary_info.width_beg);
   add_att(nc_file, "time_summary_width_beg", att_string);

   sprintf(att_string, "%d", summary_info.width_end);
   add_att(nc_file, "time_summary_width_end", att_string);

   string grib_code_string;
   for (int i = 0; i < summary_info.grib_code.n_elements(); ++i)
   {
     sprintf(att_string, "%d", summary_info.grib_code[i]);
     if (i == 0)
       grib_code_string = string(att_string);
     else
       grib_code_string += string(" ") + att_string;
   }
   add_att(nc_file, "time_summary_grib_code", grib_code_string.c_str());

   string type_string;
   for (int i = 0; i < summary_info.type.n_elements(); ++i)
   {
     if (i == 0)
       type_string = summary_info.type[i];
     else
       type_string += string(" ") + summary_info.type[i];
   }
   add_att(nc_file, "time_summary_type", type_string.c_str());
}
