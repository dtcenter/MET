// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#include "string.h"

#include "vx_python3_utils.h"
#include "check_endian.h"

#include "data_plane.h"
#include "dataplane_from_numpy_array.h"
#include "grid_from_python_dict.h"


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

   out.set((double) value, x, y);

}   //  for j



return;

}


////////////////////////////////////////////////////////////////////////


bool dataplane_from_numpy_array(Python3_Numpy & np, const Python3_Dict & attrs, DataPlane & dp_out, Grid & grid_out, VarInfoPython &vinfo)

{

int nrows, ncols, Nx, Ny;


   //
   //  make sure it's a 2D array
   //

if ( np.n_dims() != 2 )  {

   mlog << Error << "\ndataplane_from_numpy_array() -> "
        << "numpy array is not 2-dimensional! ... "
        << "(dim = " << (np.n_dims()) << ")\n\n";

   exit ( 1 );


}

nrows = np.dim(0);
ncols = np.dim(1);

Nx = ncols;
Ny = nrows;



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
else if ( dtype == ">f8"  )   load_numpy <double>    (np.buffer(), Nx, Ny,    big_endian, shuffle_8, dp_out);

      //
      //   nope ... the only other numerical data type for numpy arrays 
      //            is single or double precision complex numbers, and 
      //            we're not supporting those at this time
      //

else  {

   mlog << Error << "\ndataplane_from_numpy_array() -> "
        << "unsupported numpy data type \""  << dtype << "\"\n\n";

   exit ( 1 );

}

   //
   //  get timestamp info from the attributes dictionary
   //
   //    init and valid times are strings like YYYYMMDD_HHMMSS
   //

ConcatString s;
unixtime t;


s = attrs.lookup_string("init");

t = timestring_to_unix(s.c_str());

dp_out.set_init(t);

     ////////////////////

s = attrs.lookup_string("valid");

t = timestring_to_unix(s.c_str());

dp_out.set_valid(t);

     ////////////////////

s = attrs.lookup_string("lead");

t = timestring_to_sec(s.c_str());

dp_out.set_lead(t);

     ////////////////////

s = attrs.lookup_string("accum");

t = timestring_to_sec(s.c_str());

dp_out.set_accum(t);

     ////////////////////

PyObject * py_grid = attrs.lookup_dict("grid");

grid_from_python_dict(Python3_Dict(py_grid), grid_out);

     ////////////////////

vinfo.set_name       (attrs.lookup_string("name"));
vinfo.set_long_name  (attrs.lookup_string("long_name").c_str());
vinfo.set_level_name (attrs.lookup_string("level").c_str());
vinfo.set_units      (attrs.lookup_string("units").c_str());

     ////////////////////

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////

