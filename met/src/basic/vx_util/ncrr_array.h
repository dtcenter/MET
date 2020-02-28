// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __NONCONTIGUOUS_RETURN_REFERENCE_ARRAY_H__
#define  __NONCONTIGUOUS_RETURN_REFERENCE_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string.h>


////////////////////////////////////////////////////////////////////////


typedef int (ncrr_cmp_func)(const void *, const void *);


////////////////////////////////////////////////////////////////////////


static const int ncrr_default_alloc_inc = 20;


////////////////////////////////////////////////////////////////////////


template <typename T>

class NCRR_Array {

   protected:

      void init_from_scratch();

      void assign(const NCRR_Array &);



      int Nelements;

      int Nalloc;

      int AllocInc;

      T ** e;


   public:

      NCRR_Array()  { init_from_scratch(); }

     ~NCRR_Array()  { clear(); }

      NCRR_Array(const NCRR_Array & _a)  { init_from_scratch();  assign(_a); }

      NCRR_Array & operator=(const NCRR_Array & _a)  {

         if ( this == _a )  return ( * this );

         assign(_a);

         return ( * this );

      }

      void clear();

      void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

      void set_alloc_inc(int = 0);   //  0 means default value

         //
         //  get stuff
         //

      int n_elements() const  { return ( Nelements );}

      int n         () const  { return ( Nelements );}

      T & operator[](int) const;

         //
         //  do stuff
         //

      void add(const T &);
      void add(const NCRR_Array &);

      void bubble_sort_increasing(ncrr_cmp_func);
      void bubble_sort_decreasing(ncrr_cmp_func);

      void qsort_increasing(ncrr_cmp_func);
      void qsort_decreasing(ncrr_cmp_func);

      void extend(int, bool exact = true);

      void reverse();   //  reverse the order of the elements

};


////////////////////////////////////////////////////////////////////////


   //
   //   Code for class NCRR_Array
   //


////////////////////////////////////////////////////////////////////////


template <typename T>

void NCRR_Array<T>::init_from_scratch()

{

e = (T **) 0;

AllocInc = ncrr_default_alloc_inc;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void NCRR_Array<T>::clear()

{

if ( e )  {

   int j;

   for (j=0; j<Nalloc; ++j)  {

      if ( e[j] )  { delete e[j];  e[j] = (T *) 0; }

   }

   delete [] e;  e = (T **) 0;

}   //  if e


Nelements = 0;

Nalloc = 0;

// AllocInc = ncrr_default_alloc_inc;   //  don't reset AllocInc


return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void NCRR_Array<T>::assign(const NCRR_Array & a)

{

clear();

if ( a.n_elements() == 0 )  return;

add(a);

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void NCRR_Array<T>::extend(int N, bool exact)

{

if ( N <= Nalloc )  return;

if ( ! exact )  {

   N = AllocInc*( (N + AllocInc - 1)/AllocInc );

}

int j;
T ** u = new T * [N];

if ( !u )  {

   mlog << Error << "\nNCRR_Array<T>::extend(int, bool) -> "
        << "memory allocation error\n\n";

   exit ( 1 );

}

memset(u, 0, N*sizeof(T *));

for(j=0; j<Nelements; ++j)  {

   u[j] = e[j];

}

if ( e )  { delete [] e;  e = (T **) 0; }

e = u;

u = (T **) 0;

Nalloc = N;

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void NCRR_Array<T>::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Nelements = " << Nelements << "\n";
out << prefix << "Nalloc    = " << Nalloc    << "\n";
out << prefix << "AllocInc  = " << AllocInc  << "\n";

int j;

for(j=0; j<Nelements; ++j)  {

   out << prefix << "Element # " << j << " ... \n";

   e[j]->dump(out, depth + 1);

}

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void NCRR_Array<T>::set_alloc_inc(int N)

{

if ( N < 0 )  {

   mlog << Error << "\nNCRR_Array<T>::set_alloc_int(int) -> "
        << "bad value ... " << N << "\n\n";

   exit ( 1 );

}

if ( N == 0 )  AllocInc = ncrr_default_alloc_inc;
else           AllocInc = N;

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void NCRR_Array<T>::add(const T & a)

{

extend(Nelements + 1, false);

e[Nelements] = new T;

if ( !(e[Nelements]) )  {

   mlog << Error << "\nNCRR_Array<T>::add(const T &) -> "
        << "memory allocation error\n\n";

   exit ( 1 );

}

*(e[Nelements++]) = a;

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void NCRR_Array<T>::add(const NCRR_Array & a)

{

int j;

extend(Nelements + a.n_elements());

for (j=0; j<(a.n_elements()); ++j)  {

   add(a[j]);

}

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

T & NCRR_Array<T>::operator[](int N) const

{

if ( (N < 0) || (N >= Nelements) )  {

   mlog << Error << "\nNCRR_Array<T>::operator[](int) -> "
        << "range check error ... " << N << "\n\n";

   exit ( 1 );
}

return ( *(e[N]) );

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void NCRR_Array<T>::reverse()

{

if ( Nelements < 2 )  return;

int j, k;
int jmax;
T * temp = 0;

jmax = Nelements/2;   //  works whether Nelements is even or odd

k = Nelements - 1;

for (j=0; j<jmax; ++j, --k)  {

   temp = e[j];

   e[j] = e[k];

   e[k] = temp;

}


return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void NCRR_Array<T>::bubble_sort_decreasing(ncrr_cmp_func _cmp)

{

if ( Nelements < 2 )  return;

int j, k;
T * temp = 0;


for (j=0; j<(Nelements - 1); ++j) {

   for (k=(j + 1); k<Nelements; ++k) {

      if ( _cmp(e[j], e[k]) < 0 )  {

         temp = e[j];

         e[j] = e[k];

         e[k] = temp;

      }

   }

}


return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void NCRR_Array<T>::bubble_sort_increasing(ncrr_cmp_func _cmp)

{

if ( Nelements < 2 )  return;

bubble_sort_decreasing(_cmp);

reverse();


return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void NCRR_Array<T>::qsort_increasing(ncrr_cmp_func _cmp)

{

if ( Nelements < 2 )  return;

qsort(e, Nelements, sizeof(*e), _cmp);   //  sort in increasing order

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void NCRR_Array<T>::qsort_decreasing(ncrr_cmp_func _cmp)

{

if ( Nelements < 2 )  return;

qsort(e, Nelements, sizeof(*e), _cmp);   //  sort in increasing order

reverse();

return;

}



////////////////////////////////////////////////////////////////////////


#endif   /*  __NONCONTIGUOUS_RETURN_REFERENCE_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


