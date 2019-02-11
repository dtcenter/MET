// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __CLOUDSAT_SWATH_FILE_H__
#define  __CLOUDSAT_SWATH_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_log.h"

#include "hdf.h"
#include "mfhdf.h"

#include "HdfEosDef.h"


////////////////////////////////////////////////////////////////////////


class SatDimension {

   private:

      void init_from_scratch();

      void assign(const SatDimension &);


      ConcatString Name;

      int Size;

   public:

      SatDimension();
      virtual ~SatDimension();
      SatDimension(const SatDimension &);
      SatDimension & operator=(const SatDimension &);

      void clear();

      void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

      void set_name(const char *);

      void set_size(int);

         //
         //  get stuff
         //

      ConcatString name() const;

      int size() const;

         //
         //  do stuff
         //


};


////////////////////////////////////////////////////////////////////////


inline ConcatString SatDimension::name() const { return ( Name ); }

inline int SatDimension::size() const { return ( Size ); }


////////////////////////////////////////////////////////////////////////


class SatAttribute {

   private:

      void init_from_scratch();

      void assign(const SatAttribute &);

      ConcatString Name;

      int Numbertype;

      int Bytes;

      int Nvalues;

         //
         //  values
         //

      ConcatString Sval;

      int * Ival;      //  allocated

      double * Dval;   //  allocated

   public:

      SatAttribute();
     ~SatAttribute();
      SatAttribute(const SatAttribute &);
      SatAttribute & operator=(const SatAttribute &);


      void clear();

      void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

      void set_name(const char *);

      void set_number_type(int);

      void set_bytes(int);

      void set_value(int nt, unsigned char *, int n);

         //
         //  get stuff
         //

      ConcatString name() const;

      int number_type() const;

      int bytes() const;

      int n_values() const;

      int          ival (int) const;
      double       dval (int) const;
      ConcatString sval ()    const;

      double rank_2_data(int, int) const;

         //
         //  do stuff
         //

};


////////////////////////////////////////////////////////////////////////


inline ConcatString SatAttribute::name() const { return ( Name ); }

inline int SatAttribute::number_type() const { return ( Numbertype ); }

inline int SatAttribute::bytes() const { return ( Bytes ); }

inline int SatAttribute::n_values() const { return ( Nvalues ); }

inline int    SatAttribute::ival(int n) const { return ( Ival[n] ); }
inline double SatAttribute::dval(int n) const { return ( Dval[n] ); }

inline ConcatString SatAttribute::sval() const { return ( Sval ); }


////////////////////////////////////////////////////////////////////////


class SwathDataField {

   private:

      void init_from_scratch();

      void assign(const SwathDataField &);


      ConcatString Name;

      int Rank;

      int Numbertype;

      int Ndimensions;

      SatDimension ** Dimensions;  //  array is allocated, but not elements

   public:

      SwathDataField();
      virtual ~SwathDataField();
      SwathDataField(const SwathDataField &);
      SwathDataField & operator=(const SwathDataField &);


      void clear();

      void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

      void set_name(const char *);

      void set_rank       (int);
      void set_numbertype (int);

      void set_n_dimensions (int);

      void set_dimension(int, SatDimension *);

         //
         //  get stuff
         //

      ConcatString name() const;

      int get_rank() const;
      int numbertype() const;
      int n_dimensions() const;

      SatDimension * dimension(int) const;

      int dimension_size(int) const;

         //
         //  do stuff
         //


};


////////////////////////////////////////////////////////////////////////


inline ConcatString SwathDataField::name() const { return ( Name ); }

inline int SwathDataField::get_rank     () const { return ( Rank       ); }
inline int SwathDataField::numbertype   () const { return ( Numbertype ); }

inline int SwathDataField::n_dimensions () const { return ( Ndimensions ); }


////////////////////////////////////////////////////////////////////////


class CloudsatSwath {

   private:

      void init_from_scratch();

      void assign(const CloudsatSwath &);

      ConcatString Name;

      int SwathId;

      int Ndatafields;

      SwathDataField * DataField;   //  allocated

      int Nattributes;

      SatAttribute * Attribute;     //  allocated

      int Ngeofields;

      SwathDataField * GeoField;    //  allocated

      int Ndimensions;

      SatDimension * Dimension;     //  allocated


      SwathDataField * Latitude;     //  not allocated
      SwathDataField * Longitude;    //  not allocated
      SwathDataField * Height;       //  not allocated
      SwathDataField * Reflectivity; //  not allocated

   public:

      CloudsatSwath();
      virtual~CloudsatSwath();
      CloudsatSwath(const CloudsatSwath &);
      CloudsatSwath & operator=(const CloudsatSwath &);

      void clear();

      void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

      void set_name(const char *);

      void set_swath_id(int);


         //
         //  get stuff
         //

      ConcatString name() const;

      int swath_id     () const;

      int n_data_fields() const;

      int n_attributes () const;

      int n_geo_fields () const;

      int n_dimensions () const;

         //
         //  do stuff
         //

      void get_data_fields();

      void get_attributes();

      void get_geo_fields();

      void get_dimensions();

      SwathDataField * get_field(int)          const;
      SwathDataField * get_field(const char *) const;

      SwathDataField * get_geo_field(int)          const;
      SwathDataField * get_geo_field(const char *) const;

      SwathDataField * get_data_field(int)          const;
      SwathDataField * get_data_field(const char *) const;


      SatDimension * dimension(int)          const;
      SatDimension * dimension(const char *) const;

      void setup_geo_pointers();

      double lat(int);
      double lon(int);

      double height_m(int nray, int nbin);

      double reflectivity(int nray, int nbin);

};


////////////////////////////////////////////////////////////////////////


inline ConcatString CloudsatSwath::name() const { return ( Name ); }

inline int CloudsatSwath::swath_id() const { return ( SwathId ); }

inline int CloudsatSwath::n_data_fields() const { return ( Ndatafields ); }

inline int CloudsatSwath::n_attributes () const { return ( Nattributes ); }

inline int CloudsatSwath::n_geo_fields () const { return ( Ngeofields ); }


////////////////////////////////////////////////////////////////////////


class CloudsatSwathFile {

   private:

      CloudsatSwathFile(const CloudsatSwathFile &);
      CloudsatSwathFile & operator=(const CloudsatSwathFile &);

   protected:

      void init_from_scratch();


      ConcatString Filename;

      int FileId;

      int Nswaths;

      CloudsatSwath * Swath;   //  allocated

   public:

      CloudsatSwathFile();
      virtual ~CloudsatSwathFile();

      bool open(const char * filename);

      void close();

      void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

         //
         //  get stuff
         //

      int file_id() const;

      int n_swaths() const;

      ConcatString filename() const;

      ConcatString short_name() const;

      CloudsatSwath * swath(int) const;

         //
         //  do stuff
         //


};


////////////////////////////////////////////////////////////////////////


inline ConcatString CloudsatSwathFile::filename() const { return ( Filename ); }

inline int CloudsatSwathFile::file_id() const { return ( FileId ); }

inline int CloudsatSwathFile::n_swaths() const { return ( Nswaths ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __CLOUDSAT_SWATH_FILE_H__  */


////////////////////////////////////////////////////////////////////////


