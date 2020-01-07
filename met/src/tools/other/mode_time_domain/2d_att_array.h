// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_2D_ATT_ARRAY_H__
#define  __MTD_2D_ATT_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "2d_att.h"
#include "mm_engine.h"


////////////////////////////////////////////////////////////////////////


class SingleAtt2DArray {

   private:

      void init_from_scratch();

      void assign(const SingleAtt2DArray &);

      void extend(int);


      int Nelements;

      int Nalloc;

      int AllocInc;

      SingleAtt2D * e;


   public:

      SingleAtt2DArray();
     ~SingleAtt2DArray();
      SingleAtt2DArray(const SingleAtt2DArray &);
      SingleAtt2DArray & operator=(const SingleAtt2DArray &);

      void clear();

      void dump(ostream &, int = 0) const;

      void set_alloc_inc(int = 0);   //  0 means default value (50)

      int n_elements() const;

      int n         () const;

      void add(const SingleAtt2D &);
      void add(const SingleAtt2DArray &);

      SingleAtt2D & operator[](int) const;

      void patch_cluster_numbers(const MM_Engine &);

      unixtime valid_time(int index) const;

      int      lead_time(int index) const;


      int      time_index(int index) const;

};


////////////////////////////////////////////////////////////////////////


inline int SingleAtt2DArray::n_elements() const { return ( Nelements ); }
inline int SingleAtt2DArray::n         () const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_2D_ATT_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


