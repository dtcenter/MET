// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   nc_summary.cc
//
//   Description:
//      Common routines for time summary (into NetCDF).
//


#include <iostream>

#include <netcdf>

#include "write_netcdf.h"

#include "nc_obs_util.h"
#include "vx_summary.h"

using namespace std;
using namespace netCDF;


////////////////////////////////////////////////////////////////////////

string seconds_to_time_string(const int secs)
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

void write_summary_attributes(NcFile *nc_file, TimeSummaryInfo summary_info) {
   add_att(nc_file, "time_summary_beg",
                    seconds_to_time_string(summary_info.beg));
   add_att(nc_file, "time_summary_end",
                    seconds_to_time_string(summary_info.end));

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
