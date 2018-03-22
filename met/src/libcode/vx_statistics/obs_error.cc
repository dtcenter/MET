// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "obs_error.h"

////////////////////////////////////////////////////////////////////////

double add_obs_error(double v, FieldType t, const ObsErrorInfo &info) {
   double v_new = v;

   // Apply instrument bias correction to observation values
   if(t == FieldType_Obs) {
      if(!is_bad_data(info.inst_bias_scale)) {
         v_new *= info.inst_bias_scale;
      }
      if(!is_bad_data(info.inst_bias_offset)) {
         v_new += info.inst_bias_offset;
      }
   }

   // Apply the distribution type if set in the config file
   if(info.dist_type != DistType_None) {
      v_new += ran_draw(info.rng_ptr, info.dist_type,
                        info.dist_parm[0], info.dist_parm[1]);
   }
   // Otherwise, do an observation error table lookup
   else {
      mlog << Debug(1) << "PairBase::add_obs_error() -> "
           << "JHG: need to implement table lookup!\n";
      v_new = v;
   }

   return(v_new);
}

////////////////////////////////////////////////////////////////////////

DataPlane add_obs_error(const DataPlane &dp, FieldType t,
                        const ObsErrorInfo &info) {
   int x, y;
   double v;
   DataPlane new_dp;

   // Error check
   if(info.dist_parm.n() != 2) {
      mlog << Error << "\nadd_obs_error() -> "
           << "the distribution parameter array must have length 2!\n\n";
      exit(1);
   }

   // Initialize
   new_dp = dp;

   // Apply instrument bias correction to observation values
   if(t == FieldType_Obs) {
      if(!is_bad_data(info.inst_bias_scale) ||
         !is_bad_data(info.inst_bias_offset)) {
         new_dp.transform(info.inst_bias_scale, info.inst_bias_offset);
      }
   }

   // Apply random perturbation to each grid point
   for(x=0; x<new_dp.nx(); x++) {
      for(y=0; y<new_dp.ny(); y++) {

         v = new_dp.get(x, y);

         if(!is_bad_data(v)) {
            v += ran_draw(info.rng_ptr, info.dist_type,
                          info.dist_parm[0], info.dist_parm[1]);
         }

         new_dp.set(v, x, y);
      }
   }

   return(new_dp);
}

////////////////////////////////////////////////////////////////////////
