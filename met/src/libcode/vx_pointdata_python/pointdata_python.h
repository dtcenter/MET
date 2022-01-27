// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2021
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


//class MetPythonPointDataFile : public Met2dDataFile {
class MetPythonPointDataFile {

   private:

      void python_init_from_scratch();

      MetPythonPointDataFile(const MetPythonPointDataFile &);
      MetPythonPointDataFile & operator=(const MetPythonPointDataFile &);

      ConcatString PythonCommand;

      MetPointDataPython met_data;

      //VarInfoPython VInfo;

      //GrdFileType Type;   // FileType_Python_Xarray or FileType_Python_Numpy

   public:

      MetPythonPointDataFile();
     ~MetPythonPointDataFile();


         //
         //  set stuff
         //

      //void set_type(const GrdFileType);

         //
         //  get stuff
         //

//      GrdFileType file_type() const;

//      double operator () (int x, int y) const;

      //double get         (int x, int y) const;

//      bool data_ok       (int x, int y) const;

//      void data_minmax   (double & data_min, double & data_max) const;

         //
         //  do stuff
         //

      bool open(const char * cur_command, bool use_xarray);

      void close();


      void dump (ostream &, int depth = 0) const;

      //MetPointData  get_met_point_data();
      MetPointDataPython *get_met_point_data();

      //int  data_plane_array(VarInfo &, DataPlaneArray &);

      //bool met_point_data(MetPointData &);

};


////////////////////////////////////////////////////////////////////////


//inline double      MetPythonPointDataFile::operator  () (int x, int y) const { return ( get(x, y)    ); }
//inline GrdFileType MetPythonPointDataFile::file_type ()                const { return ( Type ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_VX_POINTDATA_PYTHON_H__  */


////////////////////////////////////////////////////////////////////////

