

   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
   // ** Copyright UCAR (c) 1992 - 2012
   // ** University Corporation for Atmospheric Research (UCAR)
   // ** National Center for Atmospheric Research (NCAR)
   // ** Research Applications Lab (RAL)
   // ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __MET_VX_DATA_2D_GRIB2_H__
#define  __MET_VX_DATA_2D_GRIB2_H__


////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <vector>

#include "data_plane.h"
#include "data_class.h"
#include "var_info_grib2.h"

extern "C" {
  #include "grib2.h"
}


////////////////////////////////////////////////////////////////////////


typedef struct {
   long ByteOffset;
   int NumFields;
   int RecNum;
   int FieldNum;
   int Discipline;
   int PdsTmpl;
   ConcatString ParmName;
   int ParmCat;
   int Parm;
   int LvlTyp;
   double LvlVal1;
   double LvlVal2;
   int RangeTyp;
   int RangeVal;
   int ResCompFlag;
   double ProbLower;
   double ProbUpper;
} Grib2Record;


////////////////////////////////////////////////////////////////////////


class MetGrib2DataFile : public Met2dDataFile {

   private:

      void grib2_init_from_scratch();

      MetGrib2DataFile(const MetGrib2DataFile &);
      MetGrib2DataFile & operator=(const MetGrib2DataFile &);

      FILE *FileGrib2;

      vector <Grib2Record*> RecList;

      map<string,string> PairMap;
      map<string,Grib2Record*> NameRecMap;

      int ScanMode;


      //
      //  utilities to read a GRIB2 information
      //

      bool read_data_plane(DataPlane &plane, gribfield  *gfld);

      void read_grid(gribfield *gfld);

      long read_grib2_record( long offset,
                              g2int unpack,
                              g2int ifld,
                              gribfield* &gfld,
                              unsigned char* &cgrib,
                              g2int &numfields
                            );

      void read_grib2_record_list();

      void update_var_info(VarInfoGrib2* vinfo, Grib2Record *rec);

      DataPlane check_uv_rotation( VarInfoGrib2 *vinfo,
                                   Grib2Record *rec,
                                   DataPlane plane
                                 );

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

      static const char* build_magic(Grib2Record *rec);

};


////////////////////////////////////////////////////////////////////////


inline GrdFileType MetGrib2DataFile::file_type () const { return ( FileType_Gb2 ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_VX_DATA_2D_GRIB2_H__  */


////////////////////////////////////////////////////////////////////////


