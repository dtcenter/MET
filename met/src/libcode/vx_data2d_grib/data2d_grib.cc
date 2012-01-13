

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "data2d_grib.h"
#include "data2d_grib_utils.h"
#include "grib_utils.h"

#include "vx_math.h"
#include "vx_log.h"


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

mlog << Error << "\n\n  MetGrib1DataFile::MetGrib1DataFile(const MetGrib1DataFile &) -> "
     << "should never be called!\n\n";

exit ( 1 );

// grib1_init_from_scratch();
//
// assign(f);

}


////////////////////////////////////////////////////////////////////////


MetGrib1DataFile & MetGrib1DataFile::operator=(const MetGrib1DataFile &)

{

mlog << Error << "\n\n  MetGrib1DataFile::operator=(const MetGrib1DataFile &) -> "
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

   mlog << Error << "\n\n  MetGrib1DataFile::open(const char *) -> "
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

_Grid = new Grid;

gds_to_grid(*(CurrentRecord.gds), *(_Grid));

get_data_plane(CurrentRecord, Plane);


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

if ( _Grid )  {

   out << prefix << "Grid:\n";

   _Grid->dump(out, depth + 1);

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

return ( value != bad_data_double );

}


////////////////////////////////////////////////////////////////////////


void MetGrib1DataFile::data_minmax(double & data_min, double & data_max) const

{

Plane.data_range(data_min, data_max);

return;

}


////////////////////////////////////////////////////////////////////////


bool MetGrib1DataFile::read_record(const int n)

{

   //
   // check range
   //
if ( (n < 0) || (n > GF->n_records()) )  {

mlog << Error << "\n\n  MetGrib1DataFile::read_record() -> "
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

mlog << Error << "\n\n  MetGrib1DataFile::read_record() -> "
     << "trouble reading record number " << n << "\n\n";

return (false);

}

   //
   // put the current record into the plane
   //
get_data_plane(CurrentRecord, Plane);

return (true);

}


////////////////////////////////////////////////////////////////////////


int MetGrib1DataFile::read_record(const VarInfoGrib & v)

{

if ( !GF )  {

   mlog << Error << "\n\n  MetGrib1DataFile::read_record(const VarInfoGrib &) -> "
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

      mlog << Error << "\n\n  MetGrib1DataFile::read_record(const VarInfoGrib &) -> trouble reading record!\n\n";

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

      mlog << Error << "\n\n  MetGrib1DataFile::read_record(const VarInfoGrib &) -> "
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

// Need to add logic for deriving wind speed and rotating U and V
// from grid to earth relative.

bool MetGrib1DataFile::data_plane(VarInfo &vinfo, DataPlane &plane) {
   int i;
   bool status = false;
   GribRecord r;
   VarInfoGrib * vinfo_grib = (VarInfoGrib *) &vinfo;

   // Initialize the data plane
   plane.clear();

   // Loop through the records in the GRIB file looking for a match
   for(i=0; i<GF->n_records(); i++) {

      // Read the current record.
      GF->seek_record(i);
      (*GF) >> r;

      // Check for an exact match
      if(is_exact_match(*vinfo_grib, r)) {

         mlog << Debug(3) << "Found exact match for VarInfo \""
              << vinfo.magic_str() << "\" in GRIB record "
              << i+1 << " of GRIB file \"" << filename()
              << "\".\n";

         // Read current record and break out of the loop
         status = get_data_plane(r, plane);

         // JHG, Need to rotate winds if necessary!

         break;
      }
   } // end for loop

   // JHG, work here
/*   
   // If no exact match found, check for wind records
   if(!status) {

      // Derive wind direction
      if(vinfo_grib.code() == wdir_grib_code) {
         status = derive_wdir_record(grib_file, grib_record, wd, gr, gc_info,
                                     req_vld_ut, req_lead_sec);
      }
      // Derive wind speed
      else if(vinfo_grib.code() == wind_grib_code) {
         status = derive_wind_record(grib_file, grib_record, wd, gr, gc_info,
                                     req_vld_ut, req_lead_sec);
      }
   }
*/
   if(!status) {
      mlog << Debug(3) << "No exact match found for VarInfo \""
           << vinfo.magic_str() << "\" in GRIB file \""
           << filename() << "\".\n";   
   }
   
   return(status);
}

////////////////////////////////////////////////////////////////////////

// Need to add logic for deriving wind speed and rotating U and V
// from grid to earth relative.

int MetGrib1DataFile::data_plane_array(VarInfo &vinfo,
                                       DataPlaneArray &plane_array) {
   int i, status, lower, upper;
   GribRecord r;
   VarInfoGrib *vinfo_grib = (VarInfoGrib *) &vinfo;
   DataPlane cur_plane;

   // Initialize
   plane_array.clear();
   
   // Loop through the records in the GRIB file looking for matches
   for(i=0; i<GF->n_records(); i++) {

      // Read the current record
      GF->seek_record(i);
      (*GF) >> r;

      // Check for a range match
      if(is_range_match(*vinfo_grib, r)) {

         mlog << Debug(3) << "Found range match for VarInfo \""
              << vinfo.magic_str() << "\" in GRIB record "
              << i+1 << " of GRIB file \"" << filename()
              << "\".\n";

         // Get the level information for this record
         read_pds_level(r, lower, upper);

         // Read current record
         status = get_data_plane(r, cur_plane);

         // Add current record to the data plane array
         plane_array.add(cur_plane, (double) lower, (double) upper);

         if(!status) {
            mlog << Warning << "\n\n  MetGrib1DataFile::data_plane_array() -> "
                 << "Can't read record number " << i+1
                 << " from GRIB file \"" << filename() << "\".\n\n";
            continue;
         }
     
      }
   } // end for loop


   mlog << Debug(3) << "Found " << plane_array.n_planes()
        << " GRIB records matching VarInfo \""
        << vinfo.magic_str() << "\" in GRIB file \""
        << filename() << "\".\n";

   return(plane_array.n_planes());
}

////////////////////////////////////////////////////////////////////////
