#ifndef _SETJMP_H_
#define _SETJMP_H_
#ifndef _jmp_buf_def_
#define _jmp_buf_def_
typedef int jmp_buf[192];
#endif
extern int setjmp(jmp_buf);
#endif
/* This is a C header file to be used by the output of the Cyclone to
   C translator.  The corresponding definitions are in file
   lib/runtime_cyc.c
*/
#ifndef _CYC_INCLUDE_H_
#define _CYC_INCLUDE_H_

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

/* Need one of these per thread (we don't have threads)
   The runtime maintains a stack that contains either _handler_cons
   structs or _RegionHandle structs.  The tag is 0 for a handler_cons
   and 1 for a region handle.  */
struct _RuntimeStack {
  int tag; /* 0 for an exception handler, 1 for a region handle */
  struct _RuntimeStack *next;
};

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
#endif
};

struct _DynRegionFrame {
  struct _RuntimeStack s;
  struct _DynRegionHandle *x;
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
extern int _throw_null();
extern int _throw_arraybounds();
extern int _throw_badalloc();
extern int _throw(void* e);
#endif

extern struct _xtunion_struct *_exn_thrown;

/* Built-in Exceptions */
extern struct _xtunion_struct ADD_PREFIX(Null_Exception_struct);
extern struct _xtunion_struct * ADD_PREFIX(Null_Exception);
extern struct _xtunion_struct ADD_PREFIX(Array_bounds_struct);
extern struct _xtunion_struct * ADD_PREFIX(Array_bounds);
extern struct _xtunion_struct ADD_PREFIX(Match_Exception_struct);
extern struct _xtunion_struct * ADD_PREFIX(Match_Exception);
extern struct _xtunion_struct ADD_PREFIX(Bad_alloc_struct);
extern struct _xtunion_struct * ADD_PREFIX(Bad_alloc);

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
_check_null(void *ptr) {
  void*_check_null_temp = (void*)(ptr);
  if (!_check_null_temp) _throw_null();
  return _check_null_temp;
}
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
_check_known_subscript_null(void *ptr, unsigned bound, unsigned elt_sz, unsigned index) {
  void*_cks_ptr = (void*)(ptr);
  unsigned _cks_bound = (bound);
  unsigned _cks_elt_sz = (elt_sz);
  unsigned _cks_index = (index);
  if (!_cks_ptr) _throw_null();
  if (_cks_index >= _cks_bound) _throw_arraybounds();
  return ((char *)_cks_ptr) + _cks_elt_sz*_cks_index;
}
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
_check_known_subscript_notnull(unsigned bound,unsigned index) { 
  unsigned _cksnn_bound = (bound); 
  unsigned _cksnn_index = (index); 
  if (_cksnn_index >= _cksnn_bound) _throw_arraybounds(); 
  return _cksnn_index;
}
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
#define _zero_arr_plus_char(orig_x,orig_sz,orig_i) ((orig_x)+(orig_i))
#define _zero_arr_plus_short(orig_x,orig_sz,orig_i) ((orig_x)+(orig_i))
#define _zero_arr_plus_int(orig_x,orig_sz,orig_i) ((orig_x)+(orig_i))
#define _zero_arr_plus_float(orig_x,orig_sz,orig_i) ((orig_x)+(orig_i))
#define _zero_arr_plus_double(orig_x,orig_sz,orig_i) ((orig_x)+(orig_i))
#define _zero_arr_plus_longdouble(orig_x,orig_sz,orig_i) ((orig_x)+(orig_i))
#define _zero_arr_plus_voidstar(orig_x,orig_sz,orig_i) ((orig_x)+(orig_i))
#else
static _INLINE char *
_zero_arr_plus_char(char *orig_x, int orig_sz, int orig_i) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null();
  if (orig_i < 0) _throw_arraybounds();
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds();
  return orig_x + orig_i;
}
static _INLINE short *
_zero_arr_plus_short(short *orig_x, int orig_sz, int orig_i) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null();
  if (orig_i < 0) _throw_arraybounds();
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds();
  return orig_x + orig_i;
}
static _INLINE int *
_zero_arr_plus_int(int *orig_x, int orig_sz, int orig_i) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null();
  if (orig_i < 0) _throw_arraybounds();
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds();
  return orig_x + orig_i;
}
static _INLINE float *
_zero_arr_plus_float(float *orig_x, int orig_sz, int orig_i) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null();
  if (orig_i < 0) _throw_arraybounds();
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds();
  return orig_x + orig_i;
}
static _INLINE double *
_zero_arr_plus_double(double *orig_x, int orig_sz, int orig_i) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null();
  if (orig_i < 0) _throw_arraybounds();
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds();
  return orig_x + orig_i;
}
static _INLINE long double *
_zero_arr_plus_longdouble(long double *orig_x, int orig_sz, int orig_i) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null();
  if (orig_i < 0) _throw_arraybounds();
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds();
  return orig_x + orig_i;
}
static _INLINE void *
_zero_arr_plus_voidstar(void **orig_x, int orig_sz, int orig_i) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null();
  if (orig_i < 0) _throw_arraybounds();
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds();
  return orig_x + orig_i;
}
#endif


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
   Note that this expands to call _zero_arr_plus. */
/*#define _zero_arr_inplace_plus(x,orig_i) ({ \
  typedef _zap_tx = (*x); \
  _zap_tx **_zap_x = &((_zap_tx*)x); \
  *_zap_x = _zero_arr_plus(*_zap_x,1,(orig_i)); })
  */
static _INLINE void 
_zero_arr_inplace_plus_char(char *x, int orig_i) {
  char **_zap_x = &x;
  *_zap_x = _zero_arr_plus_char(*_zap_x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_short(short *x, int orig_i) {
  short **_zap_x = &x;
  *_zap_x = _zero_arr_plus_short(*_zap_x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_int(int *x, int orig_i) {
  int **_zap_x = &x;
  *_zap_x = _zero_arr_plus_int(*_zap_x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_float(float *x, int orig_i) {
  float **_zap_x = &x;
  *_zap_x = _zero_arr_plus_float(*_zap_x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_double(double *x, int orig_i) {
  double **_zap_x = &x;
  *_zap_x = _zero_arr_plus_double(*_zap_x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_longdouble(long double *x, int orig_i) {
  long double **_zap_x = &x;
  *_zap_x = _zero_arr_plus_longdouble(*_zap_x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_voidstar(void **x, int orig_i) {
  void ***_zap_x = &x;
  *_zap_x = _zero_arr_plus_voidstar(*_zap_x,1,orig_i);
}




/* Does in-place increment of a zero-terminated pointer (e.g., x++).
   Note that this expands to call _zero_arr_plus. */
/*#define _zero_arr_inplace_plus_post(x,orig_i) ({ \
  typedef _zap_tx = (*x); \
  _zap_tx **_zap_x = &((_zap_tx*)x); \
  _zap_tx *_zap_res = *_zap_x; \
  *_zap_x = _zero_arr_plus(_zap_res,1,(orig_i)); \
  _zap_res; })*/
  
static _INLINE char *
_zero_arr_inplace_plus_post_char(char *x, int orig_i){
  char ** _zap_x = &x;
  char * _zap_res = *_zap_x;
  *_zap_x = _zero_arr_plus_char(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE short *
_zero_arr_inplace_plus_post_short(short *x, int orig_i){
  short **_zap_x = &x;
  short * _zap_res = *_zap_x;
  *_zap_x = _zero_arr_plus_short(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE int *
_zero_arr_inplace_plus_post_int(int *x, int orig_i){
  int **_zap_x = &x;
  int * _zap_res = *_zap_x;
  *_zap_x = _zero_arr_plus_int(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE float *
_zero_arr_inplace_plus_post_float(float *x, int orig_i){
  float **_zap_x = &x;
  float * _zap_res = *_zap_x;
  *_zap_x = _zero_arr_plus_float(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE double *
_zero_arr_inplace_plus_post_double(double *x, int orig_i){
  double **_zap_x = &x;
  double * _zap_res = *_zap_x;
  *_zap_x = _zero_arr_plus_double(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE long double *
_zero_arr_inplace_plus_post_longdouble(long double *x, int orig_i){
  long double **_zap_x = &x;
  long double * _zap_res = *_zap_x;
  *_zap_x = _zero_arr_plus_longdouble(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE void **
_zero_arr_inplace_plus_post_voidstar(void **x, int orig_i){
  void ***_zap_x = &x;
  void ** _zap_res = *_zap_x;
  *_zap_x = _zero_arr_plus_voidstar(_zap_res,1,orig_i);
  return _zap_res;
}



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
_check_dyneither_subscript(struct _dyneither_ptr arr,unsigned elt_sz,unsigned index) {
  struct _dyneither_ptr _cus_arr = (arr);
  unsigned _cus_elt_sz = (elt_sz);
  unsigned _cus_index = (index);
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index;
  /* JGM: not needed! if (!_cus_arr.base) _throw_null(); */ 
  if (_cus_ans < _cus_arr.base || _cus_ans >= _cus_arr.last_plus_one)
    _throw_arraybounds();
  return _cus_ans;
}
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
_untag_dyneither_ptr(struct _dyneither_ptr arr, 
                     unsigned elt_sz,unsigned num_elts) {
  struct _dyneither_ptr _arr = (arr);
  unsigned char *_curr = _arr.curr;
  if (_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one)
    _throw_arraybounds();
  return _curr;
}
#else
#define _untag_dyneither_ptr(arr,elt_sz,num_elts) ({ \
  struct _dyneither_ptr _arr = (arr); \
  unsigned char *_curr = _arr.curr; \
  if (_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one)\
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
extern void* _profile_GC_malloc(int,char *file,int lineno);
extern void* _profile_GC_malloc_atomic(int,char *file,int lineno);
extern void* _profile_region_malloc(struct _RegionHandle *, unsigned,
                                     char *file,int lineno);
extern struct _RegionHandle _profile_new_region(const char *rgn_name,
						char *file,int lineno);
extern void _profile_free_region(struct _RegionHandle *,
				 char *file,int lineno);
#  if !defined(RUNTIME_CYC)
#define _new_region(n) _profile_new_region(n,__FILE__ ":" __FUNCTION__,__LINE__)
#define _free_region(r) _profile_free_region(r,__FILE__ ":" __FUNCTION__,__LINE__)
#define _region_malloc(rh,n) _profile_region_malloc(rh,n,__FILE__ ":" __FUNCTION__,__LINE__)
#  endif
#define _cycalloc(n) _profile_GC_malloc(n,__FILE__ ":" __FUNCTION__,__LINE__)
#define _cycalloc_atomic(n) _profile_GC_malloc_atomic(n,__FILE__ ":" __FUNCTION__,__LINE__)
#endif
#endif

/* the next two routines swap [x] and [y]; not thread safe! */
static _INLINE void _swap_word(void *x, void *y) {
  unsigned long *lx = (unsigned long *)x, *ly = (unsigned long *)y, tmp;
  tmp = *lx;
  *lx = *ly;
  *ly = tmp;
}
static _INLINE void _swap_dyneither(struct _dyneither_ptr *x, 
				   struct _dyneither_ptr *y) {
  struct _dyneither_ptr tmp = *x;
  *x = *y;
  *y = tmp;
}
 struct Cyc_Core_NewRegion{struct _DynRegionHandle*dynregion;};struct Cyc_Core_Opt{
void*v;};extern char Cyc_Core_Invalid_argument[21];struct Cyc_Core_Invalid_argument_struct{
char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Failure[12];struct Cyc_Core_Failure_struct{
char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Impossible[15];struct Cyc_Core_Impossible_struct{
char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Not_found[14];extern char Cyc_Core_Unreachable[
16];struct Cyc_Core_Unreachable_struct{char*tag;struct _dyneither_ptr f1;};extern
struct _RegionHandle*Cyc_Core_heap_region;struct Cyc_Core_NewRegion Cyc_Core_new_dynregion();
extern char Cyc_Core_Open_Region[16];extern char Cyc_Core_Free_Region[16];void Cyc_Core_free_dynregion(
struct _DynRegionHandle*);struct Cyc___cycFILE;extern struct Cyc___cycFILE*Cyc_stderr;
struct Cyc_Cstdio___abstractFILE;struct Cyc_String_pa_struct{int tag;struct
_dyneither_ptr f1;};struct Cyc_Int_pa_struct{int tag;unsigned long f1;};struct Cyc_Double_pa_struct{
int tag;double f1;};struct Cyc_LongDouble_pa_struct{int tag;long double f1;};struct
Cyc_ShortPtr_pa_struct{int tag;short*f1;};struct Cyc_IntPtr_pa_struct{int tag;
unsigned long*f1;};struct _dyneither_ptr Cyc_aprintf(struct _dyneither_ptr,struct
_dyneither_ptr);int Cyc_fflush(struct Cyc___cycFILE*);int Cyc_fprintf(struct Cyc___cycFILE*,
struct _dyneither_ptr,struct _dyneither_ptr);struct Cyc_ShortPtr_sa_struct{int tag;
short*f1;};struct Cyc_UShortPtr_sa_struct{int tag;unsigned short*f1;};struct Cyc_IntPtr_sa_struct{
int tag;int*f1;};struct Cyc_UIntPtr_sa_struct{int tag;unsigned int*f1;};struct Cyc_StringPtr_sa_struct{
int tag;struct _dyneither_ptr f1;};struct Cyc_DoublePtr_sa_struct{int tag;double*f1;}
;struct Cyc_FloatPtr_sa_struct{int tag;float*f1;};struct Cyc_CharPtr_sa_struct{int
tag;struct _dyneither_ptr f1;};int Cyc_vfprintf(struct Cyc___cycFILE*,struct
_dyneither_ptr,struct _dyneither_ptr);extern char Cyc_FileCloseError[19];extern char
Cyc_FileOpenError[18];struct Cyc_FileOpenError_struct{char*tag;struct
_dyneither_ptr f1;};struct Cyc_List_List{void*hd;struct Cyc_List_List*tl;};struct
Cyc_List_List*Cyc_List_list(struct _dyneither_ptr);int Cyc_List_length(struct Cyc_List_List*
x);struct Cyc_List_List*Cyc_List_map(void*(*f)(void*),struct Cyc_List_List*x);
struct Cyc_List_List*Cyc_List_rmap(struct _RegionHandle*,void*(*f)(void*),struct
Cyc_List_List*x);struct Cyc_List_List*Cyc_List_rmap_c(struct _RegionHandle*,void*(*
f)(void*,void*),void*env,struct Cyc_List_List*x);extern char Cyc_List_List_mismatch[
18];void Cyc_List_iter(void(*f)(void*),struct Cyc_List_List*x);void Cyc_List_iter_c(
void(*f)(void*,void*),void*env,struct Cyc_List_List*x);struct Cyc_List_List*Cyc_List_rev(
struct Cyc_List_List*x);struct Cyc_List_List*Cyc_List_rrev(struct _RegionHandle*,
struct Cyc_List_List*x);struct Cyc_List_List*Cyc_List_imp_rev(struct Cyc_List_List*
x);struct Cyc_List_List*Cyc_List_append(struct Cyc_List_List*x,struct Cyc_List_List*
y);struct Cyc_List_List*Cyc_List_imp_append(struct Cyc_List_List*x,struct Cyc_List_List*
y);extern char Cyc_List_Nth[8];void*Cyc_List_nth(struct Cyc_List_List*x,int n);int
Cyc_List_forall(int(*pred)(void*),struct Cyc_List_List*x);int Cyc_strcmp(struct
_dyneither_ptr s1,struct _dyneither_ptr s2);struct _dyneither_ptr Cyc_strconcat(
struct _dyneither_ptr,struct _dyneither_ptr);struct Cyc_Iter_Iter{void*env;int(*
next)(void*env,void*dest);};int Cyc_Iter_next(struct Cyc_Iter_Iter,void*);struct
Cyc_Set_Set;struct Cyc_Set_Set*Cyc_Set_rempty(struct _RegionHandle*r,int(*cmp)(
void*,void*));struct Cyc_Set_Set*Cyc_Set_rinsert(struct _RegionHandle*r,struct Cyc_Set_Set*
s,void*elt);int Cyc_Set_member(struct Cyc_Set_Set*s,void*elt);extern char Cyc_Set_Absent[
11];struct Cyc_Dict_T;struct Cyc_Dict_Dict{int(*rel)(void*,void*);struct
_RegionHandle*r;struct Cyc_Dict_T*t;};extern char Cyc_Dict_Present[12];extern char
Cyc_Dict_Absent[11];struct Cyc_Dict_Dict Cyc_Dict_rempty(struct _RegionHandle*,int(*
cmp)(void*,void*));struct Cyc_Dict_Dict Cyc_Dict_rshare(struct _RegionHandle*,
struct Cyc_Dict_Dict);struct Cyc_Dict_Dict Cyc_Dict_insert(struct Cyc_Dict_Dict d,
void*k,void*v);void*Cyc_Dict_lookup(struct Cyc_Dict_Dict d,void*k);void**Cyc_Dict_lookup_opt(
struct Cyc_Dict_Dict d,void*k);struct _tuple0{void*f1;void*f2;};struct _tuple0*Cyc_Dict_rchoose(
struct _RegionHandle*r,struct Cyc_Dict_Dict d);struct _tuple0*Cyc_Dict_rchoose(
struct _RegionHandle*,struct Cyc_Dict_Dict d);struct Cyc_Lineno_Pos{struct
_dyneither_ptr logical_file;struct _dyneither_ptr line;int line_no;int col;};extern
char Cyc_Position_Exit[9];struct Cyc_Position_Segment;struct _dyneither_ptr Cyc_Position_string_of_segment(
struct Cyc_Position_Segment*);struct Cyc_Position_Error{struct _dyneither_ptr source;
struct Cyc_Position_Segment*seg;void*kind;struct _dyneither_ptr desc;};extern char
Cyc_Position_Nocontext[14];struct Cyc_Xarray_Xarray{struct _RegionHandle*r;struct
_dyneither_ptr elmts;int num_elmts;};int Cyc_Xarray_length(struct Cyc_Xarray_Xarray*);
void*Cyc_Xarray_get(struct Cyc_Xarray_Xarray*,int);struct Cyc_Xarray_Xarray*Cyc_Xarray_rcreate_empty(
struct _RegionHandle*);int Cyc_Xarray_add_ind(struct Cyc_Xarray_Xarray*,void*);
struct Cyc_Absyn_Loc_n_struct{int tag;};struct Cyc_Absyn_Rel_n_struct{int tag;struct
Cyc_List_List*f1;};struct Cyc_Absyn_Abs_n_struct{int tag;struct Cyc_List_List*f1;};
union Cyc_Absyn_Nmspace_union{struct Cyc_Absyn_Loc_n_struct Loc_n;struct Cyc_Absyn_Rel_n_struct
Rel_n;struct Cyc_Absyn_Abs_n_struct Abs_n;};struct _tuple1{union Cyc_Absyn_Nmspace_union
f1;struct _dyneither_ptr*f2;};struct Cyc_Absyn_Conref;struct Cyc_Absyn_Tqual{int
print_const;int q_volatile;int q_restrict;int real_const;struct Cyc_Position_Segment*
loc;};struct Cyc_Absyn_Eq_constr_struct{int tag;void*f1;};struct Cyc_Absyn_Forward_constr_struct{
int tag;struct Cyc_Absyn_Conref*f1;};struct Cyc_Absyn_No_constr_struct{int tag;};
union Cyc_Absyn_Constraint_union{struct Cyc_Absyn_Eq_constr_struct Eq_constr;struct
Cyc_Absyn_Forward_constr_struct Forward_constr;struct Cyc_Absyn_No_constr_struct
No_constr;};struct Cyc_Absyn_Conref{union Cyc_Absyn_Constraint_union v;};struct Cyc_Absyn_Eq_kb_struct{
int tag;void*f1;};struct Cyc_Absyn_Unknown_kb_struct{int tag;struct Cyc_Core_Opt*f1;
};struct Cyc_Absyn_Less_kb_struct{int tag;struct Cyc_Core_Opt*f1;void*f2;};struct
Cyc_Absyn_Tvar{struct _dyneither_ptr*name;int identity;void*kind;};struct Cyc_Absyn_Upper_b_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_PtrLoc{struct Cyc_Position_Segment*
ptr_loc;struct Cyc_Position_Segment*rgn_loc;struct Cyc_Position_Segment*zt_loc;};
struct Cyc_Absyn_PtrAtts{void*rgn;struct Cyc_Absyn_Conref*nullable;struct Cyc_Absyn_Conref*
bounds;struct Cyc_Absyn_Conref*zero_term;struct Cyc_Absyn_PtrLoc*ptrloc;};struct
Cyc_Absyn_PtrInfo{void*elt_typ;struct Cyc_Absyn_Tqual elt_tq;struct Cyc_Absyn_PtrAtts
ptr_atts;};struct Cyc_Absyn_VarargInfo{struct Cyc_Core_Opt*name;struct Cyc_Absyn_Tqual
tq;void*type;int inject;};struct Cyc_Absyn_FnInfo{struct Cyc_List_List*tvars;struct
Cyc_Core_Opt*effect;void*ret_typ;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*
cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_List_List*attributes;};struct
Cyc_Absyn_UnknownTunionInfo{struct _tuple1*name;int is_xtunion;int is_flat;};struct
Cyc_Absyn_UnknownTunion_struct{int tag;struct Cyc_Absyn_UnknownTunionInfo f1;};
struct Cyc_Absyn_KnownTunion_struct{int tag;struct Cyc_Absyn_Tuniondecl**f1;};union
Cyc_Absyn_TunionInfoU_union{struct Cyc_Absyn_UnknownTunion_struct UnknownTunion;
struct Cyc_Absyn_KnownTunion_struct KnownTunion;};struct Cyc_Absyn_TunionInfo{union
Cyc_Absyn_TunionInfoU_union tunion_info;struct Cyc_List_List*targs;struct Cyc_Core_Opt*
rgn;};struct Cyc_Absyn_UnknownTunionFieldInfo{struct _tuple1*tunion_name;struct
_tuple1*field_name;int is_xtunion;};struct Cyc_Absyn_UnknownTunionfield_struct{int
tag;struct Cyc_Absyn_UnknownTunionFieldInfo f1;};struct Cyc_Absyn_KnownTunionfield_struct{
int tag;struct Cyc_Absyn_Tuniondecl*f1;struct Cyc_Absyn_Tunionfield*f2;};union Cyc_Absyn_TunionFieldInfoU_union{
struct Cyc_Absyn_UnknownTunionfield_struct UnknownTunionfield;struct Cyc_Absyn_KnownTunionfield_struct
KnownTunionfield;};struct Cyc_Absyn_TunionFieldInfo{union Cyc_Absyn_TunionFieldInfoU_union
field_info;struct Cyc_List_List*targs;};struct Cyc_Absyn_UnknownAggr_struct{int tag;
void*f1;struct _tuple1*f2;};struct Cyc_Absyn_KnownAggr_struct{int tag;struct Cyc_Absyn_Aggrdecl**
f1;};union Cyc_Absyn_AggrInfoU_union{struct Cyc_Absyn_UnknownAggr_struct
UnknownAggr;struct Cyc_Absyn_KnownAggr_struct KnownAggr;};struct Cyc_Absyn_AggrInfo{
union Cyc_Absyn_AggrInfoU_union aggr_info;struct Cyc_List_List*targs;};struct Cyc_Absyn_ArrayInfo{
void*elt_type;struct Cyc_Absyn_Tqual tq;struct Cyc_Absyn_Exp*num_elts;struct Cyc_Absyn_Conref*
zero_term;struct Cyc_Position_Segment*zt_loc;};struct Cyc_Absyn_Evar_struct{int tag;
struct Cyc_Core_Opt*f1;struct Cyc_Core_Opt*f2;int f3;struct Cyc_Core_Opt*f4;};struct
Cyc_Absyn_VarType_struct{int tag;struct Cyc_Absyn_Tvar*f1;};struct Cyc_Absyn_TunionType_struct{
int tag;struct Cyc_Absyn_TunionInfo f1;};struct Cyc_Absyn_TunionFieldType_struct{int
tag;struct Cyc_Absyn_TunionFieldInfo f1;};struct Cyc_Absyn_PointerType_struct{int
tag;struct Cyc_Absyn_PtrInfo f1;};struct Cyc_Absyn_IntType_struct{int tag;void*f1;
void*f2;};struct Cyc_Absyn_DoubleType_struct{int tag;int f1;};struct Cyc_Absyn_ArrayType_struct{
int tag;struct Cyc_Absyn_ArrayInfo f1;};struct Cyc_Absyn_FnType_struct{int tag;struct
Cyc_Absyn_FnInfo f1;};struct Cyc_Absyn_TupleType_struct{int tag;struct Cyc_List_List*
f1;};struct Cyc_Absyn_AggrType_struct{int tag;struct Cyc_Absyn_AggrInfo f1;};struct
Cyc_Absyn_AnonAggrType_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_EnumType_struct{
int tag;struct _tuple1*f1;struct Cyc_Absyn_Enumdecl*f2;};struct Cyc_Absyn_AnonEnumType_struct{
int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_RgnHandleType_struct{int tag;void*
f1;};struct Cyc_Absyn_DynRgnType_struct{int tag;void*f1;void*f2;};struct Cyc_Absyn_TypedefType_struct{
int tag;struct _tuple1*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_Typedefdecl*f3;
void**f4;};struct Cyc_Absyn_ValueofType_struct{int tag;struct Cyc_Absyn_Exp*f1;};
struct Cyc_Absyn_TagType_struct{int tag;void*f1;};struct Cyc_Absyn_AccessEff_struct{
int tag;void*f1;};struct Cyc_Absyn_JoinEff_struct{int tag;struct Cyc_List_List*f1;};
struct Cyc_Absyn_RgnsEff_struct{int tag;void*f1;};struct Cyc_Absyn_NoTypes_struct{
int tag;struct Cyc_List_List*f1;struct Cyc_Position_Segment*f2;};struct Cyc_Absyn_WithTypes_struct{
int tag;struct Cyc_List_List*f1;int f2;struct Cyc_Absyn_VarargInfo*f3;struct Cyc_Core_Opt*
f4;struct Cyc_List_List*f5;};struct Cyc_Absyn_Regparm_att_struct{int tag;int f1;};
struct Cyc_Absyn_Aligned_att_struct{int tag;int f1;};struct Cyc_Absyn_Section_att_struct{
int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Format_att_struct{int tag;void*f1;
int f2;int f3;};struct Cyc_Absyn_Initializes_att_struct{int tag;int f1;};struct Cyc_Absyn_Mode_att_struct{
int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Carray_mod_struct{int tag;struct
Cyc_Absyn_Conref*f1;struct Cyc_Position_Segment*f2;};struct Cyc_Absyn_ConstArray_mod_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Conref*f2;struct Cyc_Position_Segment*
f3;};struct Cyc_Absyn_Pointer_mod_struct{int tag;struct Cyc_Absyn_PtrAtts f1;struct
Cyc_Absyn_Tqual f2;};struct Cyc_Absyn_Function_mod_struct{int tag;void*f1;};struct
Cyc_Absyn_TypeParams_mod_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Position_Segment*
f2;int f3;};struct Cyc_Absyn_Attributes_mod_struct{int tag;struct Cyc_Position_Segment*
f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Char_c_struct{int tag;void*f1;char f2;
};struct Cyc_Absyn_Short_c_struct{int tag;void*f1;short f2;};struct Cyc_Absyn_Int_c_struct{
int tag;void*f1;int f2;};struct Cyc_Absyn_LongLong_c_struct{int tag;void*f1;
long long f2;};struct Cyc_Absyn_Float_c_struct{int tag;struct _dyneither_ptr f1;};
struct Cyc_Absyn_String_c_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Null_c_struct{
int tag;};union Cyc_Absyn_Cnst_union{struct Cyc_Absyn_Char_c_struct Char_c;struct Cyc_Absyn_Short_c_struct
Short_c;struct Cyc_Absyn_Int_c_struct Int_c;struct Cyc_Absyn_LongLong_c_struct
LongLong_c;struct Cyc_Absyn_Float_c_struct Float_c;struct Cyc_Absyn_String_c_struct
String_c;struct Cyc_Absyn_Null_c_struct Null_c;};struct Cyc_Absyn_VarargCallInfo{
int num_varargs;struct Cyc_List_List*injectors;struct Cyc_Absyn_VarargInfo*vai;};
struct Cyc_Absyn_StructField_struct{int tag;struct _dyneither_ptr*f1;};struct Cyc_Absyn_TupleIndex_struct{
int tag;unsigned int f1;};struct Cyc_Absyn_MallocInfo{int is_calloc;struct Cyc_Absyn_Exp*
rgn;void**elt_type;struct Cyc_Absyn_Exp*num_elts;int fat_result;};struct Cyc_Absyn_Const_e_struct{
int tag;union Cyc_Absyn_Cnst_union f1;};struct Cyc_Absyn_Var_e_struct{int tag;struct
_tuple1*f1;void*f2;};struct Cyc_Absyn_UnknownId_e_struct{int tag;struct _tuple1*f1;
};struct Cyc_Absyn_Primop_e_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct
Cyc_Absyn_AssignOp_e_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Core_Opt*f2;
struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Increment_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;void*f2;};struct Cyc_Absyn_Conditional_e_struct{int tag;struct Cyc_Absyn_Exp*f1;
struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_And_e_struct{int
tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Or_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_SeqExp_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_UnknownCall_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_FnCall_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_VarargCallInfo*
f3;};struct Cyc_Absyn_Throw_e_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_NoInstantiate_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Instantiate_e_struct{int tag;
struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Cast_e_struct{
int tag;void*f1;struct Cyc_Absyn_Exp*f2;int f3;void*f4;};struct Cyc_Absyn_Address_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_New_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Sizeoftyp_e_struct{int tag;void*f1;};
struct Cyc_Absyn_Sizeofexp_e_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Offsetof_e_struct{
int tag;void*f1;void*f2;};struct Cyc_Absyn_Gentyp_e_struct{int tag;struct Cyc_List_List*
f1;void*f2;};struct Cyc_Absyn_Deref_e_struct{int tag;struct Cyc_Absyn_Exp*f1;};
struct Cyc_Absyn_AggrMember_e_struct{int tag;struct Cyc_Absyn_Exp*f1;struct
_dyneither_ptr*f2;};struct Cyc_Absyn_AggrArrow_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;struct _dyneither_ptr*f2;};struct Cyc_Absyn_Subscript_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Tuple_e_struct{int tag;struct Cyc_List_List*
f1;};struct _tuple2{struct Cyc_Core_Opt*f1;struct Cyc_Absyn_Tqual f2;void*f3;};
struct Cyc_Absyn_CompoundLit_e_struct{int tag;struct _tuple2*f1;struct Cyc_List_List*
f2;};struct Cyc_Absyn_Array_e_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Comprehension_e_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;
int f4;};struct Cyc_Absyn_Struct_e_struct{int tag;struct _tuple1*f1;struct Cyc_List_List*
f2;struct Cyc_List_List*f3;struct Cyc_Absyn_Aggrdecl*f4;};struct Cyc_Absyn_AnonStruct_e_struct{
int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Tunion_e_struct{int tag;
struct Cyc_List_List*f1;struct Cyc_Absyn_Tuniondecl*f2;struct Cyc_Absyn_Tunionfield*
f3;};struct Cyc_Absyn_Enum_e_struct{int tag;struct _tuple1*f1;struct Cyc_Absyn_Enumdecl*
f2;struct Cyc_Absyn_Enumfield*f3;};struct Cyc_Absyn_AnonEnum_e_struct{int tag;
struct _tuple1*f1;void*f2;struct Cyc_Absyn_Enumfield*f3;};struct Cyc_Absyn_Malloc_e_struct{
int tag;struct Cyc_Absyn_MallocInfo f1;};struct Cyc_Absyn_Swap_e_struct{int tag;
struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_UnresolvedMem_e_struct{
int tag;struct Cyc_Core_Opt*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_StmtExp_e_struct{
int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Valueof_e_struct{int tag;void*f1;
};struct Cyc_Absyn_Exp{struct Cyc_Core_Opt*topt;void*r;struct Cyc_Position_Segment*
loc;void*annot;};struct Cyc_Absyn_Exp_s_struct{int tag;struct Cyc_Absyn_Exp*f1;};
struct Cyc_Absyn_Seq_s_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_Absyn_Stmt*
f2;};struct Cyc_Absyn_Return_s_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_IfThenElse_s_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;struct Cyc_Absyn_Stmt*f3;};
struct _tuple3{struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_While_s_struct{
int tag;struct _tuple3 f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Break_s_struct{
int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Continue_s_struct{int tag;struct
Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Goto_s_struct{int tag;struct _dyneither_ptr*f1;
struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_For_s_struct{int tag;struct Cyc_Absyn_Exp*
f1;struct _tuple3 f2;struct _tuple3 f3;struct Cyc_Absyn_Stmt*f4;};struct Cyc_Absyn_Switch_s_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Fallthru_s_struct{
int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Switch_clause**f2;};struct Cyc_Absyn_Decl_s_struct{
int tag;struct Cyc_Absyn_Decl*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Label_s_struct{
int tag;struct _dyneither_ptr*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Do_s_struct{
int tag;struct Cyc_Absyn_Stmt*f1;struct _tuple3 f2;};struct Cyc_Absyn_TryCatch_s_struct{
int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Region_s_struct{
int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;int f3;struct Cyc_Absyn_Exp*
f4;struct Cyc_Absyn_Stmt*f5;};struct Cyc_Absyn_ResetRegion_s_struct{int tag;struct
Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Alias_s_struct{int tag;struct Cyc_Absyn_Exp*f1;
struct Cyc_Absyn_Tvar*f2;struct Cyc_Absyn_Vardecl*f3;struct Cyc_Absyn_Stmt*f4;};
struct Cyc_Absyn_Stmt{void*r;struct Cyc_Position_Segment*loc;struct Cyc_List_List*
non_local_preds;int try_depth;void*annot;};struct Cyc_Absyn_Var_p_struct{int tag;
struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_Reference_p_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_TagInt_p_struct{
int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;};struct Cyc_Absyn_Tuple_p_struct{
int tag;struct Cyc_List_List*f1;int f2;};struct Cyc_Absyn_Pointer_p_struct{int tag;
struct Cyc_Absyn_Pat*f1;};struct Cyc_Absyn_Aggr_p_struct{int tag;struct Cyc_Absyn_AggrInfo
f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;int f4;};struct Cyc_Absyn_Tunion_p_struct{
int tag;struct Cyc_Absyn_Tuniondecl*f1;struct Cyc_Absyn_Tunionfield*f2;struct Cyc_List_List*
f3;int f4;};struct Cyc_Absyn_Int_p_struct{int tag;void*f1;int f2;};struct Cyc_Absyn_Char_p_struct{
int tag;char f1;};struct Cyc_Absyn_Float_p_struct{int tag;struct _dyneither_ptr f1;};
struct Cyc_Absyn_Enum_p_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*
f2;};struct Cyc_Absyn_AnonEnum_p_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*
f2;};struct Cyc_Absyn_UnknownId_p_struct{int tag;struct _tuple1*f1;};struct Cyc_Absyn_UnknownCall_p_struct{
int tag;struct _tuple1*f1;struct Cyc_List_List*f2;int f3;};struct Cyc_Absyn_Exp_p_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Pat{void*r;struct Cyc_Core_Opt*
topt;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Switch_clause{struct Cyc_Absyn_Pat*
pattern;struct Cyc_Core_Opt*pat_vars;struct Cyc_Absyn_Exp*where_clause;struct Cyc_Absyn_Stmt*
body;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Global_b_struct{int tag;
struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Funname_b_struct{int tag;struct Cyc_Absyn_Fndecl*
f1;};struct Cyc_Absyn_Param_b_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct
Cyc_Absyn_Local_b_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Pat_b_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Vardecl{void*sc;struct
_tuple1*name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*initializer;
struct Cyc_Core_Opt*rgn;struct Cyc_List_List*attributes;int escapes;};struct Cyc_Absyn_Fndecl{
void*sc;int is_inline;struct _tuple1*name;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*
effect;void*ret_type;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*
cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_Absyn_Stmt*body;struct Cyc_Core_Opt*
cached_typ;struct Cyc_Core_Opt*param_vardecls;struct Cyc_Absyn_Vardecl*fn_vardecl;
struct Cyc_List_List*attributes;};struct Cyc_Absyn_Aggrfield{struct _dyneither_ptr*
name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*width;struct Cyc_List_List*
attributes;};struct Cyc_Absyn_AggrdeclImpl{struct Cyc_List_List*exist_vars;struct
Cyc_List_List*rgn_po;struct Cyc_List_List*fields;};struct Cyc_Absyn_Aggrdecl{void*
kind;void*sc;struct _tuple1*name;struct Cyc_List_List*tvs;struct Cyc_Absyn_AggrdeclImpl*
impl;struct Cyc_List_List*attributes;};struct Cyc_Absyn_Tunionfield{struct _tuple1*
name;struct Cyc_List_List*typs;struct Cyc_Position_Segment*loc;void*sc;};struct Cyc_Absyn_Tuniondecl{
void*sc;struct _tuple1*name;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*fields;int
is_xtunion;int is_flat;};struct Cyc_Absyn_Enumfield{struct _tuple1*name;struct Cyc_Absyn_Exp*
tag;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Enumdecl{void*sc;struct
_tuple1*name;struct Cyc_Core_Opt*fields;};struct Cyc_Absyn_Typedefdecl{struct
_tuple1*name;struct Cyc_Absyn_Tqual tq;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*
kind;struct Cyc_Core_Opt*defn;struct Cyc_List_List*atts;};struct Cyc_Absyn_Var_d_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Fn_d_struct{int tag;struct Cyc_Absyn_Fndecl*
f1;};struct Cyc_Absyn_Let_d_struct{int tag;struct Cyc_Absyn_Pat*f1;struct Cyc_Core_Opt*
f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Letv_d_struct{int tag;struct Cyc_List_List*
f1;};struct Cyc_Absyn_Aggr_d_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct
Cyc_Absyn_Tunion_d_struct{int tag;struct Cyc_Absyn_Tuniondecl*f1;};struct Cyc_Absyn_Enum_d_struct{
int tag;struct Cyc_Absyn_Enumdecl*f1;};struct Cyc_Absyn_Typedef_d_struct{int tag;
struct Cyc_Absyn_Typedefdecl*f1;};struct Cyc_Absyn_Namespace_d_struct{int tag;
struct _dyneither_ptr*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Using_d_struct{
int tag;struct _tuple1*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_ExternC_d_struct{
int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_ExternCinclude_d_struct{int tag;
struct Cyc_List_List*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Decl{void*r;
struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_ArrayElement_struct{int tag;
struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_FieldName_struct{int tag;struct
_dyneither_ptr*f1;};extern char Cyc_Absyn_EmptyAnnot[15];int Cyc_Absyn_qvar_cmp(
struct _tuple1*,struct _tuple1*);struct Cyc_Absyn_Conref*Cyc_Absyn_compress_conref(
struct Cyc_Absyn_Conref*x);void*Cyc_Absyn_conref_val(struct Cyc_Absyn_Conref*x);
void*Cyc_Absyn_conref_def(void*,struct Cyc_Absyn_Conref*x);extern struct Cyc_Absyn_Conref*
Cyc_Absyn_true_conref;extern struct Cyc_Absyn_Conref*Cyc_Absyn_false_conref;extern
void*Cyc_Absyn_char_typ;extern void*Cyc_Absyn_uint_typ;extern void*Cyc_Absyn_sint_typ;
extern void*Cyc_Absyn_exn_typ;extern void*Cyc_Absyn_bounds_one;void*Cyc_Absyn_star_typ(
void*t,void*rgn,struct Cyc_Absyn_Tqual tq,struct Cyc_Absyn_Conref*zero_term);void*
Cyc_Absyn_cstar_typ(void*t,struct Cyc_Absyn_Tqual tq);void*Cyc_Absyn_dyneither_typ(
void*t,void*rgn,struct Cyc_Absyn_Tqual tq,struct Cyc_Absyn_Conref*zero_term);void*
Cyc_Absyn_void_star_typ();void*Cyc_Absyn_strct(struct _dyneither_ptr*name);void*
Cyc_Absyn_strctq(struct _tuple1*name);void*Cyc_Absyn_unionq_typ(struct _tuple1*
name);void*Cyc_Absyn_array_typ(void*elt_type,struct Cyc_Absyn_Tqual tq,struct Cyc_Absyn_Exp*
num_elts,struct Cyc_Absyn_Conref*zero_term,struct Cyc_Position_Segment*ztloc);
struct Cyc_Absyn_Exp*Cyc_Absyn_new_exp(void*,struct Cyc_Position_Segment*);struct
Cyc_Absyn_Exp*Cyc_Absyn_copy_exp(struct Cyc_Absyn_Exp*);struct Cyc_Absyn_Exp*Cyc_Absyn_const_exp(
union Cyc_Absyn_Cnst_union,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_null_exp(
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_int_exp(void*,int,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_signed_int_exp(int,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_uint_exp(unsigned int,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_char_exp(char c,struct
Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_float_exp(struct
_dyneither_ptr f,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_string_exp(
struct _dyneither_ptr s,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_var_exp(
struct _tuple1*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_varb_exp(
struct _tuple1*,void*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_prim1_exp(
void*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_prim2_exp(
void*,struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_add_exp(struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_times_exp(struct Cyc_Absyn_Exp*,
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_divide_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*
Cyc_Absyn_eq_exp(struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_neq_exp(struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_lt_exp(struct Cyc_Absyn_Exp*,
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_lte_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*
Cyc_Absyn_assignop_exp(struct Cyc_Absyn_Exp*,struct Cyc_Core_Opt*,struct Cyc_Absyn_Exp*,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_assign_exp(struct Cyc_Absyn_Exp*,
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_post_inc_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_and_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*
Cyc_Absyn_fncall_exp(struct Cyc_Absyn_Exp*,struct Cyc_List_List*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_throw_exp(struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_cast_exp(void*,struct Cyc_Absyn_Exp*,int user_cast,
void*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_address_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_sizeoftyp_exp(
void*t,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_sizeofexp_exp(
struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_deref_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_aggrmember_exp(
struct Cyc_Absyn_Exp*,struct _dyneither_ptr*,struct Cyc_Position_Segment*);struct
Cyc_Absyn_Exp*Cyc_Absyn_aggrarrow_exp(struct Cyc_Absyn_Exp*,struct _dyneither_ptr*,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_subscript_exp(struct
Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*
Cyc_Absyn_stmt_exp(struct Cyc_Absyn_Stmt*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*
Cyc_Absyn_match_exn_exp(struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_valueof_exp(
void*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_unresolvedmem_exp(
struct Cyc_Core_Opt*,struct Cyc_List_List*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Stmt*
Cyc_Absyn_new_stmt(void*s,struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*
Cyc_Absyn_skip_stmt(struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_exp_stmt(
struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_seq_stmt(
struct Cyc_Absyn_Stmt*s1,struct Cyc_Absyn_Stmt*s2,struct Cyc_Position_Segment*loc);
struct Cyc_Absyn_Stmt*Cyc_Absyn_seq_stmts(struct Cyc_List_List*,struct Cyc_Position_Segment*
loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_return_stmt(struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*
loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_ifthenelse_stmt(struct Cyc_Absyn_Exp*e,struct
Cyc_Absyn_Stmt*s1,struct Cyc_Absyn_Stmt*s2,struct Cyc_Position_Segment*loc);struct
Cyc_Absyn_Stmt*Cyc_Absyn_for_stmt(struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,
struct Cyc_Absyn_Exp*e3,struct Cyc_Absyn_Stmt*s,struct Cyc_Position_Segment*loc);
struct Cyc_Absyn_Stmt*Cyc_Absyn_switch_stmt(struct Cyc_Absyn_Exp*e,struct Cyc_List_List*,
struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_decl_stmt(struct
Cyc_Absyn_Decl*d,struct Cyc_Absyn_Stmt*s,struct Cyc_Position_Segment*loc);struct
Cyc_Absyn_Stmt*Cyc_Absyn_declare_stmt(struct _tuple1*,void*,struct Cyc_Absyn_Exp*
init,struct Cyc_Absyn_Stmt*,struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*
Cyc_Absyn_label_stmt(struct _dyneither_ptr*v,struct Cyc_Absyn_Stmt*s,struct Cyc_Position_Segment*
loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_goto_stmt(struct _dyneither_ptr*lab,struct Cyc_Position_Segment*
loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_assign_stmt(struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*
e2,struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Decl*Cyc_Absyn_new_decl(void*
r,struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Vardecl*Cyc_Absyn_new_vardecl(
struct _tuple1*x,void*t,struct Cyc_Absyn_Exp*init);struct Cyc_Absyn_Vardecl*Cyc_Absyn_static_vardecl(
struct _tuple1*x,void*t,struct Cyc_Absyn_Exp*init);int Cyc_Absyn_is_lvalue(struct
Cyc_Absyn_Exp*);struct Cyc_Absyn_Aggrfield*Cyc_Absyn_lookup_field(struct Cyc_List_List*,
struct _dyneither_ptr*);struct Cyc_Absyn_Aggrfield*Cyc_Absyn_lookup_decl_field(
struct Cyc_Absyn_Aggrdecl*,struct _dyneither_ptr*);struct _dyneither_ptr*Cyc_Absyn_fieldname(
int);struct Cyc_Absyn_Aggrdecl*Cyc_Absyn_get_known_aggrdecl(union Cyc_Absyn_AggrInfoU_union
info);extern int Cyc_Absyn_no_regions;struct Cyc_PP_Ppstate;struct Cyc_PP_Out;struct
Cyc_PP_Doc;struct Cyc_Absynpp_Params{int expand_typedefs: 1;int qvar_to_Cids: 1;int
add_cyc_prefix: 1;int to_VC: 1;int decls_first: 1;int rewrite_temp_tvars: 1;int
print_all_tvars: 1;int print_all_kinds: 1;int print_all_effects: 1;int
print_using_stmts: 1;int print_externC_stmts: 1;int print_full_evars: 1;int
print_zeroterm: 1;int generate_line_directives: 1;int use_curr_namespace: 1;struct Cyc_List_List*
curr_namespace;};struct _dyneither_ptr Cyc_Absynpp_typ2string(void*);struct
_dyneither_ptr Cyc_Absynpp_exp2string(struct Cyc_Absyn_Exp*);struct _dyneither_ptr
Cyc_Absynpp_stmt2string(struct Cyc_Absyn_Stmt*);struct _dyneither_ptr Cyc_Absynpp_qvar2string(
struct _tuple1*);struct Cyc_RgnOrder_RgnPO;struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_initial_fn_po(
struct _RegionHandle*,struct Cyc_List_List*tvs,struct Cyc_List_List*po,void*effect,
struct Cyc_Absyn_Tvar*fst_rgn,struct Cyc_Position_Segment*);struct Cyc_RgnOrder_RgnPO*
Cyc_RgnOrder_add_outlives_constraint(struct _RegionHandle*,struct Cyc_RgnOrder_RgnPO*
po,void*eff,void*rgn,struct Cyc_Position_Segment*loc);struct Cyc_RgnOrder_RgnPO*
Cyc_RgnOrder_add_youngest(struct _RegionHandle*,struct Cyc_RgnOrder_RgnPO*po,
struct Cyc_Absyn_Tvar*rgn,int resetable,int opened);int Cyc_RgnOrder_is_region_resetable(
struct Cyc_RgnOrder_RgnPO*po,struct Cyc_Absyn_Tvar*r);int Cyc_RgnOrder_effect_outlives(
struct Cyc_RgnOrder_RgnPO*po,void*eff,void*rgn);int Cyc_RgnOrder_satisfies_constraints(
struct Cyc_RgnOrder_RgnPO*po,struct Cyc_List_List*constraints,void*default_bound,
int do_pin);int Cyc_RgnOrder_eff_outlives_eff(struct Cyc_RgnOrder_RgnPO*po,void*
eff1,void*eff2);void Cyc_RgnOrder_print_region_po(struct Cyc_RgnOrder_RgnPO*po);
struct Cyc_Tcenv_CList{void*hd;struct Cyc_Tcenv_CList*tl;};struct Cyc_Tcenv_VarRes_struct{
int tag;void*f1;};struct Cyc_Tcenv_AggrRes_struct{int tag;struct Cyc_Absyn_Aggrdecl*
f1;};struct Cyc_Tcenv_TunionRes_struct{int tag;struct Cyc_Absyn_Tuniondecl*f1;
struct Cyc_Absyn_Tunionfield*f2;};struct Cyc_Tcenv_EnumRes_struct{int tag;struct Cyc_Absyn_Enumdecl*
f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Tcenv_AnonEnumRes_struct{int tag;void*
f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Tcenv_Genv{struct _RegionHandle*grgn;
struct Cyc_Set_Set*namespaces;struct Cyc_Dict_Dict aggrdecls;struct Cyc_Dict_Dict
tuniondecls;struct Cyc_Dict_Dict enumdecls;struct Cyc_Dict_Dict typedefs;struct Cyc_Dict_Dict
ordinaries;struct Cyc_List_List*availables;};struct Cyc_Tcenv_Fenv;struct Cyc_Tcenv_Stmt_j_struct{
int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Tcenv_Tenv{struct Cyc_List_List*ns;
struct Cyc_Dict_Dict ae;struct Cyc_Tcenv_Fenv*le;int allow_valueof;};void*Cyc_Tcutil_impos(
struct _dyneither_ptr fmt,struct _dyneither_ptr ap);void Cyc_Tcutil_terr(struct Cyc_Position_Segment*,
struct _dyneither_ptr fmt,struct _dyneither_ptr ap);void Cyc_Tcutil_warn(struct Cyc_Position_Segment*,
struct _dyneither_ptr fmt,struct _dyneither_ptr ap);void*Cyc_Tcutil_tvar_kind(struct
Cyc_Absyn_Tvar*t);void*Cyc_Tcutil_typ_kind(void*t);void*Cyc_Tcutil_compress(void*
t);int Cyc_Tcutil_is_pointer_type(void*t);int Cyc_Tcutil_is_pointer_or_boxed(void*
t,int*is_dyneither_ptr);int Cyc_Tcutil_unify(void*,void*);struct Cyc_List_List*Cyc_Tcutil_resolve_struct_designators(
struct _RegionHandle*rgn,struct Cyc_Position_Segment*loc,struct Cyc_List_List*des,
struct Cyc_List_List*fields);int Cyc_Tcutil_is_tagged_pointer_typ(void*);int Cyc_Tcutil_is_tagged_pointer_typ_elt(
void*t,void**elt_typ_dest);int Cyc_Tcutil_is_zero_pointer_typ_elt(void*t,void**
elt_typ_dest);int Cyc_Tcutil_is_zero_ptr_deref(struct Cyc_Absyn_Exp*e1,void**
ptr_type,int*is_dyneither,void**elt_type);struct _tuple4{struct Cyc_Absyn_Tqual f1;
void*f2;};void*Cyc_Tcutil_snd_tqt(struct _tuple4*);struct _tuple5{unsigned int f1;
int f2;};struct _tuple5 Cyc_Evexp_eval_const_uint_exp(struct Cyc_Absyn_Exp*e);int Cyc_Evexp_c_can_eval(
struct Cyc_Absyn_Exp*e);int Cyc_Evexp_same_const_exp(struct Cyc_Absyn_Exp*e1,struct
Cyc_Absyn_Exp*e2);int Cyc_Evexp_lte_const_exp(struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*
e2);struct Cyc_CfFlowInfo_VarRoot_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};
struct Cyc_CfFlowInfo_MallocPt_struct{int tag;struct Cyc_Absyn_Exp*f1;void*f2;};
struct Cyc_CfFlowInfo_InitParam_struct{int tag;int f1;void*f2;};struct Cyc_CfFlowInfo_Place{
void*root;struct Cyc_List_List*fields;};struct Cyc_CfFlowInfo_EqualConst_struct{
int tag;unsigned int f1;};struct Cyc_CfFlowInfo_LessVar_struct{int tag;struct Cyc_Absyn_Vardecl*
f1;void*f2;};struct Cyc_CfFlowInfo_LessNumelts_struct{int tag;struct Cyc_Absyn_Vardecl*
f1;};struct Cyc_CfFlowInfo_LessConst_struct{int tag;unsigned int f1;};struct Cyc_CfFlowInfo_LessEqNumelts_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;};union Cyc_CfFlowInfo_RelnOp_union{struct Cyc_CfFlowInfo_EqualConst_struct
EqualConst;struct Cyc_CfFlowInfo_LessVar_struct LessVar;struct Cyc_CfFlowInfo_LessNumelts_struct
LessNumelts;struct Cyc_CfFlowInfo_LessConst_struct LessConst;struct Cyc_CfFlowInfo_LessEqNumelts_struct
LessEqNumelts;};struct Cyc_CfFlowInfo_Reln{struct Cyc_Absyn_Vardecl*vd;union Cyc_CfFlowInfo_RelnOp_union
rop;};struct Cyc_CfFlowInfo_TagCmp{void*cmp;void*bd;};extern char Cyc_CfFlowInfo_HasTagCmps[
15];struct Cyc_CfFlowInfo_HasTagCmps_struct{char*tag;struct Cyc_List_List*f1;};
extern char Cyc_CfFlowInfo_IsZero[11];extern char Cyc_CfFlowInfo_NotZero[12];struct
Cyc_CfFlowInfo_NotZero_struct{char*tag;struct Cyc_List_List*f1;};extern char Cyc_CfFlowInfo_UnknownZ[
13];struct Cyc_CfFlowInfo_UnknownZ_struct{char*tag;struct Cyc_List_List*f1;};
struct Cyc_CfFlowInfo_PlaceL_struct{int tag;struct Cyc_CfFlowInfo_Place*f1;};struct
Cyc_CfFlowInfo_UnknownL_struct{int tag;};union Cyc_CfFlowInfo_AbsLVal_union{struct
Cyc_CfFlowInfo_PlaceL_struct PlaceL;struct Cyc_CfFlowInfo_UnknownL_struct UnknownL;
};struct Cyc_CfFlowInfo_UnknownR_struct{int tag;void*f1;};struct Cyc_CfFlowInfo_Esc_struct{
int tag;void*f1;};struct Cyc_CfFlowInfo_AddressOf_struct{int tag;struct Cyc_CfFlowInfo_Place*
f1;};struct Cyc_CfFlowInfo_TagCmps_struct{int tag;struct Cyc_List_List*f1;};struct
Cyc_CfFlowInfo_Aggregate_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_CfFlowInfo_ConsumeInfo{
struct Cyc_Dict_Dict consumed;struct Cyc_List_List*may_consume;};struct Cyc_CfFlowInfo_BottomFL_struct{
int tag;};struct Cyc_CfFlowInfo_ReachableFL_struct{int tag;struct Cyc_Dict_Dict f1;
struct Cyc_List_List*f2;struct Cyc_CfFlowInfo_ConsumeInfo f3;};union Cyc_CfFlowInfo_FlowInfo_union{
struct Cyc_CfFlowInfo_BottomFL_struct BottomFL;struct Cyc_CfFlowInfo_ReachableFL_struct
ReachableFL;};struct Cyc_CfFlowInfo_FlowEnv{struct _RegionHandle*r;void*
unknown_none;void*unknown_this;void*unknown_all;void*esc_none;void*esc_this;void*
esc_all;struct Cyc_Dict_Dict mt_flowdict;struct Cyc_Dict_Dict mt_place_set;struct Cyc_CfFlowInfo_Place*
dummy_place;};struct Cyc_CfFlowInfo_Region_k_struct{int tag;struct Cyc_Absyn_Tvar*
f1;};struct Cyc_List_List*Cyc_Toc_toc(struct Cyc_List_List*ds);struct _tuple1*Cyc_Toc_temp_var();
extern struct _dyneither_ptr Cyc_Toc_globals;extern int Cyc_noexpand_r;int Cyc_Toc_warn_bounds_checks=
0;int Cyc_Toc_warn_all_null_deref=0;unsigned int Cyc_Toc_total_bounds_checks=0;
unsigned int Cyc_Toc_bounds_checks_eliminated=0;static struct Cyc_List_List*Cyc_Toc_result_decls=
0;struct Cyc_Toc_TocState{struct _DynRegionHandle*dyn;struct Cyc_List_List**
tuple_types;struct Cyc_Dict_Dict*aggrs_so_far;struct Cyc_Set_Set**tunions_so_far;
struct Cyc_Dict_Dict*xtunions_so_far;struct Cyc_Dict_Dict*qvar_tags;struct Cyc_Xarray_Xarray*
temp_labels;};static struct Cyc_Toc_TocState*Cyc_Toc_toc_state=0;struct _tuple6{
struct _tuple1*f1;struct _dyneither_ptr f2;};int Cyc_Toc_qvar_tag_cmp(struct _tuple6*
x,struct _tuple6*y);int Cyc_Toc_qvar_tag_cmp(struct _tuple6*x,struct _tuple6*y){
struct _tuple1*_tmp1;struct _dyneither_ptr _tmp2;struct _tuple6 _tmp0=*x;_tmp1=_tmp0.f1;
_tmp2=_tmp0.f2;{struct _tuple1*_tmp4;struct _dyneither_ptr _tmp5;struct _tuple6 _tmp3=*
y;_tmp4=_tmp3.f1;_tmp5=_tmp3.f2;{int i=Cyc_Absyn_qvar_cmp(_tmp1,_tmp4);if(i != 0)
return i;return Cyc_strcmp((struct _dyneither_ptr)_tmp2,(struct _dyneither_ptr)_tmp5);}}}
struct _tuple7{struct Cyc_Absyn_Aggrdecl*f1;void*f2;};void*Cyc_Toc_aggrdecl_type(
struct _tuple1*q,void*(*type_maker)(struct _tuple1*));void*Cyc_Toc_aggrdecl_type(
struct _tuple1*q,void*(*type_maker)(struct _tuple1*)){struct _DynRegionHandle*_tmp7;
struct Cyc_Dict_Dict*_tmp8;struct Cyc_Toc_TocState _tmp6=*((struct Cyc_Toc_TocState*)
_check_null(Cyc_Toc_toc_state));_tmp7=_tmp6.dyn;_tmp8=_tmp6.aggrs_so_far;{struct
_DynRegionFrame _tmp9;struct _RegionHandle*d=_open_dynregion(& _tmp9,_tmp7);{struct
_tuple7**v=((struct _tuple7**(*)(struct Cyc_Dict_Dict d,struct _tuple1*k))Cyc_Dict_lookup_opt)(*
_tmp8,q);if(v == 0){void*_tmpA=type_maker(q);_npop_handler(0);return _tmpA;}else{
struct _tuple7 _tmpC;void*_tmpD;struct _tuple7*_tmpB=*v;_tmpC=*_tmpB;_tmpD=_tmpC.f2;{
void*_tmpE=_tmpD;_npop_handler(0);return _tmpE;}}};_pop_dynregion(d);}}static int
Cyc_Toc_tuple_type_counter=0;static int Cyc_Toc_temp_var_counter=0;static int Cyc_Toc_fresh_label_counter=
0;char Cyc_Toc_Toc_Unimplemented[22]="\000\000\000\000Toc_Unimplemented\000";char
Cyc_Toc_Toc_Impossible[19]="\000\000\000\000Toc_Impossible\000";static void*Cyc_Toc_unimp(
struct _dyneither_ptr fmt,struct _dyneither_ptr ap);static void*Cyc_Toc_unimp(struct
_dyneither_ptr fmt,struct _dyneither_ptr ap){Cyc_vfprintf(Cyc_stderr,fmt,ap);{const
char*_tmp837;void*_tmp836;(_tmp836=0,Cyc_fprintf(Cyc_stderr,((_tmp837="\n",
_tag_dyneither(_tmp837,sizeof(char),2))),_tag_dyneither(_tmp836,sizeof(void*),0)));}
Cyc_fflush((struct Cyc___cycFILE*)Cyc_stderr);(int)_throw((void*)Cyc_Toc_Toc_Unimplemented);}
static void*Cyc_Toc_toc_impos(struct _dyneither_ptr fmt,struct _dyneither_ptr ap);
static void*Cyc_Toc_toc_impos(struct _dyneither_ptr fmt,struct _dyneither_ptr ap){Cyc_vfprintf(
Cyc_stderr,fmt,ap);{const char*_tmp83A;void*_tmp839;(_tmp839=0,Cyc_fprintf(Cyc_stderr,((
_tmp83A="\n",_tag_dyneither(_tmp83A,sizeof(char),2))),_tag_dyneither(_tmp839,
sizeof(void*),0)));}Cyc_fflush((struct Cyc___cycFILE*)Cyc_stderr);(int)_throw((
void*)Cyc_Toc_Toc_Impossible);}char Cyc_Toc_Match_error[16]="\000\000\000\000Match_error\000";
static char _tmp13[5]="curr";static struct _dyneither_ptr Cyc_Toc_curr_string={_tmp13,
_tmp13,_tmp13 + 5};static struct _dyneither_ptr*Cyc_Toc_curr_sp=& Cyc_Toc_curr_string;
static char _tmp14[4]="tag";static struct _dyneither_ptr Cyc_Toc_tag_string={_tmp14,
_tmp14,_tmp14 + 4};static struct _dyneither_ptr*Cyc_Toc_tag_sp=& Cyc_Toc_tag_string;
static char _tmp15[4]="val";static struct _dyneither_ptr Cyc_Toc_val_string={_tmp15,
_tmp15,_tmp15 + 4};static struct _dyneither_ptr*Cyc_Toc_val_sp=& Cyc_Toc_val_string;
static char _tmp16[14]="_handler_cons";static struct _dyneither_ptr Cyc_Toc__handler_cons_string={
_tmp16,_tmp16,_tmp16 + 14};static struct _dyneither_ptr*Cyc_Toc__handler_cons_sp=&
Cyc_Toc__handler_cons_string;static char _tmp17[8]="handler";static struct
_dyneither_ptr Cyc_Toc_handler_string={_tmp17,_tmp17,_tmp17 + 8};static struct
_dyneither_ptr*Cyc_Toc_handler_sp=& Cyc_Toc_handler_string;static char _tmp18[14]="_RegionHandle";
static struct _dyneither_ptr Cyc_Toc__RegionHandle_string={_tmp18,_tmp18,_tmp18 + 14};
static struct _dyneither_ptr*Cyc_Toc__RegionHandle_sp=& Cyc_Toc__RegionHandle_string;
static char _tmp19[17]="_DynRegionHandle";static struct _dyneither_ptr Cyc_Toc__DynRegionHandle_string={
_tmp19,_tmp19,_tmp19 + 17};static struct _dyneither_ptr*Cyc_Toc__DynRegionHandle_sp=&
Cyc_Toc__DynRegionHandle_string;static char _tmp1A[16]="_DynRegionFrame";static
struct _dyneither_ptr Cyc_Toc__DynRegionFrame_string={_tmp1A,_tmp1A,_tmp1A + 16};
static struct _dyneither_ptr*Cyc_Toc__DynRegionFrame_sp=& Cyc_Toc__DynRegionFrame_string;
struct _dyneither_ptr Cyc_Toc_globals={(void*)0,(void*)0,(void*)(0 + 0)};static char
_tmp1B[7]="_throw";static struct _dyneither_ptr Cyc_Toc__throw_str={_tmp1B,_tmp1B,
_tmp1B + 7};static struct _tuple1 Cyc_Toc__throw_pr={(union Cyc_Absyn_Nmspace_union)((
struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__throw_str};static struct Cyc_Absyn_Var_e_struct
Cyc_Toc__throw_re={1,& Cyc_Toc__throw_pr,(void*)((void*)0)};static struct Cyc_Absyn_Exp
Cyc_Toc__throw_ev={0,(void*)((void*)& Cyc_Toc__throw_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__throw_e=& Cyc_Toc__throw_ev;static char _tmp1E[7]="setjmp";
static struct _dyneither_ptr Cyc_Toc_setjmp_str={_tmp1E,_tmp1E,_tmp1E + 7};static
struct _tuple1 Cyc_Toc_setjmp_pr={(union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){
0}),& Cyc_Toc_setjmp_str};static struct Cyc_Absyn_Var_e_struct Cyc_Toc_setjmp_re={1,&
Cyc_Toc_setjmp_pr,(void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc_setjmp_ev={
0,(void*)((void*)& Cyc_Toc_setjmp_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc_setjmp_e=& Cyc_Toc_setjmp_ev;static char _tmp21[
14]="_push_handler";static struct _dyneither_ptr Cyc_Toc__push_handler_str={_tmp21,
_tmp21,_tmp21 + 14};static struct _tuple1 Cyc_Toc__push_handler_pr={(union Cyc_Absyn_Nmspace_union)((
struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__push_handler_str};static struct Cyc_Absyn_Var_e_struct
Cyc_Toc__push_handler_re={1,& Cyc_Toc__push_handler_pr,(void*)((void*)0)};static
struct Cyc_Absyn_Exp Cyc_Toc__push_handler_ev={0,(void*)((void*)& Cyc_Toc__push_handler_re),
0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static struct Cyc_Absyn_Exp*Cyc_Toc__push_handler_e=&
Cyc_Toc__push_handler_ev;static char _tmp24[13]="_pop_handler";static struct
_dyneither_ptr Cyc_Toc__pop_handler_str={_tmp24,_tmp24,_tmp24 + 13};static struct
_tuple1 Cyc_Toc__pop_handler_pr={(union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){
0}),& Cyc_Toc__pop_handler_str};static struct Cyc_Absyn_Var_e_struct Cyc_Toc__pop_handler_re={
1,& Cyc_Toc__pop_handler_pr,(void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__pop_handler_ev={
0,(void*)((void*)& Cyc_Toc__pop_handler_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__pop_handler_e=& Cyc_Toc__pop_handler_ev;static
char _tmp27[12]="_exn_thrown";static struct _dyneither_ptr Cyc_Toc__exn_thrown_str={
_tmp27,_tmp27,_tmp27 + 12};static struct _tuple1 Cyc_Toc__exn_thrown_pr={(union Cyc_Absyn_Nmspace_union)((
struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__exn_thrown_str};static struct Cyc_Absyn_Var_e_struct
Cyc_Toc__exn_thrown_re={1,& Cyc_Toc__exn_thrown_pr,(void*)((void*)0)};static
struct Cyc_Absyn_Exp Cyc_Toc__exn_thrown_ev={0,(void*)((void*)& Cyc_Toc__exn_thrown_re),
0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static struct Cyc_Absyn_Exp*Cyc_Toc__exn_thrown_e=&
Cyc_Toc__exn_thrown_ev;static char _tmp2A[14]="_npop_handler";static struct
_dyneither_ptr Cyc_Toc__npop_handler_str={_tmp2A,_tmp2A,_tmp2A + 14};static struct
_tuple1 Cyc_Toc__npop_handler_pr={(union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){
0}),& Cyc_Toc__npop_handler_str};static struct Cyc_Absyn_Var_e_struct Cyc_Toc__npop_handler_re={
1,& Cyc_Toc__npop_handler_pr,(void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__npop_handler_ev={
0,(void*)((void*)& Cyc_Toc__npop_handler_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__npop_handler_e=& Cyc_Toc__npop_handler_ev;
static char _tmp2D[12]="_check_null";static struct _dyneither_ptr Cyc_Toc__check_null_str={
_tmp2D,_tmp2D,_tmp2D + 12};static struct _tuple1 Cyc_Toc__check_null_pr={(union Cyc_Absyn_Nmspace_union)((
struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__check_null_str};static struct Cyc_Absyn_Var_e_struct
Cyc_Toc__check_null_re={1,& Cyc_Toc__check_null_pr,(void*)((void*)0)};static
struct Cyc_Absyn_Exp Cyc_Toc__check_null_ev={0,(void*)((void*)& Cyc_Toc__check_null_re),
0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static struct Cyc_Absyn_Exp*Cyc_Toc__check_null_e=&
Cyc_Toc__check_null_ev;static char _tmp30[28]="_check_known_subscript_null";static
struct _dyneither_ptr Cyc_Toc__check_known_subscript_null_str={_tmp30,_tmp30,
_tmp30 + 28};static struct _tuple1 Cyc_Toc__check_known_subscript_null_pr={(union Cyc_Absyn_Nmspace_union)((
struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__check_known_subscript_null_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__check_known_subscript_null_re={1,& Cyc_Toc__check_known_subscript_null_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__check_known_subscript_null_ev={
0,(void*)((void*)& Cyc_Toc__check_known_subscript_null_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__check_known_subscript_null_e=& Cyc_Toc__check_known_subscript_null_ev;
static char _tmp33[31]="_check_known_subscript_notnull";static struct _dyneither_ptr
Cyc_Toc__check_known_subscript_notnull_str={_tmp33,_tmp33,_tmp33 + 31};static
struct _tuple1 Cyc_Toc__check_known_subscript_notnull_pr={(union Cyc_Absyn_Nmspace_union)((
struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__check_known_subscript_notnull_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__check_known_subscript_notnull_re={1,&
Cyc_Toc__check_known_subscript_notnull_pr,(void*)((void*)0)};static struct Cyc_Absyn_Exp
Cyc_Toc__check_known_subscript_notnull_ev={0,(void*)((void*)& Cyc_Toc__check_known_subscript_notnull_re),
0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static struct Cyc_Absyn_Exp*Cyc_Toc__check_known_subscript_notnull_e=&
Cyc_Toc__check_known_subscript_notnull_ev;static char _tmp36[27]="_check_dyneither_subscript";
static struct _dyneither_ptr Cyc_Toc__check_dyneither_subscript_str={_tmp36,_tmp36,
_tmp36 + 27};static struct _tuple1 Cyc_Toc__check_dyneither_subscript_pr={(union Cyc_Absyn_Nmspace_union)((
struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__check_dyneither_subscript_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__check_dyneither_subscript_re={1,& Cyc_Toc__check_dyneither_subscript_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__check_dyneither_subscript_ev={
0,(void*)((void*)& Cyc_Toc__check_dyneither_subscript_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__check_dyneither_subscript_e=& Cyc_Toc__check_dyneither_subscript_ev;
static char _tmp39[15]="_dyneither_ptr";static struct _dyneither_ptr Cyc_Toc__dyneither_ptr_str={
_tmp39,_tmp39,_tmp39 + 15};static struct _tuple1 Cyc_Toc__dyneither_ptr_pr={(union
Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__dyneither_ptr_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__dyneither_ptr_re={1,& Cyc_Toc__dyneither_ptr_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__dyneither_ptr_ev={0,(void*)((
void*)& Cyc_Toc__dyneither_ptr_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static
struct Cyc_Absyn_Exp*Cyc_Toc__dyneither_ptr_e=& Cyc_Toc__dyneither_ptr_ev;static
char _tmp3C[15]="_tag_dyneither";static struct _dyneither_ptr Cyc_Toc__tag_dyneither_str={
_tmp3C,_tmp3C,_tmp3C + 15};static struct _tuple1 Cyc_Toc__tag_dyneither_pr={(union
Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__tag_dyneither_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__tag_dyneither_re={1,& Cyc_Toc__tag_dyneither_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__tag_dyneither_ev={0,(void*)((
void*)& Cyc_Toc__tag_dyneither_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static
struct Cyc_Absyn_Exp*Cyc_Toc__tag_dyneither_e=& Cyc_Toc__tag_dyneither_ev;static
char _tmp3F[20]="_init_dyneither_ptr";static struct _dyneither_ptr Cyc_Toc__init_dyneither_ptr_str={
_tmp3F,_tmp3F,_tmp3F + 20};static struct _tuple1 Cyc_Toc__init_dyneither_ptr_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__init_dyneither_ptr_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__init_dyneither_ptr_re={1,& Cyc_Toc__init_dyneither_ptr_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__init_dyneither_ptr_ev={0,(
void*)((void*)& Cyc_Toc__init_dyneither_ptr_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__init_dyneither_ptr_e=& Cyc_Toc__init_dyneither_ptr_ev;
static char _tmp42[21]="_untag_dyneither_ptr";static struct _dyneither_ptr Cyc_Toc__untag_dyneither_ptr_str={
_tmp42,_tmp42,_tmp42 + 21};static struct _tuple1 Cyc_Toc__untag_dyneither_ptr_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__untag_dyneither_ptr_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__untag_dyneither_ptr_re={1,& Cyc_Toc__untag_dyneither_ptr_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__untag_dyneither_ptr_ev={0,(
void*)((void*)& Cyc_Toc__untag_dyneither_ptr_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__untag_dyneither_ptr_e=& Cyc_Toc__untag_dyneither_ptr_ev;
static char _tmp45[20]="_get_dyneither_size";static struct _dyneither_ptr Cyc_Toc__get_dyneither_size_str={
_tmp45,_tmp45,_tmp45 + 20};static struct _tuple1 Cyc_Toc__get_dyneither_size_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__get_dyneither_size_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__get_dyneither_size_re={1,& Cyc_Toc__get_dyneither_size_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__get_dyneither_size_ev={0,(
void*)((void*)& Cyc_Toc__get_dyneither_size_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__get_dyneither_size_e=& Cyc_Toc__get_dyneither_size_ev;
static char _tmp48[19]="_get_zero_arr_size";static struct _dyneither_ptr Cyc_Toc__get_zero_arr_size_str={
_tmp48,_tmp48,_tmp48 + 19};static struct _tuple1 Cyc_Toc__get_zero_arr_size_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__get_zero_arr_size_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__get_zero_arr_size_re={1,& Cyc_Toc__get_zero_arr_size_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__get_zero_arr_size_ev={0,(
void*)((void*)& Cyc_Toc__get_zero_arr_size_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__get_zero_arr_size_e=& Cyc_Toc__get_zero_arr_size_ev;
static char _tmp4B[24]="_get_zero_arr_size_char";static struct _dyneither_ptr Cyc_Toc__get_zero_arr_size_char_str={
_tmp4B,_tmp4B,_tmp4B + 24};static struct _tuple1 Cyc_Toc__get_zero_arr_size_char_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__get_zero_arr_size_char_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__get_zero_arr_size_char_re={1,& Cyc_Toc__get_zero_arr_size_char_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__get_zero_arr_size_char_ev={0,(
void*)((void*)& Cyc_Toc__get_zero_arr_size_char_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__get_zero_arr_size_char_e=& Cyc_Toc__get_zero_arr_size_char_ev;
static char _tmp4E[25]="_get_zero_arr_size_short";static struct _dyneither_ptr Cyc_Toc__get_zero_arr_size_short_str={
_tmp4E,_tmp4E,_tmp4E + 25};static struct _tuple1 Cyc_Toc__get_zero_arr_size_short_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__get_zero_arr_size_short_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__get_zero_arr_size_short_re={1,& Cyc_Toc__get_zero_arr_size_short_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__get_zero_arr_size_short_ev={
0,(void*)((void*)& Cyc_Toc__get_zero_arr_size_short_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__get_zero_arr_size_short_e=& Cyc_Toc__get_zero_arr_size_short_ev;
static char _tmp51[23]="_get_zero_arr_size_int";static struct _dyneither_ptr Cyc_Toc__get_zero_arr_size_int_str={
_tmp51,_tmp51,_tmp51 + 23};static struct _tuple1 Cyc_Toc__get_zero_arr_size_int_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__get_zero_arr_size_int_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__get_zero_arr_size_int_re={1,& Cyc_Toc__get_zero_arr_size_int_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__get_zero_arr_size_int_ev={0,(
void*)((void*)& Cyc_Toc__get_zero_arr_size_int_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__get_zero_arr_size_int_e=& Cyc_Toc__get_zero_arr_size_int_ev;
static char _tmp54[25]="_get_zero_arr_size_float";static struct _dyneither_ptr Cyc_Toc__get_zero_arr_size_float_str={
_tmp54,_tmp54,_tmp54 + 25};static struct _tuple1 Cyc_Toc__get_zero_arr_size_float_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__get_zero_arr_size_float_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__get_zero_arr_size_float_re={1,& Cyc_Toc__get_zero_arr_size_float_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__get_zero_arr_size_float_ev={
0,(void*)((void*)& Cyc_Toc__get_zero_arr_size_float_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__get_zero_arr_size_float_e=& Cyc_Toc__get_zero_arr_size_float_ev;
static char _tmp57[26]="_get_zero_arr_size_double";static struct _dyneither_ptr Cyc_Toc__get_zero_arr_size_double_str={
_tmp57,_tmp57,_tmp57 + 26};static struct _tuple1 Cyc_Toc__get_zero_arr_size_double_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__get_zero_arr_size_double_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__get_zero_arr_size_double_re={1,& Cyc_Toc__get_zero_arr_size_double_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__get_zero_arr_size_double_ev={
0,(void*)((void*)& Cyc_Toc__get_zero_arr_size_double_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__get_zero_arr_size_double_e=& Cyc_Toc__get_zero_arr_size_double_ev;
static char _tmp5A[30]="_get_zero_arr_size_longdouble";static struct _dyneither_ptr
Cyc_Toc__get_zero_arr_size_longdouble_str={_tmp5A,_tmp5A,_tmp5A + 30};static
struct _tuple1 Cyc_Toc__get_zero_arr_size_longdouble_pr={(union Cyc_Absyn_Nmspace_union)((
struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__get_zero_arr_size_longdouble_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__get_zero_arr_size_longdouble_re={1,&
Cyc_Toc__get_zero_arr_size_longdouble_pr,(void*)((void*)0)};static struct Cyc_Absyn_Exp
Cyc_Toc__get_zero_arr_size_longdouble_ev={0,(void*)((void*)& Cyc_Toc__get_zero_arr_size_longdouble_re),
0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static struct Cyc_Absyn_Exp*Cyc_Toc__get_zero_arr_size_longdouble_e=&
Cyc_Toc__get_zero_arr_size_longdouble_ev;static char _tmp5D[28]="_get_zero_arr_size_voidstar";
static struct _dyneither_ptr Cyc_Toc__get_zero_arr_size_voidstar_str={_tmp5D,_tmp5D,
_tmp5D + 28};static struct _tuple1 Cyc_Toc__get_zero_arr_size_voidstar_pr={(union Cyc_Absyn_Nmspace_union)((
struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__get_zero_arr_size_voidstar_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__get_zero_arr_size_voidstar_re={1,& Cyc_Toc__get_zero_arr_size_voidstar_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__get_zero_arr_size_voidstar_ev={
0,(void*)((void*)& Cyc_Toc__get_zero_arr_size_voidstar_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__get_zero_arr_size_voidstar_e=& Cyc_Toc__get_zero_arr_size_voidstar_ev;
static char _tmp60[20]="_dyneither_ptr_plus";static struct _dyneither_ptr Cyc_Toc__dyneither_ptr_plus_str={
_tmp60,_tmp60,_tmp60 + 20};static struct _tuple1 Cyc_Toc__dyneither_ptr_plus_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__dyneither_ptr_plus_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__dyneither_ptr_plus_re={1,& Cyc_Toc__dyneither_ptr_plus_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__dyneither_ptr_plus_ev={0,(
void*)((void*)& Cyc_Toc__dyneither_ptr_plus_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__dyneither_ptr_plus_e=& Cyc_Toc__dyneither_ptr_plus_ev;
static char _tmp63[15]="_zero_arr_plus";static struct _dyneither_ptr Cyc_Toc__zero_arr_plus_str={
_tmp63,_tmp63,_tmp63 + 15};static struct _tuple1 Cyc_Toc__zero_arr_plus_pr={(union
Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_plus_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_plus_re={1,& Cyc_Toc__zero_arr_plus_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__zero_arr_plus_ev={0,(void*)((
void*)& Cyc_Toc__zero_arr_plus_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static
struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_plus_e=& Cyc_Toc__zero_arr_plus_ev;static
char _tmp66[20]="_zero_arr_plus_char";static struct _dyneither_ptr Cyc_Toc__zero_arr_plus_char_str={
_tmp66,_tmp66,_tmp66 + 20};static struct _tuple1 Cyc_Toc__zero_arr_plus_char_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_plus_char_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_plus_char_re={1,& Cyc_Toc__zero_arr_plus_char_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__zero_arr_plus_char_ev={0,(
void*)((void*)& Cyc_Toc__zero_arr_plus_char_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_plus_char_e=& Cyc_Toc__zero_arr_plus_char_ev;
static char _tmp69[21]="_zero_arr_plus_short";static struct _dyneither_ptr Cyc_Toc__zero_arr_plus_short_str={
_tmp69,_tmp69,_tmp69 + 21};static struct _tuple1 Cyc_Toc__zero_arr_plus_short_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_plus_short_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_plus_short_re={1,& Cyc_Toc__zero_arr_plus_short_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__zero_arr_plus_short_ev={0,(
void*)((void*)& Cyc_Toc__zero_arr_plus_short_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_plus_short_e=& Cyc_Toc__zero_arr_plus_short_ev;
static char _tmp6C[19]="_zero_arr_plus_int";static struct _dyneither_ptr Cyc_Toc__zero_arr_plus_int_str={
_tmp6C,_tmp6C,_tmp6C + 19};static struct _tuple1 Cyc_Toc__zero_arr_plus_int_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_plus_int_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_plus_int_re={1,& Cyc_Toc__zero_arr_plus_int_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__zero_arr_plus_int_ev={0,(
void*)((void*)& Cyc_Toc__zero_arr_plus_int_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_plus_int_e=& Cyc_Toc__zero_arr_plus_int_ev;
static char _tmp6F[21]="_zero_arr_plus_float";static struct _dyneither_ptr Cyc_Toc__zero_arr_plus_float_str={
_tmp6F,_tmp6F,_tmp6F + 21};static struct _tuple1 Cyc_Toc__zero_arr_plus_float_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_plus_float_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_plus_float_re={1,& Cyc_Toc__zero_arr_plus_float_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__zero_arr_plus_float_ev={0,(
void*)((void*)& Cyc_Toc__zero_arr_plus_float_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_plus_float_e=& Cyc_Toc__zero_arr_plus_float_ev;
static char _tmp72[22]="_zero_arr_plus_double";static struct _dyneither_ptr Cyc_Toc__zero_arr_plus_double_str={
_tmp72,_tmp72,_tmp72 + 22};static struct _tuple1 Cyc_Toc__zero_arr_plus_double_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_plus_double_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_plus_double_re={1,& Cyc_Toc__zero_arr_plus_double_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__zero_arr_plus_double_ev={0,(
void*)((void*)& Cyc_Toc__zero_arr_plus_double_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_plus_double_e=& Cyc_Toc__zero_arr_plus_double_ev;
static char _tmp75[26]="_zero_arr_plus_longdouble";static struct _dyneither_ptr Cyc_Toc__zero_arr_plus_longdouble_str={
_tmp75,_tmp75,_tmp75 + 26};static struct _tuple1 Cyc_Toc__zero_arr_plus_longdouble_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_plus_longdouble_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_plus_longdouble_re={1,& Cyc_Toc__zero_arr_plus_longdouble_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__zero_arr_plus_longdouble_ev={
0,(void*)((void*)& Cyc_Toc__zero_arr_plus_longdouble_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_plus_longdouble_e=& Cyc_Toc__zero_arr_plus_longdouble_ev;
static char _tmp78[24]="_zero_arr_plus_voidstar";static struct _dyneither_ptr Cyc_Toc__zero_arr_plus_voidstar_str={
_tmp78,_tmp78,_tmp78 + 24};static struct _tuple1 Cyc_Toc__zero_arr_plus_voidstar_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_plus_voidstar_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_plus_voidstar_re={1,& Cyc_Toc__zero_arr_plus_voidstar_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__zero_arr_plus_voidstar_ev={0,(
void*)((void*)& Cyc_Toc__zero_arr_plus_voidstar_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_plus_voidstar_e=& Cyc_Toc__zero_arr_plus_voidstar_ev;
static char _tmp7B[28]="_dyneither_ptr_inplace_plus";static struct _dyneither_ptr Cyc_Toc__dyneither_ptr_inplace_plus_str={
_tmp7B,_tmp7B,_tmp7B + 28};static struct _tuple1 Cyc_Toc__dyneither_ptr_inplace_plus_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__dyneither_ptr_inplace_plus_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__dyneither_ptr_inplace_plus_re={1,& Cyc_Toc__dyneither_ptr_inplace_plus_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__dyneither_ptr_inplace_plus_ev={
0,(void*)((void*)& Cyc_Toc__dyneither_ptr_inplace_plus_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__dyneither_ptr_inplace_plus_e=& Cyc_Toc__dyneither_ptr_inplace_plus_ev;
static char _tmp7E[23]="_zero_arr_inplace_plus";static struct _dyneither_ptr Cyc_Toc__zero_arr_inplace_plus_str={
_tmp7E,_tmp7E,_tmp7E + 23};static struct _tuple1 Cyc_Toc__zero_arr_inplace_plus_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_inplace_plus_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_inplace_plus_re={1,& Cyc_Toc__zero_arr_inplace_plus_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__zero_arr_inplace_plus_ev={0,(
void*)((void*)& Cyc_Toc__zero_arr_inplace_plus_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_inplace_plus_e=& Cyc_Toc__zero_arr_inplace_plus_ev;
static char _tmp81[28]="_zero_arr_inplace_plus_char";static struct _dyneither_ptr Cyc_Toc__zero_arr_inplace_plus_char_str={
_tmp81,_tmp81,_tmp81 + 28};static struct _tuple1 Cyc_Toc__zero_arr_inplace_plus_char_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_inplace_plus_char_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_inplace_plus_char_re={1,& Cyc_Toc__zero_arr_inplace_plus_char_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__zero_arr_inplace_plus_char_ev={
0,(void*)((void*)& Cyc_Toc__zero_arr_inplace_plus_char_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_inplace_plus_char_e=& Cyc_Toc__zero_arr_inplace_plus_char_ev;
static char _tmp84[29]="_zero_arr_inplace_plus_short";static struct _dyneither_ptr
Cyc_Toc__zero_arr_inplace_plus_short_str={_tmp84,_tmp84,_tmp84 + 29};static struct
_tuple1 Cyc_Toc__zero_arr_inplace_plus_short_pr={(union Cyc_Absyn_Nmspace_union)((
struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_inplace_plus_short_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_inplace_plus_short_re={1,&
Cyc_Toc__zero_arr_inplace_plus_short_pr,(void*)((void*)0)};static struct Cyc_Absyn_Exp
Cyc_Toc__zero_arr_inplace_plus_short_ev={0,(void*)((void*)& Cyc_Toc__zero_arr_inplace_plus_short_re),
0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_inplace_plus_short_e=&
Cyc_Toc__zero_arr_inplace_plus_short_ev;static char _tmp87[27]="_zero_arr_inplace_plus_int";
static struct _dyneither_ptr Cyc_Toc__zero_arr_inplace_plus_int_str={_tmp87,_tmp87,
_tmp87 + 27};static struct _tuple1 Cyc_Toc__zero_arr_inplace_plus_int_pr={(union Cyc_Absyn_Nmspace_union)((
struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_inplace_plus_int_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_inplace_plus_int_re={1,& Cyc_Toc__zero_arr_inplace_plus_int_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__zero_arr_inplace_plus_int_ev={
0,(void*)((void*)& Cyc_Toc__zero_arr_inplace_plus_int_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_inplace_plus_int_e=& Cyc_Toc__zero_arr_inplace_plus_int_ev;
static char _tmp8A[29]="_zero_arr_inplace_plus_float";static struct _dyneither_ptr
Cyc_Toc__zero_arr_inplace_plus_float_str={_tmp8A,_tmp8A,_tmp8A + 29};static struct
_tuple1 Cyc_Toc__zero_arr_inplace_plus_float_pr={(union Cyc_Absyn_Nmspace_union)((
struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_inplace_plus_float_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_inplace_plus_float_re={1,&
Cyc_Toc__zero_arr_inplace_plus_float_pr,(void*)((void*)0)};static struct Cyc_Absyn_Exp
Cyc_Toc__zero_arr_inplace_plus_float_ev={0,(void*)((void*)& Cyc_Toc__zero_arr_inplace_plus_float_re),
0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_inplace_plus_float_e=&
Cyc_Toc__zero_arr_inplace_plus_float_ev;static char _tmp8D[30]="_zero_arr_inplace_plus_double";
static struct _dyneither_ptr Cyc_Toc__zero_arr_inplace_plus_double_str={_tmp8D,
_tmp8D,_tmp8D + 30};static struct _tuple1 Cyc_Toc__zero_arr_inplace_plus_double_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_inplace_plus_double_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_inplace_plus_double_re={1,&
Cyc_Toc__zero_arr_inplace_plus_double_pr,(void*)((void*)0)};static struct Cyc_Absyn_Exp
Cyc_Toc__zero_arr_inplace_plus_double_ev={0,(void*)((void*)& Cyc_Toc__zero_arr_inplace_plus_double_re),
0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_inplace_plus_double_e=&
Cyc_Toc__zero_arr_inplace_plus_double_ev;static char _tmp90[34]="_zero_arr_inplace_plus_longdouble";
static struct _dyneither_ptr Cyc_Toc__zero_arr_inplace_plus_longdouble_str={_tmp90,
_tmp90,_tmp90 + 34};static struct _tuple1 Cyc_Toc__zero_arr_inplace_plus_longdouble_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_inplace_plus_longdouble_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_inplace_plus_longdouble_re={
1,& Cyc_Toc__zero_arr_inplace_plus_longdouble_pr,(void*)((void*)0)};static struct
Cyc_Absyn_Exp Cyc_Toc__zero_arr_inplace_plus_longdouble_ev={0,(void*)((void*)& Cyc_Toc__zero_arr_inplace_plus_longdouble_re),
0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_inplace_plus_longdouble_e=&
Cyc_Toc__zero_arr_inplace_plus_longdouble_ev;static char _tmp93[32]="_zero_arr_inplace_plus_voidstar";
static struct _dyneither_ptr Cyc_Toc__zero_arr_inplace_plus_voidstar_str={_tmp93,
_tmp93,_tmp93 + 32};static struct _tuple1 Cyc_Toc__zero_arr_inplace_plus_voidstar_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_inplace_plus_voidstar_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_inplace_plus_voidstar_re={1,&
Cyc_Toc__zero_arr_inplace_plus_voidstar_pr,(void*)((void*)0)};static struct Cyc_Absyn_Exp
Cyc_Toc__zero_arr_inplace_plus_voidstar_ev={0,(void*)((void*)& Cyc_Toc__zero_arr_inplace_plus_voidstar_re),
0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_inplace_plus_voidstar_e=&
Cyc_Toc__zero_arr_inplace_plus_voidstar_ev;static char _tmp96[33]="_dyneither_ptr_inplace_plus_post";
static struct _dyneither_ptr Cyc_Toc__dyneither_ptr_inplace_plus_post_str={_tmp96,
_tmp96,_tmp96 + 33};static struct _tuple1 Cyc_Toc__dyneither_ptr_inplace_plus_post_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__dyneither_ptr_inplace_plus_post_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__dyneither_ptr_inplace_plus_post_re={1,&
Cyc_Toc__dyneither_ptr_inplace_plus_post_pr,(void*)((void*)0)};static struct Cyc_Absyn_Exp
Cyc_Toc__dyneither_ptr_inplace_plus_post_ev={0,(void*)((void*)& Cyc_Toc__dyneither_ptr_inplace_plus_post_re),
0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static struct Cyc_Absyn_Exp*Cyc_Toc__dyneither_ptr_inplace_plus_post_e=&
Cyc_Toc__dyneither_ptr_inplace_plus_post_ev;static char _tmp99[28]="_zero_arr_inplace_plus_post";
static struct _dyneither_ptr Cyc_Toc__zero_arr_inplace_plus_post_str={_tmp99,_tmp99,
_tmp99 + 28};static struct _tuple1 Cyc_Toc__zero_arr_inplace_plus_post_pr={(union Cyc_Absyn_Nmspace_union)((
struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_inplace_plus_post_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_inplace_plus_post_re={1,& Cyc_Toc__zero_arr_inplace_plus_post_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__zero_arr_inplace_plus_post_ev={
0,(void*)((void*)& Cyc_Toc__zero_arr_inplace_plus_post_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_inplace_plus_post_e=& Cyc_Toc__zero_arr_inplace_plus_post_ev;
static char _tmp9C[33]="_zero_arr_inplace_plus_post_char";static struct
_dyneither_ptr Cyc_Toc__zero_arr_inplace_plus_post_char_str={_tmp9C,_tmp9C,_tmp9C
+ 33};static struct _tuple1 Cyc_Toc__zero_arr_inplace_plus_post_char_pr={(union Cyc_Absyn_Nmspace_union)((
struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_inplace_plus_post_char_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_inplace_plus_post_char_re={1,&
Cyc_Toc__zero_arr_inplace_plus_post_char_pr,(void*)((void*)0)};static struct Cyc_Absyn_Exp
Cyc_Toc__zero_arr_inplace_plus_post_char_ev={0,(void*)((void*)& Cyc_Toc__zero_arr_inplace_plus_post_char_re),
0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_inplace_plus_post_char_e=&
Cyc_Toc__zero_arr_inplace_plus_post_char_ev;static char _tmp9F[34]="_zero_arr_inplace_plus_post_short";
static struct _dyneither_ptr Cyc_Toc__zero_arr_inplace_plus_post_short_str={_tmp9F,
_tmp9F,_tmp9F + 34};static struct _tuple1 Cyc_Toc__zero_arr_inplace_plus_post_short_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_inplace_plus_post_short_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_inplace_plus_post_short_re={
1,& Cyc_Toc__zero_arr_inplace_plus_post_short_pr,(void*)((void*)0)};static struct
Cyc_Absyn_Exp Cyc_Toc__zero_arr_inplace_plus_post_short_ev={0,(void*)((void*)& Cyc_Toc__zero_arr_inplace_plus_post_short_re),
0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_inplace_plus_post_short_e=&
Cyc_Toc__zero_arr_inplace_plus_post_short_ev;static char _tmpA2[32]="_zero_arr_inplace_plus_post_int";
static struct _dyneither_ptr Cyc_Toc__zero_arr_inplace_plus_post_int_str={_tmpA2,
_tmpA2,_tmpA2 + 32};static struct _tuple1 Cyc_Toc__zero_arr_inplace_plus_post_int_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_inplace_plus_post_int_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_inplace_plus_post_int_re={1,&
Cyc_Toc__zero_arr_inplace_plus_post_int_pr,(void*)((void*)0)};static struct Cyc_Absyn_Exp
Cyc_Toc__zero_arr_inplace_plus_post_int_ev={0,(void*)((void*)& Cyc_Toc__zero_arr_inplace_plus_post_int_re),
0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_inplace_plus_post_int_e=&
Cyc_Toc__zero_arr_inplace_plus_post_int_ev;static char _tmpA5[34]="_zero_arr_inplace_plus_post_float";
static struct _dyneither_ptr Cyc_Toc__zero_arr_inplace_plus_post_float_str={_tmpA5,
_tmpA5,_tmpA5 + 34};static struct _tuple1 Cyc_Toc__zero_arr_inplace_plus_post_float_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_inplace_plus_post_float_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_inplace_plus_post_float_re={
1,& Cyc_Toc__zero_arr_inplace_plus_post_float_pr,(void*)((void*)0)};static struct
Cyc_Absyn_Exp Cyc_Toc__zero_arr_inplace_plus_post_float_ev={0,(void*)((void*)& Cyc_Toc__zero_arr_inplace_plus_post_float_re),
0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_inplace_plus_post_float_e=&
Cyc_Toc__zero_arr_inplace_plus_post_float_ev;static char _tmpA8[35]="_zero_arr_inplace_plus_post_double";
static struct _dyneither_ptr Cyc_Toc__zero_arr_inplace_plus_post_double_str={_tmpA8,
_tmpA8,_tmpA8 + 35};static struct _tuple1 Cyc_Toc__zero_arr_inplace_plus_post_double_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_inplace_plus_post_double_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_inplace_plus_post_double_re={
1,& Cyc_Toc__zero_arr_inplace_plus_post_double_pr,(void*)((void*)0)};static struct
Cyc_Absyn_Exp Cyc_Toc__zero_arr_inplace_plus_post_double_ev={0,(void*)((void*)&
Cyc_Toc__zero_arr_inplace_plus_post_double_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_inplace_plus_post_double_e=& Cyc_Toc__zero_arr_inplace_plus_post_double_ev;
static char _tmpAB[39]="_zero_arr_inplace_plus_post_longdouble";static struct
_dyneither_ptr Cyc_Toc__zero_arr_inplace_plus_post_longdouble_str={_tmpAB,_tmpAB,
_tmpAB + 39};static struct _tuple1 Cyc_Toc__zero_arr_inplace_plus_post_longdouble_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_inplace_plus_post_longdouble_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_inplace_plus_post_longdouble_re={
1,& Cyc_Toc__zero_arr_inplace_plus_post_longdouble_pr,(void*)((void*)0)};static
struct Cyc_Absyn_Exp Cyc_Toc__zero_arr_inplace_plus_post_longdouble_ev={0,(void*)((
void*)& Cyc_Toc__zero_arr_inplace_plus_post_longdouble_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_inplace_plus_post_longdouble_e=& Cyc_Toc__zero_arr_inplace_plus_post_longdouble_ev;
static char _tmpAE[37]="_zero_arr_inplace_plus_post_voidstar";static struct
_dyneither_ptr Cyc_Toc__zero_arr_inplace_plus_post_voidstar_str={_tmpAE,_tmpAE,
_tmpAE + 37};static struct _tuple1 Cyc_Toc__zero_arr_inplace_plus_post_voidstar_pr={(
union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__zero_arr_inplace_plus_post_voidstar_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__zero_arr_inplace_plus_post_voidstar_re={
1,& Cyc_Toc__zero_arr_inplace_plus_post_voidstar_pr,(void*)((void*)0)};static
struct Cyc_Absyn_Exp Cyc_Toc__zero_arr_inplace_plus_post_voidstar_ev={0,(void*)((
void*)& Cyc_Toc__zero_arr_inplace_plus_post_voidstar_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__zero_arr_inplace_plus_post_voidstar_e=& Cyc_Toc__zero_arr_inplace_plus_post_voidstar_ev;
static char _tmpB1[10]="_cycalloc";static struct _dyneither_ptr Cyc_Toc__cycalloc_str={
_tmpB1,_tmpB1,_tmpB1 + 10};static struct _tuple1 Cyc_Toc__cycalloc_pr={(union Cyc_Absyn_Nmspace_union)((
struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__cycalloc_str};static struct Cyc_Absyn_Var_e_struct
Cyc_Toc__cycalloc_re={1,& Cyc_Toc__cycalloc_pr,(void*)((void*)0)};static struct Cyc_Absyn_Exp
Cyc_Toc__cycalloc_ev={0,(void*)((void*)& Cyc_Toc__cycalloc_re),0,(void*)((void*)
Cyc_Absyn_EmptyAnnot)};static struct Cyc_Absyn_Exp*Cyc_Toc__cycalloc_e=& Cyc_Toc__cycalloc_ev;
static char _tmpB4[11]="_cyccalloc";static struct _dyneither_ptr Cyc_Toc__cyccalloc_str={
_tmpB4,_tmpB4,_tmpB4 + 11};static struct _tuple1 Cyc_Toc__cyccalloc_pr={(union Cyc_Absyn_Nmspace_union)((
struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__cyccalloc_str};static struct Cyc_Absyn_Var_e_struct
Cyc_Toc__cyccalloc_re={1,& Cyc_Toc__cyccalloc_pr,(void*)((void*)0)};static struct
Cyc_Absyn_Exp Cyc_Toc__cyccalloc_ev={0,(void*)((void*)& Cyc_Toc__cyccalloc_re),0,(
void*)((void*)Cyc_Absyn_EmptyAnnot)};static struct Cyc_Absyn_Exp*Cyc_Toc__cyccalloc_e=&
Cyc_Toc__cyccalloc_ev;static char _tmpB7[17]="_cycalloc_atomic";static struct
_dyneither_ptr Cyc_Toc__cycalloc_atomic_str={_tmpB7,_tmpB7,_tmpB7 + 17};static
struct _tuple1 Cyc_Toc__cycalloc_atomic_pr={(union Cyc_Absyn_Nmspace_union)((struct
Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__cycalloc_atomic_str};static struct Cyc_Absyn_Var_e_struct
Cyc_Toc__cycalloc_atomic_re={1,& Cyc_Toc__cycalloc_atomic_pr,(void*)((void*)0)};
static struct Cyc_Absyn_Exp Cyc_Toc__cycalloc_atomic_ev={0,(void*)((void*)& Cyc_Toc__cycalloc_atomic_re),
0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static struct Cyc_Absyn_Exp*Cyc_Toc__cycalloc_atomic_e=&
Cyc_Toc__cycalloc_atomic_ev;static char _tmpBA[18]="_cyccalloc_atomic";static
struct _dyneither_ptr Cyc_Toc__cyccalloc_atomic_str={_tmpBA,_tmpBA,_tmpBA + 18};
static struct _tuple1 Cyc_Toc__cyccalloc_atomic_pr={(union Cyc_Absyn_Nmspace_union)((
struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__cyccalloc_atomic_str};static struct
Cyc_Absyn_Var_e_struct Cyc_Toc__cyccalloc_atomic_re={1,& Cyc_Toc__cyccalloc_atomic_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__cyccalloc_atomic_ev={0,(void*)((
void*)& Cyc_Toc__cyccalloc_atomic_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__cyccalloc_atomic_e=& Cyc_Toc__cyccalloc_atomic_ev;
static char _tmpBD[15]="_region_malloc";static struct _dyneither_ptr Cyc_Toc__region_malloc_str={
_tmpBD,_tmpBD,_tmpBD + 15};static struct _tuple1 Cyc_Toc__region_malloc_pr={(union
Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__region_malloc_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__region_malloc_re={1,& Cyc_Toc__region_malloc_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__region_malloc_ev={0,(void*)((
void*)& Cyc_Toc__region_malloc_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static
struct Cyc_Absyn_Exp*Cyc_Toc__region_malloc_e=& Cyc_Toc__region_malloc_ev;static
char _tmpC0[15]="_region_calloc";static struct _dyneither_ptr Cyc_Toc__region_calloc_str={
_tmpC0,_tmpC0,_tmpC0 + 15};static struct _tuple1 Cyc_Toc__region_calloc_pr={(union
Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__region_calloc_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__region_calloc_re={1,& Cyc_Toc__region_calloc_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__region_calloc_ev={0,(void*)((
void*)& Cyc_Toc__region_calloc_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static
struct Cyc_Absyn_Exp*Cyc_Toc__region_calloc_e=& Cyc_Toc__region_calloc_ev;static
char _tmpC3[13]="_check_times";static struct _dyneither_ptr Cyc_Toc__check_times_str={
_tmpC3,_tmpC3,_tmpC3 + 13};static struct _tuple1 Cyc_Toc__check_times_pr={(union Cyc_Absyn_Nmspace_union)((
struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__check_times_str};static struct Cyc_Absyn_Var_e_struct
Cyc_Toc__check_times_re={1,& Cyc_Toc__check_times_pr,(void*)((void*)0)};static
struct Cyc_Absyn_Exp Cyc_Toc__check_times_ev={0,(void*)((void*)& Cyc_Toc__check_times_re),
0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static struct Cyc_Absyn_Exp*Cyc_Toc__check_times_e=&
Cyc_Toc__check_times_ev;static char _tmpC6[12]="_new_region";static struct
_dyneither_ptr Cyc_Toc__new_region_str={_tmpC6,_tmpC6,_tmpC6 + 12};static struct
_tuple1 Cyc_Toc__new_region_pr={(union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){
0}),& Cyc_Toc__new_region_str};static struct Cyc_Absyn_Var_e_struct Cyc_Toc__new_region_re={
1,& Cyc_Toc__new_region_pr,(void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__new_region_ev={
0,(void*)((void*)& Cyc_Toc__new_region_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__new_region_e=& Cyc_Toc__new_region_ev;static
char _tmpC9[13]="_push_region";static struct _dyneither_ptr Cyc_Toc__push_region_str={
_tmpC9,_tmpC9,_tmpC9 + 13};static struct _tuple1 Cyc_Toc__push_region_pr={(union Cyc_Absyn_Nmspace_union)((
struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__push_region_str};static struct Cyc_Absyn_Var_e_struct
Cyc_Toc__push_region_re={1,& Cyc_Toc__push_region_pr,(void*)((void*)0)};static
struct Cyc_Absyn_Exp Cyc_Toc__push_region_ev={0,(void*)((void*)& Cyc_Toc__push_region_re),
0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static struct Cyc_Absyn_Exp*Cyc_Toc__push_region_e=&
Cyc_Toc__push_region_ev;static char _tmpCC[12]="_pop_region";static struct
_dyneither_ptr Cyc_Toc__pop_region_str={_tmpCC,_tmpCC,_tmpCC + 12};static struct
_tuple1 Cyc_Toc__pop_region_pr={(union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){
0}),& Cyc_Toc__pop_region_str};static struct Cyc_Absyn_Var_e_struct Cyc_Toc__pop_region_re={
1,& Cyc_Toc__pop_region_pr,(void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__pop_region_ev={
0,(void*)((void*)& Cyc_Toc__pop_region_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__pop_region_e=& Cyc_Toc__pop_region_ev;static
char _tmpCF[16]="_open_dynregion";static struct _dyneither_ptr Cyc_Toc__open_dynregion_str={
_tmpCF,_tmpCF,_tmpCF + 16};static struct _tuple1 Cyc_Toc__open_dynregion_pr={(union
Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__open_dynregion_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__open_dynregion_re={1,& Cyc_Toc__open_dynregion_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__open_dynregion_ev={0,(void*)((
void*)& Cyc_Toc__open_dynregion_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__open_dynregion_e=& Cyc_Toc__open_dynregion_ev;
static char _tmpD2[16]="_push_dynregion";static struct _dyneither_ptr Cyc_Toc__push_dynregion_str={
_tmpD2,_tmpD2,_tmpD2 + 16};static struct _tuple1 Cyc_Toc__push_dynregion_pr={(union
Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__push_dynregion_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__push_dynregion_re={1,& Cyc_Toc__push_dynregion_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__push_dynregion_ev={0,(void*)((
void*)& Cyc_Toc__push_dynregion_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__push_dynregion_e=& Cyc_Toc__push_dynregion_ev;
static char _tmpD5[15]="_pop_dynregion";static struct _dyneither_ptr Cyc_Toc__pop_dynregion_str={
_tmpD5,_tmpD5,_tmpD5 + 15};static struct _tuple1 Cyc_Toc__pop_dynregion_pr={(union
Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__pop_dynregion_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__pop_dynregion_re={1,& Cyc_Toc__pop_dynregion_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__pop_dynregion_ev={0,(void*)((
void*)& Cyc_Toc__pop_dynregion_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static
struct Cyc_Absyn_Exp*Cyc_Toc__pop_dynregion_e=& Cyc_Toc__pop_dynregion_ev;static
char _tmpD8[14]="_reset_region";static struct _dyneither_ptr Cyc_Toc__reset_region_str={
_tmpD8,_tmpD8,_tmpD8 + 14};static struct _tuple1 Cyc_Toc__reset_region_pr={(union Cyc_Absyn_Nmspace_union)((
struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__reset_region_str};static struct Cyc_Absyn_Var_e_struct
Cyc_Toc__reset_region_re={1,& Cyc_Toc__reset_region_pr,(void*)((void*)0)};static
struct Cyc_Absyn_Exp Cyc_Toc__reset_region_ev={0,(void*)((void*)& Cyc_Toc__reset_region_re),
0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static struct Cyc_Absyn_Exp*Cyc_Toc__reset_region_e=&
Cyc_Toc__reset_region_ev;static char _tmpDB[19]="_throw_arraybounds";static struct
_dyneither_ptr Cyc_Toc__throw_arraybounds_str={_tmpDB,_tmpDB,_tmpDB + 19};static
struct _tuple1 Cyc_Toc__throw_arraybounds_pr={(union Cyc_Absyn_Nmspace_union)((
struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__throw_arraybounds_str};static struct
Cyc_Absyn_Var_e_struct Cyc_Toc__throw_arraybounds_re={1,& Cyc_Toc__throw_arraybounds_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__throw_arraybounds_ev={0,(
void*)((void*)& Cyc_Toc__throw_arraybounds_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__throw_arraybounds_e=& Cyc_Toc__throw_arraybounds_ev;
static char _tmpDE[29]="_dyneither_ptr_decrease_size";static struct _dyneither_ptr
Cyc_Toc__dyneither_ptr_decrease_size_str={_tmpDE,_tmpDE,_tmpDE + 29};static struct
_tuple1 Cyc_Toc__dyneither_ptr_decrease_size_pr={(union Cyc_Absyn_Nmspace_union)((
struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__dyneither_ptr_decrease_size_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__dyneither_ptr_decrease_size_re={1,&
Cyc_Toc__dyneither_ptr_decrease_size_pr,(void*)((void*)0)};static struct Cyc_Absyn_Exp
Cyc_Toc__dyneither_ptr_decrease_size_ev={0,(void*)((void*)& Cyc_Toc__dyneither_ptr_decrease_size_re),
0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};static struct Cyc_Absyn_Exp*Cyc_Toc__dyneither_ptr_decrease_size_e=&
Cyc_Toc__dyneither_ptr_decrease_size_ev;static char _tmpE1[11]="_swap_word";static
struct _dyneither_ptr Cyc_Toc__swap_word_str={_tmpE1,_tmpE1,_tmpE1 + 11};static
struct _tuple1 Cyc_Toc__swap_word_pr={(union Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){
0}),& Cyc_Toc__swap_word_str};static struct Cyc_Absyn_Var_e_struct Cyc_Toc__swap_word_re={
1,& Cyc_Toc__swap_word_pr,(void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__swap_word_ev={
0,(void*)((void*)& Cyc_Toc__swap_word_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__swap_word_e=& Cyc_Toc__swap_word_ev;static char
_tmpE4[16]="_swap_dyneither";static struct _dyneither_ptr Cyc_Toc__swap_dyneither_str={
_tmpE4,_tmpE4,_tmpE4 + 16};static struct _tuple1 Cyc_Toc__swap_dyneither_pr={(union
Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& Cyc_Toc__swap_dyneither_str};
static struct Cyc_Absyn_Var_e_struct Cyc_Toc__swap_dyneither_re={1,& Cyc_Toc__swap_dyneither_pr,(
void*)((void*)0)};static struct Cyc_Absyn_Exp Cyc_Toc__swap_dyneither_ev={0,(void*)((
void*)& Cyc_Toc__swap_dyneither_re),0,(void*)((void*)Cyc_Absyn_EmptyAnnot)};
static struct Cyc_Absyn_Exp*Cyc_Toc__swap_dyneither_e=& Cyc_Toc__swap_dyneither_ev;
static struct Cyc_Absyn_AggrType_struct Cyc_Toc_dyneither_ptr_typ_v={10,{(union Cyc_Absyn_AggrInfoU_union)((
struct Cyc_Absyn_UnknownAggr_struct){0,(void*)((void*)0),& Cyc_Toc__dyneither_ptr_pr}),
0}};static void*Cyc_Toc_dyneither_ptr_typ=(void*)& Cyc_Toc_dyneither_ptr_typ_v;
static struct Cyc_Absyn_Tqual Cyc_Toc_mt_tq={0,0,0,0,0};static struct Cyc_Absyn_Stmt*
Cyc_Toc_skip_stmt_dl();static struct Cyc_Absyn_Stmt*Cyc_Toc_skip_stmt_dl(){static
struct Cyc_Absyn_Stmt**skip_stmt_opt=0;if(skip_stmt_opt == 0){struct Cyc_Absyn_Stmt**
_tmp83B;skip_stmt_opt=((_tmp83B=_cycalloc(sizeof(*_tmp83B)),((_tmp83B[0]=Cyc_Absyn_skip_stmt(
0),_tmp83B))));}return*skip_stmt_opt;}static struct Cyc_Absyn_Exp*Cyc_Toc_cast_it(
void*t,struct Cyc_Absyn_Exp*e);static struct Cyc_Absyn_Exp*Cyc_Toc_cast_it(void*t,
struct Cyc_Absyn_Exp*e){return Cyc_Absyn_cast_exp(t,e,0,(void*)1,0);}static void*
Cyc_Toc_cast_it_r(void*t,struct Cyc_Absyn_Exp*e);static void*Cyc_Toc_cast_it_r(
void*t,struct Cyc_Absyn_Exp*e){struct Cyc_Absyn_Cast_e_struct _tmp83E;struct Cyc_Absyn_Cast_e_struct*
_tmp83D;return(void*)((_tmp83D=_cycalloc(sizeof(*_tmp83D)),((_tmp83D[0]=((
_tmp83E.tag=15,((_tmp83E.f1=(void*)t,((_tmp83E.f2=e,((_tmp83E.f3=0,((_tmp83E.f4=(
void*)((void*)1),_tmp83E)))))))))),_tmp83D))));}static void*Cyc_Toc_deref_exp_r(
struct Cyc_Absyn_Exp*e);static void*Cyc_Toc_deref_exp_r(struct Cyc_Absyn_Exp*e){
struct Cyc_Absyn_Deref_e_struct _tmp841;struct Cyc_Absyn_Deref_e_struct*_tmp840;
return(void*)((_tmp840=_cycalloc(sizeof(*_tmp840)),((_tmp840[0]=((_tmp841.tag=22,((
_tmp841.f1=e,_tmp841)))),_tmp840))));}static void*Cyc_Toc_subscript_exp_r(struct
Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2);static void*Cyc_Toc_subscript_exp_r(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2){struct Cyc_Absyn_Subscript_e_struct
_tmp844;struct Cyc_Absyn_Subscript_e_struct*_tmp843;return(void*)((_tmp843=
_cycalloc(sizeof(*_tmp843)),((_tmp843[0]=((_tmp844.tag=25,((_tmp844.f1=e1,((
_tmp844.f2=e2,_tmp844)))))),_tmp843))));}static void*Cyc_Toc_stmt_exp_r(struct Cyc_Absyn_Stmt*
s);static void*Cyc_Toc_stmt_exp_r(struct Cyc_Absyn_Stmt*s){struct Cyc_Absyn_StmtExp_e_struct
_tmp847;struct Cyc_Absyn_StmtExp_e_struct*_tmp846;return(void*)((_tmp846=
_cycalloc(sizeof(*_tmp846)),((_tmp846[0]=((_tmp847.tag=38,((_tmp847.f1=s,_tmp847)))),
_tmp846))));}static void*Cyc_Toc_sizeoftyp_exp_r(void*t);static void*Cyc_Toc_sizeoftyp_exp_r(
void*t){struct Cyc_Absyn_Sizeoftyp_e_struct _tmp84A;struct Cyc_Absyn_Sizeoftyp_e_struct*
_tmp849;return(void*)((_tmp849=_cycalloc(sizeof(*_tmp849)),((_tmp849[0]=((
_tmp84A.tag=18,((_tmp84A.f1=(void*)t,_tmp84A)))),_tmp849))));}static void*Cyc_Toc_fncall_exp_r(
struct Cyc_Absyn_Exp*e,struct Cyc_List_List*es);static void*Cyc_Toc_fncall_exp_r(
struct Cyc_Absyn_Exp*e,struct Cyc_List_List*es){struct Cyc_Absyn_FnCall_e_struct
_tmp84D;struct Cyc_Absyn_FnCall_e_struct*_tmp84C;return(void*)((_tmp84C=_cycalloc(
sizeof(*_tmp84C)),((_tmp84C[0]=((_tmp84D.tag=11,((_tmp84D.f1=e,((_tmp84D.f2=es,((
_tmp84D.f3=0,_tmp84D)))))))),_tmp84C))));}static void*Cyc_Toc_exp_stmt_r(struct
Cyc_Absyn_Exp*e);static void*Cyc_Toc_exp_stmt_r(struct Cyc_Absyn_Exp*e){struct Cyc_Absyn_Exp_s_struct
_tmp850;struct Cyc_Absyn_Exp_s_struct*_tmp84F;return(void*)((_tmp84F=_cycalloc(
sizeof(*_tmp84F)),((_tmp84F[0]=((_tmp850.tag=0,((_tmp850.f1=e,_tmp850)))),
_tmp84F))));}static void*Cyc_Toc_seq_stmt_r(struct Cyc_Absyn_Stmt*s1,struct Cyc_Absyn_Stmt*
s2);static void*Cyc_Toc_seq_stmt_r(struct Cyc_Absyn_Stmt*s1,struct Cyc_Absyn_Stmt*
s2){struct Cyc_Absyn_Seq_s_struct _tmp853;struct Cyc_Absyn_Seq_s_struct*_tmp852;
return(void*)((_tmp852=_cycalloc(sizeof(*_tmp852)),((_tmp852[0]=((_tmp853.tag=1,((
_tmp853.f1=s1,((_tmp853.f2=s2,_tmp853)))))),_tmp852))));}static void*Cyc_Toc_conditional_exp_r(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Absyn_Exp*e3);static void*
Cyc_Toc_conditional_exp_r(struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct
Cyc_Absyn_Exp*e3){struct Cyc_Absyn_Conditional_e_struct _tmp856;struct Cyc_Absyn_Conditional_e_struct*
_tmp855;return(void*)((_tmp855=_cycalloc(sizeof(*_tmp855)),((_tmp855[0]=((
_tmp856.tag=6,((_tmp856.f1=e1,((_tmp856.f2=e2,((_tmp856.f3=e3,_tmp856)))))))),
_tmp855))));}static void*Cyc_Toc_aggrmember_exp_r(struct Cyc_Absyn_Exp*e,struct
_dyneither_ptr*n);static void*Cyc_Toc_aggrmember_exp_r(struct Cyc_Absyn_Exp*e,
struct _dyneither_ptr*n){struct Cyc_Absyn_AggrMember_e_struct _tmp859;struct Cyc_Absyn_AggrMember_e_struct*
_tmp858;return(void*)((_tmp858=_cycalloc(sizeof(*_tmp858)),((_tmp858[0]=((
_tmp859.tag=23,((_tmp859.f1=e,((_tmp859.f2=n,_tmp859)))))),_tmp858))));}static
void*Cyc_Toc_aggrarrow_exp_r(struct Cyc_Absyn_Exp*e,struct _dyneither_ptr*n);
static void*Cyc_Toc_aggrarrow_exp_r(struct Cyc_Absyn_Exp*e,struct _dyneither_ptr*n){
struct Cyc_Absyn_AggrArrow_e_struct _tmp85C;struct Cyc_Absyn_AggrArrow_e_struct*
_tmp85B;return(void*)((_tmp85B=_cycalloc(sizeof(*_tmp85B)),((_tmp85B[0]=((
_tmp85C.tag=24,((_tmp85C.f1=e,((_tmp85C.f2=n,_tmp85C)))))),_tmp85B))));}static
void*Cyc_Toc_unresolvedmem_exp_r(struct Cyc_Core_Opt*tdopt,struct Cyc_List_List*ds);
static void*Cyc_Toc_unresolvedmem_exp_r(struct Cyc_Core_Opt*tdopt,struct Cyc_List_List*
ds){struct Cyc_Absyn_UnresolvedMem_e_struct _tmp85F;struct Cyc_Absyn_UnresolvedMem_e_struct*
_tmp85E;return(void*)((_tmp85E=_cycalloc(sizeof(*_tmp85E)),((_tmp85E[0]=((
_tmp85F.tag=37,((_tmp85F.f1=tdopt,((_tmp85F.f2=ds,_tmp85F)))))),_tmp85E))));}
static void*Cyc_Toc_goto_stmt_r(struct _dyneither_ptr*v,struct Cyc_Absyn_Stmt*s);
static void*Cyc_Toc_goto_stmt_r(struct _dyneither_ptr*v,struct Cyc_Absyn_Stmt*s){
struct Cyc_Absyn_Goto_s_struct _tmp862;struct Cyc_Absyn_Goto_s_struct*_tmp861;
return(void*)((_tmp861=_cycalloc(sizeof(*_tmp861)),((_tmp861[0]=((_tmp862.tag=7,((
_tmp862.f1=v,((_tmp862.f2=s,_tmp862)))))),_tmp861))));}static struct Cyc_Absyn_Const_e_struct
Cyc_Toc_zero_exp={0,(union Cyc_Absyn_Cnst_union)((struct Cyc_Absyn_Int_c_struct){2,(
void*)((void*)0),0})};struct Cyc_Toc_functionSet{struct Cyc_Absyn_Exp*fchar;struct
Cyc_Absyn_Exp*fshort;struct Cyc_Absyn_Exp*fint;struct Cyc_Absyn_Exp*ffloat;struct
Cyc_Absyn_Exp*fdouble;struct Cyc_Absyn_Exp*flongdouble;struct Cyc_Absyn_Exp*
fvoidstar;};struct Cyc_Toc_functionSet Cyc_Toc__zero_arr_plus_functionSet={& Cyc_Toc__zero_arr_plus_char_ev,&
Cyc_Toc__zero_arr_plus_short_ev,& Cyc_Toc__zero_arr_plus_int_ev,& Cyc_Toc__zero_arr_plus_float_ev,&
Cyc_Toc__zero_arr_plus_double_ev,& Cyc_Toc__zero_arr_plus_longdouble_ev,& Cyc_Toc__zero_arr_plus_voidstar_ev};
struct Cyc_Toc_functionSet Cyc_Toc__get_zero_arr_size_functionSet={& Cyc_Toc__get_zero_arr_size_char_ev,&
Cyc_Toc__get_zero_arr_size_short_ev,& Cyc_Toc__get_zero_arr_size_int_ev,& Cyc_Toc__get_zero_arr_size_float_ev,&
Cyc_Toc__get_zero_arr_size_double_ev,& Cyc_Toc__get_zero_arr_size_longdouble_ev,&
Cyc_Toc__get_zero_arr_size_voidstar_ev};struct Cyc_Toc_functionSet Cyc_Toc__zero_arr_inplace_plus_functionSet={&
Cyc_Toc__zero_arr_inplace_plus_char_ev,& Cyc_Toc__zero_arr_inplace_plus_short_ev,&
Cyc_Toc__zero_arr_inplace_plus_int_ev,& Cyc_Toc__zero_arr_inplace_plus_float_ev,&
Cyc_Toc__zero_arr_inplace_plus_double_ev,& Cyc_Toc__zero_arr_inplace_plus_longdouble_ev,&
Cyc_Toc__zero_arr_inplace_plus_voidstar_ev};struct Cyc_Toc_functionSet Cyc_Toc__zero_arr_inplace_plus_post_functionSet={&
Cyc_Toc__zero_arr_inplace_plus_post_char_ev,& Cyc_Toc__zero_arr_inplace_plus_post_short_ev,&
Cyc_Toc__zero_arr_inplace_plus_post_int_ev,& Cyc_Toc__zero_arr_inplace_plus_post_float_ev,&
Cyc_Toc__zero_arr_inplace_plus_post_double_ev,& Cyc_Toc__zero_arr_inplace_plus_post_longdouble_ev,&
Cyc_Toc__zero_arr_inplace_plus_post_voidstar_ev};static struct Cyc_Absyn_Exp*Cyc_Toc_getFunctionType(
struct Cyc_Toc_functionSet*fS,void*t);static struct Cyc_Absyn_Exp*Cyc_Toc_getFunctionType(
struct Cyc_Toc_functionSet*fS,void*t){struct Cyc_Absyn_Exp*function;{void*_tmp106=
t;void*_tmp107;int _tmp108;_LL1: if(_tmp106 <= (void*)4)goto _LL3;if(*((int*)_tmp106)
!= 5)goto _LL3;_tmp107=(void*)((struct Cyc_Absyn_IntType_struct*)_tmp106)->f2;_LL2:{
void*_tmp109=_tmp107;_LLC: if((int)_tmp109 != 0)goto _LLE;_LLD: function=fS->fchar;
goto _LLB;_LLE: if((int)_tmp109 != 1)goto _LL10;_LLF: function=fS->fshort;goto _LLB;
_LL10: if((int)_tmp109 != 2)goto _LL12;_LL11: function=fS->fint;goto _LLB;_LL12:;
_LL13: {struct Cyc_Core_Impossible_struct _tmp868;const char*_tmp867;struct Cyc_Core_Impossible_struct*
_tmp866;(int)_throw((void*)((_tmp866=_cycalloc(sizeof(*_tmp866)),((_tmp866[0]=((
_tmp868.tag=Cyc_Core_Impossible,((_tmp868.f1=((_tmp867="impossible IntType (not char, short or int)",
_tag_dyneither(_tmp867,sizeof(char),44))),_tmp868)))),_tmp866)))));}_LLB:;}goto
_LL0;_LL3: if((int)_tmp106 != 1)goto _LL5;_LL4: function=fS->ffloat;goto _LL0;_LL5:
if(_tmp106 <= (void*)4)goto _LL9;if(*((int*)_tmp106)!= 6)goto _LL7;_tmp108=((struct
Cyc_Absyn_DoubleType_struct*)_tmp106)->f1;_LL6: switch(_tmp108){case 1: _LL14:
function=fS->flongdouble;break;default: _LL15: function=fS->fdouble;}goto _LL0;_LL7:
if(*((int*)_tmp106)!= 4)goto _LL9;_LL8: function=fS->fvoidstar;goto _LL0;_LL9:;_LLA: {
struct Cyc_Core_Impossible_struct _tmp86E;const char*_tmp86D;struct Cyc_Core_Impossible_struct*
_tmp86C;(int)_throw((void*)((_tmp86C=_cycalloc(sizeof(*_tmp86C)),((_tmp86C[0]=((
_tmp86E.tag=Cyc_Core_Impossible,((_tmp86E.f1=((_tmp86D="impossible expression type (not int, float, double, or pointer)",
_tag_dyneither(_tmp86D,sizeof(char),64))),_tmp86E)))),_tmp86C)))));}_LL0:;}
return function;}struct Cyc_Absyn_Exp*Cyc_Toc_getFunction(struct Cyc_Toc_functionSet*
fS,struct Cyc_Absyn_Exp*arr);struct Cyc_Absyn_Exp*Cyc_Toc_getFunction(struct Cyc_Toc_functionSet*
fS,struct Cyc_Absyn_Exp*arr){return Cyc_Toc_getFunctionType(fS,Cyc_Tcutil_compress((
void*)((struct Cyc_Core_Opt*)_check_null(arr->topt))->v));}struct Cyc_Absyn_Exp*
Cyc_Toc_getFunctionRemovePointer(struct Cyc_Toc_functionSet*fS,struct Cyc_Absyn_Exp*
arr);struct Cyc_Absyn_Exp*Cyc_Toc_getFunctionRemovePointer(struct Cyc_Toc_functionSet*
fS,struct Cyc_Absyn_Exp*arr){void*_tmp110=Cyc_Tcutil_compress((void*)((struct Cyc_Core_Opt*)
_check_null(arr->topt))->v);struct Cyc_Absyn_PtrInfo _tmp111;void*_tmp112;_LL18:
if(_tmp110 <= (void*)4)goto _LL1A;if(*((int*)_tmp110)!= 4)goto _LL1A;_tmp111=((
struct Cyc_Absyn_PointerType_struct*)_tmp110)->f1;_tmp112=(void*)_tmp111.elt_typ;
_LL19: return Cyc_Toc_getFunctionType(fS,_tmp112);_LL1A:;_LL1B: {struct Cyc_Core_Impossible_struct
_tmp874;const char*_tmp873;struct Cyc_Core_Impossible_struct*_tmp872;(int)_throw((
void*)((_tmp872=_cycalloc(sizeof(*_tmp872)),((_tmp872[0]=((_tmp874.tag=Cyc_Core_Impossible,((
_tmp874.f1=((_tmp873="impossible type (not pointer)",_tag_dyneither(_tmp873,
sizeof(char),30))),_tmp874)))),_tmp872)))));}_LL17:;}struct _tuple8{struct Cyc_List_List*
f1;struct Cyc_Absyn_Exp*f2;};static int Cyc_Toc_is_zero(struct Cyc_Absyn_Exp*e);
static int Cyc_Toc_is_zero(struct Cyc_Absyn_Exp*e){void*_tmp116=(void*)e->r;union
Cyc_Absyn_Cnst_union _tmp117;char _tmp118;union Cyc_Absyn_Cnst_union _tmp119;short
_tmp11A;union Cyc_Absyn_Cnst_union _tmp11B;int _tmp11C;union Cyc_Absyn_Cnst_union
_tmp11D;long long _tmp11E;union Cyc_Absyn_Cnst_union _tmp11F;struct Cyc_Absyn_Exp*
_tmp120;struct Cyc_List_List*_tmp121;struct Cyc_List_List*_tmp122;struct Cyc_List_List*
_tmp123;struct Cyc_List_List*_tmp124;struct Cyc_List_List*_tmp125;_LL1D: if(*((int*)
_tmp116)!= 0)goto _LL1F;_tmp117=((struct Cyc_Absyn_Const_e_struct*)_tmp116)->f1;
if(((((struct Cyc_Absyn_Const_e_struct*)_tmp116)->f1).Char_c).tag != 0)goto _LL1F;
_tmp118=(_tmp117.Char_c).f2;_LL1E: return _tmp118 == '\000';_LL1F: if(*((int*)
_tmp116)!= 0)goto _LL21;_tmp119=((struct Cyc_Absyn_Const_e_struct*)_tmp116)->f1;
if(((((struct Cyc_Absyn_Const_e_struct*)_tmp116)->f1).Short_c).tag != 1)goto _LL21;
_tmp11A=(_tmp119.Short_c).f2;_LL20: return _tmp11A == 0;_LL21: if(*((int*)_tmp116)!= 
0)goto _LL23;_tmp11B=((struct Cyc_Absyn_Const_e_struct*)_tmp116)->f1;if(((((struct
Cyc_Absyn_Const_e_struct*)_tmp116)->f1).Int_c).tag != 2)goto _LL23;_tmp11C=(
_tmp11B.Int_c).f2;_LL22: return _tmp11C == 0;_LL23: if(*((int*)_tmp116)!= 0)goto
_LL25;_tmp11D=((struct Cyc_Absyn_Const_e_struct*)_tmp116)->f1;if(((((struct Cyc_Absyn_Const_e_struct*)
_tmp116)->f1).LongLong_c).tag != 3)goto _LL25;_tmp11E=(_tmp11D.LongLong_c).f2;
_LL24: return _tmp11E == 0;_LL25: if(*((int*)_tmp116)!= 0)goto _LL27;_tmp11F=((struct
Cyc_Absyn_Const_e_struct*)_tmp116)->f1;if(((((struct Cyc_Absyn_Const_e_struct*)
_tmp116)->f1).Null_c).tag != 6)goto _LL27;_LL26: return 1;_LL27: if(*((int*)_tmp116)
!= 15)goto _LL29;_tmp120=((struct Cyc_Absyn_Cast_e_struct*)_tmp116)->f2;_LL28:
return Cyc_Toc_is_zero(_tmp120);_LL29: if(*((int*)_tmp116)!= 26)goto _LL2B;_tmp121=((
struct Cyc_Absyn_Tuple_e_struct*)_tmp116)->f1;_LL2A: return((int(*)(int(*pred)(
struct Cyc_Absyn_Exp*),struct Cyc_List_List*x))Cyc_List_forall)(Cyc_Toc_is_zero,
_tmp121);_LL2B: if(*((int*)_tmp116)!= 28)goto _LL2D;_tmp122=((struct Cyc_Absyn_Array_e_struct*)
_tmp116)->f1;_LL2C: _tmp123=_tmp122;goto _LL2E;_LL2D: if(*((int*)_tmp116)!= 30)goto
_LL2F;_tmp123=((struct Cyc_Absyn_Struct_e_struct*)_tmp116)->f3;_LL2E: _tmp124=
_tmp123;goto _LL30;_LL2F: if(*((int*)_tmp116)!= 27)goto _LL31;_tmp124=((struct Cyc_Absyn_CompoundLit_e_struct*)
_tmp116)->f2;_LL30: _tmp125=_tmp124;goto _LL32;_LL31: if(*((int*)_tmp116)!= 37)goto
_LL33;_tmp125=((struct Cyc_Absyn_UnresolvedMem_e_struct*)_tmp116)->f2;_LL32: for(0;
_tmp125 != 0;_tmp125=_tmp125->tl){if(!Cyc_Toc_is_zero((*((struct _tuple8*)_tmp125->hd)).f2))
return 0;}return 1;_LL33:;_LL34: return 0;_LL1C:;}static int Cyc_Toc_is_nullable(void*
t);static int Cyc_Toc_is_nullable(void*t){void*_tmp126=Cyc_Tcutil_compress(t);
struct Cyc_Absyn_PtrInfo _tmp127;struct Cyc_Absyn_PtrAtts _tmp128;struct Cyc_Absyn_Conref*
_tmp129;_LL36: if(_tmp126 <= (void*)4)goto _LL38;if(*((int*)_tmp126)!= 4)goto _LL38;
_tmp127=((struct Cyc_Absyn_PointerType_struct*)_tmp126)->f1;_tmp128=_tmp127.ptr_atts;
_tmp129=_tmp128.nullable;_LL37: return((int(*)(int,struct Cyc_Absyn_Conref*x))Cyc_Absyn_conref_def)(
0,_tmp129);_LL38:;_LL39: {const char*_tmp877;void*_tmp876;(_tmp876=0,((int(*)(
struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmp877="is_nullable",
_tag_dyneither(_tmp877,sizeof(char),12))),_tag_dyneither(_tmp876,sizeof(void*),0)));}
_LL35:;}static char _tmp138[1]="";static char _tmp13A[8]="*bogus*";static struct
_tuple1*Cyc_Toc_collapse_qvar_tag(struct _tuple1*x,struct _dyneither_ptr tag);
static struct _tuple1*Cyc_Toc_collapse_qvar_tag(struct _tuple1*x,struct
_dyneither_ptr tag){struct _DynRegionHandle*_tmp12D;struct Cyc_Dict_Dict*_tmp12E;
struct Cyc_Toc_TocState _tmp12C=*((struct Cyc_Toc_TocState*)_check_null(Cyc_Toc_toc_state));
_tmp12D=_tmp12C.dyn;_tmp12E=_tmp12C.qvar_tags;{static struct _dyneither_ptr
bogus_string={_tmp13A,_tmp13A,_tmp13A + 8};static struct _tuple1 bogus_qvar={(union
Cyc_Absyn_Nmspace_union)((struct Cyc_Absyn_Loc_n_struct){0}),& bogus_string};
static struct _tuple6 pair={& bogus_qvar,{_tmp138,_tmp138,_tmp138 + 1}};{struct
_tuple6 _tmp878;pair=((_tmp878.f1=x,((_tmp878.f2=tag,_tmp878))));}{struct
_DynRegionFrame _tmp130;struct _RegionHandle*d=_open_dynregion(& _tmp130,_tmp12D);{
struct _tuple1**_tmp131=((struct _tuple1**(*)(struct Cyc_Dict_Dict d,struct _tuple6*k))
Cyc_Dict_lookup_opt)(*_tmp12E,(struct _tuple6*)& pair);if(_tmp131 != 0){struct
_tuple1*_tmp132=*_tmp131;_npop_handler(0);return _tmp132;}{struct _tuple6*_tmp879;
struct _tuple6*_tmp133=(_tmp879=_cycalloc(sizeof(*_tmp879)),((_tmp879->f1=x,((
_tmp879->f2=tag,_tmp879)))));struct _dyneither_ptr*_tmp87C;struct _tuple1*_tmp87B;
struct _tuple1*res=(_tmp87B=_cycalloc(sizeof(*_tmp87B)),((_tmp87B->f1=(*x).f1,((
_tmp87B->f2=((_tmp87C=_cycalloc(sizeof(*_tmp87C)),((_tmp87C[0]=(struct
_dyneither_ptr)Cyc_strconcat((struct _dyneither_ptr)*(*x).f2,(struct
_dyneither_ptr)tag),_tmp87C)))),_tmp87B)))));*_tmp12E=((struct Cyc_Dict_Dict(*)(
struct Cyc_Dict_Dict d,struct _tuple6*k,struct _tuple1*v))Cyc_Dict_insert)(*_tmp12E,(
struct _tuple6*)_tmp133,res);{struct _tuple1*_tmp134=res;_npop_handler(0);return
_tmp134;}}};_pop_dynregion(d);}}}struct _tuple9{void*f1;struct Cyc_List_List*f2;};
static void*Cyc_Toc_add_tuple_type(struct Cyc_List_List*tqs0);static void*Cyc_Toc_add_tuple_type(
struct Cyc_List_List*tqs0){struct _DynRegionHandle*_tmp13C;struct Cyc_List_List**
_tmp13D;struct Cyc_Toc_TocState _tmp13B=*((struct Cyc_Toc_TocState*)_check_null(Cyc_Toc_toc_state));
_tmp13C=_tmp13B.dyn;_tmp13D=_tmp13B.tuple_types;{struct _DynRegionFrame _tmp13E;
struct _RegionHandle*d=_open_dynregion(& _tmp13E,_tmp13C);{struct Cyc_List_List*
_tmp13F=*_tmp13D;for(0;_tmp13F != 0;_tmp13F=_tmp13F->tl){struct _tuple9 _tmp141;
void*_tmp142;struct Cyc_List_List*_tmp143;struct _tuple9*_tmp140=(struct _tuple9*)
_tmp13F->hd;_tmp141=*_tmp140;_tmp142=_tmp141.f1;_tmp143=_tmp141.f2;{struct Cyc_List_List*
_tmp144=tqs0;for(0;_tmp144 != 0  && _tmp143 != 0;(_tmp144=_tmp144->tl,_tmp143=
_tmp143->tl)){if(!Cyc_Tcutil_unify((*((struct _tuple4*)_tmp144->hd)).f2,(void*)
_tmp143->hd))break;}if(_tmp144 == 0  && _tmp143 == 0){void*_tmp145=_tmp142;
_npop_handler(0);return _tmp145;}}}}{struct Cyc_Int_pa_struct _tmp884;void*_tmp883[
1];const char*_tmp882;struct _dyneither_ptr*_tmp881;struct _dyneither_ptr*xname=(
_tmp881=_cycalloc(sizeof(*_tmp881)),((_tmp881[0]=(struct _dyneither_ptr)((_tmp884.tag=
1,((_tmp884.f1=(unsigned long)Cyc_Toc_tuple_type_counter ++,((_tmp883[0]=& _tmp884,
Cyc_aprintf(((_tmp882="_tuple%d",_tag_dyneither(_tmp882,sizeof(char),9))),
_tag_dyneither(_tmp883,sizeof(void*),1)))))))),_tmp881)));void*x=Cyc_Absyn_strct(
xname);struct Cyc_List_List*ts=((struct Cyc_List_List*(*)(struct _RegionHandle*,
void*(*f)(struct _tuple4*),struct Cyc_List_List*x))Cyc_List_rmap)(d,Cyc_Tcutil_snd_tqt,
tqs0);struct Cyc_List_List*_tmp146=0;struct Cyc_List_List*ts2=ts;{int i=1;for(0;ts2
!= 0;(ts2=ts2->tl,i ++)){struct Cyc_Absyn_Aggrfield*_tmp887;struct Cyc_List_List*
_tmp886;_tmp146=((_tmp886=_cycalloc(sizeof(*_tmp886)),((_tmp886->hd=((_tmp887=
_cycalloc(sizeof(*_tmp887)),((_tmp887->name=Cyc_Absyn_fieldname(i),((_tmp887->tq=
Cyc_Toc_mt_tq,((_tmp887->type=(void*)((void*)ts2->hd),((_tmp887->width=0,((
_tmp887->attributes=0,_tmp887)))))))))))),((_tmp886->tl=_tmp146,_tmp886))))));}}
_tmp146=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(
_tmp146);{struct _tuple1*_tmp88F;union Cyc_Absyn_Nmspace_union _tmp88E;struct Cyc_Absyn_AggrdeclImpl*
_tmp88D;struct Cyc_Absyn_Aggrdecl*_tmp88C;struct Cyc_Absyn_Aggrdecl*_tmp149=(
_tmp88C=_cycalloc(sizeof(*_tmp88C)),((_tmp88C->kind=(void*)((void*)0),((_tmp88C->sc=(
void*)((void*)2),((_tmp88C->name=((_tmp88F=_cycalloc(sizeof(*_tmp88F)),((_tmp88F->f1=(
union Cyc_Absyn_Nmspace_union)(((_tmp88E.Rel_n).tag=1,(((_tmp88E.Rel_n).f1=0,
_tmp88E)))),((_tmp88F->f2=xname,_tmp88F)))))),((_tmp88C->tvs=0,((_tmp88C->impl=((
_tmp88D=_cycalloc(sizeof(*_tmp88D)),((_tmp88D->exist_vars=0,((_tmp88D->rgn_po=0,((
_tmp88D->fields=_tmp146,_tmp88D)))))))),((_tmp88C->attributes=0,_tmp88C)))))))))))));{
struct Cyc_Absyn_Aggr_d_struct*_tmp895;struct Cyc_Absyn_Aggr_d_struct _tmp894;
struct Cyc_List_List*_tmp893;Cyc_Toc_result_decls=((_tmp893=_cycalloc(sizeof(*
_tmp893)),((_tmp893->hd=Cyc_Absyn_new_decl((void*)((_tmp895=_cycalloc(sizeof(*
_tmp895)),((_tmp895[0]=((_tmp894.tag=4,((_tmp894.f1=_tmp149,_tmp894)))),_tmp895)))),
0),((_tmp893->tl=Cyc_Toc_result_decls,_tmp893))))));}{struct _tuple9*_tmp898;
struct Cyc_List_List*_tmp897;*_tmp13D=((_tmp897=_region_malloc(d,sizeof(*_tmp897)),((
_tmp897->hd=((_tmp898=_region_malloc(d,sizeof(*_tmp898)),((_tmp898->f1=x,((
_tmp898->f2=ts,_tmp898)))))),((_tmp897->tl=*_tmp13D,_tmp897))))));}{void*_tmp14F=
x;_npop_handler(0);return _tmp14F;}}};_pop_dynregion(d);}}struct _tuple1*Cyc_Toc_temp_var();
struct _tuple1*Cyc_Toc_temp_var(){int _tmp158=Cyc_Toc_temp_var_counter ++;union Cyc_Absyn_Nmspace_union
_tmp8A7;struct _dyneither_ptr*_tmp8A6;const char*_tmp8A5;void*_tmp8A4[1];struct Cyc_Int_pa_struct
_tmp8A3;struct _tuple1*_tmp8A2;struct _tuple1*res=(_tmp8A2=_cycalloc(sizeof(*
_tmp8A2)),((_tmp8A2->f1=(union Cyc_Absyn_Nmspace_union)(((_tmp8A7.Loc_n).tag=0,
_tmp8A7)),((_tmp8A2->f2=((_tmp8A6=_cycalloc(sizeof(*_tmp8A6)),((_tmp8A6[0]=(
struct _dyneither_ptr)((_tmp8A3.tag=1,((_tmp8A3.f1=(unsigned int)_tmp158,((
_tmp8A4[0]=& _tmp8A3,Cyc_aprintf(((_tmp8A5="_tmp%X",_tag_dyneither(_tmp8A5,
sizeof(char),7))),_tag_dyneither(_tmp8A4,sizeof(void*),1)))))))),_tmp8A6)))),
_tmp8A2)))));return res;}static struct _dyneither_ptr*Cyc_Toc_fresh_label();static
struct _dyneither_ptr*Cyc_Toc_fresh_label(){struct _DynRegionHandle*_tmp160;struct
Cyc_Xarray_Xarray*_tmp161;struct Cyc_Toc_TocState _tmp15F=*((struct Cyc_Toc_TocState*)
_check_null(Cyc_Toc_toc_state));_tmp160=_tmp15F.dyn;_tmp161=_tmp15F.temp_labels;{
struct _DynRegionFrame _tmp162;struct _RegionHandle*d=_open_dynregion(& _tmp162,
_tmp160);{int _tmp163=Cyc_Toc_fresh_label_counter ++;if(_tmp163 < ((int(*)(struct
Cyc_Xarray_Xarray*))Cyc_Xarray_length)(_tmp161)){struct _dyneither_ptr*_tmp164=((
struct _dyneither_ptr*(*)(struct Cyc_Xarray_Xarray*,int))Cyc_Xarray_get)(_tmp161,
_tmp163);_npop_handler(0);return _tmp164;}{struct Cyc_Int_pa_struct _tmp8AF;void*
_tmp8AE[1];const char*_tmp8AD;struct _dyneither_ptr*_tmp8AC;struct _dyneither_ptr*
res=(_tmp8AC=_cycalloc(sizeof(*_tmp8AC)),((_tmp8AC[0]=(struct _dyneither_ptr)((
_tmp8AF.tag=1,((_tmp8AF.f1=(unsigned int)_tmp163,((_tmp8AE[0]=& _tmp8AF,Cyc_aprintf(((
_tmp8AD="_LL%X",_tag_dyneither(_tmp8AD,sizeof(char),6))),_tag_dyneither(_tmp8AE,
sizeof(void*),1)))))))),_tmp8AC)));if(((int(*)(struct Cyc_Xarray_Xarray*,struct
_dyneither_ptr*))Cyc_Xarray_add_ind)(_tmp161,res)!= _tmp163){const char*_tmp8B2;
void*_tmp8B1;(_tmp8B1=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))
Cyc_Toc_toc_impos)(((_tmp8B2="fresh_label: add_ind returned bad index...",
_tag_dyneither(_tmp8B2,sizeof(char),43))),_tag_dyneither(_tmp8B1,sizeof(void*),0)));}{
struct _dyneither_ptr*_tmp167=res;_npop_handler(0);return _tmp167;}}};
_pop_dynregion(d);}}static struct Cyc_Absyn_Exp*Cyc_Toc_tunion_tag(struct Cyc_Absyn_Tuniondecl*
td,struct _tuple1*name,int carries_value);static struct Cyc_Absyn_Exp*Cyc_Toc_tunion_tag(
struct Cyc_Absyn_Tuniondecl*td,struct _tuple1*name,int carries_value){int ans=0;
struct Cyc_List_List*_tmp16C=(struct Cyc_List_List*)((struct Cyc_Core_Opt*)
_check_null(td->fields))->v;while(Cyc_Absyn_qvar_cmp(name,((struct Cyc_Absyn_Tunionfield*)((
struct Cyc_List_List*)_check_null(_tmp16C))->hd)->name)!= 0){if((td->is_flat  || 
carries_value  && ((struct Cyc_Absyn_Tunionfield*)_tmp16C->hd)->typs != 0) || !
carries_value  && ((struct Cyc_Absyn_Tunionfield*)_tmp16C->hd)->typs == 0)++ ans;
_tmp16C=_tmp16C->tl;}return Cyc_Absyn_uint_exp((unsigned int)ans,0);}static int Cyc_Toc_num_void_tags(
struct Cyc_Absyn_Tuniondecl*td);static int Cyc_Toc_num_void_tags(struct Cyc_Absyn_Tuniondecl*
td){int ans=0;{struct Cyc_List_List*_tmp16D=(struct Cyc_List_List*)((struct Cyc_Core_Opt*)
_check_null(td->fields))->v;for(0;_tmp16D != 0;_tmp16D=_tmp16D->tl){if(((struct
Cyc_Absyn_Tunionfield*)_tmp16D->hd)->typs == 0)++ ans;}}return ans;}static void*Cyc_Toc_typ_to_c(
void*t);static struct _tuple2*Cyc_Toc_arg_to_c(struct _tuple2*a);static struct
_tuple2*Cyc_Toc_arg_to_c(struct _tuple2*a){struct Cyc_Core_Opt*_tmp16F;struct Cyc_Absyn_Tqual
_tmp170;void*_tmp171;struct _tuple2 _tmp16E=*a;_tmp16F=_tmp16E.f1;_tmp170=_tmp16E.f2;
_tmp171=_tmp16E.f3;{struct _tuple2*_tmp8B3;return(_tmp8B3=_cycalloc(sizeof(*
_tmp8B3)),((_tmp8B3->f1=_tmp16F,((_tmp8B3->f2=_tmp170,((_tmp8B3->f3=Cyc_Toc_typ_to_c(
_tmp171),_tmp8B3)))))));}}static struct _tuple4*Cyc_Toc_typ_to_c_f(struct _tuple4*x);
static struct _tuple4*Cyc_Toc_typ_to_c_f(struct _tuple4*x){struct Cyc_Absyn_Tqual
_tmp174;void*_tmp175;struct _tuple4 _tmp173=*x;_tmp174=_tmp173.f1;_tmp175=_tmp173.f2;{
struct _tuple4*_tmp8B4;return(_tmp8B4=_cycalloc(sizeof(*_tmp8B4)),((_tmp8B4->f1=
_tmp174,((_tmp8B4->f2=Cyc_Toc_typ_to_c(_tmp175),_tmp8B4)))));}}static void*Cyc_Toc_typ_to_c_array(
void*t);static void*Cyc_Toc_typ_to_c_array(void*t){void*_tmp177=Cyc_Tcutil_compress(
t);struct Cyc_Absyn_ArrayInfo _tmp178;void*_tmp179;struct Cyc_Absyn_Tqual _tmp17A;
struct Cyc_Absyn_Exp*_tmp17B;struct Cyc_Absyn_Conref*_tmp17C;struct Cyc_Position_Segment*
_tmp17D;struct Cyc_Core_Opt*_tmp17E;struct Cyc_Core_Opt _tmp17F;void*_tmp180;_LL3B:
if(_tmp177 <= (void*)4)goto _LL3F;if(*((int*)_tmp177)!= 7)goto _LL3D;_tmp178=((
struct Cyc_Absyn_ArrayType_struct*)_tmp177)->f1;_tmp179=(void*)_tmp178.elt_type;
_tmp17A=_tmp178.tq;_tmp17B=_tmp178.num_elts;_tmp17C=_tmp178.zero_term;_tmp17D=
_tmp178.zt_loc;_LL3C: return Cyc_Absyn_array_typ(Cyc_Toc_typ_to_c_array(_tmp179),
_tmp17A,_tmp17B,Cyc_Absyn_false_conref,_tmp17D);_LL3D: if(*((int*)_tmp177)!= 0)
goto _LL3F;_tmp17E=((struct Cyc_Absyn_Evar_struct*)_tmp177)->f2;if(_tmp17E == 0)
goto _LL3F;_tmp17F=*_tmp17E;_tmp180=(void*)_tmp17F.v;_LL3E: return Cyc_Toc_typ_to_c_array(
_tmp180);_LL3F:;_LL40: return Cyc_Toc_typ_to_c(t);_LL3A:;}static struct Cyc_Absyn_Aggrfield*
Cyc_Toc_aggrfield_to_c(struct Cyc_Absyn_Aggrfield*f);static struct Cyc_Absyn_Aggrfield*
Cyc_Toc_aggrfield_to_c(struct Cyc_Absyn_Aggrfield*f){struct Cyc_Absyn_Aggrfield*
_tmp8B5;return(_tmp8B5=_cycalloc(sizeof(*_tmp8B5)),((_tmp8B5->name=f->name,((
_tmp8B5->tq=Cyc_Toc_mt_tq,((_tmp8B5->type=(void*)Cyc_Toc_typ_to_c((void*)f->type),((
_tmp8B5->width=f->width,((_tmp8B5->attributes=f->attributes,_tmp8B5)))))))))));}
static void Cyc_Toc_enumfields_to_c(struct Cyc_List_List*fs);static void Cyc_Toc_enumfields_to_c(
struct Cyc_List_List*fs){return;}static void*Cyc_Toc_char_star_typ();static void*
Cyc_Toc_char_star_typ(){static void**cs=0;if(cs == 0){void**_tmp8B6;cs=((_tmp8B6=
_cycalloc(sizeof(*_tmp8B6)),((_tmp8B6[0]=Cyc_Absyn_star_typ(Cyc_Absyn_char_typ,(
void*)2,Cyc_Toc_mt_tq,Cyc_Absyn_false_conref),_tmp8B6))));}return*cs;}static void*
Cyc_Toc_rgn_typ();static void*Cyc_Toc_rgn_typ(){static void**r=0;if(r == 0){void**
_tmp8B7;r=((_tmp8B7=_cycalloc(sizeof(*_tmp8B7)),((_tmp8B7[0]=Cyc_Absyn_cstar_typ(
Cyc_Absyn_strct(Cyc_Toc__RegionHandle_sp),Cyc_Toc_mt_tq),_tmp8B7))));}return*r;}
static void*Cyc_Toc_dyn_rgn_typ();static void*Cyc_Toc_dyn_rgn_typ(){static void**r=
0;if(r == 0){void**_tmp8B8;r=((_tmp8B8=_cycalloc(sizeof(*_tmp8B8)),((_tmp8B8[0]=
Cyc_Absyn_cstar_typ(Cyc_Absyn_strct(Cyc_Toc__DynRegionHandle_sp),Cyc_Toc_mt_tq),
_tmp8B8))));}return*r;}static void*Cyc_Toc_typ_to_c(void*t);static void*Cyc_Toc_typ_to_c(
void*t){void*_tmp185=t;struct Cyc_Core_Opt*_tmp186;struct Cyc_Core_Opt*_tmp187;
struct Cyc_Core_Opt _tmp188;void*_tmp189;struct Cyc_Absyn_Tvar*_tmp18A;struct Cyc_Absyn_TunionInfo
_tmp18B;union Cyc_Absyn_TunionInfoU_union _tmp18C;struct Cyc_Absyn_Tuniondecl**
_tmp18D;struct Cyc_Absyn_Tuniondecl*_tmp18E;struct Cyc_Absyn_TunionFieldInfo
_tmp18F;union Cyc_Absyn_TunionFieldInfoU_union _tmp190;struct Cyc_Absyn_Tuniondecl*
_tmp191;struct Cyc_Absyn_Tunionfield*_tmp192;struct Cyc_Absyn_PtrInfo _tmp193;void*
_tmp194;struct Cyc_Absyn_Tqual _tmp195;struct Cyc_Absyn_PtrAtts _tmp196;struct Cyc_Absyn_Conref*
_tmp197;struct Cyc_Absyn_ArrayInfo _tmp198;void*_tmp199;struct Cyc_Absyn_Tqual
_tmp19A;struct Cyc_Absyn_Exp*_tmp19B;struct Cyc_Position_Segment*_tmp19C;struct Cyc_Absyn_FnInfo
_tmp19D;void*_tmp19E;struct Cyc_List_List*_tmp19F;int _tmp1A0;struct Cyc_Absyn_VarargInfo*
_tmp1A1;struct Cyc_List_List*_tmp1A2;struct Cyc_List_List*_tmp1A3;void*_tmp1A4;
struct Cyc_List_List*_tmp1A5;struct Cyc_Absyn_AggrInfo _tmp1A6;union Cyc_Absyn_AggrInfoU_union
_tmp1A7;struct Cyc_List_List*_tmp1A8;struct _tuple1*_tmp1A9;struct Cyc_List_List*
_tmp1AA;struct _tuple1*_tmp1AB;struct Cyc_List_List*_tmp1AC;struct Cyc_Absyn_Typedefdecl*
_tmp1AD;void**_tmp1AE;void*_tmp1AF;_LL42: if((int)_tmp185 != 0)goto _LL44;_LL43:
return t;_LL44: if(_tmp185 <= (void*)4)goto _LL56;if(*((int*)_tmp185)!= 0)goto _LL46;
_tmp186=((struct Cyc_Absyn_Evar_struct*)_tmp185)->f2;if(_tmp186 != 0)goto _LL46;
_LL45: return Cyc_Absyn_sint_typ;_LL46: if(*((int*)_tmp185)!= 0)goto _LL48;_tmp187=((
struct Cyc_Absyn_Evar_struct*)_tmp185)->f2;if(_tmp187 == 0)goto _LL48;_tmp188=*
_tmp187;_tmp189=(void*)_tmp188.v;_LL47: return Cyc_Toc_typ_to_c(_tmp189);_LL48: if(*((
int*)_tmp185)!= 1)goto _LL4A;_tmp18A=((struct Cyc_Absyn_VarType_struct*)_tmp185)->f1;
_LL49: if(Cyc_Tcutil_tvar_kind(_tmp18A)== (void*)0)return(void*)0;else{return Cyc_Absyn_void_star_typ();}
_LL4A: if(*((int*)_tmp185)!= 2)goto _LL4C;_tmp18B=((struct Cyc_Absyn_TunionType_struct*)
_tmp185)->f1;_tmp18C=_tmp18B.tunion_info;if((((((struct Cyc_Absyn_TunionType_struct*)
_tmp185)->f1).tunion_info).KnownTunion).tag != 1)goto _LL4C;_tmp18D=(_tmp18C.KnownTunion).f1;
_tmp18E=*_tmp18D;_LL4B: if(_tmp18E->is_flat){const char*_tmp8B9;return Cyc_Absyn_unionq_typ(
Cyc_Toc_collapse_qvar_tag(_tmp18E->name,((_tmp8B9="_union",_tag_dyneither(
_tmp8B9,sizeof(char),7)))));}else{return Cyc_Absyn_void_star_typ();}_LL4C: if(*((
int*)_tmp185)!= 2)goto _LL4E;_LL4D: {const char*_tmp8BC;void*_tmp8BB;(_tmp8BB=0,((
int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((
_tmp8BC="unresolved TunionType",_tag_dyneither(_tmp8BC,sizeof(char),22))),
_tag_dyneither(_tmp8BB,sizeof(void*),0)));}_LL4E: if(*((int*)_tmp185)!= 3)goto
_LL50;_tmp18F=((struct Cyc_Absyn_TunionFieldType_struct*)_tmp185)->f1;_tmp190=
_tmp18F.field_info;if((((((struct Cyc_Absyn_TunionFieldType_struct*)_tmp185)->f1).field_info).KnownTunionfield).tag
!= 1)goto _LL50;_tmp191=(_tmp190.KnownTunionfield).f1;_tmp192=(_tmp190.KnownTunionfield).f2;
_LL4F: if(_tmp191->is_flat){const char*_tmp8BD;return Cyc_Absyn_unionq_typ(Cyc_Toc_collapse_qvar_tag(
_tmp191->name,((_tmp8BD="_union",_tag_dyneither(_tmp8BD,sizeof(char),7)))));}if(
_tmp192->typs == 0){if(_tmp191->is_xtunion)return Cyc_Toc_char_star_typ();else{
return Cyc_Absyn_uint_typ;}}else{const char*_tmp8BE;return Cyc_Absyn_strctq(Cyc_Toc_collapse_qvar_tag(
_tmp192->name,((_tmp8BE="_struct",_tag_dyneither(_tmp8BE,sizeof(char),8)))));}
_LL50: if(*((int*)_tmp185)!= 3)goto _LL52;_LL51: {const char*_tmp8C1;void*_tmp8C0;(
_tmp8C0=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((
_tmp8C1="unresolved TunionFieldType",_tag_dyneither(_tmp8C1,sizeof(char),27))),
_tag_dyneither(_tmp8C0,sizeof(void*),0)));}_LL52: if(*((int*)_tmp185)!= 4)goto
_LL54;_tmp193=((struct Cyc_Absyn_PointerType_struct*)_tmp185)->f1;_tmp194=(void*)
_tmp193.elt_typ;_tmp195=_tmp193.elt_tq;_tmp196=_tmp193.ptr_atts;_tmp197=_tmp196.bounds;
_LL53: _tmp194=Cyc_Toc_typ_to_c_array(_tmp194);{union Cyc_Absyn_Constraint_union
_tmp1B7=(Cyc_Absyn_compress_conref(_tmp197))->v;void*_tmp1B8;_LL7D: if((_tmp1B7.No_constr).tag
!= 2)goto _LL7F;_LL7E: goto _LL80;_LL7F: if((_tmp1B7.Eq_constr).tag != 0)goto _LL81;
_tmp1B8=(_tmp1B7.Eq_constr).f1;if((int)_tmp1B8 != 0)goto _LL81;_LL80: return Cyc_Toc_dyneither_ptr_typ;
_LL81:;_LL82: return Cyc_Absyn_star_typ(_tmp194,(void*)2,_tmp195,Cyc_Absyn_false_conref);
_LL7C:;}_LL54: if(*((int*)_tmp185)!= 5)goto _LL56;_LL55: goto _LL57;_LL56: if((int)
_tmp185 != 1)goto _LL58;_LL57: goto _LL59;_LL58: if(_tmp185 <= (void*)4)goto _LL70;if(*((
int*)_tmp185)!= 6)goto _LL5A;_LL59: return t;_LL5A: if(*((int*)_tmp185)!= 7)goto
_LL5C;_tmp198=((struct Cyc_Absyn_ArrayType_struct*)_tmp185)->f1;_tmp199=(void*)
_tmp198.elt_type;_tmp19A=_tmp198.tq;_tmp19B=_tmp198.num_elts;_tmp19C=_tmp198.zt_loc;
_LL5B: return Cyc_Absyn_array_typ(Cyc_Toc_typ_to_c_array(_tmp199),_tmp19A,_tmp19B,
Cyc_Absyn_false_conref,_tmp19C);_LL5C: if(*((int*)_tmp185)!= 8)goto _LL5E;_tmp19D=((
struct Cyc_Absyn_FnType_struct*)_tmp185)->f1;_tmp19E=(void*)_tmp19D.ret_typ;
_tmp19F=_tmp19D.args;_tmp1A0=_tmp19D.c_varargs;_tmp1A1=_tmp19D.cyc_varargs;
_tmp1A2=_tmp19D.attributes;_LL5D: {struct Cyc_List_List*_tmp1B9=0;for(0;_tmp1A2 != 
0;_tmp1A2=_tmp1A2->tl){void*_tmp1BA=(void*)_tmp1A2->hd;_LL84: if((int)_tmp1BA != 3)
goto _LL86;_LL85: goto _LL87;_LL86: if((int)_tmp1BA != 4)goto _LL88;_LL87: goto _LL89;
_LL88: if(_tmp1BA <= (void*)17)goto _LL8C;if(*((int*)_tmp1BA)!= 3)goto _LL8A;_LL89:
continue;_LL8A: if(*((int*)_tmp1BA)!= 4)goto _LL8C;_LL8B: continue;_LL8C:;_LL8D:{
struct Cyc_List_List*_tmp8C2;_tmp1B9=((_tmp8C2=_cycalloc(sizeof(*_tmp8C2)),((
_tmp8C2->hd=(void*)((void*)_tmp1A2->hd),((_tmp8C2->tl=_tmp1B9,_tmp8C2))))));}
goto _LL83;_LL83:;}{struct Cyc_List_List*_tmp1BC=((struct Cyc_List_List*(*)(struct
_tuple2*(*f)(struct _tuple2*),struct Cyc_List_List*x))Cyc_List_map)(Cyc_Toc_arg_to_c,
_tmp19F);if(_tmp1A1 != 0){void*_tmp1BD=Cyc_Toc_typ_to_c(Cyc_Absyn_dyneither_typ((
void*)_tmp1A1->type,(void*)2,Cyc_Toc_mt_tq,Cyc_Absyn_false_conref));struct
_tuple2*_tmp8C3;struct _tuple2*_tmp1BE=(_tmp8C3=_cycalloc(sizeof(*_tmp8C3)),((
_tmp8C3->f1=_tmp1A1->name,((_tmp8C3->f2=_tmp1A1->tq,((_tmp8C3->f3=_tmp1BD,
_tmp8C3)))))));struct Cyc_List_List*_tmp8C4;_tmp1BC=((struct Cyc_List_List*(*)(
struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_imp_append)(_tmp1BC,((
_tmp8C4=_cycalloc(sizeof(*_tmp8C4)),((_tmp8C4->hd=_tmp1BE,((_tmp8C4->tl=0,
_tmp8C4)))))));}{struct Cyc_Absyn_FnType_struct _tmp8CA;struct Cyc_Absyn_FnInfo
_tmp8C9;struct Cyc_Absyn_FnType_struct*_tmp8C8;return(void*)((_tmp8C8=_cycalloc(
sizeof(*_tmp8C8)),((_tmp8C8[0]=((_tmp8CA.tag=8,((_tmp8CA.f1=((_tmp8C9.tvars=0,((
_tmp8C9.effect=0,((_tmp8C9.ret_typ=(void*)Cyc_Toc_typ_to_c(_tmp19E),((_tmp8C9.args=
_tmp1BC,((_tmp8C9.c_varargs=_tmp1A0,((_tmp8C9.cyc_varargs=0,((_tmp8C9.rgn_po=0,((
_tmp8C9.attributes=_tmp1B9,_tmp8C9)))))))))))))))),_tmp8CA)))),_tmp8C8))));}}}
_LL5E: if(*((int*)_tmp185)!= 9)goto _LL60;_tmp1A3=((struct Cyc_Absyn_TupleType_struct*)
_tmp185)->f1;_LL5F: _tmp1A3=((struct Cyc_List_List*(*)(struct _tuple4*(*f)(struct
_tuple4*),struct Cyc_List_List*x))Cyc_List_map)(Cyc_Toc_typ_to_c_f,_tmp1A3);
return Cyc_Toc_add_tuple_type(_tmp1A3);_LL60: if(*((int*)_tmp185)!= 11)goto _LL62;
_tmp1A4=(void*)((struct Cyc_Absyn_AnonAggrType_struct*)_tmp185)->f1;_tmp1A5=((
struct Cyc_Absyn_AnonAggrType_struct*)_tmp185)->f2;_LL61: {struct Cyc_Absyn_AnonAggrType_struct
_tmp8CD;struct Cyc_Absyn_AnonAggrType_struct*_tmp8CC;return(void*)((_tmp8CC=
_cycalloc(sizeof(*_tmp8CC)),((_tmp8CC[0]=((_tmp8CD.tag=11,((_tmp8CD.f1=(void*)
_tmp1A4,((_tmp8CD.f2=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Aggrfield*(*f)(
struct Cyc_Absyn_Aggrfield*),struct Cyc_List_List*x))Cyc_List_map)(Cyc_Toc_aggrfield_to_c,
_tmp1A5),_tmp8CD)))))),_tmp8CC))));}_LL62: if(*((int*)_tmp185)!= 10)goto _LL64;
_tmp1A6=((struct Cyc_Absyn_AggrType_struct*)_tmp185)->f1;_tmp1A7=_tmp1A6.aggr_info;
_tmp1A8=_tmp1A6.targs;_LL63: {struct Cyc_Absyn_Aggrdecl*_tmp1C6=Cyc_Absyn_get_known_aggrdecl(
_tmp1A7);if((void*)_tmp1C6->kind == (void*)1)return Cyc_Toc_aggrdecl_type(_tmp1C6->name,
Cyc_Absyn_unionq_typ);else{return Cyc_Toc_aggrdecl_type(_tmp1C6->name,Cyc_Absyn_strctq);}}
_LL64: if(*((int*)_tmp185)!= 12)goto _LL66;_tmp1A9=((struct Cyc_Absyn_EnumType_struct*)
_tmp185)->f1;_LL65: return t;_LL66: if(*((int*)_tmp185)!= 13)goto _LL68;_tmp1AA=((
struct Cyc_Absyn_AnonEnumType_struct*)_tmp185)->f1;_LL67: Cyc_Toc_enumfields_to_c(
_tmp1AA);return t;_LL68: if(*((int*)_tmp185)!= 16)goto _LL6A;_tmp1AB=((struct Cyc_Absyn_TypedefType_struct*)
_tmp185)->f1;_tmp1AC=((struct Cyc_Absyn_TypedefType_struct*)_tmp185)->f2;_tmp1AD=((
struct Cyc_Absyn_TypedefType_struct*)_tmp185)->f3;_tmp1AE=((struct Cyc_Absyn_TypedefType_struct*)
_tmp185)->f4;_LL69: if(_tmp1AE == 0  || Cyc_noexpand_r){if(_tmp1AC != 0){struct Cyc_Absyn_TypedefType_struct
_tmp8D0;struct Cyc_Absyn_TypedefType_struct*_tmp8CF;return(void*)((_tmp8CF=
_cycalloc(sizeof(*_tmp8CF)),((_tmp8CF[0]=((_tmp8D0.tag=16,((_tmp8D0.f1=_tmp1AB,((
_tmp8D0.f2=0,((_tmp8D0.f3=_tmp1AD,((_tmp8D0.f4=0,_tmp8D0)))))))))),_tmp8CF))));}
else{return t;}}else{struct Cyc_Absyn_TypedefType_struct _tmp8D6;void**_tmp8D5;
struct Cyc_Absyn_TypedefType_struct*_tmp8D4;return(void*)((_tmp8D4=_cycalloc(
sizeof(*_tmp8D4)),((_tmp8D4[0]=((_tmp8D6.tag=16,((_tmp8D6.f1=_tmp1AB,((_tmp8D6.f2=
0,((_tmp8D6.f3=_tmp1AD,((_tmp8D6.f4=((_tmp8D5=_cycalloc(sizeof(*_tmp8D5)),((
_tmp8D5[0]=Cyc_Toc_typ_to_c_array(*_tmp1AE),_tmp8D5)))),_tmp8D6)))))))))),
_tmp8D4))));}_LL6A: if(*((int*)_tmp185)!= 18)goto _LL6C;_LL6B: return Cyc_Absyn_uint_typ;
_LL6C: if(*((int*)_tmp185)!= 14)goto _LL6E;_tmp1AF=(void*)((struct Cyc_Absyn_RgnHandleType_struct*)
_tmp185)->f1;_LL6D: return Cyc_Toc_rgn_typ();_LL6E: if(*((int*)_tmp185)!= 15)goto
_LL70;_LL6F: return Cyc_Toc_dyn_rgn_typ();_LL70: if((int)_tmp185 != 2)goto _LL72;
_LL71: {const char*_tmp8D9;void*_tmp8D8;(_tmp8D8=0,((int(*)(struct _dyneither_ptr
fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmp8D9="Toc::typ_to_c: type translation passed the heap region",
_tag_dyneither(_tmp8D9,sizeof(char),55))),_tag_dyneither(_tmp8D8,sizeof(void*),0)));}
_LL72: if((int)_tmp185 != 3)goto _LL74;_LL73: {const char*_tmp8DC;void*_tmp8DB;(
_tmp8DB=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((
_tmp8DC="Toc::typ_to_c: type translation passed the unique region",
_tag_dyneither(_tmp8DC,sizeof(char),57))),_tag_dyneither(_tmp8DB,sizeof(void*),0)));}
_LL74: if(_tmp185 <= (void*)4)goto _LL76;if(*((int*)_tmp185)!= 19)goto _LL76;_LL75:
goto _LL77;_LL76: if(_tmp185 <= (void*)4)goto _LL78;if(*((int*)_tmp185)!= 20)goto
_LL78;_LL77: goto _LL79;_LL78: if(_tmp185 <= (void*)4)goto _LL7A;if(*((int*)_tmp185)
!= 21)goto _LL7A;_LL79: {const char*_tmp8DF;void*_tmp8DE;(_tmp8DE=0,((int(*)(
struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmp8DF="Toc::typ_to_c: type translation passed an effect",
_tag_dyneither(_tmp8DF,sizeof(char),49))),_tag_dyneither(_tmp8DE,sizeof(void*),0)));}
_LL7A: if(_tmp185 <= (void*)4)goto _LL41;if(*((int*)_tmp185)!= 17)goto _LL41;_LL7B: {
const char*_tmp8E2;void*_tmp8E1;(_tmp8E1=0,((int(*)(struct _dyneither_ptr fmt,
struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmp8E2="Toc::typ_to_c: type translation passed a valueof_t",
_tag_dyneither(_tmp8E2,sizeof(char),51))),_tag_dyneither(_tmp8E1,sizeof(void*),0)));}
_LL41:;}static struct Cyc_Absyn_Exp*Cyc_Toc_array_to_ptr_cast(void*t,struct Cyc_Absyn_Exp*
e,struct Cyc_Position_Segment*l);static struct Cyc_Absyn_Exp*Cyc_Toc_array_to_ptr_cast(
void*t,struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*l){void*_tmp1D4=t;struct
Cyc_Absyn_ArrayInfo _tmp1D5;void*_tmp1D6;struct Cyc_Absyn_Tqual _tmp1D7;_LL8F: if(
_tmp1D4 <= (void*)4)goto _LL91;if(*((int*)_tmp1D4)!= 7)goto _LL91;_tmp1D5=((struct
Cyc_Absyn_ArrayType_struct*)_tmp1D4)->f1;_tmp1D6=(void*)_tmp1D5.elt_type;_tmp1D7=
_tmp1D5.tq;_LL90: return Cyc_Toc_cast_it(Cyc_Absyn_star_typ(_tmp1D6,(void*)2,
_tmp1D7,Cyc_Absyn_false_conref),e);_LL91:;_LL92: return Cyc_Toc_cast_it(t,e);_LL8E:;}
static int Cyc_Toc_atomic_typ(void*t);static int Cyc_Toc_atomic_typ(void*t){void*
_tmp1D8=Cyc_Tcutil_compress(t);struct Cyc_Absyn_ArrayInfo _tmp1D9;void*_tmp1DA;
struct Cyc_Absyn_AggrInfo _tmp1DB;union Cyc_Absyn_AggrInfoU_union _tmp1DC;struct Cyc_List_List*
_tmp1DD;struct Cyc_Absyn_TunionFieldInfo _tmp1DE;union Cyc_Absyn_TunionFieldInfoU_union
_tmp1DF;struct Cyc_Absyn_Tuniondecl*_tmp1E0;struct Cyc_Absyn_Tunionfield*_tmp1E1;
struct Cyc_List_List*_tmp1E2;_LL94: if((int)_tmp1D8 != 0)goto _LL96;_LL95: return 1;
_LL96: if(_tmp1D8 <= (void*)4)goto _LL9E;if(*((int*)_tmp1D8)!= 1)goto _LL98;_LL97:
return 0;_LL98: if(*((int*)_tmp1D8)!= 5)goto _LL9A;_LL99: goto _LL9B;_LL9A: if(*((int*)
_tmp1D8)!= 12)goto _LL9C;_LL9B: goto _LL9D;_LL9C: if(*((int*)_tmp1D8)!= 13)goto _LL9E;
_LL9D: goto _LL9F;_LL9E: if((int)_tmp1D8 != 1)goto _LLA0;_LL9F: goto _LLA1;_LLA0: if(
_tmp1D8 <= (void*)4)goto _LLB6;if(*((int*)_tmp1D8)!= 6)goto _LLA2;_LLA1: goto _LLA3;
_LLA2: if(*((int*)_tmp1D8)!= 8)goto _LLA4;_LLA3: goto _LLA5;_LLA4: if(*((int*)_tmp1D8)
!= 18)goto _LLA6;_LLA5: return 1;_LLA6: if(*((int*)_tmp1D8)!= 7)goto _LLA8;_tmp1D9=((
struct Cyc_Absyn_ArrayType_struct*)_tmp1D8)->f1;_tmp1DA=(void*)_tmp1D9.elt_type;
_LLA7: return Cyc_Toc_atomic_typ(_tmp1DA);_LLA8: if(*((int*)_tmp1D8)!= 10)goto _LLAA;
_tmp1DB=((struct Cyc_Absyn_AggrType_struct*)_tmp1D8)->f1;_tmp1DC=_tmp1DB.aggr_info;
_LLA9:{union Cyc_Absyn_AggrInfoU_union _tmp1E3=_tmp1DC;_LLB9: if((_tmp1E3.UnknownAggr).tag
!= 0)goto _LLBB;_LLBA: return 0;_LLBB:;_LLBC: goto _LLB8;_LLB8:;}{struct Cyc_Absyn_Aggrdecl*
_tmp1E4=Cyc_Absyn_get_known_aggrdecl(_tmp1DC);if(_tmp1E4->impl == 0)return 0;{
struct Cyc_List_List*_tmp1E5=((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp1E4->impl))->fields;
for(0;_tmp1E5 != 0;_tmp1E5=_tmp1E5->tl){if(!Cyc_Toc_atomic_typ((void*)((struct Cyc_Absyn_Aggrfield*)
_tmp1E5->hd)->type))return 0;}}return 1;}_LLAA: if(*((int*)_tmp1D8)!= 11)goto _LLAC;
_tmp1DD=((struct Cyc_Absyn_AnonAggrType_struct*)_tmp1D8)->f2;_LLAB: for(0;_tmp1DD
!= 0;_tmp1DD=_tmp1DD->tl){if(!Cyc_Toc_atomic_typ((void*)((struct Cyc_Absyn_Aggrfield*)
_tmp1DD->hd)->type))return 0;}return 1;_LLAC: if(*((int*)_tmp1D8)!= 3)goto _LLAE;
_tmp1DE=((struct Cyc_Absyn_TunionFieldType_struct*)_tmp1D8)->f1;_tmp1DF=_tmp1DE.field_info;
if((((((struct Cyc_Absyn_TunionFieldType_struct*)_tmp1D8)->f1).field_info).KnownTunionfield).tag
!= 1)goto _LLAE;_tmp1E0=(_tmp1DF.KnownTunionfield).f1;_tmp1E1=(_tmp1DF.KnownTunionfield).f2;
_LLAD: _tmp1E2=_tmp1E1->typs;goto _LLAF;_LLAE: if(*((int*)_tmp1D8)!= 9)goto _LLB0;
_tmp1E2=((struct Cyc_Absyn_TupleType_struct*)_tmp1D8)->f1;_LLAF: for(0;_tmp1E2 != 0;
_tmp1E2=_tmp1E2->tl){if(!Cyc_Toc_atomic_typ((*((struct _tuple4*)_tmp1E2->hd)).f2))
return 0;}return 1;_LLB0: if(*((int*)_tmp1D8)!= 2)goto _LLB2;_LLB1: goto _LLB3;_LLB2:
if(*((int*)_tmp1D8)!= 4)goto _LLB4;_LLB3: goto _LLB5;_LLB4: if(*((int*)_tmp1D8)!= 14)
goto _LLB6;_LLB5: return 0;_LLB6:;_LLB7: {const char*_tmp8E6;void*_tmp8E5[1];struct
Cyc_String_pa_struct _tmp8E4;(_tmp8E4.tag=0,((_tmp8E4.f1=(struct _dyneither_ptr)((
struct _dyneither_ptr)Cyc_Absynpp_typ2string(t)),((_tmp8E5[0]=& _tmp8E4,((int(*)(
struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmp8E6="atomic_typ:  bad type %s",
_tag_dyneither(_tmp8E6,sizeof(char),25))),_tag_dyneither(_tmp8E5,sizeof(void*),1)))))));}
_LL93:;}static int Cyc_Toc_is_void_star(void*t);static int Cyc_Toc_is_void_star(void*
t){void*_tmp1E9=Cyc_Tcutil_compress(t);struct Cyc_Absyn_PtrInfo _tmp1EA;void*
_tmp1EB;_LLBE: if(_tmp1E9 <= (void*)4)goto _LLC0;if(*((int*)_tmp1E9)!= 4)goto _LLC0;
_tmp1EA=((struct Cyc_Absyn_PointerType_struct*)_tmp1E9)->f1;_tmp1EB=(void*)
_tmp1EA.elt_typ;_LLBF: {void*_tmp1EC=Cyc_Tcutil_compress(_tmp1EB);_LLC3: if((int)
_tmp1EC != 0)goto _LLC5;_LLC4: return 1;_LLC5:;_LLC6: return 0;_LLC2:;}_LLC0:;_LLC1:
return 0;_LLBD:;}static int Cyc_Toc_is_poly_field(void*t,struct _dyneither_ptr*f);
static int Cyc_Toc_is_poly_field(void*t,struct _dyneither_ptr*f){void*_tmp1ED=Cyc_Tcutil_compress(
t);struct Cyc_Absyn_AggrInfo _tmp1EE;union Cyc_Absyn_AggrInfoU_union _tmp1EF;struct
Cyc_List_List*_tmp1F0;_LLC8: if(_tmp1ED <= (void*)4)goto _LLCC;if(*((int*)_tmp1ED)
!= 10)goto _LLCA;_tmp1EE=((struct Cyc_Absyn_AggrType_struct*)_tmp1ED)->f1;_tmp1EF=
_tmp1EE.aggr_info;_LLC9: {struct Cyc_Absyn_Aggrdecl*_tmp1F1=Cyc_Absyn_get_known_aggrdecl(
_tmp1EF);if(_tmp1F1->impl == 0){const char*_tmp8E9;void*_tmp8E8;(_tmp8E8=0,((int(*)(
struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmp8E9="is_poly_field: type missing fields",
_tag_dyneither(_tmp8E9,sizeof(char),35))),_tag_dyneither(_tmp8E8,sizeof(void*),0)));}
_tmp1F0=((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp1F1->impl))->fields;goto
_LLCB;}_LLCA: if(*((int*)_tmp1ED)!= 11)goto _LLCC;_tmp1F0=((struct Cyc_Absyn_AnonAggrType_struct*)
_tmp1ED)->f2;_LLCB: {struct Cyc_Absyn_Aggrfield*_tmp1F4=Cyc_Absyn_lookup_field(
_tmp1F0,f);if(_tmp1F4 == 0){const char*_tmp8ED;void*_tmp8EC[1];struct Cyc_String_pa_struct
_tmp8EB;(_tmp8EB.tag=0,((_tmp8EB.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*
f),((_tmp8EC[0]=& _tmp8EB,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))
Cyc_Toc_toc_impos)(((_tmp8ED="is_poly_field: bad field %s",_tag_dyneither(
_tmp8ED,sizeof(char),28))),_tag_dyneither(_tmp8EC,sizeof(void*),1)))))));}return
Cyc_Toc_is_void_star((void*)_tmp1F4->type);}_LLCC:;_LLCD: {const char*_tmp8F1;
void*_tmp8F0[1];struct Cyc_String_pa_struct _tmp8EF;(_tmp8EF.tag=0,((_tmp8EF.f1=(
struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t)),((_tmp8F0[
0]=& _tmp8EF,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((
_tmp8F1="is_poly_field: bad type %s",_tag_dyneither(_tmp8F1,sizeof(char),27))),
_tag_dyneither(_tmp8F0,sizeof(void*),1)))))));}_LLC7:;}static int Cyc_Toc_is_poly_project(
struct Cyc_Absyn_Exp*e);static int Cyc_Toc_is_poly_project(struct Cyc_Absyn_Exp*e){
void*_tmp1FB=(void*)e->r;struct Cyc_Absyn_Exp*_tmp1FC;struct _dyneither_ptr*
_tmp1FD;struct Cyc_Absyn_Exp*_tmp1FE;struct _dyneither_ptr*_tmp1FF;_LLCF: if(*((int*)
_tmp1FB)!= 23)goto _LLD1;_tmp1FC=((struct Cyc_Absyn_AggrMember_e_struct*)_tmp1FB)->f1;
_tmp1FD=((struct Cyc_Absyn_AggrMember_e_struct*)_tmp1FB)->f2;_LLD0: return Cyc_Toc_is_poly_field((
void*)((struct Cyc_Core_Opt*)_check_null(_tmp1FC->topt))->v,_tmp1FD);_LLD1: if(*((
int*)_tmp1FB)!= 24)goto _LLD3;_tmp1FE=((struct Cyc_Absyn_AggrArrow_e_struct*)
_tmp1FB)->f1;_tmp1FF=((struct Cyc_Absyn_AggrArrow_e_struct*)_tmp1FB)->f2;_LLD2: {
void*_tmp200=Cyc_Tcutil_compress((void*)((struct Cyc_Core_Opt*)_check_null(
_tmp1FE->topt))->v);struct Cyc_Absyn_PtrInfo _tmp201;void*_tmp202;_LLD6: if(_tmp200
<= (void*)4)goto _LLD8;if(*((int*)_tmp200)!= 4)goto _LLD8;_tmp201=((struct Cyc_Absyn_PointerType_struct*)
_tmp200)->f1;_tmp202=(void*)_tmp201.elt_typ;_LLD7: return Cyc_Toc_is_poly_field(
_tmp202,_tmp1FF);_LLD8:;_LLD9: {const char*_tmp8F5;void*_tmp8F4[1];struct Cyc_String_pa_struct
_tmp8F3;(_tmp8F3.tag=0,((_tmp8F3.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Absynpp_typ2string((void*)((struct Cyc_Core_Opt*)_check_null(_tmp1FE->topt))->v)),((
_tmp8F4[0]=& _tmp8F3,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((
_tmp8F5="is_poly_project: bad type %s",_tag_dyneither(_tmp8F5,sizeof(char),29))),
_tag_dyneither(_tmp8F4,sizeof(void*),1)))))));}_LLD5:;}_LLD3:;_LLD4: return 0;
_LLCE:;}static struct Cyc_Absyn_Exp*Cyc_Toc_malloc_ptr(struct Cyc_Absyn_Exp*s);
static struct Cyc_Absyn_Exp*Cyc_Toc_malloc_ptr(struct Cyc_Absyn_Exp*s){struct Cyc_List_List*
_tmp8F6;return Cyc_Absyn_fncall_exp(Cyc_Toc__cycalloc_e,((_tmp8F6=_cycalloc(
sizeof(*_tmp8F6)),((_tmp8F6->hd=s,((_tmp8F6->tl=0,_tmp8F6)))))),0);}static struct
Cyc_Absyn_Exp*Cyc_Toc_malloc_atomic(struct Cyc_Absyn_Exp*s);static struct Cyc_Absyn_Exp*
Cyc_Toc_malloc_atomic(struct Cyc_Absyn_Exp*s){struct Cyc_List_List*_tmp8F7;return
Cyc_Absyn_fncall_exp(Cyc_Toc__cycalloc_atomic_e,((_tmp8F7=_cycalloc(sizeof(*
_tmp8F7)),((_tmp8F7->hd=s,((_tmp8F7->tl=0,_tmp8F7)))))),0);}static struct Cyc_Absyn_Exp*
Cyc_Toc_malloc_exp(void*t,struct Cyc_Absyn_Exp*s);static struct Cyc_Absyn_Exp*Cyc_Toc_malloc_exp(
void*t,struct Cyc_Absyn_Exp*s){if(Cyc_Toc_atomic_typ(t))return Cyc_Toc_malloc_atomic(
s);return Cyc_Toc_malloc_ptr(s);}static struct Cyc_Absyn_Exp*Cyc_Toc_rmalloc_exp(
struct Cyc_Absyn_Exp*rgn,struct Cyc_Absyn_Exp*s);static struct Cyc_Absyn_Exp*Cyc_Toc_rmalloc_exp(
struct Cyc_Absyn_Exp*rgn,struct Cyc_Absyn_Exp*s){struct Cyc_Absyn_Exp*_tmp8F8[2];
return Cyc_Absyn_fncall_exp(Cyc_Toc__region_malloc_e,((_tmp8F8[1]=s,((_tmp8F8[0]=
rgn,((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(
_tag_dyneither(_tmp8F8,sizeof(struct Cyc_Absyn_Exp*),2)))))),0);}static struct Cyc_Absyn_Exp*
Cyc_Toc_calloc_exp(void*elt_type,struct Cyc_Absyn_Exp*s,struct Cyc_Absyn_Exp*n);
static struct Cyc_Absyn_Exp*Cyc_Toc_calloc_exp(void*elt_type,struct Cyc_Absyn_Exp*s,
struct Cyc_Absyn_Exp*n){if(Cyc_Toc_atomic_typ(elt_type)){struct Cyc_Absyn_Exp*
_tmp8F9[2];return Cyc_Absyn_fncall_exp(Cyc_Toc__cyccalloc_atomic_e,((_tmp8F9[1]=n,((
_tmp8F9[0]=s,((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(
_tag_dyneither(_tmp8F9,sizeof(struct Cyc_Absyn_Exp*),2)))))),0);}else{struct Cyc_Absyn_Exp*
_tmp8FA[2];return Cyc_Absyn_fncall_exp(Cyc_Toc__cyccalloc_e,((_tmp8FA[1]=n,((
_tmp8FA[0]=s,((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(
_tag_dyneither(_tmp8FA,sizeof(struct Cyc_Absyn_Exp*),2)))))),0);}}static struct Cyc_Absyn_Exp*
Cyc_Toc_rcalloc_exp(struct Cyc_Absyn_Exp*rgn,struct Cyc_Absyn_Exp*s,struct Cyc_Absyn_Exp*
n);static struct Cyc_Absyn_Exp*Cyc_Toc_rcalloc_exp(struct Cyc_Absyn_Exp*rgn,struct
Cyc_Absyn_Exp*s,struct Cyc_Absyn_Exp*n){struct Cyc_Absyn_Exp*_tmp8FB[3];return Cyc_Absyn_fncall_exp(
Cyc_Toc__region_calloc_e,((_tmp8FB[2]=n,((_tmp8FB[1]=s,((_tmp8FB[0]=rgn,((struct
Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp8FB,
sizeof(struct Cyc_Absyn_Exp*),3)))))))),0);}static struct Cyc_Absyn_Exp*Cyc_Toc_newthrow_exp(
struct Cyc_Absyn_Exp*e);static struct Cyc_Absyn_Exp*Cyc_Toc_newthrow_exp(struct Cyc_Absyn_Exp*
e){struct Cyc_List_List*_tmp8FC;return Cyc_Absyn_fncall_exp(Cyc_Toc__throw_e,((
_tmp8FC=_cycalloc(sizeof(*_tmp8FC)),((_tmp8FC->hd=e,((_tmp8FC->tl=0,_tmp8FC)))))),
0);}static struct Cyc_Absyn_Exp*Cyc_Toc_make_toplevel_dyn_arr(void*t,struct Cyc_Absyn_Exp*
sz,struct Cyc_Absyn_Exp*e);static struct Cyc_Absyn_Exp*Cyc_Toc_make_toplevel_dyn_arr(
void*t,struct Cyc_Absyn_Exp*sz,struct Cyc_Absyn_Exp*e){int is_string=0;{void*
_tmp20D=(void*)e->r;union Cyc_Absyn_Cnst_union _tmp20E;_LLDB: if(*((int*)_tmp20D)!= 
0)goto _LLDD;_tmp20E=((struct Cyc_Absyn_Const_e_struct*)_tmp20D)->f1;if(((((struct
Cyc_Absyn_Const_e_struct*)_tmp20D)->f1).String_c).tag != 5)goto _LLDD;_LLDC:
is_string=1;goto _LLDA;_LLDD:;_LLDE: goto _LLDA;_LLDA:;}{struct Cyc_Absyn_Exp*xexp;
struct Cyc_Absyn_Exp*xplussz;if(is_string){struct _tuple1*x=Cyc_Toc_temp_var();
void*vd_typ=Cyc_Absyn_array_typ(Cyc_Absyn_char_typ,Cyc_Toc_mt_tq,(struct Cyc_Absyn_Exp*)
sz,Cyc_Absyn_false_conref,0);struct Cyc_Absyn_Vardecl*vd=Cyc_Absyn_static_vardecl(
x,vd_typ,(struct Cyc_Absyn_Exp*)e);{struct Cyc_Absyn_Var_d_struct*_tmp902;struct
Cyc_Absyn_Var_d_struct _tmp901;struct Cyc_List_List*_tmp900;Cyc_Toc_result_decls=((
_tmp900=_cycalloc(sizeof(*_tmp900)),((_tmp900->hd=Cyc_Absyn_new_decl((void*)((
_tmp902=_cycalloc(sizeof(*_tmp902)),((_tmp902[0]=((_tmp901.tag=0,((_tmp901.f1=vd,
_tmp901)))),_tmp902)))),0),((_tmp900->tl=Cyc_Toc_result_decls,_tmp900))))));}
xexp=Cyc_Absyn_var_exp(x,0);xplussz=Cyc_Absyn_add_exp(xexp,sz,0);}else{xexp=Cyc_Toc_cast_it(
Cyc_Absyn_void_star_typ(),e);xplussz=Cyc_Toc_cast_it(Cyc_Absyn_void_star_typ(),
Cyc_Absyn_add_exp(e,sz,0));}{struct Cyc_Absyn_Exp*urm_exp;{struct _tuple8*_tmp909;
struct _tuple8*_tmp908;struct _tuple8*_tmp907;struct _tuple8*_tmp906[3];urm_exp=Cyc_Absyn_unresolvedmem_exp(
0,((_tmp906[2]=((_tmp909=_cycalloc(sizeof(*_tmp909)),((_tmp909->f1=0,((_tmp909->f2=
xplussz,_tmp909)))))),((_tmp906[1]=((_tmp908=_cycalloc(sizeof(*_tmp908)),((
_tmp908->f1=0,((_tmp908->f2=xexp,_tmp908)))))),((_tmp906[0]=((_tmp907=_cycalloc(
sizeof(*_tmp907)),((_tmp907->f1=0,((_tmp907->f2=xexp,_tmp907)))))),((struct Cyc_List_List*(*)(
struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp906,sizeof(struct _tuple8*),
3)))))))),0);}return urm_exp;}}}struct Cyc_Toc_FallthruInfo{struct _dyneither_ptr*
label;struct Cyc_List_List*binders;struct Cyc_Dict_Dict next_case_env;};struct Cyc_Toc_Env{
struct _dyneither_ptr**break_lab;struct _dyneither_ptr**continue_lab;struct Cyc_Toc_FallthruInfo*
fallthru_info;struct Cyc_Dict_Dict varmap;int toplevel;};static int Cyc_Toc_is_toplevel(
struct Cyc_Toc_Env*nv);static int Cyc_Toc_is_toplevel(struct Cyc_Toc_Env*nv){struct
Cyc_Toc_Env _tmp217;int _tmp218;struct Cyc_Toc_Env*_tmp216=nv;_tmp217=*_tmp216;
_tmp218=_tmp217.toplevel;return _tmp218;}static struct Cyc_Absyn_Exp*Cyc_Toc_lookup_varmap(
struct Cyc_Toc_Env*nv,struct _tuple1*x);static struct Cyc_Absyn_Exp*Cyc_Toc_lookup_varmap(
struct Cyc_Toc_Env*nv,struct _tuple1*x){struct Cyc_Toc_Env _tmp21A;struct Cyc_Dict_Dict
_tmp21B;struct Cyc_Toc_Env*_tmp219=nv;_tmp21A=*_tmp219;_tmp21B=_tmp21A.varmap;
return((struct Cyc_Absyn_Exp*(*)(struct Cyc_Dict_Dict d,struct _tuple1*k))Cyc_Dict_lookup)(
_tmp21B,x);}static struct Cyc_Toc_Env*Cyc_Toc_empty_env(struct _RegionHandle*r);
static struct Cyc_Toc_Env*Cyc_Toc_empty_env(struct _RegionHandle*r){struct Cyc_Toc_Env*
_tmp90A;return(_tmp90A=_region_malloc(r,sizeof(*_tmp90A)),((_tmp90A->break_lab=(
struct _dyneither_ptr**)0,((_tmp90A->continue_lab=(struct _dyneither_ptr**)0,((
_tmp90A->fallthru_info=(struct Cyc_Toc_FallthruInfo*)0,((_tmp90A->varmap=(struct
Cyc_Dict_Dict)((struct Cyc_Dict_Dict(*)(struct _RegionHandle*,int(*cmp)(struct
_tuple1*,struct _tuple1*)))Cyc_Dict_rempty)(r,Cyc_Absyn_qvar_cmp),((_tmp90A->toplevel=(
int)1,_tmp90A)))))))))));}static struct Cyc_Toc_Env*Cyc_Toc_share_env(struct
_RegionHandle*r,struct Cyc_Toc_Env*e);static struct Cyc_Toc_Env*Cyc_Toc_share_env(
struct _RegionHandle*r,struct Cyc_Toc_Env*e){struct Cyc_Toc_Env _tmp21E;struct
_dyneither_ptr**_tmp21F;struct _dyneither_ptr**_tmp220;struct Cyc_Toc_FallthruInfo*
_tmp221;struct Cyc_Dict_Dict _tmp222;int _tmp223;struct Cyc_Toc_Env*_tmp21D=e;
_tmp21E=*_tmp21D;_tmp21F=_tmp21E.break_lab;_tmp220=_tmp21E.continue_lab;_tmp221=
_tmp21E.fallthru_info;_tmp222=_tmp21E.varmap;_tmp223=_tmp21E.toplevel;{struct Cyc_Toc_Env*
_tmp90B;return(_tmp90B=_region_malloc(r,sizeof(*_tmp90B)),((_tmp90B->break_lab=(
struct _dyneither_ptr**)((struct _dyneither_ptr**)_tmp21F),((_tmp90B->continue_lab=(
struct _dyneither_ptr**)((struct _dyneither_ptr**)_tmp220),((_tmp90B->fallthru_info=(
struct Cyc_Toc_FallthruInfo*)_tmp221,((_tmp90B->varmap=(struct Cyc_Dict_Dict)((
struct Cyc_Dict_Dict(*)(struct _RegionHandle*,struct Cyc_Dict_Dict))Cyc_Dict_rshare)(
r,_tmp222),((_tmp90B->toplevel=(int)_tmp223,_tmp90B)))))))))));}}static struct Cyc_Toc_Env*
Cyc_Toc_clear_toplevel(struct _RegionHandle*r,struct Cyc_Toc_Env*e);static struct
Cyc_Toc_Env*Cyc_Toc_clear_toplevel(struct _RegionHandle*r,struct Cyc_Toc_Env*e){
struct Cyc_Toc_Env _tmp226;struct _dyneither_ptr**_tmp227;struct _dyneither_ptr**
_tmp228;struct Cyc_Toc_FallthruInfo*_tmp229;struct Cyc_Dict_Dict _tmp22A;int _tmp22B;
struct Cyc_Toc_Env*_tmp225=e;_tmp226=*_tmp225;_tmp227=_tmp226.break_lab;_tmp228=
_tmp226.continue_lab;_tmp229=_tmp226.fallthru_info;_tmp22A=_tmp226.varmap;
_tmp22B=_tmp226.toplevel;{struct Cyc_Toc_Env*_tmp90C;return(_tmp90C=
_region_malloc(r,sizeof(*_tmp90C)),((_tmp90C->break_lab=(struct _dyneither_ptr**)((
struct _dyneither_ptr**)_tmp227),((_tmp90C->continue_lab=(struct _dyneither_ptr**)((
struct _dyneither_ptr**)_tmp228),((_tmp90C->fallthru_info=(struct Cyc_Toc_FallthruInfo*)
_tmp229,((_tmp90C->varmap=(struct Cyc_Dict_Dict)((struct Cyc_Dict_Dict(*)(struct
_RegionHandle*,struct Cyc_Dict_Dict))Cyc_Dict_rshare)(r,_tmp22A),((_tmp90C->toplevel=(
int)0,_tmp90C)))))))))));}}static struct Cyc_Toc_Env*Cyc_Toc_set_toplevel(struct
_RegionHandle*r,struct Cyc_Toc_Env*e);static struct Cyc_Toc_Env*Cyc_Toc_set_toplevel(
struct _RegionHandle*r,struct Cyc_Toc_Env*e){struct Cyc_Toc_Env _tmp22E;struct
_dyneither_ptr**_tmp22F;struct _dyneither_ptr**_tmp230;struct Cyc_Toc_FallthruInfo*
_tmp231;struct Cyc_Dict_Dict _tmp232;int _tmp233;struct Cyc_Toc_Env*_tmp22D=e;
_tmp22E=*_tmp22D;_tmp22F=_tmp22E.break_lab;_tmp230=_tmp22E.continue_lab;_tmp231=
_tmp22E.fallthru_info;_tmp232=_tmp22E.varmap;_tmp233=_tmp22E.toplevel;{struct Cyc_Toc_Env*
_tmp90D;return(_tmp90D=_region_malloc(r,sizeof(*_tmp90D)),((_tmp90D->break_lab=(
struct _dyneither_ptr**)((struct _dyneither_ptr**)_tmp22F),((_tmp90D->continue_lab=(
struct _dyneither_ptr**)((struct _dyneither_ptr**)_tmp230),((_tmp90D->fallthru_info=(
struct Cyc_Toc_FallthruInfo*)_tmp231,((_tmp90D->varmap=(struct Cyc_Dict_Dict)((
struct Cyc_Dict_Dict(*)(struct _RegionHandle*,struct Cyc_Dict_Dict))Cyc_Dict_rshare)(
r,_tmp232),((_tmp90D->toplevel=(int)1,_tmp90D)))))))))));}}static struct Cyc_Toc_Env*
Cyc_Toc_add_varmap(struct _RegionHandle*r,struct Cyc_Toc_Env*e,struct _tuple1*x,
struct Cyc_Absyn_Exp*y);static struct Cyc_Toc_Env*Cyc_Toc_add_varmap(struct
_RegionHandle*r,struct Cyc_Toc_Env*e,struct _tuple1*x,struct Cyc_Absyn_Exp*y){{
union Cyc_Absyn_Nmspace_union _tmp235=(*x).f1;_LLE0: if((_tmp235.Rel_n).tag != 1)
goto _LLE2;_LLE1: {const char*_tmp911;void*_tmp910[1];struct Cyc_String_pa_struct
_tmp90F;(_tmp90F.tag=0,((_tmp90F.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Absynpp_qvar2string(x)),((_tmp910[0]=& _tmp90F,((int(*)(struct _dyneither_ptr
fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmp911="Toc::add_varmap on Rel_n: %s\n",
_tag_dyneither(_tmp911,sizeof(char),30))),_tag_dyneither(_tmp910,sizeof(void*),1)))))));}
_LLE2:;_LLE3: goto _LLDF;_LLDF:;}{struct Cyc_Toc_Env _tmp23A;struct _dyneither_ptr**
_tmp23B;struct _dyneither_ptr**_tmp23C;struct Cyc_Toc_FallthruInfo*_tmp23D;struct
Cyc_Dict_Dict _tmp23E;int _tmp23F;struct Cyc_Toc_Env*_tmp239=e;_tmp23A=*_tmp239;
_tmp23B=_tmp23A.break_lab;_tmp23C=_tmp23A.continue_lab;_tmp23D=_tmp23A.fallthru_info;
_tmp23E=_tmp23A.varmap;_tmp23F=_tmp23A.toplevel;{struct Cyc_Dict_Dict _tmp240=((
struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct _tuple1*k,struct Cyc_Absyn_Exp*
v))Cyc_Dict_insert)(((struct Cyc_Dict_Dict(*)(struct _RegionHandle*,struct Cyc_Dict_Dict))
Cyc_Dict_rshare)(r,_tmp23E),x,y);struct Cyc_Toc_Env*_tmp912;return(_tmp912=
_region_malloc(r,sizeof(*_tmp912)),((_tmp912->break_lab=(struct _dyneither_ptr**)((
struct _dyneither_ptr**)_tmp23B),((_tmp912->continue_lab=(struct _dyneither_ptr**)((
struct _dyneither_ptr**)_tmp23C),((_tmp912->fallthru_info=(struct Cyc_Toc_FallthruInfo*)
_tmp23D,((_tmp912->varmap=(struct Cyc_Dict_Dict)_tmp240,((_tmp912->toplevel=(int)
_tmp23F,_tmp912)))))))))));}}}static struct Cyc_Toc_Env*Cyc_Toc_loop_env(struct
_RegionHandle*r,struct Cyc_Toc_Env*e);static struct Cyc_Toc_Env*Cyc_Toc_loop_env(
struct _RegionHandle*r,struct Cyc_Toc_Env*e){struct Cyc_Toc_Env _tmp243;struct
_dyneither_ptr**_tmp244;struct _dyneither_ptr**_tmp245;struct Cyc_Toc_FallthruInfo*
_tmp246;struct Cyc_Dict_Dict _tmp247;int _tmp248;struct Cyc_Toc_Env*_tmp242=e;
_tmp243=*_tmp242;_tmp244=_tmp243.break_lab;_tmp245=_tmp243.continue_lab;_tmp246=
_tmp243.fallthru_info;_tmp247=_tmp243.varmap;_tmp248=_tmp243.toplevel;{struct Cyc_Toc_Env*
_tmp913;return(_tmp913=_region_malloc(r,sizeof(*_tmp913)),((_tmp913->break_lab=(
struct _dyneither_ptr**)0,((_tmp913->continue_lab=(struct _dyneither_ptr**)0,((
_tmp913->fallthru_info=(struct Cyc_Toc_FallthruInfo*)_tmp246,((_tmp913->varmap=(
struct Cyc_Dict_Dict)((struct Cyc_Dict_Dict(*)(struct _RegionHandle*,struct Cyc_Dict_Dict))
Cyc_Dict_rshare)(r,_tmp247),((_tmp913->toplevel=(int)_tmp248,_tmp913)))))))))));}}
static struct Cyc_Toc_Env*Cyc_Toc_non_last_switchclause_env(struct _RegionHandle*r,
struct Cyc_Toc_Env*e,struct _dyneither_ptr*break_l,struct _dyneither_ptr*fallthru_l,
struct Cyc_List_List*fallthru_binders,struct Cyc_Toc_Env*next_case_env);static
struct Cyc_Toc_Env*Cyc_Toc_non_last_switchclause_env(struct _RegionHandle*r,struct
Cyc_Toc_Env*e,struct _dyneither_ptr*break_l,struct _dyneither_ptr*fallthru_l,
struct Cyc_List_List*fallthru_binders,struct Cyc_Toc_Env*next_case_env){struct Cyc_List_List*
fallthru_vars=0;for(0;fallthru_binders != 0;fallthru_binders=fallthru_binders->tl){
struct Cyc_List_List*_tmp914;fallthru_vars=((_tmp914=_region_malloc(r,sizeof(*
_tmp914)),((_tmp914->hd=((struct Cyc_Absyn_Vardecl*)fallthru_binders->hd)->name,((
_tmp914->tl=fallthru_vars,_tmp914))))));}fallthru_vars=((struct Cyc_List_List*(*)(
struct Cyc_List_List*x))Cyc_List_imp_rev)(fallthru_vars);{struct Cyc_Toc_Env
_tmp24C;struct _dyneither_ptr**_tmp24D;struct _dyneither_ptr**_tmp24E;struct Cyc_Toc_FallthruInfo*
_tmp24F;struct Cyc_Dict_Dict _tmp250;int _tmp251;struct Cyc_Toc_Env*_tmp24B=e;
_tmp24C=*_tmp24B;_tmp24D=_tmp24C.break_lab;_tmp24E=_tmp24C.continue_lab;_tmp24F=
_tmp24C.fallthru_info;_tmp250=_tmp24C.varmap;_tmp251=_tmp24C.toplevel;{struct Cyc_Toc_Env
_tmp253;struct Cyc_Dict_Dict _tmp254;struct Cyc_Toc_Env*_tmp252=next_case_env;
_tmp253=*_tmp252;_tmp254=_tmp253.varmap;{struct Cyc_Toc_FallthruInfo*_tmp915;
struct Cyc_Toc_FallthruInfo*fi=(_tmp915=_region_malloc(r,sizeof(*_tmp915)),((
_tmp915->label=fallthru_l,((_tmp915->binders=fallthru_vars,((_tmp915->next_case_env=((
struct Cyc_Dict_Dict(*)(struct _RegionHandle*,struct Cyc_Dict_Dict))Cyc_Dict_rshare)(
r,_tmp254),_tmp915)))))));struct _dyneither_ptr**_tmp918;struct Cyc_Toc_Env*
_tmp917;return(_tmp917=_region_malloc(r,sizeof(*_tmp917)),((_tmp917->break_lab=(
struct _dyneither_ptr**)((_tmp918=_region_malloc(r,sizeof(*_tmp918)),((_tmp918[0]=
break_l,_tmp918)))),((_tmp917->continue_lab=(struct _dyneither_ptr**)((struct
_dyneither_ptr**)_tmp24E),((_tmp917->fallthru_info=(struct Cyc_Toc_FallthruInfo*)
fi,((_tmp917->varmap=(struct Cyc_Dict_Dict)((struct Cyc_Dict_Dict(*)(struct
_RegionHandle*,struct Cyc_Dict_Dict))Cyc_Dict_rshare)(r,_tmp250),((_tmp917->toplevel=(
int)_tmp251,_tmp917)))))))))));}}}}static struct Cyc_Toc_Env*Cyc_Toc_last_switchclause_env(
struct _RegionHandle*r,struct Cyc_Toc_Env*e,struct _dyneither_ptr*break_l);static
struct Cyc_Toc_Env*Cyc_Toc_last_switchclause_env(struct _RegionHandle*r,struct Cyc_Toc_Env*
e,struct _dyneither_ptr*break_l){struct Cyc_Toc_Env _tmp259;struct _dyneither_ptr**
_tmp25A;struct _dyneither_ptr**_tmp25B;struct Cyc_Toc_FallthruInfo*_tmp25C;struct
Cyc_Dict_Dict _tmp25D;int _tmp25E;struct Cyc_Toc_Env*_tmp258=e;_tmp259=*_tmp258;
_tmp25A=_tmp259.break_lab;_tmp25B=_tmp259.continue_lab;_tmp25C=_tmp259.fallthru_info;
_tmp25D=_tmp259.varmap;_tmp25E=_tmp259.toplevel;{struct _dyneither_ptr**_tmp91B;
struct Cyc_Toc_Env*_tmp91A;return(_tmp91A=_region_malloc(r,sizeof(*_tmp91A)),((
_tmp91A->break_lab=(struct _dyneither_ptr**)((_tmp91B=_region_malloc(r,sizeof(*
_tmp91B)),((_tmp91B[0]=break_l,_tmp91B)))),((_tmp91A->continue_lab=(struct
_dyneither_ptr**)((struct _dyneither_ptr**)_tmp25B),((_tmp91A->fallthru_info=(
struct Cyc_Toc_FallthruInfo*)0,((_tmp91A->varmap=(struct Cyc_Dict_Dict)((struct Cyc_Dict_Dict(*)(
struct _RegionHandle*,struct Cyc_Dict_Dict))Cyc_Dict_rshare)(r,_tmp25D),((_tmp91A->toplevel=(
int)_tmp25E,_tmp91A)))))))))));}}static struct Cyc_Toc_Env*Cyc_Toc_switch_as_switch_env(
struct _RegionHandle*r,struct Cyc_Toc_Env*e,struct _dyneither_ptr*next_l);static
struct Cyc_Toc_Env*Cyc_Toc_switch_as_switch_env(struct _RegionHandle*r,struct Cyc_Toc_Env*
e,struct _dyneither_ptr*next_l){struct Cyc_Toc_Env _tmp262;struct _dyneither_ptr**
_tmp263;struct _dyneither_ptr**_tmp264;struct Cyc_Toc_FallthruInfo*_tmp265;struct
Cyc_Dict_Dict _tmp266;int _tmp267;struct Cyc_Toc_Env*_tmp261=e;_tmp262=*_tmp261;
_tmp263=_tmp262.break_lab;_tmp264=_tmp262.continue_lab;_tmp265=_tmp262.fallthru_info;
_tmp266=_tmp262.varmap;_tmp267=_tmp262.toplevel;{struct Cyc_Toc_FallthruInfo*
_tmp91E;struct Cyc_Toc_Env*_tmp91D;return(_tmp91D=_region_malloc(r,sizeof(*
_tmp91D)),((_tmp91D->break_lab=(struct _dyneither_ptr**)0,((_tmp91D->continue_lab=(
struct _dyneither_ptr**)((struct _dyneither_ptr**)_tmp264),((_tmp91D->fallthru_info=(
struct Cyc_Toc_FallthruInfo*)((_tmp91E=_region_malloc(r,sizeof(*_tmp91E)),((
_tmp91E->label=next_l,((_tmp91E->binders=0,((_tmp91E->next_case_env=((struct Cyc_Dict_Dict(*)(
struct _RegionHandle*,int(*cmp)(struct _tuple1*,struct _tuple1*)))Cyc_Dict_rempty)(
r,Cyc_Absyn_qvar_cmp),_tmp91E)))))))),((_tmp91D->varmap=(struct Cyc_Dict_Dict)((
struct Cyc_Dict_Dict(*)(struct _RegionHandle*,struct Cyc_Dict_Dict))Cyc_Dict_rshare)(
r,_tmp266),((_tmp91D->toplevel=(int)_tmp267,_tmp91D)))))))))));}}static void Cyc_Toc_exp_to_c(
struct Cyc_Toc_Env*nv,struct Cyc_Absyn_Exp*e);static void Cyc_Toc_stmt_to_c(struct
Cyc_Toc_Env*nv,struct Cyc_Absyn_Stmt*s);static int Cyc_Toc_need_null_check(struct
Cyc_Absyn_Exp*e);static int Cyc_Toc_need_null_check(struct Cyc_Absyn_Exp*e){void*
_tmp26A=(void*)e->annot;_LLE5: if(*((void**)_tmp26A)!= Cyc_CfFlowInfo_UnknownZ)
goto _LLE7;_LLE6: return Cyc_Toc_is_nullable((void*)((struct Cyc_Core_Opt*)
_check_null(e->topt))->v);_LLE7: if(*((void**)_tmp26A)!= Cyc_CfFlowInfo_NotZero)
goto _LLE9;_LLE8: return 0;_LLE9: if(_tmp26A != Cyc_CfFlowInfo_IsZero)goto _LLEB;_LLEA:{
const char*_tmp921;void*_tmp920;(_tmp920=0,Cyc_Tcutil_terr(e->loc,((_tmp921="dereference of NULL pointer",
_tag_dyneither(_tmp921,sizeof(char),28))),_tag_dyneither(_tmp920,sizeof(void*),0)));}
return 0;_LLEB: if(_tmp26A != Cyc_Absyn_EmptyAnnot)goto _LLED;_LLEC: return 0;_LLED:
if(*((void**)_tmp26A)!= Cyc_CfFlowInfo_HasTagCmps)goto _LLEF;_LLEE:{const char*
_tmp924;void*_tmp923;(_tmp923=0,Cyc_Tcutil_warn(e->loc,((_tmp924="compiler oddity: pointer compared to tag type",
_tag_dyneither(_tmp924,sizeof(char),46))),_tag_dyneither(_tmp923,sizeof(void*),0)));}
return Cyc_Toc_is_nullable((void*)((struct Cyc_Core_Opt*)_check_null(e->topt))->v);
_LLEF:;_LLF0: {const char*_tmp927;void*_tmp926;(_tmp926=0,((int(*)(struct
_dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmp927="need_null_check",
_tag_dyneither(_tmp927,sizeof(char),16))),_tag_dyneither(_tmp926,sizeof(void*),0)));}
_LLE4:;}static struct Cyc_List_List*Cyc_Toc_get_relns(struct Cyc_Absyn_Exp*e);
static struct Cyc_List_List*Cyc_Toc_get_relns(struct Cyc_Absyn_Exp*e){void*_tmp271=(
void*)e->annot;struct Cyc_List_List*_tmp272;struct Cyc_List_List*_tmp273;_LLF2: if(*((
void**)_tmp271)!= Cyc_CfFlowInfo_UnknownZ)goto _LLF4;_tmp272=((struct Cyc_CfFlowInfo_UnknownZ_struct*)
_tmp271)->f1;_LLF3: return _tmp272;_LLF4: if(*((void**)_tmp271)!= Cyc_CfFlowInfo_NotZero)
goto _LLF6;_tmp273=((struct Cyc_CfFlowInfo_NotZero_struct*)_tmp271)->f1;_LLF5:
return _tmp273;_LLF6: if(_tmp271 != Cyc_CfFlowInfo_IsZero)goto _LLF8;_LLF7:{const
char*_tmp92A;void*_tmp929;(_tmp929=0,Cyc_Tcutil_terr(e->loc,((_tmp92A="dereference of NULL pointer",
_tag_dyneither(_tmp92A,sizeof(char),28))),_tag_dyneither(_tmp929,sizeof(void*),0)));}
return 0;_LLF8: if(*((void**)_tmp271)!= Cyc_CfFlowInfo_HasTagCmps)goto _LLFA;_LLF9:
goto _LLFB;_LLFA: if(_tmp271 != Cyc_Absyn_EmptyAnnot)goto _LLFC;_LLFB: return 0;_LLFC:;
_LLFD: {const char*_tmp92D;void*_tmp92C;(_tmp92C=0,((int(*)(struct _dyneither_ptr
fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmp92D="get_relns",
_tag_dyneither(_tmp92D,sizeof(char),10))),_tag_dyneither(_tmp92C,sizeof(void*),0)));}
_LLF1:;}static int Cyc_Toc_check_const_array(unsigned int i,void*t);static int Cyc_Toc_check_const_array(
unsigned int i,void*t){void*_tmp278=Cyc_Tcutil_compress(t);struct Cyc_Absyn_PtrInfo
_tmp279;struct Cyc_Absyn_PtrAtts _tmp27A;struct Cyc_Absyn_Conref*_tmp27B;struct Cyc_Absyn_Conref*
_tmp27C;struct Cyc_Absyn_ArrayInfo _tmp27D;struct Cyc_Absyn_Exp*_tmp27E;_LLFF: if(
_tmp278 <= (void*)4)goto _LL103;if(*((int*)_tmp278)!= 4)goto _LL101;_tmp279=((
struct Cyc_Absyn_PointerType_struct*)_tmp278)->f1;_tmp27A=_tmp279.ptr_atts;
_tmp27B=_tmp27A.bounds;_tmp27C=_tmp27A.zero_term;_LL100: {void*_tmp27F=Cyc_Absyn_conref_def(
Cyc_Absyn_bounds_one,_tmp27B);struct Cyc_Absyn_Exp*_tmp280;_LL106: if((int)_tmp27F
!= 0)goto _LL108;_LL107: return 0;_LL108: if(_tmp27F <= (void*)1)goto _LL105;if(*((int*)
_tmp27F)!= 0)goto _LL105;_tmp280=((struct Cyc_Absyn_Upper_b_struct*)_tmp27F)->f1;
_LL109: {unsigned int _tmp282;int _tmp283;struct _tuple5 _tmp281=Cyc_Evexp_eval_const_uint_exp(
_tmp280);_tmp282=_tmp281.f1;_tmp283=_tmp281.f2;return _tmp283  && i <= _tmp282;}
_LL105:;}_LL101: if(*((int*)_tmp278)!= 7)goto _LL103;_tmp27D=((struct Cyc_Absyn_ArrayType_struct*)
_tmp278)->f1;_tmp27E=_tmp27D.num_elts;_LL102: if(_tmp27E == 0)return 0;{
unsigned int _tmp285;int _tmp286;struct _tuple5 _tmp284=Cyc_Evexp_eval_const_uint_exp((
struct Cyc_Absyn_Exp*)_tmp27E);_tmp285=_tmp284.f1;_tmp286=_tmp284.f2;return
_tmp286  && i <= _tmp285;}_LL103:;_LL104: return 0;_LLFE:;}static int Cyc_Toc_check_leq_size_var(
struct Cyc_List_List*relns,struct Cyc_Absyn_Vardecl*v,struct Cyc_Absyn_Vardecl*y);
static int Cyc_Toc_check_leq_size_var(struct Cyc_List_List*relns,struct Cyc_Absyn_Vardecl*
v,struct Cyc_Absyn_Vardecl*y){for(0;relns != 0;relns=relns->tl){struct Cyc_CfFlowInfo_Reln*
_tmp287=(struct Cyc_CfFlowInfo_Reln*)relns->hd;if(_tmp287->vd != y)continue;{union
Cyc_CfFlowInfo_RelnOp_union _tmp288=_tmp287->rop;struct Cyc_Absyn_Vardecl*_tmp289;
struct Cyc_Absyn_Vardecl*_tmp28A;_LL10B: if((_tmp288.LessNumelts).tag != 2)goto
_LL10D;_tmp289=(_tmp288.LessNumelts).f1;_LL10C: _tmp28A=_tmp289;goto _LL10E;_LL10D:
if((_tmp288.LessEqNumelts).tag != 4)goto _LL10F;_tmp28A=(_tmp288.LessEqNumelts).f1;
_LL10E: if(_tmp28A == v)return 1;else{goto _LL10A;}_LL10F:;_LL110: continue;_LL10A:;}}
return 0;}static int Cyc_Toc_check_leq_size(struct Cyc_List_List*relns,struct Cyc_Absyn_Vardecl*
v,struct Cyc_Absyn_Exp*e);static int Cyc_Toc_check_leq_size(struct Cyc_List_List*
relns,struct Cyc_Absyn_Vardecl*v,struct Cyc_Absyn_Exp*e){{void*_tmp28B=(void*)e->r;
void*_tmp28C;struct Cyc_Absyn_Vardecl*_tmp28D;void*_tmp28E;struct Cyc_Absyn_Vardecl*
_tmp28F;void*_tmp290;struct Cyc_Absyn_Vardecl*_tmp291;void*_tmp292;struct Cyc_Absyn_Vardecl*
_tmp293;void*_tmp294;struct Cyc_List_List*_tmp295;struct Cyc_List_List _tmp296;
struct Cyc_Absyn_Exp*_tmp297;_LL112: if(*((int*)_tmp28B)!= 1)goto _LL114;_tmp28C=(
void*)((struct Cyc_Absyn_Var_e_struct*)_tmp28B)->f2;if(_tmp28C <= (void*)1)goto
_LL114;if(*((int*)_tmp28C)!= 4)goto _LL114;_tmp28D=((struct Cyc_Absyn_Pat_b_struct*)
_tmp28C)->f1;_LL113: _tmp28F=_tmp28D;goto _LL115;_LL114: if(*((int*)_tmp28B)!= 1)
goto _LL116;_tmp28E=(void*)((struct Cyc_Absyn_Var_e_struct*)_tmp28B)->f2;if(
_tmp28E <= (void*)1)goto _LL116;if(*((int*)_tmp28E)!= 3)goto _LL116;_tmp28F=((
struct Cyc_Absyn_Local_b_struct*)_tmp28E)->f1;_LL115: _tmp291=_tmp28F;goto _LL117;
_LL116: if(*((int*)_tmp28B)!= 1)goto _LL118;_tmp290=(void*)((struct Cyc_Absyn_Var_e_struct*)
_tmp28B)->f2;if(_tmp290 <= (void*)1)goto _LL118;if(*((int*)_tmp290)!= 0)goto _LL118;
_tmp291=((struct Cyc_Absyn_Global_b_struct*)_tmp290)->f1;_LL117: _tmp293=_tmp291;
goto _LL119;_LL118: if(*((int*)_tmp28B)!= 1)goto _LL11A;_tmp292=(void*)((struct Cyc_Absyn_Var_e_struct*)
_tmp28B)->f2;if(_tmp292 <= (void*)1)goto _LL11A;if(*((int*)_tmp292)!= 2)goto _LL11A;
_tmp293=((struct Cyc_Absyn_Param_b_struct*)_tmp292)->f1;_LL119: if(_tmp293->escapes)
return 0;if(Cyc_Toc_check_leq_size_var(relns,v,_tmp293))return 1;goto _LL111;_LL11A:
if(*((int*)_tmp28B)!= 3)goto _LL11C;_tmp294=(void*)((struct Cyc_Absyn_Primop_e_struct*)
_tmp28B)->f1;if((int)_tmp294 != 19)goto _LL11C;_tmp295=((struct Cyc_Absyn_Primop_e_struct*)
_tmp28B)->f2;if(_tmp295 == 0)goto _LL11C;_tmp296=*_tmp295;_tmp297=(struct Cyc_Absyn_Exp*)
_tmp296.hd;_LL11B:{void*_tmp298=(void*)_tmp297->r;void*_tmp299;struct Cyc_Absyn_Vardecl*
_tmp29A;void*_tmp29B;struct Cyc_Absyn_Vardecl*_tmp29C;void*_tmp29D;struct Cyc_Absyn_Vardecl*
_tmp29E;void*_tmp29F;struct Cyc_Absyn_Vardecl*_tmp2A0;_LL11F: if(*((int*)_tmp298)
!= 1)goto _LL121;_tmp299=(void*)((struct Cyc_Absyn_Var_e_struct*)_tmp298)->f2;if(
_tmp299 <= (void*)1)goto _LL121;if(*((int*)_tmp299)!= 4)goto _LL121;_tmp29A=((
struct Cyc_Absyn_Pat_b_struct*)_tmp299)->f1;_LL120: _tmp29C=_tmp29A;goto _LL122;
_LL121: if(*((int*)_tmp298)!= 1)goto _LL123;_tmp29B=(void*)((struct Cyc_Absyn_Var_e_struct*)
_tmp298)->f2;if(_tmp29B <= (void*)1)goto _LL123;if(*((int*)_tmp29B)!= 3)goto _LL123;
_tmp29C=((struct Cyc_Absyn_Local_b_struct*)_tmp29B)->f1;_LL122: _tmp29E=_tmp29C;
goto _LL124;_LL123: if(*((int*)_tmp298)!= 1)goto _LL125;_tmp29D=(void*)((struct Cyc_Absyn_Var_e_struct*)
_tmp298)->f2;if(_tmp29D <= (void*)1)goto _LL125;if(*((int*)_tmp29D)!= 0)goto _LL125;
_tmp29E=((struct Cyc_Absyn_Global_b_struct*)_tmp29D)->f1;_LL124: _tmp2A0=_tmp29E;
goto _LL126;_LL125: if(*((int*)_tmp298)!= 1)goto _LL127;_tmp29F=(void*)((struct Cyc_Absyn_Var_e_struct*)
_tmp298)->f2;if(_tmp29F <= (void*)1)goto _LL127;if(*((int*)_tmp29F)!= 2)goto _LL127;
_tmp2A0=((struct Cyc_Absyn_Param_b_struct*)_tmp29F)->f1;_LL126: return _tmp2A0 == v;
_LL127:;_LL128: goto _LL11E;_LL11E:;}goto _LL111;_LL11C:;_LL11D: goto _LL111;_LL111:;}
return 0;}static int Cyc_Toc_check_bounds(struct Cyc_List_List*relns,struct Cyc_Absyn_Exp*
a,struct Cyc_Absyn_Exp*i);static int Cyc_Toc_check_bounds(struct Cyc_List_List*relns,
struct Cyc_Absyn_Exp*a,struct Cyc_Absyn_Exp*i){{void*_tmp2A1=(void*)a->r;void*
_tmp2A2;struct Cyc_Absyn_Vardecl*_tmp2A3;void*_tmp2A4;struct Cyc_Absyn_Vardecl*
_tmp2A5;void*_tmp2A6;struct Cyc_Absyn_Vardecl*_tmp2A7;void*_tmp2A8;struct Cyc_Absyn_Vardecl*
_tmp2A9;_LL12A: if(*((int*)_tmp2A1)!= 1)goto _LL12C;_tmp2A2=(void*)((struct Cyc_Absyn_Var_e_struct*)
_tmp2A1)->f2;if(_tmp2A2 <= (void*)1)goto _LL12C;if(*((int*)_tmp2A2)!= 4)goto _LL12C;
_tmp2A3=((struct Cyc_Absyn_Pat_b_struct*)_tmp2A2)->f1;_LL12B: _tmp2A5=_tmp2A3;goto
_LL12D;_LL12C: if(*((int*)_tmp2A1)!= 1)goto _LL12E;_tmp2A4=(void*)((struct Cyc_Absyn_Var_e_struct*)
_tmp2A1)->f2;if(_tmp2A4 <= (void*)1)goto _LL12E;if(*((int*)_tmp2A4)!= 3)goto _LL12E;
_tmp2A5=((struct Cyc_Absyn_Local_b_struct*)_tmp2A4)->f1;_LL12D: _tmp2A7=_tmp2A5;
goto _LL12F;_LL12E: if(*((int*)_tmp2A1)!= 1)goto _LL130;_tmp2A6=(void*)((struct Cyc_Absyn_Var_e_struct*)
_tmp2A1)->f2;if(_tmp2A6 <= (void*)1)goto _LL130;if(*((int*)_tmp2A6)!= 0)goto _LL130;
_tmp2A7=((struct Cyc_Absyn_Global_b_struct*)_tmp2A6)->f1;_LL12F: _tmp2A9=_tmp2A7;
goto _LL131;_LL130: if(*((int*)_tmp2A1)!= 1)goto _LL132;_tmp2A8=(void*)((struct Cyc_Absyn_Var_e_struct*)
_tmp2A1)->f2;if(_tmp2A8 <= (void*)1)goto _LL132;if(*((int*)_tmp2A8)!= 2)goto _LL132;
_tmp2A9=((struct Cyc_Absyn_Param_b_struct*)_tmp2A8)->f1;_LL131: if(_tmp2A9->escapes)
return 0;inner_loop: {void*_tmp2AA=(void*)i->r;void*_tmp2AB;struct Cyc_Absyn_Exp*
_tmp2AC;union Cyc_Absyn_Cnst_union _tmp2AD;void*_tmp2AE;int _tmp2AF;union Cyc_Absyn_Cnst_union
_tmp2B0;void*_tmp2B1;int _tmp2B2;union Cyc_Absyn_Cnst_union _tmp2B3;void*_tmp2B4;
int _tmp2B5;void*_tmp2B6;struct Cyc_List_List*_tmp2B7;struct Cyc_List_List _tmp2B8;
struct Cyc_Absyn_Exp*_tmp2B9;struct Cyc_List_List*_tmp2BA;struct Cyc_List_List
_tmp2BB;struct Cyc_Absyn_Exp*_tmp2BC;void*_tmp2BD;struct Cyc_Absyn_Vardecl*_tmp2BE;
void*_tmp2BF;struct Cyc_Absyn_Vardecl*_tmp2C0;void*_tmp2C1;struct Cyc_Absyn_Vardecl*
_tmp2C2;void*_tmp2C3;struct Cyc_Absyn_Vardecl*_tmp2C4;_LL135: if(*((int*)_tmp2AA)
!= 15)goto _LL137;_tmp2AB=(void*)((struct Cyc_Absyn_Cast_e_struct*)_tmp2AA)->f1;
_tmp2AC=((struct Cyc_Absyn_Cast_e_struct*)_tmp2AA)->f2;_LL136: i=_tmp2AC;goto
inner_loop;_LL137: if(*((int*)_tmp2AA)!= 0)goto _LL139;_tmp2AD=((struct Cyc_Absyn_Const_e_struct*)
_tmp2AA)->f1;if(((((struct Cyc_Absyn_Const_e_struct*)_tmp2AA)->f1).Int_c).tag != 2)
goto _LL139;_tmp2AE=(_tmp2AD.Int_c).f1;if((int)_tmp2AE != 2)goto _LL139;_tmp2AF=(
_tmp2AD.Int_c).f2;_LL138: _tmp2B2=_tmp2AF;goto _LL13A;_LL139: if(*((int*)_tmp2AA)!= 
0)goto _LL13B;_tmp2B0=((struct Cyc_Absyn_Const_e_struct*)_tmp2AA)->f1;if(((((
struct Cyc_Absyn_Const_e_struct*)_tmp2AA)->f1).Int_c).tag != 2)goto _LL13B;_tmp2B1=(
_tmp2B0.Int_c).f1;if((int)_tmp2B1 != 0)goto _LL13B;_tmp2B2=(_tmp2B0.Int_c).f2;
_LL13A: return _tmp2B2 >= 0  && Cyc_Toc_check_const_array((unsigned int)(_tmp2B2 + 1),(
void*)_tmp2A9->type);_LL13B: if(*((int*)_tmp2AA)!= 0)goto _LL13D;_tmp2B3=((struct
Cyc_Absyn_Const_e_struct*)_tmp2AA)->f1;if(((((struct Cyc_Absyn_Const_e_struct*)
_tmp2AA)->f1).Int_c).tag != 2)goto _LL13D;_tmp2B4=(_tmp2B3.Int_c).f1;if((int)
_tmp2B4 != 1)goto _LL13D;_tmp2B5=(_tmp2B3.Int_c).f2;_LL13C: return Cyc_Toc_check_const_array((
unsigned int)(_tmp2B5 + 1),(void*)_tmp2A9->type);_LL13D: if(*((int*)_tmp2AA)!= 3)
goto _LL13F;_tmp2B6=(void*)((struct Cyc_Absyn_Primop_e_struct*)_tmp2AA)->f1;if((
int)_tmp2B6 != 4)goto _LL13F;_tmp2B7=((struct Cyc_Absyn_Primop_e_struct*)_tmp2AA)->f2;
if(_tmp2B7 == 0)goto _LL13F;_tmp2B8=*_tmp2B7;_tmp2B9=(struct Cyc_Absyn_Exp*)_tmp2B8.hd;
_tmp2BA=_tmp2B8.tl;if(_tmp2BA == 0)goto _LL13F;_tmp2BB=*_tmp2BA;_tmp2BC=(struct Cyc_Absyn_Exp*)
_tmp2BB.hd;_LL13E: return Cyc_Toc_check_leq_size(relns,_tmp2A9,_tmp2BC);_LL13F: if(*((
int*)_tmp2AA)!= 1)goto _LL141;_tmp2BD=(void*)((struct Cyc_Absyn_Var_e_struct*)
_tmp2AA)->f2;if(_tmp2BD <= (void*)1)goto _LL141;if(*((int*)_tmp2BD)!= 4)goto _LL141;
_tmp2BE=((struct Cyc_Absyn_Pat_b_struct*)_tmp2BD)->f1;_LL140: _tmp2C0=_tmp2BE;goto
_LL142;_LL141: if(*((int*)_tmp2AA)!= 1)goto _LL143;_tmp2BF=(void*)((struct Cyc_Absyn_Var_e_struct*)
_tmp2AA)->f2;if(_tmp2BF <= (void*)1)goto _LL143;if(*((int*)_tmp2BF)!= 3)goto _LL143;
_tmp2C0=((struct Cyc_Absyn_Local_b_struct*)_tmp2BF)->f1;_LL142: _tmp2C2=_tmp2C0;
goto _LL144;_LL143: if(*((int*)_tmp2AA)!= 1)goto _LL145;_tmp2C1=(void*)((struct Cyc_Absyn_Var_e_struct*)
_tmp2AA)->f2;if(_tmp2C1 <= (void*)1)goto _LL145;if(*((int*)_tmp2C1)!= 0)goto _LL145;
_tmp2C2=((struct Cyc_Absyn_Global_b_struct*)_tmp2C1)->f1;_LL144: _tmp2C4=_tmp2C2;
goto _LL146;_LL145: if(*((int*)_tmp2AA)!= 1)goto _LL147;_tmp2C3=(void*)((struct Cyc_Absyn_Var_e_struct*)
_tmp2AA)->f2;if(_tmp2C3 <= (void*)1)goto _LL147;if(*((int*)_tmp2C3)!= 2)goto _LL147;
_tmp2C4=((struct Cyc_Absyn_Param_b_struct*)_tmp2C3)->f1;_LL146: if(_tmp2C4->escapes)
return 0;{struct Cyc_List_List*_tmp2C5=relns;for(0;_tmp2C5 != 0;_tmp2C5=_tmp2C5->tl){
struct Cyc_CfFlowInfo_Reln*_tmp2C6=(struct Cyc_CfFlowInfo_Reln*)_tmp2C5->hd;if(
_tmp2C6->vd == _tmp2C4){union Cyc_CfFlowInfo_RelnOp_union _tmp2C7=_tmp2C6->rop;
struct Cyc_Absyn_Vardecl*_tmp2C8;struct Cyc_Absyn_Vardecl*_tmp2C9;void*_tmp2CA;
unsigned int _tmp2CB;_LL14A: if((_tmp2C7.LessNumelts).tag != 2)goto _LL14C;_tmp2C8=(
_tmp2C7.LessNumelts).f1;_LL14B: if(_tmp2A9 == _tmp2C8)return 1;else{goto _LL149;}
_LL14C: if((_tmp2C7.LessVar).tag != 1)goto _LL14E;_tmp2C9=(_tmp2C7.LessVar).f1;
_tmp2CA=(_tmp2C7.LessVar).f2;_LL14D:{struct _tuple0 _tmp92E;struct _tuple0 _tmp2CD=(
_tmp92E.f1=Cyc_Tcutil_compress(_tmp2CA),((_tmp92E.f2=Cyc_Tcutil_compress((void*)((
struct Cyc_Core_Opt*)_check_null(a->topt))->v),_tmp92E)));void*_tmp2CE;void*
_tmp2CF;void*_tmp2D0;struct Cyc_Absyn_PtrInfo _tmp2D1;struct Cyc_Absyn_PtrAtts
_tmp2D2;struct Cyc_Absyn_Conref*_tmp2D3;_LL153: _tmp2CE=_tmp2CD.f1;if(_tmp2CE <= (
void*)4)goto _LL155;if(*((int*)_tmp2CE)!= 18)goto _LL155;_tmp2CF=(void*)((struct
Cyc_Absyn_TagType_struct*)_tmp2CE)->f1;_tmp2D0=_tmp2CD.f2;if(_tmp2D0 <= (void*)4)
goto _LL155;if(*((int*)_tmp2D0)!= 4)goto _LL155;_tmp2D1=((struct Cyc_Absyn_PointerType_struct*)
_tmp2D0)->f1;_tmp2D2=_tmp2D1.ptr_atts;_tmp2D3=_tmp2D2.bounds;_LL154:{void*
_tmp2D4=Cyc_Absyn_conref_val(_tmp2D3);struct Cyc_Absyn_Exp*_tmp2D5;_LL158: if(
_tmp2D4 <= (void*)1)goto _LL15A;if(*((int*)_tmp2D4)!= 0)goto _LL15A;_tmp2D5=((
struct Cyc_Absyn_Upper_b_struct*)_tmp2D4)->f1;_LL159: {struct Cyc_Absyn_Exp*
_tmp2D6=Cyc_Absyn_cast_exp(Cyc_Absyn_uint_typ,Cyc_Absyn_valueof_exp(_tmp2CF,0),0,(
void*)1,0);if(Cyc_Evexp_lte_const_exp(_tmp2D6,_tmp2D5))return 1;goto _LL157;}
_LL15A:;_LL15B: goto _LL157;_LL157:;}goto _LL152;_LL155:;_LL156: goto _LL152;_LL152:;}{
struct Cyc_List_List*_tmp2D7=relns;for(0;_tmp2D7 != 0;_tmp2D7=_tmp2D7->tl){struct
Cyc_CfFlowInfo_Reln*_tmp2D8=(struct Cyc_CfFlowInfo_Reln*)_tmp2D7->hd;if(_tmp2D8->vd
== _tmp2C9){union Cyc_CfFlowInfo_RelnOp_union _tmp2D9=_tmp2D8->rop;struct Cyc_Absyn_Vardecl*
_tmp2DA;struct Cyc_Absyn_Vardecl*_tmp2DB;unsigned int _tmp2DC;struct Cyc_Absyn_Vardecl*
_tmp2DD;_LL15D: if((_tmp2D9.LessEqNumelts).tag != 4)goto _LL15F;_tmp2DA=(_tmp2D9.LessEqNumelts).f1;
_LL15E: _tmp2DB=_tmp2DA;goto _LL160;_LL15F: if((_tmp2D9.LessNumelts).tag != 2)goto
_LL161;_tmp2DB=(_tmp2D9.LessNumelts).f1;_LL160: if(_tmp2A9 == _tmp2DB)return 1;goto
_LL15C;_LL161: if((_tmp2D9.EqualConst).tag != 0)goto _LL163;_tmp2DC=(_tmp2D9.EqualConst).f1;
_LL162: return Cyc_Toc_check_const_array(_tmp2DC,(void*)_tmp2A9->type);_LL163: if((
_tmp2D9.LessVar).tag != 1)goto _LL165;_tmp2DD=(_tmp2D9.LessVar).f1;_LL164: if(Cyc_Toc_check_leq_size_var(
relns,_tmp2A9,_tmp2DD))return 1;goto _LL15C;_LL165:;_LL166: goto _LL15C;_LL15C:;}}}
goto _LL149;_LL14E: if((_tmp2C7.LessConst).tag != 3)goto _LL150;_tmp2CB=(_tmp2C7.LessConst).f1;
_LL14F: return Cyc_Toc_check_const_array(_tmp2CB,(void*)_tmp2A9->type);_LL150:;
_LL151: goto _LL149;_LL149:;}}}goto _LL134;_LL147:;_LL148: goto _LL134;_LL134:;}goto
_LL129;_LL132:;_LL133: goto _LL129;_LL129:;}return 0;}static void*Cyc_Toc_get_c_typ(
struct Cyc_Absyn_Exp*e);static void*Cyc_Toc_get_c_typ(struct Cyc_Absyn_Exp*e){if(e->topt
== 0){const char*_tmp931;void*_tmp930;(_tmp930=0,((int(*)(struct _dyneither_ptr fmt,
struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmp931="Missing type in primop ",
_tag_dyneither(_tmp931,sizeof(char),24))),_tag_dyneither(_tmp930,sizeof(void*),0)));}
return Cyc_Toc_typ_to_c((void*)((struct Cyc_Core_Opt*)_check_null(e->topt))->v);}
static void*Cyc_Toc_get_cyc_typ(struct Cyc_Absyn_Exp*e);static void*Cyc_Toc_get_cyc_typ(
struct Cyc_Absyn_Exp*e){if(e->topt == 0){const char*_tmp934;void*_tmp933;(_tmp933=0,((
int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((
_tmp934="Missing type in primop ",_tag_dyneither(_tmp934,sizeof(char),24))),
_tag_dyneither(_tmp933,sizeof(void*),0)));}return(void*)((struct Cyc_Core_Opt*)
_check_null(e->topt))->v;}static struct _tuple4*Cyc_Toc_tup_to_c(struct Cyc_Absyn_Exp*
e);static struct _tuple4*Cyc_Toc_tup_to_c(struct Cyc_Absyn_Exp*e){struct _tuple4*
_tmp935;return(_tmp935=_cycalloc(sizeof(*_tmp935)),((_tmp935->f1=Cyc_Toc_mt_tq,((
_tmp935->f2=Cyc_Toc_typ_to_c((void*)((struct Cyc_Core_Opt*)_check_null(e->topt))->v),
_tmp935)))));}static struct _tuple8*Cyc_Toc_add_designator(struct Cyc_Toc_Env*nv,
struct Cyc_Absyn_Exp*e);static struct _tuple8*Cyc_Toc_add_designator(struct Cyc_Toc_Env*
nv,struct Cyc_Absyn_Exp*e){Cyc_Toc_exp_to_c(nv,e);{struct _tuple8*_tmp936;return(
_tmp936=_cycalloc(sizeof(*_tmp936)),((_tmp936->f1=0,((_tmp936->f2=e,_tmp936)))));}}
static struct Cyc_Absyn_Exp*Cyc_Toc_make_struct(struct Cyc_Toc_Env*nv,struct _tuple1*
x,void*struct_typ,struct Cyc_Absyn_Stmt*s,int pointer,struct Cyc_Absyn_Exp*rgnopt,
int is_atomic);static struct Cyc_Absyn_Exp*Cyc_Toc_make_struct(struct Cyc_Toc_Env*nv,
struct _tuple1*x,void*struct_typ,struct Cyc_Absyn_Stmt*s,int pointer,struct Cyc_Absyn_Exp*
rgnopt,int is_atomic){struct Cyc_Absyn_Exp*eo;void*t;if(pointer){t=Cyc_Absyn_cstar_typ(
struct_typ,Cyc_Toc_mt_tq);{struct Cyc_Absyn_Exp*_tmp2E4=Cyc_Absyn_sizeofexp_exp(
Cyc_Absyn_deref_exp(Cyc_Absyn_var_exp(x,0),0),0);if(rgnopt == 0  || Cyc_Absyn_no_regions)
eo=(struct Cyc_Absyn_Exp*)(is_atomic?Cyc_Toc_malloc_atomic(_tmp2E4): Cyc_Toc_malloc_ptr(
_tmp2E4));else{struct Cyc_Absyn_Exp*r=(struct Cyc_Absyn_Exp*)rgnopt;Cyc_Toc_exp_to_c(
nv,r);eo=(struct Cyc_Absyn_Exp*)Cyc_Toc_rmalloc_exp(r,_tmp2E4);}}}else{t=
struct_typ;eo=0;}return Cyc_Absyn_stmt_exp(Cyc_Absyn_declare_stmt(x,t,eo,s,0),0);}
static struct Cyc_Absyn_Stmt*Cyc_Toc_init_comprehension(struct Cyc_Toc_Env*nv,
struct Cyc_Absyn_Exp*lhs,struct Cyc_Absyn_Vardecl*vd,struct Cyc_Absyn_Exp*e1,struct
Cyc_Absyn_Exp*e2,int zero_term,struct Cyc_Absyn_Stmt*s,int e1_already_translated);
static struct Cyc_Absyn_Stmt*Cyc_Toc_init_anon_struct(struct Cyc_Toc_Env*nv,struct
Cyc_Absyn_Exp*lhs,void*struct_type,struct Cyc_List_List*dles,struct Cyc_Absyn_Stmt*
s);static struct Cyc_Absyn_Stmt*Cyc_Toc_init_array(struct Cyc_Toc_Env*nv,struct Cyc_Absyn_Exp*
lhs,struct Cyc_List_List*dles0,struct Cyc_Absyn_Stmt*s);static struct Cyc_Absyn_Stmt*
Cyc_Toc_init_array(struct Cyc_Toc_Env*nv,struct Cyc_Absyn_Exp*lhs,struct Cyc_List_List*
dles0,struct Cyc_Absyn_Stmt*s){int count=((int(*)(struct Cyc_List_List*x))Cyc_List_length)(
dles0)- 1;{struct Cyc_List_List*_tmp2E5=((struct Cyc_List_List*(*)(struct Cyc_List_List*
x))Cyc_List_rev)(dles0);for(0;_tmp2E5 != 0;_tmp2E5=_tmp2E5->tl){struct _tuple8
_tmp2E7;struct Cyc_List_List*_tmp2E8;struct Cyc_Absyn_Exp*_tmp2E9;struct _tuple8*
_tmp2E6=(struct _tuple8*)_tmp2E5->hd;_tmp2E7=*_tmp2E6;_tmp2E8=_tmp2E7.f1;_tmp2E9=
_tmp2E7.f2;{struct Cyc_Absyn_Exp*e_index;if(_tmp2E8 == 0)e_index=Cyc_Absyn_signed_int_exp(
count --,0);else{if(_tmp2E8->tl != 0){const char*_tmp939;void*_tmp938;(_tmp938=0,((
int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_unimp)(((_tmp939="multiple designators in array",
_tag_dyneither(_tmp939,sizeof(char),30))),_tag_dyneither(_tmp938,sizeof(void*),0)));}{
void*_tmp2EC=(void*)_tmp2E8->hd;void*_tmp2ED=_tmp2EC;struct Cyc_Absyn_Exp*_tmp2EE;
_LL168: if(*((int*)_tmp2ED)!= 0)goto _LL16A;_tmp2EE=((struct Cyc_Absyn_ArrayElement_struct*)
_tmp2ED)->f1;_LL169: Cyc_Toc_exp_to_c(nv,_tmp2EE);e_index=_tmp2EE;goto _LL167;
_LL16A: if(*((int*)_tmp2ED)!= 1)goto _LL167;_LL16B: {const char*_tmp93C;void*
_tmp93B;(_tmp93B=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_unimp)(((
_tmp93C="field name designators in array",_tag_dyneither(_tmp93C,sizeof(char),32))),
_tag_dyneither(_tmp93B,sizeof(void*),0)));}_LL167:;}}{struct Cyc_Absyn_Exp*lval=
Cyc_Absyn_subscript_exp(lhs,e_index,0);void*_tmp2F1=(void*)_tmp2E9->r;struct Cyc_List_List*
_tmp2F2;struct Cyc_Absyn_Vardecl*_tmp2F3;struct Cyc_Absyn_Exp*_tmp2F4;struct Cyc_Absyn_Exp*
_tmp2F5;int _tmp2F6;void*_tmp2F7;struct Cyc_List_List*_tmp2F8;_LL16D: if(*((int*)
_tmp2F1)!= 28)goto _LL16F;_tmp2F2=((struct Cyc_Absyn_Array_e_struct*)_tmp2F1)->f1;
_LL16E: s=Cyc_Toc_init_array(nv,lval,_tmp2F2,s);goto _LL16C;_LL16F: if(*((int*)
_tmp2F1)!= 29)goto _LL171;_tmp2F3=((struct Cyc_Absyn_Comprehension_e_struct*)
_tmp2F1)->f1;_tmp2F4=((struct Cyc_Absyn_Comprehension_e_struct*)_tmp2F1)->f2;
_tmp2F5=((struct Cyc_Absyn_Comprehension_e_struct*)_tmp2F1)->f3;_tmp2F6=((struct
Cyc_Absyn_Comprehension_e_struct*)_tmp2F1)->f4;_LL170: s=Cyc_Toc_init_comprehension(
nv,lval,_tmp2F3,_tmp2F4,_tmp2F5,_tmp2F6,s,0);goto _LL16C;_LL171: if(*((int*)
_tmp2F1)!= 31)goto _LL173;_tmp2F7=(void*)((struct Cyc_Absyn_AnonStruct_e_struct*)
_tmp2F1)->f1;_tmp2F8=((struct Cyc_Absyn_AnonStruct_e_struct*)_tmp2F1)->f2;_LL172:
s=Cyc_Toc_init_anon_struct(nv,lval,_tmp2F7,_tmp2F8,s);goto _LL16C;_LL173:;_LL174:
Cyc_Toc_exp_to_c(nv,_tmp2E9);s=Cyc_Absyn_seq_stmt(Cyc_Absyn_assign_stmt(Cyc_Absyn_subscript_exp(
lhs,e_index,0),_tmp2E9,0),s,0);goto _LL16C;_LL16C:;}}}}return s;}static struct Cyc_Absyn_Stmt*
Cyc_Toc_init_comprehension(struct Cyc_Toc_Env*nv,struct Cyc_Absyn_Exp*lhs,struct
Cyc_Absyn_Vardecl*vd,struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,int zero_term,
struct Cyc_Absyn_Stmt*s,int e1_already_translated);static struct Cyc_Absyn_Stmt*Cyc_Toc_init_comprehension(
struct Cyc_Toc_Env*nv,struct Cyc_Absyn_Exp*lhs,struct Cyc_Absyn_Vardecl*vd,struct
Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,int zero_term,struct Cyc_Absyn_Stmt*s,int
e1_already_translated){struct _tuple1*_tmp2F9=vd->name;void*_tmp2FA=Cyc_Toc_typ_to_c((
void*)((struct Cyc_Core_Opt*)_check_null(e2->topt))->v);if(!e1_already_translated)
Cyc_Toc_exp_to_c(nv,e1);{struct _RegionHandle _tmp2FB=_new_region("r2");struct
_RegionHandle*r2=& _tmp2FB;_push_region(r2);{struct Cyc_Absyn_Local_b_struct
_tmp93F;struct Cyc_Absyn_Local_b_struct*_tmp93E;struct Cyc_Toc_Env*nv2=Cyc_Toc_add_varmap(
r2,nv,_tmp2F9,Cyc_Absyn_varb_exp(_tmp2F9,(void*)((_tmp93E=_cycalloc(sizeof(*
_tmp93E)),((_tmp93E[0]=((_tmp93F.tag=3,((_tmp93F.f1=vd,_tmp93F)))),_tmp93E)))),0));
struct _tuple1*max=Cyc_Toc_temp_var();struct Cyc_Absyn_Exp*ea=Cyc_Absyn_assign_exp(
Cyc_Absyn_var_exp(_tmp2F9,0),Cyc_Absyn_signed_int_exp(0,0),0);struct Cyc_Absyn_Exp*
eb=Cyc_Absyn_lt_exp(Cyc_Absyn_var_exp(_tmp2F9,0),Cyc_Absyn_var_exp(max,0),0);
struct Cyc_Absyn_Exp*ec=Cyc_Absyn_post_inc_exp(Cyc_Absyn_var_exp(_tmp2F9,0),0);
struct Cyc_Absyn_Exp*lval=Cyc_Absyn_subscript_exp(lhs,Cyc_Absyn_var_exp(_tmp2F9,0),
0);struct Cyc_Absyn_Stmt*body;{void*_tmp2FC=(void*)e2->r;struct Cyc_List_List*
_tmp2FD;struct Cyc_Absyn_Vardecl*_tmp2FE;struct Cyc_Absyn_Exp*_tmp2FF;struct Cyc_Absyn_Exp*
_tmp300;int _tmp301;void*_tmp302;struct Cyc_List_List*_tmp303;_LL176: if(*((int*)
_tmp2FC)!= 28)goto _LL178;_tmp2FD=((struct Cyc_Absyn_Array_e_struct*)_tmp2FC)->f1;
_LL177: body=Cyc_Toc_init_array(nv2,lval,_tmp2FD,Cyc_Toc_skip_stmt_dl());goto
_LL175;_LL178: if(*((int*)_tmp2FC)!= 29)goto _LL17A;_tmp2FE=((struct Cyc_Absyn_Comprehension_e_struct*)
_tmp2FC)->f1;_tmp2FF=((struct Cyc_Absyn_Comprehension_e_struct*)_tmp2FC)->f2;
_tmp300=((struct Cyc_Absyn_Comprehension_e_struct*)_tmp2FC)->f3;_tmp301=((struct
Cyc_Absyn_Comprehension_e_struct*)_tmp2FC)->f4;_LL179: body=Cyc_Toc_init_comprehension(
nv2,lval,_tmp2FE,_tmp2FF,_tmp300,_tmp301,Cyc_Toc_skip_stmt_dl(),0);goto _LL175;
_LL17A: if(*((int*)_tmp2FC)!= 31)goto _LL17C;_tmp302=(void*)((struct Cyc_Absyn_AnonStruct_e_struct*)
_tmp2FC)->f1;_tmp303=((struct Cyc_Absyn_AnonStruct_e_struct*)_tmp2FC)->f2;_LL17B:
body=Cyc_Toc_init_anon_struct(nv,lval,_tmp302,_tmp303,Cyc_Toc_skip_stmt_dl());
goto _LL175;_LL17C:;_LL17D: Cyc_Toc_exp_to_c(nv2,e2);body=Cyc_Absyn_assign_stmt(
lval,e2,0);goto _LL175;_LL175:;}{struct Cyc_Absyn_Stmt*s2=Cyc_Absyn_for_stmt(ea,eb,
ec,body,0);if(zero_term){struct Cyc_Absyn_Exp*ex=Cyc_Absyn_assign_exp(Cyc_Absyn_subscript_exp(
Cyc_Absyn_new_exp((void*)lhs->r,0),Cyc_Absyn_var_exp(max,0),0),Cyc_Toc_cast_it(
_tmp2FA,Cyc_Absyn_uint_exp(0,0)),0);s2=Cyc_Absyn_seq_stmt(s2,Cyc_Absyn_exp_stmt(
ex,0),0);}{struct Cyc_Absyn_Stmt*_tmp304=Cyc_Absyn_seq_stmt(Cyc_Absyn_declare_stmt(
max,Cyc_Absyn_uint_typ,(struct Cyc_Absyn_Exp*)e1,Cyc_Absyn_declare_stmt(_tmp2F9,
Cyc_Absyn_uint_typ,0,s2,0),0),s,0);_npop_handler(0);return _tmp304;}}};
_pop_region(r2);}}static struct Cyc_Absyn_Stmt*Cyc_Toc_init_anon_struct(struct Cyc_Toc_Env*
nv,struct Cyc_Absyn_Exp*lhs,void*struct_type,struct Cyc_List_List*dles,struct Cyc_Absyn_Stmt*
s);static struct Cyc_Absyn_Stmt*Cyc_Toc_init_anon_struct(struct Cyc_Toc_Env*nv,
struct Cyc_Absyn_Exp*lhs,void*struct_type,struct Cyc_List_List*dles,struct Cyc_Absyn_Stmt*
s){{struct Cyc_List_List*_tmp307=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))
Cyc_List_rev)(dles);for(0;_tmp307 != 0;_tmp307=_tmp307->tl){struct _tuple8 _tmp309;
struct Cyc_List_List*_tmp30A;struct Cyc_Absyn_Exp*_tmp30B;struct _tuple8*_tmp308=(
struct _tuple8*)_tmp307->hd;_tmp309=*_tmp308;_tmp30A=_tmp309.f1;_tmp30B=_tmp309.f2;
if(_tmp30A == 0){const char*_tmp942;void*_tmp941;(_tmp941=0,((int(*)(struct
_dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmp942="empty designator list",
_tag_dyneither(_tmp942,sizeof(char),22))),_tag_dyneither(_tmp941,sizeof(void*),0)));}
if(_tmp30A->tl != 0){const char*_tmp945;void*_tmp944;(_tmp944=0,((int(*)(struct
_dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmp945="too many designators in anonymous struct",
_tag_dyneither(_tmp945,sizeof(char),41))),_tag_dyneither(_tmp944,sizeof(void*),0)));}{
void*_tmp310=(void*)_tmp30A->hd;struct _dyneither_ptr*_tmp311;_LL17F: if(*((int*)
_tmp310)!= 1)goto _LL181;_tmp311=((struct Cyc_Absyn_FieldName_struct*)_tmp310)->f1;
_LL180: {struct Cyc_Absyn_Exp*lval=Cyc_Absyn_aggrmember_exp(lhs,_tmp311,0);{void*
_tmp312=(void*)_tmp30B->r;struct Cyc_List_List*_tmp313;struct Cyc_Absyn_Vardecl*
_tmp314;struct Cyc_Absyn_Exp*_tmp315;struct Cyc_Absyn_Exp*_tmp316;int _tmp317;void*
_tmp318;struct Cyc_List_List*_tmp319;_LL184: if(*((int*)_tmp312)!= 28)goto _LL186;
_tmp313=((struct Cyc_Absyn_Array_e_struct*)_tmp312)->f1;_LL185: s=Cyc_Toc_init_array(
nv,lval,_tmp313,s);goto _LL183;_LL186: if(*((int*)_tmp312)!= 29)goto _LL188;_tmp314=((
struct Cyc_Absyn_Comprehension_e_struct*)_tmp312)->f1;_tmp315=((struct Cyc_Absyn_Comprehension_e_struct*)
_tmp312)->f2;_tmp316=((struct Cyc_Absyn_Comprehension_e_struct*)_tmp312)->f3;
_tmp317=((struct Cyc_Absyn_Comprehension_e_struct*)_tmp312)->f4;_LL187: s=Cyc_Toc_init_comprehension(
nv,lval,_tmp314,_tmp315,_tmp316,_tmp317,s,0);goto _LL183;_LL188: if(*((int*)
_tmp312)!= 31)goto _LL18A;_tmp318=(void*)((struct Cyc_Absyn_AnonStruct_e_struct*)
_tmp312)->f1;_tmp319=((struct Cyc_Absyn_AnonStruct_e_struct*)_tmp312)->f2;_LL189:
s=Cyc_Toc_init_anon_struct(nv,lval,_tmp318,_tmp319,s);goto _LL183;_LL18A:;_LL18B:
Cyc_Toc_exp_to_c(nv,_tmp30B);if(Cyc_Toc_is_poly_field(struct_type,_tmp311))
_tmp30B=Cyc_Toc_cast_it(Cyc_Absyn_void_star_typ(),_tmp30B);s=Cyc_Absyn_seq_stmt(
Cyc_Absyn_exp_stmt(Cyc_Absyn_assign_exp(lval,_tmp30B,0),0),s,0);goto _LL183;
_LL183:;}goto _LL17E;}_LL181:;_LL182: {const char*_tmp948;void*_tmp947;(_tmp947=0,((
int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((
_tmp948="array designator in struct",_tag_dyneither(_tmp948,sizeof(char),27))),
_tag_dyneither(_tmp947,sizeof(void*),0)));}_LL17E:;}}}return s;}static struct Cyc_Absyn_Exp*
Cyc_Toc_init_tuple(struct Cyc_Toc_Env*nv,int pointer,struct Cyc_Absyn_Exp*rgnopt,
struct Cyc_List_List*es);static struct Cyc_Absyn_Exp*Cyc_Toc_init_tuple(struct Cyc_Toc_Env*
nv,int pointer,struct Cyc_Absyn_Exp*rgnopt,struct Cyc_List_List*es){struct
_RegionHandle _tmp31C=_new_region("r");struct _RegionHandle*r=& _tmp31C;
_push_region(r);{struct Cyc_List_List*_tmp31D=((struct Cyc_List_List*(*)(struct
_RegionHandle*,struct _tuple4*(*f)(struct Cyc_Absyn_Exp*),struct Cyc_List_List*x))
Cyc_List_rmap)(r,Cyc_Toc_tup_to_c,es);void*_tmp31E=Cyc_Toc_add_tuple_type(
_tmp31D);struct _tuple1*_tmp31F=Cyc_Toc_temp_var();struct Cyc_Absyn_Exp*_tmp320=
Cyc_Absyn_var_exp(_tmp31F,0);struct Cyc_Absyn_Stmt*_tmp321=Cyc_Absyn_exp_stmt(
_tmp320,0);struct Cyc_Absyn_Exp*(*_tmp322)(struct Cyc_Absyn_Exp*,struct
_dyneither_ptr*,struct Cyc_Position_Segment*)=pointer?Cyc_Absyn_aggrarrow_exp: Cyc_Absyn_aggrmember_exp;
int is_atomic=1;struct Cyc_List_List*_tmp323=((struct Cyc_List_List*(*)(struct
_RegionHandle*,struct Cyc_List_List*x))Cyc_List_rrev)(r,es);{int i=((int(*)(struct
Cyc_List_List*x))Cyc_List_length)(_tmp323);for(0;_tmp323 != 0;(_tmp323=_tmp323->tl,
-- i)){struct Cyc_Absyn_Exp*e=(struct Cyc_Absyn_Exp*)_tmp323->hd;struct Cyc_Absyn_Exp*
lval=_tmp322(_tmp320,Cyc_Absyn_fieldname(i),0);is_atomic=is_atomic  && Cyc_Toc_atomic_typ((
void*)((struct Cyc_Core_Opt*)_check_null(e->topt))->v);{void*_tmp324=(void*)e->r;
struct Cyc_List_List*_tmp325;struct Cyc_Absyn_Vardecl*_tmp326;struct Cyc_Absyn_Exp*
_tmp327;struct Cyc_Absyn_Exp*_tmp328;int _tmp329;_LL18D: if(*((int*)_tmp324)!= 28)
goto _LL18F;_tmp325=((struct Cyc_Absyn_Array_e_struct*)_tmp324)->f1;_LL18E: _tmp321=
Cyc_Toc_init_array(nv,lval,_tmp325,_tmp321);goto _LL18C;_LL18F: if(*((int*)_tmp324)
!= 29)goto _LL191;_tmp326=((struct Cyc_Absyn_Comprehension_e_struct*)_tmp324)->f1;
_tmp327=((struct Cyc_Absyn_Comprehension_e_struct*)_tmp324)->f2;_tmp328=((struct
Cyc_Absyn_Comprehension_e_struct*)_tmp324)->f3;_tmp329=((struct Cyc_Absyn_Comprehension_e_struct*)
_tmp324)->f4;_LL190: _tmp321=Cyc_Toc_init_comprehension(nv,lval,_tmp326,_tmp327,
_tmp328,_tmp329,_tmp321,0);goto _LL18C;_LL191:;_LL192: Cyc_Toc_exp_to_c(nv,e);
_tmp321=Cyc_Absyn_seq_stmt(Cyc_Absyn_exp_stmt(Cyc_Absyn_assign_exp(_tmp322(
_tmp320,Cyc_Absyn_fieldname(i),0),e,0),0),_tmp321,0);goto _LL18C;_LL18C:;}}}{
struct Cyc_Absyn_Exp*_tmp32A=Cyc_Toc_make_struct(nv,_tmp31F,_tmp31E,_tmp321,
pointer,rgnopt,is_atomic);_npop_handler(0);return _tmp32A;}};_pop_region(r);}
static struct Cyc_Absyn_Exp*Cyc_Toc_init_struct(struct Cyc_Toc_Env*nv,void*
struct_type,int has_exists,int pointer,struct Cyc_Absyn_Exp*rgnopt,struct Cyc_List_List*
dles,struct _tuple1*tdn);static struct Cyc_Absyn_Exp*Cyc_Toc_init_struct(struct Cyc_Toc_Env*
nv,void*struct_type,int has_exists,int pointer,struct Cyc_Absyn_Exp*rgnopt,struct
Cyc_List_List*dles,struct _tuple1*tdn){struct _tuple1*_tmp32B=Cyc_Toc_temp_var();
struct Cyc_Absyn_Exp*_tmp32C=Cyc_Absyn_var_exp(_tmp32B,0);struct Cyc_Absyn_Stmt*
_tmp32D=Cyc_Absyn_exp_stmt(_tmp32C,0);struct Cyc_Absyn_Exp*(*_tmp32E)(struct Cyc_Absyn_Exp*,
struct _dyneither_ptr*,struct Cyc_Position_Segment*)=pointer?Cyc_Absyn_aggrarrow_exp:
Cyc_Absyn_aggrmember_exp;void*_tmp32F=Cyc_Toc_aggrdecl_type(tdn,Cyc_Absyn_strctq);
int is_atomic=1;struct Cyc_Absyn_Aggrdecl*ad;{void*_tmp330=Cyc_Tcutil_compress(
struct_type);struct Cyc_Absyn_AggrInfo _tmp331;union Cyc_Absyn_AggrInfoU_union
_tmp332;_LL194: if(_tmp330 <= (void*)4)goto _LL196;if(*((int*)_tmp330)!= 10)goto
_LL196;_tmp331=((struct Cyc_Absyn_AggrType_struct*)_tmp330)->f1;_tmp332=_tmp331.aggr_info;
_LL195: ad=Cyc_Absyn_get_known_aggrdecl(_tmp332);goto _LL193;_LL196:;_LL197: {
const char*_tmp94B;void*_tmp94A;(_tmp94A=0,((int(*)(struct _dyneither_ptr fmt,
struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmp94B="init_struct: bad struct type",
_tag_dyneither(_tmp94B,sizeof(char),29))),_tag_dyneither(_tmp94A,sizeof(void*),0)));}
_LL193:;}{struct _RegionHandle _tmp335=_new_region("r");struct _RegionHandle*r=&
_tmp335;_push_region(r);{struct Cyc_List_List*_tmp336=((struct Cyc_List_List*(*)(
struct _RegionHandle*,struct Cyc_List_List*x))Cyc_List_rrev)(r,dles);for(0;_tmp336
!= 0;_tmp336=_tmp336->tl){struct _tuple8 _tmp338;struct Cyc_List_List*_tmp339;
struct Cyc_Absyn_Exp*_tmp33A;struct _tuple8*_tmp337=(struct _tuple8*)_tmp336->hd;
_tmp338=*_tmp337;_tmp339=_tmp338.f1;_tmp33A=_tmp338.f2;is_atomic=is_atomic  && 
Cyc_Toc_atomic_typ((void*)((struct Cyc_Core_Opt*)_check_null(_tmp33A->topt))->v);
if(_tmp339 == 0){const char*_tmp94E;void*_tmp94D;(_tmp94D=0,((int(*)(struct
_dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmp94E="empty designator list",
_tag_dyneither(_tmp94E,sizeof(char),22))),_tag_dyneither(_tmp94D,sizeof(void*),0)));}
if(_tmp339->tl != 0){struct _tuple1*_tmp33D=Cyc_Toc_temp_var();struct Cyc_Absyn_Exp*
_tmp33E=Cyc_Absyn_var_exp(_tmp33D,0);for(0;_tmp339 != 0;_tmp339=_tmp339->tl){void*
_tmp33F=(void*)_tmp339->hd;struct _dyneither_ptr*_tmp340;_LL199: if(*((int*)
_tmp33F)!= 1)goto _LL19B;_tmp340=((struct Cyc_Absyn_FieldName_struct*)_tmp33F)->f1;
_LL19A: if(Cyc_Toc_is_poly_field(struct_type,_tmp340))_tmp33E=Cyc_Toc_cast_it(Cyc_Absyn_void_star_typ(),
_tmp33E);_tmp32D=Cyc_Absyn_seq_stmt(Cyc_Absyn_exp_stmt(Cyc_Absyn_assign_exp(
_tmp32E(_tmp32C,_tmp340,0),_tmp33E,0),0),_tmp32D,0);goto _LL198;_LL19B:;_LL19C: {
const char*_tmp951;void*_tmp950;(_tmp950=0,((int(*)(struct _dyneither_ptr fmt,
struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmp951="array designator in struct",
_tag_dyneither(_tmp951,sizeof(char),27))),_tag_dyneither(_tmp950,sizeof(void*),0)));}
_LL198:;}Cyc_Toc_exp_to_c(nv,_tmp33A);_tmp32D=Cyc_Absyn_seq_stmt(Cyc_Absyn_exp_stmt(
Cyc_Absyn_assign_exp(_tmp33E,_tmp33A,0),0),_tmp32D,0);}else{void*_tmp343=(void*)
_tmp339->hd;struct _dyneither_ptr*_tmp344;_LL19E: if(*((int*)_tmp343)!= 1)goto
_LL1A0;_tmp344=((struct Cyc_Absyn_FieldName_struct*)_tmp343)->f1;_LL19F: {struct
Cyc_Absyn_Exp*lval=_tmp32E(_tmp32C,_tmp344,0);{void*_tmp345=(void*)_tmp33A->r;
struct Cyc_List_List*_tmp346;struct Cyc_Absyn_Vardecl*_tmp347;struct Cyc_Absyn_Exp*
_tmp348;struct Cyc_Absyn_Exp*_tmp349;int _tmp34A;void*_tmp34B;struct Cyc_List_List*
_tmp34C;_LL1A3: if(*((int*)_tmp345)!= 28)goto _LL1A5;_tmp346=((struct Cyc_Absyn_Array_e_struct*)
_tmp345)->f1;_LL1A4: _tmp32D=Cyc_Toc_init_array(nv,lval,_tmp346,_tmp32D);goto
_LL1A2;_LL1A5: if(*((int*)_tmp345)!= 29)goto _LL1A7;_tmp347=((struct Cyc_Absyn_Comprehension_e_struct*)
_tmp345)->f1;_tmp348=((struct Cyc_Absyn_Comprehension_e_struct*)_tmp345)->f2;
_tmp349=((struct Cyc_Absyn_Comprehension_e_struct*)_tmp345)->f3;_tmp34A=((struct
Cyc_Absyn_Comprehension_e_struct*)_tmp345)->f4;_LL1A6: _tmp32D=Cyc_Toc_init_comprehension(
nv,lval,_tmp347,_tmp348,_tmp349,_tmp34A,_tmp32D,0);goto _LL1A2;_LL1A7: if(*((int*)
_tmp345)!= 31)goto _LL1A9;_tmp34B=(void*)((struct Cyc_Absyn_AnonStruct_e_struct*)
_tmp345)->f1;_tmp34C=((struct Cyc_Absyn_AnonStruct_e_struct*)_tmp345)->f2;_LL1A8:
_tmp32D=Cyc_Toc_init_anon_struct(nv,lval,_tmp34B,_tmp34C,_tmp32D);goto _LL1A2;
_LL1A9:;_LL1AA: {int was_ptr_type=Cyc_Tcutil_is_pointer_type((void*)((struct Cyc_Core_Opt*)
_check_null(_tmp33A->topt))->v);Cyc_Toc_exp_to_c(nv,_tmp33A);{struct Cyc_Absyn_Aggrfield*
_tmp34D=Cyc_Absyn_lookup_decl_field(ad,_tmp344);if(Cyc_Toc_is_poly_field(
struct_type,_tmp344) && !was_ptr_type)_tmp33A=Cyc_Toc_cast_it(Cyc_Absyn_void_star_typ(),
_tmp33A);if(has_exists)_tmp33A=Cyc_Toc_cast_it((void*)((struct Cyc_Absyn_Aggrfield*)
_check_null(_tmp34D))->type,_tmp33A);_tmp32D=Cyc_Absyn_seq_stmt(Cyc_Absyn_exp_stmt(
Cyc_Absyn_assign_exp(lval,_tmp33A,0),0),_tmp32D,0);goto _LL1A2;}}_LL1A2:;}goto
_LL19D;}_LL1A0:;_LL1A1: {const char*_tmp954;void*_tmp953;(_tmp953=0,((int(*)(
struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmp954="array designator in struct",
_tag_dyneither(_tmp954,sizeof(char),27))),_tag_dyneither(_tmp953,sizeof(void*),0)));}
_LL19D:;}}}{struct Cyc_Absyn_Exp*_tmp350=Cyc_Toc_make_struct(nv,_tmp32B,_tmp32F,
_tmp32D,pointer,rgnopt,is_atomic);_npop_handler(0);return _tmp350;};_pop_region(r);}}
struct _tuple10{struct Cyc_Core_Opt*f1;struct Cyc_Absyn_Exp*f2;};static struct Cyc_Absyn_Exp*
Cyc_Toc_assignop_lvalue(struct Cyc_Absyn_Exp*el,struct _tuple10*pr);static struct
Cyc_Absyn_Exp*Cyc_Toc_assignop_lvalue(struct Cyc_Absyn_Exp*el,struct _tuple10*pr){
return Cyc_Absyn_assignop_exp(el,(*pr).f1,(*pr).f2,0);}static struct Cyc_Absyn_Exp*
Cyc_Toc_address_lvalue(struct Cyc_Absyn_Exp*e1,int ignore);static struct Cyc_Absyn_Exp*
Cyc_Toc_address_lvalue(struct Cyc_Absyn_Exp*e1,int ignore){return Cyc_Absyn_address_exp(
e1,0);}static struct Cyc_Absyn_Exp*Cyc_Toc_incr_lvalue(struct Cyc_Absyn_Exp*e1,void*
incr);static struct Cyc_Absyn_Exp*Cyc_Toc_incr_lvalue(struct Cyc_Absyn_Exp*e1,void*
incr){struct Cyc_Absyn_Increment_e_struct _tmp957;struct Cyc_Absyn_Increment_e_struct*
_tmp956;return Cyc_Absyn_new_exp((void*)((_tmp956=_cycalloc(sizeof(*_tmp956)),((
_tmp956[0]=((_tmp957.tag=5,((_tmp957.f1=e1,((_tmp957.f2=(void*)incr,_tmp957)))))),
_tmp956)))),0);}static void Cyc_Toc_lvalue_assign_stmt(struct Cyc_Absyn_Stmt*s,
struct Cyc_List_List*fs,struct Cyc_Absyn_Exp*(*f)(struct Cyc_Absyn_Exp*,void*),void*
f_env);static void Cyc_Toc_lvalue_assign(struct Cyc_Absyn_Exp*e1,struct Cyc_List_List*
fs,struct Cyc_Absyn_Exp*(*f)(struct Cyc_Absyn_Exp*,void*),void*f_env);static void
Cyc_Toc_lvalue_assign(struct Cyc_Absyn_Exp*e1,struct Cyc_List_List*fs,struct Cyc_Absyn_Exp*(*
f)(struct Cyc_Absyn_Exp*,void*),void*f_env){void*_tmp353=(void*)e1->r;struct Cyc_Absyn_Stmt*
_tmp354;void*_tmp355;struct Cyc_Absyn_Exp*_tmp356;struct Cyc_Absyn_Exp*_tmp357;
struct _dyneither_ptr*_tmp358;_LL1AC: if(*((int*)_tmp353)!= 38)goto _LL1AE;_tmp354=((
struct Cyc_Absyn_StmtExp_e_struct*)_tmp353)->f1;_LL1AD: Cyc_Toc_lvalue_assign_stmt(
_tmp354,fs,f,f_env);goto _LL1AB;_LL1AE: if(*((int*)_tmp353)!= 15)goto _LL1B0;
_tmp355=(void*)((struct Cyc_Absyn_Cast_e_struct*)_tmp353)->f1;_tmp356=((struct Cyc_Absyn_Cast_e_struct*)
_tmp353)->f2;_LL1AF: Cyc_Toc_lvalue_assign(_tmp356,fs,f,f_env);goto _LL1AB;_LL1B0:
if(*((int*)_tmp353)!= 23)goto _LL1B2;_tmp357=((struct Cyc_Absyn_AggrMember_e_struct*)
_tmp353)->f1;_tmp358=((struct Cyc_Absyn_AggrMember_e_struct*)_tmp353)->f2;_LL1B1:(
void*)(e1->r=(void*)((void*)_tmp357->r));{struct Cyc_List_List*_tmp958;Cyc_Toc_lvalue_assign(
e1,(struct Cyc_List_List*)((_tmp958=_cycalloc(sizeof(*_tmp958)),((_tmp958->hd=
_tmp358,((_tmp958->tl=fs,_tmp958)))))),f,f_env);}goto _LL1AB;_LL1B2:;_LL1B3: {
struct Cyc_Absyn_Exp*e1_copy=Cyc_Absyn_copy_exp(e1);for(0;fs != 0;fs=fs->tl){
e1_copy=Cyc_Absyn_aggrmember_exp(e1_copy,(struct _dyneither_ptr*)fs->hd,e1_copy->loc);}(
void*)(e1->r=(void*)((void*)(f(e1_copy,f_env))->r));goto _LL1AB;}_LL1AB:;}static
void Cyc_Toc_lvalue_assign_stmt(struct Cyc_Absyn_Stmt*s,struct Cyc_List_List*fs,
struct Cyc_Absyn_Exp*(*f)(struct Cyc_Absyn_Exp*,void*),void*f_env);static void Cyc_Toc_lvalue_assign_stmt(
struct Cyc_Absyn_Stmt*s,struct Cyc_List_List*fs,struct Cyc_Absyn_Exp*(*f)(struct Cyc_Absyn_Exp*,
void*),void*f_env){void*_tmp35A=(void*)s->r;struct Cyc_Absyn_Exp*_tmp35B;struct
Cyc_Absyn_Decl*_tmp35C;struct Cyc_Absyn_Stmt*_tmp35D;struct Cyc_Absyn_Stmt*_tmp35E;
_LL1B5: if(_tmp35A <= (void*)1)goto _LL1BB;if(*((int*)_tmp35A)!= 0)goto _LL1B7;
_tmp35B=((struct Cyc_Absyn_Exp_s_struct*)_tmp35A)->f1;_LL1B6: Cyc_Toc_lvalue_assign(
_tmp35B,fs,f,f_env);goto _LL1B4;_LL1B7: if(*((int*)_tmp35A)!= 11)goto _LL1B9;
_tmp35C=((struct Cyc_Absyn_Decl_s_struct*)_tmp35A)->f1;_tmp35D=((struct Cyc_Absyn_Decl_s_struct*)
_tmp35A)->f2;_LL1B8: Cyc_Toc_lvalue_assign_stmt(_tmp35D,fs,f,f_env);goto _LL1B4;
_LL1B9: if(*((int*)_tmp35A)!= 1)goto _LL1BB;_tmp35E=((struct Cyc_Absyn_Seq_s_struct*)
_tmp35A)->f2;_LL1BA: Cyc_Toc_lvalue_assign_stmt(_tmp35E,fs,f,f_env);goto _LL1B4;
_LL1BB:;_LL1BC: {const char*_tmp95C;void*_tmp95B[1];struct Cyc_String_pa_struct
_tmp95A;(_tmp95A.tag=0,((_tmp95A.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Absynpp_stmt2string(s)),((_tmp95B[0]=& _tmp95A,Cyc_Toc_toc_impos(((_tmp95C="lvalue_assign_stmt: %s",
_tag_dyneither(_tmp95C,sizeof(char),23))),_tag_dyneither(_tmp95B,sizeof(void*),1)))))));}
_LL1B4:;}static struct Cyc_List_List*Cyc_Toc_rmap_2c(struct _RegionHandle*r2,void*(*
f)(void*,void*),void*env,struct Cyc_List_List*x);static struct Cyc_List_List*Cyc_Toc_rmap_2c(
struct _RegionHandle*r2,void*(*f)(void*,void*),void*env,struct Cyc_List_List*x){
struct Cyc_List_List*result;struct Cyc_List_List*prev;if(x == 0)return 0;{struct Cyc_List_List*
_tmp95D;result=((_tmp95D=_region_malloc(r2,sizeof(*_tmp95D)),((_tmp95D->hd=(void*)
f((void*)x->hd,env),((_tmp95D->tl=0,_tmp95D))))));}prev=result;for(x=x->tl;x != 0;
x=x->tl){{struct Cyc_List_List*_tmp95E;((struct Cyc_List_List*)_check_null(prev))->tl=((
_tmp95E=_region_malloc(r2,sizeof(*_tmp95E)),((_tmp95E->hd=(void*)f((void*)x->hd,
env),((_tmp95E->tl=0,_tmp95E))))));}prev=((struct Cyc_List_List*)_check_null(prev))->tl;}
return result;}static struct Cyc_List_List*Cyc_Toc_map_2c(void*(*f)(void*,void*),
void*env,struct Cyc_List_List*x);static struct Cyc_List_List*Cyc_Toc_map_2c(void*(*
f)(void*,void*),void*env,struct Cyc_List_List*x){return Cyc_Toc_rmap_2c(Cyc_Core_heap_region,
f,env,x);}static struct _tuple8*Cyc_Toc_make_dle(struct Cyc_Absyn_Exp*e);static
struct _tuple8*Cyc_Toc_make_dle(struct Cyc_Absyn_Exp*e){struct _tuple8*_tmp95F;
return(_tmp95F=_cycalloc(sizeof(*_tmp95F)),((_tmp95F->f1=0,((_tmp95F->f2=e,
_tmp95F)))));}static struct Cyc_Absyn_PtrInfo Cyc_Toc_get_ptr_type(void*t);static
struct Cyc_Absyn_PtrInfo Cyc_Toc_get_ptr_type(void*t){void*_tmp365=Cyc_Tcutil_compress(
t);struct Cyc_Absyn_PtrInfo _tmp366;_LL1BE: if(_tmp365 <= (void*)4)goto _LL1C0;if(*((
int*)_tmp365)!= 4)goto _LL1C0;_tmp366=((struct Cyc_Absyn_PointerType_struct*)
_tmp365)->f1;_LL1BF: return _tmp366;_LL1C0:;_LL1C1: {const char*_tmp962;void*
_tmp961;(_tmp961=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((
_tmp962="get_ptr_typ: not a pointer!",_tag_dyneither(_tmp962,sizeof(char),28))),
_tag_dyneither(_tmp961,sizeof(void*),0)));}_LL1BD:;}static struct Cyc_Absyn_Exp*
Cyc_Toc_generate_zero(void*t);static struct Cyc_Absyn_Exp*Cyc_Toc_generate_zero(
void*t){struct Cyc_Absyn_Exp*res;{void*_tmp369=Cyc_Tcutil_compress(t);void*
_tmp36A;void*_tmp36B;void*_tmp36C;void*_tmp36D;void*_tmp36E;void*_tmp36F;void*
_tmp370;void*_tmp371;void*_tmp372;void*_tmp373;_LL1C3: if(_tmp369 <= (void*)4)goto
_LL1D3;if(*((int*)_tmp369)!= 4)goto _LL1C5;_LL1C4: res=Cyc_Absyn_null_exp(0);goto
_LL1C2;_LL1C5: if(*((int*)_tmp369)!= 5)goto _LL1C7;_tmp36A=(void*)((struct Cyc_Absyn_IntType_struct*)
_tmp369)->f1;_tmp36B=(void*)((struct Cyc_Absyn_IntType_struct*)_tmp369)->f2;if((
int)_tmp36B != 0)goto _LL1C7;_LL1C6:{union Cyc_Absyn_Cnst_union _tmp963;res=Cyc_Absyn_const_exp((
union Cyc_Absyn_Cnst_union)(((_tmp963.Char_c).tag=0,(((_tmp963.Char_c).f1=(void*)
_tmp36A,(((_tmp963.Char_c).f2='\000',_tmp963)))))),0);}goto _LL1C2;_LL1C7: if(*((
int*)_tmp369)!= 5)goto _LL1C9;_tmp36C=(void*)((struct Cyc_Absyn_IntType_struct*)
_tmp369)->f1;_tmp36D=(void*)((struct Cyc_Absyn_IntType_struct*)_tmp369)->f2;if((
int)_tmp36D != 1)goto _LL1C9;_LL1C8:{union Cyc_Absyn_Cnst_union _tmp964;res=Cyc_Absyn_const_exp((
union Cyc_Absyn_Cnst_union)(((_tmp964.Short_c).tag=1,(((_tmp964.Short_c).f1=(void*)
_tmp36C,(((_tmp964.Short_c).f2=0,_tmp964)))))),0);}goto _LL1C2;_LL1C9: if(*((int*)
_tmp369)!= 12)goto _LL1CB;_LL1CA: goto _LL1CC;_LL1CB: if(*((int*)_tmp369)!= 13)goto
_LL1CD;_LL1CC: _tmp36E=(void*)1;goto _LL1CE;_LL1CD: if(*((int*)_tmp369)!= 5)goto
_LL1CF;_tmp36E=(void*)((struct Cyc_Absyn_IntType_struct*)_tmp369)->f1;_tmp36F=(
void*)((struct Cyc_Absyn_IntType_struct*)_tmp369)->f2;if((int)_tmp36F != 2)goto
_LL1CF;_LL1CE: _tmp370=_tmp36E;goto _LL1D0;_LL1CF: if(*((int*)_tmp369)!= 5)goto
_LL1D1;_tmp370=(void*)((struct Cyc_Absyn_IntType_struct*)_tmp369)->f1;_tmp371=(
void*)((struct Cyc_Absyn_IntType_struct*)_tmp369)->f2;if((int)_tmp371 != 3)goto
_LL1D1;_LL1D0:{union Cyc_Absyn_Cnst_union _tmp965;res=Cyc_Absyn_const_exp((union
Cyc_Absyn_Cnst_union)(((_tmp965.Int_c).tag=2,(((_tmp965.Int_c).f1=(void*)_tmp370,(((
_tmp965.Int_c).f2=0,_tmp965)))))),0);}goto _LL1C2;_LL1D1: if(*((int*)_tmp369)!= 5)
goto _LL1D3;_tmp372=(void*)((struct Cyc_Absyn_IntType_struct*)_tmp369)->f1;_tmp373=(
void*)((struct Cyc_Absyn_IntType_struct*)_tmp369)->f2;if((int)_tmp373 != 4)goto
_LL1D3;_LL1D2:{union Cyc_Absyn_Cnst_union _tmp966;res=Cyc_Absyn_const_exp((union
Cyc_Absyn_Cnst_union)(((_tmp966.LongLong_c).tag=3,(((_tmp966.LongLong_c).f1=(
void*)_tmp372,(((_tmp966.LongLong_c).f2=(long long)0,_tmp966)))))),0);}goto
_LL1C2;_LL1D3: if((int)_tmp369 != 1)goto _LL1D5;_LL1D4: goto _LL1D6;_LL1D5: if(_tmp369
<= (void*)4)goto _LL1D7;if(*((int*)_tmp369)!= 6)goto _LL1D7;_LL1D6:{const char*
_tmp969;union Cyc_Absyn_Cnst_union _tmp968;res=Cyc_Absyn_const_exp((union Cyc_Absyn_Cnst_union)(((
_tmp968.Float_c).tag=4,(((_tmp968.Float_c).f1=((_tmp969="0.0",_tag_dyneither(
_tmp969,sizeof(char),4))),_tmp968)))),0);}goto _LL1C2;_LL1D7:;_LL1D8: {const char*
_tmp96D;void*_tmp96C[1];struct Cyc_String_pa_struct _tmp96B;(_tmp96B.tag=0,((
_tmp96B.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t)),((
_tmp96C[0]=& _tmp96B,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((
_tmp96D="found non-zero type %s in generate_zero",_tag_dyneither(_tmp96D,sizeof(
char),40))),_tag_dyneither(_tmp96C,sizeof(void*),1)))))));}_LL1C2:;}{struct Cyc_Core_Opt*
_tmp96E;res->topt=((_tmp96E=_cycalloc(sizeof(*_tmp96E)),((_tmp96E->v=(void*)t,
_tmp96E))));}return res;}static void Cyc_Toc_zero_ptr_assign_to_c(struct Cyc_Toc_Env*
nv,struct Cyc_Absyn_Exp*e,struct Cyc_Absyn_Exp*e1,struct Cyc_Core_Opt*popt,struct
Cyc_Absyn_Exp*e2,void*ptr_type,int is_dyneither,void*elt_type);static void Cyc_Toc_zero_ptr_assign_to_c(
struct Cyc_Toc_Env*nv,struct Cyc_Absyn_Exp*e,struct Cyc_Absyn_Exp*e1,struct Cyc_Core_Opt*
popt,struct Cyc_Absyn_Exp*e2,void*ptr_type,int is_dyneither,void*elt_type){void*
fat_ptr_type=Cyc_Absyn_dyneither_typ(elt_type,(void*)2,Cyc_Toc_mt_tq,Cyc_Absyn_true_conref);
void*_tmp37E=Cyc_Toc_typ_to_c(elt_type);void*_tmp37F=Cyc_Toc_typ_to_c(
fat_ptr_type);void*_tmp380=Cyc_Absyn_cstar_typ(_tmp37E,Cyc_Toc_mt_tq);struct Cyc_Core_Opt*
_tmp96F;struct Cyc_Core_Opt*_tmp381=(_tmp96F=_cycalloc(sizeof(*_tmp96F)),((
_tmp96F->v=(void*)_tmp380,_tmp96F)));struct Cyc_Absyn_Exp*xinit;{void*_tmp382=(
void*)e1->r;struct Cyc_Absyn_Exp*_tmp383;struct Cyc_Absyn_Exp*_tmp384;struct Cyc_Absyn_Exp*
_tmp385;_LL1DA: if(*((int*)_tmp382)!= 22)goto _LL1DC;_tmp383=((struct Cyc_Absyn_Deref_e_struct*)
_tmp382)->f1;_LL1DB: if(!is_dyneither){_tmp383=Cyc_Toc_cast_it(fat_ptr_type,
_tmp383);{struct Cyc_Core_Opt*_tmp970;_tmp383->topt=((_tmp970=_cycalloc(sizeof(*
_tmp970)),((_tmp970->v=(void*)fat_ptr_type,_tmp970))));}}Cyc_Toc_exp_to_c(nv,
_tmp383);xinit=_tmp383;goto _LL1D9;_LL1DC: if(*((int*)_tmp382)!= 25)goto _LL1DE;
_tmp384=((struct Cyc_Absyn_Subscript_e_struct*)_tmp382)->f1;_tmp385=((struct Cyc_Absyn_Subscript_e_struct*)
_tmp382)->f2;_LL1DD: if(!is_dyneither){_tmp384=Cyc_Toc_cast_it(fat_ptr_type,
_tmp384);{struct Cyc_Core_Opt*_tmp971;_tmp384->topt=((_tmp971=_cycalloc(sizeof(*
_tmp971)),((_tmp971->v=(void*)fat_ptr_type,_tmp971))));}}Cyc_Toc_exp_to_c(nv,
_tmp384);Cyc_Toc_exp_to_c(nv,_tmp385);{struct Cyc_Absyn_Exp*_tmp972[3];xinit=Cyc_Absyn_fncall_exp(
Cyc_Toc__dyneither_ptr_plus_e,((_tmp972[2]=_tmp385,((_tmp972[1]=Cyc_Absyn_sizeoftyp_exp(
Cyc_Toc_typ_to_c(elt_type),0),((_tmp972[0]=_tmp384,((struct Cyc_List_List*(*)(
struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp972,sizeof(struct Cyc_Absyn_Exp*),
3)))))))),0);}goto _LL1D9;_LL1DE:;_LL1DF: {const char*_tmp975;void*_tmp974;(
_tmp974=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((
_tmp975="found bad lhs for zero-terminated pointer assignment",_tag_dyneither(
_tmp975,sizeof(char),53))),_tag_dyneither(_tmp974,sizeof(void*),0)));}_LL1D9:;}{
struct _tuple1*_tmp38B=Cyc_Toc_temp_var();struct _RegionHandle _tmp38C=_new_region("rgn2");
struct _RegionHandle*rgn2=& _tmp38C;_push_region(rgn2);{struct Cyc_Toc_Env*_tmp38D=
Cyc_Toc_add_varmap(rgn2,nv,_tmp38B,Cyc_Absyn_var_exp(_tmp38B,0));struct Cyc_Absyn_Vardecl*
_tmp976;struct Cyc_Absyn_Vardecl*_tmp38E=(_tmp976=_cycalloc(sizeof(*_tmp976)),((
_tmp976->sc=(void*)((void*)2),((_tmp976->name=_tmp38B,((_tmp976->tq=Cyc_Toc_mt_tq,((
_tmp976->type=(void*)_tmp37F,((_tmp976->initializer=(struct Cyc_Absyn_Exp*)xinit,((
_tmp976->rgn=0,((_tmp976->attributes=0,((_tmp976->escapes=0,_tmp976)))))))))))))))));
struct Cyc_Absyn_Local_b_struct _tmp979;struct Cyc_Absyn_Local_b_struct*_tmp978;
struct Cyc_Absyn_Local_b_struct*_tmp38F=(_tmp978=_cycalloc(sizeof(*_tmp978)),((
_tmp978[0]=((_tmp979.tag=3,((_tmp979.f1=_tmp38E,_tmp979)))),_tmp978)));struct Cyc_Absyn_Exp*
_tmp390=Cyc_Absyn_varb_exp(_tmp38B,(void*)_tmp38F,0);{struct Cyc_Core_Opt*_tmp97A;
_tmp390->topt=((_tmp97A=_cycalloc(sizeof(*_tmp97A)),((_tmp97A->v=(void*)
fat_ptr_type,_tmp97A))));}{struct Cyc_Absyn_Exp*_tmp392=Cyc_Absyn_deref_exp(
_tmp390,0);{struct Cyc_Core_Opt*_tmp97B;_tmp392->topt=((_tmp97B=_cycalloc(sizeof(*
_tmp97B)),((_tmp97B->v=(void*)elt_type,_tmp97B))));}Cyc_Toc_exp_to_c(_tmp38D,
_tmp392);{struct _tuple1*_tmp394=Cyc_Toc_temp_var();_tmp38D=Cyc_Toc_add_varmap(
rgn2,_tmp38D,_tmp394,Cyc_Absyn_var_exp(_tmp394,0));{struct Cyc_Absyn_Vardecl*
_tmp97C;struct Cyc_Absyn_Vardecl*_tmp395=(_tmp97C=_cycalloc(sizeof(*_tmp97C)),((
_tmp97C->sc=(void*)((void*)2),((_tmp97C->name=_tmp394,((_tmp97C->tq=Cyc_Toc_mt_tq,((
_tmp97C->type=(void*)_tmp37E,((_tmp97C->initializer=(struct Cyc_Absyn_Exp*)
_tmp392,((_tmp97C->rgn=0,((_tmp97C->attributes=0,((_tmp97C->escapes=0,_tmp97C)))))))))))))))));
struct Cyc_Absyn_Local_b_struct _tmp97F;struct Cyc_Absyn_Local_b_struct*_tmp97E;
struct Cyc_Absyn_Local_b_struct*_tmp396=(_tmp97E=_cycalloc(sizeof(*_tmp97E)),((
_tmp97E[0]=((_tmp97F.tag=3,((_tmp97F.f1=_tmp395,_tmp97F)))),_tmp97E)));struct Cyc_Absyn_Exp*
z_init=e2;if(popt != 0){struct Cyc_Absyn_Exp*_tmp397=Cyc_Absyn_varb_exp(_tmp394,(
void*)_tmp396,0);_tmp397->topt=_tmp392->topt;z_init=Cyc_Absyn_prim2_exp((void*)
popt->v,_tmp397,e2,0);z_init->topt=_tmp397->topt;}Cyc_Toc_exp_to_c(_tmp38D,
z_init);{struct _tuple1*_tmp398=Cyc_Toc_temp_var();struct Cyc_Absyn_Vardecl*
_tmp980;struct Cyc_Absyn_Vardecl*_tmp399=(_tmp980=_cycalloc(sizeof(*_tmp980)),((
_tmp980->sc=(void*)((void*)2),((_tmp980->name=_tmp398,((_tmp980->tq=Cyc_Toc_mt_tq,((
_tmp980->type=(void*)_tmp37E,((_tmp980->initializer=(struct Cyc_Absyn_Exp*)z_init,((
_tmp980->rgn=0,((_tmp980->attributes=0,((_tmp980->escapes=0,_tmp980)))))))))))))))));
struct Cyc_Absyn_Local_b_struct _tmp983;struct Cyc_Absyn_Local_b_struct*_tmp982;
struct Cyc_Absyn_Local_b_struct*_tmp39A=(_tmp982=_cycalloc(sizeof(*_tmp982)),((
_tmp982[0]=((_tmp983.tag=3,((_tmp983.f1=_tmp399,_tmp983)))),_tmp982)));_tmp38D=
Cyc_Toc_add_varmap(rgn2,_tmp38D,_tmp398,Cyc_Absyn_var_exp(_tmp398,0));{struct Cyc_Absyn_Exp*
_tmp39B=Cyc_Absyn_varb_exp(_tmp394,(void*)_tmp396,0);_tmp39B->topt=_tmp392->topt;{
struct Cyc_Absyn_Exp*_tmp39C=Cyc_Toc_generate_zero(elt_type);struct Cyc_Absyn_Exp*
_tmp39D=Cyc_Absyn_prim2_exp((void*)5,_tmp39B,_tmp39C,0);{struct Cyc_Core_Opt*
_tmp984;_tmp39D->topt=((_tmp984=_cycalloc(sizeof(*_tmp984)),((_tmp984->v=(void*)
Cyc_Absyn_sint_typ,_tmp984))));}Cyc_Toc_exp_to_c(_tmp38D,_tmp39D);{struct Cyc_Absyn_Exp*
_tmp39F=Cyc_Absyn_varb_exp(_tmp398,(void*)_tmp39A,0);_tmp39F->topt=_tmp392->topt;{
struct Cyc_Absyn_Exp*_tmp3A0=Cyc_Toc_generate_zero(elt_type);struct Cyc_Absyn_Exp*
_tmp3A1=Cyc_Absyn_prim2_exp((void*)6,_tmp39F,_tmp3A0,0);{struct Cyc_Core_Opt*
_tmp985;_tmp3A1->topt=((_tmp985=_cycalloc(sizeof(*_tmp985)),((_tmp985->v=(void*)
Cyc_Absyn_sint_typ,_tmp985))));}Cyc_Toc_exp_to_c(_tmp38D,_tmp3A1);{struct Cyc_Absyn_Exp*
_tmp986[2];struct Cyc_List_List*_tmp3A3=(_tmp986[1]=Cyc_Absyn_sizeoftyp_exp(Cyc_Toc_typ_to_c(
elt_type),0),((_tmp986[0]=Cyc_Absyn_varb_exp(_tmp38B,(void*)_tmp38F,0),((struct
Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp986,
sizeof(struct Cyc_Absyn_Exp*),2)))));struct Cyc_Absyn_Exp*_tmp3A4=Cyc_Absyn_uint_exp(
1,0);struct Cyc_Absyn_Exp*xsize;xsize=Cyc_Absyn_prim2_exp((void*)5,Cyc_Absyn_fncall_exp(
Cyc_Toc__get_dyneither_size_e,_tmp3A3,0),_tmp3A4,0);{struct Cyc_Absyn_Exp*_tmp3A5=
Cyc_Absyn_and_exp(xsize,Cyc_Absyn_and_exp(_tmp39D,_tmp3A1,0),0);struct Cyc_Absyn_Stmt*
_tmp3A6=Cyc_Absyn_exp_stmt(Cyc_Absyn_fncall_exp(Cyc_Toc__throw_arraybounds_e,0,0),
0);struct Cyc_Absyn_Exp*_tmp3A7=Cyc_Absyn_aggrmember_exp(Cyc_Absyn_varb_exp(
_tmp38B,(void*)_tmp38F,0),Cyc_Toc_curr_sp,0);_tmp3A7=Cyc_Toc_cast_it(_tmp380,
_tmp3A7);{struct Cyc_Absyn_Exp*_tmp3A8=Cyc_Absyn_deref_exp(_tmp3A7,0);struct Cyc_Absyn_Exp*
_tmp3A9=Cyc_Absyn_assign_exp(_tmp3A8,Cyc_Absyn_var_exp(_tmp398,0),0);struct Cyc_Absyn_Stmt*
_tmp3AA=Cyc_Absyn_exp_stmt(_tmp3A9,0);_tmp3AA=Cyc_Absyn_seq_stmt(Cyc_Absyn_ifthenelse_stmt(
_tmp3A5,_tmp3A6,Cyc_Absyn_skip_stmt(0),0),_tmp3AA,0);{struct Cyc_Absyn_Var_d_struct*
_tmp98C;struct Cyc_Absyn_Var_d_struct _tmp98B;struct Cyc_Absyn_Decl*_tmp98A;_tmp3AA=
Cyc_Absyn_decl_stmt(((_tmp98A=_cycalloc(sizeof(*_tmp98A)),((_tmp98A->r=(void*)((
void*)((_tmp98C=_cycalloc(sizeof(*_tmp98C)),((_tmp98C[0]=((_tmp98B.tag=0,((
_tmp98B.f1=_tmp399,_tmp98B)))),_tmp98C))))),((_tmp98A->loc=0,_tmp98A)))))),
_tmp3AA,0);}{struct Cyc_Absyn_Var_d_struct*_tmp992;struct Cyc_Absyn_Var_d_struct
_tmp991;struct Cyc_Absyn_Decl*_tmp990;_tmp3AA=Cyc_Absyn_decl_stmt(((_tmp990=
_cycalloc(sizeof(*_tmp990)),((_tmp990->r=(void*)((void*)((_tmp992=_cycalloc(
sizeof(*_tmp992)),((_tmp992[0]=((_tmp991.tag=0,((_tmp991.f1=_tmp395,_tmp991)))),
_tmp992))))),((_tmp990->loc=0,_tmp990)))))),_tmp3AA,0);}{struct Cyc_Absyn_Var_d_struct*
_tmp998;struct Cyc_Absyn_Var_d_struct _tmp997;struct Cyc_Absyn_Decl*_tmp996;_tmp3AA=
Cyc_Absyn_decl_stmt(((_tmp996=_cycalloc(sizeof(*_tmp996)),((_tmp996->r=(void*)((
void*)((_tmp998=_cycalloc(sizeof(*_tmp998)),((_tmp998[0]=((_tmp997.tag=0,((
_tmp997.f1=_tmp38E,_tmp997)))),_tmp998))))),((_tmp996->loc=0,_tmp996)))))),
_tmp3AA,0);}(void*)(e->r=(void*)Cyc_Toc_stmt_exp_r(_tmp3AA));}}}}}}}}}}}};
_pop_region(rgn2);}}struct _tuple11{struct _tuple1*f1;void*f2;struct Cyc_Absyn_Exp*
f3;};struct _tuple12{struct Cyc_Absyn_Aggrfield*f1;struct Cyc_Absyn_Exp*f2;};static
void Cyc_Toc_exp_to_c(struct Cyc_Toc_Env*nv,struct Cyc_Absyn_Exp*e);static void
_tmp9E3(unsigned int*_tmp9E2,unsigned int*_tmp9E1,struct _tuple1***_tmp9DF){for(*
_tmp9E2=0;*_tmp9E2 < *_tmp9E1;(*_tmp9E2)++){(*_tmp9DF)[*_tmp9E2]=Cyc_Toc_temp_var();}}
static void Cyc_Toc_exp_to_c(struct Cyc_Toc_Env*nv,struct Cyc_Absyn_Exp*e){void*
_tmp3BF=(void*)e->r;if(e->topt == 0){const char*_tmp99C;void*_tmp99B[1];struct Cyc_String_pa_struct
_tmp99A;(_tmp99A.tag=0,((_tmp99A.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Absynpp_exp2string(e)),((_tmp99B[0]=& _tmp99A,((int(*)(struct _dyneither_ptr
fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmp99C="exp_to_c: no type for %s",
_tag_dyneither(_tmp99C,sizeof(char),25))),_tag_dyneither(_tmp99B,sizeof(void*),1)))))));}{
void*old_typ=(void*)((struct Cyc_Core_Opt*)_check_null(e->topt))->v;void*_tmp3C3=
_tmp3BF;union Cyc_Absyn_Cnst_union _tmp3C4;struct _tuple1*_tmp3C5;void*_tmp3C6;
struct _tuple1*_tmp3C7;void*_tmp3C8;struct Cyc_List_List*_tmp3C9;struct Cyc_Absyn_Exp*
_tmp3CA;void*_tmp3CB;struct Cyc_Absyn_Exp*_tmp3CC;struct Cyc_Core_Opt*_tmp3CD;
struct Cyc_Absyn_Exp*_tmp3CE;struct Cyc_Absyn_Exp*_tmp3CF;struct Cyc_Absyn_Exp*
_tmp3D0;struct Cyc_Absyn_Exp*_tmp3D1;struct Cyc_Absyn_Exp*_tmp3D2;struct Cyc_Absyn_Exp*
_tmp3D3;struct Cyc_Absyn_Exp*_tmp3D4;struct Cyc_Absyn_Exp*_tmp3D5;struct Cyc_Absyn_Exp*
_tmp3D6;struct Cyc_Absyn_Exp*_tmp3D7;struct Cyc_Absyn_Exp*_tmp3D8;struct Cyc_List_List*
_tmp3D9;struct Cyc_Absyn_Exp*_tmp3DA;struct Cyc_List_List*_tmp3DB;struct Cyc_Absyn_VarargCallInfo*
_tmp3DC;struct Cyc_Absyn_Exp*_tmp3DD;struct Cyc_List_List*_tmp3DE;struct Cyc_Absyn_VarargCallInfo*
_tmp3DF;struct Cyc_Absyn_VarargCallInfo _tmp3E0;int _tmp3E1;struct Cyc_List_List*
_tmp3E2;struct Cyc_Absyn_VarargInfo*_tmp3E3;struct Cyc_Absyn_Exp*_tmp3E4;struct Cyc_Absyn_Exp*
_tmp3E5;struct Cyc_Absyn_Exp*_tmp3E6;struct Cyc_List_List*_tmp3E7;void*_tmp3E8;
void**_tmp3E9;struct Cyc_Absyn_Exp*_tmp3EA;int _tmp3EB;void*_tmp3EC;struct Cyc_Absyn_Exp*
_tmp3ED;struct Cyc_Absyn_Exp*_tmp3EE;struct Cyc_Absyn_Exp*_tmp3EF;struct Cyc_Absyn_Exp*
_tmp3F0;void*_tmp3F1;void*_tmp3F2;void*_tmp3F3;struct _dyneither_ptr*_tmp3F4;void*
_tmp3F5;void*_tmp3F6;unsigned int _tmp3F7;struct Cyc_Absyn_Exp*_tmp3F8;struct Cyc_Absyn_Exp*
_tmp3F9;struct _dyneither_ptr*_tmp3FA;struct Cyc_Absyn_Exp*_tmp3FB;struct
_dyneither_ptr*_tmp3FC;struct Cyc_Absyn_Exp*_tmp3FD;struct Cyc_Absyn_Exp*_tmp3FE;
struct Cyc_List_List*_tmp3FF;struct Cyc_List_List*_tmp400;struct Cyc_Absyn_Vardecl*
_tmp401;struct Cyc_Absyn_Exp*_tmp402;struct Cyc_Absyn_Exp*_tmp403;int _tmp404;
struct _tuple1*_tmp405;struct Cyc_List_List*_tmp406;struct Cyc_List_List*_tmp407;
struct Cyc_Absyn_Aggrdecl*_tmp408;void*_tmp409;struct Cyc_List_List*_tmp40A;struct
Cyc_List_List*_tmp40B;struct Cyc_Absyn_Tuniondecl*_tmp40C;struct Cyc_Absyn_Tunionfield*
_tmp40D;struct Cyc_List_List*_tmp40E;struct Cyc_Absyn_Tuniondecl*_tmp40F;struct Cyc_Absyn_Tunionfield*
_tmp410;struct Cyc_Absyn_MallocInfo _tmp411;int _tmp412;struct Cyc_Absyn_Exp*_tmp413;
void**_tmp414;struct Cyc_Absyn_Exp*_tmp415;int _tmp416;struct Cyc_Absyn_Exp*_tmp417;
struct Cyc_Absyn_Exp*_tmp418;struct Cyc_Absyn_Stmt*_tmp419;_LL1E1: if(*((int*)
_tmp3C3)!= 0)goto _LL1E3;_tmp3C4=((struct Cyc_Absyn_Const_e_struct*)_tmp3C3)->f1;
if(((((struct Cyc_Absyn_Const_e_struct*)_tmp3C3)->f1).Null_c).tag != 6)goto _LL1E3;
_LL1E2: {struct Cyc_Absyn_Exp*_tmp41A=Cyc_Absyn_uint_exp(0,0);if(Cyc_Tcutil_is_tagged_pointer_typ(
old_typ)){if(Cyc_Toc_is_toplevel(nv))(void*)(e->r=(void*)((void*)(Cyc_Toc_make_toplevel_dyn_arr(
old_typ,_tmp41A,_tmp41A))->r));else{struct Cyc_Absyn_Exp*_tmp99D[3];(void*)(e->r=(
void*)Cyc_Toc_fncall_exp_r(Cyc_Toc__tag_dyneither_e,((_tmp99D[2]=_tmp41A,((
_tmp99D[1]=_tmp41A,((_tmp99D[0]=_tmp41A,((struct Cyc_List_List*(*)(struct
_dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp99D,sizeof(struct Cyc_Absyn_Exp*),
3))))))))));}}else{(void*)(e->r=(void*)((void*)& Cyc_Toc_zero_exp));}goto _LL1E0;}
_LL1E3: if(*((int*)_tmp3C3)!= 0)goto _LL1E5;_LL1E4: goto _LL1E0;_LL1E5: if(*((int*)
_tmp3C3)!= 1)goto _LL1E7;_tmp3C5=((struct Cyc_Absyn_Var_e_struct*)_tmp3C3)->f1;
_tmp3C6=(void*)((struct Cyc_Absyn_Var_e_struct*)_tmp3C3)->f2;_LL1E6:{struct
_handler_cons _tmp41C;_push_handler(& _tmp41C);{int _tmp41E=0;if(setjmp(_tmp41C.handler))
_tmp41E=1;if(!_tmp41E){(void*)(e->r=(void*)((void*)(Cyc_Toc_lookup_varmap(nv,
_tmp3C5))->r));;_pop_handler();}else{void*_tmp41D=(void*)_exn_thrown;void*
_tmp420=_tmp41D;_LL23A: if(_tmp420 != Cyc_Dict_Absent)goto _LL23C;_LL23B: {const
char*_tmp9A1;void*_tmp9A0[1];struct Cyc_String_pa_struct _tmp99F;(_tmp99F.tag=0,((
_tmp99F.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(
_tmp3C5)),((_tmp9A0[0]=& _tmp99F,((int(*)(struct _dyneither_ptr fmt,struct
_dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmp9A1="Can't find %s in exp_to_c, Var\n",
_tag_dyneither(_tmp9A1,sizeof(char),32))),_tag_dyneither(_tmp9A0,sizeof(void*),1)))))));}
_LL23C:;_LL23D:(void)_throw(_tmp420);_LL239:;}}}goto _LL1E0;_LL1E7: if(*((int*)
_tmp3C3)!= 2)goto _LL1E9;_tmp3C7=((struct Cyc_Absyn_UnknownId_e_struct*)_tmp3C3)->f1;
_LL1E8: {const char*_tmp9A4;void*_tmp9A3;(_tmp9A3=0,((int(*)(struct _dyneither_ptr
fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmp9A4="unknownid",
_tag_dyneither(_tmp9A4,sizeof(char),10))),_tag_dyneither(_tmp9A3,sizeof(void*),0)));}
_LL1E9: if(*((int*)_tmp3C3)!= 3)goto _LL1EB;_tmp3C8=(void*)((struct Cyc_Absyn_Primop_e_struct*)
_tmp3C3)->f1;_tmp3C9=((struct Cyc_Absyn_Primop_e_struct*)_tmp3C3)->f2;_LL1EA: {
struct Cyc_List_List*_tmp426=((struct Cyc_List_List*(*)(void*(*f)(struct Cyc_Absyn_Exp*),
struct Cyc_List_List*x))Cyc_List_map)(Cyc_Toc_get_cyc_typ,_tmp3C9);((void(*)(void(*
f)(struct Cyc_Toc_Env*,struct Cyc_Absyn_Exp*),struct Cyc_Toc_Env*env,struct Cyc_List_List*
x))Cyc_List_iter_c)(Cyc_Toc_exp_to_c,nv,_tmp3C9);{void*_tmp427=_tmp3C8;_LL23F:
if((int)_tmp427 != 19)goto _LL241;_LL240: {struct Cyc_Absyn_Exp*arg=(struct Cyc_Absyn_Exp*)((
struct Cyc_List_List*)_check_null(_tmp3C9))->hd;{void*_tmp428=Cyc_Tcutil_compress((
void*)((struct Cyc_Core_Opt*)_check_null(arg->topt))->v);struct Cyc_Absyn_ArrayInfo
_tmp429;struct Cyc_Absyn_Exp*_tmp42A;struct Cyc_Absyn_PtrInfo _tmp42B;void*_tmp42C;
struct Cyc_Absyn_PtrAtts _tmp42D;struct Cyc_Absyn_Conref*_tmp42E;struct Cyc_Absyn_Conref*
_tmp42F;struct Cyc_Absyn_Conref*_tmp430;_LL254: if(_tmp428 <= (void*)4)goto _LL258;
if(*((int*)_tmp428)!= 7)goto _LL256;_tmp429=((struct Cyc_Absyn_ArrayType_struct*)
_tmp428)->f1;_tmp42A=_tmp429.num_elts;_LL255: if(!Cyc_Evexp_c_can_eval((struct Cyc_Absyn_Exp*)
_check_null(_tmp42A))){const char*_tmp9A7;void*_tmp9A6;(_tmp9A6=0,Cyc_Tcutil_terr(
e->loc,((_tmp9A7="can't calculate numelts",_tag_dyneither(_tmp9A7,sizeof(char),
24))),_tag_dyneither(_tmp9A6,sizeof(void*),0)));}(void*)(e->r=(void*)((void*)
_tmp42A->r));goto _LL253;_LL256: if(*((int*)_tmp428)!= 4)goto _LL258;_tmp42B=((
struct Cyc_Absyn_PointerType_struct*)_tmp428)->f1;_tmp42C=(void*)_tmp42B.elt_typ;
_tmp42D=_tmp42B.ptr_atts;_tmp42E=_tmp42D.nullable;_tmp42F=_tmp42D.bounds;_tmp430=
_tmp42D.zero_term;_LL257:{void*_tmp433=Cyc_Absyn_conref_def(Cyc_Absyn_bounds_one,
_tmp42F);struct Cyc_Absyn_Exp*_tmp434;_LL25B: if((int)_tmp433 != 0)goto _LL25D;
_LL25C:{struct Cyc_Absyn_Exp*_tmp9A8[2];(void*)(e->r=(void*)Cyc_Toc_fncall_exp_r(
Cyc_Toc__get_dyneither_size_e,((_tmp9A8[1]=Cyc_Absyn_sizeoftyp_exp(Cyc_Toc_typ_to_c(
_tmp42C),0),((_tmp9A8[0]=(struct Cyc_Absyn_Exp*)_tmp3C9->hd,((struct Cyc_List_List*(*)(
struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp9A8,sizeof(struct Cyc_Absyn_Exp*),
2))))))));}goto _LL25A;_LL25D: if(_tmp433 <= (void*)1)goto _LL25A;if(*((int*)_tmp433)
!= 0)goto _LL25A;_tmp434=((struct Cyc_Absyn_Upper_b_struct*)_tmp433)->f1;_LL25E:
if(((int(*)(int,struct Cyc_Absyn_Conref*x))Cyc_Absyn_conref_def)(0,_tmp430)){
struct Cyc_Absyn_Exp*function_e=Cyc_Toc_getFunction(& Cyc_Toc__get_zero_arr_size_functionSet,(
struct Cyc_Absyn_Exp*)_tmp3C9->hd);struct Cyc_Absyn_Exp*_tmp9A9[2];(void*)(e->r=(
void*)Cyc_Toc_fncall_exp_r(function_e,((_tmp9A9[1]=_tmp434,((_tmp9A9[0]=(struct
Cyc_Absyn_Exp*)_tmp3C9->hd,((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(
_tag_dyneither(_tmp9A9,sizeof(struct Cyc_Absyn_Exp*),2))))))));}else{if(((int(*)(
int,struct Cyc_Absyn_Conref*x))Cyc_Absyn_conref_def)(0,_tmp42E)){if(!Cyc_Evexp_c_can_eval(
_tmp434)){const char*_tmp9AC;void*_tmp9AB;(_tmp9AB=0,Cyc_Tcutil_terr(e->loc,((
_tmp9AC="can't calculate numelts",_tag_dyneither(_tmp9AC,sizeof(char),24))),
_tag_dyneither(_tmp9AB,sizeof(void*),0)));}(void*)(e->r=(void*)Cyc_Toc_conditional_exp_r(
arg,_tmp434,Cyc_Absyn_uint_exp(0,0)));}else{(void*)(e->r=(void*)((void*)_tmp434->r));
goto _LL25A;}}goto _LL25A;_LL25A:;}goto _LL253;_LL258:;_LL259: {const char*_tmp9B1;
void*_tmp9B0[2];struct Cyc_String_pa_struct _tmp9AF;struct Cyc_String_pa_struct
_tmp9AE;(_tmp9AE.tag=0,((_tmp9AE.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Absynpp_typ2string((void*)((struct Cyc_Core_Opt*)_check_null(arg->topt))->v)),((
_tmp9AF.tag=0,((_tmp9AF.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string((
void*)((struct Cyc_Core_Opt*)_check_null(arg->topt))->v)),((_tmp9B0[0]=& _tmp9AF,((
_tmp9B0[1]=& _tmp9AE,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((
_tmp9B1="size primop applied to non-array %s (%s)",_tag_dyneither(_tmp9B1,
sizeof(char),41))),_tag_dyneither(_tmp9B0,sizeof(void*),2)))))))))))));}_LL253:;}
goto _LL23E;}_LL241: if((int)_tmp427 != 0)goto _LL243;_LL242:{void*_tmp43D=Cyc_Tcutil_compress((
void*)((struct Cyc_List_List*)_check_null(_tmp426))->hd);struct Cyc_Absyn_PtrInfo
_tmp43E;void*_tmp43F;struct Cyc_Absyn_PtrAtts _tmp440;struct Cyc_Absyn_Conref*
_tmp441;struct Cyc_Absyn_Conref*_tmp442;_LL260: if(_tmp43D <= (void*)4)goto _LL262;
if(*((int*)_tmp43D)!= 4)goto _LL262;_tmp43E=((struct Cyc_Absyn_PointerType_struct*)
_tmp43D)->f1;_tmp43F=(void*)_tmp43E.elt_typ;_tmp440=_tmp43E.ptr_atts;_tmp441=
_tmp440.bounds;_tmp442=_tmp440.zero_term;_LL261:{void*_tmp443=Cyc_Absyn_conref_def(
Cyc_Absyn_bounds_one,_tmp441);struct Cyc_Absyn_Exp*_tmp444;_LL265: if((int)_tmp443
!= 0)goto _LL267;_LL266: {struct Cyc_Absyn_Exp*e1=(struct Cyc_Absyn_Exp*)((struct
Cyc_List_List*)_check_null(_tmp3C9))->hd;struct Cyc_Absyn_Exp*e2=(struct Cyc_Absyn_Exp*)((
struct Cyc_List_List*)_check_null(_tmp3C9->tl))->hd;{struct Cyc_Absyn_Exp*_tmp9B2[
3];(void*)(e->r=(void*)Cyc_Toc_fncall_exp_r(Cyc_Toc__dyneither_ptr_plus_e,((
_tmp9B2[2]=e2,((_tmp9B2[1]=Cyc_Absyn_sizeoftyp_exp(Cyc_Toc_typ_to_c(_tmp43F),0),((
_tmp9B2[0]=e1,((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(
_tag_dyneither(_tmp9B2,sizeof(struct Cyc_Absyn_Exp*),3))))))))));}goto _LL264;}
_LL267: if(_tmp443 <= (void*)1)goto _LL264;if(*((int*)_tmp443)!= 0)goto _LL264;
_tmp444=((struct Cyc_Absyn_Upper_b_struct*)_tmp443)->f1;_LL268: if(((int(*)(int,
struct Cyc_Absyn_Conref*x))Cyc_Absyn_conref_def)(0,_tmp442)){struct Cyc_Absyn_Exp*
e1=(struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)_check_null(_tmp3C9))->hd;struct
Cyc_Absyn_Exp*e2=(struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)_check_null(
_tmp3C9->tl))->hd;struct Cyc_Absyn_Exp*_tmp9B3[3];(void*)(e->r=(void*)((void*)(
Cyc_Absyn_fncall_exp(Cyc_Toc_getFunction(& Cyc_Toc__zero_arr_plus_functionSet,e1),((
_tmp9B3[2]=e2,((_tmp9B3[1]=_tmp444,((_tmp9B3[0]=e1,((struct Cyc_List_List*(*)(
struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp9B3,sizeof(struct Cyc_Absyn_Exp*),
3)))))))),0))->r));}goto _LL264;_LL264:;}goto _LL25F;_LL262:;_LL263: goto _LL25F;
_LL25F:;}goto _LL23E;_LL243: if((int)_tmp427 != 2)goto _LL245;_LL244: {void*elt_typ=(
void*)0;if(Cyc_Tcutil_is_tagged_pointer_typ_elt((void*)((struct Cyc_List_List*)
_check_null(_tmp426))->hd,& elt_typ)){struct Cyc_Absyn_Exp*e1=(struct Cyc_Absyn_Exp*)((
struct Cyc_List_List*)_check_null(_tmp3C9))->hd;struct Cyc_Absyn_Exp*e2=(struct Cyc_Absyn_Exp*)((
struct Cyc_List_List*)_check_null(_tmp3C9->tl))->hd;if(Cyc_Tcutil_is_tagged_pointer_typ((
void*)((struct Cyc_List_List*)_check_null(_tmp426->tl))->hd)){(void*)(e1->r=(void*)
Cyc_Toc_aggrmember_exp_r(Cyc_Absyn_new_exp((void*)e1->r,0),Cyc_Toc_curr_sp));(
void*)(e2->r=(void*)Cyc_Toc_aggrmember_exp_r(Cyc_Absyn_new_exp((void*)e2->r,0),
Cyc_Toc_curr_sp));(void*)(e->r=(void*)((void*)(Cyc_Absyn_divide_exp(Cyc_Absyn_copy_exp(
e),Cyc_Absyn_sizeoftyp_exp(Cyc_Toc_typ_to_c(elt_typ),0),0))->r));}else{struct Cyc_Absyn_Exp*
_tmp9B4[3];(void*)(e->r=(void*)Cyc_Toc_fncall_exp_r(Cyc_Toc__dyneither_ptr_plus_e,((
_tmp9B4[2]=Cyc_Absyn_prim1_exp((void*)2,e2,0),((_tmp9B4[1]=Cyc_Absyn_sizeoftyp_exp(
Cyc_Toc_typ_to_c(elt_typ),0),((_tmp9B4[0]=e1,((struct Cyc_List_List*(*)(struct
_dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp9B4,sizeof(struct Cyc_Absyn_Exp*),
3))))))))));}}goto _LL23E;}_LL245: if((int)_tmp427 != 5)goto _LL247;_LL246: goto
_LL248;_LL247: if((int)_tmp427 != 6)goto _LL249;_LL248: goto _LL24A;_LL249: if((int)
_tmp427 != 7)goto _LL24B;_LL24A: goto _LL24C;_LL24B: if((int)_tmp427 != 9)goto _LL24D;
_LL24C: goto _LL24E;_LL24D: if((int)_tmp427 != 8)goto _LL24F;_LL24E: goto _LL250;_LL24F:
if((int)_tmp427 != 10)goto _LL251;_LL250: {struct Cyc_Absyn_Exp*e1=(struct Cyc_Absyn_Exp*)((
struct Cyc_List_List*)_check_null(_tmp3C9))->hd;struct Cyc_Absyn_Exp*e2=(struct Cyc_Absyn_Exp*)((
struct Cyc_List_List*)_check_null(_tmp3C9->tl))->hd;void*t1=(void*)((struct Cyc_List_List*)
_check_null(_tmp426))->hd;void*t2=(void*)((struct Cyc_List_List*)_check_null(
_tmp426->tl))->hd;if(Cyc_Tcutil_is_tagged_pointer_typ(t1))(void*)(e1->r=(void*)
Cyc_Toc_aggrmember_exp_r(Cyc_Absyn_new_exp((void*)e1->r,0),Cyc_Toc_curr_sp));if(
Cyc_Tcutil_is_tagged_pointer_typ(t2))(void*)(e2->r=(void*)Cyc_Toc_aggrmember_exp_r(
Cyc_Absyn_new_exp((void*)e2->r,0),Cyc_Toc_curr_sp));goto _LL23E;}_LL251:;_LL252:
goto _LL23E;_LL23E:;}goto _LL1E0;}_LL1EB: if(*((int*)_tmp3C3)!= 5)goto _LL1ED;
_tmp3CA=((struct Cyc_Absyn_Increment_e_struct*)_tmp3C3)->f1;_tmp3CB=(void*)((
struct Cyc_Absyn_Increment_e_struct*)_tmp3C3)->f2;_LL1EC: {void*e2_cyc_typ=(void*)((
struct Cyc_Core_Opt*)_check_null(_tmp3CA->topt))->v;void*ptr_type=(void*)0;void*
elt_type=(void*)0;int is_dyneither=0;const char*_tmp9B5;struct _dyneither_ptr
incr_str=(_tmp9B5="increment",_tag_dyneither(_tmp9B5,sizeof(char),10));if(
_tmp3CB == (void*)2  || _tmp3CB == (void*)3){const char*_tmp9B6;incr_str=((_tmp9B6="decrement",
_tag_dyneither(_tmp9B6,sizeof(char),10)));}if(Cyc_Tcutil_is_zero_ptr_deref(
_tmp3CA,& ptr_type,& is_dyneither,& elt_type)){{const char*_tmp9BA;void*_tmp9B9[1];
struct Cyc_String_pa_struct _tmp9B8;(_tmp9B8.tag=0,((_tmp9B8.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)incr_str),((_tmp9B9[0]=& _tmp9B8,Cyc_Tcutil_terr(
e->loc,((_tmp9BA="in-place %s is not supported when dereferencing a zero-terminated pointer",
_tag_dyneither(_tmp9BA,sizeof(char),74))),_tag_dyneither(_tmp9B9,sizeof(void*),1)))))));}{
const char*_tmp9BD;void*_tmp9BC;(_tmp9BC=0,((int(*)(struct _dyneither_ptr fmt,
struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmp9BD="in-place inc on zero-term",
_tag_dyneither(_tmp9BD,sizeof(char),26))),_tag_dyneither(_tmp9BC,sizeof(void*),0)));}}
Cyc_Toc_exp_to_c(nv,_tmp3CA);{void*elt_typ=(void*)0;if(Cyc_Tcutil_is_tagged_pointer_typ_elt(
old_typ,& elt_typ)){struct Cyc_Absyn_Exp*fn_e;int change=1;fn_e=(_tmp3CB == (void*)1
 || _tmp3CB == (void*)3)?Cyc_Toc__dyneither_ptr_inplace_plus_post_e: Cyc_Toc__dyneither_ptr_inplace_plus_e;
if(_tmp3CB == (void*)2  || _tmp3CB == (void*)3)change=- 1;{struct Cyc_Absyn_Exp*
_tmp9BE[3];(void*)(e->r=(void*)Cyc_Toc_fncall_exp_r(fn_e,((_tmp9BE[2]=Cyc_Absyn_signed_int_exp(
change,0),((_tmp9BE[1]=Cyc_Absyn_sizeoftyp_exp(Cyc_Toc_typ_to_c(elt_typ),0),((
_tmp9BE[0]=Cyc_Absyn_address_exp(_tmp3CA,0),((struct Cyc_List_List*(*)(struct
_dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp9BE,sizeof(struct Cyc_Absyn_Exp*),
3))))))))));}}else{if(Cyc_Tcutil_is_zero_pointer_typ_elt(old_typ,& elt_typ)){
struct Cyc_Absyn_Exp*fn_e;{void*_tmp44F=Cyc_Tcutil_compress((void*)((struct Cyc_Core_Opt*)
_check_null(_tmp3CA->topt))->v);void*_tmp450;int _tmp451;_LL26A: if(_tmp44F <= (
void*)4)goto _LL26C;if(*((int*)_tmp44F)!= 5)goto _LL26C;_tmp450=(void*)((struct Cyc_Absyn_IntType_struct*)
_tmp44F)->f2;_LL26B:{void*_tmp452=_tmp450;_LL275: if((int)_tmp452 != 0)goto _LL277;
_LL276: fn_e=_tmp3CB == (void*)1?Cyc_Toc__zero_arr_inplace_plus_post_char_e: Cyc_Toc__zero_arr_inplace_plus_char_e;
goto _LL274;_LL277: if((int)_tmp452 != 1)goto _LL279;_LL278: fn_e=_tmp3CB == (void*)1?
Cyc_Toc__zero_arr_inplace_plus_post_short_e: Cyc_Toc__zero_arr_inplace_plus_short_e;
goto _LL274;_LL279: if((int)_tmp452 != 2)goto _LL27B;_LL27A: fn_e=_tmp3CB == (void*)1?
Cyc_Toc__zero_arr_inplace_plus_post_int_e: Cyc_Toc__zero_arr_inplace_plus_int_e;
goto _LL274;_LL27B:;_LL27C: {struct Cyc_Core_Impossible_struct _tmp9C4;const char*
_tmp9C3;struct Cyc_Core_Impossible_struct*_tmp9C2;(int)_throw((void*)((_tmp9C2=
_cycalloc(sizeof(*_tmp9C2)),((_tmp9C2[0]=((_tmp9C4.tag=Cyc_Core_Impossible,((
_tmp9C4.f1=((_tmp9C3="impossible IntType (not char, short or int)",
_tag_dyneither(_tmp9C3,sizeof(char),44))),_tmp9C4)))),_tmp9C2)))));}_LL274:;}
goto _LL269;_LL26C: if((int)_tmp44F != 1)goto _LL26E;_LL26D: fn_e=_tmp3CB == (void*)1?
Cyc_Toc__zero_arr_inplace_plus_post_float_e: Cyc_Toc__zero_arr_inplace_plus_float_e;
goto _LL269;_LL26E: if(_tmp44F <= (void*)4)goto _LL272;if(*((int*)_tmp44F)!= 6)goto
_LL270;_tmp451=((struct Cyc_Absyn_DoubleType_struct*)_tmp44F)->f1;_LL26F: switch(
_tmp451){case 1: _LL27D: fn_e=_tmp3CB == (void*)1?Cyc_Toc__zero_arr_inplace_plus_post_longdouble_e:
Cyc_Toc__zero_arr_inplace_plus_longdouble_e;break;default: _LL27E: fn_e=_tmp3CB == (
void*)1?Cyc_Toc__zero_arr_inplace_plus_post_double_e: Cyc_Toc__zero_arr_inplace_plus_double_e;}
goto _LL269;_LL270: if(*((int*)_tmp44F)!= 4)goto _LL272;_LL271: fn_e=_tmp3CB == (void*)
1?Cyc_Toc__zero_arr_inplace_plus_post_voidstar_e: Cyc_Toc__zero_arr_inplace_plus_voidstar_e;
goto _LL269;_LL272:;_LL273: {struct Cyc_Core_Impossible_struct _tmp9CA;const char*
_tmp9C9;struct Cyc_Core_Impossible_struct*_tmp9C8;(int)_throw((void*)((_tmp9C8=
_cycalloc(sizeof(*_tmp9C8)),((_tmp9C8[0]=((_tmp9CA.tag=Cyc_Core_Impossible,((
_tmp9CA.f1=((_tmp9C9="impossible expression type (not int, float, double, or pointer)",
_tag_dyneither(_tmp9C9,sizeof(char),64))),_tmp9CA)))),_tmp9C8)))));}_LL269:;}{
struct Cyc_Absyn_Exp*_tmp9CB[2];(void*)(e->r=(void*)Cyc_Toc_fncall_exp_r(fn_e,((
_tmp9CB[1]=Cyc_Absyn_signed_int_exp(1,0),((_tmp9CB[0]=_tmp3CA,((struct Cyc_List_List*(*)(
struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp9CB,sizeof(struct Cyc_Absyn_Exp*),
2))))))));}}else{if(elt_typ == (void*)0  && !Cyc_Absyn_is_lvalue(_tmp3CA)){Cyc_Toc_lvalue_assign(
_tmp3CA,0,Cyc_Toc_incr_lvalue,_tmp3CB);(void*)(e->r=(void*)((void*)_tmp3CA->r));}}}
goto _LL1E0;}}_LL1ED: if(*((int*)_tmp3C3)!= 4)goto _LL1EF;_tmp3CC=((struct Cyc_Absyn_AssignOp_e_struct*)
_tmp3C3)->f1;_tmp3CD=((struct Cyc_Absyn_AssignOp_e_struct*)_tmp3C3)->f2;_tmp3CE=((
struct Cyc_Absyn_AssignOp_e_struct*)_tmp3C3)->f3;_LL1EE: {void*ptr_type=(void*)0;
void*elt_type=(void*)0;int is_dyneither=0;if(Cyc_Tcutil_is_zero_ptr_deref(_tmp3CC,&
ptr_type,& is_dyneither,& elt_type)){Cyc_Toc_zero_ptr_assign_to_c(nv,e,_tmp3CC,
_tmp3CD,_tmp3CE,ptr_type,is_dyneither,elt_type);return;}{int e1_poly=Cyc_Toc_is_poly_project(
_tmp3CC);void*e1_old_typ=(void*)((struct Cyc_Core_Opt*)_check_null(_tmp3CC->topt))->v;
void*e2_old_typ=(void*)((struct Cyc_Core_Opt*)_check_null(_tmp3CE->topt))->v;Cyc_Toc_exp_to_c(
nv,_tmp3CC);Cyc_Toc_exp_to_c(nv,_tmp3CE);{int done=0;if(_tmp3CD != 0){void*elt_typ=(
void*)0;if(Cyc_Tcutil_is_tagged_pointer_typ_elt(old_typ,& elt_typ)){struct Cyc_Absyn_Exp*
change;{void*_tmp45B=(void*)_tmp3CD->v;_LL281: if((int)_tmp45B != 0)goto _LL283;
_LL282: change=_tmp3CE;goto _LL280;_LL283: if((int)_tmp45B != 2)goto _LL285;_LL284:
change=Cyc_Absyn_prim1_exp((void*)2,_tmp3CE,0);goto _LL280;_LL285:;_LL286: {const
char*_tmp9CE;void*_tmp9CD;(_tmp9CD=0,((int(*)(struct _dyneither_ptr fmt,struct
_dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmp9CE="bad t ? pointer arithmetic",
_tag_dyneither(_tmp9CE,sizeof(char),27))),_tag_dyneither(_tmp9CD,sizeof(void*),0)));}
_LL280:;}done=1;{struct Cyc_Absyn_Exp*_tmp45E=Cyc_Toc__dyneither_ptr_inplace_plus_e;
struct Cyc_Absyn_Exp*_tmp9CF[3];(void*)(e->r=(void*)Cyc_Toc_fncall_exp_r(_tmp45E,((
_tmp9CF[2]=change,((_tmp9CF[1]=Cyc_Absyn_sizeoftyp_exp(Cyc_Toc_typ_to_c(elt_typ),
0),((_tmp9CF[0]=Cyc_Absyn_address_exp(_tmp3CC,0),((struct Cyc_List_List*(*)(
struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp9CF,sizeof(struct Cyc_Absyn_Exp*),
3))))))))));}}else{if(Cyc_Tcutil_is_zero_pointer_typ_elt(old_typ,& elt_typ)){void*
_tmp460=(void*)_tmp3CD->v;_LL288: if((int)_tmp460 != 0)goto _LL28A;_LL289: done=1;{
struct Cyc_Absyn_Exp*_tmp9D0[2];(void*)(e->r=(void*)Cyc_Toc_fncall_exp_r(Cyc_Toc_getFunction(&
Cyc_Toc__zero_arr_inplace_plus_functionSet,_tmp3CC),((_tmp9D0[1]=_tmp3CE,((
_tmp9D0[0]=_tmp3CC,((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(
_tag_dyneither(_tmp9D0,sizeof(struct Cyc_Absyn_Exp*),2))))))));}goto _LL287;_LL28A:;
_LL28B: {const char*_tmp9D3;void*_tmp9D2;(_tmp9D2=0,((int(*)(struct _dyneither_ptr
fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmp9D3="bad zero-terminated pointer arithmetic",
_tag_dyneither(_tmp9D3,sizeof(char),39))),_tag_dyneither(_tmp9D2,sizeof(void*),0)));}
_LL287:;}}}if(!done){if(e1_poly)(void*)(_tmp3CE->r=(void*)Cyc_Toc_cast_it_r(Cyc_Absyn_void_star_typ(),
Cyc_Absyn_new_exp((void*)_tmp3CE->r,0)));if(!Cyc_Absyn_is_lvalue(_tmp3CC)){{
struct _tuple10 _tmp9D6;struct _tuple10*_tmp9D5;((void(*)(struct Cyc_Absyn_Exp*e1,
struct Cyc_List_List*fs,struct Cyc_Absyn_Exp*(*f)(struct Cyc_Absyn_Exp*,struct
_tuple10*),struct _tuple10*f_env))Cyc_Toc_lvalue_assign)(_tmp3CC,0,Cyc_Toc_assignop_lvalue,((
_tmp9D5=_cycalloc(sizeof(struct _tuple10)* 1),((_tmp9D5[0]=((_tmp9D6.f1=_tmp3CD,((
_tmp9D6.f2=_tmp3CE,_tmp9D6)))),_tmp9D5)))));}(void*)(e->r=(void*)((void*)_tmp3CC->r));}}
goto _LL1E0;}}}_LL1EF: if(*((int*)_tmp3C3)!= 6)goto _LL1F1;_tmp3CF=((struct Cyc_Absyn_Conditional_e_struct*)
_tmp3C3)->f1;_tmp3D0=((struct Cyc_Absyn_Conditional_e_struct*)_tmp3C3)->f2;
_tmp3D1=((struct Cyc_Absyn_Conditional_e_struct*)_tmp3C3)->f3;_LL1F0: Cyc_Toc_exp_to_c(
nv,_tmp3CF);Cyc_Toc_exp_to_c(nv,_tmp3D0);Cyc_Toc_exp_to_c(nv,_tmp3D1);goto _LL1E0;
_LL1F1: if(*((int*)_tmp3C3)!= 7)goto _LL1F3;_tmp3D2=((struct Cyc_Absyn_And_e_struct*)
_tmp3C3)->f1;_tmp3D3=((struct Cyc_Absyn_And_e_struct*)_tmp3C3)->f2;_LL1F2: Cyc_Toc_exp_to_c(
nv,_tmp3D2);Cyc_Toc_exp_to_c(nv,_tmp3D3);goto _LL1E0;_LL1F3: if(*((int*)_tmp3C3)!= 
8)goto _LL1F5;_tmp3D4=((struct Cyc_Absyn_Or_e_struct*)_tmp3C3)->f1;_tmp3D5=((
struct Cyc_Absyn_Or_e_struct*)_tmp3C3)->f2;_LL1F4: Cyc_Toc_exp_to_c(nv,_tmp3D4);
Cyc_Toc_exp_to_c(nv,_tmp3D5);goto _LL1E0;_LL1F5: if(*((int*)_tmp3C3)!= 9)goto
_LL1F7;_tmp3D6=((struct Cyc_Absyn_SeqExp_e_struct*)_tmp3C3)->f1;_tmp3D7=((struct
Cyc_Absyn_SeqExp_e_struct*)_tmp3C3)->f2;_LL1F6: Cyc_Toc_exp_to_c(nv,_tmp3D6);Cyc_Toc_exp_to_c(
nv,_tmp3D7);goto _LL1E0;_LL1F7: if(*((int*)_tmp3C3)!= 10)goto _LL1F9;_tmp3D8=((
struct Cyc_Absyn_UnknownCall_e_struct*)_tmp3C3)->f1;_tmp3D9=((struct Cyc_Absyn_UnknownCall_e_struct*)
_tmp3C3)->f2;_LL1F8: _tmp3DA=_tmp3D8;_tmp3DB=_tmp3D9;goto _LL1FA;_LL1F9: if(*((int*)
_tmp3C3)!= 11)goto _LL1FB;_tmp3DA=((struct Cyc_Absyn_FnCall_e_struct*)_tmp3C3)->f1;
_tmp3DB=((struct Cyc_Absyn_FnCall_e_struct*)_tmp3C3)->f2;_tmp3DC=((struct Cyc_Absyn_FnCall_e_struct*)
_tmp3C3)->f3;if(_tmp3DC != 0)goto _LL1FB;_LL1FA: Cyc_Toc_exp_to_c(nv,_tmp3DA);((
void(*)(void(*f)(struct Cyc_Toc_Env*,struct Cyc_Absyn_Exp*),struct Cyc_Toc_Env*env,
struct Cyc_List_List*x))Cyc_List_iter_c)(Cyc_Toc_exp_to_c,nv,_tmp3DB);goto _LL1E0;
_LL1FB: if(*((int*)_tmp3C3)!= 11)goto _LL1FD;_tmp3DD=((struct Cyc_Absyn_FnCall_e_struct*)
_tmp3C3)->f1;_tmp3DE=((struct Cyc_Absyn_FnCall_e_struct*)_tmp3C3)->f2;_tmp3DF=((
struct Cyc_Absyn_FnCall_e_struct*)_tmp3C3)->f3;if(_tmp3DF == 0)goto _LL1FD;_tmp3E0=*
_tmp3DF;_tmp3E1=_tmp3E0.num_varargs;_tmp3E2=_tmp3E0.injectors;_tmp3E3=_tmp3E0.vai;
_LL1FC:{struct _RegionHandle _tmp466=_new_region("r");struct _RegionHandle*r=&
_tmp466;_push_region(r);{struct _tuple1*argv=Cyc_Toc_temp_var();struct Cyc_Absyn_Exp*
argvexp=Cyc_Absyn_var_exp(argv,0);struct Cyc_Absyn_Exp*num_varargs_exp=Cyc_Absyn_uint_exp((
unsigned int)_tmp3E1,0);void*cva_type=Cyc_Toc_typ_to_c((void*)_tmp3E3->type);
void*arr_type=Cyc_Absyn_array_typ(cva_type,Cyc_Toc_mt_tq,(struct Cyc_Absyn_Exp*)
num_varargs_exp,Cyc_Absyn_false_conref,0);int num_args=((int(*)(struct Cyc_List_List*
x))Cyc_List_length)(_tmp3DE);int num_normargs=num_args - _tmp3E1;struct Cyc_List_List*
new_args=0;{int i=0;for(0;i < num_normargs;(++ i,_tmp3DE=_tmp3DE->tl)){Cyc_Toc_exp_to_c(
nv,(struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)_check_null(_tmp3DE))->hd);{
struct Cyc_List_List*_tmp9D7;new_args=((_tmp9D7=_cycalloc(sizeof(*_tmp9D7)),((
_tmp9D7->hd=(struct Cyc_Absyn_Exp*)_tmp3DE->hd,((_tmp9D7->tl=new_args,_tmp9D7))))));}}}{
struct Cyc_Absyn_Exp*_tmp9DA[3];struct Cyc_List_List*_tmp9D9;new_args=((_tmp9D9=
_cycalloc(sizeof(*_tmp9D9)),((_tmp9D9->hd=Cyc_Absyn_fncall_exp(Cyc_Toc__tag_dyneither_e,((
_tmp9DA[2]=num_varargs_exp,((_tmp9DA[1]=Cyc_Absyn_sizeoftyp_exp(cva_type,0),((
_tmp9DA[0]=argvexp,((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(
_tag_dyneither(_tmp9DA,sizeof(struct Cyc_Absyn_Exp*),3)))))))),0),((_tmp9D9->tl=
new_args,_tmp9D9))))));}new_args=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))
Cyc_List_imp_rev)(new_args);Cyc_Toc_exp_to_c(nv,_tmp3DD);{struct Cyc_Absyn_Stmt*s=
Cyc_Absyn_exp_stmt(Cyc_Absyn_fncall_exp(_tmp3DD,new_args,0),0);if(_tmp3E3->inject){
struct Cyc_Absyn_Tuniondecl*tud;{void*_tmp46A=Cyc_Tcutil_compress((void*)_tmp3E3->type);
struct Cyc_Absyn_TunionInfo _tmp46B;union Cyc_Absyn_TunionInfoU_union _tmp46C;struct
Cyc_Absyn_Tuniondecl**_tmp46D;struct Cyc_Absyn_Tuniondecl*_tmp46E;_LL28D: if(
_tmp46A <= (void*)4)goto _LL28F;if(*((int*)_tmp46A)!= 2)goto _LL28F;_tmp46B=((
struct Cyc_Absyn_TunionType_struct*)_tmp46A)->f1;_tmp46C=_tmp46B.tunion_info;if((((((
struct Cyc_Absyn_TunionType_struct*)_tmp46A)->f1).tunion_info).KnownTunion).tag != 
1)goto _LL28F;_tmp46D=(_tmp46C.KnownTunion).f1;_tmp46E=*_tmp46D;_LL28E: tud=
_tmp46E;goto _LL28C;_LL28F:;_LL290: {const char*_tmp9DD;void*_tmp9DC;(_tmp9DC=0,((
int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((
_tmp9DD="toc: unknown tunion in vararg with inject",_tag_dyneither(_tmp9DD,
sizeof(char),42))),_tag_dyneither(_tmp9DC,sizeof(void*),0)));}_LL28C:;}{
unsigned int _tmp9E2;unsigned int _tmp9E1;struct _dyneither_ptr _tmp9E0;struct
_tuple1**_tmp9DF;unsigned int _tmp9DE;struct _dyneither_ptr vs=(_tmp9DE=(
unsigned int)_tmp3E1,((_tmp9DF=(struct _tuple1**)_region_malloc(r,_check_times(
sizeof(struct _tuple1*),_tmp9DE)),((_tmp9E0=_tag_dyneither(_tmp9DF,sizeof(struct
_tuple1*),_tmp9DE),((((_tmp9E1=_tmp9DE,_tmp9E3(& _tmp9E2,& _tmp9E1,& _tmp9DF))),
_tmp9E0)))))));if(_tmp3E1 != 0){struct Cyc_List_List*_tmp471=0;{int i=_tmp3E1 - 1;
for(0;i >= 0;-- i){struct Cyc_List_List*_tmp9E4;_tmp471=((_tmp9E4=_cycalloc(sizeof(*
_tmp9E4)),((_tmp9E4->hd=Cyc_Toc_make_dle(Cyc_Absyn_address_exp(Cyc_Absyn_var_exp(*((
struct _tuple1**)_check_dyneither_subscript(vs,sizeof(struct _tuple1*),i)),0),0)),((
_tmp9E4->tl=_tmp471,_tmp9E4))))));}}s=Cyc_Absyn_declare_stmt(argv,arr_type,(
struct Cyc_Absyn_Exp*)Cyc_Absyn_unresolvedmem_exp(0,_tmp471,0),s,0);{int i=0;for(0;
_tmp3DE != 0;(((_tmp3DE=_tmp3DE->tl,_tmp3E2=_tmp3E2->tl)),++ i)){struct Cyc_Absyn_Exp*
arg=(struct Cyc_Absyn_Exp*)_tmp3DE->hd;void*arg_type=(void*)((struct Cyc_Core_Opt*)
_check_null(arg->topt))->v;struct _tuple1*var=*((struct _tuple1**)
_check_dyneither_subscript(vs,sizeof(struct _tuple1*),i));struct Cyc_Absyn_Exp*
varexp=Cyc_Absyn_var_exp(var,0);struct Cyc_Absyn_Tunionfield _tmp474;struct _tuple1*
_tmp475;struct Cyc_List_List*_tmp476;struct Cyc_Absyn_Tunionfield*_tmp473=(struct
Cyc_Absyn_Tunionfield*)((struct Cyc_List_List*)_check_null(_tmp3E2))->hd;_tmp474=*
_tmp473;_tmp475=_tmp474.name;_tmp476=_tmp474.typs;{void*field_typ=Cyc_Toc_typ_to_c((*((
struct _tuple4*)((struct Cyc_List_List*)_check_null(_tmp476))->hd)).f2);Cyc_Toc_exp_to_c(
nv,arg);if(Cyc_Toc_is_void_star(field_typ))arg=Cyc_Toc_cast_it(field_typ,arg);s=
Cyc_Absyn_seq_stmt(Cyc_Absyn_assign_stmt(Cyc_Absyn_aggrmember_exp(varexp,Cyc_Absyn_fieldname(
1),0),arg,0),s,0);s=Cyc_Absyn_seq_stmt(Cyc_Absyn_assign_stmt(Cyc_Absyn_aggrmember_exp(
varexp,Cyc_Toc_tag_sp,0),Cyc_Toc_tunion_tag(tud,_tmp475,1),0),s,0);{const char*
_tmp9E5;s=Cyc_Absyn_declare_stmt(var,Cyc_Absyn_strctq(Cyc_Toc_collapse_qvar_tag(
_tmp475,((_tmp9E5="_struct",_tag_dyneither(_tmp9E5,sizeof(char),8))))),0,s,0);}}}}}
else{struct _tuple8*_tmp9E6[3];struct Cyc_List_List*_tmp478=(_tmp9E6[2]=Cyc_Toc_make_dle(
Cyc_Absyn_uint_exp(0,0)),((_tmp9E6[1]=Cyc_Toc_make_dle(Cyc_Absyn_uint_exp(0,0)),((
_tmp9E6[0]=Cyc_Toc_make_dle(Cyc_Absyn_uint_exp(0,0)),((struct Cyc_List_List*(*)(
struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp9E6,sizeof(struct _tuple8*),
3)))))));s=Cyc_Absyn_declare_stmt(argv,Cyc_Absyn_void_star_typ(),(struct Cyc_Absyn_Exp*)
Cyc_Absyn_uint_exp(0,0),s,0);}}}else{{int i=0;for(0;_tmp3DE != 0;(_tmp3DE=_tmp3DE->tl,
++ i)){Cyc_Toc_exp_to_c(nv,(struct Cyc_Absyn_Exp*)_tmp3DE->hd);s=Cyc_Absyn_seq_stmt(
Cyc_Absyn_assign_stmt(Cyc_Absyn_subscript_exp(argvexp,Cyc_Absyn_uint_exp((
unsigned int)i,0),0),(struct Cyc_Absyn_Exp*)_tmp3DE->hd,0),s,0);}}s=Cyc_Absyn_declare_stmt(
argv,arr_type,0,s,0);}(void*)(e->r=(void*)Cyc_Toc_stmt_exp_r(s));}};_pop_region(
r);}goto _LL1E0;_LL1FD: if(*((int*)_tmp3C3)!= 12)goto _LL1FF;_tmp3E4=((struct Cyc_Absyn_Throw_e_struct*)
_tmp3C3)->f1;_LL1FE: Cyc_Toc_exp_to_c(nv,_tmp3E4);(void*)(e->r=(void*)((void*)(
Cyc_Toc_array_to_ptr_cast(Cyc_Toc_typ_to_c(old_typ),Cyc_Toc_newthrow_exp(_tmp3E4),
0))->r));goto _LL1E0;_LL1FF: if(*((int*)_tmp3C3)!= 13)goto _LL201;_tmp3E5=((struct
Cyc_Absyn_NoInstantiate_e_struct*)_tmp3C3)->f1;_LL200: Cyc_Toc_exp_to_c(nv,
_tmp3E5);goto _LL1E0;_LL201: if(*((int*)_tmp3C3)!= 14)goto _LL203;_tmp3E6=((struct
Cyc_Absyn_Instantiate_e_struct*)_tmp3C3)->f1;_tmp3E7=((struct Cyc_Absyn_Instantiate_e_struct*)
_tmp3C3)->f2;_LL202: Cyc_Toc_exp_to_c(nv,_tmp3E6);for(0;_tmp3E7 != 0;_tmp3E7=
_tmp3E7->tl){void*k=Cyc_Tcutil_typ_kind((void*)_tmp3E7->hd);if(((k != (void*)6
 && k != (void*)3) && k != (void*)4) && k != (void*)5){{void*_tmp47E=Cyc_Tcutil_compress((
void*)_tmp3E7->hd);_LL292: if(_tmp47E <= (void*)4)goto _LL296;if(*((int*)_tmp47E)!= 
1)goto _LL294;_LL293: goto _LL295;_LL294: if(*((int*)_tmp47E)!= 2)goto _LL296;_LL295:
continue;_LL296:;_LL297:(void*)(e->r=(void*)((void*)(Cyc_Toc_array_to_ptr_cast(
Cyc_Toc_typ_to_c((void*)((struct Cyc_Core_Opt*)_check_null(e->topt))->v),_tmp3E6,
0))->r));goto _LL291;_LL291:;}break;}}goto _LL1E0;_LL203: if(*((int*)_tmp3C3)!= 15)
goto _LL205;_tmp3E8=(void*)((struct Cyc_Absyn_Cast_e_struct*)_tmp3C3)->f1;_tmp3E9=(
void**)&((void*)((struct Cyc_Absyn_Cast_e_struct*)_tmp3C3)->f1);_tmp3EA=((struct
Cyc_Absyn_Cast_e_struct*)_tmp3C3)->f2;_tmp3EB=((struct Cyc_Absyn_Cast_e_struct*)
_tmp3C3)->f3;_tmp3EC=(void*)((struct Cyc_Absyn_Cast_e_struct*)_tmp3C3)->f4;_LL204: {
void*old_t2=(void*)((struct Cyc_Core_Opt*)_check_null(_tmp3EA->topt))->v;void*
new_typ=*_tmp3E9;*_tmp3E9=Cyc_Toc_typ_to_c(new_typ);Cyc_Toc_exp_to_c(nv,_tmp3EA);{
struct _tuple0 _tmp9E7;struct _tuple0 _tmp480=(_tmp9E7.f1=Cyc_Tcutil_compress(old_t2),((
_tmp9E7.f2=Cyc_Tcutil_compress(new_typ),_tmp9E7)));void*_tmp481;struct Cyc_Absyn_PtrInfo
_tmp482;void*_tmp483;struct Cyc_Absyn_PtrInfo _tmp484;void*_tmp485;struct Cyc_Absyn_PtrInfo
_tmp486;void*_tmp487;_LL299: _tmp481=_tmp480.f1;if(_tmp481 <= (void*)4)goto _LL29B;
if(*((int*)_tmp481)!= 4)goto _LL29B;_tmp482=((struct Cyc_Absyn_PointerType_struct*)
_tmp481)->f1;_tmp483=_tmp480.f2;if(_tmp483 <= (void*)4)goto _LL29B;if(*((int*)
_tmp483)!= 4)goto _LL29B;_tmp484=((struct Cyc_Absyn_PointerType_struct*)_tmp483)->f1;
_LL29A: {int _tmp488=((int(*)(int,struct Cyc_Absyn_Conref*x))Cyc_Absyn_conref_def)(
0,(_tmp482.ptr_atts).nullable);int _tmp489=((int(*)(int,struct Cyc_Absyn_Conref*x))
Cyc_Absyn_conref_def)(0,(_tmp484.ptr_atts).nullable);void*_tmp48A=Cyc_Absyn_conref_def(
Cyc_Absyn_bounds_one,(_tmp482.ptr_atts).bounds);void*_tmp48B=Cyc_Absyn_conref_def(
Cyc_Absyn_bounds_one,(_tmp484.ptr_atts).bounds);int _tmp48C=((int(*)(int,struct
Cyc_Absyn_Conref*x))Cyc_Absyn_conref_def)(0,(_tmp482.ptr_atts).zero_term);int
_tmp48D=((int(*)(int,struct Cyc_Absyn_Conref*x))Cyc_Absyn_conref_def)(0,(_tmp484.ptr_atts).zero_term);{
struct _tuple0 _tmp9E8;struct _tuple0 _tmp48F=(_tmp9E8.f1=_tmp48A,((_tmp9E8.f2=
_tmp48B,_tmp9E8)));void*_tmp490;struct Cyc_Absyn_Exp*_tmp491;void*_tmp492;struct
Cyc_Absyn_Exp*_tmp493;void*_tmp494;struct Cyc_Absyn_Exp*_tmp495;void*_tmp496;void*
_tmp497;void*_tmp498;struct Cyc_Absyn_Exp*_tmp499;void*_tmp49A;void*_tmp49B;
_LL2A0: _tmp490=_tmp48F.f1;if(_tmp490 <= (void*)1)goto _LL2A2;if(*((int*)_tmp490)!= 
0)goto _LL2A2;_tmp491=((struct Cyc_Absyn_Upper_b_struct*)_tmp490)->f1;_tmp492=
_tmp48F.f2;if(_tmp492 <= (void*)1)goto _LL2A2;if(*((int*)_tmp492)!= 0)goto _LL2A2;
_tmp493=((struct Cyc_Absyn_Upper_b_struct*)_tmp492)->f1;_LL2A1: if((!Cyc_Evexp_c_can_eval(
_tmp491) || !Cyc_Evexp_c_can_eval(_tmp493)) && !Cyc_Evexp_same_const_exp(_tmp491,
_tmp493)){const char*_tmp9EB;void*_tmp9EA;(_tmp9EA=0,Cyc_Tcutil_terr(e->loc,((
_tmp9EB="can't validate cast due to potential size differences",_tag_dyneither(
_tmp9EB,sizeof(char),54))),_tag_dyneither(_tmp9EA,sizeof(void*),0)));}if(_tmp488
 && !_tmp489){if(Cyc_Toc_is_toplevel(nv)){const char*_tmp9EE;void*_tmp9ED;(
_tmp9ED=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_unimp)(((
_tmp9EE="can't do NULL-check conversion at top-level",_tag_dyneither(_tmp9EE,
sizeof(char),44))),_tag_dyneither(_tmp9ED,sizeof(void*),0)));}if(_tmp3EC != (void*)
2){const char*_tmp9F2;void*_tmp9F1[1];struct Cyc_String_pa_struct _tmp9F0;(_tmp9F0.tag=
0,((_tmp9F0.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_exp2string(
e)),((_tmp9F1[0]=& _tmp9F0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr
ap))Cyc_Tcutil_impos)(((_tmp9F2="null-check conversion mis-classified: %s",
_tag_dyneither(_tmp9F2,sizeof(char),41))),_tag_dyneither(_tmp9F1,sizeof(void*),1)))))));}{
int do_null_check=Cyc_Toc_need_null_check(_tmp3EA);if(do_null_check){if(!_tmp3EB){
const char*_tmp9F5;void*_tmp9F4;(_tmp9F4=0,Cyc_Tcutil_warn(e->loc,((_tmp9F5="inserted null check due to implicit cast from * to @ type",
_tag_dyneither(_tmp9F5,sizeof(char),58))),_tag_dyneither(_tmp9F4,sizeof(void*),0)));}{
struct Cyc_List_List*_tmp9F6;(void*)(e->r=(void*)Cyc_Toc_cast_it_r(*_tmp3E9,Cyc_Absyn_fncall_exp(
Cyc_Toc__check_null_e,((_tmp9F6=_cycalloc(sizeof(*_tmp9F6)),((_tmp9F6->hd=
_tmp3EA,((_tmp9F6->tl=0,_tmp9F6)))))),0)));}}}}goto _LL29F;_LL2A2: _tmp494=_tmp48F.f1;
if(_tmp494 <= (void*)1)goto _LL2A4;if(*((int*)_tmp494)!= 0)goto _LL2A4;_tmp495=((
struct Cyc_Absyn_Upper_b_struct*)_tmp494)->f1;_tmp496=_tmp48F.f2;if((int)_tmp496
!= 0)goto _LL2A4;_LL2A3: if(!Cyc_Evexp_c_can_eval(_tmp495)){const char*_tmp9F9;void*
_tmp9F8;(_tmp9F8=0,Cyc_Tcutil_terr(e->loc,((_tmp9F9="cannot perform coercion since numelts cannot be determined statically.",
_tag_dyneither(_tmp9F9,sizeof(char),71))),_tag_dyneither(_tmp9F8,sizeof(void*),0)));}
if(_tmp3EC == (void*)2){const char*_tmp9FD;void*_tmp9FC[1];struct Cyc_String_pa_struct
_tmp9FB;(_tmp9FB.tag=0,((_tmp9FB.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Absynpp_exp2string(e)),((_tmp9FC[0]=& _tmp9FB,((int(*)(struct _dyneither_ptr
fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp9FD="conversion mis-classified as null-check: %s",
_tag_dyneither(_tmp9FD,sizeof(char),44))),_tag_dyneither(_tmp9FC,sizeof(void*),1)))))));}
if(Cyc_Toc_is_toplevel(nv)){if((_tmp48C  && !(_tmp484.elt_tq).real_const) && !
_tmp48D)_tmp495=Cyc_Absyn_prim2_exp((void*)2,_tmp495,Cyc_Absyn_uint_exp(1,0),0);(
void*)(e->r=(void*)((void*)(Cyc_Toc_make_toplevel_dyn_arr(old_t2,_tmp495,_tmp3EA))->r));}
else{struct Cyc_Absyn_Exp*_tmp4AB=Cyc_Toc__tag_dyneither_e;if(_tmp48C){struct
_tuple1*_tmp4AC=Cyc_Toc_temp_var();struct Cyc_Absyn_Exp*_tmp4AD=Cyc_Absyn_var_exp(
_tmp4AC,0);struct Cyc_Absyn_Exp*arg3;{void*_tmp4AE=(void*)_tmp3EA->r;union Cyc_Absyn_Cnst_union
_tmp4AF;_LL2A9: if(*((int*)_tmp4AE)!= 0)goto _LL2AB;_tmp4AF=((struct Cyc_Absyn_Const_e_struct*)
_tmp4AE)->f1;if(((((struct Cyc_Absyn_Const_e_struct*)_tmp4AE)->f1).String_c).tag
!= 5)goto _LL2AB;_LL2AA: arg3=_tmp495;goto _LL2A8;_LL2AB:;_LL2AC:{struct Cyc_Absyn_Exp*
_tmp9FE[2];arg3=Cyc_Absyn_fncall_exp(Cyc_Toc_getFunctionRemovePointer(& Cyc_Toc__get_zero_arr_size_functionSet,
_tmp3EA),((_tmp9FE[1]=_tmp495,((_tmp9FE[0]=_tmp4AD,((struct Cyc_List_List*(*)(
struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp9FE,sizeof(struct Cyc_Absyn_Exp*),
2)))))),0);}goto _LL2A8;_LL2A8:;}if(!_tmp48D  && !(_tmp484.elt_tq).real_const)arg3=
Cyc_Absyn_prim2_exp((void*)2,arg3,Cyc_Absyn_uint_exp(1,0),0);{struct Cyc_Absyn_Exp*
_tmp4B1=Cyc_Absyn_sizeoftyp_exp(Cyc_Toc_typ_to_c((void*)_tmp484.elt_typ),0);
struct Cyc_Absyn_Exp*_tmp9FF[3];struct Cyc_Absyn_Exp*_tmp4B2=Cyc_Absyn_fncall_exp(
_tmp4AB,((_tmp9FF[2]=arg3,((_tmp9FF[1]=_tmp4B1,((_tmp9FF[0]=_tmp4AD,((struct Cyc_List_List*(*)(
struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp9FF,sizeof(struct Cyc_Absyn_Exp*),
3)))))))),0);struct Cyc_Absyn_Stmt*_tmp4B3=Cyc_Absyn_exp_stmt(_tmp4B2,0);_tmp4B3=
Cyc_Absyn_declare_stmt(_tmp4AC,Cyc_Toc_typ_to_c(old_t2),(struct Cyc_Absyn_Exp*)
_tmp3EA,_tmp4B3,0);(void*)(e->r=(void*)Cyc_Toc_stmt_exp_r(_tmp4B3));}}else{
struct Cyc_Absyn_Exp*_tmpA00[3];(void*)(e->r=(void*)Cyc_Toc_fncall_exp_r(_tmp4AB,((
_tmpA00[2]=_tmp495,((_tmpA00[1]=Cyc_Absyn_sizeoftyp_exp(Cyc_Toc_typ_to_c((void*)
_tmp484.elt_typ),0),((_tmpA00[0]=_tmp3EA,((struct Cyc_List_List*(*)(struct
_dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmpA00,sizeof(struct Cyc_Absyn_Exp*),
3))))))))));}}goto _LL29F;_LL2A4: _tmp497=_tmp48F.f1;if((int)_tmp497 != 0)goto
_LL2A6;_tmp498=_tmp48F.f2;if(_tmp498 <= (void*)1)goto _LL2A6;if(*((int*)_tmp498)!= 
0)goto _LL2A6;_tmp499=((struct Cyc_Absyn_Upper_b_struct*)_tmp498)->f1;_LL2A5: if(!
Cyc_Evexp_c_can_eval(_tmp499)){const char*_tmpA03;void*_tmpA02;(_tmpA02=0,Cyc_Tcutil_terr(
e->loc,((_tmpA03="cannot perform coercion since numelts cannot be determined statically.",
_tag_dyneither(_tmpA03,sizeof(char),71))),_tag_dyneither(_tmpA02,sizeof(void*),0)));}
if(Cyc_Toc_is_toplevel(nv)){const char*_tmpA06;void*_tmpA05;(_tmpA05=0,((int(*)(
struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_unimp)(((_tmpA06="can't coerce t? to t* or t@ at the top-level",
_tag_dyneither(_tmpA06,sizeof(char),45))),_tag_dyneither(_tmpA05,sizeof(void*),0)));}{
struct Cyc_Absyn_Exp*_tmp4BA=_tmp499;if(_tmp48C  && !_tmp48D)_tmp4BA=Cyc_Absyn_add_exp(
_tmp499,Cyc_Absyn_uint_exp(1,0),0);{struct Cyc_Absyn_Exp*_tmp4BB=Cyc_Toc__untag_dyneither_ptr_e;
struct Cyc_Absyn_Exp*_tmpA07[3];struct Cyc_Absyn_Exp*_tmp4BC=Cyc_Absyn_fncall_exp(
_tmp4BB,((_tmpA07[2]=_tmp4BA,((_tmpA07[1]=Cyc_Absyn_sizeoftyp_exp(Cyc_Toc_typ_to_c((
void*)_tmp482.elt_typ),0),((_tmpA07[0]=_tmp3EA,((struct Cyc_List_List*(*)(struct
_dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmpA07,sizeof(struct Cyc_Absyn_Exp*),
3)))))))),0);if(_tmp489){struct Cyc_List_List*_tmpA08;(void*)(_tmp4BC->r=(void*)
Cyc_Toc_fncall_exp_r(Cyc_Toc__check_null_e,((_tmpA08=_cycalloc(sizeof(*_tmpA08)),((
_tmpA08->hd=Cyc_Absyn_copy_exp(_tmp4BC),((_tmpA08->tl=0,_tmpA08))))))));}(void*)(
e->r=(void*)Cyc_Toc_cast_it_r(*_tmp3E9,_tmp4BC));goto _LL29F;}}_LL2A6: _tmp49A=
_tmp48F.f1;if((int)_tmp49A != 0)goto _LL29F;_tmp49B=_tmp48F.f2;if((int)_tmp49B != 0)
goto _LL29F;_LL2A7: DynCast: if((_tmp48C  && !_tmp48D) && !(_tmp484.elt_tq).real_const){
if(Cyc_Toc_is_toplevel(nv)){const char*_tmpA0B;void*_tmpA0A;(_tmpA0A=0,((int(*)(
struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_unimp)(((_tmpA0B="can't coerce a ZEROTERM to a non-const NOZEROTERM pointer at toplevel",
_tag_dyneither(_tmpA0B,sizeof(char),70))),_tag_dyneither(_tmpA0A,sizeof(void*),0)));}{
struct Cyc_Absyn_Exp*_tmp4C1=Cyc_Toc__dyneither_ptr_decrease_size_e;struct Cyc_Absyn_Exp*
_tmpA0C[3];(void*)(e->r=(void*)Cyc_Toc_fncall_exp_r(_tmp4C1,((_tmpA0C[2]=Cyc_Absyn_uint_exp(
1,0),((_tmpA0C[1]=Cyc_Absyn_sizeoftyp_exp(Cyc_Toc_typ_to_c((void*)_tmp482.elt_typ),
0),((_tmpA0C[0]=_tmp3EA,((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(
_tag_dyneither(_tmpA0C,sizeof(struct Cyc_Absyn_Exp*),3))))))))));}}goto _LL29F;
_LL29F:;}goto _LL298;}_LL29B: _tmp485=_tmp480.f1;if(_tmp485 <= (void*)4)goto _LL29D;
if(*((int*)_tmp485)!= 4)goto _LL29D;_tmp486=((struct Cyc_Absyn_PointerType_struct*)
_tmp485)->f1;_tmp487=_tmp480.f2;if(_tmp487 <= (void*)4)goto _LL29D;if(*((int*)
_tmp487)!= 5)goto _LL29D;_LL29C:{void*_tmp4C3=Cyc_Absyn_conref_def(Cyc_Absyn_bounds_one,(
_tmp486.ptr_atts).bounds);_LL2AE: if((int)_tmp4C3 != 0)goto _LL2B0;_LL2AF:(void*)(
_tmp3EA->r=(void*)Cyc_Toc_aggrmember_exp_r(Cyc_Absyn_new_exp((void*)_tmp3EA->r,
_tmp3EA->loc),Cyc_Toc_curr_sp));goto _LL2AD;_LL2B0:;_LL2B1: goto _LL2AD;_LL2AD:;}
goto _LL298;_LL29D:;_LL29E: goto _LL298;_LL298:;}goto _LL1E0;}_LL205: if(*((int*)
_tmp3C3)!= 16)goto _LL207;_tmp3ED=((struct Cyc_Absyn_Address_e_struct*)_tmp3C3)->f1;
_LL206:{void*_tmp4C4=(void*)_tmp3ED->r;struct _tuple1*_tmp4C5;struct Cyc_List_List*
_tmp4C6;struct Cyc_List_List*_tmp4C7;struct Cyc_List_List*_tmp4C8;_LL2B3: if(*((int*)
_tmp4C4)!= 30)goto _LL2B5;_tmp4C5=((struct Cyc_Absyn_Struct_e_struct*)_tmp4C4)->f1;
_tmp4C6=((struct Cyc_Absyn_Struct_e_struct*)_tmp4C4)->f2;_tmp4C7=((struct Cyc_Absyn_Struct_e_struct*)
_tmp4C4)->f3;_LL2B4: if(Cyc_Toc_is_toplevel(nv)){const char*_tmpA10;void*_tmpA0F[1];
struct Cyc_String_pa_struct _tmpA0E;(_tmpA0E.tag=0,((_tmpA0E.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)Cyc_Position_string_of_segment(_tmp3ED->loc)),((
_tmpA0F[0]=& _tmpA0E,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_unimp)(((
_tmpA10="%s: & on non-identifiers at the top-level",_tag_dyneither(_tmpA10,
sizeof(char),42))),_tag_dyneither(_tmpA0F,sizeof(void*),1)))))));}(void*)(e->r=(
void*)((void*)(Cyc_Toc_init_struct(nv,(void*)((struct Cyc_Core_Opt*)_check_null(
_tmp3ED->topt))->v,_tmp4C6 != 0,1,0,_tmp4C7,_tmp4C5))->r));goto _LL2B2;_LL2B5: if(*((
int*)_tmp4C4)!= 26)goto _LL2B7;_tmp4C8=((struct Cyc_Absyn_Tuple_e_struct*)_tmp4C4)->f1;
_LL2B6: if(Cyc_Toc_is_toplevel(nv)){const char*_tmpA14;void*_tmpA13[1];struct Cyc_String_pa_struct
_tmpA12;(_tmpA12.tag=0,((_tmpA12.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Position_string_of_segment(_tmp3ED->loc)),((_tmpA13[0]=& _tmpA12,((int(*)(
struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_unimp)(((_tmpA14="%s: & on non-identifiers at the top-level",
_tag_dyneither(_tmpA14,sizeof(char),42))),_tag_dyneither(_tmpA13,sizeof(void*),1)))))));}(
void*)(e->r=(void*)((void*)(Cyc_Toc_init_tuple(nv,1,0,_tmp4C8))->r));goto _LL2B2;
_LL2B7:;_LL2B8: Cyc_Toc_exp_to_c(nv,_tmp3ED);if(!Cyc_Absyn_is_lvalue(_tmp3ED)){((
void(*)(struct Cyc_Absyn_Exp*e1,struct Cyc_List_List*fs,struct Cyc_Absyn_Exp*(*f)(
struct Cyc_Absyn_Exp*,int),int f_env))Cyc_Toc_lvalue_assign)(_tmp3ED,0,Cyc_Toc_address_lvalue,
1);(void*)(e->r=(void*)Cyc_Toc_cast_it_r(Cyc_Toc_typ_to_c((void*)((struct Cyc_Core_Opt*)
_check_null(e->topt))->v),_tmp3ED));}goto _LL2B2;_LL2B2:;}goto _LL1E0;_LL207: if(*((
int*)_tmp3C3)!= 17)goto _LL209;_tmp3EE=((struct Cyc_Absyn_New_e_struct*)_tmp3C3)->f1;
_tmp3EF=((struct Cyc_Absyn_New_e_struct*)_tmp3C3)->f2;_LL208: if(Cyc_Toc_is_toplevel(
nv)){const char*_tmpA18;void*_tmpA17[1];struct Cyc_String_pa_struct _tmpA16;(
_tmpA16.tag=0,((_tmpA16.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Position_string_of_segment(
_tmp3EF->loc)),((_tmpA17[0]=& _tmpA16,((int(*)(struct _dyneither_ptr fmt,struct
_dyneither_ptr ap))Cyc_Toc_unimp)(((_tmpA18="%s: new at top-level",_tag_dyneither(
_tmpA18,sizeof(char),21))),_tag_dyneither(_tmpA17,sizeof(void*),1)))))));}{void*
_tmp4D2=(void*)_tmp3EF->r;struct Cyc_List_List*_tmp4D3;struct Cyc_Absyn_Vardecl*
_tmp4D4;struct Cyc_Absyn_Exp*_tmp4D5;struct Cyc_Absyn_Exp*_tmp4D6;int _tmp4D7;
struct _tuple1*_tmp4D8;struct Cyc_List_List*_tmp4D9;struct Cyc_List_List*_tmp4DA;
struct Cyc_List_List*_tmp4DB;_LL2BA: if(*((int*)_tmp4D2)!= 28)goto _LL2BC;_tmp4D3=((
struct Cyc_Absyn_Array_e_struct*)_tmp4D2)->f1;_LL2BB: {struct _tuple1*_tmp4DC=Cyc_Toc_temp_var();
struct Cyc_Absyn_Exp*_tmp4DD=Cyc_Absyn_var_exp(_tmp4DC,0);struct Cyc_Absyn_Stmt*
_tmp4DE=Cyc_Toc_init_array(nv,_tmp4DD,_tmp4D3,Cyc_Absyn_exp_stmt(_tmp4DD,0));
void*old_elt_typ;{void*_tmp4DF=Cyc_Tcutil_compress(old_typ);struct Cyc_Absyn_PtrInfo
_tmp4E0;void*_tmp4E1;struct Cyc_Absyn_Tqual _tmp4E2;struct Cyc_Absyn_PtrAtts _tmp4E3;
struct Cyc_Absyn_Conref*_tmp4E4;_LL2C5: if(_tmp4DF <= (void*)4)goto _LL2C7;if(*((int*)
_tmp4DF)!= 4)goto _LL2C7;_tmp4E0=((struct Cyc_Absyn_PointerType_struct*)_tmp4DF)->f1;
_tmp4E1=(void*)_tmp4E0.elt_typ;_tmp4E2=_tmp4E0.elt_tq;_tmp4E3=_tmp4E0.ptr_atts;
_tmp4E4=_tmp4E3.zero_term;_LL2C6: old_elt_typ=_tmp4E1;goto _LL2C4;_LL2C7:;_LL2C8: {
const char*_tmpA1B;void*_tmpA1A;old_elt_typ=((_tmpA1A=0,Cyc_Toc_toc_impos(((
_tmpA1B="exp_to_c:new array expression doesn't have ptr type",_tag_dyneither(
_tmpA1B,sizeof(char),52))),_tag_dyneither(_tmpA1A,sizeof(void*),0))));}_LL2C4:;}{
void*elt_typ=Cyc_Toc_typ_to_c(old_elt_typ);void*_tmp4E7=Cyc_Absyn_cstar_typ(
elt_typ,Cyc_Toc_mt_tq);struct Cyc_Absyn_Exp*_tmp4E8=Cyc_Absyn_times_exp(Cyc_Absyn_sizeoftyp_exp(
elt_typ,0),Cyc_Absyn_signed_int_exp(((int(*)(struct Cyc_List_List*x))Cyc_List_length)(
_tmp4D3),0),0);struct Cyc_Absyn_Exp*e1;if(_tmp3EE == 0  || Cyc_Absyn_no_regions)e1=
Cyc_Toc_malloc_exp(old_elt_typ,_tmp4E8);else{struct Cyc_Absyn_Exp*r=(struct Cyc_Absyn_Exp*)
_tmp3EE;Cyc_Toc_exp_to_c(nv,r);e1=Cyc_Toc_rmalloc_exp(r,_tmp4E8);}(void*)(e->r=(
void*)Cyc_Toc_stmt_exp_r(Cyc_Absyn_declare_stmt(_tmp4DC,_tmp4E7,(struct Cyc_Absyn_Exp*)
e1,_tmp4DE,0)));goto _LL2B9;}}_LL2BC: if(*((int*)_tmp4D2)!= 29)goto _LL2BE;_tmp4D4=((
struct Cyc_Absyn_Comprehension_e_struct*)_tmp4D2)->f1;_tmp4D5=((struct Cyc_Absyn_Comprehension_e_struct*)
_tmp4D2)->f2;_tmp4D6=((struct Cyc_Absyn_Comprehension_e_struct*)_tmp4D2)->f3;
_tmp4D7=((struct Cyc_Absyn_Comprehension_e_struct*)_tmp4D2)->f4;_LL2BD: {int
is_dyneither_ptr=0;{void*_tmp4E9=Cyc_Tcutil_compress(old_typ);struct Cyc_Absyn_PtrInfo
_tmp4EA;void*_tmp4EB;struct Cyc_Absyn_Tqual _tmp4EC;struct Cyc_Absyn_PtrAtts _tmp4ED;
struct Cyc_Absyn_Conref*_tmp4EE;struct Cyc_Absyn_Conref*_tmp4EF;_LL2CA: if(_tmp4E9
<= (void*)4)goto _LL2CC;if(*((int*)_tmp4E9)!= 4)goto _LL2CC;_tmp4EA=((struct Cyc_Absyn_PointerType_struct*)
_tmp4E9)->f1;_tmp4EB=(void*)_tmp4EA.elt_typ;_tmp4EC=_tmp4EA.elt_tq;_tmp4ED=
_tmp4EA.ptr_atts;_tmp4EE=_tmp4ED.bounds;_tmp4EF=_tmp4ED.zero_term;_LL2CB:
is_dyneither_ptr=Cyc_Absyn_conref_def(Cyc_Absyn_bounds_one,_tmp4EE)== (void*)0;
goto _LL2C9;_LL2CC:;_LL2CD: {const char*_tmpA1E;void*_tmpA1D;(_tmpA1D=0,((int(*)(
struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmpA1E="exp_to_c: comprehension not an array type",
_tag_dyneither(_tmpA1E,sizeof(char),42))),_tag_dyneither(_tmpA1D,sizeof(void*),0)));}
_LL2C9:;}{struct _tuple1*max=Cyc_Toc_temp_var();struct _tuple1*a=Cyc_Toc_temp_var();
void*old_elt_typ=(void*)((struct Cyc_Core_Opt*)_check_null(_tmp4D6->topt))->v;
void*elt_typ=Cyc_Toc_typ_to_c(old_elt_typ);void*ptr_typ=Cyc_Absyn_cstar_typ(
elt_typ,Cyc_Toc_mt_tq);Cyc_Toc_exp_to_c(nv,_tmp4D5);{struct Cyc_Absyn_Exp*_tmp4F2=
Cyc_Absyn_var_exp(max,0);if(_tmp4D7)_tmp4F2=Cyc_Absyn_add_exp(_tmp4F2,Cyc_Absyn_uint_exp(
1,0),0);{struct Cyc_Absyn_Stmt*s=Cyc_Toc_init_comprehension(nv,Cyc_Absyn_var_exp(
a,0),_tmp4D4,Cyc_Absyn_var_exp(max,0),_tmp4D6,_tmp4D7,Cyc_Toc_skip_stmt_dl(),1);{
struct _RegionHandle _tmp4F3=_new_region("r");struct _RegionHandle*r=& _tmp4F3;
_push_region(r);{struct _tuple11*_tmpA21;struct Cyc_List_List*_tmpA20;struct Cyc_List_List*
decls=(_tmpA20=_region_malloc(r,sizeof(*_tmpA20)),((_tmpA20->hd=((_tmpA21=
_region_malloc(r,sizeof(*_tmpA21)),((_tmpA21->f1=max,((_tmpA21->f2=Cyc_Absyn_uint_typ,((
_tmpA21->f3=(struct Cyc_Absyn_Exp*)_tmp4D5,_tmpA21)))))))),((_tmpA20->tl=0,
_tmpA20)))));struct Cyc_Absyn_Exp*ai;if(_tmp3EE == 0  || Cyc_Absyn_no_regions){
struct Cyc_Absyn_Exp*_tmpA22[2];ai=Cyc_Toc_malloc_exp(old_elt_typ,Cyc_Absyn_fncall_exp(
Cyc_Toc__check_times_e,((_tmpA22[1]=_tmp4F2,((_tmpA22[0]=Cyc_Absyn_sizeoftyp_exp(
elt_typ,0),((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(
_tag_dyneither(_tmpA22,sizeof(struct Cyc_Absyn_Exp*),2)))))),0));}else{struct Cyc_Absyn_Exp*
r=(struct Cyc_Absyn_Exp*)_tmp3EE;Cyc_Toc_exp_to_c(nv,r);{struct Cyc_Absyn_Exp*
_tmpA23[2];ai=Cyc_Toc_rmalloc_exp(r,Cyc_Absyn_fncall_exp(Cyc_Toc__check_times_e,((
_tmpA23[1]=_tmp4F2,((_tmpA23[0]=Cyc_Absyn_sizeoftyp_exp(elt_typ,0),((struct Cyc_List_List*(*)(
struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmpA23,sizeof(struct Cyc_Absyn_Exp*),
2)))))),0));}}{struct Cyc_Absyn_Exp*ainit=Cyc_Toc_cast_it(ptr_typ,ai);{struct
_tuple11*_tmpA26;struct Cyc_List_List*_tmpA25;decls=((_tmpA25=_region_malloc(r,
sizeof(*_tmpA25)),((_tmpA25->hd=((_tmpA26=_region_malloc(r,sizeof(*_tmpA26)),((
_tmpA26->f1=a,((_tmpA26->f2=ptr_typ,((_tmpA26->f3=(struct Cyc_Absyn_Exp*)ainit,
_tmpA26)))))))),((_tmpA25->tl=decls,_tmpA25))))));}if(is_dyneither_ptr){struct
_tuple1*_tmp4F8=Cyc_Toc_temp_var();void*_tmp4F9=Cyc_Toc_typ_to_c(old_typ);struct
Cyc_Absyn_Exp*_tmp4FA=Cyc_Toc__tag_dyneither_e;struct Cyc_Absyn_Exp*_tmpA27[3];
struct Cyc_Absyn_Exp*_tmp4FB=Cyc_Absyn_fncall_exp(_tmp4FA,((_tmpA27[2]=_tmp4F2,((
_tmpA27[1]=Cyc_Absyn_sizeoftyp_exp(elt_typ,0),((_tmpA27[0]=Cyc_Absyn_var_exp(a,0),((
struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(
_tmpA27,sizeof(struct Cyc_Absyn_Exp*),3)))))))),0);{struct _tuple11*_tmpA2A;struct
Cyc_List_List*_tmpA29;decls=((_tmpA29=_region_malloc(r,sizeof(*_tmpA29)),((
_tmpA29->hd=((_tmpA2A=_region_malloc(r,sizeof(*_tmpA2A)),((_tmpA2A->f1=_tmp4F8,((
_tmpA2A->f2=_tmp4F9,((_tmpA2A->f3=(struct Cyc_Absyn_Exp*)_tmp4FB,_tmpA2A)))))))),((
_tmpA29->tl=decls,_tmpA29))))));}s=Cyc_Absyn_seq_stmt(s,Cyc_Absyn_exp_stmt(Cyc_Absyn_var_exp(
_tmp4F8,0),0),0);}else{s=Cyc_Absyn_seq_stmt(s,Cyc_Absyn_exp_stmt(Cyc_Absyn_var_exp(
a,0),0),0);}{struct Cyc_List_List*_tmp4FF=decls;for(0;_tmp4FF != 0;_tmp4FF=_tmp4FF->tl){
struct _tuple1*_tmp501;void*_tmp502;struct Cyc_Absyn_Exp*_tmp503;struct _tuple11
_tmp500=*((struct _tuple11*)_tmp4FF->hd);_tmp501=_tmp500.f1;_tmp502=_tmp500.f2;
_tmp503=_tmp500.f3;s=Cyc_Absyn_declare_stmt(_tmp501,_tmp502,_tmp503,s,0);}}(void*)(
e->r=(void*)Cyc_Toc_stmt_exp_r(s));}};_pop_region(r);}goto _LL2B9;}}}}_LL2BE: if(*((
int*)_tmp4D2)!= 30)goto _LL2C0;_tmp4D8=((struct Cyc_Absyn_Struct_e_struct*)_tmp4D2)->f1;
_tmp4D9=((struct Cyc_Absyn_Struct_e_struct*)_tmp4D2)->f2;_tmp4DA=((struct Cyc_Absyn_Struct_e_struct*)
_tmp4D2)->f3;_LL2BF:(void*)(e->r=(void*)((void*)(Cyc_Toc_init_struct(nv,(void*)((
struct Cyc_Core_Opt*)_check_null(_tmp3EF->topt))->v,_tmp4D9 != 0,1,_tmp3EE,_tmp4DA,
_tmp4D8))->r));goto _LL2B9;_LL2C0: if(*((int*)_tmp4D2)!= 26)goto _LL2C2;_tmp4DB=((
struct Cyc_Absyn_Tuple_e_struct*)_tmp4D2)->f1;_LL2C1:(void*)(e->r=(void*)((void*)(
Cyc_Toc_init_tuple(nv,1,_tmp3EE,_tmp4DB))->r));goto _LL2B9;_LL2C2:;_LL2C3: {void*
old_elt_typ=(void*)((struct Cyc_Core_Opt*)_check_null(_tmp3EF->topt))->v;void*
elt_typ=Cyc_Toc_typ_to_c(old_elt_typ);struct _tuple1*_tmp506=Cyc_Toc_temp_var();
struct Cyc_Absyn_Exp*_tmp507=Cyc_Absyn_var_exp(_tmp506,0);struct Cyc_Absyn_Exp*
mexp=Cyc_Absyn_sizeofexp_exp(Cyc_Absyn_deref_exp(_tmp507,0),0);struct Cyc_Absyn_Exp*
inner_mexp=mexp;if(_tmp3EE == 0  || Cyc_Absyn_no_regions)mexp=Cyc_Toc_malloc_exp(
old_elt_typ,mexp);else{struct Cyc_Absyn_Exp*r=(struct Cyc_Absyn_Exp*)_tmp3EE;Cyc_Toc_exp_to_c(
nv,r);mexp=Cyc_Toc_rmalloc_exp(r,mexp);}{int done=0;{void*_tmp508=(void*)_tmp3EF->r;
void*_tmp509;struct Cyc_Absyn_Exp*_tmp50A;_LL2CF: if(*((int*)_tmp508)!= 15)goto
_LL2D1;_tmp509=(void*)((struct Cyc_Absyn_Cast_e_struct*)_tmp508)->f1;_tmp50A=((
struct Cyc_Absyn_Cast_e_struct*)_tmp508)->f2;_LL2D0:{struct _tuple0 _tmpA2B;struct
_tuple0 _tmp50C=(_tmpA2B.f1=Cyc_Tcutil_compress(_tmp509),((_tmpA2B.f2=Cyc_Tcutil_compress((
void*)((struct Cyc_Core_Opt*)_check_null(_tmp50A->topt))->v),_tmpA2B)));void*
_tmp50D;struct Cyc_Absyn_PtrInfo _tmp50E;void*_tmp50F;struct Cyc_Absyn_PtrAtts
_tmp510;struct Cyc_Absyn_Conref*_tmp511;void*_tmp512;struct Cyc_Absyn_PtrInfo
_tmp513;struct Cyc_Absyn_PtrAtts _tmp514;struct Cyc_Absyn_Conref*_tmp515;_LL2D4:
_tmp50D=_tmp50C.f1;if(_tmp50D <= (void*)4)goto _LL2D6;if(*((int*)_tmp50D)!= 4)goto
_LL2D6;_tmp50E=((struct Cyc_Absyn_PointerType_struct*)_tmp50D)->f1;_tmp50F=(void*)
_tmp50E.elt_typ;_tmp510=_tmp50E.ptr_atts;_tmp511=_tmp510.bounds;_tmp512=_tmp50C.f2;
if(_tmp512 <= (void*)4)goto _LL2D6;if(*((int*)_tmp512)!= 4)goto _LL2D6;_tmp513=((
struct Cyc_Absyn_PointerType_struct*)_tmp512)->f1;_tmp514=_tmp513.ptr_atts;
_tmp515=_tmp514.bounds;_LL2D5:{struct _tuple0 _tmpA2C;struct _tuple0 _tmp517=(
_tmpA2C.f1=Cyc_Absyn_conref_def(Cyc_Absyn_bounds_one,_tmp511),((_tmpA2C.f2=Cyc_Absyn_conref_def(
Cyc_Absyn_bounds_one,_tmp515),_tmpA2C)));void*_tmp518;void*_tmp519;struct Cyc_Absyn_Exp*
_tmp51A;_LL2D9: _tmp518=_tmp517.f1;if((int)_tmp518 != 0)goto _LL2DB;_tmp519=_tmp517.f2;
if(_tmp519 <= (void*)1)goto _LL2DB;if(*((int*)_tmp519)!= 0)goto _LL2DB;_tmp51A=((
struct Cyc_Absyn_Upper_b_struct*)_tmp519)->f1;_LL2DA: Cyc_Toc_exp_to_c(nv,_tmp50A);(
void*)(inner_mexp->r=(void*)Cyc_Toc_sizeoftyp_exp_r(elt_typ));done=1;{struct Cyc_Absyn_Exp*
_tmp51B=Cyc_Toc__init_dyneither_ptr_e;{struct Cyc_Absyn_Exp*_tmpA2D[4];(void*)(e->r=(
void*)Cyc_Toc_fncall_exp_r(_tmp51B,((_tmpA2D[3]=_tmp51A,((_tmpA2D[2]=Cyc_Absyn_sizeoftyp_exp(
Cyc_Toc_typ_to_c(_tmp50F),0),((_tmpA2D[1]=_tmp50A,((_tmpA2D[0]=mexp,((struct Cyc_List_List*(*)(
struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmpA2D,sizeof(struct Cyc_Absyn_Exp*),
4))))))))))));}goto _LL2D8;}_LL2DB:;_LL2DC: goto _LL2D8;_LL2D8:;}goto _LL2D3;_LL2D6:;
_LL2D7: goto _LL2D3;_LL2D3:;}goto _LL2CE;_LL2D1:;_LL2D2: goto _LL2CE;_LL2CE:;}if(!
done){struct Cyc_Absyn_Stmt*_tmp51D=Cyc_Absyn_exp_stmt(_tmp507,0);struct Cyc_Absyn_Exp*
_tmp51E=Cyc_Absyn_signed_int_exp(0,0);Cyc_Toc_exp_to_c(nv,_tmp3EF);_tmp51D=Cyc_Absyn_seq_stmt(
Cyc_Absyn_assign_stmt(Cyc_Absyn_subscript_exp(_tmp507,_tmp51E,0),_tmp3EF,0),
_tmp51D,0);{void*_tmp51F=Cyc_Absyn_cstar_typ(elt_typ,Cyc_Toc_mt_tq);(void*)(e->r=(
void*)Cyc_Toc_stmt_exp_r(Cyc_Absyn_declare_stmt(_tmp506,_tmp51F,(struct Cyc_Absyn_Exp*)
mexp,_tmp51D,0)));}}goto _LL2B9;}}_LL2B9:;}goto _LL1E0;_LL209: if(*((int*)_tmp3C3)
!= 19)goto _LL20B;_tmp3F0=((struct Cyc_Absyn_Sizeofexp_e_struct*)_tmp3C3)->f1;
_LL20A: Cyc_Toc_exp_to_c(nv,_tmp3F0);goto _LL1E0;_LL20B: if(*((int*)_tmp3C3)!= 18)
goto _LL20D;_tmp3F1=(void*)((struct Cyc_Absyn_Sizeoftyp_e_struct*)_tmp3C3)->f1;
_LL20C:{struct Cyc_Absyn_Sizeoftyp_e_struct _tmpA30;struct Cyc_Absyn_Sizeoftyp_e_struct*
_tmpA2F;(void*)(e->r=(void*)((void*)((_tmpA2F=_cycalloc(sizeof(*_tmpA2F)),((
_tmpA2F[0]=((_tmpA30.tag=18,((_tmpA30.f1=(void*)Cyc_Toc_typ_to_c_array(_tmp3F1),
_tmpA30)))),_tmpA2F))))));}goto _LL1E0;_LL20D: if(*((int*)_tmp3C3)!= 21)goto _LL20F;
_LL20E: {const char*_tmpA33;void*_tmpA32;(_tmpA32=0,((int(*)(struct _dyneither_ptr
fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmpA33="__gen() in code generator",
_tag_dyneither(_tmpA33,sizeof(char),26))),_tag_dyneither(_tmpA32,sizeof(void*),0)));}
_LL20F: if(*((int*)_tmp3C3)!= 20)goto _LL211;_tmp3F2=(void*)((struct Cyc_Absyn_Offsetof_e_struct*)
_tmp3C3)->f1;_tmp3F3=(void*)((struct Cyc_Absyn_Offsetof_e_struct*)_tmp3C3)->f2;
if(*((int*)_tmp3F3)!= 0)goto _LL211;_tmp3F4=((struct Cyc_Absyn_StructField_struct*)
_tmp3F3)->f1;_LL210:{struct Cyc_Absyn_Offsetof_e_struct _tmpA3D;struct Cyc_Absyn_StructField_struct
_tmpA3C;struct Cyc_Absyn_StructField_struct*_tmpA3B;struct Cyc_Absyn_Offsetof_e_struct*
_tmpA3A;(void*)(e->r=(void*)((void*)((_tmpA3A=_cycalloc(sizeof(*_tmpA3A)),((
_tmpA3A[0]=((_tmpA3D.tag=20,((_tmpA3D.f1=(void*)Cyc_Toc_typ_to_c_array(_tmp3F2),((
_tmpA3D.f2=(void*)((void*)((_tmpA3B=_cycalloc(sizeof(*_tmpA3B)),((_tmpA3B[0]=((
_tmpA3C.tag=0,((_tmpA3C.f1=_tmp3F4,_tmpA3C)))),_tmpA3B))))),_tmpA3D)))))),
_tmpA3A))))));}goto _LL1E0;_LL211: if(*((int*)_tmp3C3)!= 20)goto _LL213;_tmp3F5=(
void*)((struct Cyc_Absyn_Offsetof_e_struct*)_tmp3C3)->f1;_tmp3F6=(void*)((struct
Cyc_Absyn_Offsetof_e_struct*)_tmp3C3)->f2;if(*((int*)_tmp3F6)!= 1)goto _LL213;
_tmp3F7=((struct Cyc_Absyn_TupleIndex_struct*)_tmp3F6)->f1;_LL212:{void*_tmp528=
Cyc_Tcutil_compress(_tmp3F5);struct Cyc_Absyn_AggrInfo _tmp529;union Cyc_Absyn_AggrInfoU_union
_tmp52A;struct Cyc_List_List*_tmp52B;_LL2DE: if(_tmp528 <= (void*)4)goto _LL2E6;if(*((
int*)_tmp528)!= 10)goto _LL2E0;_tmp529=((struct Cyc_Absyn_AggrType_struct*)_tmp528)->f1;
_tmp52A=_tmp529.aggr_info;_LL2DF: {struct Cyc_Absyn_Aggrdecl*_tmp52C=Cyc_Absyn_get_known_aggrdecl(
_tmp52A);if(_tmp52C->impl == 0){const char*_tmpA40;void*_tmpA3F;(_tmpA3F=0,((int(*)(
struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmpA40="struct fields must be known",
_tag_dyneither(_tmpA40,sizeof(char),28))),_tag_dyneither(_tmpA3F,sizeof(void*),0)));}
_tmp52B=((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp52C->impl))->fields;goto
_LL2E1;}_LL2E0: if(*((int*)_tmp528)!= 11)goto _LL2E2;_tmp52B=((struct Cyc_Absyn_AnonAggrType_struct*)
_tmp528)->f2;_LL2E1: {struct Cyc_Absyn_Aggrfield*_tmp52F=((struct Cyc_Absyn_Aggrfield*(*)(
struct Cyc_List_List*x,int n))Cyc_List_nth)(_tmp52B,(int)_tmp3F7);{struct Cyc_Absyn_Offsetof_e_struct
_tmpA4A;struct Cyc_Absyn_StructField_struct _tmpA49;struct Cyc_Absyn_StructField_struct*
_tmpA48;struct Cyc_Absyn_Offsetof_e_struct*_tmpA47;(void*)(e->r=(void*)((void*)((
_tmpA47=_cycalloc(sizeof(*_tmpA47)),((_tmpA47[0]=((_tmpA4A.tag=20,((_tmpA4A.f1=(
void*)Cyc_Toc_typ_to_c_array(_tmp3F5),((_tmpA4A.f2=(void*)((void*)((_tmpA48=
_cycalloc(sizeof(*_tmpA48)),((_tmpA48[0]=((_tmpA49.tag=0,((_tmpA49.f1=_tmp52F->name,
_tmpA49)))),_tmpA48))))),_tmpA4A)))))),_tmpA47))))));}goto _LL2DD;}_LL2E2: if(*((
int*)_tmp528)!= 9)goto _LL2E4;_LL2E3:{struct Cyc_Absyn_Offsetof_e_struct _tmpA54;
struct Cyc_Absyn_StructField_struct _tmpA53;struct Cyc_Absyn_StructField_struct*
_tmpA52;struct Cyc_Absyn_Offsetof_e_struct*_tmpA51;(void*)(e->r=(void*)((void*)((
_tmpA51=_cycalloc(sizeof(*_tmpA51)),((_tmpA51[0]=((_tmpA54.tag=20,((_tmpA54.f1=(
void*)Cyc_Toc_typ_to_c_array(_tmp3F5),((_tmpA54.f2=(void*)((void*)((_tmpA52=
_cycalloc(sizeof(*_tmpA52)),((_tmpA52[0]=((_tmpA53.tag=0,((_tmpA53.f1=Cyc_Absyn_fieldname((
int)(_tmp3F7 + 1)),_tmpA53)))),_tmpA52))))),_tmpA54)))))),_tmpA51))))));}goto
_LL2DD;_LL2E4: if(*((int*)_tmp528)!= 3)goto _LL2E6;_LL2E5: if(_tmp3F7 == 0){struct
Cyc_Absyn_Offsetof_e_struct _tmpA5E;struct Cyc_Absyn_StructField_struct _tmpA5D;
struct Cyc_Absyn_StructField_struct*_tmpA5C;struct Cyc_Absyn_Offsetof_e_struct*
_tmpA5B;(void*)(e->r=(void*)((void*)((_tmpA5B=_cycalloc(sizeof(*_tmpA5B)),((
_tmpA5B[0]=((_tmpA5E.tag=20,((_tmpA5E.f1=(void*)Cyc_Toc_typ_to_c_array(_tmp3F5),((
_tmpA5E.f2=(void*)((void*)((_tmpA5C=_cycalloc(sizeof(*_tmpA5C)),((_tmpA5C[0]=((
_tmpA5D.tag=0,((_tmpA5D.f1=Cyc_Toc_tag_sp,_tmpA5D)))),_tmpA5C))))),_tmpA5E)))))),
_tmpA5B))))));}else{struct Cyc_Absyn_Offsetof_e_struct _tmpA68;struct Cyc_Absyn_StructField_struct
_tmpA67;struct Cyc_Absyn_StructField_struct*_tmpA66;struct Cyc_Absyn_Offsetof_e_struct*
_tmpA65;(void*)(e->r=(void*)((void*)((_tmpA65=_cycalloc(sizeof(*_tmpA65)),((
_tmpA65[0]=((_tmpA68.tag=20,((_tmpA68.f1=(void*)Cyc_Toc_typ_to_c_array(_tmp3F5),((
_tmpA68.f2=(void*)((void*)((_tmpA66=_cycalloc(sizeof(*_tmpA66)),((_tmpA66[0]=((
_tmpA67.tag=0,((_tmpA67.f1=Cyc_Absyn_fieldname((int)_tmp3F7),_tmpA67)))),_tmpA66))))),
_tmpA68)))))),_tmpA65))))));}goto _LL2DD;_LL2E6:;_LL2E7: {const char*_tmpA6B;void*
_tmpA6A;(_tmpA6A=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((
_tmpA6B="impossible type for offsetof tuple index",_tag_dyneither(_tmpA6B,
sizeof(char),41))),_tag_dyneither(_tmpA6A,sizeof(void*),0)));}_LL2DD:;}goto
_LL1E0;_LL213: if(*((int*)_tmp3C3)!= 22)goto _LL215;_tmp3F8=((struct Cyc_Absyn_Deref_e_struct*)
_tmp3C3)->f1;_LL214: {void*_tmp542=Cyc_Tcutil_compress((void*)((struct Cyc_Core_Opt*)
_check_null(_tmp3F8->topt))->v);{void*_tmp543=_tmp542;struct Cyc_Absyn_PtrInfo
_tmp544;void*_tmp545;struct Cyc_Absyn_Tqual _tmp546;struct Cyc_Absyn_PtrAtts _tmp547;
void*_tmp548;struct Cyc_Absyn_Conref*_tmp549;struct Cyc_Absyn_Conref*_tmp54A;
struct Cyc_Absyn_Conref*_tmp54B;_LL2E9: if(_tmp543 <= (void*)4)goto _LL2EB;if(*((int*)
_tmp543)!= 4)goto _LL2EB;_tmp544=((struct Cyc_Absyn_PointerType_struct*)_tmp543)->f1;
_tmp545=(void*)_tmp544.elt_typ;_tmp546=_tmp544.elt_tq;_tmp547=_tmp544.ptr_atts;
_tmp548=(void*)_tmp547.rgn;_tmp549=_tmp547.nullable;_tmp54A=_tmp547.bounds;
_tmp54B=_tmp547.zero_term;_LL2EA:{void*_tmp54C=Cyc_Absyn_conref_def(Cyc_Absyn_bounds_one,
_tmp54A);struct Cyc_Absyn_Exp*_tmp54D;_LL2EE: if(_tmp54C <= (void*)1)goto _LL2F0;if(*((
int*)_tmp54C)!= 0)goto _LL2F0;_tmp54D=((struct Cyc_Absyn_Upper_b_struct*)_tmp54C)->f1;
_LL2EF: {int do_null_check=Cyc_Toc_need_null_check(_tmp3F8);Cyc_Toc_exp_to_c(nv,
_tmp3F8);if(do_null_check){if(Cyc_Toc_warn_all_null_deref){const char*_tmpA6E;
void*_tmpA6D;(_tmpA6D=0,Cyc_Tcutil_warn(e->loc,((_tmpA6E="inserted null check due to dereference",
_tag_dyneither(_tmpA6E,sizeof(char),39))),_tag_dyneither(_tmpA6D,sizeof(void*),0)));}{
struct Cyc_List_List*_tmpA6F;(void*)(_tmp3F8->r=(void*)Cyc_Toc_cast_it_r(Cyc_Toc_typ_to_c(
_tmp542),Cyc_Absyn_fncall_exp(Cyc_Toc__check_null_e,((_tmpA6F=_cycalloc(sizeof(*
_tmpA6F)),((_tmpA6F->hd=Cyc_Absyn_copy_exp(_tmp3F8),((_tmpA6F->tl=0,_tmpA6F)))))),
0)));}}if(!((int(*)(int,struct Cyc_Absyn_Conref*x))Cyc_Absyn_conref_def)(0,
_tmp54B)){unsigned int _tmp552;int _tmp553;struct _tuple5 _tmp551=Cyc_Evexp_eval_const_uint_exp(
_tmp54D);_tmp552=_tmp551.f1;_tmp553=_tmp551.f2;if(!_tmp553  || _tmp552 <= 0){const
char*_tmpA72;void*_tmpA71;(_tmpA71=0,Cyc_Tcutil_terr(e->loc,((_tmpA72="cannot determine dereference is in bounds",
_tag_dyneither(_tmpA72,sizeof(char),42))),_tag_dyneither(_tmpA71,sizeof(void*),0)));}}
goto _LL2ED;}_LL2F0: if((int)_tmp54C != 0)goto _LL2ED;_LL2F1: {struct Cyc_Absyn_Exp*
_tmp556=Cyc_Absyn_uint_exp(0,0);{struct Cyc_Core_Opt*_tmpA73;_tmp556->topt=((
_tmpA73=_cycalloc(sizeof(*_tmpA73)),((_tmpA73->v=(void*)Cyc_Absyn_uint_typ,
_tmpA73))));}(void*)(e->r=(void*)Cyc_Toc_subscript_exp_r(_tmp3F8,_tmp556));Cyc_Toc_exp_to_c(
nv,e);goto _LL2ED;}_LL2ED:;}goto _LL2E8;_LL2EB:;_LL2EC: {const char*_tmpA76;void*
_tmpA75;(_tmpA75=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((
_tmpA76="exp_to_c: Deref: non-pointer",_tag_dyneither(_tmpA76,sizeof(char),29))),
_tag_dyneither(_tmpA75,sizeof(void*),0)));}_LL2E8:;}goto _LL1E0;}_LL215: if(*((int*)
_tmp3C3)!= 23)goto _LL217;_tmp3F9=((struct Cyc_Absyn_AggrMember_e_struct*)_tmp3C3)->f1;
_tmp3FA=((struct Cyc_Absyn_AggrMember_e_struct*)_tmp3C3)->f2;_LL216: Cyc_Toc_exp_to_c(
nv,_tmp3F9);if(Cyc_Toc_is_poly_project(e))(void*)(e->r=(void*)((void*)(Cyc_Toc_array_to_ptr_cast(
Cyc_Toc_typ_to_c((void*)((struct Cyc_Core_Opt*)_check_null(e->topt))->v),Cyc_Absyn_new_exp((
void*)e->r,0),0))->r));goto _LL1E0;_LL217: if(*((int*)_tmp3C3)!= 24)goto _LL219;
_tmp3FB=((struct Cyc_Absyn_AggrArrow_e_struct*)_tmp3C3)->f1;_tmp3FC=((struct Cyc_Absyn_AggrArrow_e_struct*)
_tmp3C3)->f2;_LL218: {void*e1typ=Cyc_Tcutil_compress((void*)((struct Cyc_Core_Opt*)
_check_null(_tmp3FB->topt))->v);int do_null_check=Cyc_Toc_need_null_check(_tmp3FB);
Cyc_Toc_exp_to_c(nv,_tmp3FB);{int is_poly=Cyc_Toc_is_poly_project(e);void*_tmp55B;
struct Cyc_Absyn_Tqual _tmp55C;struct Cyc_Absyn_PtrAtts _tmp55D;void*_tmp55E;struct
Cyc_Absyn_Conref*_tmp55F;struct Cyc_Absyn_Conref*_tmp560;struct Cyc_Absyn_Conref*
_tmp561;struct Cyc_Absyn_PtrInfo _tmp55A=Cyc_Toc_get_ptr_type(e1typ);_tmp55B=(void*)
_tmp55A.elt_typ;_tmp55C=_tmp55A.elt_tq;_tmp55D=_tmp55A.ptr_atts;_tmp55E=(void*)
_tmp55D.rgn;_tmp55F=_tmp55D.nullable;_tmp560=_tmp55D.bounds;_tmp561=_tmp55D.zero_term;{
void*_tmp562=Cyc_Absyn_conref_def(Cyc_Absyn_bounds_one,_tmp560);struct Cyc_Absyn_Exp*
_tmp563;_LL2F3: if(_tmp562 <= (void*)1)goto _LL2F5;if(*((int*)_tmp562)!= 0)goto
_LL2F5;_tmp563=((struct Cyc_Absyn_Upper_b_struct*)_tmp562)->f1;_LL2F4: {
unsigned int _tmp565;int _tmp566;struct _tuple5 _tmp564=Cyc_Evexp_eval_const_uint_exp(
_tmp563);_tmp565=_tmp564.f1;_tmp566=_tmp564.f2;if(_tmp566){if(_tmp565 < 1){const
char*_tmpA79;void*_tmpA78;(_tmpA78=0,((int(*)(struct _dyneither_ptr fmt,struct
_dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmpA79="exp_to_c:  AggrArrow_e on pointer of size 0",
_tag_dyneither(_tmpA79,sizeof(char),44))),_tag_dyneither(_tmpA78,sizeof(void*),0)));}
if(do_null_check){if(Cyc_Toc_warn_all_null_deref){const char*_tmpA7C;void*_tmpA7B;(
_tmpA7B=0,Cyc_Tcutil_warn(e->loc,((_tmpA7C="inserted null check due to dereference",
_tag_dyneither(_tmpA7C,sizeof(char),39))),_tag_dyneither(_tmpA7B,sizeof(void*),0)));}{
struct Cyc_List_List*_tmpA7D;(void*)(e->r=(void*)Cyc_Toc_aggrarrow_exp_r(Cyc_Toc_cast_it(
Cyc_Toc_typ_to_c((void*)((struct Cyc_Core_Opt*)_check_null(_tmp3FB->topt))->v),
Cyc_Absyn_fncall_exp(Cyc_Toc__check_null_e,((_tmpA7D=_cycalloc(sizeof(*_tmpA7D)),((
_tmpA7D->hd=_tmp3FB,((_tmpA7D->tl=0,_tmpA7D)))))),0)),_tmp3FC));}}}else{if(!Cyc_Evexp_c_can_eval(
_tmp563)){const char*_tmpA80;void*_tmpA7F;(_tmpA7F=0,Cyc_Tcutil_terr(e->loc,((
_tmpA80="cannot determine pointer dereference in bounds",_tag_dyneither(_tmpA80,
sizeof(char),47))),_tag_dyneither(_tmpA7F,sizeof(void*),0)));}{struct Cyc_Absyn_Exp*
_tmpA81[4];(void*)(e->r=(void*)Cyc_Toc_aggrarrow_exp_r(Cyc_Toc_cast_it(Cyc_Toc_typ_to_c((
void*)((struct Cyc_Core_Opt*)_check_null(_tmp3FB->topt))->v),Cyc_Absyn_fncall_exp(
Cyc_Toc__check_known_subscript_null_e,((_tmpA81[3]=Cyc_Absyn_uint_exp(0,0),((
_tmpA81[2]=Cyc_Absyn_sizeoftyp_exp(_tmp55B,0),((_tmpA81[1]=_tmp563,((_tmpA81[0]=
_tmp3FB,((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(
_tag_dyneither(_tmpA81,sizeof(struct Cyc_Absyn_Exp*),4)))))))))),0)),_tmp3FC));}}
goto _LL2F2;}_LL2F5: if((int)_tmp562 != 0)goto _LL2F2;_LL2F6: {struct Cyc_Absyn_Exp*
_tmp56F=Cyc_Toc__check_dyneither_subscript_e;void*ta1=Cyc_Toc_typ_to_c_array(
_tmp55B);{struct Cyc_Absyn_Exp*_tmpA82[3];(void*)(_tmp3FB->r=(void*)Cyc_Toc_cast_it_r(
Cyc_Absyn_cstar_typ(ta1,_tmp55C),Cyc_Absyn_fncall_exp(_tmp56F,((_tmpA82[2]=Cyc_Absyn_uint_exp(
0,0),((_tmpA82[1]=Cyc_Absyn_sizeoftyp_exp(ta1,0),((_tmpA82[0]=Cyc_Absyn_copy_exp(
_tmp3FB),((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(
_tag_dyneither(_tmpA82,sizeof(struct Cyc_Absyn_Exp*),3)))))))),0)));}goto _LL2F2;}
_LL2F2:;}if(is_poly)(void*)(e->r=(void*)((void*)(Cyc_Toc_array_to_ptr_cast(Cyc_Toc_typ_to_c((
void*)((struct Cyc_Core_Opt*)_check_null(e->topt))->v),Cyc_Absyn_new_exp((void*)e->r,
0),0))->r));goto _LL1E0;}}_LL219: if(*((int*)_tmp3C3)!= 25)goto _LL21B;_tmp3FD=((
struct Cyc_Absyn_Subscript_e_struct*)_tmp3C3)->f1;_tmp3FE=((struct Cyc_Absyn_Subscript_e_struct*)
_tmp3C3)->f2;_LL21A: {void*_tmp571=Cyc_Tcutil_compress((void*)((struct Cyc_Core_Opt*)
_check_null(_tmp3FD->topt))->v);{void*_tmp572=_tmp571;struct Cyc_List_List*
_tmp573;struct Cyc_Absyn_PtrInfo _tmp574;void*_tmp575;struct Cyc_Absyn_Tqual _tmp576;
struct Cyc_Absyn_PtrAtts _tmp577;void*_tmp578;struct Cyc_Absyn_Conref*_tmp579;
struct Cyc_Absyn_Conref*_tmp57A;struct Cyc_Absyn_Conref*_tmp57B;_LL2F8: if(_tmp572
<= (void*)4)goto _LL2FC;if(*((int*)_tmp572)!= 9)goto _LL2FA;_tmp573=((struct Cyc_Absyn_TupleType_struct*)
_tmp572)->f1;_LL2F9: Cyc_Toc_exp_to_c(nv,_tmp3FD);Cyc_Toc_exp_to_c(nv,_tmp3FE);{
unsigned int _tmp57D;int _tmp57E;struct _tuple5 _tmp57C=Cyc_Evexp_eval_const_uint_exp(
_tmp3FE);_tmp57D=_tmp57C.f1;_tmp57E=_tmp57C.f2;if(!_tmp57E){const char*_tmpA85;
void*_tmpA84;(_tmpA84=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))
Cyc_Tcutil_impos)(((_tmpA85="unknown tuple subscript in translation to C",
_tag_dyneither(_tmpA85,sizeof(char),44))),_tag_dyneither(_tmpA84,sizeof(void*),0)));}(
void*)(e->r=(void*)Cyc_Toc_aggrmember_exp_r(_tmp3FD,Cyc_Absyn_fieldname((int)(
_tmp57D + 1))));goto _LL2F7;}_LL2FA: if(*((int*)_tmp572)!= 4)goto _LL2FC;_tmp574=((
struct Cyc_Absyn_PointerType_struct*)_tmp572)->f1;_tmp575=(void*)_tmp574.elt_typ;
_tmp576=_tmp574.elt_tq;_tmp577=_tmp574.ptr_atts;_tmp578=(void*)_tmp577.rgn;
_tmp579=_tmp577.nullable;_tmp57A=_tmp577.bounds;_tmp57B=_tmp577.zero_term;_LL2FB: {
struct Cyc_List_List*_tmp581=Cyc_Toc_get_relns(_tmp3FD);int in_bnds=0;{void*
_tmp582=Cyc_Absyn_conref_def(Cyc_Absyn_bounds_one,_tmp57A);_LL2FF:;_LL300:
in_bnds=Cyc_Toc_check_bounds(_tmp581,_tmp3FD,_tmp3FE);if(Cyc_Toc_warn_bounds_checks
 && !in_bnds){const char*_tmpA89;void*_tmpA88[1];struct Cyc_String_pa_struct
_tmpA87;(_tmpA87.tag=0,((_tmpA87.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Absynpp_exp2string(e)),((_tmpA88[0]=& _tmpA87,Cyc_Tcutil_warn(e->loc,((
_tmpA89="bounds check necessary for %s",_tag_dyneither(_tmpA89,sizeof(char),30))),
_tag_dyneither(_tmpA88,sizeof(void*),1)))))));}_LL2FE:;}Cyc_Toc_exp_to_c(nv,
_tmp3FD);Cyc_Toc_exp_to_c(nv,_tmp3FE);++ Cyc_Toc_total_bounds_checks;{void*
_tmp586=Cyc_Absyn_conref_def(Cyc_Absyn_bounds_one,_tmp57A);struct Cyc_Absyn_Exp*
_tmp587;_LL302: if(_tmp586 <= (void*)1)goto _LL304;if(*((int*)_tmp586)!= 0)goto
_LL304;_tmp587=((struct Cyc_Absyn_Upper_b_struct*)_tmp586)->f1;_LL303: {int
possibly_null=((int(*)(int,struct Cyc_Absyn_Conref*x))Cyc_Absyn_conref_def)(0,
_tmp579);void*ta1=Cyc_Toc_typ_to_c(_tmp575);void*ta2=Cyc_Absyn_cstar_typ(ta1,
_tmp576);if(in_bnds)++ Cyc_Toc_bounds_checks_eliminated;else{if(((int(*)(int,
struct Cyc_Absyn_Conref*x))Cyc_Absyn_conref_def)(0,_tmp57B)){if(!Cyc_Evexp_c_can_eval(
_tmp587)){const char*_tmpA8C;void*_tmpA8B;(_tmpA8B=0,Cyc_Tcutil_terr(e->loc,((
_tmpA8C="cannot determine subscript is in bounds",_tag_dyneither(_tmpA8C,sizeof(
char),40))),_tag_dyneither(_tmpA8B,sizeof(void*),0)));}{struct Cyc_Absyn_Exp*
function_e=Cyc_Toc_getFunction(& Cyc_Toc__zero_arr_plus_functionSet,_tmp3FD);
struct Cyc_Absyn_Exp*_tmpA8D[3];(void*)(e->r=(void*)Cyc_Toc_deref_exp_r(Cyc_Toc_cast_it(
ta2,Cyc_Absyn_fncall_exp(function_e,((_tmpA8D[2]=_tmp3FE,((_tmpA8D[1]=_tmp587,((
_tmpA8D[0]=_tmp3FD,((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(
_tag_dyneither(_tmpA8D,sizeof(struct Cyc_Absyn_Exp*),3)))))))),0))));}}else{if(
possibly_null){if(!Cyc_Evexp_c_can_eval(_tmp587)){const char*_tmpA90;void*_tmpA8F;(
_tmpA8F=0,Cyc_Tcutil_terr(e->loc,((_tmpA90="cannot determine subscript is in bounds",
_tag_dyneither(_tmpA90,sizeof(char),40))),_tag_dyneither(_tmpA8F,sizeof(void*),0)));}
if(Cyc_Toc_warn_all_null_deref){const char*_tmpA93;void*_tmpA92;(_tmpA92=0,Cyc_Tcutil_warn(
e->loc,((_tmpA93="inserted null check due to dereference",_tag_dyneither(_tmpA93,
sizeof(char),39))),_tag_dyneither(_tmpA92,sizeof(void*),0)));}{struct Cyc_Absyn_Exp*
_tmpA94[4];(void*)(e->r=(void*)Cyc_Toc_deref_exp_r(Cyc_Toc_cast_it(ta2,Cyc_Absyn_fncall_exp(
Cyc_Toc__check_known_subscript_null_e,((_tmpA94[3]=_tmp3FE,((_tmpA94[2]=Cyc_Absyn_sizeoftyp_exp(
ta1,0),((_tmpA94[1]=_tmp587,((_tmpA94[0]=_tmp3FD,((struct Cyc_List_List*(*)(
struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmpA94,sizeof(struct Cyc_Absyn_Exp*),
4)))))))))),0))));}}else{struct Cyc_Absyn_Exp*_tmpA95[2];(void*)(_tmp3FE->r=(void*)
Cyc_Toc_fncall_exp_r(Cyc_Toc__check_known_subscript_notnull_e,((_tmpA95[1]=Cyc_Absyn_copy_exp(
_tmp3FE),((_tmpA95[0]=_tmp587,((struct Cyc_List_List*(*)(struct _dyneither_ptr))
Cyc_List_list)(_tag_dyneither(_tmpA95,sizeof(struct Cyc_Absyn_Exp*),2))))))));}}}
goto _LL301;}_LL304: if((int)_tmp586 != 0)goto _LL301;_LL305: {void*ta1=Cyc_Toc_typ_to_c_array(
_tmp575);if(in_bnds){++ Cyc_Toc_bounds_checks_eliminated;(void*)(e->r=(void*)Cyc_Toc_subscript_exp_r(
Cyc_Toc_cast_it(Cyc_Absyn_cstar_typ(ta1,_tmp576),Cyc_Absyn_aggrmember_exp(
_tmp3FD,Cyc_Toc_curr_sp,0)),_tmp3FE));}else{struct Cyc_Absyn_Exp*_tmp591=Cyc_Toc__check_dyneither_subscript_e;
struct Cyc_Absyn_Exp*_tmpA96[3];(void*)(e->r=(void*)Cyc_Toc_deref_exp_r(Cyc_Toc_cast_it(
Cyc_Absyn_cstar_typ(ta1,_tmp576),Cyc_Absyn_fncall_exp(_tmp591,((_tmpA96[2]=
_tmp3FE,((_tmpA96[1]=Cyc_Absyn_sizeoftyp_exp(ta1,0),((_tmpA96[0]=_tmp3FD,((
struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(
_tmpA96,sizeof(struct Cyc_Absyn_Exp*),3)))))))),0))));}goto _LL301;}_LL301:;}goto
_LL2F7;}_LL2FC:;_LL2FD: {const char*_tmpA99;void*_tmpA98;(_tmpA98=0,((int(*)(
struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmpA99="exp_to_c: Subscript on non-tuple/array/tuple ptr",
_tag_dyneither(_tmpA99,sizeof(char),49))),_tag_dyneither(_tmpA98,sizeof(void*),0)));}
_LL2F7:;}goto _LL1E0;}_LL21B: if(*((int*)_tmp3C3)!= 26)goto _LL21D;_tmp3FF=((struct
Cyc_Absyn_Tuple_e_struct*)_tmp3C3)->f1;_LL21C: if(!Cyc_Toc_is_toplevel(nv))(void*)(
e->r=(void*)((void*)(Cyc_Toc_init_tuple(nv,0,0,_tmp3FF))->r));else{struct Cyc_List_List*
_tmp595=((struct Cyc_List_List*(*)(struct _tuple4*(*f)(struct Cyc_Absyn_Exp*),
struct Cyc_List_List*x))Cyc_List_map)(Cyc_Toc_tup_to_c,_tmp3FF);void*_tmp596=Cyc_Toc_add_tuple_type(
_tmp595);struct Cyc_List_List*dles=0;{int i=1;for(0;_tmp3FF != 0;(_tmp3FF=_tmp3FF->tl,
i ++)){Cyc_Toc_exp_to_c(nv,(struct Cyc_Absyn_Exp*)_tmp3FF->hd);{struct _tuple8*
_tmpA9C;struct Cyc_List_List*_tmpA9B;dles=((_tmpA9B=_cycalloc(sizeof(*_tmpA9B)),((
_tmpA9B->hd=((_tmpA9C=_cycalloc(sizeof(*_tmpA9C)),((_tmpA9C->f1=0,((_tmpA9C->f2=(
struct Cyc_Absyn_Exp*)_tmp3FF->hd,_tmpA9C)))))),((_tmpA9B->tl=dles,_tmpA9B))))));}}}
dles=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(dles);(
void*)(e->r=(void*)Cyc_Toc_unresolvedmem_exp_r(0,dles));}goto _LL1E0;_LL21D: if(*((
int*)_tmp3C3)!= 28)goto _LL21F;_tmp400=((struct Cyc_Absyn_Array_e_struct*)_tmp3C3)->f1;
_LL21E:(void*)(e->r=(void*)Cyc_Toc_unresolvedmem_exp_r(0,_tmp400));{struct Cyc_List_List*
_tmp599=_tmp400;for(0;_tmp599 != 0;_tmp599=_tmp599->tl){struct _tuple8 _tmp59B;
struct Cyc_Absyn_Exp*_tmp59C;struct _tuple8*_tmp59A=(struct _tuple8*)_tmp599->hd;
_tmp59B=*_tmp59A;_tmp59C=_tmp59B.f2;Cyc_Toc_exp_to_c(nv,_tmp59C);}}goto _LL1E0;
_LL21F: if(*((int*)_tmp3C3)!= 29)goto _LL221;_tmp401=((struct Cyc_Absyn_Comprehension_e_struct*)
_tmp3C3)->f1;_tmp402=((struct Cyc_Absyn_Comprehension_e_struct*)_tmp3C3)->f2;
_tmp403=((struct Cyc_Absyn_Comprehension_e_struct*)_tmp3C3)->f3;_tmp404=((struct
Cyc_Absyn_Comprehension_e_struct*)_tmp3C3)->f4;_LL220: {unsigned int _tmp59E;int
_tmp59F;struct _tuple5 _tmp59D=Cyc_Evexp_eval_const_uint_exp(_tmp402);_tmp59E=
_tmp59D.f1;_tmp59F=_tmp59D.f2;{void*_tmp5A0=Cyc_Toc_typ_to_c((void*)((struct Cyc_Core_Opt*)
_check_null(_tmp403->topt))->v);Cyc_Toc_exp_to_c(nv,_tmp403);{struct Cyc_List_List*
es=0;if(!Cyc_Toc_is_zero(_tmp403)){if(!_tmp59F){const char*_tmpA9F;void*_tmpA9E;(
_tmpA9E=0,Cyc_Tcutil_terr(_tmp402->loc,((_tmpA9F="cannot determine value of constant",
_tag_dyneither(_tmpA9F,sizeof(char),35))),_tag_dyneither(_tmpA9E,sizeof(void*),0)));}{
unsigned int i=0;for(0;i < _tmp59E;++ i){struct _tuple8*_tmpAA2;struct Cyc_List_List*
_tmpAA1;es=((_tmpAA1=_cycalloc(sizeof(*_tmpAA1)),((_tmpAA1->hd=((_tmpAA2=
_cycalloc(sizeof(*_tmpAA2)),((_tmpAA2->f1=0,((_tmpAA2->f2=_tmp403,_tmpAA2)))))),((
_tmpAA1->tl=es,_tmpAA1))))));}}if(_tmp404){struct Cyc_Absyn_Exp*_tmp5A5=Cyc_Toc_cast_it(
_tmp5A0,Cyc_Absyn_uint_exp(0,0));struct _tuple8*_tmpAA5;struct Cyc_List_List*
_tmpAA4;es=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))
Cyc_List_imp_append)(es,((_tmpAA4=_cycalloc(sizeof(*_tmpAA4)),((_tmpAA4->hd=((
_tmpAA5=_cycalloc(sizeof(*_tmpAA5)),((_tmpAA5->f1=0,((_tmpAA5->f2=_tmp5A5,
_tmpAA5)))))),((_tmpAA4->tl=0,_tmpAA4)))))));}}(void*)(e->r=(void*)Cyc_Toc_unresolvedmem_exp_r(
0,es));goto _LL1E0;}}}_LL221: if(*((int*)_tmp3C3)!= 30)goto _LL223;_tmp405=((struct
Cyc_Absyn_Struct_e_struct*)_tmp3C3)->f1;_tmp406=((struct Cyc_Absyn_Struct_e_struct*)
_tmp3C3)->f2;_tmp407=((struct Cyc_Absyn_Struct_e_struct*)_tmp3C3)->f3;_tmp408=((
struct Cyc_Absyn_Struct_e_struct*)_tmp3C3)->f4;_LL222: if(!Cyc_Toc_is_toplevel(nv))(
void*)(e->r=(void*)((void*)(Cyc_Toc_init_struct(nv,old_typ,_tmp406 != 0,0,0,
_tmp407,_tmp405))->r));else{if(_tmp408 == 0){const char*_tmpAA8;void*_tmpAA7;(
_tmpAA7=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((
_tmpAA8="Struct_e: missing aggrdecl pointer",_tag_dyneither(_tmpAA8,sizeof(char),
35))),_tag_dyneither(_tmpAA7,sizeof(void*),0)));}{struct Cyc_Absyn_Aggrdecl*sd2=(
struct Cyc_Absyn_Aggrdecl*)_tmp408;struct _RegionHandle _tmp5AA=_new_region("rgn");
struct _RegionHandle*rgn=& _tmp5AA;_push_region(rgn);{struct Cyc_List_List*_tmp5AB=((
struct Cyc_List_List*(*)(struct _RegionHandle*rgn,struct Cyc_Position_Segment*loc,
struct Cyc_List_List*des,struct Cyc_List_List*fields))Cyc_Tcutil_resolve_struct_designators)(
rgn,e->loc,_tmp407,((struct Cyc_Absyn_AggrdeclImpl*)_check_null(sd2->impl))->fields);
struct Cyc_List_List*_tmp5AC=0;struct Cyc_List_List*_tmp5AD=((struct Cyc_Absyn_AggrdeclImpl*)
_check_null(sd2->impl))->fields;for(0;_tmp5AD != 0;_tmp5AD=_tmp5AD->tl){struct Cyc_List_List*
_tmp5AE=_tmp5AB;for(0;_tmp5AE != 0;_tmp5AE=_tmp5AE->tl){if((*((struct _tuple12*)
_tmp5AE->hd)).f1 == (struct Cyc_Absyn_Aggrfield*)_tmp5AD->hd){struct _tuple12
_tmp5B0;struct Cyc_Absyn_Aggrfield*_tmp5B1;struct Cyc_Absyn_Exp*_tmp5B2;struct
_tuple12*_tmp5AF=(struct _tuple12*)_tmp5AE->hd;_tmp5B0=*_tmp5AF;_tmp5B1=_tmp5B0.f1;
_tmp5B2=_tmp5B0.f2;{void*_tmp5B3=(void*)_tmp5B1->type;Cyc_Toc_exp_to_c(nv,
_tmp5B2);if(Cyc_Toc_is_void_star(_tmp5B3))(void*)(_tmp5B2->r=(void*)Cyc_Toc_cast_it_r(
Cyc_Absyn_void_star_typ(),Cyc_Absyn_new_exp((void*)_tmp5B2->r,0)));{struct
_tuple8*_tmpAAB;struct Cyc_List_List*_tmpAAA;_tmp5AC=((_tmpAAA=_cycalloc(sizeof(*
_tmpAAA)),((_tmpAAA->hd=((_tmpAAB=_cycalloc(sizeof(*_tmpAAB)),((_tmpAAB->f1=0,((
_tmpAAB->f2=_tmp5B2,_tmpAAB)))))),((_tmpAAA->tl=_tmp5AC,_tmpAAA))))));}break;}}}}(
void*)(e->r=(void*)Cyc_Toc_unresolvedmem_exp_r(0,((struct Cyc_List_List*(*)(
struct Cyc_List_List*x))Cyc_List_imp_rev)(_tmp5AC)));};_pop_region(rgn);}}goto
_LL1E0;_LL223: if(*((int*)_tmp3C3)!= 31)goto _LL225;_tmp409=(void*)((struct Cyc_Absyn_AnonStruct_e_struct*)
_tmp3C3)->f1;_tmp40A=((struct Cyc_Absyn_AnonStruct_e_struct*)_tmp3C3)->f2;_LL224: {
struct Cyc_List_List*fs;{void*_tmp5B6=Cyc_Tcutil_compress(_tmp409);struct Cyc_List_List*
_tmp5B7;_LL307: if(_tmp5B6 <= (void*)4)goto _LL309;if(*((int*)_tmp5B6)!= 11)goto
_LL309;_tmp5B7=((struct Cyc_Absyn_AnonAggrType_struct*)_tmp5B6)->f2;_LL308: fs=
_tmp5B7;goto _LL306;_LL309:;_LL30A: {const char*_tmpAAF;void*_tmpAAE[1];struct Cyc_String_pa_struct
_tmpAAD;(_tmpAAD.tag=0,((_tmpAAD.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Absynpp_typ2string(_tmp409)),((_tmpAAE[0]=& _tmpAAD,((int(*)(struct
_dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmpAAF="anon struct has type %s",
_tag_dyneither(_tmpAAF,sizeof(char),24))),_tag_dyneither(_tmpAAE,sizeof(void*),1)))))));}
_LL306:;}{struct _RegionHandle _tmp5BB=_new_region("rgn");struct _RegionHandle*rgn=&
_tmp5BB;_push_region(rgn);{struct Cyc_List_List*_tmp5BC=((struct Cyc_List_List*(*)(
struct _RegionHandle*rgn,struct Cyc_Position_Segment*loc,struct Cyc_List_List*des,
struct Cyc_List_List*fields))Cyc_Tcutil_resolve_struct_designators)(rgn,e->loc,
_tmp40A,fs);for(0;_tmp5BC != 0;_tmp5BC=_tmp5BC->tl){struct _tuple12 _tmp5BE;struct
Cyc_Absyn_Aggrfield*_tmp5BF;struct Cyc_Absyn_Exp*_tmp5C0;struct _tuple12*_tmp5BD=(
struct _tuple12*)_tmp5BC->hd;_tmp5BE=*_tmp5BD;_tmp5BF=_tmp5BE.f1;_tmp5C0=_tmp5BE.f2;{
void*_tmp5C1=(void*)_tmp5BF->type;Cyc_Toc_exp_to_c(nv,_tmp5C0);if(Cyc_Toc_is_void_star(
_tmp5C1))(void*)(_tmp5C0->r=(void*)Cyc_Toc_cast_it_r(Cyc_Absyn_void_star_typ(),
Cyc_Absyn_new_exp((void*)_tmp5C0->r,0)));}}(void*)(e->r=(void*)Cyc_Toc_unresolvedmem_exp_r(
0,_tmp40A));};_pop_region(rgn);}goto _LL1E0;}_LL225: if(*((int*)_tmp3C3)!= 32)goto
_LL227;_tmp40B=((struct Cyc_Absyn_Tunion_e_struct*)_tmp3C3)->f1;if(_tmp40B != 0)
goto _LL227;_tmp40C=((struct Cyc_Absyn_Tunion_e_struct*)_tmp3C3)->f2;_tmp40D=((
struct Cyc_Absyn_Tunion_e_struct*)_tmp3C3)->f3;if(!(!_tmp40C->is_flat))goto _LL227;
_LL226: {struct _tuple1*qv=_tmp40D->name;struct Cyc_Absyn_Exp*tag_exp=_tmp40C->is_xtunion?
Cyc_Absyn_var_exp(qv,0): Cyc_Toc_tunion_tag(_tmp40C,qv,0);(void*)(e->r=(void*)((
void*)tag_exp->r));goto _LL1E0;}_LL227: if(*((int*)_tmp3C3)!= 32)goto _LL229;
_tmp40E=((struct Cyc_Absyn_Tunion_e_struct*)_tmp3C3)->f1;_tmp40F=((struct Cyc_Absyn_Tunion_e_struct*)
_tmp3C3)->f2;_tmp410=((struct Cyc_Absyn_Tunion_e_struct*)_tmp3C3)->f3;_LL228: {
void*tunion_ctype;struct Cyc_Absyn_Exp*tag_exp;struct _tuple1*_tmp5C2=Cyc_Toc_temp_var();
struct Cyc_Absyn_Exp*_tmp5C3=Cyc_Absyn_var_exp(_tmp5C2,0);struct Cyc_Absyn_Exp*
member_exp;if(_tmp40F->is_flat){{const char*_tmpAB0;tunion_ctype=Cyc_Absyn_unionq_typ(
Cyc_Toc_collapse_qvar_tag(_tmp40F->name,((_tmpAB0="_union",_tag_dyneither(
_tmpAB0,sizeof(char),7)))));}tag_exp=Cyc_Toc_tunion_tag(_tmp40F,_tmp410->name,1);{
struct _tuple1 _tmp5C6;struct _dyneither_ptr*_tmp5C7;struct _tuple1*_tmp5C5=_tmp410->name;
_tmp5C6=*_tmp5C5;_tmp5C7=_tmp5C6.f2;member_exp=Cyc_Absyn_aggrmember_exp(_tmp5C3,
_tmp5C7,0);}}else{{const char*_tmpAB1;tunion_ctype=Cyc_Absyn_strctq(Cyc_Toc_collapse_qvar_tag(
_tmp410->name,((_tmpAB1="_struct",_tag_dyneither(_tmpAB1,sizeof(char),8)))));}
tag_exp=_tmp40F->is_xtunion?Cyc_Absyn_var_exp(_tmp410->name,0): Cyc_Toc_tunion_tag(
_tmp40F,_tmp410->name,1);member_exp=_tmp5C3;}{struct Cyc_List_List*_tmp5C9=
_tmp410->typs;if(Cyc_Toc_is_toplevel(nv)){struct Cyc_List_List*dles=0;for(0;
_tmp40E != 0;(_tmp40E=_tmp40E->tl,_tmp5C9=_tmp5C9->tl)){struct Cyc_Absyn_Exp*cur_e=(
struct Cyc_Absyn_Exp*)_tmp40E->hd;void*field_typ=Cyc_Toc_typ_to_c((*((struct
_tuple4*)((struct Cyc_List_List*)_check_null(_tmp5C9))->hd)).f2);Cyc_Toc_exp_to_c(
nv,cur_e);if(Cyc_Toc_is_void_star(field_typ))cur_e=Cyc_Toc_cast_it(field_typ,
cur_e);{struct _tuple8*_tmpAB4;struct Cyc_List_List*_tmpAB3;dles=((_tmpAB3=
_cycalloc(sizeof(*_tmpAB3)),((_tmpAB3->hd=((_tmpAB4=_cycalloc(sizeof(*_tmpAB4)),((
_tmpAB4->f1=0,((_tmpAB4->f2=cur_e,_tmpAB4)))))),((_tmpAB3->tl=dles,_tmpAB3))))));}}{
struct _tuple8*_tmpAB7;struct Cyc_List_List*_tmpAB6;dles=((_tmpAB6=_cycalloc(
sizeof(*_tmpAB6)),((_tmpAB6->hd=((_tmpAB7=_cycalloc(sizeof(*_tmpAB7)),((_tmpAB7->f1=
0,((_tmpAB7->f2=tag_exp,_tmpAB7)))))),((_tmpAB6->tl=((struct Cyc_List_List*(*)(
struct Cyc_List_List*x))Cyc_List_imp_rev)(dles),_tmpAB6))))));}(void*)(e->r=(void*)
Cyc_Toc_unresolvedmem_exp_r(0,dles));if(_tmp40F->is_flat){const char*_tmpAB8;(
void*)(e->r=(void*)Cyc_Toc_cast_it_r(Cyc_Absyn_strctq(Cyc_Toc_collapse_qvar_tag(
_tmp410->name,((_tmpAB8="_struct",_tag_dyneither(_tmpAB8,sizeof(char),8))))),Cyc_Absyn_copy_exp(
e)));}}else{struct Cyc_List_List*_tmpAB9;struct Cyc_List_List*_tmp5CF=(_tmpAB9=
_cycalloc(sizeof(*_tmpAB9)),((_tmpAB9->hd=Cyc_Absyn_assign_stmt(Cyc_Absyn_aggrmember_exp(
member_exp,Cyc_Toc_tag_sp,0),tag_exp,0),((_tmpAB9->tl=0,_tmpAB9)))));{int i=1;
for(0;_tmp40E != 0;(((_tmp40E=_tmp40E->tl,i ++)),_tmp5C9=_tmp5C9->tl)){struct Cyc_Absyn_Exp*
cur_e=(struct Cyc_Absyn_Exp*)_tmp40E->hd;void*field_typ=Cyc_Toc_typ_to_c((*((
struct _tuple4*)((struct Cyc_List_List*)_check_null(_tmp5C9))->hd)).f2);Cyc_Toc_exp_to_c(
nv,cur_e);if(Cyc_Toc_is_void_star(field_typ))cur_e=Cyc_Toc_cast_it(field_typ,
cur_e);{struct Cyc_Absyn_Stmt*_tmp5D0=Cyc_Absyn_assign_stmt(Cyc_Absyn_aggrmember_exp(
member_exp,Cyc_Absyn_fieldname(i),0),cur_e,0);struct Cyc_List_List*_tmpABA;
_tmp5CF=((_tmpABA=_cycalloc(sizeof(*_tmpABA)),((_tmpABA->hd=_tmp5D0,((_tmpABA->tl=
_tmp5CF,_tmpABA))))));}}}{struct Cyc_Absyn_Stmt*_tmp5D2=Cyc_Absyn_exp_stmt(
_tmp5C3,0);struct Cyc_List_List*_tmpABB;struct Cyc_Absyn_Stmt*_tmp5D3=Cyc_Absyn_seq_stmts(((
struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(((_tmpABB=
_cycalloc(sizeof(*_tmpABB)),((_tmpABB->hd=_tmp5D2,((_tmpABB->tl=_tmp5CF,_tmpABB))))))),
0);(void*)(e->r=(void*)Cyc_Toc_stmt_exp_r(Cyc_Absyn_declare_stmt(_tmp5C2,
tunion_ctype,0,_tmp5D3,0)));}}goto _LL1E0;}}_LL229: if(*((int*)_tmp3C3)!= 33)goto
_LL22B;_LL22A: goto _LL22C;_LL22B: if(*((int*)_tmp3C3)!= 34)goto _LL22D;_LL22C: goto
_LL1E0;_LL22D: if(*((int*)_tmp3C3)!= 35)goto _LL22F;_tmp411=((struct Cyc_Absyn_Malloc_e_struct*)
_tmp3C3)->f1;_tmp412=_tmp411.is_calloc;_tmp413=_tmp411.rgn;_tmp414=_tmp411.elt_type;
_tmp415=_tmp411.num_elts;_tmp416=_tmp411.fat_result;_LL22E: {void*t_c=Cyc_Toc_typ_to_c(*((
void**)_check_null(_tmp414)));Cyc_Toc_exp_to_c(nv,_tmp415);if(_tmp416){struct
_tuple1*_tmp5D6=Cyc_Toc_temp_var();struct _tuple1*_tmp5D7=Cyc_Toc_temp_var();
struct Cyc_Absyn_Exp*pexp;struct Cyc_Absyn_Exp*xexp;struct Cyc_Absyn_Exp*rexp;if(
_tmp412){xexp=_tmp415;if(_tmp413 != 0  && !Cyc_Absyn_no_regions){struct Cyc_Absyn_Exp*
rgn=(struct Cyc_Absyn_Exp*)_tmp413;Cyc_Toc_exp_to_c(nv,rgn);pexp=Cyc_Toc_rcalloc_exp(
rgn,Cyc_Absyn_sizeoftyp_exp(t_c,0),Cyc_Absyn_var_exp(_tmp5D6,0));}else{pexp=Cyc_Toc_calloc_exp(*
_tmp414,Cyc_Absyn_sizeoftyp_exp(t_c,0),Cyc_Absyn_var_exp(_tmp5D6,0));}{struct Cyc_Absyn_Exp*
_tmpABC[3];rexp=Cyc_Absyn_fncall_exp(Cyc_Toc__tag_dyneither_e,((_tmpABC[2]=Cyc_Absyn_var_exp(
_tmp5D6,0),((_tmpABC[1]=Cyc_Absyn_sizeoftyp_exp(t_c,0),((_tmpABC[0]=Cyc_Absyn_var_exp(
_tmp5D7,0),((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(
_tag_dyneither(_tmpABC,sizeof(struct Cyc_Absyn_Exp*),3)))))))),0);}}else{xexp=Cyc_Absyn_times_exp(
Cyc_Absyn_sizeoftyp_exp(t_c,0),_tmp415,0);if(_tmp413 != 0  && !Cyc_Absyn_no_regions){
struct Cyc_Absyn_Exp*rgn=(struct Cyc_Absyn_Exp*)_tmp413;Cyc_Toc_exp_to_c(nv,rgn);
pexp=Cyc_Toc_rmalloc_exp(rgn,Cyc_Absyn_var_exp(_tmp5D6,0));}else{pexp=Cyc_Toc_malloc_exp(*
_tmp414,Cyc_Absyn_var_exp(_tmp5D6,0));}{struct Cyc_Absyn_Exp*_tmpABD[3];rexp=Cyc_Absyn_fncall_exp(
Cyc_Toc__tag_dyneither_e,((_tmpABD[2]=Cyc_Absyn_var_exp(_tmp5D6,0),((_tmpABD[1]=
Cyc_Absyn_uint_exp(1,0),((_tmpABD[0]=Cyc_Absyn_var_exp(_tmp5D7,0),((struct Cyc_List_List*(*)(
struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmpABD,sizeof(struct Cyc_Absyn_Exp*),
3)))))))),0);}}{struct Cyc_Absyn_Stmt*_tmp5DA=Cyc_Absyn_declare_stmt(_tmp5D6,Cyc_Absyn_uint_typ,(
struct Cyc_Absyn_Exp*)xexp,Cyc_Absyn_declare_stmt(_tmp5D7,Cyc_Absyn_cstar_typ(t_c,
Cyc_Toc_mt_tq),(struct Cyc_Absyn_Exp*)pexp,Cyc_Absyn_exp_stmt(rexp,0),0),0);(void*)(
e->r=(void*)Cyc_Toc_stmt_exp_r(_tmp5DA));}}else{struct Cyc_Absyn_Exp*_tmp5DB=Cyc_Absyn_sizeoftyp_exp(
t_c,0);{void*_tmp5DC=(void*)_tmp415->r;union Cyc_Absyn_Cnst_union _tmp5DD;int
_tmp5DE;_LL30C: if(*((int*)_tmp5DC)!= 0)goto _LL30E;_tmp5DD=((struct Cyc_Absyn_Const_e_struct*)
_tmp5DC)->f1;if(((((struct Cyc_Absyn_Const_e_struct*)_tmp5DC)->f1).Int_c).tag != 2)
goto _LL30E;_tmp5DE=(_tmp5DD.Int_c).f2;if(_tmp5DE != 1)goto _LL30E;_LL30D: goto
_LL30B;_LL30E:;_LL30F: _tmp5DB=Cyc_Absyn_times_exp(_tmp5DB,_tmp415,0);goto _LL30B;
_LL30B:;}if(_tmp413 != 0  && !Cyc_Absyn_no_regions){struct Cyc_Absyn_Exp*rgn=(
struct Cyc_Absyn_Exp*)_tmp413;Cyc_Toc_exp_to_c(nv,rgn);(void*)(e->r=(void*)((void*)(
Cyc_Toc_rmalloc_exp(rgn,_tmp5DB))->r));}else{(void*)(e->r=(void*)((void*)(Cyc_Toc_malloc_exp(*
_tmp414,_tmp5DB))->r));}}goto _LL1E0;}_LL22F: if(*((int*)_tmp3C3)!= 36)goto _LL231;
_tmp417=((struct Cyc_Absyn_Swap_e_struct*)_tmp3C3)->f1;_tmp418=((struct Cyc_Absyn_Swap_e_struct*)
_tmp3C3)->f2;_LL230: {int is_dyneither_ptr=0;void*e1_old_typ=(void*)((struct Cyc_Core_Opt*)
_check_null(_tmp417->topt))->v;void*e2_old_typ=(void*)((struct Cyc_Core_Opt*)
_check_null(_tmp418->topt))->v;if(!Cyc_Tcutil_is_pointer_or_boxed(e1_old_typ,&
is_dyneither_ptr)){const char*_tmpAC0;void*_tmpABF;(_tmpABF=0,((int(*)(struct
_dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmpAC0="Swap_e: is_pointer_or_boxed: not a pointer or boxed type",
_tag_dyneither(_tmpAC0,sizeof(char),57))),_tag_dyneither(_tmpABF,sizeof(void*),0)));}{
struct Cyc_Absyn_Exp*swap_fn;if(is_dyneither_ptr)swap_fn=Cyc_Toc__swap_dyneither_e;
else{swap_fn=Cyc_Toc__swap_word_e;}if(!Cyc_Absyn_is_lvalue(_tmp417)){const char*
_tmpAC4;void*_tmpAC3[1];struct Cyc_String_pa_struct _tmpAC2;(_tmpAC2.tag=0,((
_tmpAC2.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_exp2string(
_tmp417)),((_tmpAC3[0]=& _tmpAC2,((int(*)(struct _dyneither_ptr fmt,struct
_dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmpAC4="Swap_e: %s is not an l-value\n",
_tag_dyneither(_tmpAC4,sizeof(char),30))),_tag_dyneither(_tmpAC3,sizeof(void*),1)))))));}
if(!Cyc_Absyn_is_lvalue(_tmp418)){const char*_tmpAC8;void*_tmpAC7[1];struct Cyc_String_pa_struct
_tmpAC6;(_tmpAC6.tag=0,((_tmpAC6.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Absynpp_exp2string(_tmp418)),((_tmpAC7[0]=& _tmpAC6,((int(*)(struct
_dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmpAC8="Swap_e: %s is not an l-value\n",
_tag_dyneither(_tmpAC8,sizeof(char),30))),_tag_dyneither(_tmpAC7,sizeof(void*),1)))))));}
Cyc_Toc_exp_to_c(nv,_tmp417);Cyc_Toc_exp_to_c(nv,_tmp418);{struct Cyc_Absyn_Exp*
_tmpAC9[2];(void*)(e->r=(void*)Cyc_Toc_fncall_exp_r(swap_fn,((_tmpAC9[1]=Cyc_Absyn_address_exp(
_tmp418,0),((_tmpAC9[0]=Cyc_Absyn_address_exp(_tmp417,0),((struct Cyc_List_List*(*)(
struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmpAC9,sizeof(struct Cyc_Absyn_Exp*),
2))))))));}goto _LL1E0;}}_LL231: if(*((int*)_tmp3C3)!= 38)goto _LL233;_tmp419=((
struct Cyc_Absyn_StmtExp_e_struct*)_tmp3C3)->f1;_LL232: Cyc_Toc_stmt_to_c(nv,
_tmp419);goto _LL1E0;_LL233: if(*((int*)_tmp3C3)!= 37)goto _LL235;_LL234: {const
char*_tmpACC;void*_tmpACB;(_tmpACB=0,((int(*)(struct _dyneither_ptr fmt,struct
_dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmpACC="UnresolvedMem",_tag_dyneither(
_tmpACC,sizeof(char),14))),_tag_dyneither(_tmpACB,sizeof(void*),0)));}_LL235: if(*((
int*)_tmp3C3)!= 27)goto _LL237;_LL236: {const char*_tmpACF;void*_tmpACE;(_tmpACE=0,((
int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_unimp)(((_tmpACF="compoundlit",
_tag_dyneither(_tmpACF,sizeof(char),12))),_tag_dyneither(_tmpACE,sizeof(void*),0)));}
_LL237: if(*((int*)_tmp3C3)!= 39)goto _LL1E0;_LL238: {const char*_tmpAD2;void*
_tmpAD1;(_tmpAD1=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((
_tmpAD2="valueof(-)",_tag_dyneither(_tmpAD2,sizeof(char),11))),_tag_dyneither(
_tmpAD1,sizeof(void*),0)));}_LL1E0:;}}static struct Cyc_Absyn_Stmt*Cyc_Toc_if_neq_stmt(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Absyn_Stmt*fail_stmt);
static struct Cyc_Absyn_Stmt*Cyc_Toc_if_neq_stmt(struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*
e2,struct Cyc_Absyn_Stmt*fail_stmt){return Cyc_Absyn_ifthenelse_stmt(Cyc_Absyn_neq_exp(
e1,e2,0),fail_stmt,Cyc_Toc_skip_stmt_dl(),0);}struct _tuple13{struct Cyc_Toc_Env*
f1;struct Cyc_List_List*f2;struct Cyc_Absyn_Stmt*f3;};struct _tuple14{struct _tuple1*
f1;void*f2;};struct _tuple15{struct Cyc_List_List*f1;struct Cyc_Absyn_Pat*f2;};
static struct _tuple13 Cyc_Toc_xlate_pat(struct Cyc_Toc_Env*nv,struct _RegionHandle*
rgn,void*t,struct Cyc_Absyn_Exp*r,struct Cyc_Absyn_Exp*path,struct Cyc_Absyn_Pat*p,
struct Cyc_Absyn_Stmt**tag_fail_stmt,struct Cyc_Absyn_Stmt*fail_stmt,struct Cyc_List_List*
decls);static struct _tuple13 Cyc_Toc_xlate_pat(struct Cyc_Toc_Env*nv,struct
_RegionHandle*rgn,void*t,struct Cyc_Absyn_Exp*r,struct Cyc_Absyn_Exp*path,struct
Cyc_Absyn_Pat*p,struct Cyc_Absyn_Stmt**tag_fail_stmt,struct Cyc_Absyn_Stmt*
fail_stmt,struct Cyc_List_List*decls){struct Cyc_Absyn_Stmt*s;{void*_tmp5EE=(void*)
p->r;struct Cyc_Absyn_Vardecl*_tmp5EF;struct Cyc_Absyn_Vardecl _tmp5F0;struct
_tuple1*_tmp5F1;struct Cyc_Absyn_Pat*_tmp5F2;struct Cyc_Absyn_Vardecl*_tmp5F3;
struct Cyc_Absyn_Vardecl _tmp5F4;struct _tuple1*_tmp5F5;struct Cyc_Absyn_Vardecl*
_tmp5F6;struct Cyc_Absyn_Pat*_tmp5F7;void*_tmp5F8;int _tmp5F9;char _tmp5FA;struct
_dyneither_ptr _tmp5FB;struct Cyc_Absyn_Enumdecl*_tmp5FC;struct Cyc_Absyn_Enumfield*
_tmp5FD;void*_tmp5FE;struct Cyc_Absyn_Enumfield*_tmp5FF;struct Cyc_Absyn_Tuniondecl*
_tmp600;struct Cyc_Absyn_Tunionfield*_tmp601;struct Cyc_List_List*_tmp602;struct
Cyc_Absyn_Pat*_tmp603;struct Cyc_Absyn_Pat _tmp604;void*_tmp605;struct Cyc_Absyn_Tuniondecl*
_tmp606;struct Cyc_Absyn_Tunionfield*_tmp607;struct Cyc_List_List*_tmp608;struct
Cyc_Absyn_Tuniondecl*_tmp609;struct Cyc_Absyn_Tunionfield*_tmp60A;struct Cyc_List_List*
_tmp60B;struct Cyc_List_List*_tmp60C;struct Cyc_List_List*_tmp60D;struct Cyc_Absyn_AggrInfo
_tmp60E;union Cyc_Absyn_AggrInfoU_union _tmp60F;struct Cyc_List_List*_tmp610;struct
Cyc_Absyn_Pat*_tmp611;_LL311: if(_tmp5EE <= (void*)2)goto _LL315;if(*((int*)_tmp5EE)
!= 0)goto _LL313;_tmp5EF=((struct Cyc_Absyn_Var_p_struct*)_tmp5EE)->f1;_tmp5F0=*
_tmp5EF;_tmp5F1=_tmp5F0.name;_tmp5F2=((struct Cyc_Absyn_Var_p_struct*)_tmp5EE)->f2;
_LL312: return Cyc_Toc_xlate_pat(Cyc_Toc_add_varmap(rgn,nv,_tmp5F1,r),rgn,t,r,path,
_tmp5F2,tag_fail_stmt,fail_stmt,decls);_LL313: if(*((int*)_tmp5EE)!= 2)goto _LL315;
_tmp5F3=((struct Cyc_Absyn_TagInt_p_struct*)_tmp5EE)->f2;_tmp5F4=*_tmp5F3;_tmp5F5=
_tmp5F4.name;_LL314: nv=Cyc_Toc_add_varmap(rgn,nv,_tmp5F5,r);goto _LL316;_LL315:
if((int)_tmp5EE != 0)goto _LL317;_LL316: s=Cyc_Toc_skip_stmt_dl();goto _LL310;_LL317:
if(_tmp5EE <= (void*)2)goto _LL319;if(*((int*)_tmp5EE)!= 1)goto _LL319;_tmp5F6=((
struct Cyc_Absyn_Reference_p_struct*)_tmp5EE)->f1;_tmp5F7=((struct Cyc_Absyn_Reference_p_struct*)
_tmp5EE)->f2;_LL318: {struct _tuple1*_tmp612=Cyc_Toc_temp_var();{struct _tuple14*
_tmpAD5;struct Cyc_List_List*_tmpAD4;decls=((_tmpAD4=_region_malloc(rgn,sizeof(*
_tmpAD4)),((_tmpAD4->hd=((_tmpAD5=_region_malloc(rgn,sizeof(*_tmpAD5)),((_tmpAD5->f1=
_tmp612,((_tmpAD5->f2=Cyc_Absyn_cstar_typ(Cyc_Toc_typ_to_c(t),Cyc_Toc_mt_tq),
_tmpAD5)))))),((_tmpAD4->tl=decls,_tmpAD4))))));}nv=Cyc_Toc_add_varmap(rgn,nv,
_tmp5F6->name,Cyc_Absyn_var_exp(_tmp612,0));s=Cyc_Absyn_assign_stmt(Cyc_Absyn_var_exp(
_tmp612,0),Cyc_Toc_cast_it(Cyc_Absyn_cstar_typ(Cyc_Toc_typ_to_c(t),Cyc_Toc_mt_tq),
Cyc_Absyn_address_exp(path,0)),0);{struct _tuple13 _tmp615=Cyc_Toc_xlate_pat(nv,
rgn,t,r,path,_tmp5F7,tag_fail_stmt,fail_stmt,decls);_tmp615.f3=Cyc_Absyn_seq_stmt(
s,_tmp615.f3,0);return _tmp615;}}_LL319: if((int)_tmp5EE != 1)goto _LL31B;_LL31A: s=
Cyc_Toc_if_neq_stmt(r,Cyc_Absyn_signed_int_exp(0,0),fail_stmt);goto _LL310;_LL31B:
if(_tmp5EE <= (void*)2)goto _LL31D;if(*((int*)_tmp5EE)!= 7)goto _LL31D;_tmp5F8=(
void*)((struct Cyc_Absyn_Int_p_struct*)_tmp5EE)->f1;_tmp5F9=((struct Cyc_Absyn_Int_p_struct*)
_tmp5EE)->f2;_LL31C: s=Cyc_Toc_if_neq_stmt(r,Cyc_Absyn_int_exp(_tmp5F8,_tmp5F9,0),
fail_stmt);goto _LL310;_LL31D: if(_tmp5EE <= (void*)2)goto _LL31F;if(*((int*)_tmp5EE)
!= 8)goto _LL31F;_tmp5FA=((struct Cyc_Absyn_Char_p_struct*)_tmp5EE)->f1;_LL31E: s=
Cyc_Toc_if_neq_stmt(r,Cyc_Absyn_char_exp(_tmp5FA,0),fail_stmt);goto _LL310;_LL31F:
if(_tmp5EE <= (void*)2)goto _LL321;if(*((int*)_tmp5EE)!= 9)goto _LL321;_tmp5FB=((
struct Cyc_Absyn_Float_p_struct*)_tmp5EE)->f1;_LL320: s=Cyc_Toc_if_neq_stmt(r,Cyc_Absyn_float_exp(
_tmp5FB,0),fail_stmt);goto _LL310;_LL321: if(_tmp5EE <= (void*)2)goto _LL323;if(*((
int*)_tmp5EE)!= 10)goto _LL323;_tmp5FC=((struct Cyc_Absyn_Enum_p_struct*)_tmp5EE)->f1;
_tmp5FD=((struct Cyc_Absyn_Enum_p_struct*)_tmp5EE)->f2;_LL322:{struct Cyc_Absyn_Enum_e_struct
_tmpAD8;struct Cyc_Absyn_Enum_e_struct*_tmpAD7;s=Cyc_Toc_if_neq_stmt(r,Cyc_Absyn_new_exp((
void*)((_tmpAD7=_cycalloc(sizeof(*_tmpAD7)),((_tmpAD7[0]=((_tmpAD8.tag=33,((
_tmpAD8.f1=_tmp5FD->name,((_tmpAD8.f2=(struct Cyc_Absyn_Enumdecl*)_tmp5FC,((
_tmpAD8.f3=(struct Cyc_Absyn_Enumfield*)_tmp5FD,_tmpAD8)))))))),_tmpAD7)))),0),
fail_stmt);}goto _LL310;_LL323: if(_tmp5EE <= (void*)2)goto _LL325;if(*((int*)
_tmp5EE)!= 11)goto _LL325;_tmp5FE=(void*)((struct Cyc_Absyn_AnonEnum_p_struct*)
_tmp5EE)->f1;_tmp5FF=((struct Cyc_Absyn_AnonEnum_p_struct*)_tmp5EE)->f2;_LL324:{
struct Cyc_Absyn_AnonEnum_e_struct _tmpADB;struct Cyc_Absyn_AnonEnum_e_struct*
_tmpADA;s=Cyc_Toc_if_neq_stmt(r,Cyc_Absyn_new_exp((void*)((_tmpADA=_cycalloc(
sizeof(*_tmpADA)),((_tmpADA[0]=((_tmpADB.tag=34,((_tmpADB.f1=_tmp5FF->name,((
_tmpADB.f2=(void*)_tmp5FE,((_tmpADB.f3=(struct Cyc_Absyn_Enumfield*)_tmp5FF,
_tmpADB)))))))),_tmpADA)))),0),fail_stmt);}goto _LL310;_LL325: if(_tmp5EE <= (void*)
2)goto _LL327;if(*((int*)_tmp5EE)!= 6)goto _LL327;_tmp600=((struct Cyc_Absyn_Tunion_p_struct*)
_tmp5EE)->f1;_tmp601=((struct Cyc_Absyn_Tunion_p_struct*)_tmp5EE)->f2;_tmp602=((
struct Cyc_Absyn_Tunion_p_struct*)_tmp5EE)->f3;if(_tmp602 != 0)goto _LL327;if(!(!
_tmp600->is_flat))goto _LL327;_LL326: {struct Cyc_Absyn_Exp*cmp_exp;if(_tmp600->is_xtunion)
cmp_exp=Cyc_Absyn_var_exp(_tmp601->name,0);else{cmp_exp=Cyc_Toc_tunion_tag(
_tmp600,_tmp601->name,0);r=Cyc_Toc_cast_it(Cyc_Absyn_sint_typ,r);}s=Cyc_Toc_if_neq_stmt(
r,cmp_exp,fail_stmt);goto _LL310;}_LL327: if(_tmp5EE <= (void*)2)goto _LL329;if(*((
int*)_tmp5EE)!= 4)goto _LL329;_tmp603=((struct Cyc_Absyn_Pointer_p_struct*)_tmp5EE)->f1;
_tmp604=*_tmp603;_tmp605=(void*)_tmp604.r;if(_tmp605 <= (void*)2)goto _LL329;if(*((
int*)_tmp605)!= 6)goto _LL329;_tmp606=((struct Cyc_Absyn_Tunion_p_struct*)_tmp605)->f1;
_tmp607=((struct Cyc_Absyn_Tunion_p_struct*)_tmp605)->f2;_tmp608=((struct Cyc_Absyn_Tunion_p_struct*)
_tmp605)->f3;if(!(_tmp608 != 0  && !_tmp606->is_flat))goto _LL329;_LL328: s=Cyc_Toc_skip_stmt_dl();{
int cnt=1;const char*_tmpADC;struct _tuple1*tufstrct=Cyc_Toc_collapse_qvar_tag(
_tmp607->name,((_tmpADC="_struct",_tag_dyneither(_tmpADC,sizeof(char),8))));
struct Cyc_Absyn_Exp*rcast=Cyc_Toc_cast_it(Cyc_Absyn_cstar_typ(Cyc_Absyn_strctq(
tufstrct),Cyc_Toc_mt_tq),r);struct Cyc_List_List*_tmp61A=_tmp607->typs;for(0;
_tmp608 != 0;(((_tmp608=_tmp608->tl,_tmp61A=((struct Cyc_List_List*)_check_null(
_tmp61A))->tl)),++ cnt)){struct Cyc_Absyn_Pat*_tmp61B=(struct Cyc_Absyn_Pat*)
_tmp608->hd;if((void*)_tmp61B->r == (void*)0)continue;{void*_tmp61C=(*((struct
_tuple4*)((struct Cyc_List_List*)_check_null(_tmp61A))->hd)).f2;struct _tuple1*
_tmp61D=Cyc_Toc_temp_var();void*_tmp61E=(void*)((struct Cyc_Core_Opt*)_check_null(
_tmp61B->topt))->v;void*_tmp61F=Cyc_Toc_typ_to_c(_tmp61E);struct Cyc_Absyn_Exp*
_tmp620=Cyc_Absyn_aggrarrow_exp(rcast,Cyc_Absyn_fieldname(cnt),0);if(Cyc_Toc_is_void_star(
Cyc_Toc_typ_to_c(_tmp61C)))_tmp620=Cyc_Toc_cast_it(_tmp61F,_tmp620);{struct
_tuple14*_tmpADF;struct Cyc_List_List*_tmpADE;decls=((_tmpADE=_region_malloc(rgn,
sizeof(*_tmpADE)),((_tmpADE->hd=((_tmpADF=_region_malloc(rgn,sizeof(*_tmpADF)),((
_tmpADF->f1=_tmp61D,((_tmpADF->f2=_tmp61F,_tmpADF)))))),((_tmpADE->tl=decls,
_tmpADE))))));}{struct _tuple13 _tmp623=Cyc_Toc_xlate_pat(nv,rgn,_tmp61E,Cyc_Absyn_var_exp(
_tmp61D,0),_tmp620,_tmp61B,(struct Cyc_Absyn_Stmt**)& fail_stmt,fail_stmt,decls);
nv=_tmp623.f1;decls=_tmp623.f2;{struct Cyc_Absyn_Stmt*_tmp624=_tmp623.f3;struct
Cyc_Absyn_Stmt*_tmp625=Cyc_Absyn_assign_stmt(Cyc_Absyn_var_exp(_tmp61D,0),
_tmp620,0);s=Cyc_Absyn_seq_stmt(s,Cyc_Absyn_seq_stmt(_tmp625,_tmp624,0),0);}}}}{
struct Cyc_Absyn_Exp*test_exp;if(_tmp606->is_xtunion){struct Cyc_Absyn_Exp*e2=Cyc_Toc_cast_it(
Cyc_Absyn_cstar_typ(Cyc_Absyn_void_star_typ(),Cyc_Toc_mt_tq),r);struct Cyc_Absyn_Exp*
e1=Cyc_Absyn_deref_exp(e2,0);struct Cyc_Absyn_Exp*e=Cyc_Absyn_var_exp(_tmp607->name,
0);test_exp=Cyc_Absyn_neq_exp(e1,e,0);s=Cyc_Absyn_seq_stmt(Cyc_Absyn_ifthenelse_stmt(
test_exp,fail_stmt,Cyc_Toc_skip_stmt_dl(),0),s,0);}else{struct Cyc_Absyn_Exp*e3=
Cyc_Toc_cast_it(Cyc_Absyn_cstar_typ(Cyc_Absyn_sint_typ,Cyc_Toc_mt_tq),r);struct
Cyc_Absyn_Exp*e1=Cyc_Absyn_deref_exp(e3,0);struct Cyc_Absyn_Exp*e=Cyc_Toc_tunion_tag(
_tmp606,_tmp607->name,1);s=Cyc_Absyn_seq_stmt(Cyc_Absyn_ifthenelse_stmt(Cyc_Absyn_neq_exp(
e1,e,0),fail_stmt,Cyc_Toc_skip_stmt_dl(),0),s,0);if(tag_fail_stmt != 0){int
max_tag=Cyc_Toc_num_void_tags(_tmp606);if(max_tag != 0){struct Cyc_Absyn_Exp*
max_tag_exp=Cyc_Absyn_uint_exp((unsigned int)max_tag,0);struct Cyc_Absyn_Exp*e5=
Cyc_Absyn_lte_exp(r,Cyc_Toc_cast_it(Cyc_Absyn_void_star_typ(),max_tag_exp),0);s=
Cyc_Absyn_seq_stmt(Cyc_Absyn_ifthenelse_stmt(e5,*tag_fail_stmt,Cyc_Toc_skip_stmt_dl(),
0),s,0);}}}goto _LL310;}}_LL329: if(_tmp5EE <= (void*)2)goto _LL32B;if(*((int*)
_tmp5EE)!= 6)goto _LL32B;_tmp609=((struct Cyc_Absyn_Tunion_p_struct*)_tmp5EE)->f1;
_tmp60A=((struct Cyc_Absyn_Tunion_p_struct*)_tmp5EE)->f2;_tmp60B=((struct Cyc_Absyn_Tunion_p_struct*)
_tmp5EE)->f3;if(!_tmp609->is_flat)goto _LL32B;_LL32A: {struct _tuple1 _tmp628;
struct _dyneither_ptr*_tmp629;struct _tuple1*_tmp627=_tmp60A->name;_tmp628=*
_tmp627;_tmp629=_tmp628.f2;r=Cyc_Absyn_aggrmember_exp(r,_tmp629,0);path=Cyc_Absyn_aggrmember_exp(
path,_tmp629,0);s=Cyc_Absyn_ifthenelse_stmt(Cyc_Absyn_neq_exp(Cyc_Absyn_aggrmember_exp(
path,Cyc_Toc_tag_sp,0),Cyc_Toc_tunion_tag(_tmp609,_tmp60A->name,1),0),fail_stmt,
Cyc_Toc_skip_stmt_dl(),0);{int cnt=1;for(0;_tmp60B != 0;(_tmp60B=_tmp60B->tl,++ cnt)){
struct Cyc_Absyn_Pat*_tmp62A=(struct Cyc_Absyn_Pat*)_tmp60B->hd;if((void*)_tmp62A->r
== (void*)0)continue;{struct _tuple1*_tmp62B=Cyc_Toc_temp_var();void*_tmp62C=(
void*)((struct Cyc_Core_Opt*)_check_null(_tmp62A->topt))->v;{struct _tuple14*
_tmpAE2;struct Cyc_List_List*_tmpAE1;decls=((_tmpAE1=_region_malloc(rgn,sizeof(*
_tmpAE1)),((_tmpAE1->hd=((_tmpAE2=_region_malloc(rgn,sizeof(*_tmpAE2)),((_tmpAE2->f1=
_tmp62B,((_tmpAE2->f2=Cyc_Toc_typ_to_c(_tmp62C),_tmpAE2)))))),((_tmpAE1->tl=
decls,_tmpAE1))))));}{struct _tuple13 _tmp62F=Cyc_Toc_xlate_pat(nv,rgn,_tmp62C,Cyc_Absyn_var_exp(
_tmp62B,0),Cyc_Absyn_aggrmember_exp(path,Cyc_Absyn_fieldname(cnt),0),_tmp62A,(
struct Cyc_Absyn_Stmt**)& fail_stmt,fail_stmt,decls);nv=_tmp62F.f1;decls=_tmp62F.f2;{
struct Cyc_Absyn_Stmt*_tmp630=_tmp62F.f3;struct Cyc_Absyn_Stmt*_tmp631=Cyc_Absyn_assign_stmt(
Cyc_Absyn_var_exp(_tmp62B,0),Cyc_Absyn_aggrmember_exp(r,Cyc_Absyn_fieldname(cnt),
0),0);s=Cyc_Absyn_seq_stmt(s,Cyc_Absyn_seq_stmt(_tmp631,_tmp630,0),0);}}}}goto
_LL310;}}_LL32B: if(_tmp5EE <= (void*)2)goto _LL32D;if(*((int*)_tmp5EE)!= 6)goto
_LL32D;_tmp60C=((struct Cyc_Absyn_Tunion_p_struct*)_tmp5EE)->f3;_LL32C: _tmp60D=
_tmp60C;goto _LL32E;_LL32D: if(_tmp5EE <= (void*)2)goto _LL32F;if(*((int*)_tmp5EE)!= 
3)goto _LL32F;_tmp60D=((struct Cyc_Absyn_Tuple_p_struct*)_tmp5EE)->f1;_LL32E: s=Cyc_Toc_skip_stmt_dl();{
int cnt=1;for(0;_tmp60D != 0;(_tmp60D=_tmp60D->tl,++ cnt)){struct Cyc_Absyn_Pat*
_tmp632=(struct Cyc_Absyn_Pat*)_tmp60D->hd;if((void*)_tmp632->r == (void*)0)
continue;{struct _tuple1*_tmp633=Cyc_Toc_temp_var();void*_tmp634=(void*)((struct
Cyc_Core_Opt*)_check_null(_tmp632->topt))->v;{struct _tuple14*_tmpAE5;struct Cyc_List_List*
_tmpAE4;decls=((_tmpAE4=_region_malloc(rgn,sizeof(*_tmpAE4)),((_tmpAE4->hd=((
_tmpAE5=_region_malloc(rgn,sizeof(*_tmpAE5)),((_tmpAE5->f1=_tmp633,((_tmpAE5->f2=
Cyc_Toc_typ_to_c(_tmp634),_tmpAE5)))))),((_tmpAE4->tl=decls,_tmpAE4))))));}{
struct _tuple13 _tmp637=Cyc_Toc_xlate_pat(nv,rgn,_tmp634,Cyc_Absyn_var_exp(_tmp633,
0),Cyc_Absyn_aggrmember_exp(path,Cyc_Absyn_fieldname(cnt),0),_tmp632,(struct Cyc_Absyn_Stmt**)&
fail_stmt,fail_stmt,decls);nv=_tmp637.f1;decls=_tmp637.f2;{struct Cyc_Absyn_Stmt*
_tmp638=_tmp637.f3;struct Cyc_Absyn_Stmt*_tmp639=Cyc_Absyn_assign_stmt(Cyc_Absyn_var_exp(
_tmp633,0),Cyc_Absyn_aggrmember_exp(r,Cyc_Absyn_fieldname(cnt),0),0);s=Cyc_Absyn_seq_stmt(
s,Cyc_Absyn_seq_stmt(_tmp639,_tmp638,0),0);}}}}goto _LL310;}_LL32F: if(_tmp5EE <= (
void*)2)goto _LL331;if(*((int*)_tmp5EE)!= 5)goto _LL331;_tmp60E=((struct Cyc_Absyn_Aggr_p_struct*)
_tmp5EE)->f1;_tmp60F=_tmp60E.aggr_info;_tmp610=((struct Cyc_Absyn_Aggr_p_struct*)
_tmp5EE)->f3;_LL330: {struct Cyc_Absyn_Aggrdecl*_tmp63A=Cyc_Absyn_get_known_aggrdecl(
_tmp60F);s=Cyc_Toc_skip_stmt_dl();for(0;_tmp610 != 0;_tmp610=_tmp610->tl){struct
_tuple15*_tmp63B=(struct _tuple15*)_tmp610->hd;struct Cyc_Absyn_Pat*_tmp63C=(*
_tmp63B).f2;if((void*)_tmp63C->r == (void*)0)continue;{struct _dyneither_ptr*f;{
void*_tmp63D=(void*)((struct Cyc_List_List*)_check_null((*_tmp63B).f1))->hd;
struct _dyneither_ptr*_tmp63E;_LL33A: if(*((int*)_tmp63D)!= 1)goto _LL33C;_tmp63E=((
struct Cyc_Absyn_FieldName_struct*)_tmp63D)->f1;_LL33B: f=_tmp63E;goto _LL339;
_LL33C:;_LL33D:(int)_throw((void*)Cyc_Toc_Match_error);_LL339:;}{struct _tuple1*
_tmp63F=Cyc_Toc_temp_var();void*_tmp640=(void*)((struct Cyc_Core_Opt*)_check_null(
_tmp63C->topt))->v;void*_tmp641=Cyc_Toc_typ_to_c(_tmp640);{struct _tuple14*
_tmpAE8;struct Cyc_List_List*_tmpAE7;decls=((_tmpAE7=_region_malloc(rgn,sizeof(*
_tmpAE7)),((_tmpAE7->hd=((_tmpAE8=_region_malloc(rgn,sizeof(*_tmpAE8)),((_tmpAE8->f1=
_tmp63F,((_tmpAE8->f2=_tmp641,_tmpAE8)))))),((_tmpAE7->tl=decls,_tmpAE7))))));}{
struct _tuple13 _tmp644=Cyc_Toc_xlate_pat(nv,rgn,_tmp640,Cyc_Absyn_var_exp(_tmp63F,
0),Cyc_Absyn_aggrmember_exp(path,f,0),_tmp63C,(struct Cyc_Absyn_Stmt**)& fail_stmt,
fail_stmt,decls);nv=_tmp644.f1;decls=_tmp644.f2;{struct Cyc_Absyn_Exp*_tmp645=Cyc_Absyn_aggrmember_exp(
r,f,0);if(Cyc_Toc_is_void_star((void*)((struct Cyc_Absyn_Aggrfield*)_check_null(
Cyc_Absyn_lookup_field(((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp63A->impl))->fields,
f)))->type))_tmp645=Cyc_Toc_cast_it(_tmp641,_tmp645);{struct Cyc_Absyn_Stmt*
_tmp646=_tmp644.f3;struct Cyc_Absyn_Stmt*_tmp647=Cyc_Absyn_assign_stmt(Cyc_Absyn_var_exp(
_tmp63F,0),_tmp645,0);s=Cyc_Absyn_seq_stmt(s,Cyc_Absyn_seq_stmt(_tmp647,_tmp646,
0),0);}}}}}}goto _LL310;}_LL331: if(_tmp5EE <= (void*)2)goto _LL333;if(*((int*)
_tmp5EE)!= 4)goto _LL333;_tmp611=((struct Cyc_Absyn_Pointer_p_struct*)_tmp5EE)->f1;
_LL332: {struct _tuple1*_tmp648=Cyc_Toc_temp_var();void*_tmp649=(void*)((struct
Cyc_Core_Opt*)_check_null(_tmp611->topt))->v;{struct _tuple14*_tmpAEB;struct Cyc_List_List*
_tmpAEA;decls=((_tmpAEA=_region_malloc(rgn,sizeof(*_tmpAEA)),((_tmpAEA->hd=((
_tmpAEB=_region_malloc(rgn,sizeof(*_tmpAEB)),((_tmpAEB->f1=_tmp648,((_tmpAEB->f2=
Cyc_Toc_typ_to_c(_tmp649),_tmpAEB)))))),((_tmpAEA->tl=decls,_tmpAEA))))));}{
struct _tuple13 _tmp64C=Cyc_Toc_xlate_pat(nv,rgn,_tmp649,Cyc_Absyn_var_exp(_tmp648,
0),Cyc_Absyn_deref_exp(path,0),_tmp611,(struct Cyc_Absyn_Stmt**)& fail_stmt,
fail_stmt,decls);nv=_tmp64C.f1;decls=_tmp64C.f2;{struct Cyc_Absyn_Stmt*_tmp64D=
_tmp64C.f3;struct Cyc_Absyn_Stmt*_tmp64E=Cyc_Absyn_seq_stmt(Cyc_Absyn_assign_stmt(
Cyc_Absyn_var_exp(_tmp648,0),Cyc_Absyn_deref_exp(r,0),0),_tmp64D,0);if(Cyc_Toc_is_nullable(
t))s=Cyc_Absyn_seq_stmt(Cyc_Absyn_ifthenelse_stmt(Cyc_Absyn_eq_exp(r,Cyc_Absyn_signed_int_exp(
0,0),0),fail_stmt,Cyc_Toc_skip_stmt_dl(),0),_tmp64E,0);else{s=_tmp64E;}goto
_LL310;}}}_LL333: if(_tmp5EE <= (void*)2)goto _LL335;if(*((int*)_tmp5EE)!= 12)goto
_LL335;_LL334: {const char*_tmpAEE;void*_tmpAED;(_tmpAED=0,((int(*)(struct
_dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmpAEE="unknownid pat",
_tag_dyneither(_tmpAEE,sizeof(char),14))),_tag_dyneither(_tmpAED,sizeof(void*),0)));}
_LL335: if(_tmp5EE <= (void*)2)goto _LL337;if(*((int*)_tmp5EE)!= 13)goto _LL337;
_LL336: {const char*_tmpAF1;void*_tmpAF0;(_tmpAF0=0,((int(*)(struct _dyneither_ptr
fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmpAF1="unknowncall pat",
_tag_dyneither(_tmpAF1,sizeof(char),16))),_tag_dyneither(_tmpAF0,sizeof(void*),0)));}
_LL337: if(_tmp5EE <= (void*)2)goto _LL310;if(*((int*)_tmp5EE)!= 14)goto _LL310;
_LL338: {const char*_tmpAF4;void*_tmpAF3;(_tmpAF3=0,((int(*)(struct _dyneither_ptr
fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmpAF4="exp pat",
_tag_dyneither(_tmpAF4,sizeof(char),8))),_tag_dyneither(_tmpAF3,sizeof(void*),0)));}
_LL310:;}{struct _tuple13 _tmpAF5;return(_tmpAF5.f1=nv,((_tmpAF5.f2=decls,((
_tmpAF5.f3=s,_tmpAF5)))));}}struct _tuple16{struct _dyneither_ptr*f1;struct
_dyneither_ptr*f2;struct Cyc_Absyn_Switch_clause*f3;};static struct _tuple16*Cyc_Toc_gen_label(
struct _RegionHandle*r,struct Cyc_Absyn_Switch_clause*sc);static struct _tuple16*Cyc_Toc_gen_label(
struct _RegionHandle*r,struct Cyc_Absyn_Switch_clause*sc){struct _tuple16*_tmpAF6;
return(_tmpAF6=_region_malloc(r,sizeof(*_tmpAF6)),((_tmpAF6->f1=Cyc_Toc_fresh_label(),((
_tmpAF6->f2=Cyc_Toc_fresh_label(),((_tmpAF6->f3=sc,_tmpAF6)))))));}static int Cyc_Toc_is_mixed_tunion(
void*t);static int Cyc_Toc_is_mixed_tunion(void*t){{void*_tmp657=Cyc_Tcutil_compress(
t);struct Cyc_Absyn_TunionInfo _tmp658;union Cyc_Absyn_TunionInfoU_union _tmp659;
struct Cyc_Absyn_Tuniondecl**_tmp65A;struct Cyc_Absyn_Tuniondecl*_tmp65B;_LL33F:
if(_tmp657 <= (void*)4)goto _LL341;if(*((int*)_tmp657)!= 2)goto _LL341;_tmp658=((
struct Cyc_Absyn_TunionType_struct*)_tmp657)->f1;_tmp659=_tmp658.tunion_info;if((((((
struct Cyc_Absyn_TunionType_struct*)_tmp657)->f1).tunion_info).KnownTunion).tag != 
1)goto _LL341;_tmp65A=(_tmp659.KnownTunion).f1;_tmp65B=*_tmp65A;if(!(!_tmp65B->is_flat))
goto _LL341;_LL340: {int seen_novalue=0;int seen_value=0;{struct Cyc_List_List*
_tmp65C=(struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(_tmp65B->fields))->v;
for(0;(unsigned int)_tmp65C;_tmp65C=_tmp65C->tl){if(((struct Cyc_Absyn_Tunionfield*)
_tmp65C->hd)->typs == 0)seen_value=1;else{seen_novalue=1;}if(seen_value  && 
seen_novalue)return 1;}}goto _LL33E;}_LL341:;_LL342: goto _LL33E;_LL33E:;}return 0;}
static int Cyc_Toc_no_tag_test(struct Cyc_Absyn_Pat*p);static int Cyc_Toc_no_tag_test(
struct Cyc_Absyn_Pat*p){void*_tmp65D=(void*)p->r;struct Cyc_Absyn_Pat*_tmp65E;
struct Cyc_Absyn_Pat*_tmp65F;struct Cyc_Absyn_Pat _tmp660;void*_tmp661;struct Cyc_Absyn_Tuniondecl*
_tmp662;struct Cyc_Absyn_Tunionfield*_tmp663;struct Cyc_List_List*_tmp664;_LL344:
if(_tmp65D <= (void*)2)goto _LL348;if(*((int*)_tmp65D)!= 0)goto _LL346;_tmp65E=((
struct Cyc_Absyn_Var_p_struct*)_tmp65D)->f2;_LL345: return Cyc_Toc_no_tag_test(
_tmp65E);_LL346: if(*((int*)_tmp65D)!= 4)goto _LL348;_tmp65F=((struct Cyc_Absyn_Pointer_p_struct*)
_tmp65D)->f1;_tmp660=*_tmp65F;_tmp661=(void*)_tmp660.r;if(_tmp661 <= (void*)2)
goto _LL348;if(*((int*)_tmp661)!= 6)goto _LL348;_tmp662=((struct Cyc_Absyn_Tunion_p_struct*)
_tmp661)->f1;_tmp663=((struct Cyc_Absyn_Tunion_p_struct*)_tmp661)->f2;_tmp664=((
struct Cyc_Absyn_Tunion_p_struct*)_tmp661)->f3;if(!(_tmp664 != 0  && !_tmp662->is_flat))
goto _LL348;_LL347: return 0;_LL348:;_LL349: return 1;_LL343:;}static void Cyc_Toc_xlate_switch(
struct Cyc_Toc_Env*nv,struct Cyc_Absyn_Stmt*whole_s,struct Cyc_Absyn_Exp*e,struct
Cyc_List_List*scs);static void Cyc_Toc_xlate_switch(struct Cyc_Toc_Env*nv,struct Cyc_Absyn_Stmt*
whole_s,struct Cyc_Absyn_Exp*e,struct Cyc_List_List*scs){Cyc_Toc_exp_to_c(nv,e);{
void*_tmp665=(void*)((struct Cyc_Core_Opt*)_check_null(e->topt))->v;int
leave_as_switch;{void*_tmp666=Cyc_Tcutil_compress(_tmp665);_LL34B: if(_tmp666 <= (
void*)4)goto _LL34F;if(*((int*)_tmp666)!= 5)goto _LL34D;_LL34C: goto _LL34E;_LL34D:
if(*((int*)_tmp666)!= 12)goto _LL34F;_LL34E: leave_as_switch=1;goto _LL34A;_LL34F:;
_LL350: leave_as_switch=0;goto _LL34A;_LL34A:;}{struct Cyc_List_List*_tmp667=scs;
for(0;_tmp667 != 0;_tmp667=_tmp667->tl){if((struct Cyc_List_List*)((struct Cyc_Core_Opt*)
_check_null(((struct Cyc_Absyn_Switch_clause*)_tmp667->hd)->pat_vars))->v != 0  || ((
struct Cyc_Absyn_Switch_clause*)_tmp667->hd)->where_clause != 0){leave_as_switch=0;
break;}}}if(leave_as_switch){struct _dyneither_ptr*next_l=Cyc_Toc_fresh_label();{
struct Cyc_List_List*_tmp668=scs;for(0;_tmp668 != 0;_tmp668=_tmp668->tl){struct Cyc_Absyn_Stmt*
_tmp669=((struct Cyc_Absyn_Switch_clause*)_tmp668->hd)->body;((struct Cyc_Absyn_Switch_clause*)
_tmp668->hd)->body=Cyc_Absyn_label_stmt(next_l,_tmp669,0);next_l=Cyc_Toc_fresh_label();{
struct _RegionHandle _tmp66A=_new_region("rgn");struct _RegionHandle*rgn=& _tmp66A;
_push_region(rgn);Cyc_Toc_stmt_to_c(Cyc_Toc_switch_as_switch_env(rgn,nv,next_l),
_tmp669);;_pop_region(rgn);}}}return;}{struct _tuple1*v=Cyc_Toc_temp_var();struct
Cyc_Absyn_Exp*r=Cyc_Absyn_var_exp(v,0);struct Cyc_Absyn_Exp*path=Cyc_Absyn_var_exp(
v,0);struct _dyneither_ptr*end_l=Cyc_Toc_fresh_label();struct _RegionHandle _tmp66B=
_new_region("rgn");struct _RegionHandle*rgn=& _tmp66B;_push_region(rgn);{struct Cyc_Toc_Env*
_tmp66C=Cyc_Toc_share_env(rgn,nv);struct Cyc_List_List*lscs=((struct Cyc_List_List*(*)(
struct _RegionHandle*,struct _tuple16*(*f)(struct _RegionHandle*,struct Cyc_Absyn_Switch_clause*),
struct _RegionHandle*env,struct Cyc_List_List*x))Cyc_List_rmap_c)(rgn,Cyc_Toc_gen_label,
rgn,scs);struct Cyc_List_List*test_stmts=0;struct Cyc_List_List*nvs=0;struct Cyc_List_List*
decls=0;int is_tunion=Cyc_Toc_is_mixed_tunion(_tmp665);int needs_tag_test=
is_tunion;{struct Cyc_List_List*_tmp66D=lscs;for(0;_tmp66D != 0;_tmp66D=_tmp66D->tl){
struct Cyc_Absyn_Switch_clause*sc=(*((struct _tuple16*)_tmp66D->hd)).f3;struct
_dyneither_ptr*fail_lab=_tmp66D->tl == 0?end_l:(*((struct _tuple16*)((struct Cyc_List_List*)
_check_null(_tmp66D->tl))->hd)).f1;struct Cyc_Absyn_Stmt**tag_fail_stmt=0;if(
needs_tag_test  && !Cyc_Toc_no_tag_test(sc->pattern)){{struct Cyc_List_List*
_tmp66E=_tmp66D->tl;for(0;(unsigned int)_tmp66E;_tmp66E=_tmp66E->tl){if(Cyc_Toc_no_tag_test(((*((
struct _tuple16*)_tmp66E->hd)).f3)->pattern)){{struct Cyc_Absyn_Stmt**_tmpAF7;
tag_fail_stmt=((_tmpAF7=_region_malloc(rgn,sizeof(*_tmpAF7)),((_tmpAF7[0]=Cyc_Absyn_goto_stmt((*((
struct _tuple16*)_tmp66E->hd)).f1,0),_tmpAF7))));}needs_tag_test=0;break;}}}if(
tag_fail_stmt == 0){struct Cyc_Absyn_Stmt**_tmpAF8;tag_fail_stmt=((_tmpAF8=
_region_malloc(rgn,sizeof(*_tmpAF8)),((_tmpAF8[0]=Cyc_Absyn_goto_stmt(fail_lab,0),
_tmpAF8))));}}{struct Cyc_Toc_Env*_tmp672;struct Cyc_List_List*_tmp673;struct Cyc_Absyn_Stmt*
_tmp674;struct _tuple13 _tmp671=Cyc_Toc_xlate_pat(_tmp66C,rgn,_tmp665,r,path,sc->pattern,
tag_fail_stmt,Cyc_Absyn_goto_stmt(fail_lab,0),decls);_tmp672=_tmp671.f1;_tmp673=
_tmp671.f2;_tmp674=_tmp671.f3;if(is_tunion  && Cyc_Toc_no_tag_test(sc->pattern))
needs_tag_test=1;if(sc->where_clause != 0){struct Cyc_Absyn_Exp*_tmp675=(struct Cyc_Absyn_Exp*)
_check_null(sc->where_clause);Cyc_Toc_exp_to_c(_tmp672,_tmp675);_tmp674=Cyc_Absyn_seq_stmt(
_tmp674,Cyc_Absyn_ifthenelse_stmt(Cyc_Absyn_prim1_exp((void*)11,_tmp675,0),Cyc_Absyn_goto_stmt(
fail_lab,0),Cyc_Toc_skip_stmt_dl(),0),0);}decls=_tmp673;{struct Cyc_List_List*
_tmpAF9;nvs=((_tmpAF9=_region_malloc(rgn,sizeof(*_tmpAF9)),((_tmpAF9->hd=_tmp672,((
_tmpAF9->tl=nvs,_tmpAF9))))));}{struct Cyc_List_List*_tmpAFA;test_stmts=((_tmpAFA=
_region_malloc(rgn,sizeof(*_tmpAFA)),((_tmpAFA->hd=_tmp674,((_tmpAFA->tl=
test_stmts,_tmpAFA))))));}}}}nvs=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))
Cyc_List_imp_rev)(nvs);test_stmts=((struct Cyc_List_List*(*)(struct Cyc_List_List*
x))Cyc_List_imp_rev)(test_stmts);{struct Cyc_List_List*stmts=0;for(0;lscs != 0;(((
lscs=lscs->tl,nvs=nvs->tl)),test_stmts=test_stmts->tl)){struct _tuple16 _tmp679;
struct _dyneither_ptr*_tmp67A;struct _dyneither_ptr*_tmp67B;struct Cyc_Absyn_Switch_clause*
_tmp67C;struct _tuple16*_tmp678=(struct _tuple16*)lscs->hd;_tmp679=*_tmp678;
_tmp67A=_tmp679.f1;_tmp67B=_tmp679.f2;_tmp67C=_tmp679.f3;{struct Cyc_Toc_Env*
_tmp67D=(struct Cyc_Toc_Env*)((struct Cyc_List_List*)_check_null(nvs))->hd;struct
Cyc_Absyn_Stmt*s=_tmp67C->body;{struct _RegionHandle _tmp67E=_new_region("rgn2");
struct _RegionHandle*rgn2=& _tmp67E;_push_region(rgn2);if(lscs->tl != 0){struct
_tuple16 _tmp680;struct _dyneither_ptr*_tmp681;struct Cyc_Absyn_Switch_clause*
_tmp682;struct _tuple16*_tmp67F=(struct _tuple16*)((struct Cyc_List_List*)
_check_null(lscs->tl))->hd;_tmp680=*_tmp67F;_tmp681=_tmp680.f2;_tmp682=_tmp680.f3;
Cyc_Toc_stmt_to_c(Cyc_Toc_non_last_switchclause_env(rgn2,_tmp67D,end_l,_tmp681,(
struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(_tmp682->pat_vars))->v,(
struct Cyc_Toc_Env*)((struct Cyc_List_List*)_check_null(nvs->tl))->hd),s);}else{
Cyc_Toc_stmt_to_c(Cyc_Toc_last_switchclause_env(rgn2,_tmp67D,end_l),s);};
_pop_region(rgn2);}s=Cyc_Absyn_seq_stmt(Cyc_Absyn_label_stmt(_tmp67A,(struct Cyc_Absyn_Stmt*)((
struct Cyc_List_List*)_check_null(test_stmts))->hd,0),Cyc_Absyn_label_stmt(
_tmp67B,s,0),0);{struct Cyc_List_List*_tmpAFB;stmts=((_tmpAFB=_region_malloc(rgn,
sizeof(*_tmpAFB)),((_tmpAFB->hd=s,((_tmpAFB->tl=stmts,_tmpAFB))))));}}}{struct
Cyc_Absyn_Stmt*res=Cyc_Absyn_seq_stmt(Cyc_Absyn_seq_stmts(((struct Cyc_List_List*(*)(
struct Cyc_List_List*x))Cyc_List_imp_rev)(stmts),0),Cyc_Absyn_label_stmt(end_l,
Cyc_Toc_skip_stmt_dl(),0),0);for(decls;decls != 0;decls=decls->tl){struct _tuple14
_tmp685;struct _tuple1*_tmp686;void*_tmp687;struct _tuple14*_tmp684=(struct
_tuple14*)decls->hd;_tmp685=*_tmp684;_tmp686=_tmp685.f1;_tmp687=_tmp685.f2;res=
Cyc_Absyn_declare_stmt(_tmp686,_tmp687,0,res,0);}(void*)(whole_s->r=(void*)((
void*)(Cyc_Absyn_declare_stmt(v,Cyc_Toc_typ_to_c((void*)((struct Cyc_Core_Opt*)
_check_null(e->topt))->v),(struct Cyc_Absyn_Exp*)e,res,0))->r));}}};_pop_region(
rgn);}}}static struct Cyc_Absyn_Stmt*Cyc_Toc_letdecl_to_c(struct Cyc_Toc_Env*nv,
struct Cyc_Absyn_Pat*p,void*t,struct Cyc_Absyn_Exp*e,struct Cyc_Absyn_Stmt*s);
static void Cyc_Toc_local_decl_to_c(struct Cyc_Toc_Env*body_nv,struct Cyc_Toc_Env*
init_nv,struct Cyc_Absyn_Vardecl*vd,struct Cyc_Absyn_Stmt*s);static void Cyc_Toc_fndecl_to_c(
struct Cyc_Toc_Env*nv,struct Cyc_Absyn_Fndecl*f,int cinclude);struct Cyc_Absyn_Stmt*
Cyc_Toc_make_npop_handler(int n);struct Cyc_Absyn_Stmt*Cyc_Toc_make_npop_handler(
int n){struct Cyc_List_List*_tmpAFC;return Cyc_Absyn_exp_stmt(Cyc_Absyn_fncall_exp(
Cyc_Toc__npop_handler_e,((_tmpAFC=_cycalloc(sizeof(*_tmpAFC)),((_tmpAFC->hd=Cyc_Absyn_uint_exp((
unsigned int)(n - 1),0),((_tmpAFC->tl=0,_tmpAFC)))))),0),0);}void Cyc_Toc_do_npop_before(
int n,struct Cyc_Absyn_Stmt*s);void Cyc_Toc_do_npop_before(int n,struct Cyc_Absyn_Stmt*
s){if(n > 0)(void*)(s->r=(void*)Cyc_Toc_seq_stmt_r(Cyc_Toc_make_npop_handler(n),
Cyc_Absyn_new_stmt((void*)s->r,0)));}static void Cyc_Toc_stmt_to_c(struct Cyc_Toc_Env*
nv,struct Cyc_Absyn_Stmt*s);static void Cyc_Toc_stmt_to_c(struct Cyc_Toc_Env*nv,
struct Cyc_Absyn_Stmt*s){while(1){void*_tmp689=(void*)s->r;struct Cyc_Absyn_Exp*
_tmp68A;struct Cyc_Absyn_Stmt*_tmp68B;struct Cyc_Absyn_Stmt*_tmp68C;struct Cyc_Absyn_Exp*
_tmp68D;struct Cyc_Absyn_Exp*_tmp68E;struct Cyc_Absyn_Stmt*_tmp68F;struct Cyc_Absyn_Stmt*
_tmp690;struct _tuple3 _tmp691;struct Cyc_Absyn_Exp*_tmp692;struct Cyc_Absyn_Stmt*
_tmp693;struct Cyc_Absyn_Stmt*_tmp694;struct Cyc_Absyn_Stmt*_tmp695;struct Cyc_Absyn_Stmt*
_tmp696;struct Cyc_Absyn_Exp*_tmp697;struct _tuple3 _tmp698;struct Cyc_Absyn_Exp*
_tmp699;struct _tuple3 _tmp69A;struct Cyc_Absyn_Exp*_tmp69B;struct Cyc_Absyn_Stmt*
_tmp69C;struct Cyc_Absyn_Exp*_tmp69D;struct Cyc_List_List*_tmp69E;struct Cyc_List_List*
_tmp69F;struct Cyc_Absyn_Switch_clause**_tmp6A0;struct Cyc_Absyn_Decl*_tmp6A1;
struct Cyc_Absyn_Stmt*_tmp6A2;struct _dyneither_ptr*_tmp6A3;struct Cyc_Absyn_Stmt*
_tmp6A4;struct Cyc_Absyn_Stmt*_tmp6A5;struct _tuple3 _tmp6A6;struct Cyc_Absyn_Exp*
_tmp6A7;struct Cyc_Absyn_Stmt*_tmp6A8;struct Cyc_List_List*_tmp6A9;struct Cyc_Absyn_Tvar*
_tmp6AA;struct Cyc_Absyn_Vardecl*_tmp6AB;int _tmp6AC;struct Cyc_Absyn_Exp*_tmp6AD;
struct Cyc_Absyn_Stmt*_tmp6AE;struct Cyc_Absyn_Exp*_tmp6AF;struct Cyc_Absyn_Exp*
_tmp6B0;struct Cyc_Absyn_Tvar*_tmp6B1;struct Cyc_Absyn_Vardecl*_tmp6B2;struct Cyc_Absyn_Stmt*
_tmp6B3;_LL352: if((int)_tmp689 != 0)goto _LL354;_LL353: return;_LL354: if(_tmp689 <= (
void*)1)goto _LL356;if(*((int*)_tmp689)!= 0)goto _LL356;_tmp68A=((struct Cyc_Absyn_Exp_s_struct*)
_tmp689)->f1;_LL355: Cyc_Toc_exp_to_c(nv,_tmp68A);return;_LL356: if(_tmp689 <= (
void*)1)goto _LL358;if(*((int*)_tmp689)!= 1)goto _LL358;_tmp68B=((struct Cyc_Absyn_Seq_s_struct*)
_tmp689)->f1;_tmp68C=((struct Cyc_Absyn_Seq_s_struct*)_tmp689)->f2;_LL357: Cyc_Toc_stmt_to_c(
nv,_tmp68B);s=_tmp68C;continue;_LL358: if(_tmp689 <= (void*)1)goto _LL35A;if(*((int*)
_tmp689)!= 2)goto _LL35A;_tmp68D=((struct Cyc_Absyn_Return_s_struct*)_tmp689)->f1;
_LL359: {struct Cyc_Core_Opt*topt=0;if(_tmp68D != 0){{struct Cyc_Core_Opt*_tmpAFD;
topt=((_tmpAFD=_cycalloc(sizeof(*_tmpAFD)),((_tmpAFD->v=(void*)Cyc_Toc_typ_to_c((
void*)((struct Cyc_Core_Opt*)_check_null(_tmp68D->topt))->v),_tmpAFD))));}Cyc_Toc_exp_to_c(
nv,(struct Cyc_Absyn_Exp*)_tmp68D);}if(s->try_depth > 0){if(topt != 0){struct
_tuple1*_tmp6B5=Cyc_Toc_temp_var();struct Cyc_Absyn_Stmt*_tmp6B6=Cyc_Absyn_return_stmt((
struct Cyc_Absyn_Exp*)Cyc_Absyn_var_exp(_tmp6B5,0),0);(void*)(s->r=(void*)((void*)(
Cyc_Absyn_declare_stmt(_tmp6B5,(void*)topt->v,_tmp68D,Cyc_Absyn_seq_stmt(Cyc_Toc_make_npop_handler(
s->try_depth),_tmp6B6,0),0))->r));}else{Cyc_Toc_do_npop_before(s->try_depth,s);}}
return;}_LL35A: if(_tmp689 <= (void*)1)goto _LL35C;if(*((int*)_tmp689)!= 3)goto
_LL35C;_tmp68E=((struct Cyc_Absyn_IfThenElse_s_struct*)_tmp689)->f1;_tmp68F=((
struct Cyc_Absyn_IfThenElse_s_struct*)_tmp689)->f2;_tmp690=((struct Cyc_Absyn_IfThenElse_s_struct*)
_tmp689)->f3;_LL35B: Cyc_Toc_exp_to_c(nv,_tmp68E);Cyc_Toc_stmt_to_c(nv,_tmp68F);s=
_tmp690;continue;_LL35C: if(_tmp689 <= (void*)1)goto _LL35E;if(*((int*)_tmp689)!= 4)
goto _LL35E;_tmp691=((struct Cyc_Absyn_While_s_struct*)_tmp689)->f1;_tmp692=
_tmp691.f1;_tmp693=((struct Cyc_Absyn_While_s_struct*)_tmp689)->f2;_LL35D: Cyc_Toc_exp_to_c(
nv,_tmp692);{struct _RegionHandle _tmp6B7=_new_region("temp");struct _RegionHandle*
temp=& _tmp6B7;_push_region(temp);Cyc_Toc_stmt_to_c(Cyc_Toc_loop_env(temp,nv),
_tmp693);;_pop_region(temp);}return;_LL35E: if(_tmp689 <= (void*)1)goto _LL360;if(*((
int*)_tmp689)!= 5)goto _LL360;_tmp694=((struct Cyc_Absyn_Break_s_struct*)_tmp689)->f1;
_LL35F: {struct Cyc_Toc_Env _tmp6B9;struct _dyneither_ptr**_tmp6BA;struct Cyc_Toc_Env*
_tmp6B8=nv;_tmp6B9=*_tmp6B8;_tmp6BA=_tmp6B9.break_lab;if(_tmp6BA != 0)(void*)(s->r=(
void*)Cyc_Toc_goto_stmt_r(*_tmp6BA,0));{int dest_depth=_tmp694 == 0?0: _tmp694->try_depth;
Cyc_Toc_do_npop_before(s->try_depth - dest_depth,s);return;}}_LL360: if(_tmp689 <= (
void*)1)goto _LL362;if(*((int*)_tmp689)!= 6)goto _LL362;_tmp695=((struct Cyc_Absyn_Continue_s_struct*)
_tmp689)->f1;_LL361: {struct Cyc_Toc_Env _tmp6BC;struct _dyneither_ptr**_tmp6BD;
struct Cyc_Toc_Env*_tmp6BB=nv;_tmp6BC=*_tmp6BB;_tmp6BD=_tmp6BC.continue_lab;if(
_tmp6BD != 0)(void*)(s->r=(void*)Cyc_Toc_goto_stmt_r(*_tmp6BD,0));_tmp696=_tmp695;
goto _LL363;}_LL362: if(_tmp689 <= (void*)1)goto _LL364;if(*((int*)_tmp689)!= 7)goto
_LL364;_tmp696=((struct Cyc_Absyn_Goto_s_struct*)_tmp689)->f2;_LL363: Cyc_Toc_do_npop_before(
s->try_depth - ((struct Cyc_Absyn_Stmt*)_check_null(_tmp696))->try_depth,s);
return;_LL364: if(_tmp689 <= (void*)1)goto _LL366;if(*((int*)_tmp689)!= 8)goto
_LL366;_tmp697=((struct Cyc_Absyn_For_s_struct*)_tmp689)->f1;_tmp698=((struct Cyc_Absyn_For_s_struct*)
_tmp689)->f2;_tmp699=_tmp698.f1;_tmp69A=((struct Cyc_Absyn_For_s_struct*)_tmp689)->f3;
_tmp69B=_tmp69A.f1;_tmp69C=((struct Cyc_Absyn_For_s_struct*)_tmp689)->f4;_LL365:
Cyc_Toc_exp_to_c(nv,_tmp697);Cyc_Toc_exp_to_c(nv,_tmp699);Cyc_Toc_exp_to_c(nv,
_tmp69B);{struct _RegionHandle _tmp6BE=_new_region("temp");struct _RegionHandle*
temp=& _tmp6BE;_push_region(temp);Cyc_Toc_stmt_to_c(Cyc_Toc_loop_env(temp,nv),
_tmp69C);;_pop_region(temp);}return;_LL366: if(_tmp689 <= (void*)1)goto _LL368;if(*((
int*)_tmp689)!= 9)goto _LL368;_tmp69D=((struct Cyc_Absyn_Switch_s_struct*)_tmp689)->f1;
_tmp69E=((struct Cyc_Absyn_Switch_s_struct*)_tmp689)->f2;_LL367: Cyc_Toc_xlate_switch(
nv,s,_tmp69D,_tmp69E);return;_LL368: if(_tmp689 <= (void*)1)goto _LL36A;if(*((int*)
_tmp689)!= 10)goto _LL36A;_tmp69F=((struct Cyc_Absyn_Fallthru_s_struct*)_tmp689)->f1;
_tmp6A0=((struct Cyc_Absyn_Fallthru_s_struct*)_tmp689)->f2;_LL369: {struct Cyc_Toc_Env
_tmp6C0;struct Cyc_Toc_FallthruInfo*_tmp6C1;struct Cyc_Toc_Env*_tmp6BF=nv;_tmp6C0=*
_tmp6BF;_tmp6C1=_tmp6C0.fallthru_info;if(_tmp6C1 == 0){const char*_tmpB00;void*
_tmpAFF;(_tmpAFF=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((
_tmpB00="fallthru in unexpected place",_tag_dyneither(_tmpB00,sizeof(char),29))),
_tag_dyneither(_tmpAFF,sizeof(void*),0)));}{struct _dyneither_ptr*_tmp6C5;struct
Cyc_List_List*_tmp6C6;struct Cyc_Dict_Dict _tmp6C7;struct Cyc_Toc_FallthruInfo
_tmp6C4=*_tmp6C1;_tmp6C5=_tmp6C4.label;_tmp6C6=_tmp6C4.binders;_tmp6C7=_tmp6C4.next_case_env;{
struct Cyc_Absyn_Stmt*s2=Cyc_Absyn_goto_stmt(_tmp6C5,0);Cyc_Toc_do_npop_before(s->try_depth
- ((*((struct Cyc_Absyn_Switch_clause**)_check_null(_tmp6A0)))->body)->try_depth,
s2);{struct Cyc_List_List*_tmp6C8=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))
Cyc_List_rev)(_tmp6C6);struct Cyc_List_List*_tmp6C9=((struct Cyc_List_List*(*)(
struct Cyc_List_List*x))Cyc_List_rev)(_tmp69F);for(0;_tmp6C8 != 0;(_tmp6C8=_tmp6C8->tl,
_tmp6C9=_tmp6C9->tl)){Cyc_Toc_exp_to_c(nv,(struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)
_check_null(_tmp6C9))->hd);s2=Cyc_Absyn_seq_stmt(Cyc_Absyn_assign_stmt(((struct
Cyc_Absyn_Exp*(*)(struct Cyc_Dict_Dict d,struct _tuple1*k))Cyc_Dict_lookup)(_tmp6C7,(
struct _tuple1*)_tmp6C8->hd),(struct Cyc_Absyn_Exp*)_tmp6C9->hd,0),s2,0);}(void*)(
s->r=(void*)((void*)s2->r));return;}}}}_LL36A: if(_tmp689 <= (void*)1)goto _LL36C;
if(*((int*)_tmp689)!= 11)goto _LL36C;_tmp6A1=((struct Cyc_Absyn_Decl_s_struct*)
_tmp689)->f1;_tmp6A2=((struct Cyc_Absyn_Decl_s_struct*)_tmp689)->f2;_LL36B:{void*
_tmp6CA=(void*)_tmp6A1->r;struct Cyc_Absyn_Vardecl*_tmp6CB;struct Cyc_Absyn_Pat*
_tmp6CC;struct Cyc_Absyn_Exp*_tmp6CD;struct Cyc_List_List*_tmp6CE;struct Cyc_Absyn_Fndecl*
_tmp6CF;_LL379: if(_tmp6CA <= (void*)2)goto _LL381;if(*((int*)_tmp6CA)!= 0)goto
_LL37B;_tmp6CB=((struct Cyc_Absyn_Var_d_struct*)_tmp6CA)->f1;_LL37A:{struct
_RegionHandle _tmp6D0=_new_region("temp");struct _RegionHandle*temp=& _tmp6D0;
_push_region(temp);{struct Cyc_Absyn_Local_b_struct _tmpB03;struct Cyc_Absyn_Local_b_struct*
_tmpB02;struct Cyc_Toc_Env*_tmp6D1=Cyc_Toc_add_varmap(temp,nv,_tmp6CB->name,Cyc_Absyn_varb_exp(
_tmp6CB->name,(void*)((_tmpB02=_cycalloc(sizeof(*_tmpB02)),((_tmpB02[0]=((
_tmpB03.tag=3,((_tmpB03.f1=_tmp6CB,_tmpB03)))),_tmpB02)))),0));Cyc_Toc_local_decl_to_c(
_tmp6D1,_tmp6D1,_tmp6CB,_tmp6A2);};_pop_region(temp);}goto _LL378;_LL37B: if(*((
int*)_tmp6CA)!= 2)goto _LL37D;_tmp6CC=((struct Cyc_Absyn_Let_d_struct*)_tmp6CA)->f1;
_tmp6CD=((struct Cyc_Absyn_Let_d_struct*)_tmp6CA)->f3;_LL37C:{void*_tmp6D4=(void*)
_tmp6CC->r;struct Cyc_Absyn_Vardecl*_tmp6D5;struct Cyc_Absyn_Pat*_tmp6D6;struct Cyc_Absyn_Pat
_tmp6D7;void*_tmp6D8;_LL384: if(_tmp6D4 <= (void*)2)goto _LL386;if(*((int*)_tmp6D4)
!= 0)goto _LL386;_tmp6D5=((struct Cyc_Absyn_Var_p_struct*)_tmp6D4)->f1;_tmp6D6=((
struct Cyc_Absyn_Var_p_struct*)_tmp6D4)->f2;_tmp6D7=*_tmp6D6;_tmp6D8=(void*)
_tmp6D7.r;if((int)_tmp6D8 != 0)goto _LL386;_LL385: {struct _tuple1*old_name=_tmp6D5->name;
struct _tuple1*new_name=Cyc_Toc_temp_var();_tmp6D5->name=new_name;_tmp6D5->initializer=(
struct Cyc_Absyn_Exp*)_tmp6CD;{struct Cyc_Absyn_Var_d_struct _tmpB06;struct Cyc_Absyn_Var_d_struct*
_tmpB05;(void*)(_tmp6A1->r=(void*)((void*)((_tmpB05=_cycalloc(sizeof(*_tmpB05)),((
_tmpB05[0]=((_tmpB06.tag=0,((_tmpB06.f1=_tmp6D5,_tmpB06)))),_tmpB05))))));}{
struct _RegionHandle _tmp6DB=_new_region("temp");struct _RegionHandle*temp=& _tmp6DB;
_push_region(temp);{struct Cyc_Absyn_Local_b_struct _tmpB09;struct Cyc_Absyn_Local_b_struct*
_tmpB08;struct Cyc_Toc_Env*_tmp6DC=Cyc_Toc_add_varmap(temp,nv,old_name,Cyc_Absyn_varb_exp(
new_name,(void*)((_tmpB08=_cycalloc(sizeof(*_tmpB08)),((_tmpB08[0]=((_tmpB09.tag=
3,((_tmpB09.f1=_tmp6D5,_tmpB09)))),_tmpB08)))),0));Cyc_Toc_local_decl_to_c(
_tmp6DC,nv,_tmp6D5,_tmp6A2);};_pop_region(temp);}goto _LL383;}_LL386:;_LL387:(
void*)(s->r=(void*)((void*)(Cyc_Toc_letdecl_to_c(nv,_tmp6CC,(void*)((struct Cyc_Core_Opt*)
_check_null(_tmp6CD->topt))->v,_tmp6CD,_tmp6A2))->r));goto _LL383;_LL383:;}goto
_LL378;_LL37D: if(*((int*)_tmp6CA)!= 3)goto _LL37F;_tmp6CE=((struct Cyc_Absyn_Letv_d_struct*)
_tmp6CA)->f1;_LL37E: {struct Cyc_List_List*_tmp6DF=((struct Cyc_List_List*(*)(
struct Cyc_List_List*x))Cyc_List_rev)(_tmp6CE);if(_tmp6DF == 0){const char*_tmpB0C;
void*_tmpB0B;(_tmpB0B=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))
Cyc_Tcutil_impos)(((_tmpB0C="empty Letv_d",_tag_dyneither(_tmpB0C,sizeof(char),
13))),_tag_dyneither(_tmpB0B,sizeof(void*),0)));}{struct Cyc_Absyn_Var_d_struct
_tmpB0F;struct Cyc_Absyn_Var_d_struct*_tmpB0E;(void*)(_tmp6A1->r=(void*)((void*)((
_tmpB0E=_cycalloc(sizeof(*_tmpB0E)),((_tmpB0E[0]=((_tmpB0F.tag=0,((_tmpB0F.f1=(
struct Cyc_Absyn_Vardecl*)_tmp6DF->hd,_tmpB0F)))),_tmpB0E))))));}_tmp6DF=_tmp6DF->tl;
for(0;_tmp6DF != 0;_tmp6DF=_tmp6DF->tl){struct Cyc_Absyn_Var_d_struct _tmpB12;
struct Cyc_Absyn_Var_d_struct*_tmpB11;struct Cyc_Absyn_Decl*_tmp6E4=Cyc_Absyn_new_decl((
void*)((_tmpB11=_cycalloc(sizeof(*_tmpB11)),((_tmpB11[0]=((_tmpB12.tag=0,((
_tmpB12.f1=(struct Cyc_Absyn_Vardecl*)_tmp6DF->hd,_tmpB12)))),_tmpB11)))),0);(
void*)(s->r=(void*)((void*)(Cyc_Absyn_decl_stmt(_tmp6E4,Cyc_Absyn_new_stmt((void*)
s->r,0),0))->r));}Cyc_Toc_stmt_to_c(nv,s);goto _LL378;}_LL37F: if(*((int*)_tmp6CA)
!= 1)goto _LL381;_tmp6CF=((struct Cyc_Absyn_Fn_d_struct*)_tmp6CA)->f1;_LL380: {
struct _tuple1*_tmp6E7=_tmp6CF->name;{struct _RegionHandle _tmp6E8=_new_region("temp");
struct _RegionHandle*temp=& _tmp6E8;_push_region(temp);{struct Cyc_Toc_Env*_tmp6E9=
Cyc_Toc_add_varmap(temp,nv,_tmp6CF->name,Cyc_Absyn_var_exp(_tmp6E7,0));Cyc_Toc_fndecl_to_c(
_tmp6E9,_tmp6CF,0);Cyc_Toc_stmt_to_c(_tmp6E9,_tmp6A2);};_pop_region(temp);}goto
_LL378;}_LL381:;_LL382: {const char*_tmpB15;void*_tmpB14;(_tmpB14=0,((int(*)(
struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmpB15="bad nested declaration within function",
_tag_dyneither(_tmpB15,sizeof(char),39))),_tag_dyneither(_tmpB14,sizeof(void*),0)));}
_LL378:;}return;_LL36C: if(_tmp689 <= (void*)1)goto _LL36E;if(*((int*)_tmp689)!= 12)
goto _LL36E;_tmp6A3=((struct Cyc_Absyn_Label_s_struct*)_tmp689)->f1;_tmp6A4=((
struct Cyc_Absyn_Label_s_struct*)_tmp689)->f2;_LL36D: s=_tmp6A4;continue;_LL36E:
if(_tmp689 <= (void*)1)goto _LL370;if(*((int*)_tmp689)!= 13)goto _LL370;_tmp6A5=((
struct Cyc_Absyn_Do_s_struct*)_tmp689)->f1;_tmp6A6=((struct Cyc_Absyn_Do_s_struct*)
_tmp689)->f2;_tmp6A7=_tmp6A6.f1;_LL36F:{struct _RegionHandle _tmp6EC=_new_region("temp");
struct _RegionHandle*temp=& _tmp6EC;_push_region(temp);Cyc_Toc_stmt_to_c(Cyc_Toc_loop_env(
temp,nv),_tmp6A5);Cyc_Toc_exp_to_c(nv,_tmp6A7);;_pop_region(temp);}return;_LL370:
if(_tmp689 <= (void*)1)goto _LL372;if(*((int*)_tmp689)!= 14)goto _LL372;_tmp6A8=((
struct Cyc_Absyn_TryCatch_s_struct*)_tmp689)->f1;_tmp6A9=((struct Cyc_Absyn_TryCatch_s_struct*)
_tmp689)->f2;_LL371: {struct _tuple1*h_var=Cyc_Toc_temp_var();struct _tuple1*e_var=
Cyc_Toc_temp_var();struct _tuple1*was_thrown_var=Cyc_Toc_temp_var();struct Cyc_Absyn_Exp*
h_exp=Cyc_Absyn_var_exp(h_var,0);struct Cyc_Absyn_Exp*e_exp=Cyc_Absyn_var_exp(
e_var,0);struct Cyc_Absyn_Exp*was_thrown_exp=Cyc_Absyn_var_exp(was_thrown_var,0);
void*h_typ=Cyc_Absyn_strct(Cyc_Toc__handler_cons_sp);void*e_typ=Cyc_Toc_typ_to_c(
Cyc_Absyn_exn_typ);void*was_thrown_typ=Cyc_Toc_typ_to_c(Cyc_Absyn_sint_typ);{
struct Cyc_Core_Opt*_tmpB16;e_exp->topt=((_tmpB16=_cycalloc(sizeof(*_tmpB16)),((
_tmpB16->v=(void*)e_typ,_tmpB16))));}{struct _RegionHandle _tmp6EE=_new_region("temp");
struct _RegionHandle*temp=& _tmp6EE;_push_region(temp);{struct Cyc_Toc_Env*_tmp6EF=
Cyc_Toc_add_varmap(temp,nv,e_var,e_exp);Cyc_Toc_stmt_to_c(_tmp6EF,_tmp6A8);{
struct Cyc_Absyn_Stmt*_tmp6F0=Cyc_Absyn_seq_stmt(_tmp6A8,Cyc_Absyn_exp_stmt(Cyc_Absyn_fncall_exp(
Cyc_Toc__pop_handler_e,0,0),0),0);struct _tuple1*_tmp6F1=Cyc_Toc_temp_var();
struct Cyc_Absyn_Exp*_tmp6F2=Cyc_Absyn_var_exp(_tmp6F1,0);struct Cyc_Absyn_Vardecl*
_tmp6F3=Cyc_Absyn_new_vardecl(_tmp6F1,Cyc_Absyn_exn_typ,0);{struct Cyc_Core_Opt*
_tmpB17;_tmp6F2->topt=((_tmpB17=_cycalloc(sizeof(*_tmpB17)),((_tmpB17->v=(void*)
Cyc_Absyn_exn_typ,_tmpB17))));}{struct Cyc_Absyn_Var_p_struct*_tmpB28;struct Cyc_Absyn_Pat*
_tmpB27;struct Cyc_Core_Opt*_tmpB26;struct Cyc_Absyn_Var_p_struct _tmpB25;struct Cyc_Core_Opt*
_tmpB24;struct Cyc_Absyn_Pat*_tmpB23;struct Cyc_Absyn_Pat*_tmp6F5=(_tmpB23=
_cycalloc(sizeof(*_tmpB23)),((_tmpB23->r=(void*)((void*)((_tmpB28=_cycalloc(
sizeof(*_tmpB28)),((_tmpB28[0]=((_tmpB25.tag=0,((_tmpB25.f1=_tmp6F3,((_tmpB25.f2=((
_tmpB27=_cycalloc(sizeof(*_tmpB27)),((_tmpB27->r=(void*)((void*)0),((_tmpB27->topt=((
_tmpB26=_cycalloc(sizeof(*_tmpB26)),((_tmpB26->v=(void*)Cyc_Absyn_exn_typ,
_tmpB26)))),((_tmpB27->loc=0,_tmpB27)))))))),_tmpB25)))))),_tmpB28))))),((
_tmpB23->topt=((_tmpB24=_cycalloc(sizeof(*_tmpB24)),((_tmpB24->v=(void*)Cyc_Absyn_exn_typ,
_tmpB24)))),((_tmpB23->loc=0,_tmpB23)))))));struct Cyc_Absyn_Exp*_tmp6F6=Cyc_Absyn_throw_exp(
_tmp6F2,0);{struct Cyc_Core_Opt*_tmpB29;_tmp6F6->topt=((_tmpB29=_cycalloc(sizeof(*
_tmpB29)),((_tmpB29->v=(void*)((void*)0),_tmpB29))));}{struct Cyc_Absyn_Stmt*
_tmp6F8=Cyc_Absyn_exp_stmt(_tmp6F6,0);struct Cyc_Core_Opt*_tmpB2F;struct Cyc_List_List*
_tmpB2E;struct Cyc_Absyn_Switch_clause*_tmpB2D;struct Cyc_Absyn_Switch_clause*
_tmp6F9=(_tmpB2D=_cycalloc(sizeof(*_tmpB2D)),((_tmpB2D->pattern=_tmp6F5,((
_tmpB2D->pat_vars=((_tmpB2F=_cycalloc(sizeof(*_tmpB2F)),((_tmpB2F->v=((_tmpB2E=
_cycalloc(sizeof(*_tmpB2E)),((_tmpB2E->hd=_tmp6F3,((_tmpB2E->tl=0,_tmpB2E)))))),
_tmpB2F)))),((_tmpB2D->where_clause=0,((_tmpB2D->body=_tmp6F8,((_tmpB2D->loc=0,
_tmpB2D)))))))))));struct Cyc_List_List*_tmpB30;struct Cyc_Absyn_Stmt*_tmp6FA=Cyc_Absyn_switch_stmt(
e_exp,((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_append)(
_tmp6A9,((_tmpB30=_cycalloc(sizeof(*_tmpB30)),((_tmpB30->hd=_tmp6F9,((_tmpB30->tl=
0,_tmpB30))))))),0);Cyc_Toc_stmt_to_c(_tmp6EF,_tmp6FA);{struct Cyc_List_List*
_tmpB31;struct Cyc_Absyn_Exp*_tmp6FB=Cyc_Absyn_fncall_exp(Cyc_Toc_setjmp_e,((
_tmpB31=_cycalloc(sizeof(*_tmpB31)),((_tmpB31->hd=Cyc_Absyn_aggrmember_exp(h_exp,
Cyc_Toc_handler_sp,0),((_tmpB31->tl=0,_tmpB31)))))),0);struct Cyc_List_List*
_tmpB32;struct Cyc_Absyn_Stmt*_tmp6FC=Cyc_Absyn_exp_stmt(Cyc_Absyn_fncall_exp(Cyc_Toc__push_handler_e,((
_tmpB32=_cycalloc(sizeof(*_tmpB32)),((_tmpB32->hd=Cyc_Absyn_address_exp(h_exp,0),((
_tmpB32->tl=0,_tmpB32)))))),0),0);struct Cyc_Absyn_Exp*_tmp6FD=Cyc_Absyn_int_exp((
void*)0,0,0);struct Cyc_Absyn_Exp*_tmp6FE=Cyc_Absyn_int_exp((void*)0,1,0);(void*)(
s->r=(void*)((void*)(Cyc_Absyn_declare_stmt(h_var,h_typ,0,Cyc_Absyn_seq_stmt(
_tmp6FC,Cyc_Absyn_declare_stmt(was_thrown_var,was_thrown_typ,(struct Cyc_Absyn_Exp*)
_tmp6FD,Cyc_Absyn_seq_stmt(Cyc_Absyn_ifthenelse_stmt(_tmp6FB,Cyc_Absyn_assign_stmt(
was_thrown_exp,_tmp6FE,0),Cyc_Toc_skip_stmt_dl(),0),Cyc_Absyn_ifthenelse_stmt(
Cyc_Absyn_prim1_exp((void*)11,was_thrown_exp,0),_tmp6F0,Cyc_Absyn_declare_stmt(
e_var,e_typ,(struct Cyc_Absyn_Exp*)Cyc_Toc_cast_it(e_typ,Cyc_Toc__exn_thrown_e),
_tmp6FA,0),0),0),0),0),0))->r));}}}}};_pop_region(temp);}return;}_LL372: if(
_tmp689 <= (void*)1)goto _LL374;if(*((int*)_tmp689)!= 15)goto _LL374;_tmp6AA=((
struct Cyc_Absyn_Region_s_struct*)_tmp689)->f1;_tmp6AB=((struct Cyc_Absyn_Region_s_struct*)
_tmp689)->f2;_tmp6AC=((struct Cyc_Absyn_Region_s_struct*)_tmp689)->f3;_tmp6AD=((
struct Cyc_Absyn_Region_s_struct*)_tmp689)->f4;_tmp6AE=((struct Cyc_Absyn_Region_s_struct*)
_tmp689)->f5;_LL373: {void*rh_struct_typ=Cyc_Absyn_strct(Cyc_Toc__RegionHandle_sp);
void*rh_struct_ptr_typ=Cyc_Absyn_cstar_typ(rh_struct_typ,Cyc_Toc_mt_tq);struct
_tuple1*rh_var=Cyc_Toc_temp_var();struct _tuple1*x_var=_tmp6AB->name;struct Cyc_Absyn_Exp*
rh_exp=Cyc_Absyn_var_exp(rh_var,0);struct Cyc_Absyn_Exp*x_exp=Cyc_Absyn_var_exp(
x_var,0);{struct _RegionHandle _tmp70B=_new_region("temp");struct _RegionHandle*
temp=& _tmp70B;_push_region(temp);Cyc_Toc_stmt_to_c(Cyc_Toc_add_varmap(temp,nv,
x_var,x_exp),_tmp6AE);;_pop_region(temp);}if(Cyc_Absyn_no_regions)(void*)(s->r=(
void*)((void*)(Cyc_Absyn_declare_stmt(x_var,rh_struct_ptr_typ,(struct Cyc_Absyn_Exp*)
Cyc_Absyn_uint_exp(0,0),_tmp6AE,0))->r));else{if(_tmp6AD == 0){struct Cyc_Absyn_Exp*
_tmpB35[1];struct Cyc_Absyn_Exp*_tmpB34[1];struct Cyc_List_List*_tmpB33;(void*)(s->r=(
void*)((void*)(Cyc_Absyn_declare_stmt(rh_var,rh_struct_typ,(struct Cyc_Absyn_Exp*)
Cyc_Absyn_fncall_exp(Cyc_Toc__new_region_e,((_tmpB33=_cycalloc(sizeof(*_tmpB33)),((
_tmpB33->hd=Cyc_Absyn_string_exp(Cyc_Absynpp_qvar2string(x_var),0),((_tmpB33->tl=
0,_tmpB33)))))),0),Cyc_Absyn_declare_stmt(x_var,rh_struct_ptr_typ,(struct Cyc_Absyn_Exp*)
Cyc_Absyn_address_exp(rh_exp,0),Cyc_Absyn_seq_stmt(Cyc_Absyn_exp_stmt(Cyc_Absyn_fncall_exp(
Cyc_Toc__push_region_e,((_tmpB34[0]=x_exp,((struct Cyc_List_List*(*)(struct
_dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmpB34,sizeof(struct Cyc_Absyn_Exp*),
1)))),0),0),Cyc_Absyn_seq_stmt(_tmp6AE,Cyc_Absyn_exp_stmt(Cyc_Absyn_fncall_exp(
Cyc_Toc__pop_region_e,((_tmpB35[0]=x_exp,((struct Cyc_List_List*(*)(struct
_dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmpB35,sizeof(struct Cyc_Absyn_Exp*),
1)))),0),0),0),0),0),0))->r));}else{Cyc_Toc_exp_to_c(nv,(struct Cyc_Absyn_Exp*)
_tmp6AD);{struct Cyc_Absyn_Exp*_tmpB37[1];struct Cyc_Absyn_Exp*_tmpB36[2];(void*)(
s->r=(void*)((void*)(Cyc_Absyn_declare_stmt(rh_var,Cyc_Absyn_strct(Cyc_Toc__DynRegionFrame_sp),
0,Cyc_Absyn_declare_stmt(x_var,rh_struct_ptr_typ,(struct Cyc_Absyn_Exp*)Cyc_Absyn_fncall_exp(
Cyc_Toc__open_dynregion_e,((_tmpB36[1]=(struct Cyc_Absyn_Exp*)_tmp6AD,((_tmpB36[0]=
Cyc_Absyn_address_exp(rh_exp,0),((struct Cyc_List_List*(*)(struct _dyneither_ptr))
Cyc_List_list)(_tag_dyneither(_tmpB36,sizeof(struct Cyc_Absyn_Exp*),2)))))),0),
Cyc_Absyn_seq_stmt(_tmp6AE,Cyc_Absyn_exp_stmt(Cyc_Absyn_fncall_exp(Cyc_Toc__pop_dynregion_e,((
_tmpB37[0]=x_exp,((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(
_tag_dyneither(_tmpB37,sizeof(struct Cyc_Absyn_Exp*),1)))),0),0),0),0),0))->r));}}}
return;}_LL374: if(_tmp689 <= (void*)1)goto _LL376;if(*((int*)_tmp689)!= 16)goto
_LL376;_tmp6AF=((struct Cyc_Absyn_ResetRegion_s_struct*)_tmp689)->f1;_LL375: if(
Cyc_Absyn_no_regions)(void*)(s->r=(void*)((void*)0));else{Cyc_Toc_exp_to_c(nv,
_tmp6AF);{struct Cyc_List_List*_tmpB38;(void*)(s->r=(void*)Cyc_Toc_exp_stmt_r(Cyc_Absyn_fncall_exp(
Cyc_Toc__reset_region_e,((_tmpB38=_cycalloc(sizeof(*_tmpB38)),((_tmpB38->hd=
_tmp6AF,((_tmpB38->tl=0,_tmpB38)))))),0)));}}return;_LL376: if(_tmp689 <= (void*)1)
goto _LL351;if(*((int*)_tmp689)!= 17)goto _LL351;_tmp6B0=((struct Cyc_Absyn_Alias_s_struct*)
_tmp689)->f1;_tmp6B1=((struct Cyc_Absyn_Alias_s_struct*)_tmp689)->f2;_tmp6B2=((
struct Cyc_Absyn_Alias_s_struct*)_tmp689)->f3;_tmp6B3=((struct Cyc_Absyn_Alias_s_struct*)
_tmp689)->f4;_LL377: {struct _tuple1*old_name=_tmp6B2->name;struct _tuple1*
new_name=Cyc_Toc_temp_var();_tmp6B2->name=new_name;_tmp6B2->initializer=(struct
Cyc_Absyn_Exp*)_tmp6B0;{struct Cyc_Absyn_Decl_s_struct _tmpB47;struct Cyc_Absyn_Var_d_struct*
_tmpB46;struct Cyc_Absyn_Var_d_struct _tmpB45;struct Cyc_Absyn_Decl*_tmpB44;struct
Cyc_Absyn_Decl_s_struct*_tmpB43;(void*)(s->r=(void*)((void*)((_tmpB43=_cycalloc(
sizeof(*_tmpB43)),((_tmpB43[0]=((_tmpB47.tag=11,((_tmpB47.f1=((_tmpB44=_cycalloc(
sizeof(*_tmpB44)),((_tmpB44->r=(void*)((void*)((_tmpB46=_cycalloc(sizeof(*
_tmpB46)),((_tmpB46[0]=((_tmpB45.tag=0,((_tmpB45.f1=_tmp6B2,_tmpB45)))),_tmpB46))))),((
_tmpB44->loc=0,_tmpB44)))))),((_tmpB47.f2=_tmp6B3,_tmpB47)))))),_tmpB43))))));}{
struct _RegionHandle _tmp717=_new_region("temp");struct _RegionHandle*temp=& _tmp717;
_push_region(temp);{struct Cyc_Absyn_Local_b_struct _tmpB4A;struct Cyc_Absyn_Local_b_struct*
_tmpB49;struct Cyc_Toc_Env*_tmp718=Cyc_Toc_add_varmap(temp,nv,old_name,Cyc_Absyn_varb_exp(
new_name,(void*)((_tmpB49=_cycalloc(sizeof(*_tmpB49)),((_tmpB49[0]=((_tmpB4A.tag=
3,((_tmpB4A.f1=_tmp6B2,_tmpB4A)))),_tmpB49)))),0));Cyc_Toc_local_decl_to_c(
_tmp718,nv,_tmp6B2,_tmp6B3);};_pop_region(temp);}return;}_LL351:;}}static void Cyc_Toc_stmttypes_to_c(
struct Cyc_Absyn_Stmt*s);struct _tuple17{struct _dyneither_ptr*f1;struct Cyc_Absyn_Tqual
f2;void*f3;};static void Cyc_Toc_fndecl_to_c(struct Cyc_Toc_Env*nv,struct Cyc_Absyn_Fndecl*
f,int cinclude);static void Cyc_Toc_fndecl_to_c(struct Cyc_Toc_Env*nv,struct Cyc_Absyn_Fndecl*
f,int cinclude){f->tvs=0;f->effect=0;f->rgn_po=0;(void*)(f->ret_type=(void*)Cyc_Toc_typ_to_c((
void*)f->ret_type));{struct _RegionHandle _tmp71B=_new_region("frgn");struct
_RegionHandle*frgn=& _tmp71B;_push_region(frgn);{struct Cyc_Toc_Env*_tmp71C=Cyc_Toc_share_env(
frgn,nv);{struct Cyc_List_List*_tmp71D=f->args;for(0;_tmp71D != 0;_tmp71D=_tmp71D->tl){
union Cyc_Absyn_Nmspace_union _tmpB4D;struct _tuple1*_tmpB4C;struct _tuple1*_tmp71E=(
_tmpB4C=_cycalloc(sizeof(*_tmpB4C)),((_tmpB4C->f1=(union Cyc_Absyn_Nmspace_union)((
union Cyc_Absyn_Nmspace_union)(((_tmpB4D.Loc_n).tag=0,_tmpB4D))),((_tmpB4C->f2=(*((
struct _tuple17*)_tmp71D->hd)).f1,_tmpB4C)))));(*((struct _tuple17*)_tmp71D->hd)).f3=
Cyc_Toc_typ_to_c((*((struct _tuple17*)_tmp71D->hd)).f3);_tmp71C=Cyc_Toc_add_varmap(
frgn,_tmp71C,_tmp71E,Cyc_Absyn_var_exp(_tmp71E,0));}}if(cinclude){Cyc_Toc_stmttypes_to_c(
f->body);_npop_handler(0);return;}if((unsigned int)f->cyc_varargs  && ((struct Cyc_Absyn_VarargInfo*)
_check_null(f->cyc_varargs))->name != 0){struct Cyc_Core_Opt*_tmp722;struct Cyc_Absyn_Tqual
_tmp723;void*_tmp724;int _tmp725;struct Cyc_Absyn_VarargInfo _tmp721=*((struct Cyc_Absyn_VarargInfo*)
_check_null(f->cyc_varargs));_tmp722=_tmp721.name;_tmp723=_tmp721.tq;_tmp724=(
void*)_tmp721.type;_tmp725=_tmp721.inject;{void*_tmp726=Cyc_Toc_typ_to_c(Cyc_Absyn_dyneither_typ(
_tmp724,(void*)2,_tmp723,Cyc_Absyn_false_conref));union Cyc_Absyn_Nmspace_union
_tmpB50;struct _tuple1*_tmpB4F;struct _tuple1*_tmp727=(_tmpB4F=_cycalloc(sizeof(*
_tmpB4F)),((_tmpB4F->f1=(union Cyc_Absyn_Nmspace_union)((union Cyc_Absyn_Nmspace_union)(((
_tmpB50.Loc_n).tag=0,_tmpB50))),((_tmpB4F->f2=(struct _dyneither_ptr*)((struct Cyc_Core_Opt*)
_check_null(_tmp722))->v,_tmpB4F)))));{struct _tuple17*_tmpB53;struct Cyc_List_List*
_tmpB52;f->args=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*
y))Cyc_List_append)(f->args,((_tmpB52=_cycalloc(sizeof(*_tmpB52)),((_tmpB52->hd=((
_tmpB53=_cycalloc(sizeof(*_tmpB53)),((_tmpB53->f1=(struct _dyneither_ptr*)_tmp722->v,((
_tmpB53->f2=_tmp723,((_tmpB53->f3=_tmp726,_tmpB53)))))))),((_tmpB52->tl=0,
_tmpB52)))))));}_tmp71C=Cyc_Toc_add_varmap(frgn,_tmp71C,_tmp727,Cyc_Absyn_var_exp(
_tmp727,0));f->cyc_varargs=0;}}{struct Cyc_List_List*_tmp72C=(struct Cyc_List_List*)((
struct Cyc_Core_Opt*)_check_null(f->param_vardecls))->v;for(0;_tmp72C != 0;_tmp72C=
_tmp72C->tl){(void*)(((struct Cyc_Absyn_Vardecl*)_tmp72C->hd)->type=(void*)Cyc_Toc_typ_to_c((
void*)((struct Cyc_Absyn_Vardecl*)_tmp72C->hd)->type));}}Cyc_Toc_stmt_to_c(Cyc_Toc_clear_toplevel(
frgn,_tmp71C),f->body);};_pop_region(frgn);}}static void*Cyc_Toc_scope_to_c(void*
s);static void*Cyc_Toc_scope_to_c(void*s){void*_tmp72D=s;_LL389: if((int)_tmp72D != 
1)goto _LL38B;_LL38A: return(void*)2;_LL38B: if((int)_tmp72D != 4)goto _LL38D;_LL38C:
return(void*)3;_LL38D:;_LL38E: return s;_LL388:;}static void Cyc_Toc_aggrdecl_to_c(
struct Cyc_Absyn_Aggrdecl*ad);static void Cyc_Toc_aggrdecl_to_c(struct Cyc_Absyn_Aggrdecl*
ad){struct _tuple1*_tmp72E=ad->name;struct _DynRegionHandle*_tmp730;struct Cyc_Dict_Dict*
_tmp731;struct Cyc_Toc_TocState _tmp72F=*((struct Cyc_Toc_TocState*)_check_null(Cyc_Toc_toc_state));
_tmp730=_tmp72F.dyn;_tmp731=_tmp72F.aggrs_so_far;{struct _DynRegionFrame _tmp732;
struct _RegionHandle*d=_open_dynregion(& _tmp732,_tmp730);{int seen_defn_before;
struct _tuple7**_tmp733=((struct _tuple7**(*)(struct Cyc_Dict_Dict d,struct _tuple1*k))
Cyc_Dict_lookup_opt)(*_tmp731,_tmp72E);if(_tmp733 == 0){seen_defn_before=0;{
struct _tuple7*v;if((void*)ad->kind == (void*)0){struct _tuple7*_tmpB54;v=((_tmpB54=
_region_malloc(d,sizeof(*_tmpB54)),((_tmpB54->f1=ad,((_tmpB54->f2=Cyc_Absyn_strctq(
ad->name),_tmpB54))))));}else{struct _tuple7*_tmpB55;v=((_tmpB55=_region_malloc(d,
sizeof(*_tmpB55)),((_tmpB55->f1=ad,((_tmpB55->f2=Cyc_Absyn_unionq_typ(ad->name),
_tmpB55))))));}*_tmp731=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct
_tuple1*k,struct _tuple7*v))Cyc_Dict_insert)(*_tmp731,_tmp72E,v);}}else{struct
_tuple7 _tmp737;struct Cyc_Absyn_Aggrdecl*_tmp738;void*_tmp739;struct _tuple7*
_tmp736=*_tmp733;_tmp737=*_tmp736;_tmp738=_tmp737.f1;_tmp739=_tmp737.f2;if(
_tmp738->impl == 0){{struct _tuple7*_tmpB56;*_tmp731=((struct Cyc_Dict_Dict(*)(
struct Cyc_Dict_Dict d,struct _tuple1*k,struct _tuple7*v))Cyc_Dict_insert)(*_tmp731,
_tmp72E,((_tmpB56=_region_malloc(d,sizeof(*_tmpB56)),((_tmpB56->f1=ad,((_tmpB56->f2=
_tmp739,_tmpB56)))))));}seen_defn_before=0;}else{seen_defn_before=1;}}(void*)(ad->sc=(
void*)((void*)2));ad->tvs=0;if(ad->impl != 0){((struct Cyc_Absyn_AggrdeclImpl*)
_check_null(ad->impl))->exist_vars=0;((struct Cyc_Absyn_AggrdeclImpl*)_check_null(
ad->impl))->rgn_po=0;if(seen_defn_before)ad->impl=0;else{struct Cyc_List_List*
_tmp73B=((struct Cyc_Absyn_AggrdeclImpl*)_check_null(ad->impl))->fields;for(0;
_tmp73B != 0;_tmp73B=_tmp73B->tl){((struct Cyc_Absyn_Aggrfield*)_tmp73B->hd)->tq=
Cyc_Toc_mt_tq;(void*)(((struct Cyc_Absyn_Aggrfield*)_tmp73B->hd)->type=(void*)Cyc_Toc_typ_to_c_array((
void*)((struct Cyc_Absyn_Aggrfield*)_tmp73B->hd)->type));}}}};_pop_dynregion(d);}}
static void Cyc_Toc_tuniondecl_to_c(struct Cyc_Absyn_Tuniondecl*tud);static void Cyc_Toc_tuniondecl_to_c(
struct Cyc_Absyn_Tuniondecl*tud){struct _DynRegionHandle*_tmp73D;struct Cyc_Set_Set**
_tmp73E;struct Cyc_Toc_TocState _tmp73C=*((struct Cyc_Toc_TocState*)_check_null(Cyc_Toc_toc_state));
_tmp73D=_tmp73C.dyn;_tmp73E=_tmp73C.tunions_so_far;{struct _DynRegionFrame _tmp73F;
struct _RegionHandle*d=_open_dynregion(& _tmp73F,_tmp73D);{struct _tuple1*_tmp740=
tud->name;if(tud->fields == 0  || ((int(*)(struct Cyc_Set_Set*s,struct _tuple1*elt))
Cyc_Set_member)(*_tmp73E,_tmp740)){_npop_handler(0);return;}*_tmp73E=((struct Cyc_Set_Set*(*)(
struct _RegionHandle*r,struct Cyc_Set_Set*s,struct _tuple1*elt))Cyc_Set_rinsert)(d,*
_tmp73E,_tmp740);};_pop_dynregion(d);}{struct Cyc_List_List*flat_structs=0;{
struct Cyc_List_List*_tmp741=(struct Cyc_List_List*)((struct Cyc_Core_Opt*)
_check_null(tud->fields))->v;for(0;_tmp741 != 0;_tmp741=_tmp741->tl){struct Cyc_Absyn_Tunionfield*
f=(struct Cyc_Absyn_Tunionfield*)_tmp741->hd;if(f->typs != 0  || tud->is_flat){
struct Cyc_List_List*_tmp742=0;int i=1;{struct Cyc_List_List*_tmp743=f->typs;for(0;
_tmp743 != 0;(_tmp743=_tmp743->tl,i ++)){struct _dyneither_ptr*_tmp744=Cyc_Absyn_fieldname(
i);struct Cyc_Absyn_Aggrfield*_tmpB57;struct Cyc_Absyn_Aggrfield*_tmp745=(_tmpB57=
_cycalloc(sizeof(*_tmpB57)),((_tmpB57->name=_tmp744,((_tmpB57->tq=(*((struct
_tuple4*)_tmp743->hd)).f1,((_tmpB57->type=(void*)Cyc_Toc_typ_to_c_array((*((
struct _tuple4*)_tmp743->hd)).f2),((_tmpB57->width=0,((_tmpB57->attributes=0,
_tmpB57)))))))))));struct Cyc_List_List*_tmpB58;_tmp742=((_tmpB58=_cycalloc(
sizeof(*_tmpB58)),((_tmpB58->hd=_tmp745,((_tmpB58->tl=_tmp742,_tmpB58))))));}}{
struct Cyc_Absyn_Aggrfield*_tmpB5B;struct Cyc_List_List*_tmpB5A;_tmp742=((_tmpB5A=
_cycalloc(sizeof(*_tmpB5A)),((_tmpB5A->hd=((_tmpB5B=_cycalloc(sizeof(*_tmpB5B)),((
_tmpB5B->name=Cyc_Toc_tag_sp,((_tmpB5B->tq=Cyc_Toc_mt_tq,((_tmpB5B->type=(void*)
Cyc_Absyn_sint_typ,((_tmpB5B->width=0,((_tmpB5B->attributes=0,_tmpB5B)))))))))))),((
_tmpB5A->tl=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(
_tmp742),_tmpB5A))))));}{const char*_tmpB60;struct Cyc_Absyn_AggrdeclImpl*_tmpB5F;
struct Cyc_Absyn_Aggrdecl*_tmpB5E;struct Cyc_Absyn_Aggrdecl*_tmp74A=(_tmpB5E=
_cycalloc(sizeof(*_tmpB5E)),((_tmpB5E->kind=(void*)((void*)0),((_tmpB5E->sc=(
void*)((void*)2),((_tmpB5E->name=Cyc_Toc_collapse_qvar_tag(f->name,((_tmpB60="_struct",
_tag_dyneither(_tmpB60,sizeof(char),8)))),((_tmpB5E->tvs=0,((_tmpB5E->impl=((
_tmpB5F=_cycalloc(sizeof(*_tmpB5F)),((_tmpB5F->exist_vars=0,((_tmpB5F->rgn_po=0,((
_tmpB5F->fields=_tmp742,_tmpB5F)))))))),((_tmpB5E->attributes=0,_tmpB5E)))))))))))));{
struct Cyc_Absyn_Aggr_d_struct*_tmpB66;struct Cyc_Absyn_Aggr_d_struct _tmpB65;
struct Cyc_List_List*_tmpB64;Cyc_Toc_result_decls=((_tmpB64=_cycalloc(sizeof(*
_tmpB64)),((_tmpB64->hd=Cyc_Absyn_new_decl((void*)((_tmpB66=_cycalloc(sizeof(*
_tmpB66)),((_tmpB66[0]=((_tmpB65.tag=4,((_tmpB65.f1=_tmp74A,_tmpB65)))),_tmpB66)))),
0),((_tmpB64->tl=Cyc_Toc_result_decls,_tmpB64))))));}if(tud->is_flat){struct Cyc_Absyn_AggrType_struct*
_tmpB7B;struct Cyc_Absyn_AggrInfo _tmpB7A;struct Cyc_Absyn_Aggrdecl**_tmpB79;union
Cyc_Absyn_AggrInfoU_union _tmpB78;struct Cyc_Absyn_AggrType_struct _tmpB77;struct
Cyc_Absyn_Aggrfield*_tmpB76;struct Cyc_Absyn_Aggrfield*_tmp74E=(_tmpB76=_cycalloc(
sizeof(*_tmpB76)),((_tmpB76->name=(*f->name).f2,((_tmpB76->tq=Cyc_Toc_mt_tq,((
_tmpB76->type=(void*)((void*)((_tmpB7B=_cycalloc(sizeof(*_tmpB7B)),((_tmpB7B[0]=((
_tmpB77.tag=10,((_tmpB77.f1=((_tmpB7A.aggr_info=(union Cyc_Absyn_AggrInfoU_union)(((
_tmpB78.KnownAggr).tag=1,(((_tmpB78.KnownAggr).f1=((_tmpB79=_cycalloc(sizeof(*
_tmpB79)),((_tmpB79[0]=_tmp74A,_tmpB79)))),_tmpB78)))),((_tmpB7A.targs=0,_tmpB7A)))),
_tmpB77)))),_tmpB7B))))),((_tmpB76->width=0,((_tmpB76->attributes=0,_tmpB76)))))))))));
struct Cyc_List_List*_tmpB7C;flat_structs=((_tmpB7C=_cycalloc(sizeof(*_tmpB7C)),((
_tmpB7C->hd=_tmp74E,((_tmpB7C->tl=flat_structs,_tmpB7C))))));}}}}}if(tud->is_flat){
flat_structs=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(
flat_structs);{const char*_tmpB81;struct Cyc_Absyn_AggrdeclImpl*_tmpB80;struct Cyc_Absyn_Aggrdecl*
_tmpB7F;struct Cyc_Absyn_Aggrdecl*_tmp759=(_tmpB7F=_cycalloc(sizeof(*_tmpB7F)),((
_tmpB7F->kind=(void*)((void*)1),((_tmpB7F->sc=(void*)((void*)2),((_tmpB7F->name=
Cyc_Toc_collapse_qvar_tag(tud->name,((_tmpB81="_union",_tag_dyneither(_tmpB81,
sizeof(char),7)))),((_tmpB7F->tvs=0,((_tmpB7F->impl=((_tmpB80=_cycalloc(sizeof(*
_tmpB80)),((_tmpB80->exist_vars=0,((_tmpB80->rgn_po=0,((_tmpB80->fields=
flat_structs,_tmpB80)))))))),((_tmpB7F->attributes=0,_tmpB7F)))))))))))));struct
Cyc_Absyn_Aggr_d_struct*_tmpB87;struct Cyc_Absyn_Aggr_d_struct _tmpB86;struct Cyc_List_List*
_tmpB85;Cyc_Toc_result_decls=((_tmpB85=_cycalloc(sizeof(*_tmpB85)),((_tmpB85->hd=
Cyc_Absyn_new_decl((void*)((_tmpB87=_cycalloc(sizeof(*_tmpB87)),((_tmpB87[0]=((
_tmpB86.tag=4,((_tmpB86.f1=_tmp759,_tmpB86)))),_tmpB87)))),0),((_tmpB85->tl=Cyc_Toc_result_decls,
_tmpB85))))));}}}}static void Cyc_Toc_xtuniondecl_to_c(struct Cyc_Absyn_Tuniondecl*
xd);static void Cyc_Toc_xtuniondecl_to_c(struct Cyc_Absyn_Tuniondecl*xd){if(xd->fields
== 0)return;{struct _DynRegionHandle*_tmp761;struct Cyc_Dict_Dict*_tmp762;struct
Cyc_Toc_TocState _tmp760=*((struct Cyc_Toc_TocState*)_check_null(Cyc_Toc_toc_state));
_tmp761=_tmp760.dyn;_tmp762=_tmp760.xtunions_so_far;{struct _DynRegionFrame
_tmp763;struct _RegionHandle*d=_open_dynregion(& _tmp763,_tmp761);{struct _tuple1*
_tmp764=xd->name;struct Cyc_List_List*_tmp765=(struct Cyc_List_List*)((struct Cyc_Core_Opt*)
_check_null(xd->fields))->v;for(0;_tmp765 != 0;_tmp765=_tmp765->tl){struct Cyc_Absyn_Tunionfield*
f=(struct Cyc_Absyn_Tunionfield*)_tmp765->hd;struct _dyneither_ptr*fn=(*f->name).f2;
struct Cyc_Absyn_Exp*_tmp766=Cyc_Absyn_uint_exp(_get_dyneither_size(*fn,sizeof(
char))+ 4,0);void*_tmp767=Cyc_Absyn_array_typ(Cyc_Absyn_char_typ,Cyc_Toc_mt_tq,(
struct Cyc_Absyn_Exp*)_tmp766,Cyc_Absyn_false_conref,0);int*_tmp768=((int*(*)(
struct Cyc_Dict_Dict d,struct _tuple1*k))Cyc_Dict_lookup_opt)(*_tmp762,f->name);int
_tmp769;_LL390: if(_tmp768 != 0)goto _LL392;_LL391: {struct Cyc_Absyn_Exp*initopt=0;
if((void*)f->sc != (void*)3){char zero='\000';const char*_tmpB8F;void*_tmpB8E[5];
struct Cyc_Int_pa_struct _tmpB8D;struct Cyc_Int_pa_struct _tmpB8C;struct Cyc_Int_pa_struct
_tmpB8B;struct Cyc_Int_pa_struct _tmpB8A;struct Cyc_String_pa_struct _tmpB89;initopt=(
struct Cyc_Absyn_Exp*)Cyc_Absyn_string_exp((struct _dyneither_ptr)((_tmpB89.tag=0,((
_tmpB89.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*fn),((_tmpB8A.tag=1,((
_tmpB8A.f1=(unsigned long)((int)zero),((_tmpB8B.tag=1,((_tmpB8B.f1=(
unsigned long)((int)zero),((_tmpB8C.tag=1,((_tmpB8C.f1=(unsigned long)((int)zero),((
_tmpB8D.tag=1,((_tmpB8D.f1=(unsigned long)((int)zero),((_tmpB8E[0]=& _tmpB8D,((
_tmpB8E[1]=& _tmpB8C,((_tmpB8E[2]=& _tmpB8B,((_tmpB8E[3]=& _tmpB8A,((_tmpB8E[4]=&
_tmpB89,Cyc_aprintf(((_tmpB8F="%c%c%c%c%s",_tag_dyneither(_tmpB8F,sizeof(char),
11))),_tag_dyneither(_tmpB8E,sizeof(void*),5)))))))))))))))))))))))))))))))),0);}{
struct Cyc_Absyn_Vardecl*_tmp771=Cyc_Absyn_new_vardecl(f->name,_tmp767,initopt);(
void*)(_tmp771->sc=(void*)((void*)f->sc));{struct Cyc_Absyn_Var_d_struct*_tmpB95;
struct Cyc_Absyn_Var_d_struct _tmpB94;struct Cyc_List_List*_tmpB93;Cyc_Toc_result_decls=((
_tmpB93=_cycalloc(sizeof(*_tmpB93)),((_tmpB93->hd=Cyc_Absyn_new_decl((void*)((
_tmpB95=_cycalloc(sizeof(*_tmpB95)),((_tmpB95[0]=((_tmpB94.tag=0,((_tmpB94.f1=
_tmp771,_tmpB94)))),_tmpB95)))),0),((_tmpB93->tl=Cyc_Toc_result_decls,_tmpB93))))));}*
_tmp762=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct _tuple1*k,int v))Cyc_Dict_insert)(*
_tmp762,f->name,(void*)f->sc != (void*)3);if(f->typs != 0){struct Cyc_List_List*
fields=0;int i=1;{struct Cyc_List_List*_tmp775=f->typs;for(0;_tmp775 != 0;(_tmp775=
_tmp775->tl,i ++)){struct Cyc_Int_pa_struct _tmpB9D;void*_tmpB9C[1];const char*
_tmpB9B;struct _dyneither_ptr*_tmpB9A;struct _dyneither_ptr*_tmp776=(_tmpB9A=
_cycalloc(sizeof(*_tmpB9A)),((_tmpB9A[0]=(struct _dyneither_ptr)((_tmpB9D.tag=1,((
_tmpB9D.f1=(unsigned long)i,((_tmpB9C[0]=& _tmpB9D,Cyc_aprintf(((_tmpB9B="f%d",
_tag_dyneither(_tmpB9B,sizeof(char),4))),_tag_dyneither(_tmpB9C,sizeof(void*),1)))))))),
_tmpB9A)));struct Cyc_Absyn_Aggrfield*_tmpB9E;struct Cyc_Absyn_Aggrfield*_tmp777=(
_tmpB9E=_cycalloc(sizeof(*_tmpB9E)),((_tmpB9E->name=_tmp776,((_tmpB9E->tq=(*((
struct _tuple4*)_tmp775->hd)).f1,((_tmpB9E->type=(void*)Cyc_Toc_typ_to_c_array((*((
struct _tuple4*)_tmp775->hd)).f2),((_tmpB9E->width=0,((_tmpB9E->attributes=0,
_tmpB9E)))))))))));struct Cyc_List_List*_tmpB9F;fields=((_tmpB9F=_cycalloc(
sizeof(*_tmpB9F)),((_tmpB9F->hd=_tmp777,((_tmpB9F->tl=fields,_tmpB9F))))));}}{
struct Cyc_Absyn_Aggrfield*_tmpBA2;struct Cyc_List_List*_tmpBA1;fields=((_tmpBA1=
_cycalloc(sizeof(*_tmpBA1)),((_tmpBA1->hd=((_tmpBA2=_cycalloc(sizeof(*_tmpBA2)),((
_tmpBA2->name=Cyc_Toc_tag_sp,((_tmpBA2->tq=Cyc_Toc_mt_tq,((_tmpBA2->type=(void*)
Cyc_Absyn_cstar_typ(Cyc_Absyn_char_typ,Cyc_Toc_mt_tq),((_tmpBA2->width=0,((
_tmpBA2->attributes=0,_tmpBA2)))))))))))),((_tmpBA1->tl=((struct Cyc_List_List*(*)(
struct Cyc_List_List*x))Cyc_List_imp_rev)(fields),_tmpBA1))))));}{const char*
_tmpBA7;struct Cyc_Absyn_AggrdeclImpl*_tmpBA6;struct Cyc_Absyn_Aggrdecl*_tmpBA5;
struct Cyc_Absyn_Aggrdecl*_tmp780=(_tmpBA5=_cycalloc(sizeof(*_tmpBA5)),((_tmpBA5->kind=(
void*)((void*)0),((_tmpBA5->sc=(void*)((void*)2),((_tmpBA5->name=Cyc_Toc_collapse_qvar_tag(
f->name,((_tmpBA7="_struct",_tag_dyneither(_tmpBA7,sizeof(char),8)))),((_tmpBA5->tvs=
0,((_tmpBA5->impl=((_tmpBA6=_cycalloc(sizeof(*_tmpBA6)),((_tmpBA6->exist_vars=0,((
_tmpBA6->rgn_po=0,((_tmpBA6->fields=fields,_tmpBA6)))))))),((_tmpBA5->attributes=
0,_tmpBA5)))))))))))));struct Cyc_Absyn_Aggr_d_struct*_tmpBAD;struct Cyc_Absyn_Aggr_d_struct
_tmpBAC;struct Cyc_List_List*_tmpBAB;Cyc_Toc_result_decls=((_tmpBAB=_cycalloc(
sizeof(*_tmpBAB)),((_tmpBAB->hd=Cyc_Absyn_new_decl((void*)((_tmpBAD=_cycalloc(
sizeof(*_tmpBAD)),((_tmpBAD[0]=((_tmpBAC.tag=4,((_tmpBAC.f1=_tmp780,_tmpBAC)))),
_tmpBAD)))),0),((_tmpBAB->tl=Cyc_Toc_result_decls,_tmpBAB))))));}}goto _LL38F;}}
_LL392: if(_tmp768 == 0)goto _LL394;_tmp769=*_tmp768;if(_tmp769 != 0)goto _LL394;
_LL393: if((void*)f->sc != (void*)3){char zero='\000';const char*_tmpBB5;void*
_tmpBB4[5];struct Cyc_Int_pa_struct _tmpBB3;struct Cyc_Int_pa_struct _tmpBB2;struct
Cyc_Int_pa_struct _tmpBB1;struct Cyc_Int_pa_struct _tmpBB0;struct Cyc_String_pa_struct
_tmpBAF;struct Cyc_Absyn_Exp*_tmp787=Cyc_Absyn_string_exp((struct _dyneither_ptr)((
_tmpBAF.tag=0,((_tmpBAF.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*fn),((
_tmpBB0.tag=1,((_tmpBB0.f1=(unsigned long)((int)zero),((_tmpBB1.tag=1,((_tmpBB1.f1=(
unsigned long)((int)zero),((_tmpBB2.tag=1,((_tmpBB2.f1=(unsigned long)((int)zero),((
_tmpBB3.tag=1,((_tmpBB3.f1=(unsigned long)((int)zero),((_tmpBB4[0]=& _tmpBB3,((
_tmpBB4[1]=& _tmpBB2,((_tmpBB4[2]=& _tmpBB1,((_tmpBB4[3]=& _tmpBB0,((_tmpBB4[4]=&
_tmpBAF,Cyc_aprintf(((_tmpBB5="%c%c%c%c%s",_tag_dyneither(_tmpBB5,sizeof(char),
11))),_tag_dyneither(_tmpBB4,sizeof(void*),5)))))))))))))))))))))))))))))))),0);
struct Cyc_Absyn_Vardecl*_tmp788=Cyc_Absyn_new_vardecl(f->name,_tmp767,(struct Cyc_Absyn_Exp*)
_tmp787);(void*)(_tmp788->sc=(void*)((void*)f->sc));{struct Cyc_Absyn_Var_d_struct*
_tmpBBB;struct Cyc_Absyn_Var_d_struct _tmpBBA;struct Cyc_List_List*_tmpBB9;Cyc_Toc_result_decls=((
_tmpBB9=_cycalloc(sizeof(*_tmpBB9)),((_tmpBB9->hd=Cyc_Absyn_new_decl((void*)((
_tmpBBB=_cycalloc(sizeof(*_tmpBBB)),((_tmpBBB[0]=((_tmpBBA.tag=0,((_tmpBBA.f1=
_tmp788,_tmpBBA)))),_tmpBBB)))),0),((_tmpBB9->tl=Cyc_Toc_result_decls,_tmpBB9))))));}*
_tmp762=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct _tuple1*k,int v))Cyc_Dict_insert)(*
_tmp762,f->name,1);}goto _LL38F;_LL394:;_LL395: goto _LL38F;_LL38F:;}};
_pop_dynregion(d);}}}static void Cyc_Toc_enumdecl_to_c(struct Cyc_Toc_Env*nv,struct
Cyc_Absyn_Enumdecl*ed);static void Cyc_Toc_enumdecl_to_c(struct Cyc_Toc_Env*nv,
struct Cyc_Absyn_Enumdecl*ed){(void*)(ed->sc=(void*)((void*)2));if(ed->fields != 0)
Cyc_Toc_enumfields_to_c((struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(
ed->fields))->v);}static void Cyc_Toc_local_decl_to_c(struct Cyc_Toc_Env*body_nv,
struct Cyc_Toc_Env*init_nv,struct Cyc_Absyn_Vardecl*vd,struct Cyc_Absyn_Stmt*s);
static void Cyc_Toc_local_decl_to_c(struct Cyc_Toc_Env*body_nv,struct Cyc_Toc_Env*
init_nv,struct Cyc_Absyn_Vardecl*vd,struct Cyc_Absyn_Stmt*s){void*old_typ=(void*)
vd->type;(void*)(vd->type=(void*)Cyc_Toc_typ_to_c_array(old_typ));if((void*)vd->sc
== (void*)5  && Cyc_Tcutil_is_tagged_pointer_typ(old_typ))(void*)(vd->sc=(void*)((
void*)2));Cyc_Toc_stmt_to_c(body_nv,s);if(vd->initializer != 0){struct Cyc_Absyn_Exp*
init=(struct Cyc_Absyn_Exp*)_check_null(vd->initializer);void*_tmp793=(void*)init->r;
struct Cyc_Absyn_Vardecl*_tmp794;struct Cyc_Absyn_Exp*_tmp795;struct Cyc_Absyn_Exp*
_tmp796;int _tmp797;_LL397: if(*((int*)_tmp793)!= 29)goto _LL399;_tmp794=((struct
Cyc_Absyn_Comprehension_e_struct*)_tmp793)->f1;_tmp795=((struct Cyc_Absyn_Comprehension_e_struct*)
_tmp793)->f2;_tmp796=((struct Cyc_Absyn_Comprehension_e_struct*)_tmp793)->f3;
_tmp797=((struct Cyc_Absyn_Comprehension_e_struct*)_tmp793)->f4;_LL398: vd->initializer=
0;(void*)(s->r=(void*)((void*)(Cyc_Toc_init_comprehension(init_nv,Cyc_Absyn_var_exp(
vd->name,0),_tmp794,_tmp795,_tmp796,_tmp797,Cyc_Absyn_new_stmt((void*)s->r,0),0))->r));
goto _LL396;_LL399:;_LL39A: if((void*)vd->sc == (void*)0){struct _RegionHandle
_tmp798=_new_region("temp");struct _RegionHandle*temp=& _tmp798;_push_region(temp);{
struct Cyc_Toc_Env*_tmp799=Cyc_Toc_set_toplevel(temp,init_nv);Cyc_Toc_exp_to_c(
_tmp799,init);};_pop_region(temp);}else{Cyc_Toc_exp_to_c(init_nv,init);}goto
_LL396;_LL396:;}else{void*_tmp79A=Cyc_Tcutil_compress(old_typ);struct Cyc_Absyn_ArrayInfo
_tmp79B;void*_tmp79C;struct Cyc_Absyn_Exp*_tmp79D;struct Cyc_Absyn_Conref*_tmp79E;
_LL39C: if(_tmp79A <= (void*)4)goto _LL39E;if(*((int*)_tmp79A)!= 7)goto _LL39E;
_tmp79B=((struct Cyc_Absyn_ArrayType_struct*)_tmp79A)->f1;_tmp79C=(void*)_tmp79B.elt_type;
_tmp79D=_tmp79B.num_elts;_tmp79E=_tmp79B.zero_term;if(!((int(*)(int,struct Cyc_Absyn_Conref*
x))Cyc_Absyn_conref_def)(0,_tmp79E))goto _LL39E;_LL39D: if(_tmp79D == 0){const char*
_tmpBBE;void*_tmpBBD;(_tmpBBD=0,((int(*)(struct _dyneither_ptr fmt,struct
_dyneither_ptr ap))Cyc_Toc_toc_impos)(((_tmpBBE="can't initialize zero-terminated array -- size unknown",
_tag_dyneither(_tmpBBE,sizeof(char),55))),_tag_dyneither(_tmpBBD,sizeof(void*),0)));}{
struct Cyc_Absyn_Exp*num_elts=(struct Cyc_Absyn_Exp*)_tmp79D;struct Cyc_Absyn_Exp*
_tmp7A1=Cyc_Absyn_subscript_exp(Cyc_Absyn_var_exp(vd->name,0),Cyc_Absyn_add_exp(
num_elts,Cyc_Absyn_signed_int_exp(- 1,0),0),0);struct Cyc_Absyn_Exp*_tmp7A2=Cyc_Absyn_signed_int_exp(
0,0);(void*)(s->r=(void*)Cyc_Toc_seq_stmt_r(Cyc_Absyn_exp_stmt(Cyc_Absyn_assign_exp(
_tmp7A1,_tmp7A2,0),0),Cyc_Absyn_new_stmt((void*)s->r,0)));goto _LL39B;}_LL39E:;
_LL39F: goto _LL39B;_LL39B:;}}static struct Cyc_Absyn_Stmt**Cyc_Toc_throw_match_stmt_opt=
0;static struct Cyc_Absyn_Stmt*Cyc_Toc_throw_match_stmt();static struct Cyc_Absyn_Stmt*
Cyc_Toc_throw_match_stmt(){if(Cyc_Toc_throw_match_stmt_opt == 0){struct Cyc_Absyn_Stmt**
_tmpBBF;Cyc_Toc_throw_match_stmt_opt=((_tmpBBF=_cycalloc(sizeof(*_tmpBBF)),((
_tmpBBF[0]=Cyc_Absyn_exp_stmt(Cyc_Toc_newthrow_exp(Cyc_Absyn_match_exn_exp(0)),0),
_tmpBBF))));}return*((struct Cyc_Absyn_Stmt**)_check_null(Cyc_Toc_throw_match_stmt_opt));}
static struct Cyc_Absyn_Stmt*Cyc_Toc_letdecl_to_c(struct Cyc_Toc_Env*nv,struct Cyc_Absyn_Pat*
p,void*t,struct Cyc_Absyn_Exp*e,struct Cyc_Absyn_Stmt*s);static struct Cyc_Absyn_Stmt*
Cyc_Toc_letdecl_to_c(struct Cyc_Toc_Env*nv,struct Cyc_Absyn_Pat*p,void*t,struct Cyc_Absyn_Exp*
e,struct Cyc_Absyn_Stmt*s){Cyc_Toc_exp_to_c(nv,e);{struct _tuple1*x=Cyc_Toc_temp_var();{
struct _RegionHandle _tmp7A4=_new_region("prgn");struct _RegionHandle*prgn=& _tmp7A4;
_push_region(prgn);{struct Cyc_Absyn_Stmt*_tmp7A5=Cyc_Toc_throw_match_stmt();
struct Cyc_Toc_Env*_tmp7A6=Cyc_Toc_share_env(prgn,nv);struct Cyc_Toc_Env*_tmp7A8;
struct Cyc_List_List*_tmp7A9;struct Cyc_Absyn_Stmt*_tmp7AA;struct _tuple13 _tmp7A7=
Cyc_Toc_xlate_pat(_tmp7A6,prgn,t,Cyc_Absyn_var_exp(x,0),Cyc_Absyn_var_exp(x,0),p,(
struct Cyc_Absyn_Stmt**)& _tmp7A5,Cyc_Toc_throw_match_stmt(),0);_tmp7A8=_tmp7A7.f1;
_tmp7A9=_tmp7A7.f2;_tmp7AA=_tmp7A7.f3;Cyc_Toc_stmt_to_c(_tmp7A8,s);s=Cyc_Absyn_declare_stmt(
x,Cyc_Toc_typ_to_c(t),(struct Cyc_Absyn_Exp*)e,Cyc_Absyn_seq_stmt(_tmp7AA,s,0),0);
for(0;_tmp7A9 != 0;_tmp7A9=_tmp7A9->tl){struct _tuple14 _tmp7AC;struct _tuple1*
_tmp7AD;void*_tmp7AE;struct _tuple14*_tmp7AB=(struct _tuple14*)_tmp7A9->hd;_tmp7AC=*
_tmp7AB;_tmp7AD=_tmp7AC.f1;_tmp7AE=_tmp7AC.f2;s=Cyc_Absyn_declare_stmt(_tmp7AD,
_tmp7AE,0,s,0);}};_pop_region(prgn);}return s;}}static void Cyc_Toc_exptypes_to_c(
struct Cyc_Absyn_Exp*e);static void Cyc_Toc_exptypes_to_c(struct Cyc_Absyn_Exp*e){
void*_tmp7AF=(void*)e->r;struct Cyc_Absyn_Exp*_tmp7B0;struct Cyc_Absyn_Exp*_tmp7B1;
struct Cyc_Absyn_Exp*_tmp7B2;struct Cyc_Absyn_Exp*_tmp7B3;struct Cyc_Absyn_Exp*
_tmp7B4;struct Cyc_Absyn_Exp*_tmp7B5;struct Cyc_Absyn_Exp*_tmp7B6;struct Cyc_Absyn_Exp*
_tmp7B7;struct Cyc_List_List*_tmp7B8;struct Cyc_Absyn_Exp*_tmp7B9;struct Cyc_Absyn_Exp*
_tmp7BA;struct Cyc_Absyn_Exp*_tmp7BB;struct Cyc_Absyn_Exp*_tmp7BC;struct Cyc_Absyn_Exp*
_tmp7BD;struct Cyc_Absyn_Exp*_tmp7BE;struct Cyc_Absyn_Exp*_tmp7BF;struct Cyc_Absyn_Exp*
_tmp7C0;struct Cyc_Absyn_Exp*_tmp7C1;struct Cyc_Absyn_Exp*_tmp7C2;struct Cyc_Absyn_Exp*
_tmp7C3;struct Cyc_Absyn_Exp*_tmp7C4;struct Cyc_Absyn_Exp*_tmp7C5;struct Cyc_Absyn_Exp*
_tmp7C6;struct Cyc_Absyn_Exp*_tmp7C7;struct Cyc_Absyn_Exp*_tmp7C8;struct Cyc_List_List*
_tmp7C9;struct Cyc_Absyn_Exp*_tmp7CA;struct Cyc_List_List*_tmp7CB;void*_tmp7CC;
void**_tmp7CD;struct Cyc_Absyn_Exp*_tmp7CE;struct _tuple2*_tmp7CF;struct _tuple2
_tmp7D0;void*_tmp7D1;void**_tmp7D2;struct Cyc_List_List*_tmp7D3;struct Cyc_List_List*
_tmp7D4;struct Cyc_List_List*_tmp7D5;void*_tmp7D6;void**_tmp7D7;void*_tmp7D8;void**
_tmp7D9;struct Cyc_Absyn_Stmt*_tmp7DA;struct Cyc_Absyn_MallocInfo _tmp7DB;struct Cyc_Absyn_MallocInfo*
_tmp7DC;_LL3A1: if(*((int*)_tmp7AF)!= 22)goto _LL3A3;_tmp7B0=((struct Cyc_Absyn_Deref_e_struct*)
_tmp7AF)->f1;_LL3A2: _tmp7B1=_tmp7B0;goto _LL3A4;_LL3A3: if(*((int*)_tmp7AF)!= 23)
goto _LL3A5;_tmp7B1=((struct Cyc_Absyn_AggrMember_e_struct*)_tmp7AF)->f1;_LL3A4:
_tmp7B2=_tmp7B1;goto _LL3A6;_LL3A5: if(*((int*)_tmp7AF)!= 24)goto _LL3A7;_tmp7B2=((
struct Cyc_Absyn_AggrArrow_e_struct*)_tmp7AF)->f1;_LL3A6: _tmp7B3=_tmp7B2;goto
_LL3A8;_LL3A7: if(*((int*)_tmp7AF)!= 16)goto _LL3A9;_tmp7B3=((struct Cyc_Absyn_Address_e_struct*)
_tmp7AF)->f1;_LL3A8: _tmp7B4=_tmp7B3;goto _LL3AA;_LL3A9: if(*((int*)_tmp7AF)!= 12)
goto _LL3AB;_tmp7B4=((struct Cyc_Absyn_Throw_e_struct*)_tmp7AF)->f1;_LL3AA: _tmp7B5=
_tmp7B4;goto _LL3AC;_LL3AB: if(*((int*)_tmp7AF)!= 13)goto _LL3AD;_tmp7B5=((struct
Cyc_Absyn_NoInstantiate_e_struct*)_tmp7AF)->f1;_LL3AC: _tmp7B6=_tmp7B5;goto _LL3AE;
_LL3AD: if(*((int*)_tmp7AF)!= 19)goto _LL3AF;_tmp7B6=((struct Cyc_Absyn_Sizeofexp_e_struct*)
_tmp7AF)->f1;_LL3AE: _tmp7B7=_tmp7B6;goto _LL3B0;_LL3AF: if(*((int*)_tmp7AF)!= 5)
goto _LL3B1;_tmp7B7=((struct Cyc_Absyn_Increment_e_struct*)_tmp7AF)->f1;_LL3B0: Cyc_Toc_exptypes_to_c(
_tmp7B7);goto _LL3A0;_LL3B1: if(*((int*)_tmp7AF)!= 3)goto _LL3B3;_tmp7B8=((struct
Cyc_Absyn_Primop_e_struct*)_tmp7AF)->f2;_LL3B2:((void(*)(void(*f)(struct Cyc_Absyn_Exp*),
struct Cyc_List_List*x))Cyc_List_iter)(Cyc_Toc_exptypes_to_c,_tmp7B8);goto _LL3A0;
_LL3B3: if(*((int*)_tmp7AF)!= 7)goto _LL3B5;_tmp7B9=((struct Cyc_Absyn_And_e_struct*)
_tmp7AF)->f1;_tmp7BA=((struct Cyc_Absyn_And_e_struct*)_tmp7AF)->f2;_LL3B4: _tmp7BB=
_tmp7B9;_tmp7BC=_tmp7BA;goto _LL3B6;_LL3B5: if(*((int*)_tmp7AF)!= 8)goto _LL3B7;
_tmp7BB=((struct Cyc_Absyn_Or_e_struct*)_tmp7AF)->f1;_tmp7BC=((struct Cyc_Absyn_Or_e_struct*)
_tmp7AF)->f2;_LL3B6: _tmp7BD=_tmp7BB;_tmp7BE=_tmp7BC;goto _LL3B8;_LL3B7: if(*((int*)
_tmp7AF)!= 9)goto _LL3B9;_tmp7BD=((struct Cyc_Absyn_SeqExp_e_struct*)_tmp7AF)->f1;
_tmp7BE=((struct Cyc_Absyn_SeqExp_e_struct*)_tmp7AF)->f2;_LL3B8: _tmp7BF=_tmp7BD;
_tmp7C0=_tmp7BE;goto _LL3BA;_LL3B9: if(*((int*)_tmp7AF)!= 25)goto _LL3BB;_tmp7BF=((
struct Cyc_Absyn_Subscript_e_struct*)_tmp7AF)->f1;_tmp7C0=((struct Cyc_Absyn_Subscript_e_struct*)
_tmp7AF)->f2;_LL3BA: _tmp7C1=_tmp7BF;_tmp7C2=_tmp7C0;goto _LL3BC;_LL3BB: if(*((int*)
_tmp7AF)!= 36)goto _LL3BD;_tmp7C1=((struct Cyc_Absyn_Swap_e_struct*)_tmp7AF)->f1;
_tmp7C2=((struct Cyc_Absyn_Swap_e_struct*)_tmp7AF)->f2;_LL3BC: _tmp7C3=_tmp7C1;
_tmp7C4=_tmp7C2;goto _LL3BE;_LL3BD: if(*((int*)_tmp7AF)!= 4)goto _LL3BF;_tmp7C3=((
struct Cyc_Absyn_AssignOp_e_struct*)_tmp7AF)->f1;_tmp7C4=((struct Cyc_Absyn_AssignOp_e_struct*)
_tmp7AF)->f3;_LL3BE: Cyc_Toc_exptypes_to_c(_tmp7C3);Cyc_Toc_exptypes_to_c(_tmp7C4);
goto _LL3A0;_LL3BF: if(*((int*)_tmp7AF)!= 6)goto _LL3C1;_tmp7C5=((struct Cyc_Absyn_Conditional_e_struct*)
_tmp7AF)->f1;_tmp7C6=((struct Cyc_Absyn_Conditional_e_struct*)_tmp7AF)->f2;
_tmp7C7=((struct Cyc_Absyn_Conditional_e_struct*)_tmp7AF)->f3;_LL3C0: Cyc_Toc_exptypes_to_c(
_tmp7C5);Cyc_Toc_exptypes_to_c(_tmp7C6);Cyc_Toc_exptypes_to_c(_tmp7C7);goto
_LL3A0;_LL3C1: if(*((int*)_tmp7AF)!= 11)goto _LL3C3;_tmp7C8=((struct Cyc_Absyn_FnCall_e_struct*)
_tmp7AF)->f1;_tmp7C9=((struct Cyc_Absyn_FnCall_e_struct*)_tmp7AF)->f2;_LL3C2:
_tmp7CA=_tmp7C8;_tmp7CB=_tmp7C9;goto _LL3C4;_LL3C3: if(*((int*)_tmp7AF)!= 10)goto
_LL3C5;_tmp7CA=((struct Cyc_Absyn_UnknownCall_e_struct*)_tmp7AF)->f1;_tmp7CB=((
struct Cyc_Absyn_UnknownCall_e_struct*)_tmp7AF)->f2;_LL3C4: Cyc_Toc_exptypes_to_c(
_tmp7CA);((void(*)(void(*f)(struct Cyc_Absyn_Exp*),struct Cyc_List_List*x))Cyc_List_iter)(
Cyc_Toc_exptypes_to_c,_tmp7CB);goto _LL3A0;_LL3C5: if(*((int*)_tmp7AF)!= 15)goto
_LL3C7;_tmp7CC=(void*)((struct Cyc_Absyn_Cast_e_struct*)_tmp7AF)->f1;_tmp7CD=(
void**)&((void*)((struct Cyc_Absyn_Cast_e_struct*)_tmp7AF)->f1);_tmp7CE=((struct
Cyc_Absyn_Cast_e_struct*)_tmp7AF)->f2;_LL3C6:*_tmp7CD=Cyc_Toc_typ_to_c(*_tmp7CD);
Cyc_Toc_exptypes_to_c(_tmp7CE);goto _LL3A0;_LL3C7: if(*((int*)_tmp7AF)!= 27)goto
_LL3C9;_tmp7CF=((struct Cyc_Absyn_CompoundLit_e_struct*)_tmp7AF)->f1;_tmp7D0=*
_tmp7CF;_tmp7D1=_tmp7D0.f3;_tmp7D2=(void**)&(*((struct Cyc_Absyn_CompoundLit_e_struct*)
_tmp7AF)->f1).f3;_tmp7D3=((struct Cyc_Absyn_CompoundLit_e_struct*)_tmp7AF)->f2;
_LL3C8:*_tmp7D2=Cyc_Toc_typ_to_c(*_tmp7D2);_tmp7D4=_tmp7D3;goto _LL3CA;_LL3C9: if(*((
int*)_tmp7AF)!= 37)goto _LL3CB;_tmp7D4=((struct Cyc_Absyn_UnresolvedMem_e_struct*)
_tmp7AF)->f2;_LL3CA: _tmp7D5=_tmp7D4;goto _LL3CC;_LL3CB: if(*((int*)_tmp7AF)!= 28)
goto _LL3CD;_tmp7D5=((struct Cyc_Absyn_Array_e_struct*)_tmp7AF)->f1;_LL3CC: for(0;
_tmp7D5 != 0;_tmp7D5=_tmp7D5->tl){struct Cyc_Absyn_Exp*_tmp7DE;struct _tuple8
_tmp7DD=*((struct _tuple8*)_tmp7D5->hd);_tmp7DE=_tmp7DD.f2;Cyc_Toc_exptypes_to_c(
_tmp7DE);}goto _LL3A0;_LL3CD: if(*((int*)_tmp7AF)!= 20)goto _LL3CF;_tmp7D6=(void*)((
struct Cyc_Absyn_Offsetof_e_struct*)_tmp7AF)->f1;_tmp7D7=(void**)&((void*)((
struct Cyc_Absyn_Offsetof_e_struct*)_tmp7AF)->f1);_LL3CE: _tmp7D9=_tmp7D7;goto
_LL3D0;_LL3CF: if(*((int*)_tmp7AF)!= 18)goto _LL3D1;_tmp7D8=(void*)((struct Cyc_Absyn_Sizeoftyp_e_struct*)
_tmp7AF)->f1;_tmp7D9=(void**)&((void*)((struct Cyc_Absyn_Sizeoftyp_e_struct*)
_tmp7AF)->f1);_LL3D0:*_tmp7D9=Cyc_Toc_typ_to_c(*_tmp7D9);goto _LL3A0;_LL3D1: if(*((
int*)_tmp7AF)!= 38)goto _LL3D3;_tmp7DA=((struct Cyc_Absyn_StmtExp_e_struct*)
_tmp7AF)->f1;_LL3D2: Cyc_Toc_stmttypes_to_c(_tmp7DA);goto _LL3A0;_LL3D3: if(*((int*)
_tmp7AF)!= 35)goto _LL3D5;_tmp7DB=((struct Cyc_Absyn_Malloc_e_struct*)_tmp7AF)->f1;
_tmp7DC=(struct Cyc_Absyn_MallocInfo*)&((struct Cyc_Absyn_Malloc_e_struct*)_tmp7AF)->f1;
_LL3D4: if(_tmp7DC->elt_type != 0){void**_tmpBC0;_tmp7DC->elt_type=((_tmpBC0=
_cycalloc(sizeof(*_tmpBC0)),((_tmpBC0[0]=Cyc_Toc_typ_to_c(*((void**)_check_null(
_tmp7DC->elt_type))),_tmpBC0))));}Cyc_Toc_exptypes_to_c(_tmp7DC->num_elts);goto
_LL3A0;_LL3D5: if(*((int*)_tmp7AF)!= 0)goto _LL3D7;_LL3D6: goto _LL3D8;_LL3D7: if(*((
int*)_tmp7AF)!= 1)goto _LL3D9;_LL3D8: goto _LL3DA;_LL3D9: if(*((int*)_tmp7AF)!= 2)
goto _LL3DB;_LL3DA: goto _LL3DC;_LL3DB: if(*((int*)_tmp7AF)!= 33)goto _LL3DD;_LL3DC:
goto _LL3DE;_LL3DD: if(*((int*)_tmp7AF)!= 34)goto _LL3DF;_LL3DE: goto _LL3A0;_LL3DF:
if(*((int*)_tmp7AF)!= 31)goto _LL3E1;_LL3E0: goto _LL3E2;_LL3E1: if(*((int*)_tmp7AF)
!= 32)goto _LL3E3;_LL3E2: goto _LL3E4;_LL3E3: if(*((int*)_tmp7AF)!= 30)goto _LL3E5;
_LL3E4: goto _LL3E6;_LL3E5: if(*((int*)_tmp7AF)!= 29)goto _LL3E7;_LL3E6: goto _LL3E8;
_LL3E7: if(*((int*)_tmp7AF)!= 26)goto _LL3E9;_LL3E8: goto _LL3EA;_LL3E9: if(*((int*)
_tmp7AF)!= 14)goto _LL3EB;_LL3EA: goto _LL3EC;_LL3EB: if(*((int*)_tmp7AF)!= 17)goto
_LL3ED;_LL3EC: goto _LL3EE;_LL3ED: if(*((int*)_tmp7AF)!= 39)goto _LL3EF;_LL3EE: goto
_LL3F0;_LL3EF: if(*((int*)_tmp7AF)!= 21)goto _LL3A0;_LL3F0:{const char*_tmpBC3;void*
_tmpBC2;(_tmpBC2=0,Cyc_Tcutil_terr(e->loc,((_tmpBC3="Cyclone expression within C code",
_tag_dyneither(_tmpBC3,sizeof(char),33))),_tag_dyneither(_tmpBC2,sizeof(void*),0)));}
goto _LL3A0;_LL3A0:;}static void Cyc_Toc_decltypes_to_c(struct Cyc_Absyn_Decl*d);
static void Cyc_Toc_decltypes_to_c(struct Cyc_Absyn_Decl*d){void*_tmp7E2=(void*)d->r;
struct Cyc_Absyn_Vardecl*_tmp7E3;struct Cyc_Absyn_Fndecl*_tmp7E4;struct Cyc_Absyn_Aggrdecl*
_tmp7E5;struct Cyc_Absyn_Enumdecl*_tmp7E6;struct Cyc_Absyn_Typedefdecl*_tmp7E7;
_LL3F2: if(_tmp7E2 <= (void*)2)goto _LL40A;if(*((int*)_tmp7E2)!= 0)goto _LL3F4;
_tmp7E3=((struct Cyc_Absyn_Var_d_struct*)_tmp7E2)->f1;_LL3F3:(void*)(_tmp7E3->type=(
void*)Cyc_Toc_typ_to_c((void*)_tmp7E3->type));if(_tmp7E3->initializer != 0)Cyc_Toc_exptypes_to_c((
struct Cyc_Absyn_Exp*)_check_null(_tmp7E3->initializer));goto _LL3F1;_LL3F4: if(*((
int*)_tmp7E2)!= 1)goto _LL3F6;_tmp7E4=((struct Cyc_Absyn_Fn_d_struct*)_tmp7E2)->f1;
_LL3F5:(void*)(_tmp7E4->ret_type=(void*)Cyc_Toc_typ_to_c((void*)_tmp7E4->ret_type));{
struct Cyc_List_List*_tmp7E8=_tmp7E4->args;for(0;_tmp7E8 != 0;_tmp7E8=_tmp7E8->tl){(*((
struct _tuple17*)_tmp7E8->hd)).f3=Cyc_Toc_typ_to_c((*((struct _tuple17*)_tmp7E8->hd)).f3);}}
goto _LL3F1;_LL3F6: if(*((int*)_tmp7E2)!= 4)goto _LL3F8;_tmp7E5=((struct Cyc_Absyn_Aggr_d_struct*)
_tmp7E2)->f1;_LL3F7: Cyc_Toc_aggrdecl_to_c(_tmp7E5);goto _LL3F1;_LL3F8: if(*((int*)
_tmp7E2)!= 6)goto _LL3FA;_tmp7E6=((struct Cyc_Absyn_Enum_d_struct*)_tmp7E2)->f1;
_LL3F9: if(_tmp7E6->fields != 0){struct Cyc_List_List*_tmp7E9=(struct Cyc_List_List*)((
struct Cyc_Core_Opt*)_check_null(_tmp7E6->fields))->v;for(0;_tmp7E9 != 0;_tmp7E9=
_tmp7E9->tl){struct Cyc_Absyn_Enumfield*_tmp7EA=(struct Cyc_Absyn_Enumfield*)
_tmp7E9->hd;if(_tmp7EA->tag != 0)Cyc_Toc_exptypes_to_c((struct Cyc_Absyn_Exp*)
_check_null(_tmp7EA->tag));}}goto _LL3F1;_LL3FA: if(*((int*)_tmp7E2)!= 7)goto
_LL3FC;_tmp7E7=((struct Cyc_Absyn_Typedef_d_struct*)_tmp7E2)->f1;_LL3FB:{struct
Cyc_Core_Opt*_tmpBC4;_tmp7E7->defn=((_tmpBC4=_cycalloc(sizeof(*_tmpBC4)),((
_tmpBC4->v=(void*)Cyc_Toc_typ_to_c_array((void*)((struct Cyc_Core_Opt*)
_check_null(_tmp7E7->defn))->v),_tmpBC4))));}goto _LL3F1;_LL3FC: if(*((int*)
_tmp7E2)!= 2)goto _LL3FE;_LL3FD: goto _LL3FF;_LL3FE: if(*((int*)_tmp7E2)!= 3)goto
_LL400;_LL3FF: goto _LL401;_LL400: if(*((int*)_tmp7E2)!= 5)goto _LL402;_LL401: goto
_LL403;_LL402: if(*((int*)_tmp7E2)!= 8)goto _LL404;_LL403: goto _LL405;_LL404: if(*((
int*)_tmp7E2)!= 9)goto _LL406;_LL405: goto _LL407;_LL406: if(*((int*)_tmp7E2)!= 10)
goto _LL408;_LL407: goto _LL409;_LL408: if(*((int*)_tmp7E2)!= 11)goto _LL40A;_LL409:{
const char*_tmpBC7;void*_tmpBC6;(_tmpBC6=0,Cyc_Tcutil_terr(d->loc,((_tmpBC7="Cyclone declaration within C code",
_tag_dyneither(_tmpBC7,sizeof(char),34))),_tag_dyneither(_tmpBC6,sizeof(void*),0)));}
goto _LL3F1;_LL40A: if((int)_tmp7E2 != 0)goto _LL40C;_LL40B: goto _LL40D;_LL40C: if((
int)_tmp7E2 != 1)goto _LL3F1;_LL40D: goto _LL3F1;_LL3F1:;}static void Cyc_Toc_stmttypes_to_c(
struct Cyc_Absyn_Stmt*s);static void Cyc_Toc_stmttypes_to_c(struct Cyc_Absyn_Stmt*s){
void*_tmp7EE=(void*)s->r;struct Cyc_Absyn_Exp*_tmp7EF;struct Cyc_Absyn_Stmt*
_tmp7F0;struct Cyc_Absyn_Stmt*_tmp7F1;struct Cyc_Absyn_Exp*_tmp7F2;struct Cyc_Absyn_Exp*
_tmp7F3;struct Cyc_Absyn_Stmt*_tmp7F4;struct Cyc_Absyn_Stmt*_tmp7F5;struct _tuple3
_tmp7F6;struct Cyc_Absyn_Exp*_tmp7F7;struct Cyc_Absyn_Stmt*_tmp7F8;struct Cyc_Absyn_Exp*
_tmp7F9;struct _tuple3 _tmp7FA;struct Cyc_Absyn_Exp*_tmp7FB;struct _tuple3 _tmp7FC;
struct Cyc_Absyn_Exp*_tmp7FD;struct Cyc_Absyn_Stmt*_tmp7FE;struct Cyc_Absyn_Exp*
_tmp7FF;struct Cyc_List_List*_tmp800;struct Cyc_Absyn_Decl*_tmp801;struct Cyc_Absyn_Stmt*
_tmp802;struct Cyc_Absyn_Stmt*_tmp803;struct _tuple3 _tmp804;struct Cyc_Absyn_Exp*
_tmp805;_LL40F: if(_tmp7EE <= (void*)1)goto _LL421;if(*((int*)_tmp7EE)!= 0)goto
_LL411;_tmp7EF=((struct Cyc_Absyn_Exp_s_struct*)_tmp7EE)->f1;_LL410: Cyc_Toc_exptypes_to_c(
_tmp7EF);goto _LL40E;_LL411: if(*((int*)_tmp7EE)!= 1)goto _LL413;_tmp7F0=((struct
Cyc_Absyn_Seq_s_struct*)_tmp7EE)->f1;_tmp7F1=((struct Cyc_Absyn_Seq_s_struct*)
_tmp7EE)->f2;_LL412: Cyc_Toc_stmttypes_to_c(_tmp7F0);Cyc_Toc_stmttypes_to_c(
_tmp7F1);goto _LL40E;_LL413: if(*((int*)_tmp7EE)!= 2)goto _LL415;_tmp7F2=((struct
Cyc_Absyn_Return_s_struct*)_tmp7EE)->f1;_LL414: if(_tmp7F2 != 0)Cyc_Toc_exptypes_to_c((
struct Cyc_Absyn_Exp*)_tmp7F2);goto _LL40E;_LL415: if(*((int*)_tmp7EE)!= 3)goto
_LL417;_tmp7F3=((struct Cyc_Absyn_IfThenElse_s_struct*)_tmp7EE)->f1;_tmp7F4=((
struct Cyc_Absyn_IfThenElse_s_struct*)_tmp7EE)->f2;_tmp7F5=((struct Cyc_Absyn_IfThenElse_s_struct*)
_tmp7EE)->f3;_LL416: Cyc_Toc_exptypes_to_c(_tmp7F3);Cyc_Toc_stmttypes_to_c(
_tmp7F4);Cyc_Toc_stmttypes_to_c(_tmp7F5);goto _LL40E;_LL417: if(*((int*)_tmp7EE)!= 
4)goto _LL419;_tmp7F6=((struct Cyc_Absyn_While_s_struct*)_tmp7EE)->f1;_tmp7F7=
_tmp7F6.f1;_tmp7F8=((struct Cyc_Absyn_While_s_struct*)_tmp7EE)->f2;_LL418: Cyc_Toc_exptypes_to_c(
_tmp7F7);Cyc_Toc_stmttypes_to_c(_tmp7F8);goto _LL40E;_LL419: if(*((int*)_tmp7EE)!= 
8)goto _LL41B;_tmp7F9=((struct Cyc_Absyn_For_s_struct*)_tmp7EE)->f1;_tmp7FA=((
struct Cyc_Absyn_For_s_struct*)_tmp7EE)->f2;_tmp7FB=_tmp7FA.f1;_tmp7FC=((struct
Cyc_Absyn_For_s_struct*)_tmp7EE)->f3;_tmp7FD=_tmp7FC.f1;_tmp7FE=((struct Cyc_Absyn_For_s_struct*)
_tmp7EE)->f4;_LL41A: Cyc_Toc_exptypes_to_c(_tmp7F9);Cyc_Toc_exptypes_to_c(_tmp7FB);
Cyc_Toc_exptypes_to_c(_tmp7FD);Cyc_Toc_stmttypes_to_c(_tmp7FE);goto _LL40E;_LL41B:
if(*((int*)_tmp7EE)!= 9)goto _LL41D;_tmp7FF=((struct Cyc_Absyn_Switch_s_struct*)
_tmp7EE)->f1;_tmp800=((struct Cyc_Absyn_Switch_s_struct*)_tmp7EE)->f2;_LL41C: Cyc_Toc_exptypes_to_c(
_tmp7FF);for(0;_tmp800 != 0;_tmp800=_tmp800->tl){Cyc_Toc_stmttypes_to_c(((struct
Cyc_Absyn_Switch_clause*)_tmp800->hd)->body);}goto _LL40E;_LL41D: if(*((int*)
_tmp7EE)!= 11)goto _LL41F;_tmp801=((struct Cyc_Absyn_Decl_s_struct*)_tmp7EE)->f1;
_tmp802=((struct Cyc_Absyn_Decl_s_struct*)_tmp7EE)->f2;_LL41E: Cyc_Toc_decltypes_to_c(
_tmp801);Cyc_Toc_stmttypes_to_c(_tmp802);goto _LL40E;_LL41F: if(*((int*)_tmp7EE)!= 
13)goto _LL421;_tmp803=((struct Cyc_Absyn_Do_s_struct*)_tmp7EE)->f1;_tmp804=((
struct Cyc_Absyn_Do_s_struct*)_tmp7EE)->f2;_tmp805=_tmp804.f1;_LL420: Cyc_Toc_stmttypes_to_c(
_tmp803);Cyc_Toc_exptypes_to_c(_tmp805);goto _LL40E;_LL421: if((int)_tmp7EE != 0)
goto _LL423;_LL422: goto _LL424;_LL423: if(_tmp7EE <= (void*)1)goto _LL425;if(*((int*)
_tmp7EE)!= 5)goto _LL425;_LL424: goto _LL426;_LL425: if(_tmp7EE <= (void*)1)goto
_LL427;if(*((int*)_tmp7EE)!= 6)goto _LL427;_LL426: goto _LL428;_LL427: if(_tmp7EE <= (
void*)1)goto _LL429;if(*((int*)_tmp7EE)!= 7)goto _LL429;_LL428: goto _LL40E;_LL429:
if(_tmp7EE <= (void*)1)goto _LL42B;if(*((int*)_tmp7EE)!= 10)goto _LL42B;_LL42A: goto
_LL42C;_LL42B: if(_tmp7EE <= (void*)1)goto _LL42D;if(*((int*)_tmp7EE)!= 12)goto
_LL42D;_LL42C: goto _LL42E;_LL42D: if(_tmp7EE <= (void*)1)goto _LL42F;if(*((int*)
_tmp7EE)!= 14)goto _LL42F;_LL42E: goto _LL430;_LL42F: if(_tmp7EE <= (void*)1)goto
_LL431;if(*((int*)_tmp7EE)!= 15)goto _LL431;_LL430: goto _LL432;_LL431: if(_tmp7EE <= (
void*)1)goto _LL433;if(*((int*)_tmp7EE)!= 17)goto _LL433;_LL432: goto _LL434;_LL433:
if(_tmp7EE <= (void*)1)goto _LL40E;if(*((int*)_tmp7EE)!= 16)goto _LL40E;_LL434:{
const char*_tmpBCA;void*_tmpBC9;(_tmpBC9=0,Cyc_Tcutil_terr(s->loc,((_tmpBCA="Cyclone statement in C code",
_tag_dyneither(_tmpBCA,sizeof(char),28))),_tag_dyneither(_tmpBC9,sizeof(void*),0)));}
goto _LL40E;_LL40E:;}static struct Cyc_Toc_Env*Cyc_Toc_decls_to_c(struct
_RegionHandle*r,struct Cyc_Toc_Env*nv,struct Cyc_List_List*ds,int top,int cinclude);
static struct Cyc_Toc_Env*Cyc_Toc_decls_to_c(struct _RegionHandle*r,struct Cyc_Toc_Env*
nv,struct Cyc_List_List*ds,int top,int cinclude){for(0;ds != 0;ds=ds->tl){if(!Cyc_Toc_is_toplevel(
nv)){const char*_tmpBCD;void*_tmpBCC;(_tmpBCC=0,((int(*)(struct _dyneither_ptr fmt,
struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmpBCD="decls_to_c: not at toplevel!",
_tag_dyneither(_tmpBCD,sizeof(char),29))),_tag_dyneither(_tmpBCC,sizeof(void*),0)));}{
struct Cyc_Absyn_Decl*d=(struct Cyc_Absyn_Decl*)ds->hd;void*_tmp80A=(void*)d->r;
struct Cyc_Absyn_Vardecl*_tmp80B;struct Cyc_Absyn_Fndecl*_tmp80C;struct Cyc_Absyn_Aggrdecl*
_tmp80D;struct Cyc_Absyn_Tuniondecl*_tmp80E;struct Cyc_Absyn_Enumdecl*_tmp80F;
struct Cyc_Absyn_Typedefdecl*_tmp810;struct Cyc_List_List*_tmp811;struct Cyc_List_List*
_tmp812;struct Cyc_List_List*_tmp813;struct Cyc_List_List*_tmp814;_LL436: if(
_tmp80A <= (void*)2)goto _LL446;if(*((int*)_tmp80A)!= 0)goto _LL438;_tmp80B=((
struct Cyc_Absyn_Var_d_struct*)_tmp80A)->f1;_LL437: {struct _tuple1*_tmp815=
_tmp80B->name;if((void*)_tmp80B->sc == (void*)4){union Cyc_Absyn_Nmspace_union
_tmpBD0;struct _tuple1*_tmpBCF;_tmp815=((_tmpBCF=_cycalloc(sizeof(*_tmpBCF)),((
_tmpBCF->f1=(union Cyc_Absyn_Nmspace_union)(((_tmpBD0.Rel_n).tag=1,(((_tmpBD0.Rel_n).f1=
0,_tmpBD0)))),((_tmpBCF->f2=(*_tmp815).f2,_tmpBCF))))));}if(_tmp80B->initializer
!= 0){if(cinclude)Cyc_Toc_exptypes_to_c((struct Cyc_Absyn_Exp*)_check_null(
_tmp80B->initializer));else{Cyc_Toc_exp_to_c(nv,(struct Cyc_Absyn_Exp*)
_check_null(_tmp80B->initializer));}}{struct Cyc_Absyn_Global_b_struct _tmpBD3;
struct Cyc_Absyn_Global_b_struct*_tmpBD2;nv=Cyc_Toc_add_varmap(r,nv,_tmp80B->name,
Cyc_Absyn_varb_exp(_tmp815,(void*)((_tmpBD2=_cycalloc(sizeof(*_tmpBD2)),((
_tmpBD2[0]=((_tmpBD3.tag=0,((_tmpBD3.f1=_tmp80B,_tmpBD3)))),_tmpBD2)))),0));}
_tmp80B->name=_tmp815;(void*)(_tmp80B->sc=(void*)Cyc_Toc_scope_to_c((void*)
_tmp80B->sc));(void*)(_tmp80B->type=(void*)Cyc_Toc_typ_to_c_array((void*)_tmp80B->type));{
struct Cyc_List_List*_tmpBD4;Cyc_Toc_result_decls=((_tmpBD4=_cycalloc(sizeof(*
_tmpBD4)),((_tmpBD4->hd=d,((_tmpBD4->tl=Cyc_Toc_result_decls,_tmpBD4))))));}goto
_LL435;}_LL438: if(*((int*)_tmp80A)!= 1)goto _LL43A;_tmp80C=((struct Cyc_Absyn_Fn_d_struct*)
_tmp80A)->f1;_LL439: {struct _tuple1*_tmp81B=_tmp80C->name;if((void*)_tmp80C->sc
== (void*)4){{union Cyc_Absyn_Nmspace_union _tmpBD7;struct _tuple1*_tmpBD6;_tmp81B=((
_tmpBD6=_cycalloc(sizeof(*_tmpBD6)),((_tmpBD6->f1=(union Cyc_Absyn_Nmspace_union)(((
_tmpBD7.Rel_n).tag=1,(((_tmpBD7.Rel_n).f1=0,_tmpBD7)))),((_tmpBD6->f2=(*_tmp81B).f2,
_tmpBD6))))));}(void*)(_tmp80C->sc=(void*)((void*)2));}nv=Cyc_Toc_add_varmap(r,
nv,_tmp80C->name,Cyc_Absyn_var_exp(_tmp81B,0));_tmp80C->name=_tmp81B;Cyc_Toc_fndecl_to_c(
nv,_tmp80C,cinclude);{struct Cyc_List_List*_tmpBD8;Cyc_Toc_result_decls=((_tmpBD8=
_cycalloc(sizeof(*_tmpBD8)),((_tmpBD8->hd=d,((_tmpBD8->tl=Cyc_Toc_result_decls,
_tmpBD8))))));}goto _LL435;}_LL43A: if(*((int*)_tmp80A)!= 2)goto _LL43C;_LL43B: goto
_LL43D;_LL43C: if(*((int*)_tmp80A)!= 3)goto _LL43E;_LL43D: {const char*_tmpBDB;void*
_tmpBDA;(_tmpBDA=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Toc_toc_impos)(((
_tmpBDB="letdecl at toplevel",_tag_dyneither(_tmpBDB,sizeof(char),20))),
_tag_dyneither(_tmpBDA,sizeof(void*),0)));}_LL43E: if(*((int*)_tmp80A)!= 4)goto
_LL440;_tmp80D=((struct Cyc_Absyn_Aggr_d_struct*)_tmp80A)->f1;_LL43F: Cyc_Toc_aggrdecl_to_c(
_tmp80D);{struct Cyc_List_List*_tmpBDC;Cyc_Toc_result_decls=((_tmpBDC=_cycalloc(
sizeof(*_tmpBDC)),((_tmpBDC->hd=d,((_tmpBDC->tl=Cyc_Toc_result_decls,_tmpBDC))))));}
goto _LL435;_LL440: if(*((int*)_tmp80A)!= 5)goto _LL442;_tmp80E=((struct Cyc_Absyn_Tunion_d_struct*)
_tmp80A)->f1;_LL441: if(_tmp80E->is_xtunion)Cyc_Toc_xtuniondecl_to_c(_tmp80E);
else{Cyc_Toc_tuniondecl_to_c(_tmp80E);}goto _LL435;_LL442: if(*((int*)_tmp80A)!= 6)
goto _LL444;_tmp80F=((struct Cyc_Absyn_Enum_d_struct*)_tmp80A)->f1;_LL443: Cyc_Toc_enumdecl_to_c(
nv,_tmp80F);{struct Cyc_List_List*_tmpBDD;Cyc_Toc_result_decls=((_tmpBDD=
_cycalloc(sizeof(*_tmpBDD)),((_tmpBDD->hd=d,((_tmpBDD->tl=Cyc_Toc_result_decls,
_tmpBDD))))));}goto _LL435;_LL444: if(*((int*)_tmp80A)!= 7)goto _LL446;_tmp810=((
struct Cyc_Absyn_Typedef_d_struct*)_tmp80A)->f1;_LL445: _tmp810->name=_tmp810->name;
_tmp810->tvs=0;if(_tmp810->defn != 0){struct Cyc_Core_Opt*_tmpBDE;_tmp810->defn=((
_tmpBDE=_cycalloc(sizeof(*_tmpBDE)),((_tmpBDE->v=(void*)Cyc_Toc_typ_to_c_array((
void*)((struct Cyc_Core_Opt*)_check_null(_tmp810->defn))->v),_tmpBDE))));}else{
void*_tmp824=(void*)((struct Cyc_Core_Opt*)_check_null(_tmp810->kind))->v;_LL453:
if((int)_tmp824 != 2)goto _LL455;_LL454:{struct Cyc_Core_Opt*_tmpBDF;_tmp810->defn=((
_tmpBDF=_cycalloc(sizeof(*_tmpBDF)),((_tmpBDF->v=(void*)Cyc_Absyn_void_star_typ(),
_tmpBDF))));}goto _LL452;_LL455:;_LL456:{struct Cyc_Core_Opt*_tmpBE0;_tmp810->defn=((
_tmpBE0=_cycalloc(sizeof(*_tmpBE0)),((_tmpBE0->v=(void*)((void*)0),_tmpBE0))));}
goto _LL452;_LL452:;}{struct Cyc_List_List*_tmpBE1;Cyc_Toc_result_decls=((_tmpBE1=
_cycalloc(sizeof(*_tmpBE1)),((_tmpBE1->hd=d,((_tmpBE1->tl=Cyc_Toc_result_decls,
_tmpBE1))))));}goto _LL435;_LL446: if((int)_tmp80A != 0)goto _LL448;_LL447: goto
_LL449;_LL448: if((int)_tmp80A != 1)goto _LL44A;_LL449: goto _LL435;_LL44A: if(_tmp80A
<= (void*)2)goto _LL44C;if(*((int*)_tmp80A)!= 8)goto _LL44C;_tmp811=((struct Cyc_Absyn_Namespace_d_struct*)
_tmp80A)->f2;_LL44B: _tmp812=_tmp811;goto _LL44D;_LL44C: if(_tmp80A <= (void*)2)goto
_LL44E;if(*((int*)_tmp80A)!= 9)goto _LL44E;_tmp812=((struct Cyc_Absyn_Using_d_struct*)
_tmp80A)->f2;_LL44D: _tmp813=_tmp812;goto _LL44F;_LL44E: if(_tmp80A <= (void*)2)goto
_LL450;if(*((int*)_tmp80A)!= 10)goto _LL450;_tmp813=((struct Cyc_Absyn_ExternC_d_struct*)
_tmp80A)->f1;_LL44F: nv=Cyc_Toc_decls_to_c(r,nv,_tmp813,top,cinclude);goto _LL435;
_LL450: if(_tmp80A <= (void*)2)goto _LL435;if(*((int*)_tmp80A)!= 11)goto _LL435;
_tmp814=((struct Cyc_Absyn_ExternCinclude_d_struct*)_tmp80A)->f1;_LL451: nv=Cyc_Toc_decls_to_c(
r,nv,_tmp814,top,1);goto _LL435;_LL435:;}}return nv;}static void Cyc_Toc_init();
static void Cyc_Toc_init(){struct _DynRegionHandle*_tmp829;struct Cyc_Core_NewRegion
_tmp828=Cyc_Core_new_dynregion();_tmp829=_tmp828.dynregion;{struct
_DynRegionFrame _tmp82A;struct _RegionHandle*d=_open_dynregion(& _tmp82A,_tmp829);{
struct Cyc_List_List**_tmpBEC;struct Cyc_Dict_Dict*_tmpBEB;struct Cyc_Set_Set**
_tmpBEA;struct Cyc_Dict_Dict*_tmpBE9;struct Cyc_Dict_Dict*_tmpBE8;struct Cyc_Toc_TocState*
_tmpBE7;Cyc_Toc_toc_state=((_tmpBE7=_cycalloc(sizeof(*_tmpBE7)),((_tmpBE7->dyn=(
struct _DynRegionHandle*)_tmp829,((_tmpBE7->tuple_types=(struct Cyc_List_List**)((
_tmpBEC=_region_malloc(d,sizeof(*_tmpBEC)),((_tmpBEC[0]=0,_tmpBEC)))),((_tmpBE7->aggrs_so_far=(
struct Cyc_Dict_Dict*)((_tmpBEB=_region_malloc(d,sizeof(*_tmpBEB)),((_tmpBEB[0]=((
struct Cyc_Dict_Dict(*)(struct _RegionHandle*,int(*cmp)(struct _tuple1*,struct
_tuple1*)))Cyc_Dict_rempty)(d,Cyc_Absyn_qvar_cmp),_tmpBEB)))),((_tmpBE7->tunions_so_far=(
struct Cyc_Set_Set**)((_tmpBEA=_region_malloc(d,sizeof(*_tmpBEA)),((_tmpBEA[0]=((
struct Cyc_Set_Set*(*)(struct _RegionHandle*r,int(*cmp)(struct _tuple1*,struct
_tuple1*)))Cyc_Set_rempty)(d,Cyc_Absyn_qvar_cmp),_tmpBEA)))),((_tmpBE7->xtunions_so_far=(
struct Cyc_Dict_Dict*)((_tmpBE9=_region_malloc(d,sizeof(*_tmpBE9)),((_tmpBE9[0]=((
struct Cyc_Dict_Dict(*)(struct _RegionHandle*,int(*cmp)(struct _tuple1*,struct
_tuple1*)))Cyc_Dict_rempty)(d,Cyc_Absyn_qvar_cmp),_tmpBE9)))),((_tmpBE7->qvar_tags=(
struct Cyc_Dict_Dict*)((_tmpBE8=_region_malloc(d,sizeof(*_tmpBE8)),((_tmpBE8[0]=((
struct Cyc_Dict_Dict(*)(struct _RegionHandle*,int(*cmp)(struct _tuple6*,struct
_tuple6*)))Cyc_Dict_rempty)(d,Cyc_Toc_qvar_tag_cmp),_tmpBE8)))),((_tmpBE7->temp_labels=(
struct Cyc_Xarray_Xarray*)((struct Cyc_Xarray_Xarray*(*)(struct _RegionHandle*))Cyc_Xarray_rcreate_empty)(
d),_tmpBE7))))))))))))))));};_pop_dynregion(d);}Cyc_Toc_result_decls=0;Cyc_Toc_tuple_type_counter=
0;Cyc_Toc_temp_var_counter=0;Cyc_Toc_fresh_label_counter=0;Cyc_Toc_total_bounds_checks=
0;Cyc_Toc_bounds_checks_eliminated=0;{struct _dyneither_ptr**_tmpBED;Cyc_Toc_globals=
_tag_dyneither(((_tmpBED=_cycalloc(sizeof(struct _dyneither_ptr*)* 38),((_tmpBED[
0]=& Cyc_Toc__throw_str,((_tmpBED[1]=& Cyc_Toc_setjmp_str,((_tmpBED[2]=& Cyc_Toc__push_handler_str,((
_tmpBED[3]=& Cyc_Toc__pop_handler_str,((_tmpBED[4]=& Cyc_Toc__exn_thrown_str,((
_tmpBED[5]=& Cyc_Toc__npop_handler_str,((_tmpBED[6]=& Cyc_Toc__check_null_str,((
_tmpBED[7]=& Cyc_Toc__check_known_subscript_null_str,((_tmpBED[8]=& Cyc_Toc__check_known_subscript_notnull_str,((
_tmpBED[9]=& Cyc_Toc__check_dyneither_subscript_str,((_tmpBED[10]=& Cyc_Toc__dyneither_ptr_str,((
_tmpBED[11]=& Cyc_Toc__tag_dyneither_str,((_tmpBED[12]=& Cyc_Toc__init_dyneither_ptr_str,((
_tmpBED[13]=& Cyc_Toc__untag_dyneither_ptr_str,((_tmpBED[14]=& Cyc_Toc__get_dyneither_size_str,((
_tmpBED[15]=& Cyc_Toc__get_zero_arr_size_str,((_tmpBED[16]=& Cyc_Toc__dyneither_ptr_plus_str,((
_tmpBED[17]=& Cyc_Toc__zero_arr_plus_str,((_tmpBED[18]=& Cyc_Toc__dyneither_ptr_inplace_plus_str,((
_tmpBED[19]=& Cyc_Toc__zero_arr_inplace_plus_str,((_tmpBED[20]=& Cyc_Toc__dyneither_ptr_inplace_plus_post_str,((
_tmpBED[21]=& Cyc_Toc__zero_arr_inplace_plus_post_str,((_tmpBED[22]=& Cyc_Toc__cycalloc_str,((
_tmpBED[23]=& Cyc_Toc__cyccalloc_str,((_tmpBED[24]=& Cyc_Toc__cycalloc_atomic_str,((
_tmpBED[25]=& Cyc_Toc__cyccalloc_atomic_str,((_tmpBED[26]=& Cyc_Toc__region_malloc_str,((
_tmpBED[27]=& Cyc_Toc__region_calloc_str,((_tmpBED[28]=& Cyc_Toc__check_times_str,((
_tmpBED[29]=& Cyc_Toc__new_region_str,((_tmpBED[30]=& Cyc_Toc__push_region_str,((
_tmpBED[31]=& Cyc_Toc__pop_region_str,((_tmpBED[32]=& Cyc_Toc__open_dynregion_str,((
_tmpBED[33]=& Cyc_Toc__push_dynregion_str,((_tmpBED[34]=& Cyc_Toc__pop_dynregion_str,((
_tmpBED[35]=& Cyc_Toc__reset_region_str,((_tmpBED[36]=& Cyc_Toc__throw_arraybounds_str,((
_tmpBED[37]=& Cyc_Toc__dyneither_ptr_decrease_size_str,_tmpBED)))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))),
sizeof(struct _dyneither_ptr*),38);}}struct Cyc_List_List*Cyc_Toc_toc(struct Cyc_List_List*
ds);struct Cyc_List_List*Cyc_Toc_toc(struct Cyc_List_List*ds){Cyc_Toc_init();{
struct _RegionHandle _tmp832=_new_region("start");struct _RegionHandle*start=&
_tmp832;_push_region(start);Cyc_Toc_decls_to_c(start,Cyc_Toc_empty_env(start),ds,
1,0);{struct _DynRegionHandle*_tmp834;struct Cyc_Toc_TocState _tmp833=*((struct Cyc_Toc_TocState*)
_check_null(Cyc_Toc_toc_state));_tmp834=_tmp833.dyn;Cyc_Core_free_dynregion(
_tmp834);};_pop_region(start);}return((struct Cyc_List_List*(*)(struct Cyc_List_List*
x))Cyc_List_imp_rev)(Cyc_Toc_result_decls);}