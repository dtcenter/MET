// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#include "string.h"

#include "vx_python_utils.h"
#include "check_endian.h"

#include "dataplane_from_numpy_array.h"
#include "grid_from_python_dict.h"


////////////////////////////////////////////////////////////////////////


template <typename T>
void load_numpy (void * buf,
                 const int Nx, const int Ny,
                 const int data_endian,
                 void (*shuf)(void *), 
                 DataPlane & out)


{

bool need_swap = (shuf != 0) && (native_endian != data_endian);

int j, x, y, r, c;
const int Nxy = Nx*Ny;
T * u = (T *) buf;
T value;

for (j=0; j<Nxy; ++j)  {

   nympy_array_one_to_two(j, Nx, r, c);

   x = c;

   y = Ny - 1 - r;

   memcpy(&value, u + j, sizeof(T));

   if ( need_swap )  shuf(&value);

   dp_out.set((double) value, x, y);

}   //  for j



return;

}


////////////////////////////////////////////////////////////////////////


static void load_le_double (void * buf, const int Nx, const int Ny, DataPlane & out);
static void load_le_float  (void * buf, const int Nx, const int Ny, DataPlane & out);


////////////////////////////////////////////////////////////////////////

   //
   //  2D numpy arrays seem to store things in row-major order
   //

inline void nympy_array_one_to_two(const int n, const int Ncols, int & row, int & col)

{

row = n/Ncols;

col = n%Ncols;

return;

}


////////////////////////////////////////////////////////////////////////


bool dataplane_from_numpy_array(Python3_numpy & np, PyObject * attrs_dict, DataPlane & dp_out, Grid & grid_out, VarInfoPython &vinfo)

{

int j;
int dim, nrows, ncols, Nx, Ny, Nxy;
int r, c, x, y;
double value;
int sizes [max_tuple_data_dims];
void * p  = 0;
double * pp = 0;


   //
   //  make sure it's a 2D array
   //

if ( np.n_dims() != 2 )  {

   cerr << "\n\n  dataplane_from_numpy_array() -> "
        << "numpy array is not 2-dimensional! ... "
        << "(dim = " << (np.n_dims()) << ")\n\n";

   exit ( 1 );


}

nrows = np.dim(0);
ncols = np.dim(1);

Nx = ncols;
Ny = nrows;

Nxy = Nx*Ny;

dp_out.set_size(Nx, Ny);

   //
   //  load the data
   //

const ConcatString dtype = np.dtype();

      //   1 byte integers

     if ( dtype == "|i1"  )   load_numpy <int8_t>    (np.buffer(), Nx, Ny, little_endian,         0, dp_out);
else if ( dtype == "|u1"  )   load_numpy <uint8_t>   (np.buffer(), Nx, Ny, little_endian,         0, dp_out);

      //   2 byte integers

else if ( dtype == "<i2"  )   load_numpy <int16_t>   (np.buffer(), Nx, Ny, little_endian, shuffle_2, dp_out);
else if ( dtype == "<u2"  )   load_numpy <uint16_t>  (np.buffer(), Nx, Ny, little_endian, shuffle_2, dp_out);

else if ( dtype == ">i2"  )   load_numpy <int16_t>   (np.buffer(), Nx, Ny,    big_endian, shuffle_2, dp_out);
else if ( dtype == ">u2"  )   load_numpy <uint16_t>  (np.buffer(), Nx, Ny,    big_endian, shuffle_2, dp_out);

      //   4 byte integers

else if ( dtype == "<i4"  )   load_numpy <int32_t>   (np.buffer(), Nx, Ny, little_endian, shuffle_4, dp_out);
else if ( dtype == "<u4"  )   load_numpy <uint32_t>  (np.buffer(), Nx, Ny, little_endian, shuffle_4, dp_out);

else if ( dtype == ">i4"  )   load_numpy <int32_t>   (np.buffer(), Nx, Ny,    big_endian, shuffle_4, dp_out);
else if ( dtype == ">u4"  )   load_numpy <uint32_t>  (np.buffer(), Nx, Ny,    big_endian, shuffle_4, dp_out);

      //   8 byte integers

else if ( dtype == "<i8"  )   load_numpy <int64_t>   (np.buffer(), Nx, Ny, little_endian, shuffle_8, dp_out);
else if ( dtype == "<u8"  )   load_numpy <uint64_t>  (np.buffer(), Nx, Ny, little_endian, shuffle_8, dp_out);

else if ( dtype == ">i8"  )   load_numpy <int64_t>   (np.buffer(), Nx, Ny,    big_endian, shuffle_8, dp_out);
else if ( dtype == ">u8"  )   load_numpy <uint64_t>  (np.buffer(), Nx, Ny,    big_endian, shuffle_8, dp_out);

      //   single precision floats

else if ( dtype == "<f4"  )   load_numpy <float>     (np.buffer(), Nx, Ny, little_endian, shuffle_4, dp_out);
else if ( dtype == ">f4"  )   load_numpy <float>     (np.buffer(), Nx, Ny,    big_endian, shuffle_4, dp_out);

      //   double precision floats

else if ( dtype == "<f8"  )   load_numpy <double>    (np.buffer(), Nx, Ny, little_endian, shuffle_8, dp_out);
else if ( dtype == ">f8"  )   load_numpy <double>    (np.buffer(), Nx, Ny,    bit_endian, shuffle_8, dp_out);

      //
      //   nope ... the only other numerical data type for numpy arrays 
      //            is single or double precision complex numbers, and 
      //            we're not supporting those at this time
      //

else  {

   mlog << Error
        << \n\n   dataplane_from_numpy_array() -> unsupported numpy data type \"" 
        << dtype << "\"\n\n";

   exit ( 1 );

}





   //
   //  get timestamp info from the attributes dictionary
   //
   //    init and valid times are strings like YYYYMMDD_HHMMSS
   //

ConcatString s;
unixtime t;


s = dict_lookup_string(attrs_dict, "init");

t = timestring_to_unix(s.c_str());

dp_out.set_init(t);

     ////////////////////

s = dict_lookup_string(attrs_dict, "valid");

t = timestring_to_unix(s.c_str());

dp_out.set_valid(t);

     ////////////////////

s = dict_lookup_string(attrs_dict, "lead");

t = timestring_to_sec(s.c_str());

dp_out.set_lead(t);

     ////////////////////

s = dict_lookup_string(attrs_dict, "accum");

t = timestring_to_sec(s.c_str());

dp_out.set_accum(t);

     ////////////////////

PyObject * py_grid = dict_lookup_dict(attrs_dict, "grid");

if ( ! py_grid )  {

   mlog << Warning << "\ndataplane_from_numpy_array() -> "
        << "trouble getting the \"grid\"\n\n";

   return ( false );
}


grid_from_python_dict(py_grid, grid_out);

     ////////////////////

vinfo.set_name(dict_lookup_string(attrs_dict, "name"));
vinfo.set_long_name(dict_lookup_string(attrs_dict, "long_name").c_str());
vinfo.set_level_name(dict_lookup_string(attrs_dict, "level").c_str());
vinfo.set_units(dict_lookup_string(attrs_dict, "units").c_str());

     ////////////////////

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////



