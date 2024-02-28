

////////////////////////////////////////////////////////////////////////


#ifndef  __MY_BOOL_TOKEN_H__
#define  __MY_BOOL_TOKEN_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <vector>

#include "concat_string.h"


////////////////////////////////////////////////////////////////////////


static const char union_char[]          = "||";
static const char intersection_char[]   = "&&";
static const char negation_char         = '!';

static const char   mark_char           = '(';
static const char unmark_char           = ')';

static const char local_var_char        = '#';


////////////////////////////////////////////////////////////////////////


      //  delta = net pushes minus pops

static const int  union_delta           = -1;
static const int  intersection_delta    = -1;
static const int  negation_delta        =  0;
static const int  local_var_delta       =  1;

      //

static const int union_in_prec          = 2;
static const int union_out_prec         = 1;

static const int intersection_in_prec   = 4;
static const int intersection_out_prec  = 3;

static const int negation_in_prec       = 10;
static const int negation_out_prec      = 10;


////////////////////////////////////////////////////////////////////////


enum TokenType {

   tok_union, 
   tok_intersection, 
   tok_negation, 

   tok_local_var, 

   tok_mark, 
   tok_unmark,

   tok_eof,

   no_token_type, 

};


////////////////////////////////////////////////////////////////////////


class Token {

      void init_from_scratch();

      void assign(const Token &);

   public:

      Token();   //  defaults to eof
     ~Token();
      Token(const Token &);
      Token & operator=(const Token &);

      void clear();

      void dump(std::ostream &, int = 0) const;


      TokenType type;

      ConcatString text;

      int  in_prec;
      int out_prec;

      int pos;         //  0-based

      int number_1b;   //  for local variables, 1-based

      int delta;       //  net pushes minus pops



      bool is_operand  () const;   //  ie, a local variable
      bool is_operator () const;

      bool is_mark   () const;
      bool is_unmark () const;

      bool is_eof () const;

         //

      void set_eof();

      void set_mark   (int = -1);
      void set_unmark (int = -1);

      void set_local_var(int _number_1b, int _pos = -1);   //  local variable

      void set_negation(int _pos = -1);   //  negation  (ie, logical not)

      void set_union(int _pos = -1);
      void set_intersection(int _pos = -1);



      // void print() const;

      int  prec() const;   //  the "out" prec

};


////////////////////////////////////////////////////////////////////////


extern std::ostream & operator<<(std::ostream &, const Token &);


////////////////////////////////////////////////////////////////////////


// inline void Token::print() const { 
// 
//    if ( result.nonempty() )  result << ',';
// 
//    if ( type == tok_local_var )  result << local_var_char << number;
//    else                          result << value;
// 
//    cout << value << '\n' << flush;
// 
//    return;
// 
// }


////////////////////////////////////////////////////////////////////////


inline int  Token::prec        () const { return out_prec; }

inline bool Token::is_mark     () const { return ( type == tok_mark   ); }
inline bool Token::is_unmark   () const { return ( type == tok_unmark ); }

inline bool Token::is_eof      () const { return ( type == tok_eof ); }

inline bool Token::is_operand  () const { return ( type == tok_local_var  ); }


////////////////////////////////////////////////////////////////////////


typedef std::vector<Token> Program;


////////////////////////////////////////////////////////////////////////


#endif   /*  __MY_BOOL_TOKEN_H__  */


////////////////////////////////////////////////////////////////////////


