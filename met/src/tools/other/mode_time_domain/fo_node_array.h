// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_FO_NODE_ARRAY_H__
#define  __MTD_FO_NODE_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "fo_node.h"


////////////////////////////////////////////////////////////////////////


class FO_Node_Array {

   private:

      void init_from_scratch();

      void assign(const FO_Node_Array &);

      void extend(int);


      int Nelements;

      int Nalloc;

      int AllocInc;

      FO_Node * e;


   public:

      FO_Node_Array();
     ~FO_Node_Array();
      FO_Node_Array(const FO_Node_Array &);
      FO_Node_Array & operator=(const FO_Node_Array &);

      void clear();

      void dump(ostream &, int = 0) const;

      void set_alloc_inc(int = 0);   //  0 means default value (30)

      int n_elements() const;

      int n         () const;

      void add(const FO_Node &);
      void add(const FO_Node_Array &);

      FO_Node & operator[](int) const;

};


////////////////////////////////////////////////////////////////////////


inline int FO_Node_Array::n_elements() const { return ( Nelements ); }
inline int FO_Node_Array::n         () const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_FO_NODE_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


