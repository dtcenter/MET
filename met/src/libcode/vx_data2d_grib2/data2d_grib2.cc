

   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
   // ** Copyright UCAR (c) 1992 - 2012
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
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <cmath>

#include <string>
#include <map>
#include <utility>
#include <limits>
#include <list>

#include "data2d_grib2.h"
#include "vx_data2d.h"
#include "vx_math.h"
#include "vx_log.h"

extern "C" {
  #include "grib2.h"
}

static bool print_grid = false;

using namespace std;

////////////////////////////////////////////////////////////////////////
//
// Code for class MetGrib2DataFile
//
////////////////////////////////////////////////////////////////////////

MetGrib2DataFile::MetGrib2DataFile() {

   grib2_init_from_scratch();

}

////////////////////////////////////////////////////////////////////////

MetGrib2DataFile::~MetGrib2DataFile() {

   close();
}

////////////////////////////////////////////////////////////////////////

MetGrib2DataFile::MetGrib2DataFile(const MetGrib2DataFile &) {

   mlog << Error << "\nMetGrib2DataFile::MetGrib2DataFile(const MetGrib2DataFile &) -> "
        << "should never be called!\n\n";
   exit(1);
}

////////////////////////////////////////////////////////////////////////

MetGrib2DataFile & MetGrib2DataFile::operator=(const MetGrib2DataFile &) {

   mlog << Error << "\nMetGrib2DataFile::operator=(const MetGrib2DataFile &) -> "
        << "should never be called!\n\n";
   exit(1);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void MetGrib2DataFile::grib2_init_from_scratch() {

   ScanMode = -1;

   init_var_maps();

   PairMap["UGRD"] = "VGRD";
   PairMap["VGRD"] = "UGRD";

   return;
}

////////////////////////////////////////////////////////////////////////

void MetGrib2DataFile::close() {

   fclose(FileGrib2);

   return;
}

////////////////////////////////////////////////////////////////////////

bool MetGrib2DataFile::open(const char * _filename) {

   //  attempt to opent he
   Filename = _filename;
   if( NULL == (FileGrib2 = fopen(Filename, "r")) ){
      mlog << Error << "\nMetGrib2DataFile::open() - unable to open input GRIB2 file "
           << _filename << "\n\n";
      exit(1);
   }

   if( 1 > RecList.size() ) read_grib2_record_list();

   bool status = ( 0 < RecList.size() );
   if( !status ){
      mlog << Warning << "\nGRIB2 records not found in input file '" << Filename << "'\n\n";
   }

   return status;
}

////////////////////////////////////////////////////////////////////////

void MetGrib2DataFile::dump(ostream & out, int depth) const {

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

   out.flush();
   return;

}

////////////////////////////////////////////////////////////////////////

bool MetGrib2DataFile::data_plane(VarInfo &vinfo, DataPlane &plane) {

   //  narrow the vinfo pointer
   VarInfoGrib2* vinfo_g2 = (VarInfoGrib2*)(&vinfo);

   //  search the list of records for matches
   vector<Grib2Record*> listMatch, listMatchRange;
   find_record_matches(vinfo_g2, listMatch, listMatchRange);

   //  if no matches were found, check for derived records
   if( 1 > listMatch.size() ){

      DataPlaneArray plane_array = check_derived(vinfo_g2);

      //  verify that only a single data_plane was found
      if( 1 > plane_array.n_planes() ){

         mlog << Warning << "\nMetGrib2DataFile::data_plane() - No matching record found for '"
              << vinfo_g2->magic_str() << "'\n\n";
         return false;

      } else if( 1 < plane_array.n_planes() ){

         mlog << Warning << "\nMetGrib2DataFile::data_plane() - Wind speed/direction derivation "
              << "found " << plane_array.n_planes() << " matching records found for '"
              << vinfo_g2->magic_str() << "'\n\n";

      }

      //  report the matched record
      mlog << Debug(3) << "MetGrib2DataFile::data_plane() - Found derived match for '"
           << vinfo_g2->magic_str() << "' in GRIB2 file '" << Filename << "'\n";

      plane = plane_array[0];
      return true;

   }  //  END: if( 1 > listMatch.size() )

   //  verify that a only single record was found
   if( 1 < listMatch.size() ){
      ConcatString msg = "";
      for(size_t i=0; i < listMatch.size(); i++) msg << (i ? ", " : "") << listMatch[i]->RecNum;
      mlog << Warning << "\nMetGrib2DataFile::data_plane() - Multiple matching records found for '"
           << vinfo_g2->magic_str() << "' - " << msg << " - using " << listMatch[0]->RecNum << "\n\n";
   }

   //  report the matched record
   mlog << Debug(3) << "MetGrib2DataFile::data_plane() - Found exact match for '"
        << vinfo_g2->magic_str() << "' in GRIB2 record " << listMatch[0]->RecNum
        << " field " << listMatch[0]->FieldNum << " of GRIB2 file '"
        << Filename << "'\n";

   //  read the data plane for the matched record
   bool read_success = read_grib2_record_data_plane(listMatch[0], plane);

   //  check the data plane for wind rotation
   plane = check_uv_rotation(vinfo_g2, listMatch[0], plane);

   return read_success;

}

////////////////////////////////////////////////////////////////////////

int MetGrib2DataFile::data_plane_array( VarInfo &vinfo,
                                        DataPlaneArray &plane_array
                                      ){

   //  narrow the vinfo pointer
   VarInfoGrib2* vinfo_g2 = (VarInfoGrib2*)(&vinfo);

   //  search the list of records for matches
   vector<Grib2Record*> listMatchExact, listMatchRange, listRead;
   find_record_matches(vinfo_g2, listMatchExact, listMatchRange);

   //  if multiple exact matches were found, use the first one
   if( 1 <= listMatchExact.size() ){

      //  report on the exact match(es)
      if( 1 < listMatchExact.size() ){
         ConcatString msg = "";
         for(size_t i=0; i < listMatchExact.size(); i++) msg << (i ? ", " : "") << listMatchExact[i]->RecNum;
         mlog << Warning << "\nMetGrib2DataFile::data_plane_array() - Multiple exact matching records found for \""
              << vinfo_g2->magic_str() << "\" - records " << msg << "\n\n";
      } else {
         mlog << Debug(3) << "MetGrib2DataFile::data_plane_array() - Found exact match for \""
              << vinfo_g2->magic_str() << "\" in GRIB2 record " << listMatchExact[0]->RecNum
              << " field " << listMatchExact[0]->FieldNum << " of GRIB2 file \""
              << Filename << "\"\n";
      }

      listRead.push_back( listMatchExact[0] );

   }

   //  otherwise, if range matches were found, use them
   else if( 0 < listMatchRange.size() ){

      ConcatString msg = "";
      for(size_t i=0; i < listMatchRange.size(); i++) msg << (i ? ", " : "") << listMatchRange[i]->RecNum;
      mlog << Debug(3) << "MetGrib2DataFile::data_plane_array() - Found range match for \""
           << vinfo_g2->magic_str() << "\" in GRIB2 records " << msg << " of GRIB2 file \""
           << Filename << "\"\n";

      listRead = listMatchRange;

   }

   //  if nothing was found, try to build derived records
   else {

      plane_array = check_derived(vinfo_g2);

      //  if no matches were found, bail
      if( 1 > plane_array.n_planes() ){
         mlog << Warning << "\nMetGrib2DataFile::data_plane_array() - No matching records found for '"
              << vinfo_g2->magic_str() << "'\n\n";
         return 0;
      }

      //  report the matched record
      mlog << Debug(3) << "\nMetGrib2DataFile::data_plane_array() - Wind speed/direction derivation "
           << "found " << plane_array.n_planes() << " matching records found for '"
           << vinfo_g2->magic_str() << "'\n\n";

      return plane_array.n_planes();

   }

   //  read the matched data planes into a data plane array
   int num_read = 0;
   for( vector<Grib2Record*>::iterator it = listRead.begin();
       it < listRead.end();
       it++ ) {

      //  read the data plane for the current record
      DataPlane plane;
      num_read += ( read_grib2_record_data_plane(*it, plane) ? 1 : 0 );

      //  check the data plane for wind rotation
      plane = check_uv_rotation(vinfo_g2, *it, plane);

      //  add the data plane to the array at the specified level(s)
      double lvl_lower = (double)(*it)->LvlVal1, lvl_upper = (double)(*it)->LvlVal2;
      if( LevelType_Pres == VarInfoGrib2::g2_lty_to_level_type((*it)->LvlTyp) ){
         lvl_lower = ( (double)(*it)->LvlVal1 ) / 100.0;
         lvl_upper = ( (double)(*it)->LvlVal2 ) / 100.0;
      }
      plane_array.add(plane, lvl_lower, lvl_upper);

   }

   return num_read;

}

////////////////////////////////////////////////////////////////////////

void MetGrib2DataFile::find_record_matches( VarInfoGrib2* vinfo,
                                            vector<Grib2Record*> &listMatchExact,
                                            vector<Grib2Record*> &listMatchRange
                                          ){

   //  clear the contents of the result vectors
   listMatchExact.clear();
   listMatchRange.clear();

   //  capture the level information
   LevelType vinfo_lty = vinfo->level().type();
   double lvl1 = vinfo->level().lower();
   double lvl2 = vinfo->level().upper();

   //  check each record for a match against the VarInfo
   for( vector<Grib2Record*>::iterator it = RecList.begin();
       it < RecList.end();
       it++ ) {

      bool rec_match_ex = false;
      bool rec_match_rn = false;

      //  test the timing information
      if( (!is_bad_data(vinfo->lead())  && vinfo->lead()  != (*it)->LeadTime)  ||
          (vinfo->valid()               && vinfo->valid() != (*it)->ValidTime) ||
          (vinfo->init()                && vinfo->init()  != (*it)->InitTime)  ){
         continue;
      }

      //  test for a record number match
      if( vinfo->record() == (*it)->RecNum ){
         rec_match_ex = true;
      }

      //  test for an index or field name match
      else if(
               ( vinfo->discipline() == (*it)->Discipline &&
                 vinfo->parm_cat()   == (*it)->ParmCat    &&
                 vinfo->parm()       == (*it)->Parm       ) ||
               vinfo->name().text()  == (*it)->ParmName
             ){

         //  accumulation level type
         if( LevelType_Accum == vinfo_lty ){

            rec_match_ex = (lvl1 == (*it)->Accum);

         }

         //  record number level type
         else if( LevelType_RecNumber == vinfo_lty && lvl1 == (*it)->RecNum ){

            rec_match_ex = true;

         }

         //  pressure or vertical level type
         else if( vinfo_lty == vinfo->g2_lty_to_level_type((*it)->LvlTyp) ){

            double rec_lvl1 = (double)((*it)->LvlVal1);
            double rec_lvl2 = (double)((*it)->LvlVal1);
            if( LevelType_Pres == vinfo_lty ){
               rec_lvl1 /= 100;
               rec_lvl2 /= 100;
            }
            rec_match_ex = ( lvl1 == rec_lvl1 ) && ( lvl1 == lvl2 || lvl2 == rec_lvl2 );
            rec_match_rn = ( lvl1 != lvl2 && lvl1 <= rec_lvl1 && lvl2 >= rec_lvl2 );

         }

      }  //  END: else if( parameter match )

      //  add the record to the result lists, depending on the match type
      if( rec_match_ex )                 listMatchExact.push_back(*it);
      if( rec_match_ex || rec_match_rn ) listMatchRange.push_back(*it);

   }  //  END:  for( vector<Grib2Record*>::iterator it = RecList.begin(); ...)

}

////////////////////////////////////////////////////////////////////////

DataPlane MetGrib2DataFile::check_uv_rotation(VarInfoGrib2 *vinfo, Grib2Record *rec, DataPlane plane){

   //  check that the field is present in the pair map
   string parm_name = vinfo->name().text();
   if( 0 == PairMap.count( parm_name ) || 0 == (rec->ResCompFlag & 8) ) return plane;

   //  build the magic string of the pair field, and make sure it's present
   ConcatString pair_mag = build_magic( rec );
   pair_mag.replace(parm_name.data(), PairMap[parm_name].data());
   if( 0 == NameRecMap.count( pair_mag.text() ) ){
      mlog << Debug(3) << "MetGrib2DataFile::check_uv_rotation - UV rotation pair "
           << "record not found: '" << pair_mag << "'";
      return plane;
   }

   //  read the data plane for the pair record
   Grib2Record *rec_pair = NameRecMap[pair_mag.text()];
   DataPlane plane_pair;
   read_grib2_record_data_plane(NameRecMap[pair_mag.text()], plane_pair);

   mlog << Debug(3) << "MetGrib2DataFile::check_uv_rotation() - Found pair match \""
        << pair_mag << "\" in GRIB2 record " << rec_pair->RecNum << " field "
        << rec_pair->FieldNum << "\n";

   //  rotate the winds
   DataPlane u2d, v2d, u2d_rot, v2d_rot;
   if( 'U' == parm_name.at(0) ){ u2d = plane;  v2d = plane_pair; }
   else                        { v2d = plane;  u2d = plane_pair; }
   rotate_uv_grid_to_earth(u2d, v2d, *_Grid, u2d_rot, v2d_rot);
   if( 'U' == parm_name.at(0) ){ plane = u2d_rot; }
   else                        { plane = v2d_rot; }

   return plane;

}

////////////////////////////////////////////////////////////////////////

DataPlaneArray MetGrib2DataFile::check_derived( VarInfoGrib2 *vinfo ){

   DataPlaneArray array_ret;

   //  if the requested field cannot be derived, bail
   if( vinfo->name() != "WIND" && vinfo->name() != "WDIR" ) return array_ret;

   //  read the data_plane objects for each constituent
   DataPlaneArray array_u, array_v;
   VarInfoGrib2 vinfo_cons(*vinfo);
   vinfo_cons.set_name( "UGRD" );
   data_plane_array(vinfo_cons, array_u);
   vinfo_cons.set_name( "VGRD" );
   data_plane_array(vinfo_cons, array_v);

   //  derive wind speed or direction
   if( array_u.n_planes() != array_v.n_planes() ){
      mlog << Warning << "\nMetGrib2DataFile::data_plane_array() - when deriving "
           << "winds, the number of U-wind records (" << array_u.n_planes()
           << ") does not match the number of V-wind record (" << array_v.n_planes()
           << ") for GRIB2 file '" << filename() << "'\n\n";

      return array_ret;
   }

   //  loop through each of the data planes
   for(int i=0; i < array_u.n_planes(); i++) {

      //  check that the current level values match
      if( !is_eq(array_u.lower(i), array_v.lower(i)) ||
          !is_eq(array_u.upper(i), array_v.upper(i)) ){

         mlog << Warning << "\nMetGrib1DataFile::data_plane_array() - when deriving "
              << "winds for level " << i+1 << ", the U-wind levels (" << array_u.lower(i)
              << ", " << array_u.upper(i) << ") do not match the V-wind levels ("
              << array_v.lower(i) << ", " << array_v.upper(i) << ") in GRIB file '"
              << filename() << "'\n\n";

         return array_ret;
      }

      //  perform the derivation
      DataPlane plane_deriv;
      if(vinfo->name() == "WIND") derive_wind(array_u[i], array_v[i], plane_deriv);
      else                        derive_wdir(array_u[i], array_v[i], plane_deriv);

      //  add the current data plane
      array_ret.add(plane_deriv, array_u.lower(i), array_u.upper(i));

   }

   return array_ret;

}

////////////////////////////////////////////////////////////////////////

void MetGrib2DataFile::read_grib2_record_list() {

   gribfield  *gfld;
   unsigned char *cgrib;
   long offset = 0, offset_next;
   g2int numfields;
   int idx = 0, rec_num = 1;

   //  read all the records into the record list, pulling grid information from the first
   while( 0 <= (offset_next = read_grib2_record(offset, 0, 1, gfld, cgrib, numfields)) ){

      //  read the grid information, if necessary
      if( !_Grid || 1 > _Grid->nx() || 1 > _Grid->ny() ) read_grib2_grid(gfld);

      //  treat each field of the record as a separate record
      for(int i=1; i <= numfields; i++){

         //  validate the PDS template number
         if( 0 != gfld->ipdtnum && 8 != gfld->ipdtnum && 9 != gfld->ipdtnum ){
            mlog << Error << "\nMetGrib2DataFile::data_plane() - unexpected PDS template number ("
                 << gfld->ipdtnum << ")\n\n";
            exit(1);
         }

         //  store the record information
         Grib2Record *rec = new Grib2Record;
         rec->ByteOffset   = offset;
         rec->Index        = idx++;
         rec->NumFields    = (int)numfields;
         rec->RecNum       = rec_num;
         rec->FieldNum     = i;
         rec->Discipline   = gfld->discipline;
         rec->PdsTmpl      = gfld->ipdtnum;
         rec->ParmCat      = gfld->ipdtmpl[0];
         rec->Parm         = gfld->ipdtmpl[1];
         rec->LvlTyp       = (8 == gfld->ipdtnum ? 8 : gfld->ipdtmpl[9]);
         rec->LvlVal1      = gfld->ipdtmpl[11];
         rec->LvlVal2      = 255 != gfld->ipdtmpl[12] ? gfld->ipdtmpl[14] : rec->LvlVal1;
         rec->RangeTyp     = (8 == gfld->ipdtnum ? gfld->ipdtmpl[25] : 0);
         rec->RangeVal     = (8 == gfld->ipdtnum ? gfld->ipdtmpl[26] : 0);
         rec->ResCompFlag  = gfld->igdtmpl[ 0 == gfld->igdtnum ? 13 : 11 ];

         //  initialize the forecast time information
         rec->ValidTime = -1;
         rec->InitTime  = -1;
         rec->LeadTime  = -1;
         unixtime ref_time = mdyhms_to_unix(gfld->idsect[6], gfld->idsect[7], gfld->idsect[5],
                                            gfld->idsect[8], gfld->idsect[9], gfld->idsect[10]);

         //  parse the time reference indicator (Table 1.2)
         switch( gfld->idsect[4] ){
            case 0:     rec->ValidTime = ref_time;      break;      //  Analysis
            case 1:     rec->InitTime  = ref_time;      break;      //  Start of Forecast
            case 2:     rec->ValidTime = ref_time;      break;      //  Verifying Time of Forecast
            case 3:     rec->ValidTime = ref_time;      break;      //  Observation Time
            default:
               mlog << Error << "\nMetGrib2DataFile::read_grib2_record_list() - found unexpected "
                    << "time reference indicator of " << gfld->ipdtmpl[4] << ".\n\n";
               exit(1);
         }

         //  depending on the template number, determine the reference times
         if( 8 == gfld->ipdtnum ){

            if( -1 != rec->ValidTime ){
               mlog << Error << "\nMetGrib2DataFile::read_grib2_record_list() - accum valid time "
                    << "unexpectedly set for record " << rec->RecNum << " field " << rec->FieldNum
                    << "\n\n";
               exit(1);
            }

            rec->ValidTime = mdyhms_to_unix(gfld->ipdtmpl[16], gfld->ipdtmpl[17], gfld->ipdtmpl[15],
                                            gfld->ipdtmpl[18], gfld->ipdtmpl[19], gfld->ipdtmpl[20]);
            rec->LeadTime = rec->ValidTime - rec->InitTime;

         } else {

            //  determine the time unit of the lead time and calculate it
            double sec_lead_unit = VarInfoGrib2::g2_time_range_unit_to_sec( (int)gfld->ipdtmpl[7] );
            if( 0 >= sec_lead_unit ){
               mlog << Error << "\nMetGrib2DataFile::read_grib2_record_list() - found unexpected "
                    << "lead time unit of " << gfld->ipdtmpl[7] << "\n\n";
               exit(1);
            }
            rec->LeadTime = sec_lead_unit * gfld->ipdtmpl[8];

            //  set the forecast time information
            if     ( -1 == rec->ValidTime )   rec->ValidTime = rec->InitTime  + rec->LeadTime;
            else if( -1 == rec->InitTime  )   rec->InitTime  = rec->ValidTime - rec->LeadTime;

         }

         //  store the probability information, if appropriate
         if( 9 == gfld->ipdtnum ){
            rec->ProbLower = (double)(gfld->ipdtmpl[19]) / pow( 10, (double)(gfld->ipdtmpl[18]) );
            rec->ProbUpper = (double)(gfld->ipdtmpl[21]) / pow( 10, (double)(gfld->ipdtmpl[20]) );
         }

         //  set the accumulation interval
         g2int range_typ = (8 == gfld->ipdtnum ? gfld->ipdtmpl[25] : 0);
         g2int range_val = (8 == gfld->ipdtnum ? gfld->ipdtmpl[26] : 0);
         double sec_accum_unit = VarInfoGrib2::g2_time_range_unit_to_sec( range_typ );
         rec->Accum = range_val * (int)sec_accum_unit;

         //  build the parameter "name"
         ConcatString id;
         id << rec->Discipline << "_" << rec->ParmCat << "_" << rec->Parm;
         rec->ParmName = g2_id_parm(id.text());

         //  add the record to the list
         RecList.push_back(rec);

         //  build data structure for U/V wind pairs
         NameRecMap[build_magic(rec)] = rec;

         g2_free(gfld);
         delete [] cgrib;

         //  if there are more fields in the current record, read the next one
         if( i < numfields ) read_grib2_record(offset, 0, i+1, gfld, cgrib, numfields);

      }  //  END:  for(int i=1; i <= numfields; i++)

      //  set for the next record
      rec_num++;
      offset = offset_next;

   }  //  END:  while( read_grib2_record() )

}

////////////////////////////////////////////////////////////////////////

void MetGrib2DataFile::read_grib2_grid( gribfield *gfld ){

   //  grid_def info
   int n_x = -1;
   int n_y = -1;

   _Grid = new Grid();

   //  determine the radius of the earth
   double r_km = -1;
   switch( gfld->igdtmpl[0] ){
      case 0:     r_km = 6367.470;  break;
      case 6:     r_km = 6371.229;  break;
      default:
         mlog << Error << "\nMetGrib2DataFile::data_plane() - unexpected earth shape ("
              << gfld->igdtmpl[0] << ")\n\n";
         exit(1);
   }

   //  lat/long
   if( 0 == gfld->igdtnum ){

      ScanMode = gfld->igdtmpl[18];

      //  build a LatLonData struct with the projection information
      LatLonData data;
      data.name         = latlon_proj_type;
      data.delta_lat    = (double)gfld->igdtmpl[16] / 1000000.0;
      data.delta_lon    = (double)gfld->igdtmpl[17] / 1000000.0;
      data.Nlat         = gfld->igdtmpl[8];
      data.Nlon         = gfld->igdtmpl[7];
      data.lat_ll       = ((double)gfld->igdtmpl[11] / 1000000.0) - (double)data.Nlat * data.delta_lat;
      data.lon_ll       = 360 - (double)gfld->igdtmpl[12] / 1000000.0;

      //  set the data grid information
      n_x = data.Nlon;
      n_y = data.Nlat;

      //  store the grid information
      _Grid->set(data);

      if( print_grid ){
         mlog << Debug(4) << "\n"
              << "Latitude/Longitude Grid Data:\n"
              << "     lat_ll: " << data.lat_ll << "\n"
              << "     lon_ll: " << data.lon_ll << "\n"
              << "  delta_lat: " << data.delta_lat << "\n"
              << "  delta_lon: " << data.delta_lon << "\n"
              << "       Nlat: " << data.Nlat << "\n"
              << "       Nlon: " << data.Nlon << "\n\n";
      }

   }

   //  polar stereographic
   else if( 20 == gfld->igdtnum ){

      ScanMode = gfld->igdtmpl[17];

      //  determine the hemisphere
      char hem = 0;
      switch(gfld->igdtmpl[16]){
         case 0:  hem = 'N';  break;
         case 1:  hem = 'S';  break;
         default:
            mlog << Error << "\nMetGrib2DataFile::data_plane() - unexpected polar stereo "
                 << " projection center (" << gfld->igdtmpl[16] << ")\n\n";
            exit(1);
      }

      //  build a StereographicData struct with the projection information
      StereographicData data;
      data.name         = stereographic_proj_type;
      data.hemisphere   = hem;
      data.scale_lat    = (double)gfld->igdtmpl[12] / 1000000.0;
      data.lat_pin      = (double)gfld->igdtmpl[9] / 1000000.0;
      data.lon_pin      = 360 - (double)gfld->igdtmpl[10] / 1000000.0;
      data.x_pin        = 0;     //PGO depends on ScanMode?
      data.y_pin        = 0;
      data.lon_orient   = 360 - (double)gfld->igdtmpl[13] / 1000000.0;
      data.d_km         = (double)gfld->igdtmpl[14] / 1000000.0;
      data.r_km         = r_km;
      data.nx           = gfld->igdtmpl[7];
      data.ny           = gfld->igdtmpl[8];

      //  set the data grid information
      n_x = data.nx;
      n_y = data.ny;

      //  store the grid information
      _Grid->set(data);

      if( print_grid ){
         mlog << Debug(4) << "\n"
              << "Stereographic Grid Data:\n"
              << "  hemisphere: " << data.hemisphere << "\n"
              << "   scale_lat: " << data.scale_lat << "\n"
              << "     lat_pin: " << data.lat_pin << "\n"
              << "     lon_pin: " << data.lon_pin << "\n"
              << "       x_pin: " << data.x_pin << "\n"
              << "       y_pin: " << data.y_pin << "\n"
              << "  lon_orient: " << data.lon_orient << "\n"
              << "        d_km: " << data.d_km << "\n"
              << "        r_km: " << data.r_km << "\n"
              << "          nx: " << data.nx << "\n"
              << "          ny: " << data.ny << "\n\n";
      }

   }

   //  mercator
   else if( 10 == gfld->igdtnum ){

      ScanMode = gfld->igdtmpl[14];

      MercatorData data;
      data.name   = mercator_proj_type;
      data.lat_ll = (double)gfld->igdtmpl[8] / 1000000.0;   //PGO depends on ScanMode?
      data.lon_ll = (double)gfld->igdtmpl[9] / 1000000.0;
      data.lat_ur = (double)gfld->igdtmpl[12] / 1000000.0;
      data.lon_ur = (double)gfld->igdtmpl[13] / 1000000.0;
      data.ny     = gfld->igdtmpl[7];
      data.nx     = gfld->igdtmpl[8];

      //  set the data grid information
      n_x = data.nx;
      n_y = data.ny;

      //  store the grid information
      _Grid->set(data);

      if( print_grid ){
         mlog << Debug(4) << "\n"
              << "Mercator Data:\n"
              << "  lat_ll: " << data.lat_ll << "\n"
              << "  lon_ll: " << data.lon_ll << "\n"
              << "  lat_ur: " << data.lat_ur << "\n"
              << "  lon_ur: " << data.lon_ur << "\n"
              << "      ny: " << data.ny << "\n"
              << "      nx: " << data.nx << "\n\n";
      }

   }

   //  lambert conformal
   else if( 30 == gfld->igdtnum ){

      ScanMode = gfld->igdtmpl[17];

      //  build a LambertData struct with the projection information
      LambertData data;
      data.name         = lambert_proj_type;
      data.scale_lat_1  = (double)gfld->igdtmpl[18] / 1000000.0;
      data.scale_lat_2  = (double)gfld->igdtmpl[19] / 1000000.0;
      data.lat_pin      = (double)gfld->igdtmpl[9]  / 1000000.0;
      data.lon_pin      = 360 - (double)gfld->igdtmpl[10] / 1000000.0;
      data.x_pin        = 0;     //PGO depends on ScanMode?
      data.y_pin        = 0;
      data.lon_orient   = 360 - (double)gfld->igdtmpl[13] / 1000000.0;
      data.d_km         = (double)gfld->igdtmpl[14] / 1000000.0;
      data.r_km         = 6371.20;  //PGO jimmied...  r_km;
      data.nx           = gfld->igdtmpl[7];
      data.ny           = gfld->igdtmpl[8];

      //  set the data grid information
      n_x = data.nx;
      n_y = data.ny;

      //  store the grid information
      _Grid->set(data);

      if( print_grid ){
         mlog << Debug(4) << "\n"
              << "Lambert Conformal Grid Data:\n"
              << "  scale_lat_1: " << data.scale_lat_1 << "\n"
              << "  scale_lat_2: " << data.scale_lat_2 << "\n"
              << "      lat_pin: " << data.lat_pin << "\n"
              << "      lon_pin: " << data.lon_pin << "\n"
              << "        x_pin: " << data.x_pin << "\n"
              << "        y_pin: " << data.y_pin << "\n"
              << "   lon_orient: " << data.lon_orient << "\n"
              << "         d_km: " << data.d_km << "\n"
              << "         r_km: " << data.r_km << "\n"
              << "           nx: " << data.nx << "\n"
              << "           ny: " << data.ny << "\n\n";
      }

   }

   //  unrecognized grid
   else {

      mlog << Error << "MetGrib2DataFile::data_plane() found unrecognized grid definition ("
           << gfld->igdtnum << ")\n\n";
      exit(1);

   }

}

////////////////////////////////////////////////////////////////////////

bool MetGrib2DataFile::read_grib2_record_data_plane( Grib2Record *rec,
                                                     DataPlane &plane ){

   //  attempt to read the record
   gribfield *gfld;
   unsigned char *cgrib;
   g2int numfields;
   if( -1 == read_grib2_record(rec->ByteOffset, 1, rec->FieldNum, gfld, cgrib, numfields) ){
      mlog << Error << "\nMetGrib2DataFile::read_grib2_record_data_plane() - failed to read "
           << "record at offset " << rec->ByteOffset << " and field number "
           << rec->FieldNum << "\n\n";
      exit(1);
   }

   //  ensure the grid has been read, and initialize the grid size
   if( !_Grid || 1 > _Grid->nx() || 1 > _Grid->ny() ) read_grib2_grid(gfld);
   int n_x = _Grid->nx();
   int n_y = _Grid->ny();

   //  determine whether or not the data bitmap applies
   bool apply_bmap = ( 0 == gfld->ibmap || 254 == gfld->ibmap );


   //
   //  * * * *  read grid data  * * * *
   //

   //  set up the DataPlane object
   plane.clear();
   plane.set_size(n_x, n_y);

   //  copy the data into the data plane buffer
   for(int x=0; x < n_x; x++){
      for(int y=0; y < n_y; y++){

         //  determine the data index, depending on the scan mode
         int idx_data;
         switch(ScanMode){
         case 0:   /* 0000 0000 */ idx_data = (n_y - y - 1)*n_x + x;               break;
         case 128: /* 1000 0000 */ idx_data = (n_y - y - 1)*n_x + (n_x - x - 1);   break;
         case 64:  /* 0100 0000 */ idx_data =             y*n_x + x;               break;
         case 192: /* 1100 0000 */ idx_data =             y*n_x + (n_x - x - 1);   break;
         case 32:  /* 0010 0000 */ idx_data =             x*n_y + (n_y - y - 1);   break;
         case 160: /* 1010 0000 */ idx_data = (n_x - x - 1)*n_y + (n_x - x - 1);   break;
         case 96:  /* 0110 0000 */ idx_data =             x*n_y + y;               break;
         case 224: /* 1110 0000 */ idx_data = (n_x - x - 1)*n_y + y;               break;
         default:
            mlog << Error << "MetGrib2DataFile::data_plane() found unrecognized ScanMode ("
                 << ScanMode << ")\n\n";
            exit(1);
         }

         //  add the current value to the data plane, if it contains valid data
         plane.set(
            !apply_bmap || 0 != gfld->bmap[idx_data] ?
               (float)gfld->fld[idx_data] :
               bad_data_float,
            x,
            y
          );
      }
   }

   //  set the time information
   plane.set_init  ( rec->InitTime  );
   plane.set_valid ( rec->ValidTime );
   plane.set_lead  ( rec->LeadTime  );
   plane.set_accum ( rec->Accum     );

   //  print a report
   double plane_min, plane_max;
   plane.data_range(plane_min, plane_max);
   mlog << Debug(4) << "\n"
        << "Data plane information:\n"
        << "    plane min: " << plane_min << "\n"
        << "    plane max: " << plane_max << "\n"
        << "    scan mode: " << ScanMode << "\n"
        << "   valid time: " << unix_to_yyyymmdd_hhmmss(rec->ValidTime) << "\n"
        << "    lead time: " << sec_to_hhmmss(rec->LeadTime)  << "\n"
        << "    init time: " << unix_to_yyyymmdd_hhmmss(rec->InitTime)  << "\n"
        << "  bitmap flag: " << gfld->ibmap << "\n\n";

   g2_free(gfld);
   delete [] cgrib;

   return true;
}

////////////////////////////////////////////////////////////////////////

long MetGrib2DataFile::read_grib2_record( long offset,
                                          g2int unpack,
                                          g2int ifld,
                                          gribfield* &gfld,
                                          unsigned char* &cgrib,
                                          g2int &numfields
                                        ) {

   //  the following code was lifted and modified from:
   //  http://www.nco.ncep.noaa.gov/pmb/docs/grib2/download/g2clib.documentation

   //  g2c fields
   g2int  listsec0[3], listsec1[13], numlocal, lskip, lgrib;
   int    retval, ierr;
   size_t  lengrib;

   //  find the next record and read the info, return -1 if fail
   seekgb(FileGrib2, offset, 32000, &lskip, &lgrib);
   if (lgrib == 0) return -1;
   cgrib  =  new unsigned char[lgrib];
   retval = fseek(FileGrib2, lskip, SEEK_SET);
   lengrib = fread(cgrib, sizeof(unsigned char), lgrib, FileGrib2);
   if( g2_info(cgrib, listsec0, listsec1, &numfields, &numlocal) )
      return -1;

   //  read the specified field in the record
   ierr = g2_getfld(cgrib, ifld, unpack, 1, &gfld);

   //  return the offset of the next record
   return lskip + lgrib;

}

////////////////////////////////////////////////////////////////////////

const char* MetGrib2DataFile::build_magic(Grib2Record *rec){

   ConcatString lvl = "";
   int lvl_val1 = (int)rec->LvlVal1, lvl_val2 = (int)rec->LvlVal2;
   switch( VarInfoGrib2::g2_lty_to_level_type(rec->LvlTyp) ){
		case LevelType_Accum:
		   lvl = "A";
		   lvl_val1 = rec->RangeVal * (int)VarInfoGrib2::g2_time_range_unit_to_sec(rec->RangeTyp);
		   lvl_val1 = atoi( sec_to_hhmmss( lvl_val1 ) );
		   lvl_val1 = (0 == lvl_val1 % 10000 ? lvl_val1 / 10000 : lvl_val1);
		   lvl_val2 = lvl_val1;
		   break;
		case LevelType_Vert:       lvl = "Z";     break;
		case LevelType_RecNumber:  lvl = "R";     break;
		case LevelType_None:       lvl = "L";     break;
      case LevelType_Pres:
         lvl = "P";
         lvl_val1 /= 100;
         lvl_val2 /= 100;
         break;
      default: break;
   }

   ConcatString ret;
   if( rec->LvlVal1 == rec->LvlVal2 ){
      ret.format("%s/%s%d", rec->ParmName.c_str(), lvl.text(), lvl_val1);
   } else {
      ret.format("%s/%s%d-%d", rec->ParmName.c_str(), lvl.text(), lvl_val1, lvl_val2);
   }

   string ret_str = ret.text();
   return ret_str.c_str();

}

////////////////////////////////////////////////////////////////////////

int MetGrib2DataFile::index( VarInfo &vinfo ){

   vector<Grib2Record*> listMatchExact, listMatchRange;
   find_record_matches((VarInfoGrib2*)(&vinfo), listMatchExact, listMatchRange);
   return 1 > listMatchExact.size() ? -1 : listMatchExact[0]->Index;

}
