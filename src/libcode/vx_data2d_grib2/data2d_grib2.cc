

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
#include "vx_math.h"
#include "vx_log.h"

extern "C" {
  #include "grib2.h"
}

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

   return;
}

////////////////////////////////////////////////////////////////////////

void MetGrib2DataFile::close() {

   fclose(FileGrib2);

   return;
}

////////////////////////////////////////////////////////////////////////

bool MetGrib2DataFile::open(const char * _filename) {

   Filename = _filename;
   if( NULL == (FileGrib2 = fopen(Filename, "r")) ){
      mlog << Error << "\nMetGrib2DataFile::open() - unable to open input GRIB2 file "
           << _filename << "\n\n";
      exit(1);
   }

   if( 1 > RecList.size() ) read_grib2_record_list();

   return(true);
}

////////////////////////////////////////////////////////////////////////

void MetGrib2DataFile::dump(ostream & out, int depth) const {

   return;
}

////////////////////////////////////////////////////////////////////////

bool MetGrib2DataFile::data_plane(VarInfo &vinfo, DataPlane &plane) {

   //  narrow the vinfo pointer, and store level information
   VarInfoGrib2* vinfo_g2 = (VarInfoGrib2*)(&vinfo);
   LevelType vinfo_lty = vinfo_g2->level().type();
   double lvl1 = vinfo_g2->level().lower();
   double lvl2 = vinfo_g2->level().upper();

   //  start the timer
   time_t time_start, time_seek, time_read;
   int hh, mm, ss;
   char time_buf[64];
   time( &time_start );

   //  search the list of records for matches
   vector<Grib2Record*> listMatch;
   for( vector<Grib2Record*>::iterator it = RecList.begin();
       it < RecList.end();
       it++ ) {

      bool rec_match = false;

      //  test for a record number match
      if( vinfo_g2->record() == (*it)->RecNum ){
         rec_match = true;
      }

      //  test for a index match
      else if(
               ( vinfo_g2->discipline() == (*it)->Discipline &&
                 vinfo_g2->parm_cat()   == (*it)->ParmCat    &&
                 vinfo_g2->parm()       == (*it)->Parm       ) ||
               vinfo_g2->name() == (*it)->ParmName
             ){

         if( LevelType_Accum == vinfo_lty ){

            double sec_accum_unit = vinfo_g2->g2_time_range_unit_to_sec( (*it)->RangeTyp );
            rec_match = (lvl1 == (*it)->RangeVal * (int)sec_accum_unit);

         } else if( LevelType_RecNumber == vinfo_lty && lvl1 == (*it)->RecNum ){

            rec_match = true;

         } else if( vinfo_lty == vinfo_g2->g2_lty_to_level_type((*it)->LvlTyp) ){

            double rec_lvl1 = (double)((*it)->LvlVal1);
            double rec_lvl2 = (double)((*it)->LvlVal1);
            if( LevelType_Pres == vinfo_lty ){
               rec_lvl1 /= 100;
               rec_lvl2 /= 100;
            }
            rec_match = ( lvl1 == rec_lvl1 ) && ( lvl1 == lvl2 || lvl2 == rec_lvl2 );

         }

      }

      if( rec_match ) listMatch.push_back(*it);

   }

   //  report the seek time
   time( &time_seek );
   sec_to_hms( (int)time_seek - (int)time_start, hh, mm, ss );
   sprintf(time_buf, "Seek time: %02d:%02d:%02d", hh, mm, ss);
   mlog << Debug(4) << time_buf << "\n";

   //  verify that a single record was found
   if( 1 < listMatch.size() ){
      ConcatString msg = "";
      for(size_t i=0; i < listMatch.size(); i++) msg << (i ? ", " : "") << listMatch[i]->RecNum;
      mlog << Warning << "MetGrib2DataFile::data_plane() - Multiple matching records found for '"
           << vinfo_g2->magic_str() << "' - " << msg << "\n\n";
   } else if( 1 > listMatch.size() ){
      mlog << Warning << "MetGrib2DataFile::data_plane() - No matching record found for '"
           << vinfo_g2->magic_str() << "'\n\n";
      return false;
   }

   //  report the matched record
   mlog << Debug(3) << "MetGrib2DataFile::data_plane() - Found exact match for \""
        << vinfo_g2->magic_str() << "\" in GRIB2 record " << listMatch[0]->RecNum
        << " field " << listMatch[0]->FieldNum << " of GRIB2 file \""
        << Filename << "\"\n";

   //  read the record
   gribfield  *gfld;
   unsigned char *cgrib;
   long offset = listMatch[0]->ByteOffset;
   g2int numfields;

   update_var_info(vinfo_g2, listMatch[0]);
   if( -1 == read_grib2_record(offset, 1, 1, gfld, cgrib, numfields) ){
      mlog << Error << "\nMetGrib2DataFile::data_plane() - Failed to read record at offset "
           << offset << " and field number " << listMatch[0]->FieldNum << "\n\n";
      exit(1);
   }

   //  build the data plane
   bool read_success = read_data_plane(plane, gfld);

   //  print the read time
   time( &time_read );
   sec_to_hms( (int)time_read - (int)time_seek, hh, mm, ss );
   sprintf(time_buf, "Read time:  %02d:%02d:%02d", hh, mm, ss);
   mlog << Debug(4) << time_buf << "\n";

   g2_free(gfld);
   delete [] cgrib;

   return read_success;
}

