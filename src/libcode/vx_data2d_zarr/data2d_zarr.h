// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __MET_VX_DATA_2D_ZARR_H__
#define  __MET_VX_DATA_2D_ZARR_H__

////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <vector>

#include "data_plane.h"
#include "data_class.h"
#include "var_info_zarr.h"

////////////////////////////////////////////////////////////////////////

class MetZarrDataFile : public Met2dDataFile {

   private:

      void zarr_init_from_scratch();

      MetZarrDataFile(const MetZarrDataFile &);
      MetZarrDataFile & operator=(const MetZarrDataFile &);

   public:

      MetZarrDataFile();
     ~MetZarrDataFile();

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

      bool open(const char * path);
      void close();
      void dump(std::ostream &, int = 0) const;
};

////////////////////////////////////////////////////////////////////////

inline GrdFileType MetZarrDataFile::file_type () const { return FileType_Zarr; }

////////////////////////////////////////////////////////////////////////

#endif   /*  __MET_VX_DATA_2D_ZARR_H__  */

////////////////////////////////////////////////////////////////////////

