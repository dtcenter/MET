

////////////////////////////////////////////////////////////////////////


#ifndef  __MET_PYTHON3_NUMPY_H__
#define  __MET_PYTHON3_NUMPY_H__


////////////////////////////////////////////////////////////////////////


extern "C" {

#include "Python.h"

}


////////////////////////////////////////////////////////////////////////


#include <iostream>


#include "concat_string.h"

#include "python3_script.h"


////////////////////////////////////////////////////////////////////////


class Python3_Numpy {

   private:

      void clear();

      void init_from_scratch();

      Python3_Numpy(const Python3_Numpy &);
      Python3_Numpy & operator=(const Python3_Numpy &);


      PyObject * Object;        //  the numpy array, not allocated

      Python3_Script * Script;  //  not allocated, possibly null

      ConcatString Name;        //  the numpy array variable name, possibly empty


      int N_Dims;

      int * Dim;                //  allocated

      long Item_Size;

      int N_Data;

      PyObject * Data_Obj;      //  python data object, not allocated

      Py_buffer View;           //  buffer view

      void * Buffer;            //  the actual data buffer, not allocated

      ConcatString Dtype;       //  data type


   public:

      Python3_Numpy();
     ~Python3_Numpy();

      void dump(std::ostream &) const;

         //
         //  set stuff
         //

      void set(Python3_Script & , const char * _name);

      void set(PyObject *);

      void set_name(const char *);

      void set_script(Python3_Script &);

         //
         //  get stuff
         //

      const char * name () const;

      PyObject * object () const;

      PyObject * data_obj () const;

      Py_buffer & view ();

      void * buffer () const;

      int n_dims() const;

      int n_data() const;

      int dim(int) const;   //  0-based

      const char * dtype() const;

      long item_size() const;


         //
         //  do stuff
         //


};


////////////////////////////////////////////////////////////////////////


inline int Python3_Numpy::n_dims() const { return ( N_Dims ); }

inline int Python3_Numpy::n_data() const { return ( N_Data ); }

inline long Python3_Numpy::item_size() const { return ( Item_Size ); }

inline PyObject * Python3_Numpy::object() const { return ( Object ); }

inline PyObject * Python3_Numpy::data_obj() const { return ( Data_Obj ); }

inline Py_buffer & Python3_Numpy::view() { return ( View ); }

inline void * Python3_Numpy::buffer() const { return ( Buffer ); }

inline const char * Python3_Numpy::name() const { return ( Name.text() ); }

inline const char * Python3_Numpy::dtype() const { return ( Dtype.text() ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_PYTHON3_NUMPY_H__  */


////////////////////////////////////////////////////////////////////////