////////////////////////////////////////////////////////////////////////

int MetGrib2DataFile::data_plane_array(VarInfo &vinfo,
                                       DataPlaneArray &plane_array) {

   //  narrow the vinfo pointer, and store level information
   VarInfoGrib2* vinfo_g2 = (VarInfoGrib2*)(&vinfo);
   LevelType vinfo_lty = vinfo_g2->level().type();
   double lvl1 = vinfo_g2->level().lower();
   double lvl2 = vinfo_g2->level().upper();

   //  start the timer
   time_t time_start, time_seek, time_read;
   int hh, mm, ss;
   char time_buf[64];
   time( &time_start );

   //  search the list of records for matches
   vector<Grib2Record*> listMatchExact;
   vector<Grib2Record*> listMatchRange;
   for( vector<Grib2Record*>::iterator it = RecList.begin();
       it < RecList.end();
       it++ ) {

      bool rec_match_ex = false;
      bool rec_match_rn = false;

      //  test for a record number match
      if( vinfo_g2->record() == (*it)->RecNum ){
         rec_match_ex = true;
      }

      //  test for a index match
      else if(
               ( vinfo_g2->discipline() == (*it)->Discipline &&
                 vinfo_g2->parm_cat()   == (*it)->ParmCat    &&
                 vinfo_g2->parm()       == (*it)->Parm       ) ||
               vinfo_g2->name() == (*it)->ParmName
             ){

         if( LevelType_Accum == vinfo_lty ){

            double sec_accum_unit = vinfo_g2->g2_time_range_unit_to_sec( (*it)->RangeTyp );
            rec_match_ex = (lvl1 == (*it)->RangeVal * (int)sec_accum_unit);

         } else if( LevelType_RecNumber == vinfo_lty && lvl1 == (*it)->RecNum ){

            rec_match_ex = true;

         } else if( vinfo_lty == vinfo_g2->g2_lty_to_level_type((*it)->LvlTyp) ){

            double rec_lvl1 = (double)((*it)->LvlVal1);
            double rec_lvl2 = (double)((*it)->LvlVal1);
            if( LevelType_Pres == vinfo_lty ){
               rec_lvl1 /= 100;
               rec_lvl2 /= 100;
            }
            rec_match_ex = ( lvl1 == rec_lvl1 ) && ( lvl1 == lvl2 || lvl2 == rec_lvl2 );
            rec_match_rn = ( lvl1 != lvl2 && lvl1 <= rec_lvl1 && lvl2 >= rec_lvl2 );

         }

      }

      if( rec_match_ex )                 listMatchExact.push_back(*it);
      if( rec_match_ex || rec_match_rn ) listMatchRange.push_back(*it);

   }

   //  report the seek time
   time( &time_seek );
   sec_to_hms( (int)time_seek - (int)time_start, hh, mm, ss );
   sprintf(time_buf, "Seek time: %02d:%02d:%02d", hh, mm, ss);
   mlog << Debug(4) << time_buf << "\n";

   vector<Grib2Record*> listRead = listMatchExact;

   //  if multiple exact matches were found, bail
   if( 1 < listMatchExact.size() ){
      ConcatString msg = "";
      for(size_t i=0; i < listMatchExact.size(); i++) msg << (i ? ", " : "") << listMatchExact[i]->RecNum;
      mlog << Warning << "\nMetGrib2DataFile::data_plane_array() - Multiple exact matching records found for \""
           << vinfo_g2->magic_str() << "\" - records " << msg << "\n\n";

      listRead[0] = listMatchExact[0];
   }

   //  if a single exact match was found, read it
   else if( 1 == listMatchExact.size() ){

      //  report the matched record
      mlog << Debug(3) << "MetGrib2DataFile::data_plane_array() - Found exact match for \""
           << vinfo_g2->magic_str() << "\" in GRIB2 record " << listMatchExact[0]->RecNum
           << " field " << listMatchExact[0]->FieldNum << " of GRIB2 file \""
           << Filename << "\"\n";

   }

   //  if range matches were found, read them
   else if( 0 < listMatchRange.size() ){

      ConcatString msg = "";
      for(size_t i=0; i < listMatchRange.size(); i++) msg << (i ? ", " : "") << listMatchRange[i]->RecNum;
      mlog << Debug(3) << "MetGrib2DataFile::data_plane_array() - Found range match for \""
           << vinfo_g2->magic_str() << "\" in GRIB2 records " << msg << " of GRIB2 file \""
           << Filename << "\"\n";

      listRead = listMatchRange;

   }

   //  if nothing was found, bail
   else {

      mlog << Warning << "\nMetGrib2DataFile::data_plane_array() - no matching record found for '"
           << vinfo_g2->magic_str() << "'\n\n";
      return 0;

   }

   //  read the matched data planes into a data plane array
   int num_read = 0;
   for( vector<Grib2Record*>::iterator it = listRead.begin();
       it < listRead.end();
       it++ ) {

      //  read the record
      gribfield  *gfld;
      unsigned char *cgrib;
      long offset = (*it)->ByteOffset;
      g2int numfields;

      update_var_info(vinfo_g2, (*it));
      if( -1 == read_grib2_record(offset, 1, 1, gfld, cgrib, numfields) ){
         mlog << Error << "\nMetGrib2DataFile::data_plane_array() - failed to read "
              << "record at offset " << offset << " and field number "
              << (*it)->FieldNum << "\n\n";
         exit(1);
      }

      //  build the data plane
      DataPlane plane;
      num_read += read_data_plane(plane, gfld) ? 1 : 0;

      //  add the data plane to the array at the specified level(s)
      double lvl_lower = (double)(*it)->LvlVal1, lvl_upper = (double)(*it)->LvlVal2;
      if( LevelType_Pres == vinfo_g2->g2_lty_to_level_type((*it)->LvlTyp) ){
         lvl_lower = ( (double)(*it)->LvlVal1 ) / 100.0;
         lvl_upper = ( (double)(*it)->LvlVal2 ) / 100.0;
      }
      plane_array.add(plane, lvl_lower, lvl_upper);

      //  print the read time
      time( &time_read );
      sec_to_hms( (int)time_read - (int)time_seek, hh, mm, ss );
      sprintf(time_buf, "Read time:  %02d:%02d:%02d", hh, mm, ss);
      mlog << Debug(4) << time_buf << "\n";

      g2_free(gfld);
      delete [] cgrib;

   }

   return num_read;
}

