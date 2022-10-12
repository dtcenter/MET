// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __VX_SHAPEFILES_DBF_FILE_H__
#define  __VX_SHAPEFILES_DBF_FILE_H__

////////////////////////////////////////////////////////////////////////

   //
   //  Got info on the dbf file format at
   //
   //        http://www.dbf2002.com/dbf-file-format.html
   //
   //        https://en.wikipedia.org/wiki/.dbf
   //
   //        http://web.archive.org/web/20150323061445/http://ulisse.elettra.trieste.it/services/doc/dbase/DBFstruct.htm
   //

////////////////////////////////////////////////////////////////////////

struct DbfSubRecord;   //  forward reference

////////////////////////////////////////////////////////////////////////

class DbfHeader {

   private:

      void init_from_scratch();

      void assign(const DbfHeader &);

   public:

      DbfHeader();
     ~DbfHeader();
      DbfHeader(const DbfHeader &);
      DbfHeader & operator=(const DbfHeader &);


      int type;

      int last_update_mjd;

      int n_records;

      int pos_first_record;   //  0-based

      int record_length;

      int table_flag;

      int code_page_mark;

      DbfSubRecord * subrec;   //  allocated

      int n_subrecs;   //  inferred

         //

      void clear();

      void set_header(unsigned char * buf);

      void set_subrecords(int fd);

      void dump(std::ostream &, int depth = 0) const;

      DbfSubRecord * lookup_subrec(const char * text) const;   //  matches field_name

};

////////////////////////////////////////////////////////////////////////

class DbfSubRecord {

   private:

      void init_from_scratch();

      void assign(const DbfSubRecord &);

   public:

      DbfSubRecord();
     ~DbfSubRecord();
      DbfSubRecord(const DbfSubRecord &);
      DbfSubRecord & operator=(const DbfSubRecord &);

      void clear();


      std::string field_name;

      char field_type;

      int displacement;

      int field_length;   //  bytes

      int dp;             //  decimal places

      unsigned char field_flags;

      int autoinc_next_value;

      int autoinc_step_value;

      int start_pos;   //  0-based ... start position in record

      //

      void set(unsigned char * buf);

      void dump(std::ostream &, int depth = 0) const;

};

////////////////////////////////////////////////////////////////////////

void dump_record(std::ostream &, const int depth, const unsigned char * buf, const DbfHeader &);

////////////////////////////////////////////////////////////////////////

class DbfFile {

   private:

      DbfFile(const DbfFile &);
      DbfFile & operator=(const DbfFile &);

   protected:

      void init_from_scratch();

      int fd;

      bool At_Eof;

      DbfHeader Header;

      ConcatString Filename;

   public:

      DbfFile();
     ~DbfFile();

      bool open(const char * path);

      void close();

         //
         //  set stuff
         //

         //
         //  get stuff
         //

      const DbfHeader * header() const;

      const char * filename() const;

      bool at_eof() const;

      int position() const;   //  offset in bytes from beginning of file

      bool is_open() const;

         //
         //  do stuff
         //

      void lseek(int offset, int whence = SEEK_SET);   //  just like lseek(2)

      bool read(unsigned char * buf, int nbytes);

      StringArray subrecord_names();

      StringArray subrecord_values(int i_rec);

};


////////////////////////////////////////////////////////////////////////

inline const DbfHeader * DbfFile::header() const { return ( &Header ); }

inline bool  DbfFile::at_eof() const { return ( At_Eof ); }

inline bool  DbfFile::is_open() const { return ( fd >= 0 ); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_SHAPEFILES_DBF_FILE_H__  */

////////////////////////////////////////////////////////////////////////
