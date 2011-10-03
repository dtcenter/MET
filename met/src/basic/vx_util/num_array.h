// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2011
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __NUM_ARRAY_H__
#define  __NUM_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


static const int num_array_alloc_inc = 1000;


////////////////////////////////////////////////////////////////////////


class NumArray {

   private:

      void init_from_scratch();

      void assign(const NumArray &);

      void extend(int);

      double * e;

      int Nelements;

      int Nalloc;

      bool Sorted;

   public:

      NumArray();
     ~NumArray();
      NumArray(const NumArray &);
      NumArray & operator=(const NumArray &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      double operator[](int) const;

      int is_bad_data(int) const;

      int has(int)    const;
      int has(double) const;

      void add(int);
      void add(double);
      void add(const NumArray &);

      void set(int, int);
      void set(int, double);

      void   sort_array();
      void   reorder(const NumArray &);
      int    rank_array(int &);
      double percentile_array(double);
      void   compute_mean_stdev(double &, double &) const;
      double mean() const;
      double sum() const;
      double mode() const;
      double min() const;
      double max() const;

      int n_elements() const;

};


////////////////////////////////////////////////////////////////////////


inline int NumArray::n_elements() const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __NUM_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


