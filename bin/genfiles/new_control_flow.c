#include <setjmp.h>
/* This is a C header file to be used by the output of the Cyclone to
   C translator.  The corresponding definitions are in file lib/runtime_cyc.c */
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

/***********************************************************************/
/* Low-level representations etc.                                      */
/***********************************************************************/

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
struct _RegionPage
#ifdef CYC_REGION_PROFILE
{ unsigned total_bytes;
  unsigned free_bytes;
  /* MWH: wish we didn't have to include the stuff below ... */
  struct _RegionPage *next;
  char data[1];
}
#endif
; // abstract -- defined in runtime_memory.c

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
extern void* _throw_null_fn(const char *filename, unsigned lineno);
extern void* _throw_arraybounds_fn(const char *filename, unsigned lineno);
extern void* _throw_badalloc_fn(const char *filename, unsigned lineno);
extern void* _throw_match_fn(const char *filename, unsigned lineno);
extern void* _throw_fn(void* e, const char *filename, unsigned lineno);
extern void* _rethrow(void* e);
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
#ifdef CYC_ANSI_OUTPUT
#define _INLINE  
#else
#define _INLINE inline
#endif

#ifdef NO_CYC_NULL_CHECKS
#define _check_null(ptr) (ptr)
#else
#define _check_null(ptr) \
  ({ void*_cks_null = (void*)(ptr); \
     if (!_cks_null) _throw_null(); \
     _cks_null; })
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_known_subscript_null(ptr,bound,elt_sz,index)\
   ((char *)ptr) + (elt_sz)*(index))
#define _check_known_subscript_notnull(bound,index) (index)
#define _check_known_subscript_notnullX(bound,index)\
   ((char *)ptr) + (elt_sz)*(index))

#define _zero_arr_plus_char_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_short_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_int_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_float_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_double_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_longdouble_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_voidstar_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#else
#define _check_known_subscript_null(ptr,bound,elt_sz,index) ({ \
  char*_cks_ptr = (char*)(ptr); \
  unsigned _cks_bound = (bound); \
  unsigned _cks_elt_sz = (elt_sz); \
  unsigned _cks_index = (index); \
  if (!_cks_ptr) _throw_null(); \
  if (_cks_index >= _cks_bound) _throw_arraybounds(); \
  (_cks_ptr) + _cks_elt_sz*_cks_index; })

#define _check_known_subscript_notnull(bound,index) ({ \
  unsigned _cksnn_bound = (bound); \
  unsigned _cksnn_index = (index); \
  if (_cksnn_index >= _cksnn_bound) _throw_arraybounds(); \
  _cksnn_index; })
#define _check_known_subscript_nullX(ptr,bound,elt_sz,index) ({ \
  char*_cks_ptr = (char*)(ptr); \
  unsigned _cks_bound = (bound); \
  unsigned _cks_elt_sz = (elt_sz); \
  unsigned _cks_index = (index); \
  if (_cks_index >= _cks_bound) _throw_arraybounds(); \
  (_cks_ptr) + _cks_elt_sz*_cks_index; })

/* Add i to zero-terminated pointer x.  Checks for x being null and
   ensures that x[0..i-1] are not 0. */
char * _zero_arr_plus_char_fn(char *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno);
short * _zero_arr_plus_short_fn(short *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno);
int * _zero_arr_plus_int_fn(int *orig_x, unsigned int orig_sz, int orig_i, const char *filename, unsigned lineno);
float * _zero_arr_plus_float_fn(float *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno);
double * _zero_arr_plus_double_fn(double *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno);
long double * _zero_arr_plus_longdouble_fn(long double *orig_x, unsigned int orig_sz, int orig_i, const char *filename, unsigned lineno);
void * _zero_arr_plus_voidstar_fn(void **orig_x, unsigned int orig_sz, int orig_i,const char *filename,unsigned lineno);
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
int _get_zero_arr_size_char(const char *orig_x, unsigned int orig_offset);
int _get_zero_arr_size_short(const short *orig_x, unsigned int orig_offset);
int _get_zero_arr_size_int(const int *orig_x, unsigned int orig_offset);
int _get_zero_arr_size_float(const float *orig_x, unsigned int orig_offset);
int _get_zero_arr_size_double(const double *orig_x, unsigned int orig_offset);
int _get_zero_arr_size_longdouble(const long double *orig_x, unsigned int orig_offset);
int _get_zero_arr_size_voidstar(const void **orig_x, unsigned int orig_offset);

/* Does in-place addition of a zero-terminated pointer (x += e and ++x).  
   Note that this expands to call _zero_arr_plus_<type>_fn. */
char * _zero_arr_inplace_plus_char_fn(char **x, int orig_i,const char *filename,unsigned lineno);
short * _zero_arr_inplace_plus_short_fn(short **x, int orig_i,const char *filename,unsigned lineno);
int * _zero_arr_inplace_plus_int(int **x, int orig_i,const char *filename,unsigned lineno);
float * _zero_arr_inplace_plus_float_fn(float **x, int orig_i,const char *filename,unsigned lineno);
double * _zero_arr_inplace_plus_double_fn(double **x, int orig_i,const char *filename,unsigned lineno);
long double * _zero_arr_inplace_plus_longdouble_fn(long double **x, int orig_i,const char *filename,unsigned lineno);
void * _zero_arr_inplace_plus_voidstar_fn(void ***x, int orig_i,const char *filename,unsigned lineno);
#define _zero_arr_inplace_plus_char(x,i) \
  _zero_arr_inplace_plus_char_fn((char **)(x),i,__FILE__,__LINE__)
#define _zero_arr_inplace_plus_short(x,i) \
  _zero_arr_inplace_plus_short_fn((short **)(x),i,__FILE__,__LINE__)
#define _zero_arr_inplace_plus_int(x,i) \
  _zero_arr_inplace_plus_int_fn((int **)(x),i,__FILE__,__LINE__)
#define _zero_arr_inplace_plus_float(x,i) \
  _zero_arr_inplace_plus_float_fn((float **)(x),i,__FILE__,__LINE__)
#define _zero_arr_inplace_plus_double(x,i) \
  _zero_arr_inplace_plus_double_fn((double **)(x),i,__FILE__,__LINE__)
#define _zero_arr_inplace_plus_longdouble(x,i) \
  _zero_arr_inplace_plus_longdouble_fn((long double **)(x),i,__FILE__,__LINE__)
#define _zero_arr_inplace_plus_voidstar(x,i) \
  _zero_arr_inplace_plus_voidstar_fn((void ***)(x),i,__FILE__,__LINE__)

/* Does in-place increment of a zero-terminated pointer (e.g., x++). */
char * _zero_arr_inplace_plus_post_char_fn(char **x, int orig_i,const char *filename,unsigned lineno);
short * _zero_arr_inplace_plus_post_short_fn(short **x, int orig_i,const char *filename,unsigned lineno);
int * _zero_arr_inplace_plus_post_int_fn(int **x, int orig_i,const char *filename, unsigned lineno);
float * _zero_arr_inplace_plus_post_float_fn(float **x, int orig_i,const char *filename, unsigned lineno);
double * _zero_arr_inplace_plus_post_double_fn(double **x, int orig_i,const char *filename,unsigned lineno);
long double * _zero_arr_inplace_plus_post_longdouble_fn(long double **x, int orig_i,const char *filename,unsigned lineno);
void ** _zero_arr_inplace_plus_post_voidstar_fn(void ***x, int orig_i,const char *filename,unsigned lineno);
#define _zero_arr_inplace_plus_post_char(x,i) \
  _zero_arr_inplace_plus_post_char_fn((char **)(x),(i),__FILE__,__LINE__)
#define _zero_arr_inplace_plus_post_short(x,i) \
  _zero_arr_inplace_plus_post_short_fn((short **)(x),(i),__FILE__,__LINE__)
#define _zero_arr_inplace_plus_post_int(x,i) \
  _zero_arr_inplace_plus_post_int_fn((int **)(x),(i),__FILE__,__LINE__)
#define _zero_arr_inplace_plus_post_float(x,i) \
  _zero_arr_inplace_plus_post_float_fn((float **)(x),(i),__FILE__,__LINE__)
#define _zero_arr_inplace_plus_post_double(x,i) \
  _zero_arr_inplace_plus_post_double_fn((double **)(x),(i),__FILE__,__LINE__)
#define _zero_arr_inplace_plus_post_longdouble(x,i) \
  _zero_arr_inplace_plus_post_longdouble_fn((long double **)(x),(i),__FILE__,__LINE__)
#define _zero_arr_inplace_plus_post_voidstar(x,i) \
  _zero_arr_inplace_plus_post_voidstar_fn((void***)(x),(i),__FILE__,__LINE__)

/* functions for dealing with dynamically sized pointers */
#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_dyneither_subscript(arr,elt_sz,index) ({ \
  struct _dyneither_ptr _cus_arr = (arr); \
  unsigned _cus_elt_sz = (elt_sz); \
  unsigned _cus_index = (index); \
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index; \
  _cus_ans; })
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

#define _tag_dyneither(tcurr,elt_sz,num_elts) ({ \
  struct _dyneither_ptr _tag_arr_ans; \
  _tag_arr_ans.base = _tag_arr_ans.curr = (void*)(tcurr); \
  /* JGM: if we're tagging NULL, ignore num_elts */ \
  _tag_arr_ans.last_plus_one = _tag_arr_ans.base ? (_tag_arr_ans.base + (elt_sz) * (num_elts)) : 0; \
  _tag_arr_ans; })

#ifdef NO_CYC_BOUNDS_CHECKS
#define _untag_dyneither_ptr(arr,elt_sz,num_elts) ((arr).curr)
#else
#define _untag_dyneither_ptr(arr,elt_sz,num_elts) ({ \
  struct _dyneither_ptr _arr = (arr); \
  unsigned char *_curr = _arr.curr; \
  if ((_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one) &&\
      _curr != (unsigned char *)0) \
    _throw_arraybounds(); \
  _curr; })
#endif

#define _get_dyneither_size(arr,elt_sz) \
  ({struct _dyneither_ptr _get_arr_size_temp = (arr); \
    unsigned char *_get_arr_size_curr=_get_arr_size_temp.curr; \
    unsigned char *_get_arr_size_last=_get_arr_size_temp.last_plus_one; \
    (_get_arr_size_curr < _get_arr_size_temp.base || \
     _get_arr_size_curr >= _get_arr_size_last) ? 0 : \
    ((_get_arr_size_last - _get_arr_size_curr) / (elt_sz));})

#define _dyneither_ptr_plus(arr,elt_sz,change) ({ \
  struct _dyneither_ptr _ans = (arr); \
  _ans.curr += ((int)(elt_sz))*(change); \
  _ans; })

#define _dyneither_ptr_inplace_plus(arr_ptr,elt_sz,change) ({ \
  struct _dyneither_ptr * _arr_ptr = (arr_ptr); \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  *_arr_ptr; })

#define _dyneither_ptr_inplace_plus_post(arr_ptr,elt_sz,change) ({ \
  struct _dyneither_ptr * _arr_ptr = (arr_ptr); \
  struct _dyneither_ptr _ans = *_arr_ptr; \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  _ans; })

/* Decrease the upper bound on a fat pointer by numelts where sz is
   the size of the pointer's type.  Note that this can't be a macro
   if we're to get initializers right. */
static struct
 _dyneither_ptr _dyneither_ptr_decrease_size(struct _dyneither_ptr x,
                                            unsigned int sz,
                                            unsigned int numelts) {
  unsigned delta = sz * numelts;
  /* Don't let someone decrease the size so much that it wraps around.
   * This is crucial to get NULL right. */
  if (x.last_plus_one - x.base >= delta)
    x.last_plus_one -= delta;
  else x.last_plus_one = x.base;
  return x; 
}

/* Allocation */
extern void* GC_malloc(int);
extern void* GC_malloc_atomic(int);
extern void* GC_calloc(unsigned,unsigned);
extern void* GC_calloc_atomic(unsigned,unsigned);
/* bound the allocation size to be less than MAX_ALLOC_SIZE,
   which is defined in runtime_memory.c
*/
extern void* _bounded_GC_malloc(int,const char *file,int lineno);
extern void* _bounded_GC_malloc_atomic(int,const char *file,int lineno);
extern void* _bounded_GC_calloc(unsigned n, unsigned s,
                                const char *file,int lineno);
extern void* _bounded_GC_calloc_atomic(unsigned n, unsigned s,
                                       const char *file,
                                       int lineno);
/* FIX?  Not sure if we want to pass filename and lineno in here... */
#ifndef CYC_REGION_PROFILE
#define _cycalloc(n) _bounded_GC_malloc(n,__FILE__,__LINE__)
#define _cycalloc_atomic(n) _bounded_GC_malloc_atomic(n,__FILE__,__LINE__)
#define _cyccalloc(n,s) _bounded_GC_calloc(n,s,__FILE__,__LINE__)
#define _cyccalloc_atomic(n,s) _bounded_GC_calloc_atomic(n,s,__FILE__,__LINE__)
#endif

#define MAX_MALLOC_SIZE (1 << 28)
static _INLINE unsigned int _check_times(unsigned x, unsigned y) {
  unsigned long long whole_ans = 
    ((unsigned long long) x)*((unsigned long long)y);
  unsigned word_ans = (unsigned)whole_ans;
  if(word_ans < whole_ans || word_ans > MAX_MALLOC_SIZE)
    _throw_badalloc();
  return word_ans;
}

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

#if defined(CYC_REGION_PROFILE) 
extern void* _profile_GC_malloc(int,const char *file,const char *func,
                                int lineno);
extern void* _profile_GC_malloc_atomic(int,const char *file,
                                       const char *func,int lineno);
extern void* _profile_GC_calloc(unsigned n, unsigned s,
                                const char *file, const char *func, int lineno);
extern void* _profile_GC_calloc_atomic(unsigned n, unsigned s,
                                       const char *file, const char *func,
                                       int lineno);
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
#define _cyccalloc(n,s) _profile_GC_calloc(n,s,__FILE__,__FUNCTION__,__LINE__)
#define _cyccalloc_atomic(n,s) _profile_GC_calloc_atomic(n,s,__FILE__,__FUNCTION__,__LINE__)
#endif
#endif
 struct Cyc___cycFILE;
# 53 "cycboot.h"
extern struct Cyc___cycFILE*Cyc_stderr;struct Cyc_String_pa_PrintArg_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Int_pa_PrintArg_struct{int tag;unsigned long f1;};struct Cyc_Double_pa_PrintArg_struct{int tag;double f1;};struct Cyc_LongDouble_pa_PrintArg_struct{int tag;long double f1;};struct Cyc_ShortPtr_pa_PrintArg_struct{int tag;short*f1;};struct Cyc_IntPtr_pa_PrintArg_struct{int tag;unsigned long*f1;};
# 100
int Cyc_fprintf(struct Cyc___cycFILE*,struct _dyneither_ptr,struct _dyneither_ptr);struct Cyc_ShortPtr_sa_ScanfArg_struct{int tag;short*f1;};struct Cyc_UShortPtr_sa_ScanfArg_struct{int tag;unsigned short*f1;};struct Cyc_IntPtr_sa_ScanfArg_struct{int tag;int*f1;};struct Cyc_UIntPtr_sa_ScanfArg_struct{int tag;unsigned int*f1;};struct Cyc_StringPtr_sa_ScanfArg_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_DoublePtr_sa_ScanfArg_struct{int tag;double*f1;};struct Cyc_FloatPtr_sa_ScanfArg_struct{int tag;float*f1;};struct Cyc_CharPtr_sa_ScanfArg_struct{int tag;struct _dyneither_ptr f1;};extern char Cyc_FileCloseError[15U];struct Cyc_FileCloseError_exn_struct{char*tag;};extern char Cyc_FileOpenError[14U];struct Cyc_FileOpenError_exn_struct{char*tag;struct _dyneither_ptr f1;};struct Cyc_Core_Opt{void*v;};struct _tuple0{void*f1;void*f2;};
# 113 "core.h"
void*Cyc_Core_snd(struct _tuple0*);
# 128
int Cyc_Core_ptrcmp(void*,void*);extern char Cyc_Core_Invalid_argument[17U];struct Cyc_Core_Invalid_argument_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Failure[8U];struct Cyc_Core_Failure_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Impossible[11U];struct Cyc_Core_Impossible_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Not_found[10U];struct Cyc_Core_Not_found_exn_struct{char*tag;};extern char Cyc_Core_Unreachable[12U];struct Cyc_Core_Unreachable_exn_struct{char*tag;struct _dyneither_ptr f1;};
# 167
extern struct _RegionHandle*Cyc_Core_heap_region;
# 170
extern struct _RegionHandle*Cyc_Core_unique_region;struct Cyc_Core_DynamicRegion;struct Cyc_Core_NewDynamicRegion{struct Cyc_Core_DynamicRegion*key;};
# 277 "core.h"
void Cyc_Core_rethrow(void*);struct Cyc_List_List{void*hd;struct Cyc_List_List*tl;};
# 54 "list.h"
struct Cyc_List_List*Cyc_List_list(struct _dyneither_ptr);
# 61
int Cyc_List_length(struct Cyc_List_List*x);
# 70
struct Cyc_List_List*Cyc_List_copy(struct Cyc_List_List*x);
# 76
struct Cyc_List_List*Cyc_List_map(void*(*f)(void*),struct Cyc_List_List*x);extern char Cyc_List_List_mismatch[14U];struct Cyc_List_List_mismatch_exn_struct{char*tag;};
# 135
void Cyc_List_iter_c(void(*f)(void*,void*),void*env,struct Cyc_List_List*x);
# 172
struct Cyc_List_List*Cyc_List_rev(struct Cyc_List_List*x);
# 178
struct Cyc_List_List*Cyc_List_imp_rev(struct Cyc_List_List*x);
# 184
struct Cyc_List_List*Cyc_List_append(struct Cyc_List_List*x,struct Cyc_List_List*y);extern char Cyc_List_Nth[4U];struct Cyc_List_Nth_exn_struct{char*tag;};
# 242
void*Cyc_List_nth(struct Cyc_List_List*x,int n);
# 276
struct Cyc_List_List*Cyc_List_rzip(struct _RegionHandle*r1,struct _RegionHandle*r2,struct Cyc_List_List*x,struct Cyc_List_List*y);struct _tuple1{struct Cyc_List_List*f1;struct Cyc_List_List*f2;};
# 294
struct _tuple1 Cyc_List_split(struct Cyc_List_List*x);
# 319
int Cyc_List_memq(struct Cyc_List_List*l,void*x);struct Cyc_Iter_Iter{void*env;int(*next)(void*env,void*dest);};
# 37 "iter.h"
int Cyc_Iter_next(struct Cyc_Iter_Iter,void*);struct Cyc_Set_Set;extern char Cyc_Set_Absent[7U];struct Cyc_Set_Absent_exn_struct{char*tag;};struct Cyc_Dict_T;struct Cyc_Dict_Dict{int(*rel)(void*,void*);struct _RegionHandle*r;const struct Cyc_Dict_T*t;};extern char Cyc_Dict_Present[8U];struct Cyc_Dict_Present_exn_struct{char*tag;};extern char Cyc_Dict_Absent[7U];struct Cyc_Dict_Absent_exn_struct{char*tag;};
# 87 "dict.h"
struct Cyc_Dict_Dict Cyc_Dict_insert(struct Cyc_Dict_Dict d,void*k,void*v);
# 113
void*Cyc_Dict_lookup_other(struct Cyc_Dict_Dict d,int(*cmp)(void*,void*),void*k);
# 126 "dict.h"
int Cyc_Dict_lookup_bool(struct Cyc_Dict_Dict d,void*k,void**ans);
# 149
void Cyc_Dict_iter_c(void(*f)(void*,void*,void*),void*env,struct Cyc_Dict_Dict d);
# 37 "position.h"
struct _dyneither_ptr Cyc_Position_string_of_segment(unsigned int);struct Cyc_Position_Error;
# 47
extern int Cyc_Position_num_errors;struct Cyc_Relations_Reln;struct _union_Nmspace_Rel_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Abs_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_C_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Loc_n{int tag;int val;};union Cyc_Absyn_Nmspace{struct _union_Nmspace_Rel_n Rel_n;struct _union_Nmspace_Abs_n Abs_n;struct _union_Nmspace_C_n C_n;struct _union_Nmspace_Loc_n Loc_n;};
# 96 "absyn.h"
union Cyc_Absyn_Nmspace Cyc_Absyn_Loc_n;
union Cyc_Absyn_Nmspace Cyc_Absyn_Rel_n(struct Cyc_List_List*);
# 99
union Cyc_Absyn_Nmspace Cyc_Absyn_Abs_n(struct Cyc_List_List*ns,int C_scope);struct _tuple2{union Cyc_Absyn_Nmspace f1;struct _dyneither_ptr*f2;};
# 159
enum Cyc_Absyn_Scope{Cyc_Absyn_Static  = 0U,Cyc_Absyn_Abstract  = 1U,Cyc_Absyn_Public  = 2U,Cyc_Absyn_Extern  = 3U,Cyc_Absyn_ExternC  = 4U,Cyc_Absyn_Register  = 5U};struct Cyc_Absyn_Tqual{int print_const: 1;int q_volatile: 1;int q_restrict: 1;int real_const: 1;unsigned int loc;};
# 180
enum Cyc_Absyn_Size_of{Cyc_Absyn_Char_sz  = 0U,Cyc_Absyn_Short_sz  = 1U,Cyc_Absyn_Int_sz  = 2U,Cyc_Absyn_Long_sz  = 3U,Cyc_Absyn_LongLong_sz  = 4U};
# 185
enum Cyc_Absyn_AliasQual{Cyc_Absyn_Aliasable  = 0U,Cyc_Absyn_Unique  = 1U,Cyc_Absyn_Top  = 2U};
# 191
enum Cyc_Absyn_KindQual{Cyc_Absyn_AnyKind  = 0U,Cyc_Absyn_MemKind  = 1U,Cyc_Absyn_BoxKind  = 2U,Cyc_Absyn_RgnKind  = 3U,Cyc_Absyn_EffKind  = 4U,Cyc_Absyn_IntKind  = 5U,Cyc_Absyn_BoolKind  = 6U,Cyc_Absyn_PtrBndKind  = 7U};struct Cyc_Absyn_Kind{enum Cyc_Absyn_KindQual kind;enum Cyc_Absyn_AliasQual aliasqual;};
# 213
enum Cyc_Absyn_Sign{Cyc_Absyn_Signed  = 0U,Cyc_Absyn_Unsigned  = 1U,Cyc_Absyn_None  = 2U};
# 215
enum Cyc_Absyn_AggrKind{Cyc_Absyn_StructA  = 0U,Cyc_Absyn_UnionA  = 1U};struct Cyc_Absyn_Eq_kb_Absyn_KindBound_struct{int tag;struct Cyc_Absyn_Kind*f1;};struct Cyc_Absyn_Unknown_kb_Absyn_KindBound_struct{int tag;struct Cyc_Core_Opt*f1;};struct Cyc_Absyn_Less_kb_Absyn_KindBound_struct{int tag;struct Cyc_Core_Opt*f1;struct Cyc_Absyn_Kind*f2;};struct Cyc_Absyn_Tvar{struct _dyneither_ptr*name;int identity;void*kind;};struct Cyc_Absyn_PtrLoc{unsigned int ptr_loc;unsigned int rgn_loc;unsigned int zt_loc;};struct Cyc_Absyn_PtrAtts{void*rgn;void*nullable;void*bounds;void*zero_term;struct Cyc_Absyn_PtrLoc*ptrloc;};struct Cyc_Absyn_PtrInfo{void*elt_type;struct Cyc_Absyn_Tqual elt_tq;struct Cyc_Absyn_PtrAtts ptr_atts;};struct Cyc_Absyn_VarargInfo{struct _dyneither_ptr*name;struct Cyc_Absyn_Tqual tq;void*type;int inject;};struct Cyc_Absyn_FnInfo{struct Cyc_List_List*tvars;void*effect;struct Cyc_Absyn_Tqual ret_tqual;void*ret_type;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;struct Cyc_List_List*requires_relns;struct Cyc_Absyn_Exp*ensures_clause;struct Cyc_List_List*ensures_relns;};struct Cyc_Absyn_UnknownDatatypeInfo{struct _tuple2*name;int is_extensible;};struct _union_DatatypeInfo_UnknownDatatype{int tag;struct Cyc_Absyn_UnknownDatatypeInfo val;};struct _union_DatatypeInfo_KnownDatatype{int tag;struct Cyc_Absyn_Datatypedecl**val;};union Cyc_Absyn_DatatypeInfo{struct _union_DatatypeInfo_UnknownDatatype UnknownDatatype;struct _union_DatatypeInfo_KnownDatatype KnownDatatype;};struct Cyc_Absyn_UnknownDatatypeFieldInfo{struct _tuple2*datatype_name;struct _tuple2*field_name;int is_extensible;};struct _union_DatatypeFieldInfo_UnknownDatatypefield{int tag;struct Cyc_Absyn_UnknownDatatypeFieldInfo val;};struct _tuple3{struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;};struct _union_DatatypeFieldInfo_KnownDatatypefield{int tag;struct _tuple3 val;};union Cyc_Absyn_DatatypeFieldInfo{struct _union_DatatypeFieldInfo_UnknownDatatypefield UnknownDatatypefield;struct _union_DatatypeFieldInfo_KnownDatatypefield KnownDatatypefield;};struct _tuple4{enum Cyc_Absyn_AggrKind f1;struct _tuple2*f2;struct Cyc_Core_Opt*f3;};struct _union_AggrInfo_UnknownAggr{int tag;struct _tuple4 val;};struct _union_AggrInfo_KnownAggr{int tag;struct Cyc_Absyn_Aggrdecl**val;};union Cyc_Absyn_AggrInfo{struct _union_AggrInfo_UnknownAggr UnknownAggr;struct _union_AggrInfo_KnownAggr KnownAggr;};struct Cyc_Absyn_ArrayInfo{void*elt_type;struct Cyc_Absyn_Tqual tq;struct Cyc_Absyn_Exp*num_elts;void*zero_term;unsigned int zt_loc;};struct Cyc_Absyn_Aggr_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Absyn_Enum_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;};struct Cyc_Absyn_Datatype_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;};struct Cyc_Absyn_TypeDecl{void*r;unsigned int loc;};struct Cyc_Absyn_VoidCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_IntCon_Absyn_TyCon_struct{int tag;enum Cyc_Absyn_Sign f1;enum Cyc_Absyn_Size_of f2;};struct Cyc_Absyn_FloatCon_Absyn_TyCon_struct{int tag;int f1;};struct Cyc_Absyn_RgnHandleCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_TagCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_HeapCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_UniqueCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_RefCntCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_AccessCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_JoinCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_RgnsCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_TrueCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_FalseCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_ThinCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_FatCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_EnumCon_Absyn_TyCon_struct{int tag;struct _tuple2*f1;struct Cyc_Absyn_Enumdecl*f2;};struct Cyc_Absyn_AnonEnumCon_Absyn_TyCon_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_BuiltinCon_Absyn_TyCon_struct{int tag;struct _dyneither_ptr f1;struct Cyc_Absyn_Kind*f2;};struct Cyc_Absyn_DatatypeCon_Absyn_TyCon_struct{int tag;union Cyc_Absyn_DatatypeInfo f1;};struct Cyc_Absyn_DatatypeFieldCon_Absyn_TyCon_struct{int tag;union Cyc_Absyn_DatatypeFieldInfo f1;};struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct{int tag;union Cyc_Absyn_AggrInfo f1;};struct Cyc_Absyn_AppType_Absyn_Type_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Evar_Absyn_Type_struct{int tag;struct Cyc_Core_Opt*f1;void*f2;int f3;struct Cyc_Core_Opt*f4;};struct Cyc_Absyn_VarType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Tvar*f1;};struct Cyc_Absyn_PointerType_Absyn_Type_struct{int tag;struct Cyc_Absyn_PtrInfo f1;};struct Cyc_Absyn_ArrayType_Absyn_Type_struct{int tag;struct Cyc_Absyn_ArrayInfo f1;};struct Cyc_Absyn_FnType_Absyn_Type_struct{int tag;struct Cyc_Absyn_FnInfo f1;};struct Cyc_Absyn_TupleType_Absyn_Type_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct{int tag;enum Cyc_Absyn_AggrKind f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_TypedefType_Absyn_Type_struct{int tag;struct _tuple2*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_Typedefdecl*f3;void*f4;};struct Cyc_Absyn_ValueofType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_TypeDeclType_Absyn_Type_struct{int tag;struct Cyc_Absyn_TypeDecl*f1;void**f2;};struct Cyc_Absyn_TypeofType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_NoTypes_Absyn_Funcparams_struct{int tag;struct Cyc_List_List*f1;unsigned int f2;};struct Cyc_Absyn_WithTypes_Absyn_Funcparams_struct{int tag;struct Cyc_List_List*f1;int f2;struct Cyc_Absyn_VarargInfo*f3;void*f4;struct Cyc_List_List*f5;struct Cyc_Absyn_Exp*f6;struct Cyc_Absyn_Exp*f7;};
# 427 "absyn.h"
enum Cyc_Absyn_Format_Type{Cyc_Absyn_Printf_ft  = 0U,Cyc_Absyn_Scanf_ft  = 1U};struct Cyc_Absyn_Regparm_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Stdcall_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Cdecl_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Fastcall_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Noreturn_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Const_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Aligned_att_Absyn_Attribute_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Packed_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Section_att_Absyn_Attribute_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Nocommon_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Shared_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Unused_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Weak_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Dllimport_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Dllexport_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_No_instrument_function_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Constructor_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Destructor_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_No_check_memory_usage_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Format_att_Absyn_Attribute_struct{int tag;enum Cyc_Absyn_Format_Type f1;int f2;int f3;};struct Cyc_Absyn_Initializes_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Noliveunique_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Noconsume_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Pure_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Mode_att_Absyn_Attribute_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Alias_att_Absyn_Attribute_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Always_inline_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Carray_mod_Absyn_Type_modifier_struct{int tag;void*f1;unsigned int f2;};struct Cyc_Absyn_ConstArray_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_Absyn_Exp*f1;void*f2;unsigned int f3;};struct Cyc_Absyn_Pointer_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_Absyn_PtrAtts f1;struct Cyc_Absyn_Tqual f2;};struct Cyc_Absyn_Function_mod_Absyn_Type_modifier_struct{int tag;void*f1;};struct Cyc_Absyn_TypeParams_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_List_List*f1;unsigned int f2;int f3;};struct Cyc_Absyn_Attributes_mod_Absyn_Type_modifier_struct{int tag;unsigned int f1;struct Cyc_List_List*f2;};struct _union_Cnst_Null_c{int tag;int val;};struct _tuple5{enum Cyc_Absyn_Sign f1;char f2;};struct _union_Cnst_Char_c{int tag;struct _tuple5 val;};struct _union_Cnst_Wchar_c{int tag;struct _dyneither_ptr val;};struct _tuple6{enum Cyc_Absyn_Sign f1;short f2;};struct _union_Cnst_Short_c{int tag;struct _tuple6 val;};struct _tuple7{enum Cyc_Absyn_Sign f1;int f2;};struct _union_Cnst_Int_c{int tag;struct _tuple7 val;};struct _tuple8{enum Cyc_Absyn_Sign f1;long long f2;};struct _union_Cnst_LongLong_c{int tag;struct _tuple8 val;};struct _tuple9{struct _dyneither_ptr f1;int f2;};struct _union_Cnst_Float_c{int tag;struct _tuple9 val;};struct _union_Cnst_String_c{int tag;struct _dyneither_ptr val;};struct _union_Cnst_Wstring_c{int tag;struct _dyneither_ptr val;};union Cyc_Absyn_Cnst{struct _union_Cnst_Null_c Null_c;struct _union_Cnst_Char_c Char_c;struct _union_Cnst_Wchar_c Wchar_c;struct _union_Cnst_Short_c Short_c;struct _union_Cnst_Int_c Int_c;struct _union_Cnst_LongLong_c LongLong_c;struct _union_Cnst_Float_c Float_c;struct _union_Cnst_String_c String_c;struct _union_Cnst_Wstring_c Wstring_c;};
# 517
enum Cyc_Absyn_Primop{Cyc_Absyn_Plus  = 0U,Cyc_Absyn_Times  = 1U,Cyc_Absyn_Minus  = 2U,Cyc_Absyn_Div  = 3U,Cyc_Absyn_Mod  = 4U,Cyc_Absyn_Eq  = 5U,Cyc_Absyn_Neq  = 6U,Cyc_Absyn_Gt  = 7U,Cyc_Absyn_Lt  = 8U,Cyc_Absyn_Gte  = 9U,Cyc_Absyn_Lte  = 10U,Cyc_Absyn_Not  = 11U,Cyc_Absyn_Bitnot  = 12U,Cyc_Absyn_Bitand  = 13U,Cyc_Absyn_Bitor  = 14U,Cyc_Absyn_Bitxor  = 15U,Cyc_Absyn_Bitlshift  = 16U,Cyc_Absyn_Bitlrshift  = 17U,Cyc_Absyn_Bitarshift  = 18U,Cyc_Absyn_Numelts  = 19U};
# 524
enum Cyc_Absyn_Incrementor{Cyc_Absyn_PreInc  = 0U,Cyc_Absyn_PostInc  = 1U,Cyc_Absyn_PreDec  = 2U,Cyc_Absyn_PostDec  = 3U};struct Cyc_Absyn_VarargCallInfo{int num_varargs;struct Cyc_List_List*injectors;struct Cyc_Absyn_VarargInfo*vai;};struct Cyc_Absyn_StructField_Absyn_OffsetofField_struct{int tag;struct _dyneither_ptr*f1;};struct Cyc_Absyn_TupleIndex_Absyn_OffsetofField_struct{int tag;unsigned int f1;};
# 542
enum Cyc_Absyn_Coercion{Cyc_Absyn_Unknown_coercion  = 0U,Cyc_Absyn_No_coercion  = 1U,Cyc_Absyn_Null_to_NonNull  = 2U,Cyc_Absyn_Other_coercion  = 3U};struct Cyc_Absyn_MallocInfo{int is_calloc;struct Cyc_Absyn_Exp*rgn;void**elt_type;struct Cyc_Absyn_Exp*num_elts;int fat_result;int inline_call;};struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct{int tag;union Cyc_Absyn_Cnst f1;};struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Pragma_e_Absyn_Raw_exp_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct{int tag;enum Cyc_Absyn_Primop f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Increment_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;enum Cyc_Absyn_Incrementor f2;};struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_VarargCallInfo*f3;int f4;};struct Cyc_Absyn_Throw_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;int f2;};struct Cyc_Absyn_NoInstantiate_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Instantiate_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_Absyn_Exp*f2;int f3;enum Cyc_Absyn_Coercion f4;};struct Cyc_Absyn_Address_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_New_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Sizeoftype_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Sizeofexp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Offsetof_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;int f3;int f4;};struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;int f3;int f4;};struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Tuple_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;};struct _tuple10{struct _dyneither_ptr*f1;struct Cyc_Absyn_Tqual f2;void*f3;};struct Cyc_Absyn_CompoundLit_e_Absyn_Raw_exp_struct{int tag;struct _tuple10*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Array_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Comprehension_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;int f4;};struct Cyc_Absyn_ComprehensionNoinit_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;void*f2;int f3;};struct Cyc_Absyn_Aggregate_e_Absyn_Raw_exp_struct{int tag;struct _tuple2*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;struct Cyc_Absyn_Aggrdecl*f4;};struct Cyc_Absyn_AnonStruct_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Datatype_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Datatypedecl*f2;struct Cyc_Absyn_Datatypefield*f3;};struct Cyc_Absyn_Enum_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_AnonEnum_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_MallocInfo f1;};struct Cyc_Absyn_Swap_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Core_Opt*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_StmtExp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Tagcheck_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;};struct Cyc_Absyn_Valueof_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Asm_e_Absyn_Raw_exp_struct{int tag;int f1;struct _dyneither_ptr f2;};struct Cyc_Absyn_Extension_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Exp{void*topt;void*r;unsigned int loc;void*annot;};struct Cyc_Absyn_Skip_s_Absyn_Raw_stmt_struct{int tag;};struct Cyc_Absyn_Exp_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Return_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;struct Cyc_Absyn_Stmt*f3;};struct _tuple11{struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_While_s_Absyn_Raw_stmt_struct{int tag;struct _tuple11 f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Break_s_Absyn_Raw_stmt_struct{int tag;};struct Cyc_Absyn_Continue_s_Absyn_Raw_stmt_struct{int tag;};struct Cyc_Absyn_Goto_s_Absyn_Raw_stmt_struct{int tag;struct _dyneither_ptr*f1;};struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _tuple11 f2;struct _tuple11 f3;struct Cyc_Absyn_Stmt*f4;};struct Cyc_Absyn_Switch_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;void*f3;};struct Cyc_Absyn_Fallthru_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Switch_clause**f2;};struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Decl*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Label_s_Absyn_Raw_stmt_struct{int tag;struct _dyneither_ptr*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Do_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct _tuple11 f2;};struct Cyc_Absyn_TryCatch_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_List_List*f2;void*f3;};struct Cyc_Absyn_Stmt{void*r;unsigned int loc;void*annot;};struct Cyc_Absyn_Wild_p_Absyn_Raw_pat_struct{int tag;};struct Cyc_Absyn_Var_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_AliasVar_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;};struct Cyc_Absyn_Reference_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_TagInt_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;};struct Cyc_Absyn_Tuple_p_Absyn_Raw_pat_struct{int tag;struct Cyc_List_List*f1;int f2;};struct Cyc_Absyn_Pointer_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Pat*f1;};struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct{int tag;union Cyc_Absyn_AggrInfo*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;int f4;};struct Cyc_Absyn_Datatype_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;struct Cyc_List_List*f3;int f4;};struct Cyc_Absyn_Null_p_Absyn_Raw_pat_struct{int tag;};struct Cyc_Absyn_Int_p_Absyn_Raw_pat_struct{int tag;enum Cyc_Absyn_Sign f1;int f2;};struct Cyc_Absyn_Char_p_Absyn_Raw_pat_struct{int tag;char f1;};struct Cyc_Absyn_Float_p_Absyn_Raw_pat_struct{int tag;struct _dyneither_ptr f1;int f2;};struct Cyc_Absyn_Enum_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_AnonEnum_p_Absyn_Raw_pat_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_UnknownId_p_Absyn_Raw_pat_struct{int tag;struct _tuple2*f1;};struct Cyc_Absyn_UnknownCall_p_Absyn_Raw_pat_struct{int tag;struct _tuple2*f1;struct Cyc_List_List*f2;int f3;};struct Cyc_Absyn_Exp_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Pat{void*r;void*topt;unsigned int loc;};struct Cyc_Absyn_Switch_clause{struct Cyc_Absyn_Pat*pattern;struct Cyc_Core_Opt*pat_vars;struct Cyc_Absyn_Exp*where_clause;struct Cyc_Absyn_Stmt*body;unsigned int loc;};struct Cyc_Absyn_Unresolved_b_Absyn_Binding_struct{int tag;struct _tuple2*f1;};struct Cyc_Absyn_Global_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Funname_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Param_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Local_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Pat_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Vardecl{enum Cyc_Absyn_Scope sc;struct _tuple2*name;unsigned int varloc;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*initializer;void*rgn;struct Cyc_List_List*attributes;int escapes;};struct Cyc_Absyn_Fndecl{enum Cyc_Absyn_Scope sc;int is_inline;struct _tuple2*name;struct Cyc_List_List*tvs;void*effect;struct Cyc_Absyn_Tqual ret_tqual;void*ret_type;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_Absyn_Stmt*body;void*cached_type;struct Cyc_Core_Opt*param_vardecls;struct Cyc_Absyn_Vardecl*fn_vardecl;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;struct Cyc_List_List*requires_relns;struct Cyc_Absyn_Exp*ensures_clause;struct Cyc_List_List*ensures_relns;};struct Cyc_Absyn_Aggrfield{struct _dyneither_ptr*name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*width;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;};struct Cyc_Absyn_AggrdeclImpl{struct Cyc_List_List*exist_vars;struct Cyc_List_List*rgn_po;struct Cyc_List_List*fields;int tagged;};struct Cyc_Absyn_Aggrdecl{enum Cyc_Absyn_AggrKind kind;enum Cyc_Absyn_Scope sc;struct _tuple2*name;struct Cyc_List_List*tvs;struct Cyc_Absyn_AggrdeclImpl*impl;struct Cyc_List_List*attributes;int expected_mem_kind;};struct Cyc_Absyn_Datatypefield{struct _tuple2*name;struct Cyc_List_List*typs;unsigned int loc;enum Cyc_Absyn_Scope sc;};struct Cyc_Absyn_Datatypedecl{enum Cyc_Absyn_Scope sc;struct _tuple2*name;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*fields;int is_extensible;};struct Cyc_Absyn_Enumfield{struct _tuple2*name;struct Cyc_Absyn_Exp*tag;unsigned int loc;};struct Cyc_Absyn_Enumdecl{enum Cyc_Absyn_Scope sc;struct _tuple2*name;struct Cyc_Core_Opt*fields;};struct Cyc_Absyn_Typedefdecl{struct _tuple2*name;struct Cyc_Absyn_Tqual tq;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*kind;void*defn;struct Cyc_List_List*atts;int extern_c;};struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Let_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Pat*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;void*f4;};struct Cyc_Absyn_Letv_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Region_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Absyn_Datatype_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;};struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;};struct Cyc_Absyn_Typedef_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Typedefdecl*f1;};struct Cyc_Absyn_Namespace_d_Absyn_Raw_decl_struct{int tag;struct _dyneither_ptr*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Using_d_Absyn_Raw_decl_struct{int tag;struct _tuple2*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_ExternC_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_ExternCinclude_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Porton_d_Absyn_Raw_decl_struct{int tag;};struct Cyc_Absyn_Portoff_d_Absyn_Raw_decl_struct{int tag;};struct Cyc_Absyn_Decl{void*r;unsigned int loc;};struct Cyc_Absyn_ArrayElement_Absyn_Designator_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_FieldName_Absyn_Designator_struct{int tag;struct _dyneither_ptr*f1;};extern char Cyc_Absyn_EmptyAnnot[11U];struct Cyc_Absyn_EmptyAnnot_Absyn_AbsynAnnot_struct{char*tag;};
# 941 "absyn.h"
extern void*Cyc_Absyn_void_type;
# 1015
struct Cyc_Absyn_Exp*Cyc_Absyn_signed_int_exp(int,unsigned int);
struct Cyc_Absyn_Exp*Cyc_Absyn_uint_exp(unsigned int,unsigned int);
# 1027
struct Cyc_Absyn_Exp*Cyc_Absyn_prim1_exp(enum Cyc_Absyn_Primop,struct Cyc_Absyn_Exp*,unsigned int);
# 1158
struct Cyc_Absyn_Aggrfield*Cyc_Absyn_lookup_field(struct Cyc_List_List*,struct _dyneither_ptr*);
# 1160
struct Cyc_Absyn_Aggrfield*Cyc_Absyn_lookup_decl_field(struct Cyc_Absyn_Aggrdecl*,struct _dyneither_ptr*);
# 1172
struct Cyc_Absyn_Aggrdecl*Cyc_Absyn_get_known_aggrdecl(union Cyc_Absyn_AggrInfo);
# 1174
int Cyc_Absyn_is_nontagged_nonrequire_union_type(void*);
# 1176
int Cyc_Absyn_is_require_union_type(void*);struct Cyc_RgnOrder_RgnPO;
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
# 31 "tcutil.h"
void*Cyc_Tcutil_impos(struct _dyneither_ptr fmt,struct _dyneither_ptr ap);
# 41
int Cyc_Tcutil_is_void_type(void*);
# 43
int Cyc_Tcutil_is_any_int_type(void*);
int Cyc_Tcutil_is_any_float_type(void*);
# 55
int Cyc_Tcutil_is_zeroterm_pointer_type(void*t);
# 57
int Cyc_Tcutil_is_bound_one(void*b);
# 64
int Cyc_Tcutil_is_noreturn_fn_type(void*);
# 69
void*Cyc_Tcutil_pointer_elt_type(void*t);
# 80
struct Cyc_Absyn_Exp*Cyc_Tcutil_get_type_bound(void*t);
# 115
void*Cyc_Tcutil_compress(void*t);
# 204
void*Cyc_Tcutil_rsubstitute(struct _RegionHandle*,struct Cyc_List_List*,void*);
# 217
struct Cyc_Absyn_Exp*Cyc_Tcutil_rsubsexp(struct _RegionHandle*r,struct Cyc_List_List*,struct Cyc_Absyn_Exp*);
# 226
void*Cyc_Tcutil_fndecl2type(struct Cyc_Absyn_Fndecl*);
# 297 "tcutil.h"
int Cyc_Tcutil_is_zero_ptr_deref(struct Cyc_Absyn_Exp*e1,void**ptr_type,int*is_dyneither,void**elt_type);
# 305
int Cyc_Tcutil_is_noalias_pointer(void*t,int must_be_unique);
# 315
int Cyc_Tcutil_is_noalias_pointer_or_aggr(void*t);
# 374
struct Cyc_Absyn_Vardecl*Cyc_Tcutil_nonesc_vardecl(void*b);
# 377
struct Cyc_List_List*Cyc_Tcutil_filter_nulls(struct Cyc_List_List*l);struct _tuple12{unsigned int f1;int f2;};
# 28 "evexp.h"
struct _tuple12 Cyc_Evexp_eval_const_uint_exp(struct Cyc_Absyn_Exp*e);struct _union_RelnOp_RConst{int tag;unsigned int val;};struct _union_RelnOp_RVar{int tag;struct Cyc_Absyn_Vardecl*val;};struct _union_RelnOp_RNumelts{int tag;struct Cyc_Absyn_Vardecl*val;};struct _union_RelnOp_RType{int tag;void*val;};struct _union_RelnOp_RParam{int tag;unsigned int val;};struct _union_RelnOp_RParamNumelts{int tag;unsigned int val;};struct _union_RelnOp_RReturn{int tag;unsigned int val;};union Cyc_Relations_RelnOp{struct _union_RelnOp_RConst RConst;struct _union_RelnOp_RVar RVar;struct _union_RelnOp_RNumelts RNumelts;struct _union_RelnOp_RType RType;struct _union_RelnOp_RParam RParam;struct _union_RelnOp_RParamNumelts RParamNumelts;struct _union_RelnOp_RReturn RReturn;};
# 40 "relations-ap.h"
union Cyc_Relations_RelnOp Cyc_Relations_RConst(unsigned int);union Cyc_Relations_RelnOp Cyc_Relations_RVar(struct Cyc_Absyn_Vardecl*);union Cyc_Relations_RelnOp Cyc_Relations_RNumelts(struct Cyc_Absyn_Vardecl*);
# 49
enum Cyc_Relations_Relation{Cyc_Relations_Req  = 0U,Cyc_Relations_Rneq  = 1U,Cyc_Relations_Rlte  = 2U,Cyc_Relations_Rlt  = 3U};struct Cyc_Relations_Reln{union Cyc_Relations_RelnOp rop1;enum Cyc_Relations_Relation relation;union Cyc_Relations_RelnOp rop2;};struct _tuple13{struct Cyc_Absyn_Exp*f1;enum Cyc_Relations_Relation f2;struct Cyc_Absyn_Exp*f3;};
# 63
struct _tuple13 Cyc_Relations_primop2relation(struct Cyc_Absyn_Exp*e1,enum Cyc_Absyn_Primop p,struct Cyc_Absyn_Exp*e2);
# 67
enum Cyc_Relations_Relation Cyc_Relations_flip_relation(enum Cyc_Relations_Relation r);
# 69
struct Cyc_Relations_Reln*Cyc_Relations_negate(struct _RegionHandle*,struct Cyc_Relations_Reln*);
# 75
int Cyc_Relations_exp2relnop(struct Cyc_Absyn_Exp*e,union Cyc_Relations_RelnOp*p);
# 83
struct Cyc_List_List*Cyc_Relations_exp2relns(struct _RegionHandle*r,struct Cyc_Absyn_Exp*e);
# 86
struct Cyc_List_List*Cyc_Relations_add_relation(struct _RegionHandle*rgn,union Cyc_Relations_RelnOp rop1,enum Cyc_Relations_Relation r,union Cyc_Relations_RelnOp rop2,struct Cyc_List_List*relns);
# 99
struct Cyc_List_List*Cyc_Relations_reln_assign_var(struct _RegionHandle*,struct Cyc_List_List*,struct Cyc_Absyn_Vardecl*,struct Cyc_Absyn_Exp*);
# 102
struct Cyc_List_List*Cyc_Relations_reln_assign_exp(struct _RegionHandle*,struct Cyc_List_List*,struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*);
# 107
struct Cyc_List_List*Cyc_Relations_reln_kill_exp(struct _RegionHandle*,struct Cyc_List_List*,struct Cyc_Absyn_Exp*);
# 111
int Cyc_Relations_same_relns(struct Cyc_List_List*,struct Cyc_List_List*);
# 117
void Cyc_Relations_print_relns(struct Cyc___cycFILE*,struct Cyc_List_List*);
# 120
struct _dyneither_ptr Cyc_Relations_rop2string(union Cyc_Relations_RelnOp r);
struct _dyneither_ptr Cyc_Relations_relation2string(enum Cyc_Relations_Relation r);
struct _dyneither_ptr Cyc_Relations_relns2string(struct Cyc_List_List*r);
# 128
int Cyc_Relations_consistent_relations(struct Cyc_List_List*rlns);struct Cyc_Hashtable_Table;
# 39 "hashtable.h"
struct Cyc_Hashtable_Table*Cyc_Hashtable_create(int sz,int(*cmp)(void*,void*),int(*hash)(void*));
# 50
void Cyc_Hashtable_insert(struct Cyc_Hashtable_Table*t,void*key,void*val);
# 52
void*Cyc_Hashtable_lookup(struct Cyc_Hashtable_Table*t,void*key);
# 56
void**Cyc_Hashtable_lookup_opt(struct Cyc_Hashtable_Table*t,void*key);struct Cyc_JumpAnalysis_Jump_Anal_Result{struct Cyc_Hashtable_Table*pop_tables;struct Cyc_Hashtable_Table*succ_tables;struct Cyc_Hashtable_Table*pat_pop_tables;};
# 46 "jump_analysis.h"
struct Cyc_JumpAnalysis_Jump_Anal_Result*Cyc_JumpAnalysis_jump_analysis(struct Cyc_List_List*tds);
# 41 "cf_flowinfo.h"
int Cyc_CfFlowInfo_anal_error;
void Cyc_CfFlowInfo_aerr(unsigned int loc,struct _dyneither_ptr fmt,struct _dyneither_ptr ap);struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_CfFlowInfo_MallocPt_CfFlowInfo_Root_struct{int tag;struct Cyc_Absyn_Exp*f1;void*f2;};struct Cyc_CfFlowInfo_InitParam_CfFlowInfo_Root_struct{int tag;int f1;void*f2;};struct Cyc_CfFlowInfo_Dot_CfFlowInfo_PathCon_struct{int tag;int f1;};struct Cyc_CfFlowInfo_Star_CfFlowInfo_PathCon_struct{int tag;};struct Cyc_CfFlowInfo_Place{void*root;struct Cyc_List_List*path;};
# 74
enum Cyc_CfFlowInfo_InitLevel{Cyc_CfFlowInfo_NoneIL  = 0U,Cyc_CfFlowInfo_AllIL  = 1U};extern char Cyc_CfFlowInfo_IsZero[7U];struct Cyc_CfFlowInfo_IsZero_Absyn_AbsynAnnot_struct{char*tag;};extern char Cyc_CfFlowInfo_NotZero[8U];struct Cyc_CfFlowInfo_NotZero_Absyn_AbsynAnnot_struct{char*tag;struct Cyc_List_List*f1;};extern char Cyc_CfFlowInfo_UnknownZ[9U];struct Cyc_CfFlowInfo_UnknownZ_Absyn_AbsynAnnot_struct{char*tag;struct Cyc_List_List*f1;};
# 87
extern struct Cyc_CfFlowInfo_IsZero_Absyn_AbsynAnnot_struct Cyc_CfFlowInfo_IsZero_val;struct _union_AbsLVal_PlaceL{int tag;struct Cyc_CfFlowInfo_Place*val;};struct _union_AbsLVal_UnknownL{int tag;int val;};union Cyc_CfFlowInfo_AbsLVal{struct _union_AbsLVal_PlaceL PlaceL;struct _union_AbsLVal_UnknownL UnknownL;};
# 94
union Cyc_CfFlowInfo_AbsLVal Cyc_CfFlowInfo_PlaceL(struct Cyc_CfFlowInfo_Place*);
union Cyc_CfFlowInfo_AbsLVal Cyc_CfFlowInfo_UnknownL();struct Cyc_CfFlowInfo_UnionRInfo{int is_union;int fieldnum;};struct Cyc_CfFlowInfo_Zero_CfFlowInfo_AbsRVal_struct{int tag;};struct Cyc_CfFlowInfo_NotZeroAll_CfFlowInfo_AbsRVal_struct{int tag;};struct Cyc_CfFlowInfo_UnknownR_CfFlowInfo_AbsRVal_struct{int tag;enum Cyc_CfFlowInfo_InitLevel f1;};struct Cyc_CfFlowInfo_Esc_CfFlowInfo_AbsRVal_struct{int tag;enum Cyc_CfFlowInfo_InitLevel f1;};struct Cyc_CfFlowInfo_AddressOf_CfFlowInfo_AbsRVal_struct{int tag;struct Cyc_CfFlowInfo_Place*f1;};struct Cyc_CfFlowInfo_UniquePtr_CfFlowInfo_AbsRVal_struct{int tag;void*f1;};struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct{int tag;struct Cyc_CfFlowInfo_UnionRInfo f1;struct _dyneither_ptr f2;};struct Cyc_CfFlowInfo_Consumed_CfFlowInfo_AbsRVal_struct{int tag;struct Cyc_Absyn_Exp*f1;int f2;void*f3;};struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct{int tag;struct Cyc_Absyn_Vardecl*f1;void*f2;};struct _union_FlowInfo_BottomFL{int tag;int val;};struct _tuple14{struct Cyc_Dict_Dict f1;struct Cyc_List_List*f2;};struct _union_FlowInfo_ReachableFL{int tag;struct _tuple14 val;};union Cyc_CfFlowInfo_FlowInfo{struct _union_FlowInfo_BottomFL BottomFL;struct _union_FlowInfo_ReachableFL ReachableFL;};
# 138 "cf_flowinfo.h"
union Cyc_CfFlowInfo_FlowInfo Cyc_CfFlowInfo_BottomFL();
union Cyc_CfFlowInfo_FlowInfo Cyc_CfFlowInfo_ReachableFL(struct Cyc_Dict_Dict,struct Cyc_List_List*);struct Cyc_CfFlowInfo_FlowEnv{void*zero;void*notzeroall;void*unknown_none;void*unknown_all;void*esc_none;void*esc_all;struct Cyc_Dict_Dict mt_flowdict;struct Cyc_CfFlowInfo_Place*dummy_place;};
# 153
struct Cyc_CfFlowInfo_FlowEnv*Cyc_CfFlowInfo_new_flow_env();
# 155
int Cyc_CfFlowInfo_get_field_index(void*t,struct _dyneither_ptr*f);
int Cyc_CfFlowInfo_get_field_index_fs(struct Cyc_List_List*fs,struct _dyneither_ptr*f);
# 158
int Cyc_CfFlowInfo_root_cmp(void*,void*);
# 161
struct _dyneither_ptr Cyc_CfFlowInfo_aggrfields_to_aggrdict(struct Cyc_CfFlowInfo_FlowEnv*,struct Cyc_List_List*,int no_init_bits_only,void*);
void*Cyc_CfFlowInfo_typ_to_absrval(struct Cyc_CfFlowInfo_FlowEnv*,void*t,int no_init_bits_only,void*leafval);
void*Cyc_CfFlowInfo_make_unique_consumed(struct Cyc_CfFlowInfo_FlowEnv*fenv,void*t,struct Cyc_Absyn_Exp*consumer,int iteration,void*);
int Cyc_CfFlowInfo_is_unique_consumed(struct Cyc_Absyn_Exp*e,int env_iteration,void*r,int*needs_unconsume);
void*Cyc_CfFlowInfo_make_unique_unconsumed(struct Cyc_CfFlowInfo_FlowEnv*fenv,void*r);struct _tuple15{void*f1;struct Cyc_List_List*f2;};
struct _tuple15 Cyc_CfFlowInfo_unname_rval(void*rv);
# 168
enum Cyc_CfFlowInfo_InitLevel Cyc_CfFlowInfo_initlevel(struct Cyc_CfFlowInfo_FlowEnv*,struct Cyc_Dict_Dict d,void*r);
void*Cyc_CfFlowInfo_lookup_place(struct Cyc_Dict_Dict d,struct Cyc_CfFlowInfo_Place*place);
# 171
int Cyc_CfFlowInfo_is_init_pointer(void*r);
int Cyc_CfFlowInfo_flow_lessthan_approx(union Cyc_CfFlowInfo_FlowInfo f1,union Cyc_CfFlowInfo_FlowInfo f2);
# 174
void Cyc_CfFlowInfo_print_absrval(void*rval);
# 181
void Cyc_CfFlowInfo_print_flow(union Cyc_CfFlowInfo_FlowInfo f);
# 196 "cf_flowinfo.h"
struct Cyc_Dict_Dict Cyc_CfFlowInfo_escape_deref(struct Cyc_CfFlowInfo_FlowEnv*fenv,struct Cyc_Dict_Dict d,void*r);
struct Cyc_Dict_Dict Cyc_CfFlowInfo_assign_place(struct Cyc_CfFlowInfo_FlowEnv*fenv,unsigned int loc,struct Cyc_Dict_Dict d,struct Cyc_CfFlowInfo_Place*place,void*r);
# 201
union Cyc_CfFlowInfo_FlowInfo Cyc_CfFlowInfo_join_flow(struct Cyc_CfFlowInfo_FlowEnv*,union Cyc_CfFlowInfo_FlowInfo,union Cyc_CfFlowInfo_FlowInfo);struct _tuple16{union Cyc_CfFlowInfo_FlowInfo f1;void*f2;};
struct _tuple16 Cyc_CfFlowInfo_join_flow_and_rval(struct Cyc_CfFlowInfo_FlowEnv*,struct _tuple16 pr1,struct _tuple16 pr2);
# 32 "new_control_flow.h"
void Cyc_NewControlFlow_cf_check(struct Cyc_JumpAnalysis_Jump_Anal_Result*tables,struct Cyc_List_List*ds);
# 37
extern int Cyc_NewControlFlow_warn_lose_unique;struct Cyc_Tcpat_TcPatResult{struct _tuple1*tvars_and_bounds_opt;struct Cyc_List_List*patvars;};struct Cyc_Tcpat_WhereTest_Tcpat_PatTest_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Tcpat_EqNull_Tcpat_PatTest_struct{int tag;};struct Cyc_Tcpat_NeqNull_Tcpat_PatTest_struct{int tag;};struct Cyc_Tcpat_EqEnum_Tcpat_PatTest_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Tcpat_EqAnonEnum_Tcpat_PatTest_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Tcpat_EqFloat_Tcpat_PatTest_struct{int tag;struct _dyneither_ptr f1;int f2;};struct Cyc_Tcpat_EqConst_Tcpat_PatTest_struct{int tag;unsigned int f1;};struct Cyc_Tcpat_EqDatatypeTag_Tcpat_PatTest_struct{int tag;int f1;struct Cyc_Absyn_Datatypedecl*f2;struct Cyc_Absyn_Datatypefield*f3;};struct Cyc_Tcpat_EqTaggedUnion_Tcpat_PatTest_struct{int tag;struct _dyneither_ptr*f1;int f2;};struct Cyc_Tcpat_EqExtensibleDatatype_Tcpat_PatTest_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;};struct Cyc_Tcpat_Dummy_Tcpat_Access_struct{int tag;};struct Cyc_Tcpat_Deref_Tcpat_Access_struct{int tag;};struct Cyc_Tcpat_TupleField_Tcpat_Access_struct{int tag;unsigned int f1;};struct Cyc_Tcpat_DatatypeField_Tcpat_Access_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;unsigned int f3;};struct Cyc_Tcpat_AggrField_Tcpat_Access_struct{int tag;int f1;struct _dyneither_ptr*f2;};struct _union_PatOrWhere_pattern{int tag;struct Cyc_Absyn_Pat*val;};struct _union_PatOrWhere_where_clause{int tag;struct Cyc_Absyn_Exp*val;};union Cyc_Tcpat_PatOrWhere{struct _union_PatOrWhere_pattern pattern;struct _union_PatOrWhere_where_clause where_clause;};struct Cyc_Tcpat_PathNode{union Cyc_Tcpat_PatOrWhere orig_pat;void*access;};struct Cyc_Tcpat_Rhs{int used;unsigned int pat_loc;struct Cyc_Absyn_Stmt*rhs;};struct Cyc_Tcpat_Failure_Tcpat_Decision_struct{int tag;void*f1;};struct Cyc_Tcpat_Success_Tcpat_Decision_struct{int tag;struct Cyc_Tcpat_Rhs*f1;};struct Cyc_Tcpat_SwitchDec_Tcpat_Decision_struct{int tag;struct Cyc_List_List*f1;struct Cyc_List_List*f2;void*f3;};
# 115 "tcpat.h"
int Cyc_Tcpat_has_vars(struct Cyc_Core_Opt*pat_vars);
# 25 "warn.h"
void Cyc_Warn_vwarn(unsigned int loc,struct _dyneither_ptr fmt,struct _dyneither_ptr ap);
# 27
void Cyc_Warn_warn(unsigned int loc,struct _dyneither_ptr fmt,struct _dyneither_ptr ap);
# 29
void Cyc_Warn_flush_warnings();
# 31
void Cyc_Warn_verr(unsigned int loc,struct _dyneither_ptr fmt,struct _dyneither_ptr ap);
# 33
void Cyc_Warn_err(unsigned int loc,struct _dyneither_ptr fmt,struct _dyneither_ptr ap);
# 36
void*Cyc_Warn_vimpos(struct _dyneither_ptr fmt,struct _dyneither_ptr ap);
# 38
void*Cyc_Warn_impos(struct _dyneither_ptr fmt,struct _dyneither_ptr ap);
# 41
void*Cyc_Warn_vimpos_loc(unsigned int loc,struct _dyneither_ptr fmt,struct _dyneither_ptr ap);
# 44
void*Cyc_Warn_impos_loc(unsigned int loc,struct _dyneither_ptr fmt,struct _dyneither_ptr ap);struct Cyc_PP_Ppstate;struct Cyc_PP_Out;struct Cyc_PP_Doc;struct Cyc_Absynpp_Params{int expand_typedefs;int qvar_to_Cids;int add_cyc_prefix;int to_VC;int decls_first;int rewrite_temp_tvars;int print_all_tvars;int print_all_kinds;int print_all_effects;int print_using_stmts;int print_externC_stmts;int print_full_evars;int print_zeroterm;int generate_line_directives;int use_curr_namespace;struct Cyc_List_List*curr_namespace;};
# 67 "absynpp.h"
struct _dyneither_ptr Cyc_Absynpp_exp2string(struct Cyc_Absyn_Exp*);
# 69
struct _dyneither_ptr Cyc_Absynpp_qvar2string(struct _tuple2*);
# 51 "new_control_flow.cyc"
static int Cyc_NewControlFlow_strcmp(struct _dyneither_ptr s1,struct _dyneither_ptr s2){
if((char*)s1.curr == (char*)s2.curr)
return 0;{
int i=0;
unsigned int sz1=_get_dyneither_size(s1,sizeof(char));
unsigned int sz2=_get_dyneither_size(s2,sizeof(char));
unsigned int minsz=sz1 < sz2?sz1: sz2;
# 59
while(i < minsz){
char c1=*((const char*)_check_dyneither_subscript(s1,sizeof(char),i));
char c2=*((const char*)_check_dyneither_subscript(s2,sizeof(char),i));
if(c1 == '\000'){
if(c2 == '\000')return 0;else{
return - 1;}}else{
if(c2 == '\000')return 1;else{
# 67
int diff=c1 - c2;
if(diff != 0)return diff;}}
# 70
++ i;}
# 72
if(sz1 == sz2)return 0;
if(minsz < sz2){
if(*((const char*)_check_dyneither_subscript(s2,sizeof(char),i))== '\000')return 0;else{
return - 1;}}else{
# 77
if(*((const char*)_check_dyneither_subscript(s1,sizeof(char),i))== '\000')return 0;else{
return 1;}}};}
# 83
int Cyc_NewControlFlow_warn_lose_unique=0;struct Cyc_NewControlFlow_CFStmtAnnot{int visited;};static char Cyc_NewControlFlow_CFAnnot[8U]="CFAnnot";struct Cyc_NewControlFlow_CFAnnot_Absyn_AbsynAnnot_struct{char*tag;struct Cyc_NewControlFlow_CFStmtAnnot f1;};struct Cyc_NewControlFlow_AnalEnv{struct Cyc_JumpAnalysis_Jump_Anal_Result*all_tables;struct Cyc_Hashtable_Table*succ_table;struct Cyc_Hashtable_Table*pat_pop_table;struct Cyc_CfFlowInfo_FlowEnv*fenv;int iterate_again;int iteration_num;int in_try;union Cyc_CfFlowInfo_FlowInfo tryflow;int noreturn;void*return_type;struct Cyc_List_List*unique_pat_vars;struct Cyc_List_List*param_roots;struct Cyc_List_List*noconsume_params;struct Cyc_Hashtable_Table*flow_table;struct Cyc_List_List*return_relns;};struct _tuple17{void*f1;int f2;};
# 125 "new_control_flow.cyc"
static union Cyc_CfFlowInfo_FlowInfo Cyc_NewControlFlow_anal_stmt(struct Cyc_NewControlFlow_AnalEnv*,union Cyc_CfFlowInfo_FlowInfo,struct Cyc_Absyn_Stmt*,struct _tuple17*);
static union Cyc_CfFlowInfo_FlowInfo Cyc_NewControlFlow_anal_decl(struct Cyc_NewControlFlow_AnalEnv*,union Cyc_CfFlowInfo_FlowInfo,struct Cyc_Absyn_Decl*);struct _tuple18{union Cyc_CfFlowInfo_FlowInfo f1;union Cyc_CfFlowInfo_AbsLVal f2;};
static struct _tuple18 Cyc_NewControlFlow_anal_Lexp(struct Cyc_NewControlFlow_AnalEnv*,union Cyc_CfFlowInfo_FlowInfo,int expand_unique,int passthrough_consumes,struct Cyc_Absyn_Exp*);
static struct _tuple16 Cyc_NewControlFlow_anal_Rexp(struct Cyc_NewControlFlow_AnalEnv*,int copy_ctxt,union Cyc_CfFlowInfo_FlowInfo,struct Cyc_Absyn_Exp*);struct _tuple19{union Cyc_CfFlowInfo_FlowInfo f1;union Cyc_CfFlowInfo_FlowInfo f2;};
static struct _tuple19 Cyc_NewControlFlow_anal_test(struct Cyc_NewControlFlow_AnalEnv*,union Cyc_CfFlowInfo_FlowInfo,struct Cyc_Absyn_Exp*);
static struct Cyc_List_List*Cyc_NewControlFlow_noalias_ptrs_rec(struct Cyc_NewControlFlow_AnalEnv*,struct Cyc_CfFlowInfo_Place*,void*);
static union Cyc_CfFlowInfo_FlowInfo Cyc_NewControlFlow_expand_unique_places(struct Cyc_NewControlFlow_AnalEnv*,union Cyc_CfFlowInfo_FlowInfo inflow,struct Cyc_List_List*es);
# 135
static struct Cyc_NewControlFlow_CFStmtAnnot*Cyc_NewControlFlow_get_stmt_annot(struct Cyc_Absyn_Stmt*s){
void*_tmp0=s->annot;void*_tmp1=_tmp0;struct Cyc_NewControlFlow_CFStmtAnnot*_tmp3;if(((struct Cyc_NewControlFlow_CFAnnot_Absyn_AbsynAnnot_struct*)_tmp1)->tag == Cyc_NewControlFlow_CFAnnot){_LL1: _tmp3=(struct Cyc_NewControlFlow_CFStmtAnnot*)&((struct Cyc_NewControlFlow_CFAnnot_Absyn_AbsynAnnot_struct*)_tmp1)->f1;_LL2:
 return _tmp3;}else{_LL3: _LL4:
({void*_tmp638=(void*)({struct Cyc_NewControlFlow_CFAnnot_Absyn_AbsynAnnot_struct*_tmp2=_cycalloc(sizeof(*_tmp2));_tmp2->tag=Cyc_NewControlFlow_CFAnnot,(_tmp2->f1).visited=0;_tmp2;});s->annot=_tmp638;});return Cyc_NewControlFlow_get_stmt_annot(s);}_LL0:;}
# 142
static union Cyc_CfFlowInfo_FlowInfo*Cyc_NewControlFlow_get_stmt_flow(struct Cyc_NewControlFlow_AnalEnv*env,struct Cyc_Absyn_Stmt*s){
union Cyc_CfFlowInfo_FlowInfo**sflow=((union Cyc_CfFlowInfo_FlowInfo**(*)(struct Cyc_Hashtable_Table*t,struct Cyc_Absyn_Stmt*key))Cyc_Hashtable_lookup_opt)(env->flow_table,s);
if(sflow == 0){
union Cyc_CfFlowInfo_FlowInfo*res=({union Cyc_CfFlowInfo_FlowInfo*_tmp4=_cycalloc(sizeof(*_tmp4));({union Cyc_CfFlowInfo_FlowInfo _tmp639=Cyc_CfFlowInfo_BottomFL();*_tmp4=_tmp639;});_tmp4;});
((void(*)(struct Cyc_Hashtable_Table*t,struct Cyc_Absyn_Stmt*key,union Cyc_CfFlowInfo_FlowInfo*val))Cyc_Hashtable_insert)(env->flow_table,s,res);
return res;}
# 149
return*sflow;}
# 152
static struct Cyc_List_List*Cyc_NewControlFlow_flowrelns(union Cyc_CfFlowInfo_FlowInfo f){
union Cyc_CfFlowInfo_FlowInfo _tmp5=f;struct Cyc_List_List*_tmp6;if((_tmp5.BottomFL).tag == 1){_LL1: _LL2:
 return 0;}else{_LL3: _tmp6=((_tmp5.ReachableFL).val).f2;_LL4:
 return _tmp6;}_LL0:;}struct _tuple20{struct Cyc_NewControlFlow_CFStmtAnnot*f1;union Cyc_CfFlowInfo_FlowInfo*f2;};
# 159
static struct _tuple20 Cyc_NewControlFlow_pre_stmt_check(struct Cyc_NewControlFlow_AnalEnv*env,union Cyc_CfFlowInfo_FlowInfo inflow,struct Cyc_Absyn_Stmt*s){
struct Cyc_NewControlFlow_CFStmtAnnot*_tmp7=Cyc_NewControlFlow_get_stmt_annot(s);
union Cyc_CfFlowInfo_FlowInfo*_tmp8=Cyc_NewControlFlow_get_stmt_flow(env,s);
# 163
({union Cyc_CfFlowInfo_FlowInfo _tmp63A=Cyc_CfFlowInfo_join_flow(env->fenv,inflow,*_tmp8);*_tmp8=_tmp63A;});
# 169
_tmp7->visited=env->iteration_num;
return({struct _tuple20 _tmp599;_tmp599.f1=_tmp7,_tmp599.f2=_tmp8;_tmp599;});}
# 179
static void Cyc_NewControlFlow_update_tryflow(struct Cyc_NewControlFlow_AnalEnv*env,union Cyc_CfFlowInfo_FlowInfo new_flow){
if(env->in_try)
# 187
({union Cyc_CfFlowInfo_FlowInfo _tmp63B=Cyc_CfFlowInfo_join_flow(env->fenv,new_flow,env->tryflow);env->tryflow=_tmp63B;});}struct _tuple21{struct Cyc_NewControlFlow_AnalEnv*f1;unsigned int f2;struct Cyc_Dict_Dict f3;};
# 194
static void Cyc_NewControlFlow_check_unique_root(struct _tuple21*ckenv,void*root,void*rval){
# 196
struct _tuple21*_tmp9=ckenv;struct Cyc_NewControlFlow_AnalEnv*_tmp13;unsigned int _tmp12;struct Cyc_Dict_Dict _tmp11;_LL1: _tmp13=_tmp9->f1;_tmp12=_tmp9->f2;_tmp11=_tmp9->f3;_LL2:;{
void*_tmpA=root;struct Cyc_Absyn_Vardecl*_tmp10;if(((struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct*)_tmpA)->tag == 0U){_LL4: _tmp10=((struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct*)_tmpA)->f1;_LL5:
# 199
 if(!((int(*)(struct Cyc_Dict_Dict d,void*k,void**ans))Cyc_Dict_lookup_bool)(_tmp11,root,& rval) && 
Cyc_Tcutil_is_noalias_pointer_or_aggr(_tmp10->type)){
retry: {void*_tmpB=rval;void*_tmpF;switch(*((int*)_tmpB)){case 8U: _LL9: _tmpF=(void*)((struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*)_tmpB)->f2;_LLA:
 rval=_tmpF;goto retry;case 7U: _LLB: _LLC:
 goto _LLE;case 2U: if(((struct Cyc_CfFlowInfo_UnknownR_CfFlowInfo_AbsRVal_struct*)_tmpB)->f1 == Cyc_CfFlowInfo_NoneIL){_LLD: _LLE:
 goto _LL10;}else{goto _LL11;}case 0U: _LLF: _LL10:
 goto _LL8;default: _LL11: _LL12:
# 208
({struct Cyc_String_pa_PrintArg_struct _tmpE=({struct Cyc_String_pa_PrintArg_struct _tmp59A;_tmp59A.tag=0U,({struct _dyneither_ptr _tmp63C=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(_tmp10->name));_tmp59A.f1=_tmp63C;});_tmp59A;});void*_tmpC[1U];_tmpC[0]=& _tmpE;({unsigned int _tmp63E=_tmp12;struct _dyneither_ptr _tmp63D=({const char*_tmpD="alias-free pointer(s) reachable from %s may become unreachable.";_tag_dyneither(_tmpD,sizeof(char),64U);});Cyc_Warn_warn(_tmp63E,_tmp63D,_tag_dyneither(_tmpC,sizeof(void*),1U));});});
goto _LL8;}_LL8:;}}
# 212
goto _LL3;}else{_LL6: _LL7:
 goto _LL3;}_LL3:;};}
# 220
static void Cyc_NewControlFlow_update_flow(struct Cyc_NewControlFlow_AnalEnv*env,struct Cyc_Absyn_Stmt*s,union Cyc_CfFlowInfo_FlowInfo flow){
struct Cyc_NewControlFlow_CFStmtAnnot*_tmp14=Cyc_NewControlFlow_get_stmt_annot(s);
union Cyc_CfFlowInfo_FlowInfo*_tmp15=Cyc_NewControlFlow_get_stmt_flow(env,s);
union Cyc_CfFlowInfo_FlowInfo _tmp16=Cyc_CfFlowInfo_join_flow(env->fenv,flow,*_tmp15);
# 226
if(Cyc_NewControlFlow_warn_lose_unique){
struct _tuple19 _tmp17=({struct _tuple19 _tmp59C;_tmp59C.f1=flow,_tmp59C.f2=_tmp16;_tmp59C;});struct _tuple19 _tmp18=_tmp17;struct Cyc_Dict_Dict _tmp1B;struct Cyc_Dict_Dict _tmp1A;if(((_tmp18.f1).ReachableFL).tag == 2){if(((_tmp18.f2).ReachableFL).tag == 2){_LL1: _tmp1B=(((_tmp18.f1).ReachableFL).val).f1;_tmp1A=(((_tmp18.f2).ReachableFL).val).f1;_LL2: {
# 229
struct _tuple21 _tmp19=({struct _tuple21 _tmp59B;_tmp59B.f1=env,_tmp59B.f2=s->loc,_tmp59B.f3=_tmp1A;_tmp59B;});
((void(*)(void(*f)(struct _tuple21*,void*,void*),struct _tuple21*env,struct Cyc_Dict_Dict d))Cyc_Dict_iter_c)(Cyc_NewControlFlow_check_unique_root,& _tmp19,_tmp1B);
goto _LL0;}}else{goto _LL3;}}else{_LL3: _LL4:
 goto _LL0;}_LL0:;}
# 235
if(!Cyc_CfFlowInfo_flow_lessthan_approx(_tmp16,*_tmp15)){
# 242
*_tmp15=_tmp16;
# 246
if(_tmp14->visited == env->iteration_num)
# 248
env->iterate_again=1;}}
# 253
static union Cyc_CfFlowInfo_FlowInfo Cyc_NewControlFlow_add_vars(struct Cyc_CfFlowInfo_FlowEnv*fenv,union Cyc_CfFlowInfo_FlowInfo inflow,struct Cyc_List_List*vds,void*leafval,unsigned int loc,int nameit){
# 257
union Cyc_CfFlowInfo_FlowInfo _tmp1C=inflow;struct Cyc_Dict_Dict _tmp22;struct Cyc_List_List*_tmp21;if((_tmp1C.BottomFL).tag == 1){_LL1: _LL2:
 return Cyc_CfFlowInfo_BottomFL();}else{_LL3: _tmp22=((_tmp1C.ReachableFL).val).f1;_tmp21=((_tmp1C.ReachableFL).val).f2;_LL4:
# 260
 for(0;vds != 0;vds=vds->tl){
struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct*_tmp1D=({struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct*_tmp20=_cycalloc(sizeof(*_tmp20));_tmp20->tag=0U,_tmp20->f1=(struct Cyc_Absyn_Vardecl*)vds->hd;_tmp20;});
void*_tmp1E=Cyc_CfFlowInfo_typ_to_absrval(fenv,((struct Cyc_Absyn_Vardecl*)vds->hd)->type,0,leafval);
if(nameit)
({void*_tmp63F=(void*)({struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*_tmp1F=_cycalloc(sizeof(*_tmp1F));_tmp1F->tag=8U,_tmp1F->f1=(struct Cyc_Absyn_Vardecl*)vds->hd,_tmp1F->f2=_tmp1E;_tmp1F;});_tmp1E=_tmp63F;});
# 267
({struct Cyc_Dict_Dict _tmp640=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,void*k,void*v))Cyc_Dict_insert)(_tmp22,(void*)_tmp1D,_tmp1E);_tmp22=_tmp640;});}
# 269
return Cyc_CfFlowInfo_ReachableFL(_tmp22,_tmp21);}_LL0:;}
# 273
static int Cyc_NewControlFlow_relns_ok(struct Cyc_List_List*assume,struct Cyc_List_List*req){
# 280
for(0;(unsigned int)req;req=req->tl){
struct Cyc_Relations_Reln*_tmp23=Cyc_Relations_negate(Cyc_Core_heap_region,(struct Cyc_Relations_Reln*)req->hd);
if(Cyc_Relations_consistent_relations(({struct Cyc_List_List*_tmp24=_cycalloc(sizeof(*_tmp24));_tmp24->hd=_tmp23,_tmp24->tl=assume;_tmp24;})))
return 0;}
# 285
return 1;}
# 288
static struct Cyc_Absyn_Exp*Cyc_NewControlFlow_strip_cast(struct Cyc_Absyn_Exp*e){
void*_tmp25=e->r;void*_tmp26=_tmp25;struct Cyc_Absyn_Exp*_tmp27;if(((struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_tmp26)->tag == 14U){_LL1: _tmp27=((struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_tmp26)->f2;_LL2:
 return _tmp27;}else{_LL3: _LL4:
 return e;}_LL0:;}
# 295
static void Cyc_NewControlFlow_check_union_requires(unsigned int loc,void*t,struct _dyneither_ptr*f,union Cyc_CfFlowInfo_FlowInfo inflow){
# 297
union Cyc_CfFlowInfo_FlowInfo _tmp28=inflow;struct Cyc_List_List*_tmp46;if((_tmp28.BottomFL).tag == 1){_LL1: _LL2:
 return;}else{_LL3: _tmp46=((_tmp28.ReachableFL).val).f2;_LL4:
# 300
{void*_tmp29=Cyc_Tcutil_compress(t);void*_tmp2A=_tmp29;struct Cyc_List_List*_tmp45;union Cyc_Absyn_AggrInfo _tmp44;struct Cyc_List_List*_tmp43;switch(*((int*)_tmp2A)){case 0U: if(((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp2A)->f1)->tag == 20U){_LL6: _tmp44=((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp2A)->f1)->f1;_tmp43=((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp2A)->f2;_LL7: {
# 302
struct Cyc_Absyn_Aggrdecl*_tmp2B=Cyc_Absyn_get_known_aggrdecl(_tmp44);
struct Cyc_Absyn_Aggrfield*_tmp2C=Cyc_Absyn_lookup_decl_field(_tmp2B,f);
struct Cyc_Absyn_Exp*_tmp2D=((struct Cyc_Absyn_Aggrfield*)_check_null(_tmp2C))->requires_clause;
if(_tmp2D != 0){
struct _RegionHandle _tmp2E=_new_region("temp");struct _RegionHandle*temp=& _tmp2E;_push_region(temp);
{struct Cyc_Absyn_Exp*_tmp2F=({struct _RegionHandle*_tmp642=temp;struct Cyc_List_List*_tmp641=((struct Cyc_List_List*(*)(struct _RegionHandle*r1,struct _RegionHandle*r2,struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_rzip)(temp,temp,_tmp2B->tvs,_tmp43);Cyc_Tcutil_rsubsexp(_tmp642,_tmp641,_tmp2D);});
# 309
if(!({struct Cyc_List_List*_tmp643=_tmp46;Cyc_NewControlFlow_relns_ok(_tmp643,Cyc_Relations_exp2relns(temp,_tmp2F));})){
({struct Cyc_String_pa_PrintArg_struct _tmp32=({struct Cyc_String_pa_PrintArg_struct _tmp59E;_tmp59E.tag=0U,({struct _dyneither_ptr _tmp644=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_exp2string(Cyc_NewControlFlow_strip_cast(_tmp2F)));_tmp59E.f1=_tmp644;});_tmp59E;});struct Cyc_String_pa_PrintArg_struct _tmp33=({struct Cyc_String_pa_PrintArg_struct _tmp59D;_tmp59D.tag=0U,_tmp59D.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*f);_tmp59D;});void*_tmp30[2U];_tmp30[0]=& _tmp32,_tmp30[1]=& _tmp33;({unsigned int _tmp646=loc;struct _dyneither_ptr _tmp645=({const char*_tmp31="unable to prove %s, required to access %s";_tag_dyneither(_tmp31,sizeof(char),42U);});Cyc_CfFlowInfo_aerr(_tmp646,_tmp645,_tag_dyneither(_tmp30,sizeof(void*),2U));});});
({void*_tmp34=0U;({struct Cyc___cycFILE*_tmp648=Cyc_stderr;struct _dyneither_ptr _tmp647=({const char*_tmp35="  [recorded facts on path: ";_tag_dyneither(_tmp35,sizeof(char),28U);});Cyc_fprintf(_tmp648,_tmp647,_tag_dyneither(_tmp34,sizeof(void*),0U));});});
Cyc_Relations_print_relns(Cyc_stderr,_tmp46);
({void*_tmp36=0U;({struct Cyc___cycFILE*_tmp64A=Cyc_stderr;struct _dyneither_ptr _tmp649=({const char*_tmp37="]\n";_tag_dyneither(_tmp37,sizeof(char),3U);});Cyc_fprintf(_tmp64A,_tmp649,_tag_dyneither(_tmp36,sizeof(void*),0U));});});}}
# 307
;_pop_region(temp);}
# 316
goto _LL5;}}else{goto _LLA;}case 7U: _LL8: _tmp45=((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_tmp2A)->f2;_LL9: {
# 318
struct Cyc_Absyn_Aggrfield*_tmp38=Cyc_Absyn_lookup_field(_tmp45,f);
struct Cyc_Absyn_Exp*_tmp39=((struct Cyc_Absyn_Aggrfield*)_check_null(_tmp38))->requires_clause;
if(_tmp39 != 0){
struct _RegionHandle _tmp3A=_new_region("temp");struct _RegionHandle*temp=& _tmp3A;_push_region(temp);
if(!({struct Cyc_List_List*_tmp64B=_tmp46;Cyc_NewControlFlow_relns_ok(_tmp64B,Cyc_Relations_exp2relns(temp,_tmp39));})){
({struct Cyc_String_pa_PrintArg_struct _tmp3D=({struct Cyc_String_pa_PrintArg_struct _tmp5A0;_tmp5A0.tag=0U,({struct _dyneither_ptr _tmp64C=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_exp2string(Cyc_NewControlFlow_strip_cast(_tmp39)));_tmp5A0.f1=_tmp64C;});_tmp5A0;});struct Cyc_String_pa_PrintArg_struct _tmp3E=({struct Cyc_String_pa_PrintArg_struct _tmp59F;_tmp59F.tag=0U,_tmp59F.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*f);_tmp59F;});void*_tmp3B[2U];_tmp3B[0]=& _tmp3D,_tmp3B[1]=& _tmp3E;({unsigned int _tmp64E=loc;struct _dyneither_ptr _tmp64D=({const char*_tmp3C="unable to prove %s, required to access %s";_tag_dyneither(_tmp3C,sizeof(char),42U);});Cyc_CfFlowInfo_aerr(_tmp64E,_tmp64D,_tag_dyneither(_tmp3B,sizeof(void*),2U));});});
({void*_tmp3F=0U;({struct Cyc___cycFILE*_tmp650=Cyc_stderr;struct _dyneither_ptr _tmp64F=({const char*_tmp40="  [recorded facts on path: ";_tag_dyneither(_tmp40,sizeof(char),28U);});Cyc_fprintf(_tmp650,_tmp64F,_tag_dyneither(_tmp3F,sizeof(void*),0U));});});
Cyc_Relations_print_relns(Cyc_stderr,_tmp46);
({void*_tmp41=0U;({struct Cyc___cycFILE*_tmp652=Cyc_stderr;struct _dyneither_ptr _tmp651=({const char*_tmp42="]\n";_tag_dyneither(_tmp42,sizeof(char),3U);});Cyc_fprintf(_tmp652,_tmp651,_tag_dyneither(_tmp41,sizeof(void*),0U));});});}
# 322
;_pop_region(temp);}
# 329
goto _LL5;}default: _LLA: _LLB:
 goto _LL5;}_LL5:;}
# 332
goto _LL0;}_LL0:;}
# 336
static union Cyc_CfFlowInfo_FlowInfo Cyc_NewControlFlow_use_Rval(struct Cyc_NewControlFlow_AnalEnv*env,unsigned int loc,union Cyc_CfFlowInfo_FlowInfo inflow,void*r){
union Cyc_CfFlowInfo_FlowInfo _tmp47=inflow;struct Cyc_Dict_Dict _tmp4D;struct Cyc_List_List*_tmp4C;if((_tmp47.BottomFL).tag == 1){_LL1: _LL2:
 return Cyc_CfFlowInfo_BottomFL();}else{_LL3: _tmp4D=((_tmp47.ReachableFL).val).f1;_tmp4C=((_tmp47.ReachableFL).val).f2;_LL4:
# 340
 if(Cyc_CfFlowInfo_initlevel(env->fenv,_tmp4D,r)!= Cyc_CfFlowInfo_AllIL)
({void*_tmp48=0U;({unsigned int _tmp654=loc;struct _dyneither_ptr _tmp653=({const char*_tmp49="expression may not be fully initialized";_tag_dyneither(_tmp49,sizeof(char),40U);});Cyc_CfFlowInfo_aerr(_tmp654,_tmp653,_tag_dyneither(_tmp48,sizeof(void*),0U));});});{
struct Cyc_Dict_Dict _tmp4A=Cyc_CfFlowInfo_escape_deref(env->fenv,_tmp4D,r);
if(_tmp4D.t == _tmp4A.t)return inflow;{
union Cyc_CfFlowInfo_FlowInfo _tmp4B=Cyc_CfFlowInfo_ReachableFL(_tmp4A,_tmp4C);
Cyc_NewControlFlow_update_tryflow(env,_tmp4B);
return _tmp4B;};};}_LL0:;}struct _tuple22{struct Cyc_Absyn_Tqual f1;void*f2;};
# 350
static void Cyc_NewControlFlow_check_nounique(struct Cyc_NewControlFlow_AnalEnv*env,unsigned int loc,void*t,void*r){
struct _tuple0 _tmp4E=({struct _tuple0 _tmp5A1;({void*_tmp655=Cyc_Tcutil_compress(t);_tmp5A1.f1=_tmp655;}),_tmp5A1.f2=r;_tmp5A1;});struct _tuple0 _tmp4F=_tmp4E;enum Cyc_Absyn_AggrKind _tmp60;struct Cyc_List_List*_tmp5F;struct _dyneither_ptr _tmp5E;struct Cyc_List_List*_tmp5D;struct _dyneither_ptr _tmp5C;union Cyc_Absyn_AggrInfo _tmp5B;struct Cyc_List_List*_tmp5A;struct _dyneither_ptr _tmp59;struct Cyc_Absyn_Datatypefield*_tmp58;struct _dyneither_ptr _tmp57;void*_tmp56;switch(*((int*)_tmp4F.f2)){case 2U: if(((struct Cyc_CfFlowInfo_UnknownR_CfFlowInfo_AbsRVal_struct*)_tmp4F.f2)->f1 == Cyc_CfFlowInfo_NoneIL){_LL1: _LL2:
 return;}else{switch(*((int*)_tmp4F.f1)){case 0U: switch(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp4F.f1)->f1)){case 19U: if(((((struct Cyc_Absyn_DatatypeFieldCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp4F.f1)->f1)->f1).KnownDatatypefield).tag == 2)goto _LL13;else{goto _LL13;}case 20U: goto _LL13;default: goto _LL13;}case 6U: goto _LL13;case 7U: goto _LL13;case 3U: goto _LL11;default: goto _LL13;}}case 0U: _LL3: _LL4:
 return;case 7U: _LL5: _LL6:
 return;case 8U: _LL7: _tmp56=(void*)((struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*)_tmp4F.f2)->f2;_LL8:
 Cyc_NewControlFlow_check_nounique(env,loc,t,_tmp56);return;default: switch(*((int*)_tmp4F.f1)){case 0U: switch(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp4F.f1)->f1)){case 19U: if(((((struct Cyc_Absyn_DatatypeFieldCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp4F.f1)->f1)->f1).KnownDatatypefield).tag == 2){if(((struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*)_tmp4F.f2)->tag == 6U){_LL9: _tmp58=(((((struct Cyc_Absyn_DatatypeFieldCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp4F.f1)->f1)->f1).KnownDatatypefield).val).f2;_tmp57=((struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*)_tmp4F.f2)->f2;_LLA:
# 357
 if(_tmp58->typs == 0)
return;
_tmp5D=_tmp58->typs;_tmp5C=_tmp57;goto _LLC;}else{goto _LL13;}}else{goto _LL13;}case 20U: if(((struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*)_tmp4F.f2)->tag == 6U){_LLD: _tmp5B=((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp4F.f1)->f1)->f1;_tmp5A=((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp4F.f1)->f2;_tmp59=((struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*)_tmp4F.f2)->f2;_LLE: {
# 368
struct Cyc_Absyn_Aggrdecl*_tmp50=Cyc_Absyn_get_known_aggrdecl(_tmp5B);
if(_tmp50->impl == 0)return;{
struct Cyc_List_List*_tmp51=((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp50->impl))->fields;
struct _RegionHandle _tmp52=_new_region("temp");struct _RegionHandle*temp=& _tmp52;_push_region(temp);
{struct Cyc_List_List*_tmp53=((struct Cyc_List_List*(*)(struct _RegionHandle*r1,struct _RegionHandle*r2,struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_rzip)(temp,temp,_tmp50->tvs,_tmp5A);
{int i=0;for(0;i < _get_dyneither_size(_tmp59,sizeof(void*));(i ++,_tmp51=_tmp51->tl)){
void*t=((struct Cyc_Absyn_Aggrfield*)((struct Cyc_List_List*)_check_null(_tmp51))->hd)->type;
if(_tmp53 != 0)({void*_tmp656=Cyc_Tcutil_rsubstitute(temp,_tmp53,t);t=_tmp656;});
Cyc_NewControlFlow_check_nounique(env,loc,t,((void**)_tmp59.curr)[i]);}}
# 378
_npop_handler(0U);return;}
# 372
;_pop_region(temp);};}}else{goto _LL13;}default: goto _LL13;}case 6U: if(((struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*)_tmp4F.f2)->tag == 6U){_LLB: _tmp5D=((struct Cyc_Absyn_TupleType_Absyn_Type_struct*)_tmp4F.f1)->f1;_tmp5C=((struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*)_tmp4F.f2)->f2;_LLC: {
# 361
unsigned int sz=(unsigned int)((int(*)(struct Cyc_List_List*x))Cyc_List_length)(_tmp5D);
{int i=0;for(0;i < sz;(i ++,_tmp5D=_tmp5D->tl)){
({struct Cyc_NewControlFlow_AnalEnv*_tmp659=env;unsigned int _tmp658=loc;void*_tmp657=(*((struct _tuple22*)((struct Cyc_List_List*)_check_null(_tmp5D))->hd)).f2;Cyc_NewControlFlow_check_nounique(_tmp659,_tmp658,_tmp657,*((void**)_check_dyneither_subscript(_tmp5C,sizeof(void*),i)));});}}
# 365
return;}}else{goto _LL13;}case 7U: if(((struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*)_tmp4F.f2)->tag == 6U){_LLF: _tmp60=((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_tmp4F.f1)->f1;_tmp5F=((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_tmp4F.f1)->f2;_tmp5E=((struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*)_tmp4F.f2)->f2;_LL10:
# 381
{int i=0;for(0;i < _get_dyneither_size(_tmp5E,sizeof(void*));(i ++,_tmp5F=_tmp5F->tl)){
Cyc_NewControlFlow_check_nounique(env,loc,((struct Cyc_Absyn_Aggrfield*)((struct Cyc_List_List*)_check_null(_tmp5F))->hd)->type,((void**)_tmp5E.curr)[i]);}}
return;}else{goto _LL13;}case 3U: _LL11: _LL12:
# 385
 if(Cyc_Tcutil_is_noalias_pointer(t,0))
({void*_tmp54=0U;({unsigned int _tmp65B=loc;struct _dyneither_ptr _tmp65A=({const char*_tmp55="argument may still contain alias-free pointers";_tag_dyneither(_tmp55,sizeof(char),47U);});Cyc_Warn_warn(_tmp65B,_tmp65A,_tag_dyneither(_tmp54,sizeof(void*),0U));});});
return;default: _LL13: _LL14:
 return;}}_LL0:;}
# 392
static union Cyc_CfFlowInfo_FlowInfo Cyc_NewControlFlow_use_nounique_Rval(struct Cyc_NewControlFlow_AnalEnv*env,unsigned int loc,void*t,union Cyc_CfFlowInfo_FlowInfo inflow,void*r){
union Cyc_CfFlowInfo_FlowInfo _tmp61=inflow;struct Cyc_Dict_Dict _tmp6E;struct Cyc_List_List*_tmp6D;if((_tmp61.BottomFL).tag == 1){_LL1: _LL2:
 return Cyc_CfFlowInfo_BottomFL();}else{_LL3: _tmp6E=((_tmp61.ReachableFL).val).f1;_tmp6D=((_tmp61.ReachableFL).val).f2;_LL4:
# 396
 if(!Cyc_Tcutil_is_noalias_pointer(t,0))
({void*_tmp62=0U;({struct _dyneither_ptr _tmp65C=({const char*_tmp63="noliveunique attribute requires alias-free pointer";_tag_dyneither(_tmp63,sizeof(char),51U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp65C,_tag_dyneither(_tmp62,sizeof(void*),0U));});});{
void*_tmp64=Cyc_Tcutil_pointer_elt_type(t);
retry: {void*_tmp65=r;void*_tmp6A;struct Cyc_CfFlowInfo_Place*_tmp69;void*_tmp68;switch(*((int*)_tmp65)){case 8U: _LL6: _tmp68=(void*)((struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*)_tmp65)->f2;_LL7:
 r=_tmp68;goto retry;case 4U: _LL8: _tmp69=((struct Cyc_CfFlowInfo_AddressOf_CfFlowInfo_AbsRVal_struct*)_tmp65)->f1;_LL9:
({void*_tmp65D=Cyc_CfFlowInfo_lookup_place(_tmp6E,_tmp69);_tmp6A=_tmp65D;});goto _LLB;case 5U: _LLA: _tmp6A=(void*)((struct Cyc_CfFlowInfo_UniquePtr_CfFlowInfo_AbsRVal_struct*)_tmp65)->f1;_LLB:
 Cyc_NewControlFlow_check_nounique(env,loc,_tmp64,_tmp6A);goto _LL5;default: _LLC: _LLD:
# 404
 if(Cyc_Tcutil_is_noalias_pointer_or_aggr(_tmp64))
({void*_tmp66=0U;({unsigned int _tmp65F=loc;struct _dyneither_ptr _tmp65E=({const char*_tmp67="argument may contain live alias-free pointers";_tag_dyneither(_tmp67,sizeof(char),46U);});Cyc_Warn_warn(_tmp65F,_tmp65E,_tag_dyneither(_tmp66,sizeof(void*),0U));});});
return Cyc_NewControlFlow_use_Rval(env,loc,inflow,r);}_LL5:;}{
# 408
struct Cyc_Dict_Dict _tmp6B=Cyc_CfFlowInfo_escape_deref(env->fenv,_tmp6E,r);
if(_tmp6E.t == _tmp6B.t)return inflow;{
union Cyc_CfFlowInfo_FlowInfo _tmp6C=Cyc_CfFlowInfo_ReachableFL(_tmp6B,_tmp6D);
Cyc_NewControlFlow_update_tryflow(env,_tmp6C);
return _tmp6C;};};};}_LL0:;}struct _tuple23{union Cyc_CfFlowInfo_FlowInfo f1;struct Cyc_List_List*f2;};
# 418
static struct _tuple23 Cyc_NewControlFlow_anal_Rexps(struct Cyc_NewControlFlow_AnalEnv*env,union Cyc_CfFlowInfo_FlowInfo inflow,struct Cyc_List_List*es,int first_is_copy,int others_are_copy){
# 423
struct Cyc_List_List*rvals=0;
if(es == 0)
return({struct _tuple23 _tmp5A2;_tmp5A2.f1=inflow,_tmp5A2.f2=0;_tmp5A2;});
do{
struct _tuple16 _tmp6F=Cyc_NewControlFlow_anal_Rexp(env,first_is_copy,inflow,(struct Cyc_Absyn_Exp*)es->hd);struct _tuple16 _tmp70=_tmp6F;union Cyc_CfFlowInfo_FlowInfo _tmp73;void*_tmp72;_LL1: _tmp73=_tmp70.f1;_tmp72=_tmp70.f2;_LL2:;
inflow=_tmp73;
({struct Cyc_List_List*_tmp660=({struct Cyc_List_List*_tmp71=_cycalloc(sizeof(*_tmp71));_tmp71->hd=_tmp72,_tmp71->tl=rvals;_tmp71;});rvals=_tmp660;});
es=es->tl;
first_is_copy=others_are_copy;}while(es != 0);
# 434
Cyc_NewControlFlow_update_tryflow(env,inflow);
return({struct _tuple23 _tmp5A3;_tmp5A3.f1=inflow,({struct Cyc_List_List*_tmp661=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(rvals);_tmp5A3.f2=_tmp661;});_tmp5A3;});}
# 440
static struct _tuple16 Cyc_NewControlFlow_anal_use_ints(struct Cyc_NewControlFlow_AnalEnv*env,union Cyc_CfFlowInfo_FlowInfo inflow,struct Cyc_List_List*es){
# 442
struct _tuple23 _tmp74=
Cyc_NewControlFlow_anal_Rexps(env,inflow,es,0,0);
# 442
struct _tuple23 _tmp75=_tmp74;union Cyc_CfFlowInfo_FlowInfo _tmp7B;struct Cyc_List_List*_tmp7A;_LL1: _tmp7B=_tmp75.f1;_tmp7A=_tmp75.f2;_LL2:;
# 444
{union Cyc_CfFlowInfo_FlowInfo _tmp76=_tmp7B;struct Cyc_Dict_Dict _tmp79;if((_tmp76.ReachableFL).tag == 2){_LL4: _tmp79=((_tmp76.ReachableFL).val).f1;_LL5:
# 446
 for(0;_tmp7A != 0;(_tmp7A=_tmp7A->tl,es=((struct Cyc_List_List*)_check_null(es))->tl)){
if(Cyc_CfFlowInfo_initlevel(env->fenv,_tmp79,(void*)_tmp7A->hd)== Cyc_CfFlowInfo_NoneIL)
({void*_tmp77=0U;({unsigned int _tmp663=((struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)_check_null(es))->hd)->loc;struct _dyneither_ptr _tmp662=({const char*_tmp78="expression may not be initialized";_tag_dyneither(_tmp78,sizeof(char),34U);});Cyc_CfFlowInfo_aerr(_tmp663,_tmp662,_tag_dyneither(_tmp77,sizeof(void*),0U));});});}
goto _LL3;}else{_LL6: _LL7:
 goto _LL3;}_LL3:;}
# 452
return({struct _tuple16 _tmp5A4;_tmp5A4.f1=_tmp7B,_tmp5A4.f2=(env->fenv)->unknown_all;_tmp5A4;});}
# 459
static void*Cyc_NewControlFlow_consume_zero_rval(struct Cyc_NewControlFlow_AnalEnv*env,struct Cyc_Dict_Dict new_dict,struct Cyc_CfFlowInfo_Place*p,struct Cyc_Absyn_Exp*e,void*new_rval){
# 466
int needs_unconsume=0;
void*_tmp7C=Cyc_CfFlowInfo_lookup_place(new_dict,p);
if(Cyc_CfFlowInfo_is_unique_consumed(e,env->iteration_num,_tmp7C,& needs_unconsume))
({void*_tmp7D=0U;({struct _dyneither_ptr _tmp664=({const char*_tmp7E="consume_zero_rval";_tag_dyneither(_tmp7E,sizeof(char),18U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp664,_tag_dyneither(_tmp7D,sizeof(void*),0U));});});else{
# 471
if(needs_unconsume)
return Cyc_CfFlowInfo_make_unique_consumed(env->fenv,(void*)_check_null(e->topt),e,env->iteration_num,new_rval);else{
# 475
return new_rval;}}}
# 488 "new_control_flow.cyc"
static union Cyc_CfFlowInfo_FlowInfo Cyc_NewControlFlow_notzero(struct Cyc_NewControlFlow_AnalEnv*env,union Cyc_CfFlowInfo_FlowInfo inflow,union Cyc_CfFlowInfo_FlowInfo outflow,struct Cyc_Absyn_Exp*e,enum Cyc_CfFlowInfo_InitLevel il,struct Cyc_List_List*names){
# 493
union Cyc_CfFlowInfo_FlowInfo _tmp7F=outflow;struct Cyc_Dict_Dict _tmp86;struct Cyc_List_List*_tmp85;if((_tmp7F.BottomFL).tag == 1){_LL1: _LL2:
 return outflow;}else{_LL3: _tmp86=((_tmp7F.ReachableFL).val).f1;_tmp85=((_tmp7F.ReachableFL).val).f2;_LL4: {
# 496
union Cyc_CfFlowInfo_AbsLVal _tmp80=(Cyc_NewControlFlow_anal_Lexp(env,inflow,0,0,e)).f2;union Cyc_CfFlowInfo_AbsLVal _tmp81=_tmp80;struct Cyc_CfFlowInfo_Place*_tmp84;if((_tmp81.UnknownL).tag == 2){_LL6: _LL7:
# 500
 return outflow;}else{_LL8: _tmp84=(_tmp81.PlaceL).val;_LL9: {
# 504
void*nzval=il == Cyc_CfFlowInfo_AllIL?(env->fenv)->notzeroall:(env->fenv)->unknown_none;
for(0;names != 0;names=names->tl){
({void*_tmp665=(void*)({struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*_tmp82=_cycalloc(sizeof(*_tmp82));_tmp82->tag=8U,_tmp82->f1=(struct Cyc_Absyn_Vardecl*)names->hd,_tmp82->f2=nzval;_tmp82;});nzval=_tmp665;});}
# 508
({void*_tmp666=Cyc_NewControlFlow_consume_zero_rval(env,_tmp86,_tmp84,e,nzval);nzval=_tmp666;});{
union Cyc_CfFlowInfo_FlowInfo _tmp83=({struct Cyc_Dict_Dict _tmp667=Cyc_CfFlowInfo_assign_place(env->fenv,e->loc,_tmp86,_tmp84,nzval);Cyc_CfFlowInfo_ReachableFL(_tmp667,_tmp85);});
# 513
return _tmp83;};}}_LL5:;}}_LL0:;}
# 522
static struct _tuple19 Cyc_NewControlFlow_splitzero(struct Cyc_NewControlFlow_AnalEnv*env,union Cyc_CfFlowInfo_FlowInfo inflow,union Cyc_CfFlowInfo_FlowInfo outflow,struct Cyc_Absyn_Exp*e,enum Cyc_CfFlowInfo_InitLevel il,struct Cyc_List_List*names){
# 524
union Cyc_CfFlowInfo_FlowInfo _tmp87=outflow;struct Cyc_Dict_Dict _tmp8E;struct Cyc_List_List*_tmp8D;if((_tmp87.BottomFL).tag == 1){_LL1: _LL2:
 return({struct _tuple19 _tmp5A5;_tmp5A5.f1=outflow,_tmp5A5.f2=outflow;_tmp5A5;});}else{_LL3: _tmp8E=((_tmp87.ReachableFL).val).f1;_tmp8D=((_tmp87.ReachableFL).val).f2;_LL4: {
# 527
union Cyc_CfFlowInfo_AbsLVal _tmp88=(Cyc_NewControlFlow_anal_Lexp(env,inflow,0,0,e)).f2;union Cyc_CfFlowInfo_AbsLVal _tmp89=_tmp88;struct Cyc_CfFlowInfo_Place*_tmp8C;if((_tmp89.UnknownL).tag == 2){_LL6: _LL7:
 return({struct _tuple19 _tmp5A6;_tmp5A6.f1=outflow,_tmp5A6.f2=outflow;_tmp5A6;});}else{_LL8: _tmp8C=(_tmp89.PlaceL).val;_LL9: {
# 530
void*nzval=il == Cyc_CfFlowInfo_AllIL?(env->fenv)->notzeroall:(env->fenv)->unknown_none;
void*zval=(env->fenv)->zero;
for(0;names != 0;names=names->tl){
({void*_tmp668=(void*)({struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*_tmp8A=_cycalloc(sizeof(*_tmp8A));_tmp8A->tag=8U,_tmp8A->f1=(struct Cyc_Absyn_Vardecl*)names->hd,_tmp8A->f2=nzval;_tmp8A;});nzval=_tmp668;});
({void*_tmp669=(void*)({struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*_tmp8B=_cycalloc(sizeof(*_tmp8B));_tmp8B->tag=8U,_tmp8B->f1=(struct Cyc_Absyn_Vardecl*)names->hd,_tmp8B->f2=zval;_tmp8B;});zval=_tmp669;});}
# 536
({void*_tmp66A=Cyc_NewControlFlow_consume_zero_rval(env,_tmp8E,_tmp8C,e,nzval);nzval=_tmp66A;});
({void*_tmp66B=Cyc_NewControlFlow_consume_zero_rval(env,_tmp8E,_tmp8C,e,zval);zval=_tmp66B;});
return({struct _tuple19 _tmp5A7;({
union Cyc_CfFlowInfo_FlowInfo _tmp66F=({struct Cyc_Dict_Dict _tmp66E=Cyc_CfFlowInfo_assign_place(env->fenv,e->loc,_tmp8E,_tmp8C,nzval);Cyc_CfFlowInfo_ReachableFL(_tmp66E,_tmp8D);});_tmp5A7.f1=_tmp66F;}),({
union Cyc_CfFlowInfo_FlowInfo _tmp66D=({struct Cyc_Dict_Dict _tmp66C=Cyc_CfFlowInfo_assign_place(env->fenv,e->loc,_tmp8E,_tmp8C,zval);Cyc_CfFlowInfo_ReachableFL(_tmp66C,_tmp8D);});_tmp5A7.f2=_tmp66D;});_tmp5A7;});}}_LL5:;}}_LL0:;}
# 545
static struct Cyc_CfFlowInfo_NotZero_Absyn_AbsynAnnot_struct Cyc_NewControlFlow_mt_notzero_v={Cyc_CfFlowInfo_NotZero,0};
static struct Cyc_CfFlowInfo_UnknownZ_Absyn_AbsynAnnot_struct Cyc_NewControlFlow_mt_unknownz_v={Cyc_CfFlowInfo_UnknownZ,0};
# 548
static void Cyc_NewControlFlow_update_relns(struct Cyc_Absyn_Exp*e,int possibly_null,struct Cyc_List_List*relns){
void*_tmp8F=e->r;void*_tmp90=_tmp8F;if(((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmp90)->tag == 23U){_LL1: _LL2:
# 552
{void*_tmp91=e->annot;void*_tmp92=_tmp91;struct Cyc_List_List*_tmp94;struct Cyc_List_List*_tmp93;if(((struct Cyc_CfFlowInfo_UnknownZ_Absyn_AbsynAnnot_struct*)_tmp92)->tag == Cyc_CfFlowInfo_UnknownZ){_LL6: _tmp93=((struct Cyc_CfFlowInfo_UnknownZ_Absyn_AbsynAnnot_struct*)_tmp92)->f1;_LL7:
# 554
 if(possibly_null  && Cyc_Relations_same_relns(relns,_tmp93))return;
goto _LL5;}else{if(((struct Cyc_CfFlowInfo_NotZero_Absyn_AbsynAnnot_struct*)_tmp92)->tag == Cyc_CfFlowInfo_NotZero){_LL8: _tmp94=((struct Cyc_CfFlowInfo_NotZero_Absyn_AbsynAnnot_struct*)_tmp92)->f1;_LL9:
# 557
 possibly_null=0;
if(Cyc_Relations_same_relns(relns,_tmp94))return;
goto _LL5;}else{_LLA: _LLB:
 goto _LL5;}}_LL5:;}
# 562
if(possibly_null)
({void*_tmp670=(void*)({struct Cyc_CfFlowInfo_UnknownZ_Absyn_AbsynAnnot_struct*_tmp95=_cycalloc(sizeof(*_tmp95));_tmp95->tag=Cyc_CfFlowInfo_UnknownZ,_tmp95->f1=relns;_tmp95;});e->annot=_tmp670;});else{
# 566
({void*_tmp671=(void*)({struct Cyc_CfFlowInfo_NotZero_Absyn_AbsynAnnot_struct*_tmp96=_cycalloc(sizeof(*_tmp96));_tmp96->tag=Cyc_CfFlowInfo_NotZero,_tmp96->f1=relns;_tmp96;});e->annot=_tmp671;});}
# 568
goto _LL0;}else{_LL3: _LL4:
# 572
 if(possibly_null){
void*_tmp97=e->annot;void*_tmp98=_tmp97;struct Cyc_List_List*_tmp99;if(((struct Cyc_CfFlowInfo_UnknownZ_Absyn_AbsynAnnot_struct*)_tmp98)->tag == Cyc_CfFlowInfo_UnknownZ){_LLD: _tmp99=((struct Cyc_CfFlowInfo_UnknownZ_Absyn_AbsynAnnot_struct*)_tmp98)->f1;_LLE:
# 575
 e->annot=(void*)& Cyc_NewControlFlow_mt_unknownz_v;
return;}else{_LLF: _LL10:
 goto _LLC;}_LLC:;}
# 580
e->annot=(void*)& Cyc_NewControlFlow_mt_notzero_v;
goto _LL0;}_LL0:;}
# 590
static struct _tuple16 Cyc_NewControlFlow_anal_derefR(struct Cyc_NewControlFlow_AnalEnv*env,union Cyc_CfFlowInfo_FlowInfo inflow,union Cyc_CfFlowInfo_FlowInfo f,struct Cyc_Absyn_Exp*e,void*r){
# 593
void*_tmp9A=Cyc_Tcutil_compress((void*)_check_null(e->topt));void*_tmp9B=_tmp9A;void*_tmpBF;void*_tmpBE;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp9B)->tag == 3U){_LL1: _tmpBF=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp9B)->f1).elt_type;_tmpBE=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp9B)->f1).ptr_atts).bounds;_LL2: {
# 595
union Cyc_CfFlowInfo_FlowInfo _tmp9C=f;struct Cyc_Dict_Dict _tmpBB;struct Cyc_List_List*_tmpBA;if((_tmp9C.BottomFL).tag == 1){_LL6: _LL7:
# 597
 return({struct _tuple16 _tmp5A8;_tmp5A8.f1=f,({void*_tmp672=Cyc_CfFlowInfo_typ_to_absrval(env->fenv,_tmpBF,0,(env->fenv)->unknown_all);_tmp5A8.f2=_tmp672;});_tmp5A8;});}else{_LL8: _tmpBB=((_tmp9C.ReachableFL).val).f1;_tmpBA=((_tmp9C.ReachableFL).val).f2;_LL9:
# 600
 if(Cyc_Tcutil_is_bound_one(_tmpBE)){
struct _tuple15 _tmp9D=Cyc_CfFlowInfo_unname_rval(r);struct _tuple15 _tmp9E=_tmp9D;void*_tmpAB;struct Cyc_List_List*_tmpAA;_LLB: _tmpAB=_tmp9E.f1;_tmpAA=_tmp9E.f2;_LLC:;{
void*_tmp9F=_tmpAB;enum Cyc_CfFlowInfo_InitLevel _tmpA9;void*_tmpA8;struct Cyc_CfFlowInfo_Place*_tmpA7;struct Cyc_Absyn_Vardecl*_tmpA6;void*_tmpA5;switch(*((int*)_tmp9F)){case 8U: _LLE: _tmpA6=((struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*)_tmp9F)->f1;_tmpA5=(void*)((struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*)_tmp9F)->f2;_LLF:
# 604
({void*_tmpA0=0U;({struct _dyneither_ptr _tmp673=({const char*_tmpA1="named location in anal_derefR";_tag_dyneither(_tmpA1,sizeof(char),30U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp673,_tag_dyneither(_tmpA0,sizeof(void*),0U));});});case 1U: _LL10: _LL11:
# 606
 Cyc_NewControlFlow_update_relns(e,0,_tmpBA);
goto _LLD;case 4U: _LL12: _tmpA7=((struct Cyc_CfFlowInfo_AddressOf_CfFlowInfo_AbsRVal_struct*)_tmp9F)->f1;_LL13:
# 609
 Cyc_NewControlFlow_update_relns(e,0,_tmpBA);
return({struct _tuple16 _tmp5A9;_tmp5A9.f1=f,({void*_tmp674=Cyc_CfFlowInfo_lookup_place(_tmpBB,_tmpA7);_tmp5A9.f2=_tmp674;});_tmp5A9;});case 5U: _LL14: _tmpA8=(void*)((struct Cyc_CfFlowInfo_UniquePtr_CfFlowInfo_AbsRVal_struct*)_tmp9F)->f1;_LL15:
# 612
 Cyc_NewControlFlow_update_relns(e,1,_tmpBA);
return({struct _tuple16 _tmp5AA;_tmp5AA.f1=f,_tmp5AA.f2=_tmpA8;_tmp5AA;});case 0U: _LL16: _LL17:
# 615
 e->annot=(void*)& Cyc_CfFlowInfo_IsZero_val;
return({struct _tuple16 _tmp5AB;({union Cyc_CfFlowInfo_FlowInfo _tmp676=Cyc_CfFlowInfo_BottomFL();_tmp5AB.f1=_tmp676;}),({void*_tmp675=Cyc_CfFlowInfo_typ_to_absrval(env->fenv,_tmpBF,0,(env->fenv)->unknown_all);_tmp5AB.f2=_tmp675;});_tmp5AB;});case 2U: _LL18: _tmpA9=((struct Cyc_CfFlowInfo_UnknownR_CfFlowInfo_AbsRVal_struct*)_tmp9F)->f1;_LL19:
# 619
({union Cyc_CfFlowInfo_FlowInfo _tmp677=Cyc_NewControlFlow_notzero(env,inflow,f,e,_tmpA9,_tmpAA);f=_tmp677;});
goto _LL1B;default: _LL1A: _LL1B:
# 622
{void*_tmpA2=e->r;void*_tmpA3=_tmpA2;if(((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmpA3)->tag == 23U){_LL1D: _LL1E:
# 624
({void*_tmp678=(void*)({struct Cyc_CfFlowInfo_UnknownZ_Absyn_AbsynAnnot_struct*_tmpA4=_cycalloc(sizeof(*_tmpA4));_tmpA4->tag=Cyc_CfFlowInfo_UnknownZ,_tmpA4->f1=_tmpBA;_tmpA4;});e->annot=_tmp678;});
goto _LL1C;}else{_LL1F: _LL20:
# 627
 e->annot=(void*)& Cyc_NewControlFlow_mt_unknownz_v;
goto _LL1C;}_LL1C:;}
# 630
goto _LLD;}_LLD:;};}else{
# 633
void*_tmpAC=e->annot;void*_tmpAD=_tmpAC;struct Cyc_List_List*_tmpAF;if(((struct Cyc_CfFlowInfo_UnknownZ_Absyn_AbsynAnnot_struct*)_tmpAD)->tag == Cyc_CfFlowInfo_UnknownZ){_LL22: _tmpAF=((struct Cyc_CfFlowInfo_UnknownZ_Absyn_AbsynAnnot_struct*)_tmpAD)->f1;_LL23:
# 635
 if(!Cyc_Relations_same_relns(_tmpBA,_tmpAF))goto _LL25;
goto _LL21;}else{_LL24: _LL25:
# 638
({void*_tmp679=(void*)({struct Cyc_CfFlowInfo_UnknownZ_Absyn_AbsynAnnot_struct*_tmpAE=_cycalloc(sizeof(*_tmpAE));_tmpAE->tag=Cyc_CfFlowInfo_UnknownZ,_tmpAE->f1=_tmpBA;_tmpAE;});e->annot=_tmp679;});
goto _LL21;}_LL21:;}{
# 642
enum Cyc_CfFlowInfo_InitLevel _tmpB0=Cyc_CfFlowInfo_initlevel(env->fenv,_tmpBB,r);enum Cyc_CfFlowInfo_InitLevel _tmpB1=_tmpB0;if(_tmpB1 == Cyc_CfFlowInfo_NoneIL){_LL27: _LL28: {
# 644
struct _tuple15 _tmpB2=Cyc_CfFlowInfo_unname_rval(r);struct _tuple15 _tmpB3=_tmpB2;void*_tmpB9;_LL2C: _tmpB9=_tmpB3.f1;_LL2D:;
{void*_tmpB4=_tmpB9;if(((struct Cyc_CfFlowInfo_Consumed_CfFlowInfo_AbsRVal_struct*)_tmpB4)->tag == 7U){_LL2F: _LL30:
# 647
({void*_tmpB5=0U;({unsigned int _tmp67B=e->loc;struct _dyneither_ptr _tmp67A=({const char*_tmpB6="attempt to dereference a consumed alias-free pointer";_tag_dyneither(_tmpB6,sizeof(char),53U);});Cyc_CfFlowInfo_aerr(_tmp67B,_tmp67A,_tag_dyneither(_tmpB5,sizeof(void*),0U));});});
goto _LL2E;}else{_LL31: _LL32:
# 650
({void*_tmpB7=0U;({unsigned int _tmp67D=e->loc;struct _dyneither_ptr _tmp67C=({const char*_tmpB8="dereference of possibly uninitialized pointer";_tag_dyneither(_tmpB8,sizeof(char),46U);});Cyc_CfFlowInfo_aerr(_tmp67D,_tmp67C,_tag_dyneither(_tmpB7,sizeof(void*),0U));});});}_LL2E:;}
# 652
goto _LL2A;}}else{_LL29: _LL2A:
# 654
 return({struct _tuple16 _tmp5AC;_tmp5AC.f1=f,({void*_tmp67E=Cyc_CfFlowInfo_typ_to_absrval(env->fenv,_tmpBF,0,(env->fenv)->unknown_all);_tmp5AC.f2=_tmp67E;});_tmp5AC;});}_LL26:;};}_LL5:;}}else{_LL3: _LL4:
# 657
({void*_tmpBC=0U;({struct _dyneither_ptr _tmp67F=({const char*_tmpBD="right deref of non-pointer-type";_tag_dyneither(_tmpBD,sizeof(char),32U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp67F,_tag_dyneither(_tmpBC,sizeof(void*),0U));});});}_LL0:;}
# 664
static struct Cyc_List_List*Cyc_NewControlFlow_add_subscript_reln(struct Cyc_List_List*relns,struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2){
# 666
union Cyc_Relations_RelnOp n2=Cyc_Relations_RConst(0U);
int e2_valid_op=Cyc_Relations_exp2relnop(e2,& n2);
# 669
{void*_tmpC0=e1->r;void*_tmpC1=_tmpC0;void*_tmpC3;if(((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmpC1)->tag == 1U){_LL1: _tmpC3=(void*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmpC1)->f1;_LL2: {
# 671
struct Cyc_Absyn_Vardecl*_tmpC2=Cyc_Tcutil_nonesc_vardecl(_tmpC3);
if(_tmpC2 != 0){
union Cyc_Relations_RelnOp n1=Cyc_Relations_RNumelts(_tmpC2);
if(e2_valid_op)
({struct Cyc_List_List*_tmp680=Cyc_Relations_add_relation(Cyc_Core_heap_region,n2,Cyc_Relations_Rlt,n1,relns);relns=_tmp680;});}
# 677
goto _LL0;}}else{_LL3: _LL4:
 goto _LL0;}_LL0:;}{
# 681
struct Cyc_Absyn_Exp*bound=Cyc_Tcutil_get_type_bound((void*)_check_null(e1->topt));
if(bound != 0){
union Cyc_Relations_RelnOp rbound=Cyc_Relations_RConst(0U);
if(Cyc_Relations_exp2relnop(bound,& rbound))
({struct Cyc_List_List*_tmp681=Cyc_Relations_add_relation(Cyc_Core_heap_region,n2,Cyc_Relations_Rlt,rbound,relns);relns=_tmp681;});}
# 687
return relns;};}
# 695
static union Cyc_CfFlowInfo_FlowInfo Cyc_NewControlFlow_restore_noconsume_arg(struct Cyc_NewControlFlow_AnalEnv*env,union Cyc_CfFlowInfo_FlowInfo inflow,struct Cyc_Absyn_Exp*exp,unsigned int loc,void*old_rval){
# 700
struct _tuple18 _tmpC4=Cyc_NewControlFlow_anal_Lexp(env,inflow,1,1,exp);struct _tuple18 _tmpC5=_tmpC4;union Cyc_CfFlowInfo_AbsLVal _tmpD4;_LL1: _tmpD4=_tmpC5.f2;_LL2:;
{struct _tuple18 _tmpC6=({struct _tuple18 _tmp5AE;_tmp5AE.f1=inflow,_tmp5AE.f2=_tmpD4;_tmp5AE;});struct _tuple18 _tmpC7=_tmpC6;struct Cyc_Dict_Dict _tmpD3;struct Cyc_List_List*_tmpD2;struct Cyc_CfFlowInfo_Place*_tmpD1;if(((_tmpC7.f1).ReachableFL).tag == 2){if(((_tmpC7.f2).PlaceL).tag == 1){_LL4: _tmpD3=(((_tmpC7.f1).ReachableFL).val).f1;_tmpD2=(((_tmpC7.f1).ReachableFL).val).f2;_tmpD1=((_tmpC7.f2).PlaceL).val;_LL5: {
# 703
void*_tmpC8=Cyc_CfFlowInfo_typ_to_absrval(env->fenv,(void*)_check_null(exp->topt),0,(env->fenv)->unknown_all);
# 705
struct _tuple15 _tmpC9=Cyc_CfFlowInfo_unname_rval(old_rval);struct _tuple15 _tmpCA=_tmpC9;void*_tmpCD;struct Cyc_List_List*_tmpCC;_LLB: _tmpCD=_tmpCA.f1;_tmpCC=_tmpCA.f2;_LLC:;
for(0;_tmpCC != 0;_tmpCC=_tmpCC->tl){
({void*_tmp682=(void*)({struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*_tmpCB=_cycalloc(sizeof(*_tmpCB));_tmpCB->tag=8U,_tmpCB->f1=(struct Cyc_Absyn_Vardecl*)_tmpCC->hd,_tmpCB->f2=_tmpC8;_tmpCB;});_tmpC8=_tmp682;});}
# 710
({struct Cyc_Dict_Dict _tmp683=Cyc_CfFlowInfo_assign_place(env->fenv,loc,_tmpD3,_tmpD1,_tmpC8);_tmpD3=_tmp683;});
({union Cyc_CfFlowInfo_FlowInfo _tmp684=Cyc_CfFlowInfo_ReachableFL(_tmpD3,_tmpD2);inflow=_tmp684;});
Cyc_NewControlFlow_update_tryflow(env,inflow);
# 716
goto _LL3;}}else{_LL8: _LL9:
# 719
({struct Cyc_String_pa_PrintArg_struct _tmpD0=({struct Cyc_String_pa_PrintArg_struct _tmp5AD;_tmp5AD.tag=0U,({struct _dyneither_ptr _tmp685=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_exp2string(exp));_tmp5AD.f1=_tmp685;});_tmp5AD;});void*_tmpCE[1U];_tmpCE[0]=& _tmpD0;({struct _dyneither_ptr _tmp686=({const char*_tmpCF="noconsume parameter %s must be l-values";_tag_dyneither(_tmpCF,sizeof(char),40U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp686,_tag_dyneither(_tmpCE,sizeof(void*),1U));});});
goto _LL3;}}else{_LL6: _LL7:
# 717
 goto _LL3;}_LL3:;}
# 722
return inflow;}
# 727
static struct _tuple16 Cyc_NewControlFlow_do_assign(struct Cyc_CfFlowInfo_FlowEnv*fenv,struct Cyc_NewControlFlow_AnalEnv*env,union Cyc_CfFlowInfo_FlowInfo outflow,struct Cyc_Absyn_Exp*lexp,union Cyc_CfFlowInfo_AbsLVal lval,struct Cyc_Absyn_Exp*rexp,void*rval,unsigned int loc){
# 734
union Cyc_CfFlowInfo_FlowInfo _tmpD5=outflow;struct Cyc_Dict_Dict _tmpE0;struct Cyc_List_List*_tmpDF;if((_tmpD5.BottomFL).tag == 1){_LL1: _LL2:
# 736
 return({struct _tuple16 _tmp5AF;({union Cyc_CfFlowInfo_FlowInfo _tmp687=Cyc_CfFlowInfo_BottomFL();_tmp5AF.f1=_tmp687;}),_tmp5AF.f2=rval;_tmp5AF;});}else{_LL3: _tmpE0=((_tmpD5.ReachableFL).val).f1;_tmpDF=((_tmpD5.ReachableFL).val).f2;_LL4: {
# 738
union Cyc_CfFlowInfo_AbsLVal _tmpD6=lval;struct Cyc_CfFlowInfo_Place*_tmpDE;if((_tmpD6.PlaceL).tag == 1){_LL6: _tmpDE=(_tmpD6.PlaceL).val;_LL7: {
# 746
struct Cyc_Dict_Dict _tmpD7=Cyc_CfFlowInfo_assign_place(fenv,loc,_tmpE0,_tmpDE,rval);
({struct Cyc_List_List*_tmp688=Cyc_Relations_reln_assign_exp(Cyc_Core_heap_region,_tmpDF,lexp,rexp);_tmpDF=_tmp688;});
({union Cyc_CfFlowInfo_FlowInfo _tmp689=Cyc_CfFlowInfo_ReachableFL(_tmpD7,_tmpDF);outflow=_tmp689;});
if(Cyc_NewControlFlow_warn_lose_unique  && 
Cyc_Tcutil_is_noalias_pointer_or_aggr((void*)_check_null(lexp->topt))){
# 752
struct _tuple15 _tmpD8=Cyc_CfFlowInfo_unname_rval(Cyc_CfFlowInfo_lookup_place(_tmpE0,_tmpDE));struct _tuple15 _tmpD9=_tmpD8;void*_tmpDD;_LLB: _tmpDD=_tmpD9.f1;_LLC:;{
void*_tmpDA=_tmpDD;switch(*((int*)_tmpDA)){case 2U: if(((struct Cyc_CfFlowInfo_UnknownR_CfFlowInfo_AbsRVal_struct*)_tmpDA)->f1 == Cyc_CfFlowInfo_NoneIL){_LLE: _LLF:
 goto _LL11;}else{goto _LL14;}case 7U: _LL10: _LL11:
 goto _LL13;case 0U: _LL12: _LL13:
 goto _LLD;default: _LL14: _LL15:
# 758
({void*_tmpDB=0U;({unsigned int _tmp68B=lexp->loc;struct _dyneither_ptr _tmp68A=({const char*_tmpDC="assignment may overwrite alias-free pointer(s)";_tag_dyneither(_tmpDC,sizeof(char),47U);});Cyc_Warn_warn(_tmp68B,_tmp68A,_tag_dyneither(_tmpDB,sizeof(void*),0U));});});
goto _LLD;}_LLD:;};}
# 763
Cyc_NewControlFlow_update_tryflow(env,outflow);
return({struct _tuple16 _tmp5B0;_tmp5B0.f1=outflow,_tmp5B0.f2=rval;_tmp5B0;});}}else{_LL8: _LL9:
# 769
 return({struct _tuple16 _tmp5B1;({union Cyc_CfFlowInfo_FlowInfo _tmp68C=Cyc_NewControlFlow_use_Rval(env,rexp->loc,outflow,rval);_tmp5B1.f1=_tmp68C;}),_tmp5B1.f2=rval;_tmp5B1;});}_LL5:;}}_LL0:;}
# 776
static union Cyc_CfFlowInfo_FlowInfo Cyc_NewControlFlow_do_initialize_var(struct Cyc_CfFlowInfo_FlowEnv*fenv,struct Cyc_NewControlFlow_AnalEnv*env,union Cyc_CfFlowInfo_FlowInfo f,struct Cyc_Absyn_Vardecl*vd,struct Cyc_Absyn_Exp*rexp,void*rval,unsigned int loc){
# 782
union Cyc_CfFlowInfo_FlowInfo _tmpE1=f;struct Cyc_Dict_Dict _tmpE6;struct Cyc_List_List*_tmpE5;if((_tmpE1.BottomFL).tag == 1){_LL1: _LL2:
 return Cyc_CfFlowInfo_BottomFL();}else{_LL3: _tmpE6=((_tmpE1.ReachableFL).val).f1;_tmpE5=((_tmpE1.ReachableFL).val).f2;_LL4:
# 787
({struct Cyc_Dict_Dict _tmp692=({struct Cyc_CfFlowInfo_FlowEnv*_tmp691=fenv;unsigned int _tmp690=loc;struct Cyc_Dict_Dict _tmp68F=_tmpE6;struct Cyc_CfFlowInfo_Place*_tmp68E=({struct Cyc_CfFlowInfo_Place*_tmpE3=_cycalloc(sizeof(*_tmpE3));
({void*_tmp68D=(void*)({struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct*_tmpE2=_cycalloc(sizeof(*_tmpE2));_tmpE2->tag=0U,_tmpE2->f1=vd;_tmpE2;});_tmpE3->root=_tmp68D;}),_tmpE3->path=0;_tmpE3;});
# 787
Cyc_CfFlowInfo_assign_place(_tmp691,_tmp690,_tmp68F,_tmp68E,rval);});_tmpE6=_tmp692;});
# 790
({struct Cyc_List_List*_tmp693=Cyc_Relations_reln_assign_var(Cyc_Core_heap_region,_tmpE5,vd,rexp);_tmpE5=_tmp693;});{
union Cyc_CfFlowInfo_FlowInfo _tmpE4=Cyc_CfFlowInfo_ReachableFL(_tmpE6,_tmpE5);
Cyc_NewControlFlow_update_tryflow(env,_tmpE4);
# 795
return _tmpE4;};}_LL0:;}struct _tuple24{struct Cyc_Absyn_Vardecl**f1;struct Cyc_Absyn_Exp*f2;};
# 799
static union Cyc_CfFlowInfo_FlowInfo Cyc_NewControlFlow_initialize_pat_vars(struct Cyc_CfFlowInfo_FlowEnv*fenv,struct Cyc_NewControlFlow_AnalEnv*env,union Cyc_CfFlowInfo_FlowInfo inflow,struct Cyc_List_List*vds,int name_locs,unsigned int pat_loc,unsigned int exp_loc){
# 806
if(vds == 0)return inflow;{
# 809
struct Cyc_List_List*_tmpE7=((struct Cyc_List_List*(*)(struct Cyc_List_List*l))Cyc_Tcutil_filter_nulls)((((struct _tuple1(*)(struct Cyc_List_List*x))Cyc_List_split)(vds)).f1);
struct Cyc_List_List*es=0;
{struct Cyc_List_List*x=vds;for(0;x != 0;x=x->tl){
if((*((struct _tuple24*)x->hd)).f1 == 0)({struct Cyc_List_List*_tmp694=({struct Cyc_List_List*_tmpE8=_cycalloc(sizeof(*_tmpE8));_tmpE8->hd=(struct Cyc_Absyn_Exp*)_check_null((*((struct _tuple24*)x->hd)).f2),_tmpE8->tl=es;_tmpE8;});es=_tmp694;});}}
# 815
({union Cyc_CfFlowInfo_FlowInfo _tmp695=Cyc_NewControlFlow_add_vars(fenv,inflow,_tmpE7,fenv->unknown_all,pat_loc,name_locs);inflow=_tmp695;});
# 817
({union Cyc_CfFlowInfo_FlowInfo _tmp696=Cyc_NewControlFlow_expand_unique_places(env,inflow,es);inflow=_tmp696;});
{struct Cyc_List_List*x=es;for(0;x != 0;x=x->tl){
# 821
struct _tuple16 _tmpE9=Cyc_NewControlFlow_anal_Rexp(env,1,inflow,(struct Cyc_Absyn_Exp*)x->hd);struct _tuple16 _tmpEA=_tmpE9;union Cyc_CfFlowInfo_FlowInfo _tmpEC;void*_tmpEB;_LL1: _tmpEC=_tmpEA.f1;_tmpEB=_tmpEA.f2;_LL2:;
({union Cyc_CfFlowInfo_FlowInfo _tmp697=Cyc_NewControlFlow_use_Rval(env,exp_loc,_tmpEC,_tmpEB);inflow=_tmp697;});}}{
# 829
struct Cyc_List_List*_tmpED=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_rev)(vds);
for(0;_tmpED != 0;_tmpED=_tmpED->tl){
struct _tuple24*_tmpEE=(struct _tuple24*)_tmpED->hd;struct _tuple24*_tmpEF=_tmpEE;struct Cyc_Absyn_Vardecl**_tmp107;struct Cyc_Absyn_Exp*_tmp106;_LL4: _tmp107=_tmpEF->f1;_tmp106=_tmpEF->f2;_LL5:;
if(_tmp107 != 0  && _tmp106 != 0){
if(_tmp106->topt == 0)
({struct Cyc_String_pa_PrintArg_struct _tmpF2=({struct Cyc_String_pa_PrintArg_struct _tmp5B2;_tmp5B2.tag=0U,({
struct _dyneither_ptr _tmp698=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_exp2string(_tmp106));_tmp5B2.f1=_tmp698;});_tmp5B2;});void*_tmpF0[1U];_tmpF0[0]=& _tmpF2;({struct _dyneither_ptr _tmp699=({const char*_tmpF1="oops! pattern init expr %s has no type!\n";_tag_dyneither(_tmpF1,sizeof(char),41U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp699,_tag_dyneither(_tmpF0,sizeof(void*),1U));});});{
# 843
struct Cyc_List_List l=({struct Cyc_List_List _tmp5B4;_tmp5B4.hd=_tmp106,_tmp5B4.tl=0;_tmp5B4;});
union Cyc_CfFlowInfo_FlowInfo _tmpF3=Cyc_NewControlFlow_expand_unique_places(env,inflow,& l);
struct _tuple18 _tmpF4=Cyc_NewControlFlow_anal_Lexp(env,_tmpF3,0,0,_tmp106);struct _tuple18 _tmpF5=_tmpF4;union Cyc_CfFlowInfo_AbsLVal _tmp105;_LL7: _tmp105=_tmpF5.f2;_LL8:;{
struct _tuple16 _tmpF6=Cyc_NewControlFlow_anal_Rexp(env,1,_tmpF3,_tmp106);struct _tuple16 _tmpF7=_tmpF6;union Cyc_CfFlowInfo_FlowInfo _tmp104;void*_tmp103;_LLA: _tmp104=_tmpF7.f1;_tmp103=_tmpF7.f2;_LLB:;{
union Cyc_CfFlowInfo_FlowInfo _tmpF8=_tmp104;struct Cyc_Dict_Dict _tmp102;struct Cyc_List_List*_tmp101;if((_tmpF8.ReachableFL).tag == 2){_LLD: _tmp102=((_tmpF8.ReachableFL).val).f1;_tmp101=((_tmpF8.ReachableFL).val).f2;_LLE:
# 849
 if(name_locs){
union Cyc_CfFlowInfo_AbsLVal _tmpF9=_tmp105;struct Cyc_CfFlowInfo_Place*_tmp100;if((_tmpF9.PlaceL).tag == 1){_LL12: _tmp100=(_tmpF9.PlaceL).val;_LL13:
# 852
({void*_tmp69A=(void*)({struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*_tmpFA=_cycalloc(sizeof(*_tmpFA));_tmpFA->tag=8U,_tmpFA->f1=*_tmp107,_tmpFA->f2=_tmp103;_tmpFA;});_tmp103=_tmp69A;});{
# 855
void*_tmpFB=Cyc_CfFlowInfo_lookup_place(_tmp102,_tmp100);
({void*_tmp69B=(void*)({struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*_tmpFC=_cycalloc(sizeof(*_tmpFC));_tmpFC->tag=8U,_tmpFC->f1=*_tmp107,_tmpFC->f2=_tmpFB;_tmpFC;});_tmpFB=_tmp69B;});
({struct Cyc_Dict_Dict _tmp69C=Cyc_CfFlowInfo_assign_place(fenv,exp_loc,_tmp102,_tmp100,_tmpFB);_tmp102=_tmp69C;});
({union Cyc_CfFlowInfo_FlowInfo _tmp69D=Cyc_CfFlowInfo_ReachableFL(_tmp102,_tmp101);_tmp104=_tmp69D;});
goto _LL11;};}else{_LL14: _LL15:
# 862
 if(Cyc_Tcutil_is_noalias_pointer_or_aggr((void*)_check_null(_tmp106->topt)) && !
Cyc_Tcutil_is_noalias_pointer_or_aggr((*_tmp107)->type))
({struct Cyc_String_pa_PrintArg_struct _tmpFF=({struct Cyc_String_pa_PrintArg_struct _tmp5B3;_tmp5B3.tag=0U,({
struct _dyneither_ptr _tmp69E=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_exp2string(_tmp106));_tmp5B3.f1=_tmp69E;});_tmp5B3;});void*_tmpFD[1U];_tmpFD[0]=& _tmpFF;({unsigned int _tmp6A0=exp_loc;struct _dyneither_ptr _tmp69F=({const char*_tmpFE="aliased pattern expression not an l-value: %s";_tag_dyneither(_tmpFE,sizeof(char),46U);});Cyc_CfFlowInfo_aerr(_tmp6A0,_tmp69F,_tag_dyneither(_tmpFD,sizeof(void*),1U));});});}_LL11:;}
# 872
({union Cyc_CfFlowInfo_FlowInfo _tmp6A1=Cyc_NewControlFlow_do_initialize_var(fenv,env,_tmp104,*_tmp107,_tmp106,_tmp103,pat_loc);inflow=_tmp6A1;});
goto _LLC;}else{_LLF: _LL10:
# 875
 goto _LLC;}_LLC:;};};};}}
# 880
return inflow;};};}
# 883
static int Cyc_NewControlFlow_is_local_var_rooted_path(struct Cyc_Absyn_Exp*e,int cast_ok){
# 885
void*_tmp108=e->r;void*_tmp109=_tmp108;struct Cyc_Absyn_Exp*_tmp110;struct Cyc_Absyn_Exp*_tmp10F;struct Cyc_Absyn_Exp*_tmp10E;struct Cyc_Absyn_Exp*_tmp10D;struct Cyc_Absyn_Exp*_tmp10C;switch(*((int*)_tmp109)){case 1U: switch(*((int*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp109)->f1)){case 4U: _LL1: _LL2:
 goto _LL4;case 3U: _LL3: _LL4:
 goto _LL6;case 5U: _LL5: _LL6:
 return 1;default: goto _LL11;}case 20U: _LL7: _tmp10C=((struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct*)_tmp109)->f1;_LL8:
 _tmp10D=_tmp10C;goto _LLA;case 21U: _LL9: _tmp10D=((struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_tmp109)->f1;_LLA:
 _tmp10E=_tmp10D;goto _LLC;case 22U: _LLB: _tmp10E=((struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_tmp109)->f1;_LLC:
# 892
 return Cyc_NewControlFlow_is_local_var_rooted_path(_tmp10E,cast_ok);case 23U: _LLD: _tmp10F=((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmp109)->f1;_LLE: {
# 894
void*_tmp10A=Cyc_Tcutil_compress((void*)_check_null(_tmp10F->topt));void*_tmp10B=_tmp10A;if(((struct Cyc_Absyn_TupleType_Absyn_Type_struct*)_tmp10B)->tag == 6U){_LL14: _LL15:
 return Cyc_NewControlFlow_is_local_var_rooted_path(_tmp10F,cast_ok);}else{_LL16: _LL17:
 return 0;}_LL13:;}case 14U: _LLF: _tmp110=((struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_tmp109)->f2;_LL10:
# 899
 if(cast_ok)return Cyc_NewControlFlow_is_local_var_rooted_path(_tmp110,cast_ok);else{
return 0;}default: _LL11: _LL12:
 return 0;}_LL0:;}
# 905
static int Cyc_NewControlFlow_subst_param(struct Cyc_List_List*exps,union Cyc_Relations_RelnOp*rop){
union Cyc_Relations_RelnOp _tmp111=*rop;union Cyc_Relations_RelnOp _tmp112=_tmp111;unsigned int _tmp116;unsigned int _tmp115;switch((_tmp112.RParamNumelts).tag){case 5U: _LL1: _tmp115=(_tmp112.RParam).val;_LL2: {
# 908
struct Cyc_Absyn_Exp*_tmp113=((struct Cyc_Absyn_Exp*(*)(struct Cyc_List_List*x,int n))Cyc_List_nth)(exps,(int)_tmp115);
return Cyc_Relations_exp2relnop(_tmp113,rop);}case 6U: _LL3: _tmp116=(_tmp112.RParamNumelts).val;_LL4: {
# 911
struct Cyc_Absyn_Exp*_tmp114=((struct Cyc_Absyn_Exp*(*)(struct Cyc_List_List*x,int n))Cyc_List_nth)(exps,(int)_tmp116);
return({struct Cyc_Absyn_Exp*_tmp6A2=Cyc_Absyn_prim1_exp(Cyc_Absyn_Numelts,_tmp114,0U);Cyc_Relations_exp2relnop(_tmp6A2,rop);});}default: _LL5: _LL6:
 return 1;}_LL0:;}
# 917
static struct _dyneither_ptr Cyc_NewControlFlow_subst_param_string(struct Cyc_List_List*exps,union Cyc_Relations_RelnOp rop){
union Cyc_Relations_RelnOp _tmp117=rop;unsigned int _tmp119;unsigned int _tmp118;switch((_tmp117.RParamNumelts).tag){case 5U: _LL1: _tmp118=(_tmp117.RParam).val;_LL2:
# 920
 return Cyc_Absynpp_exp2string(((struct Cyc_Absyn_Exp*(*)(struct Cyc_List_List*x,int n))Cyc_List_nth)(exps,(int)_tmp118));case 6U: _LL3: _tmp119=(_tmp117.RParamNumelts).val;_LL4:
# 922
 return Cyc_Absynpp_exp2string(((struct Cyc_Absyn_Exp*(*)(struct Cyc_List_List*x,int n))Cyc_List_nth)(exps,(int)_tmp119));default: _LL5: _LL6:
 return Cyc_Relations_rop2string(rop);}_LL0:;}
# 927
static void Cyc_NewControlFlow_check_fn_requires(struct Cyc_NewControlFlow_AnalEnv*env,union Cyc_CfFlowInfo_FlowInfo inflow,struct Cyc_List_List*exps,struct Cyc_List_List*req,unsigned int loc){
# 930
union Cyc_CfFlowInfo_FlowInfo _tmp11A=inflow;struct Cyc_Dict_Dict _tmp124;struct Cyc_List_List*_tmp123;if((_tmp11A.BottomFL).tag == 1){_LL1: _LL2:
 return;}else{_LL3: _tmp124=((_tmp11A.ReachableFL).val).f1;_tmp123=((_tmp11A.ReachableFL).val).f2;_LL4:
# 933
 for(0;req != 0;req=req->tl){
struct Cyc_Relations_Reln*_tmp11B=(struct Cyc_Relations_Reln*)req->hd;
union Cyc_Relations_RelnOp rop1=_tmp11B->rop1;
union Cyc_Relations_RelnOp rop2=_tmp11B->rop2;
enum Cyc_Relations_Relation _tmp11C=Cyc_Relations_flip_relation(_tmp11B->relation);
if((!Cyc_NewControlFlow_subst_param(exps,& rop1) || !Cyc_NewControlFlow_subst_param(exps,& rop2)) || 
Cyc_Relations_consistent_relations(Cyc_Relations_add_relation(Cyc_Core_heap_region,rop2,_tmp11C,rop1,_tmp123))){
struct _dyneither_ptr s1=Cyc_NewControlFlow_subst_param_string(exps,rop1);
struct _dyneither_ptr s2=Cyc_Relations_relation2string(_tmp11B->relation);
struct _dyneither_ptr s3=Cyc_NewControlFlow_subst_param_string(exps,rop2);
({struct Cyc_String_pa_PrintArg_struct _tmp11F=({struct Cyc_String_pa_PrintArg_struct _tmp5B8;_tmp5B8.tag=0U,_tmp5B8.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)s1);_tmp5B8;});struct Cyc_String_pa_PrintArg_struct _tmp120=({struct Cyc_String_pa_PrintArg_struct _tmp5B7;_tmp5B7.tag=0U,_tmp5B7.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)s2);_tmp5B7;});struct Cyc_String_pa_PrintArg_struct _tmp121=({struct Cyc_String_pa_PrintArg_struct _tmp5B6;_tmp5B6.tag=0U,_tmp5B6.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)s3);_tmp5B6;});struct Cyc_String_pa_PrintArg_struct _tmp122=({struct Cyc_String_pa_PrintArg_struct _tmp5B5;_tmp5B5.tag=0U,({
struct _dyneither_ptr _tmp6A3=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Relations_relns2string(_tmp123));_tmp5B5.f1=_tmp6A3;});_tmp5B5;});void*_tmp11D[4U];_tmp11D[0]=& _tmp11F,_tmp11D[1]=& _tmp120,_tmp11D[2]=& _tmp121,_tmp11D[3]=& _tmp122;({unsigned int _tmp6A5=loc;struct _dyneither_ptr _tmp6A4=({const char*_tmp11E="cannot prove that @requires clause %s %s %s is satisfied\n (all I know is %s)";_tag_dyneither(_tmp11E,sizeof(char),77U);});Cyc_CfFlowInfo_aerr(_tmp6A5,_tmp6A4,_tag_dyneither(_tmp11D,sizeof(void*),4U));});});
break;}}
# 948
goto _LL0;}_LL0:;}struct _tuple25{union Cyc_CfFlowInfo_AbsLVal f1;union Cyc_CfFlowInfo_FlowInfo f2;};struct _tuple26{struct Cyc_List_List*f1;struct Cyc_Absyn_Exp*f2;};
# 953
static struct _tuple16 Cyc_NewControlFlow_anal_Rexp(struct Cyc_NewControlFlow_AnalEnv*env,int copy_ctxt,union Cyc_CfFlowInfo_FlowInfo inflow,struct Cyc_Absyn_Exp*e){
# 957
struct Cyc_CfFlowInfo_FlowEnv*_tmp125=env->fenv;
struct Cyc_Dict_Dict d;
struct Cyc_List_List*relns;
# 970
{union Cyc_CfFlowInfo_FlowInfo _tmp126=inflow;struct Cyc_Dict_Dict _tmp128;struct Cyc_List_List*_tmp127;if((_tmp126.BottomFL).tag == 1){_LL1: _LL2:
 return({struct _tuple16 _tmp5B9;({union Cyc_CfFlowInfo_FlowInfo _tmp6A6=Cyc_CfFlowInfo_BottomFL();_tmp5B9.f1=_tmp6A6;}),_tmp5B9.f2=_tmp125->unknown_all;_tmp5B9;});}else{_LL3: _tmp128=((_tmp126.ReachableFL).val).f1;_tmp127=((_tmp126.ReachableFL).val).f2;_LL4:
 d=_tmp128;relns=_tmp127;}_LL0:;}
# 985 "new_control_flow.cyc"
if((copy_ctxt  && Cyc_NewControlFlow_is_local_var_rooted_path(e,0)) && 
Cyc_Tcutil_is_noalias_pointer_or_aggr((void*)_check_null(e->topt))){
# 1006 "new_control_flow.cyc"
struct _tuple18 _tmp129=Cyc_NewControlFlow_anal_Lexp(env,inflow,1,0,e);struct _tuple18 _tmp12A=_tmp129;union Cyc_CfFlowInfo_FlowInfo _tmp136;union Cyc_CfFlowInfo_AbsLVal _tmp135;_LL6: _tmp136=_tmp12A.f1;_tmp135=_tmp12A.f2;_LL7:;{
struct _tuple18 _tmp12B=({struct _tuple18 _tmp5BD;_tmp5BD.f1=_tmp136,_tmp5BD.f2=_tmp135;_tmp5BD;});struct _tuple18 _tmp12C=_tmp12B;struct Cyc_Dict_Dict _tmp134;struct Cyc_List_List*_tmp133;struct Cyc_CfFlowInfo_Place*_tmp132;if(((_tmp12C.f1).ReachableFL).tag == 2){if(((_tmp12C.f2).PlaceL).tag == 1){_LL9: _tmp134=(((_tmp12C.f1).ReachableFL).val).f1;_tmp133=(((_tmp12C.f1).ReachableFL).val).f2;_tmp132=((_tmp12C.f2).PlaceL).val;_LLA: {
# 1009
void*_tmp12D=Cyc_CfFlowInfo_lookup_place(_tmp134,_tmp132);
int needs_unconsume=0;
if(Cyc_CfFlowInfo_is_unique_consumed(e,env->iteration_num,_tmp12D,& needs_unconsume)){
({void*_tmp12E=0U;({unsigned int _tmp6A8=e->loc;struct _dyneither_ptr _tmp6A7=({const char*_tmp12F="expression attempts to copy a consumed alias-free value";_tag_dyneither(_tmp12F,sizeof(char),56U);});Cyc_CfFlowInfo_aerr(_tmp6A8,_tmp6A7,_tag_dyneither(_tmp12E,sizeof(void*),0U));});});
return({struct _tuple16 _tmp5BA;({union Cyc_CfFlowInfo_FlowInfo _tmp6A9=Cyc_CfFlowInfo_BottomFL();_tmp5BA.f1=_tmp6A9;}),_tmp5BA.f2=_tmp125->unknown_all;_tmp5BA;});}else{
# 1015
if(needs_unconsume)
# 1017
return({struct _tuple16 _tmp5BB;_tmp5BB.f1=_tmp136,({void*_tmp6AA=Cyc_CfFlowInfo_make_unique_unconsumed(_tmp125,_tmp12D);_tmp5BB.f2=_tmp6AA;});_tmp5BB;});else{
# 1020
void*_tmp130=Cyc_CfFlowInfo_make_unique_consumed(_tmp125,(void*)_check_null(e->topt),e,env->iteration_num,_tmp12D);
struct Cyc_Dict_Dict _tmp131=Cyc_CfFlowInfo_assign_place(_tmp125,e->loc,_tmp134,_tmp132,_tmp130);
# 1032
return({struct _tuple16 _tmp5BC;({union Cyc_CfFlowInfo_FlowInfo _tmp6AB=Cyc_CfFlowInfo_ReachableFL(_tmp131,_tmp133);_tmp5BC.f1=_tmp6AB;}),_tmp5BC.f2=_tmp12D;_tmp5BC;});}}}}else{goto _LLB;}}else{_LLB: _LLC:
# 1036
 goto _LL8;}_LL8:;};}{
# 1039
void*_tmp137=e->r;void*_tmp138=_tmp137;struct Cyc_Absyn_Stmt*_tmp2F9;struct Cyc_Absyn_Exp*_tmp2F8;void*_tmp2F7;int _tmp2F6;struct Cyc_Absyn_Vardecl*_tmp2F5;struct Cyc_Absyn_Exp*_tmp2F4;struct Cyc_Absyn_Exp*_tmp2F3;int _tmp2F2;struct Cyc_List_List*_tmp2F1;struct Cyc_List_List*_tmp2F0;enum Cyc_Absyn_AggrKind _tmp2EF;struct Cyc_List_List*_tmp2EE;struct Cyc_List_List*_tmp2ED;struct Cyc_List_List*_tmp2EC;struct Cyc_Absyn_Exp*_tmp2EB;struct Cyc_Absyn_Exp*_tmp2EA;struct Cyc_Absyn_Exp*_tmp2E9;struct Cyc_Absyn_Exp*_tmp2E8;struct Cyc_Absyn_Exp*_tmp2E7;struct Cyc_Absyn_Exp*_tmp2E6;struct Cyc_Absyn_Exp*_tmp2E5;struct Cyc_Absyn_Exp*_tmp2E4;struct Cyc_Absyn_Exp*_tmp2E3;struct Cyc_Absyn_Exp*_tmp2E2;struct _dyneither_ptr*_tmp2E1;struct Cyc_Absyn_Exp*_tmp2E0;struct _dyneither_ptr*_tmp2DF;struct Cyc_Absyn_Exp*_tmp2DE;struct _dyneither_ptr*_tmp2DD;struct Cyc_Absyn_Exp*_tmp2DC;struct Cyc_Absyn_Exp*_tmp2DB;struct Cyc_Absyn_Exp*_tmp2DA;struct Cyc_Absyn_Exp*_tmp2D9;struct Cyc_Absyn_Exp*_tmp2D8;struct Cyc_Absyn_Exp*_tmp2D7;int _tmp2D6;struct Cyc_Absyn_Exp*_tmp2D5;void**_tmp2D4;struct Cyc_Absyn_Exp*_tmp2D3;int _tmp2D2;struct Cyc_Absyn_Exp*_tmp2D1;struct Cyc_List_List*_tmp2D0;struct Cyc_Absyn_Exp*_tmp2CF;struct Cyc_Absyn_Exp*_tmp2CE;struct Cyc_Absyn_Exp*_tmp2CD;struct Cyc_Absyn_Exp*_tmp2CC;struct Cyc_Absyn_Exp*_tmp2CB;struct Cyc_Absyn_Exp*_tmp2CA;struct Cyc_Absyn_Exp*_tmp2C9;struct Cyc_Absyn_Exp*_tmp2C8;enum Cyc_Absyn_Primop _tmp2C7;struct Cyc_List_List*_tmp2C6;struct Cyc_List_List*_tmp2C5;struct Cyc_Absyn_Datatypedecl*_tmp2C4;struct Cyc_Absyn_Vardecl*_tmp2C3;struct Cyc_Absyn_Vardecl*_tmp2C2;struct Cyc_Absyn_Vardecl*_tmp2C1;struct _dyneither_ptr _tmp2C0;struct Cyc_Absyn_Exp*_tmp2BF;struct Cyc_Absyn_Exp*_tmp2BE;struct Cyc_Absyn_Exp*_tmp2BD;struct Cyc_Absyn_Exp*_tmp2BC;struct Cyc_Absyn_Exp*_tmp2BB;switch(*((int*)_tmp138)){case 14U: if(((struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_tmp138)->f4 == Cyc_Absyn_Null_to_NonNull){_LLE: _tmp2BB=((struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_tmp138)->f2;_LLF: {
# 1043
struct _tuple16 _tmp139=Cyc_NewControlFlow_anal_Rexp(env,copy_ctxt,inflow,_tmp2BB);struct _tuple16 _tmp13A=_tmp139;union Cyc_CfFlowInfo_FlowInfo _tmp140;void*_tmp13F;_LL7F: _tmp140=_tmp13A.f1;_tmp13F=_tmp13A.f2;_LL80:;{
struct _tuple16 _tmp13B=Cyc_NewControlFlow_anal_derefR(env,inflow,_tmp140,_tmp2BB,_tmp13F);struct _tuple16 _tmp13C=_tmp13B;union Cyc_CfFlowInfo_FlowInfo _tmp13E;void*_tmp13D;_LL82: _tmp13E=_tmp13C.f1;_tmp13D=_tmp13C.f2;_LL83:;
return({struct _tuple16 _tmp5BE;_tmp5BE.f1=_tmp13E,_tmp5BE.f2=_tmp13F;_tmp5BE;});};}}else{_LL10: _tmp2BC=((struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_tmp138)->f2;_LL11:
# 1049
 _tmp2BD=_tmp2BC;goto _LL13;}case 12U: _LL12: _tmp2BD=((struct Cyc_Absyn_NoInstantiate_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_LL13:
 _tmp2BE=_tmp2BD;goto _LL15;case 41U: _LL14: _tmp2BE=((struct Cyc_Absyn_Extension_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_LL15:
 _tmp2BF=_tmp2BE;goto _LL17;case 13U: _LL16: _tmp2BF=((struct Cyc_Absyn_Instantiate_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_LL17:
 return Cyc_NewControlFlow_anal_Rexp(env,copy_ctxt,inflow,_tmp2BF);case 2U: _LL18: _tmp2C0=((struct Cyc_Absyn_Pragma_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_LL19:
# 1055
 if(!({struct _dyneither_ptr _tmp6AC=(struct _dyneither_ptr)_tmp2C0;Cyc_NewControlFlow_strcmp(_tmp6AC,({const char*_tmp141="print_flow";_tag_dyneither(_tmp141,sizeof(char),11U);}));})){
struct _dyneither_ptr _tmp142=Cyc_Position_string_of_segment(e->loc);
({struct Cyc_String_pa_PrintArg_struct _tmp145=({struct Cyc_String_pa_PrintArg_struct _tmp5BF;_tmp5BF.tag=0U,_tmp5BF.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp142);_tmp5BF;});void*_tmp143[1U];_tmp143[0]=& _tmp145;({struct Cyc___cycFILE*_tmp6AE=Cyc_stderr;struct _dyneither_ptr _tmp6AD=({const char*_tmp144="%s: current flow is\n";_tag_dyneither(_tmp144,sizeof(char),21U);});Cyc_fprintf(_tmp6AE,_tmp6AD,_tag_dyneither(_tmp143,sizeof(void*),1U));});});
Cyc_CfFlowInfo_print_flow(inflow);
({void*_tmp146=0U;({struct Cyc___cycFILE*_tmp6B0=Cyc_stderr;struct _dyneither_ptr _tmp6AF=({const char*_tmp147="\n";_tag_dyneither(_tmp147,sizeof(char),2U);});Cyc_fprintf(_tmp6B0,_tmp6AF,_tag_dyneither(_tmp146,sizeof(void*),0U));});});}else{
# 1061
if(!({struct _dyneither_ptr _tmp6B1=(struct _dyneither_ptr)_tmp2C0;Cyc_NewControlFlow_strcmp(_tmp6B1,({const char*_tmp148="debug_on";_tag_dyneither(_tmp148,sizeof(char),9U);}));}));else{
# 1066
if(!({struct _dyneither_ptr _tmp6B2=(struct _dyneither_ptr)_tmp2C0;Cyc_NewControlFlow_strcmp(_tmp6B2,({const char*_tmp149="debug_off";_tag_dyneither(_tmp149,sizeof(char),10U);}));}));else{
# 1072
({struct Cyc_String_pa_PrintArg_struct _tmp14C=({struct Cyc_String_pa_PrintArg_struct _tmp5C0;_tmp5C0.tag=0U,_tmp5C0.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp2C0);_tmp5C0;});void*_tmp14A[1U];_tmp14A[0]=& _tmp14C;({unsigned int _tmp6B4=e->loc;struct _dyneither_ptr _tmp6B3=({const char*_tmp14B="unknown pragma %s";_tag_dyneither(_tmp14B,sizeof(char),18U);});Cyc_Warn_warn(_tmp6B4,_tmp6B3,_tag_dyneither(_tmp14A,sizeof(void*),1U));});});}}}
return({struct _tuple16 _tmp5C1;_tmp5C1.f1=inflow,_tmp5C1.f2=_tmp125->zero;_tmp5C1;});case 0U: switch(((((struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)_tmp138)->f1).Wstring_c).tag){case 1U: _LL1A: _LL1B:
# 1075
 goto _LL1D;case 5U: if((((((struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)_tmp138)->f1).Int_c).val).f2 == 0){_LL1C: _LL1D:
 return({struct _tuple16 _tmp5C2;_tmp5C2.f1=inflow,_tmp5C2.f2=_tmp125->zero;_tmp5C2;});}else{_LL1E: _LL1F:
 goto _LL21;}case 8U: _LL20: _LL21:
 goto _LL23;case 9U: _LL22: _LL23:
 goto _LL25;default: _LL28: _LL29:
# 1083
 goto _LL2B;}case 1U: switch(*((int*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp138)->f1)){case 2U: _LL24: _LL25:
# 1080
 return({struct _tuple16 _tmp5C3;_tmp5C3.f1=inflow,_tmp5C3.f2=_tmp125->notzeroall;_tmp5C3;});case 1U: _LL34: _LL35:
# 1091
 return({struct _tuple16 _tmp5C4;_tmp5C4.f1=inflow,({void*_tmp6B5=Cyc_CfFlowInfo_typ_to_absrval(_tmp125,(void*)_check_null(e->topt),0,_tmp125->unknown_all);_tmp5C4.f2=_tmp6B5;});_tmp5C4;});case 3U: _LL36: _tmp2C1=((struct Cyc_Absyn_Param_b_Absyn_Binding_struct*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp138)->f1)->f1;_LL37:
# 1094
 _tmp2C2=_tmp2C1;goto _LL39;case 4U: _LL38: _tmp2C2=((struct Cyc_Absyn_Local_b_Absyn_Binding_struct*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp138)->f1)->f1;_LL39:
 _tmp2C3=_tmp2C2;goto _LL3B;case 5U: _LL3A: _tmp2C3=((struct Cyc_Absyn_Pat_b_Absyn_Binding_struct*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp138)->f1)->f1;_LL3B: {
# 1098
struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct vdroot=({struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct _tmp5C6;_tmp5C6.tag=0U,_tmp5C6.f1=_tmp2C3;_tmp5C6;});
return({struct _tuple16 _tmp5C5;_tmp5C5.f1=inflow,({void*_tmp6B7=({struct Cyc_Dict_Dict _tmp6B6=d;((void*(*)(struct Cyc_Dict_Dict d,int(*cmp)(void*,void*),void*k))Cyc_Dict_lookup_other)(_tmp6B6,Cyc_CfFlowInfo_root_cmp,(void*)({struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct*_tmp14D=_cycalloc(sizeof(*_tmp14D));*_tmp14D=vdroot;_tmp14D;}));});_tmp5C5.f2=_tmp6B7;});_tmp5C5;});}default: _LL74: _LL75:
# 1700
 goto _LL77;}case 31U: if(((struct Cyc_Absyn_Datatype_e_Absyn_Raw_exp_struct*)_tmp138)->f1 == 0){_LL26: _LL27:
# 1082
 goto _LL29;}else{_LL62: _tmp2C5=((struct Cyc_Absyn_Datatype_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_tmp2C4=((struct Cyc_Absyn_Datatype_e_Absyn_Raw_exp_struct*)_tmp138)->f2;_LL63:
# 1563
 _tmp2EC=_tmp2C5;goto _LL65;}case 18U: _LL2A: _LL2B:
# 1084
 goto _LL2D;case 17U: _LL2C: _LL2D:
 goto _LL2F;case 19U: _LL2E: _LL2F:
 goto _LL31;case 33U: _LL30: _LL31:
 goto _LL33;case 32U: _LL32: _LL33:
 return({struct _tuple16 _tmp5C7;_tmp5C7.f1=inflow,_tmp5C7.f2=_tmp125->unknown_all;_tmp5C7;});case 3U: _LL3C: _tmp2C7=((struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_tmp2C6=((struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_tmp138)->f2;_LL3D: {
# 1105
struct _tuple16 _tmp14E=Cyc_NewControlFlow_anal_use_ints(env,inflow,_tmp2C6);struct _tuple16 _tmp14F=_tmp14E;union Cyc_CfFlowInfo_FlowInfo _tmp151;void*_tmp150;_LL85: _tmp151=_tmp14F.f1;_tmp150=_tmp14F.f2;_LL86:;
return({struct _tuple16 _tmp5C8;_tmp5C8.f1=_tmp151,_tmp5C8.f2=_tmp150;_tmp5C8;});}case 5U: _LL3E: _tmp2C8=((struct Cyc_Absyn_Increment_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_LL3F: {
# 1109
struct Cyc_List_List _tmp152=({struct Cyc_List_List _tmp5CC;_tmp5CC.hd=_tmp2C8,_tmp5CC.tl=0;_tmp5CC;});
struct _tuple16 _tmp153=Cyc_NewControlFlow_anal_use_ints(env,inflow,& _tmp152);struct _tuple16 _tmp154=_tmp153;union Cyc_CfFlowInfo_FlowInfo _tmp161;_LL88: _tmp161=_tmp154.f1;_LL89:;{
struct _tuple18 _tmp155=Cyc_NewControlFlow_anal_Lexp(env,_tmp161,0,0,_tmp2C8);struct _tuple18 _tmp156=_tmp155;union Cyc_CfFlowInfo_AbsLVal _tmp160;_LL8B: _tmp160=_tmp156.f2;_LL8C:;{
int iszt=Cyc_Tcutil_is_zeroterm_pointer_type((void*)_check_null(_tmp2C8->topt));
if(iszt){
# 1116
struct _tuple16 _tmp157=Cyc_NewControlFlow_anal_Rexp(env,0,inflow,_tmp2C8);struct _tuple16 _tmp158=_tmp157;union Cyc_CfFlowInfo_FlowInfo _tmp15A;void*_tmp159;_LL8E: _tmp15A=_tmp158.f1;_tmp159=_tmp158.f2;_LL8F:;
Cyc_NewControlFlow_anal_derefR(env,inflow,_tmp15A,_tmp2C8,_tmp159);}
# 1119
{struct _tuple25 _tmp15B=({struct _tuple25 _tmp5C9;_tmp5C9.f1=_tmp160,_tmp5C9.f2=_tmp161;_tmp5C9;});struct _tuple25 _tmp15C=_tmp15B;struct Cyc_CfFlowInfo_Place*_tmp15F;struct Cyc_Dict_Dict _tmp15E;struct Cyc_List_List*_tmp15D;if(((_tmp15C.f1).PlaceL).tag == 1){if(((_tmp15C.f2).ReachableFL).tag == 2){_LL91: _tmp15F=((_tmp15C.f1).PlaceL).val;_tmp15E=(((_tmp15C.f2).ReachableFL).val).f1;_tmp15D=(((_tmp15C.f2).ReachableFL).val).f2;_LL92:
# 1121
({struct Cyc_List_List*_tmp6B8=Cyc_Relations_reln_kill_exp(Cyc_Core_heap_region,_tmp15D,_tmp2C8);_tmp15D=_tmp6B8;});
({union Cyc_CfFlowInfo_FlowInfo _tmp6BA=({struct Cyc_Dict_Dict _tmp6B9=Cyc_CfFlowInfo_assign_place(_tmp125,_tmp2C8->loc,_tmp15E,_tmp15F,_tmp125->unknown_all);Cyc_CfFlowInfo_ReachableFL(_tmp6B9,_tmp15D);});_tmp161=_tmp6BA;});
# 1126
goto _LL90;}else{goto _LL93;}}else{_LL93: _LL94:
 goto _LL90;}_LL90:;}
# 1130
if(iszt)
return({struct _tuple16 _tmp5CA;_tmp5CA.f1=_tmp161,_tmp5CA.f2=_tmp125->notzeroall;_tmp5CA;});else{
return({struct _tuple16 _tmp5CB;_tmp5CB.f1=_tmp161,_tmp5CB.f2=_tmp125->unknown_all;_tmp5CB;});}};};}case 4U: if(((struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct*)_tmp138)->f2 != 0){_LL40: _tmp2CA=((struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_tmp2C9=((struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct*)_tmp138)->f3;_LL41:
# 1135
 if(copy_ctxt  && Cyc_Tcutil_is_noalias_pointer_or_aggr((void*)_check_null(e->topt))){
({void*_tmp162=0U;({unsigned int _tmp6BC=e->loc;struct _dyneither_ptr _tmp6BB=({const char*_tmp163="cannot track alias-free pointers through multiple assignments";_tag_dyneither(_tmp163,sizeof(char),62U);});Cyc_CfFlowInfo_aerr(_tmp6BC,_tmp6BB,_tag_dyneither(_tmp162,sizeof(void*),0U));});});
return({struct _tuple16 _tmp5CD;({union Cyc_CfFlowInfo_FlowInfo _tmp6BD=Cyc_CfFlowInfo_BottomFL();_tmp5CD.f1=_tmp6BD;}),_tmp5CD.f2=_tmp125->unknown_all;_tmp5CD;});}{
# 1139
struct _tuple16 _tmp164=Cyc_NewControlFlow_anal_Rexp(env,1,inflow,_tmp2C9);struct _tuple16 _tmp165=_tmp164;union Cyc_CfFlowInfo_FlowInfo _tmp170;void*_tmp16F;_LL96: _tmp170=_tmp165.f1;_tmp16F=_tmp165.f2;_LL97:;{
struct _tuple18 _tmp166=Cyc_NewControlFlow_anal_Lexp(env,_tmp170,0,0,_tmp2CA);struct _tuple18 _tmp167=_tmp166;union Cyc_CfFlowInfo_FlowInfo _tmp16E;union Cyc_CfFlowInfo_AbsLVal _tmp16D;_LL99: _tmp16E=_tmp167.f1;_tmp16D=_tmp167.f2;_LL9A:;
{union Cyc_CfFlowInfo_FlowInfo _tmp168=_tmp16E;struct Cyc_Dict_Dict _tmp16C;struct Cyc_List_List*_tmp16B;if((_tmp168.ReachableFL).tag == 2){_LL9C: _tmp16C=((_tmp168.ReachableFL).val).f1;_tmp16B=((_tmp168.ReachableFL).val).f2;_LL9D:
# 1143
{union Cyc_CfFlowInfo_AbsLVal _tmp169=_tmp16D;struct Cyc_CfFlowInfo_Place*_tmp16A;if((_tmp169.PlaceL).tag == 1){_LLA1: _tmp16A=(_tmp169.PlaceL).val;_LLA2:
# 1147
({struct Cyc_List_List*_tmp6BE=Cyc_Relations_reln_kill_exp(Cyc_Core_heap_region,_tmp16B,_tmp2CA);_tmp16B=_tmp6BE;});
({struct Cyc_Dict_Dict _tmp6BF=Cyc_CfFlowInfo_assign_place(_tmp125,_tmp2CA->loc,_tmp16C,_tmp16A,_tmp125->unknown_all);_tmp16C=_tmp6BF;});
# 1150
({union Cyc_CfFlowInfo_FlowInfo _tmp6C0=Cyc_CfFlowInfo_ReachableFL(_tmp16C,_tmp16B);_tmp16E=_tmp6C0;});
# 1154
goto _LLA0;}else{_LLA3: _LLA4:
# 1157
 goto _LLA0;}_LLA0:;}
# 1159
goto _LL9B;}else{_LL9E: _LL9F:
 goto _LL9B;}_LL9B:;}
# 1162
return({struct _tuple16 _tmp5CE;_tmp5CE.f1=_tmp16E,_tmp5CE.f2=_tmp125->unknown_all;_tmp5CE;});};};}else{_LL42: _tmp2CC=((struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_tmp2CB=((struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct*)_tmp138)->f3;_LL43:
# 1165
 if(copy_ctxt  && Cyc_Tcutil_is_noalias_pointer_or_aggr((void*)_check_null(e->topt))){
({void*_tmp171=0U;({unsigned int _tmp6C2=e->loc;struct _dyneither_ptr _tmp6C1=({const char*_tmp172="cannot track alias-free pointers through multiple assignments";_tag_dyneither(_tmp172,sizeof(char),62U);});Cyc_CfFlowInfo_aerr(_tmp6C2,_tmp6C1,_tag_dyneither(_tmp171,sizeof(void*),0U));});});
return({struct _tuple16 _tmp5CF;({union Cyc_CfFlowInfo_FlowInfo _tmp6C3=Cyc_CfFlowInfo_BottomFL();_tmp5CF.f1=_tmp6C3;}),_tmp5CF.f2=_tmp125->unknown_all;_tmp5CF;});}
# 1171
({union Cyc_CfFlowInfo_FlowInfo _tmp6C6=({struct Cyc_NewControlFlow_AnalEnv*_tmp6C5=env;union Cyc_CfFlowInfo_FlowInfo _tmp6C4=inflow;Cyc_NewControlFlow_expand_unique_places(_tmp6C5,_tmp6C4,({struct Cyc_Absyn_Exp*_tmp173[2U];_tmp173[0]=_tmp2CC,_tmp173[1]=_tmp2CB;((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp173,sizeof(struct Cyc_Absyn_Exp*),2U));}));});inflow=_tmp6C6;});{
# 1173
struct _tuple16 _tmp174=Cyc_NewControlFlow_anal_Rexp(env,1,inflow,_tmp2CB);struct _tuple16 _tmp175=_tmp174;union Cyc_CfFlowInfo_FlowInfo _tmp17F;void*_tmp17E;_LLA6: _tmp17F=_tmp175.f1;_tmp17E=_tmp175.f2;_LLA7:;{
struct _tuple18 _tmp176=Cyc_NewControlFlow_anal_Lexp(env,_tmp17F,0,0,_tmp2CC);struct _tuple18 _tmp177=_tmp176;union Cyc_CfFlowInfo_FlowInfo _tmp17D;union Cyc_CfFlowInfo_AbsLVal _tmp17C;_LLA9: _tmp17D=_tmp177.f1;_tmp17C=_tmp177.f2;_LLAA:;{
struct _tuple16 _tmp178=Cyc_NewControlFlow_do_assign(_tmp125,env,_tmp17D,_tmp2CC,_tmp17C,_tmp2CB,_tmp17E,e->loc);struct _tuple16 _tmp179=_tmp178;union Cyc_CfFlowInfo_FlowInfo _tmp17B;void*_tmp17A;_LLAC: _tmp17B=_tmp179.f1;_tmp17A=_tmp179.f2;_LLAD:;
# 1179
return({struct _tuple16 _tmp5D0;_tmp5D0.f1=_tmp17B,_tmp5D0.f2=_tmp17A;_tmp5D0;});};};};}case 9U: _LL44: _tmp2CE=((struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_tmp2CD=((struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct*)_tmp138)->f2;_LL45: {
# 1182
struct _tuple16 _tmp180=Cyc_NewControlFlow_anal_Rexp(env,0,inflow,_tmp2CE);struct _tuple16 _tmp181=_tmp180;union Cyc_CfFlowInfo_FlowInfo _tmp183;void*_tmp182;_LLAF: _tmp183=_tmp181.f1;_tmp182=_tmp181.f2;_LLB0:;
return Cyc_NewControlFlow_anal_Rexp(env,copy_ctxt,_tmp183,_tmp2CD);}case 11U: _LL46: _tmp2CF=((struct Cyc_Absyn_Throw_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_LL47: {
# 1186
struct _tuple16 _tmp184=Cyc_NewControlFlow_anal_Rexp(env,1,inflow,_tmp2CF);struct _tuple16 _tmp185=_tmp184;union Cyc_CfFlowInfo_FlowInfo _tmp187;void*_tmp186;_LLB2: _tmp187=_tmp185.f1;_tmp186=_tmp185.f2;_LLB3:;
Cyc_NewControlFlow_use_Rval(env,_tmp2CF->loc,_tmp187,_tmp186);
return({struct _tuple16 _tmp5D1;({union Cyc_CfFlowInfo_FlowInfo _tmp6C8=Cyc_CfFlowInfo_BottomFL();_tmp5D1.f1=_tmp6C8;}),({void*_tmp6C7=Cyc_CfFlowInfo_typ_to_absrval(_tmp125,(void*)_check_null(e->topt),0,_tmp125->unknown_all);_tmp5D1.f2=_tmp6C7;});_tmp5D1;});}case 10U: _LL48: _tmp2D1=((struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_tmp2D0=((struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_tmp138)->f2;_LL49: {
# 1191
struct Cyc_List_List*_tmp188=_tmp2D0;
struct _tuple16 _tmp189=Cyc_NewControlFlow_anal_Rexp(env,0,inflow,_tmp2D1);struct _tuple16 _tmp18A=_tmp189;union Cyc_CfFlowInfo_FlowInfo _tmp1BA;void*_tmp1B9;_LLB5: _tmp1BA=_tmp18A.f1;_tmp1B9=_tmp18A.f2;_LLB6:;
# 1194
Cyc_NewControlFlow_anal_derefR(env,inflow,_tmp1BA,_tmp2D1,_tmp1B9);{
struct _tuple23 _tmp18B=({
struct Cyc_NewControlFlow_AnalEnv*_tmp6CA=env;union Cyc_CfFlowInfo_FlowInfo _tmp6C9=_tmp1BA;Cyc_NewControlFlow_anal_Rexps(_tmp6CA,_tmp6C9,((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_copy)(_tmp2D0),0,1);});
# 1195
struct _tuple23 _tmp18C=_tmp18B;union Cyc_CfFlowInfo_FlowInfo _tmp1B8;struct Cyc_List_List*_tmp1B7;_LLB8: _tmp1B8=_tmp18C.f1;_tmp1B7=_tmp18C.f2;_LLB9:;{
# 1198
union Cyc_CfFlowInfo_FlowInfo _tmp18D=Cyc_NewControlFlow_use_Rval(env,_tmp2D1->loc,_tmp1B8,_tmp1B9);
# 1200
struct Cyc_List_List*init_params=0;
struct Cyc_List_List*nolive_unique_params=0;
struct Cyc_List_List*noconsume_params=0;
struct Cyc_List_List*requires;
struct Cyc_List_List*ensures;
{void*_tmp18E=Cyc_Tcutil_compress((void*)_check_null(_tmp2D1->topt));void*_tmp18F=_tmp18E;void*_tmp1A1;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp18F)->tag == 3U){_LLBB: _tmp1A1=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp18F)->f1).elt_type;_LLBC:
# 1207
{void*_tmp190=Cyc_Tcutil_compress(_tmp1A1);void*_tmp191=_tmp190;struct Cyc_List_List*_tmp19E;struct Cyc_List_List*_tmp19D;struct Cyc_List_List*_tmp19C;if(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp191)->tag == 5U){_LLC0: _tmp19E=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp191)->f1).attributes;_tmp19D=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp191)->f1).requires_relns;_tmp19C=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp191)->f1).ensures_relns;_LLC1:
# 1209
 requires=_tmp19D;
ensures=_tmp19C;
for(0;_tmp19E != 0;_tmp19E=_tmp19E->tl){
# 1213
void*_tmp192=(void*)_tmp19E->hd;void*_tmp193=_tmp192;int _tmp199;int _tmp198;int _tmp197;switch(*((int*)_tmp193)){case 20U: _LLC5: _tmp197=((struct Cyc_Absyn_Initializes_att_Absyn_Attribute_struct*)_tmp193)->f1;_LLC6:
# 1215
({struct Cyc_List_List*_tmp6CB=({struct Cyc_List_List*_tmp194=_cycalloc(sizeof(*_tmp194));_tmp194->hd=(void*)_tmp197,_tmp194->tl=init_params;_tmp194;});init_params=_tmp6CB;});goto _LLC4;case 21U: _LLC7: _tmp198=((struct Cyc_Absyn_Noliveunique_att_Absyn_Attribute_struct*)_tmp193)->f1;_LLC8:
# 1217
({struct Cyc_List_List*_tmp6CC=({struct Cyc_List_List*_tmp195=_cycalloc(sizeof(*_tmp195));_tmp195->hd=(void*)_tmp198,_tmp195->tl=nolive_unique_params;_tmp195;});nolive_unique_params=_tmp6CC;});
goto _LLC4;case 22U: _LLC9: _tmp199=((struct Cyc_Absyn_Noconsume_att_Absyn_Attribute_struct*)_tmp193)->f1;_LLCA:
# 1221
({struct Cyc_List_List*_tmp6CD=({struct Cyc_List_List*_tmp196=_cycalloc(sizeof(*_tmp196));_tmp196->hd=(void*)_tmp199,_tmp196->tl=noconsume_params;_tmp196;});noconsume_params=_tmp6CD;});
goto _LLC4;default: _LLCB: _LLCC:
 goto _LLC4;}_LLC4:;}
# 1225
goto _LLBF;}else{_LLC2: _LLC3:
({void*_tmp19A=0U;({struct _dyneither_ptr _tmp6CE=({const char*_tmp19B="anal_Rexp: bad function type";_tag_dyneither(_tmp19B,sizeof(char),29U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp6CE,_tag_dyneither(_tmp19A,sizeof(void*),0U));});});}_LLBF:;}
# 1228
goto _LLBA;}else{_LLBD: _LLBE:
({void*_tmp19F=0U;({struct _dyneither_ptr _tmp6CF=({const char*_tmp1A0="anal_Rexp: bad function type";_tag_dyneither(_tmp1A0,sizeof(char),29U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp6CF,_tag_dyneither(_tmp19F,sizeof(void*),0U));});});}_LLBA:;}
# 1231
{int i=1;for(0;_tmp1B7 != 0;(_tmp1B7=_tmp1B7->tl,_tmp2D0=((struct Cyc_List_List*)_check_null(_tmp2D0))->tl,++ i)){
if(((int(*)(struct Cyc_List_List*l,int x))Cyc_List_memq)(init_params,i)){
union Cyc_CfFlowInfo_FlowInfo _tmp1A2=_tmp1B8;struct Cyc_Dict_Dict _tmp1B6;if((_tmp1A2.BottomFL).tag == 1){_LLCE: _LLCF:
 goto _LLCD;}else{_LLD0: _tmp1B6=((_tmp1A2.ReachableFL).val).f1;_LLD1: {
# 1236
struct _tuple15 _tmp1A3=Cyc_CfFlowInfo_unname_rval((void*)_tmp1B7->hd);struct _tuple15 _tmp1A4=_tmp1A3;void*_tmp1B5;_LLD3: _tmp1B5=_tmp1A4.f1;_LLD4:;
if(Cyc_CfFlowInfo_initlevel(env->fenv,_tmp1B6,(void*)_tmp1B7->hd)== Cyc_CfFlowInfo_NoneIL  && !Cyc_CfFlowInfo_is_init_pointer((void*)_tmp1B7->hd))
({void*_tmp1A5=0U;({unsigned int _tmp6D1=((struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)_check_null(_tmp2D0))->hd)->loc;struct _dyneither_ptr _tmp6D0=({const char*_tmp1A6="expression may not be initialized";_tag_dyneither(_tmp1A6,sizeof(char),34U);});Cyc_CfFlowInfo_aerr(_tmp6D1,_tmp6D0,_tag_dyneither(_tmp1A5,sizeof(void*),0U));});});
{union Cyc_CfFlowInfo_FlowInfo _tmp1A7=_tmp18D;struct Cyc_Dict_Dict _tmp1B4;struct Cyc_List_List*_tmp1B3;if((_tmp1A7.BottomFL).tag == 1){_LLD6: _LLD7:
 goto _LLD5;}else{_LLD8: _tmp1B4=((_tmp1A7.ReachableFL).val).f1;_tmp1B3=((_tmp1A7.ReachableFL).val).f2;_LLD9: {
# 1244
struct Cyc_Dict_Dict _tmp1A8=Cyc_CfFlowInfo_escape_deref(_tmp125,_tmp1B4,(void*)_tmp1B7->hd);
{void*_tmp1A9=(void*)_tmp1B7->hd;void*_tmp1AA=_tmp1A9;struct Cyc_CfFlowInfo_Place*_tmp1B2;switch(*((int*)_tmp1AA)){case 4U: _LLDB: _tmp1B2=((struct Cyc_CfFlowInfo_AddressOf_CfFlowInfo_AbsRVal_struct*)_tmp1AA)->f1;_LLDC:
# 1247
{void*_tmp1AB=Cyc_Tcutil_compress((void*)_check_null(((struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)_check_null(_tmp2D0))->hd)->topt));void*_tmp1AC=_tmp1AB;void*_tmp1AF;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1AC)->tag == 3U){_LLE2: _tmp1AF=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1AC)->f1).elt_type;_LLE3:
# 1249
({struct Cyc_Dict_Dict _tmp6D6=({struct Cyc_CfFlowInfo_FlowEnv*_tmp6D5=_tmp125;unsigned int _tmp6D4=((struct Cyc_Absyn_Exp*)_tmp2D0->hd)->loc;struct Cyc_Dict_Dict _tmp6D3=_tmp1A8;struct Cyc_CfFlowInfo_Place*_tmp6D2=_tmp1B2;Cyc_CfFlowInfo_assign_place(_tmp6D5,_tmp6D4,_tmp6D3,_tmp6D2,
Cyc_CfFlowInfo_typ_to_absrval(_tmp125,_tmp1AF,0,_tmp125->esc_all));});
# 1249
_tmp1A8=_tmp6D6;});
# 1253
goto _LLE1;}else{_LLE4: _LLE5:
({void*_tmp1AD=0U;({struct _dyneither_ptr _tmp6D7=({const char*_tmp1AE="anal_Rexp:bad type for initialized arg";_tag_dyneither(_tmp1AE,sizeof(char),39U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp6D7,_tag_dyneither(_tmp1AD,sizeof(void*),0U));});});}_LLE1:;}
# 1256
goto _LLDA;case 5U: _LLDD: _LLDE:
# 1258
({void*_tmp1B0=0U;({struct _dyneither_ptr _tmp6D8=({const char*_tmp1B1="anal_Rexp:initialize attribute on unique pointers not yet supported";_tag_dyneither(_tmp1B1,sizeof(char),68U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp6D8,_tag_dyneither(_tmp1B0,sizeof(void*),0U));});});default: _LLDF: _LLE0:
# 1260
 goto _LLDA;}_LLDA:;}
# 1262
({union Cyc_CfFlowInfo_FlowInfo _tmp6D9=Cyc_CfFlowInfo_ReachableFL(_tmp1A8,_tmp1B3);_tmp18D=_tmp6D9;});
goto _LLD5;}}_LLD5:;}
# 1265
goto _LLCD;}}_LLCD:;}else{
# 1268
if(((int(*)(struct Cyc_List_List*l,int x))Cyc_List_memq)(nolive_unique_params,i))
# 1273
({union Cyc_CfFlowInfo_FlowInfo _tmp6DE=({struct Cyc_NewControlFlow_AnalEnv*_tmp6DD=env;unsigned int _tmp6DC=((struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)_check_null(_tmp2D0))->hd)->loc;void*_tmp6DB=(void*)_check_null(((struct Cyc_Absyn_Exp*)_tmp2D0->hd)->topt);union Cyc_CfFlowInfo_FlowInfo _tmp6DA=_tmp18D;Cyc_NewControlFlow_use_nounique_Rval(_tmp6DD,_tmp6DC,_tmp6DB,_tmp6DA,(void*)_tmp1B7->hd);});_tmp18D=_tmp6DE;});else{
# 1283
({union Cyc_CfFlowInfo_FlowInfo _tmp6DF=Cyc_NewControlFlow_use_Rval(env,((struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)_check_null(_tmp2D0))->hd)->loc,_tmp18D,(void*)_tmp1B7->hd);_tmp18D=_tmp6DF;});
if(((int(*)(struct Cyc_List_List*l,int x))Cyc_List_memq)(noconsume_params,i) && 
Cyc_Tcutil_is_noalias_pointer((void*)_check_null(((struct Cyc_Absyn_Exp*)_tmp2D0->hd)->topt),0))
({union Cyc_CfFlowInfo_FlowInfo _tmp6E0=Cyc_NewControlFlow_restore_noconsume_arg(env,_tmp18D,(struct Cyc_Absyn_Exp*)_tmp2D0->hd,((struct Cyc_Absyn_Exp*)_tmp2D0->hd)->loc,(void*)_tmp1B7->hd);_tmp18D=_tmp6E0;});}}}}
# 1290
Cyc_NewControlFlow_check_fn_requires(env,_tmp18D,_tmp188,requires,e->loc);
# 1293
if(Cyc_Tcutil_is_noreturn_fn_type((void*)_check_null(_tmp2D1->topt)))
return({struct _tuple16 _tmp5D2;({union Cyc_CfFlowInfo_FlowInfo _tmp6E2=Cyc_CfFlowInfo_BottomFL();_tmp5D2.f1=_tmp6E2;}),({void*_tmp6E1=Cyc_CfFlowInfo_typ_to_absrval(_tmp125,(void*)_check_null(e->topt),0,_tmp125->unknown_all);_tmp5D2.f2=_tmp6E1;});_tmp5D2;});else{
# 1296
return({struct _tuple16 _tmp5D3;_tmp5D3.f1=_tmp18D,({void*_tmp6E3=Cyc_CfFlowInfo_typ_to_absrval(_tmp125,(void*)_check_null(e->topt),0,_tmp125->unknown_all);_tmp5D3.f2=_tmp6E3;});_tmp5D3;});}};};}case 34U: _LL4A: _tmp2D6=(((struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct*)_tmp138)->f1).is_calloc;_tmp2D5=(((struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct*)_tmp138)->f1).rgn;_tmp2D4=(((struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct*)_tmp138)->f1).elt_type;_tmp2D3=(((struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct*)_tmp138)->f1).num_elts;_tmp2D2=(((struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct*)_tmp138)->f1).fat_result;_LL4B: {
# 1299
void*place_val;
if(_tmp2D2)place_val=_tmp125->notzeroall;else{
if(_tmp2D6)({void*_tmp6E4=Cyc_CfFlowInfo_typ_to_absrval(_tmp125,*((void**)_check_null(_tmp2D4)),0,_tmp125->zero);place_val=_tmp6E4;});else{
({void*_tmp6E5=Cyc_CfFlowInfo_typ_to_absrval(_tmp125,*((void**)_check_null(_tmp2D4)),0,_tmp125->unknown_none);place_val=_tmp6E5;});}}{
union Cyc_CfFlowInfo_FlowInfo outflow;
if(_tmp2D5 != 0){
struct _tuple23 _tmp1BB=({struct Cyc_NewControlFlow_AnalEnv*_tmp6E7=env;union Cyc_CfFlowInfo_FlowInfo _tmp6E6=inflow;Cyc_NewControlFlow_anal_Rexps(_tmp6E7,_tmp6E6,({struct Cyc_Absyn_Exp*_tmp1BF[2U];_tmp1BF[0]=_tmp2D5,_tmp1BF[1]=_tmp2D3;((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp1BF,sizeof(struct Cyc_Absyn_Exp*),2U));}),1,1);});struct _tuple23 _tmp1BC=_tmp1BB;union Cyc_CfFlowInfo_FlowInfo _tmp1BE;struct Cyc_List_List*_tmp1BD;_LLE7: _tmp1BE=_tmp1BC.f1;_tmp1BD=_tmp1BC.f2;_LLE8:;
outflow=_tmp1BE;}else{
# 1308
struct _tuple16 _tmp1C0=Cyc_NewControlFlow_anal_Rexp(env,1,inflow,_tmp2D3);struct _tuple16 _tmp1C1=_tmp1C0;union Cyc_CfFlowInfo_FlowInfo _tmp1C2;_LLEA: _tmp1C2=_tmp1C1.f1;_LLEB:;
outflow=_tmp1C2;}
# 1313
if(Cyc_Tcutil_is_noalias_pointer((void*)_check_null(e->topt),1))
return({struct _tuple16 _tmp5D4;_tmp5D4.f1=outflow,({void*_tmp6E8=(void*)({struct Cyc_CfFlowInfo_UniquePtr_CfFlowInfo_AbsRVal_struct*_tmp1C3=_cycalloc(sizeof(*_tmp1C3));_tmp1C3->tag=5U,_tmp1C3->f1=place_val;_tmp1C3;});_tmp5D4.f2=_tmp6E8;});_tmp5D4;});else{
# 1317
void*root=(void*)({struct Cyc_CfFlowInfo_MallocPt_CfFlowInfo_Root_struct*_tmp1C9=_cycalloc(sizeof(*_tmp1C9));_tmp1C9->tag=1U,_tmp1C9->f1=_tmp2D3,_tmp1C9->f2=(void*)_check_null(e->topt);_tmp1C9;});
struct Cyc_CfFlowInfo_Place*place=({struct Cyc_CfFlowInfo_Place*_tmp1C8=_cycalloc(sizeof(*_tmp1C8));_tmp1C8->root=root,_tmp1C8->path=0;_tmp1C8;});
void*rval=(void*)({struct Cyc_CfFlowInfo_AddressOf_CfFlowInfo_AbsRVal_struct*_tmp1C7=_cycalloc(sizeof(*_tmp1C7));_tmp1C7->tag=4U,_tmp1C7->f1=place;_tmp1C7;});
union Cyc_CfFlowInfo_FlowInfo _tmp1C4=outflow;struct Cyc_Dict_Dict _tmp1C6;struct Cyc_List_List*_tmp1C5;if((_tmp1C4.BottomFL).tag == 1){_LLED: _LLEE:
 return({struct _tuple16 _tmp5D5;_tmp5D5.f1=outflow,_tmp5D5.f2=rval;_tmp5D5;});}else{_LLEF: _tmp1C6=((_tmp1C4.ReachableFL).val).f1;_tmp1C5=((_tmp1C4.ReachableFL).val).f2;_LLF0:
# 1323
 return({struct _tuple16 _tmp5D6;({union Cyc_CfFlowInfo_FlowInfo _tmp6EA=({struct Cyc_Dict_Dict _tmp6E9=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,void*k,void*v))Cyc_Dict_insert)(_tmp1C6,root,place_val);Cyc_CfFlowInfo_ReachableFL(_tmp6E9,_tmp1C5);});_tmp5D6.f1=_tmp6EA;}),_tmp5D6.f2=rval;_tmp5D6;});}_LLEC:;}};}case 35U: _LL4C: _tmp2D8=((struct Cyc_Absyn_Swap_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_tmp2D7=((struct Cyc_Absyn_Swap_e_Absyn_Raw_exp_struct*)_tmp138)->f2;_LL4D: {
# 1328
void*left_rval;
void*right_rval;
union Cyc_CfFlowInfo_FlowInfo outflow;
# 1334
struct _tuple23 _tmp1CA=({struct Cyc_NewControlFlow_AnalEnv*_tmp6EC=env;union Cyc_CfFlowInfo_FlowInfo _tmp6EB=inflow;Cyc_NewControlFlow_anal_Rexps(_tmp6EC,_tmp6EB,({struct Cyc_Absyn_Exp*_tmp1E5[2U];_tmp1E5[0]=_tmp2D8,_tmp1E5[1]=_tmp2D7;((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp1E5,sizeof(struct Cyc_Absyn_Exp*),2U));}),0,0);});struct _tuple23 _tmp1CB=_tmp1CA;union Cyc_CfFlowInfo_FlowInfo _tmp1E4;struct Cyc_List_List*_tmp1E3;_LLF2: _tmp1E4=_tmp1CB.f1;_tmp1E3=_tmp1CB.f2;_LLF3:;
left_rval=(void*)((struct Cyc_List_List*)_check_null(_tmp1E3))->hd;
right_rval=(void*)((struct Cyc_List_List*)_check_null(_tmp1E3->tl))->hd;
outflow=_tmp1E4;{
# 1340
void*t_ign1=Cyc_Absyn_void_type;void*t_ign2=Cyc_Absyn_void_type;
int b_ign1=0;
if(Cyc_Tcutil_is_zero_ptr_deref(_tmp2D8,& t_ign1,& b_ign1,& t_ign2)){
void*_tmp1CC=right_rval;if(((struct Cyc_CfFlowInfo_Zero_CfFlowInfo_AbsRVal_struct*)_tmp1CC)->tag == 0U){_LLF5: _LLF6:
 goto _LLF4;}else{_LLF7: _LLF8:
({void*_tmp1CD=0U;({unsigned int _tmp6EE=_tmp2D8->loc;struct _dyneither_ptr _tmp6ED=({const char*_tmp1CE="cannot swap value into zeroterm array not known to be 0";_tag_dyneither(_tmp1CE,sizeof(char),56U);});Cyc_CfFlowInfo_aerr(_tmp6EE,_tmp6ED,_tag_dyneither(_tmp1CD,sizeof(void*),0U));});});}_LLF4:;}else{
# 1351
if(Cyc_Tcutil_is_zero_ptr_deref(_tmp2D7,& t_ign1,& b_ign1,& t_ign2)){
void*_tmp1CF=left_rval;if(((struct Cyc_CfFlowInfo_Zero_CfFlowInfo_AbsRVal_struct*)_tmp1CF)->tag == 0U){_LLFA: _LLFB:
 goto _LLF9;}else{_LLFC: _LLFD:
({void*_tmp1D0=0U;({unsigned int _tmp6F0=_tmp2D7->loc;struct _dyneither_ptr _tmp6EF=({const char*_tmp1D1="cannot swap value into zeroterm array not known to be 0";_tag_dyneither(_tmp1D1,sizeof(char),56U);});Cyc_CfFlowInfo_aerr(_tmp6F0,_tmp6EF,_tag_dyneither(_tmp1D0,sizeof(void*),0U));});});}_LLF9:;}}{
# 1359
struct _tuple18 _tmp1D2=Cyc_NewControlFlow_anal_Lexp(env,outflow,0,0,_tmp2D8);struct _tuple18 _tmp1D3=_tmp1D2;union Cyc_CfFlowInfo_AbsLVal _tmp1E2;_LLFF: _tmp1E2=_tmp1D3.f2;_LL100:;{
struct _tuple18 _tmp1D4=Cyc_NewControlFlow_anal_Lexp(env,outflow,0,0,_tmp2D7);struct _tuple18 _tmp1D5=_tmp1D4;union Cyc_CfFlowInfo_AbsLVal _tmp1E1;_LL102: _tmp1E1=_tmp1D5.f2;_LL103:;
{union Cyc_CfFlowInfo_FlowInfo _tmp1D6=outflow;struct Cyc_Dict_Dict _tmp1E0;struct Cyc_List_List*_tmp1DF;if((_tmp1D6.ReachableFL).tag == 2){_LL105: _tmp1E0=((_tmp1D6.ReachableFL).val).f1;_tmp1DF=((_tmp1D6.ReachableFL).val).f2;_LL106:
# 1363
{union Cyc_CfFlowInfo_AbsLVal _tmp1D7=_tmp1E2;struct Cyc_CfFlowInfo_Place*_tmp1DA;if((_tmp1D7.PlaceL).tag == 1){_LL10A: _tmp1DA=(_tmp1D7.PlaceL).val;_LL10B:
# 1365
({struct Cyc_Dict_Dict _tmp6F1=Cyc_CfFlowInfo_assign_place(_tmp125,_tmp2D8->loc,_tmp1E0,_tmp1DA,right_rval);_tmp1E0=_tmp6F1;});
goto _LL109;}else{_LL10C: _LL10D:
# 1371
 if(Cyc_CfFlowInfo_initlevel(_tmp125,_tmp1E0,right_rval)!= Cyc_CfFlowInfo_AllIL)
({void*_tmp1D8=0U;({unsigned int _tmp6F3=_tmp2D7->loc;struct _dyneither_ptr _tmp6F2=({const char*_tmp1D9="expression may not be fully initialized";_tag_dyneither(_tmp1D9,sizeof(char),40U);});Cyc_CfFlowInfo_aerr(_tmp6F3,_tmp6F2,_tag_dyneither(_tmp1D8,sizeof(void*),0U));});});
goto _LL109;}_LL109:;}
# 1375
{union Cyc_CfFlowInfo_AbsLVal _tmp1DB=_tmp1E1;struct Cyc_CfFlowInfo_Place*_tmp1DE;if((_tmp1DB.PlaceL).tag == 1){_LL10F: _tmp1DE=(_tmp1DB.PlaceL).val;_LL110:
# 1377
({struct Cyc_Dict_Dict _tmp6F4=Cyc_CfFlowInfo_assign_place(_tmp125,_tmp2D7->loc,_tmp1E0,_tmp1DE,left_rval);_tmp1E0=_tmp6F4;});
goto _LL10E;}else{_LL111: _LL112:
# 1380
 if(Cyc_CfFlowInfo_initlevel(_tmp125,_tmp1E0,left_rval)!= Cyc_CfFlowInfo_AllIL)
({void*_tmp1DC=0U;({unsigned int _tmp6F6=_tmp2D8->loc;struct _dyneither_ptr _tmp6F5=({const char*_tmp1DD="expression may not be fully initialized";_tag_dyneither(_tmp1DD,sizeof(char),40U);});Cyc_CfFlowInfo_aerr(_tmp6F6,_tmp6F5,_tag_dyneither(_tmp1DC,sizeof(void*),0U));});});
goto _LL10E;}_LL10E:;}
# 1385
({struct Cyc_List_List*_tmp6F7=Cyc_Relations_reln_kill_exp(Cyc_Core_heap_region,_tmp1DF,_tmp2D8);_tmp1DF=_tmp6F7;});
({struct Cyc_List_List*_tmp6F8=Cyc_Relations_reln_kill_exp(Cyc_Core_heap_region,_tmp1DF,_tmp2D7);_tmp1DF=_tmp6F8;});
# 1388
({union Cyc_CfFlowInfo_FlowInfo _tmp6F9=Cyc_CfFlowInfo_ReachableFL(_tmp1E0,_tmp1DF);outflow=_tmp6F9;});
Cyc_NewControlFlow_update_tryflow(env,outflow);
goto _LL104;}else{_LL107: _LL108:
 goto _LL104;}_LL104:;}
# 1395
return({struct _tuple16 _tmp5D7;_tmp5D7.f1=outflow,_tmp5D7.f2=_tmp125->unknown_all;_tmp5D7;});};};};}case 16U: _LL4E: _tmp2DA=((struct Cyc_Absyn_New_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_tmp2D9=((struct Cyc_Absyn_New_e_Absyn_Raw_exp_struct*)_tmp138)->f2;_LL4F: {
# 1398
union Cyc_CfFlowInfo_FlowInfo outflow;
void*place_val;
if(_tmp2DA != 0){
struct _tuple23 _tmp1E6=({struct Cyc_NewControlFlow_AnalEnv*_tmp6FB=env;union Cyc_CfFlowInfo_FlowInfo _tmp6FA=inflow;Cyc_NewControlFlow_anal_Rexps(_tmp6FB,_tmp6FA,({struct Cyc_Absyn_Exp*_tmp1EA[2U];_tmp1EA[0]=_tmp2DA,_tmp1EA[1]=_tmp2D9;((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp1EA,sizeof(struct Cyc_Absyn_Exp*),2U));}),1,1);});struct _tuple23 _tmp1E7=_tmp1E6;union Cyc_CfFlowInfo_FlowInfo _tmp1E9;struct Cyc_List_List*_tmp1E8;_LL114: _tmp1E9=_tmp1E7.f1;_tmp1E8=_tmp1E7.f2;_LL115:;
outflow=_tmp1E9;
place_val=(void*)((struct Cyc_List_List*)_check_null(((struct Cyc_List_List*)_check_null(_tmp1E8))->tl))->hd;}else{
# 1405
struct _tuple16 _tmp1EB=Cyc_NewControlFlow_anal_Rexp(env,1,inflow,_tmp2D9);struct _tuple16 _tmp1EC=_tmp1EB;union Cyc_CfFlowInfo_FlowInfo _tmp1EE;void*_tmp1ED;_LL117: _tmp1EE=_tmp1EC.f1;_tmp1ED=_tmp1EC.f2;_LL118:;
outflow=_tmp1EE;
place_val=_tmp1ED;}
# 1410
if(Cyc_Tcutil_is_noalias_pointer((void*)_check_null(e->topt),1))
return({struct _tuple16 _tmp5D8;_tmp5D8.f1=outflow,({void*_tmp6FC=(void*)({struct Cyc_CfFlowInfo_UniquePtr_CfFlowInfo_AbsRVal_struct*_tmp1EF=_cycalloc(sizeof(*_tmp1EF));_tmp1EF->tag=5U,_tmp1EF->f1=place_val;_tmp1EF;});_tmp5D8.f2=_tmp6FC;});_tmp5D8;});else{
# 1414
void*root=(void*)({struct Cyc_CfFlowInfo_MallocPt_CfFlowInfo_Root_struct*_tmp1F5=_cycalloc(sizeof(*_tmp1F5));_tmp1F5->tag=1U,_tmp1F5->f1=_tmp2D9,_tmp1F5->f2=(void*)_check_null(e->topt);_tmp1F5;});
struct Cyc_CfFlowInfo_Place*place=({struct Cyc_CfFlowInfo_Place*_tmp1F4=_cycalloc(sizeof(*_tmp1F4));_tmp1F4->root=root,_tmp1F4->path=0;_tmp1F4;});
void*rval=(void*)({struct Cyc_CfFlowInfo_AddressOf_CfFlowInfo_AbsRVal_struct*_tmp1F3=_cycalloc(sizeof(*_tmp1F3));_tmp1F3->tag=4U,_tmp1F3->f1=place;_tmp1F3;});
union Cyc_CfFlowInfo_FlowInfo _tmp1F0=outflow;struct Cyc_Dict_Dict _tmp1F2;struct Cyc_List_List*_tmp1F1;if((_tmp1F0.BottomFL).tag == 1){_LL11A: _LL11B:
 return({struct _tuple16 _tmp5D9;_tmp5D9.f1=outflow,_tmp5D9.f2=rval;_tmp5D9;});}else{_LL11C: _tmp1F2=((_tmp1F0.ReachableFL).val).f1;_tmp1F1=((_tmp1F0.ReachableFL).val).f2;_LL11D:
# 1420
 return({struct _tuple16 _tmp5DA;({union Cyc_CfFlowInfo_FlowInfo _tmp6FE=({struct Cyc_Dict_Dict _tmp6FD=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,void*k,void*v))Cyc_Dict_insert)(_tmp1F2,root,place_val);Cyc_CfFlowInfo_ReachableFL(_tmp6FD,_tmp1F1);});_tmp5DA.f1=_tmp6FE;}),_tmp5DA.f2=rval;_tmp5DA;});}_LL119:;}}case 15U: _LL50: _tmp2DB=((struct Cyc_Absyn_Address_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_LL51: {
# 1425
struct _tuple18 _tmp1F6=Cyc_NewControlFlow_anal_Lexp(env,inflow,0,0,_tmp2DB);struct _tuple18 _tmp1F7=_tmp1F6;union Cyc_CfFlowInfo_FlowInfo _tmp1FC;union Cyc_CfFlowInfo_AbsLVal _tmp1FB;_LL11F: _tmp1FC=_tmp1F7.f1;_tmp1FB=_tmp1F7.f2;_LL120:;{
union Cyc_CfFlowInfo_AbsLVal _tmp1F8=_tmp1FB;struct Cyc_CfFlowInfo_Place*_tmp1FA;if((_tmp1F8.UnknownL).tag == 2){_LL122: _LL123:
 return({struct _tuple16 _tmp5DB;_tmp5DB.f1=_tmp1FC,_tmp5DB.f2=_tmp125->notzeroall;_tmp5DB;});}else{_LL124: _tmp1FA=(_tmp1F8.PlaceL).val;_LL125:
 return({struct _tuple16 _tmp5DC;_tmp5DC.f1=_tmp1FC,({void*_tmp6FF=(void*)({struct Cyc_CfFlowInfo_AddressOf_CfFlowInfo_AbsRVal_struct*_tmp1F9=_cycalloc(sizeof(*_tmp1F9));_tmp1F9->tag=4U,_tmp1F9->f1=_tmp1FA;_tmp1F9;});_tmp5DC.f2=_tmp6FF;});_tmp5DC;});}_LL121:;};}case 20U: _LL52: _tmp2DC=((struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_LL53: {
# 1432
struct _tuple16 _tmp1FD=Cyc_NewControlFlow_anal_Rexp(env,0,inflow,_tmp2DC);struct _tuple16 _tmp1FE=_tmp1FD;union Cyc_CfFlowInfo_FlowInfo _tmp200;void*_tmp1FF;_LL127: _tmp200=_tmp1FE.f1;_tmp1FF=_tmp1FE.f2;_LL128:;
return Cyc_NewControlFlow_anal_derefR(env,inflow,_tmp200,_tmp2DC,_tmp1FF);}case 21U: _LL54: _tmp2DE=((struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_tmp2DD=((struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_tmp138)->f2;_LL55: {
# 1436
struct _tuple16 _tmp201=Cyc_NewControlFlow_anal_Rexp(env,0,inflow,_tmp2DE);struct _tuple16 _tmp202=_tmp201;union Cyc_CfFlowInfo_FlowInfo _tmp212;void*_tmp211;_LL12A: _tmp212=_tmp202.f1;_tmp211=_tmp202.f2;_LL12B:;{
void*_tmp203=(void*)_check_null(_tmp2DE->topt);
# 1439
if(Cyc_Absyn_is_nontagged_nonrequire_union_type(_tmp203))
return({struct _tuple16 _tmp5DD;_tmp5DD.f1=_tmp212,({void*_tmp700=Cyc_CfFlowInfo_typ_to_absrval(_tmp125,(void*)_check_null(e->topt),0,_tmp125->unknown_all);_tmp5DD.f2=_tmp700;});_tmp5DD;});
if(Cyc_Absyn_is_require_union_type(_tmp203))
Cyc_NewControlFlow_check_union_requires(_tmp2DE->loc,_tmp203,_tmp2DD,_tmp212);{
struct _tuple15 _tmp204=Cyc_CfFlowInfo_unname_rval(_tmp211);struct _tuple15 _tmp205=_tmp204;void*_tmp210;_LL12D: _tmp210=_tmp205.f1;_LL12E:;{
void*_tmp206=_tmp210;int _tmp20F;int _tmp20E;struct _dyneither_ptr _tmp20D;if(((struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*)_tmp206)->tag == 6U){_LL130: _tmp20F=(((struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*)_tmp206)->f1).is_union;_tmp20E=(((struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*)_tmp206)->f1).fieldnum;_tmp20D=((struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*)_tmp206)->f2;_LL131: {
# 1446
int _tmp207=Cyc_CfFlowInfo_get_field_index((void*)_check_null(_tmp2DE->topt),_tmp2DD);
if((_tmp20F  && _tmp20E != - 1) && _tmp20E != _tmp207)
return({struct _tuple16 _tmp5DE;_tmp5DE.f1=_tmp212,({void*_tmp701=Cyc_CfFlowInfo_typ_to_absrval(_tmp125,(void*)_check_null(e->topt),1,_tmp125->unknown_none);_tmp5DE.f2=_tmp701;});_tmp5DE;});
return({struct _tuple16 _tmp5DF;_tmp5DF.f1=_tmp212,_tmp5DF.f2=*((void**)_check_dyneither_subscript(_tmp20D,sizeof(void*),_tmp207));_tmp5DF;});}}else{_LL132: _LL133:
# 1451
({void*_tmp208=0U;({struct Cyc___cycFILE*_tmp703=Cyc_stderr;struct _dyneither_ptr _tmp702=({const char*_tmp209="the bad rexp is :";_tag_dyneither(_tmp209,sizeof(char),18U);});Cyc_fprintf(_tmp703,_tmp702,_tag_dyneither(_tmp208,sizeof(void*),0U));});});
Cyc_CfFlowInfo_print_absrval(_tmp210);
({struct Cyc_String_pa_PrintArg_struct _tmp20C=({struct Cyc_String_pa_PrintArg_struct _tmp5E0;_tmp5E0.tag=0U,({struct _dyneither_ptr _tmp704=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_exp2string(e));_tmp5E0.f1=_tmp704;});_tmp5E0;});void*_tmp20A[1U];_tmp20A[0]=& _tmp20C;({struct _dyneither_ptr _tmp705=({const char*_tmp20B="\nanal_Rexp: AggrMember: %s";_tag_dyneither(_tmp20B,sizeof(char),27U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp705,_tag_dyneither(_tmp20A,sizeof(void*),1U));});});}_LL12F:;};};};}case 38U: _LL56: _tmp2E0=((struct Cyc_Absyn_Tagcheck_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_tmp2DF=((struct Cyc_Absyn_Tagcheck_e_Absyn_Raw_exp_struct*)_tmp138)->f2;_LL57: {
# 1459
struct _tuple16 _tmp213=Cyc_NewControlFlow_anal_Rexp(env,0,inflow,_tmp2E0);struct _tuple16 _tmp214=_tmp213;union Cyc_CfFlowInfo_FlowInfo _tmp221;void*_tmp220;_LL135: _tmp221=_tmp214.f1;_tmp220=_tmp214.f2;_LL136:;
# 1461
if(Cyc_Absyn_is_nontagged_nonrequire_union_type((void*)_check_null(_tmp2E0->topt)))
return({struct _tuple16 _tmp5E1;_tmp5E1.f1=_tmp221,_tmp5E1.f2=_tmp125->unknown_all;_tmp5E1;});{
struct _tuple15 _tmp215=Cyc_CfFlowInfo_unname_rval(_tmp220);struct _tuple15 _tmp216=_tmp215;void*_tmp21F;_LL138: _tmp21F=_tmp216.f1;_LL139:;{
void*_tmp217=_tmp21F;int _tmp21E;int _tmp21D;struct _dyneither_ptr _tmp21C;if(((struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*)_tmp217)->tag == 6U){_LL13B: _tmp21E=(((struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*)_tmp217)->f1).is_union;_tmp21D=(((struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*)_tmp217)->f1).fieldnum;_tmp21C=((struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*)_tmp217)->f2;_LL13C: {
# 1466
int _tmp218=Cyc_CfFlowInfo_get_field_index((void*)_check_null(_tmp2E0->topt),_tmp2DF);
if(_tmp21E  && _tmp21D != - 1){
if(_tmp21D != _tmp218)
return({struct _tuple16 _tmp5E2;_tmp5E2.f1=_tmp221,_tmp5E2.f2=_tmp125->zero;_tmp5E2;});else{
# 1471
return({struct _tuple16 _tmp5E3;_tmp5E3.f1=_tmp221,_tmp5E3.f2=_tmp125->notzeroall;_tmp5E3;});}}else{
# 1473
return({struct _tuple16 _tmp5E4;_tmp5E4.f1=_tmp221,_tmp5E4.f2=_tmp125->unknown_all;_tmp5E4;});}}}else{_LL13D: _LL13E:
# 1475
({struct Cyc_String_pa_PrintArg_struct _tmp21B=({struct Cyc_String_pa_PrintArg_struct _tmp5E5;_tmp5E5.tag=0U,({struct _dyneither_ptr _tmp706=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_exp2string(e));_tmp5E5.f1=_tmp706;});_tmp5E5;});void*_tmp219[1U];_tmp219[0]=& _tmp21B;({struct _dyneither_ptr _tmp707=({const char*_tmp21A="anal_Rexp: TagCheck_e: %s";_tag_dyneither(_tmp21A,sizeof(char),26U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp707,_tag_dyneither(_tmp219,sizeof(void*),1U));});});}_LL13A:;};};}case 22U: _LL58: _tmp2E2=((struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_tmp2E1=((struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_tmp138)->f2;_LL59: {
# 1479
struct _tuple16 _tmp222=Cyc_NewControlFlow_anal_Rexp(env,0,inflow,_tmp2E2);struct _tuple16 _tmp223=_tmp222;union Cyc_CfFlowInfo_FlowInfo _tmp238;void*_tmp237;_LL140: _tmp238=_tmp223.f1;_tmp237=_tmp223.f2;_LL141:;{
# 1482
struct _tuple16 _tmp224=Cyc_NewControlFlow_anal_derefR(env,inflow,_tmp238,_tmp2E2,_tmp237);struct _tuple16 _tmp225=_tmp224;union Cyc_CfFlowInfo_FlowInfo _tmp236;void*_tmp235;_LL143: _tmp236=_tmp225.f1;_tmp235=_tmp225.f2;_LL144:;{
# 1485
void*_tmp226=Cyc_Tcutil_compress((void*)_check_null(_tmp2E2->topt));void*_tmp227=_tmp226;void*_tmp234;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp227)->tag == 3U){_LL146: _tmp234=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp227)->f1).elt_type;_LL147:
# 1487
 if(Cyc_Absyn_is_nontagged_nonrequire_union_type(_tmp234))
# 1489
return({struct _tuple16 _tmp5E6;_tmp5E6.f1=_tmp236,({void*_tmp708=Cyc_CfFlowInfo_typ_to_absrval(_tmp125,(void*)_check_null(e->topt),0,_tmp125->unknown_all);_tmp5E6.f2=_tmp708;});_tmp5E6;});
# 1491
if(Cyc_Absyn_is_require_union_type(_tmp234))
Cyc_NewControlFlow_check_union_requires(_tmp2E2->loc,_tmp234,_tmp2E1,_tmp238);{
# 1494
struct _tuple15 _tmp228=Cyc_CfFlowInfo_unname_rval(_tmp235);struct _tuple15 _tmp229=_tmp228;void*_tmp231;_LL14B: _tmp231=_tmp229.f1;_LL14C:;{
void*_tmp22A=_tmp231;int _tmp230;int _tmp22F;struct _dyneither_ptr _tmp22E;if(((struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*)_tmp22A)->tag == 6U){_LL14E: _tmp230=(((struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*)_tmp22A)->f1).is_union;_tmp22F=(((struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*)_tmp22A)->f1).fieldnum;_tmp22E=((struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*)_tmp22A)->f2;_LL14F: {
# 1497
int _tmp22B=Cyc_CfFlowInfo_get_field_index(_tmp234,_tmp2E1);
if((_tmp230  && _tmp22F != - 1) && _tmp22F != _tmp22B)
return({struct _tuple16 _tmp5E7;_tmp5E7.f1=_tmp236,({void*_tmp709=Cyc_CfFlowInfo_typ_to_absrval(_tmp125,(void*)_check_null(e->topt),1,_tmp125->unknown_none);_tmp5E7.f2=_tmp709;});_tmp5E7;});
return({struct _tuple16 _tmp5E8;_tmp5E8.f1=_tmp236,_tmp5E8.f2=*((void**)_check_dyneither_subscript(_tmp22E,sizeof(void*),_tmp22B));_tmp5E8;});}}else{_LL150: _LL151:
({void*_tmp22C=0U;({struct _dyneither_ptr _tmp70A=({const char*_tmp22D="anal_Rexp: AggrArrow";_tag_dyneither(_tmp22D,sizeof(char),21U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp70A,_tag_dyneither(_tmp22C,sizeof(void*),0U));});});}_LL14D:;};};}else{_LL148: _LL149:
# 1503
({void*_tmp232=0U;({struct _dyneither_ptr _tmp70B=({const char*_tmp233="anal_Rexp: AggrArrow ptr";_tag_dyneither(_tmp233,sizeof(char),25U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp70B,_tag_dyneither(_tmp232,sizeof(void*),0U));});});}_LL145:;};};}case 6U: _LL5A: _tmp2E5=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_tmp2E4=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmp138)->f2;_tmp2E3=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmp138)->f3;_LL5B: {
# 1507
struct _tuple19 _tmp239=Cyc_NewControlFlow_anal_test(env,inflow,_tmp2E5);struct _tuple19 _tmp23A=_tmp239;union Cyc_CfFlowInfo_FlowInfo _tmp23E;union Cyc_CfFlowInfo_FlowInfo _tmp23D;_LL153: _tmp23E=_tmp23A.f1;_tmp23D=_tmp23A.f2;_LL154:;{
struct _tuple16 _tmp23B=Cyc_NewControlFlow_anal_Rexp(env,copy_ctxt,_tmp23E,_tmp2E4);
struct _tuple16 _tmp23C=Cyc_NewControlFlow_anal_Rexp(env,copy_ctxt,_tmp23D,_tmp2E3);
return Cyc_CfFlowInfo_join_flow_and_rval(_tmp125,_tmp23B,_tmp23C);};}case 7U: _LL5C: _tmp2E7=((struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_tmp2E6=((struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct*)_tmp138)->f2;_LL5D: {
# 1513
struct _tuple19 _tmp23F=Cyc_NewControlFlow_anal_test(env,inflow,_tmp2E7);struct _tuple19 _tmp240=_tmp23F;union Cyc_CfFlowInfo_FlowInfo _tmp248;union Cyc_CfFlowInfo_FlowInfo _tmp247;_LL156: _tmp248=_tmp240.f1;_tmp247=_tmp240.f2;_LL157:;{
struct _tuple16 _tmp241=Cyc_NewControlFlow_anal_Rexp(env,copy_ctxt,_tmp248,_tmp2E6);struct _tuple16 _tmp242=_tmp241;union Cyc_CfFlowInfo_FlowInfo _tmp246;void*_tmp245;_LL159: _tmp246=_tmp242.f1;_tmp245=_tmp242.f2;_LL15A:;{
struct _tuple16 _tmp243=({struct _tuple16 _tmp5EA;_tmp5EA.f1=_tmp246,_tmp5EA.f2=_tmp245;_tmp5EA;});
struct _tuple16 _tmp244=({struct _tuple16 _tmp5E9;_tmp5E9.f1=_tmp247,_tmp5E9.f2=_tmp125->zero;_tmp5E9;});
return Cyc_CfFlowInfo_join_flow_and_rval(_tmp125,_tmp243,_tmp244);};};}case 8U: _LL5E: _tmp2E9=((struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_tmp2E8=((struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct*)_tmp138)->f2;_LL5F: {
# 1520
struct _tuple19 _tmp249=Cyc_NewControlFlow_anal_test(env,inflow,_tmp2E9);struct _tuple19 _tmp24A=_tmp249;union Cyc_CfFlowInfo_FlowInfo _tmp252;union Cyc_CfFlowInfo_FlowInfo _tmp251;_LL15C: _tmp252=_tmp24A.f1;_tmp251=_tmp24A.f2;_LL15D:;{
struct _tuple16 _tmp24B=Cyc_NewControlFlow_anal_Rexp(env,copy_ctxt,_tmp251,_tmp2E8);struct _tuple16 _tmp24C=_tmp24B;union Cyc_CfFlowInfo_FlowInfo _tmp250;void*_tmp24F;_LL15F: _tmp250=_tmp24C.f1;_tmp24F=_tmp24C.f2;_LL160:;{
struct _tuple16 _tmp24D=({struct _tuple16 _tmp5EC;_tmp5EC.f1=_tmp250,_tmp5EC.f2=_tmp24F;_tmp5EC;});
struct _tuple16 _tmp24E=({struct _tuple16 _tmp5EB;_tmp5EB.f1=_tmp252,_tmp5EB.f2=_tmp125->notzeroall;_tmp5EB;});
return Cyc_CfFlowInfo_join_flow_and_rval(_tmp125,_tmp24D,_tmp24E);};};}case 23U: _LL60: _tmp2EB=((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_tmp2EA=((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmp138)->f2;_LL61: {
# 1527
struct _tuple23 _tmp253=({struct Cyc_NewControlFlow_AnalEnv*_tmp70D=env;union Cyc_CfFlowInfo_FlowInfo _tmp70C=inflow;Cyc_NewControlFlow_anal_Rexps(_tmp70D,_tmp70C,({struct Cyc_Absyn_Exp*_tmp26F[2U];_tmp26F[0]=_tmp2EB,_tmp26F[1]=_tmp2EA;((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp26F,sizeof(struct Cyc_Absyn_Exp*),2U));}),0,1);});struct _tuple23 _tmp254=_tmp253;union Cyc_CfFlowInfo_FlowInfo _tmp26E;struct Cyc_List_List*_tmp26D;_LL162: _tmp26E=_tmp254.f1;_tmp26D=_tmp254.f2;_LL163:;{
# 1531
union Cyc_CfFlowInfo_FlowInfo _tmp255=_tmp26E;
{union Cyc_CfFlowInfo_FlowInfo _tmp256=_tmp26E;struct Cyc_Dict_Dict _tmp25B;struct Cyc_List_List*_tmp25A;if((_tmp256.ReachableFL).tag == 2){_LL165: _tmp25B=((_tmp256.ReachableFL).val).f1;_tmp25A=((_tmp256.ReachableFL).val).f2;_LL166:
# 1536
 if(Cyc_CfFlowInfo_initlevel(env->fenv,_tmp25B,(void*)((struct Cyc_List_List*)_check_null(((struct Cyc_List_List*)_check_null(_tmp26D))->tl))->hd)== Cyc_CfFlowInfo_NoneIL)
({void*_tmp257=0U;({unsigned int _tmp70F=_tmp2EA->loc;struct _dyneither_ptr _tmp70E=({const char*_tmp258="expression may not be initialized";_tag_dyneither(_tmp258,sizeof(char),34U);});Cyc_CfFlowInfo_aerr(_tmp70F,_tmp70E,_tag_dyneither(_tmp257,sizeof(void*),0U));});});{
struct Cyc_List_List*_tmp259=Cyc_NewControlFlow_add_subscript_reln(_tmp25A,_tmp2EB,_tmp2EA);
if(_tmp25A != _tmp259)
({union Cyc_CfFlowInfo_FlowInfo _tmp710=Cyc_CfFlowInfo_ReachableFL(_tmp25B,_tmp259);_tmp255=_tmp710;});
goto _LL164;};}else{_LL167: _LL168:
 goto _LL164;}_LL164:;}{
# 1544
void*_tmp25C=Cyc_Tcutil_compress((void*)_check_null(_tmp2EB->topt));void*_tmp25D=_tmp25C;struct Cyc_List_List*_tmp26C;switch(*((int*)_tmp25D)){case 6U: _LL16A: _tmp26C=((struct Cyc_Absyn_TupleType_Absyn_Type_struct*)_tmp25D)->f1;_LL16B: {
# 1546
struct _tuple15 _tmp25E=Cyc_CfFlowInfo_unname_rval((void*)((struct Cyc_List_List*)_check_null(_tmp26D))->hd);struct _tuple15 _tmp25F=_tmp25E;void*_tmp264;_LL171: _tmp264=_tmp25F.f1;_LL172:;{
void*_tmp260=_tmp264;struct _dyneither_ptr _tmp263;if(((struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*)_tmp260)->tag == 6U){_LL174: _tmp263=((struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*)_tmp260)->f2;_LL175: {
# 1549
unsigned int i=(Cyc_Evexp_eval_const_uint_exp(_tmp2EA)).f1;
return({struct _tuple16 _tmp5ED;_tmp5ED.f1=_tmp255,_tmp5ED.f2=*((void**)_check_dyneither_subscript(_tmp263,sizeof(void*),(int)i));_tmp5ED;});}}else{_LL176: _LL177:
({void*_tmp261=0U;({struct _dyneither_ptr _tmp711=({const char*_tmp262="anal_Rexp: Subscript";_tag_dyneither(_tmp262,sizeof(char),21U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp711,_tag_dyneither(_tmp261,sizeof(void*),0U));});});}_LL173:;};}case 3U: _LL16C: _LL16D: {
# 1554
struct _tuple16 _tmp265=Cyc_NewControlFlow_anal_derefR(env,inflow,_tmp26E,_tmp2EB,(void*)((struct Cyc_List_List*)_check_null(_tmp26D))->hd);struct _tuple16 _tmp266=_tmp265;union Cyc_CfFlowInfo_FlowInfo _tmp269;void*_tmp268;_LL179: _tmp269=_tmp266.f1;_tmp268=_tmp266.f2;_LL17A:;{
union Cyc_CfFlowInfo_FlowInfo _tmp267=_tmp269;if((_tmp267.BottomFL).tag == 1){_LL17C: _LL17D:
 return({struct _tuple16 _tmp5EE;_tmp5EE.f1=_tmp269,_tmp5EE.f2=_tmp268;_tmp5EE;});}else{_LL17E: _LL17F:
 return({struct _tuple16 _tmp5EF;_tmp5EF.f1=_tmp255,_tmp5EF.f2=_tmp268;_tmp5EF;});}_LL17B:;};}default: _LL16E: _LL16F:
# 1559
({void*_tmp26A=0U;({struct _dyneither_ptr _tmp712=({const char*_tmp26B="anal_Rexp: Subscript -- bad type";_tag_dyneither(_tmp26B,sizeof(char),33U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp712,_tag_dyneither(_tmp26A,sizeof(void*),0U));});});}_LL169:;};};}case 24U: _LL64: _tmp2EC=((struct Cyc_Absyn_Tuple_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_LL65: {
# 1565
struct _tuple23 _tmp270=Cyc_NewControlFlow_anal_Rexps(env,inflow,_tmp2EC,1,1);struct _tuple23 _tmp271=_tmp270;union Cyc_CfFlowInfo_FlowInfo _tmp277;struct Cyc_List_List*_tmp276;_LL181: _tmp277=_tmp271.f1;_tmp276=_tmp271.f2;_LL182:;{
struct _dyneither_ptr aggrdict=({unsigned int _tmp275=(unsigned int)
((int(*)(struct Cyc_List_List*x))Cyc_List_length)(_tmp2EC);void**_tmp274=_cycalloc(_check_times(_tmp275,sizeof(void*)));({{unsigned int _tmp5F1=(unsigned int)((int(*)(struct Cyc_List_List*x))Cyc_List_length)(_tmp2EC);unsigned int i;for(i=0;i < _tmp5F1;++ i){({
void*_tmp713=({void*_tmp273=(void*)((struct Cyc_List_List*)_check_null(_tmp276))->hd;
_tmp276=_tmp276->tl;
_tmp273;});
# 1568
_tmp274[i]=_tmp713;});}}0;});_tag_dyneither(_tmp274,sizeof(void*),_tmp275);});
# 1572
return({struct _tuple16 _tmp5F0;_tmp5F0.f1=_tmp277,({void*_tmp714=(void*)({struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*_tmp272=_cycalloc(sizeof(*_tmp272));_tmp272->tag=6U,(_tmp272->f1).is_union=0,(_tmp272->f1).fieldnum=- 1,_tmp272->f2=aggrdict;_tmp272;});_tmp5F0.f2=_tmp714;});_tmp5F0;});};}case 30U: _LL66: _tmp2ED=((struct Cyc_Absyn_AnonStruct_e_Absyn_Raw_exp_struct*)_tmp138)->f2;_LL67: {
# 1575
struct Cyc_List_List*fs;
{void*_tmp278=Cyc_Tcutil_compress((void*)_check_null(e->topt));void*_tmp279=_tmp278;struct Cyc_List_List*_tmp27C;if(((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_tmp279)->tag == 7U){_LL184: _tmp27C=((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_tmp279)->f2;_LL185:
 fs=_tmp27C;goto _LL183;}else{_LL186: _LL187:
({void*_tmp27A=0U;({struct _dyneither_ptr _tmp715=({const char*_tmp27B="anal_Rexp:anon struct has bad type";_tag_dyneither(_tmp27B,sizeof(char),35U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp715,_tag_dyneither(_tmp27A,sizeof(void*),0U));});});}_LL183:;}
# 1580
_tmp2F0=_tmp2ED;_tmp2EF=0U;_tmp2EE=fs;goto _LL69;}case 29U: if(((struct Cyc_Absyn_Aggregate_e_Absyn_Raw_exp_struct*)_tmp138)->f4 != 0){if(((struct Cyc_Absyn_Aggrdecl*)((struct Cyc_Absyn_Aggregate_e_Absyn_Raw_exp_struct*)_tmp138)->f4)->impl != 0){_LL68: _tmp2F0=((struct Cyc_Absyn_Aggregate_e_Absyn_Raw_exp_struct*)_tmp138)->f3;_tmp2EF=(((struct Cyc_Absyn_Aggregate_e_Absyn_Raw_exp_struct*)_tmp138)->f4)->kind;_tmp2EE=((((struct Cyc_Absyn_Aggregate_e_Absyn_Raw_exp_struct*)_tmp138)->f4)->impl)->fields;_LL69: {
# 1582
void*exp_type=(void*)_check_null(e->topt);
struct _tuple23 _tmp27D=({struct Cyc_NewControlFlow_AnalEnv*_tmp717=env;union Cyc_CfFlowInfo_FlowInfo _tmp716=inflow;Cyc_NewControlFlow_anal_Rexps(_tmp717,_tmp716,((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*f)(struct _tuple26*),struct Cyc_List_List*x))Cyc_List_map)((struct Cyc_Absyn_Exp*(*)(struct _tuple26*))Cyc_Core_snd,_tmp2F0),1,1);});struct _tuple23 _tmp27E=_tmp27D;union Cyc_CfFlowInfo_FlowInfo _tmp288;struct Cyc_List_List*_tmp287;_LL189: _tmp288=_tmp27E.f1;_tmp287=_tmp27E.f2;_LL18A:;{
struct _dyneither_ptr aggrdict=
Cyc_CfFlowInfo_aggrfields_to_aggrdict(_tmp125,_tmp2EE,_tmp2EF == Cyc_Absyn_UnionA,_tmp125->unknown_all);
# 1587
int field_no=-1;
{int i=0;for(0;_tmp287 != 0;(_tmp287=_tmp287->tl,_tmp2F0=_tmp2F0->tl,++ i)){
struct Cyc_List_List*ds=(*((struct _tuple26*)((struct Cyc_List_List*)_check_null(_tmp2F0))->hd)).f1;for(0;ds != 0;ds=ds->tl){
void*_tmp27F=(void*)ds->hd;void*_tmp280=_tmp27F;struct _dyneither_ptr*_tmp284;if(((struct Cyc_Absyn_ArrayElement_Absyn_Designator_struct*)_tmp280)->tag == 0U){_LL18C: _LL18D:
({void*_tmp281=0U;({struct _dyneither_ptr _tmp718=({const char*_tmp282="anal_Rexp:Aggregate_e";_tag_dyneither(_tmp282,sizeof(char),22U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp718,_tag_dyneither(_tmp281,sizeof(void*),0U));});});}else{_LL18E: _tmp284=((struct Cyc_Absyn_FieldName_Absyn_Designator_struct*)_tmp280)->f1;_LL18F:
# 1594
({int _tmp719=Cyc_CfFlowInfo_get_field_index_fs(_tmp2EE,_tmp284);field_no=_tmp719;});
*((void**)_check_dyneither_subscript(aggrdict,sizeof(void*),field_no))=(void*)_tmp287->hd;
# 1597
if(_tmp2EF == Cyc_Absyn_UnionA){
struct Cyc_Absyn_Exp*_tmp283=(*((struct _tuple26*)_tmp2F0->hd)).f2;
({union Cyc_CfFlowInfo_FlowInfo _tmp71A=Cyc_NewControlFlow_use_Rval(env,_tmp283->loc,_tmp288,(void*)_tmp287->hd);_tmp288=_tmp71A;});
# 1601
Cyc_NewControlFlow_check_union_requires(_tmp283->loc,exp_type,_tmp284,_tmp288);}}_LL18B:;}}}{
# 1604
struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*_tmp285=({struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct*_tmp286=_cycalloc(sizeof(*_tmp286));_tmp286->tag=6U,(_tmp286->f1).is_union=_tmp2EF == Cyc_Absyn_UnionA,(_tmp286->f1).fieldnum=field_no,_tmp286->f2=aggrdict;_tmp286;});
return({struct _tuple16 _tmp5F2;_tmp5F2.f1=_tmp288,_tmp5F2.f2=(void*)_tmp285;_tmp5F2;});};};}}else{goto _LL6A;}}else{_LL6A: _LL6B:
# 1608
({struct Cyc_String_pa_PrintArg_struct _tmp28B=({struct Cyc_String_pa_PrintArg_struct _tmp5F3;_tmp5F3.tag=0U,({struct _dyneither_ptr _tmp71B=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_exp2string(e));_tmp5F3.f1=_tmp71B;});_tmp5F3;});void*_tmp289[1U];_tmp289[0]=& _tmp28B;({struct _dyneither_ptr _tmp71C=({const char*_tmp28A="anal_Rexp:missing aggrdeclimpl in %s";_tag_dyneither(_tmp28A,sizeof(char),37U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp71C,_tag_dyneither(_tmp289,sizeof(void*),1U));});});}case 26U: _LL6C: _tmp2F1=((struct Cyc_Absyn_Array_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_LL6D: {
# 1610
struct Cyc_List_List*_tmp28C=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*f)(struct _tuple26*),struct Cyc_List_List*x))Cyc_List_map)((struct Cyc_Absyn_Exp*(*)(struct _tuple26*))Cyc_Core_snd,_tmp2F1);
struct _tuple23 _tmp28D=Cyc_NewControlFlow_anal_Rexps(env,inflow,_tmp28C,1,1);struct _tuple23 _tmp28E=_tmp28D;union Cyc_CfFlowInfo_FlowInfo _tmp290;struct Cyc_List_List*_tmp28F;_LL191: _tmp290=_tmp28E.f1;_tmp28F=_tmp28E.f2;_LL192:;
for(0;_tmp28F != 0;(_tmp28F=_tmp28F->tl,_tmp28C=_tmp28C->tl)){
({union Cyc_CfFlowInfo_FlowInfo _tmp71D=Cyc_NewControlFlow_use_Rval(env,((struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)_check_null(_tmp28C))->hd)->loc,_tmp290,(void*)_tmp28F->hd);_tmp290=_tmp71D;});}
return({struct _tuple16 _tmp5F4;_tmp5F4.f1=_tmp290,({void*_tmp71E=Cyc_CfFlowInfo_typ_to_absrval(_tmp125,(void*)_check_null(e->topt),0,_tmp125->unknown_all);_tmp5F4.f2=_tmp71E;});_tmp5F4;});}case 27U: _LL6E: _tmp2F5=((struct Cyc_Absyn_Comprehension_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_tmp2F4=((struct Cyc_Absyn_Comprehension_e_Absyn_Raw_exp_struct*)_tmp138)->f2;_tmp2F3=((struct Cyc_Absyn_Comprehension_e_Absyn_Raw_exp_struct*)_tmp138)->f3;_tmp2F2=((struct Cyc_Absyn_Comprehension_e_Absyn_Raw_exp_struct*)_tmp138)->f4;_LL6F: {
# 1618
struct _tuple16 _tmp291=Cyc_NewControlFlow_anal_Rexp(env,1,inflow,_tmp2F4);struct _tuple16 _tmp292=_tmp291;union Cyc_CfFlowInfo_FlowInfo _tmp2AD;void*_tmp2AC;_LL194: _tmp2AD=_tmp292.f1;_tmp2AC=_tmp292.f2;_LL195:;{
union Cyc_CfFlowInfo_FlowInfo _tmp293=_tmp2AD;struct Cyc_Dict_Dict _tmp2AB;struct Cyc_List_List*_tmp2AA;if((_tmp293.BottomFL).tag == 1){_LL197: _LL198:
 return({struct _tuple16 _tmp5F5;_tmp5F5.f1=_tmp2AD,_tmp5F5.f2=_tmp125->unknown_all;_tmp5F5;});}else{_LL199: _tmp2AB=((_tmp293.ReachableFL).val).f1;_tmp2AA=((_tmp293.ReachableFL).val).f2;_LL19A:
# 1622
 if(Cyc_CfFlowInfo_initlevel(env->fenv,_tmp2AB,_tmp2AC)== Cyc_CfFlowInfo_NoneIL)
({void*_tmp294=0U;({unsigned int _tmp720=_tmp2F4->loc;struct _dyneither_ptr _tmp71F=({const char*_tmp295="expression may not be initialized";_tag_dyneither(_tmp295,sizeof(char),34U);});Cyc_CfFlowInfo_aerr(_tmp720,_tmp71F,_tag_dyneither(_tmp294,sizeof(void*),0U));});});{
# 1626
struct Cyc_List_List*new_relns=_tmp2AA;
union Cyc_Relations_RelnOp n1=Cyc_Relations_RVar(_tmp2F5);
union Cyc_Relations_RelnOp n2=Cyc_Relations_RConst(0U);
if(Cyc_Relations_exp2relnop(_tmp2F4,& n2))
({struct Cyc_List_List*_tmp721=Cyc_Relations_add_relation(Cyc_Core_heap_region,n1,Cyc_Relations_Rlt,n2,_tmp2AA);new_relns=_tmp721;});
if(_tmp2AA != new_relns)
({union Cyc_CfFlowInfo_FlowInfo _tmp722=Cyc_CfFlowInfo_ReachableFL(_tmp2AB,new_relns);_tmp2AD=_tmp722;});{
# 1635
void*_tmp296=_tmp2AC;switch(*((int*)_tmp296)){case 0U: _LL19C: _LL19D:
 return({struct _tuple16 _tmp5F6;_tmp5F6.f1=_tmp2AD,_tmp5F6.f2=_tmp125->unknown_all;_tmp5F6;});case 1U: _LL19E: _LL19F:
 goto _LL1A1;case 4U: _LL1A0: _LL1A1: {
# 1639
struct Cyc_List_List _tmp297=({struct Cyc_List_List _tmp5F9;_tmp5F9.hd=_tmp2F5,_tmp5F9.tl=0;_tmp5F9;});
({union Cyc_CfFlowInfo_FlowInfo _tmp723=Cyc_NewControlFlow_add_vars(_tmp125,_tmp2AD,& _tmp297,_tmp125->unknown_all,e->loc,0);_tmp2AD=_tmp723;});{
# 1642
struct _tuple16 _tmp298=Cyc_NewControlFlow_anal_Rexp(env,1,_tmp2AD,_tmp2F3);struct _tuple16 _tmp299=_tmp298;union Cyc_CfFlowInfo_FlowInfo _tmp29F;void*_tmp29E;_LL1A5: _tmp29F=_tmp299.f1;_tmp29E=_tmp299.f2;_LL1A6:;
{union Cyc_CfFlowInfo_FlowInfo _tmp29A=_tmp29F;struct Cyc_Dict_Dict _tmp29D;if((_tmp29A.BottomFL).tag == 1){_LL1A8: _LL1A9:
 return({struct _tuple16 _tmp5F7;_tmp5F7.f1=_tmp29F,_tmp5F7.f2=_tmp125->unknown_all;_tmp5F7;});}else{_LL1AA: _tmp29D=((_tmp29A.ReachableFL).val).f1;_LL1AB:
# 1646
 if(Cyc_CfFlowInfo_initlevel(_tmp125,_tmp29D,_tmp29E)!= Cyc_CfFlowInfo_AllIL){
({void*_tmp29B=0U;({unsigned int _tmp725=_tmp2F4->loc;struct _dyneither_ptr _tmp724=({const char*_tmp29C="expression may not be initialized";_tag_dyneither(_tmp29C,sizeof(char),34U);});Cyc_CfFlowInfo_aerr(_tmp725,_tmp724,_tag_dyneither(_tmp29B,sizeof(void*),0U));});});
return({struct _tuple16 _tmp5F8;({union Cyc_CfFlowInfo_FlowInfo _tmp726=Cyc_CfFlowInfo_BottomFL();_tmp5F8.f1=_tmp726;}),_tmp5F8.f2=_tmp125->unknown_all;_tmp5F8;});}}_LL1A7:;}
# 1651
_tmp2AD=_tmp29F;
goto _LL1A3;};}default: _LL1A2: _LL1A3:
# 1654
 while(1){
struct Cyc_List_List _tmp2A0=({struct Cyc_List_List _tmp5FB;_tmp5FB.hd=_tmp2F5,_tmp5FB.tl=0;_tmp5FB;});
({union Cyc_CfFlowInfo_FlowInfo _tmp727=Cyc_NewControlFlow_add_vars(_tmp125,_tmp2AD,& _tmp2A0,_tmp125->unknown_all,e->loc,0);_tmp2AD=_tmp727;});{
struct _tuple16 _tmp2A1=Cyc_NewControlFlow_anal_Rexp(env,1,_tmp2AD,_tmp2F3);struct _tuple16 _tmp2A2=_tmp2A1;union Cyc_CfFlowInfo_FlowInfo _tmp2A9;void*_tmp2A8;_LL1AD: _tmp2A9=_tmp2A2.f1;_tmp2A8=_tmp2A2.f2;_LL1AE:;
{union Cyc_CfFlowInfo_FlowInfo _tmp2A3=_tmp2A9;struct Cyc_Dict_Dict _tmp2A6;if((_tmp2A3.BottomFL).tag == 1){_LL1B0: _LL1B1:
 goto _LL1AF;}else{_LL1B2: _tmp2A6=((_tmp2A3.ReachableFL).val).f1;_LL1B3:
# 1661
 if(Cyc_CfFlowInfo_initlevel(_tmp125,_tmp2A6,_tmp2A8)!= Cyc_CfFlowInfo_AllIL){
({void*_tmp2A4=0U;({unsigned int _tmp729=_tmp2F4->loc;struct _dyneither_ptr _tmp728=({const char*_tmp2A5="expression may not be initialized";_tag_dyneither(_tmp2A5,sizeof(char),34U);});Cyc_CfFlowInfo_aerr(_tmp729,_tmp728,_tag_dyneither(_tmp2A4,sizeof(void*),0U));});});
return({struct _tuple16 _tmp5FA;({union Cyc_CfFlowInfo_FlowInfo _tmp72A=Cyc_CfFlowInfo_BottomFL();_tmp5FA.f1=_tmp72A;}),_tmp5FA.f2=_tmp125->unknown_all;_tmp5FA;});}}_LL1AF:;}{
# 1666
union Cyc_CfFlowInfo_FlowInfo _tmp2A7=Cyc_CfFlowInfo_join_flow(_tmp125,_tmp2AD,_tmp2A9);
if(Cyc_CfFlowInfo_flow_lessthan_approx(_tmp2A7,_tmp2AD))
break;
_tmp2AD=_tmp2A7;};};}
# 1671
return({struct _tuple16 _tmp5FC;_tmp5FC.f1=_tmp2AD,_tmp5FC.f2=_tmp125->unknown_all;_tmp5FC;});}_LL19B:;};};}_LL196:;};}case 28U: _LL70: _tmp2F8=((struct Cyc_Absyn_ComprehensionNoinit_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_tmp2F7=(void*)((struct Cyc_Absyn_ComprehensionNoinit_e_Absyn_Raw_exp_struct*)_tmp138)->f2;_tmp2F6=((struct Cyc_Absyn_ComprehensionNoinit_e_Absyn_Raw_exp_struct*)_tmp138)->f3;_LL71: {
# 1677
void*root=(void*)({struct Cyc_CfFlowInfo_MallocPt_CfFlowInfo_Root_struct*_tmp2B6=_cycalloc(sizeof(*_tmp2B6));_tmp2B6->tag=1U,_tmp2B6->f1=_tmp2F8,_tmp2B6->f2=(void*)_check_null(e->topt);_tmp2B6;});
struct Cyc_CfFlowInfo_Place*place=({struct Cyc_CfFlowInfo_Place*_tmp2B5=_cycalloc(sizeof(*_tmp2B5));_tmp2B5->root=root,_tmp2B5->path=0;_tmp2B5;});
void*rval=(void*)({struct Cyc_CfFlowInfo_AddressOf_CfFlowInfo_AbsRVal_struct*_tmp2B4=_cycalloc(sizeof(*_tmp2B4));_tmp2B4->tag=4U,_tmp2B4->f1=place;_tmp2B4;});
void*place_val;
# 1685
({void*_tmp72B=Cyc_CfFlowInfo_typ_to_absrval(_tmp125,_tmp2F7,0,_tmp125->unknown_none);place_val=_tmp72B;});{
union Cyc_CfFlowInfo_FlowInfo outflow;
struct _tuple16 _tmp2AE=Cyc_NewControlFlow_anal_Rexp(env,1,inflow,_tmp2F8);struct _tuple16 _tmp2AF=_tmp2AE;union Cyc_CfFlowInfo_FlowInfo _tmp2B3;_LL1B5: _tmp2B3=_tmp2AF.f1;_LL1B6:;
outflow=_tmp2B3;{
union Cyc_CfFlowInfo_FlowInfo _tmp2B0=outflow;struct Cyc_Dict_Dict _tmp2B2;struct Cyc_List_List*_tmp2B1;if((_tmp2B0.BottomFL).tag == 1){_LL1B8: _LL1B9:
 return({struct _tuple16 _tmp5FD;_tmp5FD.f1=outflow,_tmp5FD.f2=rval;_tmp5FD;});}else{_LL1BA: _tmp2B2=((_tmp2B0.ReachableFL).val).f1;_tmp2B1=((_tmp2B0.ReachableFL).val).f2;_LL1BB:
# 1692
 return({struct _tuple16 _tmp5FE;({union Cyc_CfFlowInfo_FlowInfo _tmp72D=({struct Cyc_Dict_Dict _tmp72C=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,void*k,void*v))Cyc_Dict_insert)(_tmp2B2,root,place_val);Cyc_CfFlowInfo_ReachableFL(_tmp72C,_tmp2B1);});_tmp5FE.f1=_tmp72D;}),_tmp5FE.f2=rval;_tmp5FE;});}_LL1B7:;};};}case 37U: _LL72: _tmp2F9=((struct Cyc_Absyn_StmtExp_e_Absyn_Raw_exp_struct*)_tmp138)->f1;_LL73: {
# 1696
struct _tuple17 _tmp2B7=({struct _tuple17 _tmp600;_tmp600.f1=(env->fenv)->unknown_all,_tmp600.f2=copy_ctxt;_tmp600;});
union Cyc_CfFlowInfo_FlowInfo _tmp2B8=Cyc_NewControlFlow_anal_stmt(env,inflow,_tmp2F9,& _tmp2B7);
return({struct _tuple16 _tmp5FF;_tmp5FF.f1=_tmp2B8,_tmp5FF.f2=_tmp2B7.f1;_tmp5FF;});}case 36U: _LL76: _LL77:
# 1701
 goto _LL79;case 25U: _LL78: _LL79:
 goto _LL7B;case 39U: _LL7A: _LL7B:
 goto _LL7D;default: _LL7C: _LL7D:
({void*_tmp2B9=0U;({struct _dyneither_ptr _tmp72E=({const char*_tmp2BA="anal_Rexp, unexpected exp form";_tag_dyneither(_tmp2BA,sizeof(char),31U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp72E,_tag_dyneither(_tmp2B9,sizeof(void*),0U));});});}_LLD:;};}
# 1716 "new_control_flow.cyc"
static struct _tuple18 Cyc_NewControlFlow_anal_derefL(struct Cyc_NewControlFlow_AnalEnv*env,union Cyc_CfFlowInfo_FlowInfo inflow,struct Cyc_Absyn_Exp*e,union Cyc_CfFlowInfo_FlowInfo f,void*r,int passthrough_consumes,int expanded_place_possibly_null,struct Cyc_List_List*path){
# 1725
struct Cyc_CfFlowInfo_FlowEnv*_tmp2FA=env->fenv;
void*_tmp2FB=Cyc_Tcutil_compress((void*)_check_null(e->topt));void*_tmp2FC=_tmp2FB;void*_tmp320;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp2FC)->tag == 3U){_LL1: _tmp320=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp2FC)->f1).ptr_atts).bounds;_LL2: {
# 1728
union Cyc_CfFlowInfo_FlowInfo _tmp2FD=f;struct Cyc_Dict_Dict _tmp31D;struct Cyc_List_List*_tmp31C;if((_tmp2FD.BottomFL).tag == 1){_LL6: _LL7:
 return({struct _tuple18 _tmp601;_tmp601.f1=f,({union Cyc_CfFlowInfo_AbsLVal _tmp72F=Cyc_CfFlowInfo_UnknownL();_tmp601.f2=_tmp72F;});_tmp601;});}else{_LL8: _tmp31D=((_tmp2FD.ReachableFL).val).f1;_tmp31C=((_tmp2FD.ReachableFL).val).f2;_LL9: {
# 1732
struct _tuple15 _tmp2FE=Cyc_CfFlowInfo_unname_rval(r);struct _tuple15 _tmp2FF=_tmp2FE;void*_tmp31B;struct Cyc_List_List*_tmp31A;_LLB: _tmp31B=_tmp2FF.f1;_tmp31A=_tmp2FF.f2;_LLC:;
retry: {void*_tmp300=_tmp31B;void*_tmp311;enum Cyc_CfFlowInfo_InitLevel _tmp310;void*_tmp30F;struct Cyc_List_List*_tmp30E;void*_tmp30D;switch(*((int*)_tmp300)){case 8U: _LLE: _LLF:
# 1735
({void*_tmp301=0U;({struct _dyneither_ptr _tmp730=({const char*_tmp302="named location in anal_derefL";_tag_dyneither(_tmp302,sizeof(char),30U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp730,_tag_dyneither(_tmp301,sizeof(void*),0U));});});case 1U: _LL10: _LL11:
# 1737
({void*_tmp731=(void*)({struct Cyc_CfFlowInfo_NotZero_Absyn_AbsynAnnot_struct*_tmp303=_cycalloc(sizeof(*_tmp303));_tmp303->tag=Cyc_CfFlowInfo_NotZero,_tmp303->f1=_tmp31C;_tmp303;});e->annot=_tmp731;});goto _LLD;case 5U: _LL12: _tmp30D=(void*)((struct Cyc_CfFlowInfo_UniquePtr_CfFlowInfo_AbsRVal_struct*)_tmp300)->f1;_LL13:
# 1740
 if(expanded_place_possibly_null)
# 1743
({void*_tmp732=(void*)({struct Cyc_CfFlowInfo_UnknownZ_Absyn_AbsynAnnot_struct*_tmp304=_cycalloc(sizeof(*_tmp304));_tmp304->tag=Cyc_CfFlowInfo_UnknownZ,_tmp304->f1=_tmp31C;_tmp304;});e->annot=_tmp732;});else{
# 1746
void*_tmp305=e->annot;void*_tmp306=_tmp305;if(((struct Cyc_CfFlowInfo_UnknownZ_Absyn_AbsynAnnot_struct*)_tmp306)->tag == Cyc_CfFlowInfo_UnknownZ){_LL1F: _LL20:
# 1750
({void*_tmp733=(void*)({struct Cyc_CfFlowInfo_UnknownZ_Absyn_AbsynAnnot_struct*_tmp307=_cycalloc(sizeof(*_tmp307));_tmp307->tag=Cyc_CfFlowInfo_UnknownZ,_tmp307->f1=_tmp31C;_tmp307;});e->annot=_tmp733;});
goto _LL1E;}else{_LL21: _LL22:
# 1753
({void*_tmp734=(void*)({struct Cyc_CfFlowInfo_NotZero_Absyn_AbsynAnnot_struct*_tmp308=_cycalloc(sizeof(*_tmp308));_tmp308->tag=Cyc_CfFlowInfo_NotZero,_tmp308->f1=_tmp31C;_tmp308;});e->annot=_tmp734;});
goto _LL1E;}_LL1E:;}
# 1758
if(Cyc_Tcutil_is_bound_one(_tmp320))return({struct _tuple18 _tmp602;_tmp602.f1=f,({union Cyc_CfFlowInfo_AbsLVal _tmp735=Cyc_CfFlowInfo_UnknownL();_tmp602.f2=_tmp735;});_tmp602;});
goto _LLD;case 4U: _LL14: _tmp30F=(((struct Cyc_CfFlowInfo_AddressOf_CfFlowInfo_AbsRVal_struct*)_tmp300)->f1)->root;_tmp30E=(((struct Cyc_CfFlowInfo_AddressOf_CfFlowInfo_AbsRVal_struct*)_tmp300)->f1)->path;_LL15:
# 1762
({void*_tmp736=(void*)({struct Cyc_CfFlowInfo_NotZero_Absyn_AbsynAnnot_struct*_tmp309=_cycalloc(sizeof(*_tmp309));_tmp309->tag=Cyc_CfFlowInfo_NotZero,_tmp309->f1=_tmp31C;_tmp309;});e->annot=_tmp736;});
if(Cyc_Tcutil_is_bound_one(_tmp320))
return({struct _tuple18 _tmp603;_tmp603.f1=f,({union Cyc_CfFlowInfo_AbsLVal _tmp738=Cyc_CfFlowInfo_PlaceL(({struct Cyc_CfFlowInfo_Place*_tmp30A=_cycalloc(sizeof(*_tmp30A));_tmp30A->root=_tmp30F,({struct Cyc_List_List*_tmp737=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_append)(_tmp30E,path);_tmp30A->path=_tmp737;});_tmp30A;}));_tmp603.f2=_tmp738;});_tmp603;});
goto _LLD;case 0U: _LL16: _LL17:
# 1767
 e->annot=(void*)& Cyc_CfFlowInfo_IsZero_val;
return({struct _tuple18 _tmp604;({union Cyc_CfFlowInfo_FlowInfo _tmp73A=Cyc_CfFlowInfo_BottomFL();_tmp604.f1=_tmp73A;}),({union Cyc_CfFlowInfo_AbsLVal _tmp739=Cyc_CfFlowInfo_UnknownL();_tmp604.f2=_tmp739;});_tmp604;});case 2U: _LL18: _tmp310=((struct Cyc_CfFlowInfo_UnknownR_CfFlowInfo_AbsRVal_struct*)_tmp300)->f1;_LL19:
# 1772
 if(Cyc_Tcutil_is_bound_one(_tmp320))
({union Cyc_CfFlowInfo_FlowInfo _tmp73B=Cyc_NewControlFlow_notzero(env,inflow,f,e,_tmp310,_tmp31A);f=_tmp73B;});
({void*_tmp73C=(void*)({struct Cyc_CfFlowInfo_UnknownZ_Absyn_AbsynAnnot_struct*_tmp30B=_cycalloc(sizeof(*_tmp30B));_tmp30B->tag=Cyc_CfFlowInfo_UnknownZ,_tmp30B->f1=_tmp31C;_tmp30B;});e->annot=_tmp73C;});
goto _LLD;case 7U: _LL1A: _tmp311=(void*)((struct Cyc_CfFlowInfo_Consumed_CfFlowInfo_AbsRVal_struct*)_tmp300)->f3;_LL1B:
# 1777
 if(passthrough_consumes){
_tmp31B=_tmp311;goto retry;}
# 1780
goto _LL1D;default: _LL1C: _LL1D:
# 1782
({void*_tmp73D=(void*)({struct Cyc_CfFlowInfo_UnknownZ_Absyn_AbsynAnnot_struct*_tmp30C=_cycalloc(sizeof(*_tmp30C));_tmp30C->tag=Cyc_CfFlowInfo_UnknownZ,_tmp30C->f1=_tmp31C;_tmp30C;});e->annot=_tmp73D;});
goto _LLD;}_LLD:;}
# 1785
if(Cyc_CfFlowInfo_initlevel(_tmp2FA,_tmp31D,_tmp31B)== Cyc_CfFlowInfo_NoneIL){
struct _tuple15 _tmp312=Cyc_CfFlowInfo_unname_rval(_tmp31B);struct _tuple15 _tmp313=_tmp312;void*_tmp319;_LL24: _tmp319=_tmp313.f1;_LL25:;{
void*_tmp314=_tmp319;if(((struct Cyc_CfFlowInfo_Consumed_CfFlowInfo_AbsRVal_struct*)_tmp314)->tag == 7U){_LL27: _LL28:
# 1789
({void*_tmp315=0U;({unsigned int _tmp73F=e->loc;struct _dyneither_ptr _tmp73E=({const char*_tmp316="attempt to dereference an alias-free that has already been copied";_tag_dyneither(_tmp316,sizeof(char),66U);});Cyc_CfFlowInfo_aerr(_tmp73F,_tmp73E,_tag_dyneither(_tmp315,sizeof(void*),0U));});});
goto _LL26;}else{_LL29: _LL2A:
# 1792
({void*_tmp317=0U;({unsigned int _tmp741=e->loc;struct _dyneither_ptr _tmp740=({const char*_tmp318="dereference of possibly uninitialized pointer";_tag_dyneither(_tmp318,sizeof(char),46U);});Cyc_CfFlowInfo_aerr(_tmp741,_tmp740,_tag_dyneither(_tmp317,sizeof(void*),0U));});});
goto _LL26;}_LL26:;};}
# 1796
return({struct _tuple18 _tmp605;_tmp605.f1=f,({union Cyc_CfFlowInfo_AbsLVal _tmp742=Cyc_CfFlowInfo_UnknownL();_tmp605.f2=_tmp742;});_tmp605;});}}_LL5:;}}else{_LL3: _LL4:
# 1798
({void*_tmp31E=0U;({struct _dyneither_ptr _tmp743=({const char*_tmp31F="left deref of non-pointer-type";_tag_dyneither(_tmp31F,sizeof(char),31U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp743,_tag_dyneither(_tmp31E,sizeof(void*),0U));});});}_LL0:;}
# 1804
static struct Cyc_CfFlowInfo_Place*Cyc_NewControlFlow_make_expanded_place(struct Cyc_CfFlowInfo_Place*p,struct Cyc_List_List*path){
struct Cyc_CfFlowInfo_Place*_tmp321=p;void*_tmp326;struct Cyc_List_List*_tmp325;_LL1: _tmp326=_tmp321->root;_tmp325=_tmp321->path;_LL2:;
({struct Cyc_List_List*_tmp745=({struct Cyc_List_List*_tmp323=_cycalloc(sizeof(*_tmp323));({void*_tmp744=(void*)({struct Cyc_CfFlowInfo_Star_CfFlowInfo_PathCon_struct*_tmp322=_cycalloc(sizeof(*_tmp322));_tmp322->tag=1U;_tmp322;});_tmp323->hd=_tmp744;}),_tmp323->tl=path;_tmp323;});path=_tmp745;});
return({struct Cyc_CfFlowInfo_Place*_tmp324=_cycalloc(sizeof(*_tmp324));_tmp324->root=_tmp326,({struct Cyc_List_List*_tmp746=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_append)(_tmp325,path);_tmp324->path=_tmp746;});_tmp324;});}
# 1814
static struct _tuple18 Cyc_NewControlFlow_anal_Lexp_rec(struct Cyc_NewControlFlow_AnalEnv*env,union Cyc_CfFlowInfo_FlowInfo inflow,int expand_unique,int passthrough_consumes,struct Cyc_Absyn_Exp*e,struct Cyc_List_List*path){
# 1818
struct Cyc_Dict_Dict d;
struct Cyc_CfFlowInfo_FlowEnv*_tmp327=env->fenv;
{union Cyc_CfFlowInfo_FlowInfo _tmp328=inflow;struct Cyc_Dict_Dict _tmp32A;struct Cyc_List_List*_tmp329;if((_tmp328.BottomFL).tag == 1){_LL1: _LL2:
 return({struct _tuple18 _tmp606;({union Cyc_CfFlowInfo_FlowInfo _tmp748=Cyc_CfFlowInfo_BottomFL();_tmp606.f1=_tmp748;}),({union Cyc_CfFlowInfo_AbsLVal _tmp747=Cyc_CfFlowInfo_UnknownL();_tmp606.f2=_tmp747;});_tmp606;});}else{_LL3: _tmp32A=((_tmp328.ReachableFL).val).f1;_tmp329=((_tmp328.ReachableFL).val).f2;_LL4:
# 1823
 d=_tmp32A;}_LL0:;}{
# 1827
void*_tmp32B=e->r;void*_tmp32C=_tmp32B;struct Cyc_Absyn_Exp*_tmp383;struct _dyneither_ptr*_tmp382;struct Cyc_Absyn_Exp*_tmp381;struct Cyc_Absyn_Exp*_tmp380;struct Cyc_Absyn_Exp*_tmp37F;struct Cyc_Absyn_Exp*_tmp37E;struct _dyneither_ptr*_tmp37D;struct Cyc_Absyn_Vardecl*_tmp37C;struct Cyc_Absyn_Vardecl*_tmp37B;struct Cyc_Absyn_Vardecl*_tmp37A;struct Cyc_Absyn_Vardecl*_tmp379;struct Cyc_Absyn_Exp*_tmp378;struct Cyc_Absyn_Exp*_tmp377;struct Cyc_Absyn_Exp*_tmp376;switch(*((int*)_tmp32C)){case 14U: _LL6: _tmp376=((struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_tmp32C)->f2;_LL7:
 _tmp377=_tmp376;goto _LL9;case 12U: _LL8: _tmp377=((struct Cyc_Absyn_NoInstantiate_e_Absyn_Raw_exp_struct*)_tmp32C)->f1;_LL9:
 _tmp378=_tmp377;goto _LLB;case 13U: _LLA: _tmp378=((struct Cyc_Absyn_Instantiate_e_Absyn_Raw_exp_struct*)_tmp32C)->f1;_LLB:
 return Cyc_NewControlFlow_anal_Lexp_rec(env,inflow,expand_unique,passthrough_consumes,_tmp378,path);case 1U: switch(*((int*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp32C)->f1)){case 3U: _LLC: _tmp379=((struct Cyc_Absyn_Param_b_Absyn_Binding_struct*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp32C)->f1)->f1;_LLD:
# 1832
 _tmp37A=_tmp379;goto _LLF;case 4U: _LLE: _tmp37A=((struct Cyc_Absyn_Local_b_Absyn_Binding_struct*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp32C)->f1)->f1;_LLF:
 _tmp37B=_tmp37A;goto _LL11;case 5U: _LL10: _tmp37B=((struct Cyc_Absyn_Pat_b_Absyn_Binding_struct*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp32C)->f1)->f1;_LL11:
# 1835
 return({struct _tuple18 _tmp607;_tmp607.f1=inflow,({union Cyc_CfFlowInfo_AbsLVal _tmp74A=Cyc_CfFlowInfo_PlaceL(({struct Cyc_CfFlowInfo_Place*_tmp32E=_cycalloc(sizeof(*_tmp32E));({void*_tmp749=(void*)({struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct*_tmp32D=_cycalloc(sizeof(*_tmp32D));_tmp32D->tag=0U,_tmp32D->f1=_tmp37B;_tmp32D;});_tmp32E->root=_tmp749;}),_tmp32E->path=path;_tmp32E;}));_tmp607.f2=_tmp74A;});_tmp607;});case 1U: _LL12: _tmp37C=((struct Cyc_Absyn_Global_b_Absyn_Binding_struct*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp32C)->f1)->f1;_LL13:
# 1837
 return({struct _tuple18 _tmp608;_tmp608.f1=inflow,({union Cyc_CfFlowInfo_AbsLVal _tmp74B=Cyc_CfFlowInfo_UnknownL();_tmp608.f2=_tmp74B;});_tmp608;});default: goto _LL1C;}case 22U: _LL14: _tmp37E=((struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_tmp32C)->f1;_tmp37D=((struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_tmp32C)->f2;_LL15:
# 1840
{void*_tmp32F=Cyc_Tcutil_compress((void*)_check_null(_tmp37E->topt));void*_tmp330=_tmp32F;void*_tmp335;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp330)->tag == 3U){_LL1F: _tmp335=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp330)->f1).elt_type;_LL20:
# 1842
 if(!Cyc_Absyn_is_nontagged_nonrequire_union_type(_tmp335)){
Cyc_NewControlFlow_check_union_requires(_tmp37E->loc,_tmp335,_tmp37D,inflow);
({struct Cyc_List_List*_tmp74E=({struct Cyc_List_List*_tmp332=_cycalloc(sizeof(*_tmp332));({void*_tmp74D=(void*)({struct Cyc_CfFlowInfo_Dot_CfFlowInfo_PathCon_struct*_tmp331=_cycalloc(sizeof(*_tmp331));_tmp331->tag=0U,({int _tmp74C=Cyc_CfFlowInfo_get_field_index(_tmp335,_tmp37D);_tmp331->f1=_tmp74C;});_tmp331;});_tmp332->hd=_tmp74D;}),_tmp332->tl=path;_tmp332;});path=_tmp74E;});}
# 1846
goto _LL1E;}else{_LL21: _LL22:
({void*_tmp333=0U;({struct _dyneither_ptr _tmp74F=({const char*_tmp334="anal_Lexp: AggrArrow ptr";_tag_dyneither(_tmp334,sizeof(char),25U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp74F,_tag_dyneither(_tmp333,sizeof(void*),0U));});});}_LL1E:;}
# 1849
_tmp37F=_tmp37E;goto _LL17;case 20U: _LL16: _tmp37F=((struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct*)_tmp32C)->f1;_LL17:
# 1853
 if(Cyc_Tcutil_is_noalias_pointer((void*)_check_null(_tmp37F->topt),1)){
# 1855
struct _tuple18 _tmp336=Cyc_NewControlFlow_anal_Lexp(env,inflow,expand_unique,passthrough_consumes,_tmp37F);struct _tuple18 _tmp337=_tmp336;union Cyc_CfFlowInfo_FlowInfo _tmp356;union Cyc_CfFlowInfo_AbsLVal _tmp355;_LL24: _tmp356=_tmp337.f1;_tmp355=_tmp337.f2;_LL25:;{
struct _tuple18 _tmp338=({struct _tuple18 _tmp60C;_tmp60C.f1=_tmp356,_tmp60C.f2=_tmp355;_tmp60C;});struct _tuple18 _tmp339=_tmp338;struct Cyc_Dict_Dict _tmp354;struct Cyc_List_List*_tmp353;struct Cyc_CfFlowInfo_Place*_tmp352;if(((_tmp339.f1).ReachableFL).tag == 2){if(((_tmp339.f2).PlaceL).tag == 1){_LL27: _tmp354=(((_tmp339.f1).ReachableFL).val).f1;_tmp353=(((_tmp339.f1).ReachableFL).val).f2;_tmp352=((_tmp339.f2).PlaceL).val;_LL28: {
# 1858
void*_tmp33A=Cyc_CfFlowInfo_lookup_place(_tmp354,_tmp352);
struct _tuple15 _tmp33B=Cyc_CfFlowInfo_unname_rval(_tmp33A);struct _tuple15 _tmp33C=_tmp33B;void*_tmp351;struct Cyc_List_List*_tmp350;_LL2C: _tmp351=_tmp33C.f1;_tmp350=_tmp33C.f2;_LL2D:;
if(expand_unique){
int _tmp33D=1;
void*_tmp33E=_tmp351;struct Cyc_Absyn_Vardecl*_tmp34B;void*_tmp34A;switch(*((int*)_tmp33E)){case 8U: _LL2F: _tmp34B=((struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*)_tmp33E)->f1;_tmp34A=(void*)((struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*)_tmp33E)->f2;_LL30:
# 1864
({void*_tmp33F=0U;({struct _dyneither_ptr _tmp750=({const char*_tmp340="bad named location in anal_Lexp:deref";_tag_dyneither(_tmp340,sizeof(char),38U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp750,_tag_dyneither(_tmp33F,sizeof(void*),0U));});});case 7U: switch(*((int*)((struct Cyc_CfFlowInfo_Consumed_CfFlowInfo_AbsRVal_struct*)_tmp33E)->f3)){case 4U: _LL31: _LL32:
 goto _LL34;case 5U: _LL35: _LL36:
# 1868
 goto _LL38;default: goto _LL3B;}case 4U: _LL33: _LL34:
# 1867
 return Cyc_NewControlFlow_anal_derefL(env,_tmp356,_tmp37F,_tmp356,_tmp33A,passthrough_consumes,0,path);case 5U: _LL37: _LL38: {
# 1872
struct _tuple18 _tmp341=Cyc_NewControlFlow_anal_derefL(env,_tmp356,_tmp37F,_tmp356,_tmp33A,passthrough_consumes,0,path);struct _tuple18 _tmp342=_tmp341;union Cyc_CfFlowInfo_FlowInfo _tmp343;_LL3E: _tmp343=_tmp342.f1;_LL3F:;
return({struct _tuple18 _tmp609;_tmp609.f1=_tmp343,({union Cyc_CfFlowInfo_AbsLVal _tmp751=Cyc_CfFlowInfo_PlaceL(Cyc_NewControlFlow_make_expanded_place(_tmp352,path));_tmp609.f2=_tmp751;});_tmp609;});}case 1U: _LL39: _LL3A:
# 1875
 _tmp33D=0;goto _LL3C;default: _LL3B: _LL3C: {
# 1882
enum Cyc_CfFlowInfo_InitLevel il=Cyc_CfFlowInfo_initlevel(_tmp327,_tmp354,_tmp351);
void*leaf=il == Cyc_CfFlowInfo_AllIL?_tmp327->unknown_all: _tmp327->unknown_none;
void*res=(void*)({struct Cyc_CfFlowInfo_UniquePtr_CfFlowInfo_AbsRVal_struct*_tmp349=_cycalloc(sizeof(*_tmp349));_tmp349->tag=5U,({void*_tmp754=({struct Cyc_CfFlowInfo_FlowEnv*_tmp753=_tmp327;void*_tmp752=Cyc_Tcutil_pointer_elt_type((void*)_check_null(_tmp37F->topt));Cyc_CfFlowInfo_typ_to_absrval(_tmp753,_tmp752,0,leaf);});_tmp349->f1=_tmp754;});_tmp349;});
for(0;_tmp350 != 0;_tmp350=_tmp350->tl){
({void*_tmp755=(void*)({struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*_tmp344=_cycalloc(sizeof(*_tmp344));_tmp344->tag=8U,_tmp344->f1=(struct Cyc_Absyn_Vardecl*)_tmp350->hd,_tmp344->f2=res;_tmp344;});res=_tmp755;});}
({struct Cyc_Dict_Dict _tmp756=Cyc_CfFlowInfo_assign_place(_tmp327,e->loc,_tmp354,_tmp352,res);_tmp354=_tmp756;});{
union Cyc_CfFlowInfo_FlowInfo _tmp345=Cyc_CfFlowInfo_ReachableFL(_tmp354,_tmp353);
# 1895
struct _tuple18 _tmp346=Cyc_NewControlFlow_anal_derefL(env,_tmp345,_tmp37F,_tmp345,res,passthrough_consumes,_tmp33D,path);struct _tuple18 _tmp347=_tmp346;union Cyc_CfFlowInfo_FlowInfo _tmp348;_LL41: _tmp348=_tmp347.f1;_LL42:;
# 1897
return({struct _tuple18 _tmp60A;_tmp60A.f1=_tmp348,({union Cyc_CfFlowInfo_AbsLVal _tmp757=Cyc_CfFlowInfo_PlaceL(Cyc_NewControlFlow_make_expanded_place(_tmp352,path));_tmp60A.f2=_tmp757;});_tmp60A;});};}}_LL2E:;}else{
# 1901
void*_tmp34C=_tmp351;switch(*((int*)_tmp34C)){case 7U: if(((struct Cyc_CfFlowInfo_UniquePtr_CfFlowInfo_AbsRVal_struct*)((struct Cyc_CfFlowInfo_Consumed_CfFlowInfo_AbsRVal_struct*)_tmp34C)->f3)->tag == 5U){_LL44: _LL45:
 goto _LL47;}else{goto _LL48;}case 5U: _LL46: _LL47: {
# 1906
struct _tuple18 _tmp34D=Cyc_NewControlFlow_anal_derefL(env,_tmp356,_tmp37F,_tmp356,_tmp33A,passthrough_consumes,0,path);struct _tuple18 _tmp34E=_tmp34D;union Cyc_CfFlowInfo_FlowInfo _tmp34F;_LL4B: _tmp34F=_tmp34E.f1;_LL4C:;
return({struct _tuple18 _tmp60B;_tmp60B.f1=_tmp34F,({union Cyc_CfFlowInfo_AbsLVal _tmp758=Cyc_CfFlowInfo_PlaceL(Cyc_NewControlFlow_make_expanded_place(_tmp352,path));_tmp60B.f2=_tmp758;});_tmp60B;});}default: _LL48: _LL49:
# 1910
 goto _LL43;}_LL43:;}
# 1913
goto _LL26;}}else{goto _LL29;}}else{_LL29: _LL2A:
 goto _LL26;}_LL26:;};}{
# 1918
struct _tuple16 _tmp357=Cyc_NewControlFlow_anal_Rexp(env,0,inflow,_tmp37F);struct _tuple16 _tmp358=_tmp357;union Cyc_CfFlowInfo_FlowInfo _tmp35A;void*_tmp359;_LL4E: _tmp35A=_tmp358.f1;_tmp359=_tmp358.f2;_LL4F:;
# 1920
return Cyc_NewControlFlow_anal_derefL(env,inflow,_tmp37F,_tmp35A,_tmp359,passthrough_consumes,0,path);};case 23U: _LL18: _tmp381=((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmp32C)->f1;_tmp380=((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmp32C)->f2;_LL19: {
# 1923
void*_tmp35B=Cyc_Tcutil_compress((void*)_check_null(_tmp381->topt));void*_tmp35C=_tmp35B;switch(*((int*)_tmp35C)){case 6U: _LL51: _LL52: {
# 1925
unsigned int _tmp35D=(Cyc_Evexp_eval_const_uint_exp(_tmp380)).f1;
return({struct Cyc_NewControlFlow_AnalEnv*_tmp75E=env;union Cyc_CfFlowInfo_FlowInfo _tmp75D=inflow;int _tmp75C=expand_unique;int _tmp75B=passthrough_consumes;struct Cyc_Absyn_Exp*_tmp75A=_tmp381;Cyc_NewControlFlow_anal_Lexp_rec(_tmp75E,_tmp75D,_tmp75C,_tmp75B,_tmp75A,({struct Cyc_List_List*_tmp35F=_cycalloc(sizeof(*_tmp35F));
({void*_tmp759=(void*)({struct Cyc_CfFlowInfo_Dot_CfFlowInfo_PathCon_struct*_tmp35E=_cycalloc(sizeof(*_tmp35E));_tmp35E->tag=0U,_tmp35E->f1=(int)_tmp35D;_tmp35E;});_tmp35F->hd=_tmp759;}),_tmp35F->tl=path;_tmp35F;}));});}case 3U: _LL53: _LL54: {
# 1929
struct _tuple23 _tmp360=({struct Cyc_NewControlFlow_AnalEnv*_tmp760=env;union Cyc_CfFlowInfo_FlowInfo _tmp75F=inflow;Cyc_NewControlFlow_anal_Rexps(_tmp760,_tmp75F,({struct Cyc_Absyn_Exp*_tmp370[2U];_tmp370[0]=_tmp381,_tmp370[1]=_tmp380;((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp370,sizeof(struct Cyc_Absyn_Exp*),2U));}),0,1);});struct _tuple23 _tmp361=_tmp360;union Cyc_CfFlowInfo_FlowInfo _tmp36F;struct Cyc_List_List*_tmp36E;_LL58: _tmp36F=_tmp361.f1;_tmp36E=_tmp361.f2;_LL59:;{
union Cyc_CfFlowInfo_FlowInfo _tmp362=_tmp36F;
{union Cyc_CfFlowInfo_FlowInfo _tmp363=_tmp36F;struct Cyc_Dict_Dict _tmp368;struct Cyc_List_List*_tmp367;if((_tmp363.ReachableFL).tag == 2){_LL5B: _tmp368=((_tmp363.ReachableFL).val).f1;_tmp367=((_tmp363.ReachableFL).val).f2;_LL5C:
# 1933
 if(Cyc_CfFlowInfo_initlevel(_tmp327,_tmp368,(void*)((struct Cyc_List_List*)_check_null(((struct Cyc_List_List*)_check_null(_tmp36E))->tl))->hd)== Cyc_CfFlowInfo_NoneIL)
({void*_tmp364=0U;({unsigned int _tmp762=_tmp380->loc;struct _dyneither_ptr _tmp761=({const char*_tmp365="expression may not be initialized";_tag_dyneither(_tmp365,sizeof(char),34U);});Cyc_CfFlowInfo_aerr(_tmp762,_tmp761,_tag_dyneither(_tmp364,sizeof(void*),0U));});});{
struct Cyc_List_List*_tmp366=Cyc_NewControlFlow_add_subscript_reln(_tmp367,_tmp381,_tmp380);
if(_tmp367 != _tmp366)
({union Cyc_CfFlowInfo_FlowInfo _tmp763=Cyc_CfFlowInfo_ReachableFL(_tmp368,_tmp366);_tmp362=_tmp763;});
goto _LL5A;};}else{_LL5D: _LL5E:
 goto _LL5A;}_LL5A:;}{
# 1942
struct _tuple18 _tmp369=Cyc_NewControlFlow_anal_derefL(env,inflow,_tmp381,_tmp36F,(void*)((struct Cyc_List_List*)_check_null(_tmp36E))->hd,passthrough_consumes,0,path);struct _tuple18 _tmp36A=_tmp369;union Cyc_CfFlowInfo_FlowInfo _tmp36D;union Cyc_CfFlowInfo_AbsLVal _tmp36C;_LL60: _tmp36D=_tmp36A.f1;_tmp36C=_tmp36A.f2;_LL61:;{
union Cyc_CfFlowInfo_FlowInfo _tmp36B=_tmp36D;if((_tmp36B.BottomFL).tag == 1){_LL63: _LL64:
 return({struct _tuple18 _tmp60D;_tmp60D.f1=_tmp36D,_tmp60D.f2=_tmp36C;_tmp60D;});}else{_LL65: _LL66:
 return({struct _tuple18 _tmp60E;_tmp60E.f1=_tmp362,_tmp60E.f2=_tmp36C;_tmp60E;});}_LL62:;};};};}default: _LL55: _LL56:
# 1947
({void*_tmp371=0U;({struct _dyneither_ptr _tmp764=({const char*_tmp372="anal_Lexp: Subscript -- bad type";_tag_dyneither(_tmp372,sizeof(char),33U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp764,_tag_dyneither(_tmp371,sizeof(void*),0U));});});}_LL50:;}case 21U: _LL1A: _tmp383=((struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_tmp32C)->f1;_tmp382=((struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_tmp32C)->f2;_LL1B: {
# 1951
void*_tmp373=(void*)_check_null(_tmp383->topt);
if(Cyc_Absyn_is_require_union_type(_tmp373))
Cyc_NewControlFlow_check_union_requires(_tmp383->loc,_tmp373,_tmp382,inflow);
# 1955
if(Cyc_Absyn_is_nontagged_nonrequire_union_type(_tmp373))
return({struct _tuple18 _tmp60F;_tmp60F.f1=inflow,({union Cyc_CfFlowInfo_AbsLVal _tmp765=Cyc_CfFlowInfo_UnknownL();_tmp60F.f2=_tmp765;});_tmp60F;});
# 1958
return({struct Cyc_NewControlFlow_AnalEnv*_tmp76C=env;union Cyc_CfFlowInfo_FlowInfo _tmp76B=inflow;int _tmp76A=expand_unique;int _tmp769=passthrough_consumes;struct Cyc_Absyn_Exp*_tmp768=_tmp383;Cyc_NewControlFlow_anal_Lexp_rec(_tmp76C,_tmp76B,_tmp76A,_tmp769,_tmp768,({struct Cyc_List_List*_tmp375=_cycalloc(sizeof(*_tmp375));
({void*_tmp767=(void*)({struct Cyc_CfFlowInfo_Dot_CfFlowInfo_PathCon_struct*_tmp374=_cycalloc(sizeof(*_tmp374));_tmp374->tag=0U,({int _tmp766=Cyc_CfFlowInfo_get_field_index(_tmp373,_tmp382);_tmp374->f1=_tmp766;});_tmp374;});_tmp375->hd=_tmp767;}),_tmp375->tl=path;_tmp375;}));});}default: _LL1C: _LL1D:
# 1962
 return({struct _tuple18 _tmp610;({union Cyc_CfFlowInfo_FlowInfo _tmp76E=Cyc_CfFlowInfo_BottomFL();_tmp610.f1=_tmp76E;}),({union Cyc_CfFlowInfo_AbsLVal _tmp76D=Cyc_CfFlowInfo_UnknownL();_tmp610.f2=_tmp76D;});_tmp610;});}_LL5:;};}
# 1966
static struct _tuple18 Cyc_NewControlFlow_anal_Lexp(struct Cyc_NewControlFlow_AnalEnv*env,union Cyc_CfFlowInfo_FlowInfo inflow,int expand_unique,int passthrough_consumes,struct Cyc_Absyn_Exp*e){
# 1970
struct _tuple18 _tmp384=Cyc_NewControlFlow_anal_Lexp_rec(env,inflow,expand_unique,passthrough_consumes,e,0);struct _tuple18 _tmp385=_tmp384;union Cyc_CfFlowInfo_FlowInfo _tmp387;union Cyc_CfFlowInfo_AbsLVal _tmp386;_LL1: _tmp387=_tmp385.f1;_tmp386=_tmp385.f2;_LL2:;
return({struct _tuple18 _tmp611;_tmp611.f1=_tmp387,_tmp611.f2=_tmp386;_tmp611;});}
# 1977
static union Cyc_CfFlowInfo_FlowInfo Cyc_NewControlFlow_expand_unique_places(struct Cyc_NewControlFlow_AnalEnv*env,union Cyc_CfFlowInfo_FlowInfo inflow,struct Cyc_List_List*es){
# 1980
{struct Cyc_List_List*x=es;for(0;x != 0;x=x->tl){
if(Cyc_NewControlFlow_is_local_var_rooted_path((struct Cyc_Absyn_Exp*)x->hd,1) && 
Cyc_Tcutil_is_noalias_pointer_or_aggr((void*)_check_null(((struct Cyc_Absyn_Exp*)x->hd)->topt))){
struct _tuple18 _tmp388=Cyc_NewControlFlow_anal_Lexp(env,inflow,1,0,(struct Cyc_Absyn_Exp*)x->hd);struct _tuple18 _tmp389=_tmp388;union Cyc_CfFlowInfo_FlowInfo _tmp38A;_LL1: _tmp38A=_tmp389.f1;_LL2:;
inflow=_tmp38A;}}}
# 1988
return inflow;}
# 1994
static struct _tuple19 Cyc_NewControlFlow_anal_primop_test(struct Cyc_NewControlFlow_AnalEnv*env,union Cyc_CfFlowInfo_FlowInfo inflow,enum Cyc_Absyn_Primop p,struct Cyc_List_List*es){
# 1996
struct Cyc_CfFlowInfo_FlowEnv*_tmp38B=env->fenv;
# 1999
struct _tuple16 _tmp38C=Cyc_NewControlFlow_anal_Rexp(env,0,inflow,(struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)_check_null(es))->hd);struct _tuple16 _tmp38D=_tmp38C;union Cyc_CfFlowInfo_FlowInfo _tmp3CA;void*_tmp3C9;_LL1: _tmp3CA=_tmp38D.f1;_tmp3C9=_tmp38D.f2;_LL2:;{
struct _tuple16 _tmp38E=Cyc_NewControlFlow_anal_Rexp(env,0,_tmp3CA,(struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)_check_null(es->tl))->hd);struct _tuple16 _tmp38F=_tmp38E;union Cyc_CfFlowInfo_FlowInfo _tmp3C8;void*_tmp3C7;_LL4: _tmp3C8=_tmp38F.f1;_tmp3C7=_tmp38F.f2;_LL5:;{
union Cyc_CfFlowInfo_FlowInfo _tmp390=_tmp3C8;
# 2004
union Cyc_CfFlowInfo_FlowInfo _tmp391=_tmp390;struct Cyc_Dict_Dict _tmp3C6;struct Cyc_List_List*_tmp3C5;if((_tmp391.BottomFL).tag == 1){_LL7: _LL8:
 return({struct _tuple19 _tmp612;_tmp612.f1=_tmp390,_tmp612.f2=_tmp390;_tmp612;});}else{_LL9: _tmp3C6=((_tmp391.ReachableFL).val).f1;_tmp3C5=((_tmp391.ReachableFL).val).f2;_LLA: {
# 2007
struct Cyc_Absyn_Exp*_tmp392=(struct Cyc_Absyn_Exp*)es->hd;
struct Cyc_Absyn_Exp*_tmp393=(struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)_check_null(es->tl))->hd;
if(Cyc_CfFlowInfo_initlevel(env->fenv,_tmp3C6,_tmp3C9)== Cyc_CfFlowInfo_NoneIL  && !Cyc_CfFlowInfo_is_init_pointer(_tmp3C9))
({void*_tmp394=0U;({unsigned int _tmp770=((struct Cyc_Absyn_Exp*)es->hd)->loc;struct _dyneither_ptr _tmp76F=({const char*_tmp395="expression may not be initialized";_tag_dyneither(_tmp395,sizeof(char),34U);});Cyc_CfFlowInfo_aerr(_tmp770,_tmp76F,_tag_dyneither(_tmp394,sizeof(void*),0U));});});
if(Cyc_CfFlowInfo_initlevel(env->fenv,_tmp3C6,_tmp3C7)== Cyc_CfFlowInfo_NoneIL  && !Cyc_CfFlowInfo_is_init_pointer(_tmp3C9))
({void*_tmp396=0U;({unsigned int _tmp772=((struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)_check_null(es->tl))->hd)->loc;struct _dyneither_ptr _tmp771=({const char*_tmp397="expression may not be initialized";_tag_dyneither(_tmp397,sizeof(char),34U);});Cyc_CfFlowInfo_aerr(_tmp772,_tmp771,_tag_dyneither(_tmp396,sizeof(void*),0U));});});{
# 2014
union Cyc_CfFlowInfo_FlowInfo _tmp398=_tmp390;
union Cyc_CfFlowInfo_FlowInfo _tmp399=_tmp390;
# 2019
if(p == Cyc_Absyn_Eq  || p == Cyc_Absyn_Neq){
struct _tuple15 _tmp39A=Cyc_CfFlowInfo_unname_rval(_tmp3C9);struct _tuple15 _tmp39B=_tmp39A;void*_tmp3B3;struct Cyc_List_List*_tmp3B2;_LLC: _tmp3B3=_tmp39B.f1;_tmp3B2=_tmp39B.f2;_LLD:;{
struct _tuple15 _tmp39C=Cyc_CfFlowInfo_unname_rval(_tmp3C7);struct _tuple15 _tmp39D=_tmp39C;void*_tmp3B1;struct Cyc_List_List*_tmp3B0;_LLF: _tmp3B1=_tmp39D.f1;_tmp3B0=_tmp39D.f2;_LL10:;{
struct _tuple0 _tmp39E=({struct _tuple0 _tmp613;_tmp613.f1=_tmp3B3,_tmp613.f2=_tmp3B1;_tmp613;});struct _tuple0 _tmp39F=_tmp39E;enum Cyc_CfFlowInfo_InitLevel _tmp3AF;enum Cyc_CfFlowInfo_InitLevel _tmp3AE;switch(*((int*)_tmp39F.f1)){case 2U: if(((struct Cyc_CfFlowInfo_Zero_CfFlowInfo_AbsRVal_struct*)_tmp39F.f2)->tag == 0U){_LL12: _tmp3AE=((struct Cyc_CfFlowInfo_UnknownR_CfFlowInfo_AbsRVal_struct*)_tmp39F.f1)->f1;_LL13: {
# 2024
struct _tuple19 _tmp3A0=Cyc_NewControlFlow_splitzero(env,inflow,_tmp390,_tmp392,_tmp3AE,_tmp3B2);struct _tuple19 _tmp3A1=_tmp3A0;union Cyc_CfFlowInfo_FlowInfo _tmp3A6;union Cyc_CfFlowInfo_FlowInfo _tmp3A5;_LL23: _tmp3A6=_tmp3A1.f1;_tmp3A5=_tmp3A1.f2;_LL24:;
{enum Cyc_Absyn_Primop _tmp3A2=p;switch(_tmp3A2){case Cyc_Absyn_Eq: _LL26: _LL27:
 _tmp398=_tmp3A5;_tmp399=_tmp3A6;goto _LL25;case Cyc_Absyn_Neq: _LL28: _LL29:
 _tmp398=_tmp3A6;_tmp399=_tmp3A5;goto _LL25;default: _LL2A: _LL2B:
({void*_tmp3A3=0U;({struct _dyneither_ptr _tmp773=({const char*_tmp3A4="anal_test, zero-split";_tag_dyneither(_tmp3A4,sizeof(char),22U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp773,_tag_dyneither(_tmp3A3,sizeof(void*),0U));});});}_LL25:;}
# 2030
goto _LL11;}}else{goto _LL20;}case 0U: switch(*((int*)_tmp39F.f2)){case 2U: _LL14: _tmp3AF=((struct Cyc_CfFlowInfo_UnknownR_CfFlowInfo_AbsRVal_struct*)_tmp39F.f2)->f1;_LL15: {
# 2032
struct _tuple19 _tmp3A7=Cyc_NewControlFlow_splitzero(env,_tmp3C8,_tmp390,_tmp393,_tmp3AF,_tmp3B0);struct _tuple19 _tmp3A8=_tmp3A7;union Cyc_CfFlowInfo_FlowInfo _tmp3AD;union Cyc_CfFlowInfo_FlowInfo _tmp3AC;_LL2D: _tmp3AD=_tmp3A8.f1;_tmp3AC=_tmp3A8.f2;_LL2E:;
{enum Cyc_Absyn_Primop _tmp3A9=p;switch(_tmp3A9){case Cyc_Absyn_Eq: _LL30: _LL31:
 _tmp398=_tmp3AC;_tmp399=_tmp3AD;goto _LL2F;case Cyc_Absyn_Neq: _LL32: _LL33:
 _tmp398=_tmp3AD;_tmp399=_tmp3AC;goto _LL2F;default: _LL34: _LL35:
({void*_tmp3AA=0U;({struct _dyneither_ptr _tmp774=({const char*_tmp3AB="anal_test, zero-split";_tag_dyneither(_tmp3AB,sizeof(char),22U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp774,_tag_dyneither(_tmp3AA,sizeof(void*),0U));});});}_LL2F:;}
# 2038
goto _LL11;}case 0U: _LL16: _LL17:
# 2040
 if(p == Cyc_Absyn_Eq)({union Cyc_CfFlowInfo_FlowInfo _tmp775=Cyc_CfFlowInfo_BottomFL();_tmp399=_tmp775;});else{
({union Cyc_CfFlowInfo_FlowInfo _tmp776=Cyc_CfFlowInfo_BottomFL();_tmp398=_tmp776;});}
goto _LL11;case 1U: _LL18: _LL19:
 goto _LL1B;case 4U: _LL1A: _LL1B:
 goto _LL1D;default: goto _LL20;}case 1U: if(((struct Cyc_CfFlowInfo_Zero_CfFlowInfo_AbsRVal_struct*)_tmp39F.f2)->tag == 0U){_LL1C: _LL1D:
 goto _LL1F;}else{goto _LL20;}case 4U: if(((struct Cyc_CfFlowInfo_Zero_CfFlowInfo_AbsRVal_struct*)_tmp39F.f2)->tag == 0U){_LL1E: _LL1F:
# 2047
 if(p == Cyc_Absyn_Neq)({union Cyc_CfFlowInfo_FlowInfo _tmp777=Cyc_CfFlowInfo_BottomFL();_tmp399=_tmp777;});else{
({union Cyc_CfFlowInfo_FlowInfo _tmp778=Cyc_CfFlowInfo_BottomFL();_tmp398=_tmp778;});}
goto _LL11;}else{goto _LL20;}default: _LL20: _LL21:
 goto _LL11;}_LL11:;};};}
# 2058
{struct _tuple0 _tmp3B4=({struct _tuple0 _tmp615;({void*_tmp77A=Cyc_Tcutil_compress((void*)_check_null(_tmp392->topt));_tmp615.f1=_tmp77A;}),({
void*_tmp779=Cyc_Tcutil_compress((void*)_check_null(_tmp393->topt));_tmp615.f2=_tmp779;});_tmp615;});
# 2058
struct _tuple0 _tmp3B5=_tmp3B4;if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3B5.f1)->tag == 0U){if(((struct Cyc_Absyn_IntCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3B5.f1)->f1)->tag == 1U){if(((struct Cyc_Absyn_IntCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3B5.f1)->f1)->f1 == Cyc_Absyn_Unsigned){_LL37: _LL38:
# 2060
 goto _LL3A;}else{if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3B5.f2)->tag == 0U)switch(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3B5.f2)->f1)){case 1U: if(((struct Cyc_Absyn_IntCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3B5.f2)->f1)->f1 == Cyc_Absyn_Unsigned)goto _LL39;else{goto _LL3F;}case 4U: goto _LL3D;default: goto _LL3F;}else{goto _LL3F;}}}else{if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3B5.f2)->tag == 0U){if(((struct Cyc_Absyn_IntCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3B5.f2)->f1)->tag == 1U){if(((struct Cyc_Absyn_IntCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3B5.f2)->f1)->f1 == Cyc_Absyn_Unsigned)goto _LL39;else{if(((struct Cyc_Absyn_TagCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3B5.f1)->f1)->tag == 4U)goto _LL3B;else{goto _LL3F;}}}else{if(((struct Cyc_Absyn_TagCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3B5.f1)->f1)->tag == 4U)goto _LL3B;else{if(((struct Cyc_Absyn_TagCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3B5.f2)->f1)->tag == 4U)goto _LL3D;else{goto _LL3F;}}}}else{if(((struct Cyc_Absyn_TagCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3B5.f1)->f1)->tag == 4U){_LL3B: _LL3C:
# 2062
 goto _LL3E;}else{goto _LL3F;}}}}else{if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3B5.f2)->tag == 0U)switch(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3B5.f2)->f1)){case 1U: if(((struct Cyc_Absyn_IntCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3B5.f2)->f1)->f1 == Cyc_Absyn_Unsigned){_LL39: _LL3A:
# 2061
 goto _LL3C;}else{goto _LL3F;}case 4U: _LL3D: _LL3E:
# 2063
 goto _LL36;default: goto _LL3F;}else{_LL3F: _LL40:
 return({struct _tuple19 _tmp614;_tmp614.f1=_tmp398,_tmp614.f2=_tmp399;_tmp614;});}}_LL36:;}
# 2068
{enum Cyc_Absyn_Primop _tmp3B6=p;switch(_tmp3B6){case Cyc_Absyn_Eq: _LL42: _LL43:
 goto _LL45;case Cyc_Absyn_Neq: _LL44: _LL45: goto _LL47;case Cyc_Absyn_Gt: _LL46: _LL47: goto _LL49;case Cyc_Absyn_Gte: _LL48: _LL49: goto _LL4B;case Cyc_Absyn_Lt: _LL4A: _LL4B: goto _LL4D;case Cyc_Absyn_Lte: _LL4C: _LL4D: goto _LL41;default: _LL4E: _LL4F:
 return({struct _tuple19 _tmp616;_tmp616.f1=_tmp398,_tmp616.f2=_tmp399;_tmp616;});}_LL41:;}{
# 2073
struct _RegionHandle*_tmp3B7=Cyc_Core_heap_region;
struct _tuple13 _tmp3B8=Cyc_Relations_primop2relation(_tmp392,p,_tmp393);struct _tuple13 _tmp3B9=_tmp3B8;struct Cyc_Absyn_Exp*_tmp3C4;enum Cyc_Relations_Relation _tmp3C3;struct Cyc_Absyn_Exp*_tmp3C2;_LL51: _tmp3C4=_tmp3B9.f1;_tmp3C3=_tmp3B9.f2;_tmp3C2=_tmp3B9.f3;_LL52:;{
union Cyc_Relations_RelnOp n1=Cyc_Relations_RConst(0U);
union Cyc_Relations_RelnOp n2=Cyc_Relations_RConst(0U);
# 2078
if(Cyc_Relations_exp2relnop(_tmp3C4,& n1) && Cyc_Relations_exp2relnop(_tmp3C2,& n2)){
# 2080
struct Cyc_List_List*_tmp3BA=Cyc_Relations_add_relation(_tmp3B7,n1,_tmp3C3,n2,_tmp3C5);
# 2084
struct Cyc_List_List*_tmp3BB=({struct _RegionHandle*_tmp77E=_tmp3B7;union Cyc_Relations_RelnOp _tmp77D=n2;enum Cyc_Relations_Relation _tmp77C=Cyc_Relations_flip_relation(_tmp3C3);union Cyc_Relations_RelnOp _tmp77B=n1;Cyc_Relations_add_relation(_tmp77E,_tmp77D,_tmp77C,_tmp77B,_tmp3C5);});
struct _tuple19 _tmp3BC=({struct _tuple19 _tmp61B;_tmp61B.f1=_tmp398,_tmp61B.f2=_tmp399;_tmp61B;});struct _tuple19 _tmp3BD=_tmp3BC;struct Cyc_Dict_Dict _tmp3C1;struct Cyc_Dict_Dict _tmp3C0;struct Cyc_Dict_Dict _tmp3BF;struct Cyc_Dict_Dict _tmp3BE;if(((_tmp3BD.f1).ReachableFL).tag == 2){if(((_tmp3BD.f2).ReachableFL).tag == 2){_LL54: _tmp3BF=(((_tmp3BD.f1).ReachableFL).val).f1;_tmp3BE=(((_tmp3BD.f2).ReachableFL).val).f1;_LL55:
# 2087
 return({struct _tuple19 _tmp617;({union Cyc_CfFlowInfo_FlowInfo _tmp780=Cyc_CfFlowInfo_ReachableFL(_tmp3BF,_tmp3BA);_tmp617.f1=_tmp780;}),({union Cyc_CfFlowInfo_FlowInfo _tmp77F=Cyc_CfFlowInfo_ReachableFL(_tmp3BE,_tmp3BB);_tmp617.f2=_tmp77F;});_tmp617;});}else{_LL58: _tmp3C0=(((_tmp3BD.f1).ReachableFL).val).f1;_LL59:
# 2091
 return({struct _tuple19 _tmp618;({union Cyc_CfFlowInfo_FlowInfo _tmp781=Cyc_CfFlowInfo_ReachableFL(_tmp3C0,_tmp3BA);_tmp618.f1=_tmp781;}),_tmp618.f2=_tmp399;_tmp618;});}}else{if(((_tmp3BD.f2).ReachableFL).tag == 2){_LL56: _tmp3C1=(((_tmp3BD.f2).ReachableFL).val).f1;_LL57:
# 2089
 return({struct _tuple19 _tmp619;_tmp619.f1=_tmp398,({union Cyc_CfFlowInfo_FlowInfo _tmp782=Cyc_CfFlowInfo_ReachableFL(_tmp3C1,_tmp3BB);_tmp619.f2=_tmp782;});_tmp619;});}else{_LL5A: _LL5B:
# 2093
 return({struct _tuple19 _tmp61A;_tmp61A.f1=_tmp398,_tmp61A.f2=_tmp399;_tmp61A;});}}_LL53:;}else{
# 2096
return({struct _tuple19 _tmp61C;_tmp61C.f1=_tmp398,_tmp61C.f2=_tmp399;_tmp61C;});}};};};}}_LL6:;};};}
# 2102
static struct _tuple19 Cyc_NewControlFlow_anal_test(struct Cyc_NewControlFlow_AnalEnv*env,union Cyc_CfFlowInfo_FlowInfo inflow,struct Cyc_Absyn_Exp*e){
struct Cyc_CfFlowInfo_FlowEnv*_tmp3CB=env->fenv;
void*_tmp3CC=e->r;void*_tmp3CD=_tmp3CC;enum Cyc_Absyn_Primop _tmp411;struct Cyc_List_List*_tmp410;struct Cyc_Absyn_Exp*_tmp40F;struct Cyc_Absyn_Exp*_tmp40E;struct Cyc_Absyn_Exp*_tmp40D;struct Cyc_Absyn_Exp*_tmp40C;struct Cyc_Absyn_Exp*_tmp40B;struct Cyc_Absyn_Exp*_tmp40A;struct Cyc_Absyn_Exp*_tmp409;struct Cyc_Absyn_Exp*_tmp408;struct Cyc_Absyn_Exp*_tmp407;struct Cyc_Absyn_Exp*_tmp406;switch(*((int*)_tmp3CD)){case 6U: _LL1: _tmp408=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmp3CD)->f1;_tmp407=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmp3CD)->f2;_tmp406=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmp3CD)->f3;_LL2: {
# 2106
struct _tuple19 _tmp3CE=Cyc_NewControlFlow_anal_test(env,inflow,_tmp408);struct _tuple19 _tmp3CF=_tmp3CE;union Cyc_CfFlowInfo_FlowInfo _tmp3D9;union Cyc_CfFlowInfo_FlowInfo _tmp3D8;_LL10: _tmp3D9=_tmp3CF.f1;_tmp3D8=_tmp3CF.f2;_LL11:;{
struct _tuple19 _tmp3D0=Cyc_NewControlFlow_anal_test(env,_tmp3D9,_tmp407);struct _tuple19 _tmp3D1=_tmp3D0;union Cyc_CfFlowInfo_FlowInfo _tmp3D7;union Cyc_CfFlowInfo_FlowInfo _tmp3D6;_LL13: _tmp3D7=_tmp3D1.f1;_tmp3D6=_tmp3D1.f2;_LL14:;{
struct _tuple19 _tmp3D2=Cyc_NewControlFlow_anal_test(env,_tmp3D8,_tmp406);struct _tuple19 _tmp3D3=_tmp3D2;union Cyc_CfFlowInfo_FlowInfo _tmp3D5;union Cyc_CfFlowInfo_FlowInfo _tmp3D4;_LL16: _tmp3D5=_tmp3D3.f1;_tmp3D4=_tmp3D3.f2;_LL17:;
return({struct _tuple19 _tmp61D;({union Cyc_CfFlowInfo_FlowInfo _tmp784=Cyc_CfFlowInfo_join_flow(_tmp3CB,_tmp3D7,_tmp3D5);_tmp61D.f1=_tmp784;}),({
union Cyc_CfFlowInfo_FlowInfo _tmp783=Cyc_CfFlowInfo_join_flow(_tmp3CB,_tmp3D6,_tmp3D4);_tmp61D.f2=_tmp783;});_tmp61D;});};};}case 7U: _LL3: _tmp40A=((struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct*)_tmp3CD)->f1;_tmp409=((struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct*)_tmp3CD)->f2;_LL4: {
# 2112
struct _tuple19 _tmp3DA=Cyc_NewControlFlow_anal_test(env,inflow,_tmp40A);struct _tuple19 _tmp3DB=_tmp3DA;union Cyc_CfFlowInfo_FlowInfo _tmp3E1;union Cyc_CfFlowInfo_FlowInfo _tmp3E0;_LL19: _tmp3E1=_tmp3DB.f1;_tmp3E0=_tmp3DB.f2;_LL1A:;{
struct _tuple19 _tmp3DC=Cyc_NewControlFlow_anal_test(env,_tmp3E1,_tmp409);struct _tuple19 _tmp3DD=_tmp3DC;union Cyc_CfFlowInfo_FlowInfo _tmp3DF;union Cyc_CfFlowInfo_FlowInfo _tmp3DE;_LL1C: _tmp3DF=_tmp3DD.f1;_tmp3DE=_tmp3DD.f2;_LL1D:;
return({struct _tuple19 _tmp61E;_tmp61E.f1=_tmp3DF,({union Cyc_CfFlowInfo_FlowInfo _tmp785=Cyc_CfFlowInfo_join_flow(_tmp3CB,_tmp3E0,_tmp3DE);_tmp61E.f2=_tmp785;});_tmp61E;});};}case 8U: _LL5: _tmp40C=((struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct*)_tmp3CD)->f1;_tmp40B=((struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct*)_tmp3CD)->f2;_LL6: {
# 2116
struct _tuple19 _tmp3E2=Cyc_NewControlFlow_anal_test(env,inflow,_tmp40C);struct _tuple19 _tmp3E3=_tmp3E2;union Cyc_CfFlowInfo_FlowInfo _tmp3E9;union Cyc_CfFlowInfo_FlowInfo _tmp3E8;_LL1F: _tmp3E9=_tmp3E3.f1;_tmp3E8=_tmp3E3.f2;_LL20:;{
struct _tuple19 _tmp3E4=Cyc_NewControlFlow_anal_test(env,_tmp3E8,_tmp40B);struct _tuple19 _tmp3E5=_tmp3E4;union Cyc_CfFlowInfo_FlowInfo _tmp3E7;union Cyc_CfFlowInfo_FlowInfo _tmp3E6;_LL22: _tmp3E7=_tmp3E5.f1;_tmp3E6=_tmp3E5.f2;_LL23:;
return({struct _tuple19 _tmp61F;({union Cyc_CfFlowInfo_FlowInfo _tmp786=Cyc_CfFlowInfo_join_flow(_tmp3CB,_tmp3E9,_tmp3E7);_tmp61F.f1=_tmp786;}),_tmp61F.f2=_tmp3E6;_tmp61F;});};}case 9U: _LL7: _tmp40E=((struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct*)_tmp3CD)->f1;_tmp40D=((struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct*)_tmp3CD)->f2;_LL8: {
# 2120
struct _tuple16 _tmp3EA=Cyc_NewControlFlow_anal_Rexp(env,0,inflow,_tmp40E);struct _tuple16 _tmp3EB=_tmp3EA;union Cyc_CfFlowInfo_FlowInfo _tmp3ED;void*_tmp3EC;_LL25: _tmp3ED=_tmp3EB.f1;_tmp3EC=_tmp3EB.f2;_LL26:;
return Cyc_NewControlFlow_anal_test(env,_tmp3ED,_tmp40D);}case 3U: if(((struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_tmp3CD)->f1 == Cyc_Absyn_Not){if(((struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_tmp3CD)->f2 != 0){if(((struct Cyc_List_List*)((struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_tmp3CD)->f2)->tl == 0){_LL9: _tmp40F=(struct Cyc_Absyn_Exp*)(((struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_tmp3CD)->f2)->hd;_LLA: {
# 2123
struct _tuple19 _tmp3EE=Cyc_NewControlFlow_anal_test(env,inflow,_tmp40F);struct _tuple19 _tmp3EF=_tmp3EE;union Cyc_CfFlowInfo_FlowInfo _tmp3F1;union Cyc_CfFlowInfo_FlowInfo _tmp3F0;_LL28: _tmp3F1=_tmp3EF.f1;_tmp3F0=_tmp3EF.f2;_LL29:;
return({struct _tuple19 _tmp620;_tmp620.f1=_tmp3F0,_tmp620.f2=_tmp3F1;_tmp620;});}}else{goto _LLB;}}else{goto _LLB;}}else{_LLB: _tmp411=((struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_tmp3CD)->f1;_tmp410=((struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_tmp3CD)->f2;_LLC:
# 2126
 return Cyc_NewControlFlow_anal_primop_test(env,inflow,_tmp411,_tmp410);}default: _LLD: _LLE: {
# 2130
struct _tuple16 _tmp3F2=Cyc_NewControlFlow_anal_Rexp(env,0,inflow,e);struct _tuple16 _tmp3F3=_tmp3F2;union Cyc_CfFlowInfo_FlowInfo _tmp405;void*_tmp404;_LL2B: _tmp405=_tmp3F3.f1;_tmp404=_tmp3F3.f2;_LL2C:;{
union Cyc_CfFlowInfo_FlowInfo _tmp3F4=_tmp405;struct Cyc_Dict_Dict _tmp403;if((_tmp3F4.BottomFL).tag == 1){_LL2E: _LL2F:
 return({struct _tuple19 _tmp621;_tmp621.f1=_tmp405,_tmp621.f2=_tmp405;_tmp621;});}else{_LL30: _tmp403=((_tmp3F4.ReachableFL).val).f1;_LL31: {
# 2134
struct _tuple15 _tmp3F5=Cyc_CfFlowInfo_unname_rval(_tmp404);struct _tuple15 _tmp3F6=_tmp3F5;void*_tmp402;struct Cyc_List_List*_tmp401;_LL33: _tmp402=_tmp3F6.f1;_tmp401=_tmp3F6.f2;_LL34:;{
void*_tmp3F7=_tmp402;enum Cyc_CfFlowInfo_InitLevel _tmp400;struct Cyc_Absyn_Vardecl*_tmp3FF;void*_tmp3FE;switch(*((int*)_tmp3F7)){case 8U: _LL36: _tmp3FF=((struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*)_tmp3F7)->f1;_tmp3FE=(void*)((struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*)_tmp3F7)->f2;_LL37:
# 2137
({void*_tmp3F8=0U;({struct _dyneither_ptr _tmp787=({const char*_tmp3F9="anal_test: bad namedlocation";_tag_dyneither(_tmp3F9,sizeof(char),29U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp787,_tag_dyneither(_tmp3F8,sizeof(void*),0U));});});case 0U: _LL38: _LL39:
 return({struct _tuple19 _tmp622;({union Cyc_CfFlowInfo_FlowInfo _tmp788=Cyc_CfFlowInfo_BottomFL();_tmp622.f1=_tmp788;}),_tmp622.f2=_tmp405;_tmp622;});case 1U: _LL3A: _LL3B:
 goto _LL3D;case 5U: _LL3C: _LL3D:
 goto _LL3F;case 4U: _LL3E: _LL3F:
 return({struct _tuple19 _tmp623;_tmp623.f1=_tmp405,({union Cyc_CfFlowInfo_FlowInfo _tmp789=Cyc_CfFlowInfo_BottomFL();_tmp623.f2=_tmp789;});_tmp623;});case 2U: if(((struct Cyc_CfFlowInfo_UnknownR_CfFlowInfo_AbsRVal_struct*)_tmp3F7)->f1 == Cyc_CfFlowInfo_NoneIL){_LL40: _LL41:
 goto _LL43;}else{_LL46: _tmp400=((struct Cyc_CfFlowInfo_UnknownR_CfFlowInfo_AbsRVal_struct*)_tmp3F7)->f1;_LL47:
# 2147
 return Cyc_NewControlFlow_splitzero(env,inflow,_tmp405,e,_tmp400,_tmp401);}case 3U: if(((struct Cyc_CfFlowInfo_Esc_CfFlowInfo_AbsRVal_struct*)_tmp3F7)->f1 == Cyc_CfFlowInfo_NoneIL){_LL42: _LL43:
# 2143
 goto _LL45;}else{_LL48: _LL49:
# 2148
 return({struct _tuple19 _tmp624;_tmp624.f1=_tmp405,_tmp624.f2=_tmp405;_tmp624;});}case 7U: _LL44: _LL45:
# 2145
({void*_tmp3FA=0U;({unsigned int _tmp78B=e->loc;struct _dyneither_ptr _tmp78A=({const char*_tmp3FB="expression may not be initialized";_tag_dyneither(_tmp3FB,sizeof(char),34U);});Cyc_CfFlowInfo_aerr(_tmp78B,_tmp78A,_tag_dyneither(_tmp3FA,sizeof(void*),0U));});});
return({struct _tuple19 _tmp625;({union Cyc_CfFlowInfo_FlowInfo _tmp78D=Cyc_CfFlowInfo_BottomFL();_tmp625.f1=_tmp78D;}),({union Cyc_CfFlowInfo_FlowInfo _tmp78C=Cyc_CfFlowInfo_BottomFL();_tmp625.f2=_tmp78C;});_tmp625;});default: _LL4A: _LL4B:
# 2149
({void*_tmp3FC=0U;({struct _dyneither_ptr _tmp78E=({const char*_tmp3FD="anal_test";_tag_dyneither(_tmp3FD,sizeof(char),10U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp78E,_tag_dyneither(_tmp3FC,sizeof(void*),0U));});});}_LL35:;};}}_LL2D:;};}}_LL0:;}struct _tuple27{unsigned int f1;struct Cyc_NewControlFlow_AnalEnv*f2;struct Cyc_Dict_Dict f3;};
# 2155
static void Cyc_NewControlFlow_check_for_unused_unique(struct _tuple27*ckenv,void*root,void*rval){
# 2157
struct _tuple27*_tmp412=ckenv;unsigned int _tmp41E;struct Cyc_NewControlFlow_AnalEnv*_tmp41D;struct Cyc_Dict_Dict _tmp41C;_LL1: _tmp41E=_tmp412->f1;_tmp41D=_tmp412->f2;_tmp41C=_tmp412->f3;_LL2:;{
void*_tmp413=root;struct Cyc_Absyn_Vardecl*_tmp41B;if(((struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct*)_tmp413)->tag == 0U){_LL4: _tmp41B=((struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct*)_tmp413)->f1;_LL5:
# 2160
 if(Cyc_Tcutil_is_noalias_pointer_or_aggr(_tmp41B->type)){
struct _tuple15 _tmp414=Cyc_CfFlowInfo_unname_rval(rval);struct _tuple15 _tmp415=_tmp414;void*_tmp41A;_LL9: _tmp41A=_tmp415.f1;_LLA:;{
void*_tmp416=_tmp41A;switch(*((int*)_tmp416)){case 7U: _LLC: _LLD:
 goto _LLF;case 0U: _LLE: _LLF:
 goto _LL11;case 2U: if(((struct Cyc_CfFlowInfo_UnknownR_CfFlowInfo_AbsRVal_struct*)_tmp416)->f1 == Cyc_CfFlowInfo_NoneIL){_LL10: _LL11:
 goto _LLB;}else{goto _LL12;}default: _LL12: _LL13:
# 2167
({struct Cyc_String_pa_PrintArg_struct _tmp419=({struct Cyc_String_pa_PrintArg_struct _tmp626;_tmp626.tag=0U,({struct _dyneither_ptr _tmp78F=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(_tmp41B->name));_tmp626.f1=_tmp78F;});_tmp626;});void*_tmp417[1U];_tmp417[0]=& _tmp419;({unsigned int _tmp791=_tmp41E;struct _dyneither_ptr _tmp790=({const char*_tmp418="unique pointers reachable from %s may become unreachable";_tag_dyneither(_tmp418,sizeof(char),57U);});Cyc_Warn_warn(_tmp791,_tmp790,_tag_dyneither(_tmp417,sizeof(void*),1U));});});
goto _LLB;}_LLB:;};}
# 2171
goto _LL3;}else{_LL6: _LL7:
 goto _LL3;}_LL3:;};}
# 2176
static void Cyc_NewControlFlow_check_init_params(unsigned int loc,struct Cyc_NewControlFlow_AnalEnv*env,union Cyc_CfFlowInfo_FlowInfo flow){
union Cyc_CfFlowInfo_FlowInfo _tmp41F=flow;struct Cyc_Dict_Dict _tmp424;if((_tmp41F.BottomFL).tag == 1){_LL1: _LL2:
 return;}else{_LL3: _tmp424=((_tmp41F.ReachableFL).val).f1;_LL4:
# 2180
{struct Cyc_List_List*_tmp420=env->param_roots;for(0;_tmp420 != 0;_tmp420=_tmp420->tl){
if(({struct Cyc_CfFlowInfo_FlowEnv*_tmp793=env->fenv;struct Cyc_Dict_Dict _tmp792=_tmp424;Cyc_CfFlowInfo_initlevel(_tmp793,_tmp792,Cyc_CfFlowInfo_lookup_place(_tmp424,(struct Cyc_CfFlowInfo_Place*)_tmp420->hd));})!= Cyc_CfFlowInfo_AllIL)
({void*_tmp421=0U;({unsigned int _tmp795=loc;struct _dyneither_ptr _tmp794=({const char*_tmp422="function may not initialize all the parameters with attribute 'initializes'";_tag_dyneither(_tmp422,sizeof(char),76U);});Cyc_CfFlowInfo_aerr(_tmp795,_tmp794,_tag_dyneither(_tmp421,sizeof(void*),0U));});});}}
# 2184
if(Cyc_NewControlFlow_warn_lose_unique){
struct _tuple27 _tmp423=({struct _tuple27 _tmp627;_tmp627.f1=loc,_tmp627.f2=env,_tmp627.f3=_tmp424;_tmp627;});
((void(*)(void(*f)(struct _tuple27*,void*,void*),struct _tuple27*env,struct Cyc_Dict_Dict d))Cyc_Dict_iter_c)(Cyc_NewControlFlow_check_for_unused_unique,& _tmp423,_tmp424);}
# 2188
return;}_LL0:;}
# 2197
static struct _tuple1 Cyc_NewControlFlow_get_unconsume_pat_vars(struct Cyc_List_List*vds){
# 2199
struct Cyc_List_List*_tmp425=0;
struct Cyc_List_List*_tmp426=0;
{struct Cyc_List_List*x=vds;for(0;x != 0;x=x->tl){
struct _tuple24*_tmp427=(struct _tuple24*)x->hd;struct _tuple24*_tmp428=_tmp427;struct Cyc_Absyn_Vardecl**_tmp430;struct Cyc_Absyn_Exp*_tmp42F;_LL1: _tmp430=_tmp428->f1;_tmp42F=_tmp428->f2;_LL2:;
if((_tmp430 != 0  && _tmp42F != 0) && 
Cyc_Tcutil_is_noalias_pointer((void*)_check_null(_tmp42F->topt),0)){
struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct*_tmp429=({struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct*_tmp42E=_cycalloc(sizeof(*_tmp42E));_tmp42E->tag=0U,_tmp42E->f1=*_tmp430;_tmp42E;});
struct Cyc_CfFlowInfo_Place*_tmp42A=({struct Cyc_CfFlowInfo_Place*_tmp42D=_cycalloc(sizeof(*_tmp42D));_tmp42D->root=(void*)_tmp429,_tmp42D->path=0;_tmp42D;});
({struct Cyc_List_List*_tmp796=({struct Cyc_List_List*_tmp42B=_cycalloc(sizeof(*_tmp42B));_tmp42B->hd=_tmp42A,_tmp42B->tl=_tmp425;_tmp42B;});_tmp425=_tmp796;});
({struct Cyc_List_List*_tmp797=({struct Cyc_List_List*_tmp42C=_cycalloc(sizeof(*_tmp42C));_tmp42C->hd=_tmp42F,_tmp42C->tl=_tmp426;_tmp42C;});_tmp426=_tmp797;});}}}
# 2211
return({struct _tuple1 _tmp628;_tmp628.f1=_tmp425,_tmp628.f2=_tmp426;_tmp628;});}struct _tuple28{int f1;void*f2;};
# 2217
static struct _tuple28 Cyc_NewControlFlow_noconsume_place_ok(struct Cyc_NewControlFlow_AnalEnv*env,struct Cyc_CfFlowInfo_Place*place,int do_unconsume,struct Cyc_Absyn_Vardecl*vd,union Cyc_CfFlowInfo_FlowInfo flow,unsigned int loc){
# 2224
union Cyc_CfFlowInfo_FlowInfo _tmp431=flow;struct Cyc_Dict_Dict _tmp445;if((_tmp431.BottomFL).tag == 1){_LL1: _LL2:
({void*_tmp432=0U;({struct _dyneither_ptr _tmp798=({const char*_tmp433="noconsume_place_ok: flow became Bottom!";_tag_dyneither(_tmp433,sizeof(char),40U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp798,_tag_dyneither(_tmp432,sizeof(void*),0U));});});}else{_LL3: _tmp445=((_tmp431.ReachableFL).val).f1;_LL4: {
# 2232
struct Cyc_Absyn_Exp*_tmp434=Cyc_Absyn_uint_exp(1U,0U);
int _tmp435=0;
int _tmp436=1;
void*_tmp437=Cyc_CfFlowInfo_lookup_place(_tmp445,place);
void*_tmp438=_tmp437;
# 2244
int varok=0;
{void*_tmp439=_tmp437;struct Cyc_Absyn_Vardecl*_tmp444;void*_tmp443;if(((struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*)_tmp439)->tag == 8U){_LL6: _tmp444=((struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*)_tmp439)->f1;_tmp443=(void*)((struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*)_tmp439)->f2;_LL7:
# 2247
 if(vd == _tmp444){
_tmp438=_tmp443;
# 2250
if(Cyc_Tcutil_is_noalias_pointer_or_aggr(vd->type)){
# 2252
if(Cyc_CfFlowInfo_is_unique_consumed(_tmp434,_tmp436,_tmp438,& _tmp435)){
if(!do_unconsume)
({struct Cyc_String_pa_PrintArg_struct _tmp43C=({struct Cyc_String_pa_PrintArg_struct _tmp629;_tmp629.tag=0U,({
# 2256
struct _dyneither_ptr _tmp799=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(vd->name));_tmp629.f1=_tmp799;});_tmp629;});void*_tmp43A[1U];_tmp43A[0]=& _tmp43C;({unsigned int _tmp79B=loc;struct _dyneither_ptr _tmp79A=({const char*_tmp43B="function consumes parameter %s with attribute 'noconsume'";_tag_dyneither(_tmp43B,sizeof(char),58U);});Cyc_CfFlowInfo_aerr(_tmp79B,_tmp79A,_tag_dyneither(_tmp43A,sizeof(void*),1U));});});}else{
# 2259
if(Cyc_CfFlowInfo_initlevel(env->fenv,_tmp445,_tmp438)!= Cyc_CfFlowInfo_AllIL  && !do_unconsume)
({struct Cyc_String_pa_PrintArg_struct _tmp43F=({struct Cyc_String_pa_PrintArg_struct _tmp62A;_tmp62A.tag=0U,({
# 2262
struct _dyneither_ptr _tmp79C=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(vd->name));_tmp62A.f1=_tmp79C;});_tmp62A;});void*_tmp43D[1U];_tmp43D[0]=& _tmp43F;({unsigned int _tmp79E=loc;struct _dyneither_ptr _tmp79D=({const char*_tmp43E="function consumes value pointed to by parameter %s, which has attribute 'noconsume'";_tag_dyneither(_tmp43E,sizeof(char),84U);});Cyc_CfFlowInfo_aerr(_tmp79E,_tmp79D,_tag_dyneither(_tmp43D,sizeof(void*),1U));});});else{
# 2264
varok=1;}}}else{
# 2267
varok=1;}}else{
# 2270
goto _LL9;}
goto _LL5;}else{_LL8: _LL9:
# 2274
 if(!Cyc_Tcutil_is_noalias_pointer_or_aggr(vd->type))
varok=1;else{
if(!do_unconsume)
({struct Cyc_String_pa_PrintArg_struct _tmp442=({struct Cyc_String_pa_PrintArg_struct _tmp62B;_tmp62B.tag=0U,({
# 2279
struct _dyneither_ptr _tmp79F=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(vd->name));_tmp62B.f1=_tmp79F;});_tmp62B;});void*_tmp440[1U];_tmp440[0]=& _tmp442;({unsigned int _tmp7A1=loc;struct _dyneither_ptr _tmp7A0=({const char*_tmp441="function parameter %s with attribute 'noconsume' no longer set to its original value";_tag_dyneither(_tmp441,sizeof(char),85U);});Cyc_CfFlowInfo_aerr(_tmp7A1,_tmp7A0,_tag_dyneither(_tmp440,sizeof(void*),1U));});});}
goto _LL5;}_LL5:;}
# 2286
return({struct _tuple28 _tmp62C;_tmp62C.f1=varok,_tmp62C.f2=_tmp438;_tmp62C;});}}_LL0:;}
# 2292
static struct Cyc_Absyn_Vardecl*Cyc_NewControlFlow_get_vd_from_place(struct Cyc_CfFlowInfo_Place*p){
struct Cyc_CfFlowInfo_Place*_tmp446=p;void*_tmp44E;struct Cyc_List_List*_tmp44D;_LL1: _tmp44E=_tmp446->root;_tmp44D=_tmp446->path;_LL2:;
if(_tmp44D != 0)
({void*_tmp447=0U;({struct _dyneither_ptr _tmp7A2=({const char*_tmp448="unconsume_params: param to unconsume is non-variable";_tag_dyneither(_tmp448,sizeof(char),53U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp7A2,_tag_dyneither(_tmp447,sizeof(void*),0U));});});{
struct Cyc_Absyn_Vardecl*vd;
void*_tmp449=_tmp44E;struct Cyc_Absyn_Vardecl*_tmp44C;if(((struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct*)_tmp449)->tag == 0U){_LL4: _tmp44C=((struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct*)_tmp449)->f1;_LL5:
 return _tmp44C;}else{_LL6: _LL7:
({void*_tmp44A=0U;({struct _dyneither_ptr _tmp7A3=({const char*_tmp44B="unconsume_params: root is not a varroot";_tag_dyneither(_tmp44B,sizeof(char),40U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp7A3,_tag_dyneither(_tmp44A,sizeof(void*),0U));});});}_LL3:;};}
# 2311 "new_control_flow.cyc"
static union Cyc_CfFlowInfo_FlowInfo Cyc_NewControlFlow_unconsume_exp(struct Cyc_NewControlFlow_AnalEnv*env,struct Cyc_Absyn_Exp*unconsume_exp,struct Cyc_Absyn_Vardecl*vd,int varok,void*ropt,union Cyc_CfFlowInfo_FlowInfo flow,unsigned int loc){
# 2319
{union Cyc_CfFlowInfo_FlowInfo _tmp44F=flow;struct Cyc_Dict_Dict _tmp466;if((_tmp44F.BottomFL).tag == 1){_LL1: _LL2:
 return flow;}else{_LL3: _tmp466=((_tmp44F.ReachableFL).val).f1;_LL4: {
# 2326
struct _tuple18 _tmp450=Cyc_NewControlFlow_anal_Lexp(env,flow,0,1,unconsume_exp);struct _tuple18 _tmp451=_tmp450;union Cyc_CfFlowInfo_FlowInfo _tmp465;union Cyc_CfFlowInfo_AbsLVal _tmp464;_LL6: _tmp465=_tmp451.f1;_tmp464=_tmp451.f2;_LL7:;
# 2329
{union Cyc_CfFlowInfo_AbsLVal _tmp452=_tmp464;struct Cyc_CfFlowInfo_Place*_tmp463;if((_tmp452.PlaceL).tag == 1){_LL9: _tmp463=(_tmp452.PlaceL).val;_LLA: {
# 2333
void*_tmp453=Cyc_CfFlowInfo_lookup_place(_tmp466,_tmp463);
{void*_tmp454=_tmp453;struct Cyc_Absyn_Vardecl*_tmp462;void*_tmp461;if(((struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*)_tmp454)->tag == 8U){_LLE: _tmp462=((struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*)_tmp454)->f1;_tmp461=(void*)((struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*)_tmp454)->f2;_LLF: {
# 2339
void*new_rval;
if(_tmp462 == vd){
# 2343
if(varok){
# 2345
({void*_tmp7A4=Cyc_CfFlowInfo_make_unique_unconsumed(env->fenv,_tmp461);_tmp453=_tmp7A4;});
# 2350
if(ropt != 0){
# 2356
struct _tuple16 _tmp455=({
struct Cyc_CfFlowInfo_FlowEnv*_tmp7A6=env->fenv;struct _tuple16 _tmp7A5=({struct _tuple16 _tmp62D;_tmp62D.f1=_tmp465,_tmp62D.f2=_tmp453;_tmp62D;});Cyc_CfFlowInfo_join_flow_and_rval(_tmp7A6,_tmp7A5,({struct _tuple16 _tmp62E;_tmp62E.f1=_tmp465,_tmp62E.f2=ropt;_tmp62E;}));});
# 2356
struct _tuple16 _tmp456=_tmp455;union Cyc_CfFlowInfo_FlowInfo _tmp458;void*_tmp457;_LL13: _tmp458=_tmp456.f1;_tmp457=_tmp456.f2;_LL14:;
# 2360
_tmp465=_tmp458;new_rval=_tmp457;}else{
# 2365
new_rval=_tmp453;}}else{
# 2368
new_rval=_tmp461;}
# 2370
{union Cyc_CfFlowInfo_FlowInfo _tmp459=_tmp465;struct Cyc_Dict_Dict _tmp45D;struct Cyc_List_List*_tmp45C;if((_tmp459.ReachableFL).tag == 2){_LL16: _tmp45D=((_tmp459.ReachableFL).val).f1;_tmp45C=((_tmp459.ReachableFL).val).f2;_LL17:
# 2372
({union Cyc_CfFlowInfo_FlowInfo _tmp7A8=({struct Cyc_Dict_Dict _tmp7A7=Cyc_CfFlowInfo_assign_place(env->fenv,loc,_tmp45D,_tmp463,new_rval);Cyc_CfFlowInfo_ReachableFL(_tmp7A7,_tmp45C);});flow=_tmp7A8;});
# 2376
goto _LL15;}else{_LL18: _LL19:
# 2378
({void*_tmp45A=0U;({struct _dyneither_ptr _tmp7A9=({const char*_tmp45B="unconsume_params: joined flow became bot!";_tag_dyneither(_tmp45B,sizeof(char),42U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp7A9,_tag_dyneither(_tmp45A,sizeof(void*),0U));});});}_LL15:;}
# 2380
goto _LLD;}else{
# 2382
goto _LL11;}
goto _LLD;}}else{_LL10: _LL11:
# 2389
 if(ropt != 0  && !
Cyc_Tcutil_is_noalias_pointer_or_aggr(vd->type))
({struct Cyc_String_pa_PrintArg_struct _tmp460=({struct Cyc_String_pa_PrintArg_struct _tmp62F;_tmp62F.tag=0U,({
struct _dyneither_ptr _tmp7AA=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_exp2string(unconsume_exp));_tmp62F.f1=_tmp7AA;});_tmp62F;});void*_tmp45E[1U];_tmp45E[0]=& _tmp460;({unsigned int _tmp7AC=loc;struct _dyneither_ptr _tmp7AB=({const char*_tmp45F="aliased expression %s was overwritten";_tag_dyneither(_tmp45F,sizeof(char),38U);});Cyc_CfFlowInfo_aerr(_tmp7AC,_tmp7AB,_tag_dyneither(_tmp45E,sizeof(void*),1U));});});
# 2406 "new_control_flow.cyc"
goto _LLD;}_LLD:;}
# 2408
goto _LL8;}}else{_LLB: _LLC:
# 2414
 goto _LL8;}_LL8:;}
# 2416
goto _LL0;}}_LL0:;}
# 2418
return flow;}
# 2431 "new_control_flow.cyc"
static union Cyc_CfFlowInfo_FlowInfo Cyc_NewControlFlow_unconsume_params(struct Cyc_NewControlFlow_AnalEnv*env,struct Cyc_List_List*consumed_vals,struct Cyc_List_List*unconsume_exps,int is_region_open,union Cyc_CfFlowInfo_FlowInfo flow,unsigned int loc){
# 2437
{union Cyc_CfFlowInfo_FlowInfo _tmp467=flow;if((_tmp467.BottomFL).tag == 1){_LL1: _LL2:
 return flow;}else{_LL3: _LL4:
 goto _LL0;}_LL0:;}{
# 2441
int _tmp468=unconsume_exps != 0;
struct Cyc_List_List*_tmp469=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_rev)(consumed_vals);
struct Cyc_List_List*_tmp46A=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_rev)(unconsume_exps);
{struct Cyc_List_List*_tmp46B=_tmp469;for(0;_tmp46B != 0;(
_tmp46B=_tmp46B->tl,
_tmp46A != 0?_tmp46A=_tmp46A->tl: 0)){
# 2450
struct Cyc_Absyn_Vardecl*_tmp46C=Cyc_NewControlFlow_get_vd_from_place((struct Cyc_CfFlowInfo_Place*)_tmp46B->hd);
struct _tuple28 _tmp46D=
is_region_open?({struct _tuple28 _tmp630;_tmp630.f1=1,_tmp630.f2=0;_tmp630;}):
 Cyc_NewControlFlow_noconsume_place_ok(env,(struct Cyc_CfFlowInfo_Place*)_tmp46B->hd,_tmp468,_tmp46C,flow,loc);
# 2451
struct _tuple28 _tmp46E=_tmp46D;int _tmp470;void*_tmp46F;_LL6: _tmp470=_tmp46E.f1;_tmp46F=_tmp46E.f2;_LL7:;
# 2457
if(_tmp468)
({union Cyc_CfFlowInfo_FlowInfo _tmp7AD=Cyc_NewControlFlow_unconsume_exp(env,(struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)_check_null(_tmp46A))->hd,_tmp46C,_tmp470,_tmp46F,flow,loc);flow=_tmp7AD;});}}
# 2460
Cyc_NewControlFlow_update_tryflow(env,flow);
return flow;};}struct _tuple29{int f1;struct Cyc_Absyn_Stmt*f2;struct Cyc_List_List*f3;struct Cyc_List_List*f4;};
# 2464
static union Cyc_CfFlowInfo_FlowInfo Cyc_NewControlFlow_anal_scs(struct Cyc_NewControlFlow_AnalEnv*env,union Cyc_CfFlowInfo_FlowInfo inflow,struct Cyc_List_List*scs,unsigned int exp_loc){
# 2466
struct Cyc_CfFlowInfo_FlowEnv*_tmp471=env->fenv;
for(0;scs != 0;scs=scs->tl){
struct Cyc_Absyn_Switch_clause*_tmp472=(struct Cyc_Absyn_Switch_clause*)scs->hd;struct Cyc_Absyn_Switch_clause*_tmp473=_tmp472;struct Cyc_Core_Opt*_tmp487;struct Cyc_Absyn_Exp*_tmp486;struct Cyc_Absyn_Stmt*_tmp485;unsigned int _tmp484;_LL1: _tmp487=_tmp473->pat_vars;_tmp486=_tmp473->where_clause;_tmp485=_tmp473->body;_tmp484=_tmp473->loc;_LL2:;{
struct _tuple1 _tmp474=Cyc_NewControlFlow_get_unconsume_pat_vars((struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(_tmp487))->v);struct _tuple1 _tmp475=_tmp474;struct Cyc_List_List*_tmp483;struct Cyc_List_List*_tmp482;_LL4: _tmp483=_tmp475.f1;_tmp482=_tmp475.f2;_LL5:;{
union Cyc_CfFlowInfo_FlowInfo clause_inflow=Cyc_NewControlFlow_initialize_pat_vars(env->fenv,env,inflow,(struct Cyc_List_List*)_tmp487->v,_tmp483 != 0,_tmp484,exp_loc);
# 2474
union Cyc_CfFlowInfo_FlowInfo clause_outflow;
struct Cyc_List_List*_tmp476=env->unique_pat_vars;
# 2477
if(Cyc_Tcpat_has_vars(_tmp487))
({struct Cyc_List_List*_tmp7AF=({struct Cyc_List_List*_tmp478=_cycalloc(sizeof(*_tmp478));
({struct _tuple29*_tmp7AE=({struct _tuple29*_tmp477=_cycalloc(sizeof(*_tmp477));_tmp477->f1=0,_tmp477->f2=_tmp485,_tmp477->f3=_tmp483,_tmp477->f4=_tmp482;_tmp477;});_tmp478->hd=_tmp7AE;}),_tmp478->tl=_tmp476;_tmp478;});
# 2478
env->unique_pat_vars=_tmp7AF;});
# 2481
if(_tmp486 != 0){
struct Cyc_Absyn_Exp*wexp=_tmp486;
struct _tuple19 _tmp479=Cyc_NewControlFlow_anal_test(env,clause_inflow,wexp);struct _tuple19 _tmp47A=_tmp479;union Cyc_CfFlowInfo_FlowInfo _tmp47C;union Cyc_CfFlowInfo_FlowInfo _tmp47B;_LL7: _tmp47C=_tmp47A.f1;_tmp47B=_tmp47A.f2;_LL8:;
inflow=_tmp47B;
({union Cyc_CfFlowInfo_FlowInfo _tmp7B0=Cyc_NewControlFlow_anal_stmt(env,_tmp47C,_tmp485,0);clause_outflow=_tmp7B0;});}else{
# 2487
({union Cyc_CfFlowInfo_FlowInfo _tmp7B1=Cyc_NewControlFlow_anal_stmt(env,clause_inflow,_tmp485,0);clause_outflow=_tmp7B1;});}
# 2489
env->unique_pat_vars=_tmp476;{
union Cyc_CfFlowInfo_FlowInfo _tmp47D=clause_outflow;if((_tmp47D.BottomFL).tag == 1){_LLA: _LLB:
 goto _LL9;}else{_LLC: _LLD:
# 2494
({union Cyc_CfFlowInfo_FlowInfo _tmp7B2=Cyc_NewControlFlow_unconsume_params(env,_tmp483,_tmp482,0,clause_outflow,_tmp484);clause_outflow=_tmp7B2;});
# 2496
if(scs->tl == 0)
return clause_outflow;else{
# 2501
if((struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(((struct Cyc_Absyn_Switch_clause*)((struct Cyc_List_List*)_check_null(scs->tl))->hd)->pat_vars))->v != 0)
({void*_tmp47E=0U;({unsigned int _tmp7B4=_tmp485->loc;struct _dyneither_ptr _tmp7B3=({const char*_tmp47F="switch clause may implicitly fallthru";_tag_dyneither(_tmp47F,sizeof(char),38U);});Cyc_CfFlowInfo_aerr(_tmp7B4,_tmp7B3,_tag_dyneither(_tmp47E,sizeof(void*),0U));});});else{
# 2504
({void*_tmp480=0U;({unsigned int _tmp7B6=_tmp485->loc;struct _dyneither_ptr _tmp7B5=({const char*_tmp481="switch clause may implicitly fallthru";_tag_dyneither(_tmp481,sizeof(char),38U);});Cyc_Warn_warn(_tmp7B6,_tmp7B5,_tag_dyneither(_tmp480,sizeof(void*),0U));});});}
# 2506
Cyc_NewControlFlow_update_flow(env,((struct Cyc_Absyn_Switch_clause*)((struct Cyc_List_List*)_check_null(scs->tl))->hd)->body,clause_outflow);}
# 2508
goto _LL9;}_LL9:;};};};}
# 2511
return Cyc_CfFlowInfo_BottomFL();}struct _tuple30{struct Cyc_NewControlFlow_AnalEnv*f1;struct Cyc_Dict_Dict f2;unsigned int f3;};
# 2516
static void Cyc_NewControlFlow_check_dropped_unique_vd(struct _tuple30*vdenv,struct Cyc_Absyn_Vardecl*vd){
# 2518
struct _tuple30*_tmp488=vdenv;struct Cyc_NewControlFlow_AnalEnv*_tmp493;struct Cyc_Dict_Dict _tmp492;unsigned int _tmp491;_LL1: _tmp493=_tmp488->f1;_tmp492=_tmp488->f2;_tmp491=_tmp488->f3;_LL2:;
if(Cyc_Tcutil_is_noalias_pointer_or_aggr(vd->type)){
# 2521
struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct vdroot=({struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct _tmp632;_tmp632.tag=0U,_tmp632.f1=vd;_tmp632;});
struct _tuple15 _tmp489=Cyc_CfFlowInfo_unname_rval(({struct Cyc_Dict_Dict _tmp7B7=_tmp492;((void*(*)(struct Cyc_Dict_Dict d,int(*cmp)(void*,void*),void*k))Cyc_Dict_lookup_other)(_tmp7B7,Cyc_CfFlowInfo_root_cmp,(void*)({struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct*_tmp490=_cycalloc(sizeof(*_tmp490));*_tmp490=vdroot;_tmp490;}));}));struct _tuple15 _tmp48A=_tmp489;void*_tmp48F;_LL4: _tmp48F=_tmp48A.f1;_LL5:;{
void*_tmp48B=_tmp48F;switch(*((int*)_tmp48B)){case 7U: _LL7: _LL8:
 goto _LLA;case 0U: _LL9: _LLA:
 goto _LLC;case 2U: if(((struct Cyc_CfFlowInfo_UnknownR_CfFlowInfo_AbsRVal_struct*)_tmp48B)->f1 == Cyc_CfFlowInfo_NoneIL){_LLB: _LLC:
 goto _LL6;}else{goto _LLD;}default: _LLD: _LLE:
# 2528
({struct Cyc_String_pa_PrintArg_struct _tmp48E=({struct Cyc_String_pa_PrintArg_struct _tmp631;_tmp631.tag=0U,({struct _dyneither_ptr _tmp7B8=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(vd->name));_tmp631.f1=_tmp7B8;});_tmp631;});void*_tmp48C[1U];_tmp48C[0]=& _tmp48E;({unsigned int _tmp7BA=_tmp491;struct _dyneither_ptr _tmp7B9=({const char*_tmp48D="unique pointers may still exist after variable %s goes out of scope";_tag_dyneither(_tmp48D,sizeof(char),68U);});Cyc_Warn_warn(_tmp7BA,_tmp7B9,_tag_dyneither(_tmp48C,sizeof(void*),1U));});});
# 2530
goto _LL6;}_LL6:;};}}
# 2535
static void Cyc_NewControlFlow_check_dropped_unique(struct Cyc_NewControlFlow_AnalEnv*env,union Cyc_CfFlowInfo_FlowInfo inflow,struct Cyc_Absyn_Decl*decl){
{union Cyc_CfFlowInfo_FlowInfo _tmp494=inflow;struct Cyc_Dict_Dict _tmp4A0;if((_tmp494.ReachableFL).tag == 2){_LL1: _tmp4A0=((_tmp494.ReachableFL).val).f1;_LL2: {
# 2538
struct _tuple30 _tmp495=({struct _tuple30 _tmp633;_tmp633.f1=env,_tmp633.f2=_tmp4A0,_tmp633.f3=decl->loc;_tmp633;});
struct Cyc_CfFlowInfo_FlowEnv*_tmp496=env->fenv;
{void*_tmp497=decl->r;void*_tmp498=_tmp497;struct Cyc_List_List*_tmp49F;struct Cyc_List_List*_tmp49E;struct Cyc_Absyn_Vardecl*_tmp49D;switch(*((int*)_tmp498)){case 0U: _LL6: _tmp49D=((struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct*)_tmp498)->f1;_LL7:
# 2542
 Cyc_NewControlFlow_check_dropped_unique_vd(& _tmp495,_tmp49D);
goto _LL5;case 2U: if(((struct Cyc_Absyn_Let_d_Absyn_Raw_decl_struct*)_tmp498)->f2 != 0){_LL8: _tmp49E=(struct Cyc_List_List*)(((struct Cyc_Absyn_Let_d_Absyn_Raw_decl_struct*)_tmp498)->f2)->v;_LL9: {
# 2545
struct _tuple1 _tmp499=((struct _tuple1(*)(struct Cyc_List_List*x))Cyc_List_split)(_tmp49E);struct _tuple1 _tmp49A=_tmp499;struct Cyc_List_List*_tmp49C;_LLF: _tmp49C=_tmp49A.f1;_LL10:;{
struct Cyc_List_List*_tmp49B=((struct Cyc_List_List*(*)(struct Cyc_List_List*l))Cyc_Tcutil_filter_nulls)(_tmp49C);
_tmp49F=_tmp49B;goto _LLB;};}}else{goto _LLC;}case 3U: _LLA: _tmp49F=((struct Cyc_Absyn_Letv_d_Absyn_Raw_decl_struct*)_tmp498)->f1;_LLB:
# 2549
((void(*)(void(*f)(struct _tuple30*,struct Cyc_Absyn_Vardecl*),struct _tuple30*env,struct Cyc_List_List*x))Cyc_List_iter_c)(Cyc_NewControlFlow_check_dropped_unique_vd,& _tmp495,_tmp49F);
goto _LL5;default: _LLC: _LLD:
 goto _LL5;}_LL5:;}
# 2553
goto _LL0;}}else{_LL3: _LL4:
 goto _LL0;}_LL0:;}
# 2556
return;}
# 2562
static union Cyc_CfFlowInfo_FlowInfo Cyc_NewControlFlow_unconsume_pat_vars(struct Cyc_NewControlFlow_AnalEnv*env,struct Cyc_Absyn_Stmt*src,union Cyc_CfFlowInfo_FlowInfo inflow,struct Cyc_Absyn_Stmt*dest){
# 2566
int num_pop=((int(*)(struct Cyc_Hashtable_Table*t,struct Cyc_Absyn_Stmt*key))Cyc_Hashtable_lookup)(env->pat_pop_table,src);
{struct Cyc_List_List*x=env->unique_pat_vars;for(0;num_pop > 0;(x=x->tl,-- num_pop)){
struct _tuple29*_tmp4A1=(struct _tuple29*)((struct Cyc_List_List*)_check_null(x))->hd;struct _tuple29*_tmp4A2=_tmp4A1;int _tmp4A6;struct Cyc_Absyn_Stmt*_tmp4A5;struct Cyc_List_List*_tmp4A4;struct Cyc_List_List*_tmp4A3;_LL1: _tmp4A6=_tmp4A2->f1;_tmp4A5=_tmp4A2->f2;_tmp4A4=_tmp4A2->f3;_tmp4A3=_tmp4A2->f4;_LL2:;
({union Cyc_CfFlowInfo_FlowInfo _tmp7BB=Cyc_NewControlFlow_unconsume_params(env,_tmp4A4,_tmp4A3,_tmp4A6,inflow,dest->loc);inflow=_tmp7BB;});}}
# 2571
return inflow;}
# 2577
static union Cyc_CfFlowInfo_FlowInfo Cyc_NewControlFlow_anal_stmt(struct Cyc_NewControlFlow_AnalEnv*env,union Cyc_CfFlowInfo_FlowInfo inflow,struct Cyc_Absyn_Stmt*s,struct _tuple17*rval_opt){
# 2579
union Cyc_CfFlowInfo_FlowInfo outflow;
struct _tuple20 _tmp4A7=Cyc_NewControlFlow_pre_stmt_check(env,inflow,s);struct _tuple20 _tmp4A8=_tmp4A7;struct Cyc_NewControlFlow_CFStmtAnnot*_tmp542;union Cyc_CfFlowInfo_FlowInfo*_tmp541;_LL1: _tmp542=_tmp4A8.f1;_tmp541=_tmp4A8.f2;_LL2:;
inflow=*_tmp541;{
struct Cyc_CfFlowInfo_FlowEnv*_tmp4A9=env->fenv;
# 2586
void*_tmp4AA=s->r;void*_tmp4AB=_tmp4AA;struct Cyc_Absyn_Stmt*_tmp540;struct Cyc_Absyn_Decl*_tmp53F;struct Cyc_Absyn_Stmt*_tmp53E;struct Cyc_Absyn_Vardecl*_tmp53D;struct Cyc_Absyn_Exp*_tmp53C;unsigned int _tmp53B;struct Cyc_Absyn_Stmt*_tmp53A;struct Cyc_List_List*_tmp539;struct Cyc_Absyn_Exp*_tmp538;unsigned int _tmp537;struct Cyc_Absyn_Stmt*_tmp536;struct Cyc_Absyn_Stmt*_tmp535;struct Cyc_List_List*_tmp534;void*_tmp533;struct Cyc_Absyn_Exp*_tmp532;struct Cyc_List_List*_tmp531;void*_tmp530;struct Cyc_List_List*_tmp52F;struct Cyc_Absyn_Switch_clause*_tmp52E;struct Cyc_Absyn_Exp*_tmp52D;struct Cyc_Absyn_Exp*_tmp52C;struct Cyc_Absyn_Stmt*_tmp52B;struct Cyc_Absyn_Exp*_tmp52A;struct Cyc_Absyn_Stmt*_tmp529;struct Cyc_Absyn_Stmt*_tmp528;struct Cyc_Absyn_Stmt*_tmp527;struct Cyc_Absyn_Exp*_tmp526;struct Cyc_Absyn_Stmt*_tmp525;struct Cyc_Absyn_Exp*_tmp524;struct Cyc_Absyn_Stmt*_tmp523;struct Cyc_Absyn_Stmt*_tmp522;struct Cyc_Absyn_Exp*_tmp521;struct Cyc_Absyn_Stmt*_tmp520;struct Cyc_Absyn_Stmt*_tmp51F;struct Cyc_Absyn_Stmt*_tmp51E;struct Cyc_Absyn_Stmt*_tmp51D;struct Cyc_Absyn_Exp*_tmp51C;struct Cyc_Absyn_Exp*_tmp51B;switch(*((int*)_tmp4AB)){case 0U: _LL4: _LL5:
 return inflow;case 3U: if(((struct Cyc_Absyn_Return_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f1 == 0){_LL6: _LL7:
# 2590
 if(env->noreturn)
({void*_tmp4AC=0U;({unsigned int _tmp7BD=s->loc;struct _dyneither_ptr _tmp7BC=({const char*_tmp4AD="`noreturn' function might return";_tag_dyneither(_tmp4AD,sizeof(char),33U);});Cyc_CfFlowInfo_aerr(_tmp7BD,_tmp7BC,_tag_dyneither(_tmp4AC,sizeof(void*),0U));});});
Cyc_NewControlFlow_check_init_params(s->loc,env,inflow);
Cyc_NewControlFlow_unconsume_params(env,env->noconsume_params,0,0,inflow,s->loc);
return Cyc_CfFlowInfo_BottomFL();}else{_LL8: _tmp51B=((struct Cyc_Absyn_Return_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f1;_LL9:
# 2596
 if(env->noreturn)
({void*_tmp4AE=0U;({unsigned int _tmp7BF=s->loc;struct _dyneither_ptr _tmp7BE=({const char*_tmp4AF="`noreturn' function might return";_tag_dyneither(_tmp4AF,sizeof(char),33U);});Cyc_CfFlowInfo_aerr(_tmp7BF,_tmp7BE,_tag_dyneither(_tmp4AE,sizeof(void*),0U));});});{
struct _tuple16 _tmp4B0=Cyc_NewControlFlow_anal_Rexp(env,1,inflow,(struct Cyc_Absyn_Exp*)_check_null(_tmp51B));struct _tuple16 _tmp4B1=_tmp4B0;union Cyc_CfFlowInfo_FlowInfo _tmp4B3;void*_tmp4B2;_LL2D: _tmp4B3=_tmp4B1.f1;_tmp4B2=_tmp4B1.f2;_LL2E:;
({union Cyc_CfFlowInfo_FlowInfo _tmp7C0=Cyc_NewControlFlow_use_Rval(env,_tmp51B->loc,_tmp4B3,_tmp4B2);_tmp4B3=_tmp7C0;});
Cyc_NewControlFlow_check_init_params(s->loc,env,_tmp4B3);
Cyc_NewControlFlow_unconsume_params(env,env->noconsume_params,0,0,_tmp4B3,s->loc);
return Cyc_CfFlowInfo_BottomFL();};}case 1U: _LLA: _tmp51C=((struct Cyc_Absyn_Exp_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f1;_LLB: {
# 2605
struct _tuple17*_tmp4B4=rval_opt;void**_tmp4BA;int _tmp4B9;if(_tmp4B4 != 0){_LL30: _tmp4BA=(void**)& _tmp4B4->f1;_tmp4B9=_tmp4B4->f2;_LL31: {
# 2607
struct _tuple16 _tmp4B5=Cyc_NewControlFlow_anal_Rexp(env,_tmp4B9,inflow,_tmp51C);struct _tuple16 _tmp4B6=_tmp4B5;union Cyc_CfFlowInfo_FlowInfo _tmp4B8;void*_tmp4B7;_LL35: _tmp4B8=_tmp4B6.f1;_tmp4B7=_tmp4B6.f2;_LL36:;
*_tmp4BA=_tmp4B7;
return _tmp4B8;}}else{_LL32: _LL33:
# 2611
 return(Cyc_NewControlFlow_anal_Rexp(env,0,inflow,_tmp51C)).f1;}_LL2F:;}case 2U: _LLC: _tmp51E=((struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f1;_tmp51D=((struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f2;_LLD:
# 2615
 return({struct Cyc_NewControlFlow_AnalEnv*_tmp7C3=env;union Cyc_CfFlowInfo_FlowInfo _tmp7C2=Cyc_NewControlFlow_anal_stmt(env,inflow,_tmp51E,0);struct Cyc_Absyn_Stmt*_tmp7C1=_tmp51D;Cyc_NewControlFlow_anal_stmt(_tmp7C3,_tmp7C2,_tmp7C1,rval_opt);});case 4U: _LLE: _tmp521=((struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f1;_tmp520=((struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f2;_tmp51F=((struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f3;_LLF: {
# 2618
struct _tuple19 _tmp4BB=Cyc_NewControlFlow_anal_test(env,inflow,_tmp521);struct _tuple19 _tmp4BC=_tmp4BB;union Cyc_CfFlowInfo_FlowInfo _tmp4C0;union Cyc_CfFlowInfo_FlowInfo _tmp4BF;_LL38: _tmp4C0=_tmp4BC.f1;_tmp4BF=_tmp4BC.f2;_LL39:;{
# 2625
union Cyc_CfFlowInfo_FlowInfo _tmp4BD=Cyc_NewControlFlow_anal_stmt(env,_tmp4BF,_tmp51F,0);
union Cyc_CfFlowInfo_FlowInfo _tmp4BE=Cyc_NewControlFlow_anal_stmt(env,_tmp4C0,_tmp520,0);
return Cyc_CfFlowInfo_join_flow(_tmp4A9,_tmp4BE,_tmp4BD);};}case 5U: _LL10: _tmp524=(((struct Cyc_Absyn_While_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f1).f1;_tmp523=(((struct Cyc_Absyn_While_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f1).f2;_tmp522=((struct Cyc_Absyn_While_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f2;_LL11: {
# 2633
struct _tuple20 _tmp4C1=Cyc_NewControlFlow_pre_stmt_check(env,inflow,_tmp523);struct _tuple20 _tmp4C2=_tmp4C1;union Cyc_CfFlowInfo_FlowInfo*_tmp4C9;_LL3B: _tmp4C9=_tmp4C2.f2;_LL3C:;{
union Cyc_CfFlowInfo_FlowInfo _tmp4C3=*_tmp4C9;
struct _tuple19 _tmp4C4=Cyc_NewControlFlow_anal_test(env,_tmp4C3,_tmp524);struct _tuple19 _tmp4C5=_tmp4C4;union Cyc_CfFlowInfo_FlowInfo _tmp4C8;union Cyc_CfFlowInfo_FlowInfo _tmp4C7;_LL3E: _tmp4C8=_tmp4C5.f1;_tmp4C7=_tmp4C5.f2;_LL3F:;{
union Cyc_CfFlowInfo_FlowInfo _tmp4C6=Cyc_NewControlFlow_anal_stmt(env,_tmp4C8,_tmp522,0);
Cyc_NewControlFlow_update_flow(env,_tmp523,_tmp4C6);
return _tmp4C7;};};}case 14U: _LL12: _tmp527=((struct Cyc_Absyn_Do_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f1;_tmp526=(((struct Cyc_Absyn_Do_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f2).f1;_tmp525=(((struct Cyc_Absyn_Do_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f2).f2;_LL13: {
# 2643
union Cyc_CfFlowInfo_FlowInfo _tmp4CA=Cyc_NewControlFlow_anal_stmt(env,inflow,_tmp527,0);
struct _tuple20 _tmp4CB=Cyc_NewControlFlow_pre_stmt_check(env,_tmp4CA,_tmp525);struct _tuple20 _tmp4CC=_tmp4CB;union Cyc_CfFlowInfo_FlowInfo*_tmp4D2;_LL41: _tmp4D2=_tmp4CC.f2;_LL42:;{
union Cyc_CfFlowInfo_FlowInfo _tmp4CD=*_tmp4D2;
struct _tuple19 _tmp4CE=Cyc_NewControlFlow_anal_test(env,_tmp4CD,_tmp526);struct _tuple19 _tmp4CF=_tmp4CE;union Cyc_CfFlowInfo_FlowInfo _tmp4D1;union Cyc_CfFlowInfo_FlowInfo _tmp4D0;_LL44: _tmp4D1=_tmp4CF.f1;_tmp4D0=_tmp4CF.f2;_LL45:;
Cyc_NewControlFlow_update_flow(env,_tmp527,_tmp4D1);
return _tmp4D0;};}case 9U: _LL14: _tmp52D=((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f1;_tmp52C=(((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f2).f1;_tmp52B=(((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f2).f2;_tmp52A=(((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f3).f1;_tmp529=(((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f3).f2;_tmp528=((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f4;_LL15: {
# 2652
union Cyc_CfFlowInfo_FlowInfo _tmp4D3=(Cyc_NewControlFlow_anal_Rexp(env,0,inflow,_tmp52D)).f1;
struct _tuple20 _tmp4D4=Cyc_NewControlFlow_pre_stmt_check(env,_tmp4D3,_tmp52B);struct _tuple20 _tmp4D5=_tmp4D4;union Cyc_CfFlowInfo_FlowInfo*_tmp4E1;_LL47: _tmp4E1=_tmp4D5.f2;_LL48:;{
union Cyc_CfFlowInfo_FlowInfo _tmp4D6=*_tmp4E1;
struct _tuple19 _tmp4D7=Cyc_NewControlFlow_anal_test(env,_tmp4D6,_tmp52C);struct _tuple19 _tmp4D8=_tmp4D7;union Cyc_CfFlowInfo_FlowInfo _tmp4E0;union Cyc_CfFlowInfo_FlowInfo _tmp4DF;_LL4A: _tmp4E0=_tmp4D8.f1;_tmp4DF=_tmp4D8.f2;_LL4B:;{
union Cyc_CfFlowInfo_FlowInfo _tmp4D9=Cyc_NewControlFlow_anal_stmt(env,_tmp4E0,_tmp528,0);
struct _tuple20 _tmp4DA=Cyc_NewControlFlow_pre_stmt_check(env,_tmp4D9,_tmp529);struct _tuple20 _tmp4DB=_tmp4DA;union Cyc_CfFlowInfo_FlowInfo*_tmp4DE;_LL4D: _tmp4DE=_tmp4DB.f2;_LL4E:;{
union Cyc_CfFlowInfo_FlowInfo _tmp4DC=*_tmp4DE;
union Cyc_CfFlowInfo_FlowInfo _tmp4DD=(Cyc_NewControlFlow_anal_Rexp(env,0,_tmp4DC,_tmp52A)).f1;
Cyc_NewControlFlow_update_flow(env,_tmp52B,_tmp4DD);
return _tmp4DF;};};};}case 11U: if(((struct Cyc_Absyn_Fallthru_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f2 != 0){_LL16: _tmp52F=((struct Cyc_Absyn_Fallthru_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f1;_tmp52E=*((struct Cyc_Absyn_Fallthru_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f2;_LL17: {
# 2664
struct _tuple23 _tmp4E2=Cyc_NewControlFlow_anal_Rexps(env,inflow,_tmp52F,1,1);struct _tuple23 _tmp4E3=_tmp4E2;union Cyc_CfFlowInfo_FlowInfo _tmp4EA;struct Cyc_List_List*_tmp4E9;_LL50: _tmp4EA=_tmp4E3.f1;_tmp4E9=_tmp4E3.f2;_LL51:;
# 2666
({union Cyc_CfFlowInfo_FlowInfo _tmp7C4=Cyc_NewControlFlow_unconsume_pat_vars(env,s,inflow,_tmp52E->body);inflow=_tmp7C4;});{
# 2668
struct Cyc_List_List*_tmp4E4=((struct Cyc_List_List*(*)(struct Cyc_List_List*l))Cyc_Tcutil_filter_nulls)((((struct _tuple1(*)(struct Cyc_List_List*x))Cyc_List_split)((struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(_tmp52E->pat_vars))->v)).f1);
({union Cyc_CfFlowInfo_FlowInfo _tmp7C5=Cyc_NewControlFlow_add_vars(_tmp4A9,_tmp4EA,_tmp4E4,_tmp4A9->unknown_all,s->loc,0);_tmp4EA=_tmp7C5;});
# 2671
{struct Cyc_List_List*x=(struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(_tmp52E->pat_vars))->v;for(0;x != 0;x=x->tl){
struct _tuple24*_tmp4E5=(struct _tuple24*)x->hd;struct _tuple24*_tmp4E6=_tmp4E5;struct Cyc_Absyn_Vardecl**_tmp4E8;struct Cyc_Absyn_Exp*_tmp4E7;_LL53: _tmp4E8=_tmp4E6->f1;_tmp4E7=_tmp4E6->f2;_LL54:;
if(_tmp4E8 != 0){
({union Cyc_CfFlowInfo_FlowInfo _tmp7CC=({struct Cyc_CfFlowInfo_FlowEnv*_tmp7CB=_tmp4A9;struct Cyc_NewControlFlow_AnalEnv*_tmp7CA=env;union Cyc_CfFlowInfo_FlowInfo _tmp7C9=_tmp4EA;struct Cyc_Absyn_Vardecl*_tmp7C8=*_tmp4E8;struct Cyc_Absyn_Exp*_tmp7C7=(struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)_check_null(_tmp52F))->hd;void*_tmp7C6=(void*)((struct Cyc_List_List*)_check_null(_tmp4E9))->hd;Cyc_NewControlFlow_do_initialize_var(_tmp7CB,_tmp7CA,_tmp7C9,_tmp7C8,_tmp7C7,_tmp7C6,s->loc);});_tmp4EA=_tmp7CC;});
_tmp4E9=_tmp4E9->tl;
_tmp52F=_tmp52F->tl;}}}
# 2679
Cyc_NewControlFlow_update_flow(env,_tmp52E->body,_tmp4EA);
return Cyc_CfFlowInfo_BottomFL();};}}else{_LL2A: _LL2B:
# 2843
({void*_tmp519=0U;({struct _dyneither_ptr _tmp7CD=({const char*_tmp51A="anal_stmt: bad stmt syntax or unimplemented stmt form";_tag_dyneither(_tmp51A,sizeof(char),54U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp7CD,_tag_dyneither(_tmp519,sizeof(void*),0U));});});}case 6U: _LL18: _LL19:
# 2685
 if(((struct Cyc_Absyn_Stmt*(*)(struct Cyc_Hashtable_Table*t,struct Cyc_Absyn_Stmt*key))Cyc_Hashtable_lookup)(env->succ_table,s)== 0){
{union Cyc_CfFlowInfo_FlowInfo _tmp4EB=inflow;if((_tmp4EB.ReachableFL).tag == 2){_LL56: _LL57:
# 2688
 if(!Cyc_Tcutil_is_void_type(env->return_type)){
if(Cyc_Tcutil_is_any_float_type(env->return_type) || 
Cyc_Tcutil_is_any_int_type(env->return_type))
({void*_tmp4EC=0U;({unsigned int _tmp7CF=s->loc;struct _dyneither_ptr _tmp7CE=({const char*_tmp4ED="break may cause function not to return a value";_tag_dyneither(_tmp4ED,sizeof(char),47U);});Cyc_Warn_warn(_tmp7CF,_tmp7CE,_tag_dyneither(_tmp4EC,sizeof(void*),0U));});});else{
# 2693
({void*_tmp4EE=0U;({unsigned int _tmp7D1=s->loc;struct _dyneither_ptr _tmp7D0=({const char*_tmp4EF="break may cause function not to return a value";_tag_dyneither(_tmp4EF,sizeof(char),47U);});Cyc_CfFlowInfo_aerr(_tmp7D1,_tmp7D0,_tag_dyneither(_tmp4EE,sizeof(void*),0U));});});}}
# 2695
goto _LL55;}else{_LL58: _LL59:
 goto _LL55;}_LL55:;}
# 2698
if(env->noreturn)
({void*_tmp4F0=0U;({unsigned int _tmp7D3=s->loc;struct _dyneither_ptr _tmp7D2=({const char*_tmp4F1="`noreturn' function might return";_tag_dyneither(_tmp4F1,sizeof(char),33U);});Cyc_CfFlowInfo_aerr(_tmp7D3,_tmp7D2,_tag_dyneither(_tmp4F0,sizeof(void*),0U));});});
Cyc_NewControlFlow_check_init_params(s->loc,env,inflow);
Cyc_NewControlFlow_unconsume_params(env,env->noconsume_params,0,0,inflow,s->loc);
return Cyc_CfFlowInfo_BottomFL();}
# 2704
goto _LL1B;case 7U: _LL1A: _LL1B:
 goto _LL1D;case 8U: _LL1C: _LL1D: {
# 2708
struct Cyc_Absyn_Stmt*_tmp4F2=((struct Cyc_Absyn_Stmt*(*)(struct Cyc_Hashtable_Table*t,struct Cyc_Absyn_Stmt*key))Cyc_Hashtable_lookup)(env->succ_table,s);
if(_tmp4F2 == 0)
({void*_tmp4F3=0U;({unsigned int _tmp7D5=s->loc;struct _dyneither_ptr _tmp7D4=({const char*_tmp4F4="jump has no target (should be impossible)";_tag_dyneither(_tmp4F4,sizeof(char),42U);});Cyc_CfFlowInfo_aerr(_tmp7D5,_tmp7D4,_tag_dyneither(_tmp4F3,sizeof(void*),0U));});});
({union Cyc_CfFlowInfo_FlowInfo _tmp7D6=Cyc_NewControlFlow_unconsume_pat_vars(env,s,inflow,(struct Cyc_Absyn_Stmt*)_check_null(_tmp4F2));inflow=_tmp7D6;});
# 2713
Cyc_NewControlFlow_update_flow(env,_tmp4F2,inflow);
return Cyc_CfFlowInfo_BottomFL();}case 10U: _LL1E: _tmp532=((struct Cyc_Absyn_Switch_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f1;_tmp531=((struct Cyc_Absyn_Switch_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f2;_tmp530=(void*)((struct Cyc_Absyn_Switch_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f3;_LL1F:
# 2719
 return Cyc_NewControlFlow_anal_scs(env,inflow,_tmp531,_tmp532->loc);case 15U: _LL20: _tmp535=((struct Cyc_Absyn_TryCatch_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f1;_tmp534=((struct Cyc_Absyn_TryCatch_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f2;_tmp533=(void*)((struct Cyc_Absyn_TryCatch_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f3;_LL21: {
# 2724
int old_in_try=env->in_try;
union Cyc_CfFlowInfo_FlowInfo old_tryflow=env->tryflow;
env->in_try=1;
env->tryflow=inflow;{
union Cyc_CfFlowInfo_FlowInfo s1_outflow=Cyc_NewControlFlow_anal_stmt(env,inflow,_tmp535,0);
union Cyc_CfFlowInfo_FlowInfo scs_inflow=env->tryflow;
# 2732
env->in_try=old_in_try;
env->tryflow=old_tryflow;
# 2735
Cyc_NewControlFlow_update_tryflow(env,scs_inflow);{
union Cyc_CfFlowInfo_FlowInfo scs_outflow=Cyc_NewControlFlow_anal_scs(env,scs_inflow,_tmp534,0U);
{union Cyc_CfFlowInfo_FlowInfo _tmp4F5=scs_outflow;if((_tmp4F5.BottomFL).tag == 1){_LL5B: _LL5C:
 goto _LL5A;}else{_LL5D: _LL5E:
({void*_tmp4F6=0U;({unsigned int _tmp7D8=s->loc;struct _dyneither_ptr _tmp7D7=({const char*_tmp4F7="last catch clause may implicitly fallthru";_tag_dyneither(_tmp4F7,sizeof(char),42U);});Cyc_CfFlowInfo_aerr(_tmp7D8,_tmp7D7,_tag_dyneither(_tmp4F6,sizeof(void*),0U));});});}_LL5A:;}
# 2741
outflow=s1_outflow;
# 2743
return outflow;};};}case 12U: switch(*((int*)((struct Cyc_Absyn_Decl*)((struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f1)->r)){case 2U: if(((struct Cyc_Absyn_Let_d_Absyn_Raw_decl_struct*)((struct Cyc_Absyn_Decl*)((struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f1)->r)->f2 != 0){_LL22: _tmp539=(struct Cyc_List_List*)(((struct Cyc_Absyn_Let_d_Absyn_Raw_decl_struct*)(((struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f1)->r)->f2)->v;_tmp538=((struct Cyc_Absyn_Let_d_Absyn_Raw_decl_struct*)(((struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f1)->r)->f3;_tmp537=(((struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f1)->loc;_tmp536=((struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f2;_LL23: {
# 2753
struct _tuple1 _tmp4F8=Cyc_NewControlFlow_get_unconsume_pat_vars(_tmp539);struct _tuple1 _tmp4F9=_tmp4F8;struct Cyc_List_List*_tmp4FE;struct Cyc_List_List*_tmp4FD;_LL60: _tmp4FE=_tmp4F9.f1;_tmp4FD=_tmp4F9.f2;_LL61:;
({union Cyc_CfFlowInfo_FlowInfo _tmp7D9=Cyc_NewControlFlow_initialize_pat_vars(_tmp4A9,env,inflow,_tmp539,_tmp4FE != 0,_tmp537,_tmp538->loc);inflow=_tmp7D9;});{
struct Cyc_List_List*_tmp4FA=env->unique_pat_vars;
# 2757
({struct Cyc_List_List*_tmp7DB=({struct Cyc_List_List*_tmp4FC=_cycalloc(sizeof(*_tmp4FC));
({struct _tuple29*_tmp7DA=({struct _tuple29*_tmp4FB=_cycalloc(sizeof(*_tmp4FB));_tmp4FB->f1=0,_tmp4FB->f2=s,_tmp4FB->f3=_tmp4FE,_tmp4FB->f4=_tmp4FD;_tmp4FB;});_tmp4FC->hd=_tmp7DA;}),_tmp4FC->tl=_tmp4FA;_tmp4FC;});
# 2757
env->unique_pat_vars=_tmp7DB;});
# 2762
({union Cyc_CfFlowInfo_FlowInfo _tmp7DC=Cyc_NewControlFlow_anal_stmt(env,inflow,_tmp536,rval_opt);inflow=_tmp7DC;});
env->unique_pat_vars=_tmp4FA;
# 2766
({union Cyc_CfFlowInfo_FlowInfo _tmp7DD=Cyc_NewControlFlow_unconsume_params(env,_tmp4FE,_tmp4FD,0,inflow,_tmp537);inflow=_tmp7DD;});
# 2770
return inflow;};}}else{goto _LL26;}case 4U: _LL24: _tmp53D=((struct Cyc_Absyn_Region_d_Absyn_Raw_decl_struct*)(((struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f1)->r)->f2;_tmp53C=((struct Cyc_Absyn_Region_d_Absyn_Raw_decl_struct*)(((struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f1)->r)->f3;_tmp53B=(((struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f1)->loc;_tmp53A=((struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f2;if(_tmp53C != 0){_LL25: {
# 2782
struct Cyc_List_List l=({struct Cyc_List_List _tmp635;_tmp635.hd=_tmp53C,_tmp635.tl=0;_tmp635;});
union Cyc_CfFlowInfo_FlowInfo _tmp4FF=Cyc_NewControlFlow_expand_unique_places(env,inflow,& l);
struct _tuple18 _tmp500=Cyc_NewControlFlow_anal_Lexp(env,_tmp4FF,0,0,_tmp53C);struct _tuple18 _tmp501=_tmp500;union Cyc_CfFlowInfo_AbsLVal _tmp518;_LL63: _tmp518=_tmp501.f2;_LL64:;{
struct _tuple16 _tmp502=Cyc_NewControlFlow_anal_Rexp(env,1,_tmp4FF,_tmp53C);struct _tuple16 _tmp503=_tmp502;union Cyc_CfFlowInfo_FlowInfo _tmp517;_LL66: _tmp517=_tmp503.f1;_LL67:;{
struct Cyc_List_List*_tmp504=0;
struct Cyc_List_List*_tmp505=0;
{union Cyc_CfFlowInfo_FlowInfo _tmp506=_tmp517;struct Cyc_Dict_Dict _tmp512;struct Cyc_List_List*_tmp511;if((_tmp506.ReachableFL).tag == 2){_LL69: _tmp512=((_tmp506.ReachableFL).val).f1;_tmp511=((_tmp506.ReachableFL).val).f2;_LL6A:
# 2790
{union Cyc_CfFlowInfo_AbsLVal _tmp507=_tmp518;struct Cyc_CfFlowInfo_Place*_tmp510;if((_tmp507.PlaceL).tag == 1){_LL6E: _tmp510=(_tmp507.PlaceL).val;_LL6F: {
# 2794
void*_tmp508=Cyc_CfFlowInfo_lookup_place(_tmp512,_tmp510);
({void*_tmp7DE=(void*)({struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct*_tmp509=_cycalloc(sizeof(*_tmp509));_tmp509->tag=8U,_tmp509->f1=_tmp53D,_tmp509->f2=_tmp508;_tmp509;});_tmp508=_tmp7DE;});
({struct Cyc_Dict_Dict _tmp7DF=Cyc_CfFlowInfo_assign_place(_tmp4A9,_tmp53C->loc,_tmp512,_tmp510,_tmp508);_tmp512=_tmp7DF;});
({union Cyc_CfFlowInfo_FlowInfo _tmp7E0=Cyc_CfFlowInfo_ReachableFL(_tmp512,_tmp511);_tmp517=_tmp7E0;});{
struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct*_tmp50A=({struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct*_tmp50F=_cycalloc(sizeof(*_tmp50F));_tmp50F->tag=0U,_tmp50F->f1=_tmp53D;_tmp50F;});
struct Cyc_CfFlowInfo_Place*_tmp50B=({struct Cyc_CfFlowInfo_Place*_tmp50E=_cycalloc(sizeof(*_tmp50E));_tmp50E->root=(void*)_tmp50A,_tmp50E->path=0;_tmp50E;});
({struct Cyc_List_List*_tmp7E1=({struct Cyc_List_List*_tmp50C=_cycalloc(sizeof(*_tmp50C));_tmp50C->hd=_tmp50B,_tmp50C->tl=_tmp504;_tmp50C;});_tmp504=_tmp7E1;});
({struct Cyc_List_List*_tmp7E2=({struct Cyc_List_List*_tmp50D=_cycalloc(sizeof(*_tmp50D));_tmp50D->hd=_tmp53C,_tmp50D->tl=_tmp505;_tmp50D;});_tmp505=_tmp7E2;});
goto _LL6D;};}}else{_LL70: _LL71:
# 2809
 goto _LL6D;}_LL6D:;}
# 2811
goto _LL68;}else{_LL6B: _LL6C:
# 2813
 goto _LL68;}_LL68:;}{
# 2816
struct Cyc_List_List _tmp513=({struct Cyc_List_List _tmp634;_tmp634.hd=_tmp53D,_tmp634.tl=0;_tmp634;});
({union Cyc_CfFlowInfo_FlowInfo _tmp7E3=Cyc_NewControlFlow_add_vars(_tmp4A9,_tmp517,& _tmp513,_tmp4A9->unknown_all,_tmp53B,0);_tmp517=_tmp7E3;});{
# 2820
struct Cyc_List_List*_tmp514=env->unique_pat_vars;
({struct Cyc_List_List*_tmp7E5=({struct Cyc_List_List*_tmp516=_cycalloc(sizeof(*_tmp516));({struct _tuple29*_tmp7E4=({struct _tuple29*_tmp515=_cycalloc(sizeof(*_tmp515));_tmp515->f1=1,_tmp515->f2=s,_tmp515->f3=_tmp504,_tmp515->f4=_tmp505;_tmp515;});_tmp516->hd=_tmp7E4;}),_tmp516->tl=_tmp514;_tmp516;});env->unique_pat_vars=_tmp7E5;});
# 2825
({union Cyc_CfFlowInfo_FlowInfo _tmp7E6=Cyc_NewControlFlow_anal_stmt(env,_tmp517,_tmp53A,rval_opt);_tmp517=_tmp7E6;});
env->unique_pat_vars=_tmp514;
# 2829
({union Cyc_CfFlowInfo_FlowInfo _tmp7E7=Cyc_NewControlFlow_unconsume_params(env,_tmp504,_tmp505,1,_tmp517,_tmp53B);_tmp517=_tmp7E7;});
# 2833
return _tmp517;};};};};}}else{goto _LL26;}default: _LL26: _tmp53F=((struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f1;_tmp53E=((struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f2;_LL27:
# 2836
({union Cyc_CfFlowInfo_FlowInfo _tmp7EB=({struct Cyc_NewControlFlow_AnalEnv*_tmp7EA=env;union Cyc_CfFlowInfo_FlowInfo _tmp7E9=Cyc_NewControlFlow_anal_decl(env,inflow,_tmp53F);struct Cyc_Absyn_Stmt*_tmp7E8=_tmp53E;Cyc_NewControlFlow_anal_stmt(_tmp7EA,_tmp7E9,_tmp7E8,rval_opt);});outflow=_tmp7EB;});
if(Cyc_NewControlFlow_warn_lose_unique)
Cyc_NewControlFlow_check_dropped_unique(env,outflow,_tmp53F);
return outflow;}default: _LL28: _tmp540=((struct Cyc_Absyn_Label_s_Absyn_Raw_stmt_struct*)_tmp4AB)->f2;_LL29:
# 2841
 return Cyc_NewControlFlow_anal_stmt(env,inflow,_tmp540,rval_opt);}_LL3:;};}
# 2847
static void Cyc_NewControlFlow_check_nested_fun(struct Cyc_JumpAnalysis_Jump_Anal_Result*tables,struct Cyc_CfFlowInfo_FlowEnv*,union Cyc_CfFlowInfo_FlowInfo inflow,struct Cyc_Absyn_Fndecl*fd);
# 2851
static union Cyc_CfFlowInfo_FlowInfo Cyc_NewControlFlow_anal_decl(struct Cyc_NewControlFlow_AnalEnv*env,union Cyc_CfFlowInfo_FlowInfo inflow,struct Cyc_Absyn_Decl*decl){
struct Cyc_CfFlowInfo_FlowEnv*_tmp543=env->fenv;
void*_tmp544=decl->r;void*_tmp545=_tmp544;struct Cyc_Absyn_Tvar*_tmp558;struct Cyc_Absyn_Vardecl*_tmp557;struct Cyc_Absyn_Exp*_tmp556;struct Cyc_Absyn_Fndecl*_tmp555;struct Cyc_List_List*_tmp554;struct Cyc_Absyn_Vardecl*_tmp553;switch(*((int*)_tmp545)){case 0U: _LL1: _tmp553=((struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct*)_tmp545)->f1;_LL2: {
# 2861
struct Cyc_List_List _tmp546=({struct Cyc_List_List _tmp636;_tmp636.hd=_tmp553,_tmp636.tl=0;_tmp636;});
({union Cyc_CfFlowInfo_FlowInfo _tmp7EC=Cyc_NewControlFlow_add_vars(_tmp543,inflow,& _tmp546,_tmp543->unknown_none,decl->loc,0);inflow=_tmp7EC;});{
struct Cyc_Absyn_Exp*_tmp547=_tmp553->initializer;
if(_tmp547 == 0)
return inflow;{
struct _tuple16 _tmp548=Cyc_NewControlFlow_anal_Rexp(env,1,inflow,_tmp547);struct _tuple16 _tmp549=_tmp548;union Cyc_CfFlowInfo_FlowInfo _tmp54B;void*_tmp54A;_LLC: _tmp54B=_tmp549.f1;_tmp54A=_tmp549.f2;_LLD:;
return Cyc_NewControlFlow_do_initialize_var(_tmp543,env,_tmp54B,_tmp553,_tmp547,_tmp54A,decl->loc);};};}case 3U: _LL3: _tmp554=((struct Cyc_Absyn_Letv_d_Absyn_Raw_decl_struct*)_tmp545)->f1;_LL4:
# 2870
 return Cyc_NewControlFlow_add_vars(_tmp543,inflow,_tmp554,_tmp543->unknown_none,decl->loc,0);case 1U: _LL5: _tmp555=((struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct*)_tmp545)->f1;_LL6:
# 2873
 Cyc_NewControlFlow_check_nested_fun(env->all_tables,_tmp543,inflow,_tmp555);{
void*t=(void*)_check_null(_tmp555->cached_type);
struct Cyc_Absyn_Vardecl*_tmp54C=(struct Cyc_Absyn_Vardecl*)_check_null(_tmp555->fn_vardecl);
# 2879
return({struct Cyc_CfFlowInfo_FlowEnv*_tmp7F0=_tmp543;union Cyc_CfFlowInfo_FlowInfo _tmp7EF=inflow;struct Cyc_List_List*_tmp7EE=({struct Cyc_List_List*_tmp54D=_cycalloc(sizeof(*_tmp54D));_tmp54D->hd=_tmp54C,_tmp54D->tl=0;_tmp54D;});void*_tmp7ED=_tmp543->unknown_all;Cyc_NewControlFlow_add_vars(_tmp7F0,_tmp7EF,_tmp7EE,_tmp7ED,decl->loc,0);});};case 4U: _LL7: _tmp558=((struct Cyc_Absyn_Region_d_Absyn_Raw_decl_struct*)_tmp545)->f1;_tmp557=((struct Cyc_Absyn_Region_d_Absyn_Raw_decl_struct*)_tmp545)->f2;_tmp556=((struct Cyc_Absyn_Region_d_Absyn_Raw_decl_struct*)_tmp545)->f3;_LL8:
# 2882
 if(_tmp556 != 0)
({void*_tmp54E=0U;({struct _dyneither_ptr _tmp7F1=({const char*_tmp54F="found open expression in declaration!";_tag_dyneither(_tmp54F,sizeof(char),38U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp7F1,_tag_dyneither(_tmp54E,sizeof(void*),0U));});});{
struct Cyc_List_List _tmp550=({struct Cyc_List_List _tmp637;_tmp637.hd=_tmp557,_tmp637.tl=0;_tmp637;});
return Cyc_NewControlFlow_add_vars(_tmp543,inflow,& _tmp550,_tmp543->unknown_all,decl->loc,0);};default: _LL9: _LLA:
# 2888
({void*_tmp551=0U;({struct _dyneither_ptr _tmp7F2=({const char*_tmp552="anal_decl: unexpected decl variant";_tag_dyneither(_tmp552,sizeof(char),35U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp7F2,_tag_dyneither(_tmp551,sizeof(void*),0U));});});}_LL0:;}
# 2896
static void Cyc_NewControlFlow_check_fun(struct Cyc_JumpAnalysis_Jump_Anal_Result*tables,struct Cyc_Absyn_Fndecl*fd){
struct _handler_cons _tmp559;_push_handler(& _tmp559);{int _tmp55B=0;if(setjmp(_tmp559.handler))_tmp55B=1;if(!_tmp55B){
{struct Cyc_CfFlowInfo_FlowEnv*fenv=Cyc_CfFlowInfo_new_flow_env();
({struct Cyc_JumpAnalysis_Jump_Anal_Result*_tmp7F5=tables;struct Cyc_CfFlowInfo_FlowEnv*_tmp7F4=fenv;union Cyc_CfFlowInfo_FlowInfo _tmp7F3=Cyc_CfFlowInfo_ReachableFL(fenv->mt_flowdict,0);Cyc_NewControlFlow_check_nested_fun(_tmp7F5,_tmp7F4,_tmp7F3,fd);});}
# 2898
;_pop_handler();}else{void*_tmp55A=(void*)_exn_thrown;void*_tmp55C=_tmp55A;void*_tmp55E;if(((struct Cyc_Dict_Absent_exn_struct*)_tmp55C)->tag == Cyc_Dict_Absent){_LL1: _LL2:
# 2904
 if(Cyc_Position_num_errors > 0)
goto _LL0;else{
Cyc_Core_rethrow((void*)({struct Cyc_Dict_Absent_exn_struct*_tmp55D=_cycalloc(sizeof(*_tmp55D));_tmp55D->tag=Cyc_Dict_Absent;_tmp55D;}));}}else{_LL3: _tmp55E=_tmp55C;_LL4:(int)_rethrow(_tmp55E);}_LL0:;}};}
# 2910
static int Cyc_NewControlFlow_hash_ptr(void*s){
return(int)s;}
# 2915
static union Cyc_Relations_RelnOp Cyc_NewControlFlow_translate_rop(struct Cyc_List_List*vds,union Cyc_Relations_RelnOp r){
union Cyc_Relations_RelnOp _tmp55F=r;unsigned int _tmp561;if((_tmp55F.RParam).tag == 5){_LL1: _tmp561=(_tmp55F.RParam).val;_LL2: {
# 2918
struct Cyc_Absyn_Vardecl*_tmp560=((struct Cyc_Absyn_Vardecl*(*)(struct Cyc_List_List*x,int n))Cyc_List_nth)(vds,(int)_tmp561);
if(!_tmp560->escapes)
return Cyc_Relations_RVar(_tmp560);
return r;}}else{_LL3: _LL4:
 return r;}_LL0:;}
# 2926
static void Cyc_NewControlFlow_check_nested_fun(struct Cyc_JumpAnalysis_Jump_Anal_Result*tables,struct Cyc_CfFlowInfo_FlowEnv*fenv,union Cyc_CfFlowInfo_FlowInfo inflow,struct Cyc_Absyn_Fndecl*fd){
# 2930
unsigned int _tmp562=(fd->body)->loc;
({union Cyc_CfFlowInfo_FlowInfo _tmp7F6=Cyc_NewControlFlow_add_vars(fenv,inflow,(struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(fd->param_vardecls))->v,fenv->unknown_all,_tmp562,1);inflow=_tmp7F6;});{
# 2935
struct Cyc_List_List*param_roots=0;
struct Cyc_List_List*noconsume_roots=0;
struct _tuple14 _tmp563=({union Cyc_CfFlowInfo_FlowInfo _tmp592=inflow;if((_tmp592.ReachableFL).tag != 2)_throw_match();(_tmp592.ReachableFL).val;});struct _tuple14 _tmp564=_tmp563;struct Cyc_Dict_Dict _tmp591;struct Cyc_List_List*_tmp590;_LL1: _tmp591=_tmp564.f1;_tmp590=_tmp564.f2;_LL2:;{
# 2940
struct Cyc_List_List*_tmp565=(struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(fd->param_vardecls))->v;
{struct Cyc_List_List*_tmp566=fd->requires_relns;for(0;_tmp566 != 0;_tmp566=_tmp566->tl){
struct Cyc_Relations_Reln*_tmp567=(struct Cyc_Relations_Reln*)_tmp566->hd;
({struct Cyc_List_List*_tmp7FB=({struct _RegionHandle*_tmp7FA=Cyc_Core_heap_region;union Cyc_Relations_RelnOp _tmp7F9=Cyc_NewControlFlow_translate_rop(_tmp565,_tmp567->rop1);enum Cyc_Relations_Relation _tmp7F8=_tmp567->relation;union Cyc_Relations_RelnOp _tmp7F7=
Cyc_NewControlFlow_translate_rop(_tmp565,_tmp567->rop2);
# 2943
Cyc_Relations_add_relation(_tmp7FA,_tmp7F9,_tmp7F8,_tmp7F7,_tmp590);});_tmp590=_tmp7FB;});}}{
# 2949
struct Cyc_List_List*atts;
{void*_tmp568=Cyc_Tcutil_compress((void*)_check_null(fd->cached_type));void*_tmp569=_tmp568;struct Cyc_List_List*_tmp56C;if(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp569)->tag == 5U){_LL4: _tmp56C=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp569)->f1).attributes;_LL5:
 atts=_tmp56C;goto _LL3;}else{_LL6: _LL7:
({void*_tmp56A=0U;({struct _dyneither_ptr _tmp7FC=({const char*_tmp56B="check_fun: non-function type cached with fndecl_t";_tag_dyneither(_tmp56B,sizeof(char),50U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp7FC,_tag_dyneither(_tmp56A,sizeof(void*),0U));});});}_LL3:;}
# 2956
for(0;atts != 0;atts=atts->tl){
void*_tmp56D=(void*)atts->hd;void*_tmp56E=_tmp56D;int _tmp587;int _tmp586;int _tmp585;switch(*((int*)_tmp56E)){case 21U: _LL9: _tmp585=((struct Cyc_Absyn_Noliveunique_att_Absyn_Attribute_struct*)_tmp56E)->f1;_LLA: {
# 2959
struct Cyc_Absyn_Exp*bogus_exp=Cyc_Absyn_signed_int_exp(- 1,0U);
struct Cyc_Absyn_Vardecl*_tmp56F=((struct Cyc_Absyn_Vardecl*(*)(struct Cyc_List_List*x,int n))Cyc_List_nth)((struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(fd->param_vardecls))->v,_tmp585 - 1);
void*t=Cyc_Tcutil_compress(_tmp56F->type);
void*elttype=Cyc_Tcutil_pointer_elt_type(t);
void*_tmp570=({
struct Cyc_CfFlowInfo_FlowEnv*_tmp7FF=fenv;void*_tmp7FE=elttype;struct Cyc_Absyn_Exp*_tmp7FD=bogus_exp;Cyc_CfFlowInfo_make_unique_consumed(_tmp7FF,_tmp7FE,_tmp7FD,- 1,
Cyc_CfFlowInfo_typ_to_absrval(fenv,elttype,0,fenv->unknown_all));});
struct Cyc_CfFlowInfo_InitParam_CfFlowInfo_Root_struct*_tmp571=({struct Cyc_CfFlowInfo_InitParam_CfFlowInfo_Root_struct*_tmp576=_cycalloc(sizeof(*_tmp576));_tmp576->tag=2U,_tmp576->f1=_tmp585,_tmp576->f2=t;_tmp576;});
struct Cyc_CfFlowInfo_Place*_tmp572=({struct Cyc_CfFlowInfo_Place*_tmp575=_cycalloc(sizeof(*_tmp575));_tmp575->root=(void*)_tmp571,_tmp575->path=0;_tmp575;});
({struct Cyc_Dict_Dict _tmp800=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,void*k,void*v))Cyc_Dict_insert)(_tmp591,(void*)_tmp571,_tmp570);_tmp591=_tmp800;});
({struct Cyc_Dict_Dict _tmp803=({struct Cyc_Dict_Dict _tmp802=_tmp591;void*_tmp801=(void*)({struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct*_tmp573=_cycalloc(sizeof(*_tmp573));_tmp573->tag=0U,_tmp573->f1=_tmp56F;_tmp573;});((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,void*k,void*v))Cyc_Dict_insert)(_tmp802,_tmp801,(void*)({struct Cyc_CfFlowInfo_AddressOf_CfFlowInfo_AbsRVal_struct*_tmp574=_cycalloc(sizeof(*_tmp574));_tmp574->tag=4U,_tmp574->f1=_tmp572;_tmp574;}));});_tmp591=_tmp803;});
goto _LL8;}case 20U: _LLB: _tmp586=((struct Cyc_Absyn_Initializes_att_Absyn_Attribute_struct*)_tmp56E)->f1;_LLC: {
# 2972
struct Cyc_Absyn_Vardecl*_tmp577=((struct Cyc_Absyn_Vardecl*(*)(struct Cyc_List_List*x,int n))Cyc_List_nth)((struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(fd->param_vardecls))->v,_tmp586 - 1);
void*elttype=Cyc_Tcutil_pointer_elt_type(_tmp577->type);
struct Cyc_CfFlowInfo_InitParam_CfFlowInfo_Root_struct*_tmp578=({struct Cyc_CfFlowInfo_InitParam_CfFlowInfo_Root_struct*_tmp57E=_cycalloc(sizeof(*_tmp57E));_tmp57E->tag=2U,_tmp57E->f1=_tmp586,_tmp57E->f2=elttype;_tmp57E;});
struct Cyc_CfFlowInfo_Place*_tmp579=({struct Cyc_CfFlowInfo_Place*_tmp57D=_cycalloc(sizeof(*_tmp57D));_tmp57D->root=(void*)_tmp578,_tmp57D->path=0;_tmp57D;});
({struct Cyc_Dict_Dict _tmp806=({struct Cyc_Dict_Dict _tmp805=_tmp591;void*_tmp804=(void*)_tmp578;((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,void*k,void*v))Cyc_Dict_insert)(_tmp805,_tmp804,Cyc_CfFlowInfo_typ_to_absrval(fenv,elttype,0,fenv->esc_none));});_tmp591=_tmp806;});
({struct Cyc_Dict_Dict _tmp809=({struct Cyc_Dict_Dict _tmp808=_tmp591;void*_tmp807=(void*)({struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct*_tmp57A=_cycalloc(sizeof(*_tmp57A));_tmp57A->tag=0U,_tmp57A->f1=_tmp577;_tmp57A;});((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,void*k,void*v))Cyc_Dict_insert)(_tmp808,_tmp807,(void*)({struct Cyc_CfFlowInfo_AddressOf_CfFlowInfo_AbsRVal_struct*_tmp57B=_cycalloc(sizeof(*_tmp57B));_tmp57B->tag=4U,_tmp57B->f1=_tmp579;_tmp57B;}));});_tmp591=_tmp809;});
({struct Cyc_List_List*_tmp80A=({struct Cyc_List_List*_tmp57C=_cycalloc(sizeof(*_tmp57C));_tmp57C->hd=_tmp579,_tmp57C->tl=param_roots;_tmp57C;});param_roots=_tmp80A;});
goto _LL8;}case 22U: _LLD: _tmp587=((struct Cyc_Absyn_Noconsume_att_Absyn_Attribute_struct*)_tmp56E)->f1;_LLE: {
# 2981
struct Cyc_Absyn_Vardecl*_tmp57F=((struct Cyc_Absyn_Vardecl*(*)(struct Cyc_List_List*x,int n))Cyc_List_nth)((struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(fd->param_vardecls))->v,_tmp587 - 1);
if(Cyc_Tcutil_is_noalias_pointer(_tmp57F->type,0)){
struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct*_tmp580=({struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct*_tmp584=_cycalloc(sizeof(*_tmp584));_tmp584->tag=0U,_tmp584->f1=_tmp57F;_tmp584;});
struct Cyc_CfFlowInfo_Place*_tmp581=({struct Cyc_CfFlowInfo_Place*_tmp583=_cycalloc(sizeof(*_tmp583));_tmp583->root=(void*)_tmp580,_tmp583->path=0;_tmp583;});
({struct Cyc_List_List*_tmp80B=({struct Cyc_List_List*_tmp582=_cycalloc(sizeof(*_tmp582));_tmp582->hd=_tmp581,_tmp582->tl=noconsume_roots;_tmp582;});noconsume_roots=_tmp80B;});}
# 2987
goto _LL8;}default: _LLF: _LL10:
 goto _LL8;}_LL8:;}
# 2990
({union Cyc_CfFlowInfo_FlowInfo _tmp80C=Cyc_CfFlowInfo_ReachableFL(_tmp591,_tmp590);inflow=_tmp80C;});{
# 2992
int noreturn=Cyc_Tcutil_is_noreturn_fn_type(Cyc_Tcutil_fndecl2type(fd));
struct Cyc_Hashtable_Table*flow_table=
((struct Cyc_Hashtable_Table*(*)(int sz,int(*cmp)(struct Cyc_Absyn_Stmt*,struct Cyc_Absyn_Stmt*),int(*hash)(struct Cyc_Absyn_Stmt*)))Cyc_Hashtable_create)(33,(int(*)(struct Cyc_Absyn_Stmt*,struct Cyc_Absyn_Stmt*))Cyc_Core_ptrcmp,(int(*)(struct Cyc_Absyn_Stmt*s))Cyc_NewControlFlow_hash_ptr);
struct Cyc_NewControlFlow_AnalEnv*env=({struct Cyc_NewControlFlow_AnalEnv*_tmp58F=_cycalloc(sizeof(*_tmp58F));
_tmp58F->all_tables=tables,({
struct Cyc_Hashtable_Table*_tmp80E=((struct Cyc_Hashtable_Table*(*)(struct Cyc_Hashtable_Table*t,struct Cyc_Absyn_Fndecl*key))Cyc_Hashtable_lookup)(tables->succ_tables,fd);_tmp58F->succ_table=_tmp80E;}),({
struct Cyc_Hashtable_Table*_tmp80D=((struct Cyc_Hashtable_Table*(*)(struct Cyc_Hashtable_Table*t,struct Cyc_Absyn_Fndecl*key))Cyc_Hashtable_lookup)(tables->pat_pop_tables,fd);_tmp58F->pat_pop_table=_tmp80D;}),_tmp58F->fenv=fenv,_tmp58F->iterate_again=1,_tmp58F->iteration_num=0,_tmp58F->in_try=0,_tmp58F->tryflow=inflow,_tmp58F->noreturn=noreturn,_tmp58F->return_type=fd->ret_type,_tmp58F->unique_pat_vars=0,_tmp58F->param_roots=param_roots,_tmp58F->noconsume_params=noconsume_roots,_tmp58F->flow_table=flow_table,_tmp58F->return_relns=fd->ensures_relns;_tmp58F;});
# 3002
union Cyc_CfFlowInfo_FlowInfo outflow=inflow;
while(env->iterate_again  && !Cyc_CfFlowInfo_anal_error){
++ env->iteration_num;
# 3008
env->iterate_again=0;
({union Cyc_CfFlowInfo_FlowInfo _tmp80F=Cyc_NewControlFlow_anal_stmt(env,inflow,fd->body,0);outflow=_tmp80F;});}{
# 3011
union Cyc_CfFlowInfo_FlowInfo _tmp588=outflow;if((_tmp588.BottomFL).tag == 1){_LL12: _LL13:
 goto _LL11;}else{_LL14: _LL15:
# 3014
 Cyc_NewControlFlow_check_init_params(_tmp562,env,outflow);
Cyc_NewControlFlow_unconsume_params(env,env->noconsume_params,0,0,outflow,_tmp562);
# 3018
if(noreturn)
({void*_tmp589=0U;({unsigned int _tmp811=_tmp562;struct _dyneither_ptr _tmp810=({const char*_tmp58A="`noreturn' function might (implicitly) return";_tag_dyneither(_tmp58A,sizeof(char),46U);});Cyc_CfFlowInfo_aerr(_tmp811,_tmp810,_tag_dyneither(_tmp589,sizeof(void*),0U));});});else{
if(!Cyc_Tcutil_is_void_type(fd->ret_type)){
if(Cyc_Tcutil_is_any_float_type(fd->ret_type) || 
Cyc_Tcutil_is_any_int_type(fd->ret_type))
({void*_tmp58B=0U;({unsigned int _tmp813=_tmp562;struct _dyneither_ptr _tmp812=({const char*_tmp58C="function may not return a value";_tag_dyneither(_tmp58C,sizeof(char),32U);});Cyc_Warn_warn(_tmp813,_tmp812,_tag_dyneither(_tmp58B,sizeof(void*),0U));});});else{
# 3025
({void*_tmp58D=0U;({unsigned int _tmp815=_tmp562;struct _dyneither_ptr _tmp814=({const char*_tmp58E="function may not return a value";_tag_dyneither(_tmp58E,sizeof(char),32U);});Cyc_CfFlowInfo_aerr(_tmp815,_tmp814,_tag_dyneither(_tmp58D,sizeof(void*),0U));});});}goto _LL11;}}
# 3027
goto _LL11;}_LL11:;};};};};};}
# 3031
void Cyc_NewControlFlow_cf_check(struct Cyc_JumpAnalysis_Jump_Anal_Result*tables,struct Cyc_List_List*ds){
for(0;ds != 0;ds=ds->tl){
Cyc_CfFlowInfo_anal_error=0;{
void*_tmp593=((struct Cyc_Absyn_Decl*)ds->hd)->r;void*_tmp594=_tmp593;struct Cyc_List_List*_tmp598;struct Cyc_List_List*_tmp597;struct Cyc_List_List*_tmp596;struct Cyc_Absyn_Fndecl*_tmp595;switch(*((int*)_tmp594)){case 1U: _LL1: _tmp595=((struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct*)_tmp594)->f1;_LL2:
 Cyc_NewControlFlow_check_fun(tables,_tmp595);goto _LL0;case 11U: _LL3: _tmp596=((struct Cyc_Absyn_ExternC_d_Absyn_Raw_decl_struct*)_tmp594)->f1;_LL4:
 _tmp597=_tmp596;goto _LL6;case 10U: _LL5: _tmp597=((struct Cyc_Absyn_Using_d_Absyn_Raw_decl_struct*)_tmp594)->f2;_LL6:
 _tmp598=_tmp597;goto _LL8;case 9U: _LL7: _tmp598=((struct Cyc_Absyn_Namespace_d_Absyn_Raw_decl_struct*)_tmp594)->f2;_LL8:
 Cyc_NewControlFlow_cf_check(tables,_tmp598);goto _LL0;case 12U: _LL9: _LLA:
 goto _LL0;default: _LLB: _LLC:
 goto _LL0;}_LL0:;};}}
