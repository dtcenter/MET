

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __SHP_ARRAY_H__
#define  __SHP_ARRAY_H__


////////////////////////////////////////////////////////////////////////


   //   contiguous, return copy


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_util.h"
#include "vx_log.h"
#include <vector>


////////////////////////////////////////////////////////////////////////


template <typename T>

class Shp_Array {

   private:

      void init_from_scratch();

      void assign(const Shp_Array <T>  &);



      int AllocInc;

      std::vector<T> E;

   public:

      Shp_Array() { init_from_scratch(); }

     ~Shp_Array() { clear(); }

      Shp_Array(const Shp_Array <T> & _a) { init_from_scratch();  assign(_a); }

      Shp_Array(Shp_Array <T> && _a) noexcept
         : AllocInc(_a.AllocInc), E(std::move(_a.E)) {

         _a.E.clear();

      }

      Shp_Array<T>  & operator=(const Shp_Array <T> & _a) {

         if ( this == &_a )  return *this;

         assign(_a);

         return *this;

      }

      Shp_Array<T>  & operator=(Shp_Array <T> && _a) noexcept {

         if ( this == &_a )  return *this;

         clear();

         AllocInc  = _a.AllocInc;
         E         = std::move(_a.E);

         _a.E.clear();

         return *this;

      }


      void clear();

      void dump(std::ostream &, int = 0) const;

      void extend(int, bool exact = true);

         //
         //  set stuff
         //

      void set_alloc_inc(int = 20);

      void set_n_elements(int);   //  careful with this


         //
         //  get stuff
         //

      int n() const { return (int) E.size(); }

      int n_elements() const { return (int) E.size(); }

         //
         //  do stuff
         //

      void add(const T &);

      void add(const Shp_Array <T> &);

      const T operator[](int) const;

      T * buf() const;   //  for writing all at once

};


////////////////////////////////////////////////////////////////////////


template <typename T>

void Shp_Array<T>::init_from_scratch()

{

AllocInc = 25;   //  default value

clear();

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void Shp_Array<T>::clear()

{

E.clear();

// AllocInc = 25;   //  don't reset AllocInc


return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void Shp_Array<T>::assign(const Shp_Array<T> & a)

{

clear();

if ( a.n_elements() == 0 )  return;

add(a);

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void Shp_Array<T>::extend(int N, bool exact)

{

if ( ! exact )  {

   N = AllocInc*( (N + AllocInc - 1)/AllocInc );

}

E.reserve(N);

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void Shp_Array<T>::dump(std::ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "(int) E.size() = " << E.size() << "\n";
out << prefix << "Nalloc    = " << E.capacity() << "\n";
out << prefix << "AllocInc  = " << AllocInc  << "\n";


int j;

for(j=0; j<(int) E.size(); ++j)  {

   out << prefix << "Element # " << j << " ... \n";

   E[j].dump(out, depth + 1);

}


out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void Shp_Array<T>::set_alloc_inc(int N)

{

if ( N < 0 )  {

   mlog << Error << "Shp_Array::set_alloc_int(int) -> "
        << "bad value ... " << N << "\n\n";

   exit ( 1 );

}

if ( N == 0 )  AllocInc = 25;   //  default value
else           AllocInc = N;

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void Shp_Array<T>::set_n_elements(int N)

{

if ( N < 0 )  {

   mlog << Error << "Shp_Array::set_n_elements(int) -> "
        << "bad value ... " << N << "\n\n";

   exit ( 1 );

}

E.resize(N);

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void Shp_Array<T>::add(const T & a)

{

E.push_back(a);

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void Shp_Array<T>::add(const Shp_Array<T> & a)

{

int j;

extend((int) E.size() + a.n_elements());

for (j=0; j<(a.n_elements()); ++j)  {

   add(a[j]);

}

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

const T Shp_Array<T>::operator[](int N) const

{

if ( (N < 0) || (N >= (int) E.size()) )  {

   mlog << Error << "\n\n  Shp_Array::operator[](int) -> "
        << "range check error ... " << N << "\n\n";

   exit ( 1 );
}

return E[N];

}


////////////////////////////////////////////////////////////////////////


template <typename T>

T * Shp_Array<T>::buf() const

{

return const_cast<T *>(E.data());

}


////////////////////////////////////////////////////////////////////////


#endif   /*  __CONTIGUOUS_RETURN_COPY_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


