// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef __GRIB_H__
#define __GRIB_H__


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>


#include "grib_strings.h"
#include "two_to_one.h"


////////////////////////////////////////////////////////////////////////


#ifdef IBM
   typedef int uint4;
#else
//    typedef unsigned long uint4; 
   typedef unsigned int uint4;
#endif


////////////////////////////////////////////////////////////////////////


static const int mem_alloc_error                =   1;
static const int range_chk_error                =   2;
static const int open_error                     =   3;

static const int missing_magic_cookie_error     =   4;
static const int record_size_error              =   5;
static const int missing_trail_7777_error       =   6;

static const int spher_harm_not_impl_error      =   7;
static const int second_ord_pkg_not_impl_error  =   8;
static const int word_size_error                =   9;

static const int file_read_error                =  10;
static const int buffer_size_error              =  11;
static const int read_overflow_error            =  12;

static const int bad_fcst_unit_val_error        =  13;
static const size_t grib_search_bytes           = 100;


////////////////////////////////////////////////////////////////////////


class GribError
{
   public:
      int error_type;
      int line_number;
      const char *filename;   //  not allocated
      char message[256];

      GribError(int ErrType, int LineNo, const char *FileName, const char *Message);
};


////////////////////////////////////////////////////////////////////////


inline GribError::GribError(int ErrType, int LineNo, const char *FileName, const char *Message) {

   error_type = ErrType;

   line_number = LineNo;

   filename = FileName;

   strcpy(message, Message);

}


////////////////////////////////////////////////////////////////////////


static const int default_gribfile_buf_size = (1 << 10);


////////////////////////////////////////////////////////////////////////


struct Section0_Header {                     //    IS

   char           grib_name[4];              //    1 - 4
                                             //
   unsigned char     length[3];              //    5 - 7
                                             //
   unsigned char        ed_num;              //    8

};


////////////////////////////////////////////////////////////////////////


struct Section1_Header {                     //    PDS

   unsigned char       length[3];            //    1 - 3
   unsigned char             ptv;            //    4
   unsigned char       center_id;            //    5

   unsigned char      process_id;            //    6
   unsigned char         grid_id;            //    7
   unsigned char            flag;            //    8
   unsigned char       grib_code;            //    9
   unsigned char            type;            //   10

   unsigned char   level_info[2];            //   11 - 12
   unsigned char            year;            //   13
   unsigned char           month;            //   14
   unsigned char             day;            //   15

   unsigned char            hour;            //   16
   unsigned char          minute;            //   17
   unsigned char       fcst_unit;            //   18
   unsigned char              p1;            //   19
   unsigned char              p2;            //   20

   unsigned char             tri;            //   21
   unsigned char          nia[2];            //   22 - 23
   unsigned char             nma;            //   24
   unsigned char         century;            //   25

   unsigned char      sub_center;            //   26
   unsigned char      d_value[2];            //   27 - 28

   unsigned char      a_value[12];           //   29 - 40

   unsigned char      ens_application;       //   41
   unsigned char      ens_type;              //   42
   unsigned char      ens_number;            //   43

};


////////////////////////////////////////////////////////////////////////


struct LatLon {                          //   Latitude/Longitude Grid

   unsigned char         lat1[3];            //   11 - 13
   unsigned char         lon1[3];            //   14 - 16

   unsigned char        res_flag;            //   17

   unsigned char         lat2[3];            //   18 - 20
   unsigned char         lon2[3];            //   21 - 23

   unsigned char           di[2];            //   24 - 25
   unsigned char           dj[2];            //   26 - 27

   unsigned char       scan_flag;            //   28

   unsigned char       unused[14];           //   29 - 42

};

struct Mercator {                            //   Mercator Grid

   unsigned char         lat1[3];            //   11 - 13
   unsigned char         lon1[3];            //   14 - 16

   unsigned char        res_flag;            //   17

   unsigned char         lat2[3];            //   18 - 20
   unsigned char         lon2[3];            //   21 - 23

   unsigned char        latin[3];            //   24 - 26

   unsigned char         unused1;            //   27

