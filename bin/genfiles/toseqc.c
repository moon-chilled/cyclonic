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
#define _check_null(ptr) (ptr ? : (void*)_throw_null())
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_known_subscript_null(ptr,bound,elt_sz,index)\
   ((char *)ptr) + (elt_sz)*(index))
#define _check_known_subscript_notnull(bound,index) (index)

#define _zero_arr_plus_char_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_short_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_int_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_float_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_double_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_longdouble_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_voidstar_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#else
#define _check_known_subscript_null(ptr,bound,elt_sz,index) ({ \
  void*_cks_ptr = (void*)(ptr); \
  unsigned _cks_bound = (bound); \
  unsigned _cks_elt_sz = (elt_sz); \
  unsigned _cks_index = (index); \
  if (!_cks_ptr) _throw_null(); \
  if (_cks_index >= _cks_bound) _throw_arraybounds(); \
  ((char *)_cks_ptr) + _cks_elt_sz*_cks_index; })

#define _check_known_subscript_notnull(bound,index) ({ \
  unsigned _cksnn_bound = (bound); \
  unsigned _cksnn_index = (index); \
  if (_cksnn_index >= _cksnn_bound) _throw_arraybounds(); \
  _cksnn_index; })

/* Add i to zero-terminated pointer x.  Checks for x being null and
   ensures that x[0..i-1] are not 0. */
char * _zero_arr_plus_char_fn(char *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno);
short * _zero_arr_plus_short_fn(short *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno);
int * _zero_arr_plus_int_fn(int *orig_x, unsigned int orig_sz, int orig_i, const char *filename, unsigned lineno);
float * _zero_arr_plus_float_fn(float *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno);
double * _zero_arr_plus_double_fn(double *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno);
long double * _zero_arr_plus_longdouble_fn(long double *orig_x, unsigned int orig_sz, int orig_i, const char *filename, unsigned lineno);
void * _zero_arr_plus_voidstar_fn(void **orig_x, unsigned int orig_sz, int orig_i,const char *filename,unsigned lineno);
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
#endif

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
  _tag_arr_ans.last_plus_one = _tag_arr_ans.base + (elt_sz) * (num_elts); \
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
  x.last_plus_one -= sz * numelts; 
  return x; 
}

/* Allocation */

extern void* GC_malloc(int);
extern void* GC_malloc_atomic(int);
extern void* GC_calloc(unsigned,unsigned);
extern void* GC_calloc_atomic(unsigned,unsigned);

/* FIX?  Not sure if we want to pass filename and lineno in here... */
#ifndef CYC_REGION_PROFILE
#define _cycalloc(n)            (GC_malloc(n)          ? : _throw_badalloc())
#define _cycalloc_atomic(n)     (GC_malloc_atomic(n)   ? : _throw_badalloc())
#define _cyccalloc(n,s)         (GC_calloc(n,s)        ? : _throw_badalloc())
#define _cyccalloc_atomic(n,s)  (GC_calloc_atomic(n,s) ? : _throw_badalloc())
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
 struct Cyc___cycFILE;struct Cyc_String_pa_PrintArg_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Int_pa_PrintArg_struct{int tag;unsigned long f1;};struct Cyc_Double_pa_PrintArg_struct{int tag;double f1;};struct Cyc_LongDouble_pa_PrintArg_struct{int tag;long double f1;};struct Cyc_ShortPtr_pa_PrintArg_struct{int tag;short*f1;};struct Cyc_IntPtr_pa_PrintArg_struct{int tag;unsigned long*f1;};struct Cyc_ShortPtr_sa_ScanfArg_struct{int tag;short*f1;};struct Cyc_UShortPtr_sa_ScanfArg_struct{int tag;unsigned short*f1;};struct Cyc_IntPtr_sa_ScanfArg_struct{int tag;int*f1;};struct Cyc_UIntPtr_sa_ScanfArg_struct{int tag;unsigned int*f1;};struct Cyc_StringPtr_sa_ScanfArg_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_DoublePtr_sa_ScanfArg_struct{int tag;double*f1;};struct Cyc_FloatPtr_sa_ScanfArg_struct{int tag;float*f1;};struct Cyc_CharPtr_sa_ScanfArg_struct{int tag;struct _dyneither_ptr f1;};extern char Cyc_FileCloseError[15U];struct Cyc_FileCloseError_exn_struct{char*tag;};extern char Cyc_FileOpenError[14U];struct Cyc_FileOpenError_exn_struct{char*tag;struct _dyneither_ptr f1;};struct Cyc_Core_Opt{void*v;};extern char Cyc_Core_Invalid_argument[17U];struct Cyc_Core_Invalid_argument_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Failure[8U];struct Cyc_Core_Failure_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Impossible[11U];struct Cyc_Core_Impossible_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Not_found[10U];struct Cyc_Core_Not_found_exn_struct{char*tag;};extern char Cyc_Core_Unreachable[12U];struct Cyc_Core_Unreachable_exn_struct{char*tag;struct _dyneither_ptr f1;};
