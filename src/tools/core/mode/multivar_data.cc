// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////


using namespace std;

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <ctype.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "multivar_data.h"
#include "vx_regrid.h"
#include "vx_shapedata.h"

////////////////////////////////////////////////////////////////////////


static void populate_bool_plane(const string &name, const int * buf, const int nx, const int ny,
                                BoolPlane & bp_out);

static void  _debug_shape_examine(const string &name, const ShapeData &sd, int nx, int ny);


void MultiVarData1::set_obj(ShapeData *sd)
{
   if (_obj_sd) {
      delete _obj_sd;
   }
   _obj_sd = new ShapeData(*sd);

   if (_obj_data) {
      delete _obj_data;
   }
   _obj_data = _fill_int_array(sd);
}

void MultiVarData1::set_raw(ShapeData *sd)
{
   if (_raw_data) {
      delete _raw_data;
   }
   _raw_data = _fill_float_array(sd);
}

void MultiVarData1::set_shapedata(const ShapeData &sd)
{
   if (_sd) delete _sd;
   _sd = new ShapeData(sd);
}

void MultiVarData1::set_conv_thresh_array(const ThreshArray &t)
{
   _convThreshArray = t;
}

void MultiVarData1::set_merge_thresh_array(const ThreshArray &t)
{
   _mergeThreshArray = t;
}

void  MultiVarData1::objects_from_arrays(bool do_clusters,
                                         BoolPlane & out)
{
   string name = _name + "_" + sprintModeDataType(_dataType);
   populate_bool_plane(name, _obj_data, _nx, _ny, out);
}

void MultiVarData1::print(const string &name) const
{
   string n;
   n = name + "_" + _name + "_" + sprintModeDataType(_dataType) + "_Obj";
   if (_obj_data) {
      _print_summary(n, _obj_data, *_obj_sd);
   } else {
      mlog << Debug(2) << n << " is empty\n";
   }
}           


int *MultiVarData1::_fill_int_array(ShapeData *sd)
{
   int *ret = new int [_nx*_ny];
   for (int x=0; x<_nx; ++x) {
      for (int y=0; y<_ny; ++y) {
         int n = DefaultTO.two_to_one(_nx, _ny, x, y);
         if (sd->is_nonzero(x,y) ) {
            ret[n] = nint(sd->data(x, y));
         } else {
            ret[n] = bad_data_int;
         }
      }
   }
   return ret;
}

float *MultiVarData1::_fill_float_array(ShapeData *sd)
{
   float *ret = new float [_nx*_ny];
   for (int x=0; x<_nx; ++x) {
      for (int y=0; y<_ny; ++y) {
         int n = DefaultTO.two_to_one(_nx, _ny, x, y);
         ret[n] = sd->data(x, y);
      }
   }
   return ret;
}

void MultiVarData1::_print_summary(const string &name, int *data, const ShapeData &sd) const
{
   vector<int> values;
   for (int i=0; i<_nx*_ny; ++i) {
      if (data[i] > 0) {
         if (find(values.begin(), values.end(), data[i]) == values.end()) {
            values.push_back(data[i]);
         }
      }
   }
   mlog << Debug(4) << name << " has " << values.size() << " unique values\n";
   _debug_shape_examine(name, sd, _nx, _ny);
}

MultiVarData::MultiVarData() :
   _dataType(ModeDataType_MvMode_Both),
   _simple(0),
   _merge(0),
   _name("notset"),
   _nx(0), _ny(0),
   _grid(0),
   _type(FileType_None)
{
}   

MultiVarData::~MultiVarData() 
{
   _clear();
}   

void MultiVarData::checkFileTypeConsistency(const MultiVarData &mvdi, int j)
{
   bool err = false;
   if (_type != mvdi._type) {
      mlog << Error << "MultivarData::checkFileTypeConsistgency() -> " 
           << "inputs of different file types not supported "
           << "Input 0:" << grdfiletype_to_string(_type).c_str()
           << "Input " << j << ":" << grdfiletype_to_string(mvdi._type).c_str()
           << "\n\n";
      exit ( 1 );
   }
}

