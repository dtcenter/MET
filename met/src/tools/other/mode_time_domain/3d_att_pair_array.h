

////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_3D_ATT_PAIR_ARRAY_H__
#define  __MTD_3D_ATT_PAIR_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "3d_att.h"


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

      void set_alloc_inc(int = 0);   //  0 means default value (100)

      int n_elements() const;

      int n         () const;

      void add(const PairAtt3D &);
      void add(const PairAtt3DArray &);

      PairAtt3D & operator[](int) const;

};


////////////////////////////////////////////////////////////////////////


inline int PairAtt3DArray::n_elements() const { return ( Nelements ); }
inline int PairAtt3DArray::n         () const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_3D_ATT_PAIR_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


