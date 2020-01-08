

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __MET_FILE_H__
#define  __MET_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <ostream>

#include "vx_grid.h"
#include "data_plane.h"
#include "long_array.h"
#include "nc_var_info.h"


////////////////////////////////////////////////////////////////////////


static const char nc_met_lat_var_name [] = "lat";
static const char nc_met_lon_var_name [] = "lon";


////////////////////////////////////////////////////////////////////////


class MetNcFile {

   private:

      void init_from_scratch();

      MetNcFile(const MetNcFile &);
      MetNcFile & operator=(const MetNcFile &);

   public:

      MetNcFile();
     ~MetNcFile();

      bool open(const char * filename);

      void close();

      void dump(ostream &, int = 0) const;


      NcFile * Nc;      //  allocated

         //
         //  time
         //

      unixtime ValidTime;

      unixtime InitTime;

      int      lead_time () const;   //  seconds


         //
         //  dimensions
         //

      int Ndims;

      NcDim ** Dim;   //  allocated

      StringArray DimNames;

      NcDim * Xdim;   //  not allocated
      NcDim * Ydim;   //  not allocated

         //
         //  variables
         //

      int Nvars;

      NcVarInfo * Var;    //  allocated

         //
         //  Grid
         //

      Grid grid;

         //
         //  data
         //

      double data(NcVar *, const LongArray &) const;

      bool data(NcVar *, const LongArray &, DataPlane &) const;

      bool data(const char *, const LongArray &, DataPlane &, NcVarInfo *&) const;

      NcVarInfo* find_var_name(const char * var_name) const;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_FILE_H__  */


////////////////////////////////////////////////////////////////////////


