#ifndef LEXER_H
#define LEXER_H

#include "core.h"
#include "cstdio.h"
//#include "cstring.h"
#include "lexing.h"

#include "syntax.h"
#include "parser.h"

using Core {
using std {
using Lexing {
using Parser {
namespace Lexer {

extern xtunion exn { extern Lexical_error(string_t,int,int); };

extern int lexmain(Lexing::Lexbuf<`a>);
extern int line_num;
extern int line_start_pos;
} // namespace Lexer
}}}}

#endif
