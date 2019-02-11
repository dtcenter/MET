// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
#include <cmath>

#include "data2d_grib.h"
#include "data2d_grib_utils.h"
#include "grib_utils.h"
#include "data2d_utils.h"

#include "vx_math.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


static bool is_grid_relative(const GribRecord &);

static int  get_bit_from_octet(unsigned char, int);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class MetGrib1DataFile
   //


////////////////////////////////////////////////////////////////////////


MetGrib1DataFile::MetGrib1DataFile()

{

grib1_init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


MetGrib1DataFile::~MetGrib1DataFile()

{

close();

}


////////////////////////////////////////////////////////////////////////


MetGrib1DataFile::MetGrib1DataFile(const MetGrib1DataFile &)

{

mlog << Error << "\nMetGrib1DataFile::MetGrib1DataFile(const MetGrib1DataFile &) -> "
     << "should never be called!\n\n";

exit ( 1 );

// grib1_init_from_scratch();
//
// assign(f);

}


////////////////////////////////////////////////////////////////////////


MetGrib1DataFile & MetGrib1DataFile::operator=(const MetGrib1DataFile &)

{

mlog << Error << "\nMetGrib1DataFile::operator=(const MetGrib1DataFile &) -> "
     << "should never be called!\n\n";

exit ( 1 );

// if ( this == &f )  return ( * this );
//
// assign(f);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void MetGrib1DataFile::grib1_init_from_scratch()

{

GF = (GribFile *) 0;

Plane.clear();

close();

return;

}


////////////////////////////////////////////////////////////////////////


void MetGrib1DataFile::close()

{

if ( GF )  { delete GF;  GF = (GribFile *) 0; }

CurrentRecord.reset();

Plane.clear();

mtddf_clear();

return;

}


////////////////////////////////////////////////////////////////////////


bool MetGrib1DataFile::open(const char * _filename)

{

close();

GF = new GribFile;

if ( ! (GF->open(_filename)) )  {

   mlog << Error << "\nMetGrib1DataFile::open(const char *) -> "
        << "unable to open grib1 file \"" << _filename << "\"\n\n";

   // exit ( 1 );

   close();

   return ( false );

}

Filename = _filename;

   //
   //  read first record & get grid
   //
   //   Note: we assume all the records in the grib file
   //
   //    use the same grid.
   //

GF->seek_record(0);

(*GF) >> CurrentRecord;

Raw_Grid = new Grid;

gds_to_grid(*(CurrentRecord.gds), *(Raw_Grid));

Dest_Grid = new Grid;

(*Dest_Grid) = (*Raw_Grid);

get_data_plane(CurrentRecord, Plane);

if ( ShiftRight != 0 )  Plane.shift_right(ShiftRight);


   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


void MetGrib1DataFile::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "File = ";

if ( Filename.empty() )  out << "(nul)\n";
else                     out << '\"' << Filename << "\"\n";

if ( Raw_Grid )  {

   out << prefix << "Grid:\n";

   Raw_Grid->dump(out, depth + 1);

} else {

   out << prefix << "No Grid!\n";

}

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


double MetGrib1DataFile::get(int x, int y) const

{

double value = Plane.get(x, y);

return ( value );

}


////////////////////////////////////////////////////////////////////////


bool MetGrib1DataFile::data_ok(int x, int y) const

{

const double value = get(x, y);

return ( !is_bad_data(value) );

}


////////////////////////////////////////////////////////////////////////


void MetGrib1DataFile::data_minmax(double & data_min, double & data_max) const

{

Plane.data_range(data_min, data_max);

return;

}


////////////////////////////////////////////////////////////////////////


bool MetGrib1DataFile::read_record(const int n, const bool read_plane /* = true */)

{

   //
   // check range
   //
if ( (n < 0) || (n > GF->n_records()) )  {

mlog << Error << "\nMetGrib1DataFile::read_record() -> "
     << "range check error ... n = " << n << "\n\n";

exit ( 1 );

}

   //
   // seek to the requested record
   //
GF->seek_record(n);

   //
   // read it into rec
   //
if ( ! ( (*GF) >> CurrentRecord) )  {

mlog << Error << "\nMetGrib1DataFile::read_record() -> "
     << "trouble reading record number " << n << "\n\n";

return (false);

}

   //
   // put the current record into the plane
   //
if( read_plane ) get_data_plane(CurrentRecord, Plane);

if ( ShiftRight != 0 )  Plane.shift_right(ShiftRight);

return (true);

}


////////////////////////////////////////////////////////////////////////


int MetGrib1DataFile::read_record( VarInfoGrib & v)

{

if ( !GF )  {

   mlog << Error << "\nMetGrib1DataFile::read_record(const VarInfoGrib &) -> "
        << "no grib file open!\n\n";

   // exit ( 1 );

   return ( -1 );

}

int j, j_match;
int count;


count = 0;

j_match = -1;

for (j=0; j<(GF->n_records()); ++j)  {

   if ( ! read_record(j) )  {

      mlog << Error << "\nMetGrib1DataFile::read_record(const VarInfoGrib &) -> trouble reading record!\n\n";

      // exit ( 1 );

      return ( -1 );

   }

   //
   //  if an exact match is found, return only the current record
   //

   if ( is_exact_match(v, CurrentRecord) )  {

      count = 1;

      j_match = j;

      break;

   }

   //
   //  otherwise, continue looking for range matches
   //

   if ( is_range_match(v, CurrentRecord) )  {

      if ( j_match < 0 )  j_match = j;

      ++count;

   }

}

if ( j_match >= 0 )  {

   if ( ! read_record(j_match) )  {

      mlog << Error << "\nMetGrib1DataFile::read_record(const VarInfoGrib &) -> "
           << "trouble reading record!\n\n";

      // exit ( 1 );

      return ( -1 );

   }

}

   //
   //  done
   //

return ( count );

}

////////////////////////////////////////////////////////////////////////

bool MetGrib1DataFile::data_plane(VarInfo &vinfo, DataPlane &plane) {
   bool status = false;
   int n_planes = 0;
   DataPlaneArray plane_array;
   VarInfoGrib *vinfo_grib = (VarInfoGrib *) &vinfo;
   int j;

   // Call data_plane_array() to retrieve all matching records
   n_planes = data_plane_array(*vinfo_grib, plane_array);

   // Find an exact match
   if(n_planes > 0) {

      // Search for an exact pressure or vertical level match
      if ( vinfo_grib->level().type() == LevelType_Pres ||
           vinfo_grib->level().type() == LevelType_Vert ) {

         for (j=0; j<n_planes; ++j )  {
            if ( is_eq(plane_array.lower(j), vinfo_grib->level().lower()) &&
                 is_eq(plane_array.upper(j), vinfo_grib->level().upper()) ) {
               plane  = plane_array[j];
               status = true;
               break;
            }
         }
      }
      // Otherwise, use the first match found.
      else {
         plane  = plane_array[0];
         status = true;

         // Check for more than one matching data_plane
         if(n_planes > 1) {
            mlog << Warning << "\nMetGrib1DataFile::data_plane() -> "
                 << "Found " << n_planes << " matches for VarInfo \""
                 << vinfo.magic_str() << "\" in GRIB file \"" << filename()
                 << "\".  Using the first match found.\n\n";
         }
      }
   } // end if n_planes

   // Check for bad status
   if(!status) {
      mlog << Warning << "\nMetGrib1DataFile::data_plane() -> "
           << "No exact match found for VarInfo \""
           << vinfo.magic_str() << "\" in GRIB file \""
           << filename() << "\".\n\n";
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

int MetGrib1DataFile::data_plane_array(VarInfo &vinfo,
                                       DataPlaneArray &plane_array) {
   bool status = false;
   bool exact;
   int i, lower, upper, type_num;
   GribRecord r;
   VarInfoGrib *vinfo_grib = (VarInfoGrib *) &vinfo;
   VarInfoGrib vinfo_grib_winds;
   LevelInfo cur_level;
   DataPlane cur_plane;
   DataPlaneArray u_plane_array, v_plane_array;

   // Initialize
   plane_array.clear();

   // Loop through the records in the GRIB file looking for matches
   for(i=0; i<GF->n_records(); i++) {

      // Read the current record
      GF->seek_record(i);
      (*GF) >> r;

      // Check for a range match
      if(is_range_match(*vinfo_grib, r)) {

         exact = is_exact_match(*vinfo_grib, r);
         mlog << Debug(3) << "MetGrib1DataFile::data_plane_array() -> "
              << "Found " << ( exact ? "exact" : "range" )
              << " match for VarInfo \"" << vinfo.magic_str()
              << "\" in GRIB record " << i+1 << " of GRIB file \""
              << filename() << "\".\n";

         // Get the level information for this record
         read_pds_level(r, lower, upper, type_num);

         // Read current record
         status = get_data_plane(r, cur_plane);

         // Check if these are winds that should be rotated
         if(status &&
            is_grid_relative(r) &&
            (vinfo_grib->is_u_wind() ||
             vinfo_grib->is_v_wind() ||
             vinfo_grib->is_wind_direction())) {

            // Initialize the current VarInfo object
            vinfo_grib_winds = *vinfo_grib;
            cur_level = vinfo_grib_winds.level();

            // Reset the level range for pressure and vertical levels
            if(cur_level.type() == LevelType_Pres ||
               cur_level.type() == LevelType_Vert) {
               cur_level.set_range(lower, upper);
               vinfo_grib_winds.set_level_info(cur_level);
            }

            // Rotate the winds
            rotate_winds(vinfo_grib_winds, cur_plane);
         }

         if(!status) {
            cur_plane.clear();
            lower = upper = bad_data_int;
            mlog << Warning << "\nMetGrib1DataFile::data_plane_array() -> "
                 << "Can't read record number " << i+1
                 << " from GRIB file \"" << filename() << "\".\n\n";
            continue;
         }
         else {
            process_data_plane(&vinfo, cur_plane);
         }

         // Add current record to the data plane array
         plane_array.add(cur_plane, (double) lower, (double) upper);

      }
   } // end for loop

   // If no matches were found, check for wind records to be derived.
   if(plane_array.n_planes() == 0) {

      // Derive wind speed and direction
      if(vinfo_grib->code() == wdir_grib_code ||
         vinfo_grib->code() == wind_grib_code) {

         mlog << Debug(3) << "MetGrib1DataFile::data_plane_array() -> "
              << "Attempt to derive winds from U and V components.\n";

         // Initialize the current VarInfo object
         vinfo_grib_winds = *vinfo_grib;

         // Retrieve U-wind, doing a rotation if necessary
         mlog << Debug(3) << "MetGrib1DataFile::data_plane_array() -> "
              << "Reading U-wind records.\n";
         vinfo_grib_winds.set_name(ugrd_abbr_str);
         data_plane_array(vinfo_grib_winds, u_plane_array);

         // Retrieve V-wind, doing a rotation if necessary
         mlog << Debug(3) << "MetGrib1DataFile::data_plane_array() -> "
              << "Reading V-wind records.\n";
         vinfo_grib_winds.set_name(vgrd_abbr_str);
         data_plane_array(vinfo_grib_winds, v_plane_array);

         // Derive wind speed or direction
         if(u_plane_array.n_planes() != v_plane_array.n_planes()) {
            mlog << Warning << "\nMetGrib1DataFile::data_plane_array() -> "
                 << "when deriving winds, the number of U-wind records ("
                 << u_plane_array.n_planes() << ") does not match the "
                 << "number of V-wind records (" << v_plane_array.n_planes()
                 << ") for GRIB file \"" << filename() << "\".\n\n";
            return(0);
         }

         // Loop through each of the data planes
         for(i=0; i<u_plane_array.n_planes(); i++) {

            // Check that the current level values match
            if(!is_eq(u_plane_array.lower(i), v_plane_array.lower(i)) ||
               !is_eq(u_plane_array.upper(i), v_plane_array.upper(i))) {

               mlog << Warning << "\nMetGrib1DataFile::data_plane_array() -> "
                    << "when deriving winds for level " << i+1
                    << ", the U-wind levels ("
                    << u_plane_array.lower(i) << ", " << u_plane_array.upper(i)
                    << ") do not match the V-wind levels ("
                    << v_plane_array.lower(i) << ", " << v_plane_array.upper(i)
                    << ") in GRIB file \"" << filename() << "\".\n\n";
               plane_array.clear();
               return(0);
            }

            // Derive wind direction
            if(vinfo_grib->code() == wdir_grib_code) {
               derive_wdir(u_plane_array[i], v_plane_array[i], cur_plane);
            }
            // Derive wind speed
            else {
               derive_wind(u_plane_array[i], v_plane_array[i], cur_plane);
            }

            // Add the current data plane
            plane_array.add(cur_plane, u_plane_array.lower(i), u_plane_array.upper(i));

         } // end for

      } // end if wdir or wind
   } // end if n_planes == 0

   mlog << Debug(3) << "MetGrib1DataFile::data_plane_array() -> "
        << "Found " << plane_array.n_planes()
        << " GRIB records matching VarInfo \""
        << vinfo.magic_str() << "\" in GRIB file \""
        << filename() << "\".\n";

   return(plane_array.n_planes());
}

////////////////////////////////////////////////////////////////////////
//
// This function rotates the wind data that's passed in from
// grid-relative to earth-relative.
//
////////////////////////////////////////////////////////////////////////

void MetGrib1DataFile::rotate_winds(VarInfoGrib &vinfo_grib, DataPlane &plane) {
   VarInfoGrib vinfo_grib_winds = vinfo_grib;
   DataPlane u2d, v2d, u2d_rot, v2d_rot;

   // For U-wind, retrieve the corresponding V-wind, and rotate
   if(vinfo_grib.is_u_wind()) {

      mlog << Debug(3) << "MetGrib1DataFile::rotate_winds() -> "
           << "Have U-wind record, reading V-wind record.\n";
      vinfo_grib_winds.set_name(vgrd_abbr_str);
      data_plane_scalar(vinfo_grib_winds, v2d);
      rotate_uv_grid_to_earth(plane, v2d, grid(), u2d_rot, v2d_rot);
      plane = u2d_rot;
   }

   // For V-wind, retrieve the corresponding U-wind, and rotate
   else if(vinfo_grib.is_v_wind()) {

      mlog << Debug(3) << "MetGrib1DataFile::rotate_winds() -> "
           << "Have V-wind record, reading U-wind record.\n";
      vinfo_grib_winds.set_name(ugrd_abbr_str);
      data_plane_scalar(vinfo_grib_winds, u2d);
      rotate_uv_grid_to_earth(u2d, plane, grid(), u2d_rot, v2d_rot);
      plane = v2d_rot;
   }

   // For wind direction, rotate
   else if(vinfo_grib.is_wind_direction()) {
      mlog << Debug(3) << "MetGrib1DataFile::rotate_winds() -> "
           << "Have wind direction, calling rotate.\n";
      rotate_wdir_grid_to_earth(plane, grid(), u2d);
      plane = u2d;
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// This function retrieves a single data plane as reqested in the
// VarInfo object but does not attempt to rotate or derive winds.
//
////////////////////////////////////////////////////////////////////////

bool MetGrib1DataFile::data_plane_scalar(VarInfoGrib &vinfo_grib,
                                         DataPlane &plane) {
   int i;
   GribRecord r;
   bool status = false;

   // Initialize the data plane
   plane.clear();

   // Loop through the records in the GRIB file looking for a match
   for(i=0; i<GF->n_records(); i++) {

      // Read the current record.
      GF->seek_record(i);
      (*GF) >> r;

      // Check for an exact match
      if(is_exact_match(vinfo_grib, r)) {

         mlog << Debug(3) << "MetGrib1DataFile::data_plane_scalar() -> "
              << "Found exact match for VarInfo \""
              << vinfo_grib.magic_str() << "\" in GRIB record "
              << i+1 << " of GRIB file \"" << filename()
              << "\".\n";

         // Read current record
         status = get_data_plane(r, plane);

         if(status) process_data_plane(&vinfo_grib, plane);

         break;
      }
   } // end for loop

   if(!status) {
      mlog << Warning << "\nMetGrib1DataFile::data_plane_scalar() -> "
           << "No exact match found for VarInfo \""
           << vinfo_grib.magic_str() << "\" in GRIB file \""
           << filename() << "\".\n\n";
   }

   return(status);
}

///////////////////////////////////////////////////////////////////////////////
//
// Check whether or not the res_flag indicates that the vectors are defined
// grid relative rather than earth relative.
//
//////////////////////////////////////////////////////////////////////////////

bool is_grid_relative(const GribRecord &r) {
   unsigned char res_flag;

   // LatLon
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
   // Gaussian
   else if(r.gds->type == 4) {
      res_flag = r.gds->grid_type.gaussian.res_flag;
   }
   // Stereographic
   else if(r.gds->type == 5) {
      res_flag = r.gds->grid_type.stereographic.res_flag;
   }
   else {
      mlog << Error << "\nis_grid_relative() -> "
           << "Unsupported grid type value: " << r.gds->type
           << "\n\n";
      exit(1);
   }

   //
   // Return whether the 5th bit of the res_flag (Octet 17) is on, which
   // indicates that U and V are defined relative to the grid
   //
   return(get_bit_from_octet(res_flag, 5) == 1);
}

//////////////////////////////////////////////////////////////////////////////

int get_bit_from_octet(unsigned char u, int bit) {

   //
   // Bit numbers start at 1, not 0, and
   // the most-significant bit is number 1
   //

   if((bit < 1) || (bit > 8)) {

      mlog << Error << "\nget_bit_from_octet() -> "
           << "bad bit number\n\n";
      exit(1);
   }

   unsigned char mask = (unsigned char) (1 << (8 - bit));

   if(u & mask) return(1);

   return(0);
}

////////////////////////////////////////////////////////////////////////

int MetGrib1DataFile::index(VarInfo &vinfo){

   int rec = -1;
   VarInfoGrib vinfo_g = *( (VarInfoGrib*)(&vinfo) );

   //  check the GRIB file
   if( !GF )  {
      mlog << Error << "\nMetGrib1DataFile::index(const VarInfoGrib &) -> "
           << "no grib file open!\n\n";
      return -1;
   }

   //  look at records until a match is found
   for (rec=0; rec < GF->n_records(); rec++){

      //  read only the header information
      if( ! read_record(rec, false) ){
         mlog << Error << "\nMetGrib1DataFile::read_record(const VarInfoGrib &) -> trouble reading record!\n\n";
         return -1;
      }

      //  if an exact match is found, break
      if( is_exact_match(vinfo_g, CurrentRecord) ) break;

   }

   //  read the time information for the matched record
   int bms_flag = 0, accum = 0;
   unixtime init_ut, valid_ut;
   read_pds(CurrentRecord, bms_flag, init_ut, valid_ut, accum);
   int lead = valid_ut - init_ut;

   //  check the record time information
   if( ( vinfo.valid()              && valid_ut != vinfo.valid() ) ||
       ( vinfo.init()               && init_ut  != vinfo.init()  ) ||
       ( !is_bad_data(vinfo.lead()) && lead     != vinfo.lead()  ) )
      return -1;

   return rec;
}
