// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_met_util/vx_met_util.h"

///////////////////////////////////////////////////////////////////////////////

static int (*two_to_one_grib)(Grid &, int, int);
extern int two_to_one_grib_0_0_0(Grid &, int, int);
extern int two_to_one_grib_0_0_1(Grid &, int, int);
extern int two_to_one_grib_0_1_0(Grid &, int, int);
extern int two_to_one_grib_0_1_1(Grid &, int, int);
extern int two_to_one_grib_1_0_0(Grid &, int, int);
extern int two_to_one_grib_1_0_1(Grid &, int, int);
extern int two_to_one_grib_1_1_0(Grid &, int, int);
extern int two_to_one_grib_1_1_1(Grid &, int, int);

///////////////////////////////////////////////////////////////////////////////

int has_grib_code(GribFile &grib_file, int gc) {
   int i;
   int found = 0;

   for(i=0; i<grib_file.n_records(); i++) {

      if(grib_file.gribcode(i) == gc) {
         found = 1;
         break;
      }
   }

   return(found);
}

///////////////////////////////////////////////////////////////////////////////

int get_grib_record(GribFile &grib_file, GribRecord &grib_record,
                    const GCInfo &gc_info,
                    WrfData &wd, Grid &gr, int &verbosity) {
   int i_rec, gc;

   // Store the GRIB code
   gc = gc_info.code;

   // Search for the GRIB record in the file
   i_rec = find_grib_record(grib_file, gc_info);

   // If the GRIB record is not found and can't be derived
   if(i_rec < 0 && gc != wdir_grib_code && gc != wind_grib_code) {
      return(-1);
   }

   // Derive the wind direction
   if(i_rec < 0 && gc == wdir_grib_code) {
      derive_wdir_record(grib_file, grib_record, wd, gr, gc_info, verbosity);
   }
   // Derive the wind speed
   else if(i_rec < 0 && gc == wind_grib_code) {
      derive_wind_record(grib_file, grib_record, wd, gr, gc_info, verbosity);
   }
   // Read the GRIB record found
   else {
      read_grib_record(grib_file, grib_record, i_rec, gc_info, wd, gr, verbosity);
   }

   return(0);
}

///////////////////////////////////////////////////////////////////////////////
//
// Attempt to locate the specified GRIB code at the specified level.  If found,
// return the index for the GRIB record.  If not found, return -1.
//
///////////////////////////////////////////////////////////////////////////////

int find_grib_record(GribFile &grib_file, const GCInfo &gc_info) {
   int i, j;
   int gc, l1, l2;
   GribRecord r;
   int bms_flag, accum;
   unixtime init_ut, valid_ut;

   //
   // Store the code and level info
   //
   gc = gc_info.code;
   l1 = gc_info.lvl_1;
   l2 = gc_info.lvl_2;

   //
   // Find the GRIB record containing the indicated GRIB code and
   // accumulation or level value
   //
   for(i=0; i<grib_file.n_records(); i++) {

      //
      // If the record matches the GRIB code, read it
      //
      if(grib_file.gribcode(i) == gc) {

         //
         // Check if the current record number matches the one requested
         //
         if(gc_info.lvl_type == RecNumber && l1 == (i+1)) break;

         //
         // Read the GRIB record's PDS
         //
         grib_file.seek_record(i);
         grib_file >> r;
         read_pds(r, bms_flag, init_ut, valid_ut, accum);

         //
         // If the TRI indicates accumulation, check whether or not the
         // accumulation equals n
         //
         if(gc_info.lvl_type == AccumLevel && r.pds->tri == 4 &&
            l1 == (accum/sec_per_hour)     && l2 == (accum/sec_per_hour)) break;

         //
         // Find the level value for this record
         //
         for(j=0; j<n_grib_level_list; j++) {
            if(r.pds->type == grib_level_list[j].level) {
               break;
            }
         }
         if(j == n_grib_level_list) {
            i = -1;
            break;
         }

         //
         // Check that the level type is consistent with the field for which
         // we're searching.
         //
              if(gc_info.lvl_type == PresLevel &&
                 grib_level_list[j].type != 3) continue;
         else if(gc_info.lvl_type == VertLevel &&
                 grib_level_list[j].type != 2) continue;

         //
         // Check if value stored in octets 11 and 12 matches lvl_1
         //
         if((grib_level_list[j].flag == 0 || grib_level_list[j].flag == 1) &&
            char2_to_int(r.pds->level_info) == l1 &&
            char2_to_int(r.pds->level_info) == l2) {
            break;
         }
         //
         // Check if a range of levels match
         //
         else if(grib_level_list[j].flag == 2 &&
                 ((int) r.pds->level_info[0]) == l1 &&
                 ((int) r.pds->level_info[1]) == l2) {
            break;
         }
      }
   } // end for i

   //
   // Grib record not found
   //
   if(i == grib_file.n_records()) i = -1;

   return(i);
}

///////////////////////////////////////////////////////////////////////////////
//
// Compute a list of GRIB records for this GRIB code which
// fall within the requested range of levels from the forecast
// and climatological data, including one level above and one
// level below the range.  For this routine, only GRIB records
// on a single level are considered.
//
///////////////////////////////////////////////////////////////////////////////