////////////////////////////////////////////////////////////////////////

void MetGrib2DataFile::update_var_info(VarInfoGrib2* vinfo, Grib2Record *rec){
   ConcatString parm_id = "";
   parm_id << rec->Discipline << "_" << rec->ParmCat << "_" << rec->Parm;

   if( !g2_id_count( parm_id ) ){
      mlog << Error << "\nMetGrib2DataFile::update_var_info() - No record found with id "
           << parm_id << "\n\n";
      exit(1);
   }

   //  parse the variable name from the table information
   char** mat = NULL;
   const char* pat_mag = "^([^\\|]+)\\s+\\|\\s+([^\\|]+)\\s+\\|\\s+(.+\\S)\\s*$";
   if( 4 != regex_apply(pat_mag, 4, g2_id_lookup( parm_id ), mat) ){
      mlog << Error << "\nMetGrib2DataFile::update_var_info - failed to parse GRIB2 table "
           << "map_id information '" << g2_id_lookup( parm_id ) << "'\n\n";
      exit(1);
   }

   //  set the var_info parameter info
   ConcatString name  = mat[1]; name.ws_strip();
   if( name == "APCP" ){
      int accum = atoi( sec_to_hhmmss( (int)vinfo->level().lower() ).text() );
      if( 0 == accum % 10000 ) accum /= 10000;
      ConcatString name_apcp;
      name_apcp.format("%s_%02d", name.text(), accum);
      name = name_apcp.text();
   }
   vinfo->set_name( name );
   ConcatString units = mat[2]; units.ws_strip(); vinfo->set_units    ( units );
   ConcatString lname = mat[3]; lname.ws_strip(); vinfo->set_long_name( lname );

   //  set the level name
   /*
   ConcatString lvl_name = "";
   int lvl_val = (int)(vinfo->level().type() == LevelType_Pres ? rec->LvlVal1 / 100 : rec->LvlVal1);
   lvl_name << vinfo->level().name() << lvl_val;
   vinfo->level().set_req_name( lvl_name );
   vinfo->level().set_name( lvl_name );
   */

   regex_clean(mat);

}

