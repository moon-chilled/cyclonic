/* Copyright (C) 2007 AT&T.
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

// Defines the interface to the termgrammar stack, which is used for creating a
// message as output from parsing a termgrammar term.

#ifndef TG_STACK_H
#define TG_STACK_H

extern void tg_init();
extern int branch_id;

////////////////
// These functions belong in separate file (e.g. pm_lib.cyc),
// but are placed here for convenience.
extern void yk_store(const char?);
extern void set_branch(int);
#endif
