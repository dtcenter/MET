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
#include "data_file_type.h"
#include "two_d_array.h"


class MultiVarData1 {

 private:

   int *_fill_int_array(ShapeData *sd);
   float *_fill_float_array(ShapeData *sd);
   void _print_summary(const string &name, int *data, const ShapeData &sd) const;

 public:

   inline MultiVarData1(int nx, int ny, const string &name) :
      _name(name),
      _fcst_obj_sd(0),
      _obs_obj_sd(0),
      _fcst_obj_data(0),
      _obs_obj_data(0),
      _fcst_raw_data(0),
      _obs_raw_data(0),
      _Fcst_sd(0), _Obs_sd(0),
      _nx(nx), _ny(ny)
      {}
      
   inline ~MultiVarData1() {
      if (_fcst_obj_sd) delete _fcst_obj_sd;
      if (_obs_obj_sd) delete _obs_obj_sd;
      if (_fcst_obj_data) delete [] _fcst_obj_data;
      if (_obs_obj_data) delete [] _obs_obj_data;
      if (_fcst_raw_data) delete [] _fcst_raw_data;
      if (_obs_raw_data) delete [] _obs_raw_data;
      if (_Fcst_sd) delete _Fcst_sd;
      if (_Obs_sd) delete _Obs_sd;
   }      

   void set_fcst_obj(ShapeData *sd);
   void set_fcst_raw(ShapeData *sd);
   void set_obs_obj(ShapeData *sd);
   void set_obs_raw(ShapeData *sd);
   void set_fcst_shapedata(const ShapeData &sd);
   void set_obs_shapedata(const ShapeData &sd);
   void objects_from_arrays(bool do_clusters,
                            BoolPlane & fcst_out, 
                            BoolPlane & obs_out);
   void print(const string &fname, const string &oname) const;

   string _name;
   ShapeData *_fcst_obj_sd;
   ShapeData *_obs_obj_sd;
   int *_fcst_obj_data;
   int *_obs_obj_data;
   float *_fcst_raw_data;
   float *_obs_raw_data;
   ShapeData *_Fcst_sd;
   ShapeData *_Obs_sd;
   int _nx, _ny;
};

class MultiVarData {

 private:

   void _clear();

 public:

   MultiVarData();
   ~MultiVarData();
      
   void checkFileTypeConsistency(const MultiVarData &mvdi, int j);

   void init(const string &fname, const string &oname,
             const Grid &grid, GrdFileType ftype, GrdFileType otype);
   void set_fcst_obj(ShapeData *sd, bool simple);
   void set_fcst_raw(ShapeData *sd, bool simple);
   void set_obs_obj(ShapeData *sd, bool simple);
   void set_obs_raw(ShapeData *sd, bool simple);
   void set_fcst_shapedata(const ShapeData &sd, bool simple);
   void set_obs_shapedata(const ShapeData &sd, bool simple);
   const ShapeData *fcst_shapedata_ptr(bool simple) const;
   const ShapeData *obs_shapedata_ptr(bool simple) const;
   void  objects_from_arrays(bool do_clusters, bool simple, BoolPlane & fcst_out, 
                             BoolPlane & obs_out);
   void print(void) const;

   
   MultiVarData1 *_simple;
   MultiVarData1 *_merge;
   string _f_name;
   string _o_name;
   int _nx, _ny;
   Grid *_grid;
   GrdFileType _ftype;
   GrdFileType _otype;
     
};


#endif   /*  __MODE_FRONT_END_H__  */


/////////////////////////////////////////////////////////////////////////