////////////////////////////////////////////////////////////////////////

void MetGrib2DataFile::read_grib2_record_list() {

   gribfield  *gfld;
   unsigned char *cgrib;
   long offset = 0, offset_next;
   g2int numfields;
   int rec_num = 1;

   //  start the timer
   time_t time_start, time_read;
   int hh, mm, ss;
   char time_buf[64];
   time( &time_start );

   //  read all the records into the record list, pulling grid information from the first
   while( 0 <= (offset_next = read_grib2_record(offset, 0, 1, gfld, cgrib, numfields)) ){

      if( !_Grid || 1 > _Grid->nx() || 1 > _Grid->ny() ) read_grid(gfld);

      for(int i=1; i <= numfields; i++){

         //  validate the PDS template number
         if( 0 != gfld->ipdtnum && 8 != gfld->ipdtnum ){
            mlog << Error << "\nMetGrib2DataFile::data_plane() - unexpected PDS template number ("
                 << gfld->ipdtnum << ")\n\n";
            exit(1);
         }

         Grib2Record *rec = new Grib2Record;
         rec->ByteOffset   = offset;
         rec->NumFields    = (int)numfields;
         rec->RecNum       = rec_num;
         rec->FieldNum     = i;
         rec->Discipline   = gfld->discipline;
         rec->PdsTmpl      = gfld->ipdtnum;
         rec->ParmCat      = gfld->ipdtmpl[0];
         rec->Parm         = gfld->ipdtmpl[1];
         rec->LvlTyp       = gfld->ipdtmpl[9];
         rec->LvlVal1      = gfld->ipdtmpl[11];
         rec->LvlVal2      = 255 != gfld->ipdtmpl[12] ? gfld->ipdtmpl[14] : rec->LvlVal1;
         rec->RangeTyp     = (8 == gfld->ipdtnum ? gfld->ipdtmpl[25] : 0);
         rec->RangeVal     = (8 == gfld->ipdtnum ? gfld->ipdtmpl[26] : 0);

         ConcatString id;
         id << rec->Discipline << "_" << rec->ParmCat << "_" << rec->Parm;
         rec->ParmName = g2_id_parm(id);

         RecList.push_back(rec);

         g2_free(gfld);
         delete [] cgrib;

         if( i <= numfields ) read_grib2_record(offset, 0, i, gfld, cgrib, numfields);

      }

      rec_num++;
      offset = offset_next;
   }

   //  report the timer
   time( &time_read );
   sec_to_hms( (int)time_read - (int)time_start, hh, mm, ss );
   sprintf(time_buf, "Table build time: %02d:%02d:%02d", hh, mm, ss);
   mlog << Debug(4) << time_buf << "\n";

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
   g2int  listsec0[3], listsec1[13], numlocal;
   long   lskip, lgrib;
   int    retval, ierr;
   size_t  lengrib;

   //  find the next record and read the info, return -1 if fail
   seekgb(FileGrib2, offset, 32000, &lskip, &lgrib);
   if (lgrib == 0) return -1;
   cgrib  =  new unsigned char[lgrib];
   retval = fseek(FileGrib2, lskip, SEEK_SET);
   lengrib = fread(cgrib, sizeof(unsigned char), lgrib, FileGrib2);
   ierr = g2_info(cgrib, listsec0, listsec1, &numfields, &numlocal);

   //  read the specified field in the record
   ierr = g2_getfld(cgrib, ifld, unpack, 1, &gfld);

   //  return the offset of the next record
   return lskip + lgrib;
}

////////////////////////////////////////////////////////////////////////

