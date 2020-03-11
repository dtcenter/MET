// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __CONTIGUOUS_RETURN_REFERENCE_ARRAY_H__
#define  __CONTIGUOUS_RETURN_REFERENCE_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_util.h"


////////////////////////////////////////////////////////////////////////


template <typename T>

class CRR_Array {

   private:

      void init_from_scratch();

      void assign(const CRR_Array <T>  &);


      int Nelements;

      int Nalloc;

      int AllocInc;

      T * e;

   public:

      CRR_Array() { init_from_scratch(); }

     ~CRR_Array() { clear(); }

      CRR_Array(const CRR_Array <T> & _a) { init_from_scratch();  assign(_a); }

      CRR_Array<T>  & operator=(const CRR_Array <T> & _a) {

         if ( this == &_a )  return ( * this );

         assign(_a);

         return ( * this );

      }


      void clear();

      void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

      void set_alloc_inc(int = 20);

      void set_all(const T);


         //
         //  get stuff
         //

      int n() const { return ( Nelements ); }

         //
         //  do stuff
         //

      void add(const T &);

      void add(const CRR_Array <T> &);

      T & operator[](int) const;

      void extend(int, bool exact = true);

      void extend(int, const T);

      void set_size(int);

};


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRR_Array<T>::init_from_scratch()

{

e = (T *) 0;

AllocInc = 25;   //  default value

clear();

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRR_Array<T>::clear()

{

if ( e )  { delete [] e;  e = (T *) 0; }



Nelements = 0;

Nalloc = 0;

// AllocInc = 25;   //  don't reset AllocInc


return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRR_Array<T>::assign(const CRR_Array<T> & a)

{

clear();

if ( a.n() == 0 )  return;

add(a);

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRR_Array<T>::extend(int N, bool exact)

{

if ( N <= Nalloc )  return;

if ( ! exact )  {

   N = AllocInc*( (N + AllocInc - 1)/AllocInc );

}

int j;
T * u = new T [N];

if ( !u )  {

   mlog << Error << "\nCRR_Array::extend(int, bool) -> "
        << "memory allocation error\n\n";

   exit ( 1 );

}

for(j=0; j<Nelements; ++j)  {

   u[j] = e[j];

}

if ( e )  { delete [] e;  e = (T *) 0; }

e = u;

u = (T *) 0;

Nalloc = N;

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRR_Array<T>::extend(int N, const T _value)

{

if ( N <= Nalloc )  return;

const int n_old  = Nalloc;

extend(N);

int j;

for (j=n_old; j<Nalloc; ++j)  e[j] = _value;




return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRR_Array<T>::set_all(const T _value)

{

int j;

for (j=0; j<Nelements; ++j)  e[j] = _value;

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRR_Array<T>::set_size(int N)

{

extend(N);

if ( N > Nelements )  Nelements = N;

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRR_Array<T>::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Nelements = " << Nelements << "\n";
out << prefix << "Nalloc    = " << Nalloc    << "\n";
out << prefix << "AllocInc  = " << AllocInc  << "\n";


int j;

for(j=0; j<Nelements; ++j)  {

   out << prefix << "Element # " << j << " ... \n";

   e[j].dump(out, depth + 1);

}


out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRR_Array<T>::set_alloc_inc(int N)

{

if ( N < 0 )  {

   mlog << Error << "\nCRR_Array::set_alloc_int(int) -> "
        << "bad value ... " << N << "\n\n";

   exit ( 1 );

}

if ( N == 0 )  AllocInc = 25;   //  default value
else           AllocInc = N;

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRR_Array<T>::add(const T & a)

{

extend(Nelements + 1, false);

e[Nelements++] = a;

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRR_Array<T>::add(const CRR_Array<T> & a)

{

int j;

extend(Nelements + a.n());

for (j=0; j<(a.n()); ++j)  {

   add(a[j]);

}

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

T & CRR_Array<T>::operator[](int N) const

{

if ( (N < 0) || (N >= Nelements) )  {

   mlog << Error << "\nCRR_Array::operator[](int) -> "
        << "range check error ... " << N << "\n\n";

   exit ( 1 );
}

return ( e[N] );

}


////////////////////////////////////////////////////////////////////////


#endif   /*  __CONTIGUOUS_RETURN_REFERENCE_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


