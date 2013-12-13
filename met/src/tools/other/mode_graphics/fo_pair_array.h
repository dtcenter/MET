

////////////////////////////////////////////////////////////////////////


   //
   //  Warning:  This file is machine generated
   //
   //            Do not edit by hand
   //
   //
   //  Created by arraygen on December 12, 2013   11:56 am   MST
   //


////////////////////////////////////////////////////////////////////////


#ifndef  __VX_FO_PAIR_ARRAY_H__
#define  __VX_FO_PAIR_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "fo_pair.h"


////////////////////////////////////////////////////////////////////////


class FO_PairArray {

   private:

      void init_from_scratch();

      void assign(const FO_PairArray &);

      void extend(int);


      int Nelements;

      int Nalloc;

      int AllocInc;

      FO_Pair * e;


   public:

      FO_PairArray();
     ~FO_PairArray();
      FO_PairArray(const FO_PairArray &);
      FO_PairArray & operator=(const FO_PairArray &);

      void clear();

      void dump(ostream &, int = 0) const;

      void set_alloc_inc(int = 0);   //  0 means default value (20)

      int n_elements() const;

      void add(const FO_Pair &);
      void add(const FO_PairArray &);

      FO_Pair operator[](int) const;

};


////////////////////////////////////////////////////////////////////////


inline int FO_PairArray::n_elements() const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_FO_PAIR_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


