// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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

bool pointdata_from_python_list(PyObject * data_array, float *data_out)
{

   // Support PyFloat, PyLong and numpy.float32 type
   for (int idx=0; idx<PyList_Size(data_array); idx++) {
      *(data_out+idx) = (float)PyFloat_AsDouble(PyList_GetItem(data_array, idx));
   }
   return ( true );

}

////////////////////////////////////////////////////////////////////////

bool pointdata_from_python_list(PyObject * data_array, NumArray *data_out)
{

   // Support PyFloat, PyLong and numpy.float32 type
   for (int idx=0; idx<PyList_Size(data_array); idx++) {
      data_out->add((float)PyFloat_AsDouble(PyList_GetItem(data_array, idx)));
   }

   return ( true );

}

////////////////////////////////////////////////////////////////////////


bool pointdata_from_python_list(PyObject * data_array, int *data_out)
{
   bool status = false;
   PyObject* item;
   item = PyList_GetItem(data_array, 0);
   if (PyLong_Check(item)) {
      for (int idx=0; idx<PyList_Size(data_array); idx++) {
         *(data_out+idx) = (int)PyLong_AsLong(PyList_GetItem(data_array, idx));
      }
      status = true;
   }
   else {
       mlog << Error << "\nOnly int type is supported at python list."
            << " Please check the data type\n\n";
       // exit by caller with additional log message
   }

   return ( status );
}

////////////////////////////////////////////////////////////////////////

bool pointdata_from_python_list(PyObject * data_array, IntArray *data_out)
{
   bool status = false;
   PyObject* item;
   item = PyList_GetItem(data_array, 0);
   if (PyLong_Check(item)) {
      for (int idx=0; idx<PyList_Size(data_array); idx++) {
         data_out->add((int)PyLong_AsLong(PyList_GetItem(data_array, idx)));
      }
      status = true;
   }
   else {
       mlog << Error << "\nOnly int type is supported at python list."
            << " Please check the data type\n\n";
       // exit by caller with additional log message
   }

   return ( status );

}


////////////////////////////////////////////////////////////////////////
