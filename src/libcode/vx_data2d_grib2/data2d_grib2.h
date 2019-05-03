

////////////////////////////////////////////////////////////////////////


#ifndef  __MET_VX_DATA_2D_GRIB2_H__
#define  __MET_VX_DATA_2D_GRIB2_H__


////////////////////////////////////////////////////////////////////////


#include "data_plane.h"
#include "data_class.h"
#include "var_info_grib2.h"


////////////////////////////////////////////////////////////////////////


class MetGrib2DataFile : public Met2dDataFile {

   private:

      void grib2_init_from_scratch();

      MetGrib2DataFile(const MetGrib2DataFile &);
      MetGrib2DataFile & operator=(const MetGrib2DataFile &);

   public:

      MetGrib2DataFile();
     ~MetGrib2DataFile();

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

         //
         //  do stuff
         //

      bool open  (const char * filename);

      void close ();

      void dump(ostream &, int = 0) const;

};


////////////////////////////////////////////////////////////////////////


inline GrdFileType MetGrib2DataFile::file_type () const { return ( FileType_Gb2 ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_VX_DATA_2D_GRIB2_H__  */


////////////////////////////////////////////////////////////////////////


