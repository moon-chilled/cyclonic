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
 struct Cyc___cycFILE;
# 53 "cycboot.h"
extern struct Cyc___cycFILE*Cyc_stderr;struct Cyc_String_pa_PrintArg_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Int_pa_PrintArg_struct{int tag;unsigned long f1;};struct Cyc_Double_pa_PrintArg_struct{int tag;double f1;};struct Cyc_LongDouble_pa_PrintArg_struct{int tag;long double f1;};struct Cyc_ShortPtr_pa_PrintArg_struct{int tag;short*f1;};struct Cyc_IntPtr_pa_PrintArg_struct{int tag;unsigned long*f1;};
# 73
struct _dyneither_ptr Cyc_aprintf(struct _dyneither_ptr,struct _dyneither_ptr);
# 100
int Cyc_fprintf(struct Cyc___cycFILE*,struct _dyneither_ptr,struct _dyneither_ptr);struct Cyc_ShortPtr_sa_ScanfArg_struct{int tag;short*f1;};struct Cyc_UShortPtr_sa_ScanfArg_struct{int tag;unsigned short*f1;};struct Cyc_IntPtr_sa_ScanfArg_struct{int tag;int*f1;};struct Cyc_UIntPtr_sa_ScanfArg_struct{int tag;unsigned int*f1;};struct Cyc_StringPtr_sa_ScanfArg_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_DoublePtr_sa_ScanfArg_struct{int tag;double*f1;};struct Cyc_FloatPtr_sa_ScanfArg_struct{int tag;float*f1;};struct Cyc_CharPtr_sa_ScanfArg_struct{int tag;struct _dyneither_ptr f1;};extern char Cyc_FileCloseError[15U];struct Cyc_FileCloseError_exn_struct{char*tag;};extern char Cyc_FileOpenError[14U];struct Cyc_FileOpenError_exn_struct{char*tag;struct _dyneither_ptr f1;};struct Cyc_Core_Opt{void*v;};struct _tuple0{void*f1;void*f2;};
# 113 "core.h"
void*Cyc_Core_snd(struct _tuple0*);extern char Cyc_Core_Invalid_argument[17U];struct Cyc_Core_Invalid_argument_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Failure[8U];struct Cyc_Core_Failure_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Impossible[11U];struct Cyc_Core_Impossible_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Not_found[10U];struct Cyc_Core_Not_found_exn_struct{char*tag;};extern char Cyc_Core_Unreachable[12U];struct Cyc_Core_Unreachable_exn_struct{char*tag;struct _dyneither_ptr f1;};
# 167
extern struct _RegionHandle*Cyc_Core_heap_region;
# 170
extern struct _RegionHandle*Cyc_Core_unique_region;struct Cyc_Core_DynamicRegion;struct Cyc_Core_NewDynamicRegion{struct Cyc_Core_DynamicRegion*key;};struct Cyc_List_List{void*hd;struct Cyc_List_List*tl;};
# 54 "list.h"
struct Cyc_List_List*Cyc_List_list(struct _dyneither_ptr);
# 61
int Cyc_List_length(struct Cyc_List_List*x);
# 76
struct Cyc_List_List*Cyc_List_map(void*(*f)(void*),struct Cyc_List_List*x);
# 83
struct Cyc_List_List*Cyc_List_map_c(void*(*f)(void*,void*),void*env,struct Cyc_List_List*x);
# 86
struct Cyc_List_List*Cyc_List_rmap_c(struct _RegionHandle*,void*(*f)(void*,void*),void*env,struct Cyc_List_List*x);extern char Cyc_List_List_mismatch[14U];struct Cyc_List_List_mismatch_exn_struct{char*tag;};
# 172
struct Cyc_List_List*Cyc_List_rev(struct Cyc_List_List*x);
# 178
struct Cyc_List_List*Cyc_List_imp_rev(struct Cyc_List_List*x);
# 184
struct Cyc_List_List*Cyc_List_append(struct Cyc_List_List*x,struct Cyc_List_List*y);
# 190
struct Cyc_List_List*Cyc_List_rappend(struct _RegionHandle*,struct Cyc_List_List*x,struct Cyc_List_List*y);
# 195
struct Cyc_List_List*Cyc_List_imp_append(struct Cyc_List_List*x,struct Cyc_List_List*y);extern char Cyc_List_Nth[4U];struct Cyc_List_Nth_exn_struct{char*tag;};
# 276
struct Cyc_List_List*Cyc_List_rzip(struct _RegionHandle*r1,struct _RegionHandle*r2,struct Cyc_List_List*x,struct Cyc_List_List*y);struct _tuple1{struct Cyc_List_List*f1;struct Cyc_List_List*f2;};
# 294
struct _tuple1 Cyc_List_split(struct Cyc_List_List*x);
# 379
struct Cyc_List_List*Cyc_List_tabulate_c(int n,void*(*f)(void*,int),void*env);struct Cyc_Iter_Iter{void*env;int(*next)(void*env,void*dest);};
# 37 "iter.h"
int Cyc_Iter_next(struct Cyc_Iter_Iter,void*);struct Cyc_Dict_T;struct Cyc_Dict_Dict{int(*rel)(void*,void*);struct _RegionHandle*r;const struct Cyc_Dict_T*t;};extern char Cyc_Dict_Present[8U];struct Cyc_Dict_Present_exn_struct{char*tag;};extern char Cyc_Dict_Absent[7U];struct Cyc_Dict_Absent_exn_struct{char*tag;};
# 49 "string.h"
int Cyc_strcmp(struct _dyneither_ptr s1,struct _dyneither_ptr s2);
int Cyc_strptrcmp(struct _dyneither_ptr*s1,struct _dyneither_ptr*s2);
# 64
struct _dyneither_ptr Cyc_strconcat_l(struct Cyc_List_List*);struct Cyc_PP_Ppstate;struct Cyc_PP_Out;struct Cyc_PP_Doc;struct Cyc_Position_Error;struct Cyc_Relations_Reln;struct _union_Nmspace_Rel_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Abs_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_C_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Loc_n{int tag;int val;};union Cyc_Absyn_Nmspace{struct _union_Nmspace_Rel_n Rel_n;struct _union_Nmspace_Abs_n Abs_n;struct _union_Nmspace_C_n C_n;struct _union_Nmspace_Loc_n Loc_n;};
# 96 "absyn.h"
union Cyc_Absyn_Nmspace Cyc_Absyn_Loc_n;
union Cyc_Absyn_Nmspace Cyc_Absyn_Rel_n(struct Cyc_List_List*);
# 99
union Cyc_Absyn_Nmspace Cyc_Absyn_Abs_n(struct Cyc_List_List*ns,int C_scope);struct _tuple2{union Cyc_Absyn_Nmspace f1;struct _dyneither_ptr*f2;};
# 158
enum Cyc_Absyn_Scope{Cyc_Absyn_Static  = 0U,Cyc_Absyn_Abstract  = 1U,Cyc_Absyn_Public  = 2U,Cyc_Absyn_Extern  = 3U,Cyc_Absyn_ExternC  = 4U,Cyc_Absyn_Register  = 5U};struct Cyc_Absyn_Tqual{int print_const: 1;int q_volatile: 1;int q_restrict: 1;int real_const: 1;unsigned int loc;};
# 179
enum Cyc_Absyn_Size_of{Cyc_Absyn_Char_sz  = 0U,Cyc_Absyn_Short_sz  = 1U,Cyc_Absyn_Int_sz  = 2U,Cyc_Absyn_Long_sz  = 3U,Cyc_Absyn_LongLong_sz  = 4U};
# 184
enum Cyc_Absyn_AliasQual{Cyc_Absyn_Aliasable  = 0U,Cyc_Absyn_Unique  = 1U,Cyc_Absyn_Top  = 2U};
# 190
enum Cyc_Absyn_KindQual{Cyc_Absyn_AnyKind  = 0U,Cyc_Absyn_MemKind  = 1U,Cyc_Absyn_BoxKind  = 2U,Cyc_Absyn_RgnKind  = 3U,Cyc_Absyn_EffKind  = 4U,Cyc_Absyn_IntKind  = 5U};struct Cyc_Absyn_Kind{enum Cyc_Absyn_KindQual kind;enum Cyc_Absyn_AliasQual aliasqual;};
# 210
enum Cyc_Absyn_Sign{Cyc_Absyn_Signed  = 0U,Cyc_Absyn_Unsigned  = 1U,Cyc_Absyn_None  = 2U};
# 212
enum Cyc_Absyn_AggrKind{Cyc_Absyn_StructA  = 0U,Cyc_Absyn_UnionA  = 1U};struct _union_Constraint_Eq_constr{int tag;void*val;};struct _union_Constraint_Forward_constr{int tag;union Cyc_Absyn_Constraint*val;};struct _union_Constraint_No_constr{int tag;int val;};union Cyc_Absyn_Constraint{struct _union_Constraint_Eq_constr Eq_constr;struct _union_Constraint_Forward_constr Forward_constr;struct _union_Constraint_No_constr No_constr;};struct Cyc_Absyn_Eq_kb_Absyn_KindBound_struct{int tag;struct Cyc_Absyn_Kind*f1;};struct Cyc_Absyn_Unknown_kb_Absyn_KindBound_struct{int tag;struct Cyc_Core_Opt*f1;};struct Cyc_Absyn_Less_kb_Absyn_KindBound_struct{int tag;struct Cyc_Core_Opt*f1;struct Cyc_Absyn_Kind*f2;};struct Cyc_Absyn_Tvar{struct _dyneither_ptr*name;int identity;void*kind;};struct Cyc_Absyn_DynEither_b_Absyn_Bounds_struct{int tag;};struct Cyc_Absyn_Upper_b_Absyn_Bounds_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_PtrLoc{unsigned int ptr_loc;unsigned int rgn_loc;unsigned int zt_loc;};struct Cyc_Absyn_PtrAtts{void*rgn;union Cyc_Absyn_Constraint*nullable;union Cyc_Absyn_Constraint*bounds;union Cyc_Absyn_Constraint*zero_term;struct Cyc_Absyn_PtrLoc*ptrloc;};struct Cyc_Absyn_PtrInfo{void*elt_typ;struct Cyc_Absyn_Tqual elt_tq;struct Cyc_Absyn_PtrAtts ptr_atts;};struct Cyc_Absyn_VarargInfo{struct _dyneither_ptr*name;struct Cyc_Absyn_Tqual tq;void*type;int inject;};struct Cyc_Absyn_FnInfo{struct Cyc_List_List*tvars;void*effect;struct Cyc_Absyn_Tqual ret_tqual;void*ret_typ;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;struct Cyc_List_List*requires_relns;struct Cyc_Absyn_Exp*ensures_clause;struct Cyc_List_List*ensures_relns;};struct Cyc_Absyn_UnknownDatatypeInfo{struct _tuple2*name;int is_extensible;};struct _union_DatatypeInfoU_UnknownDatatype{int tag;struct Cyc_Absyn_UnknownDatatypeInfo val;};struct _union_DatatypeInfoU_KnownDatatype{int tag;struct Cyc_Absyn_Datatypedecl**val;};union Cyc_Absyn_DatatypeInfoU{struct _union_DatatypeInfoU_UnknownDatatype UnknownDatatype;struct _union_DatatypeInfoU_KnownDatatype KnownDatatype;};
# 316
union Cyc_Absyn_DatatypeInfoU Cyc_Absyn_KnownDatatype(struct Cyc_Absyn_Datatypedecl**);struct Cyc_Absyn_DatatypeInfo{union Cyc_Absyn_DatatypeInfoU datatype_info;struct Cyc_List_List*targs;};struct Cyc_Absyn_UnknownDatatypeFieldInfo{struct _tuple2*datatype_name;struct _tuple2*field_name;int is_extensible;};struct _union_DatatypeFieldInfoU_UnknownDatatypefield{int tag;struct Cyc_Absyn_UnknownDatatypeFieldInfo val;};struct _tuple3{struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;};struct _union_DatatypeFieldInfoU_KnownDatatypefield{int tag;struct _tuple3 val;};union Cyc_Absyn_DatatypeFieldInfoU{struct _union_DatatypeFieldInfoU_UnknownDatatypefield UnknownDatatypefield;struct _union_DatatypeFieldInfoU_KnownDatatypefield KnownDatatypefield;};
# 334
union Cyc_Absyn_DatatypeFieldInfoU Cyc_Absyn_KnownDatatypefield(struct Cyc_Absyn_Datatypedecl*,struct Cyc_Absyn_Datatypefield*);struct Cyc_Absyn_DatatypeFieldInfo{union Cyc_Absyn_DatatypeFieldInfoU field_info;struct Cyc_List_List*targs;};struct _tuple4{enum Cyc_Absyn_AggrKind f1;struct _tuple2*f2;struct Cyc_Core_Opt*f3;};struct _union_AggrInfoU_UnknownAggr{int tag;struct _tuple4 val;};struct _union_AggrInfoU_KnownAggr{int tag;struct Cyc_Absyn_Aggrdecl**val;};union Cyc_Absyn_AggrInfoU{struct _union_AggrInfoU_UnknownAggr UnknownAggr;struct _union_AggrInfoU_KnownAggr KnownAggr;};
# 347
union Cyc_Absyn_AggrInfoU Cyc_Absyn_KnownAggr(struct Cyc_Absyn_Aggrdecl**);struct Cyc_Absyn_AggrInfo{union Cyc_Absyn_AggrInfoU aggr_info;struct Cyc_List_List*targs;};struct Cyc_Absyn_ArrayInfo{void*elt_type;struct Cyc_Absyn_Tqual tq;struct Cyc_Absyn_Exp*num_elts;union Cyc_Absyn_Constraint*zero_term;unsigned int zt_loc;};struct Cyc_Absyn_Aggr_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Absyn_Enum_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;};struct Cyc_Absyn_Datatype_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;};struct Cyc_Absyn_TypeDecl{void*r;unsigned int loc;};struct Cyc_Absyn_VoidType_Absyn_Type_struct{int tag;};struct Cyc_Absyn_Evar_Absyn_Type_struct{int tag;struct Cyc_Core_Opt*f1;void*f2;int f3;struct Cyc_Core_Opt*f4;};struct Cyc_Absyn_VarType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Tvar*f1;};struct Cyc_Absyn_DatatypeType_Absyn_Type_struct{int tag;struct Cyc_Absyn_DatatypeInfo f1;};struct Cyc_Absyn_DatatypeFieldType_Absyn_Type_struct{int tag;struct Cyc_Absyn_DatatypeFieldInfo f1;};struct Cyc_Absyn_PointerType_Absyn_Type_struct{int tag;struct Cyc_Absyn_PtrInfo f1;};struct Cyc_Absyn_IntType_Absyn_Type_struct{int tag;enum Cyc_Absyn_Sign f1;enum Cyc_Absyn_Size_of f2;};struct Cyc_Absyn_FloatType_Absyn_Type_struct{int tag;int f1;};struct Cyc_Absyn_ArrayType_Absyn_Type_struct{int tag;struct Cyc_Absyn_ArrayInfo f1;};struct Cyc_Absyn_FnType_Absyn_Type_struct{int tag;struct Cyc_Absyn_FnInfo f1;};struct Cyc_Absyn_TupleType_Absyn_Type_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_AggrType_Absyn_Type_struct{int tag;struct Cyc_Absyn_AggrInfo f1;};struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct{int tag;enum Cyc_Absyn_AggrKind f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_EnumType_Absyn_Type_struct{int tag;struct _tuple2*f1;struct Cyc_Absyn_Enumdecl*f2;};struct Cyc_Absyn_AnonEnumType_Absyn_Type_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_RgnHandleType_Absyn_Type_struct{int tag;void*f1;};struct Cyc_Absyn_DynRgnType_Absyn_Type_struct{int tag;void*f1;void*f2;};struct Cyc_Absyn_TypedefType_Absyn_Type_struct{int tag;struct _tuple2*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_Typedefdecl*f3;void*f4;};struct Cyc_Absyn_ValueofType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_TagType_Absyn_Type_struct{int tag;void*f1;};struct Cyc_Absyn_HeapRgn_Absyn_Type_struct{int tag;};struct Cyc_Absyn_UniqueRgn_Absyn_Type_struct{int tag;};struct Cyc_Absyn_RefCntRgn_Absyn_Type_struct{int tag;};struct Cyc_Absyn_AccessEff_Absyn_Type_struct{int tag;void*f1;};struct Cyc_Absyn_JoinEff_Absyn_Type_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_RgnsEff_Absyn_Type_struct{int tag;void*f1;};struct Cyc_Absyn_TypeDeclType_Absyn_Type_struct{int tag;struct Cyc_Absyn_TypeDecl*f1;void**f2;};struct Cyc_Absyn_TypeofType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_BuiltinType_Absyn_Type_struct{int tag;struct _dyneither_ptr f1;struct Cyc_Absyn_Kind*f2;};
# 428 "absyn.h"
extern struct Cyc_Absyn_UniqueRgn_Absyn_Type_struct Cyc_Absyn_UniqueRgn_val;
# 430
extern struct Cyc_Absyn_VoidType_Absyn_Type_struct Cyc_Absyn_VoidType_val;struct Cyc_Absyn_NoTypes_Absyn_Funcparams_struct{int tag;struct Cyc_List_List*f1;unsigned int f2;};struct Cyc_Absyn_WithTypes_Absyn_Funcparams_struct{int tag;struct Cyc_List_List*f1;int f2;struct Cyc_Absyn_VarargInfo*f3;void*f4;struct Cyc_List_List*f5;struct Cyc_Absyn_Exp*f6;struct Cyc_Absyn_Exp*f7;};
# 446
enum Cyc_Absyn_Format_Type{Cyc_Absyn_Printf_ft  = 0U,Cyc_Absyn_Scanf_ft  = 1U};struct Cyc_Absyn_Regparm_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Stdcall_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Cdecl_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Fastcall_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Noreturn_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Const_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Aligned_att_Absyn_Attribute_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Packed_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Section_att_Absyn_Attribute_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Nocommon_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Shared_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Unused_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Weak_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Dllimport_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Dllexport_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_No_instrument_function_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Constructor_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Destructor_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_No_check_memory_usage_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Format_att_Absyn_Attribute_struct{int tag;enum Cyc_Absyn_Format_Type f1;int f2;int f3;};struct Cyc_Absyn_Initializes_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Noliveunique_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Noconsume_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Pure_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Mode_att_Absyn_Attribute_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Alias_att_Absyn_Attribute_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Always_inline_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Carray_mod_Absyn_Type_modifier_struct{int tag;union Cyc_Absyn_Constraint*f1;unsigned int f2;};struct Cyc_Absyn_ConstArray_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_Absyn_Exp*f1;union Cyc_Absyn_Constraint*f2;unsigned int f3;};struct Cyc_Absyn_Pointer_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_Absyn_PtrAtts f1;struct Cyc_Absyn_Tqual f2;};struct Cyc_Absyn_Function_mod_Absyn_Type_modifier_struct{int tag;void*f1;};struct Cyc_Absyn_TypeParams_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_List_List*f1;unsigned int f2;int f3;};struct Cyc_Absyn_Attributes_mod_Absyn_Type_modifier_struct{int tag;unsigned int f1;struct Cyc_List_List*f2;};struct _union_Cnst_Null_c{int tag;int val;};struct _tuple5{enum Cyc_Absyn_Sign f1;char f2;};struct _union_Cnst_Char_c{int tag;struct _tuple5 val;};struct _union_Cnst_Wchar_c{int tag;struct _dyneither_ptr val;};struct _tuple6{enum Cyc_Absyn_Sign f1;short f2;};struct _union_Cnst_Short_c{int tag;struct _tuple6 val;};struct _tuple7{enum Cyc_Absyn_Sign f1;int f2;};struct _union_Cnst_Int_c{int tag;struct _tuple7 val;};struct _tuple8{enum Cyc_Absyn_Sign f1;long long f2;};struct _union_Cnst_LongLong_c{int tag;struct _tuple8 val;};struct _tuple9{struct _dyneither_ptr f1;int f2;};struct _union_Cnst_Float_c{int tag;struct _tuple9 val;};struct _union_Cnst_String_c{int tag;struct _dyneither_ptr val;};struct _union_Cnst_Wstring_c{int tag;struct _dyneither_ptr val;};union Cyc_Absyn_Cnst{struct _union_Cnst_Null_c Null_c;struct _union_Cnst_Char_c Char_c;struct _union_Cnst_Wchar_c Wchar_c;struct _union_Cnst_Short_c Short_c;struct _union_Cnst_Int_c Int_c;struct _union_Cnst_LongLong_c LongLong_c;struct _union_Cnst_Float_c Float_c;struct _union_Cnst_String_c String_c;struct _union_Cnst_Wstring_c Wstring_c;};
# 536
enum Cyc_Absyn_Primop{Cyc_Absyn_Plus  = 0U,Cyc_Absyn_Times  = 1U,Cyc_Absyn_Minus  = 2U,Cyc_Absyn_Div  = 3U,Cyc_Absyn_Mod  = 4U,Cyc_Absyn_Eq  = 5U,Cyc_Absyn_Neq  = 6U,Cyc_Absyn_Gt  = 7U,Cyc_Absyn_Lt  = 8U,Cyc_Absyn_Gte  = 9U,Cyc_Absyn_Lte  = 10U,Cyc_Absyn_Not  = 11U,Cyc_Absyn_Bitnot  = 12U,Cyc_Absyn_Bitand  = 13U,Cyc_Absyn_Bitor  = 14U,Cyc_Absyn_Bitxor  = 15U,Cyc_Absyn_Bitlshift  = 16U,Cyc_Absyn_Bitlrshift  = 17U,Cyc_Absyn_Bitarshift  = 18U,Cyc_Absyn_Numelts  = 19U};
# 543
enum Cyc_Absyn_Incrementor{Cyc_Absyn_PreInc  = 0U,Cyc_Absyn_PostInc  = 1U,Cyc_Absyn_PreDec  = 2U,Cyc_Absyn_PostDec  = 3U};struct Cyc_Absyn_VarargCallInfo{int num_varargs;struct Cyc_List_List*injectors;struct Cyc_Absyn_VarargInfo*vai;};struct Cyc_Absyn_StructField_Absyn_OffsetofField_struct{int tag;struct _dyneither_ptr*f1;};struct Cyc_Absyn_TupleIndex_Absyn_OffsetofField_struct{int tag;unsigned int f1;};
# 561
enum Cyc_Absyn_Coercion{Cyc_Absyn_Unknown_coercion  = 0U,Cyc_Absyn_No_coercion  = 1U,Cyc_Absyn_Null_to_NonNull  = 2U,Cyc_Absyn_Other_coercion  = 3U};struct Cyc_Absyn_MallocInfo{int is_calloc;struct Cyc_Absyn_Exp*rgn;void**elt_type;struct Cyc_Absyn_Exp*num_elts;int fat_result;int inline_call;};struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct{int tag;union Cyc_Absyn_Cnst f1;};struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Pragma_e_Absyn_Raw_exp_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct{int tag;enum Cyc_Absyn_Primop f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Increment_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;enum Cyc_Absyn_Incrementor f2;};struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_VarargCallInfo*f3;int f4;};struct Cyc_Absyn_Throw_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;int f2;};struct Cyc_Absyn_NoInstantiate_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Instantiate_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_Absyn_Exp*f2;int f3;enum Cyc_Absyn_Coercion f4;};struct Cyc_Absyn_Address_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_New_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Sizeoftyp_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Sizeofexp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Offsetof_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;int f3;int f4;};struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;int f3;int f4;};struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Tuple_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;};struct _tuple10{struct _dyneither_ptr*f1;struct Cyc_Absyn_Tqual f2;void*f3;};struct Cyc_Absyn_CompoundLit_e_Absyn_Raw_exp_struct{int tag;struct _tuple10*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Array_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Comprehension_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;int f4;};struct Cyc_Absyn_ComprehensionNoinit_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;void*f2;int f3;};struct Cyc_Absyn_Aggregate_e_Absyn_Raw_exp_struct{int tag;struct _tuple2*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;struct Cyc_Absyn_Aggrdecl*f4;};struct Cyc_Absyn_AnonStruct_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Datatype_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Datatypedecl*f2;struct Cyc_Absyn_Datatypefield*f3;};struct Cyc_Absyn_Enum_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_AnonEnum_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_MallocInfo f1;};struct Cyc_Absyn_Swap_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Core_Opt*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_StmtExp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Tagcheck_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;};struct Cyc_Absyn_Valueof_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Asm_e_Absyn_Raw_exp_struct{int tag;int f1;struct _dyneither_ptr f2;};struct Cyc_Absyn_Exp{void*topt;void*r;unsigned int loc;void*annot;};struct Cyc_Absyn_Skip_s_Absyn_Raw_stmt_struct{int tag;};struct Cyc_Absyn_Exp_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Return_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;struct Cyc_Absyn_Stmt*f3;};struct _tuple11{struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_While_s_Absyn_Raw_stmt_struct{int tag;struct _tuple11 f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Break_s_Absyn_Raw_stmt_struct{int tag;};struct Cyc_Absyn_Continue_s_Absyn_Raw_stmt_struct{int tag;};struct Cyc_Absyn_Goto_s_Absyn_Raw_stmt_struct{int tag;struct _dyneither_ptr*f1;};struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _tuple11 f2;struct _tuple11 f3;struct Cyc_Absyn_Stmt*f4;};struct Cyc_Absyn_Switch_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;void*f3;};struct Cyc_Absyn_Fallthru_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Switch_clause**f2;};struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Decl*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Label_s_Absyn_Raw_stmt_struct{int tag;struct _dyneither_ptr*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Do_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct _tuple11 f2;};struct Cyc_Absyn_TryCatch_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_List_List*f2;void*f3;};struct Cyc_Absyn_Stmt{void*r;unsigned int loc;void*annot;};struct Cyc_Absyn_Wild_p_Absyn_Raw_pat_struct{int tag;};struct Cyc_Absyn_Var_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_AliasVar_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;};struct Cyc_Absyn_Reference_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_TagInt_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;};struct Cyc_Absyn_Tuple_p_Absyn_Raw_pat_struct{int tag;struct Cyc_List_List*f1;int f2;};struct Cyc_Absyn_Pointer_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Pat*f1;};struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_AggrInfo*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;int f4;};struct Cyc_Absyn_Datatype_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;struct Cyc_List_List*f3;int f4;};struct Cyc_Absyn_Null_p_Absyn_Raw_pat_struct{int tag;};struct Cyc_Absyn_Int_p_Absyn_Raw_pat_struct{int tag;enum Cyc_Absyn_Sign f1;int f2;};struct Cyc_Absyn_Char_p_Absyn_Raw_pat_struct{int tag;char f1;};struct Cyc_Absyn_Float_p_Absyn_Raw_pat_struct{int tag;struct _dyneither_ptr f1;int f2;};struct Cyc_Absyn_Enum_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_AnonEnum_p_Absyn_Raw_pat_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_UnknownId_p_Absyn_Raw_pat_struct{int tag;struct _tuple2*f1;};struct Cyc_Absyn_UnknownCall_p_Absyn_Raw_pat_struct{int tag;struct _tuple2*f1;struct Cyc_List_List*f2;int f3;};struct Cyc_Absyn_Exp_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Exp*f1;};
# 724 "absyn.h"
extern struct Cyc_Absyn_Wild_p_Absyn_Raw_pat_struct Cyc_Absyn_Wild_p_val;struct Cyc_Absyn_Pat{void*r;void*topt;unsigned int loc;};struct Cyc_Absyn_Switch_clause{struct Cyc_Absyn_Pat*pattern;struct Cyc_Core_Opt*pat_vars;struct Cyc_Absyn_Exp*where_clause;struct Cyc_Absyn_Stmt*body;unsigned int loc;};struct Cyc_Absyn_Unresolved_b_Absyn_Binding_struct{int tag;struct _tuple2*f1;};struct Cyc_Absyn_Global_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Funname_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Param_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Local_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Pat_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Vardecl{enum Cyc_Absyn_Scope sc;struct _tuple2*name;unsigned int varloc;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*initializer;void*rgn;struct Cyc_List_List*attributes;int escapes;};struct Cyc_Absyn_Fndecl{enum Cyc_Absyn_Scope sc;int is_inline;struct _tuple2*name;struct Cyc_List_List*tvs;void*effect;struct Cyc_Absyn_Tqual ret_tqual;void*ret_type;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_Absyn_Stmt*body;void*cached_typ;struct Cyc_Core_Opt*param_vardecls;struct Cyc_Absyn_Vardecl*fn_vardecl;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;struct Cyc_List_List*requires_relns;struct Cyc_Absyn_Exp*ensures_clause;struct Cyc_List_List*ensures_relns;};struct Cyc_Absyn_Aggrfield{struct _dyneither_ptr*name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*width;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;};struct Cyc_Absyn_AggrdeclImpl{struct Cyc_List_List*exist_vars;struct Cyc_List_List*rgn_po;struct Cyc_List_List*fields;int tagged;};struct Cyc_Absyn_Aggrdecl{enum Cyc_Absyn_AggrKind kind;enum Cyc_Absyn_Scope sc;struct _tuple2*name;struct Cyc_List_List*tvs;struct Cyc_Absyn_AggrdeclImpl*impl;struct Cyc_List_List*attributes;int expected_mem_kind;};struct Cyc_Absyn_Datatypefield{struct _tuple2*name;struct Cyc_List_List*typs;unsigned int loc;enum Cyc_Absyn_Scope sc;};struct Cyc_Absyn_Datatypedecl{enum Cyc_Absyn_Scope sc;struct _tuple2*name;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*fields;int is_extensible;};struct Cyc_Absyn_Enumfield{struct _tuple2*name;struct Cyc_Absyn_Exp*tag;unsigned int loc;};struct Cyc_Absyn_Enumdecl{enum Cyc_Absyn_Scope sc;struct _tuple2*name;struct Cyc_Core_Opt*fields;};struct Cyc_Absyn_Typedefdecl{struct _tuple2*name;struct Cyc_Absyn_Tqual tq;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*kind;void*defn;struct Cyc_List_List*atts;int extern_c;};struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Let_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Pat*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;void*f4;};struct Cyc_Absyn_Letv_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Region_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Absyn_Datatype_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;};struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;};struct Cyc_Absyn_Typedef_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Typedefdecl*f1;};struct Cyc_Absyn_Namespace_d_Absyn_Raw_decl_struct{int tag;struct _dyneither_ptr*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Using_d_Absyn_Raw_decl_struct{int tag;struct _tuple2*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_ExternC_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_ExternCinclude_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Porton_d_Absyn_Raw_decl_struct{int tag;};struct Cyc_Absyn_Portoff_d_Absyn_Raw_decl_struct{int tag;};struct Cyc_Absyn_Decl{void*r;unsigned int loc;};struct Cyc_Absyn_ArrayElement_Absyn_Designator_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_FieldName_Absyn_Designator_struct{int tag;struct _dyneither_ptr*f1;};extern char Cyc_Absyn_EmptyAnnot[11U];struct Cyc_Absyn_EmptyAnnot_Absyn_AbsynAnnot_struct{char*tag;};
# 916
int Cyc_Absyn_qvar_cmp(struct _tuple2*,struct _tuple2*);
# 924
struct Cyc_Absyn_Tqual Cyc_Absyn_const_tqual(unsigned int);
# 926
struct Cyc_Absyn_Tqual Cyc_Absyn_empty_tqual(unsigned int);
# 930
union Cyc_Absyn_Constraint*Cyc_Absyn_empty_conref();
# 936
void*Cyc_Absyn_conref_def(void*y,union Cyc_Absyn_Constraint*x);
# 938
void*Cyc_Absyn_conref_constr(void*y,union Cyc_Absyn_Constraint*x);
extern union Cyc_Absyn_Constraint*Cyc_Absyn_true_conref;
extern union Cyc_Absyn_Constraint*Cyc_Absyn_false_conref;
extern union Cyc_Absyn_Constraint*Cyc_Absyn_bounds_one_conref;
# 944
void*Cyc_Absyn_compress_kb(void*);
# 949
void*Cyc_Absyn_new_evar(struct Cyc_Core_Opt*k,struct Cyc_Core_Opt*tenv);
# 951
void*Cyc_Absyn_wildtyp(struct Cyc_Core_Opt*);
# 954
extern void*Cyc_Absyn_char_typ;extern void*Cyc_Absyn_uint_typ;
# 956
extern void*Cyc_Absyn_sint_typ;
# 958
void*Cyc_Absyn_float_typ(int);
# 960
extern void*Cyc_Absyn_empty_effect;
# 1007
struct Cyc_Absyn_Exp*Cyc_Absyn_const_exp(union Cyc_Absyn_Cnst,unsigned int);
# 1055
struct Cyc_Absyn_Exp*Cyc_Absyn_address_exp(struct Cyc_Absyn_Exp*,unsigned int);
# 1059
struct Cyc_Absyn_Exp*Cyc_Absyn_deref_exp(struct Cyc_Absyn_Exp*,unsigned int);
struct Cyc_Absyn_Exp*Cyc_Absyn_aggrmember_exp(struct Cyc_Absyn_Exp*,struct _dyneither_ptr*,unsigned int);
# 1062
struct Cyc_Absyn_Exp*Cyc_Absyn_subscript_exp(struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,unsigned int);
# 1077
struct Cyc_Absyn_Stmt*Cyc_Absyn_skip_stmt(unsigned int loc);
# 1169
struct Cyc_Absyn_Aggrdecl*Cyc_Absyn_get_known_aggrdecl(union Cyc_Absyn_AggrInfoU info);struct Cyc_Absynpp_Params{int expand_typedefs;int qvar_to_Cids;int add_cyc_prefix;int to_VC;int decls_first;int rewrite_temp_tvars;int print_all_tvars;int print_all_kinds;int print_all_effects;int print_using_stmts;int print_externC_stmts;int print_full_evars;int print_zeroterm;int generate_line_directives;int use_curr_namespace;struct Cyc_List_List*curr_namespace;};
# 62 "absynpp.h"
struct _dyneither_ptr Cyc_Absynpp_typ2string(void*);
# 64
struct _dyneither_ptr Cyc_Absynpp_kind2string(struct Cyc_Absyn_Kind*);
struct _dyneither_ptr Cyc_Absynpp_kindbound2string(void*);
# 67
struct _dyneither_ptr Cyc_Absynpp_exp2string(struct Cyc_Absyn_Exp*);
struct _dyneither_ptr Cyc_Absynpp_stmt2string(struct Cyc_Absyn_Stmt*);
struct _dyneither_ptr Cyc_Absynpp_qvar2string(struct _tuple2*);
# 72
struct _dyneither_ptr Cyc_Absynpp_pat2string(struct Cyc_Absyn_Pat*p);struct Cyc_Set_Set;
# 54 "set.h"
struct Cyc_Set_Set*Cyc_Set_rempty(struct _RegionHandle*r,int(*cmp)(void*,void*));
# 63
struct Cyc_Set_Set*Cyc_Set_insert(struct Cyc_Set_Set*s,void*elt);
# 94
int Cyc_Set_cardinality(struct Cyc_Set_Set*s);
# 97
int Cyc_Set_is_empty(struct Cyc_Set_Set*s);
# 100
int Cyc_Set_member(struct Cyc_Set_Set*s,void*elt);extern char Cyc_Set_Absent[7U];struct Cyc_Set_Absent_exn_struct{char*tag;};
# 137
void*Cyc_Set_choose(struct Cyc_Set_Set*s);struct Cyc_RgnOrder_RgnPO;
# 32 "rgnorder.h"
struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_initial_fn_po(struct Cyc_List_List*tvs,struct Cyc_List_List*po,void*effect,struct Cyc_Absyn_Tvar*fst_rgn,unsigned int);
# 39
struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_add_outlives_constraint(struct Cyc_RgnOrder_RgnPO*,void*eff,void*rgn,unsigned int);
struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_add_youngest(struct Cyc_RgnOrder_RgnPO*,struct Cyc_Absyn_Tvar*rgn,int opened);
struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_add_unordered(struct Cyc_RgnOrder_RgnPO*,struct Cyc_Absyn_Tvar*rgn);
int Cyc_RgnOrder_effect_outlives(struct Cyc_RgnOrder_RgnPO*,void*eff,void*rgn);
int Cyc_RgnOrder_satisfies_constraints(struct Cyc_RgnOrder_RgnPO*,struct Cyc_List_List*constraints,void*default_bound,int do_pin);
# 45
int Cyc_RgnOrder_eff_outlives_eff(struct Cyc_RgnOrder_RgnPO*,void*eff1,void*eff2);
# 48
void Cyc_RgnOrder_print_region_po(struct Cyc_RgnOrder_RgnPO*po);extern char Cyc_Tcenv_Env_error[10U];struct Cyc_Tcenv_Env_error_exn_struct{char*tag;};struct Cyc_Tcenv_Genv{struct Cyc_Dict_Dict aggrdecls;struct Cyc_Dict_Dict datatypedecls;struct Cyc_Dict_Dict enumdecls;struct Cyc_Dict_Dict typedefs;struct Cyc_Dict_Dict ordinaries;};struct Cyc_Tcenv_Fenv;struct Cyc_Tcenv_Tenv{struct Cyc_List_List*ns;struct Cyc_Tcenv_Genv*ae;struct Cyc_Tcenv_Fenv*le;int allow_valueof: 1;int in_extern_c_include: 1;};
# 84 "tcenv.h"
enum Cyc_Tcenv_NewStatus{Cyc_Tcenv_NoneNew  = 0U,Cyc_Tcenv_InNew  = 1U,Cyc_Tcenv_InNewAggr  = 2U};
# 94
struct Cyc_List_List*Cyc_Tcenv_lookup_type_vars(struct Cyc_Tcenv_Tenv*);
struct Cyc_Core_Opt*Cyc_Tcenv_lookup_opt_type_vars(struct Cyc_Tcenv_Tenv*);
struct Cyc_Tcenv_Tenv*Cyc_Tcenv_add_type_vars(unsigned int,struct Cyc_Tcenv_Tenv*,struct Cyc_List_List*);
# 135
struct Cyc_Tcenv_Tenv*Cyc_Tcenv_add_region(struct Cyc_Tcenv_Tenv*,void*,int opened);
# 137
void Cyc_Tcenv_check_rgn_accessible(struct Cyc_Tcenv_Tenv*,unsigned int,void*rgn);
# 30 "tcutil.h"
void*Cyc_Tcutil_impos(struct _dyneither_ptr fmt,struct _dyneither_ptr ap);
# 32
void Cyc_Tcutil_terr(unsigned int,struct _dyneither_ptr fmt,struct _dyneither_ptr ap);
# 34
void Cyc_Tcutil_warn(unsigned int,struct _dyneither_ptr fmt,struct _dyneither_ptr ap);
# 45
struct Cyc_Absyn_Exp*Cyc_Tcutil_deep_copy_exp(int preserve_types,struct Cyc_Absyn_Exp*);
# 48
int Cyc_Tcutil_kind_leq(struct Cyc_Absyn_Kind*k1,struct Cyc_Absyn_Kind*k2);
# 53
struct Cyc_Absyn_Kind*Cyc_Tcutil_typ_kind(void*t);
# 55
void*Cyc_Tcutil_compress(void*t);
# 58
int Cyc_Tcutil_coerce_assign(struct Cyc_Tcenv_Tenv*,struct Cyc_Absyn_Exp*,void*);
# 64
int Cyc_Tcutil_coerceable(void*);
# 71
int Cyc_Tcutil_subtype(struct Cyc_Tcenv_Tenv*te,struct Cyc_List_List*assume,void*t1,void*t2);
# 95
extern struct Cyc_Absyn_Kind Cyc_Tcutil_ak;
# 104
extern struct Cyc_Absyn_Kind Cyc_Tcutil_tmk;
# 111
extern struct Cyc_Core_Opt Cyc_Tcutil_rko;
extern struct Cyc_Core_Opt Cyc_Tcutil_ako;
# 114
extern struct Cyc_Core_Opt Cyc_Tcutil_mko;
# 118
extern struct Cyc_Core_Opt Cyc_Tcutil_trko;
# 145
int Cyc_Tcutil_unify(void*,void*);
# 148
void*Cyc_Tcutil_substitute(struct Cyc_List_List*,void*);
# 150
void*Cyc_Tcutil_rsubstitute(struct _RegionHandle*,struct Cyc_List_List*,void*);struct _tuple12{struct Cyc_List_List*f1;struct _RegionHandle*f2;};struct _tuple13{struct Cyc_Absyn_Tvar*f1;void*f2;};
# 177
struct _tuple13*Cyc_Tcutil_r_make_inst_var(struct _tuple12*,struct Cyc_Absyn_Tvar*);
# 219 "tcutil.h"
void Cyc_Tcutil_check_type(unsigned int,struct Cyc_Tcenv_Tenv*,struct Cyc_List_List*bound_tvars,struct Cyc_Absyn_Kind*k,int allow_evars,int allow_abs_aggr,void*);
# 222
void Cyc_Tcutil_check_unique_vars(struct Cyc_List_List*vs,unsigned int loc,struct _dyneither_ptr err_msg);
# 237
struct Cyc_List_List*Cyc_Tcutil_resolve_aggregate_designators(struct _RegionHandle*rgn,unsigned int loc,struct Cyc_List_List*des,enum Cyc_Absyn_AggrKind,struct Cyc_List_List*fields);
# 262
int Cyc_Tcutil_is_noalias_region(void*r,int must_be_unique);
# 270
int Cyc_Tcutil_is_noalias_path(struct Cyc_Absyn_Exp*e);
# 275
int Cyc_Tcutil_is_noalias_pointer_or_aggr(void*t);
# 290
struct Cyc_Absyn_Tvar*Cyc_Tcutil_new_tvar(void*k);
# 309
int Cyc_Tcutil_is_const_exp(struct Cyc_Absyn_Exp*e);
# 354
int Cyc_Tcutil_is_array(void*t);
# 358
void*Cyc_Tcutil_promote_array(void*t,void*rgn,int convert_tag);
# 28 "tcexp.h"
void*Cyc_Tcexp_tcExp(struct Cyc_Tcenv_Tenv*,void**,struct Cyc_Absyn_Exp*);struct Cyc_Tcexp_TestEnv{struct _tuple0*eq;int isTrue;};
# 36
struct Cyc_Tcexp_TestEnv Cyc_Tcexp_tcTest(struct Cyc_Tcenv_Tenv*te,struct Cyc_Absyn_Exp*e,struct _dyneither_ptr msg_part);struct Cyc_Tcpat_TcPatResult{struct _tuple1*tvars_and_bounds_opt;struct Cyc_List_List*patvars;};
# 53 "tcpat.h"
struct Cyc_Tcpat_TcPatResult Cyc_Tcpat_tcPat(struct Cyc_Tcenv_Tenv*te,struct Cyc_Absyn_Pat*p,void**topt,struct Cyc_Absyn_Exp*pat_var_exp);
# 55
void Cyc_Tcpat_check_pat_regions(struct Cyc_Tcenv_Tenv*te,struct Cyc_Absyn_Pat*p,struct Cyc_List_List*patvars);struct Cyc_Tcpat_WhereTest_Tcpat_PatTest_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Tcpat_EqNull_Tcpat_PatTest_struct{int tag;};struct Cyc_Tcpat_NeqNull_Tcpat_PatTest_struct{int tag;};struct Cyc_Tcpat_EqEnum_Tcpat_PatTest_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Tcpat_EqAnonEnum_Tcpat_PatTest_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Tcpat_EqFloat_Tcpat_PatTest_struct{int tag;struct _dyneither_ptr f1;int f2;};struct Cyc_Tcpat_EqConst_Tcpat_PatTest_struct{int tag;unsigned int f1;};struct Cyc_Tcpat_EqDatatypeTag_Tcpat_PatTest_struct{int tag;int f1;struct Cyc_Absyn_Datatypedecl*f2;struct Cyc_Absyn_Datatypefield*f3;};struct Cyc_Tcpat_EqTaggedUnion_Tcpat_PatTest_struct{int tag;struct _dyneither_ptr*f1;int f2;};struct Cyc_Tcpat_EqExtensibleDatatype_Tcpat_PatTest_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;};struct Cyc_Tcpat_Dummy_Tcpat_Access_struct{int tag;};struct Cyc_Tcpat_Deref_Tcpat_Access_struct{int tag;};struct Cyc_Tcpat_TupleField_Tcpat_Access_struct{int tag;unsigned int f1;};struct Cyc_Tcpat_DatatypeField_Tcpat_Access_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;unsigned int f3;};struct Cyc_Tcpat_AggrField_Tcpat_Access_struct{int tag;int f1;struct _dyneither_ptr*f2;};struct _union_PatOrWhere_pattern{int tag;struct Cyc_Absyn_Pat*val;};struct _union_PatOrWhere_where_clause{int tag;struct Cyc_Absyn_Exp*val;};union Cyc_Tcpat_PatOrWhere{struct _union_PatOrWhere_pattern pattern;struct _union_PatOrWhere_where_clause where_clause;};struct Cyc_Tcpat_PathNode{union Cyc_Tcpat_PatOrWhere orig_pat;void*access;};struct Cyc_Tcpat_Rhs{int used;unsigned int pat_loc;struct Cyc_Absyn_Stmt*rhs;};struct Cyc_Tcpat_Failure_Tcpat_Decision_struct{int tag;void*f1;};struct Cyc_Tcpat_Success_Tcpat_Decision_struct{int tag;struct Cyc_Tcpat_Rhs*f1;};struct Cyc_Tcpat_SwitchDec_Tcpat_Decision_struct{int tag;struct Cyc_List_List*f1;struct Cyc_List_List*f2;void*f3;};
# 107
void Cyc_Tcpat_check_switch_exhaustive(unsigned int,struct Cyc_Tcenv_Tenv*,struct Cyc_List_List*,void**);
# 109
int Cyc_Tcpat_check_let_pat_exhaustive(unsigned int,struct Cyc_Tcenv_Tenv*,struct Cyc_Absyn_Pat*p,void**);
# 111
void Cyc_Tcpat_check_catch_overlap(unsigned int,struct Cyc_Tcenv_Tenv*,struct Cyc_List_List*,void**);
# 113
void Cyc_Tcpat_print_decision_tree(void*);
# 115
int Cyc_Tcpat_has_vars(struct Cyc_Core_Opt*pat_vars);struct _tuple14{unsigned int f1;int f2;};
# 28 "evexp.h"
struct _tuple14 Cyc_Evexp_eval_const_uint_exp(struct Cyc_Absyn_Exp*e);
# 42 "tcpat.cyc"
static void Cyc_Tcpat_resolve_pat(struct Cyc_Tcenv_Tenv*te,void**topt,struct Cyc_Absyn_Pat*p){
void*_tmp0=p->r;void*_tmp1=_tmp0;struct Cyc_Absyn_Exp*_tmp32;struct Cyc_Absyn_Aggrdecl**_tmp31;struct Cyc_List_List*_tmp30;struct Cyc_List_List**_tmp2F;struct Cyc_List_List*_tmp2E;int _tmp2D;struct Cyc_List_List*_tmp2C;struct Cyc_List_List*_tmp2B;int _tmp2A;switch(*((int*)_tmp1)){case 7U: if(((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp1)->f1 == 0){_LL1: _tmp2C=((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp1)->f2;_tmp2B=((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp1)->f3;_tmp2A=((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp1)->f4;_LL2:
# 48
 if(topt == 0)
({void*_tmp2=0U;({unsigned int _tmp3E2=p->loc;struct _dyneither_ptr _tmp3E1=({const char*_tmp3="cannot determine pattern type";_tag_dyneither(_tmp3,sizeof(char),30U);});Cyc_Tcutil_terr(_tmp3E2,_tmp3E1,_tag_dyneither(_tmp2,sizeof(void*),0U));});});{
void*t=Cyc_Tcutil_compress(*((void**)_check_null(topt)));
{void*_tmp4=t;struct Cyc_Absyn_AggrInfo _tmpB;if(((struct Cyc_Absyn_AggrType_Absyn_Type_struct*)_tmp4)->tag == 11U){_LL10: _tmpB=((struct Cyc_Absyn_AggrType_Absyn_Type_struct*)_tmp4)->f1;_LL11:
# 53
({void*_tmp3E5=(void*)({struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*_tmp5=_cycalloc(sizeof(*_tmp5));({struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct _tmp3E4=({struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct _tmp6;_tmp6.tag=7U;({struct Cyc_Absyn_AggrInfo*_tmp3E3=({struct Cyc_Absyn_AggrInfo*_tmp7=_cycalloc(sizeof(*_tmp7));_tmp7[0]=_tmpB;_tmp7;});_tmp6.f1=_tmp3E3;});_tmp6.f2=_tmp2C;_tmp6.f3=_tmp2B;_tmp6.f4=_tmp2A;_tmp6;});_tmp5[0]=_tmp3E4;});_tmp5;});p->r=_tmp3E5;});
Cyc_Tcpat_resolve_pat(te,topt,p);
goto _LLF;}else{_LL12: _LL13:
# 57
({struct Cyc_String_pa_PrintArg_struct _tmpA;_tmpA.tag=0U;({struct _dyneither_ptr _tmp3E6=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t));_tmpA.f1=_tmp3E6;});({void*_tmp8[1U]={& _tmpA};({unsigned int _tmp3E8=p->loc;struct _dyneither_ptr _tmp3E7=({const char*_tmp9="pattern expects aggregate type instead of %s";_tag_dyneither(_tmp9,sizeof(char),45U);});Cyc_Tcutil_terr(_tmp3E8,_tmp3E7,_tag_dyneither(_tmp8,sizeof(void*),1U));});});});
goto _LLF;}_LLF:;}
# 60
return;};}else{if(((((struct Cyc_Absyn_AggrInfo*)((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp1)->f1)->aggr_info).KnownAggr).tag == 2){_LL3: _tmp31=(((((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp1)->f1)->aggr_info).KnownAggr).val;_tmp30=(((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp1)->f1)->targs;_tmp2F=(struct Cyc_List_List**)&((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp1)->f2;_tmp2E=((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp1)->f3;_tmp2D=((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp1)->f4;_LL4: {
# 63
struct Cyc_Absyn_Aggrdecl*_tmpC=*_tmp31;
if(_tmpC->impl == 0){
({struct Cyc_String_pa_PrintArg_struct _tmpF;_tmpF.tag=0U;({struct _dyneither_ptr _tmp3E9=(struct _dyneither_ptr)(
_tmpC->kind == Cyc_Absyn_StructA?({const char*_tmp10="struct";_tag_dyneither(_tmp10,sizeof(char),7U);}):({const char*_tmp11="union";_tag_dyneither(_tmp11,sizeof(char),6U);}));_tmpF.f1=_tmp3E9;});({void*_tmpD[1U]={& _tmpF};({unsigned int _tmp3EB=p->loc;struct _dyneither_ptr _tmp3EA=({const char*_tmpE="can't destructure an abstract %s";_tag_dyneither(_tmpE,sizeof(char),33U);});Cyc_Tcutil_terr(_tmp3EB,_tmp3EA,_tag_dyneither(_tmpD,sizeof(void*),1U));});});});
p->r=(void*)& Cyc_Absyn_Wild_p_val;
return;}{
# 70
int more_exists=({int _tmp3EC=((int(*)(struct Cyc_List_List*x))Cyc_List_length)(((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmpC->impl))->exist_vars);_tmp3EC - ((int(*)(struct Cyc_List_List*x))Cyc_List_length)(*_tmp2F);});
if(more_exists < 0){
({void*_tmp12=0U;({unsigned int _tmp3EE=p->loc;struct _dyneither_ptr _tmp3ED=({const char*_tmp13="too many existentially bound type variables in pattern";_tag_dyneither(_tmp13,sizeof(char),55U);});Cyc_Tcutil_terr(_tmp3EE,_tmp3ED,_tag_dyneither(_tmp12,sizeof(void*),0U));});});{
struct Cyc_List_List**_tmp14=_tmp2F;
{int n=((int(*)(struct Cyc_List_List*x))Cyc_List_length)(((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmpC->impl))->exist_vars);for(0;n != 0;-- n){
_tmp14=&((struct Cyc_List_List*)_check_null(*_tmp14))->tl;}}
*_tmp14=0;};}else{
if(more_exists > 0){
# 79
struct Cyc_List_List*_tmp15=0;
for(0;more_exists != 0;-- more_exists){
({struct Cyc_List_List*_tmp3F1=({struct Cyc_List_List*_tmp16=_cycalloc(sizeof(*_tmp16));({struct Cyc_Absyn_Tvar*_tmp3F0=Cyc_Tcutil_new_tvar((void*)({struct Cyc_Absyn_Unknown_kb_Absyn_KindBound_struct*_tmp17=_cycalloc(sizeof(*_tmp17));({struct Cyc_Absyn_Unknown_kb_Absyn_KindBound_struct _tmp3EF=({struct Cyc_Absyn_Unknown_kb_Absyn_KindBound_struct _tmp18;_tmp18.tag=1U;_tmp18.f1=0;_tmp18;});_tmp17[0]=_tmp3EF;});_tmp17;}));_tmp16->hd=_tmp3F0;});_tmp16->tl=_tmp15;_tmp16;});_tmp15=_tmp3F1;});}
({struct Cyc_List_List*_tmp3F2=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_imp_append)(*_tmp2F,_tmp15);*_tmp2F=_tmp3F2;});}}
# 84
return;};}}else{_LLB: _LLC:
# 97
(int)_throw((void*)({struct Cyc_Core_Impossible_exn_struct*_tmp27=_cycalloc(sizeof(*_tmp27));({struct Cyc_Core_Impossible_exn_struct _tmp3F4=({struct Cyc_Core_Impossible_exn_struct _tmp28;_tmp28.tag=Cyc_Core_Impossible;({struct _dyneither_ptr _tmp3F3=({const char*_tmp29="resolve_pat unknownAggr";_tag_dyneither(_tmp29,sizeof(char),24U);});_tmp28.f1=_tmp3F3;});_tmp28;});_tmp27[0]=_tmp3F4;});_tmp27;}));}}case 17U: _LL5: _tmp32=((struct Cyc_Absyn_Exp_p_Absyn_Raw_pat_struct*)_tmp1)->f1;_LL6:
# 86
 Cyc_Tcexp_tcExp(te,0,_tmp32);
if(!Cyc_Tcutil_is_const_exp(_tmp32)){
({void*_tmp19=0U;({unsigned int _tmp3F6=p->loc;struct _dyneither_ptr _tmp3F5=({const char*_tmp1A="non-constant expression in case pattern";_tag_dyneither(_tmp1A,sizeof(char),40U);});Cyc_Tcutil_terr(_tmp3F6,_tmp3F5,_tag_dyneither(_tmp19,sizeof(void*),0U));});});
p->r=(void*)& Cyc_Absyn_Wild_p_val;}{
# 91
struct _tuple14 _tmp1B=Cyc_Evexp_eval_const_uint_exp(_tmp32);struct _tuple14 _tmp1C=_tmp1B;unsigned int _tmp20;int _tmp1F;_LL15: _tmp20=_tmp1C.f1;_tmp1F=_tmp1C.f2;_LL16:;
({void*_tmp3F8=(void*)({struct Cyc_Absyn_Int_p_Absyn_Raw_pat_struct*_tmp1D=_cycalloc_atomic(sizeof(*_tmp1D));({struct Cyc_Absyn_Int_p_Absyn_Raw_pat_struct _tmp3F7=({struct Cyc_Absyn_Int_p_Absyn_Raw_pat_struct _tmp1E;_tmp1E.tag=10U;_tmp1E.f1=Cyc_Absyn_None;_tmp1E.f2=(int)_tmp20;_tmp1E;});_tmp1D[0]=_tmp3F7;});_tmp1D;});p->r=_tmp3F8;});
return;};case 15U: _LL7: _LL8:
# 95
(int)_throw((void*)({struct Cyc_Core_Impossible_exn_struct*_tmp21=_cycalloc(sizeof(*_tmp21));({struct Cyc_Core_Impossible_exn_struct _tmp3FA=({struct Cyc_Core_Impossible_exn_struct _tmp22;_tmp22.tag=Cyc_Core_Impossible;({struct _dyneither_ptr _tmp3F9=({const char*_tmp23="resolve_pat UnknownId_p";_tag_dyneither(_tmp23,sizeof(char),24U);});_tmp22.f1=_tmp3F9;});_tmp22;});_tmp21[0]=_tmp3FA;});_tmp21;}));case 16U: _LL9: _LLA:
(int)_throw((void*)({struct Cyc_Core_Impossible_exn_struct*_tmp24=_cycalloc(sizeof(*_tmp24));({struct Cyc_Core_Impossible_exn_struct _tmp3FC=({struct Cyc_Core_Impossible_exn_struct _tmp25;_tmp25.tag=Cyc_Core_Impossible;({struct _dyneither_ptr _tmp3FB=({const char*_tmp26="resolve_pat UnknownCall_p";_tag_dyneither(_tmp26,sizeof(char),26U);});_tmp25.f1=_tmp3FB;});_tmp25;});_tmp24[0]=_tmp3FC;});_tmp24;}));default: _LLD: _LLE:
# 99
 return;}_LL0:;}
# 103
static struct _dyneither_ptr*Cyc_Tcpat_get_name(struct Cyc_Absyn_Vardecl*vd){
return(*vd->name).f2;}
# 106
static void*Cyc_Tcpat_any_type(struct Cyc_List_List*s,void**topt){
if(topt != 0)
return*topt;
return Cyc_Absyn_new_evar(& Cyc_Tcutil_mko,({struct Cyc_Core_Opt*_tmp33=_cycalloc(sizeof(*_tmp33));_tmp33->v=s;_tmp33;}));}
# 111
static void*Cyc_Tcpat_num_type(void**topt,void*numt){
# 115
if(topt != 0  && Cyc_Tcutil_coerceable(*topt))
return*topt;
# 118
{void*_tmp34=Cyc_Tcutil_compress(numt);void*_tmp35=_tmp34;switch(*((int*)_tmp35)){case 13U: _LL1: _LL2:
 goto _LL4;case 14U: _LL3: _LL4:
 if(topt != 0)return*topt;goto _LL0;default: _LL5: _LL6:
 goto _LL0;}_LL0:;}
# 123
return numt;}struct _tuple15{struct Cyc_Absyn_Vardecl**f1;struct Cyc_Absyn_Exp*f2;};
# 126
static void Cyc_Tcpat_set_vd(struct Cyc_Absyn_Vardecl**vd,struct Cyc_Absyn_Exp*e,struct Cyc_List_List**v_result_ptr,void*t){
# 128
if(vd != 0){
(*vd)->type=t;
({struct Cyc_Absyn_Tqual _tmp3FD=Cyc_Absyn_empty_tqual(0U);(*vd)->tq=_tmp3FD;});}
# 132
({struct Cyc_List_List*_tmp3FF=({struct Cyc_List_List*_tmp36=_cycalloc(sizeof(*_tmp36));({struct _tuple15*_tmp3FE=({struct _tuple15*_tmp37=_cycalloc(sizeof(*_tmp37));_tmp37->f1=vd;_tmp37->f2=e;_tmp37;});_tmp36->hd=_tmp3FE;});_tmp36->tl=*v_result_ptr;_tmp36;});*v_result_ptr=_tmp3FF;});}
# 134
static struct Cyc_Tcpat_TcPatResult Cyc_Tcpat_combine_results(struct Cyc_Tcpat_TcPatResult res1,struct Cyc_Tcpat_TcPatResult res2){
# 136
struct Cyc_Tcpat_TcPatResult _tmp38=res1;struct _tuple1*_tmp42;struct Cyc_List_List*_tmp41;_LL1: _tmp42=_tmp38.tvars_and_bounds_opt;_tmp41=_tmp38.patvars;_LL2:;{
struct Cyc_Tcpat_TcPatResult _tmp39=res2;struct _tuple1*_tmp40;struct Cyc_List_List*_tmp3F;_LL4: _tmp40=_tmp39.tvars_and_bounds_opt;_tmp3F=_tmp39.patvars;_LL5:;
if(_tmp42 != 0  || _tmp40 != 0){
if(_tmp42 == 0)
({struct _tuple1*_tmp400=({struct _tuple1*_tmp3A=_cycalloc(sizeof(*_tmp3A));_tmp3A->f1=0;_tmp3A->f2=0;_tmp3A;});_tmp42=_tmp400;});
if(_tmp40 == 0)
({struct _tuple1*_tmp401=({struct _tuple1*_tmp3B=_cycalloc(sizeof(*_tmp3B));_tmp3B->f1=0;_tmp3B->f2=0;_tmp3B;});_tmp40=_tmp401;});
return({struct Cyc_Tcpat_TcPatResult _tmp3C;({struct _tuple1*_tmp404=({struct _tuple1*_tmp3D=_cycalloc(sizeof(*_tmp3D));({struct Cyc_List_List*_tmp402=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_append)((*_tmp42).f1,(*_tmp40).f1);_tmp3D->f1=_tmp402;});({struct Cyc_List_List*_tmp403=
((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_append)((*_tmp42).f2,(*_tmp40).f2);_tmp3D->f2=_tmp403;});_tmp3D;});_tmp3C.tvars_and_bounds_opt=_tmp404;});({struct Cyc_List_List*_tmp405=
((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_append)(_tmp41,_tmp3F);_tmp3C.patvars=_tmp405;});_tmp3C;});}
# 147
return({struct Cyc_Tcpat_TcPatResult _tmp3E;_tmp3E.tvars_and_bounds_opt=0;({struct Cyc_List_List*_tmp406=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_append)(_tmp41,_tmp3F);_tmp3E.patvars=_tmp406;});_tmp3E;});};}
# 150
static struct Cyc_Absyn_Pat*Cyc_Tcpat_wild_pat(unsigned int loc){
return({struct Cyc_Absyn_Pat*_tmp43=_cycalloc(sizeof(*_tmp43));_tmp43->loc=loc;_tmp43->topt=0;_tmp43->r=(void*)& Cyc_Absyn_Wild_p_val;_tmp43;});}
# 155
static void*Cyc_Tcpat_pat_promote_array(struct Cyc_Tcenv_Tenv*te,void*t,void*rgn_opt){
return Cyc_Tcutil_is_array(t)?({
void*_tmp408=t;Cyc_Tcutil_promote_array(_tmp408,rgn_opt == 0?Cyc_Absyn_new_evar(& Cyc_Tcutil_rko,({struct Cyc_Core_Opt*_tmp44=_cycalloc(sizeof(*_tmp44));({struct Cyc_List_List*_tmp407=Cyc_Tcenv_lookup_type_vars(te);_tmp44->v=_tmp407;});_tmp44;})): rgn_opt,0);}): t;}struct _tuple16{struct Cyc_Absyn_Tvar*f1;int f2;};
# 162
static struct _tuple16*Cyc_Tcpat_add_false(struct Cyc_Absyn_Tvar*tv){
return({struct _tuple16*_tmp45=_cycalloc(sizeof(*_tmp45));_tmp45->f1=tv;_tmp45->f2=0;_tmp45;});}struct _tuple17{struct Cyc_Absyn_Tqual f1;void*f2;};struct _tuple18{struct Cyc_List_List*f1;struct Cyc_Absyn_Pat*f2;};struct _tuple19{struct Cyc_Absyn_Aggrfield*f1;struct Cyc_Absyn_Pat*f2;};
# 166
static struct Cyc_Tcpat_TcPatResult Cyc_Tcpat_tcPatRec(struct Cyc_Tcenv_Tenv*te,struct Cyc_Absyn_Pat*p,void**topt,void*rgn_pat,int allow_ref_pat,struct Cyc_Absyn_Exp*access_exp){
# 169
Cyc_Tcpat_resolve_pat(te,topt,p);{
void*t;
struct Cyc_Tcpat_TcPatResult res=({struct Cyc_Tcpat_TcPatResult _tmp118;_tmp118.tvars_and_bounds_opt=0;_tmp118.patvars=0;_tmp118;});
# 174
{void*_tmp46=p->r;void*_tmp47=_tmp46;struct Cyc_Absyn_Datatypedecl*_tmp117;struct Cyc_Absyn_Datatypefield*_tmp116;struct Cyc_List_List**_tmp115;int _tmp114;struct Cyc_Absyn_Aggrdecl*_tmp113;struct Cyc_List_List**_tmp112;struct Cyc_List_List*_tmp111;struct Cyc_List_List**_tmp110;int _tmp10F;struct Cyc_List_List**_tmp10E;int _tmp10D;struct Cyc_Absyn_Pat*_tmp10C;void*_tmp10B;struct Cyc_Absyn_Enumdecl*_tmp10A;int _tmp109;struct Cyc_Absyn_Tvar*_tmp108;struct Cyc_Absyn_Vardecl*_tmp107;struct Cyc_Absyn_Vardecl*_tmp106;struct Cyc_Absyn_Pat*_tmp105;struct Cyc_Absyn_Tvar*_tmp104;struct Cyc_Absyn_Vardecl*_tmp103;struct Cyc_Absyn_Vardecl*_tmp102;struct Cyc_Absyn_Pat*_tmp101;switch(*((int*)_tmp47)){case 0U: _LL1: _LL2:
# 177
 if(topt != 0)
t=*topt;else{
# 180
({void*_tmp40A=({struct Cyc_List_List*_tmp409=Cyc_Tcenv_lookup_type_vars(te);Cyc_Tcpat_any_type(_tmp409,topt);});t=_tmp40A;});}
goto _LL0;case 1U: _LL3: _tmp102=((struct Cyc_Absyn_Var_p_Absyn_Raw_pat_struct*)_tmp47)->f1;_tmp101=((struct Cyc_Absyn_Var_p_Absyn_Raw_pat_struct*)_tmp47)->f2;_LL4: {
# 184
struct _tuple2*_tmp48=_tmp102->name;struct _tuple2*_tmp49=_tmp48;struct _dyneither_ptr _tmp56;_LL2E: _tmp56=*_tmp49->f2;_LL2F:;
if(({struct _dyneither_ptr _tmp40C=(struct _dyneither_ptr)_tmp56;Cyc_strcmp(_tmp40C,({const char*_tmp4A="true";_tag_dyneither(_tmp4A,sizeof(char),5U);}));})== 0  || ({struct _dyneither_ptr _tmp40B=(struct _dyneither_ptr)_tmp56;Cyc_strcmp(_tmp40B,({const char*_tmp4B="false";_tag_dyneither(_tmp4B,sizeof(char),6U);}));})== 0)
({struct Cyc_String_pa_PrintArg_struct _tmp4E;_tmp4E.tag=0U;_tmp4E.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp56);({void*_tmp4C[1U]={& _tmp4E};({unsigned int _tmp40E=p->loc;struct _dyneither_ptr _tmp40D=({const char*_tmp4D="you probably do not want to use %s as a local variable...";_tag_dyneither(_tmp4D,sizeof(char),58U);});Cyc_Tcutil_warn(_tmp40E,_tmp40D,_tag_dyneither(_tmp4C,sizeof(void*),1U));});});});
# 188
({struct Cyc_Tcpat_TcPatResult _tmp40F=Cyc_Tcpat_tcPatRec(te,_tmp101,topt,rgn_pat,allow_ref_pat,access_exp);res=_tmp40F;});
t=(void*)_check_null(_tmp101->topt);
# 192
{void*_tmp4F=Cyc_Tcutil_compress(t);void*_tmp50=_tmp4F;if(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp50)->tag == 8U){_LL31: _LL32:
# 194
 if(rgn_pat == 0  || !allow_ref_pat)
({void*_tmp51=0U;({unsigned int _tmp411=p->loc;struct _dyneither_ptr _tmp410=({const char*_tmp52="array reference would point into unknown/unallowed region";_tag_dyneither(_tmp52,sizeof(char),58U);});Cyc_Tcutil_terr(_tmp411,_tmp410,_tag_dyneither(_tmp51,sizeof(void*),0U));});});
goto _LL30;}else{_LL33: _LL34:
# 198
 if(!Cyc_Tcutil_kind_leq(Cyc_Tcutil_typ_kind(t),& Cyc_Tcutil_tmk))
({void*_tmp53=0U;({unsigned int _tmp413=p->loc;struct _dyneither_ptr _tmp412=({const char*_tmp54="pattern would point to an abstract member";_tag_dyneither(_tmp54,sizeof(char),42U);});Cyc_Tcutil_terr(_tmp413,_tmp412,_tag_dyneither(_tmp53,sizeof(void*),0U));});});
goto _LL30;}_LL30:;}
# 202
({struct Cyc_Absyn_Vardecl**_tmp416=({struct Cyc_Absyn_Vardecl**_tmp55=_cycalloc(sizeof(*_tmp55));_tmp55[0]=_tmp102;_tmp55;});struct Cyc_Absyn_Exp*_tmp415=access_exp;struct Cyc_List_List**_tmp414=& res.patvars;Cyc_Tcpat_set_vd(_tmp416,_tmp415,_tmp414,Cyc_Tcpat_pat_promote_array(te,t,rgn_pat));});
goto _LL0;}case 2U: _LL5: _tmp104=((struct Cyc_Absyn_AliasVar_p_Absyn_Raw_pat_struct*)_tmp47)->f1;_tmp103=((struct Cyc_Absyn_AliasVar_p_Absyn_Raw_pat_struct*)_tmp47)->f2;_LL6: {
# 205
struct Cyc_Tcenv_Tenv*te2=({unsigned int _tmp418=p->loc;struct Cyc_Tcenv_Tenv*_tmp417=te;Cyc_Tcenv_add_type_vars(_tmp418,_tmp417,({struct Cyc_Absyn_Tvar*_tmp61[1U];_tmp61[0U]=_tmp104;((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp61,sizeof(struct Cyc_Absyn_Tvar*),1U));}));});
if(res.tvars_and_bounds_opt == 0)
({struct _tuple1*_tmp419=({struct _tuple1*_tmp57=_cycalloc(sizeof(*_tmp57));_tmp57->f1=0;_tmp57->f2=0;_tmp57;});res.tvars_and_bounds_opt=_tmp419;});
({struct Cyc_List_List*_tmp41C=({
struct Cyc_List_List*_tmp41B=(*res.tvars_and_bounds_opt).f1;((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_append)(_tmp41B,({struct Cyc_List_List*_tmp58=_cycalloc(sizeof(*_tmp58));({struct _tuple16*_tmp41A=({struct _tuple16*_tmp59=_cycalloc(sizeof(*_tmp59));_tmp59->f1=_tmp104;_tmp59->f2=1;_tmp59;});_tmp58->hd=_tmp41A;});_tmp58->tl=0;_tmp58;}));});
# 208
(*res.tvars_and_bounds_opt).f1=_tmp41C;});
# 210
({unsigned int _tmp41F=p->loc;struct Cyc_Tcenv_Tenv*_tmp41E=te2;struct Cyc_List_List*_tmp41D=Cyc_Tcenv_lookup_type_vars(te2);Cyc_Tcutil_check_type(_tmp41F,_tmp41E,_tmp41D,& Cyc_Tcutil_tmk,1,0,_tmp103->type);});
# 213
if(topt != 0)t=*topt;else{
({void*_tmp421=({struct Cyc_List_List*_tmp420=Cyc_Tcenv_lookup_type_vars(te);Cyc_Tcpat_any_type(_tmp420,topt);});t=_tmp421;});}
{void*_tmp5A=Cyc_Tcutil_compress(t);void*_tmp5B=_tmp5A;if(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp5B)->tag == 8U){_LL36: _LL37:
# 217
 if(rgn_pat == 0  || !allow_ref_pat)
({void*_tmp5C=0U;({unsigned int _tmp423=p->loc;struct _dyneither_ptr _tmp422=({const char*_tmp5D="array reference would point into unknown/unallowed region";_tag_dyneither(_tmp5D,sizeof(char),58U);});Cyc_Tcutil_terr(_tmp423,_tmp422,_tag_dyneither(_tmp5C,sizeof(void*),0U));});});
goto _LL35;}else{_LL38: _LL39:
# 221
 if(!Cyc_Tcutil_kind_leq(Cyc_Tcutil_typ_kind(t),& Cyc_Tcutil_tmk))
({void*_tmp5E=0U;({unsigned int _tmp425=p->loc;struct _dyneither_ptr _tmp424=({const char*_tmp5F="pattern would point to an abstract member";_tag_dyneither(_tmp5F,sizeof(char),42U);});Cyc_Tcutil_terr(_tmp425,_tmp424,_tag_dyneither(_tmp5E,sizeof(void*),0U));});});
goto _LL35;}_LL35:;}
# 225
({struct Cyc_Absyn_Vardecl**_tmp428=({struct Cyc_Absyn_Vardecl**_tmp60=_cycalloc(sizeof(*_tmp60));_tmp60[0]=_tmp103;_tmp60;});struct Cyc_Absyn_Exp*_tmp427=access_exp;struct Cyc_List_List**_tmp426=& res.patvars;Cyc_Tcpat_set_vd(_tmp428,_tmp427,_tmp426,_tmp103->type);});
goto _LL0;}case 3U: _LL7: _tmp106=((struct Cyc_Absyn_Reference_p_Absyn_Raw_pat_struct*)_tmp47)->f1;_tmp105=((struct Cyc_Absyn_Reference_p_Absyn_Raw_pat_struct*)_tmp47)->f2;_LL8:
# 229
({struct Cyc_Tcpat_TcPatResult _tmp429=Cyc_Tcpat_tcPatRec(te,_tmp105,topt,rgn_pat,allow_ref_pat,access_exp);res=_tmp429;});
t=(void*)_check_null(_tmp105->topt);
if(!allow_ref_pat  || rgn_pat == 0){
({void*_tmp62=0U;({unsigned int _tmp42B=p->loc;struct _dyneither_ptr _tmp42A=({const char*_tmp63="* pattern would point into an unknown/unallowed region";_tag_dyneither(_tmp63,sizeof(char),55U);});Cyc_Tcutil_terr(_tmp42B,_tmp42A,_tag_dyneither(_tmp62,sizeof(void*),0U));});});
goto _LL0;}else{
# 236
if(Cyc_Tcutil_is_noalias_pointer_or_aggr(t))
({void*_tmp64=0U;({unsigned int _tmp42D=p->loc;struct _dyneither_ptr _tmp42C=({const char*_tmp65="* pattern cannot take the address of an alias-free path";_tag_dyneither(_tmp65,sizeof(char),56U);});Cyc_Tcutil_terr(_tmp42D,_tmp42C,_tag_dyneither(_tmp64,sizeof(void*),0U));});});}{
# 239
struct Cyc_Absyn_Exp*new_access_exp=0;
void*t2=(void*)({struct Cyc_Absyn_PointerType_Absyn_Type_struct*_tmp67=_cycalloc(sizeof(*_tmp67));({struct Cyc_Absyn_PointerType_Absyn_Type_struct _tmp433=({struct Cyc_Absyn_PointerType_Absyn_Type_struct _tmp68;_tmp68.tag=5U;({struct Cyc_Absyn_PtrInfo _tmp432=({struct Cyc_Absyn_PtrInfo _tmp69;_tmp69.elt_typ=t;({struct Cyc_Absyn_Tqual _tmp42E=Cyc_Absyn_empty_tqual(0U);_tmp69.elt_tq=_tmp42E;});({struct Cyc_Absyn_PtrAtts _tmp431=({(_tmp69.ptr_atts).rgn=rgn_pat;(_tmp69.ptr_atts).nullable=Cyc_Absyn_false_conref;({union Cyc_Absyn_Constraint*_tmp42F=
# 242
((union Cyc_Absyn_Constraint*(*)())Cyc_Absyn_empty_conref)();(_tmp69.ptr_atts).bounds=_tmp42F;});({union Cyc_Absyn_Constraint*_tmp430=((union Cyc_Absyn_Constraint*(*)())Cyc_Absyn_empty_conref)();(_tmp69.ptr_atts).zero_term=_tmp430;});(_tmp69.ptr_atts).ptrloc=0;_tmp69.ptr_atts;});_tmp69.ptr_atts=_tmp431;});_tmp69;});_tmp68.f1=_tmp432;});_tmp68;});_tmp67[0]=_tmp433;});_tmp67;});
# 244
if((unsigned int)access_exp){
({struct Cyc_Absyn_Exp*_tmp434=Cyc_Absyn_address_exp(access_exp,0U);new_access_exp=_tmp434;});
new_access_exp->topt=t2;}
# 248
({struct Cyc_Absyn_Vardecl**_tmp437=({struct Cyc_Absyn_Vardecl**_tmp66=_cycalloc(sizeof(*_tmp66));_tmp66[0]=_tmp106;_tmp66;});struct Cyc_Absyn_Exp*_tmp436=new_access_exp;struct Cyc_List_List**_tmp435=& res.patvars;Cyc_Tcpat_set_vd(_tmp437,_tmp436,_tmp435,t2);});
goto _LL0;};case 4U: _LL9: _tmp108=((struct Cyc_Absyn_TagInt_p_Absyn_Raw_pat_struct*)_tmp47)->f1;_tmp107=((struct Cyc_Absyn_TagInt_p_Absyn_Raw_pat_struct*)_tmp47)->f2;_LLA:
# 254
({struct Cyc_Absyn_Vardecl**_tmp43A=({struct Cyc_Absyn_Vardecl**_tmp6A=_cycalloc(sizeof(*_tmp6A));_tmp6A[0]=_tmp107;_tmp6A;});struct Cyc_Absyn_Exp*_tmp439=access_exp;struct Cyc_List_List**_tmp438=& res.patvars;Cyc_Tcpat_set_vd(_tmp43A,_tmp439,_tmp438,_tmp107->type);});
# 258
({unsigned int _tmp43C=p->loc;struct Cyc_Tcenv_Tenv*_tmp43B=te;Cyc_Tcenv_add_type_vars(_tmp43C,_tmp43B,({struct Cyc_Absyn_Tvar*_tmp6B[1U];_tmp6B[0U]=_tmp108;((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp6B,sizeof(struct Cyc_Absyn_Tvar*),1U));}));});
if(res.tvars_and_bounds_opt == 0)
({struct _tuple1*_tmp43D=({struct _tuple1*_tmp6C=_cycalloc(sizeof(*_tmp6C));_tmp6C->f1=0;_tmp6C->f2=0;_tmp6C;});res.tvars_and_bounds_opt=_tmp43D;});
({struct Cyc_List_List*_tmp43F=({struct Cyc_List_List*_tmp6D=_cycalloc(sizeof(*_tmp6D));({struct _tuple16*_tmp43E=({struct _tuple16*_tmp6E=_cycalloc(sizeof(*_tmp6E));_tmp6E->f1=_tmp108;_tmp6E->f2=0;_tmp6E;});_tmp6D->hd=_tmp43E;});_tmp6D->tl=(*res.tvars_and_bounds_opt).f1;_tmp6D;});(*res.tvars_and_bounds_opt).f1=_tmp43F;});
# 263
t=Cyc_Absyn_uint_typ;
goto _LL0;case 10U: switch(((struct Cyc_Absyn_Int_p_Absyn_Raw_pat_struct*)_tmp47)->f1){case Cyc_Absyn_Unsigned: _LLB: _LLC:
# 266
({void*_tmp440=Cyc_Tcpat_num_type(topt,Cyc_Absyn_uint_typ);t=_tmp440;});goto _LL0;case Cyc_Absyn_None: _LLD: _LLE:
 goto _LL10;default: _LLF: _LL10:
({void*_tmp441=Cyc_Tcpat_num_type(topt,Cyc_Absyn_sint_typ);t=_tmp441;});goto _LL0;}case 11U: _LL11: _LL12:
({void*_tmp442=Cyc_Tcpat_num_type(topt,Cyc_Absyn_char_typ);t=_tmp442;});goto _LL0;case 12U: _LL13: _tmp109=((struct Cyc_Absyn_Float_p_Absyn_Raw_pat_struct*)_tmp47)->f2;_LL14:
({void*_tmp444=({void**_tmp443=topt;Cyc_Tcpat_num_type(_tmp443,Cyc_Absyn_float_typ(_tmp109));});t=_tmp444;});goto _LL0;case 13U: _LL15: _tmp10A=((struct Cyc_Absyn_Enum_p_Absyn_Raw_pat_struct*)_tmp47)->f1;_LL16:
# 272
({void*_tmp447=({void**_tmp446=topt;Cyc_Tcpat_num_type(_tmp446,(void*)({struct Cyc_Absyn_EnumType_Absyn_Type_struct*_tmp6F=_cycalloc(sizeof(*_tmp6F));({struct Cyc_Absyn_EnumType_Absyn_Type_struct _tmp445=({struct Cyc_Absyn_EnumType_Absyn_Type_struct _tmp70;_tmp70.tag=13U;_tmp70.f1=_tmp10A->name;_tmp70.f2=_tmp10A;_tmp70;});_tmp6F[0]=_tmp445;});_tmp6F;}));});t=_tmp447;});
goto _LL0;case 14U: _LL17: _tmp10B=(void*)((struct Cyc_Absyn_AnonEnum_p_Absyn_Raw_pat_struct*)_tmp47)->f1;_LL18:
({void*_tmp448=Cyc_Tcpat_num_type(topt,_tmp10B);t=_tmp448;});goto _LL0;case 9U: _LL19: _LL1A:
# 276
 if(topt != 0){
void*_tmp71=Cyc_Tcutil_compress(*topt);void*_tmp72=_tmp71;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp72)->tag == 5U){_LL3B: _LL3C:
 t=*topt;goto tcpat_end;}else{_LL3D: _LL3E:
 goto _LL3A;}_LL3A:;}{
# 281
struct Cyc_Core_Opt*_tmp73=Cyc_Tcenv_lookup_opt_type_vars(te);
({void*_tmp451=(void*)({struct Cyc_Absyn_PointerType_Absyn_Type_struct*_tmp74=_cycalloc(sizeof(*_tmp74));({struct Cyc_Absyn_PointerType_Absyn_Type_struct _tmp450=({struct Cyc_Absyn_PointerType_Absyn_Type_struct _tmp75;_tmp75.tag=5U;({struct Cyc_Absyn_PtrInfo _tmp44F=({struct Cyc_Absyn_PtrInfo _tmp76;({void*_tmp449=Cyc_Absyn_new_evar(& Cyc_Tcutil_ako,_tmp73);_tmp76.elt_typ=_tmp449;});({struct Cyc_Absyn_Tqual _tmp44A=
Cyc_Absyn_empty_tqual(0U);_tmp76.elt_tq=_tmp44A;});({struct Cyc_Absyn_PtrAtts _tmp44E=({({void*_tmp44B=
Cyc_Absyn_new_evar(& Cyc_Tcutil_trko,_tmp73);(_tmp76.ptr_atts).rgn=_tmp44B;});(_tmp76.ptr_atts).nullable=Cyc_Absyn_true_conref;({union Cyc_Absyn_Constraint*_tmp44C=
# 286
((union Cyc_Absyn_Constraint*(*)())Cyc_Absyn_empty_conref)();(_tmp76.ptr_atts).bounds=_tmp44C;});({union Cyc_Absyn_Constraint*_tmp44D=((union Cyc_Absyn_Constraint*(*)())Cyc_Absyn_empty_conref)();(_tmp76.ptr_atts).zero_term=_tmp44D;});(_tmp76.ptr_atts).ptrloc=0;_tmp76.ptr_atts;});_tmp76.ptr_atts=_tmp44E;});_tmp76;});_tmp75.f1=_tmp44F;});_tmp75;});_tmp74[0]=_tmp450;});_tmp74;});
# 282
t=_tmp451;});
# 287
goto _LL0;};case 6U: _LL1B: _tmp10C=((struct Cyc_Absyn_Pointer_p_Absyn_Raw_pat_struct*)_tmp47)->f1;_LL1C: {
# 292
void*inner_typ=(void*)& Cyc_Absyn_VoidType_val;
void**_tmp77=0;
int elt_const=0;
if(topt != 0){
void*_tmp78=Cyc_Tcutil_compress(*topt);void*_tmp79=_tmp78;void*_tmp7B;struct Cyc_Absyn_Tqual _tmp7A;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp79)->tag == 5U){_LL40: _tmp7B=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp79)->f1).elt_typ;_tmp7A=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp79)->f1).elt_tq;_LL41:
# 298
 inner_typ=_tmp7B;
_tmp77=& inner_typ;
elt_const=_tmp7A.real_const;
goto _LL3F;}else{_LL42: _LL43:
 goto _LL3F;}_LL3F:;}{
# 307
void*ptr_rgn=Cyc_Absyn_new_evar(& Cyc_Tcutil_trko,Cyc_Tcenv_lookup_opt_type_vars(te));
struct Cyc_Absyn_Exp*new_access_exp=0;
if((unsigned int)access_exp)({struct Cyc_Absyn_Exp*_tmp452=Cyc_Absyn_deref_exp(access_exp,0U);new_access_exp=_tmp452;});
({struct Cyc_Tcpat_TcPatResult _tmp454=({struct Cyc_Tcpat_TcPatResult _tmp453=res;Cyc_Tcpat_combine_results(_tmp453,Cyc_Tcpat_tcPatRec(te,_tmp10C,_tmp77,ptr_rgn,1,new_access_exp));});res=_tmp454;});
# 316
{void*_tmp7C=Cyc_Tcutil_compress((void*)_check_null(_tmp10C->topt));void*_tmp7D=_tmp7C;struct Cyc_Absyn_Datatypedecl*_tmp8C;struct Cyc_Absyn_Datatypefield*_tmp8B;struct Cyc_List_List*_tmp8A;if(((struct Cyc_Absyn_DatatypeFieldType_Absyn_Type_struct*)_tmp7D)->tag == 4U){if((((((struct Cyc_Absyn_DatatypeFieldType_Absyn_Type_struct*)_tmp7D)->f1).field_info).KnownDatatypefield).tag == 2){_LL45: _tmp8C=((((((struct Cyc_Absyn_DatatypeFieldType_Absyn_Type_struct*)_tmp7D)->f1).field_info).KnownDatatypefield).val).f1;_tmp8B=((((((struct Cyc_Absyn_DatatypeFieldType_Absyn_Type_struct*)_tmp7D)->f1).field_info).KnownDatatypefield).val).f2;_tmp8A=(((struct Cyc_Absyn_DatatypeFieldType_Absyn_Type_struct*)_tmp7D)->f1).targs;_LL46:
# 320
{void*_tmp7E=Cyc_Tcutil_compress(inner_typ);void*_tmp7F=_tmp7E;if(((struct Cyc_Absyn_DatatypeFieldType_Absyn_Type_struct*)_tmp7F)->tag == 4U){_LL4A: _LL4B:
# 322
 goto DONT_PROMOTE;}else{_LL4C: _LL4D:
 goto _LL49;}_LL49:;}{
# 326
void*new_type=(void*)({struct Cyc_Absyn_DatatypeType_Absyn_Type_struct*_tmp83=_cycalloc(sizeof(*_tmp83));({struct Cyc_Absyn_DatatypeType_Absyn_Type_struct _tmp457=({struct Cyc_Absyn_DatatypeType_Absyn_Type_struct _tmp84;_tmp84.tag=3U;({struct Cyc_Absyn_DatatypeInfo _tmp456=({struct Cyc_Absyn_DatatypeInfo _tmp85;({union Cyc_Absyn_DatatypeInfoU _tmp455=Cyc_Absyn_KnownDatatype(({struct Cyc_Absyn_Datatypedecl**_tmp86=_cycalloc(sizeof(*_tmp86));_tmp86[0]=_tmp8C;_tmp86;}));_tmp85.datatype_info=_tmp455;});_tmp85.targs=_tmp8A;_tmp85;});_tmp84.f1=_tmp456;});_tmp84;});_tmp83[0]=_tmp457;});_tmp83;});
# 329
_tmp10C->topt=new_type;
({void*_tmp45D=(void*)({struct Cyc_Absyn_PointerType_Absyn_Type_struct*_tmp80=_cycalloc(sizeof(*_tmp80));({struct Cyc_Absyn_PointerType_Absyn_Type_struct _tmp45C=({struct Cyc_Absyn_PointerType_Absyn_Type_struct _tmp81;_tmp81.tag=5U;({struct Cyc_Absyn_PtrInfo _tmp45B=({struct Cyc_Absyn_PtrInfo _tmp82;_tmp82.elt_typ=new_type;({struct Cyc_Absyn_Tqual _tmp458=
elt_const?Cyc_Absyn_const_tqual(0U):
 Cyc_Absyn_empty_tqual(0U);_tmp82.elt_tq=_tmp458;});({struct Cyc_Absyn_PtrAtts _tmp45A=({(_tmp82.ptr_atts).rgn=ptr_rgn;({union Cyc_Absyn_Constraint*_tmp459=
((union Cyc_Absyn_Constraint*(*)())Cyc_Absyn_empty_conref)();(_tmp82.ptr_atts).nullable=_tmp459;});(_tmp82.ptr_atts).bounds=Cyc_Absyn_bounds_one_conref;(_tmp82.ptr_atts).zero_term=Cyc_Absyn_false_conref;(_tmp82.ptr_atts).ptrloc=0;_tmp82.ptr_atts;});_tmp82.ptr_atts=_tmp45A;});_tmp82;});_tmp81.f1=_tmp45B;});_tmp81;});_tmp80[0]=_tmp45C;});_tmp80;});
# 330
t=_tmp45D;});
# 336
goto _LL44;};}else{goto _LL47;}}else{_LL47: _LL48:
# 338
 DONT_PROMOTE:
({void*_tmp465=(void*)({struct Cyc_Absyn_PointerType_Absyn_Type_struct*_tmp87=_cycalloc(sizeof(*_tmp87));({struct Cyc_Absyn_PointerType_Absyn_Type_struct _tmp464=({struct Cyc_Absyn_PointerType_Absyn_Type_struct _tmp88;_tmp88.tag=5U;({struct Cyc_Absyn_PtrInfo _tmp463=({struct Cyc_Absyn_PtrInfo _tmp89;_tmp89.elt_typ=(void*)_check_null(_tmp10C->topt);({struct Cyc_Absyn_Tqual _tmp45E=
elt_const?Cyc_Absyn_const_tqual(0U):
 Cyc_Absyn_empty_tqual(0U);_tmp89.elt_tq=_tmp45E;});({struct Cyc_Absyn_PtrAtts _tmp462=({(_tmp89.ptr_atts).rgn=ptr_rgn;({union Cyc_Absyn_Constraint*_tmp45F=
((union Cyc_Absyn_Constraint*(*)())Cyc_Absyn_empty_conref)();(_tmp89.ptr_atts).nullable=_tmp45F;});({union Cyc_Absyn_Constraint*_tmp460=
((union Cyc_Absyn_Constraint*(*)())Cyc_Absyn_empty_conref)();(_tmp89.ptr_atts).bounds=_tmp460;});({union Cyc_Absyn_Constraint*_tmp461=((union Cyc_Absyn_Constraint*(*)())Cyc_Absyn_empty_conref)();(_tmp89.ptr_atts).zero_term=_tmp461;});(_tmp89.ptr_atts).ptrloc=0;_tmp89.ptr_atts;});_tmp89.ptr_atts=_tmp462;});_tmp89;});_tmp88.f1=_tmp463;});_tmp88;});_tmp87[0]=_tmp464;});_tmp87;});
# 339
t=_tmp465;});}_LL44:;}
# 346
if((unsigned int)new_access_exp)new_access_exp->topt=_tmp10C->topt;
goto _LL0;};}case 5U: _LL1D: _tmp10E=(struct Cyc_List_List**)&((struct Cyc_Absyn_Tuple_p_Absyn_Raw_pat_struct*)_tmp47)->f1;_tmp10D=((struct Cyc_Absyn_Tuple_p_Absyn_Raw_pat_struct*)_tmp47)->f2;_LL1E: {
# 350
struct Cyc_List_List*_tmp8D=*_tmp10E;
struct Cyc_List_List*pat_ts=0;
struct Cyc_List_List*topt_ts=0;
if(topt != 0){
void*_tmp8E=Cyc_Tcutil_compress(*topt);void*_tmp8F=_tmp8E;struct Cyc_List_List*_tmp95;if(((struct Cyc_Absyn_TupleType_Absyn_Type_struct*)_tmp8F)->tag == 10U){_LL4F: _tmp95=((struct Cyc_Absyn_TupleType_Absyn_Type_struct*)_tmp8F)->f1;_LL50:
# 356
 topt_ts=_tmp95;
if(_tmp10D){
# 359
int _tmp90=((int(*)(struct Cyc_List_List*x))Cyc_List_length)(_tmp8D);
int _tmp91=((int(*)(struct Cyc_List_List*x))Cyc_List_length)(_tmp95);
if(_tmp90 < _tmp91){
struct Cyc_List_List*wild_ps=0;
{int i=0;for(0;i < _tmp91 - _tmp90;++ i){
({struct Cyc_List_List*_tmp467=({struct Cyc_List_List*_tmp92=_cycalloc(sizeof(*_tmp92));({struct Cyc_Absyn_Pat*_tmp466=Cyc_Tcpat_wild_pat(p->loc);_tmp92->hd=_tmp466;});_tmp92->tl=wild_ps;_tmp92;});wild_ps=_tmp467;});}}
({struct Cyc_List_List*_tmp468=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_imp_append)(_tmp8D,wild_ps);*_tmp10E=_tmp468;});
_tmp8D=*_tmp10E;}else{
if(({int _tmp469=((int(*)(struct Cyc_List_List*x))Cyc_List_length)(_tmp8D);_tmp469 == ((int(*)(struct Cyc_List_List*x))Cyc_List_length)(_tmp95);}))
({void*_tmp93=0U;({unsigned int _tmp46B=p->loc;struct _dyneither_ptr _tmp46A=({const char*_tmp94="unnecessary ... in tuple pattern";_tag_dyneither(_tmp94,sizeof(char),33U);});Cyc_Tcutil_warn(_tmp46B,_tmp46A,_tag_dyneither(_tmp93,sizeof(void*),0U));});});}}
# 371
goto _LL4E;}else{_LL51: _LL52:
# 373
 goto _LL4E;}_LL4E:;}else{
# 375
if(_tmp10D)
({void*_tmp96=0U;({unsigned int _tmp46D=p->loc;struct _dyneither_ptr _tmp46C=({const char*_tmp97="cannot determine missing fields for ... in tuple pattern";_tag_dyneither(_tmp97,sizeof(char),57U);});Cyc_Tcutil_terr(_tmp46D,_tmp46C,_tag_dyneither(_tmp96,sizeof(void*),0U));});});}
{int i=0;for(0;_tmp8D != 0;(_tmp8D=_tmp8D->tl,i ++)){
void**_tmp98=0;
if(topt_ts != 0){
_tmp98=&(*((struct _tuple17*)topt_ts->hd)).f2;
topt_ts=topt_ts->tl;}{
# 383
struct Cyc_Absyn_Exp*new_access_exp=0;
if((unsigned int)access_exp)
({struct Cyc_Absyn_Exp*_tmp470=({struct Cyc_Absyn_Exp*_tmp46F=access_exp;Cyc_Absyn_subscript_exp(_tmp46F,
Cyc_Absyn_const_exp(({union Cyc_Absyn_Cnst _tmp99;({struct _tuple7 _tmp46E=({struct _tuple7 _tmp9A;_tmp9A.f1=Cyc_Absyn_Unsigned;_tmp9A.f2=i;_tmp9A;});(_tmp99.Int_c).val=_tmp46E;});(_tmp99.Int_c).tag=5;_tmp99;}),0U),0U);});
# 385
new_access_exp=_tmp470;});
# 388
({struct Cyc_Tcpat_TcPatResult _tmp472=({struct Cyc_Tcpat_TcPatResult _tmp471=res;Cyc_Tcpat_combine_results(_tmp471,Cyc_Tcpat_tcPatRec(te,(struct Cyc_Absyn_Pat*)_tmp8D->hd,_tmp98,rgn_pat,allow_ref_pat,new_access_exp));});res=_tmp472;});
# 391
if((unsigned int)new_access_exp)new_access_exp->topt=((struct Cyc_Absyn_Pat*)_tmp8D->hd)->topt;
({struct Cyc_List_List*_tmp475=({struct Cyc_List_List*_tmp9B=_cycalloc(sizeof(*_tmp9B));({struct _tuple17*_tmp474=({struct _tuple17*_tmp9C=_cycalloc(sizeof(*_tmp9C));({struct Cyc_Absyn_Tqual _tmp473=Cyc_Absyn_empty_tqual(0U);_tmp9C->f1=_tmp473;});_tmp9C->f2=(void*)_check_null(((struct Cyc_Absyn_Pat*)_tmp8D->hd)->topt);_tmp9C;});_tmp9B->hd=_tmp474;});_tmp9B->tl=pat_ts;_tmp9B;});pat_ts=_tmp475;});};}}
# 394
({void*_tmp478=(void*)({struct Cyc_Absyn_TupleType_Absyn_Type_struct*_tmp9D=_cycalloc(sizeof(*_tmp9D));({struct Cyc_Absyn_TupleType_Absyn_Type_struct _tmp477=({struct Cyc_Absyn_TupleType_Absyn_Type_struct _tmp9E;_tmp9E.tag=10U;({struct Cyc_List_List*_tmp476=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(pat_ts);_tmp9E.f1=_tmp476;});_tmp9E;});_tmp9D[0]=_tmp477;});_tmp9D;});t=_tmp478;});
goto _LL0;}case 7U: if(((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp47)->f1 != 0){if(((((struct Cyc_Absyn_AggrInfo*)((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp47)->f1)->aggr_info).KnownAggr).tag == 2){_LL1F: _tmp113=*(((((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp47)->f1)->aggr_info).KnownAggr).val;_tmp112=(struct Cyc_List_List**)&(((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp47)->f1)->targs;_tmp111=((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp47)->f2;_tmp110=(struct Cyc_List_List**)&((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp47)->f3;_tmp10F=((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp47)->f4;_LL20: {
# 398
struct Cyc_List_List*_tmp9F=*_tmp110;
struct _dyneither_ptr aggr_str=_tmp113->kind == Cyc_Absyn_StructA?({const char*_tmpE3="struct";_tag_dyneither(_tmpE3,sizeof(char),7U);}):({const char*_tmpE4="union";_tag_dyneither(_tmpE4,sizeof(char),6U);});
if(_tmp113->impl == 0){
({struct Cyc_String_pa_PrintArg_struct _tmpA2;_tmpA2.tag=0U;_tmpA2.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)aggr_str);({void*_tmpA0[1U]={& _tmpA2};({unsigned int _tmp47A=p->loc;struct _dyneither_ptr _tmp479=({const char*_tmpA1="can't destructure an abstract %s";_tag_dyneither(_tmpA1,sizeof(char),33U);});Cyc_Tcutil_terr(_tmp47A,_tmp479,_tag_dyneither(_tmpA0,sizeof(void*),1U));});});});
({void*_tmp47B=Cyc_Absyn_wildtyp(Cyc_Tcenv_lookup_opt_type_vars(te));t=_tmp47B;});
goto _LL0;}
# 407
if(_tmp113->kind == Cyc_Absyn_UnionA  && ((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp113->impl))->tagged)
allow_ref_pat=0;
if(_tmp111 != 0){
# 413
if(topt == 0  || Cyc_Tcutil_typ_kind(*topt)!= & Cyc_Tcutil_ak)
allow_ref_pat=0;}
# 416
{struct _RegionHandle _tmpA3=_new_region("rgn");struct _RegionHandle*rgn=& _tmpA3;_push_region(rgn);
# 418
{struct Cyc_List_List*_tmpA4=0;
struct Cyc_List_List*outlives_constraints=0;
struct Cyc_List_List*_tmpA5=((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp113->impl))->exist_vars;
{struct Cyc_List_List*t=_tmp111;for(0;t != 0;t=t->tl){
struct Cyc_Absyn_Tvar*tv=(struct Cyc_Absyn_Tvar*)t->hd;
struct Cyc_Absyn_Tvar*uv=(struct Cyc_Absyn_Tvar*)((struct Cyc_List_List*)_check_null(_tmpA5))->hd;
_tmpA5=_tmpA5->tl;{
void*_tmpA6=Cyc_Absyn_compress_kb(tv->kind);
void*_tmpA7=Cyc_Absyn_compress_kb(uv->kind);
int error=0;
struct Cyc_Absyn_Kind*k2;
{void*_tmpA8=_tmpA7;struct Cyc_Absyn_Kind*_tmpAC;struct Cyc_Absyn_Kind*_tmpAB;switch(*((int*)_tmpA8)){case 2U: _LL54: _tmpAB=((struct Cyc_Absyn_Less_kb_Absyn_KindBound_struct*)_tmpA8)->f2;_LL55:
 _tmpAC=_tmpAB;goto _LL57;case 0U: _LL56: _tmpAC=((struct Cyc_Absyn_Eq_kb_Absyn_KindBound_struct*)_tmpA8)->f1;_LL57:
 k2=_tmpAC;goto _LL53;default: _LL58: _LL59:
({void*_tmpA9=0U;({struct _dyneither_ptr _tmp47C=({const char*_tmpAA="unconstrained existential type variable in aggregate";_tag_dyneither(_tmpAA,sizeof(char),53U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp47C,_tag_dyneither(_tmpA9,sizeof(void*),0U));});});}_LL53:;}
# 434
{void*_tmpAD=_tmpA6;struct Cyc_Core_Opt**_tmpB2;struct Cyc_Core_Opt**_tmpB1;struct Cyc_Absyn_Kind*_tmpB0;struct Cyc_Absyn_Kind*_tmpAF;switch(*((int*)_tmpAD)){case 0U: _LL5B: _tmpAF=((struct Cyc_Absyn_Eq_kb_Absyn_KindBound_struct*)_tmpAD)->f1;_LL5C:
# 437
 if(!Cyc_Tcutil_kind_leq(k2,_tmpAF))
error=1;
goto _LL5A;case 2U: _LL5D: _tmpB1=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Less_kb_Absyn_KindBound_struct*)_tmpAD)->f1;_tmpB0=((struct Cyc_Absyn_Less_kb_Absyn_KindBound_struct*)_tmpAD)->f2;_LL5E:
 _tmpB2=_tmpB1;goto _LL60;default: _LL5F: _tmpB2=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Unknown_kb_Absyn_KindBound_struct*)_tmpAD)->f1;_LL60:
({struct Cyc_Core_Opt*_tmp47D=({struct Cyc_Core_Opt*_tmpAE=_cycalloc(sizeof(*_tmpAE));_tmpAE->v=_tmpA7;_tmpAE;});*_tmpB2=_tmp47D;});goto _LL5A;}_LL5A:;}
# 443
if(error)
({struct Cyc_String_pa_PrintArg_struct _tmpB7;_tmpB7.tag=0U;({struct _dyneither_ptr _tmp47E=(struct _dyneither_ptr)((struct _dyneither_ptr)
# 447
Cyc_Absynpp_kind2string(k2));_tmpB7.f1=_tmp47E;});({struct Cyc_String_pa_PrintArg_struct _tmpB6;_tmpB6.tag=0U;({struct _dyneither_ptr _tmp47F=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_kindbound2string(_tmpA6));_tmpB6.f1=_tmp47F;});({struct Cyc_String_pa_PrintArg_struct _tmpB5;_tmpB5.tag=0U;_tmpB5.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*tv->name);({void*_tmpB3[3U]={& _tmpB5,& _tmpB6,& _tmpB7};({unsigned int _tmp481=p->loc;struct _dyneither_ptr _tmp480=({const char*_tmpB4="type variable %s has kind %s but must have at least kind %s";_tag_dyneither(_tmpB4,sizeof(char),60U);});Cyc_Tcutil_terr(_tmp481,_tmp480,_tag_dyneither(_tmpB3,sizeof(void*),3U));});});});});});{
# 449
void*vartype=(void*)({struct Cyc_Absyn_VarType_Absyn_Type_struct*_tmpBD=_cycalloc(sizeof(*_tmpBD));({struct Cyc_Absyn_VarType_Absyn_Type_struct _tmp482=({struct Cyc_Absyn_VarType_Absyn_Type_struct _tmpBE;_tmpBE.tag=2U;_tmpBE.f1=tv;_tmpBE;});_tmpBD[0]=_tmp482;});_tmpBD;});
({struct Cyc_List_List*_tmp483=({struct Cyc_List_List*_tmpB8=_region_malloc(rgn,sizeof(*_tmpB8));_tmpB8->hd=vartype;_tmpB8->tl=_tmpA4;_tmpB8;});_tmpA4=_tmp483;});
# 453
if(k2->kind == Cyc_Absyn_RgnKind){
if(k2->aliasqual == Cyc_Absyn_Aliasable)
({struct Cyc_List_List*_tmp485=({struct Cyc_List_List*_tmpB9=_cycalloc(sizeof(*_tmpB9));({struct _tuple0*_tmp484=({struct _tuple0*_tmpBA=_cycalloc(sizeof(*_tmpBA));_tmpBA->f1=Cyc_Absyn_empty_effect;_tmpBA->f2=vartype;_tmpBA;});_tmpB9->hd=_tmp484;});_tmpB9->tl=outlives_constraints;_tmpB9;});outlives_constraints=_tmp485;});else{
# 458
({void*_tmpBB=0U;({struct _dyneither_ptr _tmp486=({const char*_tmpBC="opened existential had unique or top region kind";_tag_dyneither(_tmpBC,sizeof(char),49U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp486,_tag_dyneither(_tmpBB,sizeof(void*),0U));});});}}};};}}
# 462
({struct Cyc_List_List*_tmp487=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(_tmpA4);_tmpA4=_tmp487;});{
# 464
struct Cyc_Tcenv_Tenv*te2=Cyc_Tcenv_add_type_vars(p->loc,te,_tmp111);
# 466
struct Cyc_List_List*_tmpBF=Cyc_Tcenv_lookup_type_vars(te2);
struct _tuple12 _tmpC0=({struct _tuple12 _tmpE2;_tmpE2.f1=_tmpBF;_tmpE2.f2=rgn;_tmpE2;});
struct Cyc_List_List*_tmpC1=((struct Cyc_List_List*(*)(struct _RegionHandle*,struct _tuple13*(*f)(struct _tuple12*,struct Cyc_Absyn_Tvar*),struct _tuple12*env,struct Cyc_List_List*x))Cyc_List_rmap_c)(rgn,Cyc_Tcutil_r_make_inst_var,& _tmpC0,_tmp113->tvs);
struct Cyc_List_List*_tmpC2=((struct Cyc_List_List*(*)(struct _RegionHandle*r1,struct _RegionHandle*r2,struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_rzip)(rgn,rgn,((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp113->impl))->exist_vars,_tmpA4);
struct Cyc_List_List*_tmpC3=((struct Cyc_List_List*(*)(void*(*f)(struct _tuple13*),struct Cyc_List_List*x))Cyc_List_map)((void*(*)(struct _tuple13*))Cyc_Core_snd,_tmpC1);
struct Cyc_List_List*_tmpC4=((struct Cyc_List_List*(*)(void*(*f)(struct _tuple13*),struct Cyc_List_List*x))Cyc_List_map)((void*(*)(struct _tuple13*))Cyc_Core_snd,_tmpC2);
struct Cyc_List_List*_tmpC5=((struct Cyc_List_List*(*)(struct _RegionHandle*,struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_rappend)(rgn,_tmpC1,_tmpC2);
# 474
if(_tmp111 != 0  || ((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp113->impl))->rgn_po != 0){
if(res.tvars_and_bounds_opt == 0)
({struct _tuple1*_tmp488=({struct _tuple1*_tmpC6=_cycalloc(sizeof(*_tmpC6));_tmpC6->f1=0;_tmpC6->f2=0;_tmpC6;});res.tvars_and_bounds_opt=_tmp488;});
({struct Cyc_List_List*_tmp48A=({
struct Cyc_List_List*_tmp489=(*res.tvars_and_bounds_opt).f1;((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_append)(_tmp489,((struct Cyc_List_List*(*)(struct _tuple16*(*f)(struct Cyc_Absyn_Tvar*),struct Cyc_List_List*x))Cyc_List_map)(Cyc_Tcpat_add_false,_tmp111));});
# 477
(*res.tvars_and_bounds_opt).f1=_tmp48A;});
# 479
({struct Cyc_List_List*_tmp48B=
((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_append)((*res.tvars_and_bounds_opt).f2,outlives_constraints);
# 479
(*res.tvars_and_bounds_opt).f2=_tmp48B;});{
# 481
struct Cyc_List_List*_tmpC7=0;
{struct Cyc_List_List*_tmpC8=((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp113->impl))->rgn_po;for(0;_tmpC8 != 0;_tmpC8=_tmpC8->tl){
({struct Cyc_List_List*_tmp48F=({struct Cyc_List_List*_tmpC9=_cycalloc(sizeof(*_tmpC9));({struct _tuple0*_tmp48E=({struct _tuple0*_tmpCA=_cycalloc(sizeof(*_tmpCA));({void*_tmp48C=Cyc_Tcutil_rsubstitute(rgn,_tmpC5,(*((struct _tuple0*)_tmpC8->hd)).f1);_tmpCA->f1=_tmp48C;});({void*_tmp48D=
Cyc_Tcutil_rsubstitute(rgn,_tmpC5,(*((struct _tuple0*)_tmpC8->hd)).f2);_tmpCA->f2=_tmp48D;});_tmpCA;});_tmpC9->hd=_tmp48E;});_tmpC9->tl=_tmpC7;_tmpC9;});
# 483
_tmpC7=_tmp48F;});}}
# 486
({struct Cyc_List_List*_tmp490=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(_tmpC7);_tmpC7=_tmp490;});
({struct Cyc_List_List*_tmp491=
((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_append)((*res.tvars_and_bounds_opt).f2,_tmpC7);
# 487
(*res.tvars_and_bounds_opt).f2=_tmp491;});};}
# 491
*_tmp112=_tmpC3;
({void*_tmp495=(void*)({struct Cyc_Absyn_AggrType_Absyn_Type_struct*_tmpCB=_cycalloc(sizeof(*_tmpCB));({struct Cyc_Absyn_AggrType_Absyn_Type_struct _tmp494=({struct Cyc_Absyn_AggrType_Absyn_Type_struct _tmpCC;_tmpCC.tag=11U;({struct Cyc_Absyn_AggrInfo _tmp493=({struct Cyc_Absyn_AggrInfo _tmpCD;({union Cyc_Absyn_AggrInfoU _tmp492=Cyc_Absyn_KnownAggr(({struct Cyc_Absyn_Aggrdecl**_tmpCE=_cycalloc(sizeof(*_tmpCE));_tmpCE[0]=_tmp113;_tmpCE;}));_tmpCD.aggr_info=_tmp492;});_tmpCD.targs=*_tmp112;_tmpCD;});_tmpCC.f1=_tmp493;});_tmpCC;});_tmpCB[0]=_tmp494;});_tmpCB;});t=_tmp495;});
if(_tmp10F  && _tmp113->kind == Cyc_Absyn_UnionA)
({void*_tmpCF=0U;({unsigned int _tmp497=p->loc;struct _dyneither_ptr _tmp496=({const char*_tmpD0="`...' pattern not allowed in union pattern";_tag_dyneither(_tmpD0,sizeof(char),43U);});Cyc_Tcutil_warn(_tmp497,_tmp496,_tag_dyneither(_tmpCF,sizeof(void*),0U));});});else{
if(_tmp10F){
# 497
int _tmpD1=((int(*)(struct Cyc_List_List*x))Cyc_List_length)(_tmp9F);
int _tmpD2=((int(*)(struct Cyc_List_List*x))Cyc_List_length)(((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp113->impl))->fields);
if(_tmpD1 < _tmpD2){
struct Cyc_List_List*wild_dps=0;
{int i=0;for(0;i < _tmpD2 - _tmpD1;++ i){
({struct Cyc_List_List*_tmp49A=({struct Cyc_List_List*_tmpD3=_cycalloc(sizeof(*_tmpD3));({struct _tuple18*_tmp499=({struct _tuple18*_tmpD4=_cycalloc(sizeof(*_tmpD4));_tmpD4->f1=0;({struct Cyc_Absyn_Pat*_tmp498=Cyc_Tcpat_wild_pat(p->loc);_tmpD4->f2=_tmp498;});_tmpD4;});_tmpD3->hd=_tmp499;});_tmpD3->tl=wild_dps;_tmpD3;});wild_dps=_tmp49A;});}}
({struct Cyc_List_List*_tmp49B=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_imp_append)(_tmp9F,wild_dps);*_tmp110=_tmp49B;});
_tmp9F=*_tmp110;}else{
if(_tmpD1 == _tmpD2)
({void*_tmpD5=0U;({unsigned int _tmp49D=p->loc;struct _dyneither_ptr _tmp49C=({const char*_tmpD6="unnecessary ... in struct pattern";_tag_dyneither(_tmpD6,sizeof(char),34U);});Cyc_Tcutil_warn(_tmp49D,_tmp49C,_tag_dyneither(_tmpD5,sizeof(void*),0U));});});}}}{
# 508
struct Cyc_List_List*fields=
((struct Cyc_List_List*(*)(struct _RegionHandle*rgn,unsigned int loc,struct Cyc_List_List*des,enum Cyc_Absyn_AggrKind,struct Cyc_List_List*fields))Cyc_Tcutil_resolve_aggregate_designators)(rgn,p->loc,_tmp9F,_tmp113->kind,((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp113->impl))->fields);
for(0;fields != 0;fields=fields->tl){
struct _tuple19*_tmpD7=(struct _tuple19*)fields->hd;struct _tuple19*_tmpD8=_tmpD7;struct Cyc_Absyn_Aggrfield*_tmpE1;struct Cyc_Absyn_Pat*_tmpE0;_LL62: _tmpE1=_tmpD8->f1;_tmpE0=_tmpD8->f2;_LL63:;{
void*_tmpD9=Cyc_Tcutil_rsubstitute(rgn,_tmpC5,_tmpE1->type);
# 514
struct Cyc_Absyn_Exp*new_access_exp=0;
if((unsigned int)access_exp)
({struct Cyc_Absyn_Exp*_tmp49E=Cyc_Absyn_aggrmember_exp(access_exp,_tmpE1->name,0U);new_access_exp=_tmp49E;});
({struct Cyc_Tcpat_TcPatResult _tmp4A0=({struct Cyc_Tcpat_TcPatResult _tmp49F=res;Cyc_Tcpat_combine_results(_tmp49F,Cyc_Tcpat_tcPatRec(te2,_tmpE0,& _tmpD9,rgn_pat,allow_ref_pat,new_access_exp));});res=_tmp4A0;});
# 522
if(!Cyc_Tcutil_unify((void*)_check_null(_tmpE0->topt),_tmpD9))
({struct Cyc_String_pa_PrintArg_struct _tmpDF;_tmpDF.tag=0U;({struct _dyneither_ptr _tmp4A1=(struct _dyneither_ptr)((struct _dyneither_ptr)
# 525
Cyc_Absynpp_typ2string((void*)_check_null(_tmpE0->topt)));_tmpDF.f1=_tmp4A1;});({struct Cyc_String_pa_PrintArg_struct _tmpDE;_tmpDE.tag=0U;({struct _dyneither_ptr _tmp4A2=(struct _dyneither_ptr)((struct _dyneither_ptr)
# 524
Cyc_Absynpp_typ2string(_tmpD9));_tmpDE.f1=_tmp4A2;});({struct Cyc_String_pa_PrintArg_struct _tmpDD;_tmpDD.tag=0U;_tmpDD.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)aggr_str);({struct Cyc_String_pa_PrintArg_struct _tmpDC;_tmpDC.tag=0U;_tmpDC.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*_tmpE1->name);({void*_tmpDA[4U]={& _tmpDC,& _tmpDD,& _tmpDE,& _tmpDF};({unsigned int _tmp4A4=p->loc;struct _dyneither_ptr _tmp4A3=({const char*_tmpDB="field %s of %s pattern expects type %s != %s";_tag_dyneither(_tmpDB,sizeof(char),45U);});Cyc_Tcutil_terr(_tmp4A4,_tmp4A3,_tag_dyneither(_tmpDA,sizeof(void*),4U));});});});});});});
# 526
if((unsigned int)new_access_exp)new_access_exp->topt=_tmpE0->topt;};}};};}
# 418
;_pop_region(rgn);}
# 529
goto _LL0;}}else{_LL25: _LL26:
# 583
 goto _LL28;}}else{_LL23: _LL24:
# 582
 goto _LL26;}case 8U: _LL21: _tmp117=((struct Cyc_Absyn_Datatype_p_Absyn_Raw_pat_struct*)_tmp47)->f1;_tmp116=((struct Cyc_Absyn_Datatype_p_Absyn_Raw_pat_struct*)_tmp47)->f2;_tmp115=(struct Cyc_List_List**)&((struct Cyc_Absyn_Datatype_p_Absyn_Raw_pat_struct*)_tmp47)->f3;_tmp114=((struct Cyc_Absyn_Datatype_p_Absyn_Raw_pat_struct*)_tmp47)->f4;_LL22: {
# 532
struct Cyc_List_List*_tmpE5=*_tmp115;
struct Cyc_List_List*tqts=_tmp116->typs;
# 535
struct Cyc_List_List*_tmpE6=Cyc_Tcenv_lookup_type_vars(te);
struct _tuple12 _tmpE7=({struct _tuple12 _tmp100;_tmp100.f1=_tmpE6;_tmp100.f2=Cyc_Core_heap_region;_tmp100;});
struct Cyc_List_List*_tmpE8=((struct Cyc_List_List*(*)(struct _tuple13*(*f)(struct _tuple12*,struct Cyc_Absyn_Tvar*),struct _tuple12*env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcutil_r_make_inst_var,& _tmpE7,_tmp117->tvs);
struct Cyc_List_List*_tmpE9=((struct Cyc_List_List*(*)(void*(*f)(struct _tuple13*),struct Cyc_List_List*x))Cyc_List_map)((void*(*)(struct _tuple13*))Cyc_Core_snd,_tmpE8);
({void*_tmp4A8=(void*)({struct Cyc_Absyn_DatatypeFieldType_Absyn_Type_struct*_tmpEA=_cycalloc(sizeof(*_tmpEA));({struct Cyc_Absyn_DatatypeFieldType_Absyn_Type_struct _tmp4A7=({struct Cyc_Absyn_DatatypeFieldType_Absyn_Type_struct _tmpEB;_tmpEB.tag=4U;({struct Cyc_Absyn_DatatypeFieldInfo _tmp4A6=({struct Cyc_Absyn_DatatypeFieldInfo _tmpEC;({union Cyc_Absyn_DatatypeFieldInfoU _tmp4A5=Cyc_Absyn_KnownDatatypefield(_tmp117,_tmp116);_tmpEC.field_info=_tmp4A5;});_tmpEC.targs=_tmpE9;_tmpEC;});_tmpEB.f1=_tmp4A6;});_tmpEB;});_tmpEA[0]=_tmp4A7;});_tmpEA;});t=_tmp4A8;});
# 541
if(_tmp114){
# 543
int _tmpED=((int(*)(struct Cyc_List_List*x))Cyc_List_length)(_tmpE5);
int _tmpEE=((int(*)(struct Cyc_List_List*x))Cyc_List_length)(tqts);
if(_tmpED < _tmpEE){
struct Cyc_List_List*wild_ps=0;
{int i=0;for(0;i < _tmpEE - _tmpED;++ i){
({struct Cyc_List_List*_tmp4AA=({struct Cyc_List_List*_tmpEF=_cycalloc(sizeof(*_tmpEF));({struct Cyc_Absyn_Pat*_tmp4A9=Cyc_Tcpat_wild_pat(p->loc);_tmpEF->hd=_tmp4A9;});_tmpEF->tl=wild_ps;_tmpEF;});wild_ps=_tmp4AA;});}}
({struct Cyc_List_List*_tmp4AB=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_imp_append)(_tmpE5,wild_ps);*_tmp115=_tmp4AB;});
_tmpE5=*_tmp115;}else{
if(_tmpED == _tmpEE)
({struct Cyc_String_pa_PrintArg_struct _tmpF2;_tmpF2.tag=0U;({struct _dyneither_ptr _tmp4AC=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Absynpp_qvar2string(_tmp117->name));_tmpF2.f1=_tmp4AC;});({void*_tmpF0[1U]={& _tmpF2};({unsigned int _tmp4AE=p->loc;struct _dyneither_ptr _tmp4AD=({const char*_tmpF1="unnecessary ... in datatype field %s";_tag_dyneither(_tmpF1,sizeof(char),37U);});Cyc_Tcutil_warn(_tmp4AE,_tmp4AD,_tag_dyneither(_tmpF0,sizeof(void*),1U));});});});}}
# 555
for(0;_tmpE5 != 0  && tqts != 0;(_tmpE5=_tmpE5->tl,tqts=tqts->tl)){
struct Cyc_Absyn_Pat*_tmpF3=(struct Cyc_Absyn_Pat*)_tmpE5->hd;
# 559
void*_tmpF4=Cyc_Tcutil_substitute(_tmpE8,(*((struct _tuple17*)tqts->hd)).f2);
# 562
if((unsigned int)access_exp)
Cyc_Tcpat_set_vd(0,access_exp,& res.patvars,Cyc_Absyn_char_typ);
({struct Cyc_Tcpat_TcPatResult _tmp4B0=({struct Cyc_Tcpat_TcPatResult _tmp4AF=res;Cyc_Tcpat_combine_results(_tmp4AF,Cyc_Tcpat_tcPatRec(te,_tmpF3,& _tmpF4,rgn_pat,allow_ref_pat,0));});res=_tmp4B0;});
# 569
if(!Cyc_Tcutil_unify((void*)_check_null(_tmpF3->topt),_tmpF4))
({struct Cyc_String_pa_PrintArg_struct _tmpF9;_tmpF9.tag=0U;({struct _dyneither_ptr _tmp4B1=(struct _dyneither_ptr)((struct _dyneither_ptr)
# 572
Cyc_Absynpp_typ2string((void*)_check_null(_tmpF3->topt)));_tmpF9.f1=_tmp4B1;});({struct Cyc_String_pa_PrintArg_struct _tmpF8;_tmpF8.tag=0U;({struct _dyneither_ptr _tmp4B2=(struct _dyneither_ptr)((struct _dyneither_ptr)
# 571
Cyc_Absynpp_typ2string(_tmpF4));_tmpF8.f1=_tmp4B2;});({struct Cyc_String_pa_PrintArg_struct _tmpF7;_tmpF7.tag=0U;({struct _dyneither_ptr _tmp4B3=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(_tmp116->name));_tmpF7.f1=_tmp4B3;});({void*_tmpF5[3U]={& _tmpF7,& _tmpF8,& _tmpF9};({unsigned int _tmp4B5=_tmpF3->loc;struct _dyneither_ptr _tmp4B4=({const char*_tmpF6="%s expects argument type %s, not %s";_tag_dyneither(_tmpF6,sizeof(char),36U);});Cyc_Tcutil_terr(_tmp4B5,_tmp4B4,_tag_dyneither(_tmpF5,sizeof(void*),3U));});});});});});}
# 574
if(_tmpE5 != 0)
({struct Cyc_String_pa_PrintArg_struct _tmpFC;_tmpFC.tag=0U;({struct _dyneither_ptr _tmp4B6=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Absynpp_qvar2string(_tmp116->name));_tmpFC.f1=_tmp4B6;});({void*_tmpFA[1U]={& _tmpFC};({unsigned int _tmp4B8=p->loc;struct _dyneither_ptr _tmp4B7=({const char*_tmpFB="too many arguments for datatype constructor %s";_tag_dyneither(_tmpFB,sizeof(char),47U);});Cyc_Tcutil_terr(_tmp4B8,_tmp4B7,_tag_dyneither(_tmpFA,sizeof(void*),1U));});});});
if(tqts != 0)
({struct Cyc_String_pa_PrintArg_struct _tmpFF;_tmpFF.tag=0U;({struct _dyneither_ptr _tmp4B9=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Absynpp_qvar2string(_tmp116->name));_tmpFF.f1=_tmp4B9;});({void*_tmpFD[1U]={& _tmpFF};({unsigned int _tmp4BB=p->loc;struct _dyneither_ptr _tmp4BA=({const char*_tmpFE="too few arguments for datatype constructor %s";_tag_dyneither(_tmpFE,sizeof(char),46U);});Cyc_Tcutil_terr(_tmp4BB,_tmp4BA,_tag_dyneither(_tmpFD,sizeof(void*),1U));});});});
goto _LL0;}case 15U: _LL27: _LL28:
# 584
 goto _LL2A;case 17U: _LL29: _LL2A:
 goto _LL2C;default: _LL2B: _LL2C:
# 587
({void*_tmp4BC=Cyc_Absyn_wildtyp(Cyc_Tcenv_lookup_opt_type_vars(te));t=_tmp4BC;});goto _LL0;}_LL0:;}
# 589
tcpat_end:
 p->topt=t;
return res;};}
# 594
struct Cyc_Tcpat_TcPatResult Cyc_Tcpat_tcPat(struct Cyc_Tcenv_Tenv*te,struct Cyc_Absyn_Pat*p,void**topt,struct Cyc_Absyn_Exp*pat_var_exp){
# 596
struct Cyc_Tcpat_TcPatResult _tmp119=Cyc_Tcpat_tcPatRec(te,p,topt,0,0,pat_var_exp);
# 598
struct _tuple1 _tmp11A=((struct _tuple1(*)(struct Cyc_List_List*x))Cyc_List_split)(_tmp119.patvars);struct _tuple1 _tmp11B=_tmp11A;struct Cyc_List_List*_tmp123;_LL1: _tmp123=_tmp11B.f1;_LL2:;{
struct Cyc_List_List*_tmp11C=0;
{struct Cyc_List_List*x=_tmp123;for(0;x != 0;x=x->tl){
if((struct Cyc_Absyn_Vardecl**)x->hd != 0)({struct Cyc_List_List*_tmp4BD=({struct Cyc_List_List*_tmp11D=_cycalloc(sizeof(*_tmp11D));_tmp11D->hd=*((struct Cyc_Absyn_Vardecl**)_check_null((struct Cyc_Absyn_Vardecl**)x->hd));_tmp11D->tl=_tmp11C;_tmp11D;});_tmp11C=_tmp4BD;});}}
({struct Cyc_List_List*_tmp4BF=((struct Cyc_List_List*(*)(struct _dyneither_ptr*(*f)(struct Cyc_Absyn_Vardecl*),struct Cyc_List_List*x))Cyc_List_map)(Cyc_Tcpat_get_name,_tmp11C);unsigned int _tmp4BE=p->loc;Cyc_Tcutil_check_unique_vars(_tmp4BF,_tmp4BE,({const char*_tmp11E="pattern contains a repeated variable";_tag_dyneither(_tmp11E,sizeof(char),37U);}));});
# 607
{struct Cyc_List_List*x=_tmp119.patvars;for(0;x != 0;x=x->tl){
struct _tuple15*_tmp11F=(struct _tuple15*)x->hd;struct _tuple15*_tmp120=_tmp11F;struct Cyc_Absyn_Vardecl**_tmp122;struct Cyc_Absyn_Exp**_tmp121;_LL4: _tmp122=_tmp120->f1;_tmp121=(struct Cyc_Absyn_Exp**)& _tmp120->f2;_LL5:;
if(*_tmp121 != 0  && *_tmp121 != pat_var_exp)
({struct Cyc_Absyn_Exp*_tmp4C0=Cyc_Tcutil_deep_copy_exp(1,(struct Cyc_Absyn_Exp*)_check_null(*_tmp121));*_tmp121=_tmp4C0;});}}
# 612
return _tmp119;};}
# 618
static int Cyc_Tcpat_try_alias_coerce(struct Cyc_Tcenv_Tenv*tenv,void*old_type,void*new_type,struct Cyc_Absyn_Exp*initializer,struct Cyc_List_List*assump){
# 621
struct _tuple0 _tmp124=({struct _tuple0 _tmp12C;({void*_tmp4C1=Cyc_Tcutil_compress(old_type);_tmp12C.f1=_tmp4C1;});({void*_tmp4C2=Cyc_Tcutil_compress(new_type);_tmp12C.f2=_tmp4C2;});_tmp12C;});struct _tuple0 _tmp125=_tmp124;struct Cyc_Absyn_PtrInfo _tmp12B;struct Cyc_Absyn_PtrInfo _tmp12A;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp125.f1)->tag == 5U){if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp125.f2)->tag == 5U){_LL1: _tmp12B=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp125.f1)->f1;_tmp12A=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp125.f2)->f1;_LL2: {
# 623
struct Cyc_Absyn_PointerType_Absyn_Type_struct*_tmp126=({struct Cyc_Absyn_PointerType_Absyn_Type_struct*_tmp127=_cycalloc(sizeof(*_tmp127));({struct Cyc_Absyn_PointerType_Absyn_Type_struct _tmp4C5=({struct Cyc_Absyn_PointerType_Absyn_Type_struct _tmp128;_tmp128.tag=5U;({struct Cyc_Absyn_PtrInfo _tmp4C4=({struct Cyc_Absyn_PtrInfo _tmp129;_tmp129.elt_typ=_tmp12B.elt_typ;_tmp129.elt_tq=_tmp12A.elt_tq;({struct Cyc_Absyn_PtrAtts _tmp4C3=({(_tmp129.ptr_atts).rgn=(_tmp12B.ptr_atts).rgn;(_tmp129.ptr_atts).nullable=(_tmp12A.ptr_atts).nullable;(_tmp129.ptr_atts).bounds=(_tmp12A.ptr_atts).bounds;(_tmp129.ptr_atts).zero_term=(_tmp12A.ptr_atts).zero_term;(_tmp129.ptr_atts).ptrloc=(_tmp12B.ptr_atts).ptrloc;_tmp129.ptr_atts;});_tmp129.ptr_atts=_tmp4C3;});_tmp129;});_tmp128.f1=_tmp4C4;});_tmp128;});_tmp127[0]=_tmp4C5;});_tmp127;});
# 629
return Cyc_Tcutil_subtype(tenv,assump,(void*)_tmp126,new_type) && 
Cyc_Tcutil_coerce_assign(tenv,initializer,(void*)_tmp126);}}else{goto _LL3;}}else{_LL3: _LL4:
 return 0;}_LL0:;}
