// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __VPS_TEXTNODE_H__
#define  __VPS_TEXTNODE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "afm.h"


////////////////////////////////////////////////////////////////////////


static const int vx_pstextnode_alloc_inc = 100;


////////////////////////////////////////////////////////////////////////


class VxpsTextNode {

   private:

      void init_from_scratch();

      void assign(const VxpsTextNode &);

      void extend(int);

      void set_text(const char *);


      int FontNumber;

      double FontSize;

      double Dx;   //  Kern amount

      double Width;

      double Left;
      double Right;
      double Bottom;
      double Top;

      char * Text;

      int Nchars;

      int Nalloc;

   public:

      VxpsTextNode();
     ~VxpsTextNode();
      VxpsTextNode(const VxpsTextNode &);
      VxpsTextNode & operator=(const VxpsTextNode &);

      void clear();

      void dump(ostream &, int depth = 0) const;


      void set_font_number(int);

      void set_font_size(double);

      void set_dx(double);

      void add_char(const AfmCharMetrics &);

      void add_link();


      VxpsTextNode * next;

      int font_number() const;

      double font_size() const;

      const char * text() const;

      double width () const;

      double left   () const;
      double right  () const;
      double bottom () const;
      double top    () const;

      double total_width () const;

      double total_left   () const;
      double total_right  () const;
      double total_bottom () const;
      double total_top    () const;

      double dx () const;

      int nchars() const;

      int is_empty() const;

};


////////////////////////////////////////////////////////////////////////


inline int    VxpsTextNode::font_number() const { return ( FontNumber ); }
inline double VxpsTextNode::font_size()   const { return ( FontSize   ); }

inline const char * VxpsTextNode::text() const { return ( Text ); }

inline double VxpsTextNode::width () const { return ( Width  ); }

inline double VxpsTextNode::right  () const { return ( Right  ); }
inline double VxpsTextNode::left   () const { return ( Left   ); }
inline double VxpsTextNode::bottom () const { return ( Bottom ); }
inline double VxpsTextNode::top    () const { return ( Top    ); }

inline int VxpsTextNode::nchars() const { return ( Nchars ); }

inline double VxpsTextNode::dx () const { return ( Dx ); }

inline int VxpsTextNode::is_empty() const { return ( Text ? 0 : 1 ); }


////////////////////////////////////////////////////////////////////////


#endif   //  __VPS_TEXTNODE_H__


////////////////////////////////////////////////////////////////////////


