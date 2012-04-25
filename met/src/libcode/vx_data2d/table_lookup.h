

////////////////////////////////////////////////////////////////////////


#ifndef  __VX_DATA2D_TABLE_LOOKUP_H__
#define  __VX_DATA2D_TABLE_LOOKUP_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "concat_string.h"


////////////////////////////////////////////////////////////////////////


class Grib2TableEntry {

   private:

      void init_from_scratch();

      void assign(const Grib2TableEntry &);

   public:

      Grib2TableEntry();
     ~Grib2TableEntry();
      Grib2TableEntry(const Grib2TableEntry &);
      Grib2TableEntry & operator=(const Grib2TableEntry &);

      void clear();

      void dump(ostream &, int = 0) const;

      int index_a;
      int index_b;
      int index_c;

      ConcatString parm_name;

      ConcatString full_name;

      ConcatString units;

         //
         //  set stuff
         //

         //
         //  get stuff
         //

         //
         //  do stuff
         //

      bool parse_line(const char *);

};


////////////////////////////////////////////////////////////////////////


class TableFlatFile {

   private:

      void init_from_scratch();

      void assign(const TableFlatFile &); 

      bool read_grib2(istream &);

      // Grib1TableEntry * g1e;   //  elements ... allocated
      Grib2TableEntry * g2e;   //  elements ... allocated

      int Nelements;

      ConcatString Filename;

   public:

      TableFlatFile();
      TableFlatFile(int);
     ~TableFlatFile();
      TableFlatFile(const TableFlatFile &);
      TableFlatFile & operator=(const TableFlatFile &);

      void clear();

      void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

         //
         //  get stuff
         //

      ConcatString filename() const;

      int n_elements() const;

         //
         //  do stuff
         //

      bool read(const char * filename);

      bool lookup_grib2(int a, int b, int c, Grib2TableEntry &);
      bool lookup_grib2(const char * parm_name, Grib2TableEntry &, int & n_matches);

};


////////////////////////////////////////////////////////////////////////


inline int TableFlatFile::n_elements() const { return ( Nelements ); }

inline ConcatString TableFlatFile::filename() const { return ( Filename ); }


////////////////////////////////////////////////////////////////////////


extern TableFlatFile GribTable;


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_DATA2D_TABLE_LOOKUP_H__   */


////////////////////////////////////////////////////////////////////////


