// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <iostream>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include "normalize.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////

ConcatString normalizetype_to_string(const NormalizeType type) {
   ConcatString s;

   // Convert enumerated NormalizeType to string
   switch(type) {

      case NormalizeType_None:
         s = normalizetype_none_str;
         break;

      case NormalizeType_ClimoAnom:
         s = normalizetype_climo_anom_str;
         break;

      case NormalizeType_ClimoStdAnom:
         s = normalizetype_climo_std_anom_str;
         break;

      case NormalizeType_FcstAnom:
         s = normalizetype_fcst_anom_str;
         break;

      case NormalizeType_FcstStdAnom:
         s = normalizetype_fcst_std_anom_str;
         break;

      default:
         mlog << Error << "\nnormalizetype_to_string() -> "
              << "Unexpected NormalizeType value of " << type << ".\n\n";
         exit(1);
   }

   return(s);
}

///////////////////////////////////////////////////////////////////////////////

void normalize_data(DataPlane &dp, const NormalizeType type,
                    const DataPlane *cmn_ptr, const DataPlane *csd_ptr,
                    const DataPlane *fmn_ptr, const DataPlane *fsd_ptr) {

   mlog << Debug(3) << "Normalizing input data using "
        << normalizetype_to_string(type) << ".\n";

   // Supported types
   switch(type) {

      case NormalizeType_None:
         break;

      case NormalizeType_ClimoAnom:
         if(!cmn_ptr || dp.nxy() != cmn_ptr->nxy()) {
            mlog << Error << "\nnormalize_data() -> "
                 << "the climatology mean is required for "
                 << normalizetype_to_string(type) << ".\n\n";
            exit(1);
         }
         dp.anomaly(*cmn_ptr);
         break;

      case NormalizeType_ClimoStdAnom:
         if(!cmn_ptr || dp.nxy() != cmn_ptr->nxy() ||
            !csd_ptr || dp.nxy() != csd_ptr->nxy()) {
            mlog << Error << "\nnormalize_data() -> "
                 << "the climatology mean and standard deviation are required for "
                 << normalizetype_to_string(type) << ".\n\n";
            exit(1);
         }
         dp.standard_anomaly(*cmn_ptr, *csd_ptr);
         break;

      case NormalizeType_FcstAnom:
         if(!fmn_ptr || dp.nxy() != fmn_ptr->nxy()) {
            mlog << Error << "\nnormalize_data() -> "
                 << "the forecast mean is required for "
                 << normalizetype_to_string(type) << ".\n\n";
            exit(1);
         }
         dp.anomaly(*fmn_ptr);
         break;

      case NormalizeType_FcstStdAnom:
         if(!fmn_ptr || dp.nxy() != fmn_ptr->nxy() ||
            !fsd_ptr || dp.nxy() != fsd_ptr->nxy()) {
            mlog << Error << "\nnormalize_data() -> "
                 << "the forecast mean and standard deviation are required for "
                 << normalizetype_to_string(type) << ".\n\n";
            exit(1);
         }
         dp.standard_anomaly(*fmn_ptr, *fsd_ptr);
         break;

      default:
         mlog << Error << "\nnormalize_data() -> "
              << "unexpected NormalizeType value ("
              << type << ")\n\n";
         exit(1);
   } // end switch

   return;
}

///////////////////////////////////////////////////////////////////////////////
