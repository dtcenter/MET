// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


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

using namespace std;


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

}


////////////////////////////////////////////////////////////////////////


MetGrib1DataFile & MetGrib1DataFile::operator=(const MetGrib1DataFile &)

{

mlog << Error << "\nMetGrib1DataFile::operator=(const MetGrib1DataFile &) -> "
     << "should never be called!\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


void MetGrib1DataFile::grib1_init_from_scratch()

{

GF = (GribFile *) nullptr;

Plane.clear();

close();

return;

}


////////////////////////////////////////////////////////////////////////


void MetGrib1DataFile::close()

{

if ( GF )  { delete GF;  GF = (GribFile *) nullptr; }

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

   close();

   return false;

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

return true;

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

return value;

}


////////////////////////////////////////////////////////////////////////


bool MetGrib1DataFile::data_ok(int x, int y) const

{

const double value = get(x, y);

return !is_bad_data(value);

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

return false;

}

   //
   // put the current record into the plane
   //
if( read_plane ) get_data_plane(CurrentRecord, Plane);

if ( ShiftRight != 0 )  Plane.shift_right(ShiftRight);

return true;

}


////////////////////////////////////////////////////////////////////////


int MetGrib1DataFile::read_record( VarInfoGrib & v)

{

if ( !GF )  {

   mlog << Error << "\nMetGrib1DataFile::read_record(const VarInfoGrib &) -> "
        << "no grib file open!\n\n";

   return -1;

}

int j, j_match;
int count;


count = 0;

j_match = -1;

for (j=0; j<(GF->n_records()); ++j)  {

   if ( ! read_record(j) )  {

      mlog << Error << "\nMetGrib1DataFile::read_record(const VarInfoGrib &) -> trouble reading record!\n\n";

      return -1;

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

      return -1;

   }

}

   //
   //  done
   //

return count;

}

////////////////////////////////////////////////////////////////////////

bool MetGrib1DataFile::data_plane(VarInfo &vinfo, DataPlane &plane,
                                  bool do_winds) {
   bool status = false;
   int n_planes = 0;
   DataPlaneArray plane_array;
   VarInfoGrib *vinfo_grib = (VarInfoGrib *) &vinfo;
   int j;

   // Call data_plane_array() to retrieve all matching records
   n_planes = data_plane_array(*vinfo_grib, plane_array, do_winds);

   // Process multiple matches
   if ( n_planes > 0 )  {

      int n_match = 0;

      // Search for an exact pressure or vertical level match
      if ( vinfo_grib->level().type() == LevelType_Pres ||
           vinfo_grib->level().type() == LevelType_Vert )  {

         for ( j=0; j<n_planes; ++j )  {
            if ( is_eq(plane_array.lower(j), vinfo_grib->level().lower()) &&
                 is_eq(plane_array.upper(j), vinfo_grib->level().upper()) )  {
               n_match++;
               if ( n_match == 1 )  {
                  plane  = plane_array[j];
                  status = true;
               }
            }
         }
      }
      // Store the first match found
      else {
         n_match = n_planes;
         plane   = plane_array[0];
         status  = true;
      }

      // Print warning for more multiple matches
      if(n_match > 1) {
         mlog << Warning << "\nMetGrib1DataFile::data_plane() -> "
              << "Found " << n_match << " matches for VarInfo \""
              << vinfo.magic_str() << "\" in GRIB file \"" << filename()
              << "\".  Using the first match found.\n\n";
      }
   } // end if n_planes > 0

   // Check for bad status
   if(!status) {
      mlog << Warning << "\nMetGrib1DataFile::data_plane() -> "
           << "No exact match found for VarInfo \""
           << vinfo.magic_str() << "\" in GRIB file \""
           << filename() << "\".\n\n";
   }

   return status;
}

////////////////////////////////////////////////////////////////////////

int MetGrib1DataFile::data_plane_array(VarInfo &vinfo,
                                       DataPlaneArray &plane_array,
                                       bool do_winds) {
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

         if(!status) {
            cur_plane.clear();
            lower = upper = bad_data_int;
            mlog << Warning << "\nMetGrib1DataFile::data_plane_array() -> "
                 << "Can't read record number " << i+1
                 << " from GRIB file \"" << filename() << "\".\n\n";
            continue;
         }

         // Store whether winds are grid relative
         vinfo.set_grid_relative_flag(is_grid_relative(r));

         // Add current record to the data plane array
         plane_array.add(cur_plane, (double) lower, (double) upper);

      }
   } // end for loop

   // Handle wind rotation
   if(do_winds) rotate_winds(&vinfo, plane_array);

   // If nothing was found, try to build derived records
   if(plane_array.n_planes() == 0 && do_winds) {
      derive_winds(&vinfo, plane_array);
   }

   // Post-process each data plane
   for(int i=0; i<plane_array.n_planes(); i++) {
      process_data_plane(&vinfo, plane_array.at(i));
   }

   mlog << Debug(3) << "MetGrib1DataFile::data_plane_array() -> "
        << "Found " << plane_array.n_planes()
        << " GRIB records matching VarInfo \""
        << vinfo.magic_str() << "\" in GRIB file \""
        << filename() << "\".\n";

   return plane_array.n_planes();
}

///////////////////////////////////////////////////////////////////////////////
//
// Check whether or not the res_flag indicates that the vectors are defined
// grid relative rather than earth relative.
//
//////////////////////////////////////////////////////////////////////////////

static bool is_grid_relative(const GribRecord &r) {
   unsigned char res_flag = 0;

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
   // Rotated LatLon
   else if(r.gds->type == 10) {
      res_flag = r.gds->grid_type.rot_latlon_grid.res_flag;
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

   if(u & mask) return 1;

   return 0;
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

////////////////////////////////////////////////////////////////////////
