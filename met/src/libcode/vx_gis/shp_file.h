

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __VX_SHAPEFILES_SHP_FILE_H__
#define  __VX_SHAPEFILES_SHP_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <sys/types.h>
#include <unistd.h>

#include "smart_buffer.h"
#include "check_endian.h"

#include "shp_types.h"


////////////////////////////////////////////////////////////////////////


static const int shp_file_header_bytes   = 100;

static const int shp_record_header_bytes = 8;


////////////////////////////////////////////////////////////////////////


struct ShpFileHeader {

   int file_code;

   int file_length_16;      //  length in 16-bit words

   int file_length_bytes;   //  length in bytes

   int version;

   int shape_type;          //  one of the above enum ShapeType


   double x_min, x_max; 
   double y_min, y_max;
   double z_min, z_max;
   double m_min, m_max;

      //
      //
      //

   void set(unsigned char * buf);

   void dump(ostream &, int depth = 0) const;

};


////////////////////////////////////////////////////////////////////////


struct ShpRecordHeader {


   int record_number_1;   //  NOTE:  1-based
   int record_number_0;   //  NOTE:  0-based


   int content_length_16;

   int content_length_bytes;

      //

   void set(unsigned char * buf);

   void dump(ostream &, int depth = 0) const;

};


////////////////////////////////////////////////////////////////////////


class ShpFile {

   private:

      ShpFile(const ShpFile &);
      ShpFile & operator=(const ShpFile &);

   protected:

      void init_from_scratch();


      int fd;

      bool At_Eof;

      ShpFileHeader Header;

      ConcatString Filename;


   public:

      ShpFile();
     ~ShpFile();


      bool open(const char * path);

      void close();

         //
         //  set stuff
         //

         //
         //  get stuff
         //

      const ShpFileHeader * header() const;

      const char * filename() const;

      int shape_type() const;

      bool at_eof() const;

      int position() const;   //  offset in bytes from beginning of file

      bool is_open() const;

         //
         //  do stuff
         //

      void lseek(int offset, int whence = SEEK_SET);   //  just like lseek(2)

      bool read(unsigned char * buf, int nbytes);

      bool read_sb(SmartBuffer &, int nbytes);

};


////////////////////////////////////////////////////////////////////////


inline const ShpFileHeader * ShpFile::header() const { return ( &Header ); }

inline int   ShpFile::shape_type() const { return ( Header.shape_type ); }

inline bool  ShpFile::at_eof() const { return ( At_Eof ); }

inline bool  ShpFile::is_open() const { return ( fd >= 0 ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_SHAPEFILES_SHP_FILE_H__  */


////////////////////////////////////////////////////////////////////////


