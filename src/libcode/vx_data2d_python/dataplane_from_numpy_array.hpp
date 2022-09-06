// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __MET_DATAPLANE_FROM_NUMPY_ARRAY_HPP__
#define  __MET_DATAPLANE_FROM_NUMPY_ARRAY_HPP__


////////////////////////////////////////////////////////////////////////

extern inline void numpy_array_one_to_two(const int n, const int Ncols, int & row, int & col);

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

   numpy_array_one_to_two(j, Nx, r, c);

   x = c;

   y = Ny - 1 - r;

   memcpy(&value, u + j, sizeof(T));

   if ( need_swap )  shuf(&value);

   out.set((double) value, x, y);

}   //  for j



return;

}

////////////////////////////////////////////////////////////////////////

#endif   /*  __MET_DATAPLANE_FROM_NUMPY_ARRAY_HPP__  */


////////////////////////////////////////////////////////////////////////

