// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __CONTIGUOUS_RETURNS_COPY_ARRAY_H__
#define  __CONTIGUOUS_RETURNS_COPY_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <vector>

#include "num_array.h"
#include "is_bad_data.h"
#include "nint.h"
#include "vx_cal.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


template <typename T>

class CRC_Array {

   private:

      void init_from_scratch();

      void assign(const CRC_Array &);

      std::vector<T> e;

   public:

      CRC_Array() { init_from_scratch(); }

     ~CRC_Array() { clear(); }

      CRC_Array(const CRC_Array <T> & _a)  { init_from_scratch();  assign(_a); }

      CRC_Array(CRC_Array <T> && _a) noexcept : e(move(_a.e)) {}

      CRC_Array & operator=(const CRC_Array <T> & _a)  {

         if ( this == &_a )  return *this;

         assign(_a);

         return *this;

      }

      CRC_Array & operator=(CRC_Array <T> && _a) noexcept {

         if ( this == &_a )  return *this;

         e = move(_a.e);

         return *this;

      }

      CRC_Array <T> & operator=(const NumArray &);

      bool operator==(const CRC_Array <T> &) const;

      void clear();

      void extend(int);

      void dump(std::ostream &, int depth = 0) const;

      void dump_one_line(std::ostream & out, int depth) const;

         //
         //  set stuff
         //


         //
         //  get stuff
         //

      int n_elements() const { return (int) e.size(); }
      int n         () const { return (int) e.size(); }

      T operator[] (int) const;

         //
         //  do stuff
         //

      bool has(const T &, bool forward=true) const;
      bool has(const T &, int & index, bool forward=true) const;

      void add(const T &);
      void add(const CRC_Array <T> &);
      void add_uniq(const T &, bool forward=true);
      void add_css_sec(const char *);

      void set(const T & val);
      void set(int ix, const T & val);

      void sort_increasing();

      void increment(const T &);   //  adds a constant value to all elements

      T sum() const;
      T min() const;
      T max() const;

};


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class CRC_Array
   //


////////////////////////////////////////////////////////////////////////


template <typename T>

CRC_Array <T> & CRC_Array<T>::operator=(const NumArray & a)

{

clear();

for(int j=0; j<a.n_elements(); ++j) add(nint(a[j]));

return *this;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

bool CRC_Array<T>::operator==(const CRC_Array<T> & a) const

{

if ( n() != a.n() )  return false;

for(int j=0; j<n(); ++j)  {
   if(e[j] != a.e[j])  return false;
}

return true;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRC_Array<T>::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRC_Array<T>::clear()

{

e.clear();

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRC_Array<T>::assign(const CRC_Array <T> & a)

{

clear();

if ( a.n() == 0 )  return;

extend(a.n());

e = a.e;

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRC_Array<T>::extend(int len)

{

e.reserve(len);

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRC_Array<T>::dump(std::ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Nelements = " << n() << "\n";

for (int j=0; j<n(); ++j)  {

   out << prefix << "Element # " << j << " = " << e[j] << "\n";

}

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRC_Array<T>::dump_one_line(std::ostream & out, int depth) const

{

int j;
Indent prefix(depth);

out << prefix << '(' << n() << ") ";

for (j=0; j<n(); ++j)  {

   if ( j > 0 )  out << ' ';

   out << e[j];

}

   //
   //  done
   //

 out << '\n' << std::flush;

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRC_Array<T>::set(const T & elem)

{

clear();

add(elem);

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRC_Array<T>::set(int ix, const T & elem)

{
   if ( (ix < 0) || (ix >= n()) )  {

      mlog << Error << "\nCRC_Array::set(int, T) const -> "
           << "range check error ... index = " << ix 
           << ", Nelements = " << n() 
           << "\n\n";

      exit ( 1 );

   }

   e[ix] = elem;
}

////////////////////////////////////////////////////////////////////////


template <typename T>

T CRC_Array<T>::operator[](int i) const

{

if ( (i < 0) || (i >= n()) )  {

   mlog << Error << "\nCRC_Array::operator[](int) const -> "
        << "range check error ... index = " << i 
        << ", Nelements = " << n()
        << "\n\n";

   exit ( 1 );

}

return e[i];

}


////////////////////////////////////////////////////////////////////////


template <typename T>

bool CRC_Array<T>::has(const T & k, bool forward) const

{

bool found = false;
if (forward) {
   for (int j=0; j<n(); ++j)  {
      if ( e[j] == k ) {
          found = true;
          break;
      }
   }
}
else {
   for (int j=n()-1; j>=0; --j)  {
      if ( e[j] == k ) {
          found = true;
          break;
      }
   }
}

return found;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

bool CRC_Array<T>::has(const T & k, int & index, bool forward) const

{

bool found = false;

index = -1;

if (forward) {
   for (int j=0; j<n(); ++j)  {
      if ( e[j] == k )  { index = j; found = true; break; }
   }
}
else {
   for (int j=n()-1; j>=0; --j)  {
      if ( e[j] == k )  { index = j; found = true; break; }
   }
}

return found;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRC_Array<T>::add(const T & k)

{

e.emplace_back(k);

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRC_Array<T>::add(const CRC_Array<T> & a)

{

extend(n() + a.n());

for (int j=0; j<(a.n()); ++j)  {

   e.emplace_back(a.e[j]);

}


return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRC_Array<T>::add_uniq(const T & k, bool forward)

{

if ( !has(k, forward) )  add(k);

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRC_Array<T>::add_css_sec(const char * text)

{

StringArray sa;

sa.parse_css(text);

extend(n() + sa.n());

int j;

for (j=0; j<(sa.n()); j++)  {

  add(timestring_to_sec(sa[j].c_str()));

}

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRC_Array<T>::sort_increasing()

{

if ( n() <= 1 )  return;

std::sort(e.begin(), e.end());

return;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

T CRC_Array<T>::sum() const

{

T s = 0;

int count;

for(int j=0, count=0; j<n(); j++) {

   if ( is_bad_data(e[j]) ) continue;

   s += e[j];

   count++;
}

if ( count == 0 )  s = bad_data_double;

return s;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

T CRC_Array<T>::min() const

{

if ( n() == 0 )  return bad_data_int;

T min_v = e[0];

for(int j=0; j<n(); j++) {

   if ( is_bad_data(e[j]) ) continue;

   if ( e[j] < min_v )  min_v = e[j];

}

return min_v;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

T CRC_Array<T>::max() const

{

if(n() == 0) return bad_data_int;

T max_v = e[0];

for(int j=0; j<n(); j++) {

   if ( is_bad_data(e[j]) )  continue;

   if ( e[j] > max_v )  max_v = e[j];

}

return max_v;

}


////////////////////////////////////////////////////////////////////////


template <typename T>

void CRC_Array<T>::increment(const T & k)

{

for (int j=0; j<n(); ++j)  {

   e[j] += k;

}

return;

}


////////////////////////////////////////////////////////////////////////


#endif   /*  __CONTIGUOUS_RETURNS_COPY_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