int find_grib_record_levels(GribFile &grib_file, const GCInfo &gc_info,
                            int *i_rec, int *i_lvl) {
   int i, j, n_rec;
   int gc, l1, l2;
   GCInfo gc_tmp;
   int rec_below, rec_above;
   int lvl_below, lvl_above;
   double dist_below, dist_above;
   GribRecord r;
   int bms_flag, accum;
   unixtime init_ut, valid_ut;

   //
   // Initialize
   //
   gc_tmp = gc_info;

   //
   // Store the code and level info
   //
   gc = gc_info.code;
   l1 = gc_info.lvl_1;
   l2 = gc_info.lvl_2;

   //
   // If WDIR or WIND is requested, check whether or not those are present
   // in the GRIB file.  If not, search instead for information on UGRD
   //
   if((gc == wdir_grib_code && !has_grib_code(grib_file, wdir_grib_code)) ||
      (gc == wind_grib_code && !has_grib_code(grib_file, wind_grib_code))) {
      gc          = ugrd_grib_code;
      gc_tmp.code = gc;
   }

   //
   // If l1 = l2, only need to find a single GRIB record at that level
   //
   if(l1 == l2) {
      i_rec[0] = find_grib_record(grib_file, gc_tmp);
      i_lvl[0] = l1;
      if(i_rec[0] < 0) n_rec = 0;
      else             n_rec = 1;
   }

   //
   // If l1 != l2, attempt to find a single GRIB record containing this range
   // of levels.
   //
   else if(l1 != l2 &&
           (i_rec[0] = find_grib_record(grib_file, gc_tmp)) >= 0) {

      //
      // Store the level as the midpoint
      //
      i_lvl[0] = nint((l1 + l2)/2.0);
      n_rec = 1;
   }

   //
   // If l1 != l2 and there's no single matching GRIB record, find all GRIB
   // records at levels within that range plus one level above and below.
   //
   else {

      //
      // Initialize
      //
      rec_below = rec_above = -1;
      lvl_below = lvl_above = -1;
      dist_below = dist_above = 1.0e30;
      n_rec = 0;
      for(i=0; i<grib_file.n_records(); i++) {

         //
         // If the record matches the GRIB code, read it
         //
         if(grib_file.gribcode(i) == gc) {

            //
            // Read the GRIB record's PDS
            //
            grib_file.seek_record(i);
            grib_file >> r;
            read_pds(r, bms_flag, init_ut, valid_ut, accum);

            //
            // Check that the level type is consistent with the field for
            // which we're searching.
            //

            //
            // Find the level value for this record
            //
            for(j=0; j<n_grib_level_list; j++) {
               if(r.pds->type == grib_level_list[j].level) break;
            }
            if(j == n_grib_level_list) break;

            //
            // Check that the level type is consistent with the field for which
            // we're searching.
            //
                 if(gc_tmp.lvl_type == PresLevel &&
                    grib_level_list[j].type != 3) continue;
            else if(gc_tmp.lvl_type == VertLevel &&
                    grib_level_list[j].type != 2) continue;

            //
            // Only consider records defined on a single level
            //
            if(grib_level_list[j].flag != 0 &&
               grib_level_list[j].flag != 1) continue;

            //
            // Check if the record falls within the requested
            // level range
            //
            if((char2_to_int(r.pds->level_info) > l1) &&
               (char2_to_int(r.pds->level_info) < l2))
            {
               // Retain this record
               i_rec[n_rec]  = i;
               i_lvl[n_rec] = char2_to_int(r.pds->level_info);
               n_rec++;
            }

            //
            // Look for one level below the range
            //
            if((char2_to_int(r.pds->level_info) <= l1) &&
               (l1 - char2_to_int(r.pds->level_info) < dist_below))
            {
               dist_below = l1 - char2_to_int(r.pds->level_info);
               rec_below  = i;
               lvl_below  = char2_to_int(r.pds->level_info);
            }

            //
            // Look for one level above the range
            //
            if((char2_to_int(r.pds->level_info) >= l2) &&
               (char2_to_int(r.pds->level_info - l2) < dist_above))
            {
               dist_above = char2_to_int(r.pds->level_info) - l2;
               rec_above  = i;
               lvl_above  = char2_to_int(r.pds->level_info);
            }
         } // end if
      } // end for i

      //
      // If records were found, add the records above and below the range
      //
      if(n_rec > 0) {
         if(rec_below != -1) {
            i_rec[n_rec] = rec_below;
            i_lvl[n_rec] = lvl_below;
            n_rec++;
         }
         if(rec_above != -1) {
            i_rec[n_rec] = rec_above;
            i_lvl[n_rec] = lvl_above;
            n_rec++;
         }
      }
   } // end else

   return(n_rec);
}

///////////////////////////////////////////////////////////////////////////////