   unsigned char       scan_flag;            //   28

   unsigned char           di[3];            //   29 - 31
   unsigned char           dj[3];            //   32 - 34

   unsigned char       unused2[8];           //   35 - 42

};

struct LambertConf {                         //    Lambert Conformal Secant Grid

   unsigned char         lat1[3];            //   11 - 13
   unsigned char         lon1[3];            //   14 - 16

   unsigned char        res_flag;            //   17

   unsigned char          lov[3];            //   18 - 20

   unsigned char           dx[3];            //   21 - 23
   unsigned char           dy[3];            //   24 - 26

   unsigned char         pc_flag;            //   27

   unsigned char       scan_flag;            //   28

   unsigned char       latin1[3];            //   29 - 31
   unsigned char       latin2[3];            //   32 - 34

   unsigned char       lat_sp[3];            //   35 - 37
   unsigned char       lon_sp[3];            //   38 - 40

   unsigned char       unused[2];            //   41 - 42

};

struct Stereographic {

   unsigned char         lat1[3];            //   11 - 13
   unsigned char         lon1[3];            //   14 - 16

   unsigned char        res_flag;            //   17

   unsigned char          lov[3];            //   18 - 20

   unsigned char           dx[3];            //   21 - 23
   unsigned char           dy[3];            //   24 - 26

   unsigned char         pc_flag;            //   27

   unsigned char       scan_flag;            //   28

};


struct Gaussian {

   unsigned char         lat1[3];             //  11 - 13
   unsigned char         lon1[3];             //  14 - 16

   unsigned char        res_flag;             //  17

   unsigned char         lat2[3];             //  18 - 20
   unsigned char         lon2[3];             //  21 - 23

   unsigned char           di[2];             //  24 - 25
   unsigned char            n[2];             //  26 - 27

   unsigned char        scan_flag;            //  28

   unsigned char        unused[4];            //  29 - 32

   unsigned char        sp_lat[3];            //  33 - 35
   unsigned char        sp_lon[3];            //  36 - 38

   unsigned char      rotation[4];            //  39 - 42

   unsigned char   stretch_lat[3];            //  43 - 45
   unsigned char   stretch_lon[3];            //  46 - 48

   unsigned char   stretch_fac[4];            //  49 - 52

};


union GridType {

   struct LatLon        latlon_grid;         //   Latitude/Longitude Grid
   // struct RotLatLon     rot_latlon_grid;     //   Rotated Latitude/Longitude Grid
   struct Mercator      mercator;            //   Mercator Grid
   struct LambertConf   lambert_conf;        //   Lambert Conformal Secant Grid
   struct Stereographic stereographic;       //   Stereographic Grid
   struct Gaussian      gaussian;            //   Gaussian Grid

};

struct Section2_Header {                     //    GDS

   unsigned char       length[3];            //    1 -  3

   unsigned char              nv;            //    4
   unsigned char            pvpl;            //    5
   unsigned char            type;            //    6

   unsigned char           nx[2];            //    7 -  8
   unsigned char           ny[2];            //    9 - 10

   union GridType      grid_type;            //   11 - 42

};


////////////////////////////////////////////////////////////////////////


struct Section3_Header {                     //    BMS

   unsigned char       length[3];            //    1 - 3
   unsigned char             num;            //    4
   unsigned char         flag[2];            //    5 - 6

};


////////////////////////////////////////////////////////////////////////


struct Section4_Header {                     //    BDS

   unsigned char       length[3];            //    1 -  3
   unsigned char            flag;            //    4
   unsigned char      e_value[2];            //    5 -  6
   unsigned char      r_value[4];            //    7 - 10
   unsigned char            size;            //   11
   unsigned char      data_start;            //   12

};


////////////////////////////////////////////////////////////////////////


class GribRecord  {

   public:

      vector<unsigned char>        data;   //  allocated

      vector<unsigned char>      bitmap;   //  allocated

      Section0_Header *      is;   //  allocated
      unsigned char   *     pds;   //  allocated
      Section2_Header *     gds;   //  allocated
      Section3_Header *     bms;   //  allocated
      Section4_Header *     bds;   //  allocated

