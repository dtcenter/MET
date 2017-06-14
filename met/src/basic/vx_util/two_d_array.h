// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __MET_TWO_D_ARRAY_H__
#define  __MET_TWO_D_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include <string.h>

#include "logger.h"


////////////////////////////////////////////////////////////////////////


template <typename T>

class TwoD_Array {

   protected:

      void init_from_scratch();

      void assign(const TwoD_Array <T> &);

      int two_to_one(int _x, int_y) const { return ( _y*Nx + x ); }   //  might want to generalize this later
                                                                      //  also might want to add range checking

      int Nx;
      int Ny;

      T * E;



   public:

      TwoD_Array() { init_from_scratch(); }

     ~TwoD_Array() { clear(); }

      TwoD_Array(const TwoD_Array <T> & _t) { init_from_scratch();  assign(_t); }

      TwoD_Array <T> & operator=(const TwoD_Array <T> & _t)  {

         if ( this == &_t )  return ( * this );

         assign(_t);

         return ( * this );

      }

         //////////////////////////

      void clear();

         //
         //  set stuff
         //

      set_size(int _nx, int _ny);

      set_size(int _nx, int _ny, const T & _initial_value);

         //
         //  get stuff
         //

      int nx() const { return ( Nx ); }
      int ny() const { return ( Ny ); }

      T * e() { return ( E ); }   //  careful with this

      T operator()(int, int) const;

         //
         //  do stuff
         //

      void put(const T &, int _x, int _y);

};


////////////////////////////////////////////////////////////////////////


template <typename T>

void TwoD_Array::init_from_scratch()

{

E = (T *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void TwoD_Array::clear()

{

if ( E )  { delete [] E;  E = (T *) 0; }

Nx = Ny = 0;

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void TwoD_Array::assign(const TwoD_Array <T> & _t)

{

clear();

if ( ! (_t.E) )  return;

set_size(_t.Nx, _t.Ny);

memcpy(E, _t.E, Nx*Ny*sizeof(T));


return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void TwoD_Array::set_size(int _nx, int _ny)

{

if ( (_nx <= 0) || (_ny <= 0) )  {

   mlog << Error
        << "TwoD_Array::set_size() -> bad size ... ("
        << _nx << ", " << _ny << ")\n\n";

   exit ( 1 );

}

clear();

E = new T [_nx*_ny];

Nx = _nx;
Ny = _ny;

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void TwoD_Array::set_size(int _nx, int _ny, const T & _initial_value)

{

set_size(_nx, _ny);

int j;
const int nxy = Nx*Ny;

   //
   //  don't want to use memcpy here in case T is a class
   //

for (j=0; j<nxy; ++j)  {

   E[j] = _initial_value;

}


return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void TwoD_Array::put(const T & _t, int _x, int _y)

{

E[two_to_one(_x, _y)] = _t;

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

T TwoD_Array::operator()(int _x, int _y) const

{

return ( E[two_to_one(_x, _y)] );

}

////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_TWO_D_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


