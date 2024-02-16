using namespace std;
#include "mode_superobject.h"
#include "multivar_data.h"

////////////////////////////////////////////////////////////////////////
static void _mask_super(const string &name, int nx, int ny, DataPlane &data)
{

   if (nx != data.nx() || ny != data.ny()) {
      mlog << Error << "\nModeSuperObject::mask_data_super() -> " << name 
           << " :dimensions don't match " << nx << " " <<  ny 
           << "    " << data.nx() << " " << data.ny() << "\n\n";

      exit( 1 );
   }

   int nmasked=0, nkeep=0;
   
   for (int x=0; x<nx; ++x)  {

      for (int y=0; y<ny; ++y)  {

         if(is_bad_data(data.get(x,y))) {
            nmasked ++;
         } else {
            data.set(0.0, x, y);
            nkeep ++;
         }
      }
   }
   
   mlog << Debug(1) << name << " Superobject masking.."
        << nkeep << " points of "
        << nmasked + nkeep << " are in superobjects\n";
}

////////////////////////////////////////////////////////////////////////
static void _mask(const string &name, int nx, int ny, const BoolPlane &bp,
                  DataPlane &data)
{
   if (nx != data.nx() || ny != data.ny()) {
      mlog << Error << "\nModeSuperObject::mask_data() -> " << name 
           << " :dimensions don't match " << nx << " " <<  ny 
           << "    " << data.nx() << " " << data.ny() << "\n\n";

      exit( 1 );
   }

   int nmasked=0, nkeep=0;
   
   for (int x=0; x<nx; ++x)  {

      for (int y=0; y<ny; ++y)  {

         if ( bp(x, y) == false) {
            data.set(bad_data_float, x, y);;
            nmasked ++;
         } else {
            nkeep ++;
         }
      }
   }
   
   mlog << Debug(1) << name << " Superobject masking.."
        << nkeep << " points of "
        << nmasked + nkeep << " are in superobjects\n";
}

////////////////////////////////////////////////////////////////////////

static void _debug_shape_examine(string &name, const ShapeData &sd,
                                  int nx, int ny)
{
   vector<double> values;
   vector<int> count;
   for (int x=0; x<nx; ++x) {
      for (int y=0; y<ny; ++y) {
         double v = sd.data.get(x,y);
         if (v <= 0) {
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
      mlog << Debug(1) << name << " shape value=" << values[i] << " count=" << count[i] << "\n";
   }
}   

ModeSuperObject::ModeSuperObject(bool isFcst, int n_files, bool do_clusters,
                                 const vector<MultiVarData *> &mvd,
                                 BoolCalc &calc)
{
   _hasUnion = calc.has_union();
   
   //
   //  set the BoolPlane values using the mvd content
   //

   BoolPlane * simple_plane = new BoolPlane [n_files];
   BoolPlane * merge_plane = new BoolPlane [n_files];

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

   string simple_name, merge_name;
   
   if (isFcst) {
      simple_name = "Fcst_Simple";
      merge_name = "Fcst_Merge";
   } else {
      simple_name = "Obs_Simple";
      merge_name = "Obs_Merge";
   }      
   combine_boolplanes(simple_name, simple_plane, n_files, calc, _simple_result);
   combine_boolplanes(merge_name,  merge_plane, n_files, calc, merge_result);

   // create ShapeData objects using something from mvd as a template
   // (shape data has 1's or bad)

   _simple_sd = ShapeData(*(mvd[0]->_simple->_sd));
   for (int x=0; x<nx; ++x) {
      for (int y=0; y<ny; ++y) {
         if (_simple_result.get(x, y)) {
            _simple_sd.data.put(1.0, x, y);
         } else {
            _simple_sd.data.put(bad_data_double, x, y);
         }
      }
   }

   ShapeData merge_sd = ShapeData(*(mvd[0]->_simple->_sd));
   for (int x=0; x<nx; ++x) {
      for (int y=0; y<ny; ++y) {
         if (merge_result.get(x, y)) {
            merge_sd.data.put(1.0, x, y);
         } else {
            merge_sd.data.put(bad_data_double, x, y);
         }
      }
   }
   
   int n_shapes;
   _merge_sd_split = split(merge_sd, n_shapes);
   _debug_shape_examine(merge_name, _merge_sd_split, nx, ny);

   delete [] simple_plane;
   delete [] merge_plane;
}

void ModeSuperObject::mask_data_simple(const string &name, MultiVarData &mvd) const
{
   int nx = mvd._nx;
   int ny = mvd._ny;
   _mask(name, nx, ny, _simple_result, mvd._simple->_sd->data);
}


void ModeSuperObject::mask_data_super(const string &name, const MultiVarData &mvd)
{
   int nx = mvd._nx;
   int ny = mvd._ny;
   _mask_super(name, nx, ny, _simple_sd.data);
}
