// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __NONCONTIGUOUS_RETURN_REFERENCE_ARRAY_H__
#define  __NONCONTIGUOUS_RETURN_REFERENCE_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string.h>
#include <vector>


////////////////////////////////////////////////////////////////////////


typedef int (ncrr_cmp_func)(const void *, const void *);


////////////////////////////////////////////////////////////////////////


template <typename T>

class NCRR_Array {

   protected:

      void assign(const NCRR_Array &);

         //
         //  Each element is held by pointer so that operator[] can hand back a
         //  reference that stays valid as the array grows.
         //

      std::vector<std::unique_ptr<T>> e;


   public:

      NCRR_Array() = default;

     ~NCRR_Array() = default;

      NCRR_Array(const NCRR_Array & _a)  { assign(_a); }

      NCRR_Array(NCRR_Array &&) noexcept = default;

      NCRR_Array & operator=(const NCRR_Array & _a)  {

         if ( this == &_a )  return *this;

         assign(_a);

         return *this;

      }

      NCRR_Array & operator=(NCRR_Array &&) noexcept = default;

      void clear();

      void dump(std::ostream &, int = 0) const;

         //
         //  get stuff
         //

      int n_elements() const  { return (int) e.size(); }

      int n         () const  { return (int) e.size(); }

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

      void reverse();   //  reverse the order of the elements

};


////////////////////////////////////////////////////////////////////////


   //
   //   Code for class NCRR_Array
   //


////////////////////////////////////////////////////////////////////////


template <typename T>

void NCRR_Array<T>::clear()

{

e.clear();

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

void NCRR_Array<T>::dump(std::ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Nelements = " << n_elements() << "\n";

int j;

for(j=0; j<n_elements(); ++j)  {

   out << prefix << "Element # " << j << " ... \n";

   e[j]->dump(out, depth + 1);

}

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void NCRR_Array<T>::add(const T & a)

{

e.push_back(std::make_unique<T>());

*(e.back()) = a;

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void NCRR_Array<T>::add(const NCRR_Array & a)

{

int j;

e.reserve(e.size() + a.n_elements());

for (j=0; j<(a.n_elements()); ++j)  {

   add(a[j]);

}

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

T & NCRR_Array<T>::operator[](int N) const

{

if ( (N < 0) || (N >= n_elements()) )  {

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

std::reverse(e.begin(), e.end());

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  ncrr_cmp_func is a C-style comparator: it is handed the addresses of two
   //  elements, which in the old raw T ** array meant T **.  The wrapper below
   //  keeps that contract.
   //


template <typename T>

void NCRR_Array<T>::bubble_sort_decreasing(ncrr_cmp_func _cmp)

{

std::stable_sort(e.begin(), e.end(),
   [_cmp](const std::unique_ptr<T> & a, const std::unique_ptr<T> & b) {
      const T * pa = a.get();
      const T * pb = b.get();
      return ( _cmp(&pa, &pb) > 0 );
   });

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void NCRR_Array<T>::bubble_sort_increasing(ncrr_cmp_func _cmp)

{

bubble_sort_decreasing(_cmp);

reverse();

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void NCRR_Array<T>::qsort_increasing(ncrr_cmp_func _cmp)

{

std::stable_sort(e.begin(), e.end(),
   [_cmp](const std::unique_ptr<T> & a, const std::unique_ptr<T> & b) {
      const T * pa = a.get();
      const T * pb = b.get();
      return ( _cmp(&pa, &pb) < 0 );
   });

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void NCRR_Array<T>::qsort_decreasing(ncrr_cmp_func _cmp)

{

qsort_increasing(_cmp);

reverse();

return;

}



////////////////////////////////////////////////////////////////////////


#endif   /*  __NONCONTIGUOUS_RETURN_REFERENCE_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


