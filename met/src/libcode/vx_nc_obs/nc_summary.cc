// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
#include "write_netcdf.h"

#include "nc_obs_util.h"
#include "nc_summary.h"

////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////

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

  snprintf(string_buffer, sizeof(string_buffer), "%02d%02d%02d", hour, minute, second);

  return string(string_buffer);
}

////////////////////////////////////////////////////////////////////////
// If raw_hdr_cnt is greater than 0, skip updating header index for obs.

void init_netcdf_output(NcFile *nc_file, NetcdfObsVars &obs_vars,
      NcObsOutputData &nc_out_data, string program_name)
{
   string method_name = "init_netcdf_output() ";
   SummaryObs *summary_obs = nc_out_data.summary_obs;
   bool do_summary = nc_out_data.summary_info.flag;
   
   //
   // Initialize the header and observation record counters
   //
   int obs_count = nc_out_data.observations.size();
   int hdr_count = (nc_out_data.processed_hdr_cnt > 0)
         ? nc_out_data.processed_hdr_cnt
         : summary_obs->countHeaders(nc_out_data.observations); // count and reset header index
   if (do_summary) {
      int summary_count = summary_obs->getSummaries().size();
      int summary_hdr_count = summary_obs->countSummaryHeaders();
      if (nc_out_data.summary_info.raw_data) {
         obs_count += summary_count;
         hdr_count += summary_hdr_count;
      }
      else {
         if (nc_out_data.processed_hdr_cnt > 0) {
            bool hdr_cnt = get_nc_hdr_cur_index();
            bool reset_array = true;
            reset_header_buffer(hdr_cnt, reset_array);
            mlog << Debug(5) << method_name << "reset headers (" << hdr_cnt << ") raw data.\n";
         }
         obs_count = summary_count;
         hdr_count = summary_hdr_count;
      }
   }
   mlog << Debug(7) << method_name << "obs_count: "
        << obs_count << " header count: " << hdr_count << "\n";
   obs_vars.obs_cnt = obs_count;
   
   create_nc_hdr_vars(obs_vars, nc_file, hdr_count, nc_out_data.deflate_level);
   create_nc_obs_vars(obs_vars, nc_file, nc_out_data.deflate_level,
         obs_vars.use_var_id);

   //
   // Add global attributes
   //
   write_netcdf_global(nc_file, nc_file->getName().c_str(), program_name.c_str());

   if (do_summary) write_summary_attributes(nc_file, nc_out_data.summary_info);
   
}

////////////////////////////////////////////////////////////////////////

bool write_observations(NcFile *nc_file, NetcdfObsVars &obs_vars,
      NcObsOutputData &nc_out_data)
{
  bool use_var_id = obs_vars.use_var_id;
  string method_name = "write_observations() ";
  bool do_summary = nc_out_data.summary_info.flag;
  bool do_save_raw_data = nc_out_data.summary_info.raw_data;
  bool do_header = (nc_out_data.processed_hdr_cnt == 0);

  mlog << Debug(5) << method_name << "do_header: " << (do_header ? "true" : "false")
       << ", do_summary: " << (do_summary ? "true" : "false")
       << ", save_raw_data: " << (do_save_raw_data ? "true" : "false")
       << "\n";
  
  if (!do_summary || (do_summary && do_save_raw_data)) {
    mlog << Debug(5) << method_name << "writing " 
         << (int)nc_out_data.observations.size() << " raw data...\n";
    write_nc_observations(obs_vars, nc_out_data.observations, use_var_id, do_header);
  }
  if (do_summary) {
    mlog << Debug(5) << method_name << "writing summary"
         << (do_save_raw_data ? " " : " (summary only)") << "...\n";
    bool tmp_do_header = true;
    write_nc_observations(obs_vars, nc_out_data.summary_obs->getSummaries(),
                          use_var_id, tmp_do_header);
  }
  
  int obs_buf_index = get_nc_obs_buf_index();
  if (obs_buf_index > 0) {
    write_nc_obs_buffer(obs_buf_index);
  }
  
  create_nc_table_vars (obs_vars, nc_file, nc_out_data.deflate_level);
  
  write_nc_arr_headers(obs_vars);
  
  //write_nc_table_vars(obs_vars);

  return true;
}

////////////////////////////////////////////////////////////////////////

void write_summary_attributes(NcFile *nc_file, TimeSummaryInfo summary_info) {
   add_att(nc_file, "time_summary_beg",
                    _secsToTimeString(summary_info.beg));
   add_att(nc_file, "time_summary_end",
                    _secsToTimeString(summary_info.end));

   char att_string[1024];

   snprintf(att_string, sizeof(att_string), "%d", summary_info.step);
   add_att(nc_file, "time_summary_step", att_string);

   snprintf(att_string, sizeof(att_string), "%d", summary_info.width_beg);
   add_att(nc_file, "time_summary_width_beg", att_string);

   snprintf(att_string, sizeof(att_string), "%d", summary_info.width_end);
   add_att(nc_file, "time_summary_width_end", att_string);

   string grib_code_string;
   for (int i = 0; i < summary_info.grib_code.n_elements(); ++i)
   {
     snprintf(att_string, sizeof(att_string), "%d", summary_info.grib_code[i]);
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

////////////////////////////////////////////////////////////////////////