      off_t Sec0_offset_in_record;
      off_t Sec1_offset_in_record;
      off_t Sec2_offset_in_record;
      off_t Sec3_offset_in_record;
      off_t Sec4_offset_in_record;

      off_t Sec0_offset_in_file;
      off_t Sec1_offset_in_file;
      off_t Sec2_offset_in_file;
      off_t Sec3_offset_in_file;
      off_t Sec4_offset_in_file;

      int               rec_num;
      int               pds_len;
      int              gds_flag;
      int              bms_flag;

      int                    nx;
      int                    ny;

      double            m_value;
      double            b_value;

      double            r_value;

      int               d_value;
      int               e_value;

      int                 issue;     //   unix time
      int                  lead;     //   seconds

      int             word_size;     //   bits

      off_t record_lseek_offset;     //   bytes from beginning of file
      off_t   data_lseek_offset;     //   bytes from beginning of file

      //int             data_size;
      //int            data_alloc;

      //int           bitmap_size;
      //int          bitmap_alloc;

      uint4                mask;

      TwoOne                 TO;

         //
         //  standard members
         //

      GribRecord();
     ~GribRecord();
      GribRecord(const GribRecord &);
      GribRecord &operator=(const GribRecord &);

         //
         //  utility functions
         //

      uint4  long_data_value (int n) const;
      double data_value      (int n) const;

      uint4  long_data_value (int x, int y) const;
      double data_value      (int x, int y) const;

      int bms_bit(int) const;

      int gribcode() const;

      void extend_data(int);
      void extend_bitmap(int);

      void set_TO();

      void reset();

};


////////////////////////////////////////////////////////////////////////


class GribFile;   //  forward reference


////////////////////////////////////////////////////////////////////////


struct RecordInfo {

   off_t lseek_offset;

   int   gribcode;

};


////////////////////////////////////////////////////////////////////////


class GribFileRep {

      friend class GribFile;

   protected:

      int referenceCount;

      int fd;
      long file_start;

      char * name;

      int issue;
      int lead;

      unsigned char * buf;
      size_t buf_size;

      unsigned int n_records;
      int n_alloc;

      RecordInfo * record_info;

      GribFileRep();
     ~GribFileRep();
      GribFileRep(const GribFileRep &);
      GribFileRep &operator=(const GribFileRep &);

      void record_extend(int);

      void realloc_buf(size_t size);

};


////////////////////////////////////////////////////////////////////////


class GribFile {

   protected:

      GribFileRep * rep;

   public:

         //
         //  standard members
         //

      GribFile();
      virtual ~GribFile();
      GribFile(const GribFile &);
      GribFile &operator=(const GribFile &);

      GribFile(const char *);

      virtual bool open(const char *);

      virtual void close();

      virtual int read(void *, int);

      virtual int operator>>(GribRecord &);

   protected:

      int skip_header();

      void index_records();

      int read_record(GribRecord &);

   protected:

      size_t read();
      size_t read(size_t);
      size_t read(off_t, size_t);

   public:

      virtual void seek_record(int);

      virtual unsigned int   n_records();
      virtual off_t record_offset(int);
      virtual int   gribcode(int);

      virtual int issue();
      virtual int lead();

      const char *name();

};


////////////////////////////////////////////////////////////////////////


extern double          char4_to_dbl(const unsigned char *);
extern unsigned int    char3_to_int(const unsigned char *);
extern unsigned int    char2_to_int(const unsigned char *);

extern ostream & operator<<(ostream &, const GribRecord &);

extern ostream & operator<<(ostream &, const Section0_Header &);
extern ostream & operator<<(ostream &, const Section1_Header &);
extern ostream & operator<<(ostream &, const Section2_Header &);
extern ostream & operator<<(ostream &, const Section3_Header &);
extern ostream & operator<<(ostream &, const Section4_Header &);


////////////////////////////////////////////////////////////////////////


#endif   //  __GRIB_H__


////////////////////////////////////////////////////////////////////////


