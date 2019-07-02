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
#include "grib_strings.h"
#include "vx_data2d.h"
#include "vx_math.h"
#include "vx_log.h"

extern "C" {
  #include "grib2.h"
  void g2_miss( gribfield *gfld, float *rmiss, int *nmiss );
}

using namespace std;

////////////////////////////////////////////////////////////////////////

double scaled2dbl(int scale_factor, int scale_value);

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

   PairMap[ugrd_abbr_str] = vgrd_abbr_str;
   PairMap[vgrd_abbr_str] = ugrd_abbr_str;

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
   if( NULL == (FileGrib2 = met_fopen(Filename.c_str(), "r")) ){
      mlog << Error << "\nMetGrib2DataFile::open() -> "
           << "unable to open input GRIB2 file " << _filename << "\n\n";
      exit(1);
   }

   if( 1 > RecList.size() ) read_grib2_record_list();

   bool status = ( 0 < RecList.size() );
   if( !status ){
      mlog << Warning << "\nGRIB2 records not found in input file '"
           << Filename << "'\n\n";
   }

   return status;
}

////////////////////////////////////////////////////////////////////////

void MetGrib2DataFile::dump(ostream & out, int depth) const {
   Indent prefix(depth);

   out << prefix << "File = ";
   if ( Filename.empty() )  out << "(nul)\n";
   else                     out << '\"' << Filename << "\"\n";

   if ( Raw_Grid )  {
      out << prefix << "Raw Grid:\n";
      Raw_Grid->dump(out, depth + 1);
   } else {
      out << prefix << "No raw Grid!\n";
   }

   if ( Dest_Grid )  {
      out << prefix << "Dest Grid:\n";
      Dest_Grid->dump(out, depth + 1);
   } else {
      out << prefix << "No dest Grid!\n";
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

         mlog << Warning << "\nMetGrib2DataFile::data_plane() -> "
              << "No matching record found for '"
              << vinfo_g2->magic_str() << "'\n\n";
         return false;

      } else if( 1 < plane_array.n_planes() ){

         mlog << Warning << "\nMetGrib2DataFile::data_plane() -> "
              << "Wind speed/direction derivation found "
              << plane_array.n_planes()
              << " matching records found for '"
              << vinfo_g2->magic_str() << "'\n\n";

      }

      //  report the matched record
      mlog << Debug(3) << "MetGrib2DataFile::data_plane() -> "
           << "Found derived match for '"
           << vinfo_g2->magic_str() << "' in GRIB2 file '"
           << Filename << "'\n";

      plane = plane_array[0];

      process_data_plane(vinfo_g2, plane);

      return true;

   }  //  END: if( 1 > listMatch.size() )

   //  verify that a only single record was found
   if( 1 < listMatch.size() ){
      ConcatString msg;
      for(size_t i=0; i < listMatch.size(); i++) {
         msg << "record " << listMatch[i]->RecNum
             << " field " << listMatch[i]->FieldNum
             << ": ipdtmpl[" << listMatch[i]->IPDTmpl.n()
             << "] = ";
         for(int j=0; j < listMatch[i]->IPDTmpl.n(); j++) {
            msg << listMatch[i]->IPDTmpl[j];
            if(j < (listMatch[i]->IPDTmpl.n() - 1)) msg << ", ";
         }
         msg << "\n";
      }
      mlog << Warning << "\nMetGrib2DataFile::data_plane() -> "
           << "Using the first of " << listMatch.size()
           << " matching records for \""
           << vinfo_g2->magic_str() << "\":\n" << msg
           << "Try setting the 0-based \""
           << conf_key_GRIB2_ipdtmpl_index << "\" and \""
           << conf_key_GRIB2_ipdtmpl_val
           << "\" config file options to select one.\n\n";
   }

   //  report the matched record
   mlog << Debug(3) << "MetGrib2DataFile::data_plane() -> "
        << "Found exact match for '" << vinfo_g2->magic_str()
        << "' in GRIB2 record " << listMatch[0]->RecNum
        << " field " << listMatch[0]->FieldNum << " of GRIB2 file '"
        << Filename << "'\n";

   //  read the data plane for the matched record
   bool read_success = read_grib2_record_data_plane(listMatch[0],
                                                    plane);

   //  check the data plane for wind rotation
   plane = check_uv_rotation(vinfo_g2, listMatch[0], plane);

   if(read_success) process_data_plane(vinfo_g2, plane);

   return read_success;
}

////////////////////////////////////////////////////////////////////////