void read_grib_record(const char *file_name, GribRecord &grib_record,
                      int i_rec, const GCInfo &gc_info, WrfData &wd, Grid &gr,
                      int verbosity) {
   GribFile grib_file;

   if( !(grib_file.open(file_name)) ) {
      cerr << "\n\nERROR: read_grib_record() -> "
           << "can't open GRIB file: "
           << file_name << "\n\n" << flush;
      exit(1);
   }

   read_grib_record(grib_file, grib_record, i_rec, gc_info, wd, gr, verbosity);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void read_grib_record(GribFile &grib_file, GribRecord &grib_record, int i_rec,
                      const GCInfo &gc_info, WrfData &wd, Grid &gr,
                      int verbosity) {

   WrfData wd2, rot_u_wd, rot_v_wd;
   GribRecord grib_record2;
   Grid gr2;
   int i_rec2;
   GCInfo gc_info2;

   // Read the specified record
   read_single_grib_record(grib_file, grib_record, i_rec, wd, gr, verbosity);

   // If the field is UGRD, VGRD, or WDIR, determine whether the winds need to be
   // rotated from grid relative to earth relative.
   if((grib_record.pds->grib_code == ugrd_grib_code ||
       grib_record.pds->grib_code == vgrd_grib_code ||
       grib_record.pds->grib_code == wdir_grib_code) &&
      is_grid_relative(grib_record)) {

      // If the grid type is a non-conformal Plate Carree, do not rotate
      if(gr.proj_type() == PlateCarreeProj) {
         cerr << "\n\nERROR: read_grib_record() -> "
              << "rotation of non-conformal Plate Carree projection "
              << "not yet implemented\n\n" << flush;
         exit(1);
      }

      // Handle the UGRD field
      if(grib_record.pds->grib_code == ugrd_grib_code) {

         // Set up the GCInfo object for vgrd
         gc_info2 = gc_info;
         gc_info2.code = vgrd_grib_code;

         // Find the corresponding VGRD record
         i_rec2 = find_grib_record(grib_file, gc_info2);

         // Check that the VGRD record was found
         if(i_rec2 < 0) {
            cerr << "\n\nERROR: read_grib_record() -> "
                 << "can't find matching VGRD record when trying to rotate "
                 << "the UGRD field to grid-relative coordinates\n\n" << flush;
            exit(1);
         }

         // Read the VGRD record
         read_single_grib_record(grib_file, grib_record2, i_rec2, wd2, gr2, verbosity);

         // Rotate U and V to be earth relative
         rotate_uv_grid_to_earth(gr, wd, wd2, rot_u_wd, rot_v_wd);

         // Store the rotated UGRD field
         wd = rot_u_wd;
      }
      // Handle the VGRD field
      else if(grib_record.pds->grib_code == vgrd_grib_code) {

         // Set up the GCInfo object for ugrd
         gc_info2 = gc_info;
         gc_info2.code = ugrd_grib_code;

         // Find the corresponding UGRD record
         i_rec2 = find_grib_record(grib_file, gc_info2);

         // Check that the UGRD record was found
         if(i_rec2 < 0) {
            cerr << "\n\nERROR: read_grib_record() -> "
                 << "can't find matching UGRD record when trying to rotate "
                 << "the VGRD field to grid-relative coordinates\n\n" << flush;
            exit(1);
         }

         // Read the UGRD record
         read_single_grib_record(grib_file, grib_record2, i_rec2, wd2, gr2, verbosity);

         // Rotate U and V to be earth relative
         rotate_uv_grid_to_earth(gr, wd2, wd, rot_u_wd, rot_v_wd);

         // Store the rotated VGRD field
         wd = rot_v_wd;
      }
      // Handle the WDIR field
      else { // grib_record.pds->grib_code == wdir_grib_code) {

         // Rotate WDIR to be earth relative
         rotate_wdir_grid_to_earth(gr, wd, wd2);

         // Store the rotated WDIR field
         wd = wd2;
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void read_single_grib_record(GribFile &grib_file, GribRecord &grib_record,
                             int i_rec, WrfData &wd, Grid &gr, int verbosity) {

   int bms_flag = 0;
   int i, x, y, count, accum;
   int x_dir, y_dir, order_flag;
   float v, v_min, v_max;
   unixtime init_ut, valid_ut;

   //
   // Read the record indicated
   //
   grib_file.seek_record(i_rec);
   grib_file >> grib_record;

   if(verbosity > 3) {
      cout << "GRIB RECORD:\n" << grib_record << "\n" << flush;
   }

   //
   // Read the Product Description Section
   //
   read_pds(grib_record, bms_flag, init_ut, valid_ut, accum);

   if(verbosity > 2) {
      cout << "Grib Record Index = " << i_rec+1 << "\n"
           << "Initialization time = " << init_ut << "\n"
           << "Valid time = " << valid_ut << "\n"
           << "Accumulation time = " << accum << "\n" << flush;
   }

   //
   // Read the Grid Description Section
   //
   read_gds(grib_record, gr, x_dir, y_dir, order_flag);

   //
   // Point the two_to_one_grib function pointer to the proper function
   // based on the values of x_dir, y_dir, and order_flag
   //
   if(     !x_dir && !y_dir && !order_flag) two_to_one_grib = two_to_one_grib_0_0_0;
   else if(!x_dir && !y_dir &&  order_flag) two_to_one_grib = two_to_one_grib_0_0_1;
   else if(!x_dir &&  y_dir && !order_flag) two_to_one_grib = two_to_one_grib_0_1_0;
   else if(!x_dir &&  y_dir &&  order_flag) two_to_one_grib = two_to_one_grib_0_1_1;
   else if( x_dir && !y_dir && !order_flag) two_to_one_grib = two_to_one_grib_1_0_0;
   else if( x_dir && !y_dir &&  order_flag) two_to_one_grib = two_to_one_grib_1_0_1;
   else if( x_dir &&  y_dir && !order_flag) two_to_one_grib = two_to_one_grib_1_1_0;
   else if( x_dir &&  y_dir &&  order_flag) two_to_one_grib = two_to_one_grib_1_1_1;

   //
   // Read through the data to find the min/max values
   //
   v_min = 1.0e30;
   v_max = -1.0e30;
   count = 0;
   for(y=0; y<gr.ny(); y++) {
      for(x=0; x<gr.nx(); x++) {

         i = two_to_one_grib(gr, x, y);

         //
         // Check bitmap for non-zero data at this point
         //
         if(bms_flag && grib_record.bms_bit(i) > 0) {
            v = (float) grib_record.data_value(count);
            count++;
         }
         else if(bms_flag && grib_record.bms_bit(i) <= 0) {
            v = bad_data_float;
         }
         else { // !bms_flag
            v = (float) grib_record.data_value(i);
         }

         if(!is_bad_data(v) && v > v_max) v_max = v;
         if(!is_bad_data(v) && v < v_min) v_min = v;
      }
   }

   if(verbosity > 2) {
      cout << "Grib Record (min, max) = ("
           << v_min << ", " << v_max << ")\n" << flush;
   }

   //
   // Set up the wrfdata object
   //
   wd.set_valid_time(valid_ut);
   wd.set_lead_time( (int) (valid_ut - init_ut));
   wd.set_accum_time(accum);
   wd.set_size(gr.nx(), gr.ny());
   wd.set_b(v_min);
   wd.set_m( (double) (v_max-v_min)/wrfdata_int_data_max);

   //
   // Parse the precip data into the wrfdata object
   //
   count = 0;
   for(y=0; y<gr.ny(); y++) {
      for(x=0; x<gr.nx(); x++) {

         i = two_to_one_grib(gr, x, y);

         //
         // Check bitmap for non-zero data at this point
         //
         if(bms_flag && grib_record.bms_bit(i) > 0) {
            v = (float) grib_record.data_value(count);
            count++;
         }
         else if(bms_flag && grib_record.bms_bit(i) <= 0) {
            v = bad_data_float;
         }
         else { // !bms_flag
            v = (float) grib_record.data_value(i);
         }

         wd.put_xy_int(wd.double_to_int(v), x, y);
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void derive_wdir_record(GribFile &grib_file, GribRecord &grib_record,
                        WrfData &wd, Grid &gr, const GCInfo &gc_info,
                        int verbosity) {
   int u_rec, v_rec, x, y, nx, ny, n;
   int l1, l2;
   WrfData u_wd, v_wd;
   double u, v, wdir, wdir_min, wdir_max;
   double *data = (double *) 0;
   GCInfo ugrd_info, vgrd_info;

   //
   // Store the GRIB code info
   //
   l1 = gc_info.lvl_1;
   l2 = gc_info.lvl_2;
   ugrd_info = gc_info;
   ugrd_info.code = ugrd_grib_code;
   vgrd_info = gc_info;
   vgrd_info.code = vgrd_grib_code;

   //
   // Find the GRIB record for the U and V components of the wind
   //
   u_rec = find_grib_record(grib_file, ugrd_info);
   v_rec = find_grib_record(grib_file, vgrd_info);

   if(u_rec < 0 || v_rec < 0) {
      cerr << "\n\nERROR: derive_wdir_record() -> "
           << "can't find the UGRD and/or VGRD record for level values "
           << l1 << " and " << l2 << " when trying to derive the wind "
           << "direction field\n\n" << flush;
      exit(1);
   }

   //
   // Read the U and V components of the wind
   //
   read_grib_record(grib_file, grib_record, u_rec, gc_info, u_wd, gr, verbosity);

   read_grib_record(grib_file, grib_record, v_rec, gc_info, v_wd, gr, verbosity);

   //
   // Check that the dimensions match
   //
   if(u_wd.get_nx() != v_wd.get_nx() ||
      u_wd.get_ny() != v_wd.get_ny()) {
      cerr << "\n\nERROR: derive_wdir_record() -> "
           << "the grid dimensions for the U and V components don't match"
           << ".\n\n" << flush;
      exit(1);
   }

   //
   // Allocate space to temporarily store the WDIR values
   //
   nx   = gr.nx();
   ny   = gr.ny();
   data = new double [nx*ny];

   //
   // Compute the min and max WDIR values
   //
   wdir_min = 1.0e30;
   wdir_max = -1.0e30;

   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         n = two_to_one(nx, ny, x, y);

         // Check for bad data
         if(u_wd.is_bad_xy(x, y) || v_wd.is_bad_xy(x, y)) {
            data[n] = bad_data_double;
            continue;
         }

         // Get the U and V components for this grid point
         u = u_wd.get_xy_double(x, y);
         v = v_wd.get_xy_double(x, y);

         //
         // Compute the wind direction
         //
         wdir = rescale_deg(atan2d(-1.0*u, -1.0*v), 0.0, 360.0);

         data[n] = wdir;

         if(wdir > wdir_max) wdir_max = wdir;
         if(wdir < wdir_min) wdir_min = wdir;
      }
   }

   //
   // Setup the WDIR field
   //
   wd = u_wd;
   wd.set_b(wdir_min);
   wd.set_m( (double) (wdir_max-wdir_min)/wrfdata_int_data_max);

   //
   // Store the WDIR field
   //
   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         n = two_to_one(nx, ny, x, y);

         wd.put_xy_double(data[n], x, y);
      } // end for y
   } // end for x

   if(data) { delete data; data = (double *) 0; }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void derive_wind_record(GribFile &grib_file, GribRecord &grib_record,
                        WrfData &wd, Grid &gr, const GCInfo &gc_info,
                        int verbosity) {
   int u_rec, v_rec, x, y, nx, ny, n, l1, l2;
   WrfData u_wd, v_wd;
   double u, v, wind, wind_min, wind_max;
   double *data = (double *) 0;
   GCInfo ugrd_info, vgrd_info;

   //
   // Store the GRIB code info
   //
   l1 = gc_info.lvl_1;
   l2 = gc_info.lvl_2;
   ugrd_info = gc_info;
   ugrd_info.code = ugrd_grib_code;
   vgrd_info = gc_info;
   vgrd_info.code = vgrd_grib_code;

   //
   // Find the GRIB record for the U and V components of the wind
   //
   u_rec = find_grib_record(grib_file, ugrd_info);
   v_rec = find_grib_record(grib_file, vgrd_info);

   if(u_rec < 0 || v_rec < 0) {
      cerr << "\n\nERROR: derive_wind_record() -> "
           << "can't find the UGRD and/or VGRD record for level values "
           << l1 << " and " << l2 << " when trying to derive the wind speed "
           << "field\n\n" << flush;
      exit(1);
   }

   //
   // Read the U and V components of the wind
   //
   read_grib_record(grib_file, grib_record, u_rec, ugrd_info, u_wd, gr, verbosity);

   read_grib_record(grib_file, grib_record, v_rec, vgrd_info, v_wd, gr, verbosity);

   //
   // Check that the dimensions match
   //
   if(u_wd.get_nx() != v_wd.get_nx() ||
      u_wd.get_ny() != v_wd.get_ny()) {
      cerr << "\n\nERROR: derive_wind_record() -> "
           << "the grid dimensions for the U and V components don't match"
           << ".\n\n" << flush;
      exit(1);
   }

   //
   // Allocate space to temporarily store the WIND values
   //
   nx   = gr.nx();
   ny   = gr.ny();
   data = new double [nx*ny];

   //
   // Compute the min and max WIND values
   //
   wind_min = 1.0e30;
   wind_max = -1.0e30;

   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         n = two_to_one(nx, ny, x, y);

         // Check for bad data
         if(u_wd.is_bad_xy(x, y) || v_wd.is_bad_xy(x, y)) {
            data[n] = bad_data_double;
            continue;
         }

         // Get the U and V components for this grid point
         u = u_wd.get_xy_double(x, y);
         v = v_wd.get_xy_double(x, y);

         //
         // Compute the wind speed
         //
         wind = sqrt(u*u + v*v);

         data[n] = wind;

         if(wind > wind_max) wind_max = wind;
         if(wind < wind_min) wind_min = wind;
      }
   }

   //
   // Setup the WIND field
   //
   wd = u_wd;
   wd.set_b(wind_min);
   wd.set_m( (double) (wind_max-wind_min)/wrfdata_int_data_max);

   //
   // Store the WIND field
   //
   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         n = two_to_one(nx, ny, x, y);

         wd.put_xy_double(data[n], x, y);
      } // end for y
   } // end for x

   if(data) { delete data; data = (double *) 0; }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void read_pds(GribRecord &r, int &bms_flag,
              unixtime &init_ut, unixtime &valid_ut, int &accum) {
   double sec_per_fcst_unit;
   unsigned char pp1[2];

   //
   // Check PDS for flag for the presence of a GDS and BMS section
   //
   if(!(r.pds->flag & 128)) {
      cerr << "\n\nERROR: read_pds() -> "
           << "No Grid Description Section present in the "
           << "grib record.\n\n" << flush;
      exit(1);
   }
   if(r.pds->flag & 64) bms_flag = true;

   //
   // Check PDS for the initialization time
   //
   init_ut = mdyhms_to_unix(r.pds->month, r.pds->day,
                            r.pds->year + (r.pds->century - 1)*100,
                            r.pds->hour, r.pds->minute, 0);

   //
   // Check PDS for time units
   //
   switch((int) r.pds->fcst_unit) {

      case 0: // minute
         sec_per_fcst_unit = sec_per_minute;
         break;

      case 1: // hour
         sec_per_fcst_unit = sec_per_hour;
         break;

      case 2: // day
         sec_per_fcst_unit = sec_per_day;
         break;

      case 3: // month
         sec_per_fcst_unit = sec_per_day*30.0;
         break;

      case 4: // year
         sec_per_fcst_unit = sec_per_day*365.0;
         break;

      case 5: // decade
         sec_per_fcst_unit = sec_per_day*365.0*10.0;
         break;

      case 6: // normal (30 years)
         sec_per_fcst_unit = sec_per_day*365.0*30.0;
         break;

      case 7: // century
         sec_per_fcst_unit = sec_per_day*365.0*100.0;
         break;

      case 13: // 15 minutes
         sec_per_fcst_unit = sec_per_minute*15.0;
         break;

      case 14: // 30 minutes
         sec_per_fcst_unit = sec_per_minute*30.0;
         break;

      case 254: // second
         sec_per_fcst_unit = 1.0;
         break;

      default:
         cerr << "\n\nERROR: read_pds() -> "
              << "unexpected time unit of "
              << (int) r.pds->fcst_unit << ".\n\n" << flush;
         exit(1);
         break;
   }

   //
   // Set the valid and accumulation times based on the
   // contents of the time range indicator
   //
   switch((int) r.pds->tri) {

      case 0: // Valid time = init + p1
         valid_ut = (unixtime) (init_ut + r.pds->p1*sec_per_fcst_unit);
         accum = 0;
         break;

      case 1: // Valid time = init
         valid_ut = init_ut;
         accum = 0;
         break;

      case 2: // Valid time between init + p1 and init + p2
         valid_ut = (unixtime) (init_ut + r.pds->p2*sec_per_fcst_unit);
         accum = 0;
         break;

      case 3: // Average
         valid_ut = (unixtime) (init_ut + r.pds->p2*sec_per_fcst_unit);
         accum = 0;
         break;

      case 4: // Accumulation
         valid_ut = (unixtime) (init_ut + r.pds->p2*sec_per_fcst_unit);
         accum = nint((r.pds->p2 - r.pds->p1)*sec_per_fcst_unit);
         break;

      case 5: // Difference: product valid at init + p2
         valid_ut = (unixtime) (init_ut + r.pds->p2*sec_per_fcst_unit);
         accum = 0;
         break;

      case 10: // P1 occupies octets 19 and 20: product valid at init + p1
          pp1[0] = r.pds->p1;
          pp1[1] = r.pds->p2;
          valid_ut = (unixtime) (init_ut + char2_to_int(pp1)*sec_per_fcst_unit);
          accum = 0;
          break;

      default:
         cerr << "\n\nERROR: read_pds() -> "
              << "unexpected time range indicator of "
              << (int) r.pds->tri << ".\n\n" << flush;
         exit(1);
         break;
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void read_gds(GribRecord &r, Grid &gr, int &xdir, int &ydir, int &order) {
   unsigned char c;
   double d;
   int parity;

   // Structures to store projection info
   LambertData       lc_data;
   StereographicData st_data;
   PlateCarreeData   pc_data;
   MercatorData      mc_data;

   //
   // Check GDS for the grid type.
   // The following 4 Projection types are supported:
   //    - Lat/Lon
   //    - Mercator
   //    - Lambert Conformal
   //    - Polar Stereographic
   //

   //
   // Latitude/Longitude Projections Grid
   // Also called Equidistant Cylindrical or Plate Carree Projection Grid
   //
   if(r.gds->type == 0) {

      // Store the grid name
      pc_data.name = proj_type[0];

      // Check the scan flag for the x and y scanning direction and order
      if(r.gds->grid_type.latlon_grid.scan_flag & 128) xdir  = 1; // In the -x direction
      else                                             xdir  = 0; // In the +x direction
      if(r.gds->grid_type.latlon_grid.scan_flag & 64)  ydir  = 1; // In the +y direction
      else                                             ydir  = 0; // In the -y direction
      if(r.gds->grid_type.latlon_grid.scan_flag & 32)  order = 1; // Data in (y, x) order
      else                                             order = 0; // Data in (x, y) order

      //
      // Multiply longitude values by -1 since the NCAR code considers
      // degrees west to be positive.
      //

      // Latitude of the bottom left corner
      if(ydir == 1) {
         pc_data.lat_ll_deg = decode_lat_lon(r.gds->grid_type.latlon_grid.lat1, 3);
      }
      else {
         pc_data.lat_ll_deg = decode_lat_lon(r.gds->grid_type.latlon_grid.lat2, 3);
      }

      // Longitude of the bottom left corner
      if(xdir == 0) {
         pc_data.lon_ll_deg = -1.0*rescale_lon(decode_lat_lon(r.gds->grid_type.latlon_grid.lon1, 3));
      }
      else {
         pc_data.lon_ll_deg = -1.0*rescale_lon(decode_lat_lon(r.gds->grid_type.latlon_grid.lon2, 3));
      }

      // Number of points in the Latitudinal (y) direction
      pc_data.Nlat = char2_to_int(r.gds->ny);

      // Number of points in the Longitudinal (x) direction
      pc_data.Nlon = char2_to_int(r.gds->nx);

      // Latitudinal increment.  If not given, compute from lat1 and lat2
      if(all_bits_set(r.gds->grid_type.latlon_grid.dj, 2)) {

         d = abs( (long double) decode_lat_lon(r.gds->grid_type.latlon_grid.lat1, 3)
                - decode_lat_lon(r.gds->grid_type.latlon_grid.lat2, 3));
         pc_data.delta_lat_deg = d/pc_data.Nlat;
      }
      else {

         pc_data.delta_lat_deg = decode_lat_lon(r.gds->grid_type.latlon_grid.dj, 2);
      }

      // Longitudinal increment.  If not given, compute from lon1 and lon2
      if(all_bits_set(r.gds->grid_type.latlon_grid.di, 2)) {

         d = abs( (long double) decode_lat_lon(r.gds->grid_type.latlon_grid.lon1, 3)
                - decode_lat_lon(r.gds->grid_type.latlon_grid.lon2, 3));
         pc_data.delta_lon_deg = d/pc_data.Nlon;
      }
      else {
         pc_data.delta_lon_deg = decode_lat_lon(r.gds->grid_type.latlon_grid.di, 2);
      }

      Grid pc_grid(pc_data);
      gr = pc_grid;
   }

   //
   // Mercator Projection Grid
   //
   else if(r.gds->type == 1) {

      // Store the grid name
      mc_data.name = proj_type[1];

      // Check the scan flag for the x and y scanning direction and order
      if(r.gds->grid_type.mercator.scan_flag & 128) xdir  = 1; // In the -x direction
      else                                          xdir  = 0; // In the +x direction
      if(r.gds->grid_type.mercator.scan_flag & 64)  ydir  = 1; // In the +y direction
      else                                          ydir  = 0; // In the -y direction
      if(r.gds->grid_type.mercator.scan_flag & 32)  order = 1; // Data in (y, x) order
      else                                          order = 0; // Data in (x, y) order

      //
      // Multiply longitude values by -1 since the NCAR code considers
      // degrees west to be positive.
      //

      // Latitude of the bottom left and upper right corners
      if(ydir == 1) {
         mc_data.lat_ll_deg = decode_lat_lon(r.gds->grid_type.mercator.lat1, 3);
         mc_data.lat_ur_deg = decode_lat_lon(r.gds->grid_type.mercator.lat2, 3);
      }
      else {
         mc_data.lat_ll_deg = decode_lat_lon(r.gds->grid_type.mercator.lat2, 3);
         mc_data.lat_ur_deg = decode_lat_lon(r.gds->grid_type.mercator.lat1, 3);
      }

      // Longitude of the bottom left and upper right corners
      if(xdir == 0) {
         mc_data.lon_ll_deg = -1.0*rescale_lon(decode_lat_lon(r.gds->grid_type.mercator.lon1, 3));
         mc_data.lon_ur_deg = -1.0*rescale_lon(decode_lat_lon(r.gds->grid_type.mercator.lon2, 3));
      }
      else {
         mc_data.lon_ll_deg = -1.0*rescale_lon(decode_lat_lon(r.gds->grid_type.mercator.lon2, 3));
         mc_data.lon_ur_deg = -1.0*rescale_lon(decode_lat_lon(r.gds->grid_type.mercator.lon1, 3));
      }

      // Number of points in the Latitudinal (y) direction
      mc_data.ny = char2_to_int(r.gds->ny);

      // Number of points in the Longitudinal (x) direction
      mc_data.nx = char2_to_int(r.gds->nx);

      Grid pc_grid(mc_data);
      gr = pc_grid;
   }

   //
   // Lambert Conformal Projection Grid
   //
   else if(r.gds->type == 3) {

      // Store the grid name
      lc_data.name = proj_type[2];

      //
      // Multiply longitude values by -1 since the NCAR code considers
      // degrees west to be positive.
      //

      // First latitude
      lc_data.p1_deg = decode_lat_lon(r.gds->grid_type.lambert_conf.latin1, 3);

      // Second latitude
      lc_data.p2_deg = decode_lat_lon(r.gds->grid_type.lambert_conf.latin2, 3);

      // Latitude of first point
      lc_data.p0_deg = decode_lat_lon(r.gds->grid_type.lambert_conf.lat1, 3);

      // Longitude of first point
      lc_data.l0_deg = -1.0*rescale_lon(decode_lat_lon(r.gds->grid_type.lambert_conf.lon1, 3));

      // Center longitude
      lc_data.lcen_deg = -1.0*rescale_lon(decode_lat_lon(r.gds->grid_type.lambert_conf.lov, 3));

      // Grid spacing in km (given in the GRIB file in m)
      lc_data.d_km = (double) char3_to_int(r.gds->grid_type.lambert_conf.dx)/1000.0;

      // Radius of the earth
      lc_data.r_km = grib_earth_radius_km;

      // Number of points in the x-direction
      lc_data.nx = char2_to_int(r.gds->nx);

      // Number of points in the y-direction
      lc_data.ny = char2_to_int(r.gds->ny);

      // Check the scan flag for the x and y scanning direction and order
      if(r.gds->grid_type.lambert_conf.scan_flag & 128) xdir = 1; // In the -x direction
      else                                              xdir = 0; // In the +x direction
      if(r.gds->grid_type.lambert_conf.scan_flag & 64)  ydir = 1; // In the +y direction
      else                                              ydir = 0; // In the -y direction
      if(r.gds->grid_type.lambert_conf.scan_flag & 32) order = 1; // Data in (y, x) order
      else                                             order = 0; // Data in (x, y) order

      Grid lc_grid(lc_data);
      gr = lc_grid;
   }
   //
   // Polar Stereographic Projection Grid
   //
   else if(r.gds->type == 5) {

      // Store the grid name
      st_data.name = proj_type[3];

      //
      // Multiply longitude values by -1 since the NCAR code considers
      // degrees west to be positive.
      //

      // Latitude where the scale factor is deteremined is 60.0 degrees
      // based on WMO's Guide to Grib
      c = r.gds->grid_type.stereographic.pc_flag;
      if(c & 128) parity = -1; // South Pole
      else        parity = 1;  // North Pole
      st_data.p1_deg = (double) 60.0*parity;

      // Latitude of first point
      st_data.p0_deg = decode_lat_lon(r.gds->grid_type.stereographic.lat1, 3);

      // Longitude of first point
      st_data.l0_deg = -1.0*rescale_lon(decode_lat_lon(r.gds->grid_type.stereographic.lon1, 3));

      // Center longitude
      st_data.lcen_deg = -1.0*rescale_lon(decode_lat_lon(r.gds->grid_type.stereographic.lov, 3));

      // Grid spacing in km (given in the GRIB file in m)
      st_data.d_km = (double) char3_to_int(r.gds->grid_type.stereographic.dx)/1000.0;

      // Radius of the earth
      st_data.r_km = grib_earth_radius_km;

      // Number of points in the x-direction
      st_data.nx = char2_to_int(r.gds->nx);

      // Number of points in the y-direction
      st_data.ny = char2_to_int(r.gds->ny);

      // Check the scan flag for the x and y scanning direction and order
      if(r.gds->grid_type.stereographic.scan_flag & 128) xdir = 1; // In the -x direction
      else                                               xdir = 0; // In the +x direction
      if(r.gds->grid_type.stereographic.scan_flag & 64)  ydir = 1; // In the +y direction
      else                                               ydir = 0; // In the -y direction
      if(r.gds->grid_type.stereographic.scan_flag & 32) order = 1; // Data in (x, y) order
      else                                              order = 0; // Data in (y, x) order

      Grid st_grid(st_data);
      gr = st_grid;
   }
   //
   // Unsupported grid type
   //
   else {
      cerr << "\n\nERROR: read_gds() -> "
           << "Grid type " << (int) r.gds->type
           << " not currently supported.\n\n"
           << flush;
      exit(1);
   }

   return;
}


///////////////////////////////////////////////////////////////////////////////

double decode_lat_lon(unsigned char *p, int n) {
   int i, parity;
   double answer;
   unsigned char c[3];

   //
   // For all of the lat/lon parameters, the leftmost bit indicates the
   // parity of the value.  If the leftmost bit is on, the value is
   // negative.
   //

   for(i=0; i<n; i++) c[i] = p[i];
   if(c[0] & 128) parity = -1;
   else           parity = 1;
   c[0] &= 127;

   if(n == 2)      answer = (double) char2_to_int(c)/1000.0*parity;
   else if(n == 3) answer = (double) char3_to_int(c)/1000.0*parity;
   else            answer = (double) bad_data_float;

   return(answer);
}

///////////////////////////////////////////////////////////////////////////////

int all_bits_set(unsigned char *p, int n) {
   int status, i;
   unsigned char c;

   //
   // Check whether or not all the bits in the unsigned char are set to 1
   //

   status = 1;

   for(i=0; i<n; i++) {

      c = p[i];
      if( !(c & 128) || !(c & 64) || !(c & 32) || !(c & 16) ||
          !(c & 8)   || !(c & 4)  || !(c & 2)  || !(c & 1) ) status = 0;
   }

   return(status);
}

///////////////////////////////////////////////////////////////////////////////

void get_level_info(const GribRecord &r, int &l1, int &l2) {
   int i;

   //
   // Extract the level values from the GRIB record
   //
   for(i=0; i<n_grib_level_list; i++) {
      if(r.pds->type == grib_level_list[i].level) break;
   }
   if(i == n_grib_level_list) {
      cerr << "\n\nERROR: get_level_info() -> "
           << "Can't find matching GRIB level list entry for pds->type = "
           << r.pds->type << "\n\n" << flush;
      exit(1);
   }

   // Store the level information based on the grib_level_flag value
   if(grib_level_list[i].flag == 0 ||
      grib_level_list[i].flag == 1) {
      l1 = l2 = char2_to_int(r.pds->level_info);
   }
   else if(grib_level_list[i].flag == 2) {
      l1 = (int) r.pds->level_info[1];
      l2 = (int) r.pds->level_info[0];
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
// Check whether or not the res_flag indicates that the vectors are defined
// grid relative rather than earth relative.
//
//////////////////////////////////////////////////////////////////////////////

int is_grid_relative(const GribRecord &r) {
   unsigned char res_flag;

   // LatLonGrid
   if(r.gds->type == 0) {
      res_flag = r.gds->grid_type.latlon_grid.res_flag;
   }
   // Mercator 
   else if(r.gds->type == 1) {
      res_flag = r.gds->grid_type.mercator.res_flag;
   }
   // LambertConf
   else if(r.gds->type == 3) {
      res_flag = r.gds->grid_type.lambert_conf.res_flag;
   }
   // Stereographic
   else if(r.gds->type == 5) {
      res_flag = r.gds->grid_type.stereographic.res_flag;
   }
   else {
      cerr << "\n\nERROR: is_grid_relative() -> "
           << "Unsupported grid type value: " << r.gds->type
           << "\n\n" << flush;
      exit(1);
   }

   //
   // Return whether the 5th bit of the res_flag (Octet 17) is on, which
   // indicates that U and V are defined relative to the grid
   //
   return(get_bit_from_octet(res_flag, 5));
}

//////////////////////////////////////////////////////////////////////////////

int get_bit_from_octet(unsigned char u, int bit) {

   //
   // Bit numbers start at 1, not 0, and
   // the most-significant bit is number 1
   //

   if((bit < 1) || (bit > 8)) {

      cerr << "\n\nERROR: get_bit_from_octet() -> "
           << "bad bit number\n\n" << flush;
      exit(1);
   }

   unsigned char mask = (unsigned char) (1 << (8 - bit));

   if(u & mask) return(1);

   return(0);
}

///////////////////////////////////////////////////////////////////////////////

void rotate_uv_grid_to_earth(const Grid &gr,
                             const WrfData &u_wd, const WrfData &v_wd,
                             WrfData &u_rot_wd, WrfData &v_rot_wd) {
   int x, y, nx, ny, n;
   double u, u_rot, u_rot_min, u_rot_max;
   double v, v_rot, v_rot_min, v_rot_max;
   double alpha;
   double *u_rot_data = (double *) 0;
   double *v_rot_data = (double *) 0;

   //
   // Check that the input WrfData objects have dimensions which match the grid
   //
   if(u_wd.get_nx() != gr.nx() || v_wd.get_nx() != gr.nx() ||
      u_wd.get_ny() != gr.ny() || v_wd.get_ny() != gr.ny()) {

      cerr << "\n\nERROR: rotate_uv_grid_to_earth() -> "
           << "input grid dimensions don't match the grid\n\n" << flush;
      exit(1);
   }

   //
   // Allocate space to temporarily store the rotated U and V values
   //
   nx         = gr.nx();
   ny         = gr.ny();
   u_rot_data = new double [nx*ny];
   v_rot_data = new double [nx*ny];

   //
   // Compute the min and max rotated U and V values
   //
   u_rot_min = v_rot_min = 1.0e30;
   u_rot_max = v_rot_max = -1.0e30;

   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         n = two_to_one(nx, ny, x, y);

         // Check for bad data
         if(u_wd.is_bad_xy(x, y) || v_wd.is_bad_xy(x, y)) {
            u_rot_data[n] = bad_data_double;
            v_rot_data[n] = bad_data_double;
            continue;
         }

         // Get the U and V components for this grid point
         u = u_wd.get_xy_double(x, y);
         v = v_wd.get_xy_double(x, y);

         // Get the rotation angle from grid to earth relative
         alpha = gr.rot_grid_to_earth(x, y);

         // Rotate U component
         u_rot = cosd(alpha)*u + sind(alpha)*v;

         // Rotate V component
         v_rot = -1.0*sind(alpha)*u + cosd(alpha)*v;

         // Store the rotated components
         u_rot_data[n] = u_rot;
         v_rot_data[n] = v_rot;

         if(u_rot > u_rot_max) u_rot_max = u_rot;
         if(u_rot < u_rot_min) u_rot_min = u_rot;

         if(v_rot > v_rot_max) v_rot_max = v_rot;
         if(v_rot < v_rot_min) v_rot_min = v_rot;
      } // end for y
   } // end for x

   //
   // Setup the rotated U and V fields
   //
   u_rot_wd = u_wd;
   v_rot_wd = v_wd;
   u_rot_wd.set_b(u_rot_min);
   v_rot_wd.set_b(v_rot_min);
   u_rot_wd.set_m( (double) (u_rot_max-u_rot_min)/wrfdata_int_data_max);
   v_rot_wd.set_m( (double) (v_rot_max-v_rot_min)/wrfdata_int_data_max);

   //
   // Store the rotated U and V component fields
   //
   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         n = two_to_one(nx, ny, x, y);

         u_rot_wd.put_xy_double(u_rot_data[n], x, y);
         v_rot_wd.put_xy_double(v_rot_data[n], x, y);
      } // end for y
   } // end for x

   if(u_rot_data) { delete u_rot_data; u_rot_data = (double *) 0; }
   if(v_rot_data) { delete v_rot_data; v_rot_data = (double *) 0; }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void rotate_wdir_grid_to_earth(const Grid &gr,
                               const WrfData &wdir_wd, WrfData &wdir_rot_wd) {
   int x, y, nx, ny, n;
   double wdir, wdir_rot, wdir_rot_min, wdir_rot_max, alpha;
   double *data = (double *) 0;

   //
   // Initialize the object to store the rotated winds
   //
   wdir_rot_wd = wdir_wd;

   //
   // Allocate space to temporarily store the rotated WDIR values
   //
   nx   = gr.nx();
   ny   = gr.ny();
   data = new double [nx*ny];

   //
   // Compute the min and max rotated WDIR values
   //
   wdir_rot_min = 1.0e30;
   wdir_rot_max = -1.0e30;

   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         n = two_to_one(nx, ny, x, y);

         // Check for bad data
         if(wdir_wd.is_bad_xy(x, y)) {
            data[n] = bad_data_double;
            continue;
         }

         // Get the WDIR value for this grid point
         wdir = wdir_wd.get_xy_double(x, y);

         // Get the rotation angle from grid to earth relative
         alpha = gr.rot_grid_to_earth(x, y);

         // Compute rotated WDIR value
         wdir_rot = rescale_deg(wdir + alpha, 0.0, 360.0);

         // Store the rotated WDIR value
         data[n] = wdir_rot;

         if(wdir_rot > wdir_rot_max) wdir_rot_max = wdir_rot;
         if(wdir_rot < wdir_rot_min) wdir_rot_min = wdir_rot;
      } // end for y
   } // end for x

   //
   // Setup the rotated WDIR field
   //
   wdir_rot_wd = wdir_wd;
   wdir_rot_wd.set_b(wdir_rot_min);
   wdir_rot_wd.set_m( (double) (wdir_rot_max-wdir_rot_min)/wrfdata_int_data_max);

   //
   // Store the rotated WDIR values
   //
   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         n = two_to_one(nx, ny, x, y);

         wdir_rot_wd.put_xy_double(data[n], x, y);
      } // end for y
   } // end for x

   if(data) { delete data; data = (double *) 0; }

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
// The following 8 two_to_one_grib_xdir_ydir_order functions are used
// to index into the GRIB file based on the values set in the scan flag
// in the GDS.  The flags are interpreted as follows:
//
//    xdir  == 1 -> In the -x direction
//    xdir  == 0 -> In the +x direction
//
//    ydir  == 1 -> In the +y direction
//    ydir  == 0 -> In the -y direction
//
//    order == 1 -> Data in (y, x) order
//    order == 0 -> Data in (x, y) order
//
///////////////////////////////////////////////////////////////////////////////

