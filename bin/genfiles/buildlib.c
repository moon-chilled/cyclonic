#include <setjmp.h>
/* This is a C header file to be used by the output of the Cyclone to
   C translator.  The corresponding definitions are in file
   lib/runtime_cyc.c
*/
#ifndef _CYC_INCLUDE_H_
#define _CYC_INCLUDE_H_

/***********************************************************************/
/* Runtime Stack routines (runtime_stack.c).                           */
/***********************************************************************/

/* Need one of these per thread (we don't have threads)
   The runtime maintains a stack that contains either _handler_cons
   structs or _RegionHandle structs.  The tag is 0 for a handler_cons
   and 1 for a region handle.  */
struct _RuntimeStack {
  int tag; /* 0 for an exception handler, 1 for a region handle */
  struct _RuntimeStack *next;
  void (*cleanup)(struct _RuntimeStack *frame);
};

// pushes a frame on the stack
void _push_frame(struct _RuntimeStack *frame);

// pop N+1 frames from the stack (error if stack_size < N+1)
void _npop_frame(unsigned int n);

// returns top frame on the stack (NULL if stack is empty)
struct _RuntimeStack * _top_frame();

// pops off frames until a frame with the given tag is reached.  This
// frame is returned, or else NULL if none found.
struct _RuntimeStack * _pop_frame_until(int tag);

/***********************************************************************/
/* Low-level representations etc.                                      */
/***********************************************************************/

#ifdef NO_CYC_PREFIX
#define ADD_PREFIX(x) x
#else
#define ADD_PREFIX(x) Cyc_##x
#endif

#ifndef offsetof
/* should be size_t, but int is fine. */
#define offsetof(t,n) ((int)(&(((t *)0)->n)))
#endif

/* Tagged arrays */
struct _dyneither_ptr {
  unsigned char *curr; 
  unsigned char *base; 
  unsigned char *last_plus_one; 
};  

/* Discriminated Unions */
struct _xtunion_struct { char *tag; };

/* Regions */
struct _RegionPage {
#ifdef CYC_REGION_PROFILE
  unsigned total_bytes;
  unsigned free_bytes;
#endif
  struct _RegionPage *next;
  char data[1];  /*FJS: used to be size 0, but that's forbidden in ansi c*/
};

struct _RegionHandle {
  struct _RuntimeStack s;
  struct _RegionPage *curr;
  char               *offset;
  char               *last_plus_one;
  struct _DynRegionHandle *sub_regions;
#ifdef CYC_REGION_PROFILE
  const char         *name;
#else
  unsigned used_bytes;
  unsigned wasted_bytes;
#endif
};

struct _DynRegionFrame {
  struct _RuntimeStack s;
  struct _DynRegionHandle *x;
};

// A dynamic region is just a region handle.  We have the
// wrapper struct for type abstraction reasons.
struct Cyc_Core_DynamicRegion {
  struct _RegionHandle h;
};

extern struct _RegionHandle _new_region(const char *);
extern void * _region_malloc(struct _RegionHandle *, unsigned);
extern void * _region_calloc(struct _RegionHandle *, unsigned t, unsigned n);
extern void   _free_region(struct _RegionHandle *);
extern void   _reset_region(struct _RegionHandle *);
extern struct _RegionHandle *_open_dynregion(struct _DynRegionFrame *f,
                                             struct _DynRegionHandle *h);
extern void   _pop_dynregion();

/* Exceptions */
struct _handler_cons {
  struct _RuntimeStack s;
  jmp_buf handler;
};
extern void _push_handler(struct _handler_cons *);
extern void _push_region(struct _RegionHandle *);
extern void _npop_handler(int);
extern void _pop_handler();
extern void _pop_region();

#ifndef _throw
extern int _throw_null_fn(const char *filename, unsigned lineno);
extern int _throw_arraybounds_fn(const char *filename, unsigned lineno);
extern int _throw_badalloc_fn(const char *filename, unsigned lineno);
extern int _throw_match_fn(const char *filename, unsigned lineno);
extern int _throw_fn(void* e, const char *filename, unsigned lineno);
extern int _rethrow(void* e);
#define _throw_null() (_throw_null_fn(__FILE__,__LINE__))
#define _throw_arraybounds() (_throw_arraybounds_fn(__FILE__,__LINE__))
#define _throw_badalloc() (_throw_badalloc_fn(__FILE__,__LINE__))
#define _throw_match() (_throw_match_fn(__FILE__,__LINE__))
#define _throw(e) (_throw_fn((e),__FILE__,__LINE__))
#endif

extern struct _xtunion_struct *_exn_thrown;

/* Built-in Exceptions */
struct Cyc_Null_Exception_exn_struct { char *tag; };
struct Cyc_Array_bounds_exn_struct { char *tag; };
struct Cyc_Match_Exception_exn_struct { char *tag; };
struct Cyc_Bad_alloc_exn_struct { char *tag; };
extern char Cyc_Null_Exception[];
extern char Cyc_Array_bounds[];
extern char Cyc_Match_Exception[];
extern char Cyc_Bad_alloc[];

/* Built-in Run-time Checks and company */
#ifdef __APPLE__
#define _INLINE_FUNCTIONS
#endif

#ifdef CYC_ANSI_OUTPUT
#define _INLINE  
#define _INLINE_FUNCTIONS
#else
#define _INLINE inline
#endif

#ifdef VC_C
#define _CYC_U_LONG_LONG_T __int64
#else
#ifdef GCC_C
#define _CYC_U_LONG_LONG_T unsigned long long
#else
#define _CYC_U_LONG_LONG_T unsigned long long
#endif
#endif

#ifdef NO_CYC_NULL_CHECKS
#define _check_null(ptr) (ptr)
#else
#ifdef _INLINE_FUNCTIONS
static _INLINE void *
_check_null_fn(const void *ptr, const char *filename, unsigned lineno) {
  void*_check_null_temp = (void*)(ptr);
  if (!_check_null_temp) _throw_null_fn(filename,lineno);
  return _check_null_temp;
}
#define _check_null(p) (_check_null_fn((p),__FILE__,__LINE__))
#else
#define _check_null(ptr) \
  ({ void*_check_null_temp = (void*)(ptr); \
     if (!_check_null_temp) _throw_null(); \
     _check_null_temp; })
#endif
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_known_subscript_null(ptr,bound,elt_sz,index) ({ \
  ((char *)ptr) + (elt_sz)*(index); })
#else
#ifdef _INLINE_FUNCTIONS
static _INLINE char *
_check_known_subscript_null_fn(void *ptr, unsigned bound, unsigned elt_sz, unsigned index, const char *filename, unsigned lineno) {
  void*_cks_ptr = (void*)(ptr);
  unsigned _cks_bound = (bound);
  unsigned _cks_elt_sz = (elt_sz);
  unsigned _cks_index = (index);
  if (!_cks_ptr) _throw_null_fn(filename,lineno);
  if (_cks_index >= _cks_bound) _throw_arraybounds_fn(filename,lineno);
  return ((char *)_cks_ptr) + _cks_elt_sz*_cks_index;
}
#define _check_known_subscript_null(p,b,e) (_check_known_subscript_null_fn(p,b,e,__FILE__,__LINE__))
#else
#define _check_known_subscript_null(ptr,bound,elt_sz,index) ({ \
  void*_cks_ptr = (void*)(ptr); \
  unsigned _cks_bound = (bound); \
  unsigned _cks_elt_sz = (elt_sz); \
  unsigned _cks_index = (index); \
  if (!_cks_ptr) _throw_null(); \
  if (_cks_index >= _cks_bound) _throw_arraybounds(); \
  ((char *)_cks_ptr) + _cks_elt_sz*_cks_index; })
#endif
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_known_subscript_notnull(bound,index) (index)
#else
#ifdef _INLINE_FUNCTIONS
static _INLINE unsigned
_check_known_subscript_notnull_fn(unsigned bound,unsigned index,const char *filename,unsigned lineno) { 
  unsigned _cksnn_bound = (bound); 
  unsigned _cksnn_index = (index); 
  if (_cksnn_index >= _cksnn_bound) _throw_arraybounds_fn(filename,lineno); 
  return _cksnn_index;
}
#define _check_known_subscript_notnull(b,i) (_check_known_subscript_notnull_fn(b,i,__FILE__,__LINE__))
#else
#define _check_known_subscript_notnull(bound,index) ({ \
  unsigned _cksnn_bound = (bound); \
  unsigned _cksnn_index = (index); \
  if (_cksnn_index >= _cksnn_bound) _throw_arraybounds(); \
  _cksnn_index; })
#endif
#endif

/* Add i to zero-terminated pointer x.  Checks for x being null and
   ensures that x[0..i-1] are not 0. */
#ifdef NO_CYC_BOUNDS_CHECK
#define _zero_arr_plus_char_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_short_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_int_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_float_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_double_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_longdouble_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_voidstar_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#else
static _INLINE char *
_zero_arr_plus_char_fn(char *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0 || orig_sz == 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz-1; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE short *
_zero_arr_plus_short_fn(short *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0 || orig_sz == 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz-1; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE int *
_zero_arr_plus_int_fn(int *orig_x, unsigned int orig_sz, int orig_i, const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0 || orig_sz == 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz-1; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE float *
_zero_arr_plus_float_fn(float *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0 || orig_sz == 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz-1; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE double *
_zero_arr_plus_double_fn(double *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0 || orig_sz == 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz-1; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE long double *
_zero_arr_plus_longdouble_fn(long double *orig_x, unsigned int orig_sz, int orig_i, const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0 || orig_sz == 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz-1; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE void *
_zero_arr_plus_voidstar_fn(void **orig_x, unsigned int orig_sz, int orig_i,const char *filename,unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0 || orig_sz == 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz-1; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
#endif

#define _zero_arr_plus_char(x,s,i) \
  (_zero_arr_plus_char_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_short(x,s,i) \
  (_zero_arr_plus_short_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_int(x,s,i) \
  (_zero_arr_plus_int_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_float(x,s,i) \
  (_zero_arr_plus_float_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_double(x,s,i) \
  (_zero_arr_plus_double_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_longdouble(x,s,i) \
  (_zero_arr_plus_longdouble_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_voidstar(x,s,i) \
  (_zero_arr_plus_voidstar_fn(x,s,i,__FILE__,__LINE__))


/* Calculates the number of elements in a zero-terminated, thin array.
   If non-null, the array is guaranteed to have orig_offset elements. */
static _INLINE int
_get_zero_arr_size_char(const char *orig_x, unsigned int orig_offset) {
  const char *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_short(const short *orig_x, unsigned int orig_offset) {
  const short *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_int(const int *orig_x, unsigned int orig_offset) {
  const int *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_float(const float *orig_x, unsigned int orig_offset) {
  const float *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_double(const double *orig_x, unsigned int orig_offset) {
  const double *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_longdouble(const long double *orig_x, unsigned int orig_offset) {
  const long double *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_voidstar(const void **orig_x, unsigned int orig_offset) {
  const void **_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}


/* Does in-place addition of a zero-terminated pointer (x += e and ++x).  
   Note that this expands to call _zero_arr_plus_<type>_fn. */
static _INLINE char *
_zero_arr_inplace_plus_char_fn(char **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_char_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_char(x,i) \
  _zero_arr_inplace_plus_char_fn((char **)(x),i,__FILE__,__LINE__)
static _INLINE short *
_zero_arr_inplace_plus_short_fn(short **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_short_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_short(x,i) \
  _zero_arr_inplace_plus_short_fn((short **)(x),i,__FILE__,__LINE__)
static _INLINE int *
_zero_arr_inplace_plus_int(int **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_int_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_int(x,i) \
  _zero_arr_inplace_plus_int_fn((int **)(x),i,__FILE__,__LINE__)
static _INLINE float *
_zero_arr_inplace_plus_float_fn(float **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_float_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_float(x,i) \
  _zero_arr_inplace_plus_float_fn((float **)(x),i,__FILE__,__LINE__)
static _INLINE double *
_zero_arr_inplace_plus_double_fn(double **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_double_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_double(x,i) \
  _zero_arr_inplace_plus_double_fn((double **)(x),i,__FILE__,__LINE__)
static _INLINE long double *
_zero_arr_inplace_plus_longdouble_fn(long double **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_longdouble_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_longdouble(x,i) \
  _zero_arr_inplace_plus_longdouble_fn((long double **)(x),i,__FILE__,__LINE__)
static _INLINE void *
_zero_arr_inplace_plus_voidstar_fn(void ***x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_voidstar_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_voidstar(x,i) \
  _zero_arr_inplace_plus_voidstar_fn((void ***)(x),i,__FILE__,__LINE__)

/* Does in-place increment of a zero-terminated pointer (e.g., x++). */
static _INLINE char *
_zero_arr_inplace_plus_post_char_fn(char **x, int orig_i,const char *filename,unsigned lineno){
  char * _zap_res = *x;
  *x = _zero_arr_plus_char_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_char(x,i) \
  _zero_arr_inplace_plus_post_char_fn((char **)(x),(i),__FILE__,__LINE__)
static _INLINE short *
_zero_arr_inplace_plus_post_short_fn(short **x, int orig_i,const char *filename,unsigned lineno){
  short * _zap_res = *x;
  *x = _zero_arr_plus_short_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_short(x,i) \
  _zero_arr_inplace_plus_post_short_fn((short **)(x),(i),__FILE__,__LINE__)
static _INLINE int *
_zero_arr_inplace_plus_post_int_fn(int **x, int orig_i,const char *filename, unsigned lineno){
  int * _zap_res = *x;
  *x = _zero_arr_plus_int_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_int(x,i) \
  _zero_arr_inplace_plus_post_int_fn((int **)(x),(i),__FILE__,__LINE__)
static _INLINE float *
_zero_arr_inplace_plus_post_float_fn(float **x, int orig_i,const char *filename, unsigned lineno){
  float * _zap_res = *x;
  *x = _zero_arr_plus_float_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_float(x,i) \
  _zero_arr_inplace_plus_post_float_fn((float **)(x),(i),__FILE__,__LINE__)
static _INLINE double *
_zero_arr_inplace_plus_post_double_fn(double **x, int orig_i,const char *filename,unsigned lineno){
  double * _zap_res = *x;
  *x = _zero_arr_plus_double_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_double(x,i) \
  _zero_arr_inplace_plus_post_double_fn((double **)(x),(i),__FILE__,__LINE__)
static _INLINE long double *
_zero_arr_inplace_plus_post_longdouble_fn(long double **x, int orig_i,const char *filename,unsigned lineno){
  long double * _zap_res = *x;
  *x = _zero_arr_plus_longdouble_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_longdouble(x,i) \
  _zero_arr_inplace_plus_post_longdouble_fn((long double **)(x),(i),__FILE__,__LINE__)
static _INLINE void **
_zero_arr_inplace_plus_post_voidstar_fn(void ***x, int orig_i,const char *filename,unsigned lineno){
  void ** _zap_res = *x;
  *x = _zero_arr_plus_voidstar_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_voidstar(x,i) \
  _zero_arr_inplace_plus_post_voidstar_fn((void***)(x),(i),__FILE__,__LINE__)

/* functions for dealing with dynamically sized pointers */
#ifdef NO_CYC_BOUNDS_CHECKS
#ifdef _INLINE_FUNCTIONS
static _INLINE unsigned char *
_check_dyneither_subscript(struct _dyneither_ptr arr,unsigned elt_sz,unsigned index) {
  struct _dyneither_ptr _cus_arr = (arr);
  unsigned _cus_elt_sz = (elt_sz);
  unsigned _cus_index = (index);
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index;
  return _cus_ans;
}
#else
#define _check_dyneither_subscript(arr,elt_sz,index) ({ \
  struct _dyneither_ptr _cus_arr = (arr); \
  unsigned _cus_elt_sz = (elt_sz); \
  unsigned _cus_index = (index); \
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index; \
  _cus_ans; })
#endif
#else
#ifdef _INLINE_FUNCTIONS
static _INLINE unsigned char *
_check_dyneither_subscript_fn(struct _dyneither_ptr arr,unsigned elt_sz,unsigned index,const char *filename, unsigned lineno) {
  struct _dyneither_ptr _cus_arr = (arr);
  unsigned _cus_elt_sz = (elt_sz);
  unsigned _cus_index = (index);
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index;
  /* JGM: not needed! if (!_cus_arr.base) _throw_null(); */ 
  if (_cus_ans < _cus_arr.base || _cus_ans >= _cus_arr.last_plus_one)
    _throw_arraybounds_fn(filename,lineno);
  return _cus_ans;
}
#define _check_dyneither_subscript(a,s,i) \
  _check_dyneither_subscript_fn(a,s,i,__FILE__,__LINE__)
#else
#define _check_dyneither_subscript(arr,elt_sz,index) ({ \
  struct _dyneither_ptr _cus_arr = (arr); \
  unsigned _cus_elt_sz = (elt_sz); \
  unsigned _cus_index = (index); \
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index; \
  /* JGM: not needed! if (!_cus_arr.base) _throw_null();*/ \
  if (_cus_ans < _cus_arr.base || _cus_ans >= _cus_arr.last_plus_one) \
    _throw_arraybounds(); \
  _cus_ans; })
#endif
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE struct _dyneither_ptr
_tag_dyneither(const void *tcurr,unsigned elt_sz,unsigned num_elts) {
  struct _dyneither_ptr _tag_arr_ans;
  _tag_arr_ans.base = _tag_arr_ans.curr = (void*)(tcurr);
  _tag_arr_ans.last_plus_one = _tag_arr_ans.base + (elt_sz) * (num_elts);
  return _tag_arr_ans;
}
#else
#define _tag_dyneither(tcurr,elt_sz,num_elts) ({ \
  struct _dyneither_ptr _tag_arr_ans; \
  _tag_arr_ans.base = _tag_arr_ans.curr = (void*)(tcurr); \
  _tag_arr_ans.last_plus_one = _tag_arr_ans.base + (elt_sz) * (num_elts); \
  _tag_arr_ans; })
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE struct _dyneither_ptr *
_init_dyneither_ptr(struct _dyneither_ptr *arr_ptr,
                    void *arr, unsigned elt_sz, unsigned num_elts) {
  struct _dyneither_ptr *_itarr_ptr = (arr_ptr);
  void* _itarr = (arr);
  _itarr_ptr->base = _itarr_ptr->curr = _itarr;
  _itarr_ptr->last_plus_one = ((unsigned char *)_itarr) + (elt_sz) * (num_elts);
  return _itarr_ptr;
}
#else
#define _init_dyneither_ptr(arr_ptr,arr,elt_sz,num_elts) ({ \
  struct _dyneither_ptr *_itarr_ptr = (arr_ptr); \
  void* _itarr = (arr); \
  _itarr_ptr->base = _itarr_ptr->curr = _itarr; \
  _itarr_ptr->last_plus_one = ((char *)_itarr) + (elt_sz) * (num_elts); \
  _itarr_ptr; })
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _untag_dyneither_ptr(arr,elt_sz,num_elts) ((arr).curr)
#else
#ifdef _INLINE_FUNCTIONS
static _INLINE unsigned char *
_untag_dyneither_ptr_fn(struct _dyneither_ptr arr, 
                        unsigned elt_sz,unsigned num_elts,
                        const char *filename, unsigned lineno) {
  struct _dyneither_ptr _arr = (arr);
  unsigned char *_curr = _arr.curr;
  if ((_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one) &&
      _curr != (unsigned char *)0)
    _throw_arraybounds_fn(filename,lineno);
  return _curr;
}
#define _untag_dyneither_ptr(a,s,e) \
  _untag_dyneither_ptr_fn(a,s,e,__FILE__,__LINE__)
#else
#define _untag_dyneither_ptr(arr,elt_sz,num_elts) ({ \
  struct _dyneither_ptr _arr = (arr); \
  unsigned char *_curr = _arr.curr; \
  if ((_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one) &&\
      _curr != (unsigned char *)0) \
    _throw_arraybounds(); \
  _curr; })
#endif
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE unsigned
_get_dyneither_size(struct _dyneither_ptr arr,unsigned elt_sz) {
  struct _dyneither_ptr _get_arr_size_temp = (arr);
  unsigned char *_get_arr_size_curr=_get_arr_size_temp.curr;
  unsigned char *_get_arr_size_last=_get_arr_size_temp.last_plus_one;
  return (_get_arr_size_curr < _get_arr_size_temp.base ||
          _get_arr_size_curr >= _get_arr_size_last) ? 0 :
    ((_get_arr_size_last - _get_arr_size_curr) / (elt_sz));
}
#else
#define _get_dyneither_size(arr,elt_sz) \
  ({struct _dyneither_ptr _get_arr_size_temp = (arr); \
    unsigned char *_get_arr_size_curr=_get_arr_size_temp.curr; \
    unsigned char *_get_arr_size_last=_get_arr_size_temp.last_plus_one; \
    (_get_arr_size_curr < _get_arr_size_temp.base || \
     _get_arr_size_curr >= _get_arr_size_last) ? 0 : \
    ((_get_arr_size_last - _get_arr_size_curr) / (elt_sz));})
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE struct _dyneither_ptr
_dyneither_ptr_plus(struct _dyneither_ptr arr,unsigned elt_sz,int change) {
  struct _dyneither_ptr _ans = (arr);
  _ans.curr += ((int)(elt_sz))*(change);
  return _ans;
}
#else
#define _dyneither_ptr_plus(arr,elt_sz,change) ({ \
  struct _dyneither_ptr _ans = (arr); \
  _ans.curr += ((int)(elt_sz))*(change); \
  _ans; })
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE struct _dyneither_ptr
_dyneither_ptr_inplace_plus(struct _dyneither_ptr *arr_ptr,unsigned elt_sz,
                            int change) {
  struct _dyneither_ptr * _arr_ptr = (arr_ptr);
  _arr_ptr->curr += ((int)(elt_sz))*(change);
  return *_arr_ptr;
}
#else
#define _dyneither_ptr_inplace_plus(arr_ptr,elt_sz,change) ({ \
  struct _dyneither_ptr * _arr_ptr = (arr_ptr); \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  *_arr_ptr; })
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE struct _dyneither_ptr
_dyneither_ptr_inplace_plus_post(struct _dyneither_ptr *arr_ptr,unsigned elt_sz,int change) {
  struct _dyneither_ptr * _arr_ptr = (arr_ptr);
  struct _dyneither_ptr _ans = *_arr_ptr;
  _arr_ptr->curr += ((int)(elt_sz))*(change);
  return _ans;
}
#else
#define _dyneither_ptr_inplace_plus_post(arr_ptr,elt_sz,change) ({ \
  struct _dyneither_ptr * _arr_ptr = (arr_ptr); \
  struct _dyneither_ptr _ans = *_arr_ptr; \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  _ans; })
#endif

/* Decrease the upper bound on a fat pointer by numelts where sz is
   the size of the pointer's type.  Note that this can't be a macro
   if we're to get initializers right. */
static struct 
_dyneither_ptr _dyneither_ptr_decrease_size(struct _dyneither_ptr x,
                                            unsigned int sz,
                                            unsigned int numelts) {
  x.last_plus_one -= sz * numelts; 
  return x; 
}

/* Allocation */

extern void* GC_malloc(int);
extern void* GC_malloc_atomic(int);
extern void* GC_calloc(unsigned,unsigned);
extern void* GC_calloc_atomic(unsigned,unsigned);

#define _CYC_MAX_REGION_CONST 2
#define _CYC_MIN_ALIGNMENT (sizeof(double))

#ifdef CYC_REGION_PROFILE
extern int rgn_total_bytes;
#endif

static _INLINE void *_fast_region_malloc(struct _RegionHandle *r, unsigned orig_s) {  
  if (r > (struct _RegionHandle *)_CYC_MAX_REGION_CONST && r->curr != 0) { 
#ifdef CYC_NOALIGN
    unsigned s =  orig_s;
#else
    unsigned s =  (orig_s + _CYC_MIN_ALIGNMENT - 1) & (~(_CYC_MIN_ALIGNMENT -1)); 
#endif
    char *result; 
    result = r->offset; 
    if (s <= (r->last_plus_one - result)) {
      r->offset = result + s; 
#ifdef CYC_REGION_PROFILE
    r->curr->free_bytes = r->curr->free_bytes - s;
    rgn_total_bytes += s;
#endif
      return result;
    }
  } 
  return _region_malloc(r,orig_s); 
}

/* FIX?  Not sure if we want to pass filename and lineno in here... */
static _INLINE void* _cycalloc(int n) {
  void * ans = (void *)GC_malloc(n);
  if(!ans)
    _throw_badalloc();
  return ans;
}
static _INLINE void* _cycalloc_atomic(int n) {
  void * ans = (void *)GC_malloc_atomic(n);
  if(!ans)
    _throw_badalloc();
  return ans;
}
static _INLINE void* _cyccalloc(unsigned n, unsigned s) {
  void* ans = (void*)GC_calloc(n,s);
  if (!ans)
    _throw_badalloc();
  return ans;
}
static _INLINE void* _cyccalloc_atomic(unsigned n, unsigned s) {
  void* ans = (void*)GC_calloc_atomic(n,s);
  if (!ans)
    _throw_badalloc();
  return ans;
}
#define MAX_MALLOC_SIZE (1 << 28)
static _INLINE unsigned int _check_times(unsigned x, unsigned y) {
  _CYC_U_LONG_LONG_T whole_ans = 
    ((_CYC_U_LONG_LONG_T)x)*((_CYC_U_LONG_LONG_T)y);
  unsigned word_ans = (unsigned)whole_ans;
  if(word_ans < whole_ans || word_ans > MAX_MALLOC_SIZE)
    _throw_badalloc();
  return word_ans;
}

#if defined(CYC_REGION_PROFILE) 
extern void* _profile_GC_malloc(int,const char *file,const char *func,
                                int lineno);
extern void* _profile_GC_malloc_atomic(int,const char *file,
                                       const char *func,int lineno);
extern void* _profile_region_malloc(struct _RegionHandle *, unsigned,
                                    const char *file,
                                    const char *func,
                                    int lineno);
extern void* _profile_region_calloc(struct _RegionHandle *, unsigned,
                                    unsigned,
                                    const char *file,
                                    const char *func,
                                    int lineno);
extern struct _RegionHandle _profile_new_region(const char *rgn_name,
						const char *file,
						const char *func,
                                                int lineno);
extern void _profile_free_region(struct _RegionHandle *,
				 const char *file,
                                 const char *func,
                                 int lineno);
#  if !defined(RUNTIME_CYC)
#define _new_region(n) _profile_new_region(n,__FILE__,__FUNCTION__,__LINE__)
#define _free_region(r) _profile_free_region(r,__FILE__,__FUNCTION__,__LINE__)
#define _region_malloc(rh,n) _profile_region_malloc(rh,n,__FILE__,__FUNCTION__,__LINE__)
#define _region_calloc(rh,n,t) _profile_region_calloc(rh,n,t,__FILE__,__FUNCTION__,__LINE__)
#  endif
#define _cycalloc(n) _profile_GC_malloc(n,__FILE__,__FUNCTION__,__LINE__)
#define _cycalloc_atomic(n) _profile_GC_malloc_atomic(n,__FILE__,__FUNCTION__,__LINE__)
#endif
#endif

# 35 "core.h"
 typedef char*Cyc_Cstring;
typedef char*Cyc_CstringNN;
typedef struct _dyneither_ptr Cyc_string_t;
# 40
typedef struct _dyneither_ptr Cyc_mstring_t;
# 43
typedef struct _dyneither_ptr*Cyc_stringptr_t;
# 47
typedef struct _dyneither_ptr*Cyc_mstringptr_t;
# 50
typedef char*Cyc_Cbuffer_t;
# 52
typedef char*Cyc_CbufferNN_t;
# 54
typedef struct _dyneither_ptr Cyc_buffer_t;
# 56
typedef struct _dyneither_ptr Cyc_mbuffer_t;
# 59
typedef int Cyc_bool;
# 69 "core.h"
void exit(int);
# 26 "cycboot.h"
typedef unsigned long Cyc_size_t;
# 33
typedef unsigned short Cyc_mode_t;
# 38
int Cyc_open(const char*,int,struct _dyneither_ptr);struct Cyc___cycFILE;
# 49
typedef struct Cyc___cycFILE Cyc_FILE;
# 51
extern struct Cyc___cycFILE*Cyc_stdout;
# 53
extern struct Cyc___cycFILE*Cyc_stderr;struct Cyc_String_pa_PrintArg_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Int_pa_PrintArg_struct{int tag;unsigned long f1;};struct Cyc_Double_pa_PrintArg_struct{int tag;double f1;};struct Cyc_LongDouble_pa_PrintArg_struct{int tag;long double f1;};struct Cyc_ShortPtr_pa_PrintArg_struct{int tag;short*f1;};struct Cyc_IntPtr_pa_PrintArg_struct{int tag;unsigned long*f1;};
# 68
typedef void*Cyc_parg_t;
# 73
struct _dyneither_ptr Cyc_aprintf(struct _dyneither_ptr,struct _dyneither_ptr);
# 79
int Cyc_fclose(struct Cyc___cycFILE*);
# 88
int Cyc_fflush(struct Cyc___cycFILE*);
# 98
struct Cyc___cycFILE*Cyc_fopen(const char*,const char*);
# 100
int Cyc_fprintf(struct Cyc___cycFILE*,struct _dyneither_ptr,struct _dyneither_ptr);
# 104
int Cyc_fputc(int,struct Cyc___cycFILE*);
# 106
int Cyc_fputs(const char*,struct Cyc___cycFILE*);struct Cyc_ShortPtr_sa_ScanfArg_struct{int tag;short*f1;};struct Cyc_UShortPtr_sa_ScanfArg_struct{int tag;unsigned short*f1;};struct Cyc_IntPtr_sa_ScanfArg_struct{int tag;int*f1;};struct Cyc_UIntPtr_sa_ScanfArg_struct{int tag;unsigned int*f1;};struct Cyc_StringPtr_sa_ScanfArg_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_DoublePtr_sa_ScanfArg_struct{int tag;double*f1;};struct Cyc_FloatPtr_sa_ScanfArg_struct{int tag;float*f1;};struct Cyc_CharPtr_sa_ScanfArg_struct{int tag;struct _dyneither_ptr f1;};
# 127
typedef void*Cyc_sarg_t;
# 224 "cycboot.h"
int Cyc_vfprintf(struct Cyc___cycFILE*,struct _dyneither_ptr,struct _dyneither_ptr);extern char Cyc_FileCloseError[15];struct Cyc_FileCloseError_exn_struct{char*tag;};extern char Cyc_FileOpenError[14];struct Cyc_FileOpenError_exn_struct{char*tag;struct _dyneither_ptr f1;};
# 300 "cycboot.h"
int isspace(int);
# 310
int toupper(int);
# 318
int system(const char*);
# 321
int mkdir(const char*pathname,unsigned short mode);
# 324
int close(int);
int chdir(const char*);
struct _dyneither_ptr Cyc_getcwd(struct _dyneither_ptr buf,unsigned long size);
# 79 "core.h"
typedef unsigned int Cyc_Core_sizeof_t;struct Cyc_Core_Opt{void*v;};
# 83
typedef struct Cyc_Core_Opt*Cyc_Core_opt_t;extern char Cyc_Core_Invalid_argument[17];struct Cyc_Core_Invalid_argument_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Failure[8];struct Cyc_Core_Failure_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Impossible[11];struct Cyc_Core_Impossible_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Not_found[10];struct Cyc_Core_Not_found_exn_struct{char*tag;};extern char Cyc_Core_Unreachable[12];struct Cyc_Core_Unreachable_exn_struct{char*tag;struct _dyneither_ptr f1;};
# 167 "core.h"
extern struct _RegionHandle*Cyc_Core_heap_region;
# 170
extern struct _RegionHandle*Cyc_Core_unique_region;struct Cyc_Core_DynamicRegion;
# 205
typedef struct Cyc_Core_DynamicRegion*Cyc_Core_region_key_t;
# 211
typedef struct Cyc_Core_DynamicRegion*Cyc_Core_uregion_key_t;
# 216
typedef struct Cyc_Core_DynamicRegion*Cyc_Core_rcregion_key_t;struct Cyc_Core_NewDynamicRegion{struct Cyc_Core_DynamicRegion*key;};
# 288 "core.h"
struct _dyneither_ptr Cstring_to_string(char*);
# 295
typedef void*Cyc_Core___cyclone_internal_array_t;
typedef unsigned int Cyc_Core___cyclone_internal_singleton;
# 299
inline static void* arrcast(struct _dyneither_ptr dyn,unsigned int bd,unsigned int sz){
# 304
if(bd >> 20  || sz >> 12)
return 0;{
unsigned char*ptrbd=dyn.curr + bd * sz;
if(((ptrbd < dyn.curr  || dyn.curr == 0) || dyn.curr < dyn.base) || ptrbd > dyn.last_plus_one)
# 311
return 0;
return dyn.curr;};}
# 317
static unsigned int arr_prevsize(struct _dyneither_ptr arr,unsigned int elt_sz){
unsigned char*_get_arr_size_curr=arr.curr;
unsigned char*_get_arr_size_base=arr.base;
return
(_get_arr_size_curr < _get_arr_size_base  || _get_arr_size_curr >= arr.last_plus_one)?0:(_get_arr_size_curr - _get_arr_size_base)/ elt_sz;}extern char Cyc_Lexing_Error[6];struct Cyc_Lexing_Error_exn_struct{char*tag;struct _dyneither_ptr f1;};struct Cyc_Lexing_lexbuf{void(*refill_buff)(struct Cyc_Lexing_lexbuf*);void*refill_state;struct _dyneither_ptr lex_buffer;int lex_buffer_len;int lex_abs_pos;int lex_start_pos;int lex_curr_pos;int lex_last_pos;int lex_last_action;int lex_eof_reached;};
# 57 "lexing.h"
typedef struct Cyc_Lexing_lexbuf*Cyc_Lexing_Lexbuf;struct Cyc_Lexing_function_lexbuf_state{int(*read_fun)(struct _dyneither_ptr,int,void*);void*read_fun_state;};
# 64
typedef struct Cyc_Lexing_function_lexbuf_state*Cyc_Lexing_Function_lexbuf_state;struct Cyc_Lexing_lex_tables{struct _dyneither_ptr lex_base;struct _dyneither_ptr lex_backtrk;struct _dyneither_ptr lex_default;struct _dyneither_ptr lex_trans;struct _dyneither_ptr lex_check;};
# 74
typedef struct Cyc_Lexing_lex_tables*Cyc_Lexing_LexTables;
# 80
struct Cyc_Lexing_lexbuf*Cyc_Lexing_from_file(struct Cyc___cycFILE*);
# 84
struct _dyneither_ptr Cyc_Lexing_lexeme(struct Cyc_Lexing_lexbuf*);
char Cyc_Lexing_lexeme_char(struct Cyc_Lexing_lexbuf*,int);
int Cyc_Lexing_lexeme_start(struct Cyc_Lexing_lexbuf*);
int Cyc_Lexing_lexeme_end(struct Cyc_Lexing_lexbuf*);struct Cyc_List_List{void*hd;struct Cyc_List_List*tl;};
# 39 "list.h"
typedef struct Cyc_List_List*Cyc_List_list_t;
# 49 "list.h"
typedef struct Cyc_List_List*Cyc_List_List_t;
# 54
struct Cyc_List_List*Cyc_List_list(struct _dyneither_ptr);
# 76
struct Cyc_List_List*Cyc_List_map(void*(*f)(void*),struct Cyc_List_List*x);extern char Cyc_List_List_mismatch[14];struct Cyc_List_List_mismatch_exn_struct{char*tag;};
# 172
struct Cyc_List_List*Cyc_List_rev(struct Cyc_List_List*x);
# 178
struct Cyc_List_List*Cyc_List_imp_rev(struct Cyc_List_List*x);
# 184
struct Cyc_List_List*Cyc_List_append(struct Cyc_List_List*x,struct Cyc_List_List*y);extern char Cyc_List_Nth[4];struct Cyc_List_Nth_exn_struct{char*tag;};
# 322
int Cyc_List_mem(int(*compare)(void*,void*),struct Cyc_List_List*l,void*x);struct Cyc_Iter_Iter{void*env;int(*next)(void*env,void*dest);};
# 34 "iter.h"
typedef struct Cyc_Iter_Iter Cyc_Iter_iter_t;
# 37
int Cyc_Iter_next(struct Cyc_Iter_Iter,void*);struct Cyc_Set_Set;
# 40 "set.h"
typedef struct Cyc_Set_Set*Cyc_Set_set_t;
# 51 "set.h"
struct Cyc_Set_Set*Cyc_Set_empty(int(*cmp)(void*,void*));
# 63
struct Cyc_Set_Set*Cyc_Set_insert(struct Cyc_Set_Set*s,void*elt);
# 75
struct Cyc_Set_Set*Cyc_Set_union_two(struct Cyc_Set_Set*s1,struct Cyc_Set_Set*s2);
# 82
struct Cyc_Set_Set*Cyc_Set_diff(struct Cyc_Set_Set*s1,struct Cyc_Set_Set*s2);
# 85
struct Cyc_Set_Set*Cyc_Set_delete(struct Cyc_Set_Set*s,void*elt);
# 94
int Cyc_Set_cardinality(struct Cyc_Set_Set*s);
# 100
int Cyc_Set_member(struct Cyc_Set_Set*s,void*elt);extern char Cyc_Set_Absent[7];struct Cyc_Set_Absent_exn_struct{char*tag;};
# 141
struct Cyc_Iter_Iter Cyc_Set_make_iter(struct _RegionHandle*rgn,struct Cyc_Set_Set*s);
# 38 "string.h"
unsigned long Cyc_strlen(struct _dyneither_ptr s);
# 50 "string.h"
int Cyc_strptrcmp(struct _dyneither_ptr*s1,struct _dyneither_ptr*s2);
# 62
struct _dyneither_ptr Cyc_strconcat(struct _dyneither_ptr,struct _dyneither_ptr);
# 64
struct _dyneither_ptr Cyc_strconcat_l(struct Cyc_List_List*);
# 66
struct _dyneither_ptr Cyc_str_sepstr(struct Cyc_List_List*,struct _dyneither_ptr);
# 103 "string.h"
struct _dyneither_ptr Cyc_strdup(struct _dyneither_ptr src);
# 108
struct _dyneither_ptr Cyc_substring(struct _dyneither_ptr,int ofs,unsigned long n);struct Cyc_Hashtable_Table;
# 35 "hashtable.h"
typedef struct Cyc_Hashtable_Table*Cyc_Hashtable_table_t;
# 39
struct Cyc_Hashtable_Table*Cyc_Hashtable_create(int sz,int(*cmp)(void*,void*),int(*hash)(void*));
# 50
void Cyc_Hashtable_insert(struct Cyc_Hashtable_Table*t,void*key,void*val);
# 52
void*Cyc_Hashtable_lookup(struct Cyc_Hashtable_Table*t,void*key);
# 78
int Cyc_Hashtable_hash_stringptr(struct _dyneither_ptr*p);
# 30 "filename.h"
struct _dyneither_ptr Cyc_Filename_concat(struct _dyneither_ptr,struct _dyneither_ptr);
# 34
struct _dyneither_ptr Cyc_Filename_chop_extension(struct _dyneither_ptr);
# 40
struct _dyneither_ptr Cyc_Filename_dirname(struct _dyneither_ptr);
# 43
struct _dyneither_ptr Cyc_Filename_basename(struct _dyneither_ptr);extern char Cyc_Arg_Bad[4];struct Cyc_Arg_Bad_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Arg_Error[6];struct Cyc_Arg_Error_exn_struct{char*tag;};struct Cyc_Arg_Unit_spec_Arg_Spec_struct{int tag;void(*f1)();};struct Cyc_Arg_Flag_spec_Arg_Spec_struct{int tag;void(*f1)(struct _dyneither_ptr);};struct Cyc_Arg_FlagString_spec_Arg_Spec_struct{int tag;void(*f1)(struct _dyneither_ptr,struct _dyneither_ptr);};struct Cyc_Arg_Set_spec_Arg_Spec_struct{int tag;int*f1;};struct Cyc_Arg_Clear_spec_Arg_Spec_struct{int tag;int*f1;};struct Cyc_Arg_String_spec_Arg_Spec_struct{int tag;void(*f1)(struct _dyneither_ptr);};struct Cyc_Arg_Int_spec_Arg_Spec_struct{int tag;void(*f1)(int);};struct Cyc_Arg_Rest_spec_Arg_Spec_struct{int tag;void(*f1)(struct _dyneither_ptr);};
# 55 "arg.h"
typedef void*Cyc_Arg_spec_t;
# 57
typedef struct Cyc_List_List*Cyc_Arg_speclist_t;
# 66
void Cyc_Arg_usage(struct Cyc_List_List*,struct _dyneither_ptr);
# 71
void Cyc_Arg_parse(struct Cyc_List_List*specs,void(*anonfun)(struct _dyneither_ptr),int(*anonflagfun)(struct _dyneither_ptr),struct _dyneither_ptr errmsg,struct _dyneither_ptr args);struct Cyc_Buffer_t;
# 46 "buffer.h"
typedef struct Cyc_Buffer_t*Cyc_Buffer_T;
# 49
struct Cyc_Buffer_t*Cyc_Buffer_create(unsigned int n);
# 57
struct _dyneither_ptr Cyc_Buffer_contents(struct Cyc_Buffer_t*);
# 70
void Cyc_Buffer_add_char(struct Cyc_Buffer_t*,char);
# 81 "buffer.h"
void Cyc_Buffer_add_string(struct Cyc_Buffer_t*,struct _dyneither_ptr);struct Cyc_PP_Ppstate;
# 41 "pp.h"
typedef struct Cyc_PP_Ppstate*Cyc_PP_ppstate_t;struct Cyc_PP_Out;
# 43
typedef struct Cyc_PP_Out*Cyc_PP_out_t;struct Cyc_PP_Doc;
# 45
typedef struct Cyc_PP_Doc*Cyc_PP_doc_t;
# 28 "position.h"
void Cyc_Position_reset_position(struct _dyneither_ptr);
# 33
typedef unsigned int Cyc_Position_seg_t;struct Cyc_Position_Error;
# 42
typedef struct Cyc_Position_Error*Cyc_Position_error_t;struct Cyc_Relations_Reln;
# 69 "absyn.h"
typedef struct Cyc_Relations_Reln*Cyc_Relations_reln_t;
typedef struct Cyc_List_List*Cyc_Relations_relns_t;
# 74
typedef void*Cyc_Tcpat_decision_opt_t;
# 82
typedef struct _dyneither_ptr*Cyc_Absyn_field_name_t;
typedef struct _dyneither_ptr*Cyc_Absyn_var_t;
typedef struct _dyneither_ptr*Cyc_Absyn_tvarname_t;
typedef struct _dyneither_ptr*Cyc_Absyn_var_opt_t;struct _union_Nmspace_Rel_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Abs_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_C_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Loc_n{int tag;int val;};union Cyc_Absyn_Nmspace{struct _union_Nmspace_Rel_n Rel_n;struct _union_Nmspace_Abs_n Abs_n;struct _union_Nmspace_C_n C_n;struct _union_Nmspace_Loc_n Loc_n;};
# 94
typedef union Cyc_Absyn_Nmspace Cyc_Absyn_nmspace_t;
union Cyc_Absyn_Nmspace Cyc_Absyn_Loc_n;
union Cyc_Absyn_Nmspace Cyc_Absyn_Rel_n(struct Cyc_List_List*);
# 98
union Cyc_Absyn_Nmspace Cyc_Absyn_Abs_n(struct Cyc_List_List*ns,int C_scope);struct _tuple0{union Cyc_Absyn_Nmspace f1;struct _dyneither_ptr*f2;};
# 101
typedef struct _tuple0*Cyc_Absyn_qvar_t;typedef struct _tuple0*Cyc_Absyn_qvar_opt_t;
typedef struct _tuple0*Cyc_Absyn_typedef_name_t;
typedef struct _tuple0*Cyc_Absyn_typedef_name_opt_t;
# 106
typedef enum Cyc_Absyn_Scope Cyc_Absyn_scope_t;
typedef struct Cyc_Absyn_Tqual Cyc_Absyn_tqual_t;
typedef enum Cyc_Absyn_Size_of Cyc_Absyn_size_of_t;
typedef struct Cyc_Absyn_Kind*Cyc_Absyn_kind_t;
typedef void*Cyc_Absyn_kindbound_t;
typedef struct Cyc_Absyn_Tvar*Cyc_Absyn_tvar_t;
typedef enum Cyc_Absyn_Sign Cyc_Absyn_sign_t;
typedef enum Cyc_Absyn_AggrKind Cyc_Absyn_aggr_kind_t;
typedef void*Cyc_Absyn_bounds_t;
typedef struct Cyc_Absyn_PtrAtts Cyc_Absyn_ptr_atts_t;
typedef struct Cyc_Absyn_PtrInfo Cyc_Absyn_ptr_info_t;
typedef struct Cyc_Absyn_VarargInfo Cyc_Absyn_vararg_info_t;
typedef struct Cyc_Absyn_FnInfo Cyc_Absyn_fn_info_t;
typedef struct Cyc_Absyn_DatatypeInfo Cyc_Absyn_datatype_info_t;
typedef struct Cyc_Absyn_DatatypeFieldInfo Cyc_Absyn_datatype_field_info_t;
typedef struct Cyc_Absyn_AggrInfo Cyc_Absyn_aggr_info_t;
typedef struct Cyc_Absyn_ArrayInfo Cyc_Absyn_array_info_t;
typedef void*Cyc_Absyn_type_t;typedef void*Cyc_Absyn_rgntype_t;typedef void*Cyc_Absyn_type_opt_t;
typedef union Cyc_Absyn_Cnst Cyc_Absyn_cnst_t;
typedef enum Cyc_Absyn_Primop Cyc_Absyn_primop_t;
typedef enum Cyc_Absyn_Incrementor Cyc_Absyn_incrementor_t;
typedef struct Cyc_Absyn_VarargCallInfo Cyc_Absyn_vararg_call_info_t;
typedef void*Cyc_Absyn_raw_exp_t;
typedef struct Cyc_Absyn_Exp*Cyc_Absyn_exp_t;typedef struct Cyc_Absyn_Exp*Cyc_Absyn_exp_opt_t;
typedef void*Cyc_Absyn_raw_stmt_t;
typedef struct Cyc_Absyn_Stmt*Cyc_Absyn_stmt_t;typedef struct Cyc_Absyn_Stmt*Cyc_Absyn_stmt_opt_t;
typedef void*Cyc_Absyn_raw_pat_t;
typedef struct Cyc_Absyn_Pat*Cyc_Absyn_pat_t;
typedef void*Cyc_Absyn_binding_t;
typedef struct Cyc_Absyn_Switch_clause*Cyc_Absyn_switch_clause_t;
typedef struct Cyc_Absyn_Fndecl*Cyc_Absyn_fndecl_t;
typedef struct Cyc_Absyn_Aggrdecl*Cyc_Absyn_aggrdecl_t;
typedef struct Cyc_Absyn_Datatypefield*Cyc_Absyn_datatypefield_t;
typedef struct Cyc_Absyn_Datatypedecl*Cyc_Absyn_datatypedecl_t;
typedef struct Cyc_Absyn_Typedefdecl*Cyc_Absyn_typedefdecl_t;
typedef struct Cyc_Absyn_Enumfield*Cyc_Absyn_enumfield_t;
typedef struct Cyc_Absyn_Enumdecl*Cyc_Absyn_enumdecl_t;
typedef struct Cyc_Absyn_Vardecl*Cyc_Absyn_vardecl_t;typedef struct Cyc_Absyn_Vardecl*Cyc_Absyn_vardecl_opt_t;
typedef void*Cyc_Absyn_raw_decl_t;
typedef struct Cyc_Absyn_Decl*Cyc_Absyn_decl_t;
typedef void*Cyc_Absyn_designator_t;
typedef void*Cyc_Absyn_absyn_annot_t;
typedef void*Cyc_Absyn_attribute_t;
typedef struct Cyc_List_List*Cyc_Absyn_attributes_t;
typedef struct Cyc_Absyn_Aggrfield*Cyc_Absyn_aggrfield_t;
typedef void*Cyc_Absyn_offsetof_field_t;
typedef struct Cyc_Absyn_MallocInfo Cyc_Absyn_malloc_info_t;
typedef enum Cyc_Absyn_Coercion Cyc_Absyn_coercion_t;
typedef struct Cyc_Absyn_PtrLoc*Cyc_Absyn_ptrloc_t;
# 157
enum Cyc_Absyn_Scope{Cyc_Absyn_Static  = 0,Cyc_Absyn_Abstract  = 1,Cyc_Absyn_Public  = 2,Cyc_Absyn_Extern  = 3,Cyc_Absyn_ExternC  = 4,Cyc_Absyn_Register  = 5};struct Cyc_Absyn_Tqual{int print_const: 1;int q_volatile: 1;int q_restrict: 1;int real_const: 1;unsigned int loc;};
# 178
enum Cyc_Absyn_Size_of{Cyc_Absyn_Char_sz  = 0,Cyc_Absyn_Short_sz  = 1,Cyc_Absyn_Int_sz  = 2,Cyc_Absyn_Long_sz  = 3,Cyc_Absyn_LongLong_sz  = 4};
# 183
enum Cyc_Absyn_AliasQual{Cyc_Absyn_Aliasable  = 0,Cyc_Absyn_Unique  = 1,Cyc_Absyn_Top  = 2};
# 189
enum Cyc_Absyn_KindQual{Cyc_Absyn_AnyKind  = 0,Cyc_Absyn_MemKind  = 1,Cyc_Absyn_BoxKind  = 2,Cyc_Absyn_RgnKind  = 3,Cyc_Absyn_EffKind  = 4,Cyc_Absyn_IntKind  = 5};struct Cyc_Absyn_Kind{enum Cyc_Absyn_KindQual kind;enum Cyc_Absyn_AliasQual aliasqual;};
# 209
enum Cyc_Absyn_Sign{Cyc_Absyn_Signed  = 0,Cyc_Absyn_Unsigned  = 1,Cyc_Absyn_None  = 2};
# 211
enum Cyc_Absyn_AggrKind{Cyc_Absyn_StructA  = 0,Cyc_Absyn_UnionA  = 1};struct _union_Constraint_Eq_constr{int tag;void*val;};struct _union_Constraint_Forward_constr{int tag;union Cyc_Absyn_Constraint*val;};struct _union_Constraint_No_constr{int tag;int val;};union Cyc_Absyn_Constraint{struct _union_Constraint_Eq_constr Eq_constr;struct _union_Constraint_Forward_constr Forward_constr;struct _union_Constraint_No_constr No_constr;};
# 220
typedef union Cyc_Absyn_Constraint*Cyc_Absyn_conref_t;struct Cyc_Absyn_Eq_kb_Absyn_KindBound_struct{int tag;struct Cyc_Absyn_Kind*f1;};struct Cyc_Absyn_Unknown_kb_Absyn_KindBound_struct{int tag;struct Cyc_Core_Opt*f1;};struct Cyc_Absyn_Less_kb_Absyn_KindBound_struct{int tag;struct Cyc_Core_Opt*f1;struct Cyc_Absyn_Kind*f2;};struct Cyc_Absyn_Tvar{struct _dyneither_ptr*name;int identity;void*kind;};struct Cyc_Absyn_DynEither_b_Absyn_Bounds_struct{int tag;};struct Cyc_Absyn_Upper_b_Absyn_Bounds_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_PtrLoc{unsigned int ptr_loc;unsigned int rgn_loc;unsigned int zt_loc;};struct Cyc_Absyn_PtrAtts{void*rgn;union Cyc_Absyn_Constraint*nullable;union Cyc_Absyn_Constraint*bounds;union Cyc_Absyn_Constraint*zero_term;struct Cyc_Absyn_PtrLoc*ptrloc;};struct Cyc_Absyn_PtrInfo{void*elt_typ;struct Cyc_Absyn_Tqual elt_tq;struct Cyc_Absyn_PtrAtts ptr_atts;};struct Cyc_Absyn_VarargInfo{struct _dyneither_ptr*name;struct Cyc_Absyn_Tqual tq;void*type;int inject;};struct Cyc_Absyn_FnInfo{struct Cyc_List_List*tvars;void*effect;struct Cyc_Absyn_Tqual ret_tqual;void*ret_typ;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;struct Cyc_List_List*requires_relns;struct Cyc_Absyn_Exp*ensures_clause;struct Cyc_List_List*ensures_relns;};struct Cyc_Absyn_UnknownDatatypeInfo{struct _tuple0*name;int is_extensible;};struct _union_DatatypeInfoU_UnknownDatatype{int tag;struct Cyc_Absyn_UnknownDatatypeInfo val;};struct _union_DatatypeInfoU_KnownDatatype{int tag;struct Cyc_Absyn_Datatypedecl**val;};union Cyc_Absyn_DatatypeInfoU{struct _union_DatatypeInfoU_UnknownDatatype UnknownDatatype;struct _union_DatatypeInfoU_KnownDatatype KnownDatatype;};struct Cyc_Absyn_DatatypeInfo{union Cyc_Absyn_DatatypeInfoU datatype_info;struct Cyc_List_List*targs;};struct Cyc_Absyn_UnknownDatatypeFieldInfo{struct _tuple0*datatype_name;struct _tuple0*field_name;int is_extensible;};struct _union_DatatypeFieldInfoU_UnknownDatatypefield{int tag;struct Cyc_Absyn_UnknownDatatypeFieldInfo val;};struct _tuple1{struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;};struct _union_DatatypeFieldInfoU_KnownDatatypefield{int tag;struct _tuple1 val;};union Cyc_Absyn_DatatypeFieldInfoU{struct _union_DatatypeFieldInfoU_UnknownDatatypefield UnknownDatatypefield;struct _union_DatatypeFieldInfoU_KnownDatatypefield KnownDatatypefield;};struct Cyc_Absyn_DatatypeFieldInfo{union Cyc_Absyn_DatatypeFieldInfoU field_info;struct Cyc_List_List*targs;};struct _tuple2{enum Cyc_Absyn_AggrKind f1;struct _tuple0*f2;struct Cyc_Core_Opt*f3;};struct _union_AggrInfoU_UnknownAggr{int tag;struct _tuple2 val;};struct _union_AggrInfoU_KnownAggr{int tag;struct Cyc_Absyn_Aggrdecl**val;};union Cyc_Absyn_AggrInfoU{struct _union_AggrInfoU_UnknownAggr UnknownAggr;struct _union_AggrInfoU_KnownAggr KnownAggr;};struct Cyc_Absyn_AggrInfo{union Cyc_Absyn_AggrInfoU aggr_info;struct Cyc_List_List*targs;};struct Cyc_Absyn_ArrayInfo{void*elt_type;struct Cyc_Absyn_Tqual tq;struct Cyc_Absyn_Exp*num_elts;union Cyc_Absyn_Constraint*zero_term;unsigned int zt_loc;};struct Cyc_Absyn_Aggr_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Absyn_Enum_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;};struct Cyc_Absyn_Datatype_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;};
# 366
typedef void*Cyc_Absyn_raw_type_decl_t;struct Cyc_Absyn_TypeDecl{void*r;unsigned int loc;};
# 371
typedef struct Cyc_Absyn_TypeDecl*Cyc_Absyn_type_decl_t;struct Cyc_Absyn_VoidType_Absyn_Type_struct{int tag;};struct Cyc_Absyn_Evar_Absyn_Type_struct{int tag;struct Cyc_Core_Opt*f1;void*f2;int f3;struct Cyc_Core_Opt*f4;};struct Cyc_Absyn_VarType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Tvar*f1;};struct Cyc_Absyn_DatatypeType_Absyn_Type_struct{int tag;struct Cyc_Absyn_DatatypeInfo f1;};struct Cyc_Absyn_DatatypeFieldType_Absyn_Type_struct{int tag;struct Cyc_Absyn_DatatypeFieldInfo f1;};struct Cyc_Absyn_PointerType_Absyn_Type_struct{int tag;struct Cyc_Absyn_PtrInfo f1;};struct Cyc_Absyn_IntType_Absyn_Type_struct{int tag;enum Cyc_Absyn_Sign f1;enum Cyc_Absyn_Size_of f2;};struct Cyc_Absyn_FloatType_Absyn_Type_struct{int tag;int f1;};struct Cyc_Absyn_ArrayType_Absyn_Type_struct{int tag;struct Cyc_Absyn_ArrayInfo f1;};struct Cyc_Absyn_FnType_Absyn_Type_struct{int tag;struct Cyc_Absyn_FnInfo f1;};struct Cyc_Absyn_TupleType_Absyn_Type_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_AggrType_Absyn_Type_struct{int tag;struct Cyc_Absyn_AggrInfo f1;};struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct{int tag;enum Cyc_Absyn_AggrKind f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_EnumType_Absyn_Type_struct{int tag;struct _tuple0*f1;struct Cyc_Absyn_Enumdecl*f2;};struct Cyc_Absyn_AnonEnumType_Absyn_Type_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_RgnHandleType_Absyn_Type_struct{int tag;void*f1;};struct Cyc_Absyn_DynRgnType_Absyn_Type_struct{int tag;void*f1;void*f2;};struct Cyc_Absyn_TypedefType_Absyn_Type_struct{int tag;struct _tuple0*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_Typedefdecl*f3;void*f4;};struct Cyc_Absyn_ValueofType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_TagType_Absyn_Type_struct{int tag;void*f1;};struct Cyc_Absyn_HeapRgn_Absyn_Type_struct{int tag;};struct Cyc_Absyn_UniqueRgn_Absyn_Type_struct{int tag;};struct Cyc_Absyn_RefCntRgn_Absyn_Type_struct{int tag;};struct Cyc_Absyn_AccessEff_Absyn_Type_struct{int tag;void*f1;};struct Cyc_Absyn_JoinEff_Absyn_Type_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_RgnsEff_Absyn_Type_struct{int tag;void*f1;};struct Cyc_Absyn_TypeDeclType_Absyn_Type_struct{int tag;struct Cyc_Absyn_TypeDecl*f1;void**f2;};struct Cyc_Absyn_TypeofType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_BuiltinType_Absyn_Type_struct{int tag;struct _dyneither_ptr f1;struct Cyc_Absyn_Kind*f2;};struct Cyc_Absyn_NoTypes_Absyn_Funcparams_struct{int tag;struct Cyc_List_List*f1;unsigned int f2;};struct Cyc_Absyn_WithTypes_Absyn_Funcparams_struct{int tag;struct Cyc_List_List*f1;int f2;struct Cyc_Absyn_VarargInfo*f3;void*f4;struct Cyc_List_List*f5;struct Cyc_Absyn_Exp*f6;struct Cyc_Absyn_Exp*f7;};
# 442 "absyn.h"
typedef void*Cyc_Absyn_funcparams_t;
# 445
enum Cyc_Absyn_Format_Type{Cyc_Absyn_Printf_ft  = 0,Cyc_Absyn_Scanf_ft  = 1};struct Cyc_Absyn_Regparm_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Stdcall_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Cdecl_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Fastcall_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Noreturn_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Const_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Aligned_att_Absyn_Attribute_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Packed_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Section_att_Absyn_Attribute_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Nocommon_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Shared_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Unused_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Weak_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Dllimport_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Dllexport_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_No_instrument_function_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Constructor_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Destructor_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_No_check_memory_usage_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Format_att_Absyn_Attribute_struct{int tag;enum Cyc_Absyn_Format_Type f1;int f2;int f3;};struct Cyc_Absyn_Initializes_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Noliveunique_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Noconsume_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Pure_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Mode_att_Absyn_Attribute_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Alias_att_Absyn_Attribute_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Always_inline_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Carray_mod_Absyn_Type_modifier_struct{int tag;union Cyc_Absyn_Constraint*f1;unsigned int f2;};struct Cyc_Absyn_ConstArray_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_Absyn_Exp*f1;union Cyc_Absyn_Constraint*f2;unsigned int f3;};struct Cyc_Absyn_Pointer_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_Absyn_PtrAtts f1;struct Cyc_Absyn_Tqual f2;};struct Cyc_Absyn_Function_mod_Absyn_Type_modifier_struct{int tag;void*f1;};struct Cyc_Absyn_TypeParams_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_List_List*f1;unsigned int f2;int f3;};struct Cyc_Absyn_Attributes_mod_Absyn_Type_modifier_struct{int tag;unsigned int f1;struct Cyc_List_List*f2;};
# 509
typedef void*Cyc_Absyn_type_modifier_t;struct _union_Cnst_Null_c{int tag;int val;};struct _tuple3{enum Cyc_Absyn_Sign f1;char f2;};struct _union_Cnst_Char_c{int tag;struct _tuple3 val;};struct _union_Cnst_Wchar_c{int tag;struct _dyneither_ptr val;};struct _tuple4{enum Cyc_Absyn_Sign f1;short f2;};struct _union_Cnst_Short_c{int tag;struct _tuple4 val;};struct _tuple5{enum Cyc_Absyn_Sign f1;int f2;};struct _union_Cnst_Int_c{int tag;struct _tuple5 val;};struct _tuple6{enum Cyc_Absyn_Sign f1;long long f2;};struct _union_Cnst_LongLong_c{int tag;struct _tuple6 val;};struct _tuple7{struct _dyneither_ptr f1;int f2;};struct _union_Cnst_Float_c{int tag;struct _tuple7 val;};struct _union_Cnst_String_c{int tag;struct _dyneither_ptr val;};struct _union_Cnst_Wstring_c{int tag;struct _dyneither_ptr val;};union Cyc_Absyn_Cnst{struct _union_Cnst_Null_c Null_c;struct _union_Cnst_Char_c Char_c;struct _union_Cnst_Wchar_c Wchar_c;struct _union_Cnst_Short_c Short_c;struct _union_Cnst_Int_c Int_c;struct _union_Cnst_LongLong_c LongLong_c;struct _union_Cnst_Float_c Float_c;struct _union_Cnst_String_c String_c;struct _union_Cnst_Wstring_c Wstring_c;};
# 535
enum Cyc_Absyn_Primop{Cyc_Absyn_Plus  = 0,Cyc_Absyn_Times  = 1,Cyc_Absyn_Minus  = 2,Cyc_Absyn_Div  = 3,Cyc_Absyn_Mod  = 4,Cyc_Absyn_Eq  = 5,Cyc_Absyn_Neq  = 6,Cyc_Absyn_Gt  = 7,Cyc_Absyn_Lt  = 8,Cyc_Absyn_Gte  = 9,Cyc_Absyn_Lte  = 10,Cyc_Absyn_Not  = 11,Cyc_Absyn_Bitnot  = 12,Cyc_Absyn_Bitand  = 13,Cyc_Absyn_Bitor  = 14,Cyc_Absyn_Bitxor  = 15,Cyc_Absyn_Bitlshift  = 16,Cyc_Absyn_Bitlrshift  = 17,Cyc_Absyn_Bitarshift  = 18,Cyc_Absyn_Numelts  = 19};
# 542
enum Cyc_Absyn_Incrementor{Cyc_Absyn_PreInc  = 0,Cyc_Absyn_PostInc  = 1,Cyc_Absyn_PreDec  = 2,Cyc_Absyn_PostDec  = 3};struct Cyc_Absyn_VarargCallInfo{int num_varargs;struct Cyc_List_List*injectors;struct Cyc_Absyn_VarargInfo*vai;};struct Cyc_Absyn_StructField_Absyn_OffsetofField_struct{int tag;struct _dyneither_ptr*f1;};struct Cyc_Absyn_TupleIndex_Absyn_OffsetofField_struct{int tag;unsigned int f1;};
# 560
enum Cyc_Absyn_Coercion{Cyc_Absyn_Unknown_coercion  = 0,Cyc_Absyn_No_coercion  = 1,Cyc_Absyn_Null_to_NonNull  = 2,Cyc_Absyn_Other_coercion  = 3};struct Cyc_Absyn_MallocInfo{int is_calloc;struct Cyc_Absyn_Exp*rgn;void**elt_type;struct Cyc_Absyn_Exp*num_elts;int fat_result;int inline_call;};struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct{int tag;union Cyc_Absyn_Cnst f1;};struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct{int tag;enum Cyc_Absyn_Primop f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Increment_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;enum Cyc_Absyn_Incrementor f2;};struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_VarargCallInfo*f3;int f4;};struct Cyc_Absyn_Throw_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;int f2;};struct Cyc_Absyn_NoInstantiate_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Instantiate_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_Absyn_Exp*f2;int f3;enum Cyc_Absyn_Coercion f4;};struct Cyc_Absyn_Address_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_New_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Sizeoftyp_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Sizeofexp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Offsetof_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;int f3;int f4;};struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;int f3;int f4;};struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Tuple_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;};struct _tuple8{struct _dyneither_ptr*f1;struct Cyc_Absyn_Tqual f2;void*f3;};struct Cyc_Absyn_CompoundLit_e_Absyn_Raw_exp_struct{int tag;struct _tuple8*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Array_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Comprehension_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;int f4;};struct Cyc_Absyn_ComprehensionNoinit_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;void*f2;int f3;};struct Cyc_Absyn_Aggregate_e_Absyn_Raw_exp_struct{int tag;struct _tuple0*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;struct Cyc_Absyn_Aggrdecl*f4;};struct Cyc_Absyn_AnonStruct_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Datatype_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Datatypedecl*f2;struct Cyc_Absyn_Datatypefield*f3;};struct Cyc_Absyn_Enum_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_AnonEnum_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_MallocInfo f1;};struct Cyc_Absyn_Swap_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Core_Opt*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_StmtExp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Tagcheck_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;};struct Cyc_Absyn_Valueof_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Asm_e_Absyn_Raw_exp_struct{int tag;int f1;struct _dyneither_ptr f2;};struct Cyc_Absyn_Exp{void*topt;void*r;unsigned int loc;void*annot;};struct Cyc_Absyn_Skip_s_Absyn_Raw_stmt_struct{int tag;};struct Cyc_Absyn_Exp_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Return_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;struct Cyc_Absyn_Stmt*f3;};struct _tuple9{struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_While_s_Absyn_Raw_stmt_struct{int tag;struct _tuple9 f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Break_s_Absyn_Raw_stmt_struct{int tag;};struct Cyc_Absyn_Continue_s_Absyn_Raw_stmt_struct{int tag;};struct Cyc_Absyn_Goto_s_Absyn_Raw_stmt_struct{int tag;struct _dyneither_ptr*f1;};struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _tuple9 f2;struct _tuple9 f3;struct Cyc_Absyn_Stmt*f4;};struct Cyc_Absyn_Switch_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;void*f3;};struct Cyc_Absyn_Fallthru_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Switch_clause**f2;};struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Decl*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Label_s_Absyn_Raw_stmt_struct{int tag;struct _dyneither_ptr*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Do_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct _tuple9 f2;};struct Cyc_Absyn_TryCatch_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_List_List*f2;void*f3;};struct Cyc_Absyn_ResetRegion_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Stmt{void*r;unsigned int loc;void*annot;};struct Cyc_Absyn_Wild_p_Absyn_Raw_pat_struct{int tag;};struct Cyc_Absyn_Var_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_AliasVar_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;};struct Cyc_Absyn_Reference_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_TagInt_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;};struct Cyc_Absyn_Tuple_p_Absyn_Raw_pat_struct{int tag;struct Cyc_List_List*f1;int f2;};struct Cyc_Absyn_Pointer_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Pat*f1;};struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_AggrInfo*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;int f4;};struct Cyc_Absyn_Datatype_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;struct Cyc_List_List*f3;int f4;};struct Cyc_Absyn_Null_p_Absyn_Raw_pat_struct{int tag;};struct Cyc_Absyn_Int_p_Absyn_Raw_pat_struct{int tag;enum Cyc_Absyn_Sign f1;int f2;};struct Cyc_Absyn_Char_p_Absyn_Raw_pat_struct{int tag;char f1;};struct Cyc_Absyn_Float_p_Absyn_Raw_pat_struct{int tag;struct _dyneither_ptr f1;int f2;};struct Cyc_Absyn_Enum_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_AnonEnum_p_Absyn_Raw_pat_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_UnknownId_p_Absyn_Raw_pat_struct{int tag;struct _tuple0*f1;};struct Cyc_Absyn_UnknownCall_p_Absyn_Raw_pat_struct{int tag;struct _tuple0*f1;struct Cyc_List_List*f2;int f3;};struct Cyc_Absyn_Exp_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Pat{void*r;void*topt;unsigned int loc;};struct Cyc_Absyn_Switch_clause{struct Cyc_Absyn_Pat*pattern;struct Cyc_Core_Opt*pat_vars;struct Cyc_Absyn_Exp*where_clause;struct Cyc_Absyn_Stmt*body;unsigned int loc;};struct Cyc_Absyn_Unresolved_b_Absyn_Binding_struct{int tag;struct _tuple0*f1;};struct Cyc_Absyn_Global_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Funname_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Param_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Local_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Pat_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Vardecl{enum Cyc_Absyn_Scope sc;struct _tuple0*name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*initializer;void*rgn;struct Cyc_List_List*attributes;int escapes;};struct Cyc_Absyn_Fndecl{enum Cyc_Absyn_Scope sc;int is_inline;struct _tuple0*name;struct Cyc_List_List*tvs;void*effect;struct Cyc_Absyn_Tqual ret_tqual;void*ret_type;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_Absyn_Stmt*body;void*cached_typ;struct Cyc_Core_Opt*param_vardecls;struct Cyc_Absyn_Vardecl*fn_vardecl;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;struct Cyc_List_List*requires_relns;struct Cyc_Absyn_Exp*ensures_clause;struct Cyc_List_List*ensures_relns;};struct Cyc_Absyn_Aggrfield{struct _dyneither_ptr*name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*width;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;};struct Cyc_Absyn_AggrdeclImpl{struct Cyc_List_List*exist_vars;struct Cyc_List_List*rgn_po;struct Cyc_List_List*fields;int tagged;};struct Cyc_Absyn_Aggrdecl{enum Cyc_Absyn_AggrKind kind;enum Cyc_Absyn_Scope sc;struct _tuple0*name;struct Cyc_List_List*tvs;struct Cyc_Absyn_AggrdeclImpl*impl;struct Cyc_List_List*attributes;int expected_mem_kind;};struct Cyc_Absyn_Datatypefield{struct _tuple0*name;struct Cyc_List_List*typs;unsigned int loc;enum Cyc_Absyn_Scope sc;};struct Cyc_Absyn_Datatypedecl{enum Cyc_Absyn_Scope sc;struct _tuple0*name;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*fields;int is_extensible;};struct Cyc_Absyn_Enumfield{struct _tuple0*name;struct Cyc_Absyn_Exp*tag;unsigned int loc;};struct Cyc_Absyn_Enumdecl{enum Cyc_Absyn_Scope sc;struct _tuple0*name;struct Cyc_Core_Opt*fields;};struct Cyc_Absyn_Typedefdecl{struct _tuple0*name;struct Cyc_Absyn_Tqual tq;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*kind;void*defn;struct Cyc_List_List*atts;int extern_c;};struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Let_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Pat*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;void*f4;};struct Cyc_Absyn_Letv_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Region_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;int f3;struct Cyc_Absyn_Exp*f4;};struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Absyn_Datatype_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;};struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;};struct Cyc_Absyn_Typedef_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Typedefdecl*f1;};struct Cyc_Absyn_Namespace_d_Absyn_Raw_decl_struct{int tag;struct _dyneither_ptr*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Using_d_Absyn_Raw_decl_struct{int tag;struct _tuple0*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_ExternC_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_ExternCinclude_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Porton_d_Absyn_Raw_decl_struct{int tag;};struct Cyc_Absyn_Portoff_d_Absyn_Raw_decl_struct{int tag;};struct Cyc_Absyn_Decl{void*r;unsigned int loc;};struct Cyc_Absyn_ArrayElement_Absyn_Designator_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_FieldName_Absyn_Designator_struct{int tag;struct _dyneither_ptr*f1;};extern char Cyc_Absyn_EmptyAnnot[11];struct Cyc_Absyn_EmptyAnnot_Absyn_AbsynAnnot_struct{char*tag;};
# 1109 "absyn.h"
struct Cyc_Absyn_Decl*Cyc_Absyn_new_decl(void*r,unsigned int loc);struct _tuple10{enum Cyc_Absyn_AggrKind f1;struct _tuple0*f2;};
# 1172
struct _tuple10 Cyc_Absyn_aggr_kinded_name(union Cyc_Absyn_AggrInfoU);
# 1184
struct _tuple0*Cyc_Absyn_binding2qvar(void*);struct Cyc_Absynpp_Params{int expand_typedefs;int qvar_to_Cids;int add_cyc_prefix;int to_VC;int decls_first;int rewrite_temp_tvars;int print_all_tvars;int print_all_kinds;int print_all_effects;int print_using_stmts;int print_externC_stmts;int print_full_evars;int print_zeroterm;int generate_line_directives;int use_curr_namespace;struct Cyc_List_List*curr_namespace;};
# 53 "absynpp.h"
void Cyc_Absynpp_set_params(struct Cyc_Absynpp_Params*fs);
# 55
extern struct Cyc_Absynpp_Params Cyc_Absynpp_cyc_params_r;
# 57
void Cyc_Absynpp_decllist2file(struct Cyc_List_List*tdl,struct Cyc___cycFILE*f);
# 28 "parse.h"
struct Cyc_List_List*Cyc_Parse_parse_file(struct Cyc___cycFILE*f);extern char Cyc_Parse_Exit[5];struct Cyc_Parse_Exit_exn_struct{char*tag;};struct Cyc_FlatList{struct Cyc_FlatList*tl;void*hd[0] __attribute__((aligned )) ;};
# 33
typedef struct Cyc_FlatList*Cyc_flat_list_t;struct Cyc_Type_specifier{int Signed_spec: 1;int Unsigned_spec: 1;int Short_spec: 1;int Long_spec: 1;int Long_Long_spec: 1;int Valid_type_spec: 1;void*Type_spec;unsigned int loc;};
# 44
typedef struct Cyc_Type_specifier Cyc_type_specifier_t;struct Cyc_Declarator{struct _tuple0*id;struct Cyc_List_List*tms;};
# 49
typedef struct Cyc_Declarator Cyc_declarator_t;struct _tuple12{struct Cyc_Declarator f1;struct Cyc_Absyn_Exp*f2;};struct _tuple11{struct _tuple11*tl;struct _tuple12 hd  __attribute__((aligned )) ;};
typedef struct _tuple11*Cyc_declarator_list_t;
# 52
enum Cyc_Storage_class{Cyc_Typedef_sc  = 0,Cyc_Extern_sc  = 1,Cyc_ExternC_sc  = 2,Cyc_Static_sc  = 3,Cyc_Auto_sc  = 4,Cyc_Register_sc  = 5,Cyc_Abstract_sc  = 6};
# 56
typedef enum Cyc_Storage_class Cyc_storage_class_t;struct Cyc_Declaration_spec{enum Cyc_Storage_class*sc;struct Cyc_Absyn_Tqual tq;struct Cyc_Type_specifier type_specs;int is_inline;struct Cyc_List_List*attributes;};
# 64
typedef struct Cyc_Declaration_spec Cyc_decl_spec_t;struct Cyc_Abstractdeclarator{struct Cyc_List_List*tms;};
# 68
typedef struct Cyc_Abstractdeclarator Cyc_abstractdeclarator_t;struct Cyc_Numelts_ptrqual_Pointer_qual_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Region_ptrqual_Pointer_qual_struct{int tag;void*f1;};struct Cyc_Thin_ptrqual_Pointer_qual_struct{int tag;};struct Cyc_Fat_ptrqual_Pointer_qual_struct{int tag;};struct Cyc_Zeroterm_ptrqual_Pointer_qual_struct{int tag;};struct Cyc_Nozeroterm_ptrqual_Pointer_qual_struct{int tag;};struct Cyc_Notnull_ptrqual_Pointer_qual_struct{int tag;};struct Cyc_Nullable_ptrqual_Pointer_qual_struct{int tag;};
# 80
typedef void*Cyc_pointer_qual_t;
typedef struct Cyc_List_List*Cyc_pointer_quals_t;struct _union_YYSTYPE_Int_tok{int tag;union Cyc_Absyn_Cnst val;};struct _union_YYSTYPE_Char_tok{int tag;char val;};struct _union_YYSTYPE_String_tok{int tag;struct _dyneither_ptr val;};struct _union_YYSTYPE_Stringopt_tok{int tag;struct Cyc_Core_Opt*val;};struct _union_YYSTYPE_QualId_tok{int tag;struct _tuple0*val;};struct _tuple13{int f1;struct _dyneither_ptr f2;};struct _union_YYSTYPE_Asm_tok{int tag;struct _tuple13 val;};struct _union_YYSTYPE_Exp_tok{int tag;struct Cyc_Absyn_Exp*val;};struct _union_YYSTYPE_Stmt_tok{int tag;struct Cyc_Absyn_Stmt*val;};struct _tuple14{unsigned int f1;union Cyc_Absyn_Constraint*f2;union Cyc_Absyn_Constraint*f3;};struct _union_YYSTYPE_YY1{int tag;struct _tuple14*val;};struct _union_YYSTYPE_YY2{int tag;union Cyc_Absyn_Constraint*val;};struct _union_YYSTYPE_YY3{int tag;struct Cyc_List_List*val;};struct _union_YYSTYPE_YY4{int tag;struct Cyc_List_List*val;};struct _union_YYSTYPE_YY5{int tag;struct Cyc_List_List*val;};struct _union_YYSTYPE_YY6{int tag;enum Cyc_Absyn_Primop val;};struct _union_YYSTYPE_YY7{int tag;struct Cyc_Core_Opt*val;};struct _union_YYSTYPE_YY8{int tag;struct Cyc_List_List*val;};struct _union_YYSTYPE_YY9{int tag;struct Cyc_Absyn_Pat*val;};struct _tuple15{struct Cyc_List_List*f1;int f2;};struct _union_YYSTYPE_YY10{int tag;struct _tuple15*val;};struct _union_YYSTYPE_YY11{int tag;struct Cyc_List_List*val;};struct _tuple16{struct Cyc_List_List*f1;struct Cyc_Absyn_Pat*f2;};struct _union_YYSTYPE_YY12{int tag;struct _tuple16*val;};struct _union_YYSTYPE_YY13{int tag;struct Cyc_List_List*val;};struct _union_YYSTYPE_YY14{int tag;struct _tuple15*val;};struct _union_YYSTYPE_YY15{int tag;struct Cyc_Absyn_Fndecl*val;};struct _union_YYSTYPE_YY16{int tag;struct Cyc_List_List*val;};struct _union_YYSTYPE_YY17{int tag;struct Cyc_Declaration_spec val;};struct _union_YYSTYPE_YY18{int tag;struct _tuple12 val;};struct _union_YYSTYPE_YY19{int tag;struct _tuple11*val;};struct _union_YYSTYPE_YY20{int tag;enum Cyc_Storage_class*val;};struct _union_YYSTYPE_YY21{int tag;struct Cyc_Type_specifier val;};struct _union_YYSTYPE_YY22{int tag;enum Cyc_Absyn_AggrKind val;};struct _union_YYSTYPE_YY23{int tag;struct Cyc_Absyn_Tqual val;};struct _union_YYSTYPE_YY24{int tag;struct Cyc_List_List*val;};struct _union_YYSTYPE_YY25{int tag;struct Cyc_List_List*val;};struct _union_YYSTYPE_YY26{int tag;struct Cyc_List_List*val;};struct _union_YYSTYPE_YY27{int tag;struct Cyc_Declarator val;};struct _tuple17{struct Cyc_Declarator f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;};struct _union_YYSTYPE_YY28{int tag;struct _tuple17*val;};struct _union_YYSTYPE_YY29{int tag;struct Cyc_List_List*val;};struct _union_YYSTYPE_YY30{int tag;struct Cyc_Abstractdeclarator val;};struct _union_YYSTYPE_YY31{int tag;int val;};struct _union_YYSTYPE_YY32{int tag;enum Cyc_Absyn_Scope val;};struct _union_YYSTYPE_YY33{int tag;struct Cyc_Absyn_Datatypefield*val;};struct _union_YYSTYPE_YY34{int tag;struct Cyc_List_List*val;};struct _tuple18{struct Cyc_Absyn_Tqual f1;struct Cyc_Type_specifier f2;struct Cyc_List_List*f3;};struct _union_YYSTYPE_YY35{int tag;struct _tuple18 val;};struct _union_YYSTYPE_YY36{int tag;struct Cyc_List_List*val;};struct _union_YYSTYPE_YY37{int tag;struct _tuple8*val;};struct _union_YYSTYPE_YY38{int tag;struct Cyc_List_List*val;};struct _tuple19{struct Cyc_List_List*f1;int f2;struct Cyc_Absyn_VarargInfo*f3;void*f4;struct Cyc_List_List*f5;};struct _union_YYSTYPE_YY39{int tag;struct _tuple19*val;};struct _union_YYSTYPE_YY40{int tag;struct Cyc_List_List*val;};struct _union_YYSTYPE_YY41{int tag;struct Cyc_List_List*val;};struct _union_YYSTYPE_YY42{int tag;void*val;};struct _union_YYSTYPE_YY43{int tag;struct Cyc_Absyn_Kind*val;};struct _union_YYSTYPE_YY44{int tag;void*val;};struct _union_YYSTYPE_YY45{int tag;struct Cyc_List_List*val;};struct _union_YYSTYPE_YY46{int tag;void*val;};struct _union_YYSTYPE_YY47{int tag;struct Cyc_Absyn_Enumfield*val;};struct _union_YYSTYPE_YY48{int tag;struct Cyc_List_List*val;};struct _union_YYSTYPE_YY49{int tag;void*val;};struct _union_YYSTYPE_YY50{int tag;struct Cyc_List_List*val;};struct _union_YYSTYPE_YY51{int tag;union Cyc_Absyn_Constraint*val;};struct _union_YYSTYPE_YY52{int tag;struct Cyc_List_List*val;};struct _union_YYSTYPE_YY53{int tag;void*val;};struct _union_YYSTYPE_YY54{int tag;struct Cyc_List_List*val;};struct _union_YYSTYPE_YY55{int tag;struct Cyc_Absyn_Exp*val;};struct _union_YYSTYPE_YYINITIALSVAL{int tag;int val;};union Cyc_YYSTYPE{struct _union_YYSTYPE_Int_tok Int_tok;struct _union_YYSTYPE_Char_tok Char_tok;struct _union_YYSTYPE_String_tok String_tok;struct _union_YYSTYPE_Stringopt_tok Stringopt_tok;struct _union_YYSTYPE_QualId_tok QualId_tok;struct _union_YYSTYPE_Asm_tok Asm_tok;struct _union_YYSTYPE_Exp_tok Exp_tok;struct _union_YYSTYPE_Stmt_tok Stmt_tok;struct _union_YYSTYPE_YY1 YY1;struct _union_YYSTYPE_YY2 YY2;struct _union_YYSTYPE_YY3 YY3;struct _union_YYSTYPE_YY4 YY4;struct _union_YYSTYPE_YY5 YY5;struct _union_YYSTYPE_YY6 YY6;struct _union_YYSTYPE_YY7 YY7;struct _union_YYSTYPE_YY8 YY8;struct _union_YYSTYPE_YY9 YY9;struct _union_YYSTYPE_YY10 YY10;struct _union_YYSTYPE_YY11 YY11;struct _union_YYSTYPE_YY12 YY12;struct _union_YYSTYPE_YY13 YY13;struct _union_YYSTYPE_YY14 YY14;struct _union_YYSTYPE_YY15 YY15;struct _union_YYSTYPE_YY16 YY16;struct _union_YYSTYPE_YY17 YY17;struct _union_YYSTYPE_YY18 YY18;struct _union_YYSTYPE_YY19 YY19;struct _union_YYSTYPE_YY20 YY20;struct _union_YYSTYPE_YY21 YY21;struct _union_YYSTYPE_YY22 YY22;struct _union_YYSTYPE_YY23 YY23;struct _union_YYSTYPE_YY24 YY24;struct _union_YYSTYPE_YY25 YY25;struct _union_YYSTYPE_YY26 YY26;struct _union_YYSTYPE_YY27 YY27;struct _union_YYSTYPE_YY28 YY28;struct _union_YYSTYPE_YY29 YY29;struct _union_YYSTYPE_YY30 YY30;struct _union_YYSTYPE_YY31 YY31;struct _union_YYSTYPE_YY32 YY32;struct _union_YYSTYPE_YY33 YY33;struct _union_YYSTYPE_YY34 YY34;struct _union_YYSTYPE_YY35 YY35;struct _union_YYSTYPE_YY36 YY36;struct _union_YYSTYPE_YY37 YY37;struct _union_YYSTYPE_YY38 YY38;struct _union_YYSTYPE_YY39 YY39;struct _union_YYSTYPE_YY40 YY40;struct _union_YYSTYPE_YY41 YY41;struct _union_YYSTYPE_YY42 YY42;struct _union_YYSTYPE_YY43 YY43;struct _union_YYSTYPE_YY44 YY44;struct _union_YYSTYPE_YY45 YY45;struct _union_YYSTYPE_YY46 YY46;struct _union_YYSTYPE_YY47 YY47;struct _union_YYSTYPE_YY48 YY48;struct _union_YYSTYPE_YY49 YY49;struct _union_YYSTYPE_YY50 YY50;struct _union_YYSTYPE_YY51 YY51;struct _union_YYSTYPE_YY52 YY52;struct _union_YYSTYPE_YY53 YY53;struct _union_YYSTYPE_YY54 YY54;struct _union_YYSTYPE_YY55 YY55;struct _union_YYSTYPE_YYINITIALSVAL YYINITIALSVAL;};struct Cyc_Yyltype{int timestamp;int first_line;int first_column;int last_line;int last_column;};
# 78 "parse_tab.h"
typedef struct Cyc_Yyltype Cyc_yyltype;struct Cyc_Dict_T;
# 46 "dict.h"
typedef const struct Cyc_Dict_T*Cyc_Dict_tree;struct Cyc_Dict_Dict{int(*rel)(void*,void*);struct _RegionHandle*r;const struct Cyc_Dict_T*t;};
# 52
typedef struct Cyc_Dict_Dict Cyc_Dict_dict_t;extern char Cyc_Dict_Present[8];struct Cyc_Dict_Present_exn_struct{char*tag;};extern char Cyc_Dict_Absent[7];struct Cyc_Dict_Absent_exn_struct{char*tag;};struct Cyc_RgnOrder_RgnPO;
# 30 "rgnorder.h"
typedef struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_rgn_po_t;
# 32
struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_initial_fn_po(struct _RegionHandle*,struct Cyc_List_List*tvs,struct Cyc_List_List*po,void*effect,struct Cyc_Absyn_Tvar*fst_rgn,unsigned int);
# 39
struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_add_outlives_constraint(struct _RegionHandle*,struct Cyc_RgnOrder_RgnPO*po,void*eff,void*rgn,unsigned int loc);
struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_add_youngest(struct _RegionHandle*,struct Cyc_RgnOrder_RgnPO*po,struct Cyc_Absyn_Tvar*rgn,int resetable,int opened);
struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_add_unordered(struct _RegionHandle*,struct Cyc_RgnOrder_RgnPO*po,struct Cyc_Absyn_Tvar*rgn);
int Cyc_RgnOrder_is_region_resetable(struct Cyc_RgnOrder_RgnPO*po,struct Cyc_Absyn_Tvar*r);
int Cyc_RgnOrder_effect_outlives(struct Cyc_RgnOrder_RgnPO*po,void*eff,void*rgn);
int Cyc_RgnOrder_satisfies_constraints(struct Cyc_RgnOrder_RgnPO*po,struct Cyc_List_List*constraints,void*default_bound,int do_pin);
# 46
int Cyc_RgnOrder_eff_outlives_eff(struct Cyc_RgnOrder_RgnPO*po,void*eff1,void*eff2);
# 49
void Cyc_RgnOrder_print_region_po(struct Cyc_RgnOrder_RgnPO*po);extern char Cyc_Tcenv_Env_error[10];struct Cyc_Tcenv_Env_error_exn_struct{char*tag;};struct Cyc_Tcenv_CList{void*hd;const struct Cyc_Tcenv_CList*tl;};
# 38 "tcenv.h"
typedef const struct Cyc_Tcenv_CList*Cyc_Tcenv_mclist_t;
typedef const struct Cyc_Tcenv_CList*const Cyc_Tcenv_clist_t;struct Cyc_Tcenv_VarRes_Tcenv_Resolved_struct{int tag;void*f1;};struct Cyc_Tcenv_AggrRes_Tcenv_Resolved_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Tcenv_DatatypeRes_Tcenv_Resolved_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;};struct Cyc_Tcenv_EnumRes_Tcenv_Resolved_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Tcenv_AnonEnumRes_Tcenv_Resolved_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};
# 49
typedef void*Cyc_Tcenv_resolved_t;struct Cyc_Tcenv_Genv{struct _RegionHandle*grgn;struct Cyc_Dict_Dict aggrdecls;struct Cyc_Dict_Dict datatypedecls;struct Cyc_Dict_Dict enumdecls;struct Cyc_Dict_Dict typedefs;struct Cyc_Dict_Dict ordinaries;};
# 65
typedef struct Cyc_Tcenv_Genv*Cyc_Tcenv_genv_t;struct Cyc_Tcenv_Fenv;
# 69
typedef struct Cyc_Tcenv_Fenv*Cyc_Tcenv_fenv_t;struct Cyc_Tcenv_Tenv{struct Cyc_List_List*ns;struct Cyc_Tcenv_Genv*ae;struct Cyc_Tcenv_Fenv*le;int allow_valueof;int in_extern_c_include;};
# 80
typedef struct Cyc_Tcenv_Tenv*Cyc_Tcenv_tenv_t;
# 96 "tcenv.h"
struct Cyc_Tcenv_Tenv*Cyc_Tcenv_tc_init(struct _RegionHandle*);
# 114
enum Cyc_Tcenv_NewStatus{Cyc_Tcenv_NoneNew  = 0,Cyc_Tcenv_InNew  = 1,Cyc_Tcenv_InNewAggr  = 2};
# 34 "tc.h"
void Cyc_Tc_tc(struct _RegionHandle*,struct Cyc_Tcenv_Tenv*te,int var_default_init,struct Cyc_List_List*ds);
# 31 "binding.h"
void Cyc_Binding_resolve_all(struct Cyc_List_List*tds);
# 34
typedef struct Cyc_List_List*Cyc_Binding_namespace_name_t;struct Cyc_Binding_Namespace_Binding_NSDirective_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Binding_Using_Binding_NSDirective_struct{int tag;struct Cyc_List_List*f1;};
# 39
typedef void*Cyc_Binding_nsdirective_t;struct Cyc_Binding_NSCtxt{struct Cyc_List_List*curr_ns;struct Cyc_List_List*availables;struct Cyc_Dict_Dict ns_data;};
# 45
typedef struct Cyc_Binding_NSCtxt*Cyc_Binding_nsctxt_t;
# 47
struct Cyc_Binding_NSCtxt*Cyc_Binding_mt_nsctxt(void*,void*(*mkdata)(void*));
void Cyc_Binding_enter_ns(struct Cyc_Binding_NSCtxt*,struct _dyneither_ptr*,void*,void*(*mkdata)(void*));
struct Cyc_List_List*Cyc_Binding_enter_using(unsigned int loc,struct Cyc_Binding_NSCtxt*ctxt,struct _tuple0*usename);
void Cyc_Binding_leave_ns(struct Cyc_Binding_NSCtxt*ctxt);
void Cyc_Binding_leave_using(struct Cyc_Binding_NSCtxt*ctxt);
struct Cyc_List_List*Cyc_Binding_resolve_rel_ns(unsigned int,struct Cyc_Binding_NSCtxt*,struct Cyc_List_List*);
# 75 "buildlib.cyl"
void Cyc_Lex_lex_init(int use_cyclone_keywords);
# 83
extern char* Ccomp;
# 85
static int Cyc_do_setjmp=0;
static int Cyc_verbose=0;
# 88
struct Cyc___cycFILE*Cyc_log_file=0;
struct Cyc___cycFILE*Cyc_cstubs_file=0;
struct Cyc___cycFILE*Cyc_cycstubs_file=0;
# 92
int Cyc_log(struct _dyneither_ptr fmt,struct _dyneither_ptr ap){
# 95
if(Cyc_log_file == 0){
({void*_tmp0=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp1="Internal error: log file is NULL\n";_tag_dyneither(_tmp1,sizeof(char),34);}),_tag_dyneither(_tmp0,sizeof(void*),0));});
 exit(1);}{
# 99
int _tmp2=Cyc_vfprintf((struct Cyc___cycFILE*)_check_null(Cyc_log_file),fmt,ap);
Cyc_fflush((struct Cyc___cycFILE*)_check_null(Cyc_log_file));
return _tmp2;};}
# 104
static struct _dyneither_ptr*Cyc_current_source=0;
static struct Cyc_List_List*Cyc_current_args=0;
static struct Cyc_Set_Set**Cyc_current_targets=0;
static void Cyc_add_target(struct _dyneither_ptr*sptr){
Cyc_current_targets=({struct Cyc_Set_Set**_tmp3=_cycalloc(sizeof(*_tmp3));_tmp3[0]=((struct Cyc_Set_Set*(*)(struct Cyc_Set_Set*s,struct _dyneither_ptr*elt))Cyc_Set_insert)(*((struct Cyc_Set_Set**)_check_null(Cyc_current_targets)),sptr);_tmp3;});}struct _tuple20{struct _dyneither_ptr*f1;struct Cyc_Set_Set*f2;};
# 111
typedef struct _tuple20*Cyc_line_t;
struct _tuple20*Cyc_line(struct Cyc_Lexing_lexbuf*);
int Cyc_macroname(struct Cyc_Lexing_lexbuf*);
int Cyc_args(struct Cyc_Lexing_lexbuf*);
int Cyc_token(struct Cyc_Lexing_lexbuf*);
int Cyc_string(struct Cyc_Lexing_lexbuf*);
# 118
struct Cyc___cycFILE*Cyc_slurp_out=0;
int Cyc_slurp(struct Cyc_Lexing_lexbuf*);
int Cyc_slurp_string(struct Cyc_Lexing_lexbuf*);
int Cyc_asmtok(struct Cyc_Lexing_lexbuf*);
int Cyc_asm_string(struct Cyc_Lexing_lexbuf*);
int Cyc_asm_comment(struct Cyc_Lexing_lexbuf*);struct _tuple21{struct _dyneither_ptr f1;struct _dyneither_ptr*f2;};
# 125
typedef struct _tuple21*Cyc_suck_line_t;
struct _tuple21*Cyc_suck_line(struct Cyc_Lexing_lexbuf*);
int Cyc_suck_macroname(struct Cyc_Lexing_lexbuf*);
int Cyc_suck_restofline(struct Cyc_Lexing_lexbuf*);
struct _dyneither_ptr Cyc_current_line={(void*)0,(void*)0,(void*)(0 + 0)};struct _tuple22{struct _dyneither_ptr f1;struct _dyneither_ptr f2;};
# 133
typedef struct _tuple22*Cyc_ifdefined_t;struct _tuple23{struct _dyneither_ptr f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;struct Cyc_List_List*f4;struct Cyc_List_List*f5;struct Cyc_List_List*f6;};
# 135
typedef struct _tuple23*Cyc_spec_t;
# 137
struct _tuple23*Cyc_spec(struct Cyc_Lexing_lexbuf*);
int Cyc_commands(struct Cyc_Lexing_lexbuf*);
int Cyc_snarfsymbols(struct Cyc_Lexing_lexbuf*);
int Cyc_block(struct Cyc_Lexing_lexbuf*);
int Cyc_block_string(struct Cyc_Lexing_lexbuf*);
int Cyc_block_comment(struct Cyc_Lexing_lexbuf*);
struct _dyneither_ptr Cyc_current_headerfile={(void*)0,(void*)0,(void*)(0 + 0)};
struct Cyc_List_List*Cyc_snarfed_symbols=0;
struct Cyc_List_List*Cyc_current_symbols=0;
struct Cyc_List_List*Cyc_current_cstubs=0;
struct Cyc_List_List*Cyc_current_cycstubs=0;
struct Cyc_List_List*Cyc_current_hstubs=0;
struct Cyc_List_List*Cyc_current_omit_symbols=0;
struct Cyc_List_List*Cyc_current_cpp=0;
struct Cyc_Buffer_t*Cyc_specbuf=0;
int Cyc_braces_to_match=0;
int Cyc_parens_to_match=0;
# 155
int Cyc_numdef=0;
# 157
static struct Cyc_List_List*Cyc_cppargs=0;
# 160
const int Cyc_lex_base[425]={0,0,75,192,305,310,311,166,312,91,27,384,28,523,637,715,283,325,92,- 3,0,- 1,- 2,- 8,- 3,1,- 2,323,- 4,2,166,- 5,605,797,312,- 6,- 7,- 4,16,- 3,29,11,835,- 3,910,13,- 14,223,12,- 2,216,20,26,28,33,23,48,70,54,64,74,100,91,107,94,370,386,111,103,95,122,122,375,414,111,111,153,377,1024,1139,525,176,210,228,214,216,217,242,498,1253,1368,- 9,654,- 10,224,245,508,1482,1597,684,- 8,720,- 11,432,510,515,1674,1751,1828,1909,434,465,550,1984,249,249,249,248,244,254,0,13,4,2065,5,628,2073,2138,660,49,467,6,2099,7,705,2161,2199,820,- 22,1051,1056,261,314,242,251,258,250,271,281,274,275,278,288,295,282,- 20,292,296,300,313,321,326,- 15,309,325,319,315,326,365,400,417,418,404,400,400,427,431,- 17,422,421,415,436,433,449,427,449,453,441,445,436,436,- 19,444,438,442,453,464,447,449,482,489,490,4,6,21,491,492,505,504,516,516,524,554,24,556,557,24,20,570,571,53,627,644,- 13,647,594,596,583,584,598,598,605,606,1,677,623,624,654,647,654,658,659,623,624,639,645,648,703,704,705,654,655,710,720,721,679,680,735,737,765,713,714,770,797,798,746,747,803,814,815,- 12,762,763,1030,- 21,1165,762,763,760,773,772,767,769,772,773,771,827,1144,819,820,818,832,1258,1170,871,872,862,864,862,875,1370,893,894,892,905,1375,- 7,- 8,8,1263,2231,9,996,2255,2293,1341,1279,- 49,1150,- 2,945,- 4,974,998,1028,980,1010,1023,1448,981,2320,2363,994,1055,993,996,2433,994,1057,- 36,- 42,- 37,2508,- 28,1002,- 40,- 25,1021,- 27,- 45,- 39,- 48,2583,2612,1467,1081,1091,1563,2622,2652,1582,2281,2685,2716,2754,1083,1093,2824,2862,1118,1157,1195,1206,1198,1241,- 6,- 34,1040,2794,- 47,- 30,- 32,- 46,- 29,- 31,- 33,1048,2902,1112,1117,2128,1119,1124,1125,1133,1134,1138,1146,1147,1191,2975,3059,- 23,- 16,- 18,2239,1192,- 24,- 41,- 38,- 35,- 26,1394,3141,2,3224,1191,15,1170,1174,1175,1173,1171,1185,1255};
const int Cyc_lex_backtrk[425]={- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,2,- 1,- 1,- 1,- 1,2,- 1,8,- 1,3,5,- 1,- 1,6,5,- 1,- 1,- 1,6,- 1,5,1,0,- 1,0,1,- 1,12,13,- 1,13,13,13,13,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,11,12,2,4,4,- 1,0,0,0,2,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,2,2,8,3,5,- 1,6,5,6,5,2,8,3,5,- 1,6,5,- 1,21,21,21,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,15,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,17,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,6,1,9,2,4,- 1,5,4,- 1,- 1,2,- 1,48,- 1,48,48,48,48,48,48,48,48,5,7,48,48,48,48,0,48,48,- 1,- 1,- 1,0,- 1,43,- 1,- 1,42,- 1,- 1,- 1,- 1,9,7,- 1,7,7,- 1,8,9,- 1,- 1,9,5,6,5,5,- 1,4,4,4,6,6,5,5,- 1,- 1,- 1,9,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,2,- 1,1,2,1,1,- 1,- 1,- 1,- 1,- 1,- 1,- 1};
const int Cyc_lex_default[425]={- 1,- 1,- 1,317,306,138,23,36,23,19,- 1,- 1,12,31,46,31,36,23,19,0,- 1,0,0,0,0,- 1,0,- 1,0,- 1,- 1,0,- 1,- 1,- 1,0,0,0,- 1,0,40,- 1,- 1,0,- 1,- 1,0,- 1,- 1,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,0,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,0,103,- 1,- 1,- 1,- 1,- 1,110,110,110,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,130,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,- 1,- 1,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,0,- 1,0,- 1,- 1,386,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,0,0,- 1,0,- 1,0,0,- 1,0,0,0,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,0,- 1,- 1,0,0,0,0,0,0,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,0,0,- 1,- 1,0,0,0,0,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1};
const int Cyc_lex_trans[3481]={0,0,0,0,0,0,0,0,0,0,22,19,28,417,19,28,19,28,100,19,45,45,45,45,45,22,45,0,0,0,0,0,21,234,21,418,203,22,- 1,- 1,22,- 1,- 1,45,204,45,224,22,415,415,415,415,415,415,415,415,415,415,31,103,22,205,114,40,214,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,31,217,218,221,415,130,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,413,413,413,413,413,413,413,413,413,413,121,20,74,67,54,55,56,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,57,58,59,60,413,61,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,62,63,37,318,319,318,318,319,39,22,64,65,68,69,70,129,34,34,34,34,34,34,34,34,71,72,318,320,321,75,76,322,323,324,104,104,325,326,104,327,328,329,330,331,331,331,331,331,331,331,331,331,332,77,333,334,335,104,19,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,19,- 1,- 1,337,336,101,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,307,338,37,308,139,139,24,24,139,131,122,94,84,81,85,38,82,86,28,87,24,29,83,25,309,88,95,139,96,22,26,26,21,21,115,116,117,140,118,119,120,142,192,193,26,35,35,35,35,35,35,35,35,178,172,163,30,30,30,30,30,30,30,30,66,66,156,149,66,73,73,78,78,73,150,78,151,152,153,154,66,66,310,155,66,157,158,66,132,123,141,39,73,22,78,159,143,144,145,160,146,161,27,66,147,31,162,21,73,73,164,165,73,148,166,167,168,113,113,113,113,113,113,113,113,113,113,- 1,32,- 1,- 1,73,- 1,22,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,- 1,169,- 1,- 1,113,- 1,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,89,89,28,170,89,171,100,173,174,175,97,97,104,104,97,176,104,112,112,177,179,112,180,89,181,105,105,80,80,105,19,80,21,97,182,104,183,184,185,186,112,187,188,189,190,191,271,194,105,195,80,196,112,112,91,197,112,198,199,21,21,21,106,107,106,106,106,106,106,106,106,106,106,106,21,112,200,201,202,206,207,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,208,209,210,211,106,212,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,47,47,35,213,47,215,216,33,33,33,33,33,33,33,33,33,33,92,92,219,220,92,222,47,33,33,33,33,33,33,128,128,128,128,128,128,128,128,48,223,92,225,- 1,226,- 1,227,228,99,99,229,49,99,230,231,232,233,33,33,33,33,33,33,35,35,35,35,35,35,35,35,99,235,236,237,265,- 1,238,- 1,41,41,239,260,41,101,101,255,250,101,243,240,50,241,244,245,246,51,52,247,248,249,46,41,53,251,252,253,101,137,137,137,137,137,137,137,137,254,93,42,42,42,42,42,42,42,42,42,42,242,256,257,258,93,259,28,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,102,100,261,262,42,263,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,264,91,43,266,267,102,268,33,33,33,33,33,33,33,33,33,33,269,270,272,273,288,283,279,33,33,33,33,33,33,35,35,35,35,35,35,35,35,280,281,282,36,284,285,286,44,44,44,44,44,44,44,44,44,44,21,33,33,33,33,33,33,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,287,289,290,291,44,292,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,301,296,297,28,298,299,300,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,302,303,304,305,44,411,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,78,78,410,408,78,- 1,273,273,385,376,273,315,315,315,315,315,315,315,315,351,383,346,341,78,343,344,409,293,293,273,345,293,275,275,- 1,380,275,274,384,79,79,79,79,79,79,79,79,79,79,350,293,381,382,379,407,275,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,347,348,349,339,79,387,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,31,80,80,294,155,80,287,287,276,191,287,274,412,295,412,412,404,102,277,100,100,35,35,278,80,270,223,275,275,287,46,275,293,293,340,412,293,19,162,403,79,79,79,79,79,79,79,79,79,79,275,100,100,35,35,293,31,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,177,138,21,31,79,31,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,89,89,276,31,89,292,292,294,419,292,316,316,277,420,316,421,422,295,36,423,36,375,- 1,89,424,21,316,316,292,0,316,0,0,316,0,21,31,0,0,90,90,90,90,90,90,90,90,90,90,316,36,21,36,375,0,375,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,0,0,0,0,90,375,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,91,92,92,300,300,92,0,300,305,305,0,0,305,31,31,31,31,31,31,31,31,0,0,0,92,0,300,412,0,412,412,305,0,0,28,0,0,0,0,35,90,90,90,90,90,90,90,90,90,90,412,0,0,0,0,0,0,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,0,0,0,0,90,0,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,97,97,0,377,97,378,378,378,378,378,378,378,378,378,378,0,0,0,0,357,0,357,0,97,358,358,358,358,358,358,358,358,358,358,0,0,0,0,0,98,98,98,98,98,98,98,98,98,98,0,0,0,0,0,0,0,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,0,0,0,0,98,0,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,36,99,99,0,0,99,358,358,358,358,358,358,358,358,358,358,0,0,0,0,361,0,361,0,99,362,362,362,362,362,362,362,362,362,362,0,0,0,0,0,98,98,98,98,98,98,98,98,98,98,0,0,0,0,0,0,0,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,0,0,0,0,98,0,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,108,108,108,108,108,108,108,108,108,108,108,108,22,0,0,0,0,0,0,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,0,0,0,0,108,0,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,109,108,108,108,108,108,108,108,108,108,108,22,0,0,0,0,0,0,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,0,0,0,0,108,0,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,22,0,0,0,0,0,0,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,- 1,0,0,- 1,108,0,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,0,0,0,0,109,109,109,109,109,109,109,109,109,109,109,109,111,0,0,0,0,0,0,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,0,0,0,0,109,0,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,113,113,113,113,113,113,113,113,113,113,0,0,0,0,0,0,0,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,28,0,0,124,113,0,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,0,0,28,0,0,133,125,125,125,125,125,125,125,125,127,127,127,127,127,127,127,127,127,127,0,0,0,0,0,0,0,127,127,127,127,127,127,0,0,0,134,134,134,134,134,134,134,134,0,0,0,0,0,0,0,31,0,0,- 1,0,138,0,0,127,127,127,127,127,127,405,405,405,405,405,405,405,405,0,126,127,127,127,127,127,127,127,127,127,127,31,0,0,0,0,0,0,127,127,127,127,127,127,136,136,136,136,136,136,136,136,136,136,135,0,0,0,0,0,0,136,136,136,136,136,136,0,0,0,127,127,127,127,127,127,19,0,0,311,0,0,136,136,136,136,136,136,136,136,136,136,0,136,136,136,136,136,136,136,136,136,136,136,136,0,0,0,0,0,0,0,0,138,312,312,312,312,312,312,312,312,406,406,406,406,406,406,406,406,0,136,136,136,136,136,136,0,314,314,314,314,314,314,314,314,314,314,0,0,0,0,0,0,0,314,314,314,314,314,314,0,0,28,362,362,362,362,362,362,362,362,362,362,0,0,314,314,314,314,314,314,314,314,314,314,313,314,314,314,314,314,314,314,314,314,314,314,314,0,0,352,0,363,363,363,363,363,363,363,363,364,364,0,0,0,0,0,0,0,0,0,0,0,354,314,314,314,314,314,314,365,0,0,0,0,0,0,0,0,366,0,0,367,352,0,353,353,353,353,353,353,353,353,353,353,354,0,0,0,0,0,0,365,0,0,0,354,0,0,0,0,366,0,355,367,0,0,0,0,0,0,0,356,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,354,0,0,0,0,0,0,355,0,0,0,0,0,0,0,0,356,342,342,342,342,342,342,342,342,342,342,0,0,0,0,0,0,0,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,0,0,0,0,342,0,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,0,0,0,0,0,0,0,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,0,0,0,0,342,0,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,359,359,359,359,359,359,359,359,359,359,0,0,0,0,0,0,0,0,0,0,0,360,93,0,0,0,0,352,93,353,353,353,353,353,353,353,353,353,353,358,358,358,358,358,358,358,358,358,358,0,354,0,0,360,93,0,0,355,0,0,93,91,0,0,0,0,356,91,0,359,359,359,359,359,359,359,359,359,359,0,0,0,354,0,0,0,0,0,0,355,360,93,0,91,0,0,0,93,356,91,0,0,362,362,362,362,362,362,362,362,362,362,0,0,0,0,0,0,0,0,0,0,360,93,93,0,0,0,0,93,93,352,0,363,363,363,363,363,363,363,363,364,364,0,0,0,0,0,0,0,0,0,0,0,354,0,93,0,0,0,0,373,93,0,0,0,0,0,0,352,374,364,364,364,364,364,364,364,364,364,364,0,0,0,0,0,354,0,0,0,0,0,354,373,0,0,0,0,0,371,0,0,374,0,0,0,0,0,372,0,0,378,378,378,378,378,378,378,378,378,378,0,0,0,354,0,0,0,0,0,0,371,360,93,0,0,0,0,0,93,372,368,368,368,368,368,368,368,368,368,368,0,0,0,0,0,0,0,368,368,368,368,368,368,360,93,0,0,0,0,0,93,0,0,0,0,0,0,0,368,368,368,368,368,368,368,368,368,368,0,368,368,368,368,368,368,368,368,368,368,368,368,0,0,0,388,0,369,0,0,389,0,0,0,0,0,370,0,0,390,390,390,390,390,390,390,390,0,368,368,368,368,368,368,391,0,0,0,0,369,0,0,0,0,0,0,0,0,370,0,0,0,0,0,0,0,0,0,0,0,0,0,0,392,0,0,0,0,393,394,0,0,0,395,0,0,0,0,0,0,0,396,0,0,0,397,0,398,0,399,0,400,401,401,401,401,401,401,401,401,401,401,0,0,0,0,0,0,0,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,0,0,0,0,0,0,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,402,0,0,0,0,0,0,0,0,401,401,401,401,401,401,401,401,401,401,0,0,0,0,0,0,0,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,0,0,0,0,0,0,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,19,0,0,414,0,0,0,413,413,413,413,413,413,413,413,413,413,0,0,0,0,0,0,0,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,0,0,0,0,413,0,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,416,0,0,0,0,0,0,0,415,415,415,415,415,415,415,415,415,415,0,0,0,0,0,0,0,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,0,0,0,0,415,0,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
const int Cyc_lex_check[3481]={- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,25,29,0,122,124,131,133,308,311,41,41,45,45,41,417,45,- 1,- 1,- 1,- 1,- 1,120,233,414,0,202,10,12,40,10,12,40,41,202,45,203,20,1,1,1,1,1,1,1,1,1,1,38,48,121,204,10,38,213,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,129,216,217,220,1,129,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,9,18,51,52,53,54,55,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,56,57,58,59,2,60,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,61,62,7,3,3,3,3,3,7,7,63,64,67,68,69,7,30,30,30,30,30,30,30,30,70,71,3,3,3,74,75,3,3,3,47,47,3,3,47,3,3,3,3,3,3,3,3,3,3,3,3,3,3,76,3,3,3,47,0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,10,12,40,3,3,81,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,3,16,4,5,5,6,8,5,6,8,82,83,50,84,16,50,85,27,86,17,27,50,17,4,87,94,5,95,5,6,8,9,18,114,115,116,5,117,118,119,141,143,143,17,34,34,34,34,34,34,34,34,144,145,146,27,27,27,27,27,27,27,27,65,65,147,148,65,72,72,77,77,72,149,77,150,151,152,153,66,66,4,154,66,156,157,65,6,8,5,16,72,16,77,158,142,142,142,159,142,160,17,66,142,27,161,7,73,73,163,164,73,142,165,166,167,11,11,11,11,11,11,11,11,11,11,103,27,110,103,73,110,3,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,111,168,130,111,11,130,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,88,88,66,169,88,170,171,172,173,174,96,96,104,104,96,175,104,105,105,176,178,105,179,88,180,13,13,80,80,13,73,80,16,96,181,104,182,183,184,185,105,186,187,188,189,190,192,193,13,194,80,195,112,112,4,196,112,197,198,5,6,8,13,13,13,13,13,13,13,13,13,13,13,13,17,112,199,200,201,205,206,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,207,208,209,210,13,211,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,14,14,80,212,14,214,215,32,32,32,32,32,32,32,32,32,32,92,92,218,219,92,221,14,32,32,32,32,32,32,125,125,125,125,125,125,125,125,14,222,92,224,103,225,110,226,227,99,99,228,14,99,229,230,231,232,32,32,32,32,32,32,128,128,128,128,128,128,128,128,99,234,235,236,238,111,237,130,15,15,237,239,15,101,101,240,241,101,242,237,14,237,243,244,245,14,14,246,247,248,249,15,14,250,251,252,101,134,134,134,134,134,134,134,134,253,254,15,15,15,15,15,15,15,15,15,15,237,255,256,257,92,258,13,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,259,99,260,261,15,262,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,263,264,15,265,266,101,267,33,33,33,33,33,33,33,33,33,33,268,269,271,272,276,277,278,33,33,33,33,33,33,137,137,137,137,137,137,137,137,279,280,281,282,283,284,285,42,42,42,42,42,42,42,42,42,42,14,33,33,33,33,33,33,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,286,288,289,290,42,291,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,44,44,44,44,44,44,44,44,44,44,294,295,296,15,297,298,299,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,301,302,303,304,44,320,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,78,78,322,323,78,324,273,273,325,329,273,312,312,312,312,312,312,312,312,332,326,334,337,78,335,335,323,139,139,273,344,139,140,140,324,327,140,273,326,78,78,78,78,78,78,78,78,78,78,347,139,327,327,377,386,140,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,333,333,333,338,78,324,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,79,79,139,388,79,287,287,140,389,287,391,318,139,318,318,392,393,140,355,356,365,366,140,79,394,395,275,275,287,396,275,293,293,338,318,293,287,397,398,79,79,79,79,79,79,79,79,79,79,275,355,356,365,366,293,369,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,399,406,416,370,79,369,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,89,89,275,370,89,292,292,293,418,292,309,309,275,419,309,420,421,293,371,422,372,373,324,89,423,424,316,316,292,- 1,316,- 1,- 1,309,- 1,309,292,- 1,- 1,89,89,89,89,89,89,89,89,89,89,316,371,316,372,373,- 1,374,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,- 1,- 1,- 1,- 1,89,374,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,89,90,90,300,300,90,- 1,300,305,305,- 1,- 1,305,315,315,315,315,315,315,315,315,- 1,- 1,- 1,90,- 1,300,412,- 1,412,412,305,- 1,- 1,300,- 1,- 1,- 1,- 1,305,90,90,90,90,90,90,90,90,90,90,412,- 1,- 1,- 1,- 1,- 1,- 1,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,- 1,- 1,- 1,- 1,90,- 1,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,97,97,- 1,328,97,328,328,328,328,328,328,328,328,328,328,- 1,- 1,- 1,- 1,354,- 1,354,- 1,97,354,354,354,354,354,354,354,354,354,354,- 1,- 1,- 1,- 1,- 1,97,97,97,97,97,97,97,97,97,97,- 1,- 1,- 1,- 1,- 1,- 1,- 1,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,- 1,- 1,- 1,- 1,97,- 1,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,97,98,98,- 1,- 1,98,357,357,357,357,357,357,357,357,357,357,- 1,- 1,- 1,- 1,360,- 1,360,- 1,98,360,360,360,360,360,360,360,360,360,360,- 1,- 1,- 1,- 1,- 1,98,98,98,98,98,98,98,98,98,98,- 1,- 1,- 1,- 1,- 1,- 1,- 1,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,- 1,- 1,- 1,- 1,98,- 1,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,98,106,106,106,106,106,106,106,106,106,106,106,106,106,- 1,- 1,- 1,- 1,- 1,- 1,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,- 1,- 1,- 1,- 1,106,- 1,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,107,107,107,107,107,107,107,107,107,107,107,107,107,- 1,- 1,- 1,- 1,- 1,- 1,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,- 1,- 1,- 1,- 1,107,- 1,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,108,108,108,108,108,108,108,108,108,108,108,108,108,- 1,- 1,- 1,- 1,- 1,- 1,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,109,- 1,- 1,109,108,- 1,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,- 1,- 1,- 1,- 1,109,109,109,109,109,109,109,109,109,109,109,109,109,- 1,- 1,- 1,- 1,- 1,- 1,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,- 1,- 1,- 1,- 1,109,- 1,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,113,113,113,113,113,113,113,113,113,113,- 1,- 1,- 1,- 1,- 1,- 1,- 1,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,123,- 1,- 1,123,113,- 1,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,- 1,- 1,132,- 1,- 1,132,123,123,123,123,123,123,123,123,126,126,126,126,126,126,126,126,126,126,- 1,- 1,- 1,- 1,- 1,- 1,- 1,126,126,126,126,126,126,- 1,- 1,- 1,132,132,132,132,132,132,132,132,- 1,- 1,- 1,- 1,- 1,- 1,- 1,123,- 1,- 1,109,- 1,390,- 1,- 1,126,126,126,126,126,126,390,390,390,390,390,390,390,390,- 1,123,127,127,127,127,127,127,127,127,127,127,132,- 1,- 1,- 1,- 1,- 1,- 1,127,127,127,127,127,127,135,135,135,135,135,135,135,135,135,135,132,- 1,- 1,- 1,- 1,- 1,- 1,135,135,135,135,135,135,- 1,- 1,- 1,127,127,127,127,127,127,310,- 1,- 1,310,- 1,- 1,136,136,136,136,136,136,136,136,136,136,- 1,135,135,135,135,135,135,136,136,136,136,136,136,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,405,310,310,310,310,310,310,310,310,405,405,405,405,405,405,405,405,- 1,136,136,136,136,136,136,- 1,313,313,313,313,313,313,313,313,313,313,- 1,- 1,- 1,- 1,- 1,- 1,- 1,313,313,313,313,313,313,- 1,- 1,310,361,361,361,361,361,361,361,361,361,361,- 1,- 1,314,314,314,314,314,314,314,314,314,314,310,313,313,313,313,313,313,314,314,314,314,314,314,- 1,- 1,330,- 1,330,330,330,330,330,330,330,330,330,330,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,330,314,314,314,314,314,314,330,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,330,- 1,- 1,330,331,- 1,331,331,331,331,331,331,331,331,331,331,330,- 1,- 1,- 1,- 1,- 1,- 1,330,- 1,- 1,- 1,331,- 1,- 1,- 1,- 1,330,- 1,331,330,- 1,- 1,- 1,- 1,- 1,- 1,- 1,331,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,331,- 1,- 1,- 1,- 1,- 1,- 1,331,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,331,336,336,336,336,336,336,336,336,336,336,- 1,- 1,- 1,- 1,- 1,- 1,- 1,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,- 1,- 1,- 1,- 1,336,- 1,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,336,342,342,342,342,342,342,342,342,342,342,- 1,- 1,- 1,- 1,- 1,- 1,- 1,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,- 1,- 1,- 1,- 1,342,- 1,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,352,352,352,352,352,352,352,352,352,352,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,352,352,- 1,- 1,- 1,- 1,353,352,353,353,353,353,353,353,353,353,353,353,358,358,358,358,358,358,358,358,358,358,- 1,353,- 1,- 1,352,352,- 1,- 1,353,- 1,- 1,352,358,- 1,- 1,- 1,- 1,353,358,- 1,359,359,359,359,359,359,359,359,359,359,- 1,- 1,- 1,353,- 1,- 1,- 1,- 1,- 1,- 1,353,359,359,- 1,358,- 1,- 1,- 1,359,353,358,- 1,- 1,362,362,362,362,362,362,362,362,362,362,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,359,359,362,- 1,- 1,- 1,- 1,359,362,363,- 1,363,363,363,363,363,363,363,363,363,363,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,363,- 1,362,- 1,- 1,- 1,- 1,363,362,- 1,- 1,- 1,- 1,- 1,- 1,364,363,364,364,364,364,364,364,364,364,364,364,- 1,- 1,- 1,- 1,- 1,363,- 1,- 1,- 1,- 1,- 1,364,363,- 1,- 1,- 1,- 1,- 1,364,- 1,- 1,363,- 1,- 1,- 1,- 1,- 1,364,- 1,- 1,378,378,378,378,378,378,378,378,378,378,- 1,- 1,- 1,364,- 1,- 1,- 1,- 1,- 1,- 1,364,378,378,- 1,- 1,- 1,- 1,- 1,378,364,367,367,367,367,367,367,367,367,367,367,- 1,- 1,- 1,- 1,- 1,- 1,- 1,367,367,367,367,367,367,378,378,- 1,- 1,- 1,- 1,- 1,378,- 1,- 1,- 1,- 1,- 1,- 1,- 1,368,368,368,368,368,368,368,368,368,368,- 1,367,367,367,367,367,367,368,368,368,368,368,368,- 1,- 1,- 1,387,- 1,368,- 1,- 1,387,- 1,- 1,- 1,- 1,- 1,368,- 1,- 1,387,387,387,387,387,387,387,387,- 1,368,368,368,368,368,368,387,- 1,- 1,- 1,- 1,368,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,368,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,387,- 1,- 1,- 1,- 1,387,387,- 1,- 1,- 1,387,- 1,- 1,- 1,- 1,- 1,- 1,- 1,387,- 1,- 1,- 1,387,- 1,387,- 1,387,- 1,387,400,400,400,400,400,400,400,400,400,400,- 1,- 1,- 1,- 1,- 1,- 1,- 1,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,- 1,- 1,- 1,- 1,- 1,- 1,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,401,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,401,401,401,401,401,401,401,401,401,401,- 1,- 1,- 1,- 1,- 1,- 1,- 1,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,- 1,- 1,- 1,- 1,- 1,- 1,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,401,413,- 1,- 1,413,- 1,- 1,- 1,413,413,413,413,413,413,413,413,413,413,- 1,- 1,- 1,- 1,- 1,- 1,- 1,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,- 1,- 1,- 1,- 1,413,- 1,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,413,415,- 1,- 1,- 1,- 1,- 1,- 1,- 1,415,415,415,415,415,415,415,415,415,415,- 1,- 1,- 1,- 1,- 1,- 1,- 1,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,- 1,- 1,- 1,- 1,415,- 1,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,415,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1};
int Cyc_lex_engine(int start_state,struct Cyc_Lexing_lexbuf*lbuf){
# 167
int state;int base;int backtrk;
int c;
state=start_state;
# 171
if(state >= 0){
lbuf->lex_last_pos=(lbuf->lex_start_pos=lbuf->lex_curr_pos);
lbuf->lex_last_action=- 1;}else{
# 175
state=(- state)- 1;}
# 177
while(1){
base=Cyc_lex_base[_check_known_subscript_notnull(425,state)];
if(base < 0)return(- base)- 1;
backtrk=Cyc_lex_backtrk[_check_known_subscript_notnull(425,state)];
if(backtrk >= 0){
lbuf->lex_last_pos=lbuf->lex_curr_pos;
lbuf->lex_last_action=backtrk;}
# 185
if(lbuf->lex_curr_pos >= lbuf->lex_buffer_len){
if(!lbuf->lex_eof_reached)
return(- state)- 1;else{
# 189
c=256;}}else{
# 191
c=(int)*((char*)_check_dyneither_subscript(lbuf->lex_buffer,sizeof(char),lbuf->lex_curr_pos ++));
if(c == - 1)c=256;}
# 194
if(Cyc_lex_check[_check_known_subscript_notnull(3481,base + c)]== state)
state=Cyc_lex_trans[_check_known_subscript_notnull(3481,base + c)];else{
# 197
state=Cyc_lex_default[_check_known_subscript_notnull(425,state)];}
if(state < 0){
lbuf->lex_curr_pos=lbuf->lex_last_pos;
if(lbuf->lex_last_action == - 1)
(int)_throw((void*)({struct Cyc_Lexing_Error_exn_struct*_tmp4=_cycalloc(sizeof(*_tmp4));_tmp4[0]=({struct Cyc_Lexing_Error_exn_struct _tmp5;_tmp5.tag=Cyc_Lexing_Error;_tmp5.f1=({const char*_tmp6="empty token";_tag_dyneither(_tmp6,sizeof(char),12);});_tmp5;});_tmp4;}));else{
# 203
return lbuf->lex_last_action;}}else{
# 206
if(c == 256)lbuf->lex_eof_reached=0;}}}
# 210
struct _tuple20*Cyc_line_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){
lexstate=Cyc_lex_engine(lexstate,lexbuf);
{int _tmp7=lexstate;switch(_tmp7){case 0: _LL1: _LL2:
# 169 "buildlib.cyl"
 Cyc_macroname(lexbuf);
for(0;Cyc_current_args != 0;Cyc_current_args=((struct Cyc_List_List*)_check_null(Cyc_current_args))->tl){
Cyc_current_targets=({struct Cyc_Set_Set**_tmp8=_cycalloc(sizeof(*_tmp8));_tmp8[0]=((struct Cyc_Set_Set*(*)(struct Cyc_Set_Set*s,struct _dyneither_ptr*elt))Cyc_Set_delete)(*((struct Cyc_Set_Set**)_check_null(Cyc_current_targets)),(struct _dyneither_ptr*)((struct Cyc_List_List*)_check_null(Cyc_current_args))->hd);_tmp8;});}
# 174
return({struct _tuple20*_tmp9=_cycalloc(sizeof(*_tmp9));_tmp9->f1=(struct _dyneither_ptr*)_check_null(Cyc_current_source);_tmp9->f2=*((struct Cyc_Set_Set**)_check_null(Cyc_current_targets));_tmp9;});case 1: _LL3: _LL4:
# 177 "buildlib.cyl"
 return Cyc_line(lexbuf);case 2: _LL5: _LL6:
# 179
 return 0;default: _LL7: _LL8:
(lexbuf->refill_buff)(lexbuf);
return Cyc_line_rec(lexbuf,lexstate);}_LL0:;}
# 183
(int)_throw((void*)({struct Cyc_Lexing_Error_exn_struct*_tmpA=_cycalloc(sizeof(*_tmpA));_tmpA[0]=({struct Cyc_Lexing_Error_exn_struct _tmpB;_tmpB.tag=Cyc_Lexing_Error;_tmpB.f1=({const char*_tmpC="some action didn't return!";_tag_dyneither(_tmpC,sizeof(char),27);});_tmpB;});_tmpA;}));}
# 185
struct _tuple20*Cyc_line(struct Cyc_Lexing_lexbuf*lexbuf){return Cyc_line_rec(lexbuf,0);}
int Cyc_macroname_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){
lexstate=Cyc_lex_engine(lexstate,lexbuf);
{int _tmpD=lexstate;switch(_tmpD){case 0: _LLA: _LLB:
# 183 "buildlib.cyl"
 Cyc_current_source=({struct _dyneither_ptr*_tmpE=_cycalloc(sizeof(*_tmpE));_tmpE[0]=(struct _dyneither_ptr)Cyc_substring((struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf),0,(unsigned long)((
Cyc_Lexing_lexeme_end(lexbuf)- Cyc_Lexing_lexeme_start(lexbuf))- 2));_tmpE;});
Cyc_current_args=0;
Cyc_current_targets=({struct Cyc_Set_Set**_tmpF=_cycalloc(sizeof(*_tmpF));_tmpF[0]=((struct Cyc_Set_Set*(*)(int(*cmp)(struct _dyneither_ptr*,struct _dyneither_ptr*)))Cyc_Set_empty)(Cyc_strptrcmp);_tmpF;});
Cyc_token(lexbuf);
return 0;case 1: _LLC: _LLD:
# 191
 Cyc_current_source=({struct _dyneither_ptr*_tmp10=_cycalloc(sizeof(*_tmp10));_tmp10[0]=(struct _dyneither_ptr)Cyc_substring((struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf),0,(unsigned long)((
Cyc_Lexing_lexeme_end(lexbuf)- Cyc_Lexing_lexeme_start(lexbuf))- 1));_tmp10;});
Cyc_current_args=0;
Cyc_current_targets=({struct Cyc_Set_Set**_tmp11=_cycalloc(sizeof(*_tmp11));_tmp11[0]=((struct Cyc_Set_Set*(*)(int(*cmp)(struct _dyneither_ptr*,struct _dyneither_ptr*)))Cyc_Set_empty)(Cyc_strptrcmp);_tmp11;});
Cyc_args(lexbuf);
return 0;case 2: _LLE: _LLF:
# 199
 Cyc_current_source=({struct _dyneither_ptr*_tmp12=_cycalloc(sizeof(*_tmp12));_tmp12[0]=(struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf);_tmp12;});
Cyc_current_args=0;
Cyc_current_targets=({struct Cyc_Set_Set**_tmp13=_cycalloc(sizeof(*_tmp13));_tmp13[0]=((struct Cyc_Set_Set*(*)(int(*cmp)(struct _dyneither_ptr*,struct _dyneither_ptr*)))Cyc_Set_empty)(Cyc_strptrcmp);_tmp13;});
Cyc_token(lexbuf);
return 0;default: _LL10: _LL11:
# 205
(lexbuf->refill_buff)(lexbuf);
return Cyc_macroname_rec(lexbuf,lexstate);}_LL9:;}
# 208
(int)_throw((void*)({struct Cyc_Lexing_Error_exn_struct*_tmp14=_cycalloc(sizeof(*_tmp14));_tmp14[0]=({struct Cyc_Lexing_Error_exn_struct _tmp15;_tmp15.tag=Cyc_Lexing_Error;_tmp15.f1=({const char*_tmp16="some action didn't return!";_tag_dyneither(_tmp16,sizeof(char),27);});_tmp15;});_tmp14;}));}
# 210
int Cyc_macroname(struct Cyc_Lexing_lexbuf*lexbuf){return Cyc_macroname_rec(lexbuf,1);}
int Cyc_args_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){
lexstate=Cyc_lex_engine(lexstate,lexbuf);
{int _tmp17=lexstate;switch(_tmp17){case 0: _LL13: _LL14: {
# 208 "buildlib.cyl"
struct _dyneither_ptr*_tmp18=({struct _dyneither_ptr*_tmp1A=_cycalloc(sizeof(*_tmp1A));_tmp1A[0]=(struct _dyneither_ptr)Cyc_substring((struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf),0,(unsigned long)((
Cyc_Lexing_lexeme_end(lexbuf)- Cyc_Lexing_lexeme_start(lexbuf))- 2));_tmp1A;});
Cyc_current_args=({struct Cyc_List_List*_tmp19=_cycalloc(sizeof(*_tmp19));_tmp19->hd=_tmp18;_tmp19->tl=Cyc_current_args;_tmp19;});
return Cyc_args(lexbuf);}case 1: _LL15: _LL16: {
# 214
struct _dyneither_ptr*_tmp1B=({struct _dyneither_ptr*_tmp1D=_cycalloc(sizeof(*_tmp1D));_tmp1D[0]=(struct _dyneither_ptr)Cyc_substring((struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf),0,(unsigned long)((
Cyc_Lexing_lexeme_end(lexbuf)- Cyc_Lexing_lexeme_start(lexbuf))- 1));_tmp1D;});
Cyc_current_args=({struct Cyc_List_List*_tmp1C=_cycalloc(sizeof(*_tmp1C));_tmp1C->hd=_tmp1B;_tmp1C->tl=Cyc_current_args;_tmp1C;});
return Cyc_args(lexbuf);}case 2: _LL17: _LL18: {
# 220
struct _dyneither_ptr*_tmp1E=({struct _dyneither_ptr*_tmp20=_cycalloc(sizeof(*_tmp20));_tmp20[0]=(struct _dyneither_ptr)Cyc_substring((struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf),0,(unsigned long)((
Cyc_Lexing_lexeme_end(lexbuf)- Cyc_Lexing_lexeme_start(lexbuf))- 1));_tmp20;});
Cyc_current_args=({struct Cyc_List_List*_tmp1F=_cycalloc(sizeof(*_tmp1F));_tmp1F->hd=_tmp1E;_tmp1F->tl=Cyc_current_args;_tmp1F;});
return Cyc_token(lexbuf);}default: _LL19: _LL1A:
# 225
(lexbuf->refill_buff)(lexbuf);
return Cyc_args_rec(lexbuf,lexstate);}_LL12:;}
# 228
(int)_throw((void*)({struct Cyc_Lexing_Error_exn_struct*_tmp21=_cycalloc(sizeof(*_tmp21));_tmp21[0]=({struct Cyc_Lexing_Error_exn_struct _tmp22;_tmp22.tag=Cyc_Lexing_Error;_tmp22.f1=({const char*_tmp23="some action didn't return!";_tag_dyneither(_tmp23,sizeof(char),27);});_tmp22;});_tmp21;}));}
# 230
int Cyc_args(struct Cyc_Lexing_lexbuf*lexbuf){return Cyc_args_rec(lexbuf,2);}
int Cyc_token_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){
lexstate=Cyc_lex_engine(lexstate,lexbuf);
{int _tmp24=lexstate;switch(_tmp24){case 0: _LL1C: _LL1D:
# 229 "buildlib.cyl"
 Cyc_add_target(({struct _dyneither_ptr*_tmp25=_cycalloc(sizeof(*_tmp25));_tmp25[0]=(struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf);_tmp25;}));return Cyc_token(lexbuf);case 1: _LL1E: _LL1F:
# 232 "buildlib.cyl"
 return 0;case 2: _LL20: _LL21:
# 235 "buildlib.cyl"
 return Cyc_token(lexbuf);case 3: _LL22: _LL23:
# 238 "buildlib.cyl"
 Cyc_string(lexbuf);return Cyc_token(lexbuf);case 4: _LL24: _LL25:
# 241 "buildlib.cyl"
 return Cyc_token(lexbuf);case 5: _LL26: _LL27:
# 243
 return Cyc_token(lexbuf);case 6: _LL28: _LL29:
# 245
 return Cyc_token(lexbuf);case 7: _LL2A: _LL2B:
# 247
 return Cyc_token(lexbuf);case 8: _LL2C: _LL2D:
# 250 "buildlib.cyl"
 return Cyc_token(lexbuf);case 9: _LL2E: _LL2F:
# 253 "buildlib.cyl"
 return Cyc_token(lexbuf);case 10: _LL30: _LL31:
# 256 "buildlib.cyl"
 return Cyc_token(lexbuf);case 11: _LL32: _LL33:
# 258
 return Cyc_token(lexbuf);case 12: _LL34: _LL35:
# 260
 return Cyc_token(lexbuf);case 13: _LL36: _LL37:
# 262
 return Cyc_token(lexbuf);case 14: _LL38: _LL39:
# 264
 return Cyc_token(lexbuf);case 15: _LL3A: _LL3B:
# 266
 return Cyc_token(lexbuf);case 16: _LL3C: _LL3D:
# 268
 return Cyc_token(lexbuf);case 17: _LL3E: _LL3F:
# 270
 return Cyc_token(lexbuf);case 18: _LL40: _LL41:
# 272
 return Cyc_token(lexbuf);case 19: _LL42: _LL43:
# 274
 return Cyc_token(lexbuf);case 20: _LL44: _LL45:
# 276
 return Cyc_token(lexbuf);case 21: _LL46: _LL47:
# 278
 return Cyc_token(lexbuf);case 22: _LL48: _LL49:
# 280
 return Cyc_token(lexbuf);case 23: _LL4A: _LL4B:
# 282
 return Cyc_token(lexbuf);case 24: _LL4C: _LL4D:
# 285 "buildlib.cyl"
 return Cyc_token(lexbuf);case 25: _LL4E: _LL4F:
# 287
 return Cyc_token(lexbuf);case 26: _LL50: _LL51:
# 289
 return Cyc_token(lexbuf);case 27: _LL52: _LL53:
# 291
 return Cyc_token(lexbuf);case 28: _LL54: _LL55:
# 293
 return Cyc_token(lexbuf);case 29: _LL56: _LL57:
# 295
 return Cyc_token(lexbuf);case 30: _LL58: _LL59:
# 297
 return Cyc_token(lexbuf);case 31: _LL5A: _LL5B:
# 299
 return Cyc_token(lexbuf);case 32: _LL5C: _LL5D:
# 301
 return Cyc_token(lexbuf);case 33: _LL5E: _LL5F:
# 303
 return Cyc_token(lexbuf);case 34: _LL60: _LL61:
# 305
 return Cyc_token(lexbuf);case 35: _LL62: _LL63:
# 307
 return Cyc_token(lexbuf);case 36: _LL64: _LL65:
# 309
 return Cyc_token(lexbuf);case 37: _LL66: _LL67:
# 311
 return Cyc_token(lexbuf);case 38: _LL68: _LL69:
# 313
 return Cyc_token(lexbuf);case 39: _LL6A: _LL6B:
# 315
 return Cyc_token(lexbuf);case 40: _LL6C: _LL6D:
# 317
 return Cyc_token(lexbuf);case 41: _LL6E: _LL6F:
# 319
 return Cyc_token(lexbuf);case 42: _LL70: _LL71:
# 321
 return Cyc_token(lexbuf);case 43: _LL72: _LL73:
# 323
 return Cyc_token(lexbuf);case 44: _LL74: _LL75:
# 325
 return Cyc_token(lexbuf);case 45: _LL76: _LL77:
# 327
 return Cyc_token(lexbuf);case 46: _LL78: _LL79:
# 329
 return Cyc_token(lexbuf);case 47: _LL7A: _LL7B:
# 331
 return Cyc_token(lexbuf);case 48: _LL7C: _LL7D:
# 334 "buildlib.cyl"
 return Cyc_token(lexbuf);default: _LL7E: _LL7F:
(lexbuf->refill_buff)(lexbuf);
return Cyc_token_rec(lexbuf,lexstate);}_LL1B:;}
# 338
(int)_throw((void*)({struct Cyc_Lexing_Error_exn_struct*_tmp26=_cycalloc(sizeof(*_tmp26));_tmp26[0]=({struct Cyc_Lexing_Error_exn_struct _tmp27;_tmp27.tag=Cyc_Lexing_Error;_tmp27.f1=({const char*_tmp28="some action didn't return!";_tag_dyneither(_tmp28,sizeof(char),27);});_tmp27;});_tmp26;}));}
# 340
int Cyc_token(struct Cyc_Lexing_lexbuf*lexbuf){return Cyc_token_rec(lexbuf,3);}
int Cyc_string_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){
lexstate=Cyc_lex_engine(lexstate,lexbuf);
{int _tmp29=lexstate;switch(_tmp29){case 0: _LL81: _LL82:
# 339 "buildlib.cyl"
 return Cyc_string(lexbuf);case 1: _LL83: _LL84:
# 340 "buildlib.cyl"
 return 0;case 2: _LL85: _LL86:
# 341 "buildlib.cyl"
 return Cyc_string(lexbuf);case 3: _LL87: _LL88:
# 342 "buildlib.cyl"
 return Cyc_string(lexbuf);case 4: _LL89: _LL8A:
# 345 "buildlib.cyl"
 return Cyc_string(lexbuf);case 5: _LL8B: _LL8C:
# 348 "buildlib.cyl"
 return Cyc_string(lexbuf);case 6: _LL8D: _LL8E:
# 350
 return Cyc_string(lexbuf);case 7: _LL8F: _LL90:
# 351 "buildlib.cyl"
 return 0;case 8: _LL91: _LL92:
# 352 "buildlib.cyl"
 return 0;case 9: _LL93: _LL94:
# 353 "buildlib.cyl"
 return Cyc_string(lexbuf);default: _LL95: _LL96:
(lexbuf->refill_buff)(lexbuf);
return Cyc_string_rec(lexbuf,lexstate);}_LL80:;}
# 357
(int)_throw((void*)({struct Cyc_Lexing_Error_exn_struct*_tmp2A=_cycalloc(sizeof(*_tmp2A));_tmp2A[0]=({struct Cyc_Lexing_Error_exn_struct _tmp2B;_tmp2B.tag=Cyc_Lexing_Error;_tmp2B.f1=({const char*_tmp2C="some action didn't return!";_tag_dyneither(_tmp2C,sizeof(char),27);});_tmp2B;});_tmp2A;}));}
# 359
int Cyc_string(struct Cyc_Lexing_lexbuf*lexbuf){return Cyc_string_rec(lexbuf,4);}
int Cyc_slurp_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){
lexstate=Cyc_lex_engine(lexstate,lexbuf);
{int _tmp2D=lexstate;switch(_tmp2D){case 0: _LL98: _LL99:
# 362 "buildlib.cyl"
 return 0;case 1: _LL9A: _LL9B:
# 364
 Cyc_fputc((int)'"',(struct Cyc___cycFILE*)_check_null(Cyc_slurp_out));
while(Cyc_slurp_string(lexbuf)){;}
return 1;case 2: _LL9C: _LL9D:
# 371 "buildlib.cyl"
 Cyc_fputs("*__IGNORE_FOR_CYCLONE_MALLOC(",(struct Cyc___cycFILE*)_check_null(Cyc_slurp_out));
({void*_tmp2E=0;Cyc_log(({const char*_tmp2F="Warning: declaration of malloc sidestepped\n";_tag_dyneither(_tmp2F,sizeof(char),44);}),_tag_dyneither(_tmp2E,sizeof(void*),0));});
return 1;case 3: _LL9E: _LL9F:
# 377 "buildlib.cyl"
 Cyc_fputs(" __IGNORE_FOR_CYCLONE_MALLOC(",(struct Cyc___cycFILE*)_check_null(Cyc_slurp_out));
({void*_tmp30=0;Cyc_log(({const char*_tmp31="Warning: declaration of malloc sidestepped\n";_tag_dyneither(_tmp31,sizeof(char),44);}),_tag_dyneither(_tmp30,sizeof(void*),0));});
return 1;case 4: _LLA0: _LLA1:
# 383 "buildlib.cyl"
 Cyc_fputs("*__IGNORE_FOR_CYCLONE_CALLOC(",(struct Cyc___cycFILE*)_check_null(Cyc_slurp_out));
({void*_tmp32=0;Cyc_log(({const char*_tmp33="Warning: declaration of calloc sidestepped\n";_tag_dyneither(_tmp33,sizeof(char),44);}),_tag_dyneither(_tmp32,sizeof(void*),0));});
return 1;case 5: _LLA2: _LLA3:
# 389 "buildlib.cyl"
 Cyc_fputs(" __IGNORE_FOR_CYCLONE_CALLOC(",(struct Cyc___cycFILE*)_check_null(Cyc_slurp_out));
({void*_tmp34=0;Cyc_log(({const char*_tmp35="Warning: declaration of calloc sidestepped\n";_tag_dyneither(_tmp35,sizeof(char),44);}),_tag_dyneither(_tmp34,sizeof(void*),0));});
return 1;case 6: _LLA4: _LLA5:
# 393
 Cyc_fputs("__region",(struct Cyc___cycFILE*)_check_null(Cyc_slurp_out));
({void*_tmp36=0;Cyc_log(({const char*_tmp37="Warning: use of region sidestepped\n";_tag_dyneither(_tmp37,sizeof(char),36);}),_tag_dyneither(_tmp36,sizeof(void*),0));});
return 1;case 7: _LLA6: _LLA7:
# 397
({void*_tmp38=0;Cyc_log(({const char*_tmp39="Warning: use of __extension__ deleted\n";_tag_dyneither(_tmp39,sizeof(char),39);}),_tag_dyneither(_tmp38,sizeof(void*),0));});
return 1;case 8: _LLA8: _LLA9:
# 402 "buildlib.cyl"
({void*_tmp3A=0;Cyc_log(({const char*_tmp3B="Warning: use of mode HI deleted\n";_tag_dyneither(_tmp3B,sizeof(char),33);}),_tag_dyneither(_tmp3A,sizeof(void*),0));});
return 1;case 9: _LLAA: _LLAB:
# 405
({void*_tmp3C=0;Cyc_log(({const char*_tmp3D="Warning: use of mode SI deleted\n";_tag_dyneither(_tmp3D,sizeof(char),33);}),_tag_dyneither(_tmp3C,sizeof(void*),0));});
return 1;case 10: _LLAC: _LLAD:
# 408
({void*_tmp3E=0;Cyc_log(({const char*_tmp3F="Warning: use of mode QI deleted\n";_tag_dyneither(_tmp3F,sizeof(char),33);}),_tag_dyneither(_tmp3E,sizeof(void*),0));});
return 1;case 11: _LLAE: _LLAF:
# 411
({void*_tmp40=0;Cyc_log(({const char*_tmp41="Warning: use of mode DI deleted\n";_tag_dyneither(_tmp41,sizeof(char),33);}),_tag_dyneither(_tmp40,sizeof(void*),0));});
return 1;case 12: _LLB0: _LLB1:
# 414
({void*_tmp42=0;Cyc_log(({const char*_tmp43="Warning: use of mode DI deleted\n";_tag_dyneither(_tmp43,sizeof(char),33);}),_tag_dyneither(_tmp42,sizeof(void*),0));});
return 1;case 13: _LLB2: _LLB3:
# 417
({void*_tmp44=0;Cyc_log(({const char*_tmp45="Warning: use of mode word deleted\n";_tag_dyneither(_tmp45,sizeof(char),35);}),_tag_dyneither(_tmp44,sizeof(void*),0));});
return 1;case 14: _LLB4: _LLB5:
# 420
 Cyc_fputs("inline",(struct Cyc___cycFILE*)_check_null(Cyc_slurp_out));return 1;case 15: _LLB6: _LLB7:
# 422
 Cyc_fputs("inline",(struct Cyc___cycFILE*)_check_null(Cyc_slurp_out));return 1;case 16: _LLB8: _LLB9:
# 424
 Cyc_fputs("const",(struct Cyc___cycFILE*)_check_null(Cyc_slurp_out));return 1;case 17: _LLBA: _LLBB:
# 426
 Cyc_fputs("const",(struct Cyc___cycFILE*)_check_null(Cyc_slurp_out));return 1;case 18: _LLBC: _LLBD:
# 431 "buildlib.cyl"
 Cyc_fputs("int",(struct Cyc___cycFILE*)_check_null(Cyc_slurp_out));return 1;case 19: _LLBE: _LLBF:
# 433
 return 1;case 20: _LLC0: _LLC1:
# 435
 Cyc_parens_to_match=1;
while(Cyc_asmtok(lexbuf)){;}
Cyc_fputs("0",(struct Cyc___cycFILE*)_check_null(Cyc_slurp_out));
({void*_tmp46=0;Cyc_log(({const char*_tmp47="Warning: replacing use of __asm__ with 0\n";_tag_dyneither(_tmp47,sizeof(char),42);}),_tag_dyneither(_tmp46,sizeof(void*),0));});
return 1;case 21: _LLC2: _LLC3:
# 441
 Cyc_fputc((int)Cyc_Lexing_lexeme_char(lexbuf,0),(struct Cyc___cycFILE*)_check_null(Cyc_slurp_out));return 1;default: _LLC4: _LLC5:
(lexbuf->refill_buff)(lexbuf);
return Cyc_slurp_rec(lexbuf,lexstate);}_LL97:;}
# 445
(int)_throw((void*)({struct Cyc_Lexing_Error_exn_struct*_tmp48=_cycalloc(sizeof(*_tmp48));_tmp48[0]=({struct Cyc_Lexing_Error_exn_struct _tmp49;_tmp49.tag=Cyc_Lexing_Error;_tmp49.f1=({const char*_tmp4A="some action didn't return!";_tag_dyneither(_tmp4A,sizeof(char),27);});_tmp49;});_tmp48;}));}
# 447
int Cyc_slurp(struct Cyc_Lexing_lexbuf*lexbuf){return Cyc_slurp_rec(lexbuf,5);}
int Cyc_slurp_string_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){
lexstate=Cyc_lex_engine(lexstate,lexbuf);
{int _tmp4B=lexstate;switch(_tmp4B){case 0: _LLC7: _LLC8:
# 445 "buildlib.cyl"
 return 0;case 1: _LLC9: _LLCA:
# 447
 Cyc_fputc((int)'"',(struct Cyc___cycFILE*)_check_null(Cyc_slurp_out));return 0;case 2: _LLCB: _LLCC:
# 449
({void*_tmp4C=0;Cyc_log(({const char*_tmp4D="Warning: unclosed string\n";_tag_dyneither(_tmp4D,sizeof(char),26);}),_tag_dyneither(_tmp4C,sizeof(void*),0));});
({struct Cyc_String_pa_PrintArg_struct _tmp50;_tmp50.tag=0;_tmp50.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf));({void*_tmp4E[1]={& _tmp50};Cyc_fprintf((struct Cyc___cycFILE*)_check_null(Cyc_slurp_out),({const char*_tmp4F="%s";_tag_dyneither(_tmp4F,sizeof(char),3);}),_tag_dyneither(_tmp4E,sizeof(void*),1));});});return 1;case 3: _LLCD: _LLCE:
# 452
({struct Cyc_String_pa_PrintArg_struct _tmp53;_tmp53.tag=0;_tmp53.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf));({void*_tmp51[1]={& _tmp53};Cyc_fprintf((struct Cyc___cycFILE*)_check_null(Cyc_slurp_out),({const char*_tmp52="%s";_tag_dyneither(_tmp52,sizeof(char),3);}),_tag_dyneither(_tmp51,sizeof(void*),1));});});return 1;case 4: _LLCF: _LLD0:
# 454
({struct Cyc_String_pa_PrintArg_struct _tmp56;_tmp56.tag=0;_tmp56.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf));({void*_tmp54[1]={& _tmp56};Cyc_fprintf((struct Cyc___cycFILE*)_check_null(Cyc_slurp_out),({const char*_tmp55="%s";_tag_dyneither(_tmp55,sizeof(char),3);}),_tag_dyneither(_tmp54,sizeof(void*),1));});});return 1;case 5: _LLD1: _LLD2:
# 456
({struct Cyc_String_pa_PrintArg_struct _tmp59;_tmp59.tag=0;_tmp59.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf));({void*_tmp57[1]={& _tmp59};Cyc_fprintf((struct Cyc___cycFILE*)_check_null(Cyc_slurp_out),({const char*_tmp58="%s";_tag_dyneither(_tmp58,sizeof(char),3);}),_tag_dyneither(_tmp57,sizeof(void*),1));});});return 1;case 6: _LLD3: _LLD4:
# 458
({struct Cyc_String_pa_PrintArg_struct _tmp5C;_tmp5C.tag=0;_tmp5C.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf));({void*_tmp5A[1]={& _tmp5C};Cyc_fprintf((struct Cyc___cycFILE*)_check_null(Cyc_slurp_out),({const char*_tmp5B="%s";_tag_dyneither(_tmp5B,sizeof(char),3);}),_tag_dyneither(_tmp5A,sizeof(void*),1));});});return 1;case 7: _LLD5: _LLD6:
# 460
({struct Cyc_String_pa_PrintArg_struct _tmp5F;_tmp5F.tag=0;_tmp5F.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf));({void*_tmp5D[1]={& _tmp5F};Cyc_fprintf((struct Cyc___cycFILE*)_check_null(Cyc_slurp_out),({const char*_tmp5E="%s";_tag_dyneither(_tmp5E,sizeof(char),3);}),_tag_dyneither(_tmp5D,sizeof(void*),1));});});return 1;case 8: _LLD7: _LLD8:
# 462
({struct Cyc_String_pa_PrintArg_struct _tmp62;_tmp62.tag=0;_tmp62.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf));({void*_tmp60[1]={& _tmp62};Cyc_fprintf((struct Cyc___cycFILE*)_check_null(Cyc_slurp_out),({const char*_tmp61="%s";_tag_dyneither(_tmp61,sizeof(char),3);}),_tag_dyneither(_tmp60,sizeof(void*),1));});});return 1;default: _LLD9: _LLDA:
(lexbuf->refill_buff)(lexbuf);
return Cyc_slurp_string_rec(lexbuf,lexstate);}_LLC6:;}
# 466
(int)_throw((void*)({struct Cyc_Lexing_Error_exn_struct*_tmp63=_cycalloc(sizeof(*_tmp63));_tmp63[0]=({struct Cyc_Lexing_Error_exn_struct _tmp64;_tmp64.tag=Cyc_Lexing_Error;_tmp64.f1=({const char*_tmp65="some action didn't return!";_tag_dyneither(_tmp65,sizeof(char),27);});_tmp64;});_tmp63;}));}
# 468
int Cyc_slurp_string(struct Cyc_Lexing_lexbuf*lexbuf){return Cyc_slurp_string_rec(lexbuf,6);}
int Cyc_asmtok_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){
lexstate=Cyc_lex_engine(lexstate,lexbuf);
{int _tmp66=lexstate;switch(_tmp66){case 0: _LLDC: _LLDD:
# 472 "buildlib.cyl"
 return 0;case 1: _LLDE: _LLDF:
# 474
 if(Cyc_parens_to_match == 1)return 0;
-- Cyc_parens_to_match;
return 1;case 2: _LLE0: _LLE1:
# 478
 ++ Cyc_parens_to_match;
return 1;case 3: _LLE2: _LLE3:
# 481
 while(Cyc_asm_string(lexbuf)){;}
return 1;case 4: _LLE4: _LLE5:
# 484
 while(Cyc_asm_comment(lexbuf)){;}
return 1;case 5: _LLE6: _LLE7:
# 487
 return 1;case 6: _LLE8: _LLE9:
# 489
 return 1;default: _LLEA: _LLEB:
(lexbuf->refill_buff)(lexbuf);
return Cyc_asmtok_rec(lexbuf,lexstate);}_LLDB:;}
# 493
(int)_throw((void*)({struct Cyc_Lexing_Error_exn_struct*_tmp67=_cycalloc(sizeof(*_tmp67));_tmp67[0]=({struct Cyc_Lexing_Error_exn_struct _tmp68;_tmp68.tag=Cyc_Lexing_Error;_tmp68.f1=({const char*_tmp69="some action didn't return!";_tag_dyneither(_tmp69,sizeof(char),27);});_tmp68;});_tmp67;}));}
# 495
int Cyc_asmtok(struct Cyc_Lexing_lexbuf*lexbuf){return Cyc_asmtok_rec(lexbuf,7);}
int Cyc_asm_string_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){
lexstate=Cyc_lex_engine(lexstate,lexbuf);
{int _tmp6A=lexstate;switch(_tmp6A){case 0: _LLED: _LLEE:
# 493 "buildlib.cyl"
({void*_tmp6B=0;Cyc_log(({const char*_tmp6C="Warning: unclosed string\n";_tag_dyneither(_tmp6C,sizeof(char),26);}),_tag_dyneither(_tmp6B,sizeof(void*),0));});return 0;case 1: _LLEF: _LLF0:
# 495
 return 0;case 2: _LLF1: _LLF2:
# 497
({void*_tmp6D=0;Cyc_log(({const char*_tmp6E="Warning: unclosed string\n";_tag_dyneither(_tmp6E,sizeof(char),26);}),_tag_dyneither(_tmp6D,sizeof(void*),0));});return 1;case 3: _LLF3: _LLF4:
# 499
 return 1;case 4: _LLF5: _LLF6:
# 501
 return 1;case 5: _LLF7: _LLF8:
# 503
 return 1;case 6: _LLF9: _LLFA:
# 505
 return 1;case 7: _LLFB: _LLFC:
# 507
 return 1;case 8: _LLFD: _LLFE:
# 509
 return 1;default: _LLFF: _LL100:
(lexbuf->refill_buff)(lexbuf);
return Cyc_asm_string_rec(lexbuf,lexstate);}_LLEC:;}
# 513
(int)_throw((void*)({struct Cyc_Lexing_Error_exn_struct*_tmp6F=_cycalloc(sizeof(*_tmp6F));_tmp6F[0]=({struct Cyc_Lexing_Error_exn_struct _tmp70;_tmp70.tag=Cyc_Lexing_Error;_tmp70.f1=({const char*_tmp71="some action didn't return!";_tag_dyneither(_tmp71,sizeof(char),27);});_tmp70;});_tmp6F;}));}
# 515
int Cyc_asm_string(struct Cyc_Lexing_lexbuf*lexbuf){return Cyc_asm_string_rec(lexbuf,8);}
int Cyc_asm_comment_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){
lexstate=Cyc_lex_engine(lexstate,lexbuf);
{int _tmp72=lexstate;switch(_tmp72){case 0: _LL102: _LL103:
# 513 "buildlib.cyl"
({void*_tmp73=0;Cyc_log(({const char*_tmp74="Warning: unclosed comment\n";_tag_dyneither(_tmp74,sizeof(char),27);}),_tag_dyneither(_tmp73,sizeof(void*),0));});return 0;case 1: _LL104: _LL105:
# 515
 return 0;case 2: _LL106: _LL107:
# 517
 return 1;default: _LL108: _LL109:
(lexbuf->refill_buff)(lexbuf);
return Cyc_asm_comment_rec(lexbuf,lexstate);}_LL101:;}
# 521
(int)_throw((void*)({struct Cyc_Lexing_Error_exn_struct*_tmp75=_cycalloc(sizeof(*_tmp75));_tmp75[0]=({struct Cyc_Lexing_Error_exn_struct _tmp76;_tmp76.tag=Cyc_Lexing_Error;_tmp76.f1=({const char*_tmp77="some action didn't return!";_tag_dyneither(_tmp77,sizeof(char),27);});_tmp76;});_tmp75;}));}
# 523
int Cyc_asm_comment(struct Cyc_Lexing_lexbuf*lexbuf){return Cyc_asm_comment_rec(lexbuf,9);}
struct _tuple21*Cyc_suck_line_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){
lexstate=Cyc_lex_engine(lexstate,lexbuf);
{int _tmp78=lexstate;switch(_tmp78){case 0: _LL10B: _LL10C:
# 525 "buildlib.cyl"
 Cyc_current_line=({const char*_tmp79="#define ";_tag_dyneither(_tmp79,sizeof(char),9);});
Cyc_suck_macroname(lexbuf);
return({struct _tuple21*_tmp7A=_cycalloc(sizeof(*_tmp7A));_tmp7A->f1=Cyc_current_line;_tmp7A->f2=(struct _dyneither_ptr*)_check_null(Cyc_current_source);_tmp7A;});case 1: _LL10D: _LL10E:
# 529
 return Cyc_suck_line(lexbuf);case 2: _LL10F: _LL110:
# 531
 return 0;default: _LL111: _LL112:
(lexbuf->refill_buff)(lexbuf);
return Cyc_suck_line_rec(lexbuf,lexstate);}_LL10A:;}
# 535
(int)_throw((void*)({struct Cyc_Lexing_Error_exn_struct*_tmp7B=_cycalloc(sizeof(*_tmp7B));_tmp7B[0]=({struct Cyc_Lexing_Error_exn_struct _tmp7C;_tmp7C.tag=Cyc_Lexing_Error;_tmp7C.f1=({const char*_tmp7D="some action didn't return!";_tag_dyneither(_tmp7D,sizeof(char),27);});_tmp7C;});_tmp7B;}));}
# 537
struct _tuple21*Cyc_suck_line(struct Cyc_Lexing_lexbuf*lexbuf){return Cyc_suck_line_rec(lexbuf,10);}
int Cyc_suck_macroname_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){
lexstate=Cyc_lex_engine(lexstate,lexbuf);
{int _tmp7E=lexstate;if(_tmp7E == 0){_LL114: _LL115:
# 535 "buildlib.cyl"
 Cyc_current_source=({struct _dyneither_ptr*_tmp7F=_cycalloc(sizeof(*_tmp7F));_tmp7F[0]=(struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf);_tmp7F;});
Cyc_current_line=(struct _dyneither_ptr)Cyc_strconcat((struct _dyneither_ptr)Cyc_current_line,(struct _dyneither_ptr)*((struct _dyneither_ptr*)_check_null(Cyc_current_source)));
return Cyc_suck_restofline(lexbuf);}else{_LL116: _LL117:
# 539
(lexbuf->refill_buff)(lexbuf);
return Cyc_suck_macroname_rec(lexbuf,lexstate);}_LL113:;}
# 542
(int)_throw((void*)({struct Cyc_Lexing_Error_exn_struct*_tmp80=_cycalloc(sizeof(*_tmp80));_tmp80[0]=({struct Cyc_Lexing_Error_exn_struct _tmp81;_tmp81.tag=Cyc_Lexing_Error;_tmp81.f1=({const char*_tmp82="some action didn't return!";_tag_dyneither(_tmp82,sizeof(char),27);});_tmp81;});_tmp80;}));}
# 544
int Cyc_suck_macroname(struct Cyc_Lexing_lexbuf*lexbuf){return Cyc_suck_macroname_rec(lexbuf,11);}
int Cyc_suck_restofline_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){
lexstate=Cyc_lex_engine(lexstate,lexbuf);
{int _tmp83=lexstate;if(_tmp83 == 0){_LL119: _LL11A:
# 542 "buildlib.cyl"
 Cyc_current_line=(struct _dyneither_ptr)Cyc_strconcat((struct _dyneither_ptr)Cyc_current_line,(struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf));return 0;}else{_LL11B: _LL11C:
(lexbuf->refill_buff)(lexbuf);
return Cyc_suck_restofline_rec(lexbuf,lexstate);}_LL118:;}
# 546
(int)_throw((void*)({struct Cyc_Lexing_Error_exn_struct*_tmp84=_cycalloc(sizeof(*_tmp84));_tmp84[0]=({struct Cyc_Lexing_Error_exn_struct _tmp85;_tmp85.tag=Cyc_Lexing_Error;_tmp85.f1=({const char*_tmp86="some action didn't return!";_tag_dyneither(_tmp86,sizeof(char),27);});_tmp85;});_tmp84;}));}
# 548
int Cyc_suck_restofline(struct Cyc_Lexing_lexbuf*lexbuf){return Cyc_suck_restofline_rec(lexbuf,12);}
struct _tuple23*Cyc_spec_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){
lexstate=Cyc_lex_engine(lexstate,lexbuf);
{int _tmp87=lexstate;switch(_tmp87){case 0: _LL11E: _LL11F:
# 549 "buildlib.cyl"
 return Cyc_spec(lexbuf);case 1: _LL120: _LL121:
# 551
 Cyc_current_headerfile=(struct _dyneither_ptr)
Cyc_substring((struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf),0,(unsigned long)((
Cyc_Lexing_lexeme_end(lexbuf)- Cyc_Lexing_lexeme_start(lexbuf))- 1));
Cyc_current_symbols=0;
Cyc_current_omit_symbols=0;
Cyc_current_cstubs=0;
Cyc_current_cycstubs=0;
Cyc_current_hstubs=0;
while(Cyc_commands(lexbuf)){;}
Cyc_current_hstubs=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(Cyc_current_hstubs);
Cyc_current_cstubs=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(Cyc_current_cstubs);
Cyc_current_cycstubs=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(Cyc_current_cycstubs);
return({struct _tuple23*_tmp88=_cycalloc(sizeof(*_tmp88));_tmp88->f1=Cyc_current_headerfile;_tmp88->f2=Cyc_current_symbols;_tmp88->f3=Cyc_current_omit_symbols;_tmp88->f4=Cyc_current_hstubs;_tmp88->f5=Cyc_current_cstubs;_tmp88->f6=Cyc_current_cycstubs;_tmp88;});case 2: _LL122: _LL123:
# 571
 return Cyc_spec(lexbuf);case 3: _LL124: _LL125:
# 573
 return 0;case 4: _LL126: _LL127:
# 575
({struct Cyc_Int_pa_PrintArg_struct _tmp8B;_tmp8B.tag=1;_tmp8B.f1=(unsigned long)((int)
# 577
Cyc_Lexing_lexeme_char(lexbuf,0));({void*_tmp89[1]={& _tmp8B};Cyc_fprintf(Cyc_stderr,({const char*_tmp8A="Error in .cys file: expected header file name, found '%c' instead\n";_tag_dyneither(_tmp8A,sizeof(char),67);}),_tag_dyneither(_tmp89,sizeof(void*),1));});});
return 0;default: _LL128: _LL129:
(lexbuf->refill_buff)(lexbuf);
return Cyc_spec_rec(lexbuf,lexstate);}_LL11D:;}
# 582
(int)_throw((void*)({struct Cyc_Lexing_Error_exn_struct*_tmp8C=_cycalloc(sizeof(*_tmp8C));_tmp8C[0]=({struct Cyc_Lexing_Error_exn_struct _tmp8D;_tmp8D.tag=Cyc_Lexing_Error;_tmp8D.f1=({const char*_tmp8E="some action didn't return!";_tag_dyneither(_tmp8E,sizeof(char),27);});_tmp8D;});_tmp8C;}));}
# 584
struct _tuple23*Cyc_spec(struct Cyc_Lexing_lexbuf*lexbuf){return Cyc_spec_rec(lexbuf,13);}
int Cyc_commands_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){
lexstate=Cyc_lex_engine(lexstate,lexbuf);
{int _tmp8F=lexstate;switch(_tmp8F){case 0: _LL12B: _LL12C:
# 582 "buildlib.cyl"
 return 0;case 1: _LL12D: _LL12E:
# 584
 return 0;case 2: _LL12F: _LL130:
# 586
 Cyc_snarfed_symbols=0;
while(Cyc_snarfsymbols(lexbuf)){;}
Cyc_current_symbols=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_append)(Cyc_snarfed_symbols,Cyc_current_symbols);
return 1;case 3: _LL131: _LL132:
# 591
 Cyc_snarfed_symbols=0;
while(Cyc_snarfsymbols(lexbuf)){;}
Cyc_current_omit_symbols=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_append)(Cyc_snarfed_symbols,Cyc_current_omit_symbols);
return 1;case 4: _LL133: _LL134:
# 596
 Cyc_braces_to_match=1;
Cyc_specbuf=Cyc_Buffer_create(255);
while(Cyc_block(lexbuf)){;}{
struct _tuple22*x=({struct _tuple22*_tmp91=_cycalloc(sizeof(*_tmp91));_tmp91->f1=(struct _dyneither_ptr)_tag_dyneither(0,0,0);_tmp91->f2=(struct _dyneither_ptr)
Cyc_Buffer_contents((struct Cyc_Buffer_t*)_check_null(Cyc_specbuf));_tmp91;});
Cyc_current_hstubs=({struct Cyc_List_List*_tmp90=_cycalloc(sizeof(*_tmp90));_tmp90->hd=x;_tmp90->tl=Cyc_current_hstubs;_tmp90;});
return 1;};case 5: _LL135: _LL136: {
# 604
struct _dyneither_ptr _tmp92=Cyc_Lexing_lexeme(lexbuf);
_dyneither_ptr_inplace_plus(& _tmp92,sizeof(char),(int)Cyc_strlen(({const char*_tmp93="hstub";_tag_dyneither(_tmp93,sizeof(char),6);})));
while( isspace((int)*((char*)_check_dyneither_subscript(_tmp92,sizeof(char),0)))){_dyneither_ptr_inplace_plus(& _tmp92,sizeof(char),1);}{
struct _dyneither_ptr t=_tmp92;
while(! isspace((int)*((char*)_check_dyneither_subscript(t,sizeof(char),0)))){_dyneither_ptr_inplace_plus(& t,sizeof(char),1);}{
struct _dyneither_ptr _tmp94=Cyc_substring((struct _dyneither_ptr)_tmp92,0,(unsigned long)((t.curr - _tmp92.curr)/ sizeof(char)));
Cyc_braces_to_match=1;
Cyc_specbuf=Cyc_Buffer_create(255);
while(Cyc_block(lexbuf)){;}{
struct _tuple22*x=({struct _tuple22*_tmp96=_cycalloc(sizeof(*_tmp96));_tmp96->f1=(struct _dyneither_ptr)_tmp94;_tmp96->f2=(struct _dyneither_ptr)
Cyc_Buffer_contents((struct Cyc_Buffer_t*)_check_null(Cyc_specbuf));_tmp96;});
Cyc_current_hstubs=({struct Cyc_List_List*_tmp95=_cycalloc(sizeof(*_tmp95));_tmp95->hd=x;_tmp95->tl=Cyc_current_hstubs;_tmp95;});
return 1;};};};}case 6: _LL137: _LL138:
# 618
 Cyc_braces_to_match=1;
Cyc_specbuf=Cyc_Buffer_create(255);
while(Cyc_block(lexbuf)){;}{
struct _tuple22*x=({struct _tuple22*_tmp98=_cycalloc(sizeof(*_tmp98));_tmp98->f1=(struct _dyneither_ptr)_tag_dyneither(0,0,0);_tmp98->f2=(struct _dyneither_ptr)
Cyc_Buffer_contents((struct Cyc_Buffer_t*)_check_null(Cyc_specbuf));_tmp98;});
Cyc_current_cstubs=({struct Cyc_List_List*_tmp97=_cycalloc(sizeof(*_tmp97));_tmp97->hd=x;_tmp97->tl=Cyc_current_cstubs;_tmp97;});
return 1;};case 7: _LL139: _LL13A: {
# 626
struct _dyneither_ptr _tmp99=Cyc_Lexing_lexeme(lexbuf);
_dyneither_ptr_inplace_plus(& _tmp99,sizeof(char),(int)Cyc_strlen(({const char*_tmp9A="cstub";_tag_dyneither(_tmp9A,sizeof(char),6);})));
while( isspace((int)*((char*)_check_dyneither_subscript(_tmp99,sizeof(char),0)))){_dyneither_ptr_inplace_plus(& _tmp99,sizeof(char),1);}{
struct _dyneither_ptr t=_tmp99;
while(! isspace((int)*((char*)_check_dyneither_subscript(t,sizeof(char),0)))){_dyneither_ptr_inplace_plus(& t,sizeof(char),1);}{
struct _dyneither_ptr _tmp9B=Cyc_substring((struct _dyneither_ptr)_tmp99,0,(unsigned long)((t.curr - _tmp99.curr)/ sizeof(char)));
Cyc_braces_to_match=1;
Cyc_specbuf=Cyc_Buffer_create(255);
while(Cyc_block(lexbuf)){;}{
struct _tuple22*x=({struct _tuple22*_tmp9D=_cycalloc(sizeof(*_tmp9D));_tmp9D->f1=(struct _dyneither_ptr)_tmp9B;_tmp9D->f2=(struct _dyneither_ptr)
Cyc_Buffer_contents((struct Cyc_Buffer_t*)_check_null(Cyc_specbuf));_tmp9D;});
Cyc_current_cstubs=({struct Cyc_List_List*_tmp9C=_cycalloc(sizeof(*_tmp9C));_tmp9C->hd=x;_tmp9C->tl=Cyc_current_cstubs;_tmp9C;});
return 1;};};};}case 8: _LL13B: _LL13C:
# 640
 Cyc_braces_to_match=1;
Cyc_specbuf=Cyc_Buffer_create(255);
while(Cyc_block(lexbuf)){;}{
struct _tuple22*x=({struct _tuple22*_tmp9F=_cycalloc(sizeof(*_tmp9F));_tmp9F->f1=(struct _dyneither_ptr)_tag_dyneither(0,0,0);_tmp9F->f2=(struct _dyneither_ptr)
Cyc_Buffer_contents((struct Cyc_Buffer_t*)_check_null(Cyc_specbuf));_tmp9F;});
Cyc_current_cycstubs=({struct Cyc_List_List*_tmp9E=_cycalloc(sizeof(*_tmp9E));_tmp9E->hd=x;_tmp9E->tl=Cyc_current_cycstubs;_tmp9E;});
return 1;};case 9: _LL13D: _LL13E: {
# 648
struct _dyneither_ptr _tmpA0=Cyc_Lexing_lexeme(lexbuf);
_dyneither_ptr_inplace_plus(& _tmpA0,sizeof(char),(int)Cyc_strlen(({const char*_tmpA1="cycstub";_tag_dyneither(_tmpA1,sizeof(char),8);})));
while( isspace((int)*((char*)_check_dyneither_subscript(_tmpA0,sizeof(char),0)))){_dyneither_ptr_inplace_plus(& _tmpA0,sizeof(char),1);}{
struct _dyneither_ptr t=_tmpA0;
while(! isspace((int)*((char*)_check_dyneither_subscript(t,sizeof(char),0)))){_dyneither_ptr_inplace_plus(& t,sizeof(char),1);}{
struct _dyneither_ptr _tmpA2=Cyc_substring((struct _dyneither_ptr)_tmpA0,0,(unsigned long)((t.curr - _tmpA0.curr)/ sizeof(char)));
Cyc_braces_to_match=1;
Cyc_specbuf=Cyc_Buffer_create(255);
while(Cyc_block(lexbuf)){;}{
struct _tuple22*x=({struct _tuple22*_tmpA4=_cycalloc(sizeof(*_tmpA4));_tmpA4->f1=(struct _dyneither_ptr)_tmpA2;_tmpA4->f2=(struct _dyneither_ptr)
Cyc_Buffer_contents((struct Cyc_Buffer_t*)_check_null(Cyc_specbuf));_tmpA4;});
Cyc_current_cycstubs=({struct Cyc_List_List*_tmpA3=_cycalloc(sizeof(*_tmpA3));_tmpA3->hd=x;_tmpA3->tl=Cyc_current_cycstubs;_tmpA3;});
return 1;};};};}case 10: _LL13F: _LL140:
# 662
 Cyc_braces_to_match=1;
Cyc_specbuf=Cyc_Buffer_create(255);
while(Cyc_block(lexbuf)){;}{
struct _dyneither_ptr*x=({struct _dyneither_ptr*_tmpA6=_cycalloc(sizeof(*_tmpA6));_tmpA6[0]=(struct _dyneither_ptr)Cyc_Buffer_contents((struct Cyc_Buffer_t*)_check_null(Cyc_specbuf));_tmpA6;});
Cyc_current_cpp=({struct Cyc_List_List*_tmpA5=_cycalloc(sizeof(*_tmpA5));_tmpA5->hd=x;_tmpA5->tl=Cyc_current_cpp;_tmpA5;});
return 1;};case 11: _LL141: _LL142:
# 669
 return 1;case 12: _LL143: _LL144:
# 671
 return 1;case 13: _LL145: _LL146:
# 673
({struct Cyc_Int_pa_PrintArg_struct _tmpA9;_tmpA9.tag=1;_tmpA9.f1=(unsigned long)((int)
# 675
Cyc_Lexing_lexeme_char(lexbuf,0));({void*_tmpA7[1]={& _tmpA9};Cyc_fprintf(Cyc_stderr,({const char*_tmpA8="Error in .cys file: expected command, found '%c' instead\n";_tag_dyneither(_tmpA8,sizeof(char),58);}),_tag_dyneither(_tmpA7,sizeof(void*),1));});});
return 0;default: _LL147: _LL148:
(lexbuf->refill_buff)(lexbuf);
return Cyc_commands_rec(lexbuf,lexstate);}_LL12A:;}
# 680
(int)_throw((void*)({struct Cyc_Lexing_Error_exn_struct*_tmpAA=_cycalloc(sizeof(*_tmpAA));_tmpAA[0]=({struct Cyc_Lexing_Error_exn_struct _tmpAB;_tmpAB.tag=Cyc_Lexing_Error;_tmpAB.f1=({const char*_tmpAC="some action didn't return!";_tag_dyneither(_tmpAC,sizeof(char),27);});_tmpAB;});_tmpAA;}));}
# 682
int Cyc_commands(struct Cyc_Lexing_lexbuf*lexbuf){return Cyc_commands_rec(lexbuf,14);}
int Cyc_snarfsymbols_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){
lexstate=Cyc_lex_engine(lexstate,lexbuf);
{int _tmpAD=lexstate;switch(_tmpAD){case 0: _LL14A: _LL14B:
# 685 "buildlib.cyl"
 Cyc_snarfed_symbols=({struct Cyc_List_List*_tmpAE=_cycalloc(sizeof(*_tmpAE));_tmpAE->hd=({struct _dyneither_ptr*_tmpAF=_cycalloc(sizeof(*_tmpAF));_tmpAF[0]=(struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf);_tmpAF;});_tmpAE->tl=Cyc_snarfed_symbols;_tmpAE;});
return 1;case 1: _LL14C: _LL14D:
# 688
 return 1;case 2: _LL14E: _LL14F:
# 690
 return 0;case 3: _LL150: _LL151:
# 692
({void*_tmpB0=0;Cyc_fprintf(Cyc_stderr,({const char*_tmpB1="Error in .cys file: unexpected end-of-file\n";_tag_dyneither(_tmpB1,sizeof(char),44);}),_tag_dyneither(_tmpB0,sizeof(void*),0));});
# 694
return 0;case 4: _LL152: _LL153:
# 696
({struct Cyc_Int_pa_PrintArg_struct _tmpB4;_tmpB4.tag=1;_tmpB4.f1=(unsigned long)((int)
# 698
Cyc_Lexing_lexeme_char(lexbuf,0));({void*_tmpB2[1]={& _tmpB4};Cyc_fprintf(Cyc_stderr,({const char*_tmpB3="Error in .cys file: expected symbol, found '%c' instead\n";_tag_dyneither(_tmpB3,sizeof(char),57);}),_tag_dyneither(_tmpB2,sizeof(void*),1));});});
return 0;default: _LL154: _LL155:
(lexbuf->refill_buff)(lexbuf);
return Cyc_snarfsymbols_rec(lexbuf,lexstate);}_LL149:;}
# 703
(int)_throw((void*)({struct Cyc_Lexing_Error_exn_struct*_tmpB5=_cycalloc(sizeof(*_tmpB5));_tmpB5[0]=({struct Cyc_Lexing_Error_exn_struct _tmpB6;_tmpB6.tag=Cyc_Lexing_Error;_tmpB6.f1=({const char*_tmpB7="some action didn't return!";_tag_dyneither(_tmpB7,sizeof(char),27);});_tmpB6;});_tmpB5;}));}
# 705
int Cyc_snarfsymbols(struct Cyc_Lexing_lexbuf*lexbuf){return Cyc_snarfsymbols_rec(lexbuf,15);}
int Cyc_block_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){
lexstate=Cyc_lex_engine(lexstate,lexbuf);
{int _tmpB8=lexstate;switch(_tmpB8){case 0: _LL157: _LL158:
# 709 "buildlib.cyl"
({void*_tmpB9=0;Cyc_log(({const char*_tmpBA="Warning: unclosed brace\n";_tag_dyneither(_tmpBA,sizeof(char),25);}),_tag_dyneither(_tmpB9,sizeof(void*),0));});return 0;case 1: _LL159: _LL15A:
# 711
 if(Cyc_braces_to_match == 1)return 0;
-- Cyc_braces_to_match;
Cyc_Buffer_add_char((struct Cyc_Buffer_t*)_check_null(Cyc_specbuf),'}');
return 1;case 2: _LL15B: _LL15C:
# 716
 ++ Cyc_braces_to_match;
Cyc_Buffer_add_char((struct Cyc_Buffer_t*)_check_null(Cyc_specbuf),'{');
return 1;case 3: _LL15D: _LL15E:
# 720
 Cyc_Buffer_add_char((struct Cyc_Buffer_t*)_check_null(Cyc_specbuf),'"');
while(Cyc_block_string(lexbuf)){;}
return 1;case 4: _LL15F: _LL160:
# 724
 Cyc_Buffer_add_string((struct Cyc_Buffer_t*)_check_null(Cyc_specbuf),({const char*_tmpBB="/*";_tag_dyneither(_tmpBB,sizeof(char),3);}));
while(Cyc_block_comment(lexbuf)){;}
return 1;case 5: _LL161: _LL162:
# 728
 Cyc_Buffer_add_string((struct Cyc_Buffer_t*)_check_null(Cyc_specbuf),(struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf));
return 1;case 6: _LL163: _LL164:
# 731
 Cyc_Buffer_add_char((struct Cyc_Buffer_t*)_check_null(Cyc_specbuf),Cyc_Lexing_lexeme_char(lexbuf,0));
return 1;default: _LL165: _LL166:
(lexbuf->refill_buff)(lexbuf);
return Cyc_block_rec(lexbuf,lexstate);}_LL156:;}
# 736
(int)_throw((void*)({struct Cyc_Lexing_Error_exn_struct*_tmpBC=_cycalloc(sizeof(*_tmpBC));_tmpBC[0]=({struct Cyc_Lexing_Error_exn_struct _tmpBD;_tmpBD.tag=Cyc_Lexing_Error;_tmpBD.f1=({const char*_tmpBE="some action didn't return!";_tag_dyneither(_tmpBE,sizeof(char),27);});_tmpBD;});_tmpBC;}));}
# 738
int Cyc_block(struct Cyc_Lexing_lexbuf*lexbuf){return Cyc_block_rec(lexbuf,16);}
int Cyc_block_string_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){
lexstate=Cyc_lex_engine(lexstate,lexbuf);
{int _tmpBF=lexstate;switch(_tmpBF){case 0: _LL168: _LL169:
# 736 "buildlib.cyl"
({void*_tmpC0=0;Cyc_log(({const char*_tmpC1="Warning: unclosed string\n";_tag_dyneither(_tmpC1,sizeof(char),26);}),_tag_dyneither(_tmpC0,sizeof(void*),0));});return 0;case 1: _LL16A: _LL16B:
# 738
 Cyc_Buffer_add_char((struct Cyc_Buffer_t*)_check_null(Cyc_specbuf),'"');return 0;case 2: _LL16C: _LL16D:
# 740
({void*_tmpC2=0;Cyc_log(({const char*_tmpC3="Warning: unclosed string\n";_tag_dyneither(_tmpC3,sizeof(char),26);}),_tag_dyneither(_tmpC2,sizeof(void*),0));});
Cyc_Buffer_add_string((struct Cyc_Buffer_t*)_check_null(Cyc_specbuf),(struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf));
return 1;case 3: _LL16E: _LL16F:
# 744
 Cyc_Buffer_add_string((struct Cyc_Buffer_t*)_check_null(Cyc_specbuf),(struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf));
return 1;case 4: _LL170: _LL171:
# 747
 Cyc_Buffer_add_string((struct Cyc_Buffer_t*)_check_null(Cyc_specbuf),(struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf));
return 1;case 5: _LL172: _LL173:
# 750
 Cyc_Buffer_add_string((struct Cyc_Buffer_t*)_check_null(Cyc_specbuf),(struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf));
return 1;case 6: _LL174: _LL175:
# 753
 Cyc_Buffer_add_string((struct Cyc_Buffer_t*)_check_null(Cyc_specbuf),(struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf));
return 1;case 7: _LL176: _LL177:
# 756
 Cyc_Buffer_add_string((struct Cyc_Buffer_t*)_check_null(Cyc_specbuf),(struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf));
return 1;case 8: _LL178: _LL179:
# 759
 Cyc_Buffer_add_string((struct Cyc_Buffer_t*)_check_null(Cyc_specbuf),(struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf));
return 1;default: _LL17A: _LL17B:
(lexbuf->refill_buff)(lexbuf);
return Cyc_block_string_rec(lexbuf,lexstate);}_LL167:;}
# 764
(int)_throw((void*)({struct Cyc_Lexing_Error_exn_struct*_tmpC4=_cycalloc(sizeof(*_tmpC4));_tmpC4[0]=({struct Cyc_Lexing_Error_exn_struct _tmpC5;_tmpC5.tag=Cyc_Lexing_Error;_tmpC5.f1=({const char*_tmpC6="some action didn't return!";_tag_dyneither(_tmpC6,sizeof(char),27);});_tmpC5;});_tmpC4;}));}
# 766
int Cyc_block_string(struct Cyc_Lexing_lexbuf*lexbuf){return Cyc_block_string_rec(lexbuf,17);}
int Cyc_block_comment_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){
lexstate=Cyc_lex_engine(lexstate,lexbuf);
{int _tmpC7=lexstate;switch(_tmpC7){case 0: _LL17D: _LL17E:
# 764 "buildlib.cyl"
({void*_tmpC8=0;Cyc_log(({const char*_tmpC9="Warning: unclosed comment\n";_tag_dyneither(_tmpC9,sizeof(char),27);}),_tag_dyneither(_tmpC8,sizeof(void*),0));});return 0;case 1: _LL17F: _LL180:
# 766
 Cyc_Buffer_add_string((struct Cyc_Buffer_t*)_check_null(Cyc_specbuf),({const char*_tmpCA="*/";_tag_dyneither(_tmpCA,sizeof(char),3);}));return 0;case 2: _LL181: _LL182:
# 768
 Cyc_Buffer_add_string((struct Cyc_Buffer_t*)_check_null(Cyc_specbuf),(struct _dyneither_ptr)Cyc_Lexing_lexeme(lexbuf));
return 1;default: _LL183: _LL184:
(lexbuf->refill_buff)(lexbuf);
return Cyc_block_comment_rec(lexbuf,lexstate);}_LL17C:;}
# 773
(int)_throw((void*)({struct Cyc_Lexing_Error_exn_struct*_tmpCB=_cycalloc(sizeof(*_tmpCB));_tmpCB[0]=({struct Cyc_Lexing_Error_exn_struct _tmpCC;_tmpCC.tag=Cyc_Lexing_Error;_tmpCC.f1=({const char*_tmpCD="some action didn't return!";_tag_dyneither(_tmpCD,sizeof(char),27);});_tmpCC;});_tmpCB;}));}
# 775
int Cyc_block_comment(struct Cyc_Lexing_lexbuf*lexbuf){return Cyc_block_comment_rec(lexbuf,18);}
# 778 "buildlib.cyl"
typedef struct Cyc_Hashtable_Table*Cyc_dep_t;
# 781
void Cyc_scan_type(void*t,struct Cyc_Hashtable_Table*dep);struct _tuple24{struct Cyc_List_List*f1;struct Cyc_Absyn_Exp*f2;};
void Cyc_scan_exp(struct Cyc_Absyn_Exp*e,struct Cyc_Hashtable_Table*dep){
void*_tmpCE=((struct Cyc_Absyn_Exp*)_check_null(e))->r;void*_tmpCF=_tmpCE;struct Cyc_List_List*_tmp11B;void*_tmp11A;struct Cyc_List_List*_tmp119;struct Cyc_Absyn_Exp*_tmp118;struct _dyneither_ptr*_tmp117;struct Cyc_Absyn_Exp*_tmp116;struct _dyneither_ptr*_tmp115;void*_tmp114;void*_tmp113;struct Cyc_Absyn_Exp*_tmp112;int _tmp111;struct Cyc_Absyn_Exp*_tmp110;void**_tmp10F;struct Cyc_Absyn_Exp*_tmp10E;void*_tmp10D;struct Cyc_Absyn_Exp*_tmp10C;struct Cyc_Absyn_Exp*_tmp10B;struct Cyc_List_List*_tmp10A;struct Cyc_Absyn_Exp*_tmp109;struct Cyc_Absyn_Exp*_tmp108;struct Cyc_Absyn_Exp*_tmp107;struct Cyc_Absyn_Exp*_tmp106;struct Cyc_Absyn_Exp*_tmp105;struct Cyc_Absyn_Exp*_tmp104;struct Cyc_Absyn_Exp*_tmp103;struct Cyc_Absyn_Exp*_tmp102;struct Cyc_Absyn_Exp*_tmp101;struct Cyc_Absyn_Exp*_tmp100;struct Cyc_Absyn_Exp*_tmpFF;struct Cyc_Absyn_Exp*_tmpFE;struct Cyc_Absyn_Exp*_tmpFD;struct Cyc_Absyn_Exp*_tmpFC;struct Cyc_Absyn_Exp*_tmpFB;struct Cyc_Absyn_Exp*_tmpFA;struct Cyc_Absyn_Exp*_tmpF9;struct Cyc_List_List*_tmpF8;void*_tmpF7;switch(*((int*)_tmpCF)){case 1: _LL186: _tmpF7=(void*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmpCF)->f1;_LL187: {
# 785
struct _dyneither_ptr*_tmpD0=(*Cyc_Absyn_binding2qvar(_tmpF7)).f2;
Cyc_add_target(_tmpD0);
return;}case 2: _LL188: _tmpF8=((struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_tmpCF)->f2;_LL189:
# 789
 for(0;_tmpF8 != 0;_tmpF8=_tmpF8->tl){
Cyc_scan_exp((struct Cyc_Absyn_Exp*)_tmpF8->hd,dep);}
# 792
return;case 22: _LL18A: _tmpFA=((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmpCF)->f1;_tmpF9=((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmpCF)->f2;_LL18B:
# 794
 _tmpFC=_tmpFA;_tmpFB=_tmpF9;goto _LL18D;case 8: _LL18C: _tmpFC=((struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct*)_tmpCF)->f1;_tmpFB=((struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct*)_tmpCF)->f2;_LL18D:
# 796
 _tmpFE=_tmpFC;_tmpFD=_tmpFB;goto _LL18F;case 3: _LL18E: _tmpFE=((struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct*)_tmpCF)->f1;_tmpFD=((struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct*)_tmpCF)->f3;_LL18F:
# 798
 Cyc_scan_exp(_tmpFE,dep);
Cyc_scan_exp(_tmpFD,dep);
return;case 19: _LL190: _tmpFF=((struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct*)_tmpCF)->f1;_LL191:
# 802
 _tmp100=_tmpFF;goto _LL193;case 17: _LL192: _tmp100=((struct Cyc_Absyn_Sizeofexp_e_Absyn_Raw_exp_struct*)_tmpCF)->f1;_LL193:
# 804
 _tmp101=_tmp100;goto _LL195;case 14: _LL194: _tmp101=((struct Cyc_Absyn_Address_e_Absyn_Raw_exp_struct*)_tmpCF)->f1;_LL195:
# 806
 _tmp102=_tmp101;goto _LL197;case 4: _LL196: _tmp102=((struct Cyc_Absyn_Increment_e_Absyn_Raw_exp_struct*)_tmpCF)->f1;_LL197:
# 808
 Cyc_scan_exp(_tmp102,dep);
return;case 5: _LL198: _tmp105=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmpCF)->f1;_tmp104=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmpCF)->f2;_tmp103=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmpCF)->f3;_LL199:
# 811
 Cyc_scan_exp(_tmp105,dep);
Cyc_scan_exp(_tmp104,dep);
Cyc_scan_exp(_tmp103,dep);
return;case 6: _LL19A: _tmp107=((struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct*)_tmpCF)->f1;_tmp106=((struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct*)_tmpCF)->f2;_LL19B:
 _tmp109=_tmp107;_tmp108=_tmp106;goto _LL19D;case 7: _LL19C: _tmp109=((struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct*)_tmpCF)->f1;_tmp108=((struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct*)_tmpCF)->f2;_LL19D:
# 817
 Cyc_scan_exp(_tmp109,dep);
Cyc_scan_exp(_tmp108,dep);
return;case 9: _LL19E: _tmp10B=((struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_tmpCF)->f1;_tmp10A=((struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_tmpCF)->f2;_LL19F:
# 821
 Cyc_scan_exp(_tmp10B,dep);
for(0;_tmp10A != 0;_tmp10A=_tmp10A->tl){
Cyc_scan_exp((struct Cyc_Absyn_Exp*)_tmp10A->hd,dep);}
# 825
return;case 13: _LL1A0: _tmp10D=(void*)((struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_tmpCF)->f1;_tmp10C=((struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_tmpCF)->f2;_LL1A1:
# 827
 Cyc_scan_type(_tmp10D,dep);
Cyc_scan_exp(_tmp10C,dep);
return;case 33: _LL1A2: _tmp111=(((struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct*)_tmpCF)->f1).is_calloc;_tmp110=(((struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct*)_tmpCF)->f1).rgn;_tmp10F=(((struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct*)_tmpCF)->f1).elt_type;_tmp10E=(((struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct*)_tmpCF)->f1).num_elts;_LL1A3:
# 831
 if(_tmp110 != 0)Cyc_scan_exp(_tmp110,dep);
if(_tmp10F != 0)Cyc_scan_type(*_tmp10F,dep);
Cyc_scan_exp(_tmp10E,dep);
return;case 37: _LL1A4: _tmp112=((struct Cyc_Absyn_Tagcheck_e_Absyn_Raw_exp_struct*)_tmpCF)->f1;_LL1A5:
# 836
 Cyc_scan_exp(_tmp112,dep);return;case 38: _LL1A6: _tmp113=(void*)((struct Cyc_Absyn_Valueof_e_Absyn_Raw_exp_struct*)_tmpCF)->f1;_LL1A7:
 _tmp114=_tmp113;goto _LL1A9;case 16: _LL1A8: _tmp114=(void*)((struct Cyc_Absyn_Sizeoftyp_e_Absyn_Raw_exp_struct*)_tmpCF)->f1;_LL1A9:
# 839
 Cyc_scan_type(_tmp114,dep);
return;case 20: _LL1AA: _tmp116=((struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_tmpCF)->f1;_tmp115=((struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_tmpCF)->f2;_LL1AB:
# 842
 _tmp118=_tmp116;_tmp117=_tmp115;goto _LL1AD;case 21: _LL1AC: _tmp118=((struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_tmpCF)->f1;_tmp117=((struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_tmpCF)->f2;_LL1AD:
# 844
 Cyc_scan_exp(_tmp118,dep);
Cyc_add_target(_tmp117);
return;case 18: _LL1AE: _tmp11A=(void*)((struct Cyc_Absyn_Offsetof_e_Absyn_Raw_exp_struct*)_tmpCF)->f1;_tmp119=((struct Cyc_Absyn_Offsetof_e_Absyn_Raw_exp_struct*)_tmpCF)->f2;_LL1AF:
# 848
 Cyc_scan_type(_tmp11A,dep);
# 850
{void*_tmpD1=(void*)((struct Cyc_List_List*)_check_null(_tmp119))->hd;void*_tmpD2=_tmpD1;struct _dyneither_ptr*_tmpD3;if(((struct Cyc_Absyn_StructField_Absyn_OffsetofField_struct*)_tmpD2)->tag == 0){_LL1D7: _tmpD3=((struct Cyc_Absyn_StructField_Absyn_OffsetofField_struct*)_tmpD2)->f1;_LL1D8:
 Cyc_add_target(_tmpD3);goto _LL1D6;}else{_LL1D9: _LL1DA:
 goto _LL1D6;}_LL1D6:;}
# 854
return;case 0: _LL1B0: _LL1B1:
# 856
 return;case 35: _LL1B2: _tmp11B=((struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct*)_tmpCF)->f2;_LL1B3:
# 858
 for(0;_tmp11B != 0;_tmp11B=_tmp11B->tl){
struct _tuple24*_tmpD4=(struct _tuple24*)_tmp11B->hd;struct _tuple24*_tmpD5=_tmpD4;struct Cyc_Absyn_Exp*_tmpD6;_LL1DC: _tmpD6=_tmpD5->f2;_LL1DD:;
Cyc_scan_exp(_tmpD6,dep);}
# 862
return;case 39: _LL1B4: _LL1B5:
 return;case 34: _LL1B6: _LL1B7:
# 865
({void*_tmpD7=0;Cyc_fprintf(Cyc_stderr,({const char*_tmpD8="Error: unexpected Swap_e\n";_tag_dyneither(_tmpD8,sizeof(char),26);}),_tag_dyneither(_tmpD7,sizeof(void*),0));});
 exit(1);return;case 36: _LL1B8: _LL1B9:
# 868
({void*_tmpD9=0;Cyc_fprintf(Cyc_stderr,({const char*_tmpDA="Error: unexpected Stmt_e\n";_tag_dyneither(_tmpDA,sizeof(char),26);}),_tag_dyneither(_tmpD9,sizeof(void*),0));});
 exit(1);return;case 10: _LL1BA: _LL1BB:
# 871
({void*_tmpDB=0;Cyc_fprintf(Cyc_stderr,({const char*_tmpDC="Error: unexpected Throw_e\n";_tag_dyneither(_tmpDC,sizeof(char),27);}),_tag_dyneither(_tmpDB,sizeof(void*),0));});
 exit(1);return;case 11: _LL1BC: _LL1BD:
# 874
({void*_tmpDD=0;Cyc_fprintf(Cyc_stderr,({const char*_tmpDE="Error: unexpected NoInstantiate_e\n";_tag_dyneither(_tmpDE,sizeof(char),35);}),_tag_dyneither(_tmpDD,sizeof(void*),0));});
 exit(1);return;case 12: _LL1BE: _LL1BF:
# 877
({void*_tmpDF=0;Cyc_fprintf(Cyc_stderr,({const char*_tmpE0="Error: unexpected Instantiate_e\n";_tag_dyneither(_tmpE0,sizeof(char),33);}),_tag_dyneither(_tmpDF,sizeof(void*),0));});
 exit(1);return;case 15: _LL1C0: _LL1C1:
# 880
({void*_tmpE1=0;Cyc_fprintf(Cyc_stderr,({const char*_tmpE2="Error: unexpected New_e\n";_tag_dyneither(_tmpE2,sizeof(char),25);}),_tag_dyneither(_tmpE1,sizeof(void*),0));});
 exit(1);return;case 23: _LL1C2: _LL1C3:
# 883
({void*_tmpE3=0;Cyc_fprintf(Cyc_stderr,({const char*_tmpE4="Error: unexpected Tuple_e\n";_tag_dyneither(_tmpE4,sizeof(char),27);}),_tag_dyneither(_tmpE3,sizeof(void*),0));});
 exit(1);return;case 24: _LL1C4: _LL1C5:
# 886
({void*_tmpE5=0;Cyc_fprintf(Cyc_stderr,({const char*_tmpE6="Error: unexpected CompoundLit_e\n";_tag_dyneither(_tmpE6,sizeof(char),33);}),_tag_dyneither(_tmpE5,sizeof(void*),0));});
 exit(1);return;case 25: _LL1C6: _LL1C7:
# 889
({void*_tmpE7=0;Cyc_fprintf(Cyc_stderr,({const char*_tmpE8="Error: unexpected Array_e\n";_tag_dyneither(_tmpE8,sizeof(char),27);}),_tag_dyneither(_tmpE7,sizeof(void*),0));});
 exit(1);return;case 26: _LL1C8: _LL1C9:
# 892
({void*_tmpE9=0;Cyc_fprintf(Cyc_stderr,({const char*_tmpEA="Error: unexpected Comprehension_e\n";_tag_dyneither(_tmpEA,sizeof(char),35);}),_tag_dyneither(_tmpE9,sizeof(void*),0));});
 exit(1);return;case 27: _LL1CA: _LL1CB:
# 895
({void*_tmpEB=0;Cyc_fprintf(Cyc_stderr,({const char*_tmpEC="Error: unexpected ComprehensionNoinit_e\n";_tag_dyneither(_tmpEC,sizeof(char),41);}),_tag_dyneither(_tmpEB,sizeof(void*),0));});
 exit(1);return;case 28: _LL1CC: _LL1CD:
# 898
({void*_tmpED=0;Cyc_fprintf(Cyc_stderr,({const char*_tmpEE="Error: unexpected Aggregate_e\n";_tag_dyneither(_tmpEE,sizeof(char),31);}),_tag_dyneither(_tmpED,sizeof(void*),0));});
 exit(1);return;case 29: _LL1CE: _LL1CF:
# 901
({void*_tmpEF=0;Cyc_fprintf(Cyc_stderr,({const char*_tmpF0="Error: unexpected AnonStruct_e\n";_tag_dyneither(_tmpF0,sizeof(char),32);}),_tag_dyneither(_tmpEF,sizeof(void*),0));});
 exit(1);return;case 30: _LL1D0: _LL1D1:
# 904
({void*_tmpF1=0;Cyc_fprintf(Cyc_stderr,({const char*_tmpF2="Error: unexpected Datatype_e\n";_tag_dyneither(_tmpF2,sizeof(char),30);}),_tag_dyneither(_tmpF1,sizeof(void*),0));});
 exit(1);return;case 31: _LL1D2: _LL1D3:
# 907
({void*_tmpF3=0;Cyc_fprintf(Cyc_stderr,({const char*_tmpF4="Error: unexpected Enum_e\n";_tag_dyneither(_tmpF4,sizeof(char),26);}),_tag_dyneither(_tmpF3,sizeof(void*),0));});
 exit(1);return;default: _LL1D4: _LL1D5:
# 910
({void*_tmpF5=0;Cyc_fprintf(Cyc_stderr,({const char*_tmpF6="Error: unexpected AnonEnum_e\n";_tag_dyneither(_tmpF6,sizeof(char),30);}),_tag_dyneither(_tmpF5,sizeof(void*),0));});
 exit(1);return;}_LL185:;}
# 915
void Cyc_scan_exp_opt(struct Cyc_Absyn_Exp*eo,struct Cyc_Hashtable_Table*dep){
if((unsigned int)eo)Cyc_scan_exp(eo,dep);
return;}
# 920
void Cyc_scan_decl(struct Cyc_Absyn_Decl*d,struct Cyc_Hashtable_Table*dep);
void Cyc_scan_type(void*t,struct Cyc_Hashtable_Table*dep){
void*_tmp11C=t;struct Cyc_Absyn_Datatypedecl*_tmp15A;struct Cyc_Absyn_Enumdecl*_tmp159;struct Cyc_Absyn_Aggrdecl*_tmp158;struct _dyneither_ptr*_tmp157;struct _dyneither_ptr*_tmp156;union Cyc_Absyn_AggrInfoU _tmp155;struct Cyc_List_List*_tmp154;struct Cyc_Absyn_FnInfo _tmp153;struct Cyc_Absyn_Exp*_tmp152;void*_tmp151;struct Cyc_Absyn_Exp*_tmp150;union Cyc_Absyn_Constraint*_tmp14F;struct Cyc_Absyn_PtrInfo _tmp14E;switch(*((int*)_tmp11C)){case 0: _LL1DF: _LL1E0:
 goto _LL1E2;case 6: _LL1E1: _LL1E2:
 goto _LL1E4;case 28: _LL1E3: _LL1E4:
 goto _LL1E6;case 7: _LL1E5: _LL1E6:
# 927
 return;case 5: _LL1E7: _tmp14E=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp11C)->f1;_LL1E8:
# 930
 Cyc_scan_type(_tmp14E.elt_typ,dep);
return;case 8: _LL1E9: _tmp151=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp11C)->f1).elt_type;_tmp150=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp11C)->f1).num_elts;_tmp14F=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp11C)->f1).zero_term;_LL1EA:
# 933
 Cyc_scan_type(_tmp151,dep);
Cyc_scan_exp_opt(_tmp150,dep);
return;case 27: _LL1EB: _tmp152=((struct Cyc_Absyn_TypeofType_Absyn_Type_struct*)_tmp11C)->f1;_LL1EC:
# 937
 Cyc_scan_exp(_tmp152,dep);
return;case 9: _LL1ED: _tmp153=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp11C)->f1;_LL1EE:
# 940
 Cyc_scan_type(_tmp153.ret_typ,dep);
{struct Cyc_List_List*_tmp11D=_tmp153.args;for(0;_tmp11D != 0;_tmp11D=_tmp11D->tl){
struct _tuple8*_tmp11E=(struct _tuple8*)_tmp11D->hd;struct _tuple8*_tmp11F=_tmp11E;void*_tmp120;_LL21E: _tmp120=_tmp11F->f3;_LL21F:;
Cyc_scan_type(_tmp120,dep);}}
# 945
if(_tmp153.cyc_varargs != 0)
Cyc_scan_type((_tmp153.cyc_varargs)->type,dep);
return;case 12: _LL1EF: _tmp154=((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_tmp11C)->f2;_LL1F0:
# 949
 for(0;_tmp154 != 0;_tmp154=_tmp154->tl){
Cyc_scan_type(((struct Cyc_Absyn_Aggrfield*)_tmp154->hd)->type,dep);
Cyc_scan_exp_opt(((struct Cyc_Absyn_Aggrfield*)_tmp154->hd)->width,dep);}
# 953
return;case 14: _LL1F1: _LL1F2:
# 955
 return;case 11: _LL1F3: _tmp155=(((struct Cyc_Absyn_AggrType_Absyn_Type_struct*)_tmp11C)->f1).aggr_info;_LL1F4: {
# 957
struct _tuple10 _tmp121=Cyc_Absyn_aggr_kinded_name(_tmp155);struct _tuple10 _tmp122=_tmp121;struct _dyneither_ptr*_tmp123;_LL221: _tmp123=(_tmp122.f2)->f2;_LL222:;
_tmp156=_tmp123;goto _LL1F6;}case 13: _LL1F5: _tmp156=(((struct Cyc_Absyn_EnumType_Absyn_Type_struct*)_tmp11C)->f1)->f2;_LL1F6:
# 960
 _tmp157=_tmp156;goto _LL1F8;case 17: _LL1F7: _tmp157=(((struct Cyc_Absyn_TypedefType_Absyn_Type_struct*)_tmp11C)->f1)->f2;_LL1F8:
# 962
 Cyc_add_target(_tmp157);
return;case 26: switch(*((int*)((struct Cyc_Absyn_TypeDecl*)((struct Cyc_Absyn_TypeDeclType_Absyn_Type_struct*)_tmp11C)->f1)->r)){case 0: _LL1F9: _tmp158=((struct Cyc_Absyn_Aggr_td_Absyn_Raw_typedecl_struct*)(((struct Cyc_Absyn_TypeDeclType_Absyn_Type_struct*)_tmp11C)->f1)->r)->f1;_LL1FA:
# 966
 Cyc_scan_decl(Cyc_Absyn_new_decl((void*)({struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct*_tmp124=_cycalloc(sizeof(*_tmp124));_tmp124[0]=({struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct _tmp125;_tmp125.tag=5;_tmp125.f1=_tmp158;_tmp125;});_tmp124;}),0),dep);{
struct _tuple0*_tmp126=_tmp158->name;struct _tuple0*_tmp127=_tmp126;struct _dyneither_ptr*_tmp128;_LL224: _tmp128=_tmp127->f2;_LL225:;
Cyc_add_target(_tmp128);
return;};case 1: _LL1FB: _tmp159=((struct Cyc_Absyn_Enum_td_Absyn_Raw_typedecl_struct*)(((struct Cyc_Absyn_TypeDeclType_Absyn_Type_struct*)_tmp11C)->f1)->r)->f1;_LL1FC:
# 972
 Cyc_scan_decl(Cyc_Absyn_new_decl((void*)({struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct*_tmp129=_cycalloc(sizeof(*_tmp129));_tmp129[0]=({struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct _tmp12A;_tmp12A.tag=7;_tmp12A.f1=_tmp159;_tmp12A;});_tmp129;}),0),dep);{
struct _tuple0*_tmp12B=_tmp159->name;struct _tuple0*_tmp12C=_tmp12B;struct _dyneither_ptr*_tmp12D;_LL227: _tmp12D=_tmp12C->f2;_LL228:;
Cyc_add_target(_tmp12D);
return;};default: _LL1FD: _tmp15A=((struct Cyc_Absyn_Datatype_td_Absyn_Raw_typedecl_struct*)(((struct Cyc_Absyn_TypeDeclType_Absyn_Type_struct*)_tmp11C)->f1)->r)->f1;_LL1FE:
# 978
({void*_tmp12E=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp12F="Error: unexpected Datatype declaration\n";_tag_dyneither(_tmp12F,sizeof(char),40);}),_tag_dyneither(_tmp12E,sizeof(void*),0));});
 exit(1);return;}case 1: _LL1FF: _LL200:
# 981
({void*_tmp130=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp131="Error: unexpected Evar\n";_tag_dyneither(_tmp131,sizeof(char),24);}),_tag_dyneither(_tmp130,sizeof(void*),0));});
 exit(1);return;case 2: _LL201: _LL202:
# 984
({void*_tmp132=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp133="Error: unexpected VarType\n";_tag_dyneither(_tmp133,sizeof(char),27);}),_tag_dyneither(_tmp132,sizeof(void*),0));});
 exit(1);return;case 3: _LL203: _LL204:
# 987
({void*_tmp134=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp135="Error: unexpected DatatypeType\n";_tag_dyneither(_tmp135,sizeof(char),32);}),_tag_dyneither(_tmp134,sizeof(void*),0));});
 exit(1);return;case 4: _LL205: _LL206:
# 990
({void*_tmp136=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp137="Error: unexpected DatatypeFieldType\n";_tag_dyneither(_tmp137,sizeof(char),37);}),_tag_dyneither(_tmp136,sizeof(void*),0));});
 exit(1);return;case 10: _LL207: _LL208:
# 993
({void*_tmp138=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp139="Error: unexpected TupleType\n";_tag_dyneither(_tmp139,sizeof(char),29);}),_tag_dyneither(_tmp138,sizeof(void*),0));});
 exit(1);return;case 15: _LL209: _LL20A:
# 996
({void*_tmp13A=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp13B="Error: unexpected RgnHandleType\n";_tag_dyneither(_tmp13B,sizeof(char),33);}),_tag_dyneither(_tmp13A,sizeof(void*),0));});
 exit(1);return;case 16: _LL20B: _LL20C:
# 999
({void*_tmp13C=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp13D="Error: unexpected DynRgnType\n";_tag_dyneither(_tmp13D,sizeof(char),30);}),_tag_dyneither(_tmp13C,sizeof(void*),0));});
 exit(1);return;case 20: _LL20D: _LL20E:
# 1002
({void*_tmp13E=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp13F="Error: unexpected HeapRgn\n";_tag_dyneither(_tmp13F,sizeof(char),27);}),_tag_dyneither(_tmp13E,sizeof(void*),0));});
 exit(1);return;case 21: _LL20F: _LL210:
# 1005
({void*_tmp140=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp141="Error: unexpected UniqueRgn\n";_tag_dyneither(_tmp141,sizeof(char),29);}),_tag_dyneither(_tmp140,sizeof(void*),0));});
 exit(1);return;case 22: _LL211: _LL212:
# 1008
({void*_tmp142=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp143="Error: unexpected RefCntRgn\n";_tag_dyneither(_tmp143,sizeof(char),29);}),_tag_dyneither(_tmp142,sizeof(void*),0));});
 exit(1);return;case 23: _LL213: _LL214:
# 1011
({void*_tmp144=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp145="Error: unexpected AccessEff\n";_tag_dyneither(_tmp145,sizeof(char),29);}),_tag_dyneither(_tmp144,sizeof(void*),0));});
 exit(1);return;case 24: _LL215: _LL216:
# 1014
({void*_tmp146=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp147="Error: unexpected JoinEff\n";_tag_dyneither(_tmp147,sizeof(char),27);}),_tag_dyneither(_tmp146,sizeof(void*),0));});
 exit(1);return;case 25: _LL217: _LL218:
# 1017
({void*_tmp148=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp149="Error: unexpected RgnsEff\n";_tag_dyneither(_tmp149,sizeof(char),27);}),_tag_dyneither(_tmp148,sizeof(void*),0));});
 exit(1);return;case 19: _LL219: _LL21A:
# 1020
({void*_tmp14A=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp14B="Error: unexpected tag_t\n";_tag_dyneither(_tmp14B,sizeof(char),25);}),_tag_dyneither(_tmp14A,sizeof(void*),0));});
 exit(1);return;default: _LL21B: _LL21C:
# 1023
({void*_tmp14C=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp14D="Error: unexpected valueof_t\n";_tag_dyneither(_tmp14D,sizeof(char),29);}),_tag_dyneither(_tmp14C,sizeof(void*),0));});
 exit(1);return;}_LL1DE:;}
# 1028
void Cyc_scan_decl(struct Cyc_Absyn_Decl*d,struct Cyc_Hashtable_Table*dep){
struct Cyc_Set_Set**_tmp15B=Cyc_current_targets;
struct _dyneither_ptr*_tmp15C=Cyc_current_source;
Cyc_current_targets=({struct Cyc_Set_Set**_tmp15D=_cycalloc(sizeof(*_tmp15D));_tmp15D[0]=((struct Cyc_Set_Set*(*)(int(*cmp)(struct _dyneither_ptr*,struct _dyneither_ptr*)))Cyc_Set_empty)(Cyc_strptrcmp);_tmp15D;});
{void*_tmp15E=d->r;void*_tmp15F=_tmp15E;struct Cyc_Absyn_Typedefdecl*_tmp193;struct Cyc_Absyn_Enumdecl*_tmp192;struct Cyc_Absyn_Aggrdecl*_tmp191;struct Cyc_Absyn_Fndecl*_tmp190;struct Cyc_Absyn_Vardecl*_tmp18F;switch(*((int*)_tmp15F)){case 0: _LL22A: _tmp18F=((struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct*)_tmp15F)->f1;_LL22B: {
# 1034
struct _tuple0*_tmp160=_tmp18F->name;struct _tuple0*_tmp161=_tmp160;struct _dyneither_ptr*_tmp162;_LL249: _tmp162=_tmp161->f2;_LL24A:;
Cyc_current_source=_tmp162;
Cyc_scan_type(_tmp18F->type,dep);
Cyc_scan_exp_opt(_tmp18F->initializer,dep);
goto _LL229;}case 1: _LL22C: _tmp190=((struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct*)_tmp15F)->f1;_LL22D: {
# 1040
struct _tuple0*_tmp163=_tmp190->name;struct _tuple0*_tmp164=_tmp163;struct _dyneither_ptr*_tmp16B;_LL24C: _tmp16B=_tmp164->f2;_LL24D:;
Cyc_current_source=_tmp16B;
Cyc_scan_type(_tmp190->ret_type,dep);
{struct Cyc_List_List*_tmp165=_tmp190->args;for(0;_tmp165 != 0;_tmp165=_tmp165->tl){
struct _tuple8*_tmp166=(struct _tuple8*)_tmp165->hd;struct _tuple8*_tmp167=_tmp166;void*_tmp168;_LL24F: _tmp168=_tmp167->f3;_LL250:;
Cyc_scan_type(_tmp168,dep);}}
# 1047
if(_tmp190->cyc_varargs != 0)
Cyc_scan_type(((struct Cyc_Absyn_VarargInfo*)_check_null(_tmp190->cyc_varargs))->type,dep);
if(_tmp190->is_inline)
({void*_tmp169=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp16A="Warning: ignoring inline function\n";_tag_dyneither(_tmp16A,sizeof(char),35);}),_tag_dyneither(_tmp169,sizeof(void*),0));});
# 1052
goto _LL229;}case 5: _LL22E: _tmp191=((struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct*)_tmp15F)->f1;_LL22F: {
# 1054
struct _tuple0*_tmp16C=_tmp191->name;struct _tuple0*_tmp16D=_tmp16C;struct _dyneither_ptr*_tmp171;_LL252: _tmp171=_tmp16D->f2;_LL253:;
Cyc_current_source=_tmp171;
if((unsigned int)_tmp191->impl){
{struct Cyc_List_List*_tmp16E=((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp191->impl))->fields;for(0;_tmp16E != 0;_tmp16E=_tmp16E->tl){
struct Cyc_Absyn_Aggrfield*_tmp16F=(struct Cyc_Absyn_Aggrfield*)_tmp16E->hd;
Cyc_scan_type(_tmp16F->type,dep);
Cyc_scan_exp_opt(_tmp16F->width,dep);}}{
# 1064
struct Cyc_List_List*_tmp170=((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp191->impl))->fields;for(0;_tmp170 != 0;_tmp170=_tmp170->tl){;}};}
# 1068
goto _LL229;}case 7: _LL230: _tmp192=((struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct*)_tmp15F)->f1;_LL231: {
# 1070
struct _tuple0*_tmp172=_tmp192->name;struct _tuple0*_tmp173=_tmp172;struct _dyneither_ptr*_tmp177;_LL255: _tmp177=_tmp173->f2;_LL256:;
Cyc_current_source=_tmp177;
if((unsigned int)_tmp192->fields){
{struct Cyc_List_List*_tmp174=(struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(_tmp192->fields))->v;for(0;_tmp174 != 0;_tmp174=_tmp174->tl){
struct Cyc_Absyn_Enumfield*_tmp175=(struct Cyc_Absyn_Enumfield*)_tmp174->hd;
Cyc_scan_exp_opt(_tmp175->tag,dep);}}{
# 1079
struct Cyc_List_List*_tmp176=(struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(_tmp192->fields))->v;for(0;_tmp176 != 0;_tmp176=_tmp176->tl){;}};}
# 1083
goto _LL229;}case 8: _LL232: _tmp193=((struct Cyc_Absyn_Typedef_d_Absyn_Raw_decl_struct*)_tmp15F)->f1;_LL233: {
# 1085
struct _tuple0*_tmp178=_tmp193->name;struct _tuple0*_tmp179=_tmp178;struct _dyneither_ptr*_tmp17A;_LL258: _tmp17A=_tmp179->f2;_LL259:;
Cyc_current_source=_tmp17A;
if((unsigned int)_tmp193->defn)
Cyc_scan_type((void*)_check_null(_tmp193->defn),dep);
goto _LL229;}case 4: _LL234: _LL235:
# 1091
({void*_tmp17B=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp17C="Error: unexpected region declaration";_tag_dyneither(_tmp17C,sizeof(char),37);}),_tag_dyneither(_tmp17B,sizeof(void*),0));});
 exit(1);case 13: _LL236: _LL237:
# 1097
({void*_tmp17D=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp17E="Error: unexpected __cyclone_port_on__";_tag_dyneither(_tmp17E,sizeof(char),38);}),_tag_dyneither(_tmp17D,sizeof(void*),0));});
 exit(1);case 14: _LL238: _LL239:
# 1100
({void*_tmp17F=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp180="Error: unexpected __cyclone_port_off__";_tag_dyneither(_tmp180,sizeof(char),39);}),_tag_dyneither(_tmp17F,sizeof(void*),0));});
 exit(1);case 2: _LL23A: _LL23B:
# 1103
({void*_tmp181=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp182="Error: unexpected let declaration\n";_tag_dyneither(_tmp182,sizeof(char),35);}),_tag_dyneither(_tmp181,sizeof(void*),0));});
 exit(1);case 6: _LL23C: _LL23D:
# 1106
({void*_tmp183=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp184="Error: unexpected datatype declaration\n";_tag_dyneither(_tmp184,sizeof(char),40);}),_tag_dyneither(_tmp183,sizeof(void*),0));});
 exit(1);case 3: _LL23E: _LL23F:
# 1109
({void*_tmp185=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp186="Error: unexpected let declaration\n";_tag_dyneither(_tmp186,sizeof(char),35);}),_tag_dyneither(_tmp185,sizeof(void*),0));});
 exit(1);case 9: _LL240: _LL241:
# 1112
({void*_tmp187=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp188="Error: unexpected namespace declaration\n";_tag_dyneither(_tmp188,sizeof(char),41);}),_tag_dyneither(_tmp187,sizeof(void*),0));});
 exit(1);case 10: _LL242: _LL243:
# 1115
({void*_tmp189=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp18A="Error: unexpected using declaration\n";_tag_dyneither(_tmp18A,sizeof(char),37);}),_tag_dyneither(_tmp189,sizeof(void*),0));});
 exit(1);case 11: _LL244: _LL245:
# 1118
({void*_tmp18B=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp18C="Error: unexpected extern \"C\" declaration\n";_tag_dyneither(_tmp18C,sizeof(char),42);}),_tag_dyneither(_tmp18B,sizeof(void*),0));});
 exit(1);default: _LL246: _LL247:
# 1121
({void*_tmp18D=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp18E="Error: unexpected extern \"C include\" declaration\n";_tag_dyneither(_tmp18E,sizeof(char),50);}),_tag_dyneither(_tmp18D,sizeof(void*),0));});
 exit(1);}_LL229:;}{
# 1129
struct Cyc_Set_Set*old;
struct _dyneither_ptr*_tmp194=(struct _dyneither_ptr*)_check_null(Cyc_current_source);
{struct _handler_cons _tmp195;_push_handler(& _tmp195);{int _tmp197=0;if(setjmp(_tmp195.handler))_tmp197=1;if(!_tmp197){
old=((struct Cyc_Set_Set*(*)(struct Cyc_Hashtable_Table*t,struct _dyneither_ptr*key))Cyc_Hashtable_lookup)(dep,_tmp194);;_pop_handler();}else{void*_tmp196=(void*)_exn_thrown;void*_tmp198=_tmp196;void*_tmp199;if(((struct Cyc_Core_Not_found_exn_struct*)_tmp198)->tag == Cyc_Core_Not_found){_LL25B: _LL25C:
# 1134
 old=((struct Cyc_Set_Set*(*)(int(*cmp)(struct _dyneither_ptr*,struct _dyneither_ptr*)))Cyc_Set_empty)(Cyc_strptrcmp);goto _LL25A;}else{_LL25D: _tmp199=_tmp198;_LL25E:(int)_rethrow(_tmp199);}_LL25A:;}};}{
# 1136
struct Cyc_Set_Set*_tmp19A=((struct Cyc_Set_Set*(*)(struct Cyc_Set_Set*s1,struct Cyc_Set_Set*s2))Cyc_Set_union_two)(*((struct Cyc_Set_Set**)_check_null(Cyc_current_targets)),old);
((void(*)(struct Cyc_Hashtable_Table*t,struct _dyneither_ptr*key,struct Cyc_Set_Set*val))Cyc_Hashtable_insert)(dep,_tmp194,_tmp19A);
# 1139
Cyc_current_targets=_tmp15B;
Cyc_current_source=_tmp15C;};};}
# 1143
struct Cyc_Hashtable_Table*Cyc_new_deps(){
return((struct Cyc_Hashtable_Table*(*)(int sz,int(*cmp)(struct _dyneither_ptr*,struct _dyneither_ptr*),int(*hash)(struct _dyneither_ptr*)))Cyc_Hashtable_create)(107,Cyc_strptrcmp,Cyc_Hashtable_hash_stringptr);}
# 1147
struct Cyc_Set_Set*Cyc_find(struct Cyc_Hashtable_Table*t,struct _dyneither_ptr*x){
struct _handler_cons _tmp19B;_push_handler(& _tmp19B);{int _tmp19D=0;if(setjmp(_tmp19B.handler))_tmp19D=1;if(!_tmp19D){{struct Cyc_Set_Set*_tmp19E=((struct Cyc_Set_Set*(*)(struct Cyc_Hashtable_Table*t,struct _dyneither_ptr*key))Cyc_Hashtable_lookup)(t,x);_npop_handler(0);return _tmp19E;};_pop_handler();}else{void*_tmp19C=(void*)_exn_thrown;void*_tmp19F=_tmp19C;void*_tmp1A0;if(((struct Cyc_Core_Not_found_exn_struct*)_tmp19F)->tag == Cyc_Core_Not_found){_LL260: _LL261:
# 1150
 return((struct Cyc_Set_Set*(*)(int(*cmp)(struct _dyneither_ptr*,struct _dyneither_ptr*)))Cyc_Set_empty)(Cyc_strptrcmp);}else{_LL262: _tmp1A0=_tmp19F;_LL263:(int)_rethrow(_tmp1A0);}_LL25F:;}};}
# 1154
struct Cyc_Set_Set*Cyc_reachable(struct Cyc_List_List*init,struct Cyc_Hashtable_Table*t){
# 1165 "buildlib.cyl"
struct Cyc_Set_Set*emptyset=((struct Cyc_Set_Set*(*)(int(*cmp)(struct _dyneither_ptr*,struct _dyneither_ptr*)))Cyc_Set_empty)(Cyc_strptrcmp);
struct Cyc_Set_Set*curr;
for(curr=emptyset;init != 0;init=init->tl){
curr=((struct Cyc_Set_Set*(*)(struct Cyc_Set_Set*s,struct _dyneither_ptr*elt))Cyc_Set_insert)(curr,(struct _dyneither_ptr*)init->hd);}{
struct Cyc_Set_Set*_tmp1A1=curr;
# 1171
struct _dyneither_ptr*_tmp1A2=_init_dyneither_ptr(_cycalloc(sizeof(struct _dyneither_ptr)),"",sizeof(char),1);
while(((int(*)(struct Cyc_Set_Set*s))Cyc_Set_cardinality)(_tmp1A1)> 0){
struct Cyc_Set_Set*_tmp1A3=emptyset;
struct Cyc_Iter_Iter _tmp1A4=((struct Cyc_Iter_Iter(*)(struct _RegionHandle*rgn,struct Cyc_Set_Set*s))Cyc_Set_make_iter)(Cyc_Core_heap_region,_tmp1A1);
while(((int(*)(struct Cyc_Iter_Iter,struct _dyneither_ptr**))Cyc_Iter_next)(_tmp1A4,& _tmp1A2)){
_tmp1A3=((struct Cyc_Set_Set*(*)(struct Cyc_Set_Set*s1,struct Cyc_Set_Set*s2))Cyc_Set_union_two)(_tmp1A3,Cyc_find(t,_tmp1A2));}
_tmp1A1=((struct Cyc_Set_Set*(*)(struct Cyc_Set_Set*s1,struct Cyc_Set_Set*s2))Cyc_Set_diff)(_tmp1A3,curr);
curr=((struct Cyc_Set_Set*(*)(struct Cyc_Set_Set*s1,struct Cyc_Set_Set*s2))Cyc_Set_union_two)(curr,_tmp1A1);}
# 1180
return curr;};}
# 1183
enum Cyc_buildlib_mode{Cyc_NORMAL  = 0,Cyc_GATHER  = 1,Cyc_GATHERSCRIPT  = 2,Cyc_FINISH  = 3};
static enum Cyc_buildlib_mode Cyc_mode=Cyc_NORMAL;
static int Cyc_gathering(){
return Cyc_mode == Cyc_GATHER  || Cyc_mode == Cyc_GATHERSCRIPT;}
# 1189
static struct Cyc___cycFILE*Cyc_script_file=0;
int Cyc_prscript(struct _dyneither_ptr fmt,struct _dyneither_ptr ap){
# 1193
if(Cyc_script_file == 0){
({void*_tmp1A6=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp1A7="Internal error: script file is NULL\n";_tag_dyneither(_tmp1A7,sizeof(char),37);}),_tag_dyneither(_tmp1A6,sizeof(void*),0));});
 exit(1);}
# 1197
return Cyc_vfprintf((struct Cyc___cycFILE*)_check_null(Cyc_script_file),fmt,ap);}
# 1200
int Cyc_force_directory(struct _dyneither_ptr d){
if(Cyc_mode == Cyc_GATHERSCRIPT)
({struct Cyc_String_pa_PrintArg_struct _tmp1AB;_tmp1AB.tag=0;_tmp1AB.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)d);({struct Cyc_String_pa_PrintArg_struct _tmp1AA;_tmp1AA.tag=0;_tmp1AA.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)d);({void*_tmp1A8[2]={& _tmp1AA,& _tmp1AB};Cyc_prscript(({const char*_tmp1A9="if ! test -e %s; then mkdir %s; fi\n";_tag_dyneither(_tmp1A9,sizeof(char),36);}),_tag_dyneither(_tmp1A8,sizeof(void*),2));});});});else{
# 1207
int _tmp1AC=({unsigned short _tmp1B0[0];Cyc_open((const char*)_untag_dyneither_ptr(d,sizeof(char),1),0,_tag_dyneither(_tmp1B0,sizeof(unsigned short),0));});
if(_tmp1AC == - 1){
if( mkdir((const char*)_check_null(_untag_dyneither_ptr(d,sizeof(char),1)),448)== - 1){
({struct Cyc_String_pa_PrintArg_struct _tmp1AF;_tmp1AF.tag=0;_tmp1AF.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)d);({void*_tmp1AD[1]={& _tmp1AF};Cyc_fprintf(Cyc_stderr,({const char*_tmp1AE="Error: could not create directory %s\n";_tag_dyneither(_tmp1AE,sizeof(char),38);}),_tag_dyneither(_tmp1AD,sizeof(void*),1));});});
return 1;}}else{
# 1214
 close(_tmp1AC);}}
# 1216
return 0;}
# 1219
int Cyc_force_directory_prefixes(struct _dyneither_ptr file){
# 1223
struct _dyneither_ptr _tmp1B1=Cyc_strdup((struct _dyneither_ptr)file);
# 1225
struct Cyc_List_List*_tmp1B2=0;
while(1){
_tmp1B1=Cyc_Filename_dirname((struct _dyneither_ptr)_tmp1B1);
if(Cyc_strlen((struct _dyneither_ptr)_tmp1B1)== 0)break;
_tmp1B2=({struct Cyc_List_List*_tmp1B3=_cycalloc(sizeof(*_tmp1B3));_tmp1B3->hd=({struct _dyneither_ptr*_tmp1B4=_cycalloc(sizeof(*_tmp1B4));_tmp1B4[0]=(struct _dyneither_ptr)_tmp1B1;_tmp1B4;});_tmp1B3->tl=_tmp1B2;_tmp1B3;});}
# 1232
for(0;_tmp1B2 != 0;_tmp1B2=_tmp1B2->tl){
if(Cyc_force_directory(*((struct _dyneither_ptr*)_tmp1B2->hd)))return 1;}
# 1235
return 0;}char Cyc_NO_SUPPORT[11]="NO_SUPPORT";struct Cyc_NO_SUPPORT_exn_struct{char*tag;struct _dyneither_ptr f1;};
# 1242
static int Cyc_is_other_special(char c){
char _tmp1B5=c;switch(_tmp1B5){case 92: _LL265: _LL266:
 goto _LL268;case 34: _LL267: _LL268:
 goto _LL26A;case 59: _LL269: _LL26A:
 goto _LL26C;case 38: _LL26B: _LL26C:
 goto _LL26E;case 40: _LL26D: _LL26E:
 goto _LL270;case 41: _LL26F: _LL270:
 goto _LL272;case 124: _LL271: _LL272:
 goto _LL274;case 94: _LL273: _LL274:
 goto _LL276;case 60: _LL275: _LL276:
 goto _LL278;case 62: _LL277: _LL278:
 goto _LL27A;case 10: _LL279: _LL27A:
# 1257
 goto _LL27C;case 9: _LL27B: _LL27C:
 return 1;default: _LL27D: _LL27E:
 return 0;}_LL264:;}
# 1263
static struct _dyneither_ptr Cyc_sh_escape_string(struct _dyneither_ptr s){
unsigned long _tmp1B6=Cyc_strlen((struct _dyneither_ptr)s);
# 1267
int _tmp1B7=0;
int _tmp1B8=0;
{int i=0;for(0;i < _tmp1B6;++ i){
char _tmp1B9=*((const char*)_check_dyneither_subscript(s,sizeof(char),i));
if(_tmp1B9 == '\'')++ _tmp1B7;else{
if(Cyc_is_other_special(_tmp1B9))++ _tmp1B8;}}}
# 1276
if(_tmp1B7 == 0  && _tmp1B8 == 0)
return s;
# 1280
if(_tmp1B7 == 0)
return(struct _dyneither_ptr)Cyc_strconcat_l(({struct _dyneither_ptr*_tmp1BA[3];_tmp1BA[2]=_init_dyneither_ptr(_cycalloc(sizeof(struct _dyneither_ptr)),"'",sizeof(char),2);_tmp1BA[1]=({struct _dyneither_ptr*_tmp1BC=_cycalloc(sizeof(*_tmp1BC));_tmp1BC[0]=(struct _dyneither_ptr)s;_tmp1BC;});_tmp1BA[0]=_init_dyneither_ptr(_cycalloc(sizeof(struct _dyneither_ptr)),"'",sizeof(char),2);((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp1BA,sizeof(struct _dyneither_ptr*),3));}));{
# 1284
unsigned long _tmp1BE=(_tmp1B6 + _tmp1B7)+ _tmp1B8;
struct _dyneither_ptr s2=({unsigned int _tmp1C8=_tmp1BE + 1;char*_tmp1C9=(char*)_cycalloc_atomic(_check_times(sizeof(char),_tmp1C8 + 1));struct _dyneither_ptr _tmp1CB=_tag_dyneither(_tmp1C9,sizeof(char),_tmp1C8 + 1);{unsigned int _tmp1CA=_tmp1C8;unsigned int i;for(i=0;i < _tmp1CA;i ++){_tmp1C9[i]=(char)'\000';}_tmp1C9[_tmp1CA]=(char)0;}_tmp1CB;});
int _tmp1BF=0;
int _tmp1C0=0;
for(0;_tmp1BF < _tmp1B6;++ _tmp1BF){
char _tmp1C1=*((const char*)_check_dyneither_subscript(s,sizeof(char),_tmp1BF));
if(_tmp1C1 == '\''  || Cyc_is_other_special(_tmp1C1))
({struct _dyneither_ptr _tmp1C2=_dyneither_ptr_plus(s2,sizeof(char),_tmp1C0 ++);char _tmp1C3=*((char*)_check_dyneither_subscript(_tmp1C2,sizeof(char),0));char _tmp1C4='\\';if(_get_dyneither_size(_tmp1C2,sizeof(char))== 1  && (_tmp1C3 == '\000'  && _tmp1C4 != '\000'))_throw_arraybounds();*((char*)_tmp1C2.curr)=_tmp1C4;});
({struct _dyneither_ptr _tmp1C5=_dyneither_ptr_plus(s2,sizeof(char),_tmp1C0 ++);char _tmp1C6=*((char*)_check_dyneither_subscript(_tmp1C5,sizeof(char),0));char _tmp1C7=_tmp1C1;if(_get_dyneither_size(_tmp1C5,sizeof(char))== 1  && (_tmp1C6 == '\000'  && _tmp1C7 != '\000'))_throw_arraybounds();*((char*)_tmp1C5.curr)=_tmp1C7;});}
# 1294
return(struct _dyneither_ptr)s2;};}
# 1296
static struct _dyneither_ptr*Cyc_sh_escape_stringptr(struct _dyneither_ptr*sp){
return({struct _dyneither_ptr*_tmp1CC=_cycalloc(sizeof(*_tmp1CC));_tmp1CC[0]=Cyc_sh_escape_string(*sp);_tmp1CC;});}struct _tuple25{struct Cyc_List_List*f1;struct Cyc_List_List*f2;};
# 1301
int Cyc_process_file(const char*filename,struct Cyc_List_List*start_symbols,struct Cyc_List_List*omit_symbols,struct Cyc_List_List*hstubs,struct Cyc_List_List*cstubs,struct Cyc_List_List*cycstubs){
# 1307
struct Cyc___cycFILE*maybe;
struct Cyc___cycFILE*in_file;
struct Cyc___cycFILE*out_file;
int errorcode=0;
# 1312
({struct Cyc_String_pa_PrintArg_struct _tmp1CF;_tmp1CF.tag=0;_tmp1CF.f1=(struct _dyneither_ptr)({const char*_tmp1D0=filename;_tag_dyneither(_tmp1D0,sizeof(char),_get_zero_arr_size_char((void*)_tmp1D0,1));});({void*_tmp1CD[1]={& _tmp1CF};Cyc_fprintf(Cyc_stderr,({const char*_tmp1CE="********************************* %s...\n";_tag_dyneither(_tmp1CE,sizeof(char),41);}),_tag_dyneither(_tmp1CD,sizeof(void*),1));});});
# 1315
if(!Cyc_gathering())({struct Cyc_String_pa_PrintArg_struct _tmp1D3;_tmp1D3.tag=0;_tmp1D3.f1=(struct _dyneither_ptr)({const char*_tmp1D4=filename;_tag_dyneither(_tmp1D4,sizeof(char),_get_zero_arr_size_char((void*)_tmp1D4,1));});({void*_tmp1D1[1]={& _tmp1D3};Cyc_log(({const char*_tmp1D2="\n%s:\n";_tag_dyneither(_tmp1D2,sizeof(char),6);}),_tag_dyneither(_tmp1D1,sizeof(void*),1));});});{
# 1327 "buildlib.cyl"
struct _dyneither_ptr _tmp1D5=Cyc_Filename_basename(({const char*_tmp319=filename;_tag_dyneither(_tmp319,sizeof(char),_get_zero_arr_size_char((void*)_tmp319,1));}));
struct _dyneither_ptr _tmp1D6=Cyc_Filename_dirname(({const char*_tmp318=filename;_tag_dyneither(_tmp318,sizeof(char),_get_zero_arr_size_char((void*)_tmp318,1));}));
struct _dyneither_ptr _tmp1D7=Cyc_Filename_chop_extension((struct _dyneither_ptr)_tmp1D5);
const char*_tmp1D8=(const char*)_check_null(_untag_dyneither_ptr(Cyc_strconcat((struct _dyneither_ptr)_tmp1D7,({const char*_tmp317=".iA";_tag_dyneither(_tmp317,sizeof(char),4);})),sizeof(char),1));
const char*_tmp1D9=(const char*)_check_null(_untag_dyneither_ptr(_get_dyneither_size(_tmp1D6,sizeof(char))== 0?({struct Cyc_String_pa_PrintArg_struct _tmp313;_tmp313.tag=0;_tmp313.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp1D7);({void*_tmp311[1]={& _tmp313};Cyc_aprintf(({const char*_tmp312="%s.iB";_tag_dyneither(_tmp312,sizeof(char),6);}),_tag_dyneither(_tmp311,sizeof(void*),1));});}): Cyc_Filename_concat((struct _dyneither_ptr)_tmp1D6,(struct _dyneither_ptr)({struct Cyc_String_pa_PrintArg_struct _tmp316;_tmp316.tag=0;_tmp316.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp1D7);({void*_tmp314[1]={& _tmp316};Cyc_aprintf(({const char*_tmp315="%s.iB";_tag_dyneither(_tmp315,sizeof(char),6);}),_tag_dyneither(_tmp314,sizeof(void*),1));});})),sizeof(char),1));
# 1335
const char*_tmp1DA=(const char*)_check_null(_untag_dyneither_ptr(_get_dyneither_size(_tmp1D6,sizeof(char))== 0?({struct Cyc_String_pa_PrintArg_struct _tmp30D;_tmp30D.tag=0;_tmp30D.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp1D7);({void*_tmp30B[1]={& _tmp30D};Cyc_aprintf(({const char*_tmp30C="%s.iC";_tag_dyneither(_tmp30C,sizeof(char),6);}),_tag_dyneither(_tmp30B,sizeof(void*),1));});}): Cyc_Filename_concat((struct _dyneither_ptr)_tmp1D6,(struct _dyneither_ptr)({struct Cyc_String_pa_PrintArg_struct _tmp310;_tmp310.tag=0;_tmp310.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp1D7);({void*_tmp30E[1]={& _tmp310};Cyc_aprintf(({const char*_tmp30F="%s.iC";_tag_dyneither(_tmp30F,sizeof(char),6);}),_tag_dyneither(_tmp30E,sizeof(void*),1));});})),sizeof(char),1));
# 1339
const char*_tmp1DB=(const char*)_check_null(_untag_dyneither_ptr(_get_dyneither_size(_tmp1D6,sizeof(char))== 0?({struct Cyc_String_pa_PrintArg_struct _tmp307;_tmp307.tag=0;_tmp307.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp1D7);({void*_tmp305[1]={& _tmp307};Cyc_aprintf(({const char*_tmp306="%s.iD";_tag_dyneither(_tmp306,sizeof(char),6);}),_tag_dyneither(_tmp305,sizeof(void*),1));});}): Cyc_Filename_concat((struct _dyneither_ptr)_tmp1D6,(struct _dyneither_ptr)({struct Cyc_String_pa_PrintArg_struct _tmp30A;_tmp30A.tag=0;_tmp30A.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp1D7);({void*_tmp308[1]={& _tmp30A};Cyc_aprintf(({const char*_tmp309="%s.iD";_tag_dyneither(_tmp309,sizeof(char),6);}),_tag_dyneither(_tmp308,sizeof(void*),1));});})),sizeof(char),1));
# 1344
{struct _handler_cons _tmp1DC;_push_handler(& _tmp1DC);{int _tmp1DE=0;if(setjmp(_tmp1DC.handler))_tmp1DE=1;if(!_tmp1DE){
# 1347
if(Cyc_force_directory_prefixes(({const char*_tmp1DF=filename;_tag_dyneither(_tmp1DF,sizeof(char),_get_zero_arr_size_char((void*)_tmp1DF,1));}))){
int _tmp1E0=1;_npop_handler(0);return _tmp1E0;}
# 1352
if(Cyc_mode != Cyc_FINISH){
Cyc_current_cpp=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(Cyc_current_cpp);
if(Cyc_mode == Cyc_GATHERSCRIPT){
({struct Cyc_String_pa_PrintArg_struct _tmp1E3;_tmp1E3.tag=0;_tmp1E3.f1=(struct _dyneither_ptr)({const char*_tmp1E4=_tmp1D8;_tag_dyneither(_tmp1E4,sizeof(char),_get_zero_arr_size_char((void*)_tmp1E4,1));});({void*_tmp1E1[1]={& _tmp1E3};Cyc_prscript(({const char*_tmp1E2="cat >%s <<XXX\n";_tag_dyneither(_tmp1E2,sizeof(char),15);}),_tag_dyneither(_tmp1E1,sizeof(void*),1));});});
{struct Cyc_List_List*_tmp1E5=Cyc_current_cpp;for(0;_tmp1E5 != 0;_tmp1E5=_tmp1E5->tl){
({struct Cyc_String_pa_PrintArg_struct _tmp1E8;_tmp1E8.tag=0;_tmp1E8.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*((struct _dyneither_ptr*)_tmp1E5->hd));({void*_tmp1E6[1]={& _tmp1E8};Cyc_prscript(({const char*_tmp1E7="%s";_tag_dyneither(_tmp1E7,sizeof(char),3);}),_tag_dyneither(_tmp1E6,sizeof(void*),1));});});}}
({struct Cyc_String_pa_PrintArg_struct _tmp1EB;_tmp1EB.tag=0;_tmp1EB.f1=(struct _dyneither_ptr)({const char*_tmp1EC=filename;_tag_dyneither(_tmp1EC,sizeof(char),_get_zero_arr_size_char((void*)_tmp1EC,1));});({void*_tmp1E9[1]={& _tmp1EB};Cyc_prscript(({const char*_tmp1EA="#include <%s>\n";_tag_dyneither(_tmp1EA,sizeof(char),15);}),_tag_dyneither(_tmp1E9,sizeof(void*),1));});});
({void*_tmp1ED=0;Cyc_prscript(({const char*_tmp1EE="XXX\n";_tag_dyneither(_tmp1EE,sizeof(char),5);}),_tag_dyneither(_tmp1ED,sizeof(void*),0));});
({struct Cyc_String_pa_PrintArg_struct _tmp1F2;_tmp1F2.tag=0;_tmp1F2.f1=(struct _dyneither_ptr)({const char*_tmp1F4=_tmp1D8;_tag_dyneither(_tmp1F4,sizeof(char),_get_zero_arr_size_char((void*)_tmp1F4,1));});({struct Cyc_String_pa_PrintArg_struct _tmp1F1;_tmp1F1.tag=0;_tmp1F1.f1=(struct _dyneither_ptr)({const char*_tmp1F3=_tmp1D9;_tag_dyneither(_tmp1F3,sizeof(char),_get_zero_arr_size_char((void*)_tmp1F3,1));});({void*_tmp1EF[2]={& _tmp1F1,& _tmp1F2};Cyc_prscript(({const char*_tmp1F0="$GCC -E -dM -o %s -x c %s && \\\n";_tag_dyneither(_tmp1F0,sizeof(char),32);}),_tag_dyneither(_tmp1EF,sizeof(void*),2));});});});
({struct Cyc_String_pa_PrintArg_struct _tmp1F8;_tmp1F8.tag=0;_tmp1F8.f1=(struct _dyneither_ptr)({const char*_tmp1FA=_tmp1D8;_tag_dyneither(_tmp1FA,sizeof(char),_get_zero_arr_size_char((void*)_tmp1FA,1));});({struct Cyc_String_pa_PrintArg_struct _tmp1F7;_tmp1F7.tag=0;_tmp1F7.f1=(struct _dyneither_ptr)({const char*_tmp1F9=_tmp1DA;_tag_dyneither(_tmp1F9,sizeof(char),_get_zero_arr_size_char((void*)_tmp1F9,1));});({void*_tmp1F5[2]={& _tmp1F7,& _tmp1F8};Cyc_prscript(({const char*_tmp1F6="$GCC -E     -o %s -x c %s;\n";_tag_dyneither(_tmp1F6,sizeof(char),28);}),_tag_dyneither(_tmp1F5,sizeof(void*),2));});});});
({struct Cyc_String_pa_PrintArg_struct _tmp1FD;_tmp1FD.tag=0;_tmp1FD.f1=(struct _dyneither_ptr)({const char*_tmp1FE=_tmp1D8;_tag_dyneither(_tmp1FE,sizeof(char),_get_zero_arr_size_char((void*)_tmp1FE,1));});({void*_tmp1FB[1]={& _tmp1FD};Cyc_prscript(({const char*_tmp1FC="rm %s\n";_tag_dyneither(_tmp1FC,sizeof(char),7);}),_tag_dyneither(_tmp1FB,sizeof(void*),1));});});}else{
# 1365
maybe=Cyc_fopen(_tmp1D8,"w");
if(!((unsigned int)maybe)){
({struct Cyc_String_pa_PrintArg_struct _tmp201;_tmp201.tag=0;_tmp201.f1=(struct _dyneither_ptr)({const char*_tmp202=_tmp1D8;_tag_dyneither(_tmp202,sizeof(char),_get_zero_arr_size_char((void*)_tmp202,1));});({void*_tmp1FF[1]={& _tmp201};Cyc_fprintf(Cyc_stderr,({const char*_tmp200="Error: could not create file %s\n";_tag_dyneither(_tmp200,sizeof(char),33);}),_tag_dyneither(_tmp1FF,sizeof(void*),1));});});{
int _tmp203=1;_npop_handler(0);return _tmp203;};}
# 1370
if(Cyc_verbose)
({struct Cyc_String_pa_PrintArg_struct _tmp206;_tmp206.tag=0;_tmp206.f1=(struct _dyneither_ptr)({const char*_tmp207=_tmp1D8;_tag_dyneither(_tmp207,sizeof(char),_get_zero_arr_size_char((void*)_tmp207,1));});({void*_tmp204[1]={& _tmp206};Cyc_fprintf(Cyc_stderr,({const char*_tmp205="Creating %s\n";_tag_dyneither(_tmp205,sizeof(char),13);}),_tag_dyneither(_tmp204,sizeof(void*),1));});});
out_file=maybe;
{struct Cyc_List_List*_tmp208=Cyc_current_cpp;for(0;_tmp208 != 0;_tmp208=_tmp208->tl){
Cyc_fputs((const char*)_check_null(_untag_dyneither_ptr(*((struct _dyneither_ptr*)_tmp208->hd),sizeof(char),1)),out_file);}}
# 1376
({struct Cyc_String_pa_PrintArg_struct _tmp20B;_tmp20B.tag=0;_tmp20B.f1=(struct _dyneither_ptr)({const char*_tmp20C=filename;_tag_dyneither(_tmp20C,sizeof(char),_get_zero_arr_size_char((void*)_tmp20C,1));});({void*_tmp209[1]={& _tmp20B};Cyc_fprintf(out_file,({const char*_tmp20A="#include <%s>\n";_tag_dyneither(_tmp20A,sizeof(char),15);}),_tag_dyneither(_tmp209,sizeof(void*),1));});});
Cyc_fclose(out_file);{
struct _dyneither_ptr _tmp20D= Cstring_to_string( Ccomp);
struct _dyneither_ptr _tmp20E=
Cyc_str_sepstr(({struct Cyc_List_List*_tmp227=_cycalloc(sizeof(*_tmp227));_tmp227->hd=({struct _dyneither_ptr*_tmp228=_cycalloc(sizeof(*_tmp228));_tmp228[0]=(struct _dyneither_ptr)({const char*_tmp229="";_tag_dyneither(_tmp229,sizeof(char),1);});_tmp228;});_tmp227->tl=
((struct Cyc_List_List*(*)(struct _dyneither_ptr*(*f)(struct _dyneither_ptr*),struct Cyc_List_List*x))Cyc_List_map)(Cyc_sh_escape_stringptr,((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_rev)(Cyc_cppargs));_tmp227;}),({const char*_tmp22A=" ";_tag_dyneither(_tmp22A,sizeof(char),2);}));
# 1383
char*cmd=(char*)_check_null(_untag_dyneither_ptr(({struct Cyc_String_pa_PrintArg_struct _tmp224;_tmp224.tag=0;_tmp224.f1=(struct _dyneither_ptr)({const char*_tmp226=_tmp1D8;_tag_dyneither(_tmp226,sizeof(char),_get_zero_arr_size_char((void*)_tmp226,1));});({struct Cyc_String_pa_PrintArg_struct _tmp223;_tmp223.tag=0;_tmp223.f1=(struct _dyneither_ptr)({const char*_tmp225=_tmp1D9;_tag_dyneither(_tmp225,sizeof(char),_get_zero_arr_size_char((void*)_tmp225,1));});({struct Cyc_String_pa_PrintArg_struct _tmp222;_tmp222.tag=0;_tmp222.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp20E);({struct Cyc_String_pa_PrintArg_struct _tmp221;_tmp221.tag=0;_tmp221.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp20D);({void*_tmp21F[4]={& _tmp221,& _tmp222,& _tmp223,& _tmp224};Cyc_aprintf(({const char*_tmp220="%s %s -E -dM -o %s -x c %s";_tag_dyneither(_tmp220,sizeof(char),27);}),_tag_dyneither(_tmp21F,sizeof(void*),4));});});});});}),sizeof(char),1));
# 1385
if(Cyc_verbose)
({struct Cyc_String_pa_PrintArg_struct _tmp211;_tmp211.tag=0;_tmp211.f1=(struct _dyneither_ptr)({char*_tmp212=cmd;_tag_dyneither(_tmp212,sizeof(char),_get_zero_arr_size_char((void*)_tmp212,1));});({void*_tmp20F[1]={& _tmp211};Cyc_fprintf(Cyc_stderr,({const char*_tmp210="%s\n";_tag_dyneither(_tmp210,sizeof(char),4);}),_tag_dyneither(_tmp20F,sizeof(void*),1));});});
if(! system((const char*)cmd)){
# 1390
cmd=(char*)_check_null(_untag_dyneither_ptr(({struct Cyc_String_pa_PrintArg_struct _tmp218;_tmp218.tag=0;_tmp218.f1=(struct _dyneither_ptr)({const char*_tmp21A=_tmp1D8;_tag_dyneither(_tmp21A,sizeof(char),_get_zero_arr_size_char((void*)_tmp21A,1));});({struct Cyc_String_pa_PrintArg_struct _tmp217;_tmp217.tag=0;_tmp217.f1=(struct _dyneither_ptr)({const char*_tmp219=_tmp1DA;_tag_dyneither(_tmp219,sizeof(char),_get_zero_arr_size_char((void*)_tmp219,1));});({struct Cyc_String_pa_PrintArg_struct _tmp216;_tmp216.tag=0;_tmp216.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp20E);({struct Cyc_String_pa_PrintArg_struct _tmp215;_tmp215.tag=0;_tmp215.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp20D);({void*_tmp213[4]={& _tmp215,& _tmp216,& _tmp217,& _tmp218};Cyc_aprintf(({const char*_tmp214="%s %s -E -o %s -x c %s";_tag_dyneither(_tmp214,sizeof(char),23);}),_tag_dyneither(_tmp213,sizeof(void*),4));});});});});}),sizeof(char),1));
# 1392
if(Cyc_verbose)
({struct Cyc_String_pa_PrintArg_struct _tmp21D;_tmp21D.tag=0;_tmp21D.f1=(struct _dyneither_ptr)({char*_tmp21E=cmd;_tag_dyneither(_tmp21E,sizeof(char),_get_zero_arr_size_char((void*)_tmp21E,1));});({void*_tmp21B[1]={& _tmp21D};Cyc_fprintf(Cyc_stderr,({const char*_tmp21C="%s\n";_tag_dyneither(_tmp21C,sizeof(char),4);}),_tag_dyneither(_tmp21B,sizeof(void*),1));});});
 system((const char*)cmd);}};}}
# 1399
if(Cyc_gathering()){int _tmp22B=0;_npop_handler(0);return _tmp22B;}{
# 1402
struct Cyc_Hashtable_Table*t=Cyc_new_deps();
maybe=Cyc_fopen(_tmp1D9,"r");
if(!((unsigned int)maybe))(int)_throw((void*)({struct Cyc_NO_SUPPORT_exn_struct*_tmp22C=_cycalloc(sizeof(*_tmp22C));_tmp22C[0]=({struct Cyc_NO_SUPPORT_exn_struct _tmp22D;_tmp22D.tag=Cyc_NO_SUPPORT;_tmp22D.f1=(struct _dyneither_ptr)({struct Cyc_String_pa_PrintArg_struct _tmp230;_tmp230.tag=0;_tmp230.f1=(struct _dyneither_ptr)({const char*_tmp231=_tmp1D9;_tag_dyneither(_tmp231,sizeof(char),_get_zero_arr_size_char((void*)_tmp231,1));});({void*_tmp22E[1]={& _tmp230};Cyc_aprintf(({const char*_tmp22F="can't open macrosfile %s";_tag_dyneither(_tmp22F,sizeof(char),25);}),_tag_dyneither(_tmp22E,sizeof(void*),1));});});_tmp22D;});_tmp22C;}));
# 1407
in_file=maybe;{
struct Cyc_Lexing_lexbuf*_tmp232=Cyc_Lexing_from_file(in_file);
struct _tuple20*entry;
while((entry=((struct _tuple20*(*)(struct Cyc_Lexing_lexbuf*lexbuf))Cyc_line)(_tmp232))!= 0){
struct _tuple20*_tmp233=(struct _tuple20*)_check_null(entry);struct _tuple20*_tmp234=_tmp233;struct _dyneither_ptr*_tmp236;struct Cyc_Set_Set*_tmp235;_LL280: _tmp236=_tmp234->f1;_tmp235=_tmp234->f2;_LL281:;
((void(*)(struct Cyc_Hashtable_Table*t,struct _dyneither_ptr*key,struct Cyc_Set_Set*val))Cyc_Hashtable_insert)(t,_tmp236,_tmp235);}
# 1416
Cyc_fclose(in_file);
# 1419
maybe=Cyc_fopen(_tmp1DA,"r");
if(!((unsigned int)maybe))(int)_throw((void*)({struct Cyc_NO_SUPPORT_exn_struct*_tmp237=_cycalloc(sizeof(*_tmp237));_tmp237[0]=({struct Cyc_NO_SUPPORT_exn_struct _tmp238;_tmp238.tag=Cyc_NO_SUPPORT;_tmp238.f1=(struct _dyneither_ptr)({struct Cyc_String_pa_PrintArg_struct _tmp23B;_tmp23B.tag=0;_tmp23B.f1=(struct _dyneither_ptr)({const char*_tmp23C=_tmp1DA;_tag_dyneither(_tmp23C,sizeof(char),_get_zero_arr_size_char((void*)_tmp23C,1));});({void*_tmp239[1]={& _tmp23B};Cyc_aprintf(({const char*_tmp23A="can't open declsfile %s";_tag_dyneither(_tmp23A,sizeof(char),24);}),_tag_dyneither(_tmp239,sizeof(void*),1));});});_tmp238;});_tmp237;}));
# 1423
in_file=maybe;
_tmp232=Cyc_Lexing_from_file(in_file);
Cyc_slurp_out=Cyc_fopen(_tmp1DB,"w");
if(!((unsigned int)Cyc_slurp_out)){int _tmp23D=1;_npop_handler(0);return _tmp23D;}
while(((int(*)(struct Cyc_Lexing_lexbuf*lexbuf))Cyc_slurp)(_tmp232)){;}
Cyc_fclose(in_file);
Cyc_fclose((struct Cyc___cycFILE*)_check_null(Cyc_slurp_out));
if(Cyc_mode != Cyc_FINISH)
;
# 1433
maybe=Cyc_fopen(_tmp1DB,"r");
if(!((unsigned int)maybe)){int _tmp23E=1;_npop_handler(0);return _tmp23E;}
in_file=maybe;
Cyc_Position_reset_position(({const char*_tmp23F=_tmp1DB;_tag_dyneither(_tmp23F,sizeof(char),_get_zero_arr_size_char((void*)_tmp23F,1));}));
Cyc_Lex_lex_init(0);{
struct Cyc_List_List*_tmp240=Cyc_Parse_parse_file(in_file);
Cyc_Lex_lex_init(0);
Cyc_fclose(in_file);
# 1443
{struct Cyc_List_List*_tmp241=_tmp240;for(0;_tmp241 != 0;_tmp241=_tmp241->tl){
Cyc_scan_decl((struct Cyc_Absyn_Decl*)_tmp241->hd,t);}}{
# 1447
struct Cyc_Set_Set*_tmp242=Cyc_reachable(start_symbols,t);
# 1450
struct Cyc_List_List*_tmp243=0;
struct Cyc_Set_Set*defined_symbols=((struct Cyc_Set_Set*(*)(int(*cmp)(struct _dyneither_ptr*,struct _dyneither_ptr*)))Cyc_Set_empty)(Cyc_strptrcmp);
{struct Cyc_List_List*_tmp244=_tmp240;for(0;_tmp244 != 0;_tmp244=_tmp244->tl){
struct Cyc_Absyn_Decl*_tmp245=(struct Cyc_Absyn_Decl*)_tmp244->hd;
struct _dyneither_ptr*name;
{void*_tmp246=_tmp245->r;void*_tmp247=_tmp246;struct Cyc_Absyn_Typedefdecl*_tmp262;struct Cyc_Absyn_Enumdecl*_tmp261;struct Cyc_Absyn_Aggrdecl*_tmp260;struct Cyc_Absyn_Fndecl*_tmp25F;struct Cyc_Absyn_Vardecl*_tmp25E;switch(*((int*)_tmp247)){case 0: _LL283: _tmp25E=((struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct*)_tmp247)->f1;_LL284: {
# 1459
struct _tuple0*_tmp248=_tmp25E->name;struct _tuple0*_tmp249=_tmp248;struct _dyneither_ptr*_tmp24A;_LL2A2: _tmp24A=_tmp249->f2;_LL2A3:;
defined_symbols=((struct Cyc_Set_Set*(*)(struct Cyc_Set_Set*s,struct _dyneither_ptr*elt))Cyc_Set_insert)(defined_symbols,_tmp24A);
if(((int(*)(int(*compare)(struct _dyneither_ptr*,struct _dyneither_ptr*),struct Cyc_List_List*l,struct _dyneither_ptr*x))Cyc_List_mem)(Cyc_strptrcmp,omit_symbols,_tmp24A))name=0;else{
name=_tmp24A;}
goto _LL282;}case 1: _LL285: _tmp25F=((struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct*)_tmp247)->f1;_LL286: {
# 1465
struct _tuple0*_tmp24B=_tmp25F->name;struct _tuple0*_tmp24C=_tmp24B;struct _dyneither_ptr*_tmp24D;_LL2A5: _tmp24D=_tmp24C->f2;_LL2A6:;
defined_symbols=((struct Cyc_Set_Set*(*)(struct Cyc_Set_Set*s,struct _dyneither_ptr*elt))Cyc_Set_insert)(defined_symbols,_tmp24D);
if(((int(*)(int(*compare)(struct _dyneither_ptr*,struct _dyneither_ptr*),struct Cyc_List_List*l,struct _dyneither_ptr*x))Cyc_List_mem)(Cyc_strptrcmp,omit_symbols,_tmp24D))name=0;else{
name=_tmp24D;}
goto _LL282;}case 5: _LL287: _tmp260=((struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct*)_tmp247)->f1;_LL288: {
# 1471
struct _tuple0*_tmp24E=_tmp260->name;struct _tuple0*_tmp24F=_tmp24E;struct _dyneither_ptr*_tmp250;_LL2A8: _tmp250=_tmp24F->f2;_LL2A9:;
name=_tmp250;
goto _LL282;}case 7: _LL289: _tmp261=((struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct*)_tmp247)->f1;_LL28A: {
# 1475
struct _tuple0*_tmp251=_tmp261->name;struct _tuple0*_tmp252=_tmp251;struct _dyneither_ptr*_tmp25A;_LL2AB: _tmp25A=_tmp252->f2;_LL2AC:;
name=_tmp25A;
# 1479
if(name != 0  && ((int(*)(struct Cyc_Set_Set*s,struct _dyneither_ptr*elt))Cyc_Set_member)(_tmp242,name))
_tmp243=({struct Cyc_List_List*_tmp253=_cycalloc(sizeof(*_tmp253));_tmp253->hd=_tmp245;_tmp253->tl=_tmp243;_tmp253;});else{
# 1482
if((unsigned int)_tmp261->fields){
struct Cyc_List_List*_tmp254=(struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(_tmp261->fields))->v;for(0;_tmp254 != 0;_tmp254=_tmp254->tl){
struct Cyc_Absyn_Enumfield*_tmp255=(struct Cyc_Absyn_Enumfield*)_tmp254->hd;
struct _tuple0*_tmp256=_tmp255->name;struct _tuple0*_tmp257=_tmp256;struct _dyneither_ptr*_tmp259;_LL2AE: _tmp259=_tmp257->f2;_LL2AF:;
if(((int(*)(struct Cyc_Set_Set*s,struct _dyneither_ptr*elt))Cyc_Set_member)(_tmp242,_tmp259)){
_tmp243=({struct Cyc_List_List*_tmp258=_cycalloc(sizeof(*_tmp258));_tmp258->hd=_tmp245;_tmp258->tl=_tmp243;_tmp258;});
break;}}}}
# 1492
name=0;
goto _LL282;}case 8: _LL28B: _tmp262=((struct Cyc_Absyn_Typedef_d_Absyn_Raw_decl_struct*)_tmp247)->f1;_LL28C: {
# 1495
struct _tuple0*_tmp25B=_tmp262->name;struct _tuple0*_tmp25C=_tmp25B;struct _dyneither_ptr*_tmp25D;_LL2B1: _tmp25D=_tmp25C->f2;_LL2B2:;
name=_tmp25D;
goto _LL282;}case 13: _LL28D: _LL28E:
 goto _LL290;case 14: _LL28F: _LL290:
 goto _LL292;case 2: _LL291: _LL292:
 goto _LL294;case 6: _LL293: _LL294:
 goto _LL296;case 3: _LL295: _LL296:
 goto _LL298;case 9: _LL297: _LL298:
 goto _LL29A;case 10: _LL299: _LL29A:
 goto _LL29C;case 11: _LL29B: _LL29C:
 goto _LL29E;case 12: _LL29D: _LL29E:
 goto _LL2A0;default: _LL29F: _LL2A0:
# 1509
 name=0;
goto _LL282;}_LL282:;}
# 1512
if(name != 0  && ((int(*)(struct Cyc_Set_Set*s,struct _dyneither_ptr*elt))Cyc_Set_member)(_tmp242,name))
_tmp243=({struct Cyc_List_List*_tmp263=_cycalloc(sizeof(*_tmp263));_tmp263->hd=_tmp245;_tmp263->tl=_tmp243;_tmp263;});}}
# 1517
if(!Cyc_do_setjmp){
maybe=Cyc_fopen(filename,"w");
if(!((unsigned int)maybe)){int _tmp264=1;_npop_handler(0);return _tmp264;}
out_file=maybe;}else{
out_file=Cyc_stdout;}{
struct _dyneither_ptr ifdefmacro=({struct Cyc_String_pa_PrintArg_struct _tmp2DE;_tmp2DE.tag=0;_tmp2DE.f1=(struct _dyneither_ptr)({const char*_tmp2DF=filename;_tag_dyneither(_tmp2DF,sizeof(char),_get_zero_arr_size_char((void*)_tmp2DF,1));});({void*_tmp2DC[1]={& _tmp2DE};Cyc_aprintf(({const char*_tmp2DD="_%s_";_tag_dyneither(_tmp2DD,sizeof(char),5);}),_tag_dyneither(_tmp2DC,sizeof(void*),1));});});
{int _tmp265=0;for(0;_tmp265 < _get_dyneither_size(ifdefmacro,sizeof(char));++ _tmp265){
if(((char*)ifdefmacro.curr)[_tmp265]== '.'  || ((char*)ifdefmacro.curr)[_tmp265]== '/')
({struct _dyneither_ptr _tmp266=_dyneither_ptr_plus(ifdefmacro,sizeof(char),_tmp265);char _tmp267=*((char*)_check_dyneither_subscript(_tmp266,sizeof(char),0));char _tmp268='_';if(_get_dyneither_size(_tmp266,sizeof(char))== 1  && (_tmp267 == '\000'  && _tmp268 != '\000'))_throw_arraybounds();*((char*)_tmp266.curr)=_tmp268;});else{
if(((char*)ifdefmacro.curr)[_tmp265]!= '_'  && ((char*)ifdefmacro.curr)[_tmp265]!= '/')
({struct _dyneither_ptr _tmp269=_dyneither_ptr_plus(ifdefmacro,sizeof(char),_tmp265);char _tmp26A=*((char*)_check_dyneither_subscript(_tmp269,sizeof(char),0));char _tmp26B=(char) toupper((int)((char*)ifdefmacro.curr)[_tmp265]);if(_get_dyneither_size(_tmp269,sizeof(char))== 1  && (_tmp26A == '\000'  && _tmp26B != '\000'))_throw_arraybounds();*((char*)_tmp269.curr)=_tmp26B;});}}}
# 1529
({struct Cyc_String_pa_PrintArg_struct _tmp26F;_tmp26F.tag=0;_tmp26F.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)ifdefmacro);({struct Cyc_String_pa_PrintArg_struct _tmp26E;_tmp26E.tag=0;_tmp26E.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)ifdefmacro);({void*_tmp26C[2]={& _tmp26E,& _tmp26F};Cyc_fprintf(out_file,({const char*_tmp26D="#ifndef %s\n#define %s\n";_tag_dyneither(_tmp26D,sizeof(char),23);}),_tag_dyneither(_tmp26C,sizeof(void*),2));});});});{
# 1536
struct Cyc_List_List*_tmp270=0;
struct Cyc_List_List*_tmp271=0;
{struct Cyc_List_List*_tmp272=_tmp243;for(0;_tmp272 != 0;_tmp272=_tmp272->tl){
struct Cyc_Absyn_Decl*_tmp273=(struct Cyc_Absyn_Decl*)_tmp272->hd;
int _tmp274=0;
struct _dyneither_ptr*name;
{void*_tmp275=_tmp273->r;void*_tmp276=_tmp275;struct Cyc_Absyn_Typedefdecl*_tmp28A;struct Cyc_Absyn_Enumdecl*_tmp289;struct Cyc_Absyn_Aggrdecl*_tmp288;struct Cyc_Absyn_Fndecl*_tmp287;struct Cyc_Absyn_Vardecl*_tmp286;switch(*((int*)_tmp276)){case 0: _LL2B4: _tmp286=((struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct*)_tmp276)->f1;_LL2B5: {
# 1544
struct _tuple0*_tmp277=_tmp286->name;struct _tuple0*_tmp278=_tmp277;struct _dyneither_ptr*_tmp279;_LL2D3: _tmp279=_tmp278->f2;_LL2D4:;
name=_tmp279;
goto _LL2B3;}case 1: _LL2B6: _tmp287=((struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct*)_tmp276)->f1;_LL2B7:
# 1548
 if(_tmp287->is_inline){name=0;goto _LL2B3;}{
struct _tuple0*_tmp27A=_tmp287->name;struct _tuple0*_tmp27B=_tmp27A;struct _dyneither_ptr*_tmp27C;_LL2D6: _tmp27C=_tmp27B->f2;_LL2D7:;
name=_tmp27C;
goto _LL2B3;};case 5: _LL2B8: _tmp288=((struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct*)_tmp276)->f1;_LL2B9: {
# 1553
struct _tuple0*_tmp27D=_tmp288->name;struct _tuple0*_tmp27E=_tmp27D;struct _dyneither_ptr*_tmp27F;_LL2D9: _tmp27F=_tmp27E->f2;_LL2DA:;
name=_tmp27F;
goto _LL2B3;}case 7: _LL2BA: _tmp289=((struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct*)_tmp276)->f1;_LL2BB: {
# 1557
struct _tuple0*_tmp280=_tmp289->name;struct _tuple0*_tmp281=_tmp280;struct _dyneither_ptr*_tmp282;_LL2DC: _tmp282=_tmp281->f2;_LL2DD:;
name=_tmp282;
goto _LL2B3;}case 8: _LL2BC: _tmp28A=((struct Cyc_Absyn_Typedef_d_Absyn_Raw_decl_struct*)_tmp276)->f1;_LL2BD: {
# 1561
struct _tuple0*_tmp283=_tmp28A->name;struct _tuple0*_tmp284=_tmp283;struct _dyneither_ptr*_tmp285;_LL2DF: _tmp285=_tmp284->f2;_LL2E0:;
name=_tmp285;
goto _LL2B3;}case 4: _LL2BE: _LL2BF:
 goto _LL2C1;case 13: _LL2C0: _LL2C1:
# 1566
 goto _LL2C3;case 14: _LL2C2: _LL2C3:
 goto _LL2C5;case 2: _LL2C4: _LL2C5:
 goto _LL2C7;case 6: _LL2C6: _LL2C7:
 goto _LL2C9;case 3: _LL2C8: _LL2C9:
 goto _LL2CB;case 9: _LL2CA: _LL2CB:
 goto _LL2CD;case 10: _LL2CC: _LL2CD:
 goto _LL2CF;case 11: _LL2CE: _LL2CF:
 goto _LL2D1;default: _LL2D0: _LL2D1:
# 1575
 name=0;
goto _LL2B3;}_LL2B3:;}
# 1578
if(!((unsigned int)name) && !_tmp274)continue;
_tmp270=({struct Cyc_List_List*_tmp28B=_cycalloc(sizeof(*_tmp28B));_tmp28B->hd=_tmp273;_tmp28B->tl=_tmp270;_tmp28B;});
_tmp271=({struct Cyc_List_List*_tmp28C=_cycalloc(sizeof(*_tmp28C));_tmp28C->hd=name;_tmp28C->tl=_tmp271;_tmp28C;});}}
# 1584
{struct _handler_cons _tmp28D;_push_handler(& _tmp28D);{int _tmp28F=0;if(setjmp(_tmp28D.handler))_tmp28F=1;if(!_tmp28F){
{struct _RegionHandle _tmp290=_new_region("tc_rgn");struct _RegionHandle*tc_rgn=& _tmp290;_push_region(tc_rgn);
Cyc_Binding_resolve_all(_tmp270);{
struct Cyc_Tcenv_Tenv*_tmp291=Cyc_Tcenv_tc_init(tc_rgn);
Cyc_Tc_tc(tc_rgn,_tmp291,1,_tmp270);};
# 1586
;_pop_region(tc_rgn);}
# 1585
;_pop_handler();}else{void*_tmp28E=(void*)_exn_thrown;void*_tmp292=_tmp28E;_LL2E2: _LL2E3:
# 1592
(int)_throw((void*)({struct Cyc_NO_SUPPORT_exn_struct*_tmp293=_cycalloc(sizeof(*_tmp293));_tmp293[0]=({struct Cyc_NO_SUPPORT_exn_struct _tmp294;_tmp294.tag=Cyc_NO_SUPPORT;_tmp294.f1=({const char*_tmp295="can't typecheck acquired declarations";_tag_dyneither(_tmp295,sizeof(char),38);});_tmp294;});_tmp293;}));
goto _LL2E1;_LL2E1:;}};}
# 1597
{struct _tuple25 _tmp296=({struct _tuple25 _tmp2AA;_tmp2AA.f1=_tmp270;_tmp2AA.f2=_tmp271;_tmp2AA;});struct _tuple25 _tmp297=_tmp296;struct Cyc_List_List*_tmp2A9;struct Cyc_List_List*_tmp2A8;_LL2E7: _tmp2A9=_tmp297.f1;_tmp2A8=_tmp297.f2;_LL2E8:;for(0;
_tmp2A9 != 0  && _tmp2A8 != 0;(_tmp2A9=_tmp2A9->tl,_tmp2A8=_tmp2A8->tl)){
struct Cyc_Absyn_Decl*_tmp298=(struct Cyc_Absyn_Decl*)_tmp2A9->hd;
struct _dyneither_ptr*_tmp299=(struct _dyneither_ptr*)_tmp2A8->hd;
int _tmp29A=0;
if(!((unsigned int)_tmp299))
_tmp29A=1;
# 1606
Cyc_Absynpp_set_params(& Cyc_Absynpp_cyc_params_r);
if((unsigned int)_tmp299){
ifdefmacro=({struct Cyc_String_pa_PrintArg_struct _tmp29D;_tmp29D.tag=0;_tmp29D.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*_tmp299);({void*_tmp29B[1]={& _tmp29D};Cyc_aprintf(({const char*_tmp29C="_%s_def_";_tag_dyneither(_tmp29C,sizeof(char),9);}),_tag_dyneither(_tmp29B,sizeof(void*),1));});});
({struct Cyc_String_pa_PrintArg_struct _tmp2A0;_tmp2A0.tag=0;_tmp2A0.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)ifdefmacro);({void*_tmp29E[1]={& _tmp2A0};Cyc_fprintf(out_file,({const char*_tmp29F="#ifndef %s\n";_tag_dyneither(_tmp29F,sizeof(char),12);}),_tag_dyneither(_tmp29E,sizeof(void*),1));});});
({struct Cyc_String_pa_PrintArg_struct _tmp2A3;_tmp2A3.tag=0;_tmp2A3.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)ifdefmacro);({void*_tmp2A1[1]={& _tmp2A3};Cyc_fprintf(out_file,({const char*_tmp2A2="#define %s\n";_tag_dyneither(_tmp2A2,sizeof(char),12);}),_tag_dyneither(_tmp2A1,sizeof(void*),1));});});
# 1612
Cyc_Absynpp_decllist2file(({struct Cyc_Absyn_Decl*_tmp2A4[1];_tmp2A4[0]=_tmp298;((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp2A4,sizeof(struct Cyc_Absyn_Decl*),1));}),out_file);
({void*_tmp2A5=0;Cyc_fprintf(out_file,({const char*_tmp2A6="#endif\n";_tag_dyneither(_tmp2A6,sizeof(char),8);}),_tag_dyneither(_tmp2A5,sizeof(void*),0));});}else{
# 1617
Cyc_Absynpp_decllist2file(({struct Cyc_Absyn_Decl*_tmp2A7[1];_tmp2A7[0]=_tmp298;((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp2A7,sizeof(struct Cyc_Absyn_Decl*),1));}),out_file);}}}
# 1622
maybe=Cyc_fopen(_tmp1D9,"r");
if(!((unsigned int)maybe))(int)_throw((void*)({struct Cyc_NO_SUPPORT_exn_struct*_tmp2AB=_cycalloc(sizeof(*_tmp2AB));_tmp2AB[0]=({struct Cyc_NO_SUPPORT_exn_struct _tmp2AC;_tmp2AC.tag=Cyc_NO_SUPPORT;_tmp2AC.f1=(struct _dyneither_ptr)({struct Cyc_String_pa_PrintArg_struct _tmp2AF;_tmp2AF.tag=0;_tmp2AF.f1=(struct _dyneither_ptr)({const char*_tmp2B0=_tmp1D9;_tag_dyneither(_tmp2B0,sizeof(char),_get_zero_arr_size_char((void*)_tmp2B0,1));});({void*_tmp2AD[1]={& _tmp2AF};Cyc_aprintf(({const char*_tmp2AE="can't open macrosfile %s";_tag_dyneither(_tmp2AE,sizeof(char),25);}),_tag_dyneither(_tmp2AD,sizeof(void*),1));});});_tmp2AC;});_tmp2AB;}));
# 1625
in_file=maybe;
_tmp232=Cyc_Lexing_from_file(in_file);{
struct _tuple21*entry2;
while((entry2=((struct _tuple21*(*)(struct Cyc_Lexing_lexbuf*lexbuf))Cyc_suck_line)(_tmp232))!= 0){
struct _tuple21*_tmp2B1=(struct _tuple21*)_check_null(entry2);struct _tuple21*_tmp2B2=_tmp2B1;struct _dyneither_ptr _tmp2BC;struct _dyneither_ptr*_tmp2BB;_LL2EA: _tmp2BC=_tmp2B2->f1;_tmp2BB=_tmp2B2->f2;_LL2EB:;
if(((int(*)(struct Cyc_Set_Set*s,struct _dyneither_ptr*elt))Cyc_Set_member)(_tmp242,_tmp2BB)){
({struct Cyc_String_pa_PrintArg_struct _tmp2B5;_tmp2B5.tag=0;_tmp2B5.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*_tmp2BB);({void*_tmp2B3[1]={& _tmp2B5};Cyc_fprintf(out_file,({const char*_tmp2B4="#ifndef %s\n";_tag_dyneither(_tmp2B4,sizeof(char),12);}),_tag_dyneither(_tmp2B3,sizeof(void*),1));});});
({struct Cyc_String_pa_PrintArg_struct _tmp2B8;_tmp2B8.tag=0;_tmp2B8.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp2BC);({void*_tmp2B6[1]={& _tmp2B8};Cyc_fprintf(out_file,({const char*_tmp2B7="%s\n";_tag_dyneither(_tmp2B7,sizeof(char),4);}),_tag_dyneither(_tmp2B6,sizeof(void*),1));});});
({void*_tmp2B9=0;Cyc_fprintf(out_file,({const char*_tmp2BA="#endif\n";_tag_dyneither(_tmp2BA,sizeof(char),8);}),_tag_dyneither(_tmp2B9,sizeof(void*),0));});}}
# 1636
Cyc_fclose(in_file);
if(Cyc_mode != Cyc_FINISH);
# 1639
if(hstubs != 0){
struct Cyc_List_List*_tmp2BD=hstubs;for(0;_tmp2BD != 0;_tmp2BD=_tmp2BD->tl){
struct _tuple22*_tmp2BE=(struct _tuple22*)_tmp2BD->hd;struct _tuple22*_tmp2BF=_tmp2BE;struct _dyneither_ptr _tmp2C5;struct _dyneither_ptr _tmp2C4;_LL2ED: _tmp2C5=_tmp2BF->f1;_tmp2C4=_tmp2BF->f2;_LL2EE:;
if((char*)_tmp2C4.curr != (char*)(_tag_dyneither(0,0,0)).curr  && (
(char*)_tmp2C5.curr == (char*)(_tag_dyneither(0,0,0)).curr  || ((int(*)(struct Cyc_Set_Set*s,struct _dyneither_ptr*elt))Cyc_Set_member)(defined_symbols,({struct _dyneither_ptr*_tmp2C0=_cycalloc(sizeof(*_tmp2C0));_tmp2C0[0]=_tmp2C5;_tmp2C0;}))))
# 1645
Cyc_fputs((const char*)_check_null(_untag_dyneither_ptr(_tmp2C4,sizeof(char),1)),out_file);else{
# 1647
({struct Cyc_String_pa_PrintArg_struct _tmp2C3;_tmp2C3.tag=0;_tmp2C3.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp2C5);({void*_tmp2C1[1]={& _tmp2C3};Cyc_log(({const char*_tmp2C2="%s is not supported on this platform\n";_tag_dyneither(_tmp2C2,sizeof(char),38);}),_tag_dyneither(_tmp2C1,sizeof(void*),1));});});}}}
# 1650
({void*_tmp2C6=0;Cyc_fprintf(out_file,({const char*_tmp2C7="#endif\n";_tag_dyneither(_tmp2C7,sizeof(char),8);}),_tag_dyneither(_tmp2C6,sizeof(void*),0));});
if(Cyc_do_setjmp){int _tmp2C8=0;_npop_handler(0);return _tmp2C8;}else{
Cyc_fclose(out_file);}
# 1655
if(cstubs != 0){
out_file=(struct Cyc___cycFILE*)_check_null(Cyc_cstubs_file);{
struct Cyc_List_List*_tmp2C9=cstubs;for(0;_tmp2C9 != 0;_tmp2C9=_tmp2C9->tl){
struct _tuple22*_tmp2CA=(struct _tuple22*)_tmp2C9->hd;struct _tuple22*_tmp2CB=_tmp2CA;struct _dyneither_ptr _tmp2CE;struct _dyneither_ptr _tmp2CD;_LL2F0: _tmp2CE=_tmp2CB->f1;_tmp2CD=_tmp2CB->f2;_LL2F1:;
if((char*)_tmp2CD.curr != (char*)(_tag_dyneither(0,0,0)).curr  && (
(char*)_tmp2CE.curr == (char*)(_tag_dyneither(0,0,0)).curr  || ((int(*)(struct Cyc_Set_Set*s,struct _dyneither_ptr*elt))Cyc_Set_member)(defined_symbols,({struct _dyneither_ptr*_tmp2CC=_cycalloc(sizeof(*_tmp2CC));_tmp2CC[0]=_tmp2CE;_tmp2CC;}))))
Cyc_fputs((const char*)_check_null(_untag_dyneither_ptr(_tmp2CD,sizeof(char),1)),out_file);}};}
# 1666
out_file=(struct Cyc___cycFILE*)_check_null(Cyc_cycstubs_file);
# 1668
({struct Cyc_String_pa_PrintArg_struct _tmp2D1;_tmp2D1.tag=0;_tmp2D1.f1=(struct _dyneither_ptr)({const char*_tmp2D2=filename;_tag_dyneither(_tmp2D2,sizeof(char),_get_zero_arr_size_char((void*)_tmp2D2,1));});({void*_tmp2CF[1]={& _tmp2D1};Cyc_fprintf(out_file,({const char*_tmp2D0="#include <%s>\n\n";_tag_dyneither(_tmp2D0,sizeof(char),16);}),_tag_dyneither(_tmp2CF,sizeof(void*),1));});});
if(cycstubs != 0){
out_file=(struct Cyc___cycFILE*)_check_null(Cyc_cycstubs_file);
{struct Cyc_List_List*_tmp2D3=cycstubs;for(0;_tmp2D3 != 0;_tmp2D3=_tmp2D3->tl){
struct _tuple22*_tmp2D4=(struct _tuple22*)_tmp2D3->hd;struct _tuple22*_tmp2D5=_tmp2D4;struct _dyneither_ptr _tmp2D8;struct _dyneither_ptr _tmp2D7;_LL2F3: _tmp2D8=_tmp2D5->f1;_tmp2D7=_tmp2D5->f2;_LL2F4:;
if((char*)_tmp2D7.curr != (char*)(_tag_dyneither(0,0,0)).curr  && (
(char*)_tmp2D8.curr == (char*)(_tag_dyneither(0,0,0)).curr  || ((int(*)(struct Cyc_Set_Set*s,struct _dyneither_ptr*elt))Cyc_Set_member)(defined_symbols,({struct _dyneither_ptr*_tmp2D6=_cycalloc(sizeof(*_tmp2D6));_tmp2D6[0]=_tmp2D8;_tmp2D6;}))))
Cyc_fputs((const char*)_check_null(_untag_dyneither_ptr(_tmp2D7,sizeof(char),1)),out_file);}}
# 1677
({void*_tmp2D9=0;Cyc_fprintf(out_file,({const char*_tmp2DA="\n";_tag_dyneither(_tmp2DA,sizeof(char),2);}),_tag_dyneither(_tmp2D9,sizeof(void*),0));});}{
# 1680
int _tmp2DB=0;_npop_handler(0);return _tmp2DB;};};};};};};};};
# 1347
;_pop_handler();}else{void*_tmp1DD=(void*)_exn_thrown;void*_tmp2E0=_tmp1DD;struct _dyneither_ptr _tmp2F6;struct _dyneither_ptr _tmp2F5;struct _dyneither_ptr _tmp2F4;struct _dyneither_ptr _tmp2F3;if(((struct Cyc_Core_Impossible_exn_struct*)_tmp2E0)->tag == Cyc_Core_Impossible){_LL2F6: _tmp2F3=((struct Cyc_Core_Impossible_exn_struct*)_tmp2E0)->f1;_LL2F7:
# 1684
({struct Cyc_String_pa_PrintArg_struct _tmp2E3;_tmp2E3.tag=0;_tmp2E3.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp2F3);({void*_tmp2E1[1]={& _tmp2E3};Cyc_fprintf(Cyc_stderr,({const char*_tmp2E2="Got Core::Impossible(%s)\n";_tag_dyneither(_tmp2E2,sizeof(char),26);}),_tag_dyneither(_tmp2E1,sizeof(void*),1));});});goto _LL2F5;}else{if(((struct Cyc_Dict_Absent_exn_struct*)_tmp2E0)->tag == Cyc_Dict_Absent){_LL2F8: _LL2F9:
# 1686
({void*_tmp2E4=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp2E5="Got Dict::Absent\n";_tag_dyneither(_tmp2E5,sizeof(char),18);}),_tag_dyneither(_tmp2E4,sizeof(void*),0));});goto _LL2F5;}else{if(((struct Cyc_Core_Failure_exn_struct*)_tmp2E0)->tag == Cyc_Core_Failure){_LL2FA: _tmp2F4=((struct Cyc_Core_Failure_exn_struct*)_tmp2E0)->f1;_LL2FB:
# 1688
({struct Cyc_String_pa_PrintArg_struct _tmp2E8;_tmp2E8.tag=0;_tmp2E8.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp2F4);({void*_tmp2E6[1]={& _tmp2E8};Cyc_fprintf(Cyc_stderr,({const char*_tmp2E7="Got Core::Failure(%s)\n";_tag_dyneither(_tmp2E7,sizeof(char),23);}),_tag_dyneither(_tmp2E6,sizeof(void*),1));});});goto _LL2F5;}else{if(((struct Cyc_Core_Invalid_argument_exn_struct*)_tmp2E0)->tag == Cyc_Core_Invalid_argument){_LL2FC: _tmp2F5=((struct Cyc_Core_Invalid_argument_exn_struct*)_tmp2E0)->f1;_LL2FD:
# 1690
({struct Cyc_String_pa_PrintArg_struct _tmp2EB;_tmp2EB.tag=0;_tmp2EB.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp2F5);({void*_tmp2E9[1]={& _tmp2EB};Cyc_fprintf(Cyc_stderr,({const char*_tmp2EA="Got Invalid_argument(%s)\n";_tag_dyneither(_tmp2EA,sizeof(char),26);}),_tag_dyneither(_tmp2E9,sizeof(void*),1));});});goto _LL2F5;}else{if(((struct Cyc_Core_Not_found_exn_struct*)_tmp2E0)->tag == Cyc_Core_Not_found){_LL2FE: _LL2FF:
# 1692
({void*_tmp2EC=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp2ED="Got Not_found\n";_tag_dyneither(_tmp2ED,sizeof(char),15);}),_tag_dyneither(_tmp2EC,sizeof(void*),0));});goto _LL2F5;}else{if(((struct Cyc_NO_SUPPORT_exn_struct*)_tmp2E0)->tag == Cyc_NO_SUPPORT){_LL300: _tmp2F6=((struct Cyc_NO_SUPPORT_exn_struct*)_tmp2E0)->f1;_LL301:
# 1694
({struct Cyc_String_pa_PrintArg_struct _tmp2F0;_tmp2F0.tag=0;_tmp2F0.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp2F6);({void*_tmp2EE[1]={& _tmp2F0};Cyc_fprintf(Cyc_stderr,({const char*_tmp2EF="No support because %s\n";_tag_dyneither(_tmp2EF,sizeof(char),23);}),_tag_dyneither(_tmp2EE,sizeof(void*),1));});});goto _LL2F5;}else{_LL302: _LL303:
# 1696
({void*_tmp2F1=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp2F2="Got unknown exception\n";_tag_dyneither(_tmp2F2,sizeof(char),23);}),_tag_dyneither(_tmp2F1,sizeof(void*),0));});goto _LL2F5;}}}}}}_LL2F5:;}};}
# 1701
maybe=Cyc_fopen(filename,"w");
if(!((unsigned int)maybe)){
({struct Cyc_String_pa_PrintArg_struct _tmp2F9;_tmp2F9.tag=0;_tmp2F9.f1=(struct _dyneither_ptr)({const char*_tmp2FA=filename;_tag_dyneither(_tmp2FA,sizeof(char),_get_zero_arr_size_char((void*)_tmp2FA,1));});({void*_tmp2F7[1]={& _tmp2F9};Cyc_fprintf(Cyc_stderr,({const char*_tmp2F8="Error: could not create file %s\n";_tag_dyneither(_tmp2F8,sizeof(char),33);}),_tag_dyneither(_tmp2F7,sizeof(void*),1));});});
return 1;}
# 1706
out_file=maybe;
({struct Cyc_String_pa_PrintArg_struct _tmp2FD;_tmp2FD.tag=0;_tmp2FD.f1=(struct _dyneither_ptr)({const char*_tmp2FE=filename;_tag_dyneither(_tmp2FE,sizeof(char),_get_zero_arr_size_char((void*)_tmp2FE,1));});({void*_tmp2FB[1]={& _tmp2FD};Cyc_fprintf(out_file,({const char*_tmp2FC="#error -- %s is not supported on this platform\n";_tag_dyneither(_tmp2FC,sizeof(char),48);}),_tag_dyneither(_tmp2FB,sizeof(void*),1));});});
# 1710
Cyc_fclose(out_file);
({struct Cyc_String_pa_PrintArg_struct _tmp301;_tmp301.tag=0;_tmp301.f1=(struct _dyneither_ptr)({const char*_tmp302=filename;_tag_dyneither(_tmp302,sizeof(char),_get_zero_arr_size_char((void*)_tmp302,1));});({void*_tmp2FF[1]={& _tmp301};Cyc_fprintf(Cyc_stderr,({const char*_tmp300="Warning: %s will not be supported on this platform\n";_tag_dyneither(_tmp300,sizeof(char),52);}),_tag_dyneither(_tmp2FF,sizeof(void*),1));});});
# 1713
({void*_tmp303=0;Cyc_log(({const char*_tmp304="Not supported on this platform\n";_tag_dyneither(_tmp304,sizeof(char),32);}),_tag_dyneither(_tmp303,sizeof(void*),0));});
# 1720
return 0;};}
# 1724
int Cyc_process_specfile(const char*file,const char*dir){
if(Cyc_verbose)
({struct Cyc_String_pa_PrintArg_struct _tmp31C;_tmp31C.tag=0;_tmp31C.f1=(struct _dyneither_ptr)({const char*_tmp31D=file;_tag_dyneither(_tmp31D,sizeof(char),_get_zero_arr_size_char((void*)_tmp31D,1));});({void*_tmp31A[1]={& _tmp31C};Cyc_fprintf(Cyc_stderr,({const char*_tmp31B="Processing %s\n";_tag_dyneither(_tmp31B,sizeof(char),15);}),_tag_dyneither(_tmp31A,sizeof(void*),1));});});{
struct Cyc___cycFILE*_tmp31E=Cyc_fopen(file,"r");
if(!((unsigned int)_tmp31E)){
({struct Cyc_String_pa_PrintArg_struct _tmp321;_tmp321.tag=0;_tmp321.f1=(struct _dyneither_ptr)({const char*_tmp322=file;_tag_dyneither(_tmp322,sizeof(char),_get_zero_arr_size_char((void*)_tmp322,1));});({void*_tmp31F[1]={& _tmp321};Cyc_fprintf(Cyc_stderr,({const char*_tmp320="Error: could not open %s\n";_tag_dyneither(_tmp320,sizeof(char),26);}),_tag_dyneither(_tmp31F,sizeof(void*),1));});});
return 1;}{
# 1732
struct Cyc___cycFILE*_tmp323=_tmp31E;
# 1736
struct _dyneither_ptr buf=({char*_tmp340=({unsigned int _tmp33D=(unsigned int)1024;char*_tmp33E=(char*)_cycalloc_atomic(_check_times(sizeof(char),_tmp33D + 1));{unsigned int _tmp33F=_tmp33D;unsigned int i;for(i=0;i < _tmp33F;i ++){_tmp33E[i]=(char)'\000';}_tmp33E[_tmp33F]=(char)0;}_tmp33E;});_tag_dyneither(_tmp340,sizeof(char),_get_zero_arr_size_char((void*)_tmp340,(unsigned int)1024 + 1));});
struct _dyneither_ptr _tmp324=Cyc_getcwd(buf,_get_dyneither_size(buf,sizeof(char)));
if(Cyc_mode != Cyc_GATHERSCRIPT){
if( chdir(dir)){
({struct Cyc_String_pa_PrintArg_struct _tmp327;_tmp327.tag=0;_tmp327.f1=(struct _dyneither_ptr)({const char*_tmp328=dir;_tag_dyneither(_tmp328,sizeof(char),_get_zero_arr_size_char((void*)_tmp328,1));});({void*_tmp325[1]={& _tmp327};Cyc_fprintf(Cyc_stderr,({const char*_tmp326="Error: can't change directory to %s\n";_tag_dyneither(_tmp326,sizeof(char),37);}),_tag_dyneither(_tmp325,sizeof(void*),1));});});
return 1;}}
# 1744
if(Cyc_mode == Cyc_GATHER){
# 1746
struct _dyneither_ptr _tmp329= Cstring_to_string( Ccomp);
struct _dyneither_ptr _tmp32A=({struct Cyc_String_pa_PrintArg_struct _tmp330;_tmp330.tag=0;_tmp330.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp329);({void*_tmp32E[1]={& _tmp330};Cyc_aprintf(({const char*_tmp32F="echo | %s -E -dM - -o INITMACROS.h\n";_tag_dyneither(_tmp32F,sizeof(char),36);}),_tag_dyneither(_tmp32E,sizeof(void*),1));});});
if(Cyc_verbose)
({struct Cyc_String_pa_PrintArg_struct _tmp32D;_tmp32D.tag=0;_tmp32D.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp32A);({void*_tmp32B[1]={& _tmp32D};Cyc_fprintf(Cyc_stderr,({const char*_tmp32C="%s\n";_tag_dyneither(_tmp32C,sizeof(char),4);}),_tag_dyneither(_tmp32B,sizeof(void*),1));});});
 system((const char*)_check_null(_untag_dyneither_ptr(_tmp32A,sizeof(char),1)));}{
# 1753
struct Cyc_Lexing_lexbuf*_tmp331=Cyc_Lexing_from_file(_tmp323);
struct _tuple23*entry;
while((entry=((struct _tuple23*(*)(struct Cyc_Lexing_lexbuf*lexbuf))Cyc_spec)(_tmp331))!= 0){
struct _tuple23*_tmp332=(struct _tuple23*)_check_null(entry);struct _tuple23*_tmp333=_tmp332;struct _dyneither_ptr _tmp339;struct Cyc_List_List*_tmp338;struct Cyc_List_List*_tmp337;struct Cyc_List_List*_tmp336;struct Cyc_List_List*_tmp335;struct Cyc_List_List*_tmp334;_LL307: _tmp339=_tmp333->f1;_tmp338=_tmp333->f2;_tmp337=_tmp333->f3;_tmp336=_tmp333->f4;_tmp335=_tmp333->f5;_tmp334=_tmp333->f6;_LL308:;
# 1758
if(Cyc_process_file((const char*)_check_null(_untag_dyneither_ptr(_tmp339,sizeof(char),1)),_tmp338,_tmp337,_tmp336,_tmp335,_tmp334))
# 1760
return 1;}
# 1762
Cyc_fclose(_tmp323);
# 1764
if(Cyc_mode != Cyc_GATHERSCRIPT){
if( chdir((const char*)((char*)_check_null(_untag_dyneither_ptr(_tmp324,sizeof(char),1))))){
({struct Cyc_String_pa_PrintArg_struct _tmp33C;_tmp33C.tag=0;_tmp33C.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp324);({void*_tmp33A[1]={& _tmp33C};Cyc_fprintf(Cyc_stderr,({const char*_tmp33B="Error: could not change directory to %s\n";_tag_dyneither(_tmp33B,sizeof(char),41);}),_tag_dyneither(_tmp33A,sizeof(void*),1));});});
return 1;}}
# 1770
return 0;};};};}
# 1774
int Cyc_process_setjmp(const char*dir){
# 1777
struct _dyneither_ptr buf=({char*_tmp352=({unsigned int _tmp34F=(unsigned int)1024;char*_tmp350=(char*)_cycalloc_atomic(_check_times(sizeof(char),_tmp34F + 1));{unsigned int _tmp351=_tmp34F;unsigned int i;for(i=0;i < _tmp351;i ++){_tmp350[i]=(char)'\000';}_tmp350[_tmp351]=(char)0;}_tmp350;});_tag_dyneither(_tmp352,sizeof(char),_get_zero_arr_size_char((void*)_tmp352,(unsigned int)1024 + 1));});
struct _dyneither_ptr _tmp341=Cyc_getcwd(buf,_get_dyneither_size(buf,sizeof(char)));
if( chdir(dir)){
({struct Cyc_String_pa_PrintArg_struct _tmp344;_tmp344.tag=0;_tmp344.f1=(struct _dyneither_ptr)({const char*_tmp345=dir;_tag_dyneither(_tmp345,sizeof(char),_get_zero_arr_size_char((void*)_tmp345,1));});({void*_tmp342[1]={& _tmp344};Cyc_fprintf(Cyc_stderr,({const char*_tmp343="Error: can't change directory to %s\n";_tag_dyneither(_tmp343,sizeof(char),37);}),_tag_dyneither(_tmp342,sizeof(void*),1));});});
return 1;}
# 1783
if(Cyc_process_file("setjmp.h",({struct _dyneither_ptr*_tmp346[1];_tmp346[0]=_init_dyneither_ptr(_cycalloc(sizeof(struct _dyneither_ptr)),"jmp_buf",sizeof(char),8);((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp346,sizeof(struct _dyneither_ptr*),1));}),0,({struct _tuple22*_tmp348[1];_tmp348[0]=({struct _tuple22*_tmp349=_cycalloc(sizeof(*_tmp349));_tmp349->f1=({const char*_tmp34B="setjmp";_tag_dyneither(_tmp34B,sizeof(char),7);});_tmp349->f2=({const char*_tmp34A="extern int setjmp(jmp_buf);\n";_tag_dyneither(_tmp34A,sizeof(char),29);});_tmp349;});((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp348,sizeof(struct _tuple22*),1));}),0,0))
# 1786
return 1;
if( chdir((const char*)((char*)_check_null(_untag_dyneither_ptr(_tmp341,sizeof(char),1))))){
({struct Cyc_String_pa_PrintArg_struct _tmp34E;_tmp34E.tag=0;_tmp34E.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp341);({void*_tmp34C[1]={& _tmp34E};Cyc_fprintf(Cyc_stderr,({const char*_tmp34D="Error: could not change directory to %s\n";_tag_dyneither(_tmp34D,sizeof(char),41);}),_tag_dyneither(_tmp34C,sizeof(void*),1));});});
return 1;}
# 1791
return 0;}static char _tmp353[13]="BUILDLIB.OUT";
# 1795
static struct _dyneither_ptr Cyc_output_dir={_tmp353,_tmp353,_tmp353 + 13};
static void Cyc_set_output_dir(struct _dyneither_ptr s){
Cyc_output_dir=s;}
# 1799
static struct Cyc_List_List*Cyc_spec_files=0;
static void Cyc_add_spec_file(struct _dyneither_ptr s){
Cyc_spec_files=({struct Cyc_List_List*_tmp354=_cycalloc(sizeof(*_tmp354));_tmp354->hd=(const char*)_check_null(_untag_dyneither_ptr(s,sizeof(char),1));_tmp354->tl=Cyc_spec_files;_tmp354;});}
# 1803
static int Cyc_no_other(struct _dyneither_ptr s){return 0;}
static void Cyc_set_GATHER(){
Cyc_mode=Cyc_GATHER;}
# 1807
static void Cyc_set_GATHERSCRIPT(){
Cyc_mode=Cyc_GATHERSCRIPT;}
# 1810
static void Cyc_set_FINISH(){
Cyc_mode=Cyc_FINISH;}
# 1813
static void Cyc_add_cpparg(struct _dyneither_ptr s){
Cyc_cppargs=({struct Cyc_List_List*_tmp355=_cycalloc(sizeof(*_tmp355));_tmp355->hd=({struct _dyneither_ptr*_tmp356=_cycalloc(sizeof(*_tmp356));_tmp356[0]=s;_tmp356;});_tmp355->tl=Cyc_cppargs;_tmp355;});}
# 1816
static int Cyc_badparse=0;
static void Cyc_unsupported_option(struct _dyneither_ptr s){
({struct Cyc_String_pa_PrintArg_struct _tmp359;_tmp359.tag=0;_tmp359.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)s);({void*_tmp357[1]={& _tmp359};Cyc_fprintf(Cyc_stderr,({const char*_tmp358="Unsupported option %s\n";_tag_dyneither(_tmp358,sizeof(char),23);}),_tag_dyneither(_tmp357,sizeof(void*),1));});});
Cyc_badparse=1;}
# 1821
static void Cyc_set_machine(struct _dyneither_ptr s){
Cyc_add_cpparg(({const char*_tmp35A="-b";_tag_dyneither(_tmp35A,sizeof(char),3);}));
Cyc_add_cpparg(s);}
# 1829
void GC_blacklist_warn_clear();struct _tuple26{struct _dyneither_ptr f1;int f2;struct _dyneither_ptr f3;void*f4;struct _dyneither_ptr f5;};
int Cyc_main(int argc,struct _dyneither_ptr argv){
 GC_blacklist_warn_clear();{
# 1833
struct Cyc_List_List*options=({struct _tuple26*_tmp380[8];_tmp380[7]=({struct _tuple26*_tmp3AB=_cycalloc(sizeof(*_tmp3AB));_tmp3AB->f1=({const char*_tmp3B0="-";_tag_dyneither(_tmp3B0,sizeof(char),2);});_tmp3AB->f2=1;_tmp3AB->f3=({const char*_tmp3AF="";_tag_dyneither(_tmp3AF,sizeof(char),1);});_tmp3AB->f4=(void*)({struct Cyc_Arg_Flag_spec_Arg_Spec_struct*_tmp3AD=_cycalloc(sizeof(*_tmp3AD));_tmp3AD[0]=({struct Cyc_Arg_Flag_spec_Arg_Spec_struct _tmp3AE;_tmp3AE.tag=1;_tmp3AE.f1=Cyc_add_cpparg;_tmp3AE;});_tmp3AD;});_tmp3AB->f5=({const char*_tmp3AC="";_tag_dyneither(_tmp3AC,sizeof(char),1);});_tmp3AB;});_tmp380[6]=({struct _tuple26*_tmp3A5=_cycalloc(sizeof(*_tmp3A5));_tmp3A5->f1=({const char*_tmp3AA="-v";_tag_dyneither(_tmp3AA,sizeof(char),3);});_tmp3A5->f2=0;_tmp3A5->f3=({const char*_tmp3A9="";_tag_dyneither(_tmp3A9,sizeof(char),1);});_tmp3A5->f4=(void*)({struct Cyc_Arg_Set_spec_Arg_Spec_struct*_tmp3A7=_cycalloc(sizeof(*_tmp3A7));_tmp3A7[0]=({struct Cyc_Arg_Set_spec_Arg_Spec_struct _tmp3A8;_tmp3A8.tag=3;_tmp3A8.f1=& Cyc_verbose;_tmp3A8;});_tmp3A7;});_tmp3A5->f5=({const char*_tmp3A6="Verbose operation";_tag_dyneither(_tmp3A6,sizeof(char),18);});_tmp3A5;});_tmp380[5]=({struct _tuple26*_tmp39F=_cycalloc(sizeof(*_tmp39F));_tmp39F->f1=({const char*_tmp3A4="-b";_tag_dyneither(_tmp3A4,sizeof(char),3);});_tmp39F->f2=0;_tmp39F->f3=({const char*_tmp3A3=" <machine>";_tag_dyneither(_tmp3A3,sizeof(char),11);});_tmp39F->f4=(void*)({struct Cyc_Arg_String_spec_Arg_Spec_struct*_tmp3A1=_cycalloc(sizeof(*_tmp3A1));_tmp3A1[0]=({struct Cyc_Arg_String_spec_Arg_Spec_struct _tmp3A2;_tmp3A2.tag=5;_tmp3A2.f1=Cyc_set_machine;_tmp3A2;});_tmp3A1;});_tmp39F->f5=({const char*_tmp3A0="Set the target machine for compilation to <machine>";_tag_dyneither(_tmp3A0,sizeof(char),52);});_tmp39F;});_tmp380[4]=({struct _tuple26*_tmp399=_cycalloc(sizeof(*_tmp399));_tmp399->f1=({const char*_tmp39E="-setjmp";_tag_dyneither(_tmp39E,sizeof(char),8);});_tmp399->f2=0;_tmp399->f3=({const char*_tmp39D="";_tag_dyneither(_tmp39D,sizeof(char),1);});_tmp399->f4=(void*)({struct Cyc_Arg_Set_spec_Arg_Spec_struct*_tmp39B=_cycalloc(sizeof(*_tmp39B));_tmp39B[0]=({struct Cyc_Arg_Set_spec_Arg_Spec_struct _tmp39C;_tmp39C.tag=3;_tmp39C.f1=& Cyc_do_setjmp;_tmp39C;});_tmp39B;});_tmp399->f5=({const char*_tmp39A="Produce the jmp_buf and setjmp declarations on the standard output, for use by the Cyclone compiler special file cyc_setjmp.h.  Cannot be used with -gather, -gatherscript, or specfiles.";_tag_dyneither(_tmp39A,sizeof(char),186);});_tmp399;});_tmp380[3]=({struct _tuple26*_tmp393=_cycalloc(sizeof(*_tmp393));_tmp393->f1=({const char*_tmp398="-finish";_tag_dyneither(_tmp398,sizeof(char),8);});_tmp393->f2=0;_tmp393->f3=({const char*_tmp397="";_tag_dyneither(_tmp397,sizeof(char),1);});_tmp393->f4=(void*)({struct Cyc_Arg_Unit_spec_Arg_Spec_struct*_tmp395=_cycalloc(sizeof(*_tmp395));_tmp395[0]=({struct Cyc_Arg_Unit_spec_Arg_Spec_struct _tmp396;_tmp396.tag=0;_tmp396.f1=Cyc_set_FINISH;_tmp396;});_tmp395;});_tmp393->f5=({const char*_tmp394="Produce Cyclone headers from pre-gathered C library info";_tag_dyneither(_tmp394,sizeof(char),57);});_tmp393;});_tmp380[2]=({struct _tuple26*_tmp38D=_cycalloc(sizeof(*_tmp38D));_tmp38D->f1=({const char*_tmp392="-gatherscript";_tag_dyneither(_tmp392,sizeof(char),14);});_tmp38D->f2=0;_tmp38D->f3=({const char*_tmp391="";_tag_dyneither(_tmp391,sizeof(char),1);});_tmp38D->f4=(void*)({struct Cyc_Arg_Unit_spec_Arg_Spec_struct*_tmp38F=_cycalloc(sizeof(*_tmp38F));_tmp38F[0]=({struct Cyc_Arg_Unit_spec_Arg_Spec_struct _tmp390;_tmp390.tag=0;_tmp390.f1=Cyc_set_GATHERSCRIPT;_tmp390;});_tmp38F;});_tmp38D->f5=({const char*_tmp38E="Produce a script to gather C library info";_tag_dyneither(_tmp38E,sizeof(char),42);});_tmp38D;});_tmp380[1]=({struct _tuple26*_tmp387=_cycalloc(sizeof(*_tmp387));_tmp387->f1=({const char*_tmp38C="-gather";_tag_dyneither(_tmp38C,sizeof(char),8);});_tmp387->f2=0;_tmp387->f3=({const char*_tmp38B="";_tag_dyneither(_tmp38B,sizeof(char),1);});_tmp387->f4=(void*)({struct Cyc_Arg_Unit_spec_Arg_Spec_struct*_tmp389=_cycalloc(sizeof(*_tmp389));_tmp389[0]=({struct Cyc_Arg_Unit_spec_Arg_Spec_struct _tmp38A;_tmp38A.tag=0;_tmp38A.f1=Cyc_set_GATHER;_tmp38A;});_tmp389;});_tmp387->f5=({const char*_tmp388="Gather C library info but don't produce Cyclone headers";_tag_dyneither(_tmp388,sizeof(char),56);});_tmp387;});_tmp380[0]=({struct _tuple26*_tmp381=_cycalloc(sizeof(*_tmp381));_tmp381->f1=({const char*_tmp386="-d";_tag_dyneither(_tmp386,sizeof(char),3);});_tmp381->f2=0;_tmp381->f3=({const char*_tmp385=" <file>";_tag_dyneither(_tmp385,sizeof(char),8);});_tmp381->f4=(void*)({struct Cyc_Arg_String_spec_Arg_Spec_struct*_tmp383=_cycalloc(sizeof(*_tmp383));_tmp383[0]=({struct Cyc_Arg_String_spec_Arg_Spec_struct _tmp384;_tmp384.tag=5;_tmp384.f1=Cyc_set_output_dir;_tmp384;});_tmp383;});_tmp381->f5=({const char*_tmp382="Set the output directory to <file>";_tag_dyneither(_tmp382,sizeof(char),35);});_tmp381;});((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp380,sizeof(struct _tuple26*),8));});
# 1862
Cyc_Arg_parse(options,Cyc_add_spec_file,Cyc_no_other,({const char*_tmp35B="Options:";_tag_dyneither(_tmp35B,sizeof(char),9);}),argv);
if((((Cyc_badparse  || 
!Cyc_do_setjmp  && Cyc_spec_files == 0) || 
Cyc_do_setjmp  && Cyc_spec_files != 0) || 
Cyc_do_setjmp  && Cyc_mode == Cyc_GATHER) || 
Cyc_do_setjmp  && Cyc_mode == Cyc_GATHERSCRIPT){
Cyc_Arg_usage(options,({const char*_tmp35C="Usage: buildlib [options] specfile1 specfile2 ...\nOptions:";_tag_dyneither(_tmp35C,sizeof(char),59);}));
# 1871
return 1;}
# 1874
if(Cyc_mode == Cyc_GATHERSCRIPT){
if(Cyc_verbose)
({void*_tmp35D=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp35E="Creating BUILDLIB.sh\n";_tag_dyneither(_tmp35E,sizeof(char),22);}),_tag_dyneither(_tmp35D,sizeof(void*),0));});
Cyc_script_file=Cyc_fopen("BUILDLIB.sh","w");
if(!((unsigned int)Cyc_script_file)){
({void*_tmp35F=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp360="Could not create file BUILDLIB.sh\n";_tag_dyneither(_tmp360,sizeof(char),35);}),_tag_dyneither(_tmp35F,sizeof(void*),0));});
 exit(1);}
# 1882
({void*_tmp361=0;Cyc_prscript(({const char*_tmp362="#!/bin/sh\n";_tag_dyneither(_tmp362,sizeof(char),11);}),_tag_dyneither(_tmp361,sizeof(void*),0));});
({void*_tmp363=0;Cyc_prscript(({const char*_tmp364="GCC=\"gcc\"\n";_tag_dyneither(_tmp364,sizeof(char),11);}),_tag_dyneither(_tmp363,sizeof(void*),0));});}
# 1887
if(Cyc_force_directory_prefixes(Cyc_output_dir) || Cyc_force_directory(Cyc_output_dir)){
({struct Cyc_String_pa_PrintArg_struct _tmp367;_tmp367.tag=0;_tmp367.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_output_dir);({void*_tmp365[1]={& _tmp367};Cyc_fprintf(Cyc_stderr,({const char*_tmp366="Error: could not create directory %s\n";_tag_dyneither(_tmp366,sizeof(char),38);}),_tag_dyneither(_tmp365,sizeof(void*),1));});});
return 1;}
# 1891
if(Cyc_verbose)
({struct Cyc_String_pa_PrintArg_struct _tmp36A;_tmp36A.tag=0;_tmp36A.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_output_dir);({void*_tmp368[1]={& _tmp36A};Cyc_fprintf(Cyc_stderr,({const char*_tmp369="Output directory is %s\n";_tag_dyneither(_tmp369,sizeof(char),24);}),_tag_dyneither(_tmp368,sizeof(void*),1));});});
# 1894
if(Cyc_mode == Cyc_GATHERSCRIPT){
({struct Cyc_String_pa_PrintArg_struct _tmp36D;_tmp36D.tag=0;_tmp36D.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_output_dir);({void*_tmp36B[1]={& _tmp36D};Cyc_prscript(({const char*_tmp36C="cd %s\n";_tag_dyneither(_tmp36C,sizeof(char),7);}),_tag_dyneither(_tmp36B,sizeof(void*),1));});});
({void*_tmp36E=0;Cyc_prscript(({const char*_tmp36F="echo | $GCC -E -dM - -o INITMACROS.h\n";_tag_dyneither(_tmp36F,sizeof(char),38);}),_tag_dyneither(_tmp36E,sizeof(void*),0));});}
# 1899
if(!Cyc_gathering()){
# 1902
Cyc_log_file=Cyc_fopen((const char*)_check_null(_untag_dyneither_ptr(Cyc_Filename_concat(Cyc_output_dir,({const char*_tmp370="BUILDLIB.LOG";_tag_dyneither(_tmp370,sizeof(char),13);})),sizeof(char),1)),"w");
if(!((unsigned int)Cyc_log_file)){
({struct Cyc_String_pa_PrintArg_struct _tmp373;_tmp373.tag=0;_tmp373.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_output_dir);({void*_tmp371[1]={& _tmp373};Cyc_fprintf(Cyc_stderr,({const char*_tmp372="Error: could not create log file in directory %s\n";_tag_dyneither(_tmp372,sizeof(char),50);}),_tag_dyneither(_tmp371,sizeof(void*),1));});});
return 1;}
# 1908
if(!Cyc_do_setjmp){
# 1910
Cyc_cstubs_file=Cyc_fopen((const char*)_check_null(_untag_dyneither_ptr(Cyc_Filename_concat(Cyc_output_dir,({const char*_tmp374="cstubs.c";_tag_dyneither(_tmp374,sizeof(char),9);})),sizeof(char),1)),"w");
if(!((unsigned int)Cyc_cstubs_file)){
({struct Cyc_String_pa_PrintArg_struct _tmp377;_tmp377.tag=0;_tmp377.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_output_dir);({void*_tmp375[1]={& _tmp377};Cyc_fprintf(Cyc_stderr,({const char*_tmp376="Error: could not create cstubs.c in directory %s\n";_tag_dyneither(_tmp376,sizeof(char),50);}),_tag_dyneither(_tmp375,sizeof(void*),1));});});
return 1;}
# 1917
Cyc_cycstubs_file=Cyc_fopen((const char*)_check_null(_untag_dyneither_ptr(Cyc_Filename_concat(Cyc_output_dir,({const char*_tmp378="cycstubs.cyc";_tag_dyneither(_tmp378,sizeof(char),13);})),sizeof(char),1)),"w");
if(!((unsigned int)Cyc_cycstubs_file)){
({struct Cyc_String_pa_PrintArg_struct _tmp37B;_tmp37B.tag=0;_tmp37B.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_output_dir);({void*_tmp379[1]={& _tmp37B};Cyc_fprintf(Cyc_stderr,({const char*_tmp37A="Error: could not create cycstubs.c in directory %s\n";_tag_dyneither(_tmp37A,sizeof(char),52);}),_tag_dyneither(_tmp379,sizeof(void*),1));});});
# 1922
return 1;}
# 1924
({void*_tmp37C=0;Cyc_fprintf((struct Cyc___cycFILE*)_check_null(Cyc_cycstubs_file),({const char*_tmp37D="#include <core.h>\nusing Core;\n\n";_tag_dyneither(_tmp37D,sizeof(char),32);}),_tag_dyneither(_tmp37C,sizeof(void*),0));});}}{
# 1931
const char*outdir=(const char*)_check_null(_untag_dyneither_ptr(Cyc_output_dir,sizeof(char),1));
if(Cyc_do_setjmp  && Cyc_process_setjmp(outdir))
return 1;else{
# 1937
for(0;Cyc_spec_files != 0;Cyc_spec_files=((struct Cyc_List_List*)_check_null(Cyc_spec_files))->tl){
if(Cyc_process_specfile((const char*)((struct Cyc_List_List*)_check_null(Cyc_spec_files))->hd,outdir)){
({void*_tmp37E=0;Cyc_fprintf(Cyc_stderr,({const char*_tmp37F="FATAL ERROR -- QUIT!\n";_tag_dyneither(_tmp37F,sizeof(char),22);}),_tag_dyneither(_tmp37E,sizeof(void*),0));});
 exit(1);}}}
# 1945
if(Cyc_mode == Cyc_GATHERSCRIPT)
Cyc_fclose((struct Cyc___cycFILE*)_check_null(Cyc_script_file));else{
# 1948
if(!Cyc_gathering()){
Cyc_fclose((struct Cyc___cycFILE*)_check_null(Cyc_log_file));
if(!Cyc_do_setjmp){
Cyc_fclose((struct Cyc___cycFILE*)_check_null(Cyc_cstubs_file));
Cyc_fclose((struct Cyc___cycFILE*)_check_null(Cyc_cycstubs_file));}}}
# 1956
return 0;};};}
