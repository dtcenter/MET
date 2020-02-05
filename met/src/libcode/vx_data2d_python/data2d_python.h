// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MET_VX_DATA_2D_PYTHON_H__
#define  __MET_VX_DATA_2D_PYTHON_H__


////////////////////////////////////////////////////////////////////////


#include "data_plane.h"
#include "data_class.h"
#include "two_to_one.h"

#include "python_dataplane.h"             //  the main access point
#include "dataplane_from_numpy_array.h"   //  takes a NumPy array and an attributes dictionary
#include "dataplane_from_xarray.h"        //  takes an xarray DataArray
#include "grid_from_python_dict.h"

#include "global_python.h"


////////////////////////////////////////////////////////////////////////


class MetPythonDataFile : public Met2dDataFile {

   private:

      void python_init_from_scratch();

      MetPythonDataFile(const MetPythonDataFile &);
      MetPythonDataFile & operator=(const MetPythonDataFile &);

      ConcatString PythonCommand;

      DataPlane Plane;

      VarInfoPython VInfo;

      GrdFileType Type;   // FileType_Python_Xarray or FileType_Python_Numpy

   public:

      MetPythonDataFile();
     ~MetPythonDataFile();


         //
         //  set stuff
         //

      void set_type(const GrdFileType);

         //
         //  get stuff
         //

      GrdFileType file_type() const;

      double operator () (int x, int y) const;

      double get         (int x, int y) const;

      bool data_ok       (int x, int y) const;

      void data_minmax   (double & data_min, double & data_max) const;

         //
         //  do stuff
         //

      bool open(const char * cur_command);

      void close();


      void dump (ostream &, int depth = 0) const;

      bool data_plane(VarInfo &, DataPlane &);

      int  data_plane_array(VarInfo &, DataPlaneArray &);

      int  index(VarInfo &);

      bool data_plane(DataPlane &);

};


////////////////////////////////////////////////////////////////////////


inline double      MetPythonDataFile::operator  () (int x, int y) const { return ( get(x, y)    ); }
inline GrdFileType MetPythonDataFile::file_type ()                const { return ( Type ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_VX_DATA_2D_PYTHON_H__  */


////////////////////////////////////////////////////////////////////////

