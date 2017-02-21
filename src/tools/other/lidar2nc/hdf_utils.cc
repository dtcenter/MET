
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;


////////////////////////////////////////////////////////////////////////


#include "hdf_utils.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for struct HdfVarInfo
   //


////////////////////////////////////////////////////////////////////////


HdfVarInfo::HdfVarInfo()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void HdfVarInfo::clear()

{

hdf_index = -1;

hdf_id = -1;

hdf_rank = -1;

hdf_type = -1;

hdf_atts = -1;

int j;

for (j=0; j<MAX_VAR_DIMS; ++j)  hdf_dimsizes[j] = -1;


return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


void get_hdf_var_info(const int hdf_sd_id, const char * hdf_name, HdfVarInfo & info)

{

info.hdf_name = hdf_name;

if ( (info.hdf_index = SDnametoindex(hdf_sd_id, hdf_name)) < 0 )  {

   mlog << Error
        << "\n\n  get_hdf_var_info() -> failed to get index for \""
        << hdf_name << "\"\n\n";

   exit ( 1 );

}

if ( (info.hdf_id = SDselect(hdf_sd_id, info.hdf_index)) < 0 )  {

   mlog << Error
        << "\n\n  get_hdf_var_info() -> failed to get id for \""
        << hdf_name << "\"\n\n";

   exit ( 1 );

}

if ( SDgetinfo(info.hdf_id, 0, &(info.hdf_rank), info.hdf_dimsizes, &(info.hdf_type), &(info.hdf_atts)) < 0 )  {

   mlog << Error
        << "\n\n  get_hdf_var_info() -> SDgetinfo failed\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return;

}



////////////////////////////////////////////////////////////////////////


int sizeof_hdf_type(const int type)

{

int k = 0;

switch ( type )  {

   case DFNT_CHAR8:    k =  8;  break;   // 8-bit character type
   case DFNT_UCHAR8:   k =  8;  break;   // 8-bit unsigned character type
   case DFNT_INT8:     k =  8;  break;   // 8-bit integer type
   case DFNT_UINT8:    k =  8;  break;   // 8-bit unsigned integer type

   case DFNT_INT16:    k = 16;  break;   // 16-bit integer type
   case DFNT_UINT16:   k = 16;  break;   // 16-bit unsigned integer type

   case DFNT_INT32:    k = 32;  break;   // 32-bit integer type
   case DFNT_UINT32:   k = 32;  break;   // 32-bit unsigned integer type
   case DFNT_FLOAT32:  k = 32;  break;   // 32-bit floating-point type

   case DFNT_FLOAT64:  k = 64;  break;   // 64-bit floating-point type

   default:
      mlog << Error
           << "sizeof_hdf_type() -> unrecognized hdf data type\n\n";
      exit ( 1 );
      break;

}   //  switch



return ( k );

}


////////////////////////////////////////////////////////////////////////



