// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __DATA_LINE_H__
#define  __DATA_LINE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <vector>

#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


static const char dataline_default_delim[] = " \t";


////////////////////////////////////////////////////////////////////////


class LineDataFile;  //  forward reference


#ifdef WITH_PYTHON
class PyLineDataFile;   //  forward reference
#endif


////////////////////////////////////////////////////////////////////////


class DataLine {

      friend class LineDataFile;
      friend ostream & operator<<(ostream &, const DataLine &);
      friend Logger & operator<<(Logger &, const DataLine &);

   protected:

      std::string Line;
      std::vector<std::string> Items;
      
      int N_chars;

      std::vector<int> Offset;

      int N_ints;

      int LineNumber;

      std::string Delimiter;

      LineDataFile * File;  //  not allocated

      bool IsHeader;

   
   protected:
   
      void init_from_scratch();
      void assign(const DataLine &);
      int N_items;

   public:

      bool read_single_text_line(LineDataFile *);   //  reads a line of text into Line

#ifdef WITH_PYTHON
      bool read_py_single_text_line(PyLineDataFile *);
#endif


   public:

      DataLine();
      virtual ~DataLine();
      DataLine(const DataLine &);
      DataLine & operator=(const DataLine &);


      void clear();

      void dump(ostream &, int depth = 0) const;

         //
         //  retrieve stuff
         //

      const char * get_line() const;

      const char * get_item(int) const;

      const char * get_delimiter() const;

      const LineDataFile * get_file() const;

      int n_items() const;

      const char * operator [] (int) const;

      int line_number() const;

      int max_item_width() const;

         //
         //  read line from file
         //

      virtual int read_line(LineDataFile *);

      virtual int read_fwf_line(LineDataFile *, const int *wdth, int n_wdth);

      virtual bool is_ok() const;

      virtual bool is_header() const;

      virtual void set_is_header(bool = true);

      virtual void set_delimiter(const char *delimiter);

};


////////////////////////////////////////////////////////////////////////


inline  int           DataLine::n_items      () const { return ( N_items ); }

inline  int           DataLine::line_number  () const { return ( LineNumber ); }

inline  const char *  DataLine::get_line     () const { return ( Line.c_str() ); }

inline  const char *  DataLine::get_delimiter() const { return ( Delimiter.c_str() ); }

inline  void          DataLine::set_is_header(bool __tf__) { IsHeader = __tf__;  return; }


////////////////////////////////////////////////////////////////////////


class LineDataFile {

   protected:

      void init_from_scratch();

      LineDataFile(const LineDataFile &);
      LineDataFile & operator=(const LineDataFile &);

      std::string Filename;

      std::string ShortFilename;

      int Last_Line_Number;

      StringArray Header;

      void set_header(DataLine &);

   public:

      LineDataFile();
      virtual ~LineDataFile();

      ifstream * in;

      int open(const char *);

      void close();

      void rewind();

      int ok() const;

      virtual int operator>>(DataLine &);

      int read_fwf_line(DataLine &, const int *wdth, int n_wdth);

      const char * filename() const;
      const char * short_filename() const;

      int last_line_number() const;

      const StringArray & header() const;

};


////////////////////////////////////////////////////////////////////////


inline const char * LineDataFile::filename() const { return ( Filename.c_str() ); }

inline const char * LineDataFile::short_filename() const { return ( ShortFilename.c_str() ); }

inline int LineDataFile::last_line_number() const { return ( Last_Line_Number ); }

inline const StringArray & LineDataFile::header() const { return ( Header ); }


////////////////////////////////////////////////////////////////////////


extern ostream & operator<<(ostream &, const DataLine &);


////////////////////////////////////////////////////////////////////////


extern Logger & operator<<(Logger &, const DataLine &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __DATA_LINE_H__  */


////////////////////////////////////////////////////////////////////////


