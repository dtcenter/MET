

////////////////////////////////////////////////////////////////////////


#ifndef  __ALG_LINE_H__
#define  __ALG_LINE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


static const int alg_line_size = 256;


////////////////////////////////////////////////////////////////////////


class AlgLine {

      friend ostream & operator<<(ostream &, const AlgLine &);

   private:

      char line[alg_line_size];

      int L;
      int R;

      void assign(const AlgLine &);

      void init_from_scratch();


      void start(const char *);


   public:

      AlgLine();
     ~AlgLine();
      AlgLine(const AlgLine &);
      AlgLine & operator=(const AlgLine &);

      void clear();

      int prec;


      void prepend(const char *);
      void append(const char *);

      void prepend(const AlgLine &);
      void append(const AlgLine &);

      void parenthesize();

};


////////////////////////////////////////////////////////////////////////


extern ostream & operator<<(ostream &, const AlgLine &);


////////////////////////////////////////////////////////////////////////


static const int max_algline_stack_depth = 30;


////////////////////////////////////////////////////////////////////////


class AlgLineStack {

   private:

      AlgLine s[max_algline_stack_depth];

      int Depth;

      void init_from_scratch();

      void assign(const AlgLineStack &);

   public:

      AlgLineStack();
     ~AlgLineStack();
      AlgLineStack(const AlgLineStack &);
      AlgLineStack & operator=(const AlgLineStack &);

      void clear();

      void push(const AlgLine &);

      const AlgLine & pop();

      int depth() const;

};


////////////////////////////////////////////////////////////////////////


inline int AlgLineStack::depth() const { return ( Depth ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __ALG_LINE_H__  */


////////////////////////////////////////////////////////////////////////


