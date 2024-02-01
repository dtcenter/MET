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
#include "mode_data_type.h"

class MultiVarData1 {

 private:

   int *_fill_int_array(ShapeData *sd);
   float *_fill_float_array(ShapeData *sd);
   void _print_summary(const string &name, int *data, const ShapeData &sd) const;

 public:

   inline MultiVarData1(int nx, int ny, const string &name,
                        ModeDataType dataType) :
      _dataType(dataType),
      _name(name),
      _obj_sd(0),
      _obj_data(0),
      _raw_data(0),
      _sd(0),
      _nx(nx), _ny(ny),
      _convThreshArray(),
      _mergeThreshArray()
      {}
      
   inline ~MultiVarData1() {
      if (_obj_sd) delete _obj_sd;
      if (_obj_data) delete [] _obj_data;
      if (_raw_data) delete [] _raw_data;
      if (_sd) delete _sd;
   }      

   void set_obj(ShapeData *sd);
   void set_raw(ShapeData *sd);
   void set_shapedata(const ShapeData &sd);
   void set_conv_thresh_array(const ThreshArray &t);
   void set_merge_thresh_array(const ThreshArray &t);
   void objects_from_arrays(bool do_clusters,
                            BoolPlane & out);
   void print(const string &name) const;

   ModeDataType _dataType;
   string _name;
   ShapeData *_obj_sd;
   int *_obj_data;
   float *_raw_data;
   ShapeData *_sd;
   int _nx, _ny;
   ThreshArray _convThreshArray;
   ThreshArray _mergeThreshArray;
};

class MultiVarData {

 private:

   void _clear();

 public:

   MultiVarData();
   ~MultiVarData();
      
   void init(ModeDataType dataType,
             const string &name, 
             const Grid &grid, 
             const string &units, 
             const string &level,
             double data_min, double data_max);

   void set_obj(ShapeData *sd, bool simple);
   void set_raw(ShapeData *sd, bool simple);
   void set_shapedata(const ShapeData &sd, bool simple);
   void set_conv_thresh_array(const ThreshArray &t, bool simple);
   void set_merge_thresh_array(const ThreshArray &t, bool simple);
   const ShapeData *shapedata_ptr(bool simple) const;
   void  objects_from_arrays(bool do_clusters, bool simple, BoolPlane & out);
   void print(void) const;

   ModeDataType _dataType;
   MultiVarData1 *_simple;
   MultiVarData1 *_merge;
   string _name;
   int _nx, _ny;
   Grid *_grid;
   string _units;
   string _level;
   double _data_min, _data_max;
};

#endif   /*  __MODE_FRONT_END_H__  */


/////////////////////////////////////////////////////////////////////////