void MultiVarData::init(ModeDataType dataType,
                        const string &name, 
                        const Grid &grid, GrdFileType type,
                        const string &units,
                        const string &level,
                        double data_min, double data_max)
{
   _clear();

   _dataType = dataType;
   _name = name;
   _nx = grid.nx();
   _ny = grid.ny();
   _grid = new Grid(grid);
   _type = type;
   _units = units;
   _level = level;
   _data_min = data_min;
   _data_max = data_max;
   _simple = new MultiVarData1(_nx, _ny, "Simple", _dataType);
   _merge = new MultiVarData1(_nx, _ny, "Merge", _dataType);
}

void MultiVarData::set_obj(ShapeData *sd, bool simple)
{
   if (simple) {
      _simple->set_obj(sd);
   }
   else {
      _merge->set_obj(sd);
   }
}
      
void MultiVarData::set_raw(ShapeData *sd, bool simple)
{
   if (simple) {
      _simple->set_raw(sd);
   } else {
      _merge->set_raw(sd);
   }
}
      
void MultiVarData::set_shapedata(const ShapeData &sd, bool simple)
{
   if (simple) {
      _simple->set_shapedata(sd);
   } else {
      _merge->set_shapedata(sd);
   }
}

void MultiVarData::set_conv_thresh_array(const ThreshArray &t, bool simple)
{
   if (simple) {
      _simple->set_conv_thresh_array(t);
   } else {
      _merge->set_conv_thresh_array(t);
   }
}

void MultiVarData::set_merge_thresh_array(const ThreshArray &t, bool simple)
{
   if (simple) {
      _simple->set_merge_thresh_array(t);
   } else {
      _merge->set_merge_thresh_array(t);
   }
}

const ShapeData *MultiVarData::shapedata_ptr(bool simple) const
{
   if (simple) {
      return _simple->_sd;
   } else {
      return _merge->_sd;
   }
}

////////////////////////////////////////////////////////////////////////


void  MultiVarData::objects_from_arrays(bool do_clusters,
                                         bool simple,
                                         BoolPlane & out)
{
   if (simple) {
      _simple->objects_from_arrays(do_clusters, out);
   } else {
      _merge->objects_from_arrays(do_clusters, out);
   }
}  


////////////////////////////////////////////////////////////////////////


void  MultiVarData::print(void) const
{
   _simple->print(_name);
   _merge->print(_name);
}  


////////////////////////////////////////////////////////////////////////


void MultiVarData::_clear()
{
   if (_simple) {
      delete _simple;
      _simple = 0;
   }
   if (_merge) {
      delete _merge;
      _merge = 0;
   }
   if (_grid) {
      delete _grid;
      _grid = 0;
   }
   _nx = _ny = 0;
   _type = FileType_None;
}


////////////////////////////////////////////////////////////////////////


void populate_bool_plane(const string &name, const int * buf, const int nx, const int ny, BoolPlane & bp_out)

{

   int x, y, n, k;
   bool tf;
   double nyes=0.0;
   double ntotal = (double)(nx*ny);
   bp_out.set_size(nx, ny);

   for (x=0; x<nx; ++x)  {

      for (y=0; y<ny; ++y)  {

         n = y*nx + x;

         k = buf[n];

         tf = ( k > 0 );
         if (tf) ++nyes;
         bp_out.put(tf, x, y);

      }   //  for y

   }   //  for x

   mlog << Debug(4) << " Bool plane for " << name << " has " << nyes << " true values\n";

   return;

}


////////////////////////////////////////////////////////////////////////


static void  _debug_shape_examine(const string &name, const ShapeData &sd, int nx, int ny)
{
   vector<double> values;
   vector<int> count;
   for (int x=0; x<nx; ++x) {
      for (int y=0; y<ny; ++y) {
         double v = sd.data.get(x,y);
         if (v == 0.0) {
            continue;
         }
         vector<double>::iterator vi;
         vi = find(values.begin(), values.end(), v);
         if (vi == values.end()) {
            values.push_back(v);
            count.push_back(1);
         } else {
            int ii = vi - values.begin();
            count[ii] = count[ii] + 1;
         }
      }
   }
   for (size_t i=0; i<values.size(); ++i) {
      mlog << Debug(4) << name << " shapeid=" << values[i] << " npt=" << count[i] << "\n";
   }
}   