# 170 "core.h"
extern struct _RegionHandle*Cyc_Core_unique_region;struct Cyc_Core_DynamicRegion;struct Cyc_Core_NewDynamicRegion{struct Cyc_Core_DynamicRegion*key;};struct Cyc_List_List{void*hd;struct Cyc_List_List*tl;};
# 54 "list.h"
struct Cyc_List_List*Cyc_List_list(struct _dyneither_ptr);
# 61
int Cyc_List_length(struct Cyc_List_List*x);extern char Cyc_List_List_mismatch[14U];struct Cyc_List_List_mismatch_exn_struct{char*tag;};
# 133
void Cyc_List_iter(void(*f)(void*),struct Cyc_List_List*x);
# 172
struct Cyc_List_List*Cyc_List_rev(struct Cyc_List_List*x);extern char Cyc_List_Nth[4U];struct Cyc_List_Nth_exn_struct{char*tag;};struct _tuple0{struct Cyc_List_List*f1;struct Cyc_List_List*f2;};
# 294
struct _tuple0 Cyc_List_split(struct Cyc_List_List*x);struct Cyc_Iter_Iter{void*env;int(*next)(void*env,void*dest);};
# 37 "iter.h"
int Cyc_Iter_next(struct Cyc_Iter_Iter,void*);struct Cyc_Dict_T;struct Cyc_Dict_Dict{int(*rel)(void*,void*);struct _RegionHandle*r;const struct Cyc_Dict_T*t;};extern char Cyc_Dict_Present[8U];struct Cyc_Dict_Present_exn_struct{char*tag;};extern char Cyc_Dict_Absent[7U];struct Cyc_Dict_Absent_exn_struct{char*tag;};
# 49 "string.h"
int Cyc_strcmp(struct _dyneither_ptr s1,struct _dyneither_ptr s2);struct Cyc_PP_Ppstate;struct Cyc_PP_Out;struct Cyc_PP_Doc;struct Cyc_Position_Error;struct Cyc_Relations_Reln;struct _union_Nmspace_Rel_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Abs_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_C_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Loc_n{int tag;int val;};union Cyc_Absyn_Nmspace{struct _union_Nmspace_Rel_n Rel_n;struct _union_Nmspace_Abs_n Abs_n;struct _union_Nmspace_C_n C_n;struct _union_Nmspace_Loc_n Loc_n;};
# 96 "absyn.h"
union Cyc_Absyn_Nmspace Cyc_Absyn_Loc_n;
union Cyc_Absyn_Nmspace Cyc_Absyn_Rel_n(struct Cyc_List_List*);
# 99
union Cyc_Absyn_Nmspace Cyc_Absyn_Abs_n(struct Cyc_List_List*ns,int C_scope);struct _tuple1{union Cyc_Absyn_Nmspace f1;struct _dyneither_ptr*f2;};
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
enum Cyc_Absyn_AggrKind{Cyc_Absyn_StructA  = 0U,Cyc_Absyn_UnionA  = 1U};struct _union_Constraint_Eq_constr{int tag;void*val;};struct _union_Constraint_Forward_constr{int tag;union Cyc_Absyn_Constraint*val;};struct _union_Constraint_No_constr{int tag;int val;};union Cyc_Absyn_Constraint{struct _union_Constraint_Eq_constr Eq_constr;struct _union_Constraint_Forward_constr Forward_constr;struct _union_Constraint_No_constr No_constr;};struct Cyc_Absyn_Eq_kb_Absyn_KindBound_struct{int tag;struct Cyc_Absyn_Kind*f1;};struct Cyc_Absyn_Unknown_kb_Absyn_KindBound_struct{int tag;struct Cyc_Core_Opt*f1;};struct Cyc_Absyn_Less_kb_Absyn_KindBound_struct{int tag;struct Cyc_Core_Opt*f1;struct Cyc_Absyn_Kind*f2;};struct Cyc_Absyn_Tvar{struct _dyneither_ptr*name;int identity;void*kind;};struct Cyc_Absyn_DynEither_b_Absyn_Bounds_struct{int tag;};struct Cyc_Absyn_Upper_b_Absyn_Bounds_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_PtrLoc{unsigned int ptr_loc;unsigned int rgn_loc;unsigned int zt_loc;};struct Cyc_Absyn_PtrAtts{void*rgn;union Cyc_Absyn_Constraint*nullable;union Cyc_Absyn_Constraint*bounds;union Cyc_Absyn_Constraint*zero_term;struct Cyc_Absyn_PtrLoc*ptrloc;};struct Cyc_Absyn_PtrInfo{void*elt_typ;struct Cyc_Absyn_Tqual elt_tq;struct Cyc_Absyn_PtrAtts ptr_atts;};struct Cyc_Absyn_VarargInfo{struct _dyneither_ptr*name;struct Cyc_Absyn_Tqual tq;void*type;int inject;};struct Cyc_Absyn_FnInfo{struct Cyc_List_List*tvars;void*effect;struct Cyc_Absyn_Tqual ret_tqual;void*ret_typ;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;struct Cyc_List_List*requires_relns;struct Cyc_Absyn_Exp*ensures_clause;struct Cyc_List_List*ensures_relns;};struct Cyc_Absyn_UnknownDatatypeInfo{struct _tuple1*name;int is_extensible;};struct _union_DatatypeInfoU_UnknownDatatype{int tag;struct Cyc_Absyn_UnknownDatatypeInfo val;};struct _union_DatatypeInfoU_KnownDatatype{int tag;struct Cyc_Absyn_Datatypedecl**val;};union Cyc_Absyn_DatatypeInfoU{struct _union_DatatypeInfoU_UnknownDatatype UnknownDatatype;struct _union_DatatypeInfoU_KnownDatatype KnownDatatype;};struct Cyc_Absyn_DatatypeInfo{union Cyc_Absyn_DatatypeInfoU datatype_info;struct Cyc_List_List*targs;};struct Cyc_Absyn_UnknownDatatypeFieldInfo{struct _tuple1*datatype_name;struct _tuple1*field_name;int is_extensible;};struct _union_DatatypeFieldInfoU_UnknownDatatypefield{int tag;struct Cyc_Absyn_UnknownDatatypeFieldInfo val;};struct _tuple2{struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;};struct _union_DatatypeFieldInfoU_KnownDatatypefield{int tag;struct _tuple2 val;};union Cyc_Absyn_DatatypeFieldInfoU{struct _union_DatatypeFieldInfoU_UnknownDatatypefield UnknownDatatypefield;struct _union_DatatypeFieldInfoU_KnownDatatypefield KnownDatatypefield;};struct Cyc_Absyn_DatatypeFieldInfo{union Cyc_Absyn_DatatypeFieldInfoU field_info;struct Cyc_List_List*targs;};struct _tuple3{enum Cyc_Absyn_AggrKind f1;struct _tuple1*f2;struct Cyc_Core_Opt*f3;};struct _union_AggrInfoU_UnknownAggr{int tag;struct _tuple3 val;};struct _union_AggrInfoU_KnownAggr{int tag;struct Cyc_Absyn_Aggrdecl**val;};union Cyc_Absyn_AggrInfoU{struct _union_AggrInfoU_UnknownAggr UnknownAggr;struct _union_AggrInfoU_KnownAggr KnownAggr;};struct Cyc_Absyn_AggrInfo{union Cyc_Absyn_AggrInfoU aggr_info;struct Cyc_List_List*targs;};struct Cyc_Absyn_ArrayInfo{void*elt_type;struct Cyc_Absyn_Tqual tq;struct Cyc_Absyn_Exp*num_elts;union Cyc_Absyn_Constraint*zero_term;unsigned int zt_loc;};struct Cyc_Absyn_Aggr_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Absyn_Enum_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;};struct Cyc_Absyn_Datatype_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;};struct Cyc_Absyn_TypeDecl{void*r;unsigned int loc;};struct Cyc_Absyn_VoidType_Absyn_Type_struct{int tag;};struct Cyc_Absyn_Evar_Absyn_Type_struct{int tag;struct Cyc_Core_Opt*f1;void*f2;int f3;struct Cyc_Core_Opt*f4;};struct Cyc_Absyn_VarType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Tvar*f1;};struct Cyc_Absyn_DatatypeType_Absyn_Type_struct{int tag;struct Cyc_Absyn_DatatypeInfo f1;};struct Cyc_Absyn_DatatypeFieldType_Absyn_Type_struct{int tag;struct Cyc_Absyn_DatatypeFieldInfo f1;};struct Cyc_Absyn_PointerType_Absyn_Type_struct{int tag;struct Cyc_Absyn_PtrInfo f1;};struct Cyc_Absyn_IntType_Absyn_Type_struct{int tag;enum Cyc_Absyn_Sign f1;enum Cyc_Absyn_Size_of f2;};struct Cyc_Absyn_FloatType_Absyn_Type_struct{int tag;int f1;};struct Cyc_Absyn_ArrayType_Absyn_Type_struct{int tag;struct Cyc_Absyn_ArrayInfo f1;};struct Cyc_Absyn_FnType_Absyn_Type_struct{int tag;struct Cyc_Absyn_FnInfo f1;};struct Cyc_Absyn_TupleType_Absyn_Type_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_AggrType_Absyn_Type_struct{int tag;struct Cyc_Absyn_AggrInfo f1;};struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct{int tag;enum Cyc_Absyn_AggrKind f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_EnumType_Absyn_Type_struct{int tag;struct _tuple1*f1;struct Cyc_Absyn_Enumdecl*f2;};struct Cyc_Absyn_AnonEnumType_Absyn_Type_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_RgnHandleType_Absyn_Type_struct{int tag;void*f1;};struct Cyc_Absyn_DynRgnType_Absyn_Type_struct{int tag;void*f1;void*f2;};struct Cyc_Absyn_TypedefType_Absyn_Type_struct{int tag;struct _tuple1*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_Typedefdecl*f3;void*f4;};struct Cyc_Absyn_ValueofType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_TagType_Absyn_Type_struct{int tag;void*f1;};struct Cyc_Absyn_HeapRgn_Absyn_Type_struct{int tag;};struct Cyc_Absyn_UniqueRgn_Absyn_Type_struct{int tag;};struct Cyc_Absyn_RefCntRgn_Absyn_Type_struct{int tag;};struct Cyc_Absyn_AccessEff_Absyn_Type_struct{int tag;void*f1;};struct Cyc_Absyn_JoinEff_Absyn_Type_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_RgnsEff_Absyn_Type_struct{int tag;void*f1;};struct Cyc_Absyn_TypeDeclType_Absyn_Type_struct{int tag;struct Cyc_Absyn_TypeDecl*f1;void**f2;};struct Cyc_Absyn_TypeofType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_BuiltinType_Absyn_Type_struct{int tag;struct _dyneither_ptr f1;struct Cyc_Absyn_Kind*f2;};struct Cyc_Absyn_NoTypes_Absyn_Funcparams_struct{int tag;struct Cyc_List_List*f1;unsigned int f2;};struct Cyc_Absyn_WithTypes_Absyn_Funcparams_struct{int tag;struct Cyc_List_List*f1;int f2;struct Cyc_Absyn_VarargInfo*f3;void*f4;struct Cyc_List_List*f5;struct Cyc_Absyn_Exp*f6;struct Cyc_Absyn_Exp*f7;};
# 446 "absyn.h"
enum Cyc_Absyn_Format_Type{Cyc_Absyn_Printf_ft  = 0U,Cyc_Absyn_Scanf_ft  = 1U};struct Cyc_Absyn_Regparm_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Stdcall_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Cdecl_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Fastcall_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Noreturn_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Const_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Aligned_att_Absyn_Attribute_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Packed_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Section_att_Absyn_Attribute_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Nocommon_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Shared_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Unused_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Weak_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Dllimport_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Dllexport_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_No_instrument_function_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Constructor_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Destructor_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_No_check_memory_usage_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Format_att_Absyn_Attribute_struct{int tag;enum Cyc_Absyn_Format_Type f1;int f2;int f3;};struct Cyc_Absyn_Initializes_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Noliveunique_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Noconsume_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Pure_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Mode_att_Absyn_Attribute_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Alias_att_Absyn_Attribute_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Always_inline_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Carray_mod_Absyn_Type_modifier_struct{int tag;union Cyc_Absyn_Constraint*f1;unsigned int f2;};struct Cyc_Absyn_ConstArray_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_Absyn_Exp*f1;union Cyc_Absyn_Constraint*f2;unsigned int f3;};struct Cyc_Absyn_Pointer_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_Absyn_PtrAtts f1;struct Cyc_Absyn_Tqual f2;};struct Cyc_Absyn_Function_mod_Absyn_Type_modifier_struct{int tag;void*f1;};struct Cyc_Absyn_TypeParams_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_List_List*f1;unsigned int f2;int f3;};struct Cyc_Absyn_Attributes_mod_Absyn_Type_modifier_struct{int tag;unsigned int f1;struct Cyc_List_List*f2;};struct _union_Cnst_Null_c{int tag;int val;};struct _tuple4{enum Cyc_Absyn_Sign f1;char f2;};struct _union_Cnst_Char_c{int tag;struct _tuple4 val;};struct _union_Cnst_Wchar_c{int tag;struct _dyneither_ptr val;};struct _tuple5{enum Cyc_Absyn_Sign f1;short f2;};struct _union_Cnst_Short_c{int tag;struct _tuple5 val;};struct _tuple6{enum Cyc_Absyn_Sign f1;int f2;};struct _union_Cnst_Int_c{int tag;struct _tuple6 val;};struct _tuple7{enum Cyc_Absyn_Sign f1;long long f2;};struct _union_Cnst_LongLong_c{int tag;struct _tuple7 val;};struct _tuple8{struct _dyneither_ptr f1;int f2;};struct _union_Cnst_Float_c{int tag;struct _tuple8 val;};struct _union_Cnst_String_c{int tag;struct _dyneither_ptr val;};struct _union_Cnst_Wstring_c{int tag;struct _dyneither_ptr val;};union Cyc_Absyn_Cnst{struct _union_Cnst_Null_c Null_c;struct _union_Cnst_Char_c Char_c;struct _union_Cnst_Wchar_c Wchar_c;struct _union_Cnst_Short_c Short_c;struct _union_Cnst_Int_c Int_c;struct _union_Cnst_LongLong_c LongLong_c;struct _union_Cnst_Float_c Float_c;struct _union_Cnst_String_c String_c;struct _union_Cnst_Wstring_c Wstring_c;};
# 536
enum Cyc_Absyn_Primop{Cyc_Absyn_Plus  = 0U,Cyc_Absyn_Times  = 1U,Cyc_Absyn_Minus  = 2U,Cyc_Absyn_Div  = 3U,Cyc_Absyn_Mod  = 4U,Cyc_Absyn_Eq  = 5U,Cyc_Absyn_Neq  = 6U,Cyc_Absyn_Gt  = 7U,Cyc_Absyn_Lt  = 8U,Cyc_Absyn_Gte  = 9U,Cyc_Absyn_Lte  = 10U,Cyc_Absyn_Not  = 11U,Cyc_Absyn_Bitnot  = 12U,Cyc_Absyn_Bitand  = 13U,Cyc_Absyn_Bitor  = 14U,Cyc_Absyn_Bitxor  = 15U,Cyc_Absyn_Bitlshift  = 16U,Cyc_Absyn_Bitlrshift  = 17U,Cyc_Absyn_Bitarshift  = 18U,Cyc_Absyn_Numelts  = 19U};
# 543
enum Cyc_Absyn_Incrementor{Cyc_Absyn_PreInc  = 0U,Cyc_Absyn_PostInc  = 1U,Cyc_Absyn_PreDec  = 2U,Cyc_Absyn_PostDec  = 3U};struct Cyc_Absyn_VarargCallInfo{int num_varargs;struct Cyc_List_List*injectors;struct Cyc_Absyn_VarargInfo*vai;};struct Cyc_Absyn_StructField_Absyn_OffsetofField_struct{int tag;struct _dyneither_ptr*f1;};struct Cyc_Absyn_TupleIndex_Absyn_OffsetofField_struct{int tag;unsigned int f1;};
# 561
enum Cyc_Absyn_Coercion{Cyc_Absyn_Unknown_coercion  = 0U,Cyc_Absyn_No_coercion  = 1U,Cyc_Absyn_Null_to_NonNull  = 2U,Cyc_Absyn_Other_coercion  = 3U};struct Cyc_Absyn_MallocInfo{int is_calloc;struct Cyc_Absyn_Exp*rgn;void**elt_type;struct Cyc_Absyn_Exp*num_elts;int fat_result;int inline_call;};struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct{int tag;union Cyc_Absyn_Cnst f1;};struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Pragma_e_Absyn_Raw_exp_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct{int tag;enum Cyc_Absyn_Primop f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Increment_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;enum Cyc_Absyn_Incrementor f2;};struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_VarargCallInfo*f3;int f4;};struct Cyc_Absyn_Throw_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;int f2;};struct Cyc_Absyn_NoInstantiate_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Instantiate_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_Absyn_Exp*f2;int f3;enum Cyc_Absyn_Coercion f4;};struct Cyc_Absyn_Address_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_New_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Sizeoftyp_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Sizeofexp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Offsetof_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;int f3;int f4;};struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;int f3;int f4;};struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Tuple_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;};struct _tuple9{struct _dyneither_ptr*f1;struct Cyc_Absyn_Tqual f2;void*f3;};struct Cyc_Absyn_CompoundLit_e_Absyn_Raw_exp_struct{int tag;struct _tuple9*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Array_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Comprehension_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;int f4;};struct Cyc_Absyn_ComprehensionNoinit_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;void*f2;int f3;};struct Cyc_Absyn_Aggregate_e_Absyn_Raw_exp_struct{int tag;struct _tuple1*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;struct Cyc_Absyn_Aggrdecl*f4;};struct Cyc_Absyn_AnonStruct_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Datatype_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Datatypedecl*f2;struct Cyc_Absyn_Datatypefield*f3;};struct Cyc_Absyn_Enum_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_AnonEnum_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_MallocInfo f1;};struct Cyc_Absyn_Swap_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Core_Opt*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_StmtExp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Tagcheck_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;};struct Cyc_Absyn_Valueof_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Asm_e_Absyn_Raw_exp_struct{int tag;int f1;struct _dyneither_ptr f2;};struct Cyc_Absyn_Exp{void*topt;void*r;unsigned int loc;void*annot;};struct Cyc_Absyn_Skip_s_Absyn_Raw_stmt_struct{int tag;};struct Cyc_Absyn_Exp_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Return_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;struct Cyc_Absyn_Stmt*f3;};struct _tuple10{struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_While_s_Absyn_Raw_stmt_struct{int tag;struct _tuple10 f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Break_s_Absyn_Raw_stmt_struct{int tag;};struct Cyc_Absyn_Continue_s_Absyn_Raw_stmt_struct{int tag;};struct Cyc_Absyn_Goto_s_Absyn_Raw_stmt_struct{int tag;struct _dyneither_ptr*f1;};struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _tuple10 f2;struct _tuple10 f3;struct Cyc_Absyn_Stmt*f4;};struct Cyc_Absyn_Switch_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;void*f3;};struct Cyc_Absyn_Fallthru_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Switch_clause**f2;};struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Decl*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Label_s_Absyn_Raw_stmt_struct{int tag;struct _dyneither_ptr*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Do_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct _tuple10 f2;};struct Cyc_Absyn_TryCatch_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_List_List*f2;void*f3;};struct Cyc_Absyn_Stmt{void*r;unsigned int loc;void*annot;};struct Cyc_Absyn_Wild_p_Absyn_Raw_pat_struct{int tag;};struct Cyc_Absyn_Var_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_AliasVar_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;};struct Cyc_Absyn_Reference_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_TagInt_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;};struct Cyc_Absyn_Tuple_p_Absyn_Raw_pat_struct{int tag;struct Cyc_List_List*f1;int f2;};struct Cyc_Absyn_Pointer_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Pat*f1;};struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_AggrInfo*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;int f4;};struct Cyc_Absyn_Datatype_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;struct Cyc_List_List*f3;int f4;};struct Cyc_Absyn_Null_p_Absyn_Raw_pat_struct{int tag;};struct Cyc_Absyn_Int_p_Absyn_Raw_pat_struct{int tag;enum Cyc_Absyn_Sign f1;int f2;};struct Cyc_Absyn_Char_p_Absyn_Raw_pat_struct{int tag;char f1;};struct Cyc_Absyn_Float_p_Absyn_Raw_pat_struct{int tag;struct _dyneither_ptr f1;int f2;};struct Cyc_Absyn_Enum_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_AnonEnum_p_Absyn_Raw_pat_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_UnknownId_p_Absyn_Raw_pat_struct{int tag;struct _tuple1*f1;};struct Cyc_Absyn_UnknownCall_p_Absyn_Raw_pat_struct{int tag;struct _tuple1*f1;struct Cyc_List_List*f2;int f3;};struct Cyc_Absyn_Exp_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Pat{void*r;void*topt;unsigned int loc;};struct Cyc_Absyn_Switch_clause{struct Cyc_Absyn_Pat*pattern;struct Cyc_Core_Opt*pat_vars;struct Cyc_Absyn_Exp*where_clause;struct Cyc_Absyn_Stmt*body;unsigned int loc;};struct Cyc_Absyn_Unresolved_b_Absyn_Binding_struct{int tag;struct _tuple1*f1;};struct Cyc_Absyn_Global_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Funname_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Param_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Local_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Pat_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Vardecl{enum Cyc_Absyn_Scope sc;struct _tuple1*name;unsigned int varloc;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*initializer;void*rgn;struct Cyc_List_List*attributes;int escapes;};struct Cyc_Absyn_Fndecl{enum Cyc_Absyn_Scope sc;int is_inline;struct _tuple1*name;struct Cyc_List_List*tvs;void*effect;struct Cyc_Absyn_Tqual ret_tqual;void*ret_type;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_Absyn_Stmt*body;void*cached_typ;struct Cyc_Core_Opt*param_vardecls;struct Cyc_Absyn_Vardecl*fn_vardecl;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;struct Cyc_List_List*requires_relns;struct Cyc_Absyn_Exp*ensures_clause;struct Cyc_List_List*ensures_relns;};struct Cyc_Absyn_Aggrfield{struct _dyneither_ptr*name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*width;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;};struct Cyc_Absyn_AggrdeclImpl{struct Cyc_List_List*exist_vars;struct Cyc_List_List*rgn_po;struct Cyc_List_List*fields;int tagged;};struct Cyc_Absyn_Aggrdecl{enum Cyc_Absyn_AggrKind kind;enum Cyc_Absyn_Scope sc;struct _tuple1*name;struct Cyc_List_List*tvs;struct Cyc_Absyn_AggrdeclImpl*impl;struct Cyc_List_List*attributes;int expected_mem_kind;};struct Cyc_Absyn_Datatypefield{struct _tuple1*name;struct Cyc_List_List*typs;unsigned int loc;enum Cyc_Absyn_Scope sc;};struct Cyc_Absyn_Datatypedecl{enum Cyc_Absyn_Scope sc;struct _tuple1*name;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*fields;int is_extensible;};struct Cyc_Absyn_Enumfield{struct _tuple1*name;struct Cyc_Absyn_Exp*tag;unsigned int loc;};struct Cyc_Absyn_Enumdecl{enum Cyc_Absyn_Scope sc;struct _tuple1*name;struct Cyc_Core_Opt*fields;};struct Cyc_Absyn_Typedefdecl{struct _tuple1*name;struct Cyc_Absyn_Tqual tq;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*kind;void*defn;struct Cyc_List_List*atts;int extern_c;};struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Let_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Pat*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;void*f4;};struct Cyc_Absyn_Letv_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Region_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Absyn_Datatype_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;};struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;};struct Cyc_Absyn_Typedef_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Typedefdecl*f1;};struct Cyc_Absyn_Namespace_d_Absyn_Raw_decl_struct{int tag;struct _dyneither_ptr*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Using_d_Absyn_Raw_decl_struct{int tag;struct _tuple1*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_ExternC_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_ExternCinclude_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Porton_d_Absyn_Raw_decl_struct{int tag;};struct Cyc_Absyn_Portoff_d_Absyn_Raw_decl_struct{int tag;};struct Cyc_Absyn_Decl{void*r;unsigned int loc;};struct Cyc_Absyn_ArrayElement_Absyn_Designator_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_FieldName_Absyn_Designator_struct{int tag;struct _dyneither_ptr*f1;};extern char Cyc_Absyn_EmptyAnnot[11U];struct Cyc_Absyn_EmptyAnnot_Absyn_AbsynAnnot_struct{char*tag;};
# 1004 "absyn.h"
struct Cyc_Absyn_Exp*Cyc_Absyn_new_exp(void*,unsigned int);
# 1006
struct Cyc_Absyn_Exp*Cyc_Absyn_copy_exp(struct Cyc_Absyn_Exp*);
# 1020
struct Cyc_Absyn_Exp*Cyc_Absyn_var_exp(struct _tuple1*,unsigned int);
# 1064
struct Cyc_Absyn_Exp*Cyc_Absyn_stmt_exp(struct Cyc_Absyn_Stmt*,unsigned int);
# 1078
struct Cyc_Absyn_Stmt*Cyc_Absyn_exp_stmt(struct Cyc_Absyn_Exp*e,unsigned int loc);
# 1090
struct Cyc_Absyn_Stmt*Cyc_Absyn_declare_stmt(struct _tuple1*,void*,struct Cyc_Absyn_Exp*init,struct Cyc_Absyn_Stmt*,unsigned int loc);struct Cyc_Absynpp_Params{int expand_typedefs;int qvar_to_Cids;int add_cyc_prefix;int to_VC;int decls_first;int rewrite_temp_tvars;int print_all_tvars;int print_all_kinds;int print_all_effects;int print_using_stmts;int print_externC_stmts;int print_full_evars;int print_zeroterm;int generate_line_directives;int use_curr_namespace;struct Cyc_List_List*curr_namespace;};
# 67 "absynpp.h"
struct _dyneither_ptr Cyc_Absynpp_exp2string(struct Cyc_Absyn_Exp*);
struct _dyneither_ptr Cyc_Absynpp_stmt2string(struct Cyc_Absyn_Stmt*);
struct _dyneither_ptr Cyc_Absynpp_qvar2string(struct _tuple1*);
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
void*Cyc_Warn_impos(struct _dyneither_ptr fmt,struct _dyneither_ptr ap);struct Cyc_Set_Set;extern char Cyc_Set_Absent[7U];struct Cyc_Set_Absent_exn_struct{char*tag;};struct Cyc_RgnOrder_RgnPO;
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
# 313 "tcutil.h"
int Cyc_Tcutil_is_const_exp(struct Cyc_Absyn_Exp*e);struct Cyc_Hashtable_Table;
# 38 "toc.h"
void*Cyc_Toc_typ_to_c(void*);
# 40
struct _tuple1*Cyc_Toc_temp_var();extern char Cyc_Toc_Dest[5U];struct Cyc_Toc_Dest_Absyn_AbsynAnnot_struct{char*tag;struct Cyc_Absyn_Exp*f1;};
# 49
void Cyc_Toc_finish();
# 27 "toseqc.h"
struct Cyc_List_List*Cyc_Toseqc_toseqc(struct Cyc_List_List*decls);
# 35 "toseqc.cyc"
enum Cyc_Toseqc_SideEffect{Cyc_Toseqc_Const  = 0U,Cyc_Toseqc_NoEffect  = 1U,Cyc_Toseqc_ExnEffect  = 2U,Cyc_Toseqc_AnyEffect  = 3U};
# 39
static struct _dyneither_ptr Cyc_Toseqc_eff_to_string(enum Cyc_Toseqc_SideEffect e){
enum Cyc_Toseqc_SideEffect _tmp0=e;switch(_tmp0){case Cyc_Toseqc_Const: _LL1: _LL2:
 return({const char*_tmp1="Const";_tag_dyneither(_tmp1,sizeof(char),6U);});case Cyc_Toseqc_NoEffect: _LL3: _LL4:
 return({const char*_tmp2="NoEffect";_tag_dyneither(_tmp2,sizeof(char),9U);});case Cyc_Toseqc_ExnEffect: _LL5: _LL6:
 return({const char*_tmp3="ExnEffect";_tag_dyneither(_tmp3,sizeof(char),10U);});default: _LL7: _LL8:
 return({const char*_tmp4="AnyEffect";_tag_dyneither(_tmp4,sizeof(char),10U);});}_LL0:;}struct _tuple11{enum Cyc_Toseqc_SideEffect f1;enum Cyc_Toseqc_SideEffect f2;};
# 48
static enum Cyc_Toseqc_SideEffect Cyc_Toseqc_join_side_effect(enum Cyc_Toseqc_SideEffect e1,enum Cyc_Toseqc_SideEffect e2){
struct _tuple11 _tmp5=({struct _tuple11 _tmpCC;_tmpCC.f1=e1,_tmpCC.f2=e2;_tmpCC;});struct _tuple11 _tmp6=_tmp5;if(_tmp6.f1 == Cyc_Toseqc_AnyEffect){_LL1: _LL2:
 return Cyc_Toseqc_AnyEffect;}else{if(_tmp6.f2 == Cyc_Toseqc_AnyEffect){_LL3: _LL4:
 return Cyc_Toseqc_AnyEffect;}else{if(_tmp6.f1 == Cyc_Toseqc_ExnEffect){if(_tmp6.f2 == Cyc_Toseqc_ExnEffect){_LL5: _LL6:
# 53
 return Cyc_Toseqc_AnyEffect;}else{_LL7: _LL8:
 return Cyc_Toseqc_ExnEffect;}}else{if(_tmp6.f2 == Cyc_Toseqc_ExnEffect){_LL9: _LLA:
 return Cyc_Toseqc_ExnEffect;}else{if(_tmp6.f1 == Cyc_Toseqc_NoEffect){_LLB: _LLC:
 return Cyc_Toseqc_NoEffect;}else{if(_tmp6.f2 == Cyc_Toseqc_NoEffect){_LLD: _LLE:
 return Cyc_Toseqc_NoEffect;}else{_LLF: _LL10:
 return Cyc_Toseqc_Const;}}}}}}_LL0:;}static char _tmp7[20U]="_get_dyneither_size";static char _tmp8[24U]="_get_zero_arr_size_char";static char _tmp9[25U]="_get_zero_arr_size_short";static char _tmpA[23U]="_get_zero_arr_size_int";static char _tmpB[25U]="_get_zero_arr_size_float";static char _tmpC[26U]="_get_zero_arr_size_double";static char _tmpD[30U]="_get_zero_arr_size_longdouble";static char _tmpE[28U]="_get_zero_arr_size_voidstar";
# 63
static struct _dyneither_ptr Cyc_Toseqc_pure_funs[8U]={{_tmp7,_tmp7,_tmp7 + 20U},{_tmp8,_tmp8,_tmp8 + 24U},{_tmp9,_tmp9,_tmp9 + 25U},{_tmpA,_tmpA,_tmpA + 23U},{_tmpB,_tmpB,_tmpB + 25U},{_tmpC,_tmpC,_tmpC + 26U},{_tmpD,_tmpD,_tmpD + 30U},{_tmpE,_tmpE,_tmpE + 28U}};static char _tmpF[12U]="_check_null";static char _tmp10[28U]="_check_known_subscript_null";static char _tmp11[31U]="_check_known_subscript_notnull";static char _tmp12[27U]="_check_dyneither_subscript";static char _tmp13[21U]="_untag_dyneither_ptr";static char _tmp14[15U]="_region_malloc";
# 74
static struct _dyneither_ptr Cyc_Toseqc_exn_effect_funs[6U]={{_tmpF,_tmpF,_tmpF + 12U},{_tmp10,_tmp10,_tmp10 + 28U},{_tmp11,_tmp11,_tmp11 + 31U},{_tmp12,_tmp12,_tmp12 + 27U},{_tmp13,_tmp13,_tmp13 + 21U},{_tmp14,_tmp14,_tmp14 + 15U}};
# 82
static enum Cyc_Toseqc_SideEffect Cyc_Toseqc_fun_effect(struct _dyneither_ptr fn){
{int i=0;for(0;i < 8U;++ i){
if(!Cyc_strcmp((struct _dyneither_ptr)Cyc_Toseqc_pure_funs[i],(struct _dyneither_ptr)fn))return Cyc_Toseqc_NoEffect;}}
# 86
{int i=0;for(0;i < 6U;++ i){
if(!Cyc_strcmp((struct _dyneither_ptr)Cyc_Toseqc_exn_effect_funs[i],(struct _dyneither_ptr)fn))return Cyc_Toseqc_ExnEffect;}}
# 89
return Cyc_Toseqc_AnyEffect;}
# 92
static enum Cyc_Toseqc_SideEffect Cyc_Toseqc_exp_effect(struct Cyc_Absyn_Exp*e);
static enum Cyc_Toseqc_SideEffect Cyc_Toseqc_stmt_effect(struct Cyc_Absyn_Stmt*s);
static enum Cyc_Toseqc_SideEffect Cyc_Toseqc_exps_effect(struct Cyc_List_List*es){
enum Cyc_Toseqc_SideEffect res=Cyc_Toseqc_Const;
# 98
{struct Cyc_List_List*x=es;for(0;x != 0;x=x->tl){
enum Cyc_Toseqc_SideEffect _tmp15=Cyc_Toseqc_exp_effect((struct Cyc_Absyn_Exp*)x->hd);
({enum Cyc_Toseqc_SideEffect _tmpD3=Cyc_Toseqc_join_side_effect(_tmp15,res);res=_tmpD3;});
# 103
if(res == Cyc_Toseqc_AnyEffect)
# 105
return res;}}
# 109
return res;}
# 113
static enum Cyc_Toseqc_SideEffect Cyc_Toseqc_exp_effect(struct Cyc_Absyn_Exp*e){
struct _dyneither_ptr bad_form=({const char*_tmp4C="";_tag_dyneither(_tmp4C,sizeof(char),1U);});
{void*_tmp16=e->r;void*_tmp17=_tmp16;struct Cyc_Absyn_Stmt*_tmp47;struct Cyc_Absyn_Exp*_tmp46;struct Cyc_Absyn_Exp*_tmp45;struct Cyc_Absyn_Exp*_tmp44;struct Cyc_Absyn_Exp*_tmp43;struct Cyc_Absyn_Exp*_tmp42;struct Cyc_Absyn_Exp*_tmp41;struct Cyc_Absyn_Exp*_tmp40;struct Cyc_Absyn_Exp*_tmp3F;struct Cyc_Absyn_Exp*_tmp3E;struct Cyc_Absyn_Exp*_tmp3D;struct Cyc_Absyn_Exp*_tmp3C;struct Cyc_Absyn_Exp*_tmp3B;struct Cyc_Absyn_Exp*_tmp3A;struct Cyc_Absyn_Exp*_tmp39;struct Cyc_Absyn_Exp*_tmp38;struct Cyc_Absyn_Exp*_tmp37;struct Cyc_Absyn_Exp*_tmp36;struct Cyc_Absyn_Exp*_tmp35;struct Cyc_Absyn_Exp*_tmp34;struct Cyc_List_List*_tmp33;struct Cyc_List_List*_tmp32;struct Cyc_Absyn_Exp*_tmp31;switch(*((int*)_tmp17)){case 0U: _LL1: _LL2:
 goto _LL4;case 18U: _LL3: _LL4:
 goto _LL6;case 17U: _LL5: _LL6:
 goto _LL8;case 19U: _LL7: _LL8:
 goto _LLA;case 33U: _LL9: _LLA:
 goto _LLC;case 32U: _LLB: _LLC:
 return Cyc_Toseqc_Const;case 1U: _LLD: _LLE:
 return Cyc_Toseqc_NoEffect;case 4U: _LLF: _LL10:
# 124
 goto _LL12;case 40U: _LL11: _LL12:
 return Cyc_Toseqc_AnyEffect;case 10U: _LL13: _tmp31=((struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_tmp17)->f1;_LL14: {
# 128
void*_tmp18=_tmp31->r;void*_tmp19=_tmp18;struct _tuple1*_tmp1C;struct Cyc_Absyn_Vardecl*_tmp1B;struct Cyc_Absyn_Fndecl*_tmp1A;if(((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp19)->tag == 1U)switch(*((int*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp19)->f1)){case 2U: _LL54: _tmp1A=((struct Cyc_Absyn_Funname_b_Absyn_Binding_struct*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp19)->f1)->f1;_LL55:
 return Cyc_Toseqc_fun_effect(Cyc_Absynpp_qvar2string(_tmp1A->name));case 1U: _LL56: _tmp1B=((struct Cyc_Absyn_Global_b_Absyn_Binding_struct*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp19)->f1)->f1;_LL57:
 return Cyc_Toseqc_fun_effect(Cyc_Absynpp_qvar2string(_tmp1B->name));case 0U: _LL58: _tmp1C=((struct Cyc_Absyn_Unresolved_b_Absyn_Binding_struct*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp19)->f1)->f1;_LL59:
 return Cyc_Toseqc_fun_effect(Cyc_Absynpp_qvar2string(_tmp1C));default: goto _LL5A;}else{_LL5A: _LL5B:
# 133
 return Cyc_Toseqc_AnyEffect;}_LL53:;}case 36U: _LL15: _tmp32=((struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct*)_tmp17)->f2;_LL16: {
# 137
struct _tuple0 _tmp1D=((struct _tuple0(*)(struct Cyc_List_List*x))Cyc_List_split)(_tmp32);struct _tuple0 _tmp1E=_tmp1D;struct Cyc_List_List*_tmp1F;_LL5D: _tmp1F=_tmp1E.f2;_LL5E:;
_tmp33=_tmp1F;goto _LL18;}case 3U: _LL17: _tmp33=((struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_tmp17)->f2;_LL18:
# 141
 return Cyc_Toseqc_exps_effect(_tmp33);case 6U: _LL19: _tmp36=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmp17)->f1;_tmp35=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmp17)->f2;_tmp34=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmp17)->f3;_LL1A:
# 144
 return Cyc_Toseqc_exps_effect(({struct Cyc_Absyn_Exp*_tmp20[3U];_tmp20[0]=_tmp36,_tmp20[1]=_tmp35,_tmp20[2]=_tmp34;((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp20,sizeof(struct Cyc_Absyn_Exp*),3U));}));case 23U: _LL1B: _tmp38=((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmp17)->f1;_tmp37=((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmp17)->f2;_LL1C:
# 146
 _tmp3A=_tmp38;_tmp39=_tmp37;goto _LL1E;case 7U: _LL1D: _tmp3A=((struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct*)_tmp17)->f1;_tmp39=((struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct*)_tmp17)->f2;_LL1E:
 _tmp3C=_tmp3A;_tmp3B=_tmp39;goto _LL20;case 8U: _LL1F: _tmp3C=((struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct*)_tmp17)->f1;_tmp3B=((struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct*)_tmp17)->f2;_LL20:
 _tmp3E=_tmp3C;_tmp3D=_tmp3B;goto _LL22;case 9U: _LL21: _tmp3E=((struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct*)_tmp17)->f1;_tmp3D=((struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct*)_tmp17)->f2;_LL22:
# 150
 return Cyc_Toseqc_exps_effect(({struct Cyc_Absyn_Exp*_tmp21[2U];_tmp21[0]=_tmp3E,_tmp21[1]=_tmp3D;((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp21,sizeof(struct Cyc_Absyn_Exp*),2U));}));case 12U: _LL23: _tmp3F=((struct Cyc_Absyn_NoInstantiate_e_Absyn_Raw_exp_struct*)_tmp17)->f1;_LL24:
# 152
 _tmp40=_tmp3F;goto _LL26;case 13U: _LL25: _tmp40=((struct Cyc_Absyn_Instantiate_e_Absyn_Raw_exp_struct*)_tmp17)->f1;_LL26:
 _tmp41=_tmp40;goto _LL28;case 14U: _LL27: _tmp41=((struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_tmp17)->f2;_LL28:
 _tmp42=_tmp41;goto _LL2A;case 15U: _LL29: _tmp42=((struct Cyc_Absyn_Address_e_Absyn_Raw_exp_struct*)_tmp17)->f1;_LL2A:
 _tmp43=_tmp42;goto _LL2C;case 20U: _LL2B: _tmp43=((struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct*)_tmp17)->f1;_LL2C:
 _tmp44=_tmp43;goto _LL2E;case 21U: _LL2D: _tmp44=((struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_tmp17)->f1;_LL2E:
 _tmp45=_tmp44;goto _LL30;case 22U: _LL2F: _tmp45=((struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_tmp17)->f1;_LL30:
 _tmp46=_tmp45;goto _LL32;case 5U: _LL31: _tmp46=((struct Cyc_Absyn_Increment_e_Absyn_Raw_exp_struct*)_tmp17)->f1;_LL32:
# 160
 return Cyc_Toseqc_exp_effect(_tmp46);case 37U: _LL33: _tmp47=((struct Cyc_Absyn_StmtExp_e_Absyn_Raw_exp_struct*)_tmp17)->f1;_LL34:
# 163
 return Cyc_Toseqc_stmt_effect(_tmp47);case 2U: _LL35: _LL36:
# 165
({struct _dyneither_ptr _tmpD4=({const char*_tmp22="Pragma_e";_tag_dyneither(_tmp22,sizeof(char),9U);});bad_form=_tmpD4;});goto _LL0;case 11U: _LL37: _LL38:
({struct _dyneither_ptr _tmpD5=({const char*_tmp23="Throw_e";_tag_dyneither(_tmp23,sizeof(char),8U);});bad_form=_tmpD5;});goto _LL0;case 16U: _LL39: _LL3A:
({struct _dyneither_ptr _tmpD6=({const char*_tmp24="New_e";_tag_dyneither(_tmp24,sizeof(char),6U);});bad_form=_tmpD6;});goto _LL0;case 24U: _LL3B: _LL3C:
({struct _dyneither_ptr _tmpD7=({const char*_tmp25="Tuple_e";_tag_dyneither(_tmp25,sizeof(char),8U);});bad_form=_tmpD7;});goto _LL0;case 25U: _LL3D: _LL3E:
({struct _dyneither_ptr _tmpD8=({const char*_tmp26="CompoundLit_e";_tag_dyneither(_tmp26,sizeof(char),14U);});bad_form=_tmpD8;});goto _LL0;case 26U: _LL3F: _LL40:
({struct _dyneither_ptr _tmpD9=({const char*_tmp27="Array_e";_tag_dyneither(_tmp27,sizeof(char),8U);});bad_form=_tmpD9;});goto _LL0;case 27U: _LL41: _LL42:
({struct _dyneither_ptr _tmpDA=({const char*_tmp28="Comprehension_e";_tag_dyneither(_tmp28,sizeof(char),16U);});bad_form=_tmpDA;});goto _LL0;case 28U: _LL43: _LL44:
({struct _dyneither_ptr _tmpDB=({const char*_tmp29="ComprehensionNoinit_e";_tag_dyneither(_tmp29,sizeof(char),22U);});bad_form=_tmpDB;});goto _LL0;case 29U: _LL45: _LL46:
({struct _dyneither_ptr _tmpDC=({const char*_tmp2A="Aggregate_e";_tag_dyneither(_tmp2A,sizeof(char),12U);});bad_form=_tmpDC;});goto _LL0;case 30U: _LL47: _LL48:
({struct _dyneither_ptr _tmpDD=({const char*_tmp2B="AnonStruct_e";_tag_dyneither(_tmp2B,sizeof(char),13U);});bad_form=_tmpDD;});goto _LL0;case 31U: _LL49: _LL4A:
({struct _dyneither_ptr _tmpDE=({const char*_tmp2C="Datatype_e";_tag_dyneither(_tmp2C,sizeof(char),11U);});bad_form=_tmpDE;});goto _LL0;case 34U: _LL4B: _LL4C:
({struct _dyneither_ptr _tmpDF=({const char*_tmp2D="Malloc_e";_tag_dyneither(_tmp2D,sizeof(char),9U);});bad_form=_tmpDF;});goto _LL0;case 35U: _LL4D: _LL4E:
({struct _dyneither_ptr _tmpE0=({const char*_tmp2E="Swap_e";_tag_dyneither(_tmp2E,sizeof(char),7U);});bad_form=_tmpE0;});goto _LL0;case 38U: _LL4F: _LL50:
({struct _dyneither_ptr _tmpE1=({const char*_tmp2F="Tagcheck_e";_tag_dyneither(_tmp2F,sizeof(char),11U);});bad_form=_tmpE1;});goto _LL0;default: _LL51: _LL52:
({struct _dyneither_ptr _tmpE2=({const char*_tmp30="Valueof_e";_tag_dyneither(_tmp30,sizeof(char),10U);});bad_form=_tmpE2;});goto _LL0;}_LL0:;}
# 181
({struct Cyc_String_pa_PrintArg_struct _tmp4A=({struct Cyc_String_pa_PrintArg_struct _tmpCE;_tmpCE.tag=0U,_tmpCE.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)bad_form);_tmpCE;});struct Cyc_String_pa_PrintArg_struct _tmp4B=({struct Cyc_String_pa_PrintArg_struct _tmpCD;_tmpCD.tag=0U,({
struct _dyneither_ptr _tmpE3=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_exp2string(e));_tmpCD.f1=_tmpE3;});_tmpCD;});void*_tmp48[2U];_tmp48[0]=& _tmp4A,_tmp48[1]=& _tmp4B;({struct _dyneither_ptr _tmpE4=({const char*_tmp49="bad exp form %s (exp=|%s|) after xlation to C";_tag_dyneither(_tmp49,sizeof(char),46U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmpE4,_tag_dyneither(_tmp48,sizeof(void*),2U));});});}
# 184
static enum Cyc_Toseqc_SideEffect Cyc_Toseqc_stmt_effect(struct Cyc_Absyn_Stmt*s){
# 186
enum Cyc_Toseqc_SideEffect res=Cyc_Toseqc_Const;
while(1){
void*_tmp4D=s->r;void*_tmp4E=_tmp4D;struct Cyc_Absyn_Stmt*_tmp54;struct Cyc_Absyn_Stmt*_tmp53;struct Cyc_Absyn_Stmt*_tmp52;struct Cyc_Absyn_Exp*_tmp51;switch(*((int*)_tmp4E)){case 0U: _LL1: _LL2:
 return res;case 1U: _LL3: _tmp51=((struct Cyc_Absyn_Exp_s_Absyn_Raw_stmt_struct*)_tmp4E)->f1;_LL4:
 return({enum Cyc_Toseqc_SideEffect _tmpE5=res;Cyc_Toseqc_join_side_effect(_tmpE5,Cyc_Toseqc_exp_effect(_tmp51));});case 13U: _LL5: _tmp52=((struct Cyc_Absyn_Label_s_Absyn_Raw_stmt_struct*)_tmp4E)->f2;_LL6:
# 193
 s=_tmp52;
continue;case 2U: _LL7: _tmp54=((struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct*)_tmp4E)->f1;_tmp53=((struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct*)_tmp4E)->f2;_LL8:
# 196
({enum Cyc_Toseqc_SideEffect _tmpE7=({enum Cyc_Toseqc_SideEffect _tmpE6=res;Cyc_Toseqc_join_side_effect(_tmpE6,Cyc_Toseqc_stmt_effect(_tmp54));});res=_tmpE7;});
s=_tmp53;
continue;case 6U: _LL9: _LLA:
# 200
 goto _LLC;case 7U: _LLB: _LLC:
 goto _LLE;case 8U: _LLD: _LLE:
 goto _LL10;case 3U: _LLF: _LL10:
 goto _LL12;case 4U: _LL11: _LL12:
 goto _LL14;case 5U: _LL13: _LL14:
 goto _LL16;case 9U: _LL15: _LL16:
 goto _LL18;case 14U: _LL17: _LL18:
 goto _LL1A;case 12U: _LL19: _LL1A:
 goto _LL1C;case 10U: _LL1B: _LL1C:
 return Cyc_Toseqc_AnyEffect;default: _LL1D: _LL1E:
({void*_tmp4F=0U;({struct _dyneither_ptr _tmpE8=({const char*_tmp50="bad stmt after xlation to C";_tag_dyneither(_tmp50,sizeof(char),28U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmpE8,_tag_dyneither(_tmp4F,sizeof(void*),0U));});});}_LL0:;}}
# 217
static int Cyc_Toseqc_is_toc_var(struct Cyc_Absyn_Exp*e){
void*_tmp55=e->r;void*_tmp56=_tmp55;struct _tuple1*_tmp57;if(((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp56)->tag == 1U){if(((struct Cyc_Absyn_Unresolved_b_Absyn_Binding_struct*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp56)->f1)->tag == 0U){_LL1: _tmp57=((struct Cyc_Absyn_Unresolved_b_Absyn_Binding_struct*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp56)->f1)->f1;_LL2:
 return 1;}else{goto _LL3;}}else{_LL3: _LL4:
 return 0;}_LL0:;}
# 224
static void Cyc_Toseqc_stmt_to_seqc(struct Cyc_Absyn_Stmt*s);
static void Cyc_Toseqc_exp_to_seqc(struct Cyc_Absyn_Exp*s);
# 242 "toseqc.cyc"
static struct Cyc_Absyn_Stmt*Cyc_Toseqc_exps_to_seqc(struct Cyc_Absyn_Exp*e,struct Cyc_List_List*es){
# 244
((void(*)(void(*f)(struct Cyc_Absyn_Exp*),struct Cyc_List_List*x))Cyc_List_iter)(Cyc_Toseqc_exp_to_seqc,es);
# 247
if(((int(*)(struct Cyc_List_List*x))Cyc_List_length)(es)<= 1  || Cyc_Toseqc_exps_effect(es)!= Cyc_Toseqc_AnyEffect)return 0;{
# 251
struct Cyc_Absyn_Stmt*stmt=({struct Cyc_Absyn_Exp*_tmpE9=Cyc_Absyn_copy_exp(e);Cyc_Absyn_exp_stmt(_tmpE9,e->loc);});
struct Cyc_Absyn_Stmt*laststmt=stmt;
int did_skip=0;
int did_convert=0;
{struct Cyc_List_List*x=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_rev)(es);for(0;x != 0;x=x->tl){
struct Cyc_Absyn_Exp*_tmp58=(struct Cyc_Absyn_Exp*)x->hd;
# 259
if(Cyc_Tcutil_is_const_exp(_tmp58) || Cyc_Toseqc_is_toc_var(_tmp58))continue;
# 264
if(!did_skip){
did_skip=1;
continue;}
# 268
did_convert=1;{
# 271
struct _tuple1*_tmp59=Cyc_Toc_temp_var();
struct Cyc_Absyn_Exp*_tmp5A=Cyc_Absyn_var_exp(_tmp59,_tmp58->loc);
# 275
struct Cyc_Absyn_Exp*_tmp5B=Cyc_Absyn_new_exp(_tmp58->r,_tmp58->loc);
_tmp5B->annot=_tmp58->annot;
_tmp5B->topt=_tmp58->topt;{
# 279
void*t=_tmp5B->topt == 0?(void*)({struct Cyc_Absyn_TypeofType_Absyn_Type_struct*_tmp5C=_cycalloc(sizeof(*_tmp5C));_tmp5C->tag=27U,_tmp5C->f1=_tmp5B;_tmp5C;}): Cyc_Toc_typ_to_c((void*)_check_null(_tmp5B->topt));
# 283
({struct Cyc_Absyn_Stmt*_tmpEA=Cyc_Absyn_declare_stmt(_tmp59,t,_tmp5B,stmt,e->loc);stmt=_tmpEA;});
# 286
_tmp58->r=_tmp5A->r;};};}}
# 290
if(did_convert){
({void*_tmpEB=(Cyc_Absyn_stmt_exp(stmt,e->loc))->r;e->r=_tmpEB;});
return laststmt;}else{
# 294
return 0;}};}
# 300
static void Cyc_Toseqc_exp_to_seqc(struct Cyc_Absyn_Exp*e){
struct _dyneither_ptr bad_form=({const char*_tmp92="";_tag_dyneither(_tmp92,sizeof(char),1U);});
{void*_tmp5D=e->r;void*_tmp5E=_tmp5D;struct Cyc_Absyn_Stmt*_tmp8D;struct Cyc_List_List*_tmp8C;struct Cyc_Absyn_Exp*_tmp8B;struct Cyc_Absyn_Exp*_tmp8A;struct Cyc_Absyn_Exp*_tmp89;struct Cyc_Absyn_Exp*_tmp88;struct Cyc_Absyn_Exp*_tmp87;struct Cyc_Absyn_Exp*_tmp86;struct Cyc_Absyn_Exp*_tmp85;struct Cyc_Absyn_Exp*_tmp84;struct Cyc_Absyn_Exp*_tmp83;struct Cyc_Absyn_Exp*_tmp82;struct Cyc_Absyn_Exp*_tmp81;struct Cyc_Absyn_Exp*_tmp80;struct Cyc_Absyn_Exp*_tmp7F;struct Cyc_Absyn_Exp*_tmp7E;struct Cyc_Absyn_Exp*_tmp7D;struct Cyc_Absyn_Exp*_tmp7C;struct Cyc_Absyn_Exp*_tmp7B;struct Cyc_Absyn_Exp*_tmp7A;struct Cyc_Absyn_Exp*_tmp79;struct Cyc_Absyn_Exp*_tmp78;struct Cyc_Absyn_Exp*_tmp77;struct Cyc_Absyn_Exp*_tmp76;struct Cyc_List_List*_tmp75;struct Cyc_Absyn_Exp*_tmp74;struct Cyc_List_List*_tmp73;switch(*((int*)_tmp5E)){case 0U: _LL1: _LL2:
 return;case 1U: _LL3: _LL4:
 return;case 10U: _LL5: _tmp74=((struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_tmp5E)->f1;_tmp73=((struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_tmp5E)->f2;_LL6:
# 306
({struct Cyc_List_List*_tmpEC=({struct Cyc_List_List*_tmp5F=_cycalloc(sizeof(*_tmp5F));_tmp5F->hd=_tmp74,_tmp5F->tl=_tmp73;_tmp5F;});_tmp75=_tmpEC;});goto _LL8;case 3U: _LL7: _tmp75=((struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_tmp5E)->f2;_LL8:
# 308
 Cyc_Toseqc_exps_to_seqc(e,_tmp75);
return;case 23U: _LL9: _tmp77=((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmp5E)->f1;_tmp76=((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmp5E)->f2;_LLA:
# 311
 _tmp79=_tmp77;_tmp78=_tmp76;goto _LLC;case 4U: _LLB: _tmp79=((struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct*)_tmp5E)->f1;_tmp78=((struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct*)_tmp5E)->f3;_LLC:
# 314
({struct Cyc_Absyn_Exp*_tmpED=e;Cyc_Toseqc_exps_to_seqc(_tmpED,({struct Cyc_Absyn_Exp*_tmp60[2U];_tmp60[0]=_tmp78,_tmp60[1]=_tmp79;((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp60,sizeof(struct Cyc_Absyn_Exp*),2U));}));});
return;case 6U: _LLD: _tmp7C=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmp5E)->f1;_tmp7B=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmp5E)->f2;_tmp7A=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmp5E)->f3;_LLE:
# 318
 Cyc_Toseqc_exp_to_seqc(_tmp7A);_tmp7E=_tmp7C;_tmp7D=_tmp7B;goto _LL10;case 7U: _LLF: _tmp7E=((struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct*)_tmp5E)->f1;_tmp7D=((struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct*)_tmp5E)->f2;_LL10:
 _tmp80=_tmp7E;_tmp7F=_tmp7D;goto _LL12;case 8U: _LL11: _tmp80=((struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct*)_tmp5E)->f1;_tmp7F=((struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct*)_tmp5E)->f2;_LL12:
 _tmp82=_tmp80;_tmp81=_tmp7F;goto _LL14;case 9U: _LL13: _tmp82=((struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct*)_tmp5E)->f1;_tmp81=((struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct*)_tmp5E)->f2;_LL14:
# 322
 Cyc_Toseqc_exp_to_seqc(_tmp81);_tmp83=_tmp82;goto _LL16;case 12U: _LL15: _tmp83=((struct Cyc_Absyn_NoInstantiate_e_Absyn_Raw_exp_struct*)_tmp5E)->f1;_LL16:
# 324
 _tmp84=_tmp83;goto _LL18;case 13U: _LL17: _tmp84=((struct Cyc_Absyn_Instantiate_e_Absyn_Raw_exp_struct*)_tmp5E)->f1;_LL18:
 _tmp85=_tmp84;goto _LL1A;case 14U: _LL19: _tmp85=((struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_tmp5E)->f2;_LL1A:
 _tmp86=_tmp85;goto _LL1C;case 15U: _LL1B: _tmp86=((struct Cyc_Absyn_Address_e_Absyn_Raw_exp_struct*)_tmp5E)->f1;_LL1C:
 _tmp87=_tmp86;goto _LL1E;case 18U: _LL1D: _tmp87=((struct Cyc_Absyn_Sizeofexp_e_Absyn_Raw_exp_struct*)_tmp5E)->f1;_LL1E:
 _tmp88=_tmp87;goto _LL20;case 20U: _LL1F: _tmp88=((struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct*)_tmp5E)->f1;_LL20:
 _tmp89=_tmp88;goto _LL22;case 21U: _LL21: _tmp89=((struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_tmp5E)->f1;_LL22:
 _tmp8A=_tmp89;goto _LL24;case 22U: _LL23: _tmp8A=((struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_tmp5E)->f1;_LL24:
 _tmp8B=_tmp8A;goto _LL26;case 5U: _LL25: _tmp8B=((struct Cyc_Absyn_Increment_e_Absyn_Raw_exp_struct*)_tmp5E)->f1;_LL26:
# 333
 Cyc_Toseqc_exp_to_seqc(_tmp8B);
return;case 17U: _LL27: _LL28:
# 336
 goto _LL2A;case 19U: _LL29: _LL2A:
 goto _LL2C;case 33U: _LL2B: _LL2C:
 goto _LL2E;case 32U: _LL2D: _LL2E:
 return;case 36U: _LL2F: _tmp8C=((struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct*)_tmp5E)->f2;_LL30: {
# 342
struct _tuple0 _tmp61=((struct _tuple0(*)(struct Cyc_List_List*x))Cyc_List_split)(_tmp8C);struct _tuple0 _tmp62=_tmp61;struct Cyc_List_List*_tmp63;_LL54: _tmp63=_tmp62.f2;_LL55:;
Cyc_Toseqc_exps_to_seqc(e,_tmp63);
return;}case 37U: _LL31: _tmp8D=((struct Cyc_Absyn_StmtExp_e_Absyn_Raw_exp_struct*)_tmp5E)->f1;_LL32:
# 347
 Cyc_Toseqc_stmt_to_seqc(_tmp8D);
return;case 40U: _LL33: _LL34:
# 350
 return;case 2U: _LL35: _LL36:
# 352
({struct _dyneither_ptr _tmpEE=({const char*_tmp64="Pragma_e";_tag_dyneither(_tmp64,sizeof(char),9U);});bad_form=_tmpEE;});goto _LL0;case 11U: _LL37: _LL38:
({struct _dyneither_ptr _tmpEF=({const char*_tmp65="Throw_e";_tag_dyneither(_tmp65,sizeof(char),8U);});bad_form=_tmpEF;});goto _LL0;case 16U: _LL39: _LL3A:
({struct _dyneither_ptr _tmpF0=({const char*_tmp66="New_e";_tag_dyneither(_tmp66,sizeof(char),6U);});bad_form=_tmpF0;});goto _LL0;case 24U: _LL3B: _LL3C:
({struct _dyneither_ptr _tmpF1=({const char*_tmp67="Tuple_e";_tag_dyneither(_tmp67,sizeof(char),8U);});bad_form=_tmpF1;});goto _LL0;case 25U: _LL3D: _LL3E:
({struct _dyneither_ptr _tmpF2=({const char*_tmp68="CompoundLit_e";_tag_dyneither(_tmp68,sizeof(char),14U);});bad_form=_tmpF2;});goto _LL0;case 26U: _LL3F: _LL40:
({struct _dyneither_ptr _tmpF3=({const char*_tmp69="Array_e";_tag_dyneither(_tmp69,sizeof(char),8U);});bad_form=_tmpF3;});goto _LL0;case 27U: _LL41: _LL42:
({struct _dyneither_ptr _tmpF4=({const char*_tmp6A="Comprehension_e";_tag_dyneither(_tmp6A,sizeof(char),16U);});bad_form=_tmpF4;});goto _LL0;case 28U: _LL43: _LL44:
({struct _dyneither_ptr _tmpF5=({const char*_tmp6B="ComprehensionNoinit_e";_tag_dyneither(_tmp6B,sizeof(char),22U);});bad_form=_tmpF5;});goto _LL0;case 29U: _LL45: _LL46:
({struct _dyneither_ptr _tmpF6=({const char*_tmp6C="Aggregate_e";_tag_dyneither(_tmp6C,sizeof(char),12U);});bad_form=_tmpF6;});goto _LL0;case 30U: _LL47: _LL48:
({struct _dyneither_ptr _tmpF7=({const char*_tmp6D="AnonStruct_e";_tag_dyneither(_tmp6D,sizeof(char),13U);});bad_form=_tmpF7;});goto _LL0;case 31U: _LL49: _LL4A:
# 363
({struct _dyneither_ptr _tmpF8=({const char*_tmp6E="Datatype_e";_tag_dyneither(_tmp6E,sizeof(char),11U);});bad_form=_tmpF8;});goto _LL0;case 34U: _LL4B: _LL4C:
({struct _dyneither_ptr _tmpF9=({const char*_tmp6F="Malloc_e";_tag_dyneither(_tmp6F,sizeof(char),9U);});bad_form=_tmpF9;});goto _LL0;case 35U: _LL4D: _LL4E:
({struct _dyneither_ptr _tmpFA=({const char*_tmp70="Swap_e";_tag_dyneither(_tmp70,sizeof(char),7U);});bad_form=_tmpFA;});goto _LL0;case 38U: _LL4F: _LL50:
({struct _dyneither_ptr _tmpFB=({const char*_tmp71="Tagcheck_e";_tag_dyneither(_tmp71,sizeof(char),11U);});bad_form=_tmpFB;});goto _LL0;default: _LL51: _LL52:
({struct _dyneither_ptr _tmpFC=({const char*_tmp72="Valueof_e";_tag_dyneither(_tmp72,sizeof(char),10U);});bad_form=_tmpFC;});goto _LL0;}_LL0:;}
# 369
({struct Cyc_String_pa_PrintArg_struct _tmp90=({struct Cyc_String_pa_PrintArg_struct _tmpD0;_tmpD0.tag=0U,_tmpD0.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)bad_form);_tmpD0;});struct Cyc_String_pa_PrintArg_struct _tmp91=({struct Cyc_String_pa_PrintArg_struct _tmpCF;_tmpCF.tag=0U,({
struct _dyneither_ptr _tmpFD=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_exp2string(e));_tmpCF.f1=_tmpFD;});_tmpCF;});void*_tmp8E[2U];_tmp8E[0]=& _tmp90,_tmp8E[1]=& _tmp91;({struct _dyneither_ptr _tmpFE=({const char*_tmp8F="bad exp form %s (exp=|%s|) after xlation to C";_tag_dyneither(_tmp8F,sizeof(char),46U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmpFE,_tag_dyneither(_tmp8E,sizeof(void*),2U));});});}
# 373
static void Cyc_Toseqc_stmt_to_seqc(struct Cyc_Absyn_Stmt*s){
# 375
while(1){
void*_tmp93=s->r;void*_tmp94=_tmp93;struct Cyc_Absyn_Decl*_tmpC0;struct Cyc_Absyn_Stmt*_tmpBF;struct Cyc_Absyn_Exp*_tmpBE;struct Cyc_List_List*_tmpBD;struct Cyc_Absyn_Stmt*_tmpBC;struct Cyc_Absyn_Exp*_tmpBB;struct Cyc_Absyn_Exp*_tmpBA;struct Cyc_Absyn_Exp*_tmpB9;struct Cyc_Absyn_Exp*_tmpB8;struct Cyc_Absyn_Stmt*_tmpB7;struct Cyc_Absyn_Exp*_tmpB6;struct Cyc_Absyn_Stmt*_tmpB5;struct Cyc_Absyn_Exp*_tmpB4;struct Cyc_Absyn_Stmt*_tmpB3;struct Cyc_Absyn_Stmt*_tmpB2;struct Cyc_Absyn_Stmt*_tmpB1;struct Cyc_Absyn_Stmt*_tmpB0;struct Cyc_Absyn_Exp*_tmpAF;struct Cyc_Absyn_Exp*_tmpAE;struct Cyc_Absyn_Stmt*_tmpAD;switch(*((int*)_tmp94)){case 0U: _LL1: _LL2:
 goto _LL4;case 6U: _LL3: _LL4:
 goto _LL6;case 7U: _LL5: _LL6:
 goto _LL8;case 8U: _LL7: _LL8:
 return;case 13U: _LL9: _tmpAD=((struct Cyc_Absyn_Label_s_Absyn_Raw_stmt_struct*)_tmp94)->f2;_LLA:
# 382
 s=_tmpAD;
continue;case 3U: _LLB: _tmpAE=((struct Cyc_Absyn_Return_s_Absyn_Raw_stmt_struct*)_tmp94)->f1;_LLC:
# 385
 if(_tmpAE == 0)
return;
_tmpAF=_tmpAE;goto _LLE;case 1U: _LLD: _tmpAF=((struct Cyc_Absyn_Exp_s_Absyn_Raw_stmt_struct*)_tmp94)->f1;_LLE:
# 389
 Cyc_Toseqc_exp_to_seqc(_tmpAF);
# 397
return;case 2U: _LLF: _tmpB1=((struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct*)_tmp94)->f1;_tmpB0=((struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct*)_tmp94)->f2;_LL10:
# 399
 Cyc_Toseqc_stmt_to_seqc(_tmpB1);
s=_tmpB0;
continue;case 4U: _LL11: _tmpB4=((struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct*)_tmp94)->f1;_tmpB3=((struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct*)_tmp94)->f2;_tmpB2=((struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct*)_tmp94)->f3;_LL12:
# 403
 Cyc_Toseqc_exp_to_seqc(_tmpB4);
Cyc_Toseqc_stmt_to_seqc(_tmpB3);
s=_tmpB2;
continue;case 5U: _LL13: _tmpB6=(((struct Cyc_Absyn_While_s_Absyn_Raw_stmt_struct*)_tmp94)->f1).f1;_tmpB5=((struct Cyc_Absyn_While_s_Absyn_Raw_stmt_struct*)_tmp94)->f2;_LL14:
# 408
 Cyc_Toseqc_exp_to_seqc(_tmpB6);
s=_tmpB5;
continue;case 9U: _LL15: _tmpBA=((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp94)->f1;_tmpB9=(((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp94)->f2).f1;_tmpB8=(((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp94)->f3).f1;_tmpB7=((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp94)->f4;_LL16:
# 412
 Cyc_Toseqc_exp_to_seqc(_tmpBA);
Cyc_Toseqc_exp_to_seqc(_tmpB9);
Cyc_Toseqc_exp_to_seqc(_tmpB8);
s=_tmpB7;
continue;case 14U: _LL17: _tmpBC=((struct Cyc_Absyn_Do_s_Absyn_Raw_stmt_struct*)_tmp94)->f1;_tmpBB=(((struct Cyc_Absyn_Do_s_Absyn_Raw_stmt_struct*)_tmp94)->f2).f1;_LL18:
# 418
 Cyc_Toseqc_exp_to_seqc(_tmpBB);
s=_tmpBC;
continue;case 10U: _LL19: _tmpBE=((struct Cyc_Absyn_Switch_s_Absyn_Raw_stmt_struct*)_tmp94)->f1;_tmpBD=((struct Cyc_Absyn_Switch_s_Absyn_Raw_stmt_struct*)_tmp94)->f2;_LL1A:
# 424
 Cyc_Toseqc_exp_to_seqc(_tmpBE);
for(0;_tmpBD != 0;_tmpBD=_tmpBD->tl){
Cyc_Toseqc_stmt_to_seqc(((struct Cyc_Absyn_Switch_clause*)_tmpBD->hd)->body);}
return;case 12U: _LL1B: _tmpC0=((struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*)_tmp94)->f1;_tmpBF=((struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*)_tmp94)->f2;_LL1C:
# 429
{void*_tmp95=_tmpC0->r;void*_tmp96=_tmp95;struct Cyc_Absyn_Vardecl*_tmpAA;if(((struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct*)_tmp96)->tag == 0U){_LL20: _tmpAA=((struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct*)_tmp96)->f1;_LL21:
# 431
 if(_tmpAA->initializer != 0){
# 434
void*_tmp97=((struct Cyc_Absyn_Exp*)_check_null(_tmpAA->initializer))->r;void*_tmp98=_tmp97;struct Cyc_List_List*_tmpA9;if(((struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct*)_tmp98)->tag == 36U){_LL25: _tmpA9=((struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct*)_tmp98)->f2;_LL26: {
# 447 "toseqc.cyc"
struct _tuple0 _tmp99=((struct _tuple0(*)(struct Cyc_List_List*x))Cyc_List_split)(_tmpA9);struct _tuple0 _tmp9A=_tmp99;struct Cyc_List_List*_tmpA8;_LL2A: _tmpA8=_tmp9A.f2;_LL2B:;{
struct Cyc_Absyn_Stmt*_tmp9B=Cyc_Toseqc_exps_to_seqc((struct Cyc_Absyn_Exp*)_check_null(_tmpAA->initializer),_tmpA8);
# 450
if(_tmp9B == 0)goto _LL24;
# 456
{void*_tmp9C=_tmp9B->r;void*_tmp9D=_tmp9C;struct Cyc_Absyn_Exp*_tmpA7;if(((struct Cyc_Absyn_Exp_s_Absyn_Raw_stmt_struct*)_tmp9D)->tag == 1U){_LL2D: _tmpA7=((struct Cyc_Absyn_Exp_s_Absyn_Raw_stmt_struct*)_tmp9D)->f1;_LL2E:
# 458
{void*_tmp9E=((struct Cyc_Absyn_Exp*)_check_null(_tmpAA->initializer))->r;void*_tmp9F=_tmp9E;struct Cyc_Absyn_Stmt*_tmpA3;if(((struct Cyc_Absyn_StmtExp_e_Absyn_Raw_exp_struct*)_tmp9F)->tag == 37U){_LL32: _tmpA3=((struct Cyc_Absyn_StmtExp_e_Absyn_Raw_exp_struct*)_tmp9F)->f1;_LL33:
# 460
 _tmpAA->initializer=_tmpA7;
_tmp9B->r=s->r;
s->r=_tmpA3->r;
goto _LL31;}else{_LL34: _LL35:
# 465
({struct Cyc_String_pa_PrintArg_struct _tmpA2=({struct Cyc_String_pa_PrintArg_struct _tmpD1;_tmpD1.tag=0U,({
struct _dyneither_ptr _tmpFF=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_exp2string((struct Cyc_Absyn_Exp*)_check_null(_tmpAA->initializer)));_tmpD1.f1=_tmpFF;});_tmpD1;});void*_tmpA0[1U];_tmpA0[0]=& _tmpA2;({struct _dyneither_ptr _tmp100=({const char*_tmpA1="exps_to_seqc updated to non-stmt-exp |%s|";_tag_dyneither(_tmpA1,sizeof(char),42U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp100,_tag_dyneither(_tmpA0,sizeof(void*),1U));});});}_LL31:;}
# 468
goto _LL2C;}else{_LL2F: _LL30:
# 470
({struct Cyc_String_pa_PrintArg_struct _tmpA6=({struct Cyc_String_pa_PrintArg_struct _tmpD2;_tmpD2.tag=0U,({
struct _dyneither_ptr _tmp101=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_stmt2string(_tmp9B));_tmpD2.f1=_tmp101;});_tmpD2;});void*_tmpA4[1U];_tmpA4[0]=& _tmpA6;({struct _dyneither_ptr _tmp102=({const char*_tmpA5="exps_to_seqc returned non-exp-stmt |%s|";_tag_dyneither(_tmpA5,sizeof(char),40U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp102,_tag_dyneither(_tmpA4,sizeof(void*),1U));});});}_LL2C:;}
# 473
goto _LL24;};}}else{_LL27: _LL28:
# 476
 Cyc_Toseqc_exp_to_seqc((struct Cyc_Absyn_Exp*)_check_null(_tmpAA->initializer));
goto _LL24;}_LL24:;}
# 480
s=_tmpBF;
continue;}else{_LL22: _LL23:
 goto _LL1F;}_LL1F:;}
# 484
goto _LL1E;default: _LL1D: _LL1E:
({void*_tmpAB=0U;({struct _dyneither_ptr _tmp103=({const char*_tmpAC="bad stmt after xlation to C";_tag_dyneither(_tmpAC,sizeof(char),28U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp103,_tag_dyneither(_tmpAB,sizeof(void*),0U));});});}_LL0:;}}
# 490
struct Cyc_List_List*Cyc_Toseqc_toseqc(struct Cyc_List_List*ds){
struct Cyc_List_List*_tmpC1=ds;
# 493
for(0;_tmpC1 != 0;_tmpC1=_tmpC1->tl){
struct Cyc_Absyn_Decl*_tmpC2=(struct Cyc_Absyn_Decl*)_tmpC1->hd;
void*_tmpC3=_tmpC2->r;void*_tmpC4=_tmpC3;struct Cyc_List_List*_tmpCB;struct Cyc_List_List*_tmpCA;struct Cyc_List_List*_tmpC9;struct Cyc_List_List*_tmpC8;struct Cyc_Absyn_Fndecl*_tmpC7;switch(*((int*)_tmpC4)){case 1U: _LL1: _tmpC7=((struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct*)_tmpC4)->f1;_LL2:
# 497
 Cyc_Toseqc_stmt_to_seqc(_tmpC7->body);
goto _LL0;case 0U: _LL3: _LL4:
 goto _LL6;case 2U: _LL5: _LL6:
 goto _LL8;case 3U: _LL7: _LL8:
 goto _LLA;case 4U: _LL9: _LLA:
 goto _LLC;case 5U: _LLB: _LLC:
 goto _LLE;case 6U: _LLD: _LLE:
 goto _LL10;case 7U: _LLF: _LL10:
 goto _LL12;case 8U: _LL11: _LL12:
# 507
 goto _LL0;case 9U: _LL13: _tmpC8=((struct Cyc_Absyn_Namespace_d_Absyn_Raw_decl_struct*)_tmpC4)->f2;_LL14:
 _tmpC9=_tmpC8;goto _LL16;case 10U: _LL15: _tmpC9=((struct Cyc_Absyn_Using_d_Absyn_Raw_decl_struct*)_tmpC4)->f2;_LL16:
 _tmpCA=_tmpC9;goto _LL18;case 11U: _LL17: _tmpCA=((struct Cyc_Absyn_ExternC_d_Absyn_Raw_decl_struct*)_tmpC4)->f1;_LL18:
 _tmpCB=_tmpCA;goto _LL1A;case 12U: _LL19: _tmpCB=((struct Cyc_Absyn_ExternCinclude_d_Absyn_Raw_decl_struct*)_tmpC4)->f1;_LL1A:
 goto _LL1C;case 13U: _LL1B: _LL1C:
 goto _LL1E;default: _LL1D: _LL1E:
# 514
({void*_tmpC5=0U;({struct _dyneither_ptr _tmp104=({const char*_tmpC6="nested translation unit after translation to C";_tag_dyneither(_tmpC6,sizeof(char),47U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Warn_impos)(_tmp104,_tag_dyneither(_tmpC5,sizeof(void*),0U));});});}_LL0:;}
# 517
return ds;}
