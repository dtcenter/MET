

////////////////////////////////////////////////////////////////////////


#ifndef  __MET_PYTHON3_SCRIPT_H__
#define  __MET_PYTHON3_SCRIPT_H__


////////////////////////////////////////////////////////////////////////


#include "concat_string.h"
#include "string_array.h"


////////////////////////////////////////////////////////////////////////


extern "C" {

#include "Python.h"

}


////////////////////////////////////////////////////////////////////////


class Python3_Script {

   private:

      void clear();

      Python3_Script(const Python3_Script &);
      Python3_Script & operator=(const Python3_Script &);
      Python3_Script();

      PyObject * Module;   //  not allocated

      PyObject * Dict;     //  script dictionary, not allocated

      ConcatString Script_Filename;


   public:

      Python3_Script(const char * _script_filename);
     ~Python3_Script();

         //
         //  set stuff
         //


         //
         //  get stuff
         //

      ConcatString filename() const;

      PyObject * module();
      PyObject * dict();

         //
         //  do stuff
         //


      void reset_argv(const char * script_name, const StringArray & args);


      PyObject * lookup(const char * name) const;

      void run(const char * command) const;   //  runs a command in the namespace of the script

      void read_pickle  (const char * variable_name, const char * pickle_filename) const;

};


////////////////////////////////////////////////////////////////////////


inline PyObject * Python3_Script::module() { return ( Module ); }

inline PyObject * Python3_Script::dict() { return ( Dict ); }

inline ConcatString Python3_Script::filename() const { return ( Script_Filename ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_PYTHON3_SCRIPT_H__  */


////////////////////////////////////////////////////////////////////////

