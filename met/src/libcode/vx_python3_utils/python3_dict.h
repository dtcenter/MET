// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __MET_PYTHON_DICT_H__
#define  __MET_PYTHON_DICT_H__


////////////////////////////////////////////////////////////////////////


#include "concat_string.h"


////////////////////////////////////////////////////////////////////////


extern "C" {

#include "Python.h"

}


////////////////////////////////////////////////////////////////////////


class Python3_Dict {

   private:

      void clear();

      void init_from_scratch();


      Python3_Dict(const Python3_Dict &);
      Python3_Dict & operator=(const Python3_Dict &);


      PyObject * Object;   //  the dictionary, not allocated

      void set_from_dict   (PyObject *);
      void set_from_module (PyObject *);

      int Size;   //  the size of the dictionary


   public:

      Python3_Dict();
      Python3_Dict(PyObject *);
     ~Python3_Dict();

      void dump(std::ostream &, int depth) const;

      void set(PyObject *);   //  dict or module

      int size() const;


      int           lookup_int    (const char * key) const;
      double        lookup_double (const char * key) const;
      ConcatString  lookup_string (const char * key) const;

      PyObject *    lookup_dict   (const char * key) const;
      PyObject *    lookup_list   (const char * key) const;

};


////////////////////////////////////////////////////////////////////////


inline int Python3_Dict::size() const { return ( Size ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_PYTHON_DICT_H__  */


////////////////////////////////////////////////////////////////////////

