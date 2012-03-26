

   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
   // ** Copyright UCAR (c) 1992 - 2012
   // ** University Corporation for Atmospheric Research (UCAR)
   // ** National Center for Atmospheric Research (NCAR)
   // ** Research Applications Lab (RAL)
   // ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __MET_DATA_FILE_CLASSES_H__
#define  __MET_DATA_FILE_CLASSES_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_cal.h"
#include "vx_grid.h"
#include "var_info.h"


////////////////////////////////////////////////////////////////////////


   //
   //  abstract base class for 2D data
   //


class Met2dData {

   private:

      Met2dData(const Met2dData &);
      Met2dData & operator=(const Met2dData &);

      void mtdd_init_from_scratch();

      // void assign(const Met2dData &);   //  don't allow this

   public:

      Met2dData();
      virtual ~Met2dData();

      void mtdd_clear();

      virtual void dump(ostream &, int depth = 0) const = 0;   //  dump grid and filename, etc., not data

         //
         //  set stuff
         //

         //
         //  get stuff
         //

      virtual const Grid & grid() const = 0;

      virtual int nx() const = 0;
      virtual int ny() const = 0;


         //
         //  do stuff
         //

};


////////////////////////////////////////////////////////////////////////


class Met2dDataFile : public Met2dData {

   private:

      Met2dDataFile(const Met2dDataFile &);
      Met2dDataFile & operator=(const Met2dDataFile &);

      void mtddf_init_from_scratch();

      // void assign(const Met2dDataFile &);   //  don't allow this

   protected:

      Grid * _Grid;        //  allocated

      ConcatString Filename;

   public:

      Met2dDataFile();
      virtual ~Met2dDataFile();

      void mtddf_clear();

      virtual void dump(ostream &, int depth = 0) const = 0;   //  dump grid and filename, etc., not data

         //
         //  set stuff
         //


         //
         //  get stuff
         //

      virtual const char * filename() const;

      virtual const Grid & grid() const;

      virtual int nx() const;

      virtual int ny() const;

      virtual GrdFileType file_type() const = 0;

         //
         //  do stuff
         //

      virtual bool open(const char * filename) = 0;

      virtual void close() = 0;

         //  retrieve the first matching data plane

      virtual bool data_plane(VarInfo &, DataPlane &) = 0;

         //  retrieve all matching data planes
      
      virtual int data_plane_array(VarInfo &, DataPlaneArray &) = 0;

};


////////////////////////////////////////////////////////////////////////


inline int Met2dDataFile::nx() const { return ( _Grid ? (_Grid->nx()) : 0 ); }
inline int Met2dDataFile::ny() const { return ( _Grid ? (_Grid->ny()) : 0 ); }

inline const char * Met2dDataFile::filename() const { return ( Filename ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_DATA_FILE_CLASSES_H__  */


////////////////////////////////////////////////////////////////////////
