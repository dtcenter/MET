// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_3D_ATT_PAIR_ARRAY_H__
#define  __MTD_3D_ATT_PAIR_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "3d_att.h"
#include "mm_engine.h"


////////////////////////////////////////////////////////////////////////


class PairAtt3DArray {

   private:

      void init_from_scratch();

      void assign(const PairAtt3DArray &);

      void extend(int);


      int Nelements;

      int Nalloc;

      int AllocInc;

      PairAtt3D * e;


   public:

      PairAtt3DArray();
     ~PairAtt3DArray();
      PairAtt3DArray(const PairAtt3DArray &);
      PairAtt3DArray & operator=(const PairAtt3DArray &);

      void clear();

      void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //


         //
         //  get stuff
         //

      int n_elements() const;

      int n         () const;

      PairAtt3D & operator[](int) const;

      int fcst_obj_number(int index) const;  //  one-based
      int  obs_obj_number(int index) const;  //  one-based

      int fcst_cluster_number(int index) const;  //  one-based
      int  obs_cluster_number(int index) const;  //  one-based

      double total_interest(int index) const;

         //
         //  do stuff
         //

      void set_alloc_inc(int = 0);   //  0 means default value (100)

      void add(const PairAtt3D &);
      void add(const PairAtt3DArray &);

      void patch_cluster_numbers(const MM_Engine &);


};


////////////////////////////////////////////////////////////////////////


inline int PairAtt3DArray::n_elements() const { return ( Nelements ); }
inline int PairAtt3DArray::n         () const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_3D_ATT_PAIR_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