void MetGrib2DataFile::read_grid( gribfield *gfld ){

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

      mlog << Debug(4) << "\n"
           << "Latitude/Longitude Grid Data:\n"
           << "     lat_ll: " << data.lat_ll << "\n"
           << "     lon_ll: " << data.lon_ll << "\n"
           << "  delta_lat: " << data.delta_lat << "\n"
           << "  delta_lon: " << data.delta_lon << "\n"
           << "       Nlat: " << data.Nlat << "\n"
           << "       Nlon: " << data.Nlon << "\n\n";
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

   //  mercator
   else if( 20 == gfld->igdtnum ){

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

      mlog << Debug(4) << "\n"
           << "Mercator Data:\n"
           << "  lat_ll: " << data.lat_ll << "\n"
           << "  lon_ll: " << data.lon_ll << "\n"
           << "  lat_ur: " << data.lat_ur << "\n"
           << "  lon_ur: " << data.lon_ur << "\n"
           << "      ny: " << data.ny << "\n"
           << "      nx: " << data.nx << "\n\n";
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

   //  unrecognized grid
   else {
      mlog << Error << "MetGrib2DataFile::data_plane() found unrecognized grid definition ("
           << gfld->igdtnum << ")\n\n";
      exit(1);
   }

}

////////////////////////////////////////////////////////////////////////

bool MetGrib2DataFile::read_data_plane( DataPlane &plane,
                                        gribfield *gfld ){

   //  ensure the grid has been read, and initialize the grid size
   if( !_Grid || 1 > _Grid->nx() || 1 > _Grid->ny() ) read_grid(gfld);
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


   //
   //  * * * *  parse time information  * * * *
   //

   //  initialize the forecast time information
   unixtime ValidTime = -1;
   unixtime InitTime  = -1;
   unixtime LeadTime  = -1;
   unixtime ref_time = mdyhms_to_unix(gfld->idsect[6], gfld->idsect[7], gfld->idsect[5],
                                      gfld->idsect[8], gfld->idsect[9], gfld->idsect[10]);

   //  parse the time reference indicator
   switch( gfld->idsect[4] ){
      case 0:     ValidTime = ref_time;      break;      //  Analysis
      case 1:     InitTime  = ref_time;      break;      //  Start of Forecast
      case 2:     ValidTime = ref_time;      break;      //  Verifying Time of Forecast
      case 3:     ValidTime = ref_time;      break;      //  Observation Time
      default:
         mlog << Error << "\nMetGrib2DataFile::data_plane() found unexpected time reference "
              << " indicator of " << gfld->ipdtmpl[4] << ".\n\n";
         exit(1);
   }

   //  calculate the lead, valid and init time depending on the PDS template
   //  template 0 is typically non-accum fields
   if( 0 == gfld->ipdtnum ){

      //  determine the time unit of the lead time and calculate it
      double sec_lead_unit = VarInfoGrib2::g2_time_range_unit_to_sec( (int)gfld->ipdtmpl[7] );
      if( 0 >= sec_lead_unit ){
         mlog << Error << "\nMetGrib2DataFile::data_plane() found unexpected lead time unit of "
              << gfld->ipdtmpl[7] << "\n\n";
         exit(1);
      }
      LeadTime = sec_lead_unit * gfld->ipdtmpl[8];

      //  set the forecast time information
      if( -1 == ValidTime )   ValidTime = InitTime  + LeadTime;
      if( -1 == InitTime  )   InitTime  = ValidTime - LeadTime;

   }

   //  template 8 is typically accum fields
   else if( 8 == gfld->ipdtnum ){

      ValidTime = mdyhms_to_unix(gfld->ipdtmpl[16], gfld->ipdtmpl[17], gfld->ipdtmpl[15],
                                 gfld->ipdtmpl[18], gfld->ipdtmpl[19], gfld->ipdtmpl[20]);

      if( -1 == InitTime ){
         mlog << Error << "\nMetGrib2DataFile::data_plane() - Init time not set when calculating "
              << "APCP valid time\n\n";
         exit(1);
      }

      LeadTime = ValidTime - InitTime;

   }
   plane.set_init  ( InitTime );
   plane.set_valid ( ValidTime );
   plane.set_lead  ( LeadTime );

   //  set the accumulation interval
   g2int range_typ = (8 == gfld->ipdtnum ? gfld->ipdtmpl[25] : 0);
   g2int range_val = (8 == gfld->ipdtnum ? gfld->ipdtmpl[26] : 0);
   double sec_accum_unit = VarInfoGrib2::g2_time_range_unit_to_sec( range_typ );
   plane.set_accum ( range_val * (int)sec_accum_unit );

   //  print a report
   double plane_min, plane_max;
   plane.data_range(plane_min, plane_max);
   mlog << Debug(4) << "\n"
        << "Data plane information:\n"
        << "    plane min: " << plane_min << "\n"
        << "    plane max: " << plane_max << "\n"
        << "    scan mode: " << ScanMode << "\n"
        << "   valid time: " << unix_to_yyyymmdd_hhmmss(ValidTime) << "\n"
        << "    lead time: " << sec_to_hhmmss(LeadTime)  << "\n"
        << "    init time: " << unix_to_yyyymmdd_hhmmss(InitTime)  << "\n"
        << "  bitmap flag: " << gfld->ibmap << "\n\n";

   return(true);
}
