// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include "mode_superobject.h"
#include "multivar_data.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

static void _mask_super(const string &name, int nx, int ny,
                        DataPlane &data) {

   if(nx != data.nx() || ny != data.ny()) {
      mlog << Error << "\nModeSuperObject::mask_data_super() -> " << name 
           << " :dimensions don't match " << nx << " " <<  ny 
           << "    " << data.nx() << " " << data.ny() << "\n\n";
      exit(1);
   }

   int nmasked = 0;
   int nkeep = 0;

#pragma omp parallel default(none) \
   shared(nx, ny, data, nmasked, nkeep)
   {

#pragma omp for schedule(static) \
                reduction(+: nmasked, nkeep) \
                collapse(2)
      for(int x=0; x<nx; x++) {
         for(int y=0; y<ny; y++) {
            if(is_bad_data(data.get(x,y))) {
               nmasked++;
            }
            else {
               data.set(0.0, x, y);
               nkeep++;
            }
         } // for y 
      } // for x
   } // End omp parallel
   
   mlog << Debug(1) << name << " Superobject masking... "
        << nkeep << " points of "
        << nmasked + nkeep << " are in superobjects\n";
}

////////////////////////////////////////////////////////////////////////

static void _mask(const string &name, int nx, int ny,
                  const BoolPlane &bp, DataPlane &data) {

   if(nx != data.nx() || ny != data.ny()) {
      mlog << Error << "\nModeSuperObject::mask_data() -> " << name 
           << " :dimensions don't match " << nx << " " <<  ny 
           << "    " << data.nx() << " " << data.ny() << "\n\n";
      exit(1);
   }

   int nmasked = 0;
   int nkeep = 0;
   
#pragma omp parallel default(none) \
   shared(nx, ny, bp, data, nmasked, nkeep)
   {

#pragma omp for schedule(static) \
                reduction(+: nmasked, nkeep) \
                collapse(2)
      for(int x=0; x<nx; x++) {
         for(int y=0; y<ny; y++) {
            if(!bp(x, y)) {
               data.set(bad_data_float, x, y);
               nmasked++;
            }
            else {
               nkeep++;
            }
         } // for y
      } // for x
   } // End omp parallel
   
   mlog << Debug(1) << name << " Superobject masking... "
        << nkeep << " points of "
        << nmasked + nkeep << " are in superobjects\n";
}

////////////////////////////////////////////////////////////////////////

static void _debug_shape_examine(const string &name, const ShapeData &sd,
                                 int nx, int ny) {
   vector<double> values;
   vector<int> count;

   for(int x=0; x<nx; x++) {
      for(int y=0; y<ny; y++) {
         double v = sd.data.get(x,y);
         if(v <= 0) continue;
         vector<double>::iterator vi;
         vi = find(values.begin(), values.end(), v);
         if(vi == values.end()) {
            values.emplace_back(v);
            count.emplace_back(1);
         }
         else {
            auto ii = (int) (vi - values.begin());
            count[ii] = count[ii] + 1;
         }
      }
   }
   for(size_t i=0; i<values.size(); ++i) {
      mlog << Debug(1) << name << " shape value=" << values[i]
           << " count=" << count[i] << "\n";
   }
}   

////////////////////////////////////////////////////////////////////////

ModeSuperObject::ModeSuperObject()
{
}

////////////////////////////////////////////////////////////////////////

ModeSuperObject::ModeSuperObject(bool isFcst, int n_files, bool do_clusters,
                                 int r_index, int t_index,
                                 const vector<MultiVarData *> &mvd,
                                 BoolCalc &calc)
{
   _hasUnion = calc.has_union();
   _rIndex = r_index;
   _tIndex = t_index;
   
   //
   //  set the BoolPlane values using the mvd content
   //

   vector<BoolPlane> simple_plane(n_files);
   vector<BoolPlane> merge_plane(n_files);

   for (int j=0; j<n_files; ++j)  {
      mvd[j]->objects_from_arrays(do_clusters, true, simple_plane[j]);
      mvd[j]->objects_from_arrays(do_clusters, false, merge_plane[j]);
   }

   //
   //  combine the objects into super-objects
   //
   const int nx = simple_plane[0].nx();
   const int ny = simple_plane[0].ny();

   BoolPlane merge_result;  // local, not used
   _simple_result.set_size(nx, ny);
   merge_result.set_size(nx, ny);

   string simple_name;
   string merge_name;
   
   if (isFcst) {
      simple_name = "Fcst_Simple";
      merge_name = "Fcst_Merge";
   } else {
      simple_name = "Obs_Simple";
      merge_name = "Obs_Merge";
   }      

   mlog << Debug(1) << "\n";
   combine_boolplanes(simple_name, _rIndex, _tIndex,
                      simple_plane.data(), n_files, calc, _simple_result);
   combine_boolplanes(merge_name,  _rIndex, _tIndex,
                      merge_plane.data(), n_files, calc, merge_result);

   // create ShapeData objects using something from mvd as a template
   // (shape data has 1's or bad)

   _simple_sd    = ShapeData(*(mvd[0]->_simple->_sd));
   auto merge_sd = ShapeData(*(mvd[0]->_simple->_sd));

#pragma omp parallel default(none) \
   shared(nx, ny, bad_data_double) \
   shared(_simple_result, _simple_sd) \
   shared(merge_result, merge_sd)
   {

#pragma omp for schedule(static) \
                collapse(2)
      for(int x=0; x<nx; x++) {
         for(int y=0; y<ny; y++) {
            if(_simple_result.get(x, y)) {
               _simple_sd.data.put(1.0, x, y);
            }
            else {
               _simple_sd.data.put(bad_data_double, x, y);
            }
         } // for y
      } // for x

#pragma omp for schedule(static) \
                collapse(2)
      for(int x=0; x<nx; x++) {
         for(int y=0; y<ny; y++) {
            if(merge_result.get(x, y)) {
               merge_sd.data.put(1.0, x, y);
            }
            else {
               merge_sd.data.put(bad_data_double, x, y);
            }
         } // for y
      } // for x
   } // End omp parallel
   
   int n_shapes;
   _merge_sd_split = split(merge_sd, n_shapes);
   _debug_shape_examine(merge_name, _merge_sd_split, nx, ny);

}

////////////////////////////////////////////////////////////////////////

void ModeSuperObject::mask_data_simple(const string &name, MultiVarData &mvd) const
{
   int nx = mvd._nx;
   int ny = mvd._ny;
   _mask(name, nx, ny, _simple_result, mvd._simple->_sd->data);
}

////////////////////////////////////////////////////////////////////////

void ModeSuperObject::mask_data_super(const string &name, const MultiVarData &mvd)
{
   int nx = mvd._nx;
   int ny = mvd._ny;
   _mask_super(name, nx, ny, _simple_sd.data);
}

////////////////////////////////////////////////////////////////////////
