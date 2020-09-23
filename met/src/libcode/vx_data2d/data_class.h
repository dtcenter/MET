// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

      Grid * Raw_Grid;       //  Grid for raw data ... allocated
      Grid * Dest_Grid;      //  Grid for destination data ... allocated

      void copy_raw_grid_to_dest();

      ConcatString Filename;

      int ShiftRight;

   public:

      Met2dDataFile();
      virtual ~Met2dDataFile();

      void mtddf_clear();

      virtual void dump(ostream &, int depth = 0) const = 0;   //  dump grid and filename, etc., not data

         //
         //  set stuff
         //

      void set_shift_right(int);
      void set_grid(const Grid &);

         //
         //  get stuff
         //

      virtual const char * filename() const;

      virtual const Grid & grid     () const;
      virtual const Grid & raw_grid () const;

      virtual int nx() const;
      virtual int ny() const;

      virtual int raw_nx() const;
      virtual int raw_ny() const;

      virtual GrdFileType file_type() const = 0;

      int shift_right() const;

         //
         //  do stuff
         //

      virtual bool open(const char * filename) = 0;

      virtual void close() = 0;

         //  retrieve the first matching data plane

      virtual bool data_plane(VarInfo &, DataPlane &) = 0;

         //  retrieve all matching data planes

      virtual int data_plane_array(VarInfo &, DataPlaneArray &) = 0;

         //  retrieve the indexes of the first matching data plane

      virtual int index(VarInfo &) = 0;

         //  post-process data after reading it

      void process_data_plane(VarInfo *, DataPlane &);

};


////////////////////////////////////////////////////////////////////////


inline int Met2dDataFile::nx() const { return ( Dest_Grid ? (Dest_Grid->nx()) : 0 ); }
inline int Met2dDataFile::ny() const { return ( Dest_Grid ? (Dest_Grid->ny()) : 0 ); }

inline int Met2dDataFile::raw_nx() const { return ( Raw_Grid ? (Raw_Grid->nx()) : 0 ); }
inline int Met2dDataFile::raw_ny() const { return ( Raw_Grid ? (Raw_Grid->ny()) : 0 ); }

inline const char * Met2dDataFile::filename() const { return ( Filename.c_str() ); }

inline int Met2dDataFile::shift_right() const { return ( ShiftRight ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_DATA_FILE_CLASSES_H__  */


////////////////////////////////////////////////////////////////////////