int two_to_one_grib_0_0_0(Grid &gr, int x, int y) { // +x, -y, (x, y)

   if( (x < 0) || (x >= gr.nx()) ||
       (y < 0) || (y >= gr.ny()) ) {
      cerr << "\n\nERROR: two_to_one_grib() -> "
           << "range check error: (nx, ny) = ("
           << gr.nx() << ", " << gr.ny() << "), (x, y) = ("
           << x << ", " << y << ")\n\n" << flush;
      exit(1);
   }

   return((gr.ny() - 1 - y)*gr.nx() + x);
}

///////////////////////////////////////////////////////////////////////////////

int two_to_one_grib_0_0_1(Grid &gr, int x, int y) { // +x, -y, (y, x)

   if( (x < 0) || (x >= gr.nx()) ||
       (y < 0) || (y >= gr.ny()) ) {
      cerr << "\n\nERROR: two_to_one_grib() -> "
           << "range check error: (nx, ny) = ("
           << gr.nx() << ", " << gr.ny() << "), (x, y) = ("
           << x << ", " << y << ")\n\n" << flush;
      exit(1);
   }

   return((gr.nx() - 1 - x)*gr.ny() + y);
}

///////////////////////////////////////////////////////////////////////////////

int two_to_one_grib_0_1_0(Grid &gr, int x, int y) { // +x, +y, (x, y)

   if( (x < 0) || (x >= gr.nx()) ||
       (y < 0) || (y >= gr.ny()) ) {
      cerr << "\n\nERROR: two_to_one_grib() -> "
           << "range check error: (nx, ny) = ("
           << gr.nx() << ", " << gr.ny() << "), (x, y) = ("
           << x << ", " << y << ")\n\n" << flush;
      exit(1);
   }

   return(y*gr.nx() + x);
}

