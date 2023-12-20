// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MET_VX_DATA_2D_NC_WRF_H__
#define  __MET_VX_DATA_2D_NC_WRF_H__


////////////////////////////////////////////////////////////////////////


#include <netcdf>

#include "data_plane.h"
#include "data_class.h"
#include "var_info_nc_wrf.h"
#include "wrf_file.h"
#include "two_to_one.h"


////////////////////////////////////////////////////////////////////////


class MetNcWrfDataFile : public Met2dDataFile {

   private:

      void nc_pinterp_init_from_scratch();

      MetNcWrfDataFile(const MetNcWrfDataFile &);
      MetNcWrfDataFile & operator=(const MetNcWrfDataFile &);

         //
         //  NetCDF file
         //
      
      PinterpFile * PinterpNc;  //  allocated

   public:

      MetNcWrfDataFile();
     ~MetNcWrfDataFile();
     
         //
         //  set stuff
         //

         //
         //  get stuff
         //

      GrdFileType file_type() const;

         //  retrieve the first matching data plane

      bool data_plane(VarInfo &, DataPlane &);

         //  retrieve all matching data planes

      int data_plane_array(VarInfo &, DataPlaneArray &);

         //  retrieve the index of the first matching record

      int index(VarInfo &);

         //
         //  do stuff
         //

      bool open  (const char * filename);

      void close ();

      void dump(std::ostream &, int = 0) const;

};


////////////////////////////////////////////////////////////////////////


inline GrdFileType MetNcWrfDataFile::file_type () const { return ( FileType_NcPinterp ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_VX_DATA_2D_NC_WRF_H__  */


////////////////////////////////////////////////////////////////////////


