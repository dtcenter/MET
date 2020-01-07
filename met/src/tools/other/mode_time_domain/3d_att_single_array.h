// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_3D_ATT_SINGLE_ARRAY_H__
#define  __MTD_3D_ATT_SINGLE_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "3d_att.h"
#include "mm_engine.h"


////////////////////////////////////////////////////////////////////////


class SingleAtt3DArray {

   private:

      void init_from_scratch();

      void assign(const SingleAtt3DArray &);

      void extend(int);


      int Nelements;

      int Nalloc;

      int AllocInc;

      SingleAtt3D * e;


   public:

      SingleAtt3DArray();
     ~SingleAtt3DArray();
      SingleAtt3DArray(const SingleAtt3DArray &);
      SingleAtt3DArray & operator=(const SingleAtt3DArray &);

      void clear();

      void dump(ostream &, int = 0) const;

      void set_alloc_inc(int = 0);   //  0 means default value (100)

      int n_elements() const;

      int n         () const;

      void add(const SingleAtt3D &);
      void add(const SingleAtt3DArray &);

      SingleAtt3D & operator[](int) const;

      void patch_cluster_numbers(const MM_Engine &);

};


////////////////////////////////////////////////////////////////////////


inline int SingleAtt3DArray::n_elements() const { return ( Nelements ); }
inline int SingleAtt3DArray::n         () const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_3D_ATT_SINGLE_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


