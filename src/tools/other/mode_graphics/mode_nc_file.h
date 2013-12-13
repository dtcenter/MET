

////////////////////////////////////////////////////////////////////////


#ifndef  __MODE_NC_FILE_H__
#define  __MODE_NC_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <netcdf.hh>

#include "data_plane.h"
#include "vx_grid.h"
#include "get_met_grid.h"


////////////////////////////////////////////////////////////////////////


enum ModeObjectField {

   mof_fcst_obj, 
   mof_fcst_comp, 

   mof_obs_obj, 
   mof_obs_comp, 

};


////////////////////////////////////////////////////////////////////////


class ModeNcFile {

   private:

      void init_from_scratch();

      int    get_int   (NcVar *, int x, int y) const;
      double get_float (NcVar *, int x, int y) const;

      DataPlane select_obj(ModeObjectField, int) const;

      NcFile * f;   //  allocated

      Grid * _Grid;   //  allocated

      NcVar * FcstObjId;   //  NOT allocated
      NcVar * FcstCompId;  //  NOT allocated

      NcVar * ObsObjId;    //  NOT allocated
      NcVar * ObsCompId;   //  NOT allocated

      NcVar * FcstRaw;     //  NOT allocated
      NcVar * ObsRaw;      //  NOT allocated

      ModeNcFile(const ModeNcFile &);
      ModeNcFile & operator=(const ModeNcFile &);

      int Nx;
      int Ny;

      // int NFcstObjs;
      // int NFcstComps;

      // int NObsObjs;
      // int NObsComps;

   public:

      ModeNcFile();
     ~ModeNcFile();

      bool open(const char * path);

      void close();

      void dump(ostream &) const;

      int nx() const;
      int ny() const;

      const Grid & grid() const;

      // int n_fcst_objs  () const;
      // int n_obs_objs   () const;

      // int n_fcst_comps () const;
      // int n_obs_comps  () const;

      double fcst_raw  (int x, int y) const;
      double  obs_raw  (int x, int y) const;

      int fcst_obj_id  (int x, int y) const;
      int fcst_comp_id (int x, int y) const;

      int obs_obj_id   (int x, int y) const;
      int obs_comp_id  (int x, int y) const;

      DataPlane select_fcst_obj  (int) const;   //  -1 for all
      DataPlane select_fcst_comp (int) const;   //  -1 for all

      DataPlane select_obs_obj   (int) const;   //  -1 for all
      DataPlane select_obs_comp  (int) const;   //  -1 for all

      bool x_line_valid(const int x) const;
      bool y_line_valid(const int y) const;

      void get_fcst_raw_range(double & min_value, double & max_value) const;

};


////////////////////////////////////////////////////////////////////////


inline int ModeNcFile::nx() const { return ( Nx ); }
inline int ModeNcFile::ny() const { return ( Ny ); }

inline const Grid & ModeNcFile::grid() const { return ( *_Grid ); }

// inline int ModeNcFile::n_fcst_objs () const { return ( NFcstObjs ); }
// inline int ModeNcFile::n_obs_objs  () const { return ( NObsObjs ); }
// 
// inline int ModeNcFile::n_fcst_comps () const { return ( NFcstComps ); }
// inline int ModeNcFile::n_obs_comps  () const { return ( NObsComps ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MODE_NC_FILE_H__  */


////////////////////////////////////////////////////////////////////////


