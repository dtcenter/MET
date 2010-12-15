// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __CONFIG_ARRAY_H__
#define  __CONFIG_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


class ArrayInfo;   //  forward reference


////////////////////////////////////////////////////////////////////////


#include "indent.h"



#ifndef  __ICODE_H__

#include "icode.h"

#endif   /*  __ICODE_H__  */


////////////////////////////////////////////////////////////////////////


static const int max_array_dim = 10;


////////////////////////////////////////////////////////////////////////


class ArrayInfo {

   private:

      void init_from_scratch();

      void assign(const ArrayInfo &);

      void delete_v();


      int Dim;

      int sizes[max_array_dim];

      int Nalloc;

      IcodeVector ** v;

   public:

      ArrayInfo();
     ~ArrayInfo();
      ArrayInfo(const ArrayInfo &);
      ArrayInfo & operator=(const ArrayInfo &);

      void clear();

      void set(const int *, int);


      void put(int, const IcodeVector &);
      void put(const int *, const IcodeVector &);

      const IcodeVector * get(int) const;
      const IcodeVector * get(const int * indices) const;


      int dim() const;

      int n_alloc() const;

      int size(int) const;

      int indices_to_n(const int *) const;

      void n_to_indices(int, int *) const;

      void dump(ostream &, int depth = 0) const;

};


////////////////////////////////////////////////////////////////////////


inline int ArrayInfo::dim() const { return ( Dim ); }

inline int ArrayInfo::n_alloc() const { return ( Nalloc ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __CONFIG_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


