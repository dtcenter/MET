

////////////////////////////////////////////////////////////////////////


using namespace std;


////////////////////////////////////////////////////////////////////////


#include "indent.h"

#include "token.h"
#include "tokentype_to_string.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Token
   //


////////////////////////////////////////////////////////////////////////


Token::Token()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Token::~Token()

{

// set_eof();

}


////////////////////////////////////////////////////////////////////////


Token::Token(const Token & t)

{

init_from_scratch();

assign(t);

}


////////////////////////////////////////////////////////////////////////


Token & Token::operator=(const Token & t)

{

if ( this == &t )  return ( * this );

assign(t);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Token::assign(const Token & t)

{

clear();

type        = t.type;

text        = t.text;

 in_prec    = t.in_prec;
out_prec    = t.out_prec;

pos         = t.pos;

number_1b   = t.number_1b;

delta       = t.delta;

return;

}


////////////////////////////////////////////////////////////////////////


void Token::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Token::clear()

{

set_eof();

return;

}


////////////////////////////////////////////////////////////////////////


void Token::dump(ostream & out, int depth) const

{

Indent prefix(depth);


out << '\n';

out << prefix << "type      = " << tokentype_to_string(type) << '\n';


out << prefix << "text      = ";

if ( text.nonempty() )  out << '\"' << text << '\"';

out << '\n';

// out << prefix << "prec      = (" << in_prec << ' ' << out_prec << ")\n";

// out << prefix << "pos       = " << pos << '\n';

out << prefix << "number_1b = " << number_1b << '\n';

// out << prefix << "delta     = " << delta << '\n';


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void Token::set_eof()

{

text.clear();

delta = 0;

in_prec = out_prec = 0;

pos = -1;

number_1b = 0;

type = tok_eof;

return;

}


////////////////////////////////////////////////////////////////////////


void Token::set_union(int _pos)

{

pos = _pos;

type = tok_union;

text = union_char;

 in_prec = union_in_prec;
out_prec = union_out_prec;

number_1b = 0;

delta = union_delta;


return;

}


////////////////////////////////////////////////////////////////////////


void Token::set_intersection(int _pos)

{

pos = _pos;

type = tok_intersection;

text = intersection_char;

 in_prec = intersection_in_prec;
out_prec = intersection_out_prec;

number_1b = 0;

delta = intersection_delta;


return;

}


////////////////////////////////////////////////////////////////////////


void Token::set_mark(int _pos)

{

type = tok_mark;

text = mark_char;

in_prec = out_prec = 100;

pos = _pos;

delta = 0;

number_1b = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void Token::set_unmark(int _pos)

{

type = tok_unmark;

text = unmark_char;

in_prec = out_prec = 100;

pos = _pos;

delta = 0;

number_1b = 0;

return;

}

////////////////////////////////////////////////////////////////////////


void Token::set_local_var(int _number_1b, int _pos)

{

type = tok_local_var;

number_1b = _number_1b;

text.erase();

in_prec = out_prec = 0;

pos = _pos;

delta = 1;


return;

}


////////////////////////////////////////////////////////////////////////


void Token::set_negation(int _pos)

{

text = negation_char;

type = tok_negation;

 in_prec = negation_in_prec;
out_prec = negation_out_prec;

pos = _pos;

delta = negation_delta;


return;

}


////////////////////////////////////////////////////////////////////////


bool Token::is_operator() const

{

const bool tf =    (type == tok_union)
                || (type == tok_intersection)
                || (type == tok_negation);


return ( tf );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream & out, const Token & t)

{

t.dump(out);

return ( out );

}

////////////////////////////////////////////////////////////////////////




