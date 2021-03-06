/* Copyright (C) 2005 Greg Morrisett, AT&T.
 This file is part of the Cyclone project.

 Cyclone is free software; you can redistribute it
 and/or modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2 of
 the License, or (at your option) any later version.

 Cyclone is distributed in the hope that it will be
 useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Cyclone; see the file COPYING. If not,
 write to the Free Software Foundation, Inc., 59 Temple Place -
 Suite 330, Boston, MA 02111-1307, USA. */

#ifndef CRAWLERGEN_H
#define CRAWLERGEN_H
#include "bnf.h"

namespace Crawlergen {
  extern void gen_crawl(grammar_t<`H> grm, const char ?`H symb,
      List::list_t<const char ?@>textblobs,
      int all_start, unsigned int eof_val);
  /* Print forward definitions of parsing functions in grammar */
  extern void gen_header(grammar_t<`H> grm,
			 List::list_t<const char ?@>textblobs);
  extern const char ?cyc_namespace;
  extern int gen_fun_table;
}
#endif
