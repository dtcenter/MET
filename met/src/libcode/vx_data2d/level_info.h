// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////////////

#ifndef __LEVEL_INFO_H__
#define __LEVEL_INFO_H__

///////////////////////////////////////////////////////////////////////////////

#include "concat_string.h"

///////////////////////////////////////////////////////////////////////////////

   //
   // Enumeration for level types
   //

enum LevelType
{
   LevelType_None,      // Default
   LevelType_Accum,     // Accumulation Interval
   LevelType_Vert,      // Vertical Level
   LevelType_Pres,      // Pressure Level
   LevelType_RecNumber, // GRIB version 1 Record Number
   LevelType_SFC,       // Surface
   LevelType_Time       // Time
};

///////////////////////////////////////////////////////////////////////////////

class LevelInfo
{
   protected:

      LevelType    Type;    // Type of level
      int          TypeNum; // Level type number
      ConcatString ReqName; // Requested level name
      ConcatString Name;    // Level name
      ConcatString Units;   // Units for the level

      double Upper;         // Upper level limit
      double Lower;         // Lower level limit
      double Increment;     // Increment (time: seconds, 0 for no increment)
      bool   time_as_offset;// default: true, false: the (time) value instead
                            // of the offset at Lower and Upper

      void init_from_scratch();
      void assign(const LevelInfo &);

   public:

      LevelInfo();
      ~LevelInfo();
      LevelInfo(const LevelInfo &);
      LevelInfo & operator=(const LevelInfo &);

      void clear();

      void dump(ostream &) const;

         //
         // get stuff
         //

      LevelType    type()     const;
      int          type_num() const;
      ConcatString req_name() const;
      ConcatString name()     const;
      ConcatString units()    const;
      double       upper()       const;
      double       lower()       const;
      double       increment()   const;
      bool         is_time_as_offset()const;

      //
      // set stuff
      //

      void set_type(LevelType);
      void set_type_num(int);
      void set_req_name(const char *);
      void set_name(const char *);
      void set_units(const char *);
      void set_upper(double u);
      void set_lower(double l);
      void set_range(double l, double u);
      void set_increment(double i);
      void set_time_as_offset(bool b);

};

///////////////////////////////////////////////////////////////////////////////

inline LevelType    LevelInfo::type()     const { return(Type);    }
inline int          LevelInfo::type_num() const { return(TypeNum); }
inline ConcatString LevelInfo::req_name() const { return(ReqName); }
inline ConcatString LevelInfo::name()     const { return(Name);    }
inline ConcatString LevelInfo::units()    const { return(Units);   }
inline double       LevelInfo::upper()    const { return(Upper);   }
inline double       LevelInfo::lower()    const { return(Lower);   }
inline double       LevelInfo::increment()const { return(Increment);}
inline bool         LevelInfo::is_time_as_offset()const { return(time_as_offset);}

///////////////////////////////////////////////////////////////////////////////

#endif  // __LEVEL_INFO_H__

///////////////////////////////////////////////////////////////////////////////
