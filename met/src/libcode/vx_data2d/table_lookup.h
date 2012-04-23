

////////////////////////////////////////////////////////////////////////


#ifndef  __VX_DATA2D_TABLE_LOOKUP_H__
#define  __VX_DATA2D_TABLE_LOOKUP_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


class Grib2TableEntry {

   private:

      init_from_scratch();

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

};


////////////////////////////////////////////////////////////////////////


class Grib2FlatFile {

   private:

      Grib2TableEntry * e;   //  elements ... allocated

      int Nelements;

   public:

      Grib2FlatFile();
     ~Grib2FlatFile();
      Grib2FlatFile(const Grib2FlatFile &);
      Grib2FlatFile & operator=(const Grib2FlatFile &);

      void clear();

      void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

         //
         //  get stuff
         //

      bool lookup(int a, int b, int c, Grib2TableEntry &);

      bool lookup(const char * part_name, Grib2TableEntry &, int & n_matches);

         //
         //  do stuff
         //


};


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_DATA2D_TABLE_LOOKUP_H__   */


////////////////////////////////////////////////////////////////////////