///////////////////////////////////////////////////////////////////////////////

int two_to_one_grib_0_1_1(Grid &gr, int x, int y) { // +x, +y, (y, x)

   if( (x < 0) || (x >= gr.nx()) ||
       (y < 0) || (y >= gr.ny()) ) {
      cerr << "\n\nERROR: two_to_one_grib() -> "
           << "range check error: (nx, ny) = ("
           << gr.nx() << ", " << gr.ny() << "), (x, y) = ("
           << x << ", " << y << ")\n\n" << flush;
      exit(1);
   }

   return(x*gr.ny() + y);
}

///////////////////////////////////////////////////////////////////////////////

int two_to_one_grib_1_0_0(Grid &gr, int x, int y) { // -x, -y, (x, y)

   if( (x < 0) || (x >= gr.nx()) ||
       (y < 0) || (y >= gr.ny()) ) {
      cerr << "\n\nERROR: two_to_one_grib() -> "
           << "range check error: (nx, ny) = ("
           << gr.nx() << ", " << gr.ny() << "), (x, y) = ("
           << x << ", " << y << ")\n\n" << flush;
      exit(1);
   }

   return((gr.ny() - 1 - y)*gr.nx() + (gr.nx() - 1 - x));
}

///////////////////////////////////////////////////////////////////////////////

