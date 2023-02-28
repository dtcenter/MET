// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __VX_GEN_SHAPE_INFO_H__
#define  __VX_GEN_SHAPE_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <vector>

#include "vx_cal.h"
#include "vx_math.h"
#include "vx_util.h"
#include "vx_gis.h"

////////////////////////////////////////////////////////////////////////
//
// GenShapeInfo class stores information about genesis shapefiles.
//
////////////////////////////////////////////////////////////////////////

class GenShapeInfo {

   private:

      void assign(const GenShapeInfo &);

      // Shapefile information
      ConcatString Basin;
      unixtime     FileTime;
      unixtime     IssueTime;

      ShpPolyRecord Poly;

      IntArray LeadSec;
      NumArray ProbVal;

   public:

      GenShapeInfo();
     ~GenShapeInfo();
      GenShapeInfo(const GenShapeInfo &);

      GenShapeInfo & operator=(const GenShapeInfo &);
      bool           operator==(const GenShapeInfo &) const;
      bool           is_duplicate(const GenShapeInfo &) const;

      void clear();
      ConcatString serialize() const;

         //
         //  set stuff
         //

      void set_basin(const char *);
      void set_time(unixtime);
      void set_poly(const ShpPolyRecord &);
      void add_prob(int, double);

         //
         //  get stuff
         //

      const ConcatString & basin() const;

      unixtime file_time()  const;
      unixtime issue_time() const;
      int      issue_hour() const;

      const ShpPolyRecord & poly() const;

      double center_lat() const;
      double center_lon() const;

      int    n_prob()      const;
      int    lead_sec(int) const;
      double prob_val(int) const;

         //
         //  do stuff
         //
};

////////////////////////////////////////////////////////////////////////

inline const ConcatString & GenShapeInfo::basin() const { return(Basin); }

inline unixtime GenShapeInfo::file_time()  const { return(FileTime);  }
inline unixtime GenShapeInfo::issue_time() const { return(IssueTime); }
inline int      GenShapeInfo::issue_hour() const { return(unix_to_sec_of_day(IssueTime)); }

////////////////////////////////////////////////////////////////////////
//
// GenShapeInfoArray class stores an array of GenShapeInfo objects.
//
////////////////////////////////////////////////////////////////////////

class GenShapeInfoArray {

   private:

      void init_from_scratch();
      void assign(const GenShapeInfoArray &);

      std::vector<GenShapeInfo> GenShape;

   public:

      GenShapeInfoArray();
     ~GenShapeInfoArray();
      GenShapeInfoArray(const GenShapeInfoArray &);
      GenShapeInfoArray & operator=(const GenShapeInfoArray &);

      void clear();

      ConcatString serialize() const;

         //
         //  set stuff
         //

      bool add(const GenShapeInfo &, bool check_dup = false);
      bool has(const GenShapeInfo &) const;

         //
         //  get stuff
         //

      const GenShapeInfo & operator[](int) const;
      int n() const;
};

////////////////////////////////////////////////////////////////////////

inline int GenShapeInfoArray::n() const { return(GenShape.size()); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_GEN_SHAPE_INFO_H__  */

////////////////////////////////////////////////////////////////////////