int MetGrib2DataFile::data_plane_array( VarInfo &vinfo,
                                        DataPlaneArray &plane_array ){

   // Initialize
   plane_array.clear();

   //  narrow the vinfo pointer
   VarInfoGrib2* vinfo_g2 = (VarInfoGrib2*)(&vinfo);

   //  search the list of records for matches
   vector<Grib2Record*> listMatchExact, listMatchRange, listRead;
   find_record_matches(vinfo_g2, listMatchExact, listMatchRange);

   //  if multiple exact matches were found, use the first one
   if( 1 <= listMatchExact.size() ){

      //  report on the exact match(es)
      if( 1 < listMatchExact.size() ){
         ConcatString msg;
         for(size_t i=0; i < listMatchExact.size(); i++) {
            msg << "record " << listMatchExact[i]->RecNum
                << " field " << listMatchExact[i]->FieldNum
                << ": ipdtmpl[" << listMatchExact[i]->IPDTmpl.n()
                << "] = ";
            for(int j=0; j < listMatchExact[i]->IPDTmpl.n(); j++) {
               msg << listMatchExact[i]->IPDTmpl[j];
               if(j < (listMatchExact[i]->IPDTmpl.n() - 1)) msg << ", ";
            }
            msg << "\n";
         }
         mlog << Warning << "\nMetGrib2DataFile::data_plane_array() -> "
              << "Using the first of " << listMatchExact.size()
              << " exact matching records for \""
              << vinfo_g2->magic_str() << "\":\n" << msg
              << "Try setting the 0-based \""
              << conf_key_GRIB2_ipdtmpl_index << "\" and \""
              << conf_key_GRIB2_ipdtmpl_val
              << "\" config file options to select one.\n\n";
      } else {
         mlog << Debug(3) << "MetGrib2DataFile::data_plane_array() -> "
              << "Found exact match for \"" << vinfo_g2->magic_str()
              << "\" in GRIB2 record " << listMatchExact[0]->RecNum
              << " field " << listMatchExact[0]->FieldNum
              << " of GRIB2 file \"" << Filename << "\"\n";
      }

      listRead.push_back( listMatchExact[0] );
   }

   //  otherwise, if range matches were found, use them
   else if( 0 < listMatchRange.size() ){

      ConcatString msg;
      for(size_t i=0; i < listMatchRange.size(); i++) {
         msg << (i ? ", " : "") << listMatchRange[i]->RecNum;
      }
      mlog << Debug(3) << "MetGrib2DataFile::data_plane_array() -> "
           << "Found range match for \"" << vinfo_g2->magic_str()
           << "\" in GRIB2 records " << msg << " of GRIB2 file \""
           << Filename << "\"\n";

      listRead = listMatchRange;
   }

   //  if nothing was found, try to build derived records
   else {

      plane_array = check_derived(vinfo_g2);

      //  if no matches were found, bail
      if( 1 > plane_array.n_planes() ){
         mlog << Warning << "\nMetGrib2DataFile::data_plane_array() -> "
              << "No matching records found for '"
              << vinfo_g2->magic_str() << "'\n\n";
         return 0;
      }

      //  report the matched record
      mlog << Debug(3) << "\nMetGrib2DataFile::data_plane_array() -> "
           << "Wind speed/direction derivation "
           << "found " << plane_array.n_planes()
           << " matching records found for '"
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
      double lvl_lower = (double)(*it)->LvlVal1;
      double lvl_upper = (double)(*it)->LvlVal2;
      if( LevelType_Pres == VarInfoGrib2::g2_lty_to_level_type((*it)->LvlTyp) ){
         lvl_lower = ( (double)(*it)->LvlVal1 ) / 100.0;
         lvl_upper = ( (double)(*it)->LvlVal2 ) / 100.0;
      }

      process_data_plane(vinfo_g2, plane);

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

   int vinfo_ens_type=bad_data_int, vinfo_ens_number=bad_data_int;
   ConcatString vinfo_ens = vinfo->ens();
   if( vinfo_ens.length() > 0){
      if( vinfo_ens == conf_key_grib_ens_hi_res_ctl ){
         vinfo_ens_type = 0;
         vinfo_ens_number = 0;
      } else if( vinfo_ens == conf_key_grib_ens_low_res_ctl ){
         vinfo_ens_type = 1;
         vinfo_ens_number = 0;
      } else {
         char sign = vinfo_ens.text()[0];
         if( sign == '+') {
            vinfo_ens_type = 3;
         } else if ( sign == '-' ) {
            vinfo_ens_type = 2;
         }
         char* ens_number_str  = new char[vinfo_ens.length()  ];
         strncpy(ens_number_str, vinfo_ens.text()+1, (size_t) vinfo_ens.length());
         ens_number_str[vinfo_ens.length()-1] = (char) 0;

         //  if the  string is numeric
         if( check_reg_exp("^[0-9]*$", ens_number_str) ) vinfo_ens_number = atoi(ens_number_str);
         delete[] ens_number_str;
      }

      // if one of the parameters was not set - error
      if( is_bad_data(vinfo_ens_number) || is_bad_data(vinfo_ens_type) ){
         mlog << Error << "\nfind_record_matches() -> "
              << "unrecognized GRIB_ens value '" << vinfo_ens << "' Should be '"
              << conf_key_grib_ens_hi_res_ctl << "' or '"
              << conf_key_grib_ens_low_res_ctl << "' or '+/-' followed by the number\n\n";
              exit(1);
      }
   }

   //  check each record for a match against the VarInfo
   for( vector<Grib2Record*>::iterator it = RecList.begin();
        it < RecList.end();
        it++ ) {

      bool rec_match_ex = false;
      bool rec_match_rn = false;

      double rec_lvl1 = min((double)((*it)->LvlVal1), (double)((*it)->LvlVal2));
      double rec_lvl2 = max((double)((*it)->LvlVal1), (double)((*it)->LvlVal2));

      //  test the timing information
      if( (!is_bad_data(vinfo->lead())  && vinfo->lead()  != (*it)->LeadTime)  ||
          (vinfo->valid()               && vinfo->valid() != (*it)->ValidTime) ||
          (vinfo->init()                && vinfo->init()  != (*it)->InitTime)  ){
         continue;
      }

      //  test additional config file options
      if( (!is_bad_data(vinfo->pdt())       && vinfo->pdt()       != (*it)->PdsTmpl  ) ||
          (!is_bad_data(vinfo->process())   && vinfo->process()   != (*it)->Process  ) ||
          (!is_bad_data(vinfo->ens_type())  && vinfo->ens_type()  != (*it)->EnsType  ) ||
          (!is_bad_data(vinfo->der_type())  && vinfo->der_type()  != (*it)->DerType  ) ||
          (!is_bad_data(vinfo->stat_type()) && vinfo->stat_type() != (*it)->StatType ) ){
         continue;
      }

      //  test ipdtmpl array values
      if(vinfo->n_ipdtmpl() > 0) {
         int i, j;
         bool skip = false;
         for(i=0; i<vinfo->n_ipdtmpl(); i++) {
            j = vinfo->ipdtmpl_index(i);
            if(j >= (*it)->IPDTmpl.n() ||
              (j < (*it)->IPDTmpl.n() &&
              (*it)->IPDTmpl[j] != vinfo->ipdtmpl_val(i))) {
               skip = true;
               break;
            }
         }
         if(skip) continue;
      }

      // if this is ensemble record - test for ensemble information
      if((*it)->PdsTmpl == 1 || (*it)->PdsTmpl == 11){

         int ens_type   = (*it)->EnsType;
         int ens_number = (*it)->EnsNumber;

         // if both of ens info properties are valid in vinfo - use them to match value
         if( !is_bad_data(vinfo_ens_number) && !is_bad_data(vinfo_ens_type)){
            ens_number = vinfo_ens_number;
            ens_type = vinfo_ens_type;
         }


         if( (*it)->EnsNumber != ens_number || (*it)->EnsType != ens_type){
            continue;
         }
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

         //  test the level type number, if specified
         if ( !is_bad_data(vinfo->level().type_num()) &&
              vinfo->level().type_num() != (*it)->LvlTyp ){

            mlog << Debug(4)
                 << "For GRIB2 record number " << (*it)->RecNum
                 << ", the requested level type (" << vinfo->level().type_num()
                 << ") does not match the current level type ("
                 << (*it)->LvlTyp << ").\n";

            continue;
         }

         //  record number level type
         if( LevelType_RecNumber == vinfo_lty && is_eq(lvl1, (*it)->RecNum) ){
            rec_match_ex = true;
         }

         //  generic level type
         else if( LevelType_None == vinfo_lty ){
            rec_match_ex = ( is_eq(lvl1, rec_lvl1) && is_eq(lvl2, rec_lvl2) );
            rec_match_rn = ( !is_eq(lvl1, lvl2) && lvl1 <= rec_lvl1 && lvl2 >= rec_lvl2 );
         }

         //  accumulation level type
         else if( LevelType_Accum == vinfo_lty ){
            rec_match_ex = ( is_eq(lvl1, (*it)->Accum) );
         }

         //  pressure or vertical level type
         else if( vinfo_lty == vinfo->g2_lty_to_level_type((*it)->LvlTyp) ){
            if( LevelType_Pres == vinfo_lty ){
               rec_lvl1 /= 100;
               rec_lvl2 /= 100;
            }
            rec_match_ex = ( is_eq(lvl1, rec_lvl1) && is_eq(lvl2, rec_lvl2) );
            rec_match_rn = ( !is_eq(lvl1, lvl2) && lvl1 <= rec_lvl1 && lvl2 >= rec_lvl2 );
         }  //  END: if( level match )

         //  if seeking a probabilistic field, check the prob info
         if( (rec_match_ex || rec_match_rn) && vinfo->p_flag() && (*it)->ProbFlag ){

            rec_match_ex = rec_match_rn = false;

            SingleThresh v_thr_lo = vinfo->p_thresh_lo();
            SingleThresh v_thr_hi = vinfo->p_thresh_hi();

            //  if both thresholds are in effect, check both values
            if( !is_bad_data(v_thr_lo.get_value()) &&
                !is_bad_data(v_thr_hi.get_value()) ){
               rec_match_ex = ( is_eq(v_thr_lo.get_value(), (*it)->ProbLower) &&
                                is_eq(v_thr_hi.get_value(), (*it)->ProbUpper) &&
                                2 == (*it)->ProbType
                              );
            }

            //  compare the single upper threshold case
            else if( !is_bad_data(v_thr_hi.get_value()) ){
               rec_match_ex = ( 0 == (*it)->ProbType &&
                                is_eq(v_thr_hi.get_value(), (*it)->ProbLower) ) ||
                              ( 4 == (*it)->ProbType &&
                                is_eq(v_thr_hi.get_value(), (*it)->ProbUpper) );
            }

            //  compare the single lower threshold case
            else if( !is_bad_data(v_thr_lo.get_value()) ){
               rec_match_ex = ( 1 == (*it)->ProbType &&
                                is_eq(v_thr_lo.get_value(), (*it)->ProbUpper) ) ||
                              ( 3 == (*it)->ProbType &&
                                is_eq(v_thr_lo.get_value(), (*it)->ProbLower) );
            }

         }
      }  //  END: else if( parameter match )

      //  add the record to the result lists, depending on the match type
      if( rec_match_ex )                 listMatchExact.push_back(*it);
      if( rec_match_ex || rec_match_rn ) listMatchRange.push_back(*it);

   }  //  END:  for( vector<Grib2Record*>::iterator it = RecList.begin(); ...)

   return;
}

////////////////////////////////////////////////////////////////////////

DataPlane MetGrib2DataFile::check_uv_rotation(VarInfoGrib2 *vinfo, Grib2Record *rec, DataPlane plane){

   //  check that the field is present in the pair map
   string parm_name = vinfo->name().text();
   if( 0 == PairMap.count( parm_name ) ||
       0 == (rec->ResCompFlag & 8) ) {
      return plane;
   }

   //  build the magic string of the pair field, and check it
   ConcatString pair_mag = build_magic( rec );
   pair_mag.replace(parm_name.data(), PairMap[parm_name].data());
   if( 0 == NameRecMap.count( string(pair_mag.text()) ) ){
      mlog << Debug(3) << "MetGrib2DataFile::check_uv_rotation -> "
           << "UV rotation pair record not found: '" << pair_mag
           << "'\n";
      return plane;
   }

   //  read the data plane for the pair record
   Grib2Record *rec_pair = NameRecMap[pair_mag.text()];
   DataPlane plane_pair;
   read_grib2_record_data_plane(NameRecMap[pair_mag.text()],
                                plane_pair);

   mlog << Debug(3) << "MetGrib2DataFile::check_uv_rotation() -> "
        << "Found pair match \"" << pair_mag << "\" in GRIB2 record "
        << rec_pair->RecNum << " field " << rec_pair->FieldNum << "\n";

   //  rotate the winds
   DataPlane u2d, v2d, u2d_rot, v2d_rot;
   if( 'U' == parm_name.at(0) ){ u2d = plane;  v2d = plane_pair; }
   else                        { v2d = plane;  u2d = plane_pair; }
   rotate_uv_grid_to_earth(u2d, v2d, *Raw_Grid, u2d_rot, v2d_rot);
   if( 'U' == parm_name.at(0) ){ plane = u2d_rot; }
   else                        { plane = v2d_rot; }

   return plane;
}

////////////////////////////////////////////////////////////////////////

DataPlaneArray MetGrib2DataFile::check_derived( VarInfoGrib2 *vinfo ){
   DataPlaneArray array_ret;

   //  if the requested field cannot be derived, bail
   if( vinfo->name() != "WIND" &&
       vinfo->name() != "WDIR" ) {
      return array_ret;
   }

   //  read the data_plane objects for each constituent
   DataPlaneArray array_u, array_v;
   VarInfoGrib2 vinfo_cons(*vinfo);
   vinfo_cons.set_name( ugrd_abbr_str );
   data_plane_array(vinfo_cons, array_u);
   vinfo_cons.set_name( vgrd_abbr_str );
   data_plane_array(vinfo_cons, array_v);

   //  derive wind speed or direction
   if( array_u.n_planes() != array_v.n_planes() ){
      mlog << Warning << "\nMetGrib2DataFile::data_plane_array() -> "
           << "when deriving winds, the number of U-wind records ("
           << array_u.n_planes()
           << ") does not match the number of V-wind record ("
           << array_v.n_planes()
           << ") for GRIB2 file '" << filename() << "'\n\n";

      return array_ret;
   }

   //  loop through each of the data planes
   for(int i=0; i < array_u.n_planes(); i++) {

      //  check that the current level values match
      if( !is_eq(array_u.lower(i), array_v.lower(i)) ||
          !is_eq(array_u.upper(i), array_v.upper(i)) ){

         mlog << Warning << "\nMetGrib1DataFile::data_plane_array() -> "
              << "when deriving winds for level " << i+1
              << ", the U-wind levels (" << array_u.lower(i)
              << ", " << array_u.upper(i)
              << ") do not match the V-wind levels ("
              << array_v.lower(i) << ", " << array_v.upper(i)
              << ") in GRIB file '" << filename() << "'\n\n";

         return array_ret;
      }

      //  perform the derivation
      DataPlane plane_deriv;
      if(vinfo->name() == "WIND") derive_wind(array_u[i], array_v[i],
                                              plane_deriv);
      else                        derive_wdir(array_u[i], array_v[i],
                                              plane_deriv);

      //  add the current data plane
      array_ret.add(plane_deriv, array_u.lower(i), array_u.upper(i));

   }

   return array_ret;
}

////////////////////////////////////////////////////////////////////////

void MetGrib2DataFile::read_grib2_record_list() {
   gribfield  *gfld;
   long offset = 0, offset_next;
   g2int numfields;
   int idx = 0, rec_num = 1;

   //  read all the records into the record list, pulling grid information from the first
   while( 0 <= (offset_next = read_grib2_record(offset, 0, 1, gfld, numfields)) ){

      //  read the grid information, if necessary
      if( !Raw_Grid || 1 > Raw_Grid->nx() || 1 > Raw_Grid->ny() ) read_grib2_grid(gfld);

      //  treat each field of the record as a separate record
      for(int i=1; i <= numfields; i++){

         //  validate the PDS template number
         if(  0 != gfld->ipdtnum &&     //  analysis or forecast
              1 != gfld->ipdtnum &&     //  individual ensemble forecast, control and perturbed, at a horizontal level or in a horizontal layer at a point in time
              2 != gfld->ipdtnum &&     //  ensemble mean
              5 != gfld->ipdtnum &&     //  probability forecast
              8 != gfld->ipdtnum &&     //  accumulation forecast
              9 != gfld->ipdtnum &&     //  probabilistic accumulation forecast
             11 != gfld->ipdtnum &&     //  individual ensemble forecast, control and perturbed, at a horizontal level or in a horizontal layer, in a continuous or non-continuous time interval
             12 != gfld->ipdtnum &&     //  derived accumulation forecast (?)
             46 != gfld->ipdtnum &&     //  average, accumulation, and/or extreme values or other statistically processed values at a horizontal level or in a horizontal layer in a continuous or non-continuous time interval for aerosol.
             48 != gfld->ipdtnum ){     //  analysis or forecast at a horizontal level or in a horizontal layer at a point in time for aerosol.
            mlog << Error << "\nMetGrib2DataFile::data_plane() -> "
                 << "PDS template number ("
                 << gfld->ipdtnum << ") is not supported. "
                 << "Please email met_help@ucar.edu.\n\n";
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
         rec->Process      = gfld->ipdtmpl[2];

         //  get the level type
         if( gfld->ipdtnum == 46 ) {
            rec->LvlTyp    = gfld->ipdtmpl[15];
         } else {
            rec->LvlTyp    = gfld->ipdtmpl[9];
         }

         //  store the full pdtmpl values
         for(int j=0; j < gfld->ipdtlen; j++){ rec->IPDTmpl.add((int) gfld->ipdtmpl[j]); }

         //  check for template number 46
         if( gfld->ipdtnum == 46 ) {
            rec->LvlVal1 = scaled2dbl(gfld->ipdtmpl[16], gfld->ipdtmpl[17]);
            rec->LvlVal2 = rec->LvlVal1;

         //  check for special fixed level types (1 through 10 or 101) and set the level values to 0
         //  Reference: https://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc/grib2_table4-5.shtml
         } else if( (rec->LvlTyp >= 1 && rec->LvlTyp <= 10) || rec->LvlTyp == 101 ) {
            rec->LvlVal1 = 0;
            rec->LvlVal2 = 0;
         } else {
            rec->LvlVal1 = scaled2dbl(gfld->ipdtmpl[10], gfld->ipdtmpl[11]);
            rec->LvlVal2 = ( 255 != gfld->ipdtmpl[12] ?
                             scaled2dbl(gfld->ipdtmpl[13], gfld->ipdtmpl[14]) :
                             rec->LvlVal1 );
         }

         rec->RangeTyp     = (8 == gfld->ipdtnum || 12 == gfld->ipdtnum ? gfld->ipdtmpl[25] : 0);
         rec->RangeVal     = (8 == gfld->ipdtnum || 12 == gfld->ipdtnum ? gfld->ipdtmpl[26] : 0);
         rec->ResCompFlag  = gfld->igdtmpl[ 0 == gfld->igdtnum ? 13 : 11 ];

         //  initialize the forecast time information
         rec->ValidTime = -1;
         rec->InitTime  = -1;
         rec->LeadTime  = -1;
         unixtime ref_time = mdyhms_to_unix(gfld->idsect[6], gfld->idsect[7], gfld->idsect[5],
                                            gfld->idsect[8], gfld->idsect[9], gfld->idsect[10]);

         //  parse the time reference indicator (Table 1.2)
         switch( gfld->idsect[4] ){
            case 0:                                                 //  Analysis: interpret as the
                                                                    //  model initialization time
            case 1:     rec->InitTime  = ref_time;      break;      //  Start of Forecast
            case 2:     rec->ValidTime = ref_time;      break;      //  Verifying Time of Forecast
            case 3:     rec->ValidTime = ref_time;      break;      //  Observation Time
            default:
               mlog << Error << "\nMetGrib2DataFile::read_grib2_record_list() -> "
                    << "found unexpected time reference indicator of "
                    << gfld->ipdtmpl[4] << ".\n\n";
               exit(1);
         }

         //  ensemble type and number for templates 1 and 11 (Table 4.6)
         if( 1 == gfld->ipdtnum || 11 == gfld->ipdtnum ){
            rec->EnsType   = gfld->ipdtmpl[15];
            rec->EnsNumber = gfld->ipdtmpl[16];
         }

         //  derived type and number for templates 2 to 4 and 12 to 14 (Table 4.7)
         if( (  2 <= gfld->ipdtnum &&  4 >= gfld->ipdtnum ) ||
             ( 12 <= gfld->ipdtnum && 14 >= gfld->ipdtnum ) ){
            rec->DerType = gfld->ipdtmpl[15];
         }

         //  statistical processing type for template 8 (Table 4.10)
         if( 8 == gfld->ipdtnum ){
            rec->StatType = gfld->ipdtmpl[23];
         }

         //  depending on the template number, determine the reference times
         if( 8 <= gfld->ipdtnum && 12 >= gfld->ipdtnum ){

            if( -1 != rec->ValidTime ){
               mlog << Error << "\nMetGrib2DataFile::read_grib2_record_list() -> "
                    << "accum valid time unexpectedly set for record " << rec->RecNum
                    << " field " << rec->FieldNum << "\n\n";
               exit(1);
            }

            switch(gfld->ipdtnum){
               case 8:
                  rec->ValidTime = mdyhms_to_unix(gfld->ipdtmpl[16], gfld->ipdtmpl[17], gfld->ipdtmpl[15],
                                                  gfld->ipdtmpl[18], gfld->ipdtmpl[19], gfld->ipdtmpl[20]);
                  break;
               case 9:
                  rec->ValidTime = mdyhms_to_unix(gfld->ipdtmpl[23], gfld->ipdtmpl[24], gfld->ipdtmpl[22],
                                                  gfld->ipdtmpl[25], gfld->ipdtmpl[26], gfld->ipdtmpl[27]);
                  break;
               case 10:
                  rec->ValidTime = mdyhms_to_unix(gfld->ipdtmpl[17], gfld->ipdtmpl[18], gfld->ipdtmpl[16],
                                                  gfld->ipdtmpl[19], gfld->ipdtmpl[20], gfld->ipdtmpl[21]);
                  break;
               case 11:
                  rec->ValidTime = mdyhms_to_unix(gfld->ipdtmpl[19], gfld->ipdtmpl[20], gfld->ipdtmpl[18],
                                                  gfld->ipdtmpl[21], gfld->ipdtmpl[22], gfld->ipdtmpl[23]);
                  break;
               case 12:
                  rec->ValidTime = mdyhms_to_unix(gfld->ipdtmpl[18], gfld->ipdtmpl[19], gfld->ipdtmpl[17],
                                                  gfld->ipdtmpl[20], gfld->ipdtmpl[21], gfld->ipdtmpl[22]);
                  break;
            }
            rec->LeadTime = rec->ValidTime - rec->InitTime;

         //  https://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc/grib2_temp4-46.shtml
         } else if ( 46 == gfld->ipdtnum ) {

            // ValidTime is the end of the overall time interval
            rec->ValidTime = mdyhms_to_unix(gfld->ipdtmpl[22], gfld->ipdtmpl[23], gfld->ipdtmpl[21],
                                            gfld->ipdtmpl[24], gfld->ipdtmpl[25], gfld->ipdtmpl[26]);

            //  set the forecast time information
            if ( -1 == rec->LeadTime )   rec->LeadTime = rec->ValidTime - rec->InitTime;

         } else {

            //  determine the index for the time unit and forecast time
            int i_time_unit, i_fcst_time;
            switch(gfld->ipdtnum){

               //  https://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc/grib2_temp4-48.shtml
               case 48:
                  i_time_unit = 18;
                  i_fcst_time = 19;
                  break;
               default:
                  i_time_unit = 7;
                  i_fcst_time = 8;
                  break;
            }

            //  determine the time unit of the lead time and calculate it
            double sec_lead_unit = VarInfoGrib2::g2_time_range_unit_to_sec( (int)gfld->ipdtmpl[i_time_unit] );
            if( 0 >= sec_lead_unit ){
               mlog << Error << "\nMetGrib2DataFile::read_grib2_record_list() -> "
                    << "found unexpected lead time unit of " << gfld->ipdtmpl[i_time_unit] << "\n\n";
               exit(1);
            }
            rec->LeadTime = nint(sec_lead_unit * gfld->ipdtmpl[i_fcst_time]);

            //  set the forecast time information
            if     ( -1 == rec->ValidTime )   rec->ValidTime = rec->InitTime  + rec->LeadTime;
            else if( -1 == rec->InitTime  )   rec->InitTime  = rec->ValidTime - rec->LeadTime;

         }

         //  store the probability information, if appropriate
         if( 5 == gfld->ipdtnum || 9 == gfld->ipdtnum ){
            rec->ProbFlag  = true;
            rec->ProbType  = gfld->ipdtmpl[17];
            rec->ProbLower = (double)(gfld->ipdtmpl[19]) / pow( 10, (double)(gfld->ipdtmpl[18]) );
            rec->ProbUpper = (double)(gfld->ipdtmpl[21]) / pow( 10, (double)(gfld->ipdtmpl[20]) );
         } else {
            rec->ProbFlag  = false;
            rec->ProbType  = -1;
            rec->ProbLower = bad_data_double;
            rec->ProbUpper = bad_data_double;
         }

         //  set the accumulation interval
         g2int range_typ = ( 8 == gfld->ipdtnum ? gfld->ipdtmpl[25] :
                             9 == gfld->ipdtnum ? gfld->ipdtmpl[32] :
                            10 == gfld->ipdtnum ? gfld->ipdtmpl[26] :
                            11 == gfld->ipdtnum ? gfld->ipdtmpl[28] :
                            12 == gfld->ipdtnum ? gfld->ipdtmpl[27] : 0);
         g2int range_val = ( 8 == gfld->ipdtnum ? gfld->ipdtmpl[26] :
                             9 == gfld->ipdtnum ? gfld->ipdtmpl[33] :
                            10 == gfld->ipdtnum ? gfld->ipdtmpl[27] :
                            11 == gfld->ipdtnum ? gfld->ipdtmpl[29] :
                            12 == gfld->ipdtnum ? gfld->ipdtmpl[28] : 0);
         double sec_accum_unit = VarInfoGrib2::g2_time_range_unit_to_sec( range_typ );
         rec->Accum = range_val * (int)sec_accum_unit;

         //  build the parameter "name"
         ConcatString id;
         id << rec->Discipline << "_" << rec->ParmCat << "_" << rec->Parm;

         //  use the index to look up the parameter name
         Grib2TableEntry tab;
         if( !GribTable.lookup_grib2(rec->Discipline, rec->ParmCat, rec->Parm,
                                     gfld->idsect[2], gfld->idsect[0], gfld->idsect[3], tab) ){
            mlog << Debug(4) << "MetGrib2DataFile::read_grib2_record_list() - unrecognized GRIB2 "
                 << "field indexes - disc: " << rec->Discipline << ", master table: " << gfld->idsect[2]
                 << ", center: " << gfld->idsect[0] << ", local table: " << gfld->idsect[3]
                 << ", parm_cat: " << rec->ParmCat << ", parm: " << rec->Parm << "\n";
            rec->ParmName = str_format("DISC%d_CAT%d_PARM%d", rec->Discipline, rec->ParmCat, rec->Parm);
         } else {
            rec->ParmName = tab.parm_name.text();
         }

         //  add the record to the list
         RecList.push_back(rec);

         //  build data structure for U/V wind pairs
         string rec_mag = build_magic(rec).text();
         NameRecMap[rec_mag] = rec;

         g2_free(gfld);

         //  if there are more fields in the current record, read the next one
         if( i < numfields ) read_grib2_record(offset, 0, i+1, gfld, numfields);

      }  //  END:  for(int i=1; i <= numfields; i++)

      //  set for the next record
      rec_num++;
      offset = offset_next;

   }  //  END:  while( read_grib2_record() )

   return;
}

////////////////////////////////////////////////////////////////////////

void MetGrib2DataFile::read_grib2_grid( gribfield *gfld) {

   double d;
   int ResCompFlag;
   char hem = 0;

   Raw_Grid = new Grid();

   //  determine the radius of the earth
   double r_km = -1;
   switch( gfld->igdtmpl[0] ){
      case 0:     r_km = 6367.470;  break;
      //  parse earth radius from header
      case 1:     r_km = gfld->igdtmpl[2] / 1000.0; break;
      case 6:     r_km = 6371.229;  break;
      default:
         mlog << Debug(3)
              << "Unexpected earth shape (" << gfld->igdtmpl[0]
              << ").  Assuming spherical earth with radius "
              << grib_earth_radius_km
              << " km for internal consistentcy.\n";
         r_km = grib_earth_radius_km;
   }

   //  switch radius of the earth for internal consistency
   if( !is_eq(r_km, grib_earth_radius_km) ) {
      mlog << Debug(4)
           << "Switching the GRIB2 radius of the earth value of "
           << r_km << " km to " << grib_earth_radius_km
           << " km for internal consistency.\n";
      r_km = grib_earth_radius_km;
   }

   //////////////////////////////////////////////////////////////////////////
   //  lat/long
   if( 0 == gfld->igdtnum ){

      ScanMode    = gfld->igdtmpl[18];
      ResCompFlag = gfld->igdtmpl[13];

      //  build a LatLonData struct with the projection information
      LatLonData data;
      data.name         = latlon_proj_type;
      data.Nlat         = gfld->igdtmpl[8];
      data.Nlon         = gfld->igdtmpl[7];
      data.lat_ll       = ((double)gfld->igdtmpl[11] / 1000000.0);
      data.lon_ll       = -1.0*rescale_lon( (double)gfld->igdtmpl[12] / 1000000.0 );

      //  latitudinal increment.  If not given, compute from lat1 and lat2
      if( ResCompFlag & 16 ) {
         data.delta_lat = (double)gfld->igdtmpl[17] / 1000000.0;
      }
      else {
         d = fabs(((double)gfld->igdtmpl[11] / 1000000.0)
                - ((double)gfld->igdtmpl[14] / 1000000.0));
         data.delta_lat = d/(data.Nlat);
      }

      //  longitudinal increment.  If not given, compute from lon1 and lon2
      if( ResCompFlag & 32 ) {
         data.delta_lon = (double)gfld->igdtmpl[16] / 1000000.0;
      }
      else {
         d = fabs(((double)gfld->igdtmpl[12] / 1000000.0)
                - ((double)gfld->igdtmpl[15] / 1000000.0));
         data.delta_lon = d/(data.Nlon);
      }

      //  if y scan order is -y, move lat_ll
      if( !(ScanMode & 64) )
         data.lat_ll -= (data.Nlat - 1) * data.delta_lat;

      //  if x scan order is -x, move lon_ll
      if( ScanMode & 128 )
         data.lon_ll = rescale_lon(data.lon_ll - (data.Nlon - 1) * data.delta_lon);

      //  store the grid information
      Raw_Grid->set(data);

      copy_raw_grid_to_dest();

      data.dump();

   }

   //////////////////////////////////////////////////////////////////////////
   //  rotated lat/lon
   else if( 1 == gfld->igdtnum )  {

      const g2int * t = gfld->igdtmpl;
      const double angle_factor = (1.0/1000000.0);

      ScanMode    = t[18];
      ResCompFlag = t[13];

         //
         //  build a RotatedLatLonData struct with the projection information
         //

      RotatedLatLonData data;

      data.name        = rotated_latlon_proj_type;
      data.Nlat        = t[8];
      data.Nlon        = t[7];
      data.rot_lat_ll  = (t[11])*angle_factor;
      data.rot_lon_ll  = -1.0*rescale_lon( (t[12])*angle_factor );

         //
         //  latitude increment.  If not given, compute from lat1 and lat2
         //

      if( ResCompFlag & 16 ) {
         data.delta_rot_lat = (t[17])*angle_factor;
      }
      else {
         d = fabs( ((t[11])*angle_factor) - ((t[14])*angle_factor) );
         data.delta_rot_lat = d/(data.Nlat);
      }

         //
         //  longitude increment.  If not given, compute from lon1 and lon2
         //

      if( ResCompFlag & 32 ) {
         data.delta_rot_lon = (t[16])*angle_factor;
      }
      else {
         d = fabs( ((t[12])*angle_factor) - ((t[15])*angle_factor) );
         data.delta_rot_lon = d/(data.Nlon);
      }

         //
         //  location of (rotated) south pole
         //

      double s_lat, s_lon;

      s_lat = (t[19])*angle_factor;

      s_lon = (t[20])*angle_factor;

      s_lon = -s_lon;   //  west longitude positive

      data.true_lat_south_pole = s_lat;

      data.true_lon_south_pole = s_lon;

         //
         //  auxilliary rotation around the rotated polar axis
         //

      data.aux_rotation = (t[21])*angle_factor;

         //
         //  if y scan order is -y, move lat_ll
         //

      if( !(ScanMode & 64) )
         data.rot_lat_ll -= (data.Nlat - 1) * data.delta_rot_lat;

         //
         //  if x scan order is -x, move lon_ll
         //

      if( ScanMode & 128 )
         data.rot_lon_ll = rescale_lon(data.rot_lon_ll - (data.Nlon - 1) * data.delta_rot_lon);

         //
         //  store the grid information
         //

      Raw_Grid->set(data);

      copy_raw_grid_to_dest();

      data.dump();

   }   //  else

   //////////////////////////////////////////////////////////////////////////
   //  polar stereographic
   else if( 20 == gfld->igdtnum ){

      ScanMode = gfld->igdtmpl[17];

      //  determine the hemisphere
      if( gfld->igdtmpl[16] & 128 ) hem = 'S';
      else                          hem = 'N';

      //  build a StereographicData struct with the projection information
      StereographicData data;
      data.name         = stereographic_proj_type;
      data.hemisphere   = hem;
      data.scale_lat    = (double)gfld->igdtmpl[12] / 1000000.0;
      data.lat_pin      = (double)gfld->igdtmpl[9] / 1000000.0;
      data.lon_pin      = -1.0*rescale_lon( (double)gfld->igdtmpl[10] / 1000000.0 );
      data.x_pin        = 0;
      data.y_pin        = 0;
      data.lon_orient   = -1.0*rescale_lon( (double)gfld->igdtmpl[13] / 1000000.0 );
      data.d_km         = (double)gfld->igdtmpl[14] / 1000000.0;
      data.r_km         = r_km;
      data.nx           = gfld->igdtmpl[7];
      data.ny           = gfld->igdtmpl[8];

      //  check for dx != dy
      if( !is_eq((double)gfld->igdtmpl[14] / 1000000.0,
                 (double)gfld->igdtmpl[15] / 1000000.0) ){
         mlog << Warning << "\nMetGrib2DataFile::read_grib2_grid() -> "
              << "MET does not currently support Polar Stereographic grids where dx ("
              << (double)gfld->igdtmpl[14] / 1000000.0 << ") != dy ("
              << (double)gfld->igdtmpl[15] / 1000000.0
              << ") and may produce unexpected results!\n\n";
      }

      //  store the grid information
      Raw_Grid->set(data);

      copy_raw_grid_to_dest();

      data.dump();

   }

   //  mercator
   else if( 10 == gfld->igdtnum ){

      ScanMode = gfld->igdtmpl[15];

      MercatorData data;
      data.name   = mercator_proj_type;
      data.lat_ll = (double)gfld->igdtmpl[9] / 1000000.0;
      data.lon_ll = -1.0*rescale_lon( (double)gfld->igdtmpl[10] / 1000000.0 );
      data.lat_ur = (double)gfld->igdtmpl[13] / 1000000.0;
      data.lon_ur = -1.0*rescale_lon( (double)gfld->igdtmpl[14] / 1000000.0 );
      data.nx     = gfld->igdtmpl[7];
      data.ny     = gfld->igdtmpl[8];

      //  store the grid information
      Raw_Grid->set(data);

      copy_raw_grid_to_dest();

      data.dump();

   }

   //  lambert conformal
   else if( 30 == gfld->igdtnum ){

      ScanMode = gfld->igdtmpl[17];

      //  determine the hemisphere
      if( gfld->igdtmpl[16] & 128 ) hem = 'S';
      else                          hem = 'N';

      //  build a LambertData struct with the projection information
      LambertData data;
      data.name         = lambert_proj_type;
      data.hemisphere   = hem;
      data.scale_lat_1  = (double)gfld->igdtmpl[18] / 1000000.0;
      data.scale_lat_2  = (double)gfld->igdtmpl[19] / 1000000.0;
      data.lat_pin      = (double)gfld->igdtmpl[9]  / 1000000.0;
      data.lon_pin      = -1.0*rescale_lon( (double)gfld->igdtmpl[10] / 1000000.0 );
      data.x_pin        = 0;
      data.y_pin        = 0;
      data.lon_orient   = -1.0*rescale_lon( (double)gfld->igdtmpl[13] / 1000000.0 );
      data.d_km         = (double)gfld->igdtmpl[14] / 1000000.0;
      data.r_km         = 6371.20;
      data.nx           = gfld->igdtmpl[7];
      data.ny           = gfld->igdtmpl[8];
      data.so2_angle    = 0.0;

      //  check for dx != dy
      if( !is_eq((double)gfld->igdtmpl[14] / 1000000.0,
                 (double)gfld->igdtmpl[15] / 1000000.0) ){
         mlog << Warning << "\nMetGrib2DataFile::read_grib2_grid() -> "
              << "MET does not currently support Lambert Conformal grids where dx ("
              << (double)gfld->igdtmpl[14] / 1000000.0 << ") != dy ("
              << (double)gfld->igdtmpl[15] / 1000000.0
              << ") and may produce unexpected results!\n\n";
      }

      //  store the grid information
      Raw_Grid->set(data);

      copy_raw_grid_to_dest();

      data.dump();

   }

   //  Gaussian lat/lon
   else if ( gfld->igdtnum == 40 )  {

      GaussianData gauss;
      const g2int * p = gfld->igdtmpl;

      gauss.name = gaussian_proj_type;

      ScanMode = p[18];

         //  check that the earth is spherical, not oblate

      gauss.lon_zero = -1.0*rescale_lon( (double)gfld->igdtmpl[12] / 1000000.0 );


      gauss.nx = p[7];
      gauss.ny = p[8];

         //  store the grid information

      Raw_Grid->set(gauss);

      copy_raw_grid_to_dest();

      gauss.dump();

   }   //  gaussian


   //  unrecognized grid
   else {

      mlog << Error << "\nMetGrib2DataFile::read_grib2_grid() -> "
           << "found unrecognized grid definition (" << gfld->igdtnum << ")\n\n";
      exit(1);

   }

   return;
}

////////////////////////////////////////////////////////////////////////

bool MetGrib2DataFile::read_grib2_record_data_plane(Grib2Record *rec,
                                                     DataPlane &plane) {

   //  attempt to read the record
   gribfield *gfld;
   unsigned char *cgrib;
   g2int numfields;
   float v, v_miss[2];
   int n_miss, i;
   if( -1 == read_grib2_record(rec->ByteOffset, 1, rec->FieldNum, gfld,
                               numfields) ){
      mlog << Error
           << "\nMetGrib2DataFile::read_grib2_record_data_plane() -> "
           << "failed to read record at offset " << rec->ByteOffset
           << " and field number " << rec->FieldNum << "\n\n";
      exit(1);
   }

   //  ensure the grid has been read, and initialize the grid size
   if( !Raw_Grid || 1 > Raw_Grid->nx() || 1 > Raw_Grid->ny() ) {
      read_grib2_grid(gfld);
   }
   int n_x = Raw_Grid->nx();
   int n_y = Raw_Grid->ny();

   //  determine whether or not the data bitmap applies
   bool apply_bmap = ( 0 == gfld->ibmap || 254 == gfld->ibmap );

   //
   //  * * * *  read grid data  * * * *
   //

   //  set up the DataPlane object
   plane.clear();
   plane.set_size(n_x, n_y);

   //  get the missing data value(s), if specified
   g2_miss(gfld, v_miss, &n_miss);

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
         case 80:  /* 0101 0000 */ idx_data = ( y % 2 == 0 ?
                                                y*n_x + x :
                                                y*n_x + (n_x - x - 1) );           break;
         default:
            mlog << Error << "\nMetGrib2DataFile::data_plane() -> "
                 << "found unrecognized ScanMode (" << ScanMode << ")\n\n";
            exit(1);
         }

         //  check bitmap for bad data
         v = (!apply_bmap || 0 != gfld->bmap[idx_data] ?
              (float)gfld->fld[idx_data] : bad_data_float);

         //  check missing data values, if specified
         for(i=0; i < n_miss; i++) {
            if(is_eq(v, v_miss[i])) { v = bad_data_float; break; }
         }

         //  set the current data value
         plane.set(v, x, y);
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
        << "      plane min: " << plane_min << "\n"
        << "      plane max: " << plane_max << "\n"
        << "      scan mode: " << ScanMode << "\n"
        << "     valid time: " << unix_to_yyyymmdd_hhmmss(rec->ValidTime) << "\n"
        << "      lead time: " << sec_to_hhmmss(rec->LeadTime)  << "\n"
        << "      init time: " << unix_to_yyyymmdd_hhmmss(rec->InitTime)  << "\n"
        << "    bitmap flag: " << gfld->ibmap << "\n";
   for(i=0; i < n_miss; i++) {
      mlog << Debug(4)
           << " missing val(" << i+1 << "): " << v_miss[i] << "\n";
   }
   mlog << Debug(4) << "\n";

   g2_free(gfld);

   return true;
}

////////////////////////////////////////////////////////////////////////

long MetGrib2DataFile::read_grib2_record(long offset, g2int unpack,
                                         g2int ifld, gribfield* &gfld,
                                         g2int &numfields) {

   //  the following code was lifted and modified from:
   //  http://www.nco.ncep.noaa.gov/pmb/docs/grib2/download/g2clib.documentation

   //  g2c fields
   g2int listsec0[3], listsec1[13], numlocal, lskip, lgrib;

   //  find the next record and read the info, return -1 if fail
   seekgb(FileGrib2, offset, 32000, &lskip, &lgrib);
   if(lgrib == 0) return -1;

   //  allocate memory to store the record
   unsigned char * cgrib = new unsigned char[lgrib];
   fseek(FileGrib2, lskip, SEEK_SET);
   fread(cgrib, sizeof(unsigned char), lgrib, FileGrib2);

   if(g2_info(cgrib, listsec0, listsec1, &numfields, &numlocal)) {
      if(cgrib) { delete [] cgrib; cgrib = (unsigned char *) 0; }
      return -1;
   }

   //  read the specified field in the record
   g2_getfld(cgrib, ifld, unpack, 1, &gfld);

   //  cleanup
   if(cgrib) { delete [] cgrib; cgrib = (unsigned char *) 0; }

   //  return the offset of the next record
   return lskip + lgrib;
}

////////////////////////////////////////////////////////////////////////

ConcatString MetGrib2DataFile::build_magic(Grib2Record *rec){
   ConcatString lvl;
   int lvl_val1 = (int)rec->LvlVal1, lvl_val2 = (int)rec->LvlVal2;
   switch( VarInfoGrib2::g2_lty_to_level_type(rec->LvlTyp) ){
      case LevelType_Accum:
         lvl = "A";
         lvl_val1 = rec->RangeVal * (int)VarInfoGrib2::g2_time_range_unit_to_sec(rec->RangeTyp);
         lvl_val1 = atoi( sec_to_hhmmss( lvl_val1 ).c_str() );
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
   if( is_eq(rec->LvlVal1, rec->LvlVal2) ){
      ret.format("%s/%s%d", rec->ParmName.c_str(), lvl.text(), lvl_val1);
   } else {
      ret.format("%s/%s%d-%d", rec->ParmName.c_str(), lvl.text(), lvl_val1, lvl_val2);
   }

   return ret;
}

////////////////////////////////////////////////////////////////////////

int MetGrib2DataFile::index( VarInfo &vinfo ){
   vector<Grib2Record*> listMatchExact, listMatchRange;
   find_record_matches((VarInfoGrib2*)(&vinfo), listMatchExact, listMatchRange);
   return 1 > listMatchExact.size() ? -1 : listMatchExact[0]->Index;
}

////////////////////////////////////////////////////////////////////////
//
// Begin utility functions.
//
////////////////////////////////////////////////////////////////////////

double scaled2dbl(int scale_factor, int scale_value) {
   if (scale_factor == 0) return ( (double) scale_value );
   if (scale_factor < 0)  return ( scale_value * pow(10.0, -scale_factor) );
   return                        ( scale_value / pow(10.0,  scale_factor) );
}

////////////////////////////////////////////////////////////////////////
