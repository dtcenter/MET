// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __MET_PYTHON_LIST_H__
#define  __MET_PYTHON_LIST_H__


////////////////////////////////////////////////////////////////////////


#include "concat_string.h"


////////////////////////////////////////////////////////////////////////


extern "C" {

#include "Python.h"

}


////////////////////////////////////////////////////////////////////////


class Python3_List {

   private:

      void clear();

      void init_from_scratch();


      Python3_List(const Python3_List &);
      Python3_List & operator=(const Python3_List &);


      PyObject * Object;   //  the list, not allocated

      int Size;   //  the size of the list


   public:

      Python3_List();
      Python3_List(PyObject *);
     ~Python3_List();

      // void dump(std::ostream &, int depth) const;

      void set(PyObject *);

      int size() const;


      PyObject * operator[](int) const;

};


////////////////////////////////////////////////////////////////////////


inline int Python3_List::size() const { return ( Size ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_PYTHON_LIST_H__  */


////////////////////////////////////////////////////////////////////////

