// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MET_VX_DATA_2D_NC_CF_H__
#define  __MET_VX_DATA_2D_NC_CF_H__


////////////////////////////////////////////////////////////////////////


#include <netcdf>
using namespace netCDF;

#include "data_plane.h"
#include "data_class.h"
#include "var_info_nccf.h"
#include "nccf_file.h"
#include "two_to_one.h"


////////////////////////////////////////////////////////////////////////


class MetNcCFDataFile : public Met2dDataFile {

   private:

      void nccf_init_from_scratch();
      NcVarInfo *find_first_data_var();
      long convert_time_to_offset(long time_dim_value);
      LongArray collect_time_offsets(VarInfo &vinfo);

      MetNcCFDataFile(const MetNcCFDataFile &);
      MetNcCFDataFile & operator=(const MetNcCFDataFile &);

      //
      //  NetCDF file
      //
      
      NcCfFile * _file;    //  allocated
      long _time_dim_offset; // current time offset to get the data plane

   public:

      MetNcCFDataFile();
     ~MetNcCFDataFile();

      virtual int nx() const
      {
         if (_file == 0)
            return 0;
    
         return _file->getNx();
      }
      

      virtual int ny() const
      {
         if (_file == 0)
            return 0;
    
         return _file->getNy();
      }
      

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

      void dump(ostream &, int = 0) const;

};


////////////////////////////////////////////////////////////////////////


inline GrdFileType MetNcCFDataFile::file_type () const { return ( FileType_NcCF ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_VX_DATA_2D_NC_CF_H__  */


////////////////////////////////////////////////////////////////////////


