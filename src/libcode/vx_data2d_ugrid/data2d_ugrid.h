// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MET_VX_DATA_2D_UGRID_H__
#define  __MET_VX_DATA_2D_UGRID_H__


////////////////////////////////////////////////////////////////////////


#include "data_plane.h"
#include "data_class.h"
#include "var_info_ugrid.h"
#include "ugrid_file.h"
#include "two_to_one.h"


////////////////////////////////////////////////////////////////////////


class MetUGridDataFile : public Met2dDataFile {

   private:

      void ugrid_init_from_scratch();
      //NcVarInfo *find_first_data_var();
      long convert_time_to_offset(long time_value);
      long convert_value_to_offset(double z_value, std::string z_dim_name);
      LongArray collect_time_offsets(VarInfo &vinfo);

      MetUGridDataFile(const MetUGridDataFile &);
      MetUGridDataFile & operator=(const MetUGridDataFile &);

      //
      //  NetCDF file
      //
      
      UGridFile * _file;    //  allocated
      long _cur_time_index; // current time index to get the data plane (for array of data_plane)

   protected:

      ConcatString meta_filename;

   public:

      MetUGridDataFile();
     ~MetUGridDataFile();

      virtual int nx() const
      {
         if (_file == 0)
            return 0;
    
         return _file->getNx();
      }
      

      virtual int ny() const
      {
         return 1;
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
      bool open_metadata(const char * filename);

      void close ();

      void dump(std::ostream &, int = 0) const;

};


////////////////////////////////////////////////////////////////////////


inline GrdFileType MetUGridDataFile::file_type () const { return ( FileType_UGrid ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_VX_DATA_2D_UGRID_H__  */


////////////////////////////////////////////////////////////////////////