# 638
static void Cyc_Tcpat_check_alias_coercion(struct Cyc_Tcenv_Tenv*tenv,unsigned int loc,void*old_type,struct Cyc_Absyn_Tvar*tv,void*new_type,struct Cyc_Absyn_Exp*initializer){
# 641
struct Cyc_List_List*assump=({struct Cyc_List_List*_tmp138=_cycalloc(sizeof(*_tmp138));({struct _tuple0*_tmp4C8=({struct _tuple0*_tmp139=_cycalloc(sizeof(*_tmp139));_tmp139->f1=(void*)& Cyc_Absyn_UniqueRgn_val;({void*_tmp4C7=(void*)({struct Cyc_Absyn_VarType_Absyn_Type_struct*_tmp13A=_cycalloc(sizeof(*_tmp13A));({struct Cyc_Absyn_VarType_Absyn_Type_struct _tmp4C6=({struct Cyc_Absyn_VarType_Absyn_Type_struct _tmp13B;_tmp13B.tag=2U;_tmp13B.f1=tv;_tmp13B;});_tmp13A[0]=_tmp4C6;});_tmp13A;});_tmp139->f2=_tmp4C7;});_tmp139;});_tmp138->hd=_tmp4C8;});_tmp138->tl=0;_tmp138;});
if(Cyc_Tcutil_subtype(tenv,assump,old_type,new_type)){
# 659 "tcpat.cyc"
struct _tuple0 _tmp12D=({struct _tuple0 _tmp133;({void*_tmp4C9=Cyc_Tcutil_compress(old_type);_tmp133.f1=_tmp4C9;});({void*_tmp4CA=Cyc_Tcutil_compress(new_type);_tmp133.f2=_tmp4CA;});_tmp133;});struct _tuple0 _tmp12E=_tmp12D;struct Cyc_Absyn_PtrInfo _tmp132;struct Cyc_Absyn_PtrInfo _tmp131;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp12E.f1)->tag == 5U){if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp12E.f2)->tag == 5U){_LL1: _tmp132=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp12E.f1)->f1;_tmp131=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp12E.f2)->f1;_LL2:
 goto _LL0;}else{goto _LL3;}}else{_LL3: _LL4:
({void*_tmp12F=0U;({unsigned int _tmp4CC=loc;struct _dyneither_ptr _tmp4CB=({const char*_tmp130="alias requires pointer type";_tag_dyneither(_tmp130,sizeof(char),28U);});Cyc_Tcutil_terr(_tmp4CC,_tmp4CB,_tag_dyneither(_tmp12F,sizeof(void*),0U));});});goto _LL0;}_LL0:;}else{
# 664
({struct Cyc_String_pa_PrintArg_struct _tmp137;_tmp137.tag=0U;({struct _dyneither_ptr _tmp4CD=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Absynpp_typ2string(new_type));_tmp137.f1=_tmp4CD;});({struct Cyc_String_pa_PrintArg_struct _tmp136;_tmp136.tag=0U;({struct _dyneither_ptr _tmp4CE=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(old_type));_tmp136.f1=_tmp4CE;});({void*_tmp134[2U]={& _tmp136,& _tmp137};({unsigned int _tmp4D0=loc;struct _dyneither_ptr _tmp4CF=({const char*_tmp135="cannot alias value of type %s to type %s";_tag_dyneither(_tmp135,sizeof(char),41U);});Cyc_Tcutil_terr(_tmp4D0,_tmp4CF,_tag_dyneither(_tmp134,sizeof(void*),2U));});});});});}}
# 671
void Cyc_Tcpat_check_pat_regions_rec(struct Cyc_Tcenv_Tenv*te,struct Cyc_Absyn_Pat*p,int did_noalias_deref,struct Cyc_List_List*patvars){
# 675
void*_tmp13C=p->r;void*_tmp13D=_tmp13C;struct Cyc_Absyn_Tvar*_tmp166;struct Cyc_Absyn_Vardecl*_tmp165;struct Cyc_Absyn_Vardecl*_tmp164;struct Cyc_Absyn_Pat*_tmp163;struct Cyc_Absyn_Vardecl*_tmp162;struct Cyc_Absyn_Pat*_tmp161;struct Cyc_List_List*_tmp160;struct Cyc_List_List*_tmp15F;struct Cyc_Absyn_AggrInfo*_tmp15E;struct Cyc_List_List*_tmp15D;struct Cyc_List_List*_tmp15C;struct Cyc_Absyn_Pat*_tmp15B;switch(*((int*)_tmp13D)){case 6U: _LL1: _tmp15B=((struct Cyc_Absyn_Pointer_p_Absyn_Raw_pat_struct*)_tmp13D)->f1;_LL2: {
# 677
void*_tmp13E=(void*)_check_null(p->topt);void*_tmp13F=_tmp13E;void*_tmp142;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp13F)->tag == 5U){_LL12: _tmp142=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp13F)->f1).ptr_atts).rgn;_LL13:
# 679
 Cyc_Tcenv_check_rgn_accessible(te,p->loc,_tmp142);
