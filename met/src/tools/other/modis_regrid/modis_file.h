// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MODIS_FILE_H__
#define  __MODIS_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "cloudsat_swath_file.h"

#include "vx_log.h"
#include "vx_cal.h"


////////////////////////////////////////////////////////////////////////


class ModisFile {

   private:

      ModisFile(const ModisFile &);
      ModisFile & operator=(const ModisFile &);

   protected:

      void init_from_scratch();


      void get_geo_field   (SwathDataField * &, const char * name);
      void get_data_field  (SwathDataField * &, const char * name);

      bool get_double_data (SwathDataField *, int, int, double &) const;
      bool get_float_data  (SwathDataField *, int, int, float  &) const;
      bool get_int16_data  (SwathDataField *, int, int, short  &) const;
      bool get_int8_data   (SwathDataField *, int, int, char   &) const;

      // void get_scale_offset();


      ConcatString Filename;

      int FileId;

      int Dim0;
      int Dim1;

      unixtime ScanStartTime;

      CloudsatSwath * Swath;   //  allocated

         //
         //  geolocation fields
         //

      SwathDataField * Latitude;    //  not allocated
      SwathDataField * Longitude;   //  not allocated

         //
         //  data field
         //

      SwathDataField * Field;   //  not allocated

      int NumberType;

      double DataScale;
      double DataOffset;

      double DataFillValue;

   public:

      ModisFile();
      virtual ~ModisFile();

      bool open(const char * filename);

      void close();

      void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

     void set_data_scale      (double);
     void set_data_offset     (double);
     void set_data_fill_value (double);

         //
         //  get stuff
         //

      int file_id() const;

      ConcatString filename() const;

      ConcatString short_name() const;

      CloudsatSwath * swath() const;

      unixtime scan_start_time() const;

      int dim0 () const;
      int dim1 () const;

      double lat(int, int) const;
      double lon(int, int) const;

      void latlon_range(double & lat_min, double & lat_max, double & lon_min, double & lon_max) const;

      bool data (int, int, double &) const;

         //
         //  do stuff
         //

      void select_data_field (const char * name);

};


////////////////////////////////////////////////////////////////////////


inline ConcatString ModisFile::filename() const { return ( Filename ); }

inline int ModisFile::file_id() const { return ( FileId ); }

inline int ModisFile::dim0() const { return ( Dim0 ); }
inline int ModisFile::dim1() const { return ( Dim1 ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MODIS_FILE_H__  */


////////////////////////////////////////////////////////////////////////


