// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __MET_VX_DATA_2D_NC_MET_RA_H__
#define  __MET_VX_DATA_2D_NC_MET_RA_H__

////////////////////////////////////////////////////////////////////////

#include "data_plane.h"
#include "data_class.h"
#include "var_info_nc_met.h"
#include "met_file.h"
#include "two_to_one.h"

////////////////////////////////////////////////////////////////////////

class MetNcMetRADataFile : public Met2dDataFile {

   private:

      void nc_met_init_from_scratch();

      MetNcMetRADataFile(const MetNcMetRADataFile &);
      MetNcMetRADataFile & operator=(const MetNcMetRADataFile &);

   public:

      MetNcMetRADataFile();
     ~MetNcMetRADataFile();

         //
         //  NetCDF file
         //
      
      MetNcFile * MetNc;    //  allocated

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


inline GrdFileType MetNcMetRADataFile::file_type () const { return FileType_NcMetRA; }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_VX_DATA_2D_NC_MET_RA_H__  */


////////////////////////////////////////////////////////////////////////


