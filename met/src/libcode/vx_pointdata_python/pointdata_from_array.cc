// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2021
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#include "string.h"

#include "vx_python3_utils.h"
#include "vx_statistics.h"
#include "check_endian.h"

#include "pointdata_from_array.h"

////////////////////////////////////////////////////////////////////////

static const int api_delug_level = 11;

////////////////////////////////////////////////////////////////////////


template <typename T>
void load_numpy (void * buf,
                 const int n,
                 const int data_endian,
                 void (*shuf)(void *), 
                 float * out)
{

bool need_swap = (shuf != 0) && (native_endian != data_endian);

int j, x, y, r, c;
T * u = (T *) buf;
T value;

for (j=0; j<n; ++j)  {

   //nympy_array_one_to_two(j, Nx, r, c);

   memcpy(&value, u + j, sizeof(T));

   if ( need_swap )  shuf(&value);

   out[j] = value;
   
   mlog << Debug(api_delug_level) << "load_numpy(float): [" << j << "] value=" << value << "\n";
}   //  for j


return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>
void load_numpy (void * buf,
                 const int n,
                 const int data_endian,
                 void (*shuf)(void *), 
                 int * out)
{

const char *method_name = "load_numpy(int *) ";
bool need_swap = (shuf != 0) && (native_endian != data_endian);

int j;
T * u = (T *) buf;
T value;

for (j=0; j<n; ++j)  {

   //nympy_array_one_to_two(j, Nx, r, c);

   memcpy(&value, u + j, sizeof(T));

   if ( need_swap )  shuf(&value);

   out[j] = (int)value;

   mlog << Debug(api_delug_level) << method_name << "[" << j << "] value=" << value << "\n";
}   //  for j


return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>
void load_numpy_int (void * buf,
                     const int n,
                     const int data_endian,
                     void (*shuf)(void *),
                     IntArray *out)
{

const char *method_name = "load_numpy_int(IntArray *) ";
bool need_swap = (shuf != 0) && (native_endian != data_endian);

int j;
T * u = (T *) buf;
T value;

out->extend(n);

for (j=0; j<n; ++j)  {

   //nympy_array_one_to_two(j, Nx, r, c);

   memcpy(&value, u + j, sizeof(T));

   if ( need_swap )  shuf(&value);

   out->add((int)value);

   mlog << Debug(api_delug_level) << method_name << " [" << j << "] value=" << value << "\n";
}   //  for j



return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>
void load_numpy_num (void * buf,
                     const int n,
                     const int data_endian,
                     void (*shuf)(void *),
                     NumArray *out)
{

const char *method_name = "load_numpy_num(NumArray *) ";
bool need_swap = (shuf != 0) && (native_endian != data_endian);

int j;
T * u = (T *) buf;
T value;

out->extend(n);

for (j=0; j<n; ++j)  {

   //nympy_array_one_to_two(j, Nx, r, c);

   memcpy(&value, u + j, sizeof(T));

   if ( need_swap )  shuf(&value);

   out->add((float)value);

   mlog << Debug(api_delug_level) << method_name << "[" << j << "] value=" << value << "\n";
}   //  for j



return;

}


////////////////////////////////////////////////////////////////////////


bool pointdata_from_np_array(Python3_Numpy & np, float * data_out)
{

const char *method_name = "pointdata_from_np_array(float) -> ";

   //
   //  make sure it's a 1D array
   //

if ( np.n_dims() != 1 )  {

   mlog << Error << "\n" << method_name
        << "data array is not 1-dimensional! ... "
        << "(dim = " << (np.n_dims()) << ")\n\n";

   exit ( 1 );


}

int n = np.dim(0);


   //
   //  load the data
   //

const ConcatString dtype = np.dtype();

      //   1 byte integers

     if ( dtype == "|i1"  )   load_numpy <int8_t>    (np.buffer(), n, little_endian,         0, data_out);
else if ( dtype == "|u1"  )   load_numpy <uint8_t>   (np.buffer(), n, little_endian,         0, data_out);

      //   2 byte integers

else if ( dtype == "<i2"  )   load_numpy <int16_t>   (np.buffer(), n, little_endian, shuffle_2, data_out);
else if ( dtype == "<u2"  )   load_numpy <uint16_t>  (np.buffer(), n, little_endian, shuffle_2, data_out);

else if ( dtype == ">i2"  )   load_numpy <int16_t>   (np.buffer(), n,    big_endian, shuffle_2, data_out);
else if ( dtype == ">u2"  )   load_numpy <uint16_t>  (np.buffer(), n,    big_endian, shuffle_2, data_out);

      //   4 byte integers

else if ( dtype == "<i4"  )   load_numpy <int32_t>   (np.buffer(), n, little_endian, shuffle_4, data_out);
else if ( dtype == "<u4"  )   load_numpy <uint32_t>  (np.buffer(), n, little_endian, shuffle_4, data_out);

else if ( dtype == ">i4"  )   load_numpy <int32_t>   (np.buffer(), n,    big_endian, shuffle_4, data_out);
else if ( dtype == ">u4"  )   load_numpy <uint32_t>  (np.buffer(), n,    big_endian, shuffle_4, data_out);

      //   8 byte integers

else if ( dtype == "<i8"  )   load_numpy <int64_t>   (np.buffer(), n, little_endian, shuffle_8, data_out);
else if ( dtype == "<u8"  )   load_numpy <uint64_t>  (np.buffer(), n, little_endian, shuffle_8, data_out);

else if ( dtype == ">i8"  )   load_numpy <int64_t>   (np.buffer(), n,    big_endian, shuffle_8, data_out);
else if ( dtype == ">u8"  )   load_numpy <uint64_t>  (np.buffer(), n,    big_endian, shuffle_8, data_out);

      //   single precision floats

else if ( dtype == "<f4"  )   load_numpy <float>     (np.buffer(), n, little_endian, shuffle_4, data_out);
else if ( dtype == ">f4"  )   load_numpy <float>     (np.buffer(), n,    big_endian, shuffle_4, data_out);

      //   double precision floats

else if ( dtype == "<f8"  )   load_numpy <double>    (np.buffer(), n, little_endian, shuffle_8, data_out);
else if ( dtype == ">f8"  )   load_numpy <double>    (np.buffer(), n,    big_endian, shuffle_8, data_out);

      //
      //   nope ... the only other numerical data type for numpy arrays 
      //            is single or double precision complex numbers, and 
      //            we're not supporting those at this time
      //

else  {

   mlog << Error << "\n" << method_name
        << "unsupported data type \""  << dtype << "\"\n\n";

   exit ( 1 );

}

     ////////////////////


   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool pointdata_from_np_array(Python3_Numpy & np, int * data_out)
{

const char *method_name = "pointdata_from_np_array(int) -> ";

   //
   //  make sure it's a 1D array
   //

if ( np.n_dims() != 1 )  {

   mlog << Error << "\n" << method_name
        << "data array is not 1-dimensional! ... "
        << "(dim = " << (np.n_dims()) << ")\n\n";

   exit ( 1 );


}

int n = np.dim(0);


   //
   //  load the data
   //

const ConcatString dtype = np.dtype();

      //   1 byte integers

     if ( dtype == "|i1"  )   load_numpy <int8_t>    (np.buffer(), n, little_endian,         0, data_out);
else if ( dtype == "|u1"  )   load_numpy <uint8_t>   (np.buffer(), n, little_endian,         0, data_out);

      //   2 byte integers

else if ( dtype == "<i2"  )   load_numpy <int16_t>   (np.buffer(), n, little_endian, shuffle_2, data_out);
else if ( dtype == "<u2"  )   load_numpy <uint16_t>  (np.buffer(), n, little_endian, shuffle_2, data_out);

else if ( dtype == ">i2"  )   load_numpy <int16_t>   (np.buffer(), n,    big_endian, shuffle_2, data_out);
else if ( dtype == ">u2"  )   load_numpy <uint16_t>  (np.buffer(), n,    big_endian, shuffle_2, data_out);

      //   4 byte integers

else if ( dtype == "<i4"  )   load_numpy <int32_t>   (np.buffer(), n, little_endian, shuffle_4, data_out);
else if ( dtype == "<u4"  )   load_numpy <uint32_t>  (np.buffer(), n, little_endian, shuffle_4, data_out);

else if ( dtype == ">i4"  )   load_numpy <int32_t>   (np.buffer(), n,    big_endian, shuffle_4, data_out);
else if ( dtype == ">u4"  )   load_numpy <uint32_t>  (np.buffer(), n,    big_endian, shuffle_4, data_out);

      //   8 byte integers

else if ( dtype == "<i8"  )   load_numpy <int64_t>   (np.buffer(), n, little_endian, shuffle_8, data_out);
else if ( dtype == "<u8"  )   load_numpy <uint64_t>  (np.buffer(), n, little_endian, shuffle_8, data_out);

else if ( dtype == ">i8"  )   load_numpy <int64_t>   (np.buffer(), n,    big_endian, shuffle_8, data_out);
else if ( dtype == ">u8"  )   load_numpy <uint64_t>  (np.buffer(), n,    big_endian, shuffle_8, data_out);

      //   single precision floats

else if ( dtype == "<f4"  )   load_numpy <float>     (np.buffer(), n, little_endian, shuffle_4, data_out);
else if ( dtype == ">f4"  )   load_numpy <float>     (np.buffer(), n,    big_endian, shuffle_4, data_out);

      //   double precision floats

else if ( dtype == "<f8"  )   load_numpy <double>    (np.buffer(), n, little_endian, shuffle_8, data_out);
else if ( dtype == ">f8"  )   load_numpy <double>    (np.buffer(), n,    big_endian, shuffle_8, data_out);

      //
      //   nope ... the only other numerical data type for numpy arrays 
      //            is single or double precision complex numbers, and 
      //            we're not supporting those at this time
      //

else  {

   mlog << Error << "\n" << method_name
        << "unsupported data type \""  << dtype << "\"\n\n";

   exit ( 1 );

}

     ////////////////////


   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool pointdata_from_np_array(Python3_Numpy & np, IntArray *data_out)
{

const char *method_name = "pointdata_from_np_array(IntArray) -> ";

   //
   //  make sure it's a 1D array
   //

if ( np.n_dims() != 1 )  {

   mlog << Error << "\n" << method_name
        << "data array is not 1-dimensional! ... "
        << "(dim = " << (np.n_dims()) << ")\n\n";

   exit ( 1 );


}

int n = np.dim(0);


   //
   //  load the data
   //

IntArray data;
const ConcatString dtype = np.dtype();

      //   1 byte integers

     if ( dtype == "|i1"  ) load_numpy_int <int8_t>    (np.buffer(), n, little_endian,         0, data_out);
else if ( dtype == "|u1"  ) load_numpy_int <uint8_t>   (np.buffer(), n, little_endian,         0, data_out);

      //   2 byte integers

else if ( dtype == "<i2"  ) load_numpy_int <int16_t>   (np.buffer(), n, little_endian, shuffle_2, data_out);
else if ( dtype == "<u2"  ) load_numpy_int <uint16_t>  (np.buffer(), n, little_endian, shuffle_2, data_out);

else if ( dtype == ">i2"  ) load_numpy_int <int16_t>   (np.buffer(), n,    big_endian, shuffle_2, data_out);
else if ( dtype == ">u2"  ) load_numpy_int <uint16_t>  (np.buffer(), n,    big_endian, shuffle_2, data_out);

      //   4 byte integers

else if ( dtype == "<i4"  ) load_numpy_int <int32_t>   (np.buffer(), n, little_endian, shuffle_4, data_out);
else if ( dtype == "<u4"  ) load_numpy_int <uint32_t>  (np.buffer(), n, little_endian, shuffle_4, data_out);

else if ( dtype == ">i4"  ) load_numpy_int <int32_t>   (np.buffer(), n,    big_endian, shuffle_4, data_out);
else if ( dtype == ">u4"  ) load_numpy_int <uint32_t>  (np.buffer(), n,    big_endian, shuffle_4, data_out);

      //   8 byte integers

else if ( dtype == "<i8"  ) load_numpy_int <int64_t>   (np.buffer(), n, little_endian, shuffle_8, data_out);
else if ( dtype == "<u8"  ) load_numpy_int <uint64_t>  (np.buffer(), n, little_endian, shuffle_8, data_out);

else if ( dtype == ">i8"  ) load_numpy_int <int64_t>   (np.buffer(), n,    big_endian, shuffle_8, data_out);
else if ( dtype == ">u8"  ) load_numpy_int <uint64_t>  (np.buffer(), n,    big_endian, shuffle_8, data_out);

      //   single precision floats

else if ( dtype == "<f4"  ) load_numpy_int <float>     (np.buffer(), n, little_endian, shuffle_4, data_out);
else if ( dtype == ">f4"  ) load_numpy_int <float>     (np.buffer(), n,    big_endian, shuffle_4, data_out);

      //   double precision floats

else if ( dtype == "<f8"  ) load_numpy_int <double>    (np.buffer(), n, little_endian, shuffle_8, data_out);
else if ( dtype == ">f8"  ) load_numpy_int <double>    (np.buffer(), n,    big_endian, shuffle_8, data_out);

      //
      //   nope ... the only other numerical data type for numpy arrays 
      //            is single or double precision complex numbers, and 
      //            we're not supporting those at this time
      //

else  {

   mlog << Error << "\n" << method_name
        << "unsupported data type \""  << dtype << "\"\n\n";

   exit ( 1 );

}

     ////////////////////


   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool pointdata_from_np_array(Python3_Numpy & np, NumArray *data_out)
{

const char *method_name = "pointdata_from_np_array(NumArray) -> ";

   //
   //  make sure it's a 1D array
   //

if ( np.n_dims() != 1 )  {

   mlog << Error << "\n" << method_name
        << "data array is not 1-dimensional! ... "
        << "(dim = " << (np.n_dims()) << ")\n\n";

   exit ( 1 );


}

int n = np.dim(0);


   //
   //  load the data
   //

NumArray data;
const ConcatString dtype = np.dtype();

      //   1 byte integers

     if ( dtype == "|i1"  ) load_numpy_num <int8_t>    (np.buffer(), n, little_endian,         0, data_out);
else if ( dtype == "|u1"  ) load_numpy_num <uint8_t>   (np.buffer(), n, little_endian,         0, data_out);

      //   2 byte integers

else if ( dtype == "<i2"  ) load_numpy_num <int16_t>   (np.buffer(), n, little_endian, shuffle_2, data_out);
else if ( dtype == "<u2"  ) load_numpy_num <uint16_t>  (np.buffer(), n, little_endian, shuffle_2, data_out);

else if ( dtype == ">i2"  ) load_numpy_num <int16_t>   (np.buffer(), n,    big_endian, shuffle_2, data_out);
else if ( dtype == ">u2"  ) load_numpy_num <uint16_t>  (np.buffer(), n,    big_endian, shuffle_2, data_out);

      //   4 byte integers

else if ( dtype == "<i4"  ) load_numpy_num <int32_t>   (np.buffer(), n, little_endian, shuffle_4, data_out);
else if ( dtype == "<u4"  ) load_numpy_num <uint32_t>  (np.buffer(), n, little_endian, shuffle_4, data_out);

else if ( dtype == ">i4"  ) load_numpy_num <int32_t>   (np.buffer(), n,    big_endian, shuffle_4, data_out);
else if ( dtype == ">u4"  ) load_numpy_num <uint32_t>  (np.buffer(), n,    big_endian, shuffle_4, data_out);

      //   8 byte integers

else if ( dtype == "<i8"  ) load_numpy_num <int64_t>   (np.buffer(), n, little_endian, shuffle_8, data_out);
else if ( dtype == "<u8"  ) load_numpy_num <uint64_t>  (np.buffer(), n, little_endian, shuffle_8, data_out);

else if ( dtype == ">i8"  ) load_numpy_num <int64_t>   (np.buffer(), n,    big_endian, shuffle_8, data_out);
else if ( dtype == ">u8"  ) load_numpy_num <uint64_t>  (np.buffer(), n,    big_endian, shuffle_8, data_out);

      //   single precision floats

else if ( dtype == "<f4"  ) load_numpy_num <float>     (np.buffer(), n, little_endian, shuffle_4, data_out);
else if ( dtype == ">f4"  ) load_numpy_num <float>     (np.buffer(), n,    big_endian, shuffle_4, data_out);

      //   double precision floats

else if ( dtype == "<f8"  ) load_numpy_num <double>    (np.buffer(), n, little_endian, shuffle_8, data_out);
else if ( dtype == ">f8"  ) load_numpy_num <double>    (np.buffer(), n,    big_endian, shuffle_8, data_out);

      //
      //   nope ... the only other numerical data type for numpy arrays 
      //            is single or double precision complex numbers, and 
      //            we're not supporting those at this time
      //

else  {

   mlog << Error << "\n" << method_name
        << "unsupported data type \""  << dtype << "\"\n\n";

   exit ( 1 );

}

     ////////////////////


   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool pointdata_from_str_array(PyObject *data_array, StringArray *data_out)
{

//const char *method_name = "pointdata_from_str_array(StringArray) -> ";

StringArray a = pyobject_as_string_array(data_array);
data_out->clear();
data_out->add(a);

     ////////////////////


   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////

   //
   //  we just grab the numpy array and the attributes dictionary
   //
   //    from the xarray DataArray object, and then hand them
   //
   //    off to pointdata_from_numpy_array
   //

bool pointdata_from_xarray(PyObject * data_array, float *data_out)
{

Python3_Numpy np;
PyObject *numpy_array = PyObject_GetAttrString(data_array, data_attr_name);

   /////////////////////

np.set(numpy_array);

bool status = pointdata_from_np_array(np, data_out);

   //
   //  done
   //

return ( status );

}


////////////////////////////////////////////////////////////////////////

   //
   //  we just grab the numpy array and the attributes dictionary
   //
   //    from the xarray DataArray object, and then hand them
   //
   //    off to pointdata_from_numpy_array
   //

bool pointdata_from_xarray(PyObject * data_array, int *data_out)
{

Python3_Numpy np;
PyObject *numpy_array = PyObject_GetAttrString(data_array, data_attr_name);


   /////////////////////


np.set(numpy_array);

bool status = pointdata_from_np_array(np, data_out);

   //
   //  done
   //

return ( status );

}


////////////////////////////////////////////////////////////////////////
