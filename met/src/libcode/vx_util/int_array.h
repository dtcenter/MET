// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2011
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __INT_ARRAY_H__
#define  __INT_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


static const int intarray_alloc_inc = 1000;


////////////////////////////////////////////////////////////////////////


class IntArray {

   private:

      void init_from_scratch();

      void assign(const IntArray &);

      void extend(int);


      int * e;

      int Nelements;

      int Nalloc;

   public:

      IntArray();
     ~IntArray();
      IntArray(const IntArray &);
      IntArray & operator=(const IntArray &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      int operator[](int) const;

      int has(int) const;
      int has(int, int & index) const;

      void add(int);
      void add(const IntArray &);

      int n_elements() const;

      void sort_increasing();

      int sum() const;

};


////////////////////////////////////////////////////////////////////////


inline int IntArray::n_elements() const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __INT_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


