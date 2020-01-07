// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_cal.h"
#include "vx_log.h"
#include "vx_util.h"

#include "ascii_header.h"

////////////////////////////////////////////////////////////////////////

//
// global instantiation of the AsciiHeader class
//

AsciiHeader METHdrTable;

////////////////////////////////////////////////////////////////////////

static const char * header_file_tmpl   = "MET_BASE/table_files/met_header_columns_VERSION.txt";
static const char * ascii_header_delim = ":";
static const char * var_index_reg_exp  = "^(.*)$";
static const char * var_col_name_str   = "[0-9]*";

////////////////////////////////////////////////////////////////////////

static void parse_mctc_fi_oj(const char *, int &, int &);

////////////////////////////////////////////////////////////////////////
//
// Code for class AsciiHeaderLine
//
////////////////////////////////////////////////////////////////////////

AsciiHeaderLine::AsciiHeaderLine() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

AsciiHeaderLine::~AsciiHeaderLine() {
   clear();
}

////////////////////////////////////////////////////////////////////////

AsciiHeaderLine::AsciiHeaderLine(const AsciiHeaderLine &a) {
   init_from_scratch();
   assign(a);
}

////////////////////////////////////////////////////////////////////////

AsciiHeaderLine & AsciiHeaderLine::operator=(const AsciiHeaderLine & a) {

   if(this == &a) return(*this);

   assign(a);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void AsciiHeaderLine::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void AsciiHeaderLine::assign(const AsciiHeaderLine &a) {

   Version        = a.Version;
   DataType       = a.DataType;
   LineType       = a.LineType;

   NVarCols       = a.NVarCols;
   VarIndexName   = a.VarIndexName;
   VarIndexOffset = a.VarIndexOffset;
   VarBegOffset   = a.VarBegOffset;

   ColNames       = a.ColNames;

   return;
}

////////////////////////////////////////////////////////////////////////

void AsciiHeaderLine::clear() {

   Version.clear();
   DataType.clear();
   LineType.clear();

   NVarCols       = 0;
   VarIndexName.clear();
   VarIndexOffset = bad_data_int;
   VarBegOffset   = bad_data_int;

   ColNames.clear();
   ColNames.set_ignore_case(true);

   return;
}

////////////////////////////////////////////////////////////////////////

void AsciiHeaderLine::set_col_names(const char *s) {
   ConcatString cs;
   StringArray tok;

   // Parse the header column names
   ColNames.parse_wsss(s);

   // Handle variable lenght columns
   for(int i=0; i<ColNames.n_elements(); i++) {

      // Check for the variable index column
     if(check_reg_exp(var_index_reg_exp, ColNames[i].c_str())) {

         // Can only have one variable index column
         if(VarIndexName.nonempty()) {
            mlog << Error << "\nAsciiHeaderLine::set(const char *line) -> "
                << "can't have multiple variable index columns in line:\n"
                << s << "\n\n";
            exit(1);
         }

         // Store the variable column name and offset
         cs             = ColNames[i];
         tok            = cs.split("()"); // Strip off parenthesis
         VarIndexName   = tok[0];
         VarIndexOffset = i;

         // Update the variable column name
         ColNames.set(i, (string)VarIndexName);
      }

      // Check for a variable length column
     else if(strstr(ColNames[i].c_str(), var_col_name_str) != 0) {

         // Check that the variable index column has already been set
         if(VarIndexName.empty()) {
            mlog << Error << "\nAsciiHeaderLine::set(const char *line) -> "
                 << "lines with variable length columns must also "
                 << "contain a variable index column:\n"
                 << s << "\n\n";
            exit(1);
         }

         // Increment the variable column counter
         NVarCols++;

         // Check for the first variable length column
         if(is_bad_data(VarBegOffset) || i < VarBegOffset) {
            VarBegOffset = i;
         }
      } // end else if

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

bool AsciiHeaderLine::is_mctc() const {
   return(LineType == "MCTC");
}

////////////////////////////////////////////////////////////////////////

bool AsciiHeaderLine::is_var_length() const {
   return(NVarCols > 0);
}

////////////////////////////////////////////////////////////////////////

int AsciiHeaderLine::n_var_cols(int dim) const {
   return(is_mctc() ? NVarCols * dim * dim : NVarCols * dim);
}

////////////////////////////////////////////////////////////////////////

int AsciiHeaderLine::length(int dim) const {
   return(ColNames.n_elements() - NVarCols + n_var_cols(dim));
}

////////////////////////////////////////////////////////////////////////

int AsciiHeaderLine::col_offset(const char *name, const int dim) const {
   int i, j, icur, match, offset;
   ConcatString reg_exp;

   // Handle fixed length lines
   if(!is_var_length()) {

      // Check for no match
      if(!ColNames.has(name, offset)) return(bad_data_int);

   }
   // Handle variable length lines
   else {

      // Search for column names using regular expressions
      for(i=0, match=bad_data_int; i<ColNames.n_elements(); i++) {
	reg_exp.format("^%s$", ColNames[i].c_str());
	if(check_reg_exp(reg_exp.c_str(), name)) {
            match = i;
            break;
         }
      }

      // Check for no match
      if(is_bad_data(match)) return(bad_data_int);

      // Fixed columns before variable ones
      if(match < VarBegOffset) {
         offset = match;             // Matching column offset
      }

      // Fixed columns after variable ones
      else if(match >= (VarBegOffset + NVarCols)) {
         offset = match +            // Matching column offset
                  (dim * NVarCols) - // Plus total of variable columns
                  NVarCols;          // Minus variable column names
      }

      // Variable columns
      else {

         // Handle MCTC special case (Fi_Oj) for NxN contingency table
         if(is_mctc()) {
            parse_mctc_fi_oj(name, i, j);
            offset = VarBegOffset + (i-1)*dim + (j-1);
         }

         // Handle the rest
         else {

            // Parse index from the column name
            icur = parse_thresh_index(name) - 1; // 0-based index

            offset = VarBegOffset +          // Beginning of variable columns
                     (icur * NVarCols) +     // Plus index times variable columns
                     (match - VarBegOffset); // Plus matching column minus beginning
         }
      }
   } // end else

   return(offset);
}

////////////////////////////////////////////////////////////////////////

ConcatString AsciiHeaderLine::col_name(const int offset, int const dim) const {
   int i, j;
   ConcatString name, str;

   // Range check
   if(offset < 0 || offset >= length(dim)) {
      mlog << Error << "\nAsciiHeaderLine::col_name() -> "
           << "range check error for data/line type \""
           << DataType << "/" << LineType << "\" and offset = "
           << offset << "\n\n";
      exit(1);
   }

   // Handle fixed length lines and columns
   if(!is_var_length() || offset < VarBegOffset) {
      name = ColNames[offset];
   }

   // Fixed columns after variable ones
   else if(offset >= (VarBegOffset + n_var_cols(dim))) {
      name = ColNames[offset - n_var_cols(dim) + 1];
   }

   // Variable columns
   else {

      // Handle MCTC special case (Fi_Oj) for NxN contingency table
      if(is_mctc()) {
         i = (offset - VarBegOffset) / dim;
         j = (offset - VarBegOffset) % dim;
         name.format("F%i_O%i", i+1, j+1);
      }

      // Handle the rest
      else {
         i = VarBegOffset + (offset - VarBegOffset) % NVarCols;
         str << cs_erase << (offset - VarBegOffset) / NVarCols + 1;
         name = str_replace(ColNames[i].c_str(), "[0-9]*", str.c_str());
      } // end else
   }

   return(name);
}

////////////////////////////////////////////////////////////////////////
//
// Code for class AsciiHeader
//
////////////////////////////////////////////////////////////////////////

AsciiHeader::AsciiHeader() {
   init_from_scratch();
   ConcatString version_mm(parse_version_major_minor(met_version));
   read(version_mm.c_str());
}

////////////////////////////////////////////////////////////////////////

AsciiHeader::~AsciiHeader() {
   clear();
}

////////////////////////////////////////////////////////////////////////

AsciiHeader::AsciiHeader(const AsciiHeader &a) {
   init_from_scratch();
   assign(a);
}

////////////////////////////////////////////////////////////////////////

AsciiHeader::AsciiHeader(const char *version) {
   init_from_scratch();
   read(version);
}

////////////////////////////////////////////////////////////////////////

AsciiHeader & AsciiHeader::operator=(const AsciiHeader & a) {

   if(this == &a) return(*this);

   assign(a);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void AsciiHeader::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void AsciiHeader::assign(const AsciiHeader &a) {

   Versions = a.Versions;
   Headers  = a.Headers;

   return;
}

////////////////////////////////////////////////////////////////////////

void AsciiHeader::clear() {

   Versions.clear();
   Headers.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void AsciiHeader::read(const char *version) {
   ConcatString file_name;
   DataLine line;
   AsciiHeaderLine header_line;
   LineDataFile in;

   // Set the delimiter for parsing this data
   line.set_delimiter(ascii_header_delim);

   // Read headers for the specified version of MET
   if(!Versions.has(version)) {

      // Substitute in the current version and handle MET_BASE
      file_name = replace_path(str_replace(header_file_tmpl, "VERSION", version));

      mlog << Debug(4)
           << "Reading MET header columns:\n" << file_name << "\n";

      // Open the data file
      if(!in.open(file_name.c_str())) {
         mlog << Error << "\nAsciiHeader::read() -> "
              << "trouble reading file:\n" << file_name << "\n\n";
         exit(1);
      }

      // Process each line from the input file
      while(in >> line) {

         // Check for blank line
         if(line.n_items() == 0) continue;

         // Check for the expected number of tokens
         if(line.n_items() != 4) {
            mlog << Error << "\nAsciiHeader::read() -> "
                 << "expected 4 tokens but found " << line.n_items()
                 << " on line number " << line.line_number()
                 << " of file:\n" << file_name << "\n\n";
            exit(1);
         }

         // Process data from the current line
         header_line.clear();
         header_line.set_version  (line[0]);
         header_line.set_data_type(line[1]);
         header_line.set_line_type(line[2]);
         header_line.set_col_names(line[3]);

         // Store the current line
         Headers.push_back(header_line);
      }

      // Store the version we just loaded
      Versions.add(version);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

const AsciiHeaderLine * AsciiHeader::header(const char *version,
                                            const char *data_type,
                                            const char *line_type) {

   ConcatString version_mm(parse_version_major_minor(version));

   // Check if the version needs to be loaded
   if(!Versions.has(version_mm)) read(version_mm.c_str());

   // Find matching header line
   // Allow NA for line_type to match any line type
   vector<AsciiHeaderLine>::const_iterator it;
   for(it = Headers.begin(); it != Headers.end(); ++it) {
      if(strcmp(it->version(),    version_mm.c_str()) == 0 &&
         strcmp(it->data_type(),  data_type)          == 0 &&
         (strcmp(it->line_type(), line_type)          == 0 ||
          strcmp(na_str,          line_type)          == 0)) break;
   }

   // Check for no match
   if(it == Headers.end()) {
      mlog << Error << "\nAsciiHeaderLine::header() -> "
           << "can't find header columns for MET version \"" << version
           << "\", data type \"" << data_type << "\", line type \""
           << line_type << "\"!\n\n";
      exit(1);
   }

   return(&(*it));
}

////////////////////////////////////////////////////////////////////////

int AsciiHeader::col_offset(const char *version,   const char *data_type,
                            const char *line_type, const char *name,
                            const int dim) {
   const AsciiHeaderLine *line = header(version, data_type, line_type);

   return(line->col_offset(name, dim));
}

////////////////////////////////////////////////////////////////////////

ConcatString AsciiHeader::col_name(const char *version,   const char *data_type,
                                   const char *line_type, const int offset,
                                   const int dim) {
   const AsciiHeaderLine *line = header(version, data_type, line_type);

   return(line->col_name(offset, dim));
}

////////////////////////////////////////////////////////////////////////
//
// Begin miscellaneous utility functions
//
////////////////////////////////////////////////////////////////////////

void parse_mctc_fi_oj(const char *str, int &i, int &j) {
   const char *ptr = str + 1;

   // Parse Fi_Oj strings
   i = atoi(ptr);

   if((ptr = strrchr(str, '_')) != NULL) {
      ptr += 2;
      j = atoi(ptr);
   }
   else {
      mlog << Error << "\nparse_mctc_fi_oj() -> "
           << "unexpected column name specified: \""
           << str << "\"\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
