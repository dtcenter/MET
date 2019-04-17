// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

      int two_to_one(int _x, int _y) const { return ( _y*Nx + _x ); }   //  might want to generalize this later
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

      void set_size(int _nx, int _ny);

      void set_size(int _nx, int _ny, const T & _initial_value);

         //
         //  get stuff
         //

      int  nx()       const { return ( Nx ); }
      int  ny()       const { return ( Ny ); }
      bool is_empty() const { return ( Nx*Ny == 0 ); }

      const T * data() const { return ( E ); }

      T * buf() { return ( E ); }   //  careful with this

      T operator()(int, int) const;

         //
         //  do stuff
         //

      void put(const T &, int _x, int _y);

      bool s_is_on(int _x, int _y) const;
      bool f_is_on(int _x, int _y) const;

      int x_left  (const int _y) const;
      int x_right (const int _y) const;

};


////////////////////////////////////////////////////////////////////////


template <typename T>

void TwoD_Array<T>::init_from_scratch()

{

E = (T *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void TwoD_Array<T>::clear()

{

if ( E )  { delete [] E;  E = (T *) 0; }

Nx = Ny = 0;

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void TwoD_Array<T>::assign(const TwoD_Array <T> & _t)

{

clear();

if ( ! (_t.E) )  return;

set_size(_t.Nx, _t.Ny);

memcpy(E, _t.E, Nx*Ny*sizeof(T));


return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void TwoD_Array<T>::set_size(int _nx, int _ny)

{

if ( (_nx <= 0) || (_ny <= 0) )  {

   mlog << Error << "\nTwoD_Array::set_size() -> "
        << "bad size ... (" << _nx << ", " << _ny << ")\n\n";

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

void TwoD_Array<T>::set_size(int _nx, int _ny, const T & _initial_value)

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

void TwoD_Array<T>::put(const T & _t, int _x, int _y)

{

E[two_to_one(_x, _y)] = _t;

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

T TwoD_Array<T>::operator()(int _x, int _y) const

{

return ( E[two_to_one(_x, _y)] );

}

////////////////////////////////////////////////////////////////////////


template <typename T>

bool TwoD_Array<T>::s_is_on(int _x, int _y) const

{

return ( (bool) (E[two_to_one(_x, _y)]) );

}


////////////////////////////////////////////////////////////////////////


template <typename T>

bool TwoD_Array<T>::f_is_on(int _x, int _y) const

{

if ( s_is_on(_x, _y) )                                 return ( true );

if( (_x > 0) && s_is_on(_x - 1, _y) )                  return ( true );

if( (_x > 0) && (_y > 0) && s_is_on(_x - 1, _y - 1))   return ( true );

if( (_y > 0 ) && s_is_on(_x, _y - 1) )                 return ( true );

return ( false );

}


////////////////////////////////////////////////////////////////////////


template <typename T>

int TwoD_Array<T>::x_left(const int _y) const

{

if ( (_y < 0) || (_y >= Ny) )  {

   mlog << Error << "\nTwoD_Array<T>::x_left(const int _y) const -> "
        << "range check error ... y = " << _y << "\n\n";

   exit ( 1 );

}

int j;

for (j=0; j<Nx; ++j)  {

   if ( f_is_on(j, _y)  )  return ( j );

}


return ( -1 );

}


////////////////////////////////////////////////////////////////////////


template <typename T>

int TwoD_Array<T>::x_right(const int _y) const

{

if ( (_y < 0) || (_y >= Ny) )  {

   mlog << Error << "\nTwoD_Array<T>::x_right(const int _y) const -> "
        << "range check error ... y = " << _y << "\n\n";

   exit ( 1 );

}

int j;

for (j=(Nx - 1); j>=0; --j)  {

   if ( f_is_on(j, _y)  )  return ( j );

}


return ( -1 );

}


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_TWO_D_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


