

////////////////////////////////////////////////////////////////////////


#ifndef  __MODE_2D_INPUT_FILE_H__
#define  __MODE_2D_INPUT_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "netcdf.hh"

#include "vx_util.h"
#include "vx_cal.h"
#include "grid.h"


////////////////////////////////////////////////////////////////////////


class Mode2DInputFile {

   private:

      void init_from_scratch();

      double data_value(int x, int y) const;

      void find_data_range();

      Mode2DInputFile(const Mode2DInputFile &);
      Mode2DInputFile & operator=(const Mode2DInputFile &);

      ConcatString FileName;

      ConcatString FieldName;

      Unixtime ValidTime;   //  UTC

      int LeadTime;   //  units: seconds

      double DataMin;
      double DataMax;

      Grid * G;   //  allocated

      NcFile * Nc;   //  allocated

      NcVar * Var;   //  not allocated


   public:

      Mode2DInputFile();
     ~Mode2DInputFile();

      void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

         //
         //  get stuff
         //

      Unixtime valid_time () const;
      Unixtime issue_time () const;

      int      lead_time  () const;   //  units: seconds

      int nx () const;
      int ny () const;

      double operator ()(int x, int y) const;

      ConcatString filename() const;

      ConcatString field_name() const;

      double data_min() const;
      double data_max() const;

      Grid grid() const;

      bool same_grid(const Grid &) const;

         //
         //  do stuff
         //

      bool open(const char * filename, const char * field_name);

      void close();


      void latlon_to_xy (double lat, double lon, double & x, double & y) const;

      void xy_to_latlon (double x, double y, double & lat, double & lon) const;

};


////////////////////////////////////////////////////////////////////////


inline Unixtime Mode2DInputFile::valid_time() const { return ( ValidTime ); }

inline Unixtime Mode2DInputFile::issue_time() const { return ( ValidTime - LeadTime ); }

inline int Mode2DInputFile::lead_time() const { return ( LeadTime ); }

inline ConcatString Mode2DInputFile::filename() const { return ( FileName ); }

inline ConcatString Mode2DInputFile::field_name() const { return ( FieldName ); }

inline double Mode2DInputFile::operator()(int x, int y) const { return ( data_value(x, y) ); }

inline double Mode2DInputFile::data_min() const { return ( DataMin ); }
inline double Mode2DInputFile::data_max() const { return ( DataMax ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MODE_2D_INPUT_FILE_H__  */


////////////////////////////////////////////////////////////////////////


