

////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_2D_MOMENTS_H__
#define  __MTD_2D_MOMENTS_H__


////////////////////////////////////////////////////////////////////////


class Mtd_2D_Moments {

   public:

      void init_from_scratch();

      void assign(const Mtd_2D_Moments &);

      int N;    //  0th order moments

      double Sx;    //  1st order moments
      double Sy;    // 

      double Sxx;   //  2nd order CENTRAL moments
      double Sxy;   //  
      double Syy;   //  

      bool IsCentralized;   //  default: false

   public:

      Mtd_2D_Moments();
     ~Mtd_2D_Moments();
      Mtd_2D_Moments(const Mtd_2D_Moments &);
      Mtd_2D_Moments & operator=(const Mtd_2D_Moments &);

      void clear();

         //
         //  set stuff
         //


         //
         //  get stuff
         //

      int area() const;


         //
         //  do stuff
         //

      void add(double x, double y);

      void centralize();


      double calc_2D_axis_plane_angle() const;



};


////////////////////////////////////////////////////////////////////////


inline int Mtd_2D_Moments::area() const { return ( N ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_2D_MOMENTS_H__  */


////////////////////////////////////////////////////////////////////////


