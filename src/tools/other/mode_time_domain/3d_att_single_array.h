

////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_3D_ATT_SINGLE_ARRAY_H__
#define  __MTD_3D_ATT_SINGLE_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "3d_att.h"


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

};


////////////////////////////////////////////////////////////////////////


inline int SingleAtt3DArray::n_elements() const { return ( Nelements ); }
inline int SingleAtt3DArray::n         () const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_3D_ATT_SINGLE_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


