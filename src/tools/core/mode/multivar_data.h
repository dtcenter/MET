// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MULTIVAR_DATA_H__
#define  __MULTIVAR_DATA_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <vector>
#include "shapedata.h"
#include "vx_grid.h"

class MultiVarData {

   private:

     //void init_from_scratch();
     
   public:

  inline MultiVarData() :
    fcst_obj_data(0),
    obs_obj_data(0),
    fcst_raw_data(0),
    obs_raw_data(0),
    nx(0), ny(0),
    Fcst_sd(0), Obs_sd(0),
    grid(0),
    ftype(FileType_None),
    otype(FileType_None)
    {}
      
    inline ~MultiVarData() {
      if (fcst_obj_data) delete [] fcst_obj_data;
      if (obs_obj_data) delete [] obs_obj_data;
      if (fcst_raw_data) delete [] fcst_raw_data;
      if (obs_raw_data) delete [] obs_raw_data;
      if (Fcst_sd) delete Fcst_sd;
      if (Obs_sd) delete Obs_sd;
      if (grid) delete grid;
    }      

     int *fcst_obj_data;
     int *obs_obj_data;
     float *fcst_raw_data;
     float *obs_raw_data;
     int nx, ny;
     ShapeData *Fcst_sd;
     ShapeData *Obs_sd;
     Grid *grid;
     GrdFileType ftype;
     GrdFileType otype;
     
  /* int get_fcst_obj_nx(); */
  /* int get_fcst_obj_ny(); */
  /* int *get_fcst_obj_data(); */
  /* float *get_fcst_raw_data(); */
  /* int get_obs_obj_nx(); */
  /* int get_obs_obj_ny(); */
  /* int *get_obs_obj_data(); */
  /* float *get_obs_raw_data(); */


};


#endif   /*  __MODE_FRONT_END_H__  */


/////////////////////////////////////////////////////////////////////////
