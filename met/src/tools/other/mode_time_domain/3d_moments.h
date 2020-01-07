// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_3D_MOMENTS_H__
#define  __MTD_3D_MOMENTS_H__


////////////////////////////////////////////////////////////////////////


class Mtd_3D_Moments {

   public:

      void init_from_scratch();

      void assign(const Mtd_3D_Moments &);

      int N;    //  0th order moments

      double Sx;    //  1st order moments
      double Sy;    // 
      double St;    // 

      double Sxx;   //  2nd order CENTRAL moments
      double Sxy;   //  
      double Sxt;   //  
      double Syy;   //  
      double Syt;   //  
      double Stt;   //  

      bool IsCentralized;   //  default: false

   public:

      Mtd_3D_Moments();
     ~Mtd_3D_Moments();
      Mtd_3D_Moments(const Mtd_3D_Moments &);
      Mtd_3D_Moments & operator=(const Mtd_3D_Moments &);

      void clear();

         //
         //  set stuff
         //


         //
         //  get stuff
         //

      int volume() const;


         //
         //  do stuff
         //

      void add(double x, double y, double t);

      void centralize();


      void calc_3d_velocity(double & vx, double & vy) const;

      double calc_3d_axis_plane_angle() const;



};


////////////////////////////////////////////////////////////////////////


inline int Mtd_3D_Moments::volume() const { return ( N ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_3D_MOMENTS_H__  */


////////////////////////////////////////////////////////////////////////


