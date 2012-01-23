

////////////////////////////////////////////////////////////////////////


#ifndef  __MET_VX_DATA_2D_GRIB_H__
#define  __MET_VX_DATA_2D_GRIB_H__


////////////////////////////////////////////////////////////////////////


#include "data_plane.h"
#include "data_class.h"
#include "grib_classes.h"
#include "var_info_grib.h"
#include "two_to_one.h"


////////////////////////////////////////////////////////////////////////


class MetGrib1DataFile : public Met2dDataFile {

   private:

      void grib1_init_from_scratch();

      MetGrib1DataFile(const MetGrib1DataFile &);
      MetGrib1DataFile & operator=(const MetGrib1DataFile &);

      GribFile * GF;   //  allocated

      GribRecord CurrentRecord;

      DataPlane Plane;

      bool data_plane_winds(VarInfoGrib &, DataPlane &);

      bool data_plane_scalar(VarInfoGrib &, DataPlane &, GribRecord &);

   public:

      MetGrib1DataFile();
     ~MetGrib1DataFile();


         //
         //  set stuff
         //

         //
         //  get stuff
         //

      GrdFileType file_type() const;

      double operator () (int x, int y) const;

      double get         (int x, int y) const;

      bool data_ok       (int x, int y) const;

      void data_minmax(double & data_min, double & data_max) const;

         //
         //  do stuff
         //

      bool open(const char * filename);

      void close();

      void dump(ostream &, int depth = 0) const;

      bool read_record(const int);

      int read_record(const VarInfoGrib &);   //  returns match count (>=0), or -1 on error

      bool data_plane(VarInfo &, DataPlane &);

      int data_plane_array(VarInfo &, DataPlaneArray &);
      
};


////////////////////////////////////////////////////////////////////////


inline double      MetGrib1DataFile::operator  () (int x, int y) const { return ( get(x, y)    ); }
inline GrdFileType MetGrib1DataFile::file_type ()                const { return ( FileType_Gb1 ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_VX_DATA_2D_GRIB_H__  */


////////////////////////////////////////////////////////////////////////