({struct Cyc_Tcenv_Tenv*_tmp4D3=te;struct Cyc_Absyn_Pat*_tmp4D2=_tmp15B;int _tmp4D1=Cyc_Tcutil_is_noalias_region(_tmp142,0);Cyc_Tcpat_check_pat_regions_rec(_tmp4D3,_tmp4D2,_tmp4D1,patvars);});
return;}else{_LL14: _LL15:
({void*_tmp140=0U;({struct _dyneither_ptr _tmp4D4=({const char*_tmp141="check_pat_regions: bad pointer type";_tag_dyneither(_tmp141,sizeof(char),36U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp4D4,_tag_dyneither(_tmp140,sizeof(void*),0U));});});}_LL11:;}case 7U: _LL3: _tmp15E=((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp13D)->f1;_tmp15D=((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp13D)->f2;_tmp15C=((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp13D)->f3;_LL4:
# 685
 for(0;_tmp15C != 0;_tmp15C=_tmp15C->tl){
Cyc_Tcpat_check_pat_regions_rec(te,(*((struct _tuple18*)_tmp15C->hd)).f2,did_noalias_deref,patvars);}
return;case 8U: _LL5: _tmp15F=((struct Cyc_Absyn_Datatype_p_Absyn_Raw_pat_struct*)_tmp13D)->f3;_LL6:
# 689
 did_noalias_deref=0;_tmp160=_tmp15F;goto _LL8;case 5U: _LL7: _tmp160=((struct Cyc_Absyn_Tuple_p_Absyn_Raw_pat_struct*)_tmp13D)->f1;_LL8:
# 691
 for(0;_tmp160 != 0;_tmp160=_tmp160->tl){
Cyc_Tcpat_check_pat_regions_rec(te,(struct Cyc_Absyn_Pat*)_tmp160->hd,did_noalias_deref,patvars);}
return;case 3U: _LL9: _tmp162=((struct Cyc_Absyn_Reference_p_Absyn_Raw_pat_struct*)_tmp13D)->f1;_tmp161=((struct Cyc_Absyn_Reference_p_Absyn_Raw_pat_struct*)_tmp13D)->f2;_LLA:
# 695
{struct Cyc_List_List*x=patvars;for(0;x != 0;x=x->tl){
struct _tuple15*_tmp143=(struct _tuple15*)x->hd;struct _tuple15*_tmp144=_tmp143;struct Cyc_Absyn_Vardecl**_tmp14D;struct Cyc_Absyn_Exp*_tmp14C;_LL17: _tmp14D=_tmp144->f1;_tmp14C=_tmp144->f2;_LL18:;
# 701
if((_tmp14D != 0  && *_tmp14D == _tmp162) && _tmp14C != 0){
{void*_tmp145=_tmp14C->r;void*_tmp146=_tmp145;struct Cyc_Absyn_Exp*_tmp14B;if(((struct Cyc_Absyn_Address_e_Absyn_Raw_exp_struct*)_tmp146)->tag == 15U){_LL1A: _tmp14B=((struct Cyc_Absyn_Address_e_Absyn_Raw_exp_struct*)_tmp146)->f1;_LL1B:
# 704
 if(Cyc_Tcutil_is_noalias_pointer_or_aggr((void*)_check_null(_tmp14B->topt)))
({void*_tmp147=0U;({unsigned int _tmp4D6=p->loc;struct _dyneither_ptr _tmp4D5=({const char*_tmp148="reference pattern not allowed on alias-free pointers";_tag_dyneither(_tmp148,sizeof(char),53U);});Cyc_Tcutil_terr(_tmp4D6,_tmp4D5,_tag_dyneither(_tmp147,sizeof(void*),0U));});});
goto _LL19;}else{_LL1C: _LL1D:
# 708
({void*_tmp149=0U;({struct _dyneither_ptr _tmp4D7=({const char*_tmp14A="check_pat_regions: bad reference access expression";_tag_dyneither(_tmp14A,sizeof(char),51U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp4D7,_tag_dyneither(_tmp149,sizeof(void*),0U));});});}_LL19:;}
# 710
break;}}}
# 713
Cyc_Tcpat_check_pat_regions_rec(te,_tmp161,did_noalias_deref,patvars);
return;case 1U: _LLB: _tmp164=((struct Cyc_Absyn_Var_p_Absyn_Raw_pat_struct*)_tmp13D)->f1;_tmp163=((struct Cyc_Absyn_Var_p_Absyn_Raw_pat_struct*)_tmp13D)->f2;_LLC:
# 716
{void*_tmp14E=p->topt;void*_tmp14F=_tmp14E;if(_tmp14F != 0){if(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp14F)->tag == 8U){_LL1F: _LL20:
# 718
 if(did_noalias_deref){
({void*_tmp150=0U;({unsigned int _tmp4D9=p->loc;struct _dyneither_ptr _tmp4D8=({const char*_tmp151="pattern to array would create alias of no-alias pointer";_tag_dyneither(_tmp151,sizeof(char),56U);});Cyc_Tcutil_terr(_tmp4D9,_tmp4D8,_tag_dyneither(_tmp150,sizeof(void*),0U));});});
return;}
# 722
goto _LL1E;}else{goto _LL21;}}else{_LL21: _LL22:
 goto _LL1E;}_LL1E:;}
# 725
Cyc_Tcpat_check_pat_regions_rec(te,_tmp163,did_noalias_deref,patvars);
return;case 2U: _LLD: _tmp166=((struct Cyc_Absyn_AliasVar_p_Absyn_Raw_pat_struct*)_tmp13D)->f1;_tmp165=((struct Cyc_Absyn_AliasVar_p_Absyn_Raw_pat_struct*)_tmp13D)->f2;_LLE:
# 728
{struct Cyc_List_List*x=patvars;for(0;x != 0;x=x->tl){
struct _tuple15*_tmp152=(struct _tuple15*)x->hd;struct _tuple15*_tmp153=_tmp152;struct Cyc_Absyn_Vardecl**_tmp15A;struct Cyc_Absyn_Exp*_tmp159;_LL24: _tmp15A=_tmp153->f1;_tmp159=_tmp153->f2;_LL25:;
# 732
if(_tmp15A != 0  && *_tmp15A == _tmp165){
if(_tmp159 == 0)
({void*_tmp154=0U;({unsigned int _tmp4DB=p->loc;struct _dyneither_ptr _tmp4DA=({const char*_tmp155="cannot alias pattern expression in datatype";_tag_dyneither(_tmp155,sizeof(char),44U);});Cyc_Tcutil_terr(_tmp4DB,_tmp4DA,_tag_dyneither(_tmp154,sizeof(void*),0U));});});else{
# 736
struct Cyc_Tcenv_Tenv*te2=({unsigned int _tmp4DD=p->loc;struct Cyc_Tcenv_Tenv*_tmp4DC=te;Cyc_Tcenv_add_type_vars(_tmp4DD,_tmp4DC,({struct Cyc_Absyn_Tvar*_tmp158[1U];_tmp158[0U]=_tmp166;((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp158,sizeof(struct Cyc_Absyn_Tvar*),1U));}));});
({struct Cyc_Tcenv_Tenv*_tmp4E0=({struct Cyc_Tcenv_Tenv*_tmp4DF=te2;Cyc_Tcenv_add_region(_tmp4DF,(void*)({struct Cyc_Absyn_VarType_Absyn_Type_struct*_tmp156=_cycalloc(sizeof(*_tmp156));({struct Cyc_Absyn_VarType_Absyn_Type_struct _tmp4DE=({struct Cyc_Absyn_VarType_Absyn_Type_struct _tmp157;_tmp157.tag=2U;_tmp157.f1=_tmp166;_tmp157;});_tmp156[0]=_tmp4DE;});_tmp156;}),1);});te2=_tmp4E0;});
# 739
Cyc_Tcpat_check_alias_coercion(te2,p->loc,(void*)_check_null(_tmp159->topt),_tmp166,_tmp165->type,_tmp159);}
# 742
break;}}}
# 745
goto _LL0;default: _LLF: _LL10:
 return;}_LL0:;}
# 761 "tcpat.cyc"
void Cyc_Tcpat_check_pat_regions(struct Cyc_Tcenv_Tenv*te,struct Cyc_Absyn_Pat*p,struct Cyc_List_List*patvars){
# 763
Cyc_Tcpat_check_pat_regions_rec(te,p,0,patvars);{
struct Cyc_List_List*x=patvars;for(0;x != 0;x=x->tl){
struct _tuple15*_tmp167=(struct _tuple15*)x->hd;struct _tuple15*_tmp168=_tmp167;struct Cyc_Absyn_Vardecl**_tmp172;struct Cyc_Absyn_Exp*_tmp171;_LL1: _tmp172=_tmp168->f1;_tmp171=_tmp168->f2;_LL2:;
if(_tmp171 != 0){
struct Cyc_Absyn_Exp*_tmp169=_tmp171;
# 770
if(Cyc_Tcutil_is_noalias_pointer_or_aggr((void*)_check_null(_tmp169->topt)) && !Cyc_Tcutil_is_noalias_path(_tmp169))
# 772
({struct Cyc_String_pa_PrintArg_struct _tmp16C;_tmp16C.tag=0U;({struct _dyneither_ptr _tmp4E3=(struct _dyneither_ptr)(
# 774
_tmp172 != 0?(struct _dyneither_ptr)({struct Cyc_String_pa_PrintArg_struct _tmp16F;_tmp16F.tag=0U;({struct _dyneither_ptr _tmp4E1=(struct _dyneither_ptr)((struct _dyneither_ptr)
# 776
Cyc_Absynpp_qvar2string((*_tmp172)->name));_tmp16F.f1=_tmp4E1;});({void*_tmp16D[1U]={& _tmp16F};({struct _dyneither_ptr _tmp4E2=({const char*_tmp16E="for variable %s";_tag_dyneither(_tmp16E,sizeof(char),16U);});Cyc_aprintf(_tmp4E2,_tag_dyneither(_tmp16D,sizeof(void*),1U));});});}):({const char*_tmp170="";_tag_dyneither(_tmp170,sizeof(char),1U);}));_tmp16C.f1=_tmp4E3;});({void*_tmp16A[1U]={& _tmp16C};({unsigned int _tmp4E5=p->loc;struct _dyneither_ptr _tmp4E4=({const char*_tmp16B="pattern %s dereferences a alias-free pointer from a non-unique path";_tag_dyneither(_tmp16B,sizeof(char),68U);});Cyc_Tcutil_terr(_tmp4E5,_tmp4E4,_tag_dyneither(_tmp16A,sizeof(void*),1U));});});});}}};}
# 830 "tcpat.cyc"
struct Cyc_Tcpat_EqNull_Tcpat_PatTest_struct Cyc_Tcpat_EqNull_val={1U};
struct Cyc_Tcpat_NeqNull_Tcpat_PatTest_struct Cyc_Tcpat_NeqNull_val={2U};
# 840
struct Cyc_Tcpat_Dummy_Tcpat_Access_struct Cyc_Tcpat_Dummy_val={0U};
struct Cyc_Tcpat_Deref_Tcpat_Access_struct Cyc_Tcpat_Deref_val={1U};union Cyc_Tcpat_PatOrWhere;struct Cyc_Tcpat_PathNode;struct _union_Name_value_Name_v{int tag;struct _dyneither_ptr val;};struct _union_Name_value_Int_v{int tag;int val;};union Cyc_Tcpat_Name_value{struct _union_Name_value_Name_v Name_v;struct _union_Name_value_Int_v Int_v;};
# 854
union Cyc_Tcpat_Name_value Cyc_Tcpat_Name_v(struct _dyneither_ptr s){return({union Cyc_Tcpat_Name_value _tmp177;(_tmp177.Name_v).val=s;(_tmp177.Name_v).tag=1;_tmp177;});}
union Cyc_Tcpat_Name_value Cyc_Tcpat_Int_v(int i){return({union Cyc_Tcpat_Name_value _tmp178;(_tmp178.Int_v).val=i;(_tmp178.Int_v).tag=2;_tmp178;});}struct Cyc_Tcpat_Con_s{union Cyc_Tcpat_Name_value name;int arity;int*span;union Cyc_Tcpat_PatOrWhere orig_pat;};struct Cyc_Tcpat_Any_Tcpat_Simple_pat_struct{int tag;};struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct{int tag;struct Cyc_Tcpat_Con_s*f1;struct Cyc_List_List*f2;};
# 871
static int Cyc_Tcpat_compare_con(struct Cyc_Tcpat_Con_s*c1,struct Cyc_Tcpat_Con_s*c2){
union Cyc_Tcpat_Name_value _tmp179=c1->name;union Cyc_Tcpat_Name_value _tmp17A=_tmp179;int _tmp182;struct _dyneither_ptr _tmp181;if((_tmp17A.Name_v).tag == 1){_LL1: _tmp181=(_tmp17A.Name_v).val;_LL2: {
# 874
union Cyc_Tcpat_Name_value _tmp17B=c2->name;union Cyc_Tcpat_Name_value _tmp17C=_tmp17B;struct _dyneither_ptr _tmp17D;if((_tmp17C.Name_v).tag == 1){_LL6: _tmp17D=(_tmp17C.Name_v).val;_LL7:
 return Cyc_strcmp((struct _dyneither_ptr)_tmp181,(struct _dyneither_ptr)_tmp17D);}else{_LL8: _LL9:
 return - 1;}_LL5:;}}else{_LL3: _tmp182=(_tmp17A.Int_v).val;_LL4: {
# 879
union Cyc_Tcpat_Name_value _tmp17E=c2->name;union Cyc_Tcpat_Name_value _tmp17F=_tmp17E;int _tmp180;if((_tmp17F.Name_v).tag == 1){_LLB: _LLC:
 return 1;}else{_LLD: _tmp180=(_tmp17F.Int_v).val;_LLE:
 return _tmp182 - _tmp180;}_LLA:;}}_LL0:;}
# 887
static struct Cyc_Set_Set*Cyc_Tcpat_empty_con_set(){
return((struct Cyc_Set_Set*(*)(struct _RegionHandle*r,int(*cmp)(struct Cyc_Tcpat_Con_s*,struct Cyc_Tcpat_Con_s*)))Cyc_Set_rempty)(Cyc_Core_heap_region,Cyc_Tcpat_compare_con);}
# 891
static int Cyc_Tcpat_one_opt=1;
static int Cyc_Tcpat_two_opt=2;
static int Cyc_Tcpat_twofiftysix_opt=256;
# 895
static unsigned int Cyc_Tcpat_datatype_tag_number(struct Cyc_Absyn_Datatypedecl*td,struct _tuple2*name){
unsigned int ans=0U;
struct Cyc_List_List*_tmp183=(struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(td->fields))->v;
while(Cyc_Absyn_qvar_cmp(name,((struct Cyc_Absyn_Datatypefield*)((struct Cyc_List_List*)_check_null(_tmp183))->hd)->name)!= 0){
++ ans;
_tmp183=_tmp183->tl;}
# 902
return ans;}
# 905
static int Cyc_Tcpat_get_member_offset(struct Cyc_Absyn_Aggrdecl*ad,struct _dyneither_ptr*f){
int i=1;
{struct Cyc_List_List*_tmp184=((struct Cyc_Absyn_AggrdeclImpl*)_check_null(ad->impl))->fields;for(0;_tmp184 != 0;_tmp184=_tmp184->tl){
struct Cyc_Absyn_Aggrfield*_tmp185=(struct Cyc_Absyn_Aggrfield*)_tmp184->hd;
if(Cyc_strcmp((struct _dyneither_ptr)*_tmp185->name,(struct _dyneither_ptr)*f)== 0)return i;
++ i;}}
# 912
({void*_tmp186=0U;({struct _dyneither_ptr _tmp4E7=(struct _dyneither_ptr)({struct Cyc_String_pa_PrintArg_struct _tmp189;_tmp189.tag=0U;_tmp189.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*f);({void*_tmp187[1U]={& _tmp189};({struct _dyneither_ptr _tmp4E6=({const char*_tmp188="get_member_offset %s failed";_tag_dyneither(_tmp188,sizeof(char),28U);});Cyc_aprintf(_tmp4E6,_tag_dyneither(_tmp187,sizeof(void*),1U));});});});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp4E7,_tag_dyneither(_tmp186,sizeof(void*),0U));});});}
# 915
static void*Cyc_Tcpat_get_pat_test(union Cyc_Tcpat_PatOrWhere pw){
union Cyc_Tcpat_PatOrWhere _tmp18A=pw;struct Cyc_Absyn_Pat*_tmp1C3;struct Cyc_Absyn_Exp*_tmp1C2;if((_tmp18A.where_clause).tag == 2){_LL1: _tmp1C2=(_tmp18A.where_clause).val;_LL2:
 return(void*)({struct Cyc_Tcpat_WhereTest_Tcpat_PatTest_struct*_tmp18B=_cycalloc(sizeof(*_tmp18B));({struct Cyc_Tcpat_WhereTest_Tcpat_PatTest_struct _tmp4E8=({struct Cyc_Tcpat_WhereTest_Tcpat_PatTest_struct _tmp18C;_tmp18C.tag=0U;_tmp18C.f1=_tmp1C2;_tmp18C;});_tmp18B[0]=_tmp4E8;});_tmp18B;});}else{_LL3: _tmp1C3=(_tmp18A.pattern).val;_LL4: {
# 919
void*_tmp18D=_tmp1C3->r;void*_tmp18E=_tmp18D;union Cyc_Absyn_AggrInfoU _tmp1C1;struct Cyc_List_List*_tmp1C0;struct Cyc_Absyn_Datatypedecl*_tmp1BF;struct Cyc_Absyn_Datatypefield*_tmp1BE;void*_tmp1BD;struct Cyc_Absyn_Enumfield*_tmp1BC;struct Cyc_Absyn_Enumdecl*_tmp1BB;struct Cyc_Absyn_Enumfield*_tmp1BA;struct _dyneither_ptr _tmp1B9;int _tmp1B8;char _tmp1B7;enum Cyc_Absyn_Sign _tmp1B6;int _tmp1B5;struct Cyc_Absyn_Pat*_tmp1B4;struct Cyc_Absyn_Pat*_tmp1B3;switch(*((int*)_tmp18E)){case 1U: _LL6: _tmp1B3=((struct Cyc_Absyn_Var_p_Absyn_Raw_pat_struct*)_tmp18E)->f2;_LL7:
 _tmp1B4=_tmp1B3;goto _LL9;case 3U: _LL8: _tmp1B4=((struct Cyc_Absyn_Reference_p_Absyn_Raw_pat_struct*)_tmp18E)->f2;_LL9:
 return Cyc_Tcpat_get_pat_test(({union Cyc_Tcpat_PatOrWhere _tmp18F;(_tmp18F.pattern).val=_tmp1B4;(_tmp18F.pattern).tag=1;_tmp18F;}));case 9U: _LLA: _LLB:
 return(void*)& Cyc_Tcpat_EqNull_val;case 10U: _LLC: _tmp1B6=((struct Cyc_Absyn_Int_p_Absyn_Raw_pat_struct*)_tmp18E)->f1;_tmp1B5=((struct Cyc_Absyn_Int_p_Absyn_Raw_pat_struct*)_tmp18E)->f2;_LLD:
 return(void*)({struct Cyc_Tcpat_EqConst_Tcpat_PatTest_struct*_tmp190=_cycalloc_atomic(sizeof(*_tmp190));({struct Cyc_Tcpat_EqConst_Tcpat_PatTest_struct _tmp4E9=({struct Cyc_Tcpat_EqConst_Tcpat_PatTest_struct _tmp191;_tmp191.tag=6U;_tmp191.f1=(unsigned int)_tmp1B5;_tmp191;});_tmp190[0]=_tmp4E9;});_tmp190;});case 11U: _LLE: _tmp1B7=((struct Cyc_Absyn_Char_p_Absyn_Raw_pat_struct*)_tmp18E)->f1;_LLF:
 return(void*)({struct Cyc_Tcpat_EqConst_Tcpat_PatTest_struct*_tmp192=_cycalloc_atomic(sizeof(*_tmp192));({struct Cyc_Tcpat_EqConst_Tcpat_PatTest_struct _tmp4EA=({struct Cyc_Tcpat_EqConst_Tcpat_PatTest_struct _tmp193;_tmp193.tag=6U;_tmp193.f1=(unsigned int)_tmp1B7;_tmp193;});_tmp192[0]=_tmp4EA;});_tmp192;});case 12U: _LL10: _tmp1B9=((struct Cyc_Absyn_Float_p_Absyn_Raw_pat_struct*)_tmp18E)->f1;_tmp1B8=((struct Cyc_Absyn_Float_p_Absyn_Raw_pat_struct*)_tmp18E)->f2;_LL11:
 return(void*)({struct Cyc_Tcpat_EqFloat_Tcpat_PatTest_struct*_tmp194=_cycalloc(sizeof(*_tmp194));({struct Cyc_Tcpat_EqFloat_Tcpat_PatTest_struct _tmp4EB=({struct Cyc_Tcpat_EqFloat_Tcpat_PatTest_struct _tmp195;_tmp195.tag=5U;_tmp195.f1=_tmp1B9;_tmp195.f2=_tmp1B8;_tmp195;});_tmp194[0]=_tmp4EB;});_tmp194;});case 13U: _LL12: _tmp1BB=((struct Cyc_Absyn_Enum_p_Absyn_Raw_pat_struct*)_tmp18E)->f1;_tmp1BA=((struct Cyc_Absyn_Enum_p_Absyn_Raw_pat_struct*)_tmp18E)->f2;_LL13:
 return(void*)({struct Cyc_Tcpat_EqEnum_Tcpat_PatTest_struct*_tmp196=_cycalloc(sizeof(*_tmp196));({struct Cyc_Tcpat_EqEnum_Tcpat_PatTest_struct _tmp4EC=({struct Cyc_Tcpat_EqEnum_Tcpat_PatTest_struct _tmp197;_tmp197.tag=3U;_tmp197.f1=_tmp1BB;_tmp197.f2=_tmp1BA;_tmp197;});_tmp196[0]=_tmp4EC;});_tmp196;});case 14U: _LL14: _tmp1BD=(void*)((struct Cyc_Absyn_AnonEnum_p_Absyn_Raw_pat_struct*)_tmp18E)->f1;_tmp1BC=((struct Cyc_Absyn_AnonEnum_p_Absyn_Raw_pat_struct*)_tmp18E)->f2;_LL15:
 return(void*)({struct Cyc_Tcpat_EqAnonEnum_Tcpat_PatTest_struct*_tmp198=_cycalloc(sizeof(*_tmp198));({struct Cyc_Tcpat_EqAnonEnum_Tcpat_PatTest_struct _tmp4ED=({struct Cyc_Tcpat_EqAnonEnum_Tcpat_PatTest_struct _tmp199;_tmp199.tag=4U;_tmp199.f1=_tmp1BD;_tmp199.f2=_tmp1BC;_tmp199;});_tmp198[0]=_tmp4ED;});_tmp198;});case 6U: _LL16: _LL17:
# 929
{void*_tmp19A=Cyc_Tcutil_compress((void*)_check_null(_tmp1C3->topt));void*_tmp19B=_tmp19A;union Cyc_Absyn_Constraint*_tmp19C;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp19B)->tag == 5U){_LL1F: _tmp19C=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp19B)->f1).ptr_atts).nullable;_LL20:
# 931
 if(((int(*)(int y,union Cyc_Absyn_Constraint*x))Cyc_Absyn_conref_constr)(0,_tmp19C))
return(void*)& Cyc_Tcpat_NeqNull_val;
goto _LL1E;}else{_LL21: _LL22:
 goto _LL1E;}_LL1E:;}
