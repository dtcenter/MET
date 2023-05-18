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


void MultiVarData1::set_fcst_obj(ShapeData *sd)
{
   if (_fcst_obj_sd) {
      delete _fcst_obj_sd;
   }
   _fcst_obj_sd = new ShapeData(*sd);

   if (_fcst_obj_data) {
      delete _fcst_obj_data;
   }
   _fcst_obj_data = _fill_int_array(sd);
}

void MultiVarData1::set_fcst_raw(ShapeData *sd)
{
   if (_fcst_raw_data) {
      delete _fcst_raw_data;
   }
   _fcst_raw_data = _fill_float_array(sd);
   
}

void MultiVarData1::set_obs_obj(ShapeData *sd)
{
   if (_obs_obj_sd) {
      delete _obs_obj_sd;
   }
   _obs_obj_sd = new ShapeData(*sd);

   if (_obs_obj_data) {
      delete _obs_obj_data;
   }
   _obs_obj_data = _fill_int_array(sd);
}

void MultiVarData1::set_obs_raw(ShapeData *sd)
{
   if (_obs_raw_data) {
      delete _obs_raw_data;
   }
   _obs_raw_data = _fill_float_array(sd);
   
}

void MultiVarData1::set_fcst_shapedata(const ShapeData &sd)
{
   if (_Fcst_sd) delete _Fcst_sd;
   _Fcst_sd = new ShapeData(sd);
}

void MultiVarData1::set_obs_shapedata(const ShapeData &sd)
{
   if (_Obs_sd) delete _Obs_sd;
   _Obs_sd = new ShapeData(sd);
}

void  MultiVarData1::objects_from_arrays(bool do_clusters,
                                        BoolPlane & fcst_out, 
                                        BoolPlane & obs_out)
{
   string name = _name + "_Fcst";
   populate_bool_plane(name, _fcst_obj_data, _nx, _ny, fcst_out);
   name = _name + "_Obs";
   populate_bool_plane(name, _obs_obj_data, _nx, _ny, obs_out);
}

void MultiVarData1::print(const string &fname, const string &oname) const
{
   string n;
   n = fname + "_" + _name + "_Fcst_Obj";
   if (_fcst_obj_data) {
      _print_summary(n, _fcst_obj_data, *_fcst_obj_sd);
   } else {
      mlog << Debug(1) << n << " is empty\n";
   }
   n = oname + "_" + _name + "_Obs_Obj";
   if (_obs_obj_data) {
      _print_summary(n, _obs_obj_data, *_obs_obj_sd);
   } else {
      mlog << Debug(1) << n << " is empty\n";
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
   mlog << Debug(1) << name << " has " << values.size() << " unique values\n";
   // for (size_t i=0; i<values.size(); ++i) {
   //    printf("\n  %d", values[i]);
   // }
   // printf("\n");
   _debug_shape_examine(name, sd, _nx, _ny);
}

MultiVarData::MultiVarData() :
   _simple(0),
   _merge(0),
   _f_name("notset"),
   _o_name("notset"),
   _nx(0), _ny(0),
   _grid(0),
   _ftype(FileType_None),
   _otype(FileType_None)
{
}   


MultiVarData::~MultiVarData() 
{
   _clear();
}   

void MultiVarData::checkFileTypeConsistency(const MultiVarData &mvdi, int j)
{
   bool err = false;
   if (_ftype != mvdi._ftype) {
      err = true;
      mlog << Error << "\n" 
           << ": forecast inputs of different file types not supported "
           << "Input 0:" << grdfiletype_to_string(_ftype).c_str()
           << "Input " << j << ":" << grdfiletype_to_string(mvdi._ftype).c_str()
           << "\n\n";
   }
         
   if (_otype != mvdi._otype) {
      err = true;
      mlog << Error << "\n" 
           << ": obs inputs of different file types not supported "
           << "Input 0:" << grdfiletype_to_string(_otype).c_str()
           << "Input " << j << ":" << grdfiletype_to_string(mvdi._otype).c_str()
           << "\n\n";
   }
   if (err) {
      exit ( 1 );
   }
}

void MultiVarData::init(const string &fname, const string &oname,
                        const Grid &grid, GrdFileType ftype,
                        GrdFileType otype)
{
   _clear();
   _f_name = fname;
   _o_name = oname;
   _nx = grid.nx();
   _ny = grid.ny();
   _grid = new Grid(grid);
   _ftype = ftype;
   _otype = otype;
   _simple = new MultiVarData1(_nx, _ny, "Simple");
   _merge = new MultiVarData1(_nx, _ny, "Merge");
}


void MultiVarData::set_fcst_obj(ShapeData *sd, bool simple)
{
   if (simple) {
      _simple->set_fcst_obj(sd);
   }
   else {
      _merge->set_fcst_obj(sd);
   }
}
      
void MultiVarData::set_fcst_raw(ShapeData *sd, bool simple)
{
   if (simple) {
      _simple->set_fcst_raw(sd);
   } else {
      _merge->set_fcst_raw(sd);
   }
}
      
void MultiVarData::set_obs_obj(ShapeData *sd, bool simple)
{
   if (simple) {
      _simple->set_obs_obj(sd);
   } else {
      _merge->set_obs_obj(sd);
   }
}
      
void MultiVarData::set_obs_raw(ShapeData *sd, bool simple)
{
   if (simple) {
      _simple->set_obs_raw(sd);
   } else {
      _merge->set_obs_raw(sd);
   }
}
      
void MultiVarData::set_fcst_shapedata(const ShapeData &sd, bool simple)
{
   if (simple) {
      _simple->set_fcst_shapedata(sd);
   } else {
      _merge->set_fcst_shapedata(sd);
   }
}

void MultiVarData::set_obs_shapedata(const ShapeData &sd, bool simple)
{
   if (simple) {
      _simple->set_obs_shapedata(sd);
   } else {
      _merge->set_obs_shapedata(sd);
   }
}

const ShapeData *MultiVarData::fcst_shapedata_ptr(bool simple) const
{
   if (simple) {
      return _simple->_Fcst_sd;
   } else {
      return _merge->_Fcst_sd;
   }
}

const ShapeData *MultiVarData::obs_shapedata_ptr(bool simple) const
{
   if (simple) {
      return _simple->_Obs_sd;
   } else {
      return _merge->_Obs_sd;
   }
}

////////////////////////////////////////////////////////////////////////


void  MultiVarData::objects_from_arrays(bool do_clusters,
                                        bool simple,
                                        BoolPlane & fcst_out, 
                                        BoolPlane & obs_out)
{
   if (simple) {
      _simple->objects_from_arrays(do_clusters, fcst_out, obs_out);
   } else {
      _merge->objects_from_arrays(do_clusters, fcst_out, obs_out);
   }
}  


////////////////////////////////////////////////////////////////////////



void  MultiVarData::print(void) const
{
   _simple->print(_f_name, _o_name);
   _merge->print(_f_name, _o_name);
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
   _ftype = FileType_None;
   _otype = FileType_None;
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

   mlog << Debug(1) << " Bool plane for " << name << " has " << nyes << " true values\n";

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
      mlog << Debug(1) << name << " shapeid=" << values[i] << " npt=" << count[i] << "\n";
   }
}   
