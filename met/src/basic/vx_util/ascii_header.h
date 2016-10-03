// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __ASCII_HEADER_H__
#define  __ASCII_HEADER_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "concat_string.h"
#include "string_array.h"

////////////////////////////////////////////////////////////////////////

class AsciiHeader {

   protected:

      void init_from_scratch();

      void assign(const AsciiHeader &);

      int NHdrCols;
      int NDataCols;

      ConcatString Version;
      StringArray  HdrCols;
      StringArray  DataCols;

   public:

      AsciiHeader();
      ~AsciiHeader();
      AsciiHeader(const AsciiHeader &);
      AsciiHeader(const char *version, const char *hdr, const char *data);
      AsciiHeader & operator=(const AsciiHeader &);

      void clear();

      //
      //  set stuff
      //

      void set        (const char *version, const char *hdr, const char *data);

      void set_version(const char *);
      void set_hdr    (const char *);
      void set_data   (const char *);

      //
      //  get stuff
      //

      int         n()                     const;
      int         nhdr()                  const;
      int         ndata()                 const;

      const char *version()               const;

      const char *col_name(int i)         const;
      int         col_index(const char *) const;
};


////////////////////////////////////////////////////////////////////////

inline int         AsciiHeader::n()       const { return(NHdrCols + NDataCols);  }
inline int         AsciiHeader::nhdr()    const { return(NHdrCols);              }
inline int         AsciiHeader::ndata()   const { return(NDataCols);             }
inline const char *AsciiHeader::version() const { return(Version);        }

////////////////////////////////////////////////////////////////////////

#endif   /*  __ASCII_HEADER_H__  */

////////////////////////////////////////////////////////////////////////
