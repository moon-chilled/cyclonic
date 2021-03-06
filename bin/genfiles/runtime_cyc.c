/* This file is part of the Cyclone Library.
   Copyright (C) 2001 Greg Morrisett, AT&T

   This library is free software; you can redistribute it and/or it
   under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place, Suite
   330, Boston, MA 02111-1307 USA. */

// This is the C "runtime library" to be used with the output of the
// Cyclone to C translator

#include <stdio.h>
#include "runtime_internal.h"

/* struct _fat_ptr Cstring_to_string(Cstring s) { */
/*   struct _fat_ptr str; */
/*   if (s == NULL) { */
/*     str.base = str.curr = str.last_plus_one = NULL; */
/*   } */
/*   else { */
/*     int sz = strlen(s)+1; */
/*     str.curr = (char *)_cycalloc_atomic(sz); */
/*     if (str.curr == NULL)  */
/*       _throw_badalloc(); */
/*     str.base = str.curr; */
/*     str.last_plus_one = str.curr + sz; */

/*     // Copy the string in case the C code frees it or mangles it */
/*     str.curr[--sz] = '\0'; */
/*     while(--sz>=0) */
/*       str.curr[sz]=s[sz]; */
/*   } */
/*   return str; */
/* } */

// no longer copying the C string (see above if this is bogus)
static struct _fat_ptr Cstring_to_string(char *s) {
  struct _fat_ptr str;
  if (s == NULL) {
    str.base = str.curr = str.last_plus_one = NULL;
  }
  else {
    unsigned int sz = strlen(s)+1;
    str.curr = str.base = (unsigned char *)s;
    str.last_plus_one = str.curr + sz;
  }
  return str;
}

// argc is redundant
struct _fat_argv {
  struct _fat_ptr *base;
  struct _fat_ptr *curr;
  struct _fat_ptr *last_plus_one;
};

// Define struct __cycFILE, and initialize stdin, stdout, stderr
struct Cyc___cycFILE { // must match defn in boot_cstubs.c and boot_cycstubs.cyc
  FILE *file;
} Cyc_stdin_v, Cyc_stdout_v, Cyc_stderr_v,
  *Cyc_stdin = &Cyc_stdin_v,
  *Cyc_stdout = &Cyc_stdout_v,
  *Cyc_stderr = &Cyc_stderr_v;

extern int Cyc_main(int argc, struct _fat_argv argv);
extern char *_set_top_handler(); // defined in runtime_exception.c

/* #ifdef _HAVE_PTHREAD_ */
/* static pthread_once_t key_once = PTHREAD_ONCE_INIT; */
/* void init_keys_once() { */
/*   int status; */
/*   //  fprintf(stderr, "Initing keys ... \n"); */
/*   _init_exceptions(); */
/*   _init_stack(); */
/* } */
/* #endif */

extern void GC_init();
int main(int argc, char **argv) {
  GC_init();
  // initialize region system
  int status;
  _init_regions();
  // MWH: could do this in pthread_once above, but there's no need
  // because we won't have multiple main() threads
  _init_stack();
  _init_exceptions();
  // install outermost exception handler
  _set_top_handler();
  // set standard file descriptors
  Cyc_stdin->file  = stdin;
  Cyc_stdout->file = stdout;
  Cyc_stderr->file = stderr;
  // convert command-line args to Cyclone strings -- we add an extra
  // NULL to the end of the argv so that people can step through argv
  // until they hit NULL. (Calling calloc with argc+1 takes care of this.)
  {
    struct _fat_argv args;
    int i, result;
    args.curr = (struct _fat_ptr *)GC_calloc(argc+1,sizeof(struct _fat_ptr));
    args.base = args.curr;
    args.last_plus_one = args.curr + argc + 1;
    for(i = 0; i < argc; ++i)
      args.curr[i] = Cstring_to_string(argv[i]);
    result = Cyc_main(argc, args);
    _fini_regions();
    return result;
  }
}
