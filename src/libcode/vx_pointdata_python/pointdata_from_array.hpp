// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __MET_POINTDATA_FROM_ARRAY_HPP__
#define  __MET_POINTDATA_FROM_ARRAY_HPP__


////////////////////////////////////////////////////////////////////////

static const int api_debug_level = 11;

////////////////////////////////////////////////////////////////////////


template <typename T>
void load_numpy (void * buf,
                 const int n,
                 const int data_endian,
                 void (*shuf)(void *), 
                 float * out)
{

bool need_swap = (shuf != 0) && (native_endian != data_endian);

int j;
T * u = (T *) buf;
T value;

for (j=0; j<n; ++j)  {

   //numpy_array_one_to_two(j, Nx, r, c);

   memcpy(&value, u + j, sizeof(T));

   if ( need_swap )  shuf(&value);

   out[j] = value;
   
   mlog << Debug(api_debug_level) << "load_numpy(float): [" << j << "] value=" << value << "\n";
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

   //numpy_array_one_to_two(j, Nx, r, c);

   memcpy(&value, u + j, sizeof(T));

   if ( need_swap )  shuf(&value);

   out[j] = (int)value;

   mlog << Debug(api_debug_level) << method_name << "[" << j << "] value=" << value << "\n";
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

   //numpy_array_one_to_two(j, Nx, r, c);

   memcpy(&value, u + j, sizeof(T));

   if ( need_swap )  shuf(&value);

   out->add((int)value);

   mlog << Debug(api_debug_level) << method_name << " [" << j << "] value=" << value << "\n";
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

   //numpy_array_one_to_two(j, Nx, r, c);

   memcpy(&value, u + j, sizeof(T));

   if ( need_swap )  shuf(&value);

   out->add((float)value);

   mlog << Debug(api_debug_level) << method_name << "[" << j << "] value=" << value << "\n";
}   //  for j



return;

}

#endif   /*  __MET_POINTDATA_FROM_NUMPY_ARRAY_HPP__  */


////////////////////////////////////////////////////////////////////////

