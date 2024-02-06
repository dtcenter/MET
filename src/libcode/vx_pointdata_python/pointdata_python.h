// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MET_VX_POINTDATA_PYTHON_H__
#define  __MET_VX_POINTDATA_PYTHON_H__


////////////////////////////////////////////////////////////////////////


#include "data_plane.h"
#include "data_class.h"
#include "two_to_one.h"

#include "python_pointdata.h"             //  the main access point
#include "pointdata_from_array.h"        //  takes an NumPy array or xarray DataArray

#include "global_python.h"


////////////////////////////////////////////////////////////////////////


class MetPythonPointDataFile {

   private:

      void python_init_from_scratch();

      MetPythonPointDataFile(const MetPythonPointDataFile &);
      MetPythonPointDataFile & operator=(const MetPythonPointDataFile &);

      ConcatString PythonCommand;

      MetPointDataPython met_data;

   public:

      MetPythonPointDataFile();
     ~MetPythonPointDataFile();


         //
         //  set stuff
         //

         //
         //  get stuff
         //

      bool is_using_var_id() const;

         //
         //  do stuff
         //

      bool open(const char * cur_command, bool use_xarray);

      void close();


      void dump (std::ostream &, int depth = 0) const;

      MetPointDataPython *get_met_point_data();

};


////////////////////////////////////////////////////////////////////////


inline bool MetPythonPointDataFile::is_using_var_id() const { return(met_data.is_using_var_id()); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_VX_POINTDATA_PYTHON_H__  */


////////////////////////////////////////////////////////////////////////

