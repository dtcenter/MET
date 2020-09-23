// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __ASCII_HEADER_H__
#define  __ASCII_HEADER_H__

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <vector>

#include "concat_string.h"
#include "string_array.h"

////////////////////////////////////////////////////////////////////////

class AsciiHeaderLine {

   protected:

      void init_from_scratch();

      void assign(const AsciiHeaderLine &);

      ConcatString Version;        // MET version number
      ConcatString DataType;       // Line date type (e.g. STAT, MODE, TCST)
      ConcatString LineType;       // Line type names

      int          NVarCols;       // Number of variable columns
      ConcatString VarIndexName;   // Name of the variable index column
      int          VarIndexOffset; // Offset to the variable index column
      int          VarBegOffset;   // Offset to first variable length column

      StringArray  ColNames;       // Names of the header columns

   public:

      AsciiHeaderLine();
      ~AsciiHeaderLine();
      AsciiHeaderLine(const AsciiHeaderLine &);
      AsciiHeaderLine & operator=(const AsciiHeaderLine &);

      void clear();

      //
      //  set stuff
      //

      void set_version  (const char *s);
      void set_data_type(const char *s);
      void set_line_type(const char *s);
      void set_col_names(const char *s);

      //
      //  get stuff
      //

      const char * version()               const;
      const char * data_type()             const;
      const char * line_type()             const;

      const char * var_index_name()        const;
      int          var_index_offset()      const;
      int          var_beg_offset()        const;

      bool         is_mctc()               const;
      bool         is_var_length()         const;
      int          n_var_cols(int dim = 0) const;
      int          length(int dim = 0)     const;

      int          col_offset(const char *name, const int dim = 0) const;
      ConcatString col_name  (const int offset, const int dim = 0) const;

};

////////////////////////////////////////////////////////////////////////

inline void AsciiHeaderLine::set_version  (const char *s) { Version  = s;  Version.ws_strip(); }
inline void AsciiHeaderLine::set_data_type(const char *s) { DataType = s; DataType.ws_strip(); }
inline void AsciiHeaderLine::set_line_type(const char *s) { LineType = s; LineType.ws_strip(); }

inline const char * AsciiHeaderLine::version()          const { return(Version.c_str());        }
inline const char * AsciiHeaderLine::data_type()        const { return(DataType.c_str());       }
inline const char * AsciiHeaderLine::line_type()        const { return(LineType.c_str());       }
inline const char * AsciiHeaderLine::var_index_name()   const { return(VarIndexName.c_str());   }
inline int          AsciiHeaderLine::var_index_offset() const { return(VarIndexOffset); }
inline int          AsciiHeaderLine::var_beg_offset()   const { return(VarBegOffset);   }

////////////////////////////////////////////////////////////////////////

class AsciiHeader {

   protected:

      void init_from_scratch();

      void assign(const AsciiHeader &);

      StringArray             Versions;
      vector<AsciiHeaderLine> Headers;

   public:

      AsciiHeader();
      ~AsciiHeader();
      AsciiHeader(const AsciiHeader &);
      AsciiHeader(const char *version);
      AsciiHeader & operator=(const AsciiHeader &);

      void clear();

      //
      //  set stuff
      //

      void read(const char *version);

      //
      //  get stuff
      //

      const AsciiHeaderLine * header(const char *version, const char *data_type,
                                     const char *line_type);

      int                 col_offset(const char *version,   const char *data_type,
                                     const char *line_type, const char *name,
                                     const int dim = 0);

      ConcatString          col_name(const char *version,   const char *data_type,
                                     const char *line_type, const int offset,
                                     const int dim = 0);
};

//////////////////////////////////////////////////////////////////

//
// global instantiation of the AsciiHeader class
//

extern AsciiHeader METHdrTable;

////////////////////////////////////////////////////////////////////////

#endif   /*  __ASCII_HEADER_H__  */

////////////////////////////////////////////////////////////////////////