int two_to_one_grib_1_0_1(Grid &gr, int x, int y) { // -x, -y, (y, x)

   if( (x < 0) || (x >= gr.nx()) ||
       (y < 0) || (y >= gr.ny()) ) {
      cerr << "\n\nERROR: two_to_one_grib() -> "
           << "range check error: (nx, ny) = ("
           << gr.nx() << ", " << gr.ny() << "), (x, y) = ("
           << x << ", " << y << ")\n\n" << flush;
      exit(1);
   }

   return((gr.nx() - 1 - x)*gr.ny() + (gr.ny() - 1 - y));
}

///////////////////////////////////////////////////////////////////////////////

int two_to_one_grib_1_1_0(Grid &gr, int x, int y) { // -x, +y, (x, y)

   if( (x < 0) || (x >= gr.nx()) ||
       (y < 0) || (y >= gr.ny()) ) {
      cerr << "\n\nERROR: two_to_one_grib() -> "
           << "range check error: (nx, ny) = ("
           << gr.nx() << ", " << gr.ny() << "), (x, y) = ("
           << x << ", " << y << ")\n\n" << flush;
      exit(1);
   }

   return(y*gr.nx() + (gr.nx() - 1 - x));
}

///////////////////////////////////////////////////////////////////////////////

int two_to_one_grib_1_1_1(Grid &gr, int x, int y) { // -x, +y, (y, x)

   if( (x < 0) || (x >= gr.nx()) ||
       (y < 0) || (y >= gr.ny()) ) {
      cerr << "\n\nERROR: two_to_one_grib() -> "
           << "range check error: (nx, ny) = ("
           << gr.nx() << ", " << gr.ny() << "), (x, y) = ("
           << x << ", " << y << ")\n\n" << flush;
      exit(1);
   }

   return(x*gr.ny() + (gr.ny() - 1 - y));
}

///////////////////////////////////////////////////////////////////////////////