# 936
({void*_tmp19D=0U;({struct _dyneither_ptr _tmp4EE=({const char*_tmp19E="non-null pointer type or non-pointer type in pointer pattern";_tag_dyneither(_tmp19E,sizeof(char),61U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp4EE,_tag_dyneither(_tmp19D,sizeof(void*),0U));});});case 8U: _LL18: _tmp1BF=((struct Cyc_Absyn_Datatype_p_Absyn_Raw_pat_struct*)_tmp18E)->f1;_tmp1BE=((struct Cyc_Absyn_Datatype_p_Absyn_Raw_pat_struct*)_tmp18E)->f2;_LL19:
# 938
 if(_tmp1BF->is_extensible)
return(void*)({struct Cyc_Tcpat_EqExtensibleDatatype_Tcpat_PatTest_struct*_tmp19F=_cycalloc(sizeof(*_tmp19F));({struct Cyc_Tcpat_EqExtensibleDatatype_Tcpat_PatTest_struct _tmp4EF=({struct Cyc_Tcpat_EqExtensibleDatatype_Tcpat_PatTest_struct _tmp1A0;_tmp1A0.tag=9U;_tmp1A0.f1=_tmp1BF;_tmp1A0.f2=_tmp1BE;_tmp1A0;});_tmp19F[0]=_tmp4EF;});_tmp19F;});else{
# 941
return(void*)({struct Cyc_Tcpat_EqDatatypeTag_Tcpat_PatTest_struct*_tmp1A1=_cycalloc(sizeof(*_tmp1A1));({struct Cyc_Tcpat_EqDatatypeTag_Tcpat_PatTest_struct _tmp4F1=({struct Cyc_Tcpat_EqDatatypeTag_Tcpat_PatTest_struct _tmp1A2;_tmp1A2.tag=7U;({int _tmp4F0=(int)Cyc_Tcpat_datatype_tag_number(_tmp1BF,_tmp1BE->name);_tmp1A2.f1=_tmp4F0;});_tmp1A2.f2=_tmp1BF;_tmp1A2.f3=_tmp1BE;_tmp1A2;});_tmp1A1[0]=_tmp4F1;});_tmp1A1;});}case 7U: if(((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp18E)->f1 != 0){_LL1A: _tmp1C1=(((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp18E)->f1)->aggr_info;_tmp1C0=((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp18E)->f3;_LL1B: {
# 943
struct Cyc_Absyn_Aggrdecl*_tmp1A3=Cyc_Absyn_get_known_aggrdecl(_tmp1C1);
if(((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp1A3->impl))->tagged){
struct _tuple18*_tmp1A4=(struct _tuple18*)((struct Cyc_List_List*)_check_null(_tmp1C0))->hd;struct _tuple18*_tmp1A5=_tmp1A4;struct Cyc_List_List*_tmp1AE;struct Cyc_Absyn_Pat*_tmp1AD;_LL24: _tmp1AE=_tmp1A5->f1;_tmp1AD=_tmp1A5->f2;_LL25:;{
struct _dyneither_ptr*f;
{void*_tmp1A6=(void*)((struct Cyc_List_List*)_check_null(_tmp1AE))->hd;void*_tmp1A7=_tmp1A6;struct _dyneither_ptr*_tmp1AA;if(((struct Cyc_Absyn_FieldName_Absyn_Designator_struct*)_tmp1A7)->tag == 1U){_LL27: _tmp1AA=((struct Cyc_Absyn_FieldName_Absyn_Designator_struct*)_tmp1A7)->f1;_LL28:
 f=_tmp1AA;goto _LL26;}else{_LL29: _LL2A:
({void*_tmp1A8=0U;({struct _dyneither_ptr _tmp4F2=({const char*_tmp1A9="no field name in tagged union pattern";_tag_dyneither(_tmp1A9,sizeof(char),38U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp4F2,_tag_dyneither(_tmp1A8,sizeof(void*),0U));});});}_LL26:;}
# 951
return(void*)({struct Cyc_Tcpat_EqTaggedUnion_Tcpat_PatTest_struct*_tmp1AB=_cycalloc(sizeof(*_tmp1AB));({struct Cyc_Tcpat_EqTaggedUnion_Tcpat_PatTest_struct _tmp4F4=({struct Cyc_Tcpat_EqTaggedUnion_Tcpat_PatTest_struct _tmp1AC;_tmp1AC.tag=8U;_tmp1AC.f1=f;({int _tmp4F3=Cyc_Tcpat_get_member_offset(_tmp1A3,f);_tmp1AC.f2=_tmp4F3;});_tmp1AC;});_tmp1AB[0]=_tmp4F4;});_tmp1AB;});};}else{
# 953
({void*_tmp1AF=0U;({struct _dyneither_ptr _tmp4F5=({const char*_tmp1B0="non-tagged aggregate in pattern test";_tag_dyneither(_tmp1B0,sizeof(char),37U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp4F5,_tag_dyneither(_tmp1AF,sizeof(void*),0U));});});}}}else{goto _LL1C;}default: _LL1C: _LL1D:
({void*_tmp1B1=0U;({struct _dyneither_ptr _tmp4F6=({const char*_tmp1B2="non-test pattern in pattern test";_tag_dyneither(_tmp1B2,sizeof(char),33U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp4F6,_tag_dyneither(_tmp1B1,sizeof(void*),0U));});});}_LL5:;}}_LL0:;}
# 959
static union Cyc_Tcpat_PatOrWhere Cyc_Tcpat_pw(struct Cyc_Absyn_Pat*p){
return({union Cyc_Tcpat_PatOrWhere _tmp1C4;(_tmp1C4.pattern).val=p;(_tmp1C4.pattern).tag=1;_tmp1C4;});}
# 963
static struct Cyc_Tcpat_Con_s*Cyc_Tcpat_null_con(struct Cyc_Absyn_Pat*p){
return({struct Cyc_Tcpat_Con_s*_tmp1C5=_cycalloc(sizeof(*_tmp1C5));({union Cyc_Tcpat_Name_value _tmp4F7=Cyc_Tcpat_Name_v(({const char*_tmp1C6="NULL";_tag_dyneither(_tmp1C6,sizeof(char),5U);}));_tmp1C5->name=_tmp4F7;});_tmp1C5->arity=0;_tmp1C5->span=& Cyc_Tcpat_two_opt;({union Cyc_Tcpat_PatOrWhere _tmp4F8=Cyc_Tcpat_pw(p);_tmp1C5->orig_pat=_tmp4F8;});_tmp1C5;});}
# 966
static struct Cyc_Tcpat_Con_s*Cyc_Tcpat_null_ptr_con(struct Cyc_Absyn_Pat*p){
return({struct Cyc_Tcpat_Con_s*_tmp1C7=_cycalloc(sizeof(*_tmp1C7));({union Cyc_Tcpat_Name_value _tmp4F9=Cyc_Tcpat_Name_v(({const char*_tmp1C8="&";_tag_dyneither(_tmp1C8,sizeof(char),2U);}));_tmp1C7->name=_tmp4F9;});_tmp1C7->arity=1;_tmp1C7->span=& Cyc_Tcpat_two_opt;({union Cyc_Tcpat_PatOrWhere _tmp4FA=Cyc_Tcpat_pw(p);_tmp1C7->orig_pat=_tmp4FA;});_tmp1C7;});}
# 969
static struct Cyc_Tcpat_Con_s*Cyc_Tcpat_ptr_con(struct Cyc_Absyn_Pat*p){
return({struct Cyc_Tcpat_Con_s*_tmp1C9=_cycalloc(sizeof(*_tmp1C9));({union Cyc_Tcpat_Name_value _tmp4FB=Cyc_Tcpat_Name_v(({const char*_tmp1CA="&";_tag_dyneither(_tmp1CA,sizeof(char),2U);}));_tmp1C9->name=_tmp4FB;});_tmp1C9->arity=1;_tmp1C9->span=& Cyc_Tcpat_one_opt;({union Cyc_Tcpat_PatOrWhere _tmp4FC=Cyc_Tcpat_pw(p);_tmp1C9->orig_pat=_tmp4FC;});_tmp1C9;});}
# 972
static struct Cyc_Tcpat_Con_s*Cyc_Tcpat_int_con(int i,union Cyc_Tcpat_PatOrWhere p){
return({struct Cyc_Tcpat_Con_s*_tmp1CB=_cycalloc(sizeof(*_tmp1CB));({union Cyc_Tcpat_Name_value _tmp4FD=Cyc_Tcpat_Int_v(i);_tmp1CB->name=_tmp4FD;});_tmp1CB->arity=0;_tmp1CB->span=0;_tmp1CB->orig_pat=p;_tmp1CB;});}
# 975
static struct Cyc_Tcpat_Con_s*Cyc_Tcpat_float_con(struct _dyneither_ptr f,struct Cyc_Absyn_Pat*p){
return({struct Cyc_Tcpat_Con_s*_tmp1CC=_cycalloc(sizeof(*_tmp1CC));({union Cyc_Tcpat_Name_value _tmp4FE=Cyc_Tcpat_Name_v(f);_tmp1CC->name=_tmp4FE;});_tmp1CC->arity=0;_tmp1CC->span=0;({union Cyc_Tcpat_PatOrWhere _tmp4FF=Cyc_Tcpat_pw(p);_tmp1CC->orig_pat=_tmp4FF;});_tmp1CC;});}
# 978
static struct Cyc_Tcpat_Con_s*Cyc_Tcpat_char_con(char c,struct Cyc_Absyn_Pat*p){
return({struct Cyc_Tcpat_Con_s*_tmp1CD=_cycalloc(sizeof(*_tmp1CD));({union Cyc_Tcpat_Name_value _tmp500=Cyc_Tcpat_Int_v((int)c);_tmp1CD->name=_tmp500;});_tmp1CD->arity=0;_tmp1CD->span=& Cyc_Tcpat_twofiftysix_opt;({union Cyc_Tcpat_PatOrWhere _tmp501=Cyc_Tcpat_pw(p);_tmp1CD->orig_pat=_tmp501;});_tmp1CD;});}
# 981
static struct Cyc_Tcpat_Con_s*Cyc_Tcpat_tuple_con(int i,union Cyc_Tcpat_PatOrWhere p){
return({struct Cyc_Tcpat_Con_s*_tmp1CE=_cycalloc(sizeof(*_tmp1CE));({union Cyc_Tcpat_Name_value _tmp502=Cyc_Tcpat_Name_v(({const char*_tmp1CF="$";_tag_dyneither(_tmp1CF,sizeof(char),2U);}));_tmp1CE->name=_tmp502;});_tmp1CE->arity=i;_tmp1CE->span=& Cyc_Tcpat_one_opt;_tmp1CE->orig_pat=p;_tmp1CE;});}
# 986
static void*Cyc_Tcpat_null_pat(struct Cyc_Absyn_Pat*p){
return(void*)({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct*_tmp1D0=_cycalloc(sizeof(*_tmp1D0));({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct _tmp504=({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct _tmp1D1;_tmp1D1.tag=1U;({struct Cyc_Tcpat_Con_s*_tmp503=Cyc_Tcpat_null_con(p);_tmp1D1.f1=_tmp503;});_tmp1D1.f2=0;_tmp1D1;});_tmp1D0[0]=_tmp504;});_tmp1D0;});}
# 989
static void*Cyc_Tcpat_int_pat(int i,union Cyc_Tcpat_PatOrWhere p){
return(void*)({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct*_tmp1D2=_cycalloc(sizeof(*_tmp1D2));({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct _tmp506=({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct _tmp1D3;_tmp1D3.tag=1U;({struct Cyc_Tcpat_Con_s*_tmp505=Cyc_Tcpat_int_con(i,p);_tmp1D3.f1=_tmp505;});_tmp1D3.f2=0;_tmp1D3;});_tmp1D2[0]=_tmp506;});_tmp1D2;});}
# 992
static void*Cyc_Tcpat_char_pat(char c,struct Cyc_Absyn_Pat*p){
return(void*)({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct*_tmp1D4=_cycalloc(sizeof(*_tmp1D4));({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct _tmp508=({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct _tmp1D5;_tmp1D5.tag=1U;({struct Cyc_Tcpat_Con_s*_tmp507=Cyc_Tcpat_char_con(c,p);_tmp1D5.f1=_tmp507;});_tmp1D5.f2=0;_tmp1D5;});_tmp1D4[0]=_tmp508;});_tmp1D4;});}
# 995
static void*Cyc_Tcpat_float_pat(struct _dyneither_ptr f,struct Cyc_Absyn_Pat*p){
return(void*)({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct*_tmp1D6=_cycalloc(sizeof(*_tmp1D6));({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct _tmp50A=({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct _tmp1D7;_tmp1D7.tag=1U;({struct Cyc_Tcpat_Con_s*_tmp509=Cyc_Tcpat_float_con(f,p);_tmp1D7.f1=_tmp509;});_tmp1D7.f2=0;_tmp1D7;});_tmp1D6[0]=_tmp50A;});_tmp1D6;});}
# 998
static void*Cyc_Tcpat_null_ptr_pat(void*p,struct Cyc_Absyn_Pat*p0){
return(void*)({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct*_tmp1D8=_cycalloc(sizeof(*_tmp1D8));({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct _tmp50D=({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct _tmp1D9;_tmp1D9.tag=1U;({struct Cyc_Tcpat_Con_s*_tmp50B=Cyc_Tcpat_null_ptr_con(p0);_tmp1D9.f1=_tmp50B;});({struct Cyc_List_List*_tmp50C=({struct Cyc_List_List*_tmp1DA=_cycalloc(sizeof(*_tmp1DA));_tmp1DA->hd=p;_tmp1DA->tl=0;_tmp1DA;});_tmp1D9.f2=_tmp50C;});_tmp1D9;});_tmp1D8[0]=_tmp50D;});_tmp1D8;});}
# 1001
static void*Cyc_Tcpat_ptr_pat(void*p,struct Cyc_Absyn_Pat*p0){
return(void*)({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct*_tmp1DB=_cycalloc(sizeof(*_tmp1DB));({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct _tmp510=({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct _tmp1DC;_tmp1DC.tag=1U;({struct Cyc_Tcpat_Con_s*_tmp50E=Cyc_Tcpat_ptr_con(p0);_tmp1DC.f1=_tmp50E;});({struct Cyc_List_List*_tmp50F=({struct Cyc_List_List*_tmp1DD=_cycalloc(sizeof(*_tmp1DD));_tmp1DD->hd=p;_tmp1DD->tl=0;_tmp1DD;});_tmp1DC.f2=_tmp50F;});_tmp1DC;});_tmp1DB[0]=_tmp510;});_tmp1DB;});}
# 1004
static void*Cyc_Tcpat_tuple_pat(struct Cyc_List_List*ss,union Cyc_Tcpat_PatOrWhere p){
return(void*)({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct*_tmp1DE=_cycalloc(sizeof(*_tmp1DE));({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct _tmp513=({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct _tmp1DF;_tmp1DF.tag=1U;({struct Cyc_Tcpat_Con_s*_tmp512=({int _tmp511=((int(*)(struct Cyc_List_List*x))Cyc_List_length)(ss);Cyc_Tcpat_tuple_con(_tmp511,p);});_tmp1DF.f1=_tmp512;});_tmp1DF.f2=ss;_tmp1DF;});_tmp1DE[0]=_tmp513;});_tmp1DE;});}
# 1007
static void*Cyc_Tcpat_con_pat(struct _dyneither_ptr con_name,int*span,struct Cyc_List_List*ps,struct Cyc_Absyn_Pat*p){
# 1009
struct Cyc_Tcpat_Con_s*c=({struct Cyc_Tcpat_Con_s*_tmp1E2=_cycalloc(sizeof(*_tmp1E2));({union Cyc_Tcpat_Name_value _tmp514=Cyc_Tcpat_Name_v(con_name);_tmp1E2->name=_tmp514;});({int _tmp515=((int(*)(struct Cyc_List_List*x))Cyc_List_length)(ps);_tmp1E2->arity=_tmp515;});_tmp1E2->span=span;({union Cyc_Tcpat_PatOrWhere _tmp516=Cyc_Tcpat_pw(p);_tmp1E2->orig_pat=_tmp516;});_tmp1E2;});
return(void*)({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct*_tmp1E0=_cycalloc(sizeof(*_tmp1E0));({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct _tmp517=({struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct _tmp1E1;_tmp1E1.tag=1U;_tmp1E1.f1=c;_tmp1E1.f2=ps;_tmp1E1;});_tmp1E0[0]=_tmp517;});_tmp1E0;});}
# 1014
static void*Cyc_Tcpat_compile_pat(struct Cyc_Absyn_Pat*p){
void*s;
{void*_tmp1E3=p->r;void*_tmp1E4=_tmp1E3;void*_tmp227;struct Cyc_Absyn_Enumfield*_tmp226;struct Cyc_Absyn_Enumdecl*_tmp225;struct Cyc_Absyn_Enumfield*_tmp224;struct Cyc_Absyn_Aggrdecl*_tmp223;struct Cyc_List_List*_tmp222;struct Cyc_List_List*_tmp221;struct Cyc_Absyn_Datatypedecl*_tmp220;struct Cyc_Absyn_Datatypefield*_tmp21F;struct Cyc_List_List*_tmp21E;struct Cyc_Absyn_Pat*_tmp21D;struct Cyc_Absyn_Pat*_tmp21C;struct Cyc_Absyn_Pat*_tmp21B;struct _dyneither_ptr _tmp21A;char _tmp219;enum Cyc_Absyn_Sign _tmp218;int _tmp217;switch(*((int*)_tmp1E4)){case 0U: _LL1: _LL2:
 goto _LL4;case 2U: _LL3: _LL4:
 goto _LL6;case 4U: _LL5: _LL6:
({void*_tmp519=(void*)({struct Cyc_Tcpat_Any_Tcpat_Simple_pat_struct*_tmp1E5=_cycalloc_atomic(sizeof(*_tmp1E5));({struct Cyc_Tcpat_Any_Tcpat_Simple_pat_struct _tmp518=({struct Cyc_Tcpat_Any_Tcpat_Simple_pat_struct _tmp1E6;_tmp1E6.tag=0U;_tmp1E6;});_tmp1E5[0]=_tmp518;});_tmp1E5;});s=_tmp519;});goto _LL0;case 9U: _LL7: _LL8:
({void*_tmp51A=Cyc_Tcpat_null_pat(p);s=_tmp51A;});goto _LL0;case 10U: _LL9: _tmp218=((struct Cyc_Absyn_Int_p_Absyn_Raw_pat_struct*)_tmp1E4)->f1;_tmp217=((struct Cyc_Absyn_Int_p_Absyn_Raw_pat_struct*)_tmp1E4)->f2;_LLA:
({void*_tmp51C=({int _tmp51B=_tmp217;Cyc_Tcpat_int_pat(_tmp51B,Cyc_Tcpat_pw(p));});s=_tmp51C;});goto _LL0;case 11U: _LLB: _tmp219=((struct Cyc_Absyn_Char_p_Absyn_Raw_pat_struct*)_tmp1E4)->f1;_LLC:
({void*_tmp51D=Cyc_Tcpat_char_pat(_tmp219,p);s=_tmp51D;});goto _LL0;case 12U: _LLD: _tmp21A=((struct Cyc_Absyn_Float_p_Absyn_Raw_pat_struct*)_tmp1E4)->f1;_LLE:
({void*_tmp51E=Cyc_Tcpat_float_pat(_tmp21A,p);s=_tmp51E;});goto _LL0;case 1U: _LLF: _tmp21B=((struct Cyc_Absyn_Var_p_Absyn_Raw_pat_struct*)_tmp1E4)->f2;_LL10:
({void*_tmp51F=Cyc_Tcpat_compile_pat(_tmp21B);s=_tmp51F;});goto _LL0;case 3U: _LL11: _tmp21C=((struct Cyc_Absyn_Reference_p_Absyn_Raw_pat_struct*)_tmp1E4)->f2;_LL12:
({void*_tmp520=Cyc_Tcpat_compile_pat(_tmp21C);s=_tmp520;});goto _LL0;case 6U: _LL13: _tmp21D=((struct Cyc_Absyn_Pointer_p_Absyn_Raw_pat_struct*)_tmp1E4)->f1;_LL14:
# 1027
{void*_tmp1E7=Cyc_Tcutil_compress((void*)_check_null(p->topt));void*_tmp1E8=_tmp1E7;union Cyc_Absyn_Constraint*_tmp1EF;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1E8)->tag == 5U){_LL28: _tmp1EF=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1E8)->f1).ptr_atts).nullable;_LL29: {
# 1029
int is_nullable=0;
int still_working=1;
while(still_working){
union Cyc_Absyn_Constraint*_tmp1E9=_tmp1EF;int _tmp1EC;union Cyc_Absyn_Constraint*_tmp1EB;switch((((union Cyc_Absyn_Constraint*)_tmp1E9)->No_constr).tag){case 2U: _LL2D: _tmp1EB=(_tmp1E9->Forward_constr).val;_LL2E:
# 1034
*_tmp1EF=*_tmp1EB;
continue;case 3U: _LL2F: _LL30:
# 1037
({struct _union_Constraint_Eq_constr*_tmp1EA=& _tmp1EF->Eq_constr;_tmp1EA->tag=1;_tmp1EA->val=0;});
is_nullable=0;
still_working=0;
goto _LL2C;default: _LL31: _tmp1EC=(int)(_tmp1E9->Eq_constr).val;_LL32:
# 1042
 is_nullable=_tmp1EC;
still_working=0;
goto _LL2C;}_LL2C:;}{
# 1047
void*ss=Cyc_Tcpat_compile_pat(_tmp21D);
if(is_nullable)
({void*_tmp521=Cyc_Tcpat_null_ptr_pat(ss,p);s=_tmp521;});else{
# 1051
({void*_tmp522=Cyc_Tcpat_ptr_pat(ss,p);s=_tmp522;});}
goto _LL27;};}}else{_LL2A: _LL2B:
({void*_tmp1ED=0U;({struct _dyneither_ptr _tmp523=({const char*_tmp1EE="expecting pointertype for pattern!";_tag_dyneither(_tmp1EE,sizeof(char),35U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp523,_tag_dyneither(_tmp1ED,sizeof(void*),0U));});});}_LL27:;}
# 1055
goto _LL0;case 8U: _LL15: _tmp220=((struct Cyc_Absyn_Datatype_p_Absyn_Raw_pat_struct*)_tmp1E4)->f1;_tmp21F=((struct Cyc_Absyn_Datatype_p_Absyn_Raw_pat_struct*)_tmp1E4)->f2;_tmp21E=((struct Cyc_Absyn_Datatype_p_Absyn_Raw_pat_struct*)_tmp1E4)->f3;_LL16: {
# 1057
int*span;
{void*_tmp1F0=Cyc_Tcutil_compress((void*)_check_null(p->topt));void*_tmp1F1=_tmp1F0;switch(*((int*)_tmp1F1)){case 3U: _LL34: _LL35:
# 1060
 if(_tmp220->is_extensible)
span=0;else{
# 1063
({int*_tmp525=({int*_tmp1F2=_cycalloc_atomic(sizeof(*_tmp1F2));({int _tmp524=((int(*)(struct Cyc_List_List*x))Cyc_List_length)((struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(_tmp220->fields))->v);_tmp1F2[0]=_tmp524;});_tmp1F2;});span=_tmp525;});}
goto _LL33;case 4U: _LL36: _LL37:
 span=& Cyc_Tcpat_one_opt;goto _LL33;default: _LL38: _LL39:
({int*_tmp527=({void*_tmp1F3=0U;({struct _dyneither_ptr _tmp526=({const char*_tmp1F4="void datatype pattern has bad type";_tag_dyneither(_tmp1F4,sizeof(char),35U);});((int*(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp526,_tag_dyneither(_tmp1F3,sizeof(void*),0U));});});span=_tmp527;});goto _LL33;}_LL33:;}
# 1068
({void*_tmp52B=({struct _dyneither_ptr _tmp52A=*(*_tmp21F->name).f2;int*_tmp529=span;struct Cyc_List_List*_tmp528=((struct Cyc_List_List*(*)(void*(*f)(struct Cyc_Absyn_Pat*),struct Cyc_List_List*x))Cyc_List_map)(Cyc_Tcpat_compile_pat,_tmp21E);Cyc_Tcpat_con_pat(_tmp52A,_tmp529,_tmp528,p);});s=_tmp52B;});
goto _LL0;}case 5U: _LL17: _tmp221=((struct Cyc_Absyn_Tuple_p_Absyn_Raw_pat_struct*)_tmp1E4)->f1;_LL18:
# 1072
({void*_tmp52D=({struct Cyc_List_List*_tmp52C=((struct Cyc_List_List*(*)(void*(*f)(struct Cyc_Absyn_Pat*),struct Cyc_List_List*x))Cyc_List_map)(Cyc_Tcpat_compile_pat,_tmp221);Cyc_Tcpat_tuple_pat(_tmp52C,Cyc_Tcpat_pw(p));});s=_tmp52D;});goto _LL0;case 7U: if(((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp1E4)->f1 != 0){if(((((struct Cyc_Absyn_AggrInfo*)((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp1E4)->f1)->aggr_info).KnownAggr).tag == 2){_LL19: _tmp223=*(((((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp1E4)->f1)->aggr_info).KnownAggr).val;_tmp222=((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp1E4)->f3;_LL1A:
# 1077
 if(_tmp223->kind == Cyc_Absyn_StructA){
struct Cyc_List_List*ps=0;
{struct Cyc_List_List*fields=((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp223->impl))->fields;for(0;fields != 0;fields=fields->tl){
# 1081
int found=({struct _dyneither_ptr _tmp52E=(struct _dyneither_ptr)*((struct Cyc_Absyn_Aggrfield*)fields->hd)->name;Cyc_strcmp(_tmp52E,({const char*_tmp200="";_tag_dyneither(_tmp200,sizeof(char),1U);}));})== 0;
{struct Cyc_List_List*dlps0=_tmp222;for(0;!found  && dlps0 != 0;dlps0=dlps0->tl){
struct _tuple18*_tmp1F5=(struct _tuple18*)dlps0->hd;struct _tuple18*_tmp1F6=_tmp1F5;struct Cyc_List_List*_tmp1FD;struct Cyc_Absyn_Pat*_tmp1FC;_LL3B: _tmp1FD=_tmp1F6->f1;_tmp1FC=_tmp1F6->f2;_LL3C:;{
struct Cyc_List_List*_tmp1F7=_tmp1FD;struct _dyneither_ptr*_tmp1FB;if(_tmp1F7 != 0){if(((struct Cyc_Absyn_FieldName_Absyn_Designator_struct*)((struct Cyc_List_List*)_tmp1F7)->hd)->tag == 1U){if(((struct Cyc_List_List*)_tmp1F7)->tl == 0){_LL3E: _tmp1FB=((struct Cyc_Absyn_FieldName_Absyn_Designator_struct*)_tmp1F7->hd)->f1;_LL3F:
# 1086
 if(Cyc_strptrcmp(_tmp1FB,((struct Cyc_Absyn_Aggrfield*)fields->hd)->name)== 0){
({struct Cyc_List_List*_tmp530=({struct Cyc_List_List*_tmp1F8=_cycalloc(sizeof(*_tmp1F8));({void*_tmp52F=Cyc_Tcpat_compile_pat(_tmp1FC);_tmp1F8->hd=_tmp52F;});_tmp1F8->tl=ps;_tmp1F8;});ps=_tmp530;});
found=1;}
# 1090
goto _LL3D;}else{goto _LL40;}}else{goto _LL40;}}else{_LL40: _LL41:
({void*_tmp1F9=0U;({struct _dyneither_ptr _tmp531=({const char*_tmp1FA="bad designator(s)";_tag_dyneither(_tmp1FA,sizeof(char),18U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp531,_tag_dyneither(_tmp1F9,sizeof(void*),0U));});});}_LL3D:;};}}
# 1094
if(!found)
({void*_tmp1FE=0U;({struct _dyneither_ptr _tmp532=({const char*_tmp1FF="bad designator";_tag_dyneither(_tmp1FF,sizeof(char),15U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp532,_tag_dyneither(_tmp1FE,sizeof(void*),0U));});});}}
# 1097
({void*_tmp534=({struct Cyc_List_List*_tmp533=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(ps);Cyc_Tcpat_tuple_pat(_tmp533,Cyc_Tcpat_pw(p));});s=_tmp534;});}else{
# 1100
if(!((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp223->impl))->tagged)
({void*_tmp201=0U;({unsigned int _tmp536=p->loc;struct _dyneither_ptr _tmp535=({const char*_tmp202="patterns on untagged unions not yet supported.";_tag_dyneither(_tmp202,sizeof(char),47U);});Cyc_Tcutil_terr(_tmp536,_tmp535,_tag_dyneither(_tmp201,sizeof(void*),0U));});});{
int*span=({int*_tmp209=_cycalloc_atomic(sizeof(*_tmp209));({int _tmp537=((int(*)(struct Cyc_List_List*x))Cyc_List_length)(((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp223->impl))->fields);_tmp209[0]=_tmp537;});_tmp209;});
struct Cyc_List_List*_tmp203=_tmp222;struct _dyneither_ptr*_tmp208;struct Cyc_Absyn_Pat*_tmp207;if(_tmp203 != 0){if(((struct _tuple18*)((struct Cyc_List_List*)_tmp203)->hd)->f1 != 0){if(((struct Cyc_Absyn_FieldName_Absyn_Designator_struct*)((struct Cyc_List_List*)((struct _tuple18*)((struct Cyc_List_List*)_tmp203)->hd)->f1)->hd)->tag == 1U){if(((struct Cyc_List_List*)((struct _tuple18*)((struct Cyc_List_List*)_tmp203)->hd)->f1)->tl == 0){if(((struct Cyc_List_List*)_tmp203)->tl == 0){_LL43: _tmp208=((struct Cyc_Absyn_FieldName_Absyn_Designator_struct*)(((struct _tuple18*)_tmp203->hd)->f1)->hd)->f1;_tmp207=((struct _tuple18*)_tmp203->hd)->f2;_LL44:
# 1105
({void*_tmp53C=({struct _dyneither_ptr _tmp53B=*_tmp208;int*_tmp53A=span;struct Cyc_List_List*_tmp539=({struct Cyc_List_List*_tmp204=_cycalloc(sizeof(*_tmp204));({void*_tmp538=Cyc_Tcpat_compile_pat(_tmp207);_tmp204->hd=_tmp538;});_tmp204->tl=0;_tmp204;});Cyc_Tcpat_con_pat(_tmp53B,_tmp53A,_tmp539,p);});s=_tmp53C;});
goto _LL42;}else{goto _LL45;}}else{goto _LL45;}}else{goto _LL45;}}else{goto _LL45;}}else{_LL45: _LL46:
({void*_tmp205=0U;({struct _dyneither_ptr _tmp53D=({const char*_tmp206="bad union pattern";_tag_dyneither(_tmp206,sizeof(char),18U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp53D,_tag_dyneither(_tmp205,sizeof(void*),0U));});});}_LL42:;};}
# 1110
goto _LL0;}else{goto _LL23;}}else{_LL23: _LL24:
# 1152
 goto _LL26;}case 13U: _LL1B: _tmp225=((struct Cyc_Absyn_Enum_p_Absyn_Raw_pat_struct*)_tmp1E4)->f1;_tmp224=((struct Cyc_Absyn_Enum_p_Absyn_Raw_pat_struct*)_tmp1E4)->f2;_LL1C:
# 1115
{void*_tmp20A=Cyc_Tcutil_compress((void*)_check_null(p->topt));void*_tmp20B=_tmp20A;if(((struct Cyc_Absyn_IntType_Absyn_Type_struct*)_tmp20B)->tag == 6U){_LL48: _LL49:
# 1119
({void*_tmp53E=Cyc_Tcpat_con_pat(*(*_tmp224->name).f2,0,0,p);s=_tmp53E;});
goto _LL47;}else{_LL4A: _LL4B: {
# 1122
int span=((int(*)(struct Cyc_List_List*x))Cyc_List_length)((struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(_tmp225->fields))->v);
({void*_tmp541=({struct _dyneither_ptr _tmp540=*(*_tmp224->name).f2;int*_tmp53F=({int*_tmp20C=_cycalloc_atomic(sizeof(*_tmp20C));_tmp20C[0]=span;_tmp20C;});Cyc_Tcpat_con_pat(_tmp540,_tmp53F,0,p);});s=_tmp541;});
goto _LL47;}}_LL47:;}
# 1126
goto _LL0;case 14U: _LL1D: _tmp227=(void*)((struct Cyc_Absyn_AnonEnum_p_Absyn_Raw_pat_struct*)_tmp1E4)->f1;_tmp226=((struct Cyc_Absyn_AnonEnum_p_Absyn_Raw_pat_struct*)_tmp1E4)->f2;_LL1E: {
# 1131
struct Cyc_List_List*fields;
{void*_tmp20D=Cyc_Tcutil_compress(_tmp227);void*_tmp20E=_tmp20D;struct Cyc_List_List*_tmp211;if(((struct Cyc_Absyn_AnonEnumType_Absyn_Type_struct*)_tmp20E)->tag == 14U){_LL4D: _tmp211=((struct Cyc_Absyn_AnonEnumType_Absyn_Type_struct*)_tmp20E)->f1;_LL4E:
 fields=_tmp211;goto _LL4C;}else{_LL4F: _LL50:
({void*_tmp20F=0U;({struct _dyneither_ptr _tmp542=({const char*_tmp210="bad type in AnonEnum_p";_tag_dyneither(_tmp210,sizeof(char),23U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp542,_tag_dyneither(_tmp20F,sizeof(void*),0U));});});}_LL4C:;}
# 1138
{void*_tmp212=Cyc_Tcutil_compress((void*)_check_null(p->topt));void*_tmp213=_tmp212;if(((struct Cyc_Absyn_IntType_Absyn_Type_struct*)_tmp213)->tag == 6U){_LL52: _LL53:
# 1142
({void*_tmp543=Cyc_Tcpat_con_pat(*(*_tmp226->name).f2,0,0,p);s=_tmp543;});
goto _LL51;}else{_LL54: _LL55: {
# 1145
int span=((int(*)(struct Cyc_List_List*x))Cyc_List_length)(fields);
({void*_tmp546=({struct _dyneither_ptr _tmp545=*(*_tmp226->name).f2;int*_tmp544=({int*_tmp214=_cycalloc_atomic(sizeof(*_tmp214));_tmp214[0]=span;_tmp214;});Cyc_Tcpat_con_pat(_tmp545,_tmp544,0,p);});s=_tmp546;});
goto _LL51;}}_LL51:;}
# 1149
goto _LL0;}case 15U: _LL1F: _LL20:
 goto _LL22;case 16U: _LL21: _LL22:
 goto _LL24;default: _LL25: _LL26:
# 1153
({void*_tmp548=(void*)({struct Cyc_Tcpat_Any_Tcpat_Simple_pat_struct*_tmp215=_cycalloc_atomic(sizeof(*_tmp215));({struct Cyc_Tcpat_Any_Tcpat_Simple_pat_struct _tmp547=({struct Cyc_Tcpat_Any_Tcpat_Simple_pat_struct _tmp216;_tmp216.tag=0U;_tmp216;});_tmp215[0]=_tmp547;});_tmp215;});s=_tmp548;});}_LL0:;}
# 1155
return s;}struct Cyc_Tcpat_Pos_Tcpat_Term_desc_struct{int tag;struct Cyc_Tcpat_Con_s*f1;struct Cyc_List_List*f2;};struct Cyc_Tcpat_Neg_Tcpat_Term_desc_struct{int tag;struct Cyc_Set_Set*f1;};
# 1184
static int Cyc_Tcpat_same_access(void*a1,void*a2){
struct _tuple0 _tmp228=({struct _tuple0 _tmp234;_tmp234.f1=a1;_tmp234.f2=a2;_tmp234;});struct _tuple0 _tmp229=_tmp228;int _tmp233;struct _dyneither_ptr*_tmp232;int _tmp231;struct _dyneither_ptr*_tmp230;struct Cyc_Absyn_Datatypefield*_tmp22F;unsigned int _tmp22E;struct Cyc_Absyn_Datatypefield*_tmp22D;unsigned int _tmp22C;unsigned int _tmp22B;unsigned int _tmp22A;switch(*((int*)_tmp229.f1)){case 0U: if(((struct Cyc_Tcpat_Dummy_Tcpat_Access_struct*)_tmp229.f2)->tag == 0U){_LL1: _LL2:
 return 1;}else{goto _LLB;}case 1U: if(((struct Cyc_Tcpat_Deref_Tcpat_Access_struct*)_tmp229.f2)->tag == 1U){_LL3: _LL4:
 return 1;}else{goto _LLB;}case 2U: if(((struct Cyc_Tcpat_TupleField_Tcpat_Access_struct*)_tmp229.f2)->tag == 2U){_LL5: _tmp22B=((struct Cyc_Tcpat_TupleField_Tcpat_Access_struct*)_tmp229.f1)->f1;_tmp22A=((struct Cyc_Tcpat_TupleField_Tcpat_Access_struct*)_tmp229.f2)->f1;_LL6:
 return _tmp22B == _tmp22A;}else{goto _LLB;}case 3U: if(((struct Cyc_Tcpat_DatatypeField_Tcpat_Access_struct*)_tmp229.f2)->tag == 3U){_LL7: _tmp22F=((struct Cyc_Tcpat_DatatypeField_Tcpat_Access_struct*)_tmp229.f1)->f2;_tmp22E=((struct Cyc_Tcpat_DatatypeField_Tcpat_Access_struct*)_tmp229.f1)->f3;_tmp22D=((struct Cyc_Tcpat_DatatypeField_Tcpat_Access_struct*)_tmp229.f2)->f2;_tmp22C=((struct Cyc_Tcpat_DatatypeField_Tcpat_Access_struct*)_tmp229.f2)->f3;_LL8:
# 1190
 return _tmp22F == _tmp22D  && _tmp22E == _tmp22C;}else{goto _LLB;}default: if(((struct Cyc_Tcpat_AggrField_Tcpat_Access_struct*)_tmp229.f2)->tag == 4U){_LL9: _tmp233=((struct Cyc_Tcpat_AggrField_Tcpat_Access_struct*)_tmp229.f1)->f1;_tmp232=((struct Cyc_Tcpat_AggrField_Tcpat_Access_struct*)_tmp229.f1)->f2;_tmp231=((struct Cyc_Tcpat_AggrField_Tcpat_Access_struct*)_tmp229.f2)->f1;_tmp230=((struct Cyc_Tcpat_AggrField_Tcpat_Access_struct*)_tmp229.f2)->f2;_LLA:
# 1192
 return _tmp233 == _tmp231  && Cyc_strptrcmp(_tmp232,_tmp230)== 0;}else{_LLB: _LLC:
 return 0;}}_LL0:;}
# 1197
static int Cyc_Tcpat_same_path(struct Cyc_List_List*p1,struct Cyc_List_List*p2){
while(p1 != 0  && p2 != 0){
if(!Cyc_Tcpat_same_access(((struct Cyc_Tcpat_PathNode*)p1->hd)->access,((struct Cyc_Tcpat_PathNode*)p2->hd)->access))return 0;
p1=p1->tl;
p2=p2->tl;}
# 1203
if(p1 != p2)return 0;
return 1;}
# 1207
static void*Cyc_Tcpat_ifeq(struct Cyc_List_List*access,struct Cyc_Tcpat_Con_s*con,void*d1,void*d2){
void*_tmp235=Cyc_Tcpat_get_pat_test(con->orig_pat);
{void*_tmp236=d2;struct Cyc_List_List*_tmp23D;struct Cyc_List_List*_tmp23C;void*_tmp23B;if(((struct Cyc_Tcpat_SwitchDec_Tcpat_Decision_struct*)_tmp236)->tag == 2U){_LL1: _tmp23D=((struct Cyc_Tcpat_SwitchDec_Tcpat_Decision_struct*)_tmp236)->f1;_tmp23C=((struct Cyc_Tcpat_SwitchDec_Tcpat_Decision_struct*)_tmp236)->f2;_tmp23B=(void*)((struct Cyc_Tcpat_SwitchDec_Tcpat_Decision_struct*)_tmp236)->f3;_LL2:
# 1211
 if(Cyc_Tcpat_same_path(access,_tmp23D) && (int)(((con->orig_pat).pattern).tag == 1))
return(void*)({struct Cyc_Tcpat_SwitchDec_Tcpat_Decision_struct*_tmp237=_cycalloc(sizeof(*_tmp237));({struct Cyc_Tcpat_SwitchDec_Tcpat_Decision_struct _tmp54B=({struct Cyc_Tcpat_SwitchDec_Tcpat_Decision_struct _tmp238;_tmp238.tag=2U;_tmp238.f1=_tmp23D;({struct Cyc_List_List*_tmp54A=({struct Cyc_List_List*_tmp239=_cycalloc(sizeof(*_tmp239));({struct _tuple0*_tmp549=({struct _tuple0*_tmp23A=_cycalloc(sizeof(*_tmp23A));_tmp23A->f1=_tmp235;_tmp23A->f2=d1;_tmp23A;});_tmp239->hd=_tmp549;});_tmp239->tl=_tmp23C;_tmp239;});_tmp238.f2=_tmp54A;});_tmp238.f3=_tmp23B;_tmp238;});_tmp237[0]=_tmp54B;});_tmp237;});else{
# 1215
goto _LL0;}}else{_LL3: _LL4:
 goto _LL0;}_LL0:;}
# 1218
return(void*)({struct Cyc_Tcpat_SwitchDec_Tcpat_Decision_struct*_tmp23E=_cycalloc(sizeof(*_tmp23E));({struct Cyc_Tcpat_SwitchDec_Tcpat_Decision_struct _tmp54E=({struct Cyc_Tcpat_SwitchDec_Tcpat_Decision_struct _tmp23F;_tmp23F.tag=2U;_tmp23F.f1=access;({struct Cyc_List_List*_tmp54D=({struct Cyc_List_List*_tmp240=_cycalloc(sizeof(*_tmp240));({struct _tuple0*_tmp54C=({struct _tuple0*_tmp241=_cycalloc(sizeof(*_tmp241));_tmp241->f1=_tmp235;_tmp241->f2=d1;_tmp241;});_tmp240->hd=_tmp54C;});_tmp240->tl=0;_tmp240;});_tmp23F.f2=_tmp54D;});_tmp23F.f3=d2;_tmp23F;});_tmp23E[0]=_tmp54E;});_tmp23E;});}struct _tuple20{struct Cyc_List_List*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;};
# 1229
enum Cyc_Tcpat_Answer{Cyc_Tcpat_Yes  = 0U,Cyc_Tcpat_No  = 1U,Cyc_Tcpat_Maybe  = 2U};
# 1232
static void Cyc_Tcpat_print_tab(int i){
for(0;i != 0;-- i){
({void*_tmp242=0U;({struct Cyc___cycFILE*_tmp550=Cyc_stderr;struct _dyneither_ptr _tmp54F=({const char*_tmp243=" ";_tag_dyneither(_tmp243,sizeof(char),2U);});Cyc_fprintf(_tmp550,_tmp54F,_tag_dyneither(_tmp242,sizeof(void*),0U));});});}}
# 1238
static void Cyc_Tcpat_print_con(struct Cyc_Tcpat_Con_s*c){
union Cyc_Tcpat_Name_value _tmp244=c->name;
union Cyc_Tcpat_Name_value _tmp245=_tmp244;int _tmp24D;struct _dyneither_ptr _tmp24C;if((_tmp245.Name_v).tag == 1){_LL1: _tmp24C=(_tmp245.Name_v).val;_LL2:
({struct Cyc_String_pa_PrintArg_struct _tmp248;_tmp248.tag=0U;_tmp248.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp24C);({void*_tmp246[1U]={& _tmp248};({struct Cyc___cycFILE*_tmp552=Cyc_stderr;struct _dyneither_ptr _tmp551=({const char*_tmp247="%s";_tag_dyneither(_tmp247,sizeof(char),3U);});Cyc_fprintf(_tmp552,_tmp551,_tag_dyneither(_tmp246,sizeof(void*),1U));});});});goto _LL0;}else{_LL3: _tmp24D=(_tmp245.Int_v).val;_LL4:
({struct Cyc_Int_pa_PrintArg_struct _tmp24B;_tmp24B.tag=1U;_tmp24B.f1=(unsigned long)_tmp24D;({void*_tmp249[1U]={& _tmp24B};({struct Cyc___cycFILE*_tmp554=Cyc_stderr;struct _dyneither_ptr _tmp553=({const char*_tmp24A="%d";_tag_dyneither(_tmp24A,sizeof(char),3U);});Cyc_fprintf(_tmp554,_tmp553,_tag_dyneither(_tmp249,sizeof(void*),1U));});});});goto _LL0;}_LL0:;}
# 1246
static void Cyc_Tcpat_print_access(void*a){
void*_tmp24E=a;int _tmp264;struct _dyneither_ptr*_tmp263;struct Cyc_Absyn_Datatypefield*_tmp262;unsigned int _tmp261;unsigned int _tmp260;switch(*((int*)_tmp24E)){case 0U: _LL1: _LL2:
({void*_tmp24F=0U;({struct Cyc___cycFILE*_tmp556=Cyc_stderr;struct _dyneither_ptr _tmp555=({const char*_tmp250="DUMMY";_tag_dyneither(_tmp250,sizeof(char),6U);});Cyc_fprintf(_tmp556,_tmp555,_tag_dyneither(_tmp24F,sizeof(void*),0U));});});goto _LL0;case 1U: _LL3: _LL4:
({void*_tmp251=0U;({struct Cyc___cycFILE*_tmp558=Cyc_stderr;struct _dyneither_ptr _tmp557=({const char*_tmp252="*";_tag_dyneither(_tmp252,sizeof(char),2U);});Cyc_fprintf(_tmp558,_tmp557,_tag_dyneither(_tmp251,sizeof(void*),0U));});});goto _LL0;case 2U: _LL5: _tmp260=((struct Cyc_Tcpat_TupleField_Tcpat_Access_struct*)_tmp24E)->f1;_LL6:
({struct Cyc_Int_pa_PrintArg_struct _tmp255;_tmp255.tag=1U;_tmp255.f1=(unsigned long)((int)_tmp260);({void*_tmp253[1U]={& _tmp255};({struct Cyc___cycFILE*_tmp55A=Cyc_stderr;struct _dyneither_ptr _tmp559=({const char*_tmp254="[%d]";_tag_dyneither(_tmp254,sizeof(char),5U);});Cyc_fprintf(_tmp55A,_tmp559,_tag_dyneither(_tmp253,sizeof(void*),1U));});});});goto _LL0;case 3U: _LL7: _tmp262=((struct Cyc_Tcpat_DatatypeField_Tcpat_Access_struct*)_tmp24E)->f2;_tmp261=((struct Cyc_Tcpat_DatatypeField_Tcpat_Access_struct*)_tmp24E)->f3;_LL8:
# 1252
({struct Cyc_Int_pa_PrintArg_struct _tmp259;_tmp259.tag=1U;_tmp259.f1=(unsigned long)((int)_tmp261);({struct Cyc_String_pa_PrintArg_struct _tmp258;_tmp258.tag=0U;({struct _dyneither_ptr _tmp55B=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(_tmp262->name));_tmp258.f1=_tmp55B;});({void*_tmp256[2U]={& _tmp258,& _tmp259};({struct Cyc___cycFILE*_tmp55D=Cyc_stderr;struct _dyneither_ptr _tmp55C=({const char*_tmp257="%s[%d]";_tag_dyneither(_tmp257,sizeof(char),7U);});Cyc_fprintf(_tmp55D,_tmp55C,_tag_dyneither(_tmp256,sizeof(void*),2U));});});});});goto _LL0;default: _LL9: _tmp264=((struct Cyc_Tcpat_AggrField_Tcpat_Access_struct*)_tmp24E)->f1;_tmp263=((struct Cyc_Tcpat_AggrField_Tcpat_Access_struct*)_tmp24E)->f2;_LLA:
# 1254
 if(_tmp264)
({struct Cyc_String_pa_PrintArg_struct _tmp25C;_tmp25C.tag=0U;_tmp25C.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*_tmp263);({void*_tmp25A[1U]={& _tmp25C};({struct Cyc___cycFILE*_tmp55F=Cyc_stderr;struct _dyneither_ptr _tmp55E=({const char*_tmp25B=".tagunion.%s";_tag_dyneither(_tmp25B,sizeof(char),13U);});Cyc_fprintf(_tmp55F,_tmp55E,_tag_dyneither(_tmp25A,sizeof(void*),1U));});});});else{
# 1257
({struct Cyc_String_pa_PrintArg_struct _tmp25F;_tmp25F.tag=0U;_tmp25F.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*_tmp263);({void*_tmp25D[1U]={& _tmp25F};({struct Cyc___cycFILE*_tmp561=Cyc_stderr;struct _dyneither_ptr _tmp560=({const char*_tmp25E=".%s";_tag_dyneither(_tmp25E,sizeof(char),4U);});Cyc_fprintf(_tmp561,_tmp560,_tag_dyneither(_tmp25D,sizeof(void*),1U));});});});}
goto _LL0;}_LL0:;}
# 1262
static void Cyc_Tcpat_print_pat_test(void*p){
void*_tmp265=p;struct Cyc_Absyn_Datatypefield*_tmp28A;struct _dyneither_ptr*_tmp289;int _tmp288;int _tmp287;unsigned int _tmp286;struct _dyneither_ptr _tmp285;struct Cyc_Absyn_Enumfield*_tmp284;struct Cyc_Absyn_Enumfield*_tmp283;struct Cyc_Absyn_Exp*_tmp282;switch(*((int*)_tmp265)){case 0U: if(((struct Cyc_Tcpat_WhereTest_Tcpat_PatTest_struct*)_tmp265)->f1 == 0){_LL1: _LL2:
({void*_tmp266=0U;({struct Cyc___cycFILE*_tmp563=Cyc_stderr;struct _dyneither_ptr _tmp562=({const char*_tmp267="where(NULL)";_tag_dyneither(_tmp267,sizeof(char),12U);});Cyc_fprintf(_tmp563,_tmp562,_tag_dyneither(_tmp266,sizeof(void*),0U));});});goto _LL0;}else{_LL3: _tmp282=((struct Cyc_Tcpat_WhereTest_Tcpat_PatTest_struct*)_tmp265)->f1;_LL4:
({struct Cyc_String_pa_PrintArg_struct _tmp26A;_tmp26A.tag=0U;({struct _dyneither_ptr _tmp564=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_exp2string((struct Cyc_Absyn_Exp*)_check_null(_tmp282)));_tmp26A.f1=_tmp564;});({void*_tmp268[1U]={& _tmp26A};({struct Cyc___cycFILE*_tmp566=Cyc_stderr;struct _dyneither_ptr _tmp565=({const char*_tmp269="where(%s)";_tag_dyneither(_tmp269,sizeof(char),10U);});Cyc_fprintf(_tmp566,_tmp565,_tag_dyneither(_tmp268,sizeof(void*),1U));});});});goto _LL0;}case 1U: _LL5: _LL6:
({void*_tmp26B=0U;({struct Cyc___cycFILE*_tmp568=Cyc_stderr;struct _dyneither_ptr _tmp567=({const char*_tmp26C="NULL";_tag_dyneither(_tmp26C,sizeof(char),5U);});Cyc_fprintf(_tmp568,_tmp567,_tag_dyneither(_tmp26B,sizeof(void*),0U));});});goto _LL0;case 2U: _LL7: _LL8:
({void*_tmp26D=0U;({struct Cyc___cycFILE*_tmp56A=Cyc_stderr;struct _dyneither_ptr _tmp569=({const char*_tmp26E="NOT-NULL:";_tag_dyneither(_tmp26E,sizeof(char),10U);});Cyc_fprintf(_tmp56A,_tmp569,_tag_dyneither(_tmp26D,sizeof(void*),0U));});});goto _LL0;case 4U: _LL9: _tmp283=((struct Cyc_Tcpat_EqAnonEnum_Tcpat_PatTest_struct*)_tmp265)->f2;_LLA:
 _tmp284=_tmp283;goto _LLC;case 3U: _LLB: _tmp284=((struct Cyc_Tcpat_EqEnum_Tcpat_PatTest_struct*)_tmp265)->f2;_LLC:
({struct Cyc_String_pa_PrintArg_struct _tmp271;_tmp271.tag=0U;({struct _dyneither_ptr _tmp56B=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(_tmp284->name));_tmp271.f1=_tmp56B;});({void*_tmp26F[1U]={& _tmp271};({struct Cyc___cycFILE*_tmp56D=Cyc_stderr;struct _dyneither_ptr _tmp56C=({const char*_tmp270="%s";_tag_dyneither(_tmp270,sizeof(char),3U);});Cyc_fprintf(_tmp56D,_tmp56C,_tag_dyneither(_tmp26F,sizeof(void*),1U));});});});goto _LL0;case 5U: _LLD: _tmp285=((struct Cyc_Tcpat_EqFloat_Tcpat_PatTest_struct*)_tmp265)->f1;_LLE:
({struct Cyc_String_pa_PrintArg_struct _tmp274;_tmp274.tag=0U;_tmp274.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp285);({void*_tmp272[1U]={& _tmp274};({struct Cyc___cycFILE*_tmp56F=Cyc_stderr;struct _dyneither_ptr _tmp56E=({const char*_tmp273="%s";_tag_dyneither(_tmp273,sizeof(char),3U);});Cyc_fprintf(_tmp56F,_tmp56E,_tag_dyneither(_tmp272,sizeof(void*),1U));});});});goto _LL0;case 6U: _LLF: _tmp286=((struct Cyc_Tcpat_EqConst_Tcpat_PatTest_struct*)_tmp265)->f1;_LL10:
({struct Cyc_Int_pa_PrintArg_struct _tmp277;_tmp277.tag=1U;_tmp277.f1=(unsigned long)((int)_tmp286);({void*_tmp275[1U]={& _tmp277};({struct Cyc___cycFILE*_tmp571=Cyc_stderr;struct _dyneither_ptr _tmp570=({const char*_tmp276="%d";_tag_dyneither(_tmp276,sizeof(char),3U);});Cyc_fprintf(_tmp571,_tmp570,_tag_dyneither(_tmp275,sizeof(void*),1U));});});});goto _LL0;case 7U: _LL11: _tmp287=((struct Cyc_Tcpat_EqDatatypeTag_Tcpat_PatTest_struct*)_tmp265)->f1;_LL12:
({struct Cyc_Int_pa_PrintArg_struct _tmp27A;_tmp27A.tag=1U;_tmp27A.f1=(unsigned long)_tmp287;({void*_tmp278[1U]={& _tmp27A};({struct Cyc___cycFILE*_tmp573=Cyc_stderr;struct _dyneither_ptr _tmp572=({const char*_tmp279="datatypetag(%d)";_tag_dyneither(_tmp279,sizeof(char),16U);});Cyc_fprintf(_tmp573,_tmp572,_tag_dyneither(_tmp278,sizeof(void*),1U));});});});goto _LL0;case 8U: _LL13: _tmp289=((struct Cyc_Tcpat_EqTaggedUnion_Tcpat_PatTest_struct*)_tmp265)->f1;_tmp288=((struct Cyc_Tcpat_EqTaggedUnion_Tcpat_PatTest_struct*)_tmp265)->f2;_LL14:
({struct Cyc_Int_pa_PrintArg_struct _tmp27E;_tmp27E.tag=1U;_tmp27E.f1=(unsigned long)_tmp288;({struct Cyc_String_pa_PrintArg_struct _tmp27D;_tmp27D.tag=0U;_tmp27D.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*_tmp289);({void*_tmp27B[2U]={& _tmp27D,& _tmp27E};({struct Cyc___cycFILE*_tmp575=Cyc_stderr;struct _dyneither_ptr _tmp574=({const char*_tmp27C="uniontag[%s](%d)";_tag_dyneither(_tmp27C,sizeof(char),17U);});Cyc_fprintf(_tmp575,_tmp574,_tag_dyneither(_tmp27B,sizeof(void*),2U));});});});});goto _LL0;default: _LL15: _tmp28A=((struct Cyc_Tcpat_EqExtensibleDatatype_Tcpat_PatTest_struct*)_tmp265)->f2;_LL16:
# 1275
({struct Cyc_String_pa_PrintArg_struct _tmp281;_tmp281.tag=0U;({struct _dyneither_ptr _tmp576=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(_tmp28A->name));_tmp281.f1=_tmp576;});({void*_tmp27F[1U]={& _tmp281};({struct Cyc___cycFILE*_tmp578=Cyc_stderr;struct _dyneither_ptr _tmp577=({const char*_tmp280="datatypefield(%s)";_tag_dyneither(_tmp280,sizeof(char),18U);});Cyc_fprintf(_tmp578,_tmp577,_tag_dyneither(_tmp27F,sizeof(void*),1U));});});});}_LL0:;}
# 1279
static void Cyc_Tcpat_print_rhs(struct Cyc_Tcpat_Rhs*r){
({struct Cyc_String_pa_PrintArg_struct _tmp28D;_tmp28D.tag=0U;({struct _dyneither_ptr _tmp579=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_stmt2string(r->rhs));_tmp28D.f1=_tmp579;});({void*_tmp28B[1U]={& _tmp28D};({struct Cyc___cycFILE*_tmp57B=Cyc_stderr;struct _dyneither_ptr _tmp57A=({const char*_tmp28C="%s";_tag_dyneither(_tmp28C,sizeof(char),3U);});Cyc_fprintf(_tmp57B,_tmp57A,_tag_dyneither(_tmp28B,sizeof(void*),1U));});});});}
# 1283
static void Cyc_Tcpat_print_dec_tree(void*d,int tab){
void*_tmp28E=d;struct Cyc_List_List*_tmp2AA;struct Cyc_List_List*_tmp2A9;void*_tmp2A8;struct Cyc_Tcpat_Rhs*_tmp2A7;switch(*((int*)_tmp28E)){case 1U: _LL1: _tmp2A7=((struct Cyc_Tcpat_Success_Tcpat_Decision_struct*)_tmp28E)->f1;_LL2:
# 1286
 Cyc_Tcpat_print_tab(tab);
({void*_tmp28F=0U;({struct Cyc___cycFILE*_tmp57D=Cyc_stderr;struct _dyneither_ptr _tmp57C=({const char*_tmp290="Success(";_tag_dyneither(_tmp290,sizeof(char),9U);});Cyc_fprintf(_tmp57D,_tmp57C,_tag_dyneither(_tmp28F,sizeof(void*),0U));});});Cyc_Tcpat_print_rhs(_tmp2A7);({void*_tmp291=0U;({struct Cyc___cycFILE*_tmp57F=Cyc_stderr;struct _dyneither_ptr _tmp57E=({const char*_tmp292=")\n";_tag_dyneither(_tmp292,sizeof(char),3U);});Cyc_fprintf(_tmp57F,_tmp57E,_tag_dyneither(_tmp291,sizeof(void*),0U));});});
goto _LL0;case 0U: _LL3: _LL4:
({void*_tmp293=0U;({struct Cyc___cycFILE*_tmp581=Cyc_stderr;struct _dyneither_ptr _tmp580=({const char*_tmp294="Failure\n";_tag_dyneither(_tmp294,sizeof(char),9U);});Cyc_fprintf(_tmp581,_tmp580,_tag_dyneither(_tmp293,sizeof(void*),0U));});});goto _LL0;default: _LL5: _tmp2AA=((struct Cyc_Tcpat_SwitchDec_Tcpat_Decision_struct*)_tmp28E)->f1;_tmp2A9=((struct Cyc_Tcpat_SwitchDec_Tcpat_Decision_struct*)_tmp28E)->f2;_tmp2A8=(void*)((struct Cyc_Tcpat_SwitchDec_Tcpat_Decision_struct*)_tmp28E)->f3;_LL6:
# 1291
 Cyc_Tcpat_print_tab(tab);
({void*_tmp295=0U;({struct Cyc___cycFILE*_tmp583=Cyc_stderr;struct _dyneither_ptr _tmp582=({const char*_tmp296="Switch[";_tag_dyneither(_tmp296,sizeof(char),8U);});Cyc_fprintf(_tmp583,_tmp582,_tag_dyneither(_tmp295,sizeof(void*),0U));});});
for(0;_tmp2AA != 0;_tmp2AA=_tmp2AA->tl){
Cyc_Tcpat_print_access(((struct Cyc_Tcpat_PathNode*)_tmp2AA->hd)->access);
if(_tmp2AA->tl != 0)({void*_tmp297=0U;({struct Cyc___cycFILE*_tmp585=Cyc_stderr;struct _dyneither_ptr _tmp584=({const char*_tmp298=",";_tag_dyneither(_tmp298,sizeof(char),2U);});Cyc_fprintf(_tmp585,_tmp584,_tag_dyneither(_tmp297,sizeof(void*),0U));});});}
# 1297
({void*_tmp299=0U;({struct Cyc___cycFILE*_tmp587=Cyc_stderr;struct _dyneither_ptr _tmp586=({const char*_tmp29A="] {\n";_tag_dyneither(_tmp29A,sizeof(char),5U);});Cyc_fprintf(_tmp587,_tmp586,_tag_dyneither(_tmp299,sizeof(void*),0U));});});
for(0;_tmp2A9 != 0;_tmp2A9=_tmp2A9->tl){
struct _tuple0 _tmp29B=*((struct _tuple0*)_tmp2A9->hd);struct _tuple0 _tmp29C=_tmp29B;void*_tmp2A2;void*_tmp2A1;_LL8: _tmp2A2=_tmp29C.f1;_tmp2A1=_tmp29C.f2;_LL9:;
Cyc_Tcpat_print_tab(tab);
({void*_tmp29D=0U;({struct Cyc___cycFILE*_tmp589=Cyc_stderr;struct _dyneither_ptr _tmp588=({const char*_tmp29E="case ";_tag_dyneither(_tmp29E,sizeof(char),6U);});Cyc_fprintf(_tmp589,_tmp588,_tag_dyneither(_tmp29D,sizeof(void*),0U));});});
Cyc_Tcpat_print_pat_test(_tmp2A2);
({void*_tmp29F=0U;({struct Cyc___cycFILE*_tmp58B=Cyc_stderr;struct _dyneither_ptr _tmp58A=({const char*_tmp2A0=":\n";_tag_dyneither(_tmp2A0,sizeof(char),3U);});Cyc_fprintf(_tmp58B,_tmp58A,_tag_dyneither(_tmp29F,sizeof(void*),0U));});});
Cyc_Tcpat_print_dec_tree(_tmp2A1,tab + 7);}
# 1306
Cyc_Tcpat_print_tab(tab);
({void*_tmp2A3=0U;({struct Cyc___cycFILE*_tmp58D=Cyc_stderr;struct _dyneither_ptr _tmp58C=({const char*_tmp2A4="default:\n";_tag_dyneither(_tmp2A4,sizeof(char),10U);});Cyc_fprintf(_tmp58D,_tmp58C,_tag_dyneither(_tmp2A3,sizeof(void*),0U));});});
Cyc_Tcpat_print_dec_tree(_tmp2A8,tab + 7);
Cyc_Tcpat_print_tab(tab);
({void*_tmp2A5=0U;({struct Cyc___cycFILE*_tmp58F=Cyc_stderr;struct _dyneither_ptr _tmp58E=({const char*_tmp2A6="}\n";_tag_dyneither(_tmp2A6,sizeof(char),3U);});Cyc_fprintf(_tmp58F,_tmp58E,_tag_dyneither(_tmp2A5,sizeof(void*),0U));});});}_LL0:;}
# 1314
void Cyc_Tcpat_print_decision_tree(void*d){
Cyc_Tcpat_print_dec_tree(d,0);}
# 1322
static void*Cyc_Tcpat_add_neg(void*td,struct Cyc_Tcpat_Con_s*c){
void*_tmp2AB=td;struct Cyc_Set_Set*_tmp2B0;if(((struct Cyc_Tcpat_Neg_Tcpat_Term_desc_struct*)_tmp2AB)->tag == 1U){_LL1: _tmp2B0=((struct Cyc_Tcpat_Neg_Tcpat_Term_desc_struct*)_tmp2AB)->f1;_LL2:
# 1332
 return(void*)({struct Cyc_Tcpat_Neg_Tcpat_Term_desc_struct*_tmp2AC=_cycalloc(sizeof(*_tmp2AC));({struct Cyc_Tcpat_Neg_Tcpat_Term_desc_struct _tmp591=({struct Cyc_Tcpat_Neg_Tcpat_Term_desc_struct _tmp2AD;_tmp2AD.tag=1U;({struct Cyc_Set_Set*_tmp590=((struct Cyc_Set_Set*(*)(struct Cyc_Set_Set*s,struct Cyc_Tcpat_Con_s*elt))Cyc_Set_insert)(_tmp2B0,c);_tmp2AD.f1=_tmp590;});_tmp2AD;});_tmp2AC[0]=_tmp591;});_tmp2AC;});}else{_LL3: _LL4:
({void*_tmp2AE=0U;({struct _dyneither_ptr _tmp592=({const char*_tmp2AF="add_neg called when td is Positive";_tag_dyneither(_tmp2AF,sizeof(char),35U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp592,_tag_dyneither(_tmp2AE,sizeof(void*),0U));});});}_LL0:;}
# 1339
static enum Cyc_Tcpat_Answer Cyc_Tcpat_static_match(struct Cyc_Tcpat_Con_s*c,void*td){
void*_tmp2B1=td;struct Cyc_Set_Set*_tmp2B3;struct Cyc_Tcpat_Con_s*_tmp2B2;if(((struct Cyc_Tcpat_Pos_Tcpat_Term_desc_struct*)_tmp2B1)->tag == 0U){_LL1: _tmp2B2=((struct Cyc_Tcpat_Pos_Tcpat_Term_desc_struct*)_tmp2B1)->f1;_LL2:
# 1343
 if(Cyc_Tcpat_compare_con(c,_tmp2B2)== 0)return Cyc_Tcpat_Yes;else{
return Cyc_Tcpat_No;}}else{_LL3: _tmp2B3=((struct Cyc_Tcpat_Neg_Tcpat_Term_desc_struct*)_tmp2B1)->f1;_LL4:
# 1347
 if(((int(*)(struct Cyc_Set_Set*s,struct Cyc_Tcpat_Con_s*elt))Cyc_Set_member)(_tmp2B3,c))return Cyc_Tcpat_No;else{
# 1350
if(c->span != 0  && ({int _tmp593=*((int*)_check_null(c->span));_tmp593 == ((int(*)(struct Cyc_Set_Set*s))Cyc_Set_cardinality)(_tmp2B3)+ 1;}))
return Cyc_Tcpat_Yes;else{
# 1353
return Cyc_Tcpat_Maybe;}}}_LL0:;}struct _tuple21{struct Cyc_Tcpat_Con_s*f1;struct Cyc_List_List*f2;};
# 1361
static struct Cyc_List_List*Cyc_Tcpat_augment(struct Cyc_List_List*ctxt,void*dsc){
struct Cyc_List_List*_tmp2B4=ctxt;struct Cyc_Tcpat_Con_s*_tmp2BA;struct Cyc_List_List*_tmp2B9;struct Cyc_List_List*_tmp2B8;if(_tmp2B4 == 0){_LL1: _LL2:
 return 0;}else{_LL3: _tmp2BA=((struct _tuple21*)_tmp2B4->hd)->f1;_tmp2B9=((struct _tuple21*)_tmp2B4->hd)->f2;_tmp2B8=_tmp2B4->tl;_LL4:
# 1365
 return({struct Cyc_List_List*_tmp2B5=_cycalloc(sizeof(*_tmp2B5));({struct _tuple21*_tmp595=({struct _tuple21*_tmp2B6=_cycalloc(sizeof(*_tmp2B6));_tmp2B6->f1=_tmp2BA;({struct Cyc_List_List*_tmp594=({struct Cyc_List_List*_tmp2B7=_cycalloc(sizeof(*_tmp2B7));_tmp2B7->hd=dsc;_tmp2B7->tl=_tmp2B9;_tmp2B7;});_tmp2B6->f2=_tmp594;});_tmp2B6;});_tmp2B5->hd=_tmp595;});_tmp2B5->tl=_tmp2B8;_tmp2B5;});}_LL0:;}
# 1373
static struct Cyc_List_List*Cyc_Tcpat_norm_context(struct Cyc_List_List*ctxt){
struct Cyc_List_List*_tmp2BB=ctxt;struct Cyc_Tcpat_Con_s*_tmp2C2;struct Cyc_List_List*_tmp2C1;struct Cyc_List_List*_tmp2C0;if(_tmp2BB == 0){_LL1: _LL2:
({void*_tmp2BC=0U;({struct _dyneither_ptr _tmp596=({const char*_tmp2BD="norm_context: empty context";_tag_dyneither(_tmp2BD,sizeof(char),28U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp596,_tag_dyneither(_tmp2BC,sizeof(void*),0U));});});}else{_LL3: _tmp2C2=((struct _tuple21*)_tmp2BB->hd)->f1;_tmp2C1=((struct _tuple21*)_tmp2BB->hd)->f2;_tmp2C0=_tmp2BB->tl;_LL4:
# 1377
 return({struct Cyc_List_List*_tmp599=_tmp2C0;Cyc_Tcpat_augment(_tmp599,(void*)({struct Cyc_Tcpat_Pos_Tcpat_Term_desc_struct*_tmp2BE=_cycalloc(sizeof(*_tmp2BE));({struct Cyc_Tcpat_Pos_Tcpat_Term_desc_struct _tmp598=({struct Cyc_Tcpat_Pos_Tcpat_Term_desc_struct _tmp2BF;_tmp2BF.tag=0U;_tmp2BF.f1=_tmp2C2;({struct Cyc_List_List*_tmp597=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_rev)(_tmp2C1);_tmp2BF.f2=_tmp597;});_tmp2BF;});_tmp2BE[0]=_tmp598;});_tmp2BE;}));});}_LL0:;}
# 1386
static void*Cyc_Tcpat_build_desc(struct Cyc_List_List*ctxt,void*dsc,struct Cyc_List_List*work){
# 1388
struct _tuple1 _tmp2C3=({struct _tuple1 _tmp2D0;_tmp2D0.f1=ctxt;_tmp2D0.f2=work;_tmp2D0;});struct _tuple1 _tmp2C4=_tmp2C3;struct Cyc_Tcpat_Con_s*_tmp2CF;struct Cyc_List_List*_tmp2CE;struct Cyc_List_List*_tmp2CD;struct Cyc_List_List*_tmp2CC;struct Cyc_List_List*_tmp2CB;if(_tmp2C4.f1 == 0){if(_tmp2C4.f2 == 0){_LL1: _LL2:
 return dsc;}else{_LL3: _LL4:
 goto _LL6;}}else{if(_tmp2C4.f2 == 0){_LL5: _LL6:
({void*_tmp2C5=0U;({struct _dyneither_ptr _tmp59A=({const char*_tmp2C6="build_desc: ctxt and work don't match";_tag_dyneither(_tmp2C6,sizeof(char),38U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp59A,_tag_dyneither(_tmp2C5,sizeof(void*),0U));});});}else{_LL7: _tmp2CF=((struct _tuple21*)(_tmp2C4.f1)->hd)->f1;_tmp2CE=((struct _tuple21*)(_tmp2C4.f1)->hd)->f2;_tmp2CD=(_tmp2C4.f1)->tl;_tmp2CC=((struct _tuple20*)(_tmp2C4.f2)->hd)->f3;_tmp2CB=(_tmp2C4.f2)->tl;_LL8: {
# 1393
struct Cyc_Tcpat_Pos_Tcpat_Term_desc_struct*_tmp2C7=({struct Cyc_Tcpat_Pos_Tcpat_Term_desc_struct*_tmp2C8=_cycalloc(sizeof(*_tmp2C8));({struct Cyc_Tcpat_Pos_Tcpat_Term_desc_struct _tmp59D=({struct Cyc_Tcpat_Pos_Tcpat_Term_desc_struct _tmp2C9;_tmp2C9.tag=0U;_tmp2C9.f1=_tmp2CF;({struct Cyc_List_List*_tmp59C=({struct Cyc_List_List*_tmp59B=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_rev)(_tmp2CE);((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_append)(_tmp59B,({struct Cyc_List_List*_tmp2CA=_cycalloc(sizeof(*_tmp2CA));_tmp2CA->hd=dsc;_tmp2CA->tl=_tmp2CC;_tmp2CA;}));});_tmp2C9.f2=_tmp59C;});_tmp2C9;});_tmp2C8[0]=_tmp59D;});_tmp2C8;});
return Cyc_Tcpat_build_desc(_tmp2CD,(void*)_tmp2C7,_tmp2CB);}}}_LL0:;}
# 1398
static void*Cyc_Tcpat_match(void*p,struct Cyc_List_List*obj,void*dsc,struct Cyc_List_List*ctx,struct Cyc_List_List*work,struct Cyc_Tcpat_Rhs*right_hand_side,struct Cyc_List_List*rules);struct _tuple22{void*f1;struct Cyc_Tcpat_Rhs*f2;};
# 1405
static void*Cyc_Tcpat_or_match(void*dsc,struct Cyc_List_List*allmrules){
struct Cyc_List_List*_tmp2D1=allmrules;void*_tmp2D6;struct Cyc_Tcpat_Rhs*_tmp2D5;struct Cyc_List_List*_tmp2D4;if(_tmp2D1 == 0){_LL1: _LL2:
 return(void*)({struct Cyc_Tcpat_Failure_Tcpat_Decision_struct*_tmp2D2=_cycalloc(sizeof(*_tmp2D2));({struct Cyc_Tcpat_Failure_Tcpat_Decision_struct _tmp59E=({struct Cyc_Tcpat_Failure_Tcpat_Decision_struct _tmp2D3;_tmp2D3.tag=0U;_tmp2D3.f1=dsc;_tmp2D3;});_tmp2D2[0]=_tmp59E;});_tmp2D2;});}else{_LL3: _tmp2D6=((struct _tuple22*)_tmp2D1->hd)->f1;_tmp2D5=((struct _tuple22*)_tmp2D1->hd)->f2;_tmp2D4=_tmp2D1->tl;_LL4:
# 1409
 return Cyc_Tcpat_match(_tmp2D6,0,dsc,0,0,_tmp2D5,_tmp2D4);}_LL0:;}
# 1414
static void*Cyc_Tcpat_match_compile(struct Cyc_List_List*allmrules){
return({void*_tmp5A1=(void*)({struct Cyc_Tcpat_Neg_Tcpat_Term_desc_struct*_tmp2D7=_cycalloc(sizeof(*_tmp2D7));({struct Cyc_Tcpat_Neg_Tcpat_Term_desc_struct _tmp5A0=({struct Cyc_Tcpat_Neg_Tcpat_Term_desc_struct _tmp2D8;_tmp2D8.tag=1U;({struct Cyc_Set_Set*_tmp59F=Cyc_Tcpat_empty_con_set();_tmp2D8.f1=_tmp59F;});_tmp2D8;});_tmp2D7[0]=_tmp5A0;});_tmp2D7;});Cyc_Tcpat_or_match(_tmp5A1,allmrules);});}
# 1421
static void*Cyc_Tcpat_and_match(struct Cyc_List_List*ctx,struct Cyc_List_List*work,struct Cyc_Tcpat_Rhs*right_hand_side,struct Cyc_List_List*rules){
# 1424
struct Cyc_List_List*_tmp2D9=work;struct Cyc_List_List*_tmp2F1;struct Cyc_List_List*_tmp2F0;struct Cyc_List_List*_tmp2EF;struct Cyc_List_List*_tmp2EE;struct Cyc_List_List*_tmp2ED;if(_tmp2D9 == 0){_LL1: _LL2:
 return(void*)({struct Cyc_Tcpat_Success_Tcpat_Decision_struct*_tmp2DA=_cycalloc(sizeof(*_tmp2DA));({struct Cyc_Tcpat_Success_Tcpat_Decision_struct _tmp5A2=({struct Cyc_Tcpat_Success_Tcpat_Decision_struct _tmp2DB;_tmp2DB.tag=1U;_tmp2DB.f1=right_hand_side;_tmp2DB;});_tmp2DA[0]=_tmp5A2;});_tmp2DA;});}else{if(((struct _tuple20*)((struct Cyc_List_List*)_tmp2D9)->hd)->f1 == 0){if(((struct _tuple20*)((struct Cyc_List_List*)_tmp2D9)->hd)->f2 == 0){if(((struct _tuple20*)((struct Cyc_List_List*)_tmp2D9)->hd)->f3 == 0){_LL3: _tmp2ED=_tmp2D9->tl;_LL4:
# 1427
 return({struct Cyc_List_List*_tmp5A5=Cyc_Tcpat_norm_context(ctx);struct Cyc_List_List*_tmp5A4=_tmp2ED;struct Cyc_Tcpat_Rhs*_tmp5A3=right_hand_side;Cyc_Tcpat_and_match(_tmp5A5,_tmp5A4,_tmp5A3,rules);});}else{goto _LL5;}}else{goto _LL5;}}else{_LL5: _tmp2F1=((struct _tuple20*)_tmp2D9->hd)->f1;_tmp2F0=((struct _tuple20*)_tmp2D9->hd)->f2;_tmp2EF=((struct _tuple20*)_tmp2D9->hd)->f3;_tmp2EE=_tmp2D9->tl;_LL6:
# 1429
 if((_tmp2F1 == 0  || _tmp2F0 == 0) || _tmp2EF == 0)
({void*_tmp2DC=0U;({struct _dyneither_ptr _tmp5A6=({const char*_tmp2DD="tcpat:and_match: malformed work frame";_tag_dyneither(_tmp2DD,sizeof(char),38U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp5A6,_tag_dyneither(_tmp2DC,sizeof(void*),0U));});});{
struct Cyc_List_List*_tmp2DE=_tmp2F1;struct Cyc_List_List*_tmp2DF=_tmp2DE;void*_tmp2EC;struct Cyc_List_List*_tmp2EB;_LL8: _tmp2EC=(void*)_tmp2DF->hd;_tmp2EB=_tmp2DF->tl;_LL9:;{
struct Cyc_List_List*_tmp2E0=_tmp2F0;struct Cyc_List_List*_tmp2E1=_tmp2E0;struct Cyc_List_List*_tmp2EA;struct Cyc_List_List*_tmp2E9;_LLB: _tmp2EA=(struct Cyc_List_List*)_tmp2E1->hd;_tmp2E9=_tmp2E1->tl;_LLC:;{
struct Cyc_List_List*_tmp2E2=_tmp2EF;struct Cyc_List_List*_tmp2E3=_tmp2E2;void*_tmp2E8;struct Cyc_List_List*_tmp2E7;_LLE: _tmp2E8=(void*)_tmp2E3->hd;_tmp2E7=_tmp2E3->tl;_LLF:;{
struct _tuple20*_tmp2E4=({struct _tuple20*_tmp2E6=_cycalloc(sizeof(*_tmp2E6));_tmp2E6->f1=_tmp2EB;_tmp2E6->f2=_tmp2E9;_tmp2E6->f3=_tmp2E7;_tmp2E6;});
return({void*_tmp5AC=_tmp2EC;struct Cyc_List_List*_tmp5AB=_tmp2EA;void*_tmp5AA=_tmp2E8;struct Cyc_List_List*_tmp5A9=ctx;struct Cyc_List_List*_tmp5A8=({struct Cyc_List_List*_tmp2E5=_cycalloc(sizeof(*_tmp2E5));_tmp2E5->hd=_tmp2E4;_tmp2E5->tl=_tmp2EE;_tmp2E5;});struct Cyc_Tcpat_Rhs*_tmp5A7=right_hand_side;Cyc_Tcpat_match(_tmp5AC,_tmp5AB,_tmp5AA,_tmp5A9,_tmp5A8,_tmp5A7,rules);});};};};};}}_LL0:;}
# 1440
static struct Cyc_List_List*Cyc_Tcpat_getdargs(struct Cyc_Tcpat_Con_s*pcon,void*dsc){
void*_tmp2F2=dsc;struct Cyc_List_List*_tmp2F8;struct Cyc_Set_Set*_tmp2F7;if(((struct Cyc_Tcpat_Neg_Tcpat_Term_desc_struct*)_tmp2F2)->tag == 1U){_LL1: _tmp2F7=((struct Cyc_Tcpat_Neg_Tcpat_Term_desc_struct*)_tmp2F2)->f1;_LL2: {
# 1446
void*any=(void*)({struct Cyc_Tcpat_Neg_Tcpat_Term_desc_struct*_tmp2F5=_cycalloc(sizeof(*_tmp2F5));({struct Cyc_Tcpat_Neg_Tcpat_Term_desc_struct _tmp5AE=({struct Cyc_Tcpat_Neg_Tcpat_Term_desc_struct _tmp2F6;_tmp2F6.tag=1U;({struct Cyc_Set_Set*_tmp5AD=Cyc_Tcpat_empty_con_set();_tmp2F6.f1=_tmp5AD;});_tmp2F6;});_tmp2F5[0]=_tmp5AE;});_tmp2F5;});
struct Cyc_List_List*_tmp2F3=0;
{int i=0;for(0;i < pcon->arity;++ i){
({struct Cyc_List_List*_tmp5AF=({struct Cyc_List_List*_tmp2F4=_cycalloc(sizeof(*_tmp2F4));_tmp2F4->hd=any;_tmp2F4->tl=_tmp2F3;_tmp2F4;});_tmp2F3=_tmp5AF;});}}
return _tmp2F3;}}else{_LL3: _tmp2F8=((struct Cyc_Tcpat_Pos_Tcpat_Term_desc_struct*)_tmp2F2)->f2;_LL4:
 return _tmp2F8;}_LL0:;}
# 1455
static void*Cyc_Tcpat_get_access(union Cyc_Tcpat_PatOrWhere pw,int i){
union Cyc_Tcpat_PatOrWhere _tmp2F9=pw;struct Cyc_Absyn_Pat*_tmp31A;if((_tmp2F9.where_clause).tag == 2){_LL1: _LL2:
 return(void*)& Cyc_Tcpat_Dummy_val;}else{_LL3: _tmp31A=(_tmp2F9.pattern).val;_LL4: {
# 1459
void*_tmp2FA=_tmp31A->r;void*_tmp2FB=_tmp2FA;union Cyc_Absyn_AggrInfoU _tmp319;struct Cyc_List_List*_tmp318;struct Cyc_Absyn_Datatypedecl*_tmp317;struct Cyc_Absyn_Datatypefield*_tmp316;switch(*((int*)_tmp2FB)){case 6U: _LL6: _LL7:
# 1461
 if(i != 0)
({void*_tmp2FC=0U;({struct _dyneither_ptr _tmp5B1=(struct _dyneither_ptr)({struct Cyc_Int_pa_PrintArg_struct _tmp2FF;_tmp2FF.tag=1U;_tmp2FF.f1=(unsigned long)i;({void*_tmp2FD[1U]={& _tmp2FF};({struct _dyneither_ptr _tmp5B0=({const char*_tmp2FE="get_access on pointer pattern with offset %d\n";_tag_dyneither(_tmp2FE,sizeof(char),46U);});Cyc_aprintf(_tmp5B0,_tag_dyneither(_tmp2FD,sizeof(void*),1U));});});});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp5B1,_tag_dyneither(_tmp2FC,sizeof(void*),0U));});});
return(void*)& Cyc_Tcpat_Deref_val;case 5U: _LL8: _LL9:
 return(void*)({struct Cyc_Tcpat_TupleField_Tcpat_Access_struct*_tmp300=_cycalloc_atomic(sizeof(*_tmp300));({struct Cyc_Tcpat_TupleField_Tcpat_Access_struct _tmp5B2=({struct Cyc_Tcpat_TupleField_Tcpat_Access_struct _tmp301;_tmp301.tag=2U;_tmp301.f1=(unsigned int)i;_tmp301;});_tmp300[0]=_tmp5B2;});_tmp300;});case 8U: _LLA: _tmp317=((struct Cyc_Absyn_Datatype_p_Absyn_Raw_pat_struct*)_tmp2FB)->f1;_tmp316=((struct Cyc_Absyn_Datatype_p_Absyn_Raw_pat_struct*)_tmp2FB)->f2;_LLB:
 return(void*)({struct Cyc_Tcpat_DatatypeField_Tcpat_Access_struct*_tmp302=_cycalloc(sizeof(*_tmp302));({struct Cyc_Tcpat_DatatypeField_Tcpat_Access_struct _tmp5B3=({struct Cyc_Tcpat_DatatypeField_Tcpat_Access_struct _tmp303;_tmp303.tag=3U;_tmp303.f1=_tmp317;_tmp303.f2=_tmp316;_tmp303.f3=(unsigned int)i;_tmp303;});_tmp302[0]=_tmp5B3;});_tmp302;});case 7U: if(((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp2FB)->f1 != 0){_LLC: _tmp319=(((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp2FB)->f1)->aggr_info;_tmp318=((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp2FB)->f3;_LLD: {
# 1467
struct Cyc_Absyn_Aggrdecl*_tmp304=Cyc_Absyn_get_known_aggrdecl(_tmp319);
if(((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp304->impl))->tagged){
struct Cyc_List_List*_tmp305=(*((struct _tuple18*)((struct Cyc_List_List*)_check_null(_tmp318))->hd)).f1;struct Cyc_List_List*_tmp306=_tmp305;struct _dyneither_ptr*_tmp30D;if(_tmp306 != 0){if(((struct Cyc_Absyn_FieldName_Absyn_Designator_struct*)((struct Cyc_List_List*)_tmp306)->hd)->tag == 1U){if(((struct Cyc_List_List*)_tmp306)->tl == 0){_LL11: _tmp30D=((struct Cyc_Absyn_FieldName_Absyn_Designator_struct*)_tmp306->hd)->f1;_LL12:
# 1471
 return(void*)({struct Cyc_Tcpat_AggrField_Tcpat_Access_struct*_tmp307=_cycalloc(sizeof(*_tmp307));({struct Cyc_Tcpat_AggrField_Tcpat_Access_struct _tmp5B4=({struct Cyc_Tcpat_AggrField_Tcpat_Access_struct _tmp308;_tmp308.tag=4U;_tmp308.f1=((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp304->impl))->tagged;_tmp308.f2=_tmp30D;_tmp308;});_tmp307[0]=_tmp5B4;});_tmp307;});}else{goto _LL13;}}else{goto _LL13;}}else{_LL13: _LL14:
({void*_tmp309=0U;({struct _dyneither_ptr _tmp5B7=(struct _dyneither_ptr)({struct Cyc_String_pa_PrintArg_struct _tmp30C;_tmp30C.tag=0U;({struct _dyneither_ptr _tmp5B5=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_pat2string(_tmp31A));_tmp30C.f1=_tmp5B5;});({void*_tmp30A[1U]={& _tmp30C};({struct _dyneither_ptr _tmp5B6=({const char*_tmp30B="get_access on bad aggr pattern: %s";_tag_dyneither(_tmp30B,sizeof(char),35U);});Cyc_aprintf(_tmp5B6,_tag_dyneither(_tmp30A,sizeof(void*),1U));});});});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp5B7,_tag_dyneither(_tmp309,sizeof(void*),0U));});});}_LL10:;}{
# 1475
struct Cyc_List_List*_tmp30E=((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp304->impl))->fields;
int _tmp30F=i;
for(0;i != 0;-- i){_tmp30E=((struct Cyc_List_List*)_check_null(_tmp30E))->tl;}
return(void*)({struct Cyc_Tcpat_AggrField_Tcpat_Access_struct*_tmp310=_cycalloc(sizeof(*_tmp310));({struct Cyc_Tcpat_AggrField_Tcpat_Access_struct _tmp5B8=({struct Cyc_Tcpat_AggrField_Tcpat_Access_struct _tmp311;_tmp311.tag=4U;_tmp311.f1=((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp304->impl))->tagged;_tmp311.f2=((struct Cyc_Absyn_Aggrfield*)((struct Cyc_List_List*)_check_null(_tmp30E))->hd)->name;_tmp311;});_tmp310[0]=_tmp5B8;});_tmp310;});};}}else{goto _LLE;}default: _LLE: _LLF:
({void*_tmp312=0U;({struct _dyneither_ptr _tmp5BB=(struct _dyneither_ptr)({struct Cyc_String_pa_PrintArg_struct _tmp315;_tmp315.tag=0U;({struct _dyneither_ptr _tmp5B9=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_pat2string(_tmp31A));_tmp315.f1=_tmp5B9;});({void*_tmp313[1U]={& _tmp315};({struct _dyneither_ptr _tmp5BA=({const char*_tmp314="get_access on bad pattern: %s";_tag_dyneither(_tmp314,sizeof(char),30U);});Cyc_aprintf(_tmp5BA,_tag_dyneither(_tmp313,sizeof(void*),1U));});});});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp5BB,_tag_dyneither(_tmp312,sizeof(void*),0U));});});}_LL5:;}}_LL0:;}struct _tuple23{struct Cyc_List_List*f1;struct Cyc_Tcpat_Con_s*f2;};
# 1485
static struct Cyc_List_List*Cyc_Tcpat_getoarg(struct _tuple23*env,int i){
struct _tuple23*_tmp31B=env;struct Cyc_List_List*_tmp31F;struct Cyc_Tcpat_Con_s*_tmp31E;_LL1: _tmp31F=_tmp31B->f1;_tmp31E=_tmp31B->f2;_LL2:;{
void*acc=Cyc_Tcpat_get_access(_tmp31E->orig_pat,i);
return({struct Cyc_List_List*_tmp31C=_cycalloc(sizeof(*_tmp31C));({struct Cyc_Tcpat_PathNode*_tmp5BC=({struct Cyc_Tcpat_PathNode*_tmp31D=_cycalloc(sizeof(*_tmp31D));_tmp31D->orig_pat=_tmp31E->orig_pat;_tmp31D->access=acc;_tmp31D;});_tmp31C->hd=_tmp5BC;});_tmp31C->tl=_tmp31F;_tmp31C;});};}
# 1490
static struct Cyc_List_List*Cyc_Tcpat_getoargs(struct Cyc_Tcpat_Con_s*pcon,struct Cyc_List_List*obj){
struct _tuple23 _tmp320=({struct _tuple23 _tmp321;_tmp321.f1=obj;_tmp321.f2=pcon;_tmp321;});
return((struct Cyc_List_List*(*)(int n,struct Cyc_List_List*(*f)(struct _tuple23*,int),struct _tuple23*env))Cyc_List_tabulate_c)(pcon->arity,Cyc_Tcpat_getoarg,& _tmp320);}
# 1498
static void*Cyc_Tcpat_match(void*p,struct Cyc_List_List*obj,void*dsc,struct Cyc_List_List*ctx,struct Cyc_List_List*work,struct Cyc_Tcpat_Rhs*right_hand_side,struct Cyc_List_List*rules){
# 1502
void*_tmp322=p;struct Cyc_Tcpat_Con_s*_tmp330;struct Cyc_List_List*_tmp32F;if(((struct Cyc_Tcpat_Any_Tcpat_Simple_pat_struct*)_tmp322)->tag == 0U){_LL1: _LL2:
# 1504
 return({struct Cyc_List_List*_tmp5BF=Cyc_Tcpat_augment(ctx,dsc);struct Cyc_List_List*_tmp5BE=work;struct Cyc_Tcpat_Rhs*_tmp5BD=right_hand_side;Cyc_Tcpat_and_match(_tmp5BF,_tmp5BE,_tmp5BD,rules);});}else{_LL3: _tmp330=((struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct*)_tmp322)->f1;_tmp32F=((struct Cyc_Tcpat_Con_Tcpat_Simple_pat_struct*)_tmp322)->f2;_LL4: {
# 1506
enum Cyc_Tcpat_Answer _tmp323=Cyc_Tcpat_static_match(_tmp330,dsc);enum Cyc_Tcpat_Answer _tmp324=_tmp323;switch(_tmp324){case Cyc_Tcpat_Yes: _LL6: _LL7: {
# 1508
struct Cyc_List_List*ctx2=({struct Cyc_List_List*_tmp327=_cycalloc(sizeof(*_tmp327));({struct _tuple21*_tmp5C0=({struct _tuple21*_tmp328=_cycalloc(sizeof(*_tmp328));_tmp328->f1=_tmp330;_tmp328->f2=0;_tmp328;});_tmp327->hd=_tmp5C0;});_tmp327->tl=ctx;_tmp327;});
struct _tuple20*work_frame=({struct _tuple20*_tmp326=_cycalloc(sizeof(*_tmp326));_tmp326->f1=_tmp32F;({struct Cyc_List_List*_tmp5C1=Cyc_Tcpat_getoargs(_tmp330,obj);_tmp326->f2=_tmp5C1;});({struct Cyc_List_List*_tmp5C2=
Cyc_Tcpat_getdargs(_tmp330,dsc);_tmp326->f3=_tmp5C2;});_tmp326;});
struct Cyc_List_List*work2=({struct Cyc_List_List*_tmp325=_cycalloc(sizeof(*_tmp325));_tmp325->hd=work_frame;_tmp325->tl=work;_tmp325;});
return Cyc_Tcpat_and_match(ctx2,work2,right_hand_side,rules);}case Cyc_Tcpat_No: _LL8: _LL9:
# 1514
 return({void*_tmp5C3=Cyc_Tcpat_build_desc(ctx,dsc,work);Cyc_Tcpat_or_match(_tmp5C3,rules);});default: _LLA: _LLB: {
# 1516
struct Cyc_List_List*ctx2=({struct Cyc_List_List*_tmp32D=_cycalloc(sizeof(*_tmp32D));({struct _tuple21*_tmp5C4=({struct _tuple21*_tmp32E=_cycalloc(sizeof(*_tmp32E));_tmp32E->f1=_tmp330;_tmp32E->f2=0;_tmp32E;});_tmp32D->hd=_tmp5C4;});_tmp32D->tl=ctx;_tmp32D;});
struct _tuple20*work_frame=({struct _tuple20*_tmp32C=_cycalloc(sizeof(*_tmp32C));_tmp32C->f1=_tmp32F;({struct Cyc_List_List*_tmp5C5=Cyc_Tcpat_getoargs(_tmp330,obj);_tmp32C->f2=_tmp5C5;});({struct Cyc_List_List*_tmp5C6=
Cyc_Tcpat_getdargs(_tmp330,dsc);_tmp32C->f3=_tmp5C6;});_tmp32C;});
struct Cyc_List_List*work2=({struct Cyc_List_List*_tmp32B=_cycalloc(sizeof(*_tmp32B));_tmp32B->hd=work_frame;_tmp32B->tl=work;_tmp32B;});
void*_tmp329=Cyc_Tcpat_and_match(ctx2,work2,right_hand_side,rules);
void*_tmp32A=({void*_tmp5C9=({struct Cyc_List_List*_tmp5C8=ctx;void*_tmp5C7=Cyc_Tcpat_add_neg(dsc,_tmp330);Cyc_Tcpat_build_desc(_tmp5C8,_tmp5C7,work);});Cyc_Tcpat_or_match(_tmp5C9,rules);});
# 1523
return Cyc_Tcpat_ifeq(obj,_tmp330,_tmp329,_tmp32A);}}_LL5:;}}_LL0:;}
# 1533
static void Cyc_Tcpat_check_exhaust_overlap(void*d,void(*not_exhaust)(void*,void*),void*env1,void(*rhs_appears)(void*,struct Cyc_Tcpat_Rhs*),void*env2,int*exhaust_done){
# 1538
void*_tmp331=d;struct Cyc_List_List*_tmp338;void*_tmp337;struct Cyc_Tcpat_Rhs*_tmp336;void*_tmp335;switch(*((int*)_tmp331)){case 0U: _LL1: _tmp335=(void*)((struct Cyc_Tcpat_Failure_Tcpat_Decision_struct*)_tmp331)->f1;_LL2:
# 1540
 if(!(*exhaust_done)){not_exhaust(env1,_tmp335);*exhaust_done=1;}
goto _LL0;case 1U: _LL3: _tmp336=((struct Cyc_Tcpat_Success_Tcpat_Decision_struct*)_tmp331)->f1;_LL4:
 rhs_appears(env2,_tmp336);goto _LL0;default: _LL5: _tmp338=((struct Cyc_Tcpat_SwitchDec_Tcpat_Decision_struct*)_tmp331)->f2;_tmp337=(void*)((struct Cyc_Tcpat_SwitchDec_Tcpat_Decision_struct*)_tmp331)->f3;_LL6:
# 1544
 for(0;_tmp338 != 0;_tmp338=_tmp338->tl){
struct _tuple0 _tmp332=*((struct _tuple0*)_tmp338->hd);struct _tuple0 _tmp333=_tmp332;void*_tmp334;_LL8: _tmp334=_tmp333.f2;_LL9:;
Cyc_Tcpat_check_exhaust_overlap(_tmp334,not_exhaust,env1,rhs_appears,env2,exhaust_done);}
# 1549
Cyc_Tcpat_check_exhaust_overlap(_tmp337,not_exhaust,env1,rhs_appears,env2,exhaust_done);
# 1551
goto _LL0;}_LL0:;}
# 1559
static struct _tuple22*Cyc_Tcpat_get_match(int*ctr,struct Cyc_Absyn_Switch_clause*swc){
# 1561
void*sp0=Cyc_Tcpat_compile_pat(swc->pattern);
struct Cyc_Tcpat_Rhs*rhs=({struct Cyc_Tcpat_Rhs*_tmp341=_cycalloc(sizeof(*_tmp341));_tmp341->used=0;_tmp341->pat_loc=(swc->pattern)->loc;_tmp341->rhs=swc->body;_tmp341;});
void*sp;
if(swc->where_clause != 0){
union Cyc_Tcpat_PatOrWhere _tmp339=({union Cyc_Tcpat_PatOrWhere _tmp33B;(_tmp33B.where_clause).val=swc->where_clause;(_tmp33B.where_clause).tag=2;_tmp33B;});
({void*_tmp5CC=({struct Cyc_List_List*_tmp5CB=({void*_tmp33A[2U];({void*_tmp5CA=Cyc_Tcpat_int_pat(*ctr,_tmp339);_tmp33A[1U]=_tmp5CA;});_tmp33A[0U]=sp0;((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp33A,sizeof(void*),2U));});Cyc_Tcpat_tuple_pat(_tmp5CB,_tmp339);});sp=_tmp5CC;});
*ctr=*ctr + 1;}else{
# 1569
({void*_tmp5D0=({struct Cyc_List_List*_tmp5CF=({void*_tmp33C[2U];({void*_tmp5CE=(void*)({struct Cyc_Tcpat_Any_Tcpat_Simple_pat_struct*_tmp33D=_cycalloc_atomic(sizeof(*_tmp33D));({struct Cyc_Tcpat_Any_Tcpat_Simple_pat_struct _tmp5CD=({struct Cyc_Tcpat_Any_Tcpat_Simple_pat_struct _tmp33E;_tmp33E.tag=0U;_tmp33E;});_tmp33D[0]=_tmp5CD;});_tmp33D;});_tmp33C[1U]=_tmp5CE;});_tmp33C[0U]=sp0;((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp33C,sizeof(void*),2U));});Cyc_Tcpat_tuple_pat(_tmp5CF,({union Cyc_Tcpat_PatOrWhere _tmp33F;(_tmp33F.where_clause).val=0;(_tmp33F.where_clause).tag=2;_tmp33F;}));});sp=_tmp5D0;});}
return({struct _tuple22*_tmp340=_cycalloc(sizeof(*_tmp340));_tmp340->f1=sp;_tmp340->f2=rhs;_tmp340;});}char Cyc_Tcpat_Desc2string[12U]="Desc2string";struct Cyc_Tcpat_Desc2string_exn_struct{char*tag;};
# 1575
struct Cyc_Tcpat_Desc2string_exn_struct Cyc_Tcpat_Desc2string_val={Cyc_Tcpat_Desc2string};
# 1577
static struct _dyneither_ptr Cyc_Tcpat_descs2string(struct Cyc_List_List*);
static struct _dyneither_ptr Cyc_Tcpat_desc2string(void*d){
void*_tmp343=d;struct Cyc_Set_Set*_tmp3A4;struct Cyc_Tcpat_Con_s*_tmp3A3;struct Cyc_List_List*_tmp3A2;if(((struct Cyc_Tcpat_Pos_Tcpat_Term_desc_struct*)_tmp343)->tag == 0U){_LL1: _tmp3A3=((struct Cyc_Tcpat_Pos_Tcpat_Term_desc_struct*)_tmp343)->f1;_tmp3A2=((struct Cyc_Tcpat_Pos_Tcpat_Term_desc_struct*)_tmp343)->f2;_LL2: {
# 1581
union Cyc_Tcpat_Name_value _tmp344=_tmp3A3->name;
struct Cyc_Absyn_Pat*p;
{union Cyc_Tcpat_PatOrWhere _tmp345=_tmp3A3->orig_pat;union Cyc_Tcpat_PatOrWhere _tmp346=_tmp345;struct Cyc_Absyn_Pat*_tmp347;if((_tmp346.where_clause).tag == 2){_LL6: _LL7:
 return Cyc_Tcpat_desc2string((void*)((struct Cyc_List_List*)_check_null(_tmp3A2))->hd);}else{_LL8: _tmp347=(_tmp346.pattern).val;_LL9:
 p=_tmp347;goto _LL5;}_LL5:;}{
# 1587
void*_tmp348=p->r;void*_tmp349=_tmp348;struct Cyc_Absyn_Exp*_tmp37E;struct Cyc_Absyn_Enumfield*_tmp37D;struct Cyc_Absyn_Enumfield*_tmp37C;struct _dyneither_ptr _tmp37B;int _tmp37A;char _tmp379;int _tmp378;struct Cyc_Absyn_Datatypefield*_tmp377;struct Cyc_Absyn_Aggrdecl*_tmp376;struct Cyc_List_List*_tmp375;struct Cyc_Absyn_Tvar*_tmp374;struct Cyc_Absyn_Vardecl*_tmp373;struct Cyc_Absyn_Vardecl*_tmp372;struct Cyc_Absyn_Vardecl*_tmp371;switch(*((int*)_tmp349)){case 0U: _LLB: _LLC:
 return({const char*_tmp34A="_";_tag_dyneither(_tmp34A,sizeof(char),2U);});case 1U: _LLD: _tmp371=((struct Cyc_Absyn_Var_p_Absyn_Raw_pat_struct*)_tmp349)->f1;_LLE:
 return Cyc_Absynpp_qvar2string(_tmp371->name);case 3U: _LLF: _tmp372=((struct Cyc_Absyn_Reference_p_Absyn_Raw_pat_struct*)_tmp349)->f1;_LL10:
 return(struct _dyneither_ptr)({struct Cyc_String_pa_PrintArg_struct _tmp34D;_tmp34D.tag=0U;({struct _dyneither_ptr _tmp5D1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(_tmp372->name));_tmp34D.f1=_tmp5D1;});({void*_tmp34B[1U]={& _tmp34D};({struct _dyneither_ptr _tmp5D2=({const char*_tmp34C="*%s";_tag_dyneither(_tmp34C,sizeof(char),4U);});Cyc_aprintf(_tmp5D2,_tag_dyneither(_tmp34B,sizeof(void*),1U));});});});case 4U: _LL11: _tmp374=((struct Cyc_Absyn_TagInt_p_Absyn_Raw_pat_struct*)_tmp349)->f1;_tmp373=((struct Cyc_Absyn_TagInt_p_Absyn_Raw_pat_struct*)_tmp349)->f2;_LL12:
 return(struct _dyneither_ptr)({struct Cyc_String_pa_PrintArg_struct _tmp351;_tmp351.tag=0U;_tmp351.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*_tmp374->name);({struct Cyc_String_pa_PrintArg_struct _tmp350;_tmp350.tag=0U;({struct _dyneither_ptr _tmp5D3=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(_tmp373->name));_tmp350.f1=_tmp5D3;});({void*_tmp34E[2U]={& _tmp350,& _tmp351};({struct _dyneither_ptr _tmp5D4=({const char*_tmp34F="%s<`%s>";_tag_dyneither(_tmp34F,sizeof(char),8U);});Cyc_aprintf(_tmp5D4,_tag_dyneither(_tmp34E,sizeof(void*),2U));});});});});case 5U: _LL13: _LL14:
# 1593
 return(struct _dyneither_ptr)({struct Cyc_String_pa_PrintArg_struct _tmp354;_tmp354.tag=0U;({struct _dyneither_ptr _tmp5D5=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Tcpat_descs2string(_tmp3A2));_tmp354.f1=_tmp5D5;});({void*_tmp352[1U]={& _tmp354};({struct _dyneither_ptr _tmp5D6=({const char*_tmp353="$(%s)";_tag_dyneither(_tmp353,sizeof(char),6U);});Cyc_aprintf(_tmp5D6,_tag_dyneither(_tmp352,sizeof(void*),1U));});});});case 6U: _LL15: _LL16:
# 1595
 return(struct _dyneither_ptr)({struct Cyc_String_pa_PrintArg_struct _tmp357;_tmp357.tag=0U;({struct _dyneither_ptr _tmp5D7=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Tcpat_descs2string(_tmp3A2));_tmp357.f1=_tmp5D7;});({void*_tmp355[1U]={& _tmp357};({struct _dyneither_ptr _tmp5D8=({const char*_tmp356="&%s";_tag_dyneither(_tmp356,sizeof(char),4U);});Cyc_aprintf(_tmp5D8,_tag_dyneither(_tmp355,sizeof(void*),1U));});});});case 7U: if(((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp349)->f1 != 0){if(((((struct Cyc_Absyn_AggrInfo*)((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp349)->f1)->aggr_info).KnownAggr).tag == 2){_LL17: _tmp376=*(((((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp349)->f1)->aggr_info).KnownAggr).val;_tmp375=((struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct*)_tmp349)->f3;_LL18:
# 1597
 if(_tmp376->kind == Cyc_Absyn_UnionA){
struct Cyc_List_List*_tmp358=_tmp375;struct _dyneither_ptr*_tmp35E;if(_tmp358 != 0){if(((struct _tuple18*)((struct Cyc_List_List*)_tmp358)->hd)->f1 != 0){if(((struct Cyc_Absyn_FieldName_Absyn_Designator_struct*)((struct Cyc_List_List*)((struct _tuple18*)((struct Cyc_List_List*)_tmp358)->hd)->f1)->hd)->tag == 1U){_LL2C: _tmp35E=((struct Cyc_Absyn_FieldName_Absyn_Designator_struct*)(((struct _tuple18*)_tmp358->hd)->f1)->hd)->f1;_LL2D:
# 1600
 return(struct _dyneither_ptr)({struct Cyc_String_pa_PrintArg_struct _tmp35D;_tmp35D.tag=0U;({struct _dyneither_ptr _tmp5D9=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Tcpat_descs2string(_tmp3A2));_tmp35D.f1=_tmp5D9;});({struct Cyc_String_pa_PrintArg_struct _tmp35C;_tmp35C.tag=0U;_tmp35C.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*_tmp35E);({struct Cyc_String_pa_PrintArg_struct _tmp35B;_tmp35B.tag=0U;({struct _dyneither_ptr _tmp5DA=(struct _dyneither_ptr)((struct _dyneither_ptr)
# 1600
Cyc_Absynpp_qvar2string(_tmp376->name));_tmp35B.f1=_tmp5DA;});({void*_tmp359[3U]={& _tmp35B,& _tmp35C,& _tmp35D};({struct _dyneither_ptr _tmp5DB=({const char*_tmp35A="%s{.%s = %s}";_tag_dyneither(_tmp35A,sizeof(char),13U);});Cyc_aprintf(_tmp5DB,_tag_dyneither(_tmp359,sizeof(void*),3U));});});});});});}else{goto _LL2E;}}else{goto _LL2E;}}else{_LL2E: _LL2F:
# 1602
 goto _LL2B;}_LL2B:;}
# 1605
return(struct _dyneither_ptr)({struct Cyc_String_pa_PrintArg_struct _tmp362;_tmp362.tag=0U;({struct _dyneither_ptr _tmp5DC=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Tcpat_descs2string(_tmp3A2));_tmp362.f1=_tmp5DC;});({struct Cyc_String_pa_PrintArg_struct _tmp361;_tmp361.tag=0U;({struct _dyneither_ptr _tmp5DD=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(_tmp376->name));_tmp361.f1=_tmp5DD;});({void*_tmp35F[2U]={& _tmp361,& _tmp362};({struct _dyneither_ptr _tmp5DE=({const char*_tmp360="%s{%s}";_tag_dyneither(_tmp360,sizeof(char),7U);});Cyc_aprintf(_tmp5DE,_tag_dyneither(_tmp35F,sizeof(void*),2U));});});});});}else{goto _LL29;}}else{goto _LL29;}case 8U: _LL19: _tmp377=((struct Cyc_Absyn_Datatype_p_Absyn_Raw_pat_struct*)_tmp349)->f2;_LL1A:
# 1607
 if(_tmp3A2 == 0)
return(struct _dyneither_ptr)({struct Cyc_String_pa_PrintArg_struct _tmp365;_tmp365.tag=0U;({struct _dyneither_ptr _tmp5DF=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(_tmp377->name));_tmp365.f1=_tmp5DF;});({void*_tmp363[1U]={& _tmp365};({struct _dyneither_ptr _tmp5E0=({const char*_tmp364="%s";_tag_dyneither(_tmp364,sizeof(char),3U);});Cyc_aprintf(_tmp5E0,_tag_dyneither(_tmp363,sizeof(void*),1U));});});});else{
# 1610
return(struct _dyneither_ptr)({struct Cyc_String_pa_PrintArg_struct _tmp369;_tmp369.tag=0U;({struct _dyneither_ptr _tmp5E1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Tcpat_descs2string(_tmp3A2));_tmp369.f1=_tmp5E1;});({struct Cyc_String_pa_PrintArg_struct _tmp368;_tmp368.tag=0U;({struct _dyneither_ptr _tmp5E2=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(_tmp377->name));_tmp368.f1=_tmp5E2;});({void*_tmp366[2U]={& _tmp368,& _tmp369};({struct _dyneither_ptr _tmp5E3=({const char*_tmp367="%s(%s)";_tag_dyneither(_tmp367,sizeof(char),7U);});Cyc_aprintf(_tmp5E3,_tag_dyneither(_tmp366,sizeof(void*),2U));});});});});}case 9U: _LL1B: _LL1C:
 return({const char*_tmp36A="NULL";_tag_dyneither(_tmp36A,sizeof(char),5U);});case 10U: _LL1D: _tmp378=((struct Cyc_Absyn_Int_p_Absyn_Raw_pat_struct*)_tmp349)->f2;_LL1E:
 return(struct _dyneither_ptr)({struct Cyc_Int_pa_PrintArg_struct _tmp36D;_tmp36D.tag=1U;_tmp36D.f1=(unsigned long)_tmp378;({void*_tmp36B[1U]={& _tmp36D};({struct _dyneither_ptr _tmp5E4=({const char*_tmp36C="%d";_tag_dyneither(_tmp36C,sizeof(char),3U);});Cyc_aprintf(_tmp5E4,_tag_dyneither(_tmp36B,sizeof(void*),1U));});});});case 11U: _LL1F: _tmp379=((struct Cyc_Absyn_Char_p_Absyn_Raw_pat_struct*)_tmp349)->f1;_LL20:
 return(struct _dyneither_ptr)({struct Cyc_Int_pa_PrintArg_struct _tmp370;_tmp370.tag=1U;_tmp370.f1=(unsigned long)((int)_tmp379);({void*_tmp36E[1U]={& _tmp370};({struct _dyneither_ptr _tmp5E5=({const char*_tmp36F="%d";_tag_dyneither(_tmp36F,sizeof(char),3U);});Cyc_aprintf(_tmp5E5,_tag_dyneither(_tmp36E,sizeof(void*),1U));});});});case 12U: _LL21: _tmp37B=((struct Cyc_Absyn_Float_p_Absyn_Raw_pat_struct*)_tmp349)->f1;_tmp37A=((struct Cyc_Absyn_Float_p_Absyn_Raw_pat_struct*)_tmp349)->f2;_LL22:
 return _tmp37B;case 13U: _LL23: _tmp37C=((struct Cyc_Absyn_Enum_p_Absyn_Raw_pat_struct*)_tmp349)->f2;_LL24:
 _tmp37D=_tmp37C;goto _LL26;case 14U: _LL25: _tmp37D=((struct Cyc_Absyn_AnonEnum_p_Absyn_Raw_pat_struct*)_tmp349)->f2;_LL26:
 return Cyc_Absynpp_qvar2string(_tmp37D->name);case 17U: _LL27: _tmp37E=((struct Cyc_Absyn_Exp_p_Absyn_Raw_pat_struct*)_tmp349)->f1;_LL28:
 return Cyc_Absynpp_exp2string(_tmp37E);default: _LL29: _LL2A:
(int)_throw((void*)& Cyc_Tcpat_Desc2string_val);}_LLA:;};}}else{_LL3: _tmp3A4=((struct Cyc_Tcpat_Neg_Tcpat_Term_desc_struct*)_tmp343)->f1;_LL4:
# 1621
 if(((int(*)(struct Cyc_Set_Set*s))Cyc_Set_is_empty)(_tmp3A4))return({const char*_tmp37F="_";_tag_dyneither(_tmp37F,sizeof(char),2U);});{
struct Cyc_Tcpat_Con_s*_tmp380=((struct Cyc_Tcpat_Con_s*(*)(struct Cyc_Set_Set*s))Cyc_Set_choose)(_tmp3A4);
# 1625
if((int)(((_tmp380->orig_pat).where_clause).tag == 2))(int)_throw((void*)& Cyc_Tcpat_Desc2string_val);{
struct Cyc_Absyn_Pat*_tmp381=({union Cyc_Tcpat_PatOrWhere _tmp3A1=_tmp380->orig_pat;if((_tmp3A1.pattern).tag != 1)_throw_match();(_tmp3A1.pattern).val;});
void*_tmp382=Cyc_Tcutil_compress((void*)_check_null(_tmp381->topt));void*_tmp383=_tmp382;struct Cyc_Absyn_Aggrdecl*_tmp3A0;struct Cyc_Absyn_Datatypedecl*_tmp39F;struct Cyc_Absyn_PtrAtts _tmp39E;switch(*((int*)_tmp383)){case 6U: if(((struct Cyc_Absyn_IntType_Absyn_Type_struct*)_tmp383)->f2 == Cyc_Absyn_Char_sz){_LL31: _LL32:
# 1630
{int i=0;for(0;i < 256;++ i){
struct Cyc_Tcpat_Con_s*_tmp384=Cyc_Tcpat_char_con((char)i,_tmp381);
if(!((int(*)(struct Cyc_Set_Set*s,struct Cyc_Tcpat_Con_s*elt))Cyc_Set_member)(_tmp3A4,_tmp384))
return(struct _dyneither_ptr)({struct Cyc_Int_pa_PrintArg_struct _tmp387;_tmp387.tag=1U;_tmp387.f1=(unsigned long)i;({void*_tmp385[1U]={& _tmp387};({struct _dyneither_ptr _tmp5E6=({const char*_tmp386="%d";_tag_dyneither(_tmp386,sizeof(char),3U);});Cyc_aprintf(_tmp5E6,_tag_dyneither(_tmp385,sizeof(void*),1U));});});});}}
# 1635
(int)_throw((void*)& Cyc_Tcpat_Desc2string_val);}else{_LL33: _LL34:
# 1638
{unsigned int i=0U;for(0;i < -1;++ i){
struct Cyc_Tcpat_Con_s*_tmp388=Cyc_Tcpat_int_con((int)i,_tmp380->orig_pat);
if(!((int(*)(struct Cyc_Set_Set*s,struct Cyc_Tcpat_Con_s*elt))Cyc_Set_member)(_tmp3A4,_tmp388))
return(struct _dyneither_ptr)({struct Cyc_Int_pa_PrintArg_struct _tmp38B;_tmp38B.tag=1U;_tmp38B.f1=(unsigned long)((int)i);({void*_tmp389[1U]={& _tmp38B};({struct _dyneither_ptr _tmp5E7=({const char*_tmp38A="%d";_tag_dyneither(_tmp38A,sizeof(char),3U);});Cyc_aprintf(_tmp5E7,_tag_dyneither(_tmp389,sizeof(void*),1U));});});});}}
# 1643
(int)_throw((void*)& Cyc_Tcpat_Desc2string_val);}case 5U: _LL35: _tmp39E=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp383)->f1).ptr_atts;_LL36: {
# 1645
union Cyc_Absyn_Constraint*_tmp38C=_tmp39E.nullable;
int is_nullable=((int(*)(int y,union Cyc_Absyn_Constraint*x))Cyc_Absyn_conref_def)(0,_tmp38C);
if(is_nullable){
if(!({struct Cyc_Set_Set*_tmp5E8=_tmp3A4;((int(*)(struct Cyc_Set_Set*s,struct Cyc_Tcpat_Con_s*elt))Cyc_Set_member)(_tmp5E8,Cyc_Tcpat_null_con(_tmp381));}))
return({const char*_tmp38D="NULL";_tag_dyneither(_tmp38D,sizeof(char),5U);});}
# 1651
return({const char*_tmp38E="&_";_tag_dyneither(_tmp38E,sizeof(char),3U);});}case 3U: if((((((struct Cyc_Absyn_DatatypeType_Absyn_Type_struct*)_tmp383)->f1).datatype_info).KnownDatatype).tag == 2){_LL37: _tmp39F=*(((((struct Cyc_Absyn_DatatypeType_Absyn_Type_struct*)_tmp383)->f1).datatype_info).KnownDatatype).val;_LL38:
# 1653
 if(_tmp39F->is_extensible)(int)_throw((void*)& Cyc_Tcpat_Desc2string_val);{
struct Cyc_List_List*_tmp38F=(struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(_tmp39F->fields))->v;
int span=((int(*)(struct Cyc_List_List*x))Cyc_List_length)(_tmp38F);
for(0;(unsigned int)_tmp38F;_tmp38F=_tmp38F->tl){
struct _dyneither_ptr n=*(*((struct Cyc_Absyn_Datatypefield*)_tmp38F->hd)->name).f2;
struct Cyc_List_List*_tmp390=((struct Cyc_Absyn_Datatypefield*)_tmp38F->hd)->typs;
if(!({struct Cyc_Set_Set*_tmp5EA=_tmp3A4;((int(*)(struct Cyc_Set_Set*s,struct Cyc_Tcpat_Con_s*elt))Cyc_Set_member)(_tmp5EA,({struct Cyc_Tcpat_Con_s*_tmp391=_cycalloc(sizeof(*_tmp391));({union Cyc_Tcpat_Name_value _tmp5E9=Cyc_Tcpat_Name_v(n);_tmp391->name=_tmp5E9;});_tmp391->arity=0;_tmp391->span=0;_tmp391->orig_pat=_tmp380->orig_pat;_tmp391;}));})){
if(((int(*)(struct Cyc_List_List*x))Cyc_List_length)(_tmp390)== 0)
return n;else{
# 1663
return(struct _dyneither_ptr)({struct Cyc_String_pa_PrintArg_struct _tmp394;_tmp394.tag=0U;_tmp394.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)n);({void*_tmp392[1U]={& _tmp394};({struct _dyneither_ptr _tmp5EB=({const char*_tmp393="%s(...)";_tag_dyneither(_tmp393,sizeof(char),8U);});Cyc_aprintf(_tmp5EB,_tag_dyneither(_tmp392,sizeof(void*),1U));});});});}}}
# 1666
(int)_throw((void*)& Cyc_Tcpat_Desc2string_val);};}else{goto _LL3B;}case 11U: if((((((struct Cyc_Absyn_AggrType_Absyn_Type_struct*)_tmp383)->f1).aggr_info).KnownAggr).tag == 2){_LL39: _tmp3A0=*(((((struct Cyc_Absyn_AggrType_Absyn_Type_struct*)_tmp383)->f1).aggr_info).KnownAggr).val;if(_tmp3A0->kind == Cyc_Absyn_UnionA){_LL3A: {
# 1668
struct Cyc_List_List*_tmp395=((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp3A0->impl))->fields;
int span=((int(*)(struct Cyc_List_List*x))Cyc_List_length)(_tmp395);
struct _tuple2*_tmp396=_tmp3A0->name;struct _tuple2*_tmp397=_tmp396;struct _dyneither_ptr _tmp39D;_LL3E: _tmp39D=*_tmp397->f2;_LL3F:;
for(0;(unsigned int)_tmp395;_tmp395=_tmp395->tl){
struct _dyneither_ptr n=*((struct Cyc_Absyn_Aggrfield*)_tmp395->hd)->name;
if(!({struct Cyc_Set_Set*_tmp5ED=_tmp3A4;((int(*)(struct Cyc_Set_Set*s,struct Cyc_Tcpat_Con_s*elt))Cyc_Set_member)(_tmp5ED,({struct Cyc_Tcpat_Con_s*_tmp398=_cycalloc(sizeof(*_tmp398));({union Cyc_Tcpat_Name_value _tmp5EC=Cyc_Tcpat_Name_v(n);_tmp398->name=_tmp5EC;});_tmp398->arity=0;_tmp398->span=0;_tmp398->orig_pat=_tmp380->orig_pat;_tmp398;}));}))
return(struct _dyneither_ptr)({struct Cyc_String_pa_PrintArg_struct _tmp39C;_tmp39C.tag=0U;_tmp39C.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)n);({struct Cyc_String_pa_PrintArg_struct _tmp39B;_tmp39B.tag=0U;_tmp39B.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp39D);({void*_tmp399[2U]={& _tmp39B,& _tmp39C};({struct _dyneither_ptr _tmp5EE=({const char*_tmp39A="%s{.%s = _}";_tag_dyneither(_tmp39A,sizeof(char),12U);});Cyc_aprintf(_tmp5EE,_tag_dyneither(_tmp399,sizeof(void*),2U));});});});});}
# 1676
(int)_throw((void*)& Cyc_Tcpat_Desc2string_val);}}else{goto _LL3B;}}else{goto _LL3B;}default: _LL3B: _LL3C:
(int)_throw((void*)& Cyc_Tcpat_Desc2string_val);}_LL30:;};};}_LL0:;}
# 1681
static struct _dyneither_ptr*Cyc_Tcpat_desc2stringptr(void*d){
return({struct _dyneither_ptr*_tmp3A5=_cycalloc(sizeof(*_tmp3A5));({struct _dyneither_ptr _tmp5EF=Cyc_Tcpat_desc2string(d);_tmp3A5[0]=_tmp5EF;});_tmp3A5;});}
# 1684
static struct _dyneither_ptr Cyc_Tcpat_descs2string(struct Cyc_List_List*ds){
struct Cyc_List_List*_tmp3A6=((struct Cyc_List_List*(*)(struct _dyneither_ptr*(*f)(void*),struct Cyc_List_List*x))Cyc_List_map)(Cyc_Tcpat_desc2stringptr,ds);
struct _dyneither_ptr*comma=_init_dyneither_ptr(_cycalloc(sizeof(struct _dyneither_ptr)),",",sizeof(char),2U);
{struct Cyc_List_List*_tmp3A7=_tmp3A6;for(0;_tmp3A7 != 0;_tmp3A7=((struct Cyc_List_List*)_check_null(_tmp3A7))->tl){
if(_tmp3A7->tl != 0){
({struct Cyc_List_List*_tmp5F0=({struct Cyc_List_List*_tmp3A8=_cycalloc(sizeof(*_tmp3A8));_tmp3A8->hd=comma;_tmp3A8->tl=_tmp3A7->tl;_tmp3A8;});_tmp3A7->tl=_tmp5F0;});
_tmp3A7=_tmp3A7->tl;}}}
# 1693
return(struct _dyneither_ptr)Cyc_strconcat_l(_tmp3A6);}
# 1696
static void Cyc_Tcpat_not_exhaust_err(unsigned int loc,void*desc){
struct _handler_cons _tmp3AA;_push_handler(& _tmp3AA);{int _tmp3AC=0;if(setjmp(_tmp3AA.handler))_tmp3AC=1;if(!_tmp3AC){
{struct _dyneither_ptr _tmp3AD=Cyc_Tcpat_desc2string(desc);
({struct Cyc_String_pa_PrintArg_struct _tmp3B0;_tmp3B0.tag=0U;_tmp3B0.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp3AD);({void*_tmp3AE[1U]={& _tmp3B0};({unsigned int _tmp5F2=loc;struct _dyneither_ptr _tmp5F1=({const char*_tmp3AF="patterns may not be exhaustive.\n\tmissing case for %s";_tag_dyneither(_tmp3AF,sizeof(char),53U);});Cyc_Tcutil_terr(_tmp5F2,_tmp5F1,_tag_dyneither(_tmp3AE,sizeof(void*),1U));});});});}
# 1698
;_pop_handler();}else{void*_tmp3AB=(void*)_exn_thrown;void*_tmp3B1=_tmp3AB;void*_tmp3B4;if(((struct Cyc_Tcpat_Desc2string_exn_struct*)_tmp3B1)->tag == Cyc_Tcpat_Desc2string){_LL1: _LL2:
# 1702
({void*_tmp3B2=0U;({unsigned int _tmp5F4=loc;struct _dyneither_ptr _tmp5F3=({const char*_tmp3B3="patterns may not be exhaustive.";_tag_dyneither(_tmp3B3,sizeof(char),32U);});Cyc_Tcutil_terr(_tmp5F4,_tmp5F3,_tag_dyneither(_tmp3B2,sizeof(void*),0U));});});
goto _LL0;}else{_LL3: _tmp3B4=_tmp3B1;_LL4:(int)_rethrow(_tmp3B4);}_LL0:;}};}
# 1706
static void Cyc_Tcpat_rule_occurs(int dummy,struct Cyc_Tcpat_Rhs*rhs){
rhs->used=1;}
# 1710
void Cyc_Tcpat_check_switch_exhaustive(unsigned int loc,struct Cyc_Tcenv_Tenv*te,struct Cyc_List_List*swcs,void**dopt){
# 1716
int _tmp3B5=0;
int*_tmp3B6=& _tmp3B5;
struct Cyc_List_List*_tmp3B7=((struct Cyc_List_List*(*)(struct _tuple22*(*f)(int*,struct Cyc_Absyn_Switch_clause*),int*env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcpat_get_match,_tmp3B6,swcs);
void*_tmp3B8=Cyc_Tcpat_match_compile(_tmp3B7);
*dopt=_tmp3B8;{
# 1722
int _tmp3B9=0;
((void(*)(void*d,void(*not_exhaust)(unsigned int,void*),unsigned int env1,void(*rhs_appears)(int,struct Cyc_Tcpat_Rhs*),int env2,int*exhaust_done))Cyc_Tcpat_check_exhaust_overlap)(_tmp3B8,Cyc_Tcpat_not_exhaust_err,loc,Cyc_Tcpat_rule_occurs,0,& _tmp3B9);
# 1725
for(0;_tmp3B7 != 0;_tmp3B7=_tmp3B7->tl){
struct _tuple22*_tmp3BA=(struct _tuple22*)_tmp3B7->hd;struct _tuple22*_tmp3BB=_tmp3BA;int _tmp3BF;unsigned int _tmp3BE;_LL1: _tmp3BF=(_tmp3BB->f2)->used;_tmp3BE=(_tmp3BB->f2)->pat_loc;_LL2:;
if(!_tmp3BF){
({void*_tmp3BC=0U;({unsigned int _tmp5F6=_tmp3BE;struct _dyneither_ptr _tmp5F5=({const char*_tmp3BD="redundant pattern (check for misspelled constructors in earlier patterns)";_tag_dyneither(_tmp3BD,sizeof(char),74U);});Cyc_Tcutil_terr(_tmp5F6,_tmp5F5,_tag_dyneither(_tmp3BC,sizeof(void*),0U));});});
# 1730
break;}}};}
# 1739
static void Cyc_Tcpat_not_exhaust_warn(struct _tuple14*pr,void*desc){
(*pr).f2=0;{
struct _handler_cons _tmp3C0;_push_handler(& _tmp3C0);{int _tmp3C2=0;if(setjmp(_tmp3C0.handler))_tmp3C2=1;if(!_tmp3C2){
{struct _dyneither_ptr _tmp3C3=Cyc_Tcpat_desc2string(desc);
({struct Cyc_String_pa_PrintArg_struct _tmp3C6;_tmp3C6.tag=0U;_tmp3C6.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp3C3);({void*_tmp3C4[1U]={& _tmp3C6};({unsigned int _tmp5F8=(*pr).f1;struct _dyneither_ptr _tmp5F7=({const char*_tmp3C5="pattern not exhaustive.\n\tmissing case for %s";_tag_dyneither(_tmp3C5,sizeof(char),45U);});Cyc_Tcutil_warn(_tmp5F8,_tmp5F7,_tag_dyneither(_tmp3C4,sizeof(void*),1U));});});});}
# 1742
;_pop_handler();}else{void*_tmp3C1=(void*)_exn_thrown;void*_tmp3C7=_tmp3C1;void*_tmp3CA;if(((struct Cyc_Tcpat_Desc2string_exn_struct*)_tmp3C7)->tag == Cyc_Tcpat_Desc2string){_LL1: _LL2:
# 1746
({void*_tmp3C8=0U;({unsigned int _tmp5FA=(*pr).f1;struct _dyneither_ptr _tmp5F9=({const char*_tmp3C9="pattern not exhaustive.";_tag_dyneither(_tmp3C9,sizeof(char),24U);});Cyc_Tcutil_warn(_tmp5FA,_tmp5F9,_tag_dyneither(_tmp3C8,sizeof(void*),0U));});});
goto _LL0;}else{_LL3: _tmp3CA=_tmp3C7;_LL4:(int)_rethrow(_tmp3CA);}_LL0:;}};};}
# 1750
static void Cyc_Tcpat_dummy_fn(int i,struct Cyc_Tcpat_Rhs*rhs){
return;}
# 1754
int Cyc_Tcpat_check_let_pat_exhaustive(unsigned int loc,struct Cyc_Tcenv_Tenv*te,struct Cyc_Absyn_Pat*p,void**dopt){
struct Cyc_Tcpat_Rhs*rhs=({struct Cyc_Tcpat_Rhs*_tmp3D2=_cycalloc(sizeof(*_tmp3D2));_tmp3D2->used=0;_tmp3D2->pat_loc=p->loc;({struct Cyc_Absyn_Stmt*_tmp5FB=Cyc_Absyn_skip_stmt(0U);_tmp3D2->rhs=_tmp5FB;});_tmp3D2;});
struct Cyc_List_List*_tmp3CB=({struct Cyc_List_List*_tmp3D0=_cycalloc(sizeof(*_tmp3D0));({struct _tuple22*_tmp5FD=({struct _tuple22*_tmp3D1=_cycalloc(sizeof(*_tmp3D1));({void*_tmp5FC=Cyc_Tcpat_compile_pat(p);_tmp3D1->f1=_tmp5FC;});_tmp3D1->f2=rhs;_tmp3D1;});_tmp3D0->hd=_tmp5FD;});_tmp3D0->tl=0;_tmp3D0;});
void*_tmp3CC=Cyc_Tcpat_match_compile(_tmp3CB);
struct _tuple14 _tmp3CD=({struct _tuple14 _tmp3CF;_tmp3CF.f1=loc;_tmp3CF.f2=1;_tmp3CF;});
int _tmp3CE=0;
((void(*)(void*d,void(*not_exhaust)(struct _tuple14*,void*),struct _tuple14*env1,void(*rhs_appears)(int,struct Cyc_Tcpat_Rhs*),int env2,int*exhaust_done))Cyc_Tcpat_check_exhaust_overlap)(_tmp3CC,Cyc_Tcpat_not_exhaust_warn,& _tmp3CD,Cyc_Tcpat_dummy_fn,0,& _tmp3CE);
# 1762
*dopt=_tmp3CC;
return _tmp3CD.f2;}
# 1770
static struct _tuple22*Cyc_Tcpat_get_match2(struct Cyc_Absyn_Switch_clause*swc){
void*sp0=Cyc_Tcpat_compile_pat(swc->pattern);
# 1774
if(swc->where_clause != 0)
({void*_tmp3D3=0U;({unsigned int _tmp5FF=((struct Cyc_Absyn_Exp*)_check_null(swc->where_clause))->loc;struct _dyneither_ptr _tmp5FE=({const char*_tmp3D4="&&-clauses not supported in exception handlers.";_tag_dyneither(_tmp3D4,sizeof(char),48U);});Cyc_Tcutil_terr(_tmp5FF,_tmp5FE,_tag_dyneither(_tmp3D3,sizeof(void*),0U));});});{
# 1777
struct Cyc_Tcpat_Rhs*rhs=({struct Cyc_Tcpat_Rhs*_tmp3D6=_cycalloc(sizeof(*_tmp3D6));_tmp3D6->used=0;_tmp3D6->pat_loc=(swc->pattern)->loc;_tmp3D6->rhs=swc->body;_tmp3D6;});
return({struct _tuple22*_tmp3D5=_cycalloc(sizeof(*_tmp3D5));_tmp3D5->f1=sp0;_tmp3D5->f2=rhs;_tmp3D5;});};}
# 1780
static void Cyc_Tcpat_not_exhaust_err2(unsigned int loc,void*d){;}
# 1783
void Cyc_Tcpat_check_catch_overlap(unsigned int loc,struct Cyc_Tcenv_Tenv*te,struct Cyc_List_List*swcs,void**dopt){
# 1787
struct Cyc_List_List*_tmp3D7=((struct Cyc_List_List*(*)(struct _tuple22*(*f)(struct Cyc_Absyn_Switch_clause*),struct Cyc_List_List*x))Cyc_List_map)(Cyc_Tcpat_get_match2,swcs);
void*_tmp3D8=Cyc_Tcpat_match_compile(_tmp3D7);
*dopt=_tmp3D8;{
int _tmp3D9=0;
((void(*)(void*d,void(*not_exhaust)(unsigned int,void*),unsigned int env1,void(*rhs_appears)(int,struct Cyc_Tcpat_Rhs*),int env2,int*exhaust_done))Cyc_Tcpat_check_exhaust_overlap)(_tmp3D8,Cyc_Tcpat_not_exhaust_err2,loc,Cyc_Tcpat_rule_occurs,0,& _tmp3D9);
# 1793
for(0;_tmp3D7 != 0;_tmp3D7=_tmp3D7->tl){
# 1795
if(_tmp3D7->tl == 0)break;{
struct _tuple22*_tmp3DA=(struct _tuple22*)_tmp3D7->hd;struct _tuple22*_tmp3DB=_tmp3DA;int _tmp3DF;unsigned int _tmp3DE;_LL1: _tmp3DF=(_tmp3DB->f2)->used;_tmp3DE=(_tmp3DB->f2)->pat_loc;_LL2:;
if(!_tmp3DF){
({void*_tmp3DC=0U;({unsigned int _tmp601=_tmp3DE;struct _dyneither_ptr _tmp600=({const char*_tmp3DD="redundant pattern (check for misspelled constructors in earlier patterns)";_tag_dyneither(_tmp3DD,sizeof(char),74U);});Cyc_Tcutil_terr(_tmp601,_tmp600,_tag_dyneither(_tmp3DC,sizeof(void*),0U));});});
break;}};}};}
# 1804
int Cyc_Tcpat_has_vars(struct Cyc_Core_Opt*pat_vars){
{struct Cyc_List_List*_tmp3E0=(struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(pat_vars))->v;for(0;_tmp3E0 != 0;_tmp3E0=_tmp3E0->tl){
if((*((struct _tuple15*)_tmp3E0->hd)).f1 != 0)
return 1;}}
return 0;}
