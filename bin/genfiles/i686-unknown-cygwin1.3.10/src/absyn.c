// This is a C header file to be used by the output of the Cyclone
// to C translator.  The corresponding definitions are in file lib/runtime_cyc.c
#ifndef _CYC_INCLUDE_H_
#define _CYC_INCLUDE_H_

#include <setjmp.h>

#ifdef NO_CYC_PREFIX
#define ADD_PREFIX(x) x
#else
#define ADD_PREFIX(x) Cyc_##x
#endif

#ifndef offsetof
// should be size_t, but int is fine.
#define offsetof(t,n) ((int)(&(((t *)0)->n)))
#endif

//// Tagged arrays
struct _tagged_arr { 
  unsigned char *curr; 
  unsigned char *base; 
  unsigned char *last_plus_one; 
};

//// Discriminated Unions
struct _xtunion_struct { char *tag; };

// Need one of these per thread (we don't have threads)
// The runtime maintains a stack that contains either _handler_cons
// structs or _RegionHandle structs.  The tag is 0 for a handler_cons
// and 1 for a region handle.  
struct _RuntimeStack {
  int tag; // 0 for an exception handler, 1 for a region handle
  struct _RuntimeStack *next;
};

//// Regions
struct _RegionPage {
#ifdef CYC_REGION_PROFILE
  unsigned total_bytes;
  unsigned free_bytes;
#endif
  struct _RegionPage *next;
  char data[0];
};

struct _RegionHandle {
  struct _RuntimeStack s;
  struct _RegionPage *curr;
  char               *offset;
  char               *last_plus_one;
#ifdef CYC_REGION_PROFILE
  const char         *name;
#endif
};

extern struct _RegionHandle _new_region(const char *);
extern void * _region_malloc(struct _RegionHandle *, unsigned);
extern void * _region_calloc(struct _RegionHandle *, unsigned t, unsigned n);
extern void   _free_region(struct _RegionHandle *);
extern void   _reset_region(struct _RegionHandle *);

//// Exceptions 
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

//// Built-in Exceptions
extern struct _xtunion_struct ADD_PREFIX(Null_Exception_struct);
extern struct _xtunion_struct * ADD_PREFIX(Null_Exception);
extern struct _xtunion_struct ADD_PREFIX(Array_bounds_struct);
extern struct _xtunion_struct * ADD_PREFIX(Array_bounds);
extern struct _xtunion_struct ADD_PREFIX(Match_Exception_struct);
extern struct _xtunion_struct * ADD_PREFIX(Match_Exception);
extern struct _xtunion_struct ADD_PREFIX(Bad_alloc_struct);
extern struct _xtunion_struct * ADD_PREFIX(Bad_alloc);

//// Built-in Run-time Checks and company
#ifdef __APPLE__
#define _INLINE_FUNCTIONS
#endif

#ifdef NO_CYC_NULL_CHECKS
#define _check_null(ptr) (ptr)
#else
#ifdef _INLINE_FUNCTIONS
static inline void *
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
static inline char *
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
static inline unsigned
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

#ifdef NO_CYC_BOUNDS_CHECKS
#ifdef _INLINE_FUNCTIONS
static inline unsigned char *
_check_unknown_subscript(struct _tagged_arr arr,unsigned elt_sz,unsigned index) {
  struct _tagged_arr _cus_arr = (arr);
  unsigned _cus_elt_sz = (elt_sz);
  unsigned _cus_index = (index);
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index;
  return _cus_ans;
}
#else
#define _check_unknown_subscript(arr,elt_sz,index) ({ \
  struct _tagged_arr _cus_arr = (arr); \
  unsigned _cus_elt_sz = (elt_sz); \
  unsigned _cus_index = (index); \
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index; \
  _cus_ans; })
#endif
#else
#ifdef _INLINE_FUNCTIONS
static inline unsigned char *
_check_unknown_subscript(struct _tagged_arr arr,unsigned elt_sz,unsigned index) {
  struct _tagged_arr _cus_arr = (arr);
  unsigned _cus_elt_sz = (elt_sz);
  unsigned _cus_index = (index);
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index;
  if (!_cus_arr.base) _throw_null();
  if (_cus_ans < _cus_arr.base || _cus_ans >= _cus_arr.last_plus_one)
    _throw_arraybounds();
  return _cus_ans;
}
#else
#define _check_unknown_subscript(arr,elt_sz,index) ({ \
  struct _tagged_arr _cus_arr = (arr); \
  unsigned _cus_elt_sz = (elt_sz); \
  unsigned _cus_index = (index); \
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index; \
  if (!_cus_arr.base) _throw_null(); \
  if (_cus_ans < _cus_arr.base || _cus_ans >= _cus_arr.last_plus_one) \
    _throw_arraybounds(); \
  _cus_ans; })
#endif
#endif

#ifdef _INLINE_FUNCTIONS
static inline struct _tagged_arr
_tag_arr(const void *tcurr,unsigned elt_sz,unsigned num_elts) {
  struct _tagged_arr _tag_arr_ans;
  _tag_arr_ans.base = _tag_arr_ans.curr = (void*)(tcurr);
  _tag_arr_ans.last_plus_one = _tag_arr_ans.base + (elt_sz) * (num_elts);
  return _tag_arr_ans;
}
#else
#define _tag_arr(tcurr,elt_sz,num_elts) ({ \
  struct _tagged_arr _tag_arr_ans; \
  _tag_arr_ans.base = _tag_arr_ans.curr = (void*)(tcurr); \
  _tag_arr_ans.last_plus_one = _tag_arr_ans.base + (elt_sz) * (num_elts); \
  _tag_arr_ans; })
#endif

#ifdef _INLINE_FUNCTIONS
static inline struct _tagged_arr *
_init_tag_arr(struct _tagged_arr *arr_ptr,
              void *arr, unsigned elt_sz, unsigned num_elts) {
  struct _tagged_arr *_itarr_ptr = (arr_ptr);
  void* _itarr = (arr);
  _itarr_ptr->base = _itarr_ptr->curr = _itarr;
  _itarr_ptr->last_plus_one = ((char *)_itarr) + (elt_sz) * (num_elts);
  return _itarr_ptr;
}
#else
#define _init_tag_arr(arr_ptr,arr,elt_sz,num_elts) ({ \
  struct _tagged_arr *_itarr_ptr = (arr_ptr); \
  void* _itarr = (arr); \
  _itarr_ptr->base = _itarr_ptr->curr = _itarr; \
  _itarr_ptr->last_plus_one = ((char *)_itarr) + (elt_sz) * (num_elts); \
  _itarr_ptr; })
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _untag_arr(arr,elt_sz,num_elts) ((arr).curr)
#else
#ifdef _INLINE_FUNCTIONS
static inline unsigned char *
_untag_arr(struct _tagged_arr arr, unsigned elt_sz,unsigned num_elts) {
  struct _tagged_arr _arr = (arr);
  unsigned char *_curr = _arr.curr;
  if (_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one)
    _throw_arraybounds();
  return _curr;
}
#else
#define _untag_arr(arr,elt_sz,num_elts) ({ \
  struct _tagged_arr _arr = (arr); \
  unsigned char *_curr = _arr.curr; \
  if (_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one)\
    _throw_arraybounds(); \
  _curr; })
#endif
#endif

#ifdef _INLINE_FUNCTIONS
static inline unsigned
_get_arr_size(struct _tagged_arr arr,unsigned elt_sz) {
  struct _tagged_arr _get_arr_size_temp = (arr);
  unsigned char *_get_arr_size_curr=_get_arr_size_temp.curr;
  unsigned char *_get_arr_size_last=_get_arr_size_temp.last_plus_one;
  return (_get_arr_size_curr < _get_arr_size_temp.base ||
          _get_arr_size_curr >= _get_arr_size_last) ? 0 :
    ((_get_arr_size_last - _get_arr_size_curr) / (elt_sz));
}
#else
#define _get_arr_size(arr,elt_sz) \
  ({struct _tagged_arr _get_arr_size_temp = (arr); \
    unsigned char *_get_arr_size_curr=_get_arr_size_temp.curr; \
    unsigned char *_get_arr_size_last=_get_arr_size_temp.last_plus_one; \
    (_get_arr_size_curr < _get_arr_size_temp.base || \
     _get_arr_size_curr >= _get_arr_size_last) ? 0 : \
    ((_get_arr_size_last - _get_arr_size_curr) / (elt_sz));})
#endif

#ifdef _INLINE_FUNCTIONS
static inline struct _tagged_arr
_tagged_arr_plus(struct _tagged_arr arr,unsigned elt_sz,int change) {
  struct _tagged_arr _ans = (arr);
  _ans.curr += ((int)(elt_sz))*(change);
  return _ans;
}
#else
#define _tagged_arr_plus(arr,elt_sz,change) ({ \
  struct _tagged_arr _ans = (arr); \
  _ans.curr += ((int)(elt_sz))*(change); \
  _ans; })
#endif

#ifdef _INLINE_FUNCTIONS
static inline struct _tagged_arr
_tagged_arr_inplace_plus(struct _tagged_arr *arr_ptr,unsigned elt_sz,int change) {
  struct _tagged_arr * _arr_ptr = (arr_ptr);
  _arr_ptr->curr += ((int)(elt_sz))*(change);
  return *_arr_ptr;
}
#else
#define _tagged_arr_inplace_plus(arr_ptr,elt_sz,change) ({ \
  struct _tagged_arr * _arr_ptr = (arr_ptr); \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  *_arr_ptr; })
#endif

#ifdef _INLINE_FUNCTIONS
static inline struct _tagged_arr
_tagged_arr_inplace_plus_post(struct _tagged_arr *arr_ptr,unsigned elt_sz,int change) {
  struct _tagged_arr * _arr_ptr = (arr_ptr);
  struct _tagged_arr _ans = *_arr_ptr;
  _arr_ptr->curr += ((int)(elt_sz))*(change);
  return _ans;
}
#else
#define _tagged_arr_inplace_plus_post(arr_ptr,elt_sz,change) ({ \
  struct _tagged_arr * _arr_ptr = (arr_ptr); \
  struct _tagged_arr _ans = *_arr_ptr; \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  _ans; })
#endif

// Decrease the upper bound on a fat pointer by numelts where sz is
// the size of the pointer's type.  Note that this can't be a macro
// if we're to get initializers right.
static struct _tagged_arr _tagged_ptr_decrease_size(struct _tagged_arr x,
                                                    unsigned int sz,
                                                    unsigned int numelts) {
  x.last_plus_one -= sz * numelts; 
  return x; 
}

// Add i to zero-terminated pointer x.  Checks for x being null and
// ensures that x[0..i-1] are not 0.
#ifdef NO_CYC_BOUNDS_CHECK
#define _zero_arr_plus(orig_x,orig_sz,orig_i) ((orig_x)+(orig_i))
#else
#define _zero_arr_plus(orig_x,orig_sz,orig_i) ({ \
  typedef _czs_tx = (*orig_x); \
  _czs_tx *_czs_x = (_czs_tx *)(orig_x); \
  unsigned int _czs_sz = (orig_sz); \
  int _czs_i = (orig_i); \
  unsigned int _czs_temp; \
  if ((_czs_x) == 0) _throw_null(); \
  if (_czs_i < 0) _throw_arraybounds(); \
  for (_czs_temp=_czs_sz; _czs_temp < _czs_i; _czs_temp++) \
    if (_czs_x[_czs_temp] == 0) _throw_arraybounds(); \
  _czs_x+_czs_i; })
#endif

// Calculates the number of elements in a zero-terminated, thin array.
// If non-null, the array is guaranteed to have orig_offset elements.
#define _get_zero_arr_size(orig_x,orig_offset) ({ \
  typedef _gres_tx = (*orig_x); \
  _gres_tx *_gres_x = (_gres_tx *)(orig_x); \
  unsigned int _gres_offset = (orig_offset); \
  unsigned int _gres = 0; \
  if (_gres_x != 0) { \
     _gres = _gres_offset; \
     _gres_x += _gres_offset - 1; \
     while (*_gres_x != 0) { _gres_x++; _gres++; } \
  } _gres; })

// Does in-place addition of a zero-terminated pointer (x += e and ++x).  
// Note that this expands to call _zero_arr_plus.
#define _zero_arr_inplace_plus(x,orig_i) ({ \
  typedef _zap_tx = (*x); \
  _zap_tx **_zap_x = &((_zap_tx*)x); \
  *_zap_x = _zero_arr_plus(*_zap_x,1,(orig_i)); })

// Does in-place increment of a zero-terminated pointer (e.g., x++).
// Note that this expands to call _zero_arr_plus.
#define _zero_arr_inplace_plus_post(x,orig_i) ({ \
  typedef _zap_tx = (*x); \
  _zap_tx **_zap_x = &((_zap_tx*)x); \
  _zap_tx *_zap_res = *_zap_x; \
  *_zap_x = _zero_arr_plus(_zap_res,1,(orig_i)); \
  _zap_res; })
  
//// Allocation
extern void* GC_malloc(int);
extern void* GC_malloc_atomic(int);
extern void* GC_calloc(unsigned,unsigned);
extern void* GC_calloc_atomic(unsigned,unsigned);

static inline void* _cycalloc(int n) {
  void * ans = (void *)GC_malloc(n);
  if(!ans)
    _throw_badalloc();
  return ans;
}
static inline void* _cycalloc_atomic(int n) {
  void * ans = (void *)GC_malloc_atomic(n);
  if(!ans)
    _throw_badalloc();
  return ans;
}
static inline void* _cyccalloc(unsigned n, unsigned s) {
  void* ans = (void*)GC_calloc(n,s);
  if (!ans)
    _throw_badalloc();
  return ans;
}
static inline void* _cyccalloc_atomic(unsigned n, unsigned s) {
  void* ans = (void*)GC_calloc_atomic(n,s);
  if (!ans)
    _throw_badalloc();
  return ans;
}
#define MAX_MALLOC_SIZE (1 << 28)
static inline unsigned int _check_times(unsigned x, unsigned y) {
  unsigned long long whole_ans = 
    ((unsigned long long)x)*((unsigned long long)y);
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
 struct Cyc_Core_Opt{void*v;};extern char Cyc_Core_Invalid_argument[21];struct Cyc_Core_Invalid_argument_struct{
char*tag;struct _tagged_arr f1;};extern char Cyc_Core_Failure[12];struct Cyc_Core_Failure_struct{
char*tag;struct _tagged_arr f1;};extern char Cyc_Core_Impossible[15];struct Cyc_Core_Impossible_struct{
char*tag;struct _tagged_arr f1;};extern char Cyc_Core_Not_found[14];extern char Cyc_Core_Unreachable[
16];struct Cyc_Core_Unreachable_struct{char*tag;struct _tagged_arr f1;};struct Cyc_List_List{
void*hd;struct Cyc_List_List*tl;};struct Cyc_List_List*Cyc_List_map(void*(*f)(void*),
struct Cyc_List_List*x);extern char Cyc_List_List_mismatch[18];struct Cyc_List_List*
Cyc_List_imp_rev(struct Cyc_List_List*x);extern char Cyc_List_Nth[8];int Cyc_List_list_cmp(
int(*cmp)(void*,void*),struct Cyc_List_List*l1,struct Cyc_List_List*l2);struct Cyc_Lineno_Pos{
struct _tagged_arr logical_file;struct _tagged_arr line;int line_no;int col;};extern
char Cyc_Position_Exit[9];struct Cyc_Position_Segment;struct Cyc_Position_Error{
struct _tagged_arr source;struct Cyc_Position_Segment*seg;void*kind;struct
_tagged_arr desc;};extern char Cyc_Position_Nocontext[14];struct Cyc_Absyn_Rel_n_struct{
int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Abs_n_struct{int tag;struct Cyc_List_List*
f1;};struct _tuple0{void*f1;struct _tagged_arr*f2;};struct Cyc_Absyn_Conref;struct
Cyc_Absyn_Tqual{int print_const: 1;int q_volatile: 1;int q_restrict: 1;int real_const: 1;
};struct Cyc_Absyn_Conref{void*v;};struct Cyc_Absyn_Eq_constr_struct{int tag;void*
f1;};struct Cyc_Absyn_Forward_constr_struct{int tag;struct Cyc_Absyn_Conref*f1;};
struct Cyc_Absyn_Eq_kb_struct{int tag;void*f1;};struct Cyc_Absyn_Unknown_kb_struct{
int tag;struct Cyc_Core_Opt*f1;};struct Cyc_Absyn_Less_kb_struct{int tag;struct Cyc_Core_Opt*
f1;void*f2;};struct Cyc_Absyn_Tvar{struct _tagged_arr*name;int*identity;void*kind;
};struct Cyc_Absyn_Upper_b_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_AbsUpper_b_struct{
int tag;void*f1;};struct Cyc_Absyn_PtrAtts{void*rgn;struct Cyc_Absyn_Conref*
nullable;struct Cyc_Absyn_Conref*bounds;struct Cyc_Absyn_Conref*zero_term;};struct
Cyc_Absyn_PtrInfo{void*elt_typ;struct Cyc_Absyn_Tqual elt_tq;struct Cyc_Absyn_PtrAtts
ptr_atts;};struct Cyc_Absyn_VarargInfo{struct Cyc_Core_Opt*name;struct Cyc_Absyn_Tqual
tq;void*type;int inject;};struct Cyc_Absyn_FnInfo{struct Cyc_List_List*tvars;struct
Cyc_Core_Opt*effect;void*ret_typ;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*
cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_List_List*attributes;};struct
Cyc_Absyn_UnknownTunionInfo{struct _tuple0*name;int is_xtunion;};struct Cyc_Absyn_UnknownTunion_struct{
int tag;struct Cyc_Absyn_UnknownTunionInfo f1;};struct Cyc_Absyn_KnownTunion_struct{
int tag;struct Cyc_Absyn_Tuniondecl**f1;};struct Cyc_Absyn_TunionInfo{void*
tunion_info;struct Cyc_List_List*targs;void*rgn;};struct Cyc_Absyn_UnknownTunionFieldInfo{
struct _tuple0*tunion_name;struct _tuple0*field_name;int is_xtunion;};struct Cyc_Absyn_UnknownTunionfield_struct{
int tag;struct Cyc_Absyn_UnknownTunionFieldInfo f1;};struct Cyc_Absyn_KnownTunionfield_struct{
int tag;struct Cyc_Absyn_Tuniondecl*f1;struct Cyc_Absyn_Tunionfield*f2;};struct Cyc_Absyn_TunionFieldInfo{
void*field_info;struct Cyc_List_List*targs;};struct Cyc_Absyn_UnknownAggr_struct{
int tag;void*f1;struct _tuple0*f2;};struct Cyc_Absyn_KnownAggr_struct{int tag;struct
Cyc_Absyn_Aggrdecl**f1;};struct Cyc_Absyn_AggrInfo{void*aggr_info;struct Cyc_List_List*
targs;};struct Cyc_Absyn_ArrayInfo{void*elt_type;struct Cyc_Absyn_Tqual tq;struct
Cyc_Absyn_Exp*num_elts;struct Cyc_Absyn_Conref*zero_term;};struct Cyc_Absyn_Evar_struct{
int tag;struct Cyc_Core_Opt*f1;struct Cyc_Core_Opt*f2;int f3;struct Cyc_Core_Opt*f4;}
;struct Cyc_Absyn_VarType_struct{int tag;struct Cyc_Absyn_Tvar*f1;};struct Cyc_Absyn_TunionType_struct{
int tag;struct Cyc_Absyn_TunionInfo f1;};struct Cyc_Absyn_TunionFieldType_struct{int
tag;struct Cyc_Absyn_TunionFieldInfo f1;};struct Cyc_Absyn_PointerType_struct{int
tag;struct Cyc_Absyn_PtrInfo f1;};struct Cyc_Absyn_IntType_struct{int tag;void*f1;
void*f2;};struct Cyc_Absyn_DoubleType_struct{int tag;int f1;};struct Cyc_Absyn_ArrayType_struct{
int tag;struct Cyc_Absyn_ArrayInfo f1;};struct Cyc_Absyn_FnType_struct{int tag;struct
Cyc_Absyn_FnInfo f1;};struct Cyc_Absyn_TupleType_struct{int tag;struct Cyc_List_List*
f1;};struct Cyc_Absyn_AggrType_struct{int tag;struct Cyc_Absyn_AggrInfo f1;};struct
Cyc_Absyn_AnonAggrType_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_EnumType_struct{
int tag;struct _tuple0*f1;struct Cyc_Absyn_Enumdecl*f2;};struct Cyc_Absyn_AnonEnumType_struct{
int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_SizeofType_struct{int tag;void*f1;
};struct Cyc_Absyn_RgnHandleType_struct{int tag;void*f1;};struct Cyc_Absyn_TypedefType_struct{
int tag;struct _tuple0*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_Typedefdecl*f3;
void**f4;};struct Cyc_Absyn_TagType_struct{int tag;void*f1;};struct Cyc_Absyn_TypeInt_struct{
int tag;int f1;};struct Cyc_Absyn_AccessEff_struct{int tag;void*f1;};struct Cyc_Absyn_JoinEff_struct{
int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_RgnsEff_struct{int tag;void*f1;};
struct Cyc_Absyn_NoTypes_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Position_Segment*
f2;};struct Cyc_Absyn_WithTypes_struct{int tag;struct Cyc_List_List*f1;int f2;struct
Cyc_Absyn_VarargInfo*f3;struct Cyc_Core_Opt*f4;struct Cyc_List_List*f5;};struct Cyc_Absyn_Regparm_att_struct{
int tag;int f1;};struct Cyc_Absyn_Aligned_att_struct{int tag;int f1;};struct Cyc_Absyn_Section_att_struct{
int tag;struct _tagged_arr f1;};struct Cyc_Absyn_Format_att_struct{int tag;void*f1;
int f2;int f3;};struct Cyc_Absyn_Initializes_att_struct{int tag;int f1;};struct Cyc_Absyn_Mode_att_struct{
int tag;struct _tagged_arr f1;};struct Cyc_Absyn_Carray_mod_struct{int tag;struct Cyc_Absyn_Conref*
f1;};struct Cyc_Absyn_ConstArray_mod_struct{int tag;struct Cyc_Absyn_Exp*f1;struct
Cyc_Absyn_Conref*f2;};struct Cyc_Absyn_Pointer_mod_struct{int tag;struct Cyc_Absyn_PtrAtts
f1;struct Cyc_Absyn_Tqual f2;};struct Cyc_Absyn_Function_mod_struct{int tag;void*f1;
};struct Cyc_Absyn_TypeParams_mod_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Position_Segment*
f2;int f3;};struct Cyc_Absyn_Attributes_mod_struct{int tag;struct Cyc_Position_Segment*
f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Char_c_struct{int tag;void*f1;char f2;
};struct Cyc_Absyn_Short_c_struct{int tag;void*f1;short f2;};struct Cyc_Absyn_Int_c_struct{
int tag;void*f1;int f2;};struct Cyc_Absyn_LongLong_c_struct{int tag;void*f1;
long long f2;};struct Cyc_Absyn_Float_c_struct{int tag;struct _tagged_arr f1;};struct
Cyc_Absyn_String_c_struct{int tag;struct _tagged_arr f1;};struct Cyc_Absyn_VarargCallInfo{
int num_varargs;struct Cyc_List_List*injectors;struct Cyc_Absyn_VarargInfo*vai;};
struct Cyc_Absyn_StructField_struct{int tag;struct _tagged_arr*f1;};struct Cyc_Absyn_TupleIndex_struct{
int tag;unsigned int f1;};struct Cyc_Absyn_MallocInfo{int is_calloc;struct Cyc_Absyn_Exp*
rgn;void**elt_type;struct Cyc_Absyn_Exp*num_elts;int fat_result;};struct Cyc_Absyn_Const_e_struct{
int tag;void*f1;};struct Cyc_Absyn_Var_e_struct{int tag;struct _tuple0*f1;void*f2;};
struct Cyc_Absyn_UnknownId_e_struct{int tag;struct _tuple0*f1;};struct Cyc_Absyn_Primop_e_struct{
int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_AssignOp_e_struct{int tag;
struct Cyc_Absyn_Exp*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Increment_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;void*f2;};struct Cyc_Absyn_Conditional_e_struct{int
tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;};
struct Cyc_Absyn_SeqExp_e_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*
f2;};struct Cyc_Absyn_UnknownCall_e_struct{int tag;struct Cyc_Absyn_Exp*f1;struct
Cyc_List_List*f2;};struct Cyc_Absyn_FnCall_e_struct{int tag;struct Cyc_Absyn_Exp*f1;
struct Cyc_List_List*f2;struct Cyc_Absyn_VarargCallInfo*f3;};struct Cyc_Absyn_Throw_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_NoInstantiate_e_struct{int tag;
struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Instantiate_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Cast_e_struct{int tag;void*f1;struct
Cyc_Absyn_Exp*f2;int f3;void*f4;};struct Cyc_Absyn_Address_e_struct{int tag;struct
Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_New_e_struct{int tag;struct Cyc_Absyn_Exp*f1;
struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Sizeoftyp_e_struct{int tag;void*f1;};
struct Cyc_Absyn_Sizeofexp_e_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Offsetof_e_struct{
int tag;void*f1;void*f2;};struct Cyc_Absyn_Gentyp_e_struct{int tag;struct Cyc_List_List*
f1;void*f2;};struct Cyc_Absyn_Deref_e_struct{int tag;struct Cyc_Absyn_Exp*f1;};
struct Cyc_Absyn_AggrMember_e_struct{int tag;struct Cyc_Absyn_Exp*f1;struct
_tagged_arr*f2;};struct Cyc_Absyn_AggrArrow_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;struct _tagged_arr*f2;};struct Cyc_Absyn_Subscript_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Tuple_e_struct{int tag;struct Cyc_List_List*
f1;};struct _tuple1{struct Cyc_Core_Opt*f1;struct Cyc_Absyn_Tqual f2;void*f3;};
struct Cyc_Absyn_CompoundLit_e_struct{int tag;struct _tuple1*f1;struct Cyc_List_List*
f2;};struct Cyc_Absyn_Array_e_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Comprehension_e_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;
int f4;};struct Cyc_Absyn_Struct_e_struct{int tag;struct _tuple0*f1;struct Cyc_List_List*
f2;struct Cyc_List_List*f3;struct Cyc_Absyn_Aggrdecl*f4;};struct Cyc_Absyn_AnonStruct_e_struct{
int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Tunion_e_struct{int tag;
struct Cyc_List_List*f1;struct Cyc_Absyn_Tuniondecl*f2;struct Cyc_Absyn_Tunionfield*
f3;};struct Cyc_Absyn_Enum_e_struct{int tag;struct _tuple0*f1;struct Cyc_Absyn_Enumdecl*
f2;struct Cyc_Absyn_Enumfield*f3;};struct Cyc_Absyn_AnonEnum_e_struct{int tag;
struct _tuple0*f1;void*f2;struct Cyc_Absyn_Enumfield*f3;};struct Cyc_Absyn_Malloc_e_struct{
int tag;struct Cyc_Absyn_MallocInfo f1;};struct Cyc_Absyn_UnresolvedMem_e_struct{int
tag;struct Cyc_Core_Opt*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_StmtExp_e_struct{
int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Codegen_e_struct{int tag;struct
Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Fill_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;};struct Cyc_Absyn_Exp{struct Cyc_Core_Opt*topt;void*r;struct Cyc_Position_Segment*
loc;void*annot;};struct _tuple2{struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;};
struct Cyc_Absyn_ForArrayInfo{struct Cyc_List_List*defns;struct _tuple2 condition;
struct _tuple2 delta;struct Cyc_Absyn_Stmt*body;};struct Cyc_Absyn_Exp_s_struct{int
tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Seq_s_struct{int tag;struct Cyc_Absyn_Stmt*
f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Return_s_struct{int tag;struct Cyc_Absyn_Exp*
f1;};struct Cyc_Absyn_IfThenElse_s_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*
f2;struct Cyc_Absyn_Stmt*f3;};struct Cyc_Absyn_While_s_struct{int tag;struct _tuple2
f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Break_s_struct{int tag;struct Cyc_Absyn_Stmt*
f1;};struct Cyc_Absyn_Continue_s_struct{int tag;struct Cyc_Absyn_Stmt*f1;};struct
Cyc_Absyn_Goto_s_struct{int tag;struct _tagged_arr*f1;struct Cyc_Absyn_Stmt*f2;};
struct Cyc_Absyn_For_s_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _tuple2 f2;
struct _tuple2 f3;struct Cyc_Absyn_Stmt*f4;};struct Cyc_Absyn_Switch_s_struct{int tag;
struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_SwitchC_s_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Fallthru_s_struct{
int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Switch_clause**f2;};struct Cyc_Absyn_Decl_s_struct{
int tag;struct Cyc_Absyn_Decl*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Cut_s_struct{
int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Splice_s_struct{int tag;struct
Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Label_s_struct{int tag;struct _tagged_arr*f1;
struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Do_s_struct{int tag;struct Cyc_Absyn_Stmt*
f1;struct _tuple2 f2;};struct Cyc_Absyn_TryCatch_s_struct{int tag;struct Cyc_Absyn_Stmt*
f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Region_s_struct{int tag;struct Cyc_Absyn_Tvar*
f1;struct Cyc_Absyn_Vardecl*f2;int f3;struct Cyc_Absyn_Stmt*f4;};struct Cyc_Absyn_ForArray_s_struct{
int tag;struct Cyc_Absyn_ForArrayInfo f1;};struct Cyc_Absyn_ResetRegion_s_struct{int
tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Stmt{void*r;struct Cyc_Position_Segment*
loc;struct Cyc_List_List*non_local_preds;int try_depth;void*annot;};struct Cyc_Absyn_Var_p_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Reference_p_struct{int tag;
struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_TagInt_p_struct{int tag;struct Cyc_Absyn_Tvar*
f1;struct Cyc_Absyn_Vardecl*f2;};struct Cyc_Absyn_Tuple_p_struct{int tag;struct Cyc_List_List*
f1;};struct Cyc_Absyn_Pointer_p_struct{int tag;struct Cyc_Absyn_Pat*f1;};struct Cyc_Absyn_Aggr_p_struct{
int tag;struct Cyc_Absyn_AggrInfo f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;
};struct Cyc_Absyn_Tunion_p_struct{int tag;struct Cyc_Absyn_Tuniondecl*f1;struct Cyc_Absyn_Tunionfield*
f2;struct Cyc_List_List*f3;};struct Cyc_Absyn_Int_p_struct{int tag;void*f1;int f2;};
struct Cyc_Absyn_Char_p_struct{int tag;char f1;};struct Cyc_Absyn_Float_p_struct{int
tag;struct _tagged_arr f1;};struct Cyc_Absyn_Enum_p_struct{int tag;struct Cyc_Absyn_Enumdecl*
f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_AnonEnum_p_struct{int tag;void*
f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_UnknownId_p_struct{int tag;
struct _tuple0*f1;};struct Cyc_Absyn_UnknownCall_p_struct{int tag;struct _tuple0*f1;
struct Cyc_List_List*f2;};struct Cyc_Absyn_Pat{void*r;struct Cyc_Core_Opt*topt;
struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Switch_clause{struct Cyc_Absyn_Pat*
pattern;struct Cyc_Core_Opt*pat_vars;struct Cyc_Absyn_Exp*where_clause;struct Cyc_Absyn_Stmt*
body;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_SwitchC_clause{struct Cyc_Absyn_Exp*
cnst_exp;struct Cyc_Absyn_Stmt*body;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Global_b_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Funname_b_struct{int tag;
struct Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Param_b_struct{int tag;struct Cyc_Absyn_Vardecl*
f1;};struct Cyc_Absyn_Local_b_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct
Cyc_Absyn_Pat_b_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Vardecl{
void*sc;struct _tuple0*name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*
initializer;struct Cyc_Core_Opt*rgn;struct Cyc_List_List*attributes;int escapes;};
struct Cyc_Absyn_Fndecl{void*sc;int is_inline;struct _tuple0*name;struct Cyc_List_List*
tvs;struct Cyc_Core_Opt*effect;void*ret_type;struct Cyc_List_List*args;int
c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*rgn_po;
struct Cyc_Absyn_Stmt*body;struct Cyc_Core_Opt*cached_typ;struct Cyc_Core_Opt*
param_vardecls;struct Cyc_List_List*attributes;};struct Cyc_Absyn_Aggrfield{struct
_tagged_arr*name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*width;
struct Cyc_List_List*attributes;};struct Cyc_Absyn_AggrdeclImpl{struct Cyc_List_List*
exist_vars;struct Cyc_List_List*rgn_po;struct Cyc_List_List*fields;};struct Cyc_Absyn_Aggrdecl{
void*kind;void*sc;struct _tuple0*name;struct Cyc_List_List*tvs;struct Cyc_Absyn_AggrdeclImpl*
impl;struct Cyc_List_List*attributes;};struct Cyc_Absyn_Tunionfield{struct _tuple0*
name;struct Cyc_List_List*typs;struct Cyc_Position_Segment*loc;void*sc;};struct Cyc_Absyn_Tuniondecl{
void*sc;struct _tuple0*name;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*fields;int
is_xtunion;};struct Cyc_Absyn_Enumfield{struct _tuple0*name;struct Cyc_Absyn_Exp*
tag;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Enumdecl{void*sc;struct
_tuple0*name;struct Cyc_Core_Opt*fields;};struct Cyc_Absyn_Typedefdecl{struct
_tuple0*name;struct Cyc_Absyn_Tqual tq;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*
kind;struct Cyc_Core_Opt*defn;struct Cyc_List_List*atts;};struct Cyc_Absyn_Var_d_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Fn_d_struct{int tag;struct Cyc_Absyn_Fndecl*
f1;};struct Cyc_Absyn_Let_d_struct{int tag;struct Cyc_Absyn_Pat*f1;struct Cyc_Core_Opt*
f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Letv_d_struct{int tag;struct Cyc_List_List*
f1;};struct Cyc_Absyn_Aggr_d_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct
Cyc_Absyn_Tunion_d_struct{int tag;struct Cyc_Absyn_Tuniondecl*f1;};struct Cyc_Absyn_Enum_d_struct{
int tag;struct Cyc_Absyn_Enumdecl*f1;};struct Cyc_Absyn_Typedef_d_struct{int tag;
struct Cyc_Absyn_Typedefdecl*f1;};struct Cyc_Absyn_Namespace_d_struct{int tag;
struct _tagged_arr*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Using_d_struct{int
tag;struct _tuple0*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_ExternC_d_struct{
int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Decl{void*r;struct Cyc_Position_Segment*
loc;};struct Cyc_Absyn_ArrayElement_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct
Cyc_Absyn_FieldName_struct{int tag;struct _tagged_arr*f1;};char Cyc_Absyn_EmptyAnnot[
15]="\000\000\000\000EmptyAnnot\000";int Cyc_Absyn_qvar_cmp(struct _tuple0*,struct
_tuple0*);int Cyc_Absyn_varlist_cmp(struct Cyc_List_List*,struct Cyc_List_List*);
int Cyc_Absyn_tvar_cmp(struct Cyc_Absyn_Tvar*,struct Cyc_Absyn_Tvar*);extern struct
Cyc_Absyn_Rel_n_struct Cyc_Absyn_rel_ns_null_value;extern void*Cyc_Absyn_rel_ns_null;
int Cyc_Absyn_is_qvar_qualified(struct _tuple0*);struct Cyc_Absyn_Tqual Cyc_Absyn_const_tqual();
struct Cyc_Absyn_Tqual Cyc_Absyn_combine_tqual(struct Cyc_Absyn_Tqual x,struct Cyc_Absyn_Tqual
y);struct Cyc_Absyn_Tqual Cyc_Absyn_empty_tqual();struct Cyc_Absyn_Conref*Cyc_Absyn_new_conref(
void*x);struct Cyc_Absyn_Conref*Cyc_Absyn_empty_conref();struct Cyc_Absyn_Conref*
Cyc_Absyn_compress_conref(struct Cyc_Absyn_Conref*x);void*Cyc_Absyn_conref_val(
struct Cyc_Absyn_Conref*x);void*Cyc_Absyn_conref_def(void*,struct Cyc_Absyn_Conref*
x);extern struct Cyc_Absyn_Conref*Cyc_Absyn_true_conref;extern struct Cyc_Absyn_Conref*
Cyc_Absyn_false_conref;extern struct Cyc_Absyn_Conref*Cyc_Absyn_bounds_one_conref;
extern struct Cyc_Absyn_Conref*Cyc_Absyn_bounds_unknown_conref;void*Cyc_Absyn_compress_kb(
void*);void*Cyc_Absyn_force_kb(void*kb);void*Cyc_Absyn_new_evar(struct Cyc_Core_Opt*
k,struct Cyc_Core_Opt*tenv);void*Cyc_Absyn_wildtyp(struct Cyc_Core_Opt*);extern
void*Cyc_Absyn_char_typ;extern void*Cyc_Absyn_uchar_typ;extern void*Cyc_Absyn_ushort_typ;
extern void*Cyc_Absyn_uint_typ;extern void*Cyc_Absyn_ulong_typ;extern void*Cyc_Absyn_ulonglong_typ;
extern void*Cyc_Absyn_schar_typ;extern void*Cyc_Absyn_sshort_typ;extern void*Cyc_Absyn_sint_typ;
extern void*Cyc_Absyn_slong_typ;extern void*Cyc_Absyn_slonglong_typ;extern void*Cyc_Absyn_float_typ;
void*Cyc_Absyn_double_typ(int);extern void*Cyc_Absyn_empty_effect;extern struct
_tuple0*Cyc_Absyn_exn_name;extern struct Cyc_Absyn_Tuniondecl*Cyc_Absyn_exn_tud;
extern void*Cyc_Absyn_exn_typ;extern struct _tuple0*Cyc_Absyn_tunion_print_arg_qvar;
extern struct _tuple0*Cyc_Absyn_tunion_scanf_arg_qvar;void*Cyc_Absyn_string_typ(
void*rgn);void*Cyc_Absyn_const_string_typ(void*rgn);void*Cyc_Absyn_file_typ();
extern struct Cyc_Absyn_Exp*Cyc_Absyn_exp_unsigned_one;extern void*Cyc_Absyn_bounds_one;
void*Cyc_Absyn_starb_typ(void*t,void*rgn,struct Cyc_Absyn_Tqual tq,void*b,struct
Cyc_Absyn_Conref*zero_term);void*Cyc_Absyn_atb_typ(void*t,void*rgn,struct Cyc_Absyn_Tqual
tq,void*b,struct Cyc_Absyn_Conref*zero_term);void*Cyc_Absyn_star_typ(void*t,void*
rgn,struct Cyc_Absyn_Tqual tq,struct Cyc_Absyn_Conref*zero_term);void*Cyc_Absyn_at_typ(
void*t,void*rgn,struct Cyc_Absyn_Tqual tq,struct Cyc_Absyn_Conref*zero_term);void*
Cyc_Absyn_cstar_typ(void*t,struct Cyc_Absyn_Tqual tq);void*Cyc_Absyn_tagged_typ(
void*t,void*rgn,struct Cyc_Absyn_Tqual tq,struct Cyc_Absyn_Conref*zero_term);void*
Cyc_Absyn_void_star_typ();void*Cyc_Absyn_strct(struct _tagged_arr*name);void*Cyc_Absyn_strctq(
struct _tuple0*name);void*Cyc_Absyn_unionq_typ(struct _tuple0*name);void*Cyc_Absyn_union_typ(
struct _tagged_arr*name);void*Cyc_Absyn_array_typ(void*elt_type,struct Cyc_Absyn_Tqual
tq,struct Cyc_Absyn_Exp*num_elts,struct Cyc_Absyn_Conref*zero_term);struct Cyc_Absyn_Exp*
Cyc_Absyn_new_exp(void*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_New_exp(
struct Cyc_Absyn_Exp*rgn_handle,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_copy_exp(struct Cyc_Absyn_Exp*);struct Cyc_Absyn_Exp*
Cyc_Absyn_const_exp(void*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_null_exp(
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_bool_exp(int,struct
Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_true_exp(struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_false_exp(struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*
Cyc_Absyn_int_exp(void*,int,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_signed_int_exp(
int,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_uint_exp(
unsigned int,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_char_exp(
char c,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_float_exp(
struct _tagged_arr f,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_string_exp(
struct _tagged_arr s,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_var_exp(
struct _tuple0*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_varb_exp(
struct _tuple0*,void*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_unknownid_exp(
struct _tuple0*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_primop_exp(
void*,struct Cyc_List_List*es,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*
Cyc_Absyn_prim1_exp(void*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_prim2_exp(void*,struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_add_exp(struct Cyc_Absyn_Exp*,
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_times_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*
Cyc_Absyn_divide_exp(struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_eq_exp(struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_neq_exp(struct Cyc_Absyn_Exp*,
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_gt_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*
Cyc_Absyn_lt_exp(struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_gte_exp(struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_lte_exp(struct Cyc_Absyn_Exp*,
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_assignop_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Core_Opt*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_assign_exp(struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_increment_exp(struct
Cyc_Absyn_Exp*,void*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_post_inc_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_post_dec_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_pre_inc_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_pre_dec_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_conditional_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_and_exp(struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_or_exp(struct Cyc_Absyn_Exp*,
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_seq_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*
Cyc_Absyn_unknowncall_exp(struct Cyc_Absyn_Exp*,struct Cyc_List_List*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_fncall_exp(struct Cyc_Absyn_Exp*,struct Cyc_List_List*,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_throw_exp(struct Cyc_Absyn_Exp*,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_noinstantiate_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_instantiate_exp(
struct Cyc_Absyn_Exp*,struct Cyc_List_List*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*
Cyc_Absyn_cast_exp(void*,struct Cyc_Absyn_Exp*,int user_cast,void*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_address_exp(struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_sizeoftyp_exp(void*t,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_sizeofexp_exp(struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_offsetof_exp(void*,void*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_gentyp_exp(struct Cyc_List_List*,void*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_deref_exp(struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_aggrmember_exp(struct Cyc_Absyn_Exp*,struct
_tagged_arr*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_aggrarrow_exp(
struct Cyc_Absyn_Exp*,struct _tagged_arr*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*
Cyc_Absyn_subscript_exp(struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_tuple_exp(struct Cyc_List_List*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_stmt_exp(struct Cyc_Absyn_Stmt*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_match_exn_exp(struct Cyc_Position_Segment*);struct
Cyc_Absyn_Exp*Cyc_Absyn_array_exp(struct Cyc_List_List*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_unresolvedmem_exp(struct Cyc_Core_Opt*,struct Cyc_List_List*,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Stmt*Cyc_Absyn_new_stmt(void*s,
struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_skip_stmt(struct
Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_exp_stmt(struct Cyc_Absyn_Exp*
e,struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_seq_stmt(struct
Cyc_Absyn_Stmt*s1,struct Cyc_Absyn_Stmt*s2,struct Cyc_Position_Segment*loc);struct
Cyc_Absyn_Stmt*Cyc_Absyn_seq_stmts(struct Cyc_List_List*,struct Cyc_Position_Segment*
loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_return_stmt(struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*
loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_ifthenelse_stmt(struct Cyc_Absyn_Exp*e,struct
Cyc_Absyn_Stmt*s1,struct Cyc_Absyn_Stmt*s2,struct Cyc_Position_Segment*loc);struct
Cyc_Absyn_Stmt*Cyc_Absyn_while_stmt(struct Cyc_Absyn_Exp*e,struct Cyc_Absyn_Stmt*s,
struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_break_stmt(struct
Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_continue_stmt(struct Cyc_Position_Segment*
loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_for_stmt(struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*
e2,struct Cyc_Absyn_Exp*e3,struct Cyc_Absyn_Stmt*s,struct Cyc_Position_Segment*loc);
struct Cyc_Absyn_Stmt*Cyc_Absyn_switch_stmt(struct Cyc_Absyn_Exp*e,struct Cyc_List_List*,
struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_fallthru_stmt(
struct Cyc_List_List*el,struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_decl_stmt(
struct Cyc_Absyn_Decl*d,struct Cyc_Absyn_Stmt*s,struct Cyc_Position_Segment*loc);
struct Cyc_Absyn_Stmt*Cyc_Absyn_declare_stmt(struct _tuple0*,void*,struct Cyc_Absyn_Exp*
init,struct Cyc_Absyn_Stmt*,struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*
Cyc_Absyn_cut_stmt(struct Cyc_Absyn_Stmt*s,struct Cyc_Position_Segment*loc);struct
Cyc_Absyn_Stmt*Cyc_Absyn_splice_stmt(struct Cyc_Absyn_Stmt*s,struct Cyc_Position_Segment*
loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_label_stmt(struct _tagged_arr*v,struct Cyc_Absyn_Stmt*
s,struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_do_stmt(struct
Cyc_Absyn_Stmt*s,struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*
Cyc_Absyn_goto_stmt(struct _tagged_arr*lab,struct Cyc_Position_Segment*loc);struct
Cyc_Absyn_Stmt*Cyc_Absyn_assign_stmt(struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*
e2,struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_trycatch_stmt(
struct Cyc_Absyn_Stmt*,struct Cyc_List_List*,struct Cyc_Position_Segment*);struct
Cyc_Absyn_Stmt*Cyc_Absyn_forarray_stmt(struct Cyc_List_List*,struct Cyc_Absyn_Exp*,
struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Stmt*,struct Cyc_Position_Segment*);struct
Cyc_Absyn_Pat*Cyc_Absyn_new_pat(void*p,struct Cyc_Position_Segment*s);struct Cyc_Absyn_Decl*
Cyc_Absyn_new_decl(void*r,struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Decl*
Cyc_Absyn_let_decl(struct Cyc_Absyn_Pat*p,struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*
loc);struct Cyc_Absyn_Decl*Cyc_Absyn_letv_decl(struct Cyc_List_List*,struct Cyc_Position_Segment*
loc);struct Cyc_Absyn_Vardecl*Cyc_Absyn_new_vardecl(struct _tuple0*x,void*t,struct
Cyc_Absyn_Exp*init);struct Cyc_Absyn_Vardecl*Cyc_Absyn_static_vardecl(struct
_tuple0*x,void*t,struct Cyc_Absyn_Exp*init);struct Cyc_Absyn_AggrdeclImpl*Cyc_Absyn_aggrdecl_impl(
struct Cyc_List_List*exists,struct Cyc_List_List*po,struct Cyc_List_List*fs);struct
Cyc_Absyn_Decl*Cyc_Absyn_aggr_decl(void*k,void*s,struct _tuple0*n,struct Cyc_List_List*
ts,struct Cyc_Absyn_AggrdeclImpl*i,struct Cyc_List_List*atts,struct Cyc_Position_Segment*
loc);struct Cyc_Absyn_Decl*Cyc_Absyn_struct_decl(void*s,struct _tuple0*n,struct Cyc_List_List*
ts,struct Cyc_Absyn_AggrdeclImpl*i,struct Cyc_List_List*atts,struct Cyc_Position_Segment*
loc);struct Cyc_Absyn_Decl*Cyc_Absyn_union_decl(void*s,struct _tuple0*n,struct Cyc_List_List*
ts,struct Cyc_Absyn_AggrdeclImpl*i,struct Cyc_List_List*atts,struct Cyc_Position_Segment*
loc);struct Cyc_Absyn_Decl*Cyc_Absyn_tunion_decl(void*s,struct _tuple0*n,struct Cyc_List_List*
ts,struct Cyc_Core_Opt*fs,int is_xtunion,struct Cyc_Position_Segment*loc);void*Cyc_Absyn_function_typ(
struct Cyc_List_List*tvs,struct Cyc_Core_Opt*eff_typ,void*ret_typ,struct Cyc_List_List*
args,int c_varargs,struct Cyc_Absyn_VarargInfo*cyc_varargs,struct Cyc_List_List*
rgn_po,struct Cyc_List_List*atts);void*Cyc_Absyn_pointer_expand(void*);int Cyc_Absyn_is_lvalue(
struct Cyc_Absyn_Exp*);struct Cyc_Absyn_Aggrfield*Cyc_Absyn_lookup_field(struct Cyc_List_List*,
struct _tagged_arr*);struct Cyc_Absyn_Aggrfield*Cyc_Absyn_lookup_decl_field(struct
Cyc_Absyn_Aggrdecl*,struct _tagged_arr*);struct _tuple3{struct Cyc_Absyn_Tqual f1;
void*f2;};struct _tuple3*Cyc_Absyn_lookup_tuple_field(struct Cyc_List_List*,int);
struct _tagged_arr Cyc_Absyn_attribute2string(void*);int Cyc_Absyn_fntype_att(void*
a);struct _tagged_arr*Cyc_Absyn_fieldname(int);struct _tuple4{void*f1;struct
_tuple0*f2;};struct _tuple4 Cyc_Absyn_aggr_kinded_name(void*);struct Cyc_Absyn_Aggrdecl*
Cyc_Absyn_get_known_aggrdecl(void*info);int Cyc_Absyn_is_union_type(void*);void
Cyc_Absyn_print_decls(struct Cyc_List_List*);struct Cyc_Typerep_Int_struct{int tag;
int f1;unsigned int f2;};struct Cyc_Typerep_ThinPtr_struct{int tag;unsigned int f1;
void*f2;};struct Cyc_Typerep_FatPtr_struct{int tag;void*f1;};struct _tuple5{
unsigned int f1;struct _tagged_arr f2;void*f3;};struct Cyc_Typerep_Struct_struct{int
tag;struct _tagged_arr*f1;unsigned int f2;struct _tagged_arr f3;};struct _tuple6{
unsigned int f1;void*f2;};struct Cyc_Typerep_Tuple_struct{int tag;unsigned int f1;
struct _tagged_arr f2;};struct _tuple7{unsigned int f1;struct _tagged_arr f2;};struct
Cyc_Typerep_TUnion_struct{int tag;struct _tagged_arr f1;struct _tagged_arr f2;struct
_tagged_arr f3;};struct Cyc_Typerep_TUnionField_struct{int tag;struct _tagged_arr f1;
struct _tagged_arr f2;unsigned int f3;struct _tagged_arr f4;};struct _tuple8{struct
_tagged_arr f1;void*f2;};struct Cyc_Typerep_XTUnion_struct{int tag;struct
_tagged_arr f1;struct _tagged_arr f2;};struct Cyc_Typerep_Union_struct{int tag;struct
_tagged_arr*f1;int f2;struct _tagged_arr f3;};struct Cyc_Typerep_Enum_struct{int tag;
struct _tagged_arr*f1;int f2;struct _tagged_arr f3;};unsigned int Cyc_Typerep_size_type(
void*rep);extern void*Cyc_decls_rep;extern struct Cyc_Typerep_Struct_struct Cyc_struct_Position_Segment_rep;
int Cyc_strptrcmp(struct _tagged_arr*s1,struct _tagged_arr*s2);struct Cyc___cycFILE;
struct Cyc_Cstdio___abstractFILE;struct Cyc_String_pa_struct{int tag;struct
_tagged_arr f1;};struct Cyc_Int_pa_struct{int tag;unsigned int f1;};struct Cyc_Double_pa_struct{
int tag;double f1;};struct Cyc_LongDouble_pa_struct{int tag;long double f1;};struct
Cyc_ShortPtr_pa_struct{int tag;short*f1;};struct Cyc_IntPtr_pa_struct{int tag;
unsigned int*f1;};struct _tagged_arr Cyc_aprintf(struct _tagged_arr,struct
_tagged_arr);struct Cyc_ShortPtr_sa_struct{int tag;short*f1;};struct Cyc_UShortPtr_sa_struct{
int tag;unsigned short*f1;};struct Cyc_IntPtr_sa_struct{int tag;int*f1;};struct Cyc_UIntPtr_sa_struct{
int tag;unsigned int*f1;};struct Cyc_StringPtr_sa_struct{int tag;struct _tagged_arr
f1;};struct Cyc_DoublePtr_sa_struct{int tag;double*f1;};struct Cyc_FloatPtr_sa_struct{
int tag;float*f1;};struct Cyc_CharPtr_sa_struct{int tag;struct _tagged_arr f1;};int
Cyc_printf(struct _tagged_arr,struct _tagged_arr);extern char Cyc_FileCloseError[19];
extern char Cyc_FileOpenError[18];struct Cyc_FileOpenError_struct{char*tag;struct
_tagged_arr f1;};struct Cyc_Iter_Iter{void*env;int(*next)(void*env,void*dest);};
int Cyc_Iter_next(struct Cyc_Iter_Iter,void*);struct Cyc_Set_Set;extern char Cyc_Set_Absent[
11];struct Cyc_Dict_Dict;extern char Cyc_Dict_Present[12];extern char Cyc_Dict_Absent[
11];struct _tuple9{void*f1;void*f2;};struct _tuple9*Cyc_Dict_rchoose(struct
_RegionHandle*r,struct Cyc_Dict_Dict*d);struct _tuple9*Cyc_Dict_rchoose(struct
_RegionHandle*,struct Cyc_Dict_Dict*d);struct Cyc_RgnOrder_RgnPO;struct Cyc_RgnOrder_RgnPO*
Cyc_RgnOrder_initial_fn_po(struct Cyc_List_List*tvs,struct Cyc_List_List*po,void*
effect,struct Cyc_Absyn_Tvar*fst_rgn);struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_add_outlives_constraint(
struct Cyc_RgnOrder_RgnPO*po,void*eff,void*rgn);struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_add_youngest(
struct Cyc_RgnOrder_RgnPO*po,struct Cyc_Absyn_Tvar*rgn,int resetable);int Cyc_RgnOrder_is_region_resetable(
struct Cyc_RgnOrder_RgnPO*po,struct Cyc_Absyn_Tvar*r);int Cyc_RgnOrder_effect_outlives(
struct Cyc_RgnOrder_RgnPO*po,void*eff,void*rgn);int Cyc_RgnOrder_satisfies_constraints(
struct Cyc_RgnOrder_RgnPO*po,struct Cyc_List_List*constraints,void*default_bound,
int do_pin);int Cyc_RgnOrder_eff_outlives_eff(struct Cyc_RgnOrder_RgnPO*po,void*
eff1,void*eff2);struct Cyc_Tcenv_VarRes_struct{int tag;void*f1;};struct Cyc_Tcenv_AggrRes_struct{
int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Tcenv_TunionRes_struct{int tag;
struct Cyc_Absyn_Tuniondecl*f1;struct Cyc_Absyn_Tunionfield*f2;};struct Cyc_Tcenv_EnumRes_struct{
int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Tcenv_AnonEnumRes_struct{
int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Tcenv_Genv{struct Cyc_Set_Set*
namespaces;struct Cyc_Dict_Dict*aggrdecls;struct Cyc_Dict_Dict*tuniondecls;struct
Cyc_Dict_Dict*enumdecls;struct Cyc_Dict_Dict*typedefs;struct Cyc_Dict_Dict*
ordinaries;struct Cyc_List_List*availables;};struct Cyc_Tcenv_Fenv;struct Cyc_Tcenv_Stmt_j_struct{
int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Tcenv_Outermost_struct{int tag;void*f1;
};struct Cyc_Tcenv_Frame_struct{int tag;void*f1;void*f2;};struct Cyc_Tcenv_Hidden_struct{
int tag;void*f1;void*f2;};struct Cyc_Tcenv_Tenv{struct Cyc_List_List*ns;struct Cyc_Dict_Dict*
ae;struct Cyc_Core_Opt*le;};void*Cyc_Tcutil_impos(struct _tagged_arr fmt,struct
_tagged_arr ap);void*Cyc_Tcutil_compress(void*t);void Cyc_Marshal_print_type(void*
rep,void*val);struct _tuple10{struct Cyc_Dict_Dict*f1;int f2;};struct _tuple11{
struct _tagged_arr f1;int f2;};static int Cyc_Absyn_strlist_cmp(struct Cyc_List_List*
ss1,struct Cyc_List_List*ss2){return((int(*)(int(*cmp)(struct _tagged_arr*,struct
_tagged_arr*),struct Cyc_List_List*l1,struct Cyc_List_List*l2))Cyc_List_list_cmp)(
Cyc_strptrcmp,ss1,ss2);}int Cyc_Absyn_varlist_cmp(struct Cyc_List_List*vs1,struct
Cyc_List_List*vs2){if((int)vs1 == (int)vs2)return 0;return Cyc_Absyn_strlist_cmp(
vs1,vs2);}int Cyc_Absyn_qvar_cmp(struct _tuple0*q1,struct _tuple0*q2){void*_tmp0=(*
q1).f1;void*_tmp1=(*q2).f1;{struct _tuple9 _tmp3=({struct _tuple9 _tmp2;_tmp2.f1=
_tmp0;_tmp2.f2=_tmp1;_tmp2;});void*_tmp4;void*_tmp5;void*_tmp6;struct Cyc_List_List*
_tmp7;void*_tmp8;struct Cyc_List_List*_tmp9;void*_tmpA;struct Cyc_List_List*_tmpB;
void*_tmpC;struct Cyc_List_List*_tmpD;void*_tmpE;void*_tmpF;void*_tmp10;void*
_tmp11;_LL1: _tmp4=_tmp3.f1;if((int)_tmp4 != 0)goto _LL3;_tmp5=_tmp3.f2;if((int)
_tmp5 != 0)goto _LL3;_LL2: goto _LL0;_LL3: _tmp6=_tmp3.f1;if(_tmp6 <= (void*)1?1:*((
int*)_tmp6)!= 0)goto _LL5;_tmp7=((struct Cyc_Absyn_Rel_n_struct*)_tmp6)->f1;_tmp8=
_tmp3.f2;if(_tmp8 <= (void*)1?1:*((int*)_tmp8)!= 0)goto _LL5;_tmp9=((struct Cyc_Absyn_Rel_n_struct*)
_tmp8)->f1;_LL4: _tmpB=_tmp7;_tmpD=_tmp9;goto _LL6;_LL5: _tmpA=_tmp3.f1;if(_tmpA <= (
void*)1?1:*((int*)_tmpA)!= 1)goto _LL7;_tmpB=((struct Cyc_Absyn_Abs_n_struct*)
_tmpA)->f1;_tmpC=_tmp3.f2;if(_tmpC <= (void*)1?1:*((int*)_tmpC)!= 1)goto _LL7;
_tmpD=((struct Cyc_Absyn_Abs_n_struct*)_tmpC)->f1;_LL6: {int i=Cyc_Absyn_strlist_cmp(
_tmpB,_tmpD);if(i != 0)return i;goto _LL0;}_LL7: _tmpE=_tmp3.f1;if((int)_tmpE != 0)
goto _LL9;_LL8: return - 1;_LL9: _tmpF=_tmp3.f2;if((int)_tmpF != 0)goto _LLB;_LLA:
return 1;_LLB: _tmp10=_tmp3.f1;if(_tmp10 <= (void*)1?1:*((int*)_tmp10)!= 0)goto _LLD;
_LLC: return - 1;_LLD: _tmp11=_tmp3.f2;if(_tmp11 <= (void*)1?1:*((int*)_tmp11)!= 0)
goto _LL0;_LLE: return 1;_LL0:;}return Cyc_strptrcmp((*q1).f2,(*q2).f2);}int Cyc_Absyn_tvar_cmp(
struct Cyc_Absyn_Tvar*tv1,struct Cyc_Absyn_Tvar*tv2){int i=Cyc_strptrcmp(tv1->name,
tv2->name);if(i != 0)return i;if(tv1->identity == tv2->identity)return 0;if(tv1->identity
!= 0?tv2->identity != 0: 0)return*((int*)_check_null(tv1->identity))- *((int*)
_check_null(tv2->identity));else{if(tv1->identity == 0)return - 1;else{return 1;}}}
struct Cyc_Absyn_Rel_n_struct Cyc_Absyn_rel_ns_null_value={0,0};void*Cyc_Absyn_rel_ns_null=(
void*)& Cyc_Absyn_rel_ns_null_value;int Cyc_Absyn_is_qvar_qualified(struct _tuple0*
qv){void*_tmp13=(*qv).f1;struct Cyc_List_List*_tmp14;struct Cyc_List_List*_tmp15;
_LL10: if(_tmp13 <= (void*)1?1:*((int*)_tmp13)!= 0)goto _LL12;_tmp14=((struct Cyc_Absyn_Rel_n_struct*)
_tmp13)->f1;if(_tmp14 != 0)goto _LL12;_LL11: goto _LL13;_LL12: if(_tmp13 <= (void*)1?1:*((
int*)_tmp13)!= 1)goto _LL14;_tmp15=((struct Cyc_Absyn_Abs_n_struct*)_tmp13)->f1;
if(_tmp15 != 0)goto _LL14;_LL13: goto _LL15;_LL14: if((int)_tmp13 != 0)goto _LL16;_LL15:
return 0;_LL16:;_LL17: return 1;_LLF:;}static int Cyc_Absyn_new_type_counter=0;void*
Cyc_Absyn_new_evar(struct Cyc_Core_Opt*k,struct Cyc_Core_Opt*env){return(void*)({
struct Cyc_Absyn_Evar_struct*_tmp16=_cycalloc(sizeof(*_tmp16));_tmp16[0]=({struct
Cyc_Absyn_Evar_struct _tmp17;_tmp17.tag=0;_tmp17.f1=k;_tmp17.f2=0;_tmp17.f3=Cyc_Absyn_new_type_counter
++;_tmp17.f4=env;_tmp17;});_tmp16;});}static struct Cyc_Core_Opt Cyc_Absyn_mk={(
void*)((void*)1)};void*Cyc_Absyn_wildtyp(struct Cyc_Core_Opt*tenv){return Cyc_Absyn_new_evar((
struct Cyc_Core_Opt*)& Cyc_Absyn_mk,tenv);}struct Cyc_Absyn_Tqual Cyc_Absyn_combine_tqual(
struct Cyc_Absyn_Tqual x,struct Cyc_Absyn_Tqual y){return({struct Cyc_Absyn_Tqual
_tmp18;_tmp18.print_const=x.print_const?1: y.print_const;_tmp18.q_volatile=x.q_volatile?
1: y.q_volatile;_tmp18.q_restrict=x.q_restrict?1: y.q_restrict;_tmp18.real_const=x.real_const?
1: y.real_const;_tmp18;});}struct Cyc_Absyn_Tqual Cyc_Absyn_empty_tqual(){return({
struct Cyc_Absyn_Tqual _tmp19;_tmp19.print_const=0;_tmp19.q_volatile=0;_tmp19.q_restrict=
0;_tmp19.real_const=0;_tmp19;});}struct Cyc_Absyn_Tqual Cyc_Absyn_const_tqual(){
return({struct Cyc_Absyn_Tqual _tmp1A;_tmp1A.print_const=1;_tmp1A.q_volatile=0;
_tmp1A.q_restrict=0;_tmp1A.real_const=1;_tmp1A;});}static struct Cyc_Absyn_Int_c_struct
Cyc_Absyn_one_intc={2,(void*)((void*)1),1};static struct Cyc_Absyn_Const_e_struct
Cyc_Absyn_one_b_raw={0,(void*)((void*)& Cyc_Absyn_one_intc)};struct Cyc_Absyn_Exp
Cyc_Absyn_exp_unsigned_one_v={0,(void*)((void*)& Cyc_Absyn_one_b_raw),0,(void*)((
void*)Cyc_Absyn_EmptyAnnot)};struct Cyc_Absyn_Exp*Cyc_Absyn_exp_unsigned_one=& Cyc_Absyn_exp_unsigned_one_v;
static struct Cyc_Absyn_Upper_b_struct Cyc_Absyn_one_bt={0,& Cyc_Absyn_exp_unsigned_one_v};
void*Cyc_Absyn_bounds_one=(void*)& Cyc_Absyn_one_bt;struct Cyc_Absyn_Conref*Cyc_Absyn_new_conref(
void*x){return({struct Cyc_Absyn_Conref*_tmp1E=_cycalloc(sizeof(*_tmp1E));_tmp1E->v=(
void*)((void*)({struct Cyc_Absyn_Eq_constr_struct*_tmp1F=_cycalloc(sizeof(*_tmp1F));
_tmp1F[0]=({struct Cyc_Absyn_Eq_constr_struct _tmp20;_tmp20.tag=0;_tmp20.f1=(void*)
x;_tmp20;});_tmp1F;}));_tmp1E;});}struct Cyc_Absyn_Conref*Cyc_Absyn_empty_conref(){
return({struct Cyc_Absyn_Conref*_tmp21=_cycalloc(sizeof(*_tmp21));_tmp21->v=(void*)((
void*)0);_tmp21;});}static struct Cyc_Absyn_Eq_constr_struct Cyc_Absyn_true_conref_constraint={
0,(void*)1};static struct Cyc_Absyn_Conref Cyc_Absyn_true_conref_v={(void*)((void*)&
Cyc_Absyn_true_conref_constraint)};struct Cyc_Absyn_Conref*Cyc_Absyn_true_conref=&
Cyc_Absyn_true_conref_v;static struct Cyc_Absyn_Eq_constr_struct Cyc_Absyn_false_conref_constraint={
0,(void*)0};static struct Cyc_Absyn_Conref Cyc_Absyn_false_conref_v={(void*)((void*)&
Cyc_Absyn_false_conref_constraint)};struct Cyc_Absyn_Conref*Cyc_Absyn_false_conref=&
Cyc_Absyn_false_conref_v;static struct Cyc_Absyn_Eq_constr_struct Cyc_Absyn_bounds_one_conref_constraint={
0,(void*)((void*)& Cyc_Absyn_one_bt)};static struct Cyc_Absyn_Conref Cyc_Absyn_bounds_one_conref_v={(
void*)((void*)& Cyc_Absyn_bounds_one_conref_constraint)};struct Cyc_Absyn_Conref*
Cyc_Absyn_bounds_one_conref=& Cyc_Absyn_bounds_one_conref_v;static struct Cyc_Absyn_Eq_constr_struct
Cyc_Absyn_bounds_unknown_conref_constraint={0,(void*)((void*)0)};static struct Cyc_Absyn_Conref
Cyc_Absyn_bounds_unknown_conref_v={(void*)((void*)& Cyc_Absyn_bounds_unknown_conref_constraint)};
struct Cyc_Absyn_Conref*Cyc_Absyn_bounds_unknown_conref=& Cyc_Absyn_bounds_unknown_conref_v;
struct Cyc_Absyn_Conref*Cyc_Absyn_compress_conref(struct Cyc_Absyn_Conref*x){void*
_tmp26=(void*)x->v;struct Cyc_Absyn_Conref*_tmp27;struct Cyc_Absyn_Conref**_tmp28;
_LL19: if((int)_tmp26 != 0)goto _LL1B;_LL1A: goto _LL1C;_LL1B: if(_tmp26 <= (void*)1?1:*((
int*)_tmp26)!= 0)goto _LL1D;_LL1C: return x;_LL1D: if(_tmp26 <= (void*)1?1:*((int*)
_tmp26)!= 1)goto _LL18;_tmp27=((struct Cyc_Absyn_Forward_constr_struct*)_tmp26)->f1;
_tmp28=(struct Cyc_Absyn_Conref**)&((struct Cyc_Absyn_Forward_constr_struct*)
_tmp26)->f1;_LL1E: {struct Cyc_Absyn_Conref*_tmp29=Cyc_Absyn_compress_conref(*
_tmp28);*_tmp28=_tmp29;return _tmp29;}_LL18:;}void*Cyc_Absyn_conref_val(struct Cyc_Absyn_Conref*
x){void*_tmp2A=(void*)(Cyc_Absyn_compress_conref(x))->v;void*_tmp2B;_LL20: if(
_tmp2A <= (void*)1?1:*((int*)_tmp2A)!= 0)goto _LL22;_tmp2B=(void*)((struct Cyc_Absyn_Eq_constr_struct*)
_tmp2A)->f1;_LL21: return _tmp2B;_LL22:;_LL23:({void*_tmp2C[0]={};Cyc_Tcutil_impos(({
const char*_tmp2D="conref_val";_tag_arr(_tmp2D,sizeof(char),_get_zero_arr_size(
_tmp2D,11));}),_tag_arr(_tmp2C,sizeof(void*),0));});_LL1F:;}void*Cyc_Absyn_conref_def(
void*y,struct Cyc_Absyn_Conref*x){void*_tmp2E=(void*)(Cyc_Absyn_compress_conref(x))->v;
void*_tmp2F;_LL25: if(_tmp2E <= (void*)1?1:*((int*)_tmp2E)!= 0)goto _LL27;_tmp2F=(
void*)((struct Cyc_Absyn_Eq_constr_struct*)_tmp2E)->f1;_LL26: return _tmp2F;_LL27:;
_LL28: return y;_LL24:;}void*Cyc_Absyn_compress_kb(void*k){void*_tmp30=k;struct Cyc_Core_Opt*
_tmp31;struct Cyc_Core_Opt*_tmp32;struct Cyc_Core_Opt*_tmp33;struct Cyc_Core_Opt
_tmp34;void*_tmp35;void**_tmp36;struct Cyc_Core_Opt*_tmp37;struct Cyc_Core_Opt
_tmp38;void*_tmp39;void**_tmp3A;_LL2A: if(*((int*)_tmp30)!= 0)goto _LL2C;_LL2B:
goto _LL2D;_LL2C: if(*((int*)_tmp30)!= 1)goto _LL2E;_tmp31=((struct Cyc_Absyn_Unknown_kb_struct*)
_tmp30)->f1;if(_tmp31 != 0)goto _LL2E;_LL2D: goto _LL2F;_LL2E: if(*((int*)_tmp30)!= 2)
goto _LL30;_tmp32=((struct Cyc_Absyn_Less_kb_struct*)_tmp30)->f1;if(_tmp32 != 0)
goto _LL30;_LL2F: return k;_LL30: if(*((int*)_tmp30)!= 1)goto _LL32;_tmp33=((struct
Cyc_Absyn_Unknown_kb_struct*)_tmp30)->f1;if(_tmp33 == 0)goto _LL32;_tmp34=*_tmp33;
_tmp35=(void*)_tmp34.v;_tmp36=(void**)&(*((struct Cyc_Absyn_Unknown_kb_struct*)
_tmp30)->f1).v;_LL31: _tmp3A=_tmp36;goto _LL33;_LL32: if(*((int*)_tmp30)!= 2)goto
_LL29;_tmp37=((struct Cyc_Absyn_Less_kb_struct*)_tmp30)->f1;if(_tmp37 == 0)goto
_LL29;_tmp38=*_tmp37;_tmp39=(void*)_tmp38.v;_tmp3A=(void**)&(*((struct Cyc_Absyn_Less_kb_struct*)
_tmp30)->f1).v;_LL33:*_tmp3A=Cyc_Absyn_compress_kb(*_tmp3A);return*_tmp3A;_LL29:;}
void*Cyc_Absyn_force_kb(void*kb){void*_tmp3B=Cyc_Absyn_compress_kb(kb);void*
_tmp3C;struct Cyc_Core_Opt*_tmp3D;struct Cyc_Core_Opt**_tmp3E;struct Cyc_Core_Opt*
_tmp3F;struct Cyc_Core_Opt**_tmp40;void*_tmp41;_LL35: if(*((int*)_tmp3B)!= 0)goto
_LL37;_tmp3C=(void*)((struct Cyc_Absyn_Eq_kb_struct*)_tmp3B)->f1;_LL36: return
_tmp3C;_LL37: if(*((int*)_tmp3B)!= 1)goto _LL39;_tmp3D=((struct Cyc_Absyn_Unknown_kb_struct*)
_tmp3B)->f1;_tmp3E=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Unknown_kb_struct*)
_tmp3B)->f1;_LL38: _tmp40=_tmp3E;_tmp41=(void*)2;goto _LL3A;_LL39: if(*((int*)
_tmp3B)!= 2)goto _LL34;_tmp3F=((struct Cyc_Absyn_Less_kb_struct*)_tmp3B)->f1;
_tmp40=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Less_kb_struct*)_tmp3B)->f1;
_tmp41=(void*)((struct Cyc_Absyn_Less_kb_struct*)_tmp3B)->f2;_LL3A:*_tmp40=({
struct Cyc_Core_Opt*_tmp42=_cycalloc(sizeof(*_tmp42));_tmp42->v=(void*)((void*)({
struct Cyc_Absyn_Eq_kb_struct*_tmp43=_cycalloc(sizeof(*_tmp43));_tmp43[0]=({
struct Cyc_Absyn_Eq_kb_struct _tmp44;_tmp44.tag=0;_tmp44.f1=(void*)_tmp41;_tmp44;});
_tmp43;}));_tmp42;});return _tmp41;_LL34:;}static struct Cyc_Absyn_IntType_struct
Cyc_Absyn_char_tt={5,(void*)((void*)2),(void*)((void*)0)};static struct Cyc_Absyn_IntType_struct
Cyc_Absyn_uchar_tt={5,(void*)((void*)1),(void*)((void*)0)};static struct Cyc_Absyn_IntType_struct
Cyc_Absyn_ushort_tt={5,(void*)((void*)1),(void*)((void*)1)};static struct Cyc_Absyn_IntType_struct
Cyc_Absyn_uint_tt={5,(void*)((void*)1),(void*)((void*)2)};static struct Cyc_Absyn_IntType_struct
Cyc_Absyn_ulong_tt={5,(void*)((void*)1),(void*)((void*)2)};static struct Cyc_Absyn_IntType_struct
Cyc_Absyn_ulonglong_tt={5,(void*)((void*)1),(void*)((void*)3)};void*Cyc_Absyn_char_typ=(
void*)& Cyc_Absyn_char_tt;void*Cyc_Absyn_uchar_typ=(void*)& Cyc_Absyn_uchar_tt;
void*Cyc_Absyn_ushort_typ=(void*)& Cyc_Absyn_ushort_tt;void*Cyc_Absyn_uint_typ=(
void*)& Cyc_Absyn_uint_tt;void*Cyc_Absyn_ulong_typ=(void*)& Cyc_Absyn_ulong_tt;
void*Cyc_Absyn_ulonglong_typ=(void*)& Cyc_Absyn_ulonglong_tt;static struct Cyc_Absyn_IntType_struct
Cyc_Absyn_schar_tt={5,(void*)((void*)0),(void*)((void*)0)};static struct Cyc_Absyn_IntType_struct
Cyc_Absyn_sshort_tt={5,(void*)((void*)0),(void*)((void*)1)};static struct Cyc_Absyn_IntType_struct
Cyc_Absyn_sint_tt={5,(void*)((void*)0),(void*)((void*)2)};static struct Cyc_Absyn_IntType_struct
Cyc_Absyn_slong_tt={5,(void*)((void*)0),(void*)((void*)2)};static struct Cyc_Absyn_IntType_struct
Cyc_Absyn_slonglong_tt={5,(void*)((void*)0),(void*)((void*)3)};void*Cyc_Absyn_schar_typ=(
void*)& Cyc_Absyn_schar_tt;void*Cyc_Absyn_sshort_typ=(void*)& Cyc_Absyn_sshort_tt;
void*Cyc_Absyn_sint_typ=(void*)& Cyc_Absyn_sint_tt;void*Cyc_Absyn_slong_typ=(void*)&
Cyc_Absyn_slong_tt;void*Cyc_Absyn_slonglong_typ=(void*)& Cyc_Absyn_slonglong_tt;
void*Cyc_Absyn_float_typ=(void*)1;void*Cyc_Absyn_double_typ(int b){return(void*)({
struct Cyc_Absyn_DoubleType_struct*_tmp50=_cycalloc_atomic(sizeof(*_tmp50));
_tmp50[0]=({struct Cyc_Absyn_DoubleType_struct _tmp51;_tmp51.tag=6;_tmp51.f1=b;
_tmp51;});_tmp50;});}static struct Cyc_Absyn_Abs_n_struct Cyc_Absyn_abs_null={1,0};
static char _tmp53[4]="exn";static struct _tagged_arr Cyc_Absyn_exn_str={_tmp53,
_tmp53,_tmp53 + 4};static struct _tuple0 Cyc_Absyn_exn_name_v={(void*)& Cyc_Absyn_abs_null,&
Cyc_Absyn_exn_str};struct _tuple0*Cyc_Absyn_exn_name=& Cyc_Absyn_exn_name_v;static
char _tmp54[15]="Null_Exception";static struct _tagged_arr Cyc_Absyn_Null_Exception_str={
_tmp54,_tmp54,_tmp54 + 15};static struct _tuple0 Cyc_Absyn_Null_Exception_pr={(void*)&
Cyc_Absyn_abs_null,& Cyc_Absyn_Null_Exception_str};struct _tuple0*Cyc_Absyn_Null_Exception_name=&
Cyc_Absyn_Null_Exception_pr;static struct Cyc_Absyn_Tunionfield Cyc_Absyn_Null_Exception_tuf_v={&
Cyc_Absyn_Null_Exception_pr,0,0,(void*)((void*)3)};struct Cyc_Absyn_Tunionfield*
Cyc_Absyn_Null_Exception_tuf=& Cyc_Absyn_Null_Exception_tuf_v;static char _tmp55[13]="Array_bounds";
static struct _tagged_arr Cyc_Absyn_Array_bounds_str={_tmp55,_tmp55,_tmp55 + 13};
static struct _tuple0 Cyc_Absyn_Array_bounds_pr={(void*)& Cyc_Absyn_abs_null,& Cyc_Absyn_Array_bounds_str};
struct _tuple0*Cyc_Absyn_Array_bounds_name=& Cyc_Absyn_Array_bounds_pr;static
struct Cyc_Absyn_Tunionfield Cyc_Absyn_Array_bounds_tuf_v={& Cyc_Absyn_Array_bounds_pr,
0,0,(void*)((void*)3)};struct Cyc_Absyn_Tunionfield*Cyc_Absyn_Array_bounds_tuf=&
Cyc_Absyn_Array_bounds_tuf_v;static char _tmp56[16]="Match_Exception";static struct
_tagged_arr Cyc_Absyn_Match_Exception_str={_tmp56,_tmp56,_tmp56 + 16};static struct
_tuple0 Cyc_Absyn_Match_Exception_pr={(void*)& Cyc_Absyn_abs_null,& Cyc_Absyn_Match_Exception_str};
struct _tuple0*Cyc_Absyn_Match_Exception_name=& Cyc_Absyn_Match_Exception_pr;
static struct Cyc_Absyn_Tunionfield Cyc_Absyn_Match_Exception_tuf_v={& Cyc_Absyn_Match_Exception_pr,
0,0,(void*)((void*)3)};struct Cyc_Absyn_Tunionfield*Cyc_Absyn_Match_Exception_tuf=&
Cyc_Absyn_Match_Exception_tuf_v;static char _tmp57[10]="Bad_alloc";static struct
_tagged_arr Cyc_Absyn_Bad_alloc_str={_tmp57,_tmp57,_tmp57 + 10};static struct
_tuple0 Cyc_Absyn_Bad_alloc_pr={(void*)& Cyc_Absyn_abs_null,& Cyc_Absyn_Bad_alloc_str};
struct _tuple0*Cyc_Absyn_Bad_alloc_name=& Cyc_Absyn_Bad_alloc_pr;static struct Cyc_Absyn_Tunionfield
Cyc_Absyn_Bad_alloc_tuf_v={& Cyc_Absyn_Bad_alloc_pr,0,0,(void*)((void*)3)};struct
Cyc_Absyn_Tunionfield*Cyc_Absyn_Bad_alloc_tuf=& Cyc_Absyn_Bad_alloc_tuf_v;static
struct Cyc_List_List Cyc_Absyn_exn_l0={(void*)& Cyc_Absyn_Null_Exception_tuf_v,0};
static struct Cyc_List_List Cyc_Absyn_exn_l1={(void*)& Cyc_Absyn_Array_bounds_tuf_v,(
struct Cyc_List_List*)& Cyc_Absyn_exn_l0};static struct Cyc_List_List Cyc_Absyn_exn_l2={(
void*)& Cyc_Absyn_Match_Exception_tuf_v,(struct Cyc_List_List*)& Cyc_Absyn_exn_l1};
static struct Cyc_List_List Cyc_Absyn_exn_l3={(void*)& Cyc_Absyn_Bad_alloc_tuf_v,(
struct Cyc_List_List*)& Cyc_Absyn_exn_l2};static struct Cyc_Core_Opt Cyc_Absyn_exn_ol={(
void*)((struct Cyc_List_List*)& Cyc_Absyn_exn_l3)};static struct Cyc_Absyn_Tuniondecl
Cyc_Absyn_exn_tud_v={(void*)((void*)3),& Cyc_Absyn_exn_name_v,0,(struct Cyc_Core_Opt*)&
Cyc_Absyn_exn_ol,1};struct Cyc_Absyn_Tuniondecl*Cyc_Absyn_exn_tud=& Cyc_Absyn_exn_tud_v;
static struct Cyc_Absyn_KnownTunion_struct Cyc_Absyn_exn_tinfou={1,& Cyc_Absyn_exn_tud};
static struct Cyc_Absyn_TunionType_struct Cyc_Absyn_exn_typ_tt={2,{(void*)((void*)&
Cyc_Absyn_exn_tinfou),0,(void*)((void*)2)}};void*Cyc_Absyn_exn_typ=(void*)& Cyc_Absyn_exn_typ_tt;
static char _tmp5A[9]="PrintArg";static struct _tagged_arr Cyc_Absyn_printarg_str={
_tmp5A,_tmp5A,_tmp5A + 9};static char _tmp5B[9]="ScanfArg";static struct _tagged_arr
Cyc_Absyn_scanfarg_str={_tmp5B,_tmp5B,_tmp5B + 9};static struct Cyc_Absyn_Abs_n_struct
Cyc_Absyn_std_namespace={1,0};static struct _tuple0 Cyc_Absyn_tunion_print_arg_qvar_p={(
void*)& Cyc_Absyn_std_namespace,& Cyc_Absyn_printarg_str};static struct _tuple0 Cyc_Absyn_tunion_scanf_arg_qvar_p={(
void*)& Cyc_Absyn_std_namespace,& Cyc_Absyn_scanfarg_str};struct _tuple0*Cyc_Absyn_tunion_print_arg_qvar=&
Cyc_Absyn_tunion_print_arg_qvar_p;struct _tuple0*Cyc_Absyn_tunion_scanf_arg_qvar=&
Cyc_Absyn_tunion_scanf_arg_qvar_p;static void**Cyc_Absyn_string_t_opt=0;void*Cyc_Absyn_string_typ(
void*rgn){if(rgn != (void*)2)return Cyc_Absyn_starb_typ(Cyc_Absyn_char_typ,rgn,Cyc_Absyn_empty_tqual(),(
void*)0,Cyc_Absyn_true_conref);if(Cyc_Absyn_string_t_opt == 0){void*t=Cyc_Absyn_starb_typ(
Cyc_Absyn_char_typ,(void*)2,Cyc_Absyn_empty_tqual(),(void*)0,Cyc_Absyn_true_conref);
Cyc_Absyn_string_t_opt=({void**_tmp5D=_cycalloc(sizeof(*_tmp5D));_tmp5D[0]=t;
_tmp5D;});}return*((void**)_check_null(Cyc_Absyn_string_t_opt));}static void**Cyc_Absyn_const_string_t_opt=
0;void*Cyc_Absyn_const_string_typ(void*rgn){if(rgn != (void*)2)return Cyc_Absyn_starb_typ(
Cyc_Absyn_char_typ,rgn,Cyc_Absyn_const_tqual(),(void*)0,Cyc_Absyn_true_conref);
if(Cyc_Absyn_const_string_t_opt == 0){void*t=Cyc_Absyn_starb_typ(Cyc_Absyn_char_typ,(
void*)2,Cyc_Absyn_const_tqual(),(void*)0,Cyc_Absyn_true_conref);Cyc_Absyn_const_string_t_opt=({
void**_tmp5E=_cycalloc(sizeof(*_tmp5E));_tmp5E[0]=t;_tmp5E;});}return*((void**)
_check_null(Cyc_Absyn_const_string_t_opt));}void*Cyc_Absyn_starb_typ(void*t,void*
r,struct Cyc_Absyn_Tqual tq,void*b,struct Cyc_Absyn_Conref*zeroterm){return(void*)({
struct Cyc_Absyn_PointerType_struct*_tmp5F=_cycalloc(sizeof(*_tmp5F));_tmp5F[0]=({
struct Cyc_Absyn_PointerType_struct _tmp60;_tmp60.tag=4;_tmp60.f1=({struct Cyc_Absyn_PtrInfo
_tmp61;_tmp61.elt_typ=(void*)t;_tmp61.elt_tq=tq;_tmp61.ptr_atts=({struct Cyc_Absyn_PtrAtts
_tmp62;_tmp62.rgn=(void*)r;_tmp62.nullable=Cyc_Absyn_true_conref;_tmp62.bounds=
Cyc_Absyn_new_conref(b);_tmp62.zero_term=zeroterm;_tmp62;});_tmp61;});_tmp60;});
_tmp5F;});}void*Cyc_Absyn_atb_typ(void*t,void*r,struct Cyc_Absyn_Tqual tq,void*b,
struct Cyc_Absyn_Conref*zeroterm){return(void*)({struct Cyc_Absyn_PointerType_struct*
_tmp63=_cycalloc(sizeof(*_tmp63));_tmp63[0]=({struct Cyc_Absyn_PointerType_struct
_tmp64;_tmp64.tag=4;_tmp64.f1=({struct Cyc_Absyn_PtrInfo _tmp65;_tmp65.elt_typ=(
void*)t;_tmp65.elt_tq=tq;_tmp65.ptr_atts=({struct Cyc_Absyn_PtrAtts _tmp66;_tmp66.rgn=(
void*)r;_tmp66.nullable=Cyc_Absyn_false_conref;_tmp66.bounds=Cyc_Absyn_new_conref(
b);_tmp66.zero_term=zeroterm;_tmp66;});_tmp65;});_tmp64;});_tmp63;});}void*Cyc_Absyn_star_typ(
void*t,void*r,struct Cyc_Absyn_Tqual tq,struct Cyc_Absyn_Conref*zeroterm){return(
void*)({struct Cyc_Absyn_PointerType_struct*_tmp67=_cycalloc(sizeof(*_tmp67));
_tmp67[0]=({struct Cyc_Absyn_PointerType_struct _tmp68;_tmp68.tag=4;_tmp68.f1=({
struct Cyc_Absyn_PtrInfo _tmp69;_tmp69.elt_typ=(void*)t;_tmp69.elt_tq=tq;_tmp69.ptr_atts=({
struct Cyc_Absyn_PtrAtts _tmp6A;_tmp6A.rgn=(void*)r;_tmp6A.nullable=Cyc_Absyn_true_conref;
_tmp6A.bounds=Cyc_Absyn_bounds_one_conref;_tmp6A.zero_term=zeroterm;_tmp6A;});
_tmp69;});_tmp68;});_tmp67;});}void*Cyc_Absyn_cstar_typ(void*t,struct Cyc_Absyn_Tqual
tq){return(void*)({struct Cyc_Absyn_PointerType_struct*_tmp6B=_cycalloc(sizeof(*
_tmp6B));_tmp6B[0]=({struct Cyc_Absyn_PointerType_struct _tmp6C;_tmp6C.tag=4;
_tmp6C.f1=({struct Cyc_Absyn_PtrInfo _tmp6D;_tmp6D.elt_typ=(void*)t;_tmp6D.elt_tq=
tq;_tmp6D.ptr_atts=({struct Cyc_Absyn_PtrAtts _tmp6E;_tmp6E.rgn=(void*)((void*)2);
_tmp6E.nullable=Cyc_Absyn_true_conref;_tmp6E.bounds=Cyc_Absyn_bounds_one_conref;
_tmp6E.zero_term=Cyc_Absyn_false_conref;_tmp6E;});_tmp6D;});_tmp6C;});_tmp6B;});}
void*Cyc_Absyn_at_typ(void*t,void*r,struct Cyc_Absyn_Tqual tq,struct Cyc_Absyn_Conref*
zeroterm){return(void*)({struct Cyc_Absyn_PointerType_struct*_tmp6F=_cycalloc(
sizeof(*_tmp6F));_tmp6F[0]=({struct Cyc_Absyn_PointerType_struct _tmp70;_tmp70.tag=
4;_tmp70.f1=({struct Cyc_Absyn_PtrInfo _tmp71;_tmp71.elt_typ=(void*)t;_tmp71.elt_tq=
tq;_tmp71.ptr_atts=({struct Cyc_Absyn_PtrAtts _tmp72;_tmp72.rgn=(void*)r;_tmp72.nullable=
Cyc_Absyn_false_conref;_tmp72.bounds=Cyc_Absyn_bounds_one_conref;_tmp72.zero_term=
zeroterm;_tmp72;});_tmp71;});_tmp70;});_tmp6F;});}void*Cyc_Absyn_tagged_typ(void*
t,void*r,struct Cyc_Absyn_Tqual tq,struct Cyc_Absyn_Conref*zeroterm){return(void*)({
struct Cyc_Absyn_PointerType_struct*_tmp73=_cycalloc(sizeof(*_tmp73));_tmp73[0]=({
struct Cyc_Absyn_PointerType_struct _tmp74;_tmp74.tag=4;_tmp74.f1=({struct Cyc_Absyn_PtrInfo
_tmp75;_tmp75.elt_typ=(void*)t;_tmp75.elt_tq=tq;_tmp75.ptr_atts=({struct Cyc_Absyn_PtrAtts
_tmp76;_tmp76.rgn=(void*)r;_tmp76.nullable=Cyc_Absyn_true_conref;_tmp76.bounds=
Cyc_Absyn_bounds_unknown_conref;_tmp76.zero_term=zeroterm;_tmp76;});_tmp75;});
_tmp74;});_tmp73;});}void*Cyc_Absyn_array_typ(void*elt_type,struct Cyc_Absyn_Tqual
tq,struct Cyc_Absyn_Exp*num_elts,struct Cyc_Absyn_Conref*zero_term){return(void*)({
struct Cyc_Absyn_ArrayType_struct*_tmp77=_cycalloc(sizeof(*_tmp77));_tmp77[0]=({
struct Cyc_Absyn_ArrayType_struct _tmp78;_tmp78.tag=7;_tmp78.f1=({struct Cyc_Absyn_ArrayInfo
_tmp79;_tmp79.elt_type=(void*)elt_type;_tmp79.tq=tq;_tmp79.num_elts=num_elts;
_tmp79.zero_term=zero_term;_tmp79;});_tmp78;});_tmp77;});}static char _tmp83[8]="__sFILE";
void*Cyc_Absyn_file_typ(){static void**file_t_opt=0;static struct _tagged_arr sf_str={
_tmp83,_tmp83,_tmp83 + 8};static struct _tagged_arr*sf=& sf_str;if(file_t_opt == 0){
struct _tuple0*file_t_name=({struct _tuple0*_tmp82=_cycalloc(sizeof(*_tmp82));
_tmp82->f1=(void*)& Cyc_Absyn_std_namespace;_tmp82->f2=sf;_tmp82;});struct Cyc_Absyn_Aggrdecl*
sd=({struct Cyc_Absyn_Aggrdecl*_tmp81=_cycalloc(sizeof(*_tmp81));_tmp81->kind=(
void*)((void*)0);_tmp81->sc=(void*)((void*)1);_tmp81->name=file_t_name;_tmp81->tvs=
0;_tmp81->impl=0;_tmp81->attributes=0;_tmp81;});void*file_struct_typ=(void*)({
struct Cyc_Absyn_AggrType_struct*_tmp7B=_cycalloc(sizeof(*_tmp7B));_tmp7B[0]=({
struct Cyc_Absyn_AggrType_struct _tmp7C;_tmp7C.tag=10;_tmp7C.f1=({struct Cyc_Absyn_AggrInfo
_tmp7D;_tmp7D.aggr_info=(void*)((void*)({struct Cyc_Absyn_KnownAggr_struct*_tmp7E=
_cycalloc(sizeof(*_tmp7E));_tmp7E[0]=({struct Cyc_Absyn_KnownAggr_struct _tmp7F;
_tmp7F.tag=1;_tmp7F.f1=({struct Cyc_Absyn_Aggrdecl**_tmp80=_cycalloc(sizeof(*
_tmp80));_tmp80[0]=sd;_tmp80;});_tmp7F;});_tmp7E;}));_tmp7D.targs=0;_tmp7D;});
_tmp7C;});_tmp7B;});file_t_opt=({void**_tmp7A=_cycalloc(sizeof(*_tmp7A));_tmp7A[
0]=Cyc_Absyn_at_typ(file_struct_typ,(void*)2,Cyc_Absyn_empty_tqual(),Cyc_Absyn_false_conref);
_tmp7A;});}return*file_t_opt;}void*Cyc_Absyn_void_star_typ(){static void**
void_star_t_opt=0;if(void_star_t_opt == 0)void_star_t_opt=({void**_tmp84=
_cycalloc(sizeof(*_tmp84));_tmp84[0]=Cyc_Absyn_star_typ((void*)0,(void*)2,Cyc_Absyn_empty_tqual(),
Cyc_Absyn_false_conref);_tmp84;});return*void_star_t_opt;}static struct Cyc_Absyn_JoinEff_struct
Cyc_Absyn_empty_eff={20,0};void*Cyc_Absyn_empty_effect=(void*)& Cyc_Absyn_empty_eff;
void*Cyc_Absyn_aggr_typ(void*k,struct _tagged_arr*name){return(void*)({struct Cyc_Absyn_AggrType_struct*
_tmp86=_cycalloc(sizeof(*_tmp86));_tmp86[0]=({struct Cyc_Absyn_AggrType_struct
_tmp87;_tmp87.tag=10;_tmp87.f1=({struct Cyc_Absyn_AggrInfo _tmp88;_tmp88.aggr_info=(
void*)((void*)({struct Cyc_Absyn_UnknownAggr_struct*_tmp89=_cycalloc(sizeof(*
_tmp89));_tmp89[0]=({struct Cyc_Absyn_UnknownAggr_struct _tmp8A;_tmp8A.tag=0;
_tmp8A.f1=(void*)k;_tmp8A.f2=({struct _tuple0*_tmp8B=_cycalloc(sizeof(*_tmp8B));
_tmp8B->f1=Cyc_Absyn_rel_ns_null;_tmp8B->f2=name;_tmp8B;});_tmp8A;});_tmp89;}));
_tmp88.targs=0;_tmp88;});_tmp87;});_tmp86;});}void*Cyc_Absyn_strct(struct
_tagged_arr*name){return Cyc_Absyn_aggr_typ((void*)0,name);}void*Cyc_Absyn_union_typ(
struct _tagged_arr*name){return Cyc_Absyn_aggr_typ((void*)1,name);}void*Cyc_Absyn_strctq(
struct _tuple0*name){return(void*)({struct Cyc_Absyn_AggrType_struct*_tmp8C=
_cycalloc(sizeof(*_tmp8C));_tmp8C[0]=({struct Cyc_Absyn_AggrType_struct _tmp8D;
_tmp8D.tag=10;_tmp8D.f1=({struct Cyc_Absyn_AggrInfo _tmp8E;_tmp8E.aggr_info=(void*)((
void*)({struct Cyc_Absyn_UnknownAggr_struct*_tmp8F=_cycalloc(sizeof(*_tmp8F));
_tmp8F[0]=({struct Cyc_Absyn_UnknownAggr_struct _tmp90;_tmp90.tag=0;_tmp90.f1=(
void*)((void*)0);_tmp90.f2=name;_tmp90;});_tmp8F;}));_tmp8E.targs=0;_tmp8E;});
_tmp8D;});_tmp8C;});}void*Cyc_Absyn_unionq_typ(struct _tuple0*name){return(void*)({
struct Cyc_Absyn_AggrType_struct*_tmp91=_cycalloc(sizeof(*_tmp91));_tmp91[0]=({
struct Cyc_Absyn_AggrType_struct _tmp92;_tmp92.tag=10;_tmp92.f1=({struct Cyc_Absyn_AggrInfo
_tmp93;_tmp93.aggr_info=(void*)((void*)({struct Cyc_Absyn_UnknownAggr_struct*
_tmp94=_cycalloc(sizeof(*_tmp94));_tmp94[0]=({struct Cyc_Absyn_UnknownAggr_struct
_tmp95;_tmp95.tag=0;_tmp95.f1=(void*)((void*)1);_tmp95.f2=name;_tmp95;});_tmp94;}));
_tmp93.targs=0;_tmp93;});_tmp92;});_tmp91;});}struct Cyc_Absyn_Exp*Cyc_Absyn_new_exp(
void*r,struct Cyc_Position_Segment*loc){return({struct Cyc_Absyn_Exp*_tmp96=
_cycalloc(sizeof(*_tmp96));_tmp96->topt=0;_tmp96->r=(void*)r;_tmp96->loc=loc;
_tmp96->annot=(void*)((void*)Cyc_Absyn_EmptyAnnot);_tmp96;});}struct Cyc_Absyn_Exp*
Cyc_Absyn_New_exp(struct Cyc_Absyn_Exp*rgn_handle,struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_New_e_struct*_tmp97=
_cycalloc(sizeof(*_tmp97));_tmp97[0]=({struct Cyc_Absyn_New_e_struct _tmp98;_tmp98.tag=
15;_tmp98.f1=rgn_handle;_tmp98.f2=e;_tmp98;});_tmp97;}),loc);}struct Cyc_Absyn_Exp*
Cyc_Absyn_copy_exp(struct Cyc_Absyn_Exp*e){return({struct Cyc_Absyn_Exp*_tmp99=
_cycalloc(sizeof(*_tmp99));_tmp99[0]=*e;_tmp99;});}struct Cyc_Absyn_Exp*Cyc_Absyn_const_exp(
void*c,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((void*)({struct
Cyc_Absyn_Const_e_struct*_tmp9A=_cycalloc(sizeof(*_tmp9A));_tmp9A[0]=({struct Cyc_Absyn_Const_e_struct
_tmp9B;_tmp9B.tag=0;_tmp9B.f1=(void*)c;_tmp9B;});_tmp9A;}),loc);}struct Cyc_Absyn_Exp*
Cyc_Absyn_null_exp(struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((void*)({
struct Cyc_Absyn_Const_e_struct*_tmp9C=_cycalloc(sizeof(*_tmp9C));_tmp9C[0]=({
struct Cyc_Absyn_Const_e_struct _tmp9D;_tmp9D.tag=0;_tmp9D.f1=(void*)((void*)0);
_tmp9D;});_tmp9C;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_int_exp(void*s,int i,
struct Cyc_Position_Segment*seg){return Cyc_Absyn_const_exp((void*)({struct Cyc_Absyn_Int_c_struct*
_tmp9E=_cycalloc(sizeof(*_tmp9E));_tmp9E[0]=({struct Cyc_Absyn_Int_c_struct _tmp9F;
_tmp9F.tag=2;_tmp9F.f1=(void*)s;_tmp9F.f2=i;_tmp9F;});_tmp9E;}),seg);}struct Cyc_Absyn_Exp*
Cyc_Absyn_signed_int_exp(int i,struct Cyc_Position_Segment*loc){return Cyc_Absyn_int_exp((
void*)0,i,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_uint_exp(unsigned int i,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_int_exp((void*)1,(int)i,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_bool_exp(
int b,struct Cyc_Position_Segment*loc){return Cyc_Absyn_signed_int_exp(b?1: 0,loc);}
struct Cyc_Absyn_Exp*Cyc_Absyn_true_exp(struct Cyc_Position_Segment*loc){return Cyc_Absyn_bool_exp(
1,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_false_exp(struct Cyc_Position_Segment*loc){
return Cyc_Absyn_bool_exp(0,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_char_exp(char c,
struct Cyc_Position_Segment*loc){return Cyc_Absyn_const_exp((void*)({struct Cyc_Absyn_Char_c_struct*
_tmpA0=_cycalloc(sizeof(*_tmpA0));_tmpA0[0]=({struct Cyc_Absyn_Char_c_struct
_tmpA1;_tmpA1.tag=0;_tmpA1.f1=(void*)((void*)2);_tmpA1.f2=c;_tmpA1;});_tmpA0;}),
loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_float_exp(struct _tagged_arr f,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_const_exp((void*)({struct Cyc_Absyn_Float_c_struct*_tmpA2=
_cycalloc(sizeof(*_tmpA2));_tmpA2[0]=({struct Cyc_Absyn_Float_c_struct _tmpA3;
_tmpA3.tag=4;_tmpA3.f1=f;_tmpA3;});_tmpA2;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_string_exp(
struct _tagged_arr s,struct Cyc_Position_Segment*loc){return Cyc_Absyn_const_exp((
void*)({struct Cyc_Absyn_String_c_struct*_tmpA4=_cycalloc(sizeof(*_tmpA4));_tmpA4[
0]=({struct Cyc_Absyn_String_c_struct _tmpA5;_tmpA5.tag=5;_tmpA5.f1=s;_tmpA5;});
_tmpA4;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_var_exp(struct _tuple0*q,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_Var_e_struct*_tmpA6=
_cycalloc(sizeof(*_tmpA6));_tmpA6[0]=({struct Cyc_Absyn_Var_e_struct _tmpA7;_tmpA7.tag=
1;_tmpA7.f1=q;_tmpA7.f2=(void*)((void*)0);_tmpA7;});_tmpA6;}),loc);}struct Cyc_Absyn_Exp*
Cyc_Absyn_varb_exp(struct _tuple0*q,void*b,struct Cyc_Position_Segment*loc){return
Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_Var_e_struct*_tmpA8=_cycalloc(sizeof(*
_tmpA8));_tmpA8[0]=({struct Cyc_Absyn_Var_e_struct _tmpA9;_tmpA9.tag=1;_tmpA9.f1=q;
_tmpA9.f2=(void*)b;_tmpA9;});_tmpA8;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_unknownid_exp(
struct _tuple0*q,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((void*)({
struct Cyc_Absyn_UnknownId_e_struct*_tmpAA=_cycalloc(sizeof(*_tmpAA));_tmpAA[0]=({
struct Cyc_Absyn_UnknownId_e_struct _tmpAB;_tmpAB.tag=2;_tmpAB.f1=q;_tmpAB;});
_tmpAA;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_primop_exp(void*p,struct Cyc_List_List*
es,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_Primop_e_struct*
_tmpAC=_cycalloc(sizeof(*_tmpAC));_tmpAC[0]=({struct Cyc_Absyn_Primop_e_struct
_tmpAD;_tmpAD.tag=3;_tmpAD.f1=(void*)p;_tmpAD.f2=es;_tmpAD;});_tmpAC;}),loc);}
struct Cyc_Absyn_Exp*Cyc_Absyn_prim1_exp(void*p,struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_primop_exp(p,({struct Cyc_List_List*_tmpAE=_cycalloc(sizeof(*
_tmpAE));_tmpAE->hd=e;_tmpAE->tl=0;_tmpAE;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_prim2_exp(
void*p,struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_primop_exp(p,({struct Cyc_List_List*_tmpAF=_cycalloc(sizeof(*
_tmpAF));_tmpAF->hd=e1;_tmpAF->tl=({struct Cyc_List_List*_tmpB0=_cycalloc(sizeof(*
_tmpB0));_tmpB0->hd=e2;_tmpB0->tl=0;_tmpB0;});_tmpAF;}),loc);}struct Cyc_Absyn_Exp*
Cyc_Absyn_add_exp(struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_prim2_exp((void*)0,e1,e2,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_times_exp(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_prim2_exp((void*)1,e1,e2,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_divide_exp(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_prim2_exp((void*)3,e1,e2,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_eq_exp(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_prim2_exp((void*)5,e1,e2,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_neq_exp(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_prim2_exp((void*)6,e1,e2,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_gt_exp(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_prim2_exp((void*)7,e1,e2,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_lt_exp(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_prim2_exp((void*)8,e1,e2,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_gte_exp(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_prim2_exp((void*)9,e1,e2,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_lte_exp(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_prim2_exp((void*)10,e1,e2,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_assignop_exp(
struct Cyc_Absyn_Exp*e1,struct Cyc_Core_Opt*popt,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_AssignOp_e_struct*_tmpB1=
_cycalloc(sizeof(*_tmpB1));_tmpB1[0]=({struct Cyc_Absyn_AssignOp_e_struct _tmpB2;
_tmpB2.tag=4;_tmpB2.f1=e1;_tmpB2.f2=popt;_tmpB2.f3=e2;_tmpB2;});_tmpB1;}),loc);}
struct Cyc_Absyn_Exp*Cyc_Absyn_assign_exp(struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*
e2,struct Cyc_Position_Segment*loc){return Cyc_Absyn_assignop_exp(e1,0,e2,loc);}
struct Cyc_Absyn_Exp*Cyc_Absyn_increment_exp(struct Cyc_Absyn_Exp*e,void*i,struct
Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_Increment_e_struct*
_tmpB3=_cycalloc(sizeof(*_tmpB3));_tmpB3[0]=({struct Cyc_Absyn_Increment_e_struct
_tmpB4;_tmpB4.tag=5;_tmpB4.f1=e;_tmpB4.f2=(void*)i;_tmpB4;});_tmpB3;}),loc);}
struct Cyc_Absyn_Exp*Cyc_Absyn_post_inc_exp(struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_increment_exp(e,(void*)1,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_pre_inc_exp(
struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*loc){return Cyc_Absyn_increment_exp(
e,(void*)0,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_pre_dec_exp(struct Cyc_Absyn_Exp*e,
struct Cyc_Position_Segment*loc){return Cyc_Absyn_increment_exp(e,(void*)2,loc);}
struct Cyc_Absyn_Exp*Cyc_Absyn_post_dec_exp(struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_increment_exp(e,(void*)3,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_conditional_exp(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Absyn_Exp*e3,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_Conditional_e_struct*
_tmpB5=_cycalloc(sizeof(*_tmpB5));_tmpB5[0]=({struct Cyc_Absyn_Conditional_e_struct
_tmpB6;_tmpB6.tag=6;_tmpB6.f1=e1;_tmpB6.f2=e2;_tmpB6.f3=e3;_tmpB6;});_tmpB5;}),
loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_and_exp(struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*
e2,struct Cyc_Position_Segment*loc){return Cyc_Absyn_conditional_exp(e1,e2,Cyc_Absyn_false_exp(
loc),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_or_exp(struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*
e2,struct Cyc_Position_Segment*loc){return Cyc_Absyn_conditional_exp(e1,Cyc_Absyn_true_exp(
loc),e2,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_seq_exp(struct Cyc_Absyn_Exp*e1,
struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((
void*)({struct Cyc_Absyn_SeqExp_e_struct*_tmpB7=_cycalloc(sizeof(*_tmpB7));_tmpB7[
0]=({struct Cyc_Absyn_SeqExp_e_struct _tmpB8;_tmpB8.tag=7;_tmpB8.f1=e1;_tmpB8.f2=
e2;_tmpB8;});_tmpB7;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_unknowncall_exp(
struct Cyc_Absyn_Exp*e,struct Cyc_List_List*es,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_UnknownCall_e_struct*_tmpB9=
_cycalloc(sizeof(*_tmpB9));_tmpB9[0]=({struct Cyc_Absyn_UnknownCall_e_struct
_tmpBA;_tmpBA.tag=8;_tmpBA.f1=e;_tmpBA.f2=es;_tmpBA;});_tmpB9;}),loc);}struct Cyc_Absyn_Exp*
Cyc_Absyn_fncall_exp(struct Cyc_Absyn_Exp*e,struct Cyc_List_List*es,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_FnCall_e_struct*_tmpBB=
_cycalloc(sizeof(*_tmpBB));_tmpBB[0]=({struct Cyc_Absyn_FnCall_e_struct _tmpBC;
_tmpBC.tag=9;_tmpBC.f1=e;_tmpBC.f2=es;_tmpBC.f3=0;_tmpBC;});_tmpBB;}),loc);}
struct Cyc_Absyn_Exp*Cyc_Absyn_noinstantiate_exp(struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_NoInstantiate_e_struct*
_tmpBD=_cycalloc(sizeof(*_tmpBD));_tmpBD[0]=({struct Cyc_Absyn_NoInstantiate_e_struct
_tmpBE;_tmpBE.tag=11;_tmpBE.f1=e;_tmpBE;});_tmpBD;}),loc);}struct Cyc_Absyn_Exp*
Cyc_Absyn_instantiate_exp(struct Cyc_Absyn_Exp*e,struct Cyc_List_List*ts,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_Instantiate_e_struct*
_tmpBF=_cycalloc(sizeof(*_tmpBF));_tmpBF[0]=({struct Cyc_Absyn_Instantiate_e_struct
_tmpC0;_tmpC0.tag=12;_tmpC0.f1=e;_tmpC0.f2=ts;_tmpC0;});_tmpBF;}),loc);}struct
Cyc_Absyn_Exp*Cyc_Absyn_cast_exp(void*t,struct Cyc_Absyn_Exp*e,int user_cast,void*
c,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_Cast_e_struct*
_tmpC1=_cycalloc(sizeof(*_tmpC1));_tmpC1[0]=({struct Cyc_Absyn_Cast_e_struct
_tmpC2;_tmpC2.tag=13;_tmpC2.f1=(void*)t;_tmpC2.f2=e;_tmpC2.f3=user_cast;_tmpC2.f4=(
void*)c;_tmpC2;});_tmpC1;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_throw_exp(struct
Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((void*)({
struct Cyc_Absyn_Throw_e_struct*_tmpC3=_cycalloc(sizeof(*_tmpC3));_tmpC3[0]=({
struct Cyc_Absyn_Throw_e_struct _tmpC4;_tmpC4.tag=10;_tmpC4.f1=e;_tmpC4;});_tmpC3;}),
loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_address_exp(struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_Address_e_struct*_tmpC5=
_cycalloc(sizeof(*_tmpC5));_tmpC5[0]=({struct Cyc_Absyn_Address_e_struct _tmpC6;
_tmpC6.tag=14;_tmpC6.f1=e;_tmpC6;});_tmpC5;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_sizeoftyp_exp(
void*t,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((void*)({struct
Cyc_Absyn_Sizeoftyp_e_struct*_tmpC7=_cycalloc(sizeof(*_tmpC7));_tmpC7[0]=({
struct Cyc_Absyn_Sizeoftyp_e_struct _tmpC8;_tmpC8.tag=16;_tmpC8.f1=(void*)t;_tmpC8;});
_tmpC7;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_sizeofexp_exp(struct Cyc_Absyn_Exp*
e,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_Sizeofexp_e_struct*
_tmpC9=_cycalloc(sizeof(*_tmpC9));_tmpC9[0]=({struct Cyc_Absyn_Sizeofexp_e_struct
_tmpCA;_tmpCA.tag=17;_tmpCA.f1=e;_tmpCA;});_tmpC9;}),loc);}struct Cyc_Absyn_Exp*
Cyc_Absyn_offsetof_exp(void*t,void*of,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((
void*)({struct Cyc_Absyn_Offsetof_e_struct*_tmpCB=_cycalloc(sizeof(*_tmpCB));
_tmpCB[0]=({struct Cyc_Absyn_Offsetof_e_struct _tmpCC;_tmpCC.tag=18;_tmpCC.f1=(
void*)t;_tmpCC.f2=(void*)of;_tmpCC;});_tmpCB;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_gentyp_exp(
struct Cyc_List_List*tvs,void*t,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((
void*)({struct Cyc_Absyn_Gentyp_e_struct*_tmpCD=_cycalloc(sizeof(*_tmpCD));_tmpCD[
0]=({struct Cyc_Absyn_Gentyp_e_struct _tmpCE;_tmpCE.tag=19;_tmpCE.f1=tvs;_tmpCE.f2=(
void*)t;_tmpCE;});_tmpCD;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_deref_exp(struct
Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((void*)({
struct Cyc_Absyn_Deref_e_struct*_tmpCF=_cycalloc(sizeof(*_tmpCF));_tmpCF[0]=({
struct Cyc_Absyn_Deref_e_struct _tmpD0;_tmpD0.tag=20;_tmpD0.f1=e;_tmpD0;});_tmpCF;}),
loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_aggrmember_exp(struct Cyc_Absyn_Exp*e,struct
_tagged_arr*n,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((void*)({
struct Cyc_Absyn_AggrMember_e_struct*_tmpD1=_cycalloc(sizeof(*_tmpD1));_tmpD1[0]=({
struct Cyc_Absyn_AggrMember_e_struct _tmpD2;_tmpD2.tag=21;_tmpD2.f1=e;_tmpD2.f2=n;
_tmpD2;});_tmpD1;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_aggrarrow_exp(struct Cyc_Absyn_Exp*
e,struct _tagged_arr*n,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((
void*)({struct Cyc_Absyn_AggrArrow_e_struct*_tmpD3=_cycalloc(sizeof(*_tmpD3));
_tmpD3[0]=({struct Cyc_Absyn_AggrArrow_e_struct _tmpD4;_tmpD4.tag=22;_tmpD4.f1=e;
_tmpD4.f2=n;_tmpD4;});_tmpD3;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_subscript_exp(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_Subscript_e_struct*_tmpD5=
_cycalloc(sizeof(*_tmpD5));_tmpD5[0]=({struct Cyc_Absyn_Subscript_e_struct _tmpD6;
_tmpD6.tag=23;_tmpD6.f1=e1;_tmpD6.f2=e2;_tmpD6;});_tmpD5;}),loc);}struct Cyc_Absyn_Exp*
Cyc_Absyn_tuple_exp(struct Cyc_List_List*es,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_Tuple_e_struct*_tmpD7=_cycalloc(
sizeof(*_tmpD7));_tmpD7[0]=({struct Cyc_Absyn_Tuple_e_struct _tmpD8;_tmpD8.tag=24;
_tmpD8.f1=es;_tmpD8;});_tmpD7;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_stmt_exp(
struct Cyc_Absyn_Stmt*s,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((
void*)({struct Cyc_Absyn_StmtExp_e_struct*_tmpD9=_cycalloc(sizeof(*_tmpD9));
_tmpD9[0]=({struct Cyc_Absyn_StmtExp_e_struct _tmpDA;_tmpDA.tag=35;_tmpDA.f1=s;
_tmpDA;});_tmpD9;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_match_exn_exp(struct Cyc_Position_Segment*
loc){return Cyc_Absyn_var_exp(Cyc_Absyn_Match_Exception_name,loc);}struct _tuple12{
struct Cyc_List_List*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Exp*Cyc_Absyn_array_exp(
struct Cyc_List_List*es,struct Cyc_Position_Segment*loc){struct Cyc_List_List*dles=
0;for(0;es != 0;es=es->tl){dles=({struct Cyc_List_List*_tmpDB=_cycalloc(sizeof(*
_tmpDB));_tmpDB->hd=({struct _tuple12*_tmpDC=_cycalloc(sizeof(*_tmpDC));_tmpDC->f1=
0;_tmpDC->f2=(struct Cyc_Absyn_Exp*)es->hd;_tmpDC;});_tmpDB->tl=dles;_tmpDB;});}
dles=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(dles);
return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_Array_e_struct*_tmpDD=_cycalloc(
sizeof(*_tmpDD));_tmpDD[0]=({struct Cyc_Absyn_Array_e_struct _tmpDE;_tmpDE.tag=26;
_tmpDE.f1=dles;_tmpDE;});_tmpDD;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_unresolvedmem_exp(
struct Cyc_Core_Opt*n,struct Cyc_List_List*dles,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_UnresolvedMem_e_struct*_tmpDF=
_cycalloc(sizeof(*_tmpDF));_tmpDF[0]=({struct Cyc_Absyn_UnresolvedMem_e_struct
_tmpE0;_tmpE0.tag=34;_tmpE0.f1=n;_tmpE0.f2=dles;_tmpE0;});_tmpDF;}),loc);}struct
Cyc_Absyn_Stmt*Cyc_Absyn_new_stmt(void*s,struct Cyc_Position_Segment*loc){return({
struct Cyc_Absyn_Stmt*_tmpE1=_cycalloc(sizeof(*_tmpE1));_tmpE1->r=(void*)s;_tmpE1->loc=
loc;_tmpE1->non_local_preds=0;_tmpE1->try_depth=0;_tmpE1->annot=(void*)((void*)
Cyc_Absyn_EmptyAnnot);_tmpE1;});}struct Cyc_Absyn_Stmt*Cyc_Absyn_skip_stmt(struct
Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((void*)0,loc);}struct Cyc_Absyn_Stmt*
Cyc_Absyn_exp_stmt(struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*loc){return
Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_Exp_s_struct*_tmpE2=_cycalloc(
sizeof(*_tmpE2));_tmpE2[0]=({struct Cyc_Absyn_Exp_s_struct _tmpE3;_tmpE3.tag=0;
_tmpE3.f1=e;_tmpE3;});_tmpE2;}),loc);}struct Cyc_Absyn_Stmt*Cyc_Absyn_seq_stmts(
struct Cyc_List_List*ss,struct Cyc_Position_Segment*loc){if(ss == 0)return Cyc_Absyn_skip_stmt(
loc);else{if(ss->tl == 0)return(struct Cyc_Absyn_Stmt*)ss->hd;else{return Cyc_Absyn_seq_stmt((
struct Cyc_Absyn_Stmt*)ss->hd,Cyc_Absyn_seq_stmts(ss->tl,loc),loc);}}}struct Cyc_Absyn_Stmt*
Cyc_Absyn_return_stmt(struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_Return_s_struct*_tmpE4=
_cycalloc(sizeof(*_tmpE4));_tmpE4[0]=({struct Cyc_Absyn_Return_s_struct _tmpE5;
_tmpE5.tag=2;_tmpE5.f1=e;_tmpE5;});_tmpE4;}),loc);}struct Cyc_Absyn_Stmt*Cyc_Absyn_ifthenelse_stmt(
struct Cyc_Absyn_Exp*e,struct Cyc_Absyn_Stmt*s1,struct Cyc_Absyn_Stmt*s2,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_IfThenElse_s_struct*
_tmpE6=_cycalloc(sizeof(*_tmpE6));_tmpE6[0]=({struct Cyc_Absyn_IfThenElse_s_struct
_tmpE7;_tmpE7.tag=3;_tmpE7.f1=e;_tmpE7.f2=s1;_tmpE7.f3=s2;_tmpE7;});_tmpE6;}),
loc);}struct Cyc_Absyn_Stmt*Cyc_Absyn_while_stmt(struct Cyc_Absyn_Exp*e,struct Cyc_Absyn_Stmt*
s,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_While_s_struct*
_tmpE8=_cycalloc(sizeof(*_tmpE8));_tmpE8[0]=({struct Cyc_Absyn_While_s_struct
_tmpE9;_tmpE9.tag=4;_tmpE9.f1=({struct _tuple2 _tmpEA;_tmpEA.f1=e;_tmpEA.f2=Cyc_Absyn_skip_stmt(
e->loc);_tmpEA;});_tmpE9.f2=s;_tmpE9;});_tmpE8;}),loc);}struct Cyc_Absyn_Stmt*Cyc_Absyn_break_stmt(
struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_Break_s_struct*
_tmpEB=_cycalloc(sizeof(*_tmpEB));_tmpEB[0]=({struct Cyc_Absyn_Break_s_struct
_tmpEC;_tmpEC.tag=5;_tmpEC.f1=0;_tmpEC;});_tmpEB;}),loc);}struct Cyc_Absyn_Stmt*
Cyc_Absyn_continue_stmt(struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((
void*)({struct Cyc_Absyn_Continue_s_struct*_tmpED=_cycalloc(sizeof(*_tmpED));
_tmpED[0]=({struct Cyc_Absyn_Continue_s_struct _tmpEE;_tmpEE.tag=6;_tmpEE.f1=0;
_tmpEE;});_tmpED;}),loc);}struct Cyc_Absyn_Stmt*Cyc_Absyn_for_stmt(struct Cyc_Absyn_Exp*
e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Absyn_Exp*e3,struct Cyc_Absyn_Stmt*s,struct
Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_For_s_struct*
_tmpEF=_cycalloc(sizeof(*_tmpEF));_tmpEF[0]=({struct Cyc_Absyn_For_s_struct _tmpF0;
_tmpF0.tag=8;_tmpF0.f1=e1;_tmpF0.f2=({struct _tuple2 _tmpF1;_tmpF1.f1=e2;_tmpF1.f2=
Cyc_Absyn_skip_stmt(e3->loc);_tmpF1;});_tmpF0.f3=({struct _tuple2 _tmpF2;_tmpF2.f1=
e3;_tmpF2.f2=Cyc_Absyn_skip_stmt(e3->loc);_tmpF2;});_tmpF0.f4=s;_tmpF0;});_tmpEF;}),
loc);}struct Cyc_Absyn_Stmt*Cyc_Absyn_switch_stmt(struct Cyc_Absyn_Exp*e,struct Cyc_List_List*
scs,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_Switch_s_struct*
_tmpF3=_cycalloc(sizeof(*_tmpF3));_tmpF3[0]=({struct Cyc_Absyn_Switch_s_struct
_tmpF4;_tmpF4.tag=9;_tmpF4.f1=e;_tmpF4.f2=scs;_tmpF4;});_tmpF3;}),loc);}struct
Cyc_Absyn_Stmt*Cyc_Absyn_seq_stmt(struct Cyc_Absyn_Stmt*s1,struct Cyc_Absyn_Stmt*
s2,struct Cyc_Position_Segment*loc){struct _tuple9 _tmpF6=({struct _tuple9 _tmpF5;
_tmpF5.f1=(void*)s1->r;_tmpF5.f2=(void*)s2->r;_tmpF5;});void*_tmpF7;void*_tmpF8;
_LL3C: _tmpF7=_tmpF6.f1;if((int)_tmpF7 != 0)goto _LL3E;_LL3D: return s2;_LL3E: _tmpF8=
_tmpF6.f2;if((int)_tmpF8 != 0)goto _LL40;_LL3F: return s1;_LL40:;_LL41: return Cyc_Absyn_new_stmt((
void*)({struct Cyc_Absyn_Seq_s_struct*_tmpF9=_cycalloc(sizeof(*_tmpF9));_tmpF9[0]=({
struct Cyc_Absyn_Seq_s_struct _tmpFA;_tmpFA.tag=1;_tmpFA.f1=s1;_tmpFA.f2=s2;_tmpFA;});
_tmpF9;}),loc);_LL3B:;}struct Cyc_Absyn_Stmt*Cyc_Absyn_fallthru_stmt(struct Cyc_List_List*
el,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_Fallthru_s_struct*
_tmpFB=_cycalloc(sizeof(*_tmpFB));_tmpFB[0]=({struct Cyc_Absyn_Fallthru_s_struct
_tmpFC;_tmpFC.tag=11;_tmpFC.f1=el;_tmpFC.f2=0;_tmpFC;});_tmpFB;}),loc);}struct
Cyc_Absyn_Stmt*Cyc_Absyn_decl_stmt(struct Cyc_Absyn_Decl*d,struct Cyc_Absyn_Stmt*s,
struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_Decl_s_struct*
_tmpFD=_cycalloc(sizeof(*_tmpFD));_tmpFD[0]=({struct Cyc_Absyn_Decl_s_struct
_tmpFE;_tmpFE.tag=12;_tmpFE.f1=d;_tmpFE.f2=s;_tmpFE;});_tmpFD;}),loc);}struct Cyc_Absyn_Stmt*
Cyc_Absyn_declare_stmt(struct _tuple0*x,void*t,struct Cyc_Absyn_Exp*init,struct Cyc_Absyn_Stmt*
s,struct Cyc_Position_Segment*loc){struct Cyc_Absyn_Decl*d=Cyc_Absyn_new_decl((
void*)({struct Cyc_Absyn_Var_d_struct*_tmp101=_cycalloc(sizeof(*_tmp101));_tmp101[
0]=({struct Cyc_Absyn_Var_d_struct _tmp102;_tmp102.tag=0;_tmp102.f1=Cyc_Absyn_new_vardecl(
x,t,init);_tmp102;});_tmp101;}),loc);return Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_Decl_s_struct*
_tmpFF=_cycalloc(sizeof(*_tmpFF));_tmpFF[0]=({struct Cyc_Absyn_Decl_s_struct
_tmp100;_tmp100.tag=12;_tmp100.f1=d;_tmp100.f2=s;_tmp100;});_tmpFF;}),loc);}
struct Cyc_Absyn_Stmt*Cyc_Absyn_cut_stmt(struct Cyc_Absyn_Stmt*s,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_Cut_s_struct*_tmp103=
_cycalloc(sizeof(*_tmp103));_tmp103[0]=({struct Cyc_Absyn_Cut_s_struct _tmp104;
_tmp104.tag=13;_tmp104.f1=s;_tmp104;});_tmp103;}),loc);}struct Cyc_Absyn_Stmt*Cyc_Absyn_splice_stmt(
struct Cyc_Absyn_Stmt*s,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((
void*)({struct Cyc_Absyn_Splice_s_struct*_tmp105=_cycalloc(sizeof(*_tmp105));
_tmp105[0]=({struct Cyc_Absyn_Splice_s_struct _tmp106;_tmp106.tag=14;_tmp106.f1=s;
_tmp106;});_tmp105;}),loc);}struct Cyc_Absyn_Stmt*Cyc_Absyn_label_stmt(struct
_tagged_arr*v,struct Cyc_Absyn_Stmt*s,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((
void*)({struct Cyc_Absyn_Label_s_struct*_tmp107=_cycalloc(sizeof(*_tmp107));
_tmp107[0]=({struct Cyc_Absyn_Label_s_struct _tmp108;_tmp108.tag=15;_tmp108.f1=v;
_tmp108.f2=s;_tmp108;});_tmp107;}),loc);}struct Cyc_Absyn_Stmt*Cyc_Absyn_do_stmt(
struct Cyc_Absyn_Stmt*s,struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_Do_s_struct*_tmp109=_cycalloc(
sizeof(*_tmp109));_tmp109[0]=({struct Cyc_Absyn_Do_s_struct _tmp10A;_tmp10A.tag=16;
_tmp10A.f1=s;_tmp10A.f2=({struct _tuple2 _tmp10B;_tmp10B.f1=e;_tmp10B.f2=Cyc_Absyn_skip_stmt(
e->loc);_tmp10B;});_tmp10A;});_tmp109;}),loc);}struct Cyc_Absyn_Stmt*Cyc_Absyn_trycatch_stmt(
struct Cyc_Absyn_Stmt*s,struct Cyc_List_List*scs,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_TryCatch_s_struct*_tmp10C=
_cycalloc(sizeof(*_tmp10C));_tmp10C[0]=({struct Cyc_Absyn_TryCatch_s_struct
_tmp10D;_tmp10D.tag=17;_tmp10D.f1=s;_tmp10D.f2=scs;_tmp10D;});_tmp10C;}),loc);}
struct Cyc_Absyn_Stmt*Cyc_Absyn_goto_stmt(struct _tagged_arr*lab,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_Goto_s_struct*_tmp10E=
_cycalloc(sizeof(*_tmp10E));_tmp10E[0]=({struct Cyc_Absyn_Goto_s_struct _tmp10F;
_tmp10F.tag=7;_tmp10F.f1=lab;_tmp10F.f2=0;_tmp10F;});_tmp10E;}),loc);}struct Cyc_Absyn_Stmt*
Cyc_Absyn_assign_stmt(struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_exp_stmt(Cyc_Absyn_assign_exp(e1,e2,loc),loc);}struct Cyc_Absyn_Stmt*
Cyc_Absyn_forarray_stmt(struct Cyc_List_List*d,struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*
e2,struct Cyc_Absyn_Stmt*s,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((
void*)({struct Cyc_Absyn_ForArray_s_struct*_tmp110=_cycalloc(sizeof(*_tmp110));
_tmp110[0]=({struct Cyc_Absyn_ForArray_s_struct _tmp111;_tmp111.tag=19;_tmp111.f1=({
struct Cyc_Absyn_ForArrayInfo _tmp112;_tmp112.defns=d;_tmp112.condition=({struct
_tuple2 _tmp114;_tmp114.f1=e1;_tmp114.f2=Cyc_Absyn_skip_stmt(e1->loc);_tmp114;});
_tmp112.delta=({struct _tuple2 _tmp113;_tmp113.f1=e2;_tmp113.f2=Cyc_Absyn_skip_stmt(
e2->loc);_tmp113;});_tmp112.body=s;_tmp112;});_tmp111;});_tmp110;}),loc);}struct
Cyc_Absyn_Pat*Cyc_Absyn_new_pat(void*p,struct Cyc_Position_Segment*s){return({
struct Cyc_Absyn_Pat*_tmp115=_cycalloc(sizeof(*_tmp115));_tmp115->r=(void*)p;
_tmp115->topt=0;_tmp115->loc=s;_tmp115;});}struct Cyc_Absyn_Decl*Cyc_Absyn_new_decl(
void*r,struct Cyc_Position_Segment*loc){return({struct Cyc_Absyn_Decl*_tmp116=
_cycalloc(sizeof(*_tmp116));_tmp116->r=(void*)r;_tmp116->loc=loc;_tmp116;});}
struct Cyc_Absyn_Decl*Cyc_Absyn_let_decl(struct Cyc_Absyn_Pat*p,struct Cyc_Absyn_Exp*
e,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_decl((void*)({struct Cyc_Absyn_Let_d_struct*
_tmp117=_cycalloc(sizeof(*_tmp117));_tmp117[0]=({struct Cyc_Absyn_Let_d_struct
_tmp118;_tmp118.tag=2;_tmp118.f1=p;_tmp118.f2=0;_tmp118.f3=e;_tmp118;});_tmp117;}),
loc);}struct Cyc_Absyn_Decl*Cyc_Absyn_letv_decl(struct Cyc_List_List*vds,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_new_decl((void*)({struct Cyc_Absyn_Letv_d_struct*_tmp119=
_cycalloc(sizeof(*_tmp119));_tmp119[0]=({struct Cyc_Absyn_Letv_d_struct _tmp11A;
_tmp11A.tag=3;_tmp11A.f1=vds;_tmp11A;});_tmp119;}),loc);}struct Cyc_Absyn_Vardecl*
Cyc_Absyn_new_vardecl(struct _tuple0*x,void*t,struct Cyc_Absyn_Exp*init){return({
struct Cyc_Absyn_Vardecl*_tmp11B=_cycalloc(sizeof(*_tmp11B));_tmp11B->sc=(void*)((
void*)2);_tmp11B->name=x;_tmp11B->tq=Cyc_Absyn_empty_tqual();_tmp11B->type=(void*)
t;_tmp11B->initializer=init;_tmp11B->rgn=0;_tmp11B->attributes=0;_tmp11B->escapes=
0;_tmp11B;});}struct Cyc_Absyn_Vardecl*Cyc_Absyn_static_vardecl(struct _tuple0*x,
void*t,struct Cyc_Absyn_Exp*init){return({struct Cyc_Absyn_Vardecl*_tmp11C=
_cycalloc(sizeof(*_tmp11C));_tmp11C->sc=(void*)((void*)0);_tmp11C->name=x;
_tmp11C->tq=Cyc_Absyn_empty_tqual();_tmp11C->type=(void*)t;_tmp11C->initializer=
init;_tmp11C->rgn=0;_tmp11C->attributes=0;_tmp11C->escapes=0;_tmp11C;});}struct
Cyc_Absyn_AggrdeclImpl*Cyc_Absyn_aggrdecl_impl(struct Cyc_List_List*exists,struct
Cyc_List_List*po,struct Cyc_List_List*fs){return({struct Cyc_Absyn_AggrdeclImpl*
_tmp11D=_cycalloc(sizeof(*_tmp11D));_tmp11D->exist_vars=exists;_tmp11D->rgn_po=
po;_tmp11D->fields=fs;_tmp11D;});}struct Cyc_Absyn_Decl*Cyc_Absyn_aggr_decl(void*
k,void*s,struct _tuple0*n,struct Cyc_List_List*ts,struct Cyc_Absyn_AggrdeclImpl*i,
struct Cyc_List_List*atts,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_decl((
void*)({struct Cyc_Absyn_Aggr_d_struct*_tmp11E=_cycalloc(sizeof(*_tmp11E));
_tmp11E[0]=({struct Cyc_Absyn_Aggr_d_struct _tmp11F;_tmp11F.tag=4;_tmp11F.f1=({
struct Cyc_Absyn_Aggrdecl*_tmp120=_cycalloc(sizeof(*_tmp120));_tmp120->kind=(void*)
k;_tmp120->sc=(void*)s;_tmp120->name=n;_tmp120->tvs=ts;_tmp120->impl=i;_tmp120->attributes=
atts;_tmp120;});_tmp11F;});_tmp11E;}),loc);}struct Cyc_Absyn_Decl*Cyc_Absyn_struct_decl(
void*s,struct _tuple0*n,struct Cyc_List_List*ts,struct Cyc_Absyn_AggrdeclImpl*i,
struct Cyc_List_List*atts,struct Cyc_Position_Segment*loc){return Cyc_Absyn_aggr_decl((
void*)0,s,n,ts,i,atts,loc);}struct Cyc_Absyn_Decl*Cyc_Absyn_union_decl(void*s,
struct _tuple0*n,struct Cyc_List_List*ts,struct Cyc_Absyn_AggrdeclImpl*i,struct Cyc_List_List*
atts,struct Cyc_Position_Segment*loc){return Cyc_Absyn_aggr_decl((void*)1,s,n,ts,i,
atts,loc);}struct Cyc_Absyn_Decl*Cyc_Absyn_tunion_decl(void*s,struct _tuple0*n,
struct Cyc_List_List*ts,struct Cyc_Core_Opt*fs,int is_xtunion,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_new_decl((void*)({struct Cyc_Absyn_Tunion_d_struct*_tmp121=
_cycalloc(sizeof(*_tmp121));_tmp121[0]=({struct Cyc_Absyn_Tunion_d_struct _tmp122;
_tmp122.tag=5;_tmp122.f1=({struct Cyc_Absyn_Tuniondecl*_tmp123=_cycalloc(sizeof(*
_tmp123));_tmp123->sc=(void*)s;_tmp123->name=n;_tmp123->tvs=ts;_tmp123->fields=
fs;_tmp123->is_xtunion=is_xtunion;_tmp123;});_tmp122;});_tmp121;}),loc);}static
struct _tuple1*Cyc_Absyn_expand_arg(struct _tuple1*a){return({struct _tuple1*
_tmp124=_cycalloc(sizeof(*_tmp124));_tmp124->f1=(*a).f1;_tmp124->f2=(*a).f2;
_tmp124->f3=Cyc_Absyn_pointer_expand((*a).f3);_tmp124;});}void*Cyc_Absyn_function_typ(
struct Cyc_List_List*tvs,struct Cyc_Core_Opt*eff_typ,void*ret_typ,struct Cyc_List_List*
args,int c_varargs,struct Cyc_Absyn_VarargInfo*cyc_varargs,struct Cyc_List_List*
rgn_po,struct Cyc_List_List*atts){return(void*)({struct Cyc_Absyn_FnType_struct*
_tmp125=_cycalloc(sizeof(*_tmp125));_tmp125[0]=({struct Cyc_Absyn_FnType_struct
_tmp126;_tmp126.tag=8;_tmp126.f1=({struct Cyc_Absyn_FnInfo _tmp127;_tmp127.tvars=
tvs;_tmp127.ret_typ=(void*)Cyc_Absyn_pointer_expand(ret_typ);_tmp127.effect=
eff_typ;_tmp127.args=((struct Cyc_List_List*(*)(struct _tuple1*(*f)(struct _tuple1*),
struct Cyc_List_List*x))Cyc_List_map)(Cyc_Absyn_expand_arg,args);_tmp127.c_varargs=
c_varargs;_tmp127.cyc_varargs=cyc_varargs;_tmp127.rgn_po=rgn_po;_tmp127.attributes=
atts;_tmp127;});_tmp126;});_tmp125;});}void*Cyc_Absyn_pointer_expand(void*t){
void*_tmp128=Cyc_Tcutil_compress(t);_LL43: if(_tmp128 <= (void*)3?1:*((int*)
_tmp128)!= 8)goto _LL45;_LL44: return Cyc_Absyn_at_typ(t,(void*)2,Cyc_Absyn_empty_tqual(),
Cyc_Absyn_false_conref);_LL45:;_LL46: return t;_LL42:;}int Cyc_Absyn_is_lvalue(
struct Cyc_Absyn_Exp*e){void*_tmp129=(void*)e->r;void*_tmp12A;void*_tmp12B;struct
Cyc_Absyn_Vardecl*_tmp12C;void*_tmp12D;struct Cyc_Absyn_Vardecl*_tmp12E;struct Cyc_Absyn_Exp*
_tmp12F;struct Cyc_Absyn_Exp*_tmp130;struct Cyc_Absyn_Exp*_tmp131;_LL48: if(*((int*)
_tmp129)!= 1)goto _LL4A;_tmp12A=(void*)((struct Cyc_Absyn_Var_e_struct*)_tmp129)->f2;
if(_tmp12A <= (void*)1?1:*((int*)_tmp12A)!= 1)goto _LL4A;_LL49: return 0;_LL4A: if(*((
int*)_tmp129)!= 1)goto _LL4C;_tmp12B=(void*)((struct Cyc_Absyn_Var_e_struct*)
_tmp129)->f2;if(_tmp12B <= (void*)1?1:*((int*)_tmp12B)!= 0)goto _LL4C;_tmp12C=((
struct Cyc_Absyn_Global_b_struct*)_tmp12B)->f1;_LL4B: _tmp12E=_tmp12C;goto _LL4D;
_LL4C: if(*((int*)_tmp129)!= 1)goto _LL4E;_tmp12D=(void*)((struct Cyc_Absyn_Var_e_struct*)
_tmp129)->f2;if(_tmp12D <= (void*)1?1:*((int*)_tmp12D)!= 3)goto _LL4E;_tmp12E=((
struct Cyc_Absyn_Local_b_struct*)_tmp12D)->f1;_LL4D: {void*_tmp132=Cyc_Tcutil_compress((
void*)_tmp12E->type);_LL5F: if(_tmp132 <= (void*)3?1:*((int*)_tmp132)!= 7)goto
_LL61;_LL60: return 0;_LL61:;_LL62: return 1;_LL5E:;}_LL4E: if(*((int*)_tmp129)!= 1)
goto _LL50;_LL4F: goto _LL51;_LL50: if(*((int*)_tmp129)!= 22)goto _LL52;_LL51: goto
_LL53;_LL52: if(*((int*)_tmp129)!= 20)goto _LL54;_LL53: goto _LL55;_LL54: if(*((int*)
_tmp129)!= 23)goto _LL56;_LL55: return 1;_LL56: if(*((int*)_tmp129)!= 21)goto _LL58;
_tmp12F=((struct Cyc_Absyn_AggrMember_e_struct*)_tmp129)->f1;_LL57: return Cyc_Absyn_is_lvalue(
_tmp12F);_LL58: if(*((int*)_tmp129)!= 12)goto _LL5A;_tmp130=((struct Cyc_Absyn_Instantiate_e_struct*)
_tmp129)->f1;_LL59: return Cyc_Absyn_is_lvalue(_tmp130);_LL5A: if(*((int*)_tmp129)
!= 11)goto _LL5C;_tmp131=((struct Cyc_Absyn_NoInstantiate_e_struct*)_tmp129)->f1;
_LL5B: return Cyc_Absyn_is_lvalue(_tmp131);_LL5C:;_LL5D: return 0;_LL47:;}struct Cyc_Absyn_Aggrfield*
Cyc_Absyn_lookup_field(struct Cyc_List_List*fields,struct _tagged_arr*v){{struct
Cyc_List_List*_tmp133=fields;for(0;_tmp133 != 0;_tmp133=_tmp133->tl){if(Cyc_strptrcmp(((
struct Cyc_Absyn_Aggrfield*)_tmp133->hd)->name,v)== 0)return(struct Cyc_Absyn_Aggrfield*)((
struct Cyc_Absyn_Aggrfield*)_tmp133->hd);}}return 0;}struct Cyc_Absyn_Aggrfield*Cyc_Absyn_lookup_decl_field(
struct Cyc_Absyn_Aggrdecl*ad,struct _tagged_arr*v){return ad->impl == 0?0: Cyc_Absyn_lookup_field(((
struct Cyc_Absyn_AggrdeclImpl*)_check_null(ad->impl))->fields,v);}struct _tuple3*
Cyc_Absyn_lookup_tuple_field(struct Cyc_List_List*ts,int i){for(0;i != 0;-- i){if(ts
== 0)return 0;ts=ts->tl;}if(ts == 0)return 0;return(struct _tuple3*)((struct _tuple3*)
ts->hd);}struct _tagged_arr Cyc_Absyn_attribute2string(void*a){void*_tmp134=a;int
_tmp135;int _tmp136;struct _tagged_arr _tmp137;void*_tmp138;int _tmp139;int _tmp13A;
void*_tmp13B;int _tmp13C;int _tmp13D;int _tmp13E;struct _tagged_arr _tmp13F;_LL64: if(
_tmp134 <= (void*)17?1:*((int*)_tmp134)!= 0)goto _LL66;_tmp135=((struct Cyc_Absyn_Regparm_att_struct*)
_tmp134)->f1;_LL65: return(struct _tagged_arr)({struct Cyc_Int_pa_struct _tmp142;
_tmp142.tag=1;_tmp142.f1=(unsigned int)_tmp135;{void*_tmp140[1]={& _tmp142};Cyc_aprintf(({
const char*_tmp141="regparm(%d)";_tag_arr(_tmp141,sizeof(char),_get_zero_arr_size(
_tmp141,12));}),_tag_arr(_tmp140,sizeof(void*),1));}});_LL66: if((int)_tmp134 != 0)
goto _LL68;_LL67: return({const char*_tmp143="stdcall";_tag_arr(_tmp143,sizeof(char),
_get_zero_arr_size(_tmp143,8));});_LL68: if((int)_tmp134 != 1)goto _LL6A;_LL69:
return({const char*_tmp144="cdecl";_tag_arr(_tmp144,sizeof(char),
_get_zero_arr_size(_tmp144,6));});_LL6A: if((int)_tmp134 != 2)goto _LL6C;_LL6B:
return({const char*_tmp145="fastcall";_tag_arr(_tmp145,sizeof(char),
_get_zero_arr_size(_tmp145,9));});_LL6C: if((int)_tmp134 != 3)goto _LL6E;_LL6D:
return({const char*_tmp146="noreturn";_tag_arr(_tmp146,sizeof(char),
_get_zero_arr_size(_tmp146,9));});_LL6E: if((int)_tmp134 != 4)goto _LL70;_LL6F:
return({const char*_tmp147="const";_tag_arr(_tmp147,sizeof(char),
_get_zero_arr_size(_tmp147,6));});_LL70: if(_tmp134 <= (void*)17?1:*((int*)_tmp134)
!= 1)goto _LL72;_tmp136=((struct Cyc_Absyn_Aligned_att_struct*)_tmp134)->f1;_LL71:
if(_tmp136 == - 1)return({const char*_tmp148="aligned";_tag_arr(_tmp148,sizeof(char),
_get_zero_arr_size(_tmp148,8));});else{return(struct _tagged_arr)({struct Cyc_Int_pa_struct
_tmp14B;_tmp14B.tag=1;_tmp14B.f1=(unsigned int)_tmp136;{void*_tmp149[1]={&
_tmp14B};Cyc_aprintf(({const char*_tmp14A="aligned(%d)";_tag_arr(_tmp14A,sizeof(
char),_get_zero_arr_size(_tmp14A,12));}),_tag_arr(_tmp149,sizeof(void*),1));}});}
_LL72: if((int)_tmp134 != 5)goto _LL74;_LL73: return({const char*_tmp14C="packed";
_tag_arr(_tmp14C,sizeof(char),_get_zero_arr_size(_tmp14C,7));});_LL74: if(_tmp134
<= (void*)17?1:*((int*)_tmp134)!= 2)goto _LL76;_tmp137=((struct Cyc_Absyn_Section_att_struct*)
_tmp134)->f1;_LL75: return(struct _tagged_arr)({struct Cyc_String_pa_struct _tmp14F;
_tmp14F.tag=0;_tmp14F.f1=(struct _tagged_arr)((struct _tagged_arr)_tmp137);{void*
_tmp14D[1]={& _tmp14F};Cyc_aprintf(({const char*_tmp14E="section(\"%s\")";_tag_arr(
_tmp14E,sizeof(char),_get_zero_arr_size(_tmp14E,14));}),_tag_arr(_tmp14D,sizeof(
void*),1));}});_LL76: if((int)_tmp134 != 6)goto _LL78;_LL77: return({const char*
_tmp150="nocommon";_tag_arr(_tmp150,sizeof(char),_get_zero_arr_size(_tmp150,9));});
_LL78: if((int)_tmp134 != 7)goto _LL7A;_LL79: return({const char*_tmp151="shared";
_tag_arr(_tmp151,sizeof(char),_get_zero_arr_size(_tmp151,7));});_LL7A: if((int)
_tmp134 != 8)goto _LL7C;_LL7B: return({const char*_tmp152="unused";_tag_arr(_tmp152,
sizeof(char),_get_zero_arr_size(_tmp152,7));});_LL7C: if((int)_tmp134 != 9)goto
_LL7E;_LL7D: return({const char*_tmp153="weak";_tag_arr(_tmp153,sizeof(char),
_get_zero_arr_size(_tmp153,5));});_LL7E: if((int)_tmp134 != 10)goto _LL80;_LL7F:
return({const char*_tmp154="dllimport";_tag_arr(_tmp154,sizeof(char),
_get_zero_arr_size(_tmp154,10));});_LL80: if((int)_tmp134 != 11)goto _LL82;_LL81:
return({const char*_tmp155="dllexport";_tag_arr(_tmp155,sizeof(char),
_get_zero_arr_size(_tmp155,10));});_LL82: if((int)_tmp134 != 12)goto _LL84;_LL83:
return({const char*_tmp156="no_instrument_function";_tag_arr(_tmp156,sizeof(char),
_get_zero_arr_size(_tmp156,23));});_LL84: if((int)_tmp134 != 13)goto _LL86;_LL85:
return({const char*_tmp157="constructor";_tag_arr(_tmp157,sizeof(char),
_get_zero_arr_size(_tmp157,12));});_LL86: if((int)_tmp134 != 14)goto _LL88;_LL87:
return({const char*_tmp158="destructor";_tag_arr(_tmp158,sizeof(char),
_get_zero_arr_size(_tmp158,11));});_LL88: if((int)_tmp134 != 15)goto _LL8A;_LL89:
return({const char*_tmp159="no_check_memory_usage";_tag_arr(_tmp159,sizeof(char),
_get_zero_arr_size(_tmp159,22));});_LL8A: if(_tmp134 <= (void*)17?1:*((int*)
_tmp134)!= 3)goto _LL8C;_tmp138=(void*)((struct Cyc_Absyn_Format_att_struct*)
_tmp134)->f1;if((int)_tmp138 != 0)goto _LL8C;_tmp139=((struct Cyc_Absyn_Format_att_struct*)
_tmp134)->f2;_tmp13A=((struct Cyc_Absyn_Format_att_struct*)_tmp134)->f3;_LL8B:
return(struct _tagged_arr)({struct Cyc_Int_pa_struct _tmp15D;_tmp15D.tag=1;_tmp15D.f1=(
unsigned int)_tmp13A;{struct Cyc_Int_pa_struct _tmp15C;_tmp15C.tag=1;_tmp15C.f1=(
unsigned int)_tmp139;{void*_tmp15A[2]={& _tmp15C,& _tmp15D};Cyc_aprintf(({const
char*_tmp15B="format(printf,%u,%u)";_tag_arr(_tmp15B,sizeof(char),
_get_zero_arr_size(_tmp15B,21));}),_tag_arr(_tmp15A,sizeof(void*),2));}}});_LL8C:
if(_tmp134 <= (void*)17?1:*((int*)_tmp134)!= 3)goto _LL8E;_tmp13B=(void*)((struct
Cyc_Absyn_Format_att_struct*)_tmp134)->f1;if((int)_tmp13B != 1)goto _LL8E;_tmp13C=((
struct Cyc_Absyn_Format_att_struct*)_tmp134)->f2;_tmp13D=((struct Cyc_Absyn_Format_att_struct*)
_tmp134)->f3;_LL8D: return(struct _tagged_arr)({struct Cyc_Int_pa_struct _tmp161;
_tmp161.tag=1;_tmp161.f1=(unsigned int)_tmp13D;{struct Cyc_Int_pa_struct _tmp160;
_tmp160.tag=1;_tmp160.f1=(unsigned int)_tmp13C;{void*_tmp15E[2]={& _tmp160,&
_tmp161};Cyc_aprintf(({const char*_tmp15F="format(scanf,%u,%u)";_tag_arr(_tmp15F,
sizeof(char),_get_zero_arr_size(_tmp15F,20));}),_tag_arr(_tmp15E,sizeof(void*),2));}}});
_LL8E: if(_tmp134 <= (void*)17?1:*((int*)_tmp134)!= 4)goto _LL90;_tmp13E=((struct
Cyc_Absyn_Initializes_att_struct*)_tmp134)->f1;_LL8F: return(struct _tagged_arr)({
struct Cyc_Int_pa_struct _tmp164;_tmp164.tag=1;_tmp164.f1=(unsigned int)_tmp13E;{
void*_tmp162[1]={& _tmp164};Cyc_aprintf(({const char*_tmp163="initializes(%d)";
_tag_arr(_tmp163,sizeof(char),_get_zero_arr_size(_tmp163,16));}),_tag_arr(
_tmp162,sizeof(void*),1));}});_LL90: if((int)_tmp134 != 16)goto _LL92;_LL91: return({
const char*_tmp165="pure";_tag_arr(_tmp165,sizeof(char),_get_zero_arr_size(
_tmp165,5));});_LL92: if(_tmp134 <= (void*)17?1:*((int*)_tmp134)!= 5)goto _LL63;
_tmp13F=((struct Cyc_Absyn_Mode_att_struct*)_tmp134)->f1;_LL93: return(struct
_tagged_arr)({struct Cyc_String_pa_struct _tmp168;_tmp168.tag=0;_tmp168.f1=(struct
_tagged_arr)((struct _tagged_arr)_tmp13F);{void*_tmp166[1]={& _tmp168};Cyc_aprintf(({
const char*_tmp167="__mode__(\"%s\")";_tag_arr(_tmp167,sizeof(char),
_get_zero_arr_size(_tmp167,15));}),_tag_arr(_tmp166,sizeof(void*),1));}});_LL63:;}
int Cyc_Absyn_fntype_att(void*a){void*_tmp169=a;_LL95: if(_tmp169 <= (void*)17?1:*((
int*)_tmp169)!= 0)goto _LL97;_LL96: goto _LL98;_LL97: if((int)_tmp169 != 2)goto _LL99;
_LL98: goto _LL9A;_LL99: if((int)_tmp169 != 0)goto _LL9B;_LL9A: goto _LL9C;_LL9B: if((
int)_tmp169 != 1)goto _LL9D;_LL9C: goto _LL9E;_LL9D: if((int)_tmp169 != 3)goto _LL9F;
_LL9E: goto _LLA0;_LL9F: if((int)_tmp169 != 16)goto _LLA1;_LLA0: goto _LLA2;_LLA1: if(
_tmp169 <= (void*)17?1:*((int*)_tmp169)!= 3)goto _LLA3;_LLA2: goto _LLA4;_LLA3: if((
int)_tmp169 != 4)goto _LLA5;_LLA4: return 1;_LLA5: if(_tmp169 <= (void*)17?1:*((int*)
_tmp169)!= 4)goto _LLA7;_LLA6: return 1;_LLA7:;_LLA8: return 0;_LL94:;}static char
_tmp16A[3]="f0";static struct _tagged_arr Cyc_Absyn_f0={_tmp16A,_tmp16A,_tmp16A + 3};
static struct _tagged_arr*Cyc_Absyn_field_names_v[1]={& Cyc_Absyn_f0};static struct
_tagged_arr Cyc_Absyn_field_names={(void*)((struct _tagged_arr**)Cyc_Absyn_field_names_v),(
void*)((struct _tagged_arr**)Cyc_Absyn_field_names_v),(void*)((struct _tagged_arr**)
Cyc_Absyn_field_names_v + 1)};struct _tagged_arr*Cyc_Absyn_fieldname(int i){
unsigned int fsz=_get_arr_size(Cyc_Absyn_field_names,sizeof(struct _tagged_arr*));
if(i >= fsz)Cyc_Absyn_field_names=({unsigned int _tmp16B=(unsigned int)(i + 1);
struct _tagged_arr**_tmp16C=(struct _tagged_arr**)_cycalloc(_check_times(sizeof(
struct _tagged_arr*),_tmp16B));struct _tagged_arr _tmp172=_tag_arr(_tmp16C,sizeof(
struct _tagged_arr*),_tmp16B);{unsigned int _tmp16D=_tmp16B;unsigned int j;for(j=0;
j < _tmp16D;j ++){_tmp16C[j]=j < fsz?*((struct _tagged_arr**)
_check_unknown_subscript(Cyc_Absyn_field_names,sizeof(struct _tagged_arr*),(int)j)):({
struct _tagged_arr*_tmp16E=_cycalloc(sizeof(*_tmp16E));_tmp16E[0]=(struct
_tagged_arr)({struct Cyc_Int_pa_struct _tmp171;_tmp171.tag=1;_tmp171.f1=(
unsigned int)((int)j);{void*_tmp16F[1]={& _tmp171};Cyc_aprintf(({const char*
_tmp170="f%d";_tag_arr(_tmp170,sizeof(char),_get_zero_arr_size(_tmp170,4));}),
_tag_arr(_tmp16F,sizeof(void*),1));}});_tmp16E;});}}_tmp172;});return*((struct
_tagged_arr**)_check_unknown_subscript(Cyc_Absyn_field_names,sizeof(struct
_tagged_arr*),i));}struct _tuple4 Cyc_Absyn_aggr_kinded_name(void*info){void*
_tmp173=info;void*_tmp174;struct _tuple0*_tmp175;struct Cyc_Absyn_Aggrdecl**
_tmp176;struct Cyc_Absyn_Aggrdecl*_tmp177;struct Cyc_Absyn_Aggrdecl _tmp178;void*
_tmp179;struct _tuple0*_tmp17A;_LLAA: if(*((int*)_tmp173)!= 0)goto _LLAC;_tmp174=(
void*)((struct Cyc_Absyn_UnknownAggr_struct*)_tmp173)->f1;_tmp175=((struct Cyc_Absyn_UnknownAggr_struct*)
_tmp173)->f2;_LLAB: return({struct _tuple4 _tmp17B;_tmp17B.f1=_tmp174;_tmp17B.f2=
_tmp175;_tmp17B;});_LLAC: if(*((int*)_tmp173)!= 1)goto _LLA9;_tmp176=((struct Cyc_Absyn_KnownAggr_struct*)
_tmp173)->f1;_tmp177=*_tmp176;_tmp178=*_tmp177;_tmp179=(void*)_tmp178.kind;
_tmp17A=_tmp178.name;_LLAD: return({struct _tuple4 _tmp17C;_tmp17C.f1=_tmp179;
_tmp17C.f2=_tmp17A;_tmp17C;});_LLA9:;}struct Cyc_Absyn_Aggrdecl*Cyc_Absyn_get_known_aggrdecl(
void*info){void*_tmp17D=info;void*_tmp17E;struct _tuple0*_tmp17F;struct Cyc_Absyn_Aggrdecl**
_tmp180;struct Cyc_Absyn_Aggrdecl*_tmp181;_LLAF: if(*((int*)_tmp17D)!= 0)goto _LLB1;
_tmp17E=(void*)((struct Cyc_Absyn_UnknownAggr_struct*)_tmp17D)->f1;_tmp17F=((
struct Cyc_Absyn_UnknownAggr_struct*)_tmp17D)->f2;_LLB0:({void*_tmp182[0]={};((
int(*)(struct _tagged_arr fmt,struct _tagged_arr ap))Cyc_Tcutil_impos)(({const char*
_tmp183="unchecked aggrdecl";_tag_arr(_tmp183,sizeof(char),_get_zero_arr_size(
_tmp183,19));}),_tag_arr(_tmp182,sizeof(void*),0));});_LLB1: if(*((int*)_tmp17D)
!= 1)goto _LLAE;_tmp180=((struct Cyc_Absyn_KnownAggr_struct*)_tmp17D)->f1;_tmp181=*
_tmp180;_LLB2: return _tmp181;_LLAE:;}int Cyc_Absyn_is_union_type(void*t){void*
_tmp184=Cyc_Tcutil_compress(t);void*_tmp185;struct Cyc_Absyn_AggrInfo _tmp186;void*
_tmp187;_LLB4: if(_tmp184 <= (void*)3?1:*((int*)_tmp184)!= 11)goto _LLB6;_tmp185=(
void*)((struct Cyc_Absyn_AnonAggrType_struct*)_tmp184)->f1;if((int)_tmp185 != 1)
goto _LLB6;_LLB5: return 1;_LLB6: if(_tmp184 <= (void*)3?1:*((int*)_tmp184)!= 10)goto
_LLB8;_tmp186=((struct Cyc_Absyn_AggrType_struct*)_tmp184)->f1;_tmp187=(void*)
_tmp186.aggr_info;_LLB7: return(Cyc_Absyn_aggr_kinded_name(_tmp187)).f1 == (void*)
1;_LLB8:;_LLB9: return 0;_LLB3:;}void Cyc_Absyn_print_decls(struct Cyc_List_List*
decls){((void(*)(void*rep,struct Cyc_List_List**val))Cyc_Marshal_print_type)(Cyc_decls_rep,&
decls);({void*_tmp188[0]={};Cyc_printf(({const char*_tmp189="\n";_tag_arr(_tmp189,
sizeof(char),_get_zero_arr_size(_tmp189,2));}),_tag_arr(_tmp188,sizeof(void*),0));});}
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_0;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List0Absyn_decl_t46H2_rep;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_1;extern struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Decl_rep;
extern struct Cyc_Typerep_TUnion_struct Cyc_Absyn_raw_decl_t_rep;extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_431;static struct Cyc_Typerep_Int_struct Cyc__genrep_5={0,0,32};extern
struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_134;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_Vardecl_rep;extern struct Cyc_Typerep_TUnion_struct Cyc_Absyn_scope_t_rep;
static char _tmp18B[7]="Static";static struct _tuple7 Cyc__gentuple_138={0,(struct
_tagged_arr){_tmp18B,_tmp18B,_tmp18B + 7}};static char _tmp18C[9]="Abstract";static
struct _tuple7 Cyc__gentuple_139={1,(struct _tagged_arr){_tmp18C,_tmp18C,_tmp18C + 9}};
static char _tmp18D[7]="Public";static struct _tuple7 Cyc__gentuple_140={2,(struct
_tagged_arr){_tmp18D,_tmp18D,_tmp18D + 7}};static char _tmp18E[7]="Extern";static
struct _tuple7 Cyc__gentuple_141={3,(struct _tagged_arr){_tmp18E,_tmp18E,_tmp18E + 7}};
static char _tmp18F[8]="ExternC";static struct _tuple7 Cyc__gentuple_142={4,(struct
_tagged_arr){_tmp18F,_tmp18F,_tmp18F + 8}};static char _tmp190[9]="Register";static
struct _tuple7 Cyc__gentuple_143={5,(struct _tagged_arr){_tmp190,_tmp190,_tmp190 + 9}};
static struct _tuple7*Cyc__genarr_144[6]={& Cyc__gentuple_138,& Cyc__gentuple_139,&
Cyc__gentuple_140,& Cyc__gentuple_141,& Cyc__gentuple_142,& Cyc__gentuple_143};
static struct _tuple5*Cyc__genarr_145[0]={};static char _tmp192[6]="Scope";struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_scope_t_rep={5,(struct _tagged_arr){_tmp192,_tmp192,_tmp192 + 6},{(void*)((
struct _tuple7**)Cyc__genarr_144),(void*)((struct _tuple7**)Cyc__genarr_144),(void*)((
struct _tuple7**)Cyc__genarr_144 + 6)},{(void*)((struct _tuple5**)Cyc__genarr_145),(
void*)((struct _tuple5**)Cyc__genarr_145),(void*)((struct _tuple5**)Cyc__genarr_145
+ 0)}};extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_10;extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_11;extern struct Cyc_Typerep_TUnion_struct Cyc_Absyn_nmspace_t_rep;
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_17;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_18;extern struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_var_t46H2_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_12;extern struct Cyc_Typerep_FatPtr_struct
Cyc__genrep_13;static struct Cyc_Typerep_Int_struct Cyc__genrep_14={0,0,8};static
struct Cyc_Typerep_FatPtr_struct Cyc__genrep_13={2,(void*)((void*)& Cyc__genrep_14)};
static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_12={1,1,(void*)((void*)& Cyc__genrep_13)};
static char _tmp196[5]="List";static struct _tagged_arr Cyc__genname_22=(struct
_tagged_arr){_tmp196,_tmp196,_tmp196 + 5};static char _tmp197[3]="hd";static struct
_tuple5 Cyc__gentuple_19={offsetof(struct Cyc_List_List,hd),(struct _tagged_arr){
_tmp197,_tmp197,_tmp197 + 3},(void*)& Cyc__genrep_12};static char _tmp198[3]="tl";
static struct _tuple5 Cyc__gentuple_20={offsetof(struct Cyc_List_List,tl),(struct
_tagged_arr){_tmp198,_tmp198,_tmp198 + 3},(void*)& Cyc__genrep_18};static struct
_tuple5*Cyc__genarr_21[2]={& Cyc__gentuple_19,& Cyc__gentuple_20};struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List0Absyn_var_t46H2_rep={3,(struct _tagged_arr*)& Cyc__genname_22,
sizeof(struct Cyc_List_List),{(void*)((struct _tuple5**)Cyc__genarr_21),(void*)((
struct _tuple5**)Cyc__genarr_21),(void*)((struct _tuple5**)Cyc__genarr_21 + 2)}};
static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_18={1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_var_t46H2_rep)};
struct _tuple13{unsigned int f1;struct Cyc_List_List*f2;};static struct _tuple6 Cyc__gentuple_23={
offsetof(struct _tuple13,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_24={
offsetof(struct _tuple13,f2),(void*)& Cyc__genrep_18};static struct _tuple6*Cyc__genarr_25[
2]={& Cyc__gentuple_23,& Cyc__gentuple_24};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_17={
4,sizeof(struct _tuple13),{(void*)((struct _tuple6**)Cyc__genarr_25),(void*)((
struct _tuple6**)Cyc__genarr_25),(void*)((struct _tuple6**)Cyc__genarr_25 + 2)}};
static char _tmp19C[6]="Loc_n";static struct _tuple7 Cyc__gentuple_15={0,(struct
_tagged_arr){_tmp19C,_tmp19C,_tmp19C + 6}};static struct _tuple7*Cyc__genarr_16[1]={&
Cyc__gentuple_15};static char _tmp19D[6]="Rel_n";static struct _tuple5 Cyc__gentuple_26={
0,(struct _tagged_arr){_tmp19D,_tmp19D,_tmp19D + 6},(void*)& Cyc__genrep_17};static
char _tmp19E[6]="Abs_n";static struct _tuple5 Cyc__gentuple_27={1,(struct _tagged_arr){
_tmp19E,_tmp19E,_tmp19E + 6},(void*)& Cyc__genrep_17};static struct _tuple5*Cyc__genarr_28[
2]={& Cyc__gentuple_26,& Cyc__gentuple_27};static char _tmp1A0[8]="Nmspace";struct
Cyc_Typerep_TUnion_struct Cyc_Absyn_nmspace_t_rep={5,(struct _tagged_arr){_tmp1A0,
_tmp1A0,_tmp1A0 + 8},{(void*)((struct _tuple7**)Cyc__genarr_16),(void*)((struct
_tuple7**)Cyc__genarr_16),(void*)((struct _tuple7**)Cyc__genarr_16 + 1)},{(void*)((
struct _tuple5**)Cyc__genarr_28),(void*)((struct _tuple5**)Cyc__genarr_28),(void*)((
struct _tuple5**)Cyc__genarr_28 + 2)}};static struct _tuple6 Cyc__gentuple_29={
offsetof(struct _tuple0,f1),(void*)& Cyc_Absyn_nmspace_t_rep};static struct _tuple6
Cyc__gentuple_30={offsetof(struct _tuple0,f2),(void*)& Cyc__genrep_12};static
struct _tuple6*Cyc__genarr_31[2]={& Cyc__gentuple_29,& Cyc__gentuple_30};static
struct Cyc_Typerep_Tuple_struct Cyc__genrep_11={4,sizeof(struct _tuple0),{(void*)((
struct _tuple6**)Cyc__genarr_31),(void*)((struct _tuple6**)Cyc__genarr_31),(void*)((
struct _tuple6**)Cyc__genarr_31 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_10={
1,1,(void*)((void*)& Cyc__genrep_11)};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_135;
struct _tuple14{char f1;};static struct _tuple6 Cyc__gentuple_136={offsetof(struct
_tuple14,f1),(void*)((void*)& Cyc__genrep_14)};static struct _tuple6*Cyc__genarr_137[
1]={& Cyc__gentuple_136};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_135={4,
sizeof(struct _tuple14),{(void*)((struct _tuple6**)Cyc__genarr_137),(void*)((
struct _tuple6**)Cyc__genarr_137),(void*)((struct _tuple6**)Cyc__genarr_137 + 1)}};
extern struct Cyc_Typerep_TUnion_struct Cyc_Absyn_type_t_rep;extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_1086;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_1091;extern
struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0Absyn_kind_t2_rep;extern
struct Cyc_Typerep_TUnion_struct Cyc_Absyn_kind_t_rep;static char _tmp1A4[8]="AnyKind";
static struct _tuple7 Cyc__gentuple_189={0,(struct _tagged_arr){_tmp1A4,_tmp1A4,
_tmp1A4 + 8}};static char _tmp1A5[8]="MemKind";static struct _tuple7 Cyc__gentuple_190={
1,(struct _tagged_arr){_tmp1A5,_tmp1A5,_tmp1A5 + 8}};static char _tmp1A6[8]="BoxKind";
static struct _tuple7 Cyc__gentuple_191={2,(struct _tagged_arr){_tmp1A6,_tmp1A6,
_tmp1A6 + 8}};static char _tmp1A7[8]="RgnKind";static struct _tuple7 Cyc__gentuple_192={
3,(struct _tagged_arr){_tmp1A7,_tmp1A7,_tmp1A7 + 8}};static char _tmp1A8[8]="EffKind";
static struct _tuple7 Cyc__gentuple_193={4,(struct _tagged_arr){_tmp1A8,_tmp1A8,
_tmp1A8 + 8}};static char _tmp1A9[8]="IntKind";static struct _tuple7 Cyc__gentuple_194={
5,(struct _tagged_arr){_tmp1A9,_tmp1A9,_tmp1A9 + 8}};static struct _tuple7*Cyc__genarr_195[
6]={& Cyc__gentuple_189,& Cyc__gentuple_190,& Cyc__gentuple_191,& Cyc__gentuple_192,&
Cyc__gentuple_193,& Cyc__gentuple_194};static struct _tuple5*Cyc__genarr_196[0]={};
static char _tmp1AB[5]="Kind";struct Cyc_Typerep_TUnion_struct Cyc_Absyn_kind_t_rep={
5,(struct _tagged_arr){_tmp1AB,_tmp1AB,_tmp1AB + 5},{(void*)((struct _tuple7**)Cyc__genarr_195),(
void*)((struct _tuple7**)Cyc__genarr_195),(void*)((struct _tuple7**)Cyc__genarr_195
+ 6)},{(void*)((struct _tuple5**)Cyc__genarr_196),(void*)((struct _tuple5**)Cyc__genarr_196),(
void*)((struct _tuple5**)Cyc__genarr_196 + 0)}};static char _tmp1AC[4]="Opt";static
struct _tagged_arr Cyc__genname_1094=(struct _tagged_arr){_tmp1AC,_tmp1AC,_tmp1AC + 
4};static char _tmp1AD[2]="v";static struct _tuple5 Cyc__gentuple_1092={offsetof(
struct Cyc_Core_Opt,v),(struct _tagged_arr){_tmp1AD,_tmp1AD,_tmp1AD + 2},(void*)&
Cyc_Absyn_kind_t_rep};static struct _tuple5*Cyc__genarr_1093[1]={& Cyc__gentuple_1092};
struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0Absyn_kind_t2_rep={3,(struct
_tagged_arr*)& Cyc__genname_1094,sizeof(struct Cyc_Core_Opt),{(void*)((struct
_tuple5**)Cyc__genarr_1093),(void*)((struct _tuple5**)Cyc__genarr_1093),(void*)((
struct _tuple5**)Cyc__genarr_1093 + 1)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_1091={
1,1,(void*)((void*)& Cyc_struct_Core_Opt0Absyn_kind_t2_rep)};extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_92;extern struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0Absyn_type_t2_rep;
static char _tmp1B0[4]="Opt";static struct _tagged_arr Cyc__genname_1126=(struct
_tagged_arr){_tmp1B0,_tmp1B0,_tmp1B0 + 4};static char _tmp1B1[2]="v";static struct
_tuple5 Cyc__gentuple_1124={offsetof(struct Cyc_Core_Opt,v),(struct _tagged_arr){
_tmp1B1,_tmp1B1,_tmp1B1 + 2},(void*)& Cyc_Absyn_type_t_rep};static struct _tuple5*
Cyc__genarr_1125[1]={& Cyc__gentuple_1124};struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0Absyn_type_t2_rep={
3,(struct _tagged_arr*)& Cyc__genname_1126,sizeof(struct Cyc_Core_Opt),{(void*)((
struct _tuple5**)Cyc__genarr_1125),(void*)((struct _tuple5**)Cyc__genarr_1125),(
void*)((struct _tuple5**)Cyc__genarr_1125 + 1)}};static struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_92={1,1,(void*)((void*)& Cyc_struct_Core_Opt0Absyn_type_t2_rep)};
static struct Cyc_Typerep_Int_struct Cyc__genrep_67={0,1,32};extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_1087;extern struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0List_list_t0Absyn_tvar_t46H22_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_312;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List0Absyn_tvar_t46H2_rep;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_186;extern struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Tvar_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_217;static struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_217={1,1,(void*)((void*)& Cyc__genrep_67)};extern struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_kindbound_t_rep;extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_209;
static struct _tuple6 Cyc__gentuple_210={offsetof(struct _tuple6,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_211={offsetof(struct _tuple6,f2),(void*)& Cyc_Absyn_kind_t_rep};
static struct _tuple6*Cyc__genarr_212[2]={& Cyc__gentuple_210,& Cyc__gentuple_211};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_209={4,sizeof(struct _tuple6),{(
void*)((struct _tuple6**)Cyc__genarr_212),(void*)((struct _tuple6**)Cyc__genarr_212),(
void*)((struct _tuple6**)Cyc__genarr_212 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_205;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_197;extern
struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0Absyn_kindbound_t2_rep;static
char _tmp1B7[4]="Opt";static struct _tagged_arr Cyc__genname_200=(struct _tagged_arr){
_tmp1B7,_tmp1B7,_tmp1B7 + 4};static char _tmp1B8[2]="v";static struct _tuple5 Cyc__gentuple_198={
offsetof(struct Cyc_Core_Opt,v),(struct _tagged_arr){_tmp1B8,_tmp1B8,_tmp1B8 + 2},(
void*)& Cyc_Absyn_kindbound_t_rep};static struct _tuple5*Cyc__genarr_199[1]={& Cyc__gentuple_198};
struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0Absyn_kindbound_t2_rep={3,(
struct _tagged_arr*)& Cyc__genname_200,sizeof(struct Cyc_Core_Opt),{(void*)((struct
_tuple5**)Cyc__genarr_199),(void*)((struct _tuple5**)Cyc__genarr_199),(void*)((
struct _tuple5**)Cyc__genarr_199 + 1)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_197={
1,1,(void*)((void*)& Cyc_struct_Core_Opt0Absyn_kindbound_t2_rep)};struct _tuple15{
unsigned int f1;struct Cyc_Core_Opt*f2;};static struct _tuple6 Cyc__gentuple_206={
offsetof(struct _tuple15,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_207={
offsetof(struct _tuple15,f2),(void*)& Cyc__genrep_197};static struct _tuple6*Cyc__genarr_208[
2]={& Cyc__gentuple_206,& Cyc__gentuple_207};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_205={4,sizeof(struct _tuple15),{(void*)((struct _tuple6**)Cyc__genarr_208),(
void*)((struct _tuple6**)Cyc__genarr_208),(void*)((struct _tuple6**)Cyc__genarr_208
+ 2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_188;struct _tuple16{
unsigned int f1;struct Cyc_Core_Opt*f2;void*f3;};static struct _tuple6 Cyc__gentuple_201={
offsetof(struct _tuple16,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_202={
offsetof(struct _tuple16,f2),(void*)& Cyc__genrep_197};static struct _tuple6 Cyc__gentuple_203={
offsetof(struct _tuple16,f3),(void*)& Cyc_Absyn_kind_t_rep};static struct _tuple6*
Cyc__genarr_204[3]={& Cyc__gentuple_201,& Cyc__gentuple_202,& Cyc__gentuple_203};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_188={4,sizeof(struct _tuple16),{(
void*)((struct _tuple6**)Cyc__genarr_204),(void*)((struct _tuple6**)Cyc__genarr_204),(
void*)((struct _tuple6**)Cyc__genarr_204 + 3)}};static struct _tuple7*Cyc__genarr_187[
0]={};static char _tmp1BD[6]="Eq_kb";static struct _tuple5 Cyc__gentuple_213={0,(
struct _tagged_arr){_tmp1BD,_tmp1BD,_tmp1BD + 6},(void*)& Cyc__genrep_209};static
char _tmp1BE[11]="Unknown_kb";static struct _tuple5 Cyc__gentuple_214={1,(struct
_tagged_arr){_tmp1BE,_tmp1BE,_tmp1BE + 11},(void*)& Cyc__genrep_205};static char
_tmp1BF[8]="Less_kb";static struct _tuple5 Cyc__gentuple_215={2,(struct _tagged_arr){
_tmp1BF,_tmp1BF,_tmp1BF + 8},(void*)& Cyc__genrep_188};static struct _tuple5*Cyc__genarr_216[
3]={& Cyc__gentuple_213,& Cyc__gentuple_214,& Cyc__gentuple_215};static char _tmp1C1[
10]="KindBound";struct Cyc_Typerep_TUnion_struct Cyc_Absyn_kindbound_t_rep={5,(
struct _tagged_arr){_tmp1C1,_tmp1C1,_tmp1C1 + 10},{(void*)((struct _tuple7**)Cyc__genarr_187),(
void*)((struct _tuple7**)Cyc__genarr_187),(void*)((struct _tuple7**)Cyc__genarr_187
+ 0)},{(void*)((struct _tuple5**)Cyc__genarr_216),(void*)((struct _tuple5**)Cyc__genarr_216),(
void*)((struct _tuple5**)Cyc__genarr_216 + 3)}};static char _tmp1C2[5]="Tvar";static
struct _tagged_arr Cyc__genname_222=(struct _tagged_arr){_tmp1C2,_tmp1C2,_tmp1C2 + 5};
static char _tmp1C3[5]="name";static struct _tuple5 Cyc__gentuple_218={offsetof(
struct Cyc_Absyn_Tvar,name),(struct _tagged_arr){_tmp1C3,_tmp1C3,_tmp1C3 + 5},(void*)&
Cyc__genrep_12};static char _tmp1C4[9]="identity";static struct _tuple5 Cyc__gentuple_219={
offsetof(struct Cyc_Absyn_Tvar,identity),(struct _tagged_arr){_tmp1C4,_tmp1C4,
_tmp1C4 + 9},(void*)& Cyc__genrep_217};static char _tmp1C5[5]="kind";static struct
_tuple5 Cyc__gentuple_220={offsetof(struct Cyc_Absyn_Tvar,kind),(struct _tagged_arr){
_tmp1C5,_tmp1C5,_tmp1C5 + 5},(void*)& Cyc_Absyn_kindbound_t_rep};static struct
_tuple5*Cyc__genarr_221[3]={& Cyc__gentuple_218,& Cyc__gentuple_219,& Cyc__gentuple_220};
struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Tvar_rep={3,(struct _tagged_arr*)&
Cyc__genname_222,sizeof(struct Cyc_Absyn_Tvar),{(void*)((struct _tuple5**)Cyc__genarr_221),(
void*)((struct _tuple5**)Cyc__genarr_221),(void*)((struct _tuple5**)Cyc__genarr_221
+ 3)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_186={1,1,(void*)((void*)&
Cyc_struct_Absyn_Tvar_rep)};static char _tmp1C8[5]="List";static struct _tagged_arr
Cyc__genname_316=(struct _tagged_arr){_tmp1C8,_tmp1C8,_tmp1C8 + 5};static char
_tmp1C9[3]="hd";static struct _tuple5 Cyc__gentuple_313={offsetof(struct Cyc_List_List,hd),(
struct _tagged_arr){_tmp1C9,_tmp1C9,_tmp1C9 + 3},(void*)& Cyc__genrep_186};static
char _tmp1CA[3]="tl";static struct _tuple5 Cyc__gentuple_314={offsetof(struct Cyc_List_List,tl),(
struct _tagged_arr){_tmp1CA,_tmp1CA,_tmp1CA + 3},(void*)& Cyc__genrep_312};static
struct _tuple5*Cyc__genarr_315[2]={& Cyc__gentuple_313,& Cyc__gentuple_314};struct
Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_tvar_t46H2_rep={3,(struct
_tagged_arr*)& Cyc__genname_316,sizeof(struct Cyc_List_List),{(void*)((struct
_tuple5**)Cyc__genarr_315),(void*)((struct _tuple5**)Cyc__genarr_315),(void*)((
struct _tuple5**)Cyc__genarr_315 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_312={
1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_tvar_t46H2_rep)};static char
_tmp1CD[4]="Opt";static struct _tagged_arr Cyc__genname_1090=(struct _tagged_arr){
_tmp1CD,_tmp1CD,_tmp1CD + 4};static char _tmp1CE[2]="v";static struct _tuple5 Cyc__gentuple_1088={
offsetof(struct Cyc_Core_Opt,v),(struct _tagged_arr){_tmp1CE,_tmp1CE,_tmp1CE + 2},(
void*)& Cyc__genrep_312};static struct _tuple5*Cyc__genarr_1089[1]={& Cyc__gentuple_1088};
struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0List_list_t0Absyn_tvar_t46H22_rep={
3,(struct _tagged_arr*)& Cyc__genname_1090,sizeof(struct Cyc_Core_Opt),{(void*)((
struct _tuple5**)Cyc__genarr_1089),(void*)((struct _tuple5**)Cyc__genarr_1089),(
void*)((struct _tuple5**)Cyc__genarr_1089 + 1)}};static struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_1087={1,1,(void*)((void*)& Cyc_struct_Core_Opt0List_list_t0Absyn_tvar_t46H22_rep)};
struct _tuple17{unsigned int f1;struct Cyc_Core_Opt*f2;struct Cyc_Core_Opt*f3;int f4;
struct Cyc_Core_Opt*f5;};static struct _tuple6 Cyc__gentuple_1095={offsetof(struct
_tuple17,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_1096={
offsetof(struct _tuple17,f2),(void*)& Cyc__genrep_1091};static struct _tuple6 Cyc__gentuple_1097={
offsetof(struct _tuple17,f3),(void*)& Cyc__genrep_92};static struct _tuple6 Cyc__gentuple_1098={
offsetof(struct _tuple17,f4),(void*)((void*)& Cyc__genrep_67)};static struct _tuple6
Cyc__gentuple_1099={offsetof(struct _tuple17,f5),(void*)& Cyc__genrep_1087};static
struct _tuple6*Cyc__genarr_1100[5]={& Cyc__gentuple_1095,& Cyc__gentuple_1096,& Cyc__gentuple_1097,&
Cyc__gentuple_1098,& Cyc__gentuple_1099};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_1086={
4,sizeof(struct _tuple17),{(void*)((struct _tuple6**)Cyc__genarr_1100),(void*)((
struct _tuple6**)Cyc__genarr_1100),(void*)((struct _tuple6**)Cyc__genarr_1100 + 5)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_1082;struct _tuple18{unsigned int
f1;struct Cyc_Absyn_Tvar*f2;};static struct _tuple6 Cyc__gentuple_1083={offsetof(
struct _tuple18,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_1084={
offsetof(struct _tuple18,f2),(void*)& Cyc__genrep_186};static struct _tuple6*Cyc__genarr_1085[
2]={& Cyc__gentuple_1083,& Cyc__gentuple_1084};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_1082={4,sizeof(struct _tuple18),{(void*)((struct _tuple6**)Cyc__genarr_1085),(
void*)((struct _tuple6**)Cyc__genarr_1085),(void*)((struct _tuple6**)Cyc__genarr_1085
+ 2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_1056;extern struct Cyc_Typerep_Struct_struct
Cyc_Absyn_tunion_info_t_rep;extern struct Cyc_Typerep_TUnion_struct Cyc_tunion_Absyn_TunionInfoU_rep;
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_1063;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_UnknownTunionInfo_rep;static char _tmp1D3[18]="UnknownTunionInfo";
static struct _tagged_arr Cyc__genname_1067=(struct _tagged_arr){_tmp1D3,_tmp1D3,
_tmp1D3 + 18};static char _tmp1D4[5]="name";static struct _tuple5 Cyc__gentuple_1064={
offsetof(struct Cyc_Absyn_UnknownTunionInfo,name),(struct _tagged_arr){_tmp1D4,
_tmp1D4,_tmp1D4 + 5},(void*)& Cyc__genrep_10};static char _tmp1D5[11]="is_xtunion";
static struct _tuple5 Cyc__gentuple_1065={offsetof(struct Cyc_Absyn_UnknownTunionInfo,is_xtunion),(
struct _tagged_arr){_tmp1D5,_tmp1D5,_tmp1D5 + 11},(void*)((void*)& Cyc__genrep_67)};
static struct _tuple5*Cyc__genarr_1066[2]={& Cyc__gentuple_1064,& Cyc__gentuple_1065};
struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_UnknownTunionInfo_rep={3,(struct
_tagged_arr*)& Cyc__genname_1067,sizeof(struct Cyc_Absyn_UnknownTunionInfo),{(void*)((
struct _tuple5**)Cyc__genarr_1066),(void*)((struct _tuple5**)Cyc__genarr_1066),(
void*)((struct _tuple5**)Cyc__genarr_1066 + 2)}};struct _tuple19{unsigned int f1;
struct Cyc_Absyn_UnknownTunionInfo f2;};static struct _tuple6 Cyc__gentuple_1068={
offsetof(struct _tuple19,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_1069={
offsetof(struct _tuple19,f2),(void*)& Cyc_struct_Absyn_UnknownTunionInfo_rep};
static struct _tuple6*Cyc__genarr_1070[2]={& Cyc__gentuple_1068,& Cyc__gentuple_1069};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_1063={4,sizeof(struct _tuple19),{(
void*)((struct _tuple6**)Cyc__genarr_1070),(void*)((struct _tuple6**)Cyc__genarr_1070),(
void*)((struct _tuple6**)Cyc__genarr_1070 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_1058;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_1059;extern
struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_302;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_Tuniondecl_rep;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_303;
extern struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0List_list_t0Absyn_tunionfield_t46H22_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_304;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List0Absyn_tunionfield_t46H2_rep;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_285;extern struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Tunionfield_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_286;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List060Absyn_tqual_t4Absyn_type_t1_446H2_rep;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_287;extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_288;static struct
_tuple6 Cyc__gentuple_289={offsetof(struct _tuple3,f1),(void*)& Cyc__genrep_135};
static struct _tuple6 Cyc__gentuple_290={offsetof(struct _tuple3,f2),(void*)((void*)&
Cyc_Absyn_type_t_rep)};static struct _tuple6*Cyc__genarr_291[2]={& Cyc__gentuple_289,&
Cyc__gentuple_290};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_288={4,
sizeof(struct _tuple3),{(void*)((struct _tuple6**)Cyc__genarr_291),(void*)((struct
_tuple6**)Cyc__genarr_291),(void*)((struct _tuple6**)Cyc__genarr_291 + 2)}};static
struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_287={1,1,(void*)((void*)& Cyc__genrep_288)};
static char _tmp1DA[5]="List";static struct _tagged_arr Cyc__genname_295=(struct
_tagged_arr){_tmp1DA,_tmp1DA,_tmp1DA + 5};static char _tmp1DB[3]="hd";static struct
_tuple5 Cyc__gentuple_292={offsetof(struct Cyc_List_List,hd),(struct _tagged_arr){
_tmp1DB,_tmp1DB,_tmp1DB + 3},(void*)& Cyc__genrep_287};static char _tmp1DC[3]="tl";
static struct _tuple5 Cyc__gentuple_293={offsetof(struct Cyc_List_List,tl),(struct
_tagged_arr){_tmp1DC,_tmp1DC,_tmp1DC + 3},(void*)& Cyc__genrep_286};static struct
_tuple5*Cyc__genarr_294[2]={& Cyc__gentuple_292,& Cyc__gentuple_293};struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List060Absyn_tqual_t4Absyn_type_t1_446H2_rep={3,(struct
_tagged_arr*)& Cyc__genname_295,sizeof(struct Cyc_List_List),{(void*)((struct
_tuple5**)Cyc__genarr_294),(void*)((struct _tuple5**)Cyc__genarr_294),(void*)((
struct _tuple5**)Cyc__genarr_294 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_286={
1,1,(void*)((void*)& Cyc_struct_List_List060Absyn_tqual_t4Absyn_type_t1_446H2_rep)};
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_2;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Position_Segment_rep;static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_2={
1,1,(void*)((void*)& Cyc_struct_Position_Segment_rep)};static char _tmp1E0[12]="Tunionfield";
static struct _tagged_arr Cyc__genname_301=(struct _tagged_arr){_tmp1E0,_tmp1E0,
_tmp1E0 + 12};static char _tmp1E1[5]="name";static struct _tuple5 Cyc__gentuple_296={
offsetof(struct Cyc_Absyn_Tunionfield,name),(struct _tagged_arr){_tmp1E1,_tmp1E1,
_tmp1E1 + 5},(void*)& Cyc__genrep_10};static char _tmp1E2[5]="typs";static struct
_tuple5 Cyc__gentuple_297={offsetof(struct Cyc_Absyn_Tunionfield,typs),(struct
_tagged_arr){_tmp1E2,_tmp1E2,_tmp1E2 + 5},(void*)& Cyc__genrep_286};static char
_tmp1E3[4]="loc";static struct _tuple5 Cyc__gentuple_298={offsetof(struct Cyc_Absyn_Tunionfield,loc),(
struct _tagged_arr){_tmp1E3,_tmp1E3,_tmp1E3 + 4},(void*)& Cyc__genrep_2};static char
_tmp1E4[3]="sc";static struct _tuple5 Cyc__gentuple_299={offsetof(struct Cyc_Absyn_Tunionfield,sc),(
struct _tagged_arr){_tmp1E4,_tmp1E4,_tmp1E4 + 3},(void*)& Cyc_Absyn_scope_t_rep};
static struct _tuple5*Cyc__genarr_300[4]={& Cyc__gentuple_296,& Cyc__gentuple_297,&
Cyc__gentuple_298,& Cyc__gentuple_299};struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Tunionfield_rep={
3,(struct _tagged_arr*)& Cyc__genname_301,sizeof(struct Cyc_Absyn_Tunionfield),{(
void*)((struct _tuple5**)Cyc__genarr_300),(void*)((struct _tuple5**)Cyc__genarr_300),(
void*)((struct _tuple5**)Cyc__genarr_300 + 4)}};static struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_285={1,1,(void*)((void*)& Cyc_struct_Absyn_Tunionfield_rep)};static
char _tmp1E7[5]="List";static struct _tagged_arr Cyc__genname_308=(struct _tagged_arr){
_tmp1E7,_tmp1E7,_tmp1E7 + 5};static char _tmp1E8[3]="hd";static struct _tuple5 Cyc__gentuple_305={
offsetof(struct Cyc_List_List,hd),(struct _tagged_arr){_tmp1E8,_tmp1E8,_tmp1E8 + 3},(
void*)& Cyc__genrep_285};static char _tmp1E9[3]="tl";static struct _tuple5 Cyc__gentuple_306={
offsetof(struct Cyc_List_List,tl),(struct _tagged_arr){_tmp1E9,_tmp1E9,_tmp1E9 + 3},(
void*)& Cyc__genrep_304};static struct _tuple5*Cyc__genarr_307[2]={& Cyc__gentuple_305,&
Cyc__gentuple_306};struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_tunionfield_t46H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_308,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_307),(void*)((struct _tuple5**)Cyc__genarr_307),(void*)((
struct _tuple5**)Cyc__genarr_307 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_304={
1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_tunionfield_t46H2_rep)};static
char _tmp1EC[4]="Opt";static struct _tagged_arr Cyc__genname_311=(struct _tagged_arr){
_tmp1EC,_tmp1EC,_tmp1EC + 4};static char _tmp1ED[2]="v";static struct _tuple5 Cyc__gentuple_309={
offsetof(struct Cyc_Core_Opt,v),(struct _tagged_arr){_tmp1ED,_tmp1ED,_tmp1ED + 2},(
void*)& Cyc__genrep_304};static struct _tuple5*Cyc__genarr_310[1]={& Cyc__gentuple_309};
struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0List_list_t0Absyn_tunionfield_t46H22_rep={
3,(struct _tagged_arr*)& Cyc__genname_311,sizeof(struct Cyc_Core_Opt),{(void*)((
struct _tuple5**)Cyc__genarr_310),(void*)((struct _tuple5**)Cyc__genarr_310),(void*)((
struct _tuple5**)Cyc__genarr_310 + 1)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_303={
1,1,(void*)((void*)& Cyc_struct_Core_Opt0List_list_t0Absyn_tunionfield_t46H22_rep)};
static char _tmp1F0[11]="Tuniondecl";static struct _tagged_arr Cyc__genname_323=(
struct _tagged_arr){_tmp1F0,_tmp1F0,_tmp1F0 + 11};static char _tmp1F1[3]="sc";static
struct _tuple5 Cyc__gentuple_317={offsetof(struct Cyc_Absyn_Tuniondecl,sc),(struct
_tagged_arr){_tmp1F1,_tmp1F1,_tmp1F1 + 3},(void*)& Cyc_Absyn_scope_t_rep};static
char _tmp1F2[5]="name";static struct _tuple5 Cyc__gentuple_318={offsetof(struct Cyc_Absyn_Tuniondecl,name),(
struct _tagged_arr){_tmp1F2,_tmp1F2,_tmp1F2 + 5},(void*)& Cyc__genrep_10};static
char _tmp1F3[4]="tvs";static struct _tuple5 Cyc__gentuple_319={offsetof(struct Cyc_Absyn_Tuniondecl,tvs),(
struct _tagged_arr){_tmp1F3,_tmp1F3,_tmp1F3 + 4},(void*)& Cyc__genrep_312};static
char _tmp1F4[7]="fields";static struct _tuple5 Cyc__gentuple_320={offsetof(struct Cyc_Absyn_Tuniondecl,fields),(
struct _tagged_arr){_tmp1F4,_tmp1F4,_tmp1F4 + 7},(void*)& Cyc__genrep_303};static
char _tmp1F5[11]="is_xtunion";static struct _tuple5 Cyc__gentuple_321={offsetof(
struct Cyc_Absyn_Tuniondecl,is_xtunion),(struct _tagged_arr){_tmp1F5,_tmp1F5,
_tmp1F5 + 11},(void*)((void*)& Cyc__genrep_67)};static struct _tuple5*Cyc__genarr_322[
5]={& Cyc__gentuple_317,& Cyc__gentuple_318,& Cyc__gentuple_319,& Cyc__gentuple_320,&
Cyc__gentuple_321};struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Tuniondecl_rep={
3,(struct _tagged_arr*)& Cyc__genname_323,sizeof(struct Cyc_Absyn_Tuniondecl),{(
void*)((struct _tuple5**)Cyc__genarr_322),(void*)((struct _tuple5**)Cyc__genarr_322),(
void*)((struct _tuple5**)Cyc__genarr_322 + 5)}};static struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_302={1,1,(void*)((void*)& Cyc_struct_Absyn_Tuniondecl_rep)};static
struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_1059={1,1,(void*)((void*)& Cyc__genrep_302)};
struct _tuple20{unsigned int f1;struct Cyc_Absyn_Tuniondecl**f2;};static struct
_tuple6 Cyc__gentuple_1060={offsetof(struct _tuple20,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_1061={offsetof(struct _tuple20,f2),(void*)& Cyc__genrep_1059};
static struct _tuple6*Cyc__genarr_1062[2]={& Cyc__gentuple_1060,& Cyc__gentuple_1061};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_1058={4,sizeof(struct _tuple20),{(
void*)((struct _tuple6**)Cyc__genarr_1062),(void*)((struct _tuple6**)Cyc__genarr_1062),(
void*)((struct _tuple6**)Cyc__genarr_1062 + 2)}};static struct _tuple7*Cyc__genarr_1057[
0]={};static char _tmp1FA[14]="UnknownTunion";static struct _tuple5 Cyc__gentuple_1071={
0,(struct _tagged_arr){_tmp1FA,_tmp1FA,_tmp1FA + 14},(void*)& Cyc__genrep_1063};
static char _tmp1FB[12]="KnownTunion";static struct _tuple5 Cyc__gentuple_1072={1,(
struct _tagged_arr){_tmp1FB,_tmp1FB,_tmp1FB + 12},(void*)& Cyc__genrep_1058};static
struct _tuple5*Cyc__genarr_1073[2]={& Cyc__gentuple_1071,& Cyc__gentuple_1072};
static char _tmp1FD[12]="TunionInfoU";struct Cyc_Typerep_TUnion_struct Cyc_tunion_Absyn_TunionInfoU_rep={
5,(struct _tagged_arr){_tmp1FD,_tmp1FD,_tmp1FD + 12},{(void*)((struct _tuple7**)Cyc__genarr_1057),(
void*)((struct _tuple7**)Cyc__genarr_1057),(void*)((struct _tuple7**)Cyc__genarr_1057
+ 0)},{(void*)((struct _tuple5**)Cyc__genarr_1073),(void*)((struct _tuple5**)Cyc__genarr_1073),(
void*)((struct _tuple5**)Cyc__genarr_1073 + 2)}};extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_102;extern struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_type_t46H2_rep;
static char _tmp1FE[5]="List";static struct _tagged_arr Cyc__genname_106=(struct
_tagged_arr){_tmp1FE,_tmp1FE,_tmp1FE + 5};static char _tmp1FF[3]="hd";static struct
_tuple5 Cyc__gentuple_103={offsetof(struct Cyc_List_List,hd),(struct _tagged_arr){
_tmp1FF,_tmp1FF,_tmp1FF + 3},(void*)((void*)& Cyc_Absyn_type_t_rep)};static char
_tmp200[3]="tl";static struct _tuple5 Cyc__gentuple_104={offsetof(struct Cyc_List_List,tl),(
struct _tagged_arr){_tmp200,_tmp200,_tmp200 + 3},(void*)& Cyc__genrep_102};static
struct _tuple5*Cyc__genarr_105[2]={& Cyc__gentuple_103,& Cyc__gentuple_104};struct
Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_type_t46H2_rep={3,(struct
_tagged_arr*)& Cyc__genname_106,sizeof(struct Cyc_List_List),{(void*)((struct
_tuple5**)Cyc__genarr_105),(void*)((struct _tuple5**)Cyc__genarr_105),(void*)((
struct _tuple5**)Cyc__genarr_105 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_102={
1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_type_t46H2_rep)};static char
_tmp203[11]="TunionInfo";static struct _tagged_arr Cyc__genname_1078=(struct
_tagged_arr){_tmp203,_tmp203,_tmp203 + 11};static char _tmp204[12]="tunion_info";
static struct _tuple5 Cyc__gentuple_1074={offsetof(struct Cyc_Absyn_TunionInfo,tunion_info),(
struct _tagged_arr){_tmp204,_tmp204,_tmp204 + 12},(void*)& Cyc_tunion_Absyn_TunionInfoU_rep};
static char _tmp205[6]="targs";static struct _tuple5 Cyc__gentuple_1075={offsetof(
struct Cyc_Absyn_TunionInfo,targs),(struct _tagged_arr){_tmp205,_tmp205,_tmp205 + 6},(
void*)& Cyc__genrep_102};static char _tmp206[4]="rgn";static struct _tuple5 Cyc__gentuple_1076={
offsetof(struct Cyc_Absyn_TunionInfo,rgn),(struct _tagged_arr){_tmp206,_tmp206,
_tmp206 + 4},(void*)((void*)& Cyc_Absyn_type_t_rep)};static struct _tuple5*Cyc__genarr_1077[
3]={& Cyc__gentuple_1074,& Cyc__gentuple_1075,& Cyc__gentuple_1076};struct Cyc_Typerep_Struct_struct
Cyc_Absyn_tunion_info_t_rep={3,(struct _tagged_arr*)& Cyc__genname_1078,sizeof(
struct Cyc_Absyn_TunionInfo),{(void*)((struct _tuple5**)Cyc__genarr_1077),(void*)((
struct _tuple5**)Cyc__genarr_1077),(void*)((struct _tuple5**)Cyc__genarr_1077 + 3)}};
struct _tuple21{unsigned int f1;struct Cyc_Absyn_TunionInfo f2;};static struct _tuple6
Cyc__gentuple_1079={offsetof(struct _tuple21,f1),(void*)& Cyc__genrep_5};static
struct _tuple6 Cyc__gentuple_1080={offsetof(struct _tuple21,f2),(void*)& Cyc_Absyn_tunion_info_t_rep};
static struct _tuple6*Cyc__genarr_1081[2]={& Cyc__gentuple_1079,& Cyc__gentuple_1080};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_1056={4,sizeof(struct _tuple21),{(
void*)((struct _tuple6**)Cyc__genarr_1081),(void*)((struct _tuple6**)Cyc__genarr_1081),(
void*)((struct _tuple6**)Cyc__genarr_1081 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_1030;extern struct Cyc_Typerep_Struct_struct Cyc_Absyn_tunion_field_info_t_rep;
extern struct Cyc_Typerep_TUnion_struct Cyc_tunion_Absyn_TunionFieldInfoU_rep;
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_1037;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_UnknownTunionFieldInfo_rep;static char _tmp209[23]="UnknownTunionFieldInfo";
static struct _tagged_arr Cyc__genname_1042=(struct _tagged_arr){_tmp209,_tmp209,
_tmp209 + 23};static char _tmp20A[12]="tunion_name";static struct _tuple5 Cyc__gentuple_1038={
offsetof(struct Cyc_Absyn_UnknownTunionFieldInfo,tunion_name),(struct _tagged_arr){
_tmp20A,_tmp20A,_tmp20A + 12},(void*)& Cyc__genrep_10};static char _tmp20B[11]="field_name";
static struct _tuple5 Cyc__gentuple_1039={offsetof(struct Cyc_Absyn_UnknownTunionFieldInfo,field_name),(
struct _tagged_arr){_tmp20B,_tmp20B,_tmp20B + 11},(void*)& Cyc__genrep_10};static
char _tmp20C[11]="is_xtunion";static struct _tuple5 Cyc__gentuple_1040={offsetof(
struct Cyc_Absyn_UnknownTunionFieldInfo,is_xtunion),(struct _tagged_arr){_tmp20C,
_tmp20C,_tmp20C + 11},(void*)((void*)& Cyc__genrep_67)};static struct _tuple5*Cyc__genarr_1041[
3]={& Cyc__gentuple_1038,& Cyc__gentuple_1039,& Cyc__gentuple_1040};struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_UnknownTunionFieldInfo_rep={3,(struct _tagged_arr*)& Cyc__genname_1042,
sizeof(struct Cyc_Absyn_UnknownTunionFieldInfo),{(void*)((struct _tuple5**)Cyc__genarr_1041),(
void*)((struct _tuple5**)Cyc__genarr_1041),(void*)((struct _tuple5**)Cyc__genarr_1041
+ 3)}};struct _tuple22{unsigned int f1;struct Cyc_Absyn_UnknownTunionFieldInfo f2;};
static struct _tuple6 Cyc__gentuple_1043={offsetof(struct _tuple22,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_1044={offsetof(struct _tuple22,f2),(void*)& Cyc_struct_Absyn_UnknownTunionFieldInfo_rep};
static struct _tuple6*Cyc__genarr_1045[2]={& Cyc__gentuple_1043,& Cyc__gentuple_1044};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_1037={4,sizeof(struct _tuple22),{(
void*)((struct _tuple6**)Cyc__genarr_1045),(void*)((struct _tuple6**)Cyc__genarr_1045),(
void*)((struct _tuple6**)Cyc__genarr_1045 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_1032;struct _tuple23{unsigned int f1;struct Cyc_Absyn_Tuniondecl*f2;
struct Cyc_Absyn_Tunionfield*f3;};static struct _tuple6 Cyc__gentuple_1033={
offsetof(struct _tuple23,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_1034={
offsetof(struct _tuple23,f2),(void*)((void*)& Cyc__genrep_302)};static struct
_tuple6 Cyc__gentuple_1035={offsetof(struct _tuple23,f3),(void*)& Cyc__genrep_285};
static struct _tuple6*Cyc__genarr_1036[3]={& Cyc__gentuple_1033,& Cyc__gentuple_1034,&
Cyc__gentuple_1035};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_1032={4,
sizeof(struct _tuple23),{(void*)((struct _tuple6**)Cyc__genarr_1036),(void*)((
struct _tuple6**)Cyc__genarr_1036),(void*)((struct _tuple6**)Cyc__genarr_1036 + 3)}};
static struct _tuple7*Cyc__genarr_1031[0]={};static char _tmp210[19]="UnknownTunionfield";
static struct _tuple5 Cyc__gentuple_1046={0,(struct _tagged_arr){_tmp210,_tmp210,
_tmp210 + 19},(void*)& Cyc__genrep_1037};static char _tmp211[17]="KnownTunionfield";
static struct _tuple5 Cyc__gentuple_1047={1,(struct _tagged_arr){_tmp211,_tmp211,
_tmp211 + 17},(void*)& Cyc__genrep_1032};static struct _tuple5*Cyc__genarr_1048[2]={&
Cyc__gentuple_1046,& Cyc__gentuple_1047};static char _tmp213[17]="TunionFieldInfoU";
struct Cyc_Typerep_TUnion_struct Cyc_tunion_Absyn_TunionFieldInfoU_rep={5,(struct
_tagged_arr){_tmp213,_tmp213,_tmp213 + 17},{(void*)((struct _tuple7**)Cyc__genarr_1031),(
void*)((struct _tuple7**)Cyc__genarr_1031),(void*)((struct _tuple7**)Cyc__genarr_1031
+ 0)},{(void*)((struct _tuple5**)Cyc__genarr_1048),(void*)((struct _tuple5**)Cyc__genarr_1048),(
void*)((struct _tuple5**)Cyc__genarr_1048 + 2)}};static char _tmp214[16]="TunionFieldInfo";
static struct _tagged_arr Cyc__genname_1052=(struct _tagged_arr){_tmp214,_tmp214,
_tmp214 + 16};static char _tmp215[11]="field_info";static struct _tuple5 Cyc__gentuple_1049={
offsetof(struct Cyc_Absyn_TunionFieldInfo,field_info),(struct _tagged_arr){_tmp215,
_tmp215,_tmp215 + 11},(void*)& Cyc_tunion_Absyn_TunionFieldInfoU_rep};static char
_tmp216[6]="targs";static struct _tuple5 Cyc__gentuple_1050={offsetof(struct Cyc_Absyn_TunionFieldInfo,targs),(
struct _tagged_arr){_tmp216,_tmp216,_tmp216 + 6},(void*)& Cyc__genrep_102};static
struct _tuple5*Cyc__genarr_1051[2]={& Cyc__gentuple_1049,& Cyc__gentuple_1050};
struct Cyc_Typerep_Struct_struct Cyc_Absyn_tunion_field_info_t_rep={3,(struct
_tagged_arr*)& Cyc__genname_1052,sizeof(struct Cyc_Absyn_TunionFieldInfo),{(void*)((
struct _tuple5**)Cyc__genarr_1051),(void*)((struct _tuple5**)Cyc__genarr_1051),(
void*)((struct _tuple5**)Cyc__genarr_1051 + 2)}};struct _tuple24{unsigned int f1;
struct Cyc_Absyn_TunionFieldInfo f2;};static struct _tuple6 Cyc__gentuple_1053={
offsetof(struct _tuple24,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_1054={
offsetof(struct _tuple24,f2),(void*)& Cyc_Absyn_tunion_field_info_t_rep};static
struct _tuple6*Cyc__genarr_1055[2]={& Cyc__gentuple_1053,& Cyc__gentuple_1054};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_1030={4,sizeof(struct _tuple24),{(
void*)((struct _tuple6**)Cyc__genarr_1055),(void*)((struct _tuple6**)Cyc__genarr_1055),(
void*)((struct _tuple6**)Cyc__genarr_1055 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_1011;extern struct Cyc_Typerep_Struct_struct Cyc_Absyn_ptr_info_t_rep;
extern struct Cyc_Typerep_Struct_struct Cyc_Absyn_ptr_atts_t_rep;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_978;extern struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Conref0bool2_rep;
extern struct Cyc_Typerep_TUnion_struct Cyc_tunion_Absyn_Constraint0bool2_rep;
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_66;struct _tuple25{unsigned int f1;
int f2;};static struct _tuple6 Cyc__gentuple_68={offsetof(struct _tuple25,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_69={offsetof(struct _tuple25,f2),(
void*)((void*)& Cyc__genrep_67)};static struct _tuple6*Cyc__genarr_70[2]={& Cyc__gentuple_68,&
Cyc__gentuple_69};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_66={4,sizeof(
struct _tuple25),{(void*)((struct _tuple6**)Cyc__genarr_70),(void*)((struct _tuple6**)
Cyc__genarr_70),(void*)((struct _tuple6**)Cyc__genarr_70 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_981;struct _tuple26{unsigned int f1;struct Cyc_Absyn_Conref*f2;};static
struct _tuple6 Cyc__gentuple_982={offsetof(struct _tuple26,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_983={offsetof(struct _tuple26,f2),(void*)& Cyc__genrep_978};
static struct _tuple6*Cyc__genarr_984[2]={& Cyc__gentuple_982,& Cyc__gentuple_983};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_981={4,sizeof(struct _tuple26),{(
void*)((struct _tuple6**)Cyc__genarr_984),(void*)((struct _tuple6**)Cyc__genarr_984),(
void*)((struct _tuple6**)Cyc__genarr_984 + 2)}};static char _tmp21B[10]="No_constr";
static struct _tuple7 Cyc__gentuple_979={0,(struct _tagged_arr){_tmp21B,_tmp21B,
_tmp21B + 10}};static struct _tuple7*Cyc__genarr_980[1]={& Cyc__gentuple_979};static
char _tmp21C[10]="Eq_constr";static struct _tuple5 Cyc__gentuple_985={0,(struct
_tagged_arr){_tmp21C,_tmp21C,_tmp21C + 10},(void*)& Cyc__genrep_66};static char
_tmp21D[15]="Forward_constr";static struct _tuple5 Cyc__gentuple_986={1,(struct
_tagged_arr){_tmp21D,_tmp21D,_tmp21D + 15},(void*)& Cyc__genrep_981};static struct
_tuple5*Cyc__genarr_987[2]={& Cyc__gentuple_985,& Cyc__gentuple_986};static char
_tmp21F[11]="Constraint";struct Cyc_Typerep_TUnion_struct Cyc_tunion_Absyn_Constraint0bool2_rep={
5,(struct _tagged_arr){_tmp21F,_tmp21F,_tmp21F + 11},{(void*)((struct _tuple7**)Cyc__genarr_980),(
void*)((struct _tuple7**)Cyc__genarr_980),(void*)((struct _tuple7**)Cyc__genarr_980
+ 1)},{(void*)((struct _tuple5**)Cyc__genarr_987),(void*)((struct _tuple5**)Cyc__genarr_987),(
void*)((struct _tuple5**)Cyc__genarr_987 + 2)}};static char _tmp220[7]="Conref";
static struct _tagged_arr Cyc__genname_990=(struct _tagged_arr){_tmp220,_tmp220,
_tmp220 + 7};static char _tmp221[2]="v";static struct _tuple5 Cyc__gentuple_988={
offsetof(struct Cyc_Absyn_Conref,v),(struct _tagged_arr){_tmp221,_tmp221,_tmp221 + 
2},(void*)& Cyc_tunion_Absyn_Constraint0bool2_rep};static struct _tuple5*Cyc__genarr_989[
1]={& Cyc__gentuple_988};struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Conref0bool2_rep={
3,(struct _tagged_arr*)& Cyc__genname_990,sizeof(struct Cyc_Absyn_Conref),{(void*)((
struct _tuple5**)Cyc__genarr_989),(void*)((struct _tuple5**)Cyc__genarr_989),(void*)((
struct _tuple5**)Cyc__genarr_989 + 1)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_978={
1,1,(void*)((void*)& Cyc_struct_Absyn_Conref0bool2_rep)};extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_1012;extern struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Conref0Absyn_bounds_t2_rep;
static char _tmp224[7]="Conref";static struct _tagged_arr Cyc__genname_1015=(struct
_tagged_arr){_tmp224,_tmp224,_tmp224 + 7};static char _tmp225[2]="v";static struct
_tuple5 Cyc__gentuple_1013={offsetof(struct Cyc_Absyn_Conref,v),(struct _tagged_arr){
_tmp225,_tmp225,_tmp225 + 2},(void*)& Cyc_tunion_Absyn_Constraint0bool2_rep};
static struct _tuple5*Cyc__genarr_1014[1]={& Cyc__gentuple_1013};struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_Conref0Absyn_bounds_t2_rep={3,(struct _tagged_arr*)& Cyc__genname_1015,
sizeof(struct Cyc_Absyn_Conref),{(void*)((struct _tuple5**)Cyc__genarr_1014),(void*)((
struct _tuple5**)Cyc__genarr_1014),(void*)((struct _tuple5**)Cyc__genarr_1014 + 1)}};
static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_1012={1,1,(void*)((void*)& Cyc_struct_Absyn_Conref0Absyn_bounds_t2_rep)};
static char _tmp228[8]="PtrAtts";static struct _tagged_arr Cyc__genname_1021=(struct
_tagged_arr){_tmp228,_tmp228,_tmp228 + 8};static char _tmp229[4]="rgn";static struct
_tuple5 Cyc__gentuple_1016={offsetof(struct Cyc_Absyn_PtrAtts,rgn),(struct
_tagged_arr){_tmp229,_tmp229,_tmp229 + 4},(void*)((void*)& Cyc_Absyn_type_t_rep)};
static char _tmp22A[9]="nullable";static struct _tuple5 Cyc__gentuple_1017={offsetof(
struct Cyc_Absyn_PtrAtts,nullable),(struct _tagged_arr){_tmp22A,_tmp22A,_tmp22A + 9},(
void*)& Cyc__genrep_978};static char _tmp22B[7]="bounds";static struct _tuple5 Cyc__gentuple_1018={
offsetof(struct Cyc_Absyn_PtrAtts,bounds),(struct _tagged_arr){_tmp22B,_tmp22B,
_tmp22B + 7},(void*)& Cyc__genrep_1012};static char _tmp22C[10]="zero_term";static
struct _tuple5 Cyc__gentuple_1019={offsetof(struct Cyc_Absyn_PtrAtts,zero_term),(
struct _tagged_arr){_tmp22C,_tmp22C,_tmp22C + 10},(void*)& Cyc__genrep_978};static
struct _tuple5*Cyc__genarr_1020[4]={& Cyc__gentuple_1016,& Cyc__gentuple_1017,& Cyc__gentuple_1018,&
Cyc__gentuple_1019};struct Cyc_Typerep_Struct_struct Cyc_Absyn_ptr_atts_t_rep={3,(
struct _tagged_arr*)& Cyc__genname_1021,sizeof(struct Cyc_Absyn_PtrAtts),{(void*)((
struct _tuple5**)Cyc__genarr_1020),(void*)((struct _tuple5**)Cyc__genarr_1020),(
void*)((struct _tuple5**)Cyc__genarr_1020 + 4)}};static char _tmp22E[8]="PtrInfo";
static struct _tagged_arr Cyc__genname_1026=(struct _tagged_arr){_tmp22E,_tmp22E,
_tmp22E + 8};static char _tmp22F[8]="elt_typ";static struct _tuple5 Cyc__gentuple_1022={
offsetof(struct Cyc_Absyn_PtrInfo,elt_typ),(struct _tagged_arr){_tmp22F,_tmp22F,
_tmp22F + 8},(void*)((void*)& Cyc_Absyn_type_t_rep)};static char _tmp230[7]="elt_tq";
static struct _tuple5 Cyc__gentuple_1023={offsetof(struct Cyc_Absyn_PtrInfo,elt_tq),(
struct _tagged_arr){_tmp230,_tmp230,_tmp230 + 7},(void*)& Cyc__genrep_135};static
char _tmp231[9]="ptr_atts";static struct _tuple5 Cyc__gentuple_1024={offsetof(struct
Cyc_Absyn_PtrInfo,ptr_atts),(struct _tagged_arr){_tmp231,_tmp231,_tmp231 + 9},(
void*)& Cyc_Absyn_ptr_atts_t_rep};static struct _tuple5*Cyc__genarr_1025[3]={& Cyc__gentuple_1022,&
Cyc__gentuple_1023,& Cyc__gentuple_1024};struct Cyc_Typerep_Struct_struct Cyc_Absyn_ptr_info_t_rep={
3,(struct _tagged_arr*)& Cyc__genname_1026,sizeof(struct Cyc_Absyn_PtrInfo),{(void*)((
struct _tuple5**)Cyc__genarr_1025),(void*)((struct _tuple5**)Cyc__genarr_1025),(
void*)((struct _tuple5**)Cyc__genarr_1025 + 3)}};struct _tuple27{unsigned int f1;
struct Cyc_Absyn_PtrInfo f2;};static struct _tuple6 Cyc__gentuple_1027={offsetof(
struct _tuple27,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_1028={
offsetof(struct _tuple27,f2),(void*)& Cyc_Absyn_ptr_info_t_rep};static struct
_tuple6*Cyc__genarr_1029[2]={& Cyc__gentuple_1027,& Cyc__gentuple_1028};static
struct Cyc_Typerep_Tuple_struct Cyc__genrep_1011={4,sizeof(struct _tuple27),{(void*)((
struct _tuple6**)Cyc__genarr_1029),(void*)((struct _tuple6**)Cyc__genarr_1029),(
void*)((struct _tuple6**)Cyc__genarr_1029 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_1000;extern struct Cyc_Typerep_TUnion_struct Cyc_Absyn_sign_t_rep;
static char _tmp234[7]="Signed";static struct _tuple7 Cyc__gentuple_275={0,(struct
_tagged_arr){_tmp234,_tmp234,_tmp234 + 7}};static char _tmp235[9]="Unsigned";static
struct _tuple7 Cyc__gentuple_276={1,(struct _tagged_arr){_tmp235,_tmp235,_tmp235 + 9}};
static char _tmp236[5]="None";static struct _tuple7 Cyc__gentuple_277={2,(struct
_tagged_arr){_tmp236,_tmp236,_tmp236 + 5}};static struct _tuple7*Cyc__genarr_278[3]={&
Cyc__gentuple_275,& Cyc__gentuple_276,& Cyc__gentuple_277};static struct _tuple5*Cyc__genarr_279[
0]={};static char _tmp238[5]="Sign";struct Cyc_Typerep_TUnion_struct Cyc_Absyn_sign_t_rep={
5,(struct _tagged_arr){_tmp238,_tmp238,_tmp238 + 5},{(void*)((struct _tuple7**)Cyc__genarr_278),(
void*)((struct _tuple7**)Cyc__genarr_278),(void*)((struct _tuple7**)Cyc__genarr_278
+ 3)},{(void*)((struct _tuple5**)Cyc__genarr_279),(void*)((struct _tuple5**)Cyc__genarr_279),(
void*)((struct _tuple5**)Cyc__genarr_279 + 0)}};extern struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_size_of_t_rep;static char _tmp239[3]="B1";static struct _tuple7 Cyc__gentuple_1001={
0,(struct _tagged_arr){_tmp239,_tmp239,_tmp239 + 3}};static char _tmp23A[3]="B2";
static struct _tuple7 Cyc__gentuple_1002={1,(struct _tagged_arr){_tmp23A,_tmp23A,
_tmp23A + 3}};static char _tmp23B[3]="B4";static struct _tuple7 Cyc__gentuple_1003={2,(
struct _tagged_arr){_tmp23B,_tmp23B,_tmp23B + 3}};static char _tmp23C[3]="B8";static
struct _tuple7 Cyc__gentuple_1004={3,(struct _tagged_arr){_tmp23C,_tmp23C,_tmp23C + 
3}};static struct _tuple7*Cyc__genarr_1005[4]={& Cyc__gentuple_1001,& Cyc__gentuple_1002,&
Cyc__gentuple_1003,& Cyc__gentuple_1004};static struct _tuple5*Cyc__genarr_1006[0]={};
static char _tmp23E[8]="Size_of";struct Cyc_Typerep_TUnion_struct Cyc_Absyn_size_of_t_rep={
5,(struct _tagged_arr){_tmp23E,_tmp23E,_tmp23E + 8},{(void*)((struct _tuple7**)Cyc__genarr_1005),(
void*)((struct _tuple7**)Cyc__genarr_1005),(void*)((struct _tuple7**)Cyc__genarr_1005
+ 4)},{(void*)((struct _tuple5**)Cyc__genarr_1006),(void*)((struct _tuple5**)Cyc__genarr_1006),(
void*)((struct _tuple5**)Cyc__genarr_1006 + 0)}};struct _tuple28{unsigned int f1;
void*f2;void*f3;};static struct _tuple6 Cyc__gentuple_1007={offsetof(struct _tuple28,f1),(
void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_1008={offsetof(struct
_tuple28,f2),(void*)& Cyc_Absyn_sign_t_rep};static struct _tuple6 Cyc__gentuple_1009={
offsetof(struct _tuple28,f3),(void*)& Cyc_Absyn_size_of_t_rep};static struct _tuple6*
Cyc__genarr_1010[3]={& Cyc__gentuple_1007,& Cyc__gentuple_1008,& Cyc__gentuple_1009};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_1000={4,sizeof(struct _tuple28),{(
void*)((struct _tuple6**)Cyc__genarr_1010),(void*)((struct _tuple6**)Cyc__genarr_1010),(
void*)((struct _tuple6**)Cyc__genarr_1010 + 3)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_977;extern struct Cyc_Typerep_Struct_struct Cyc_Absyn_array_info_t_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_122;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_Exp_rep;extern struct Cyc_Typerep_TUnion_struct Cyc_Absyn_raw_exp_t_rep;
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_853;extern struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_cnst_t_rep;extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_868;struct
_tuple29{unsigned int f1;void*f2;char f3;};static struct _tuple6 Cyc__gentuple_869={
offsetof(struct _tuple29,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_870={
offsetof(struct _tuple29,f2),(void*)& Cyc_Absyn_sign_t_rep};static struct _tuple6 Cyc__gentuple_871={
offsetof(struct _tuple29,f3),(void*)((void*)& Cyc__genrep_14)};static struct _tuple6*
Cyc__genarr_872[3]={& Cyc__gentuple_869,& Cyc__gentuple_870,& Cyc__gentuple_871};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_868={4,sizeof(struct _tuple29),{(
void*)((struct _tuple6**)Cyc__genarr_872),(void*)((struct _tuple6**)Cyc__genarr_872),(
void*)((struct _tuple6**)Cyc__genarr_872 + 3)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_862;static struct Cyc_Typerep_Int_struct Cyc__genrep_863={0,1,16};
struct _tuple30{unsigned int f1;void*f2;short f3;};static struct _tuple6 Cyc__gentuple_864={
offsetof(struct _tuple30,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_865={
offsetof(struct _tuple30,f2),(void*)& Cyc_Absyn_sign_t_rep};static struct _tuple6 Cyc__gentuple_866={
offsetof(struct _tuple30,f3),(void*)& Cyc__genrep_863};static struct _tuple6*Cyc__genarr_867[
3]={& Cyc__gentuple_864,& Cyc__gentuple_865,& Cyc__gentuple_866};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_862={4,sizeof(struct _tuple30),{(void*)((struct _tuple6**)Cyc__genarr_867),(
void*)((struct _tuple6**)Cyc__genarr_867),(void*)((struct _tuple6**)Cyc__genarr_867
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_274;struct _tuple31{
unsigned int f1;void*f2;int f3;};static struct _tuple6 Cyc__gentuple_280={offsetof(
struct _tuple31,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_281={
offsetof(struct _tuple31,f2),(void*)& Cyc_Absyn_sign_t_rep};static struct _tuple6 Cyc__gentuple_282={
offsetof(struct _tuple31,f3),(void*)((void*)& Cyc__genrep_67)};static struct _tuple6*
Cyc__genarr_283[3]={& Cyc__gentuple_280,& Cyc__gentuple_281,& Cyc__gentuple_282};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_274={4,sizeof(struct _tuple31),{(
void*)((struct _tuple6**)Cyc__genarr_283),(void*)((struct _tuple6**)Cyc__genarr_283),(
void*)((struct _tuple6**)Cyc__genarr_283 + 3)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_856;static struct Cyc_Typerep_Int_struct Cyc__genrep_857={0,1,64};
struct _tuple32{unsigned int f1;void*f2;long long f3;};static struct _tuple6 Cyc__gentuple_858={
offsetof(struct _tuple32,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_859={
offsetof(struct _tuple32,f2),(void*)& Cyc_Absyn_sign_t_rep};static struct _tuple6 Cyc__gentuple_860={
offsetof(struct _tuple32,f3),(void*)& Cyc__genrep_857};static struct _tuple6*Cyc__genarr_861[
3]={& Cyc__gentuple_858,& Cyc__gentuple_859,& Cyc__gentuple_860};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_856={4,sizeof(struct _tuple32),{(void*)((struct _tuple6**)Cyc__genarr_861),(
void*)((struct _tuple6**)Cyc__genarr_861),(void*)((struct _tuple6**)Cyc__genarr_861
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_62;static struct _tuple6 Cyc__gentuple_63={
offsetof(struct _tuple7,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_64={
offsetof(struct _tuple7,f2),(void*)((void*)& Cyc__genrep_13)};static struct _tuple6*
Cyc__genarr_65[2]={& Cyc__gentuple_63,& Cyc__gentuple_64};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_62={4,sizeof(struct _tuple7),{(void*)((struct _tuple6**)Cyc__genarr_65),(
void*)((struct _tuple6**)Cyc__genarr_65),(void*)((struct _tuple6**)Cyc__genarr_65 + 
2)}};static char _tmp247[7]="Null_c";static struct _tuple7 Cyc__gentuple_854={0,(
struct _tagged_arr){_tmp247,_tmp247,_tmp247 + 7}};static struct _tuple7*Cyc__genarr_855[
1]={& Cyc__gentuple_854};static char _tmp248[7]="Char_c";static struct _tuple5 Cyc__gentuple_873={
0,(struct _tagged_arr){_tmp248,_tmp248,_tmp248 + 7},(void*)& Cyc__genrep_868};
static char _tmp249[8]="Short_c";static struct _tuple5 Cyc__gentuple_874={1,(struct
_tagged_arr){_tmp249,_tmp249,_tmp249 + 8},(void*)& Cyc__genrep_862};static char
_tmp24A[6]="Int_c";static struct _tuple5 Cyc__gentuple_875={2,(struct _tagged_arr){
_tmp24A,_tmp24A,_tmp24A + 6},(void*)& Cyc__genrep_274};static char _tmp24B[11]="LongLong_c";
static struct _tuple5 Cyc__gentuple_876={3,(struct _tagged_arr){_tmp24B,_tmp24B,
_tmp24B + 11},(void*)& Cyc__genrep_856};static char _tmp24C[8]="Float_c";static
struct _tuple5 Cyc__gentuple_877={4,(struct _tagged_arr){_tmp24C,_tmp24C,_tmp24C + 8},(
void*)& Cyc__genrep_62};static char _tmp24D[9]="String_c";static struct _tuple5 Cyc__gentuple_878={
5,(struct _tagged_arr){_tmp24D,_tmp24D,_tmp24D + 9},(void*)& Cyc__genrep_62};static
struct _tuple5*Cyc__genarr_879[6]={& Cyc__gentuple_873,& Cyc__gentuple_874,& Cyc__gentuple_875,&
Cyc__gentuple_876,& Cyc__gentuple_877,& Cyc__gentuple_878};static char _tmp24F[5]="Cnst";
struct Cyc_Typerep_TUnion_struct Cyc_Absyn_cnst_t_rep={5,(struct _tagged_arr){
_tmp24F,_tmp24F,_tmp24F + 5},{(void*)((struct _tuple7**)Cyc__genarr_855),(void*)((
struct _tuple7**)Cyc__genarr_855),(void*)((struct _tuple7**)Cyc__genarr_855 + 1)},{(
void*)((struct _tuple5**)Cyc__genarr_879),(void*)((struct _tuple5**)Cyc__genarr_879),(
void*)((struct _tuple5**)Cyc__genarr_879 + 6)}};static struct _tuple6 Cyc__gentuple_880={
offsetof(struct _tuple6,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_881={
offsetof(struct _tuple6,f2),(void*)& Cyc_Absyn_cnst_t_rep};static struct _tuple6*Cyc__genarr_882[
2]={& Cyc__gentuple_880,& Cyc__gentuple_881};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_853={4,sizeof(struct _tuple6),{(void*)((struct _tuple6**)Cyc__genarr_882),(
void*)((struct _tuple6**)Cyc__genarr_882),(void*)((struct _tuple6**)Cyc__genarr_882
+ 2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_840;extern struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_binding_t_rep;extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_130;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_131;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_Fndecl_rep;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_598;
extern struct Cyc_Typerep_Struct_struct Cyc_struct_List_List060Absyn_var_t4Absyn_tqual_t4Absyn_type_t1_446H2_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_599;extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_600;struct _tuple33{struct _tagged_arr*f1;struct Cyc_Absyn_Tqual f2;void*
f3;};static struct _tuple6 Cyc__gentuple_601={offsetof(struct _tuple33,f1),(void*)&
Cyc__genrep_12};static struct _tuple6 Cyc__gentuple_602={offsetof(struct _tuple33,f2),(
void*)& Cyc__genrep_135};static struct _tuple6 Cyc__gentuple_603={offsetof(struct
_tuple33,f3),(void*)((void*)& Cyc_Absyn_type_t_rep)};static struct _tuple6*Cyc__genarr_604[
3]={& Cyc__gentuple_601,& Cyc__gentuple_602,& Cyc__gentuple_603};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_600={4,sizeof(struct _tuple33),{(void*)((struct _tuple6**)Cyc__genarr_604),(
void*)((struct _tuple6**)Cyc__genarr_604),(void*)((struct _tuple6**)Cyc__genarr_604
+ 3)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_599={1,1,(void*)((void*)&
Cyc__genrep_600)};static char _tmp253[5]="List";static struct _tagged_arr Cyc__genname_608=(
struct _tagged_arr){_tmp253,_tmp253,_tmp253 + 5};static char _tmp254[3]="hd";static
struct _tuple5 Cyc__gentuple_605={offsetof(struct Cyc_List_List,hd),(struct
_tagged_arr){_tmp254,_tmp254,_tmp254 + 3},(void*)& Cyc__genrep_599};static char
_tmp255[3]="tl";static struct _tuple5 Cyc__gentuple_606={offsetof(struct Cyc_List_List,tl),(
struct _tagged_arr){_tmp255,_tmp255,_tmp255 + 3},(void*)& Cyc__genrep_598};static
struct _tuple5*Cyc__genarr_607[2]={& Cyc__gentuple_605,& Cyc__gentuple_606};struct
Cyc_Typerep_Struct_struct Cyc_struct_List_List060Absyn_var_t4Absyn_tqual_t4Absyn_type_t1_446H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_608,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_607),(void*)((struct _tuple5**)Cyc__genarr_607),(void*)((
struct _tuple5**)Cyc__genarr_607 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_598={
1,1,(void*)((void*)& Cyc_struct_List_List060Absyn_var_t4Absyn_tqual_t4Absyn_type_t1_446H2_rep)};
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_587;extern struct Cyc_Typerep_Struct_struct
Cyc_Absyn_vararg_info_t_rep;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_588;
extern struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0Absyn_var_t2_rep;static
char _tmp258[4]="Opt";static struct _tagged_arr Cyc__genname_591=(struct _tagged_arr){
_tmp258,_tmp258,_tmp258 + 4};static char _tmp259[2]="v";static struct _tuple5 Cyc__gentuple_589={
offsetof(struct Cyc_Core_Opt,v),(struct _tagged_arr){_tmp259,_tmp259,_tmp259 + 2},(
void*)& Cyc__genrep_12};static struct _tuple5*Cyc__genarr_590[1]={& Cyc__gentuple_589};
struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0Absyn_var_t2_rep={3,(struct
_tagged_arr*)& Cyc__genname_591,sizeof(struct Cyc_Core_Opt),{(void*)((struct
_tuple5**)Cyc__genarr_590),(void*)((struct _tuple5**)Cyc__genarr_590),(void*)((
struct _tuple5**)Cyc__genarr_590 + 1)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_588={
1,1,(void*)((void*)& Cyc_struct_Core_Opt0Absyn_var_t2_rep)};static char _tmp25C[11]="VarargInfo";
static struct _tagged_arr Cyc__genname_597=(struct _tagged_arr){_tmp25C,_tmp25C,
_tmp25C + 11};static char _tmp25D[5]="name";static struct _tuple5 Cyc__gentuple_592={
offsetof(struct Cyc_Absyn_VarargInfo,name),(struct _tagged_arr){_tmp25D,_tmp25D,
_tmp25D + 5},(void*)& Cyc__genrep_588};static char _tmp25E[3]="tq";static struct
_tuple5 Cyc__gentuple_593={offsetof(struct Cyc_Absyn_VarargInfo,tq),(struct
_tagged_arr){_tmp25E,_tmp25E,_tmp25E + 3},(void*)& Cyc__genrep_135};static char
_tmp25F[5]="type";static struct _tuple5 Cyc__gentuple_594={offsetof(struct Cyc_Absyn_VarargInfo,type),(
struct _tagged_arr){_tmp25F,_tmp25F,_tmp25F + 5},(void*)((void*)& Cyc_Absyn_type_t_rep)};
static char _tmp260[7]="inject";static struct _tuple5 Cyc__gentuple_595={offsetof(
struct Cyc_Absyn_VarargInfo,inject),(struct _tagged_arr){_tmp260,_tmp260,_tmp260 + 
7},(void*)((void*)& Cyc__genrep_67)};static struct _tuple5*Cyc__genarr_596[4]={& Cyc__gentuple_592,&
Cyc__gentuple_593,& Cyc__gentuple_594,& Cyc__gentuple_595};struct Cyc_Typerep_Struct_struct
Cyc_Absyn_vararg_info_t_rep={3,(struct _tagged_arr*)& Cyc__genname_597,sizeof(
struct Cyc_Absyn_VarargInfo),{(void*)((struct _tuple5**)Cyc__genarr_596),(void*)((
struct _tuple5**)Cyc__genarr_596),(void*)((struct _tuple5**)Cyc__genarr_596 + 4)}};
static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_587={1,1,(void*)((void*)& Cyc_Absyn_vararg_info_t_rep)};
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_371;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List060Absyn_type_t4Absyn_type_t1_446H2_rep;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_372;extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_373;static struct
_tuple6 Cyc__gentuple_374={offsetof(struct _tuple9,f1),(void*)((void*)& Cyc_Absyn_type_t_rep)};
static struct _tuple6 Cyc__gentuple_375={offsetof(struct _tuple9,f2),(void*)((void*)&
Cyc_Absyn_type_t_rep)};static struct _tuple6*Cyc__genarr_376[2]={& Cyc__gentuple_374,&
Cyc__gentuple_375};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_373={4,
sizeof(struct _tuple9),{(void*)((struct _tuple6**)Cyc__genarr_376),(void*)((struct
_tuple6**)Cyc__genarr_376),(void*)((struct _tuple6**)Cyc__genarr_376 + 2)}};static
struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_372={1,1,(void*)((void*)& Cyc__genrep_373)};
static char _tmp265[5]="List";static struct _tagged_arr Cyc__genname_380=(struct
_tagged_arr){_tmp265,_tmp265,_tmp265 + 5};static char _tmp266[3]="hd";static struct
_tuple5 Cyc__gentuple_377={offsetof(struct Cyc_List_List,hd),(struct _tagged_arr){
_tmp266,_tmp266,_tmp266 + 3},(void*)& Cyc__genrep_372};static char _tmp267[3]="tl";
static struct _tuple5 Cyc__gentuple_378={offsetof(struct Cyc_List_List,tl),(struct
_tagged_arr){_tmp267,_tmp267,_tmp267 + 3},(void*)& Cyc__genrep_371};static struct
_tuple5*Cyc__genarr_379[2]={& Cyc__gentuple_377,& Cyc__gentuple_378};struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List060Absyn_type_t4Absyn_type_t1_446H2_rep={3,(struct
_tagged_arr*)& Cyc__genname_380,sizeof(struct Cyc_List_List),{(void*)((struct
_tuple5**)Cyc__genarr_379),(void*)((struct _tuple5**)Cyc__genarr_379),(void*)((
struct _tuple5**)Cyc__genarr_379 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_371={
1,1,(void*)((void*)& Cyc_struct_List_List060Absyn_type_t4Absyn_type_t1_446H2_rep)};
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_163;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_Stmt_rep;extern struct Cyc_Typerep_TUnion_struct Cyc_Absyn_raw_stmt_t_rep;
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_125;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_126;static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_126={1,1,(void*)((
void*)& Cyc_struct_Absyn_Exp_rep)};struct _tuple34{unsigned int f1;struct Cyc_Absyn_Exp*
f2;};static struct _tuple6 Cyc__gentuple_127={offsetof(struct _tuple34,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_128={offsetof(struct _tuple34,f2),(
void*)& Cyc__genrep_126};static struct _tuple6*Cyc__genarr_129[2]={& Cyc__gentuple_127,&
Cyc__gentuple_128};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_125={4,
sizeof(struct _tuple34),{(void*)((struct _tuple6**)Cyc__genarr_129),(void*)((
struct _tuple6**)Cyc__genarr_129),(void*)((struct _tuple6**)Cyc__genarr_129 + 2)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_553;struct _tuple35{unsigned int
f1;struct Cyc_Absyn_Stmt*f2;struct Cyc_Absyn_Stmt*f3;};static struct _tuple6 Cyc__gentuple_554={
offsetof(struct _tuple35,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_555={
offsetof(struct _tuple35,f2),(void*)& Cyc__genrep_163};static struct _tuple6 Cyc__gentuple_556={
offsetof(struct _tuple35,f3),(void*)& Cyc__genrep_163};static struct _tuple6*Cyc__genarr_557[
3]={& Cyc__gentuple_554,& Cyc__gentuple_555,& Cyc__gentuple_556};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_553={4,sizeof(struct _tuple35),{(void*)((struct _tuple6**)Cyc__genarr_557),(
void*)((struct _tuple6**)Cyc__genarr_557),(void*)((struct _tuple6**)Cyc__genarr_557
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_549;static struct _tuple6
Cyc__gentuple_550={offsetof(struct _tuple34,f1),(void*)& Cyc__genrep_5};static
struct _tuple6 Cyc__gentuple_551={offsetof(struct _tuple34,f2),(void*)& Cyc__genrep_122};
static struct _tuple6*Cyc__genarr_552[2]={& Cyc__gentuple_550,& Cyc__gentuple_551};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_549={4,sizeof(struct _tuple34),{(
void*)((struct _tuple6**)Cyc__genarr_552),(void*)((struct _tuple6**)Cyc__genarr_552),(
void*)((struct _tuple6**)Cyc__genarr_552 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_543;struct _tuple36{unsigned int f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Stmt*
f3;struct Cyc_Absyn_Stmt*f4;};static struct _tuple6 Cyc__gentuple_544={offsetof(
struct _tuple36,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_545={
offsetof(struct _tuple36,f2),(void*)& Cyc__genrep_126};static struct _tuple6 Cyc__gentuple_546={
offsetof(struct _tuple36,f3),(void*)& Cyc__genrep_163};static struct _tuple6 Cyc__gentuple_547={
offsetof(struct _tuple36,f4),(void*)& Cyc__genrep_163};static struct _tuple6*Cyc__genarr_548[
4]={& Cyc__gentuple_544,& Cyc__gentuple_545,& Cyc__gentuple_546,& Cyc__gentuple_547};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_543={4,sizeof(struct _tuple36),{(
void*)((struct _tuple6**)Cyc__genarr_548),(void*)((struct _tuple6**)Cyc__genarr_548),(
void*)((struct _tuple6**)Cyc__genarr_548 + 4)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_538;extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_172;static struct
_tuple6 Cyc__gentuple_173={offsetof(struct _tuple2,f1),(void*)& Cyc__genrep_126};
static struct _tuple6 Cyc__gentuple_174={offsetof(struct _tuple2,f2),(void*)& Cyc__genrep_163};
static struct _tuple6*Cyc__genarr_175[2]={& Cyc__gentuple_173,& Cyc__gentuple_174};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_172={4,sizeof(struct _tuple2),{(
void*)((struct _tuple6**)Cyc__genarr_175),(void*)((struct _tuple6**)Cyc__genarr_175),(
void*)((struct _tuple6**)Cyc__genarr_175 + 2)}};struct _tuple37{unsigned int f1;
struct _tuple2 f2;struct Cyc_Absyn_Stmt*f3;};static struct _tuple6 Cyc__gentuple_539={
offsetof(struct _tuple37,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_540={
offsetof(struct _tuple37,f2),(void*)& Cyc__genrep_172};static struct _tuple6 Cyc__gentuple_541={
offsetof(struct _tuple37,f3),(void*)& Cyc__genrep_163};static struct _tuple6*Cyc__genarr_542[
3]={& Cyc__gentuple_539,& Cyc__gentuple_540,& Cyc__gentuple_541};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_538={4,sizeof(struct _tuple37),{(void*)((struct _tuple6**)Cyc__genarr_542),(
void*)((struct _tuple6**)Cyc__genarr_542),(void*)((struct _tuple6**)Cyc__genarr_542
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_534;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_529;static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_529={1,1,(void*)((
void*)& Cyc_struct_Absyn_Stmt_rep)};struct _tuple38{unsigned int f1;struct Cyc_Absyn_Stmt*
f2;};static struct _tuple6 Cyc__gentuple_535={offsetof(struct _tuple38,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_536={offsetof(struct _tuple38,f2),(
void*)& Cyc__genrep_529};static struct _tuple6*Cyc__genarr_537[2]={& Cyc__gentuple_535,&
Cyc__gentuple_536};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_534={4,
sizeof(struct _tuple38),{(void*)((struct _tuple6**)Cyc__genarr_537),(void*)((
struct _tuple6**)Cyc__genarr_537),(void*)((struct _tuple6**)Cyc__genarr_537 + 2)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_528;struct _tuple39{unsigned int
f1;struct _tagged_arr*f2;struct Cyc_Absyn_Stmt*f3;};static struct _tuple6 Cyc__gentuple_530={
offsetof(struct _tuple39,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_531={
offsetof(struct _tuple39,f2),(void*)& Cyc__genrep_12};static struct _tuple6 Cyc__gentuple_532={
offsetof(struct _tuple39,f3),(void*)& Cyc__genrep_529};static struct _tuple6*Cyc__genarr_533[
3]={& Cyc__gentuple_530,& Cyc__gentuple_531,& Cyc__gentuple_532};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_528={4,sizeof(struct _tuple39),{(void*)((struct _tuple6**)Cyc__genarr_533),(
void*)((struct _tuple6**)Cyc__genarr_533),(void*)((struct _tuple6**)Cyc__genarr_533
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_521;struct _tuple40{
unsigned int f1;struct Cyc_Absyn_Exp*f2;struct _tuple2 f3;struct _tuple2 f4;struct Cyc_Absyn_Stmt*
f5;};static struct _tuple6 Cyc__gentuple_522={offsetof(struct _tuple40,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_523={offsetof(struct _tuple40,f2),(
void*)& Cyc__genrep_126};static struct _tuple6 Cyc__gentuple_524={offsetof(struct
_tuple40,f3),(void*)& Cyc__genrep_172};static struct _tuple6 Cyc__gentuple_525={
offsetof(struct _tuple40,f4),(void*)& Cyc__genrep_172};static struct _tuple6 Cyc__gentuple_526={
offsetof(struct _tuple40,f5),(void*)& Cyc__genrep_163};static struct _tuple6*Cyc__genarr_527[
5]={& Cyc__gentuple_522,& Cyc__gentuple_523,& Cyc__gentuple_524,& Cyc__gentuple_525,&
Cyc__gentuple_526};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_521={4,
sizeof(struct _tuple40),{(void*)((struct _tuple6**)Cyc__genarr_527),(void*)((
struct _tuple6**)Cyc__genarr_527),(void*)((struct _tuple6**)Cyc__genarr_527 + 5)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_516;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_230;extern struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_switch_clause_t46H2_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_231;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_Switch_clause_rep;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_232;
extern struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Pat_rep;extern struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_raw_pat_t_rep;extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_426;
struct _tuple41{unsigned int f1;struct Cyc_Absyn_Tvar*f2;struct Cyc_Absyn_Vardecl*f3;
};static struct _tuple6 Cyc__gentuple_427={offsetof(struct _tuple41,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_428={offsetof(struct _tuple41,f2),(void*)& Cyc__genrep_186};
static struct _tuple6 Cyc__gentuple_429={offsetof(struct _tuple41,f3),(void*)& Cyc__genrep_134};
static struct _tuple6*Cyc__genarr_430[3]={& Cyc__gentuple_427,& Cyc__gentuple_428,&
Cyc__gentuple_429};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_426={4,
sizeof(struct _tuple41),{(void*)((struct _tuple6**)Cyc__genarr_430),(void*)((
struct _tuple6**)Cyc__genarr_430),(void*)((struct _tuple6**)Cyc__genarr_430 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_422;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_237;extern struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_pat_t46H2_rep;
static char _tmp276[5]="List";static struct _tagged_arr Cyc__genname_241=(struct
_tagged_arr){_tmp276,_tmp276,_tmp276 + 5};static char _tmp277[3]="hd";static struct
_tuple5 Cyc__gentuple_238={offsetof(struct Cyc_List_List,hd),(struct _tagged_arr){
_tmp277,_tmp277,_tmp277 + 3},(void*)& Cyc__genrep_232};static char _tmp278[3]="tl";
static struct _tuple5 Cyc__gentuple_239={offsetof(struct Cyc_List_List,tl),(struct
_tagged_arr){_tmp278,_tmp278,_tmp278 + 3},(void*)& Cyc__genrep_237};static struct
_tuple5*Cyc__genarr_240[2]={& Cyc__gentuple_238,& Cyc__gentuple_239};struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List0Absyn_pat_t46H2_rep={3,(struct _tagged_arr*)& Cyc__genname_241,
sizeof(struct Cyc_List_List),{(void*)((struct _tuple5**)Cyc__genarr_240),(void*)((
struct _tuple5**)Cyc__genarr_240),(void*)((struct _tuple5**)Cyc__genarr_240 + 2)}};
static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_237={1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_pat_t46H2_rep)};
static struct _tuple6 Cyc__gentuple_423={offsetof(struct _tuple13,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_424={offsetof(struct _tuple13,f2),(void*)& Cyc__genrep_237};
static struct _tuple6*Cyc__genarr_425[2]={& Cyc__gentuple_423,& Cyc__gentuple_424};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_422={4,sizeof(struct _tuple13),{(
void*)((struct _tuple6**)Cyc__genarr_425),(void*)((struct _tuple6**)Cyc__genarr_425),(
void*)((struct _tuple6**)Cyc__genarr_425 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_418;struct _tuple42{unsigned int f1;struct Cyc_Absyn_Pat*f2;};static
struct _tuple6 Cyc__gentuple_419={offsetof(struct _tuple42,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_420={offsetof(struct _tuple42,f2),(void*)& Cyc__genrep_232};
static struct _tuple6*Cyc__genarr_421[2]={& Cyc__gentuple_419,& Cyc__gentuple_420};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_418={4,sizeof(struct _tuple42),{(
void*)((struct _tuple6**)Cyc__genarr_421),(void*)((struct _tuple6**)Cyc__genarr_421),(
void*)((struct _tuple6**)Cyc__genarr_421 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_329;extern struct Cyc_Typerep_Struct_struct Cyc_Absyn_aggr_info_t_rep;
extern struct Cyc_Typerep_TUnion_struct Cyc_tunion_Absyn_AggrInfoU_rep;extern struct
Cyc_Typerep_Tuple_struct Cyc__genrep_401;extern struct Cyc_Typerep_TUnion_struct Cyc_Absyn_aggr_kind_t_rep;
static char _tmp27D[8]="StructA";static struct _tuple7 Cyc__gentuple_386={0,(struct
_tagged_arr){_tmp27D,_tmp27D,_tmp27D + 8}};static char _tmp27E[7]="UnionA";static
struct _tuple7 Cyc__gentuple_387={1,(struct _tagged_arr){_tmp27E,_tmp27E,_tmp27E + 7}};
static struct _tuple7*Cyc__genarr_388[2]={& Cyc__gentuple_386,& Cyc__gentuple_387};
static struct _tuple5*Cyc__genarr_389[0]={};static char _tmp280[9]="AggrKind";struct
Cyc_Typerep_TUnion_struct Cyc_Absyn_aggr_kind_t_rep={5,(struct _tagged_arr){
_tmp280,_tmp280,_tmp280 + 9},{(void*)((struct _tuple7**)Cyc__genarr_388),(void*)((
struct _tuple7**)Cyc__genarr_388),(void*)((struct _tuple7**)Cyc__genarr_388 + 2)},{(
void*)((struct _tuple5**)Cyc__genarr_389),(void*)((struct _tuple5**)Cyc__genarr_389),(
void*)((struct _tuple5**)Cyc__genarr_389 + 0)}};struct _tuple43{unsigned int f1;void*
f2;struct _tuple0*f3;};static struct _tuple6 Cyc__gentuple_402={offsetof(struct
_tuple43,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_403={
offsetof(struct _tuple43,f2),(void*)& Cyc_Absyn_aggr_kind_t_rep};static struct
_tuple6 Cyc__gentuple_404={offsetof(struct _tuple43,f3),(void*)& Cyc__genrep_10};
static struct _tuple6*Cyc__genarr_405[3]={& Cyc__gentuple_402,& Cyc__gentuple_403,&
Cyc__gentuple_404};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_401={4,
sizeof(struct _tuple43),{(void*)((struct _tuple6**)Cyc__genarr_405),(void*)((
struct _tuple6**)Cyc__genarr_405),(void*)((struct _tuple6**)Cyc__genarr_405 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_354;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_355;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_356;extern
struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Aggrdecl_rep;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_357;extern struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_AggrdeclImpl_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_358;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List0Absyn_aggrfield_t46H2_rep;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_359;extern struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Aggrfield_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_43;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List0Absyn_attribute_t46H2_rep;extern struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_attribute_t_rep;extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_71;
extern struct Cyc_Typerep_TUnion_struct Cyc_tunion_Absyn_Format_Type_rep;static char
_tmp282[10]="Printf_ft";static struct _tuple7 Cyc__gentuple_72={0,(struct
_tagged_arr){_tmp282,_tmp282,_tmp282 + 10}};static char _tmp283[9]="Scanf_ft";
static struct _tuple7 Cyc__gentuple_73={1,(struct _tagged_arr){_tmp283,_tmp283,
_tmp283 + 9}};static struct _tuple7*Cyc__genarr_74[2]={& Cyc__gentuple_72,& Cyc__gentuple_73};
static struct _tuple5*Cyc__genarr_75[0]={};static char _tmp285[12]="Format_Type";
struct Cyc_Typerep_TUnion_struct Cyc_tunion_Absyn_Format_Type_rep={5,(struct
_tagged_arr){_tmp285,_tmp285,_tmp285 + 12},{(void*)((struct _tuple7**)Cyc__genarr_74),(
void*)((struct _tuple7**)Cyc__genarr_74),(void*)((struct _tuple7**)Cyc__genarr_74 + 
2)},{(void*)((struct _tuple5**)Cyc__genarr_75),(void*)((struct _tuple5**)Cyc__genarr_75),(
void*)((struct _tuple5**)Cyc__genarr_75 + 0)}};struct _tuple44{unsigned int f1;void*
f2;int f3;int f4;};static struct _tuple6 Cyc__gentuple_76={offsetof(struct _tuple44,f1),(
void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_77={offsetof(struct
_tuple44,f2),(void*)& Cyc_tunion_Absyn_Format_Type_rep};static struct _tuple6 Cyc__gentuple_78={
offsetof(struct _tuple44,f3),(void*)((void*)& Cyc__genrep_67)};static struct _tuple6
Cyc__gentuple_79={offsetof(struct _tuple44,f4),(void*)((void*)& Cyc__genrep_67)};
static struct _tuple6*Cyc__genarr_80[4]={& Cyc__gentuple_76,& Cyc__gentuple_77,& Cyc__gentuple_78,&
Cyc__gentuple_79};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_71={4,sizeof(
struct _tuple44),{(void*)((struct _tuple6**)Cyc__genarr_80),(void*)((struct _tuple6**)
Cyc__genarr_80),(void*)((struct _tuple6**)Cyc__genarr_80 + 4)}};static char _tmp287[
12]="Stdcall_att";static struct _tuple7 Cyc__gentuple_44={0,(struct _tagged_arr){
_tmp287,_tmp287,_tmp287 + 12}};static char _tmp288[10]="Cdecl_att";static struct
_tuple7 Cyc__gentuple_45={1,(struct _tagged_arr){_tmp288,_tmp288,_tmp288 + 10}};
static char _tmp289[13]="Fastcall_att";static struct _tuple7 Cyc__gentuple_46={2,(
struct _tagged_arr){_tmp289,_tmp289,_tmp289 + 13}};static char _tmp28A[13]="Noreturn_att";
static struct _tuple7 Cyc__gentuple_47={3,(struct _tagged_arr){_tmp28A,_tmp28A,
_tmp28A + 13}};static char _tmp28B[10]="Const_att";static struct _tuple7 Cyc__gentuple_48={
4,(struct _tagged_arr){_tmp28B,_tmp28B,_tmp28B + 10}};static char _tmp28C[11]="Packed_att";
static struct _tuple7 Cyc__gentuple_49={5,(struct _tagged_arr){_tmp28C,_tmp28C,
_tmp28C + 11}};static char _tmp28D[13]="Nocommon_att";static struct _tuple7 Cyc__gentuple_50={
6,(struct _tagged_arr){_tmp28D,_tmp28D,_tmp28D + 13}};static char _tmp28E[11]="Shared_att";
static struct _tuple7 Cyc__gentuple_51={7,(struct _tagged_arr){_tmp28E,_tmp28E,
_tmp28E + 11}};static char _tmp28F[11]="Unused_att";static struct _tuple7 Cyc__gentuple_52={
8,(struct _tagged_arr){_tmp28F,_tmp28F,_tmp28F + 11}};static char _tmp290[9]="Weak_att";
static struct _tuple7 Cyc__gentuple_53={9,(struct _tagged_arr){_tmp290,_tmp290,
_tmp290 + 9}};static char _tmp291[14]="Dllimport_att";static struct _tuple7 Cyc__gentuple_54={
10,(struct _tagged_arr){_tmp291,_tmp291,_tmp291 + 14}};static char _tmp292[14]="Dllexport_att";
static struct _tuple7 Cyc__gentuple_55={11,(struct _tagged_arr){_tmp292,_tmp292,
_tmp292 + 14}};static char _tmp293[27]="No_instrument_function_att";static struct
_tuple7 Cyc__gentuple_56={12,(struct _tagged_arr){_tmp293,_tmp293,_tmp293 + 27}};
static char _tmp294[16]="Constructor_att";static struct _tuple7 Cyc__gentuple_57={13,(
struct _tagged_arr){_tmp294,_tmp294,_tmp294 + 16}};static char _tmp295[15]="Destructor_att";
static struct _tuple7 Cyc__gentuple_58={14,(struct _tagged_arr){_tmp295,_tmp295,
_tmp295 + 15}};static char _tmp296[26]="No_check_memory_usage_att";static struct
_tuple7 Cyc__gentuple_59={15,(struct _tagged_arr){_tmp296,_tmp296,_tmp296 + 26}};
static char _tmp297[9]="Pure_att";static struct _tuple7 Cyc__gentuple_60={16,(struct
_tagged_arr){_tmp297,_tmp297,_tmp297 + 9}};static struct _tuple7*Cyc__genarr_61[17]={&
Cyc__gentuple_44,& Cyc__gentuple_45,& Cyc__gentuple_46,& Cyc__gentuple_47,& Cyc__gentuple_48,&
Cyc__gentuple_49,& Cyc__gentuple_50,& Cyc__gentuple_51,& Cyc__gentuple_52,& Cyc__gentuple_53,&
Cyc__gentuple_54,& Cyc__gentuple_55,& Cyc__gentuple_56,& Cyc__gentuple_57,& Cyc__gentuple_58,&
Cyc__gentuple_59,& Cyc__gentuple_60};static char _tmp298[12]="Regparm_att";static
struct _tuple5 Cyc__gentuple_81={0,(struct _tagged_arr){_tmp298,_tmp298,_tmp298 + 12},(
void*)& Cyc__genrep_66};static char _tmp299[12]="Aligned_att";static struct _tuple5
Cyc__gentuple_82={1,(struct _tagged_arr){_tmp299,_tmp299,_tmp299 + 12},(void*)& Cyc__genrep_66};
static char _tmp29A[12]="Section_att";static struct _tuple5 Cyc__gentuple_83={2,(
struct _tagged_arr){_tmp29A,_tmp29A,_tmp29A + 12},(void*)& Cyc__genrep_62};static
char _tmp29B[11]="Format_att";static struct _tuple5 Cyc__gentuple_84={3,(struct
_tagged_arr){_tmp29B,_tmp29B,_tmp29B + 11},(void*)& Cyc__genrep_71};static char
_tmp29C[16]="Initializes_att";static struct _tuple5 Cyc__gentuple_85={4,(struct
_tagged_arr){_tmp29C,_tmp29C,_tmp29C + 16},(void*)& Cyc__genrep_66};static char
_tmp29D[9]="Mode_att";static struct _tuple5 Cyc__gentuple_86={5,(struct _tagged_arr){
_tmp29D,_tmp29D,_tmp29D + 9},(void*)& Cyc__genrep_62};static struct _tuple5*Cyc__genarr_87[
6]={& Cyc__gentuple_81,& Cyc__gentuple_82,& Cyc__gentuple_83,& Cyc__gentuple_84,& Cyc__gentuple_85,&
Cyc__gentuple_86};static char _tmp29F[10]="Attribute";struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_attribute_t_rep={5,(struct _tagged_arr){_tmp29F,_tmp29F,_tmp29F + 10},{(
void*)((struct _tuple7**)Cyc__genarr_61),(void*)((struct _tuple7**)Cyc__genarr_61),(
void*)((struct _tuple7**)Cyc__genarr_61 + 17)},{(void*)((struct _tuple5**)Cyc__genarr_87),(
void*)((struct _tuple5**)Cyc__genarr_87),(void*)((struct _tuple5**)Cyc__genarr_87 + 
6)}};static char _tmp2A0[5]="List";static struct _tagged_arr Cyc__genname_91=(struct
_tagged_arr){_tmp2A0,_tmp2A0,_tmp2A0 + 5};static char _tmp2A1[3]="hd";static struct
_tuple5 Cyc__gentuple_88={offsetof(struct Cyc_List_List,hd),(struct _tagged_arr){
_tmp2A1,_tmp2A1,_tmp2A1 + 3},(void*)& Cyc_Absyn_attribute_t_rep};static char _tmp2A2[
3]="tl";static struct _tuple5 Cyc__gentuple_89={offsetof(struct Cyc_List_List,tl),(
struct _tagged_arr){_tmp2A2,_tmp2A2,_tmp2A2 + 3},(void*)& Cyc__genrep_43};static
struct _tuple5*Cyc__genarr_90[2]={& Cyc__gentuple_88,& Cyc__gentuple_89};struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List0Absyn_attribute_t46H2_rep={3,(struct _tagged_arr*)& Cyc__genname_91,
sizeof(struct Cyc_List_List),{(void*)((struct _tuple5**)Cyc__genarr_90),(void*)((
struct _tuple5**)Cyc__genarr_90),(void*)((struct _tuple5**)Cyc__genarr_90 + 2)}};
static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_43={1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_attribute_t46H2_rep)};
static char _tmp2A5[10]="Aggrfield";static struct _tagged_arr Cyc__genname_366=(
struct _tagged_arr){_tmp2A5,_tmp2A5,_tmp2A5 + 10};static char _tmp2A6[5]="name";
static struct _tuple5 Cyc__gentuple_360={offsetof(struct Cyc_Absyn_Aggrfield,name),(
struct _tagged_arr){_tmp2A6,_tmp2A6,_tmp2A6 + 5},(void*)& Cyc__genrep_12};static
char _tmp2A7[3]="tq";static struct _tuple5 Cyc__gentuple_361={offsetof(struct Cyc_Absyn_Aggrfield,tq),(
struct _tagged_arr){_tmp2A7,_tmp2A7,_tmp2A7 + 3},(void*)& Cyc__genrep_135};static
char _tmp2A8[5]="type";static struct _tuple5 Cyc__gentuple_362={offsetof(struct Cyc_Absyn_Aggrfield,type),(
struct _tagged_arr){_tmp2A8,_tmp2A8,_tmp2A8 + 5},(void*)((void*)& Cyc_Absyn_type_t_rep)};
static char _tmp2A9[6]="width";static struct _tuple5 Cyc__gentuple_363={offsetof(
struct Cyc_Absyn_Aggrfield,width),(struct _tagged_arr){_tmp2A9,_tmp2A9,_tmp2A9 + 6},(
void*)& Cyc__genrep_122};static char _tmp2AA[11]="attributes";static struct _tuple5
Cyc__gentuple_364={offsetof(struct Cyc_Absyn_Aggrfield,attributes),(struct
_tagged_arr){_tmp2AA,_tmp2AA,_tmp2AA + 11},(void*)& Cyc__genrep_43};static struct
_tuple5*Cyc__genarr_365[5]={& Cyc__gentuple_360,& Cyc__gentuple_361,& Cyc__gentuple_362,&
Cyc__gentuple_363,& Cyc__gentuple_364};struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Aggrfield_rep={
3,(struct _tagged_arr*)& Cyc__genname_366,sizeof(struct Cyc_Absyn_Aggrfield),{(void*)((
struct _tuple5**)Cyc__genarr_365),(void*)((struct _tuple5**)Cyc__genarr_365),(void*)((
struct _tuple5**)Cyc__genarr_365 + 5)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_359={
1,1,(void*)((void*)& Cyc_struct_Absyn_Aggrfield_rep)};static char _tmp2AD[5]="List";
static struct _tagged_arr Cyc__genname_370=(struct _tagged_arr){_tmp2AD,_tmp2AD,
_tmp2AD + 5};static char _tmp2AE[3]="hd";static struct _tuple5 Cyc__gentuple_367={
offsetof(struct Cyc_List_List,hd),(struct _tagged_arr){_tmp2AE,_tmp2AE,_tmp2AE + 3},(
void*)& Cyc__genrep_359};static char _tmp2AF[3]="tl";static struct _tuple5 Cyc__gentuple_368={
offsetof(struct Cyc_List_List,tl),(struct _tagged_arr){_tmp2AF,_tmp2AF,_tmp2AF + 3},(
void*)& Cyc__genrep_358};static struct _tuple5*Cyc__genarr_369[2]={& Cyc__gentuple_367,&
Cyc__gentuple_368};struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_aggrfield_t46H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_370,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_369),(void*)((struct _tuple5**)Cyc__genarr_369),(void*)((
struct _tuple5**)Cyc__genarr_369 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_358={
1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_aggrfield_t46H2_rep)};static char
_tmp2B2[13]="AggrdeclImpl";static struct _tagged_arr Cyc__genname_385=(struct
_tagged_arr){_tmp2B2,_tmp2B2,_tmp2B2 + 13};static char _tmp2B3[11]="exist_vars";
static struct _tuple5 Cyc__gentuple_381={offsetof(struct Cyc_Absyn_AggrdeclImpl,exist_vars),(
struct _tagged_arr){_tmp2B3,_tmp2B3,_tmp2B3 + 11},(void*)& Cyc__genrep_312};static
char _tmp2B4[7]="rgn_po";static struct _tuple5 Cyc__gentuple_382={offsetof(struct Cyc_Absyn_AggrdeclImpl,rgn_po),(
struct _tagged_arr){_tmp2B4,_tmp2B4,_tmp2B4 + 7},(void*)& Cyc__genrep_371};static
char _tmp2B5[7]="fields";static struct _tuple5 Cyc__gentuple_383={offsetof(struct Cyc_Absyn_AggrdeclImpl,fields),(
struct _tagged_arr){_tmp2B5,_tmp2B5,_tmp2B5 + 7},(void*)& Cyc__genrep_358};static
struct _tuple5*Cyc__genarr_384[3]={& Cyc__gentuple_381,& Cyc__gentuple_382,& Cyc__gentuple_383};
struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_AggrdeclImpl_rep={3,(struct
_tagged_arr*)& Cyc__genname_385,sizeof(struct Cyc_Absyn_AggrdeclImpl),{(void*)((
struct _tuple5**)Cyc__genarr_384),(void*)((struct _tuple5**)Cyc__genarr_384),(void*)((
struct _tuple5**)Cyc__genarr_384 + 3)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_357={
1,1,(void*)((void*)& Cyc_struct_Absyn_AggrdeclImpl_rep)};static char _tmp2B8[9]="Aggrdecl";
static struct _tagged_arr Cyc__genname_397=(struct _tagged_arr){_tmp2B8,_tmp2B8,
_tmp2B8 + 9};static char _tmp2B9[5]="kind";static struct _tuple5 Cyc__gentuple_390={
offsetof(struct Cyc_Absyn_Aggrdecl,kind),(struct _tagged_arr){_tmp2B9,_tmp2B9,
_tmp2B9 + 5},(void*)& Cyc_Absyn_aggr_kind_t_rep};static char _tmp2BA[3]="sc";static
struct _tuple5 Cyc__gentuple_391={offsetof(struct Cyc_Absyn_Aggrdecl,sc),(struct
_tagged_arr){_tmp2BA,_tmp2BA,_tmp2BA + 3},(void*)& Cyc_Absyn_scope_t_rep};static
char _tmp2BB[5]="name";static struct _tuple5 Cyc__gentuple_392={offsetof(struct Cyc_Absyn_Aggrdecl,name),(
struct _tagged_arr){_tmp2BB,_tmp2BB,_tmp2BB + 5},(void*)& Cyc__genrep_10};static
char _tmp2BC[4]="tvs";static struct _tuple5 Cyc__gentuple_393={offsetof(struct Cyc_Absyn_Aggrdecl,tvs),(
struct _tagged_arr){_tmp2BC,_tmp2BC,_tmp2BC + 4},(void*)& Cyc__genrep_312};static
char _tmp2BD[5]="impl";static struct _tuple5 Cyc__gentuple_394={offsetof(struct Cyc_Absyn_Aggrdecl,impl),(
struct _tagged_arr){_tmp2BD,_tmp2BD,_tmp2BD + 5},(void*)& Cyc__genrep_357};static
char _tmp2BE[11]="attributes";static struct _tuple5 Cyc__gentuple_395={offsetof(
struct Cyc_Absyn_Aggrdecl,attributes),(struct _tagged_arr){_tmp2BE,_tmp2BE,_tmp2BE
+ 11},(void*)& Cyc__genrep_43};static struct _tuple5*Cyc__genarr_396[6]={& Cyc__gentuple_390,&
Cyc__gentuple_391,& Cyc__gentuple_392,& Cyc__gentuple_393,& Cyc__gentuple_394,& Cyc__gentuple_395};
struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Aggrdecl_rep={3,(struct
_tagged_arr*)& Cyc__genname_397,sizeof(struct Cyc_Absyn_Aggrdecl),{(void*)((struct
_tuple5**)Cyc__genarr_396),(void*)((struct _tuple5**)Cyc__genarr_396),(void*)((
struct _tuple5**)Cyc__genarr_396 + 6)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_356={
1,1,(void*)((void*)& Cyc_struct_Absyn_Aggrdecl_rep)};static struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_355={1,1,(void*)((void*)& Cyc__genrep_356)};struct _tuple45{
unsigned int f1;struct Cyc_Absyn_Aggrdecl**f2;};static struct _tuple6 Cyc__gentuple_398={
offsetof(struct _tuple45,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_399={
offsetof(struct _tuple45,f2),(void*)& Cyc__genrep_355};static struct _tuple6*Cyc__genarr_400[
2]={& Cyc__gentuple_398,& Cyc__gentuple_399};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_354={4,sizeof(struct _tuple45),{(void*)((struct _tuple6**)Cyc__genarr_400),(
void*)((struct _tuple6**)Cyc__genarr_400),(void*)((struct _tuple6**)Cyc__genarr_400
+ 2)}};static struct _tuple7*Cyc__genarr_353[0]={};static char _tmp2C3[12]="UnknownAggr";
static struct _tuple5 Cyc__gentuple_406={0,(struct _tagged_arr){_tmp2C3,_tmp2C3,
_tmp2C3 + 12},(void*)& Cyc__genrep_401};static char _tmp2C4[10]="KnownAggr";static
struct _tuple5 Cyc__gentuple_407={1,(struct _tagged_arr){_tmp2C4,_tmp2C4,_tmp2C4 + 
10},(void*)& Cyc__genrep_354};static struct _tuple5*Cyc__genarr_408[2]={& Cyc__gentuple_406,&
Cyc__gentuple_407};static char _tmp2C6[10]="AggrInfoU";struct Cyc_Typerep_TUnion_struct
Cyc_tunion_Absyn_AggrInfoU_rep={5,(struct _tagged_arr){_tmp2C6,_tmp2C6,_tmp2C6 + 
10},{(void*)((struct _tuple7**)Cyc__genarr_353),(void*)((struct _tuple7**)Cyc__genarr_353),(
void*)((struct _tuple7**)Cyc__genarr_353 + 0)},{(void*)((struct _tuple5**)Cyc__genarr_408),(
void*)((struct _tuple5**)Cyc__genarr_408),(void*)((struct _tuple5**)Cyc__genarr_408
+ 2)}};static char _tmp2C7[9]="AggrInfo";static struct _tagged_arr Cyc__genname_412=(
struct _tagged_arr){_tmp2C7,_tmp2C7,_tmp2C7 + 9};static char _tmp2C8[10]="aggr_info";
static struct _tuple5 Cyc__gentuple_409={offsetof(struct Cyc_Absyn_AggrInfo,aggr_info),(
struct _tagged_arr){_tmp2C8,_tmp2C8,_tmp2C8 + 10},(void*)& Cyc_tunion_Absyn_AggrInfoU_rep};
static char _tmp2C9[6]="targs";static struct _tuple5 Cyc__gentuple_410={offsetof(
struct Cyc_Absyn_AggrInfo,targs),(struct _tagged_arr){_tmp2C9,_tmp2C9,_tmp2C9 + 6},(
void*)& Cyc__genrep_102};static struct _tuple5*Cyc__genarr_411[2]={& Cyc__gentuple_409,&
Cyc__gentuple_410};struct Cyc_Typerep_Struct_struct Cyc_Absyn_aggr_info_t_rep={3,(
struct _tagged_arr*)& Cyc__genname_412,sizeof(struct Cyc_Absyn_AggrInfo),{(void*)((
struct _tuple5**)Cyc__genarr_411),(void*)((struct _tuple5**)Cyc__genarr_411),(void*)((
struct _tuple5**)Cyc__genarr_411 + 2)}};extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_330;
extern struct Cyc_Typerep_Struct_struct Cyc_struct_List_List060List_list_t0Absyn_designator_t46H24Absyn_pat_t1_446H2_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_331;extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_332;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_333;extern
struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_designator_t46H2_rep;
extern struct Cyc_Typerep_TUnion_struct Cyc_Absyn_designator_t_rep;extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_335;struct _tuple46{unsigned int f1;struct _tagged_arr*f2;};static
struct _tuple6 Cyc__gentuple_336={offsetof(struct _tuple46,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_337={offsetof(struct _tuple46,f2),(void*)& Cyc__genrep_12};
static struct _tuple6*Cyc__genarr_338[2]={& Cyc__gentuple_336,& Cyc__gentuple_337};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_335={4,sizeof(struct _tuple46),{(
void*)((struct _tuple6**)Cyc__genarr_338),(void*)((struct _tuple6**)Cyc__genarr_338),(
void*)((struct _tuple6**)Cyc__genarr_338 + 2)}};static struct _tuple7*Cyc__genarr_334[
0]={};static char _tmp2CC[13]="ArrayElement";static struct _tuple5 Cyc__gentuple_339={
0,(struct _tagged_arr){_tmp2CC,_tmp2CC,_tmp2CC + 13},(void*)& Cyc__genrep_125};
static char _tmp2CD[10]="FieldName";static struct _tuple5 Cyc__gentuple_340={1,(
struct _tagged_arr){_tmp2CD,_tmp2CD,_tmp2CD + 10},(void*)& Cyc__genrep_335};static
struct _tuple5*Cyc__genarr_341[2]={& Cyc__gentuple_339,& Cyc__gentuple_340};static
char _tmp2CF[11]="Designator";struct Cyc_Typerep_TUnion_struct Cyc_Absyn_designator_t_rep={
5,(struct _tagged_arr){_tmp2CF,_tmp2CF,_tmp2CF + 11},{(void*)((struct _tuple7**)Cyc__genarr_334),(
void*)((struct _tuple7**)Cyc__genarr_334),(void*)((struct _tuple7**)Cyc__genarr_334
+ 0)},{(void*)((struct _tuple5**)Cyc__genarr_341),(void*)((struct _tuple5**)Cyc__genarr_341),(
void*)((struct _tuple5**)Cyc__genarr_341 + 2)}};static char _tmp2D0[5]="List";static
struct _tagged_arr Cyc__genname_345=(struct _tagged_arr){_tmp2D0,_tmp2D0,_tmp2D0 + 5};
static char _tmp2D1[3]="hd";static struct _tuple5 Cyc__gentuple_342={offsetof(struct
Cyc_List_List,hd),(struct _tagged_arr){_tmp2D1,_tmp2D1,_tmp2D1 + 3},(void*)& Cyc_Absyn_designator_t_rep};
static char _tmp2D2[3]="tl";static struct _tuple5 Cyc__gentuple_343={offsetof(struct
Cyc_List_List,tl),(struct _tagged_arr){_tmp2D2,_tmp2D2,_tmp2D2 + 3},(void*)& Cyc__genrep_333};
static struct _tuple5*Cyc__genarr_344[2]={& Cyc__gentuple_342,& Cyc__gentuple_343};
struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_designator_t46H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_345,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_344),(void*)((struct _tuple5**)Cyc__genarr_344),(void*)((
struct _tuple5**)Cyc__genarr_344 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_333={
1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_designator_t46H2_rep)};struct
_tuple47{struct Cyc_List_List*f1;struct Cyc_Absyn_Pat*f2;};static struct _tuple6 Cyc__gentuple_346={
offsetof(struct _tuple47,f1),(void*)& Cyc__genrep_333};static struct _tuple6 Cyc__gentuple_347={
offsetof(struct _tuple47,f2),(void*)& Cyc__genrep_232};static struct _tuple6*Cyc__genarr_348[
2]={& Cyc__gentuple_346,& Cyc__gentuple_347};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_332={4,sizeof(struct _tuple47),{(void*)((struct _tuple6**)Cyc__genarr_348),(
void*)((struct _tuple6**)Cyc__genarr_348),(void*)((struct _tuple6**)Cyc__genarr_348
+ 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_331={1,1,(void*)((void*)&
Cyc__genrep_332)};static char _tmp2D7[5]="List";static struct _tagged_arr Cyc__genname_352=(
struct _tagged_arr){_tmp2D7,_tmp2D7,_tmp2D7 + 5};static char _tmp2D8[3]="hd";static
struct _tuple5 Cyc__gentuple_349={offsetof(struct Cyc_List_List,hd),(struct
_tagged_arr){_tmp2D8,_tmp2D8,_tmp2D8 + 3},(void*)& Cyc__genrep_331};static char
_tmp2D9[3]="tl";static struct _tuple5 Cyc__gentuple_350={offsetof(struct Cyc_List_List,tl),(
struct _tagged_arr){_tmp2D9,_tmp2D9,_tmp2D9 + 3},(void*)& Cyc__genrep_330};static
struct _tuple5*Cyc__genarr_351[2]={& Cyc__gentuple_349,& Cyc__gentuple_350};struct
Cyc_Typerep_Struct_struct Cyc_struct_List_List060List_list_t0Absyn_designator_t46H24Absyn_pat_t1_446H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_352,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_351),(void*)((struct _tuple5**)Cyc__genarr_351),(void*)((
struct _tuple5**)Cyc__genarr_351 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_330={
1,1,(void*)((void*)& Cyc_struct_List_List060List_list_t0Absyn_designator_t46H24Absyn_pat_t1_446H2_rep)};
struct _tuple48{unsigned int f1;struct Cyc_Absyn_AggrInfo f2;struct Cyc_List_List*f3;
struct Cyc_List_List*f4;};static struct _tuple6 Cyc__gentuple_413={offsetof(struct
_tuple48,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_414={
offsetof(struct _tuple48,f2),(void*)& Cyc_Absyn_aggr_info_t_rep};static struct
_tuple6 Cyc__gentuple_415={offsetof(struct _tuple48,f3),(void*)& Cyc__genrep_312};
static struct _tuple6 Cyc__gentuple_416={offsetof(struct _tuple48,f4),(void*)& Cyc__genrep_330};
static struct _tuple6*Cyc__genarr_417[4]={& Cyc__gentuple_413,& Cyc__gentuple_414,&
Cyc__gentuple_415,& Cyc__gentuple_416};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_329={
4,sizeof(struct _tuple48),{(void*)((struct _tuple6**)Cyc__genarr_417),(void*)((
struct _tuple6**)Cyc__genarr_417),(void*)((struct _tuple6**)Cyc__genarr_417 + 4)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_284;struct _tuple49{unsigned int
f1;struct Cyc_Absyn_Tuniondecl*f2;struct Cyc_Absyn_Tunionfield*f3;struct Cyc_List_List*
f4;};static struct _tuple6 Cyc__gentuple_324={offsetof(struct _tuple49,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_325={offsetof(struct _tuple49,f2),(
void*)((void*)& Cyc__genrep_302)};static struct _tuple6 Cyc__gentuple_326={offsetof(
struct _tuple49,f3),(void*)& Cyc__genrep_285};static struct _tuple6 Cyc__gentuple_327={
offsetof(struct _tuple49,f4),(void*)& Cyc__genrep_237};static struct _tuple6*Cyc__genarr_328[
4]={& Cyc__gentuple_324,& Cyc__gentuple_325,& Cyc__gentuple_326,& Cyc__gentuple_327};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_284={4,sizeof(struct _tuple49),{(
void*)((struct _tuple6**)Cyc__genarr_328),(void*)((struct _tuple6**)Cyc__genarr_328),(
void*)((struct _tuple6**)Cyc__genarr_328 + 4)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_270;struct _tuple50{unsigned int f1;char f2;};static struct _tuple6 Cyc__gentuple_271={
offsetof(struct _tuple50,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_272={
offsetof(struct _tuple50,f2),(void*)((void*)& Cyc__genrep_14)};static struct _tuple6*
Cyc__genarr_273[2]={& Cyc__gentuple_271,& Cyc__gentuple_272};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_270={4,sizeof(struct _tuple50),{(void*)((struct _tuple6**)Cyc__genarr_273),(
void*)((struct _tuple6**)Cyc__genarr_273),(void*)((struct _tuple6**)Cyc__genarr_273
+ 2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_255;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_256;extern struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Enumdecl_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_257;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Core_Opt0List_list_t0Absyn_enumfield_t46H22_rep;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_120;extern struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_enumfield_t46H2_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_121;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_Enumfield_rep;static char _tmp2DF[10]="Enumfield";static struct
_tagged_arr Cyc__genname_932=(struct _tagged_arr){_tmp2DF,_tmp2DF,_tmp2DF + 10};
static char _tmp2E0[5]="name";static struct _tuple5 Cyc__gentuple_928={offsetof(
struct Cyc_Absyn_Enumfield,name),(struct _tagged_arr){_tmp2E0,_tmp2E0,_tmp2E0 + 5},(
void*)& Cyc__genrep_10};static char _tmp2E1[4]="tag";static struct _tuple5 Cyc__gentuple_929={
offsetof(struct Cyc_Absyn_Enumfield,tag),(struct _tagged_arr){_tmp2E1,_tmp2E1,
_tmp2E1 + 4},(void*)& Cyc__genrep_122};static char _tmp2E2[4]="loc";static struct
_tuple5 Cyc__gentuple_930={offsetof(struct Cyc_Absyn_Enumfield,loc),(struct
_tagged_arr){_tmp2E2,_tmp2E2,_tmp2E2 + 4},(void*)& Cyc__genrep_2};static struct
_tuple5*Cyc__genarr_931[3]={& Cyc__gentuple_928,& Cyc__gentuple_929,& Cyc__gentuple_930};
struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Enumfield_rep={3,(struct
_tagged_arr*)& Cyc__genname_932,sizeof(struct Cyc_Absyn_Enumfield),{(void*)((
struct _tuple5**)Cyc__genarr_931),(void*)((struct _tuple5**)Cyc__genarr_931),(void*)((
struct _tuple5**)Cyc__genarr_931 + 3)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_121={
1,1,(void*)((void*)& Cyc_struct_Absyn_Enumfield_rep)};static char _tmp2E5[5]="List";
static struct _tagged_arr Cyc__genname_936=(struct _tagged_arr){_tmp2E5,_tmp2E5,
_tmp2E5 + 5};static char _tmp2E6[3]="hd";static struct _tuple5 Cyc__gentuple_933={
offsetof(struct Cyc_List_List,hd),(struct _tagged_arr){_tmp2E6,_tmp2E6,_tmp2E6 + 3},(
void*)& Cyc__genrep_121};static char _tmp2E7[3]="tl";static struct _tuple5 Cyc__gentuple_934={
offsetof(struct Cyc_List_List,tl),(struct _tagged_arr){_tmp2E7,_tmp2E7,_tmp2E7 + 3},(
void*)& Cyc__genrep_120};static struct _tuple5*Cyc__genarr_935[2]={& Cyc__gentuple_933,&
Cyc__gentuple_934};struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_enumfield_t46H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_936,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_935),(void*)((struct _tuple5**)Cyc__genarr_935),(void*)((
struct _tuple5**)Cyc__genarr_935 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_120={
1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_enumfield_t46H2_rep)};static char
_tmp2EA[4]="Opt";static struct _tagged_arr Cyc__genname_260=(struct _tagged_arr){
_tmp2EA,_tmp2EA,_tmp2EA + 4};static char _tmp2EB[2]="v";static struct _tuple5 Cyc__gentuple_258={
offsetof(struct Cyc_Core_Opt,v),(struct _tagged_arr){_tmp2EB,_tmp2EB,_tmp2EB + 2},(
void*)& Cyc__genrep_120};static struct _tuple5*Cyc__genarr_259[1]={& Cyc__gentuple_258};
struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0List_list_t0Absyn_enumfield_t46H22_rep={
3,(struct _tagged_arr*)& Cyc__genname_260,sizeof(struct Cyc_Core_Opt),{(void*)((
struct _tuple5**)Cyc__genarr_259),(void*)((struct _tuple5**)Cyc__genarr_259),(void*)((
struct _tuple5**)Cyc__genarr_259 + 1)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_257={
1,1,(void*)((void*)& Cyc_struct_Core_Opt0List_list_t0Absyn_enumfield_t46H22_rep)};
static char _tmp2EE[9]="Enumdecl";static struct _tagged_arr Cyc__genname_265=(struct
_tagged_arr){_tmp2EE,_tmp2EE,_tmp2EE + 9};static char _tmp2EF[3]="sc";static struct
_tuple5 Cyc__gentuple_261={offsetof(struct Cyc_Absyn_Enumdecl,sc),(struct
_tagged_arr){_tmp2EF,_tmp2EF,_tmp2EF + 3},(void*)& Cyc_Absyn_scope_t_rep};static
char _tmp2F0[5]="name";static struct _tuple5 Cyc__gentuple_262={offsetof(struct Cyc_Absyn_Enumdecl,name),(
struct _tagged_arr){_tmp2F0,_tmp2F0,_tmp2F0 + 5},(void*)& Cyc__genrep_10};static
char _tmp2F1[7]="fields";static struct _tuple5 Cyc__gentuple_263={offsetof(struct Cyc_Absyn_Enumdecl,fields),(
struct _tagged_arr){_tmp2F1,_tmp2F1,_tmp2F1 + 7},(void*)& Cyc__genrep_257};static
struct _tuple5*Cyc__genarr_264[3]={& Cyc__gentuple_261,& Cyc__gentuple_262,& Cyc__gentuple_263};
struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Enumdecl_rep={3,(struct
_tagged_arr*)& Cyc__genname_265,sizeof(struct Cyc_Absyn_Enumdecl),{(void*)((struct
_tuple5**)Cyc__genarr_264),(void*)((struct _tuple5**)Cyc__genarr_264),(void*)((
struct _tuple5**)Cyc__genarr_264 + 3)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_256={
1,1,(void*)((void*)& Cyc_struct_Absyn_Enumdecl_rep)};struct _tuple51{unsigned int
f1;struct Cyc_Absyn_Enumdecl*f2;struct Cyc_Absyn_Enumfield*f3;};static struct
_tuple6 Cyc__gentuple_266={offsetof(struct _tuple51,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_267={offsetof(struct _tuple51,f2),(void*)& Cyc__genrep_256};
static struct _tuple6 Cyc__gentuple_268={offsetof(struct _tuple51,f3),(void*)& Cyc__genrep_121};
static struct _tuple6*Cyc__genarr_269[3]={& Cyc__gentuple_266,& Cyc__gentuple_267,&
Cyc__gentuple_268};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_255={4,
sizeof(struct _tuple51),{(void*)((struct _tuple6**)Cyc__genarr_269),(void*)((
struct _tuple6**)Cyc__genarr_269),(void*)((struct _tuple6**)Cyc__genarr_269 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_250;struct _tuple52{unsigned int
f1;void*f2;struct Cyc_Absyn_Enumfield*f3;};static struct _tuple6 Cyc__gentuple_251={
offsetof(struct _tuple52,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_252={
offsetof(struct _tuple52,f2),(void*)((void*)& Cyc_Absyn_type_t_rep)};static struct
_tuple6 Cyc__gentuple_253={offsetof(struct _tuple52,f3),(void*)& Cyc__genrep_121};
static struct _tuple6*Cyc__genarr_254[3]={& Cyc__gentuple_251,& Cyc__gentuple_252,&
Cyc__gentuple_253};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_250={4,
sizeof(struct _tuple52),{(void*)((struct _tuple6**)Cyc__genarr_254),(void*)((
struct _tuple6**)Cyc__genarr_254),(void*)((struct _tuple6**)Cyc__genarr_254 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_246;struct _tuple53{unsigned int
f1;struct _tuple0*f2;};static struct _tuple6 Cyc__gentuple_247={offsetof(struct
_tuple53,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_248={
offsetof(struct _tuple53,f2),(void*)& Cyc__genrep_10};static struct _tuple6*Cyc__genarr_249[
2]={& Cyc__gentuple_247,& Cyc__gentuple_248};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_246={4,sizeof(struct _tuple53),{(void*)((struct _tuple6**)Cyc__genarr_249),(
void*)((struct _tuple6**)Cyc__genarr_249),(void*)((struct _tuple6**)Cyc__genarr_249
+ 2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_236;struct _tuple54{
unsigned int f1;struct _tuple0*f2;struct Cyc_List_List*f3;};static struct _tuple6 Cyc__gentuple_242={
offsetof(struct _tuple54,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_243={
offsetof(struct _tuple54,f2),(void*)& Cyc__genrep_10};static struct _tuple6 Cyc__gentuple_244={
offsetof(struct _tuple54,f3),(void*)& Cyc__genrep_237};static struct _tuple6*Cyc__genarr_245[
3]={& Cyc__gentuple_242,& Cyc__gentuple_243,& Cyc__gentuple_244};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_236={4,sizeof(struct _tuple54),{(void*)((struct _tuple6**)Cyc__genarr_245),(
void*)((struct _tuple6**)Cyc__genarr_245),(void*)((struct _tuple6**)Cyc__genarr_245
+ 3)}};static char _tmp2F8[7]="Wild_p";static struct _tuple7 Cyc__gentuple_233={0,(
struct _tagged_arr){_tmp2F8,_tmp2F8,_tmp2F8 + 7}};static char _tmp2F9[7]="Null_p";
static struct _tuple7 Cyc__gentuple_234={1,(struct _tagged_arr){_tmp2F9,_tmp2F9,
_tmp2F9 + 7}};static struct _tuple7*Cyc__genarr_235[2]={& Cyc__gentuple_233,& Cyc__gentuple_234};
static char _tmp2FA[6]="Var_p";static struct _tuple5 Cyc__gentuple_435={0,(struct
_tagged_arr){_tmp2FA,_tmp2FA,_tmp2FA + 6},(void*)& Cyc__genrep_431};static char
_tmp2FB[12]="Reference_p";static struct _tuple5 Cyc__gentuple_436={1,(struct
_tagged_arr){_tmp2FB,_tmp2FB,_tmp2FB + 12},(void*)& Cyc__genrep_431};static char
_tmp2FC[9]="TagInt_p";static struct _tuple5 Cyc__gentuple_437={2,(struct _tagged_arr){
_tmp2FC,_tmp2FC,_tmp2FC + 9},(void*)& Cyc__genrep_426};static char _tmp2FD[8]="Tuple_p";
static struct _tuple5 Cyc__gentuple_438={3,(struct _tagged_arr){_tmp2FD,_tmp2FD,
_tmp2FD + 8},(void*)& Cyc__genrep_422};static char _tmp2FE[10]="Pointer_p";static
struct _tuple5 Cyc__gentuple_439={4,(struct _tagged_arr){_tmp2FE,_tmp2FE,_tmp2FE + 
10},(void*)& Cyc__genrep_418};static char _tmp2FF[7]="Aggr_p";static struct _tuple5
Cyc__gentuple_440={5,(struct _tagged_arr){_tmp2FF,_tmp2FF,_tmp2FF + 7},(void*)& Cyc__genrep_329};
static char _tmp300[9]="Tunion_p";static struct _tuple5 Cyc__gentuple_441={6,(struct
_tagged_arr){_tmp300,_tmp300,_tmp300 + 9},(void*)& Cyc__genrep_284};static char
_tmp301[6]="Int_p";static struct _tuple5 Cyc__gentuple_442={7,(struct _tagged_arr){
_tmp301,_tmp301,_tmp301 + 6},(void*)& Cyc__genrep_274};static char _tmp302[7]="Char_p";
static struct _tuple5 Cyc__gentuple_443={8,(struct _tagged_arr){_tmp302,_tmp302,
_tmp302 + 7},(void*)& Cyc__genrep_270};static char _tmp303[8]="Float_p";static struct
_tuple5 Cyc__gentuple_444={9,(struct _tagged_arr){_tmp303,_tmp303,_tmp303 + 8},(
void*)& Cyc__genrep_62};static char _tmp304[7]="Enum_p";static struct _tuple5 Cyc__gentuple_445={
10,(struct _tagged_arr){_tmp304,_tmp304,_tmp304 + 7},(void*)& Cyc__genrep_255};
static char _tmp305[11]="AnonEnum_p";static struct _tuple5 Cyc__gentuple_446={11,(
struct _tagged_arr){_tmp305,_tmp305,_tmp305 + 11},(void*)& Cyc__genrep_250};static
char _tmp306[12]="UnknownId_p";static struct _tuple5 Cyc__gentuple_447={12,(struct
_tagged_arr){_tmp306,_tmp306,_tmp306 + 12},(void*)& Cyc__genrep_246};static char
_tmp307[14]="UnknownCall_p";static struct _tuple5 Cyc__gentuple_448={13,(struct
_tagged_arr){_tmp307,_tmp307,_tmp307 + 14},(void*)& Cyc__genrep_236};static struct
_tuple5*Cyc__genarr_449[14]={& Cyc__gentuple_435,& Cyc__gentuple_436,& Cyc__gentuple_437,&
Cyc__gentuple_438,& Cyc__gentuple_439,& Cyc__gentuple_440,& Cyc__gentuple_441,& Cyc__gentuple_442,&
Cyc__gentuple_443,& Cyc__gentuple_444,& Cyc__gentuple_445,& Cyc__gentuple_446,& Cyc__gentuple_447,&
Cyc__gentuple_448};static char _tmp309[8]="Raw_pat";struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_raw_pat_t_rep={5,(struct _tagged_arr){_tmp309,_tmp309,_tmp309 + 8},{(
void*)((struct _tuple7**)Cyc__genarr_235),(void*)((struct _tuple7**)Cyc__genarr_235),(
void*)((struct _tuple7**)Cyc__genarr_235 + 2)},{(void*)((struct _tuple5**)Cyc__genarr_449),(
void*)((struct _tuple5**)Cyc__genarr_449),(void*)((struct _tuple5**)Cyc__genarr_449
+ 14)}};static char _tmp30A[4]="Pat";static struct _tagged_arr Cyc__genname_454=(
struct _tagged_arr){_tmp30A,_tmp30A,_tmp30A + 4};static char _tmp30B[2]="r";static
struct _tuple5 Cyc__gentuple_450={offsetof(struct Cyc_Absyn_Pat,r),(struct
_tagged_arr){_tmp30B,_tmp30B,_tmp30B + 2},(void*)& Cyc_Absyn_raw_pat_t_rep};static
char _tmp30C[5]="topt";static struct _tuple5 Cyc__gentuple_451={offsetof(struct Cyc_Absyn_Pat,topt),(
struct _tagged_arr){_tmp30C,_tmp30C,_tmp30C + 5},(void*)& Cyc__genrep_92};static
char _tmp30D[4]="loc";static struct _tuple5 Cyc__gentuple_452={offsetof(struct Cyc_Absyn_Pat,loc),(
struct _tagged_arr){_tmp30D,_tmp30D,_tmp30D + 4},(void*)& Cyc__genrep_2};static
struct _tuple5*Cyc__genarr_453[3]={& Cyc__gentuple_450,& Cyc__gentuple_451,& Cyc__gentuple_452};
struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Pat_rep={3,(struct _tagged_arr*)&
Cyc__genname_454,sizeof(struct Cyc_Absyn_Pat),{(void*)((struct _tuple5**)Cyc__genarr_453),(
void*)((struct _tuple5**)Cyc__genarr_453),(void*)((struct _tuple5**)Cyc__genarr_453
+ 3)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_232={1,1,(void*)((void*)&
Cyc_struct_Absyn_Pat_rep)};extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_132;
extern struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0List_list_t0Absyn_vardecl_t46H22_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_133;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List0Absyn_vardecl_t46H2_rep;static char _tmp310[5]="List";static
struct _tagged_arr Cyc__genname_159=(struct _tagged_arr){_tmp310,_tmp310,_tmp310 + 5};
static char _tmp311[3]="hd";static struct _tuple5 Cyc__gentuple_156={offsetof(struct
Cyc_List_List,hd),(struct _tagged_arr){_tmp311,_tmp311,_tmp311 + 3},(void*)& Cyc__genrep_134};
static char _tmp312[3]="tl";static struct _tuple5 Cyc__gentuple_157={offsetof(struct
Cyc_List_List,tl),(struct _tagged_arr){_tmp312,_tmp312,_tmp312 + 3},(void*)& Cyc__genrep_133};
static struct _tuple5*Cyc__genarr_158[2]={& Cyc__gentuple_156,& Cyc__gentuple_157};
struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_vardecl_t46H2_rep={3,(
struct _tagged_arr*)& Cyc__genname_159,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_158),(void*)((struct _tuple5**)Cyc__genarr_158),(void*)((
struct _tuple5**)Cyc__genarr_158 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_133={
1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_vardecl_t46H2_rep)};static char
_tmp315[4]="Opt";static struct _tagged_arr Cyc__genname_162=(struct _tagged_arr){
_tmp315,_tmp315,_tmp315 + 4};static char _tmp316[2]="v";static struct _tuple5 Cyc__gentuple_160={
offsetof(struct Cyc_Core_Opt,v),(struct _tagged_arr){_tmp316,_tmp316,_tmp316 + 2},(
void*)& Cyc__genrep_133};static struct _tuple5*Cyc__genarr_161[1]={& Cyc__gentuple_160};
struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0List_list_t0Absyn_vardecl_t46H22_rep={
3,(struct _tagged_arr*)& Cyc__genname_162,sizeof(struct Cyc_Core_Opt),{(void*)((
struct _tuple5**)Cyc__genarr_161),(void*)((struct _tuple5**)Cyc__genarr_161),(void*)((
struct _tuple5**)Cyc__genarr_161 + 1)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_132={
1,1,(void*)((void*)& Cyc_struct_Core_Opt0List_list_t0Absyn_vardecl_t46H22_rep)};
static char _tmp319[14]="Switch_clause";static struct _tagged_arr Cyc__genname_461=(
struct _tagged_arr){_tmp319,_tmp319,_tmp319 + 14};static char _tmp31A[8]="pattern";
static struct _tuple5 Cyc__gentuple_455={offsetof(struct Cyc_Absyn_Switch_clause,pattern),(
struct _tagged_arr){_tmp31A,_tmp31A,_tmp31A + 8},(void*)& Cyc__genrep_232};static
char _tmp31B[9]="pat_vars";static struct _tuple5 Cyc__gentuple_456={offsetof(struct
Cyc_Absyn_Switch_clause,pat_vars),(struct _tagged_arr){_tmp31B,_tmp31B,_tmp31B + 9},(
void*)& Cyc__genrep_132};static char _tmp31C[13]="where_clause";static struct _tuple5
Cyc__gentuple_457={offsetof(struct Cyc_Absyn_Switch_clause,where_clause),(struct
_tagged_arr){_tmp31C,_tmp31C,_tmp31C + 13},(void*)& Cyc__genrep_122};static char
_tmp31D[5]="body";static struct _tuple5 Cyc__gentuple_458={offsetof(struct Cyc_Absyn_Switch_clause,body),(
struct _tagged_arr){_tmp31D,_tmp31D,_tmp31D + 5},(void*)& Cyc__genrep_163};static
char _tmp31E[4]="loc";static struct _tuple5 Cyc__gentuple_459={offsetof(struct Cyc_Absyn_Switch_clause,loc),(
struct _tagged_arr){_tmp31E,_tmp31E,_tmp31E + 4},(void*)& Cyc__genrep_2};static
struct _tuple5*Cyc__genarr_460[5]={& Cyc__gentuple_455,& Cyc__gentuple_456,& Cyc__gentuple_457,&
Cyc__gentuple_458,& Cyc__gentuple_459};struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Switch_clause_rep={
3,(struct _tagged_arr*)& Cyc__genname_461,sizeof(struct Cyc_Absyn_Switch_clause),{(
void*)((struct _tuple5**)Cyc__genarr_460),(void*)((struct _tuple5**)Cyc__genarr_460),(
void*)((struct _tuple5**)Cyc__genarr_460 + 5)}};static struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_231={1,1,(void*)((void*)& Cyc_struct_Absyn_Switch_clause_rep)};static
char _tmp321[5]="List";static struct _tagged_arr Cyc__genname_465=(struct _tagged_arr){
_tmp321,_tmp321,_tmp321 + 5};static char _tmp322[3]="hd";static struct _tuple5 Cyc__gentuple_462={
offsetof(struct Cyc_List_List,hd),(struct _tagged_arr){_tmp322,_tmp322,_tmp322 + 3},(
void*)((void*)& Cyc__genrep_231)};static char _tmp323[3]="tl";static struct _tuple5
Cyc__gentuple_463={offsetof(struct Cyc_List_List,tl),(struct _tagged_arr){_tmp323,
_tmp323,_tmp323 + 3},(void*)& Cyc__genrep_230};static struct _tuple5*Cyc__genarr_464[
2]={& Cyc__gentuple_462,& Cyc__gentuple_463};struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_switch_clause_t46H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_465,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_464),(void*)((struct _tuple5**)Cyc__genarr_464),(void*)((
struct _tuple5**)Cyc__genarr_464 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_230={
1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_switch_clause_t46H2_rep)};struct
_tuple55{unsigned int f1;struct Cyc_Absyn_Exp*f2;struct Cyc_List_List*f3;};static
struct _tuple6 Cyc__gentuple_517={offsetof(struct _tuple55,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_518={offsetof(struct _tuple55,f2),(void*)& Cyc__genrep_126};
static struct _tuple6 Cyc__gentuple_519={offsetof(struct _tuple55,f3),(void*)& Cyc__genrep_230};
static struct _tuple6*Cyc__genarr_520[3]={& Cyc__gentuple_517,& Cyc__gentuple_518,&
Cyc__gentuple_519};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_516={4,
sizeof(struct _tuple55),{(void*)((struct _tuple6**)Cyc__genarr_520),(void*)((
struct _tuple6**)Cyc__genarr_520),(void*)((struct _tuple6**)Cyc__genarr_520 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_500;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_501;extern struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_switchC_clause_t46H2_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_502;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_SwitchC_clause_rep;static char _tmp327[15]="SwitchC_clause";
static struct _tagged_arr Cyc__genname_507=(struct _tagged_arr){_tmp327,_tmp327,
_tmp327 + 15};static char _tmp328[9]="cnst_exp";static struct _tuple5 Cyc__gentuple_503={
offsetof(struct Cyc_Absyn_SwitchC_clause,cnst_exp),(struct _tagged_arr){_tmp328,
_tmp328,_tmp328 + 9},(void*)& Cyc__genrep_122};static char _tmp329[5]="body";static
struct _tuple5 Cyc__gentuple_504={offsetof(struct Cyc_Absyn_SwitchC_clause,body),(
struct _tagged_arr){_tmp329,_tmp329,_tmp329 + 5},(void*)& Cyc__genrep_163};static
char _tmp32A[4]="loc";static struct _tuple5 Cyc__gentuple_505={offsetof(struct Cyc_Absyn_SwitchC_clause,loc),(
struct _tagged_arr){_tmp32A,_tmp32A,_tmp32A + 4},(void*)& Cyc__genrep_2};static
struct _tuple5*Cyc__genarr_506[3]={& Cyc__gentuple_503,& Cyc__gentuple_504,& Cyc__gentuple_505};
struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_SwitchC_clause_rep={3,(struct
_tagged_arr*)& Cyc__genname_507,sizeof(struct Cyc_Absyn_SwitchC_clause),{(void*)((
struct _tuple5**)Cyc__genarr_506),(void*)((struct _tuple5**)Cyc__genarr_506),(void*)((
struct _tuple5**)Cyc__genarr_506 + 3)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_502={
1,1,(void*)((void*)& Cyc_struct_Absyn_SwitchC_clause_rep)};static char _tmp32D[5]="List";
static struct _tagged_arr Cyc__genname_511=(struct _tagged_arr){_tmp32D,_tmp32D,
_tmp32D + 5};static char _tmp32E[3]="hd";static struct _tuple5 Cyc__gentuple_508={
offsetof(struct Cyc_List_List,hd),(struct _tagged_arr){_tmp32E,_tmp32E,_tmp32E + 3},(
void*)& Cyc__genrep_502};static char _tmp32F[3]="tl";static struct _tuple5 Cyc__gentuple_509={
offsetof(struct Cyc_List_List,tl),(struct _tagged_arr){_tmp32F,_tmp32F,_tmp32F + 3},(
void*)& Cyc__genrep_501};static struct _tuple5*Cyc__genarr_510[2]={& Cyc__gentuple_508,&
Cyc__gentuple_509};struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_switchC_clause_t46H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_511,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_510),(void*)((struct _tuple5**)Cyc__genarr_510),(void*)((
struct _tuple5**)Cyc__genarr_510 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_501={
1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_switchC_clause_t46H2_rep)};static
struct _tuple6 Cyc__gentuple_512={offsetof(struct _tuple55,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_513={offsetof(struct _tuple55,f2),(void*)& Cyc__genrep_126};
static struct _tuple6 Cyc__gentuple_514={offsetof(struct _tuple55,f3),(void*)& Cyc__genrep_501};
static struct _tuple6*Cyc__genarr_515[3]={& Cyc__gentuple_512,& Cyc__gentuple_513,&
Cyc__gentuple_514};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_500={4,
sizeof(struct _tuple55),{(void*)((struct _tuple6**)Cyc__genarr_515),(void*)((
struct _tuple6**)Cyc__genarr_515),(void*)((struct _tuple6**)Cyc__genarr_515 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_489;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_491;extern struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_exp_t46H2_rep;
static char _tmp333[5]="List";static struct _tagged_arr Cyc__genname_495=(struct
_tagged_arr){_tmp333,_tmp333,_tmp333 + 5};static char _tmp334[3]="hd";static struct
_tuple5 Cyc__gentuple_492={offsetof(struct Cyc_List_List,hd),(struct _tagged_arr){
_tmp334,_tmp334,_tmp334 + 3},(void*)& Cyc__genrep_126};static char _tmp335[3]="tl";
static struct _tuple5 Cyc__gentuple_493={offsetof(struct Cyc_List_List,tl),(struct
_tagged_arr){_tmp335,_tmp335,_tmp335 + 3},(void*)& Cyc__genrep_491};static struct
_tuple5*Cyc__genarr_494[2]={& Cyc__gentuple_492,& Cyc__gentuple_493};struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List0Absyn_exp_t46H2_rep={3,(struct _tagged_arr*)& Cyc__genname_495,
sizeof(struct Cyc_List_List),{(void*)((struct _tuple5**)Cyc__genarr_494),(void*)((
struct _tuple5**)Cyc__genarr_494),(void*)((struct _tuple5**)Cyc__genarr_494 + 2)}};
static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_491={1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_exp_t46H2_rep)};
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_490;static struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_490={1,1,(void*)((void*)& Cyc__genrep_231)};struct _tuple56{
unsigned int f1;struct Cyc_List_List*f2;struct Cyc_Absyn_Switch_clause**f3;};static
struct _tuple6 Cyc__gentuple_496={offsetof(struct _tuple56,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_497={offsetof(struct _tuple56,f2),(void*)& Cyc__genrep_491};
static struct _tuple6 Cyc__gentuple_498={offsetof(struct _tuple56,f3),(void*)& Cyc__genrep_490};
static struct _tuple6*Cyc__genarr_499[3]={& Cyc__gentuple_496,& Cyc__gentuple_497,&
Cyc__gentuple_498};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_489={4,
sizeof(struct _tuple56),{(void*)((struct _tuple6**)Cyc__genarr_499),(void*)((
struct _tuple6**)Cyc__genarr_499),(void*)((struct _tuple6**)Cyc__genarr_499 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_484;struct _tuple57{unsigned int
f1;struct Cyc_Absyn_Decl*f2;struct Cyc_Absyn_Stmt*f3;};static struct _tuple6 Cyc__gentuple_485={
offsetof(struct _tuple57,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_486={
offsetof(struct _tuple57,f2),(void*)& Cyc__genrep_1};static struct _tuple6 Cyc__gentuple_487={
offsetof(struct _tuple57,f3),(void*)& Cyc__genrep_163};static struct _tuple6*Cyc__genarr_488[
3]={& Cyc__gentuple_485,& Cyc__gentuple_486,& Cyc__gentuple_487};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_484={4,sizeof(struct _tuple57),{(void*)((struct _tuple6**)Cyc__genarr_488),(
void*)((struct _tuple6**)Cyc__genarr_488),(void*)((struct _tuple6**)Cyc__genarr_488
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_480;static struct _tuple6
Cyc__gentuple_481={offsetof(struct _tuple38,f1),(void*)& Cyc__genrep_5};static
struct _tuple6 Cyc__gentuple_482={offsetof(struct _tuple38,f2),(void*)& Cyc__genrep_163};
static struct _tuple6*Cyc__genarr_483[2]={& Cyc__gentuple_481,& Cyc__gentuple_482};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_480={4,sizeof(struct _tuple38),{(
void*)((struct _tuple6**)Cyc__genarr_483),(void*)((struct _tuple6**)Cyc__genarr_483),(
void*)((struct _tuple6**)Cyc__genarr_483 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_475;static struct _tuple6 Cyc__gentuple_476={offsetof(struct _tuple39,f1),(
void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_477={offsetof(struct
_tuple39,f2),(void*)& Cyc__genrep_12};static struct _tuple6 Cyc__gentuple_478={
offsetof(struct _tuple39,f3),(void*)& Cyc__genrep_163};static struct _tuple6*Cyc__genarr_479[
3]={& Cyc__gentuple_476,& Cyc__gentuple_477,& Cyc__gentuple_478};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_475={4,sizeof(struct _tuple39),{(void*)((struct _tuple6**)Cyc__genarr_479),(
void*)((struct _tuple6**)Cyc__genarr_479),(void*)((struct _tuple6**)Cyc__genarr_479
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_470;struct _tuple58{
unsigned int f1;struct Cyc_Absyn_Stmt*f2;struct _tuple2 f3;};static struct _tuple6 Cyc__gentuple_471={
offsetof(struct _tuple58,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_472={
offsetof(struct _tuple58,f2),(void*)& Cyc__genrep_163};static struct _tuple6 Cyc__gentuple_473={
offsetof(struct _tuple58,f3),(void*)& Cyc__genrep_172};static struct _tuple6*Cyc__genarr_474[
3]={& Cyc__gentuple_471,& Cyc__gentuple_472,& Cyc__gentuple_473};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_470={4,sizeof(struct _tuple58),{(void*)((struct _tuple6**)Cyc__genarr_474),(
void*)((struct _tuple6**)Cyc__genarr_474),(void*)((struct _tuple6**)Cyc__genarr_474
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_229;struct _tuple59{
unsigned int f1;struct Cyc_Absyn_Stmt*f2;struct Cyc_List_List*f3;};static struct
_tuple6 Cyc__gentuple_466={offsetof(struct _tuple59,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_467={offsetof(struct _tuple59,f2),(void*)& Cyc__genrep_163};
static struct _tuple6 Cyc__gentuple_468={offsetof(struct _tuple59,f3),(void*)& Cyc__genrep_230};
static struct _tuple6*Cyc__genarr_469[3]={& Cyc__gentuple_466,& Cyc__gentuple_467,&
Cyc__gentuple_468};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_229={4,
sizeof(struct _tuple59),{(void*)((struct _tuple6**)Cyc__genarr_469),(void*)((
struct _tuple6**)Cyc__genarr_469),(void*)((struct _tuple6**)Cyc__genarr_469 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_185;struct _tuple60{unsigned int
f1;struct Cyc_Absyn_Tvar*f2;struct Cyc_Absyn_Vardecl*f3;int f4;struct Cyc_Absyn_Stmt*
f5;};static struct _tuple6 Cyc__gentuple_223={offsetof(struct _tuple60,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_224={offsetof(struct _tuple60,f2),(
void*)& Cyc__genrep_186};static struct _tuple6 Cyc__gentuple_225={offsetof(struct
_tuple60,f3),(void*)& Cyc__genrep_134};static struct _tuple6 Cyc__gentuple_226={
offsetof(struct _tuple60,f4),(void*)((void*)& Cyc__genrep_67)};static struct _tuple6
Cyc__gentuple_227={offsetof(struct _tuple60,f5),(void*)& Cyc__genrep_163};static
struct _tuple6*Cyc__genarr_228[5]={& Cyc__gentuple_223,& Cyc__gentuple_224,& Cyc__gentuple_225,&
Cyc__gentuple_226,& Cyc__gentuple_227};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_185={
4,sizeof(struct _tuple60),{(void*)((struct _tuple6**)Cyc__genarr_228),(void*)((
struct _tuple6**)Cyc__genarr_228),(void*)((struct _tuple6**)Cyc__genarr_228 + 5)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_171;extern struct Cyc_Typerep_Struct_struct
Cyc_Absyn_forarray_info_t_rep;static char _tmp340[13]="ForArrayInfo";static struct
_tagged_arr Cyc__genname_181=(struct _tagged_arr){_tmp340,_tmp340,_tmp340 + 13};
static char _tmp341[6]="defns";static struct _tuple5 Cyc__gentuple_176={offsetof(
struct Cyc_Absyn_ForArrayInfo,defns),(struct _tagged_arr){_tmp341,_tmp341,_tmp341 + 
6},(void*)& Cyc__genrep_133};static char _tmp342[10]="condition";static struct
_tuple5 Cyc__gentuple_177={offsetof(struct Cyc_Absyn_ForArrayInfo,condition),(
struct _tagged_arr){_tmp342,_tmp342,_tmp342 + 10},(void*)& Cyc__genrep_172};static
char _tmp343[6]="delta";static struct _tuple5 Cyc__gentuple_178={offsetof(struct Cyc_Absyn_ForArrayInfo,delta),(
struct _tagged_arr){_tmp343,_tmp343,_tmp343 + 6},(void*)& Cyc__genrep_172};static
char _tmp344[5]="body";static struct _tuple5 Cyc__gentuple_179={offsetof(struct Cyc_Absyn_ForArrayInfo,body),(
struct _tagged_arr){_tmp344,_tmp344,_tmp344 + 5},(void*)& Cyc__genrep_163};static
struct _tuple5*Cyc__genarr_180[4]={& Cyc__gentuple_176,& Cyc__gentuple_177,& Cyc__gentuple_178,&
Cyc__gentuple_179};struct Cyc_Typerep_Struct_struct Cyc_Absyn_forarray_info_t_rep={
3,(struct _tagged_arr*)& Cyc__genname_181,sizeof(struct Cyc_Absyn_ForArrayInfo),{(
void*)((struct _tuple5**)Cyc__genarr_180),(void*)((struct _tuple5**)Cyc__genarr_180),(
void*)((struct _tuple5**)Cyc__genarr_180 + 4)}};struct _tuple61{unsigned int f1;
struct Cyc_Absyn_ForArrayInfo f2;};static struct _tuple6 Cyc__gentuple_182={offsetof(
struct _tuple61,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_183={
offsetof(struct _tuple61,f2),(void*)& Cyc_Absyn_forarray_info_t_rep};static struct
_tuple6*Cyc__genarr_184[2]={& Cyc__gentuple_182,& Cyc__gentuple_183};static struct
Cyc_Typerep_Tuple_struct Cyc__genrep_171={4,sizeof(struct _tuple61),{(void*)((
struct _tuple6**)Cyc__genarr_184),(void*)((struct _tuple6**)Cyc__genarr_184),(void*)((
struct _tuple6**)Cyc__genarr_184 + 2)}};static char _tmp347[7]="Skip_s";static struct
_tuple7 Cyc__gentuple_169={0,(struct _tagged_arr){_tmp347,_tmp347,_tmp347 + 7}};
static struct _tuple7*Cyc__genarr_170[1]={& Cyc__gentuple_169};static char _tmp348[6]="Exp_s";
static struct _tuple5 Cyc__gentuple_558={0,(struct _tagged_arr){_tmp348,_tmp348,
_tmp348 + 6},(void*)& Cyc__genrep_125};static char _tmp349[6]="Seq_s";static struct
_tuple5 Cyc__gentuple_559={1,(struct _tagged_arr){_tmp349,_tmp349,_tmp349 + 6},(
void*)& Cyc__genrep_553};static char _tmp34A[9]="Return_s";static struct _tuple5 Cyc__gentuple_560={
2,(struct _tagged_arr){_tmp34A,_tmp34A,_tmp34A + 9},(void*)& Cyc__genrep_549};
static char _tmp34B[13]="IfThenElse_s";static struct _tuple5 Cyc__gentuple_561={3,(
struct _tagged_arr){_tmp34B,_tmp34B,_tmp34B + 13},(void*)& Cyc__genrep_543};static
char _tmp34C[8]="While_s";static struct _tuple5 Cyc__gentuple_562={4,(struct
_tagged_arr){_tmp34C,_tmp34C,_tmp34C + 8},(void*)& Cyc__genrep_538};static char
_tmp34D[8]="Break_s";static struct _tuple5 Cyc__gentuple_563={5,(struct _tagged_arr){
_tmp34D,_tmp34D,_tmp34D + 8},(void*)& Cyc__genrep_534};static char _tmp34E[11]="Continue_s";
static struct _tuple5 Cyc__gentuple_564={6,(struct _tagged_arr){_tmp34E,_tmp34E,
_tmp34E + 11},(void*)& Cyc__genrep_534};static char _tmp34F[7]="Goto_s";static struct
_tuple5 Cyc__gentuple_565={7,(struct _tagged_arr){_tmp34F,_tmp34F,_tmp34F + 7},(
void*)& Cyc__genrep_528};static char _tmp350[6]="For_s";static struct _tuple5 Cyc__gentuple_566={
8,(struct _tagged_arr){_tmp350,_tmp350,_tmp350 + 6},(void*)& Cyc__genrep_521};
static char _tmp351[9]="Switch_s";static struct _tuple5 Cyc__gentuple_567={9,(struct
_tagged_arr){_tmp351,_tmp351,_tmp351 + 9},(void*)& Cyc__genrep_516};static char
_tmp352[10]="SwitchC_s";static struct _tuple5 Cyc__gentuple_568={10,(struct
_tagged_arr){_tmp352,_tmp352,_tmp352 + 10},(void*)& Cyc__genrep_500};static char
_tmp353[11]="Fallthru_s";static struct _tuple5 Cyc__gentuple_569={11,(struct
_tagged_arr){_tmp353,_tmp353,_tmp353 + 11},(void*)& Cyc__genrep_489};static char
_tmp354[7]="Decl_s";static struct _tuple5 Cyc__gentuple_570={12,(struct _tagged_arr){
_tmp354,_tmp354,_tmp354 + 7},(void*)& Cyc__genrep_484};static char _tmp355[6]="Cut_s";
static struct _tuple5 Cyc__gentuple_571={13,(struct _tagged_arr){_tmp355,_tmp355,
_tmp355 + 6},(void*)& Cyc__genrep_480};static char _tmp356[9]="Splice_s";static
struct _tuple5 Cyc__gentuple_572={14,(struct _tagged_arr){_tmp356,_tmp356,_tmp356 + 
9},(void*)& Cyc__genrep_480};static char _tmp357[8]="Label_s";static struct _tuple5
Cyc__gentuple_573={15,(struct _tagged_arr){_tmp357,_tmp357,_tmp357 + 8},(void*)&
Cyc__genrep_475};static char _tmp358[5]="Do_s";static struct _tuple5 Cyc__gentuple_574={
16,(struct _tagged_arr){_tmp358,_tmp358,_tmp358 + 5},(void*)& Cyc__genrep_470};
static char _tmp359[11]="TryCatch_s";static struct _tuple5 Cyc__gentuple_575={17,(
struct _tagged_arr){_tmp359,_tmp359,_tmp359 + 11},(void*)& Cyc__genrep_229};static
char _tmp35A[9]="Region_s";static struct _tuple5 Cyc__gentuple_576={18,(struct
_tagged_arr){_tmp35A,_tmp35A,_tmp35A + 9},(void*)& Cyc__genrep_185};static char
_tmp35B[11]="ForArray_s";static struct _tuple5 Cyc__gentuple_577={19,(struct
_tagged_arr){_tmp35B,_tmp35B,_tmp35B + 11},(void*)& Cyc__genrep_171};static char
_tmp35C[14]="ResetRegion_s";static struct _tuple5 Cyc__gentuple_578={20,(struct
_tagged_arr){_tmp35C,_tmp35C,_tmp35C + 14},(void*)& Cyc__genrep_125};static struct
_tuple5*Cyc__genarr_579[21]={& Cyc__gentuple_558,& Cyc__gentuple_559,& Cyc__gentuple_560,&
Cyc__gentuple_561,& Cyc__gentuple_562,& Cyc__gentuple_563,& Cyc__gentuple_564,& Cyc__gentuple_565,&
Cyc__gentuple_566,& Cyc__gentuple_567,& Cyc__gentuple_568,& Cyc__gentuple_569,& Cyc__gentuple_570,&
Cyc__gentuple_571,& Cyc__gentuple_572,& Cyc__gentuple_573,& Cyc__gentuple_574,& Cyc__gentuple_575,&
Cyc__gentuple_576,& Cyc__gentuple_577,& Cyc__gentuple_578};static char _tmp35E[9]="Raw_stmt";
struct Cyc_Typerep_TUnion_struct Cyc_Absyn_raw_stmt_t_rep={5,(struct _tagged_arr){
_tmp35E,_tmp35E,_tmp35E + 9},{(void*)((struct _tuple7**)Cyc__genarr_170),(void*)((
struct _tuple7**)Cyc__genarr_170),(void*)((struct _tuple7**)Cyc__genarr_170 + 1)},{(
void*)((struct _tuple5**)Cyc__genarr_579),(void*)((struct _tuple5**)Cyc__genarr_579),(
void*)((struct _tuple5**)Cyc__genarr_579 + 21)}};extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_164;extern struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_stmt_t46H2_rep;
static char _tmp35F[5]="List";static struct _tagged_arr Cyc__genname_168=(struct
_tagged_arr){_tmp35F,_tmp35F,_tmp35F + 5};static char _tmp360[3]="hd";static struct
_tuple5 Cyc__gentuple_165={offsetof(struct Cyc_List_List,hd),(struct _tagged_arr){
_tmp360,_tmp360,_tmp360 + 3},(void*)& Cyc__genrep_163};static char _tmp361[3]="tl";
static struct _tuple5 Cyc__gentuple_166={offsetof(struct Cyc_List_List,tl),(struct
_tagged_arr){_tmp361,_tmp361,_tmp361 + 3},(void*)& Cyc__genrep_164};static struct
_tuple5*Cyc__genarr_167[2]={& Cyc__gentuple_165,& Cyc__gentuple_166};struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List0Absyn_stmt_t46H2_rep={3,(struct _tagged_arr*)& Cyc__genname_168,
sizeof(struct Cyc_List_List),{(void*)((struct _tuple5**)Cyc__genarr_167),(void*)((
struct _tuple5**)Cyc__genarr_167),(void*)((struct _tuple5**)Cyc__genarr_167 + 2)}};
static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_164={1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_stmt_t46H2_rep)};
extern struct Cyc_Typerep_XTUnion_struct Cyc_Absyn_absyn_annot_t_rep;static struct
_tuple8*Cyc__genarr_123[0]={};static char _tmp365[11]="AbsynAnnot";struct Cyc_Typerep_XTUnion_struct
Cyc_Absyn_absyn_annot_t_rep={7,(struct _tagged_arr){_tmp365,_tmp365,_tmp365 + 11},{(
void*)((struct _tuple8**)Cyc__genarr_123),(void*)((struct _tuple8**)Cyc__genarr_123),(
void*)((struct _tuple8**)Cyc__genarr_123 + 0)}};static char _tmp366[5]="Stmt";static
struct _tagged_arr Cyc__genname_586=(struct _tagged_arr){_tmp366,_tmp366,_tmp366 + 5};
static char _tmp367[2]="r";static struct _tuple5 Cyc__gentuple_580={offsetof(struct
Cyc_Absyn_Stmt,r),(struct _tagged_arr){_tmp367,_tmp367,_tmp367 + 2},(void*)& Cyc_Absyn_raw_stmt_t_rep};
static char _tmp368[4]="loc";static struct _tuple5 Cyc__gentuple_581={offsetof(struct
Cyc_Absyn_Stmt,loc),(struct _tagged_arr){_tmp368,_tmp368,_tmp368 + 4},(void*)& Cyc__genrep_2};
static char _tmp369[16]="non_local_preds";static struct _tuple5 Cyc__gentuple_582={
offsetof(struct Cyc_Absyn_Stmt,non_local_preds),(struct _tagged_arr){_tmp369,
_tmp369,_tmp369 + 16},(void*)& Cyc__genrep_164};static char _tmp36A[10]="try_depth";
static struct _tuple5 Cyc__gentuple_583={offsetof(struct Cyc_Absyn_Stmt,try_depth),(
struct _tagged_arr){_tmp36A,_tmp36A,_tmp36A + 10},(void*)((void*)& Cyc__genrep_67)};
static char _tmp36B[6]="annot";static struct _tuple5 Cyc__gentuple_584={offsetof(
struct Cyc_Absyn_Stmt,annot),(struct _tagged_arr){_tmp36B,_tmp36B,_tmp36B + 6},(
void*)& Cyc_Absyn_absyn_annot_t_rep};static struct _tuple5*Cyc__genarr_585[5]={& Cyc__gentuple_580,&
Cyc__gentuple_581,& Cyc__gentuple_582,& Cyc__gentuple_583,& Cyc__gentuple_584};
struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Stmt_rep={3,(struct _tagged_arr*)&
Cyc__genname_586,sizeof(struct Cyc_Absyn_Stmt),{(void*)((struct _tuple5**)Cyc__genarr_585),(
void*)((struct _tuple5**)Cyc__genarr_585),(void*)((struct _tuple5**)Cyc__genarr_585
+ 5)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_163={1,1,(void*)((void*)&
Cyc_struct_Absyn_Stmt_rep)};static char _tmp36E[7]="Fndecl";static struct
_tagged_arr Cyc__genname_624=(struct _tagged_arr){_tmp36E,_tmp36E,_tmp36E + 7};
static char _tmp36F[3]="sc";static struct _tuple5 Cyc__gentuple_609={offsetof(struct
Cyc_Absyn_Fndecl,sc),(struct _tagged_arr){_tmp36F,_tmp36F,_tmp36F + 3},(void*)& Cyc_Absyn_scope_t_rep};
static char _tmp370[10]="is_inline";static struct _tuple5 Cyc__gentuple_610={
offsetof(struct Cyc_Absyn_Fndecl,is_inline),(struct _tagged_arr){_tmp370,_tmp370,
_tmp370 + 10},(void*)((void*)& Cyc__genrep_67)};static char _tmp371[5]="name";static
struct _tuple5 Cyc__gentuple_611={offsetof(struct Cyc_Absyn_Fndecl,name),(struct
_tagged_arr){_tmp371,_tmp371,_tmp371 + 5},(void*)& Cyc__genrep_10};static char
_tmp372[4]="tvs";static struct _tuple5 Cyc__gentuple_612={offsetof(struct Cyc_Absyn_Fndecl,tvs),(
struct _tagged_arr){_tmp372,_tmp372,_tmp372 + 4},(void*)& Cyc__genrep_312};static
char _tmp373[7]="effect";static struct _tuple5 Cyc__gentuple_613={offsetof(struct Cyc_Absyn_Fndecl,effect),(
struct _tagged_arr){_tmp373,_tmp373,_tmp373 + 7},(void*)& Cyc__genrep_92};static
char _tmp374[9]="ret_type";static struct _tuple5 Cyc__gentuple_614={offsetof(struct
Cyc_Absyn_Fndecl,ret_type),(struct _tagged_arr){_tmp374,_tmp374,_tmp374 + 9},(void*)((
void*)& Cyc_Absyn_type_t_rep)};static char _tmp375[5]="args";static struct _tuple5 Cyc__gentuple_615={
offsetof(struct Cyc_Absyn_Fndecl,args),(struct _tagged_arr){_tmp375,_tmp375,
_tmp375 + 5},(void*)& Cyc__genrep_598};static char _tmp376[10]="c_varargs";static
struct _tuple5 Cyc__gentuple_616={offsetof(struct Cyc_Absyn_Fndecl,c_varargs),(
struct _tagged_arr){_tmp376,_tmp376,_tmp376 + 10},(void*)((void*)& Cyc__genrep_67)};
static char _tmp377[12]="cyc_varargs";static struct _tuple5 Cyc__gentuple_617={
offsetof(struct Cyc_Absyn_Fndecl,cyc_varargs),(struct _tagged_arr){_tmp377,_tmp377,
_tmp377 + 12},(void*)& Cyc__genrep_587};static char _tmp378[7]="rgn_po";static struct
_tuple5 Cyc__gentuple_618={offsetof(struct Cyc_Absyn_Fndecl,rgn_po),(struct
_tagged_arr){_tmp378,_tmp378,_tmp378 + 7},(void*)& Cyc__genrep_371};static char
_tmp379[5]="body";static struct _tuple5 Cyc__gentuple_619={offsetof(struct Cyc_Absyn_Fndecl,body),(
struct _tagged_arr){_tmp379,_tmp379,_tmp379 + 5},(void*)& Cyc__genrep_163};static
char _tmp37A[11]="cached_typ";static struct _tuple5 Cyc__gentuple_620={offsetof(
struct Cyc_Absyn_Fndecl,cached_typ),(struct _tagged_arr){_tmp37A,_tmp37A,_tmp37A + 
11},(void*)& Cyc__genrep_92};static char _tmp37B[15]="param_vardecls";static struct
_tuple5 Cyc__gentuple_621={offsetof(struct Cyc_Absyn_Fndecl,param_vardecls),(
struct _tagged_arr){_tmp37B,_tmp37B,_tmp37B + 15},(void*)& Cyc__genrep_132};static
char _tmp37C[11]="attributes";static struct _tuple5 Cyc__gentuple_622={offsetof(
struct Cyc_Absyn_Fndecl,attributes),(struct _tagged_arr){_tmp37C,_tmp37C,_tmp37C + 
11},(void*)& Cyc__genrep_43};static struct _tuple5*Cyc__genarr_623[14]={& Cyc__gentuple_609,&
Cyc__gentuple_610,& Cyc__gentuple_611,& Cyc__gentuple_612,& Cyc__gentuple_613,& Cyc__gentuple_614,&
Cyc__gentuple_615,& Cyc__gentuple_616,& Cyc__gentuple_617,& Cyc__gentuple_618,& Cyc__gentuple_619,&
Cyc__gentuple_620,& Cyc__gentuple_621,& Cyc__gentuple_622};struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_Fndecl_rep={3,(struct _tagged_arr*)& Cyc__genname_624,sizeof(
struct Cyc_Absyn_Fndecl),{(void*)((struct _tuple5**)Cyc__genarr_623),(void*)((
struct _tuple5**)Cyc__genarr_623),(void*)((struct _tuple5**)Cyc__genarr_623 + 14)}};
static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_131={1,1,(void*)((void*)& Cyc_struct_Absyn_Fndecl_rep)};
struct _tuple62{unsigned int f1;struct Cyc_Absyn_Fndecl*f2;};static struct _tuple6 Cyc__gentuple_625={
offsetof(struct _tuple62,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_626={
offsetof(struct _tuple62,f2),(void*)& Cyc__genrep_131};static struct _tuple6*Cyc__genarr_627[
2]={& Cyc__gentuple_625,& Cyc__gentuple_626};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_130={4,sizeof(struct _tuple62),{(void*)((struct _tuple6**)Cyc__genarr_627),(
void*)((struct _tuple6**)Cyc__genarr_627),(void*)((struct _tuple6**)Cyc__genarr_627
+ 2)}};static char _tmp380[13]="Unresolved_b";static struct _tuple7 Cyc__gentuple_841={
0,(struct _tagged_arr){_tmp380,_tmp380,_tmp380 + 13}};static struct _tuple7*Cyc__genarr_842[
1]={& Cyc__gentuple_841};static char _tmp381[9]="Global_b";static struct _tuple5 Cyc__gentuple_843={
0,(struct _tagged_arr){_tmp381,_tmp381,_tmp381 + 9},(void*)& Cyc__genrep_431};
static char _tmp382[10]="Funname_b";static struct _tuple5 Cyc__gentuple_844={1,(
struct _tagged_arr){_tmp382,_tmp382,_tmp382 + 10},(void*)& Cyc__genrep_130};static
char _tmp383[8]="Param_b";static struct _tuple5 Cyc__gentuple_845={2,(struct
_tagged_arr){_tmp383,_tmp383,_tmp383 + 8},(void*)& Cyc__genrep_431};static char
_tmp384[8]="Local_b";static struct _tuple5 Cyc__gentuple_846={3,(struct _tagged_arr){
_tmp384,_tmp384,_tmp384 + 8},(void*)& Cyc__genrep_431};static char _tmp385[6]="Pat_b";
static struct _tuple5 Cyc__gentuple_847={4,(struct _tagged_arr){_tmp385,_tmp385,
_tmp385 + 6},(void*)& Cyc__genrep_431};static struct _tuple5*Cyc__genarr_848[5]={&
Cyc__gentuple_843,& Cyc__gentuple_844,& Cyc__gentuple_845,& Cyc__gentuple_846,& Cyc__gentuple_847};
static char _tmp387[8]="Binding";struct Cyc_Typerep_TUnion_struct Cyc_Absyn_binding_t_rep={
5,(struct _tagged_arr){_tmp387,_tmp387,_tmp387 + 8},{(void*)((struct _tuple7**)Cyc__genarr_842),(
void*)((struct _tuple7**)Cyc__genarr_842),(void*)((struct _tuple7**)Cyc__genarr_842
+ 1)},{(void*)((struct _tuple5**)Cyc__genarr_848),(void*)((struct _tuple5**)Cyc__genarr_848),(
void*)((struct _tuple5**)Cyc__genarr_848 + 5)}};struct _tuple63{unsigned int f1;
struct _tuple0*f2;void*f3;};static struct _tuple6 Cyc__gentuple_849={offsetof(struct
_tuple63,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_850={
offsetof(struct _tuple63,f2),(void*)& Cyc__genrep_10};static struct _tuple6 Cyc__gentuple_851={
offsetof(struct _tuple63,f3),(void*)& Cyc_Absyn_binding_t_rep};static struct _tuple6*
Cyc__genarr_852[3]={& Cyc__gentuple_849,& Cyc__gentuple_850,& Cyc__gentuple_851};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_840={4,sizeof(struct _tuple63),{(
void*)((struct _tuple6**)Cyc__genarr_852),(void*)((struct _tuple6**)Cyc__genarr_852),(
void*)((struct _tuple6**)Cyc__genarr_852 + 3)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_835;extern struct Cyc_Typerep_TUnion_struct Cyc_Absyn_primop_t_rep;
static char _tmp389[5]="Plus";static struct _tuple7 Cyc__gentuple_805={0,(struct
_tagged_arr){_tmp389,_tmp389,_tmp389 + 5}};static char _tmp38A[6]="Times";static
struct _tuple7 Cyc__gentuple_806={1,(struct _tagged_arr){_tmp38A,_tmp38A,_tmp38A + 6}};
static char _tmp38B[6]="Minus";static struct _tuple7 Cyc__gentuple_807={2,(struct
_tagged_arr){_tmp38B,_tmp38B,_tmp38B + 6}};static char _tmp38C[4]="Div";static
struct _tuple7 Cyc__gentuple_808={3,(struct _tagged_arr){_tmp38C,_tmp38C,_tmp38C + 4}};
static char _tmp38D[4]="Mod";static struct _tuple7 Cyc__gentuple_809={4,(struct
_tagged_arr){_tmp38D,_tmp38D,_tmp38D + 4}};static char _tmp38E[3]="Eq";static struct
_tuple7 Cyc__gentuple_810={5,(struct _tagged_arr){_tmp38E,_tmp38E,_tmp38E + 3}};
static char _tmp38F[4]="Neq";static struct _tuple7 Cyc__gentuple_811={6,(struct
_tagged_arr){_tmp38F,_tmp38F,_tmp38F + 4}};static char _tmp390[3]="Gt";static struct
_tuple7 Cyc__gentuple_812={7,(struct _tagged_arr){_tmp390,_tmp390,_tmp390 + 3}};
static char _tmp391[3]="Lt";static struct _tuple7 Cyc__gentuple_813={8,(struct
_tagged_arr){_tmp391,_tmp391,_tmp391 + 3}};static char _tmp392[4]="Gte";static
struct _tuple7 Cyc__gentuple_814={9,(struct _tagged_arr){_tmp392,_tmp392,_tmp392 + 4}};
static char _tmp393[4]="Lte";static struct _tuple7 Cyc__gentuple_815={10,(struct
_tagged_arr){_tmp393,_tmp393,_tmp393 + 4}};static char _tmp394[4]="Not";static
struct _tuple7 Cyc__gentuple_816={11,(struct _tagged_arr){_tmp394,_tmp394,_tmp394 + 
4}};static char _tmp395[7]="Bitnot";static struct _tuple7 Cyc__gentuple_817={12,(
struct _tagged_arr){_tmp395,_tmp395,_tmp395 + 7}};static char _tmp396[7]="Bitand";
static struct _tuple7 Cyc__gentuple_818={13,(struct _tagged_arr){_tmp396,_tmp396,
_tmp396 + 7}};static char _tmp397[6]="Bitor";static struct _tuple7 Cyc__gentuple_819={
14,(struct _tagged_arr){_tmp397,_tmp397,_tmp397 + 6}};static char _tmp398[7]="Bitxor";
static struct _tuple7 Cyc__gentuple_820={15,(struct _tagged_arr){_tmp398,_tmp398,
_tmp398 + 7}};static char _tmp399[10]="Bitlshift";static struct _tuple7 Cyc__gentuple_821={
16,(struct _tagged_arr){_tmp399,_tmp399,_tmp399 + 10}};static char _tmp39A[11]="Bitlrshift";
static struct _tuple7 Cyc__gentuple_822={17,(struct _tagged_arr){_tmp39A,_tmp39A,
_tmp39A + 11}};static char _tmp39B[11]="Bitarshift";static struct _tuple7 Cyc__gentuple_823={
18,(struct _tagged_arr){_tmp39B,_tmp39B,_tmp39B + 11}};static char _tmp39C[5]="Size";
static struct _tuple7 Cyc__gentuple_824={19,(struct _tagged_arr){_tmp39C,_tmp39C,
_tmp39C + 5}};static struct _tuple7*Cyc__genarr_825[20]={& Cyc__gentuple_805,& Cyc__gentuple_806,&
Cyc__gentuple_807,& Cyc__gentuple_808,& Cyc__gentuple_809,& Cyc__gentuple_810,& Cyc__gentuple_811,&
Cyc__gentuple_812,& Cyc__gentuple_813,& Cyc__gentuple_814,& Cyc__gentuple_815,& Cyc__gentuple_816,&
Cyc__gentuple_817,& Cyc__gentuple_818,& Cyc__gentuple_819,& Cyc__gentuple_820,& Cyc__gentuple_821,&
Cyc__gentuple_822,& Cyc__gentuple_823,& Cyc__gentuple_824};static struct _tuple5*Cyc__genarr_826[
0]={};static char _tmp39E[7]="Primop";struct Cyc_Typerep_TUnion_struct Cyc_Absyn_primop_t_rep={
5,(struct _tagged_arr){_tmp39E,_tmp39E,_tmp39E + 7},{(void*)((struct _tuple7**)Cyc__genarr_825),(
void*)((struct _tuple7**)Cyc__genarr_825),(void*)((struct _tuple7**)Cyc__genarr_825
+ 20)},{(void*)((struct _tuple5**)Cyc__genarr_826),(void*)((struct _tuple5**)Cyc__genarr_826),(
void*)((struct _tuple5**)Cyc__genarr_826 + 0)}};struct _tuple64{unsigned int f1;void*
f2;struct Cyc_List_List*f3;};static struct _tuple6 Cyc__gentuple_836={offsetof(
struct _tuple64,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_837={
offsetof(struct _tuple64,f2),(void*)& Cyc_Absyn_primop_t_rep};static struct _tuple6
Cyc__gentuple_838={offsetof(struct _tuple64,f3),(void*)& Cyc__genrep_491};static
struct _tuple6*Cyc__genarr_839[3]={& Cyc__gentuple_836,& Cyc__gentuple_837,& Cyc__gentuple_838};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_835={4,sizeof(struct _tuple64),{(
void*)((struct _tuple6**)Cyc__genarr_839),(void*)((struct _tuple6**)Cyc__genarr_839),(
void*)((struct _tuple6**)Cyc__genarr_839 + 3)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_803;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_804;extern
struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0Absyn_primop_t2_rep;static
char _tmp3A0[4]="Opt";static struct _tagged_arr Cyc__genname_829=(struct _tagged_arr){
_tmp3A0,_tmp3A0,_tmp3A0 + 4};static char _tmp3A1[2]="v";static struct _tuple5 Cyc__gentuple_827={
offsetof(struct Cyc_Core_Opt,v),(struct _tagged_arr){_tmp3A1,_tmp3A1,_tmp3A1 + 2},(
void*)& Cyc_Absyn_primop_t_rep};static struct _tuple5*Cyc__genarr_828[1]={& Cyc__gentuple_827};
struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0Absyn_primop_t2_rep={3,(
struct _tagged_arr*)& Cyc__genname_829,sizeof(struct Cyc_Core_Opt),{(void*)((struct
_tuple5**)Cyc__genarr_828),(void*)((struct _tuple5**)Cyc__genarr_828),(void*)((
struct _tuple5**)Cyc__genarr_828 + 1)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_804={
1,1,(void*)((void*)& Cyc_struct_Core_Opt0Absyn_primop_t2_rep)};struct _tuple65{
unsigned int f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Core_Opt*f3;struct Cyc_Absyn_Exp*
f4;};static struct _tuple6 Cyc__gentuple_830={offsetof(struct _tuple65,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_831={offsetof(struct _tuple65,f2),(
void*)& Cyc__genrep_126};static struct _tuple6 Cyc__gentuple_832={offsetof(struct
_tuple65,f3),(void*)& Cyc__genrep_804};static struct _tuple6 Cyc__gentuple_833={
offsetof(struct _tuple65,f4),(void*)& Cyc__genrep_126};static struct _tuple6*Cyc__genarr_834[
4]={& Cyc__gentuple_830,& Cyc__gentuple_831,& Cyc__gentuple_832,& Cyc__gentuple_833};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_803={4,sizeof(struct _tuple65),{(
void*)((struct _tuple6**)Cyc__genarr_834),(void*)((struct _tuple6**)Cyc__genarr_834),(
void*)((struct _tuple6**)Cyc__genarr_834 + 4)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_792;extern struct Cyc_Typerep_TUnion_struct Cyc_Absyn_incrementor_t_rep;
static char _tmp3A5[7]="PreInc";static struct _tuple7 Cyc__gentuple_793={0,(struct
_tagged_arr){_tmp3A5,_tmp3A5,_tmp3A5 + 7}};static char _tmp3A6[8]="PostInc";static
struct _tuple7 Cyc__gentuple_794={1,(struct _tagged_arr){_tmp3A6,_tmp3A6,_tmp3A6 + 8}};
static char _tmp3A7[7]="PreDec";static struct _tuple7 Cyc__gentuple_795={2,(struct
_tagged_arr){_tmp3A7,_tmp3A7,_tmp3A7 + 7}};static char _tmp3A8[8]="PostDec";static
struct _tuple7 Cyc__gentuple_796={3,(struct _tagged_arr){_tmp3A8,_tmp3A8,_tmp3A8 + 8}};
static struct _tuple7*Cyc__genarr_797[4]={& Cyc__gentuple_793,& Cyc__gentuple_794,&
Cyc__gentuple_795,& Cyc__gentuple_796};static struct _tuple5*Cyc__genarr_798[0]={};
static char _tmp3AA[12]="Incrementor";struct Cyc_Typerep_TUnion_struct Cyc_Absyn_incrementor_t_rep={
5,(struct _tagged_arr){_tmp3AA,_tmp3AA,_tmp3AA + 12},{(void*)((struct _tuple7**)Cyc__genarr_797),(
void*)((struct _tuple7**)Cyc__genarr_797),(void*)((struct _tuple7**)Cyc__genarr_797
+ 4)},{(void*)((struct _tuple5**)Cyc__genarr_798),(void*)((struct _tuple5**)Cyc__genarr_798),(
void*)((struct _tuple5**)Cyc__genarr_798 + 0)}};struct _tuple66{unsigned int f1;
struct Cyc_Absyn_Exp*f2;void*f3;};static struct _tuple6 Cyc__gentuple_799={offsetof(
struct _tuple66,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_800={
offsetof(struct _tuple66,f2),(void*)& Cyc__genrep_126};static struct _tuple6 Cyc__gentuple_801={
offsetof(struct _tuple66,f3),(void*)& Cyc_Absyn_incrementor_t_rep};static struct
_tuple6*Cyc__genarr_802[3]={& Cyc__gentuple_799,& Cyc__gentuple_800,& Cyc__gentuple_801};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_792={4,sizeof(struct _tuple66),{(
void*)((struct _tuple6**)Cyc__genarr_802),(void*)((struct _tuple6**)Cyc__genarr_802),(
void*)((struct _tuple6**)Cyc__genarr_802 + 3)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_786;struct _tuple67{unsigned int f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*
f3;struct Cyc_Absyn_Exp*f4;};static struct _tuple6 Cyc__gentuple_787={offsetof(
struct _tuple67,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_788={
offsetof(struct _tuple67,f2),(void*)& Cyc__genrep_126};static struct _tuple6 Cyc__gentuple_789={
offsetof(struct _tuple67,f3),(void*)& Cyc__genrep_126};static struct _tuple6 Cyc__gentuple_790={
offsetof(struct _tuple67,f4),(void*)& Cyc__genrep_126};static struct _tuple6*Cyc__genarr_791[
4]={& Cyc__gentuple_787,& Cyc__gentuple_788,& Cyc__gentuple_789,& Cyc__gentuple_790};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_786={4,sizeof(struct _tuple67),{(
void*)((struct _tuple6**)Cyc__genarr_791),(void*)((struct _tuple6**)Cyc__genarr_791),(
void*)((struct _tuple6**)Cyc__genarr_791 + 4)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_717;struct _tuple68{unsigned int f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*
f3;};static struct _tuple6 Cyc__gentuple_718={offsetof(struct _tuple68,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_719={offsetof(struct _tuple68,f2),(
void*)& Cyc__genrep_126};static struct _tuple6 Cyc__gentuple_720={offsetof(struct
_tuple68,f3),(void*)& Cyc__genrep_126};static struct _tuple6*Cyc__genarr_721[3]={&
Cyc__gentuple_718,& Cyc__gentuple_719,& Cyc__gentuple_720};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_717={4,sizeof(struct _tuple68),{(void*)((struct _tuple6**)Cyc__genarr_721),(
void*)((struct _tuple6**)Cyc__genarr_721),(void*)((struct _tuple6**)Cyc__genarr_721
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_781;static struct _tuple6
Cyc__gentuple_782={offsetof(struct _tuple55,f1),(void*)& Cyc__genrep_5};static
struct _tuple6 Cyc__gentuple_783={offsetof(struct _tuple55,f2),(void*)& Cyc__genrep_126};
static struct _tuple6 Cyc__gentuple_784={offsetof(struct _tuple55,f3),(void*)& Cyc__genrep_491};
static struct _tuple6*Cyc__genarr_785[3]={& Cyc__gentuple_782,& Cyc__gentuple_783,&
Cyc__gentuple_784};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_781={4,
sizeof(struct _tuple55),{(void*)((struct _tuple6**)Cyc__genarr_785),(void*)((
struct _tuple6**)Cyc__genarr_785),(void*)((struct _tuple6**)Cyc__genarr_785 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_768;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_769;extern struct Cyc_Typerep_Struct_struct Cyc_Absyn_vararg_call_info_t_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_770;static struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_770={1,1,(void*)((void*)& Cyc_Absyn_vararg_info_t_rep)};static char
_tmp3B0[15]="VarargCallInfo";static struct _tagged_arr Cyc__genname_775=(struct
_tagged_arr){_tmp3B0,_tmp3B0,_tmp3B0 + 15};static char _tmp3B1[12]="num_varargs";
static struct _tuple5 Cyc__gentuple_771={offsetof(struct Cyc_Absyn_VarargCallInfo,num_varargs),(
struct _tagged_arr){_tmp3B1,_tmp3B1,_tmp3B1 + 12},(void*)((void*)& Cyc__genrep_67)};
static char _tmp3B2[10]="injectors";static struct _tuple5 Cyc__gentuple_772={
offsetof(struct Cyc_Absyn_VarargCallInfo,injectors),(struct _tagged_arr){_tmp3B2,
_tmp3B2,_tmp3B2 + 10},(void*)& Cyc__genrep_304};static char _tmp3B3[4]="vai";static
struct _tuple5 Cyc__gentuple_773={offsetof(struct Cyc_Absyn_VarargCallInfo,vai),(
struct _tagged_arr){_tmp3B3,_tmp3B3,_tmp3B3 + 4},(void*)& Cyc__genrep_770};static
struct _tuple5*Cyc__genarr_774[3]={& Cyc__gentuple_771,& Cyc__gentuple_772,& Cyc__gentuple_773};
struct Cyc_Typerep_Struct_struct Cyc_Absyn_vararg_call_info_t_rep={3,(struct
_tagged_arr*)& Cyc__genname_775,sizeof(struct Cyc_Absyn_VarargCallInfo),{(void*)((
struct _tuple5**)Cyc__genarr_774),(void*)((struct _tuple5**)Cyc__genarr_774),(void*)((
struct _tuple5**)Cyc__genarr_774 + 3)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_769={
1,1,(void*)((void*)& Cyc_Absyn_vararg_call_info_t_rep)};struct _tuple69{
unsigned int f1;struct Cyc_Absyn_Exp*f2;struct Cyc_List_List*f3;struct Cyc_Absyn_VarargCallInfo*
f4;};static struct _tuple6 Cyc__gentuple_776={offsetof(struct _tuple69,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_777={offsetof(struct _tuple69,f2),(
void*)& Cyc__genrep_126};static struct _tuple6 Cyc__gentuple_778={offsetof(struct
_tuple69,f3),(void*)& Cyc__genrep_491};static struct _tuple6 Cyc__gentuple_779={
offsetof(struct _tuple69,f4),(void*)& Cyc__genrep_769};static struct _tuple6*Cyc__genarr_780[
4]={& Cyc__gentuple_776,& Cyc__gentuple_777,& Cyc__gentuple_778,& Cyc__gentuple_779};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_768={4,sizeof(struct _tuple69),{(
void*)((struct _tuple6**)Cyc__genarr_780),(void*)((struct _tuple6**)Cyc__genarr_780),(
void*)((struct _tuple6**)Cyc__genarr_780 + 4)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_763;static struct _tuple6 Cyc__gentuple_764={offsetof(struct _tuple55,f1),(
void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_765={offsetof(struct
_tuple55,f2),(void*)& Cyc__genrep_126};static struct _tuple6 Cyc__gentuple_766={
offsetof(struct _tuple55,f3),(void*)& Cyc__genrep_102};static struct _tuple6*Cyc__genarr_767[
3]={& Cyc__gentuple_764,& Cyc__gentuple_765,& Cyc__gentuple_766};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_763={4,sizeof(struct _tuple55),{(void*)((struct _tuple6**)Cyc__genarr_767),(
void*)((struct _tuple6**)Cyc__genarr_767),(void*)((struct _tuple6**)Cyc__genarr_767
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_750;extern struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_coercion_t_rep;static char _tmp3B8[17]="Unknown_coercion";static struct
_tuple7 Cyc__gentuple_751={0,(struct _tagged_arr){_tmp3B8,_tmp3B8,_tmp3B8 + 17}};
static char _tmp3B9[12]="No_coercion";static struct _tuple7 Cyc__gentuple_752={1,(
struct _tagged_arr){_tmp3B9,_tmp3B9,_tmp3B9 + 12}};static char _tmp3BA[16]="NonNull_to_Null";
static struct _tuple7 Cyc__gentuple_753={2,(struct _tagged_arr){_tmp3BA,_tmp3BA,
_tmp3BA + 16}};static char _tmp3BB[15]="Other_coercion";static struct _tuple7 Cyc__gentuple_754={
3,(struct _tagged_arr){_tmp3BB,_tmp3BB,_tmp3BB + 15}};static struct _tuple7*Cyc__genarr_755[
4]={& Cyc__gentuple_751,& Cyc__gentuple_752,& Cyc__gentuple_753,& Cyc__gentuple_754};
static struct _tuple5*Cyc__genarr_756[0]={};static char _tmp3BD[9]="Coercion";struct
Cyc_Typerep_TUnion_struct Cyc_Absyn_coercion_t_rep={5,(struct _tagged_arr){_tmp3BD,
_tmp3BD,_tmp3BD + 9},{(void*)((struct _tuple7**)Cyc__genarr_755),(void*)((struct
_tuple7**)Cyc__genarr_755),(void*)((struct _tuple7**)Cyc__genarr_755 + 4)},{(void*)((
struct _tuple5**)Cyc__genarr_756),(void*)((struct _tuple5**)Cyc__genarr_756),(void*)((
struct _tuple5**)Cyc__genarr_756 + 0)}};struct _tuple70{unsigned int f1;void*f2;
struct Cyc_Absyn_Exp*f3;int f4;void*f5;};static struct _tuple6 Cyc__gentuple_757={
offsetof(struct _tuple70,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_758={
offsetof(struct _tuple70,f2),(void*)((void*)& Cyc_Absyn_type_t_rep)};static struct
_tuple6 Cyc__gentuple_759={offsetof(struct _tuple70,f3),(void*)& Cyc__genrep_126};
static struct _tuple6 Cyc__gentuple_760={offsetof(struct _tuple70,f4),(void*)((void*)&
Cyc__genrep_67)};static struct _tuple6 Cyc__gentuple_761={offsetof(struct _tuple70,f5),(
void*)& Cyc_Absyn_coercion_t_rep};static struct _tuple6*Cyc__genarr_762[5]={& Cyc__gentuple_757,&
Cyc__gentuple_758,& Cyc__gentuple_759,& Cyc__gentuple_760,& Cyc__gentuple_761};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_750={4,sizeof(struct _tuple70),{(
void*)((struct _tuple6**)Cyc__genarr_762),(void*)((struct _tuple6**)Cyc__genarr_762),(
void*)((struct _tuple6**)Cyc__genarr_762 + 5)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_745;static struct _tuple6 Cyc__gentuple_746={offsetof(struct _tuple68,f1),(
void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_747={offsetof(struct
_tuple68,f2),(void*)& Cyc__genrep_122};static struct _tuple6 Cyc__gentuple_748={
offsetof(struct _tuple68,f3),(void*)& Cyc__genrep_126};static struct _tuple6*Cyc__genarr_749[
3]={& Cyc__gentuple_746,& Cyc__gentuple_747,& Cyc__gentuple_748};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_745={4,sizeof(struct _tuple68),{(void*)((struct _tuple6**)Cyc__genarr_749),(
void*)((struct _tuple6**)Cyc__genarr_749),(void*)((struct _tuple6**)Cyc__genarr_749
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_97;static struct _tuple6 Cyc__gentuple_98={
offsetof(struct _tuple6,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_99={
offsetof(struct _tuple6,f2),(void*)((void*)& Cyc_Absyn_type_t_rep)};static struct
_tuple6*Cyc__genarr_100[2]={& Cyc__gentuple_98,& Cyc__gentuple_99};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_97={4,sizeof(struct _tuple6),{(void*)((struct _tuple6**)Cyc__genarr_100),(
void*)((struct _tuple6**)Cyc__genarr_100),(void*)((struct _tuple6**)Cyc__genarr_100
+ 2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_732;extern struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_offsetof_field_t_rep;extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_734;
struct _tuple71{unsigned int f1;unsigned int f2;};static struct _tuple6 Cyc__gentuple_735={
offsetof(struct _tuple71,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_736={
offsetof(struct _tuple71,f2),(void*)& Cyc__genrep_5};static struct _tuple6*Cyc__genarr_737[
2]={& Cyc__gentuple_735,& Cyc__gentuple_736};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_734={4,sizeof(struct _tuple71),{(void*)((struct _tuple6**)Cyc__genarr_737),(
void*)((struct _tuple6**)Cyc__genarr_737),(void*)((struct _tuple6**)Cyc__genarr_737
+ 2)}};static struct _tuple7*Cyc__genarr_733[0]={};static char _tmp3C2[12]="StructField";
static struct _tuple5 Cyc__gentuple_738={0,(struct _tagged_arr){_tmp3C2,_tmp3C2,
_tmp3C2 + 12},(void*)& Cyc__genrep_335};static char _tmp3C3[11]="TupleIndex";static
struct _tuple5 Cyc__gentuple_739={1,(struct _tagged_arr){_tmp3C3,_tmp3C3,_tmp3C3 + 
11},(void*)& Cyc__genrep_734};static struct _tuple5*Cyc__genarr_740[2]={& Cyc__gentuple_738,&
Cyc__gentuple_739};static char _tmp3C5[14]="OffsetofField";struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_offsetof_field_t_rep={5,(struct _tagged_arr){_tmp3C5,_tmp3C5,_tmp3C5 + 
14},{(void*)((struct _tuple7**)Cyc__genarr_733),(void*)((struct _tuple7**)Cyc__genarr_733),(
void*)((struct _tuple7**)Cyc__genarr_733 + 0)},{(void*)((struct _tuple5**)Cyc__genarr_740),(
void*)((struct _tuple5**)Cyc__genarr_740),(void*)((struct _tuple5**)Cyc__genarr_740
+ 2)}};static struct _tuple6 Cyc__gentuple_741={offsetof(struct _tuple28,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_742={offsetof(struct _tuple28,f2),(
void*)((void*)& Cyc_Absyn_type_t_rep)};static struct _tuple6 Cyc__gentuple_743={
offsetof(struct _tuple28,f3),(void*)& Cyc_Absyn_offsetof_field_t_rep};static struct
_tuple6*Cyc__genarr_744[3]={& Cyc__gentuple_741,& Cyc__gentuple_742,& Cyc__gentuple_743};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_732={4,sizeof(struct _tuple28),{(
void*)((struct _tuple6**)Cyc__genarr_744),(void*)((struct _tuple6**)Cyc__genarr_744),(
void*)((struct _tuple6**)Cyc__genarr_744 + 3)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_727;struct _tuple72{unsigned int f1;struct Cyc_List_List*f2;void*f3;};
static struct _tuple6 Cyc__gentuple_728={offsetof(struct _tuple72,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_729={offsetof(struct _tuple72,f2),(void*)& Cyc__genrep_312};
static struct _tuple6 Cyc__gentuple_730={offsetof(struct _tuple72,f3),(void*)((void*)&
Cyc_Absyn_type_t_rep)};static struct _tuple6*Cyc__genarr_731[3]={& Cyc__gentuple_728,&
Cyc__gentuple_729,& Cyc__gentuple_730};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_727={
4,sizeof(struct _tuple72),{(void*)((struct _tuple6**)Cyc__genarr_731),(void*)((
struct _tuple6**)Cyc__genarr_731),(void*)((struct _tuple6**)Cyc__genarr_731 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_722;struct _tuple73{unsigned int
f1;struct Cyc_Absyn_Exp*f2;struct _tagged_arr*f3;};static struct _tuple6 Cyc__gentuple_723={
offsetof(struct _tuple73,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_724={
offsetof(struct _tuple73,f2),(void*)& Cyc__genrep_126};static struct _tuple6 Cyc__gentuple_725={
offsetof(struct _tuple73,f3),(void*)& Cyc__genrep_12};static struct _tuple6*Cyc__genarr_726[
3]={& Cyc__gentuple_723,& Cyc__gentuple_724,& Cyc__gentuple_725};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_722={4,sizeof(struct _tuple73),{(void*)((struct _tuple6**)Cyc__genarr_726),(
void*)((struct _tuple6**)Cyc__genarr_726),(void*)((struct _tuple6**)Cyc__genarr_726
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_713;static struct _tuple6
Cyc__gentuple_714={offsetof(struct _tuple13,f1),(void*)& Cyc__genrep_5};static
struct _tuple6 Cyc__gentuple_715={offsetof(struct _tuple13,f2),(void*)& Cyc__genrep_491};
static struct _tuple6*Cyc__genarr_716[2]={& Cyc__gentuple_714,& Cyc__gentuple_715};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_713={4,sizeof(struct _tuple13),{(
void*)((struct _tuple6**)Cyc__genarr_716),(void*)((struct _tuple6**)Cyc__genarr_716),(
void*)((struct _tuple6**)Cyc__genarr_716 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_702;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_703;extern
struct Cyc_Typerep_Tuple_struct Cyc__genrep_704;static struct _tuple6 Cyc__gentuple_705={
offsetof(struct _tuple1,f1),(void*)& Cyc__genrep_588};static struct _tuple6 Cyc__gentuple_706={
offsetof(struct _tuple1,f2),(void*)& Cyc__genrep_135};static struct _tuple6 Cyc__gentuple_707={
offsetof(struct _tuple1,f3),(void*)((void*)& Cyc_Absyn_type_t_rep)};static struct
_tuple6*Cyc__genarr_708[3]={& Cyc__gentuple_705,& Cyc__gentuple_706,& Cyc__gentuple_707};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_704={4,sizeof(struct _tuple1),{(
void*)((struct _tuple6**)Cyc__genarr_708),(void*)((struct _tuple6**)Cyc__genarr_708),(
void*)((struct _tuple6**)Cyc__genarr_708 + 3)}};static struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_703={1,1,(void*)((void*)& Cyc__genrep_704)};extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_629;extern struct Cyc_Typerep_Struct_struct Cyc_struct_List_List060List_list_t0Absyn_designator_t46H24Absyn_exp_t1_446H2_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_630;extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_631;static struct _tuple6 Cyc__gentuple_632={offsetof(struct _tuple12,f1),(
void*)& Cyc__genrep_333};static struct _tuple6 Cyc__gentuple_633={offsetof(struct
_tuple12,f2),(void*)& Cyc__genrep_126};static struct _tuple6*Cyc__genarr_634[2]={&
Cyc__gentuple_632,& Cyc__gentuple_633};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_631={
4,sizeof(struct _tuple12),{(void*)((struct _tuple6**)Cyc__genarr_634),(void*)((
struct _tuple6**)Cyc__genarr_634),(void*)((struct _tuple6**)Cyc__genarr_634 + 2)}};
static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_630={1,1,(void*)((void*)& Cyc__genrep_631)};
static char _tmp3CE[5]="List";static struct _tagged_arr Cyc__genname_638=(struct
_tagged_arr){_tmp3CE,_tmp3CE,_tmp3CE + 5};static char _tmp3CF[3]="hd";static struct
_tuple5 Cyc__gentuple_635={offsetof(struct Cyc_List_List,hd),(struct _tagged_arr){
_tmp3CF,_tmp3CF,_tmp3CF + 3},(void*)& Cyc__genrep_630};static char _tmp3D0[3]="tl";
static struct _tuple5 Cyc__gentuple_636={offsetof(struct Cyc_List_List,tl),(struct
_tagged_arr){_tmp3D0,_tmp3D0,_tmp3D0 + 3},(void*)& Cyc__genrep_629};static struct
_tuple5*Cyc__genarr_637[2]={& Cyc__gentuple_635,& Cyc__gentuple_636};struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List060List_list_t0Absyn_designator_t46H24Absyn_exp_t1_446H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_638,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_637),(void*)((struct _tuple5**)Cyc__genarr_637),(void*)((
struct _tuple5**)Cyc__genarr_637 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_629={
1,1,(void*)((void*)& Cyc_struct_List_List060List_list_t0Absyn_designator_t46H24Absyn_exp_t1_446H2_rep)};
struct _tuple74{unsigned int f1;struct _tuple1*f2;struct Cyc_List_List*f3;};static
struct _tuple6 Cyc__gentuple_709={offsetof(struct _tuple74,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_710={offsetof(struct _tuple74,f2),(void*)& Cyc__genrep_703};
static struct _tuple6 Cyc__gentuple_711={offsetof(struct _tuple74,f3),(void*)& Cyc__genrep_629};
static struct _tuple6*Cyc__genarr_712[3]={& Cyc__gentuple_709,& Cyc__gentuple_710,&
Cyc__gentuple_711};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_702={4,
sizeof(struct _tuple74),{(void*)((struct _tuple6**)Cyc__genarr_712),(void*)((
struct _tuple6**)Cyc__genarr_712),(void*)((struct _tuple6**)Cyc__genarr_712 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_698;static struct _tuple6 Cyc__gentuple_699={
offsetof(struct _tuple13,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_700={
offsetof(struct _tuple13,f2),(void*)& Cyc__genrep_629};static struct _tuple6*Cyc__genarr_701[
2]={& Cyc__gentuple_699,& Cyc__gentuple_700};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_698={4,sizeof(struct _tuple13),{(void*)((struct _tuple6**)Cyc__genarr_701),(
void*)((struct _tuple6**)Cyc__genarr_701),(void*)((struct _tuple6**)Cyc__genarr_701
+ 2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_691;struct _tuple75{
unsigned int f1;struct Cyc_Absyn_Vardecl*f2;struct Cyc_Absyn_Exp*f3;struct Cyc_Absyn_Exp*
f4;int f5;};static struct _tuple6 Cyc__gentuple_692={offsetof(struct _tuple75,f1),(
void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_693={offsetof(struct
_tuple75,f2),(void*)& Cyc__genrep_134};static struct _tuple6 Cyc__gentuple_694={
offsetof(struct _tuple75,f3),(void*)& Cyc__genrep_126};static struct _tuple6 Cyc__gentuple_695={
offsetof(struct _tuple75,f4),(void*)& Cyc__genrep_126};static struct _tuple6 Cyc__gentuple_696={
offsetof(struct _tuple75,f5),(void*)((void*)& Cyc__genrep_67)};static struct _tuple6*
Cyc__genarr_697[5]={& Cyc__gentuple_692,& Cyc__gentuple_693,& Cyc__gentuple_694,&
Cyc__gentuple_695,& Cyc__gentuple_696};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_691={
4,sizeof(struct _tuple75),{(void*)((struct _tuple6**)Cyc__genarr_697),(void*)((
struct _tuple6**)Cyc__genarr_697),(void*)((struct _tuple6**)Cyc__genarr_697 + 5)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_683;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_684;static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_684={1,1,(void*)((
void*)& Cyc_struct_Absyn_Aggrdecl_rep)};struct _tuple76{unsigned int f1;struct
_tuple0*f2;struct Cyc_List_List*f3;struct Cyc_List_List*f4;struct Cyc_Absyn_Aggrdecl*
f5;};static struct _tuple6 Cyc__gentuple_685={offsetof(struct _tuple76,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_686={offsetof(struct _tuple76,f2),(
void*)& Cyc__genrep_10};static struct _tuple6 Cyc__gentuple_687={offsetof(struct
_tuple76,f3),(void*)& Cyc__genrep_102};static struct _tuple6 Cyc__gentuple_688={
offsetof(struct _tuple76,f4),(void*)& Cyc__genrep_629};static struct _tuple6 Cyc__gentuple_689={
offsetof(struct _tuple76,f5),(void*)& Cyc__genrep_684};static struct _tuple6*Cyc__genarr_690[
5]={& Cyc__gentuple_685,& Cyc__gentuple_686,& Cyc__gentuple_687,& Cyc__gentuple_688,&
Cyc__gentuple_689};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_683={4,
sizeof(struct _tuple76),{(void*)((struct _tuple6**)Cyc__genarr_690),(void*)((
struct _tuple6**)Cyc__genarr_690),(void*)((struct _tuple6**)Cyc__genarr_690 + 5)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_678;static struct _tuple6 Cyc__gentuple_679={
offsetof(struct _tuple64,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_680={
offsetof(struct _tuple64,f2),(void*)((void*)& Cyc_Absyn_type_t_rep)};static struct
_tuple6 Cyc__gentuple_681={offsetof(struct _tuple64,f3),(void*)& Cyc__genrep_629};
static struct _tuple6*Cyc__genarr_682[3]={& Cyc__gentuple_679,& Cyc__gentuple_680,&
Cyc__gentuple_681};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_678={4,
sizeof(struct _tuple64),{(void*)((struct _tuple6**)Cyc__genarr_682),(void*)((
struct _tuple6**)Cyc__genarr_682),(void*)((struct _tuple6**)Cyc__genarr_682 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_672;struct _tuple77{unsigned int
f1;struct Cyc_List_List*f2;struct Cyc_Absyn_Tuniondecl*f3;struct Cyc_Absyn_Tunionfield*
f4;};static struct _tuple6 Cyc__gentuple_673={offsetof(struct _tuple77,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_674={offsetof(struct _tuple77,f2),(
void*)& Cyc__genrep_491};static struct _tuple6 Cyc__gentuple_675={offsetof(struct
_tuple77,f3),(void*)((void*)& Cyc__genrep_302)};static struct _tuple6 Cyc__gentuple_676={
offsetof(struct _tuple77,f4),(void*)& Cyc__genrep_285};static struct _tuple6*Cyc__genarr_677[
4]={& Cyc__gentuple_673,& Cyc__gentuple_674,& Cyc__gentuple_675,& Cyc__gentuple_676};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_672={4,sizeof(struct _tuple77),{(
void*)((struct _tuple6**)Cyc__genarr_677),(void*)((struct _tuple6**)Cyc__genarr_677),(
void*)((struct _tuple6**)Cyc__genarr_677 + 4)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_665;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_666;static
struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_666={1,1,(void*)((void*)& Cyc_struct_Absyn_Enumdecl_rep)};
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_659;static struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_659={1,1,(void*)((void*)& Cyc_struct_Absyn_Enumfield_rep)};struct
_tuple78{unsigned int f1;struct _tuple0*f2;struct Cyc_Absyn_Enumdecl*f3;struct Cyc_Absyn_Enumfield*
f4;};static struct _tuple6 Cyc__gentuple_667={offsetof(struct _tuple78,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_668={offsetof(struct _tuple78,f2),(
void*)& Cyc__genrep_10};static struct _tuple6 Cyc__gentuple_669={offsetof(struct
_tuple78,f3),(void*)& Cyc__genrep_666};static struct _tuple6 Cyc__gentuple_670={
offsetof(struct _tuple78,f4),(void*)& Cyc__genrep_659};static struct _tuple6*Cyc__genarr_671[
4]={& Cyc__gentuple_667,& Cyc__gentuple_668,& Cyc__gentuple_669,& Cyc__gentuple_670};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_665={4,sizeof(struct _tuple78),{(
void*)((struct _tuple6**)Cyc__genarr_671),(void*)((struct _tuple6**)Cyc__genarr_671),(
void*)((struct _tuple6**)Cyc__genarr_671 + 4)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_658;struct _tuple79{unsigned int f1;struct _tuple0*f2;void*f3;struct Cyc_Absyn_Enumfield*
f4;};static struct _tuple6 Cyc__gentuple_660={offsetof(struct _tuple79,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_661={offsetof(struct _tuple79,f2),(
void*)& Cyc__genrep_10};static struct _tuple6 Cyc__gentuple_662={offsetof(struct
_tuple79,f3),(void*)((void*)& Cyc_Absyn_type_t_rep)};static struct _tuple6 Cyc__gentuple_663={
offsetof(struct _tuple79,f4),(void*)& Cyc__genrep_659};static struct _tuple6*Cyc__genarr_664[
4]={& Cyc__gentuple_660,& Cyc__gentuple_661,& Cyc__gentuple_662,& Cyc__gentuple_663};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_658={4,sizeof(struct _tuple79),{(
void*)((struct _tuple6**)Cyc__genarr_664),(void*)((struct _tuple6**)Cyc__genarr_664),(
void*)((struct _tuple6**)Cyc__genarr_664 + 4)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_647;extern struct Cyc_Typerep_Struct_struct Cyc_Absyn_malloc_info_t_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_111;static struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_111={1,1,(void*)((void*)& Cyc_Absyn_type_t_rep)};static char _tmp3DF[11]="MallocInfo";
static struct _tagged_arr Cyc__genname_654=(struct _tagged_arr){_tmp3DF,_tmp3DF,
_tmp3DF + 11};static char _tmp3E0[10]="is_calloc";static struct _tuple5 Cyc__gentuple_648={
offsetof(struct Cyc_Absyn_MallocInfo,is_calloc),(struct _tagged_arr){_tmp3E0,
_tmp3E0,_tmp3E0 + 10},(void*)((void*)& Cyc__genrep_67)};static char _tmp3E1[4]="rgn";
static struct _tuple5 Cyc__gentuple_649={offsetof(struct Cyc_Absyn_MallocInfo,rgn),(
struct _tagged_arr){_tmp3E1,_tmp3E1,_tmp3E1 + 4},(void*)& Cyc__genrep_122};static
char _tmp3E2[9]="elt_type";static struct _tuple5 Cyc__gentuple_650={offsetof(struct
Cyc_Absyn_MallocInfo,elt_type),(struct _tagged_arr){_tmp3E2,_tmp3E2,_tmp3E2 + 9},(
void*)& Cyc__genrep_111};static char _tmp3E3[9]="num_elts";static struct _tuple5 Cyc__gentuple_651={
offsetof(struct Cyc_Absyn_MallocInfo,num_elts),(struct _tagged_arr){_tmp3E3,
_tmp3E3,_tmp3E3 + 9},(void*)& Cyc__genrep_126};static char _tmp3E4[11]="fat_result";
static struct _tuple5 Cyc__gentuple_652={offsetof(struct Cyc_Absyn_MallocInfo,fat_result),(
struct _tagged_arr){_tmp3E4,_tmp3E4,_tmp3E4 + 11},(void*)((void*)& Cyc__genrep_67)};
static struct _tuple5*Cyc__genarr_653[5]={& Cyc__gentuple_648,& Cyc__gentuple_649,&
Cyc__gentuple_650,& Cyc__gentuple_651,& Cyc__gentuple_652};struct Cyc_Typerep_Struct_struct
Cyc_Absyn_malloc_info_t_rep={3,(struct _tagged_arr*)& Cyc__genname_654,sizeof(
struct Cyc_Absyn_MallocInfo),{(void*)((struct _tuple5**)Cyc__genarr_653),(void*)((
struct _tuple5**)Cyc__genarr_653),(void*)((struct _tuple5**)Cyc__genarr_653 + 5)}};
struct _tuple80{unsigned int f1;struct Cyc_Absyn_MallocInfo f2;};static struct _tuple6
Cyc__gentuple_655={offsetof(struct _tuple80,f1),(void*)& Cyc__genrep_5};static
struct _tuple6 Cyc__gentuple_656={offsetof(struct _tuple80,f2),(void*)& Cyc_Absyn_malloc_info_t_rep};
static struct _tuple6*Cyc__genarr_657[2]={& Cyc__gentuple_655,& Cyc__gentuple_656};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_647={4,sizeof(struct _tuple80),{(
void*)((struct _tuple6**)Cyc__genarr_657),(void*)((struct _tuple6**)Cyc__genarr_657),(
void*)((struct _tuple6**)Cyc__genarr_657 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_628;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_639;extern
struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0Absyn_typedef_name_t2_rep;
static char _tmp3E7[4]="Opt";static struct _tagged_arr Cyc__genname_642=(struct
_tagged_arr){_tmp3E7,_tmp3E7,_tmp3E7 + 4};static char _tmp3E8[2]="v";static struct
_tuple5 Cyc__gentuple_640={offsetof(struct Cyc_Core_Opt,v),(struct _tagged_arr){
_tmp3E8,_tmp3E8,_tmp3E8 + 2},(void*)& Cyc__genrep_10};static struct _tuple5*Cyc__genarr_641[
1]={& Cyc__gentuple_640};struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0Absyn_typedef_name_t2_rep={
3,(struct _tagged_arr*)& Cyc__genname_642,sizeof(struct Cyc_Core_Opt),{(void*)((
struct _tuple5**)Cyc__genarr_641),(void*)((struct _tuple5**)Cyc__genarr_641),(void*)((
struct _tuple5**)Cyc__genarr_641 + 1)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_639={
1,1,(void*)((void*)& Cyc_struct_Core_Opt0Absyn_typedef_name_t2_rep)};struct
_tuple81{unsigned int f1;struct Cyc_Core_Opt*f2;struct Cyc_List_List*f3;};static
struct _tuple6 Cyc__gentuple_643={offsetof(struct _tuple81,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_644={offsetof(struct _tuple81,f2),(void*)& Cyc__genrep_639};
static struct _tuple6 Cyc__gentuple_645={offsetof(struct _tuple81,f3),(void*)& Cyc__genrep_629};
static struct _tuple6*Cyc__genarr_646[3]={& Cyc__gentuple_643,& Cyc__gentuple_644,&
Cyc__gentuple_645};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_628={4,
sizeof(struct _tuple81),{(void*)((struct _tuple6**)Cyc__genarr_646),(void*)((
struct _tuple6**)Cyc__genarr_646),(void*)((struct _tuple6**)Cyc__genarr_646 + 3)}};
static struct _tuple7*Cyc__genarr_124[0]={};static char _tmp3EC[8]="Const_e";static
struct _tuple5 Cyc__gentuple_883={0,(struct _tagged_arr){_tmp3EC,_tmp3EC,_tmp3EC + 8},(
void*)& Cyc__genrep_853};static char _tmp3ED[6]="Var_e";static struct _tuple5 Cyc__gentuple_884={
1,(struct _tagged_arr){_tmp3ED,_tmp3ED,_tmp3ED + 6},(void*)& Cyc__genrep_840};
static char _tmp3EE[12]="UnknownId_e";static struct _tuple5 Cyc__gentuple_885={2,(
struct _tagged_arr){_tmp3EE,_tmp3EE,_tmp3EE + 12},(void*)& Cyc__genrep_246};static
char _tmp3EF[9]="Primop_e";static struct _tuple5 Cyc__gentuple_886={3,(struct
_tagged_arr){_tmp3EF,_tmp3EF,_tmp3EF + 9},(void*)& Cyc__genrep_835};static char
_tmp3F0[11]="AssignOp_e";static struct _tuple5 Cyc__gentuple_887={4,(struct
_tagged_arr){_tmp3F0,_tmp3F0,_tmp3F0 + 11},(void*)& Cyc__genrep_803};static char
_tmp3F1[12]="Increment_e";static struct _tuple5 Cyc__gentuple_888={5,(struct
_tagged_arr){_tmp3F1,_tmp3F1,_tmp3F1 + 12},(void*)& Cyc__genrep_792};static char
_tmp3F2[14]="Conditional_e";static struct _tuple5 Cyc__gentuple_889={6,(struct
_tagged_arr){_tmp3F2,_tmp3F2,_tmp3F2 + 14},(void*)& Cyc__genrep_786};static char
_tmp3F3[9]="SeqExp_e";static struct _tuple5 Cyc__gentuple_890={7,(struct _tagged_arr){
_tmp3F3,_tmp3F3,_tmp3F3 + 9},(void*)& Cyc__genrep_717};static char _tmp3F4[14]="UnknownCall_e";
static struct _tuple5 Cyc__gentuple_891={8,(struct _tagged_arr){_tmp3F4,_tmp3F4,
_tmp3F4 + 14},(void*)& Cyc__genrep_781};static char _tmp3F5[9]="FnCall_e";static
struct _tuple5 Cyc__gentuple_892={9,(struct _tagged_arr){_tmp3F5,_tmp3F5,_tmp3F5 + 9},(
void*)& Cyc__genrep_768};static char _tmp3F6[8]="Throw_e";static struct _tuple5 Cyc__gentuple_893={
10,(struct _tagged_arr){_tmp3F6,_tmp3F6,_tmp3F6 + 8},(void*)& Cyc__genrep_125};
static char _tmp3F7[16]="NoInstantiate_e";static struct _tuple5 Cyc__gentuple_894={11,(
struct _tagged_arr){_tmp3F7,_tmp3F7,_tmp3F7 + 16},(void*)& Cyc__genrep_125};static
char _tmp3F8[14]="Instantiate_e";static struct _tuple5 Cyc__gentuple_895={12,(struct
_tagged_arr){_tmp3F8,_tmp3F8,_tmp3F8 + 14},(void*)& Cyc__genrep_763};static char
_tmp3F9[7]="Cast_e";static struct _tuple5 Cyc__gentuple_896={13,(struct _tagged_arr){
_tmp3F9,_tmp3F9,_tmp3F9 + 7},(void*)& Cyc__genrep_750};static char _tmp3FA[10]="Address_e";
static struct _tuple5 Cyc__gentuple_897={14,(struct _tagged_arr){_tmp3FA,_tmp3FA,
_tmp3FA + 10},(void*)& Cyc__genrep_125};static char _tmp3FB[6]="New_e";static struct
_tuple5 Cyc__gentuple_898={15,(struct _tagged_arr){_tmp3FB,_tmp3FB,_tmp3FB + 6},(
void*)& Cyc__genrep_745};static char _tmp3FC[12]="Sizeoftyp_e";static struct _tuple5
Cyc__gentuple_899={16,(struct _tagged_arr){_tmp3FC,_tmp3FC,_tmp3FC + 12},(void*)&
Cyc__genrep_97};static char _tmp3FD[12]="Sizeofexp_e";static struct _tuple5 Cyc__gentuple_900={
17,(struct _tagged_arr){_tmp3FD,_tmp3FD,_tmp3FD + 12},(void*)& Cyc__genrep_125};
static char _tmp3FE[11]="Offsetof_e";static struct _tuple5 Cyc__gentuple_901={18,(
struct _tagged_arr){_tmp3FE,_tmp3FE,_tmp3FE + 11},(void*)& Cyc__genrep_732};static
char _tmp3FF[9]="Gentyp_e";static struct _tuple5 Cyc__gentuple_902={19,(struct
_tagged_arr){_tmp3FF,_tmp3FF,_tmp3FF + 9},(void*)& Cyc__genrep_727};static char
_tmp400[8]="Deref_e";static struct _tuple5 Cyc__gentuple_903={20,(struct _tagged_arr){
_tmp400,_tmp400,_tmp400 + 8},(void*)& Cyc__genrep_125};static char _tmp401[13]="AggrMember_e";
static struct _tuple5 Cyc__gentuple_904={21,(struct _tagged_arr){_tmp401,_tmp401,
_tmp401 + 13},(void*)& Cyc__genrep_722};static char _tmp402[12]="AggrArrow_e";static
struct _tuple5 Cyc__gentuple_905={22,(struct _tagged_arr){_tmp402,_tmp402,_tmp402 + 
12},(void*)& Cyc__genrep_722};static char _tmp403[12]="Subscript_e";static struct
_tuple5 Cyc__gentuple_906={23,(struct _tagged_arr){_tmp403,_tmp403,_tmp403 + 12},(
void*)& Cyc__genrep_717};static char _tmp404[8]="Tuple_e";static struct _tuple5 Cyc__gentuple_907={
24,(struct _tagged_arr){_tmp404,_tmp404,_tmp404 + 8},(void*)& Cyc__genrep_713};
static char _tmp405[14]="CompoundLit_e";static struct _tuple5 Cyc__gentuple_908={25,(
struct _tagged_arr){_tmp405,_tmp405,_tmp405 + 14},(void*)& Cyc__genrep_702};static
char _tmp406[8]="Array_e";static struct _tuple5 Cyc__gentuple_909={26,(struct
_tagged_arr){_tmp406,_tmp406,_tmp406 + 8},(void*)& Cyc__genrep_698};static char
_tmp407[16]="Comprehension_e";static struct _tuple5 Cyc__gentuple_910={27,(struct
_tagged_arr){_tmp407,_tmp407,_tmp407 + 16},(void*)& Cyc__genrep_691};static char
_tmp408[9]="Struct_e";static struct _tuple5 Cyc__gentuple_911={28,(struct
_tagged_arr){_tmp408,_tmp408,_tmp408 + 9},(void*)& Cyc__genrep_683};static char
_tmp409[13]="AnonStruct_e";static struct _tuple5 Cyc__gentuple_912={29,(struct
_tagged_arr){_tmp409,_tmp409,_tmp409 + 13},(void*)& Cyc__genrep_678};static char
_tmp40A[9]="Tunion_e";static struct _tuple5 Cyc__gentuple_913={30,(struct
_tagged_arr){_tmp40A,_tmp40A,_tmp40A + 9},(void*)& Cyc__genrep_672};static char
_tmp40B[7]="Enum_e";static struct _tuple5 Cyc__gentuple_914={31,(struct _tagged_arr){
_tmp40B,_tmp40B,_tmp40B + 7},(void*)& Cyc__genrep_665};static char _tmp40C[11]="AnonEnum_e";
static struct _tuple5 Cyc__gentuple_915={32,(struct _tagged_arr){_tmp40C,_tmp40C,
_tmp40C + 11},(void*)& Cyc__genrep_658};static char _tmp40D[9]="Malloc_e";static
struct _tuple5 Cyc__gentuple_916={33,(struct _tagged_arr){_tmp40D,_tmp40D,_tmp40D + 
9},(void*)& Cyc__genrep_647};static char _tmp40E[16]="UnresolvedMem_e";static struct
_tuple5 Cyc__gentuple_917={34,(struct _tagged_arr){_tmp40E,_tmp40E,_tmp40E + 16},(
void*)& Cyc__genrep_628};static char _tmp40F[10]="StmtExp_e";static struct _tuple5 Cyc__gentuple_918={
35,(struct _tagged_arr){_tmp40F,_tmp40F,_tmp40F + 10},(void*)& Cyc__genrep_480};
static char _tmp410[10]="Codegen_e";static struct _tuple5 Cyc__gentuple_919={36,(
struct _tagged_arr){_tmp410,_tmp410,_tmp410 + 10},(void*)& Cyc__genrep_130};static
char _tmp411[7]="Fill_e";static struct _tuple5 Cyc__gentuple_920={37,(struct
_tagged_arr){_tmp411,_tmp411,_tmp411 + 7},(void*)& Cyc__genrep_125};static struct
_tuple5*Cyc__genarr_921[38]={& Cyc__gentuple_883,& Cyc__gentuple_884,& Cyc__gentuple_885,&
Cyc__gentuple_886,& Cyc__gentuple_887,& Cyc__gentuple_888,& Cyc__gentuple_889,& Cyc__gentuple_890,&
Cyc__gentuple_891,& Cyc__gentuple_892,& Cyc__gentuple_893,& Cyc__gentuple_894,& Cyc__gentuple_895,&
Cyc__gentuple_896,& Cyc__gentuple_897,& Cyc__gentuple_898,& Cyc__gentuple_899,& Cyc__gentuple_900,&
Cyc__gentuple_901,& Cyc__gentuple_902,& Cyc__gentuple_903,& Cyc__gentuple_904,& Cyc__gentuple_905,&
Cyc__gentuple_906,& Cyc__gentuple_907,& Cyc__gentuple_908,& Cyc__gentuple_909,& Cyc__gentuple_910,&
Cyc__gentuple_911,& Cyc__gentuple_912,& Cyc__gentuple_913,& Cyc__gentuple_914,& Cyc__gentuple_915,&
Cyc__gentuple_916,& Cyc__gentuple_917,& Cyc__gentuple_918,& Cyc__gentuple_919,& Cyc__gentuple_920};
static char _tmp413[8]="Raw_exp";struct Cyc_Typerep_TUnion_struct Cyc_Absyn_raw_exp_t_rep={
5,(struct _tagged_arr){_tmp413,_tmp413,_tmp413 + 8},{(void*)((struct _tuple7**)Cyc__genarr_124),(
void*)((struct _tuple7**)Cyc__genarr_124),(void*)((struct _tuple7**)Cyc__genarr_124
+ 0)},{(void*)((struct _tuple5**)Cyc__genarr_921),(void*)((struct _tuple5**)Cyc__genarr_921),(
void*)((struct _tuple5**)Cyc__genarr_921 + 38)}};static char _tmp414[4]="Exp";static
struct _tagged_arr Cyc__genname_927=(struct _tagged_arr){_tmp414,_tmp414,_tmp414 + 4};
static char _tmp415[5]="topt";static struct _tuple5 Cyc__gentuple_922={offsetof(
struct Cyc_Absyn_Exp,topt),(struct _tagged_arr){_tmp415,_tmp415,_tmp415 + 5},(void*)&
Cyc__genrep_92};static char _tmp416[2]="r";static struct _tuple5 Cyc__gentuple_923={
offsetof(struct Cyc_Absyn_Exp,r),(struct _tagged_arr){_tmp416,_tmp416,_tmp416 + 2},(
void*)& Cyc_Absyn_raw_exp_t_rep};static char _tmp417[4]="loc";static struct _tuple5
Cyc__gentuple_924={offsetof(struct Cyc_Absyn_Exp,loc),(struct _tagged_arr){_tmp417,
_tmp417,_tmp417 + 4},(void*)& Cyc__genrep_2};static char _tmp418[6]="annot";static
struct _tuple5 Cyc__gentuple_925={offsetof(struct Cyc_Absyn_Exp,annot),(struct
_tagged_arr){_tmp418,_tmp418,_tmp418 + 6},(void*)& Cyc_Absyn_absyn_annot_t_rep};
static struct _tuple5*Cyc__genarr_926[4]={& Cyc__gentuple_922,& Cyc__gentuple_923,&
Cyc__gentuple_924,& Cyc__gentuple_925};struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Exp_rep={
3,(struct _tagged_arr*)& Cyc__genname_927,sizeof(struct Cyc_Absyn_Exp),{(void*)((
struct _tuple5**)Cyc__genarr_926),(void*)((struct _tuple5**)Cyc__genarr_926),(void*)((
struct _tuple5**)Cyc__genarr_926 + 4)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_122={
1,1,(void*)((void*)& Cyc_struct_Absyn_Exp_rep)};static char _tmp41B[10]="ArrayInfo";
static struct _tagged_arr Cyc__genname_996=(struct _tagged_arr){_tmp41B,_tmp41B,
_tmp41B + 10};static char _tmp41C[9]="elt_type";static struct _tuple5 Cyc__gentuple_991={
offsetof(struct Cyc_Absyn_ArrayInfo,elt_type),(struct _tagged_arr){_tmp41C,_tmp41C,
_tmp41C + 9},(void*)((void*)& Cyc_Absyn_type_t_rep)};static char _tmp41D[3]="tq";
static struct _tuple5 Cyc__gentuple_992={offsetof(struct Cyc_Absyn_ArrayInfo,tq),(
struct _tagged_arr){_tmp41D,_tmp41D,_tmp41D + 3},(void*)& Cyc__genrep_135};static
char _tmp41E[9]="num_elts";static struct _tuple5 Cyc__gentuple_993={offsetof(struct
Cyc_Absyn_ArrayInfo,num_elts),(struct _tagged_arr){_tmp41E,_tmp41E,_tmp41E + 9},(
void*)& Cyc__genrep_122};static char _tmp41F[10]="zero_term";static struct _tuple5 Cyc__gentuple_994={
offsetof(struct Cyc_Absyn_ArrayInfo,zero_term),(struct _tagged_arr){_tmp41F,
_tmp41F,_tmp41F + 10},(void*)& Cyc__genrep_978};static struct _tuple5*Cyc__genarr_995[
4]={& Cyc__gentuple_991,& Cyc__gentuple_992,& Cyc__gentuple_993,& Cyc__gentuple_994};
struct Cyc_Typerep_Struct_struct Cyc_Absyn_array_info_t_rep={3,(struct _tagged_arr*)&
Cyc__genname_996,sizeof(struct Cyc_Absyn_ArrayInfo),{(void*)((struct _tuple5**)Cyc__genarr_995),(
void*)((struct _tuple5**)Cyc__genarr_995),(void*)((struct _tuple5**)Cyc__genarr_995
+ 4)}};struct _tuple82{unsigned int f1;struct Cyc_Absyn_ArrayInfo f2;};static struct
_tuple6 Cyc__gentuple_997={offsetof(struct _tuple82,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_998={offsetof(struct _tuple82,f2),(void*)& Cyc_Absyn_array_info_t_rep};
static struct _tuple6*Cyc__genarr_999[2]={& Cyc__gentuple_997,& Cyc__gentuple_998};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_977={4,sizeof(struct _tuple82),{(
void*)((struct _tuple6**)Cyc__genarr_999),(void*)((struct _tuple6**)Cyc__genarr_999),(
void*)((struct _tuple6**)Cyc__genarr_999 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_958;extern struct Cyc_Typerep_Struct_struct Cyc_Absyn_fn_info_t_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_959;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List060Core_opt_t0Absyn_var_t46H24Absyn_tqual_t4Absyn_type_t1_44099_6H2_rep;
static char _tmp422[5]="List";static struct _tagged_arr Cyc__genname_963=(struct
_tagged_arr){_tmp422,_tmp422,_tmp422 + 5};static char _tmp423[3]="hd";static struct
_tuple5 Cyc__gentuple_960={offsetof(struct Cyc_List_List,hd),(struct _tagged_arr){
_tmp423,_tmp423,_tmp423 + 3},(void*)& Cyc__genrep_703};static char _tmp424[3]="tl";
static struct _tuple5 Cyc__gentuple_961={offsetof(struct Cyc_List_List,tl),(struct
_tagged_arr){_tmp424,_tmp424,_tmp424 + 3},(void*)& Cyc__genrep_959};static struct
_tuple5*Cyc__genarr_962[2]={& Cyc__gentuple_960,& Cyc__gentuple_961};struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List060Core_opt_t0Absyn_var_t46H24Absyn_tqual_t4Absyn_type_t1_44099_6H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_963,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_962),(void*)((struct _tuple5**)Cyc__genarr_962),(void*)((
struct _tuple5**)Cyc__genarr_962 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_959={
1,1,(void*)((void*)& Cyc_struct_List_List060Core_opt_t0Absyn_var_t46H24Absyn_tqual_t4Absyn_type_t1_44099_6H2_rep)};
static char _tmp427[7]="FnInfo";static struct _tagged_arr Cyc__genname_973=(struct
_tagged_arr){_tmp427,_tmp427,_tmp427 + 7};static char _tmp428[6]="tvars";static
struct _tuple5 Cyc__gentuple_964={offsetof(struct Cyc_Absyn_FnInfo,tvars),(struct
_tagged_arr){_tmp428,_tmp428,_tmp428 + 6},(void*)& Cyc__genrep_312};static char
_tmp429[7]="effect";static struct _tuple5 Cyc__gentuple_965={offsetof(struct Cyc_Absyn_FnInfo,effect),(
struct _tagged_arr){_tmp429,_tmp429,_tmp429 + 7},(void*)& Cyc__genrep_92};static
char _tmp42A[8]="ret_typ";static struct _tuple5 Cyc__gentuple_966={offsetof(struct
Cyc_Absyn_FnInfo,ret_typ),(struct _tagged_arr){_tmp42A,_tmp42A,_tmp42A + 8},(void*)((
void*)& Cyc_Absyn_type_t_rep)};static char _tmp42B[5]="args";static struct _tuple5 Cyc__gentuple_967={
offsetof(struct Cyc_Absyn_FnInfo,args),(struct _tagged_arr){_tmp42B,_tmp42B,
_tmp42B + 5},(void*)& Cyc__genrep_959};static char _tmp42C[10]="c_varargs";static
struct _tuple5 Cyc__gentuple_968={offsetof(struct Cyc_Absyn_FnInfo,c_varargs),(
struct _tagged_arr){_tmp42C,_tmp42C,_tmp42C + 10},(void*)((void*)& Cyc__genrep_67)};
static char _tmp42D[12]="cyc_varargs";static struct _tuple5 Cyc__gentuple_969={
offsetof(struct Cyc_Absyn_FnInfo,cyc_varargs),(struct _tagged_arr){_tmp42D,_tmp42D,
_tmp42D + 12},(void*)& Cyc__genrep_587};static char _tmp42E[7]="rgn_po";static struct
_tuple5 Cyc__gentuple_970={offsetof(struct Cyc_Absyn_FnInfo,rgn_po),(struct
_tagged_arr){_tmp42E,_tmp42E,_tmp42E + 7},(void*)& Cyc__genrep_371};static char
_tmp42F[11]="attributes";static struct _tuple5 Cyc__gentuple_971={offsetof(struct
Cyc_Absyn_FnInfo,attributes),(struct _tagged_arr){_tmp42F,_tmp42F,_tmp42F + 11},(
void*)& Cyc__genrep_43};static struct _tuple5*Cyc__genarr_972[8]={& Cyc__gentuple_964,&
Cyc__gentuple_965,& Cyc__gentuple_966,& Cyc__gentuple_967,& Cyc__gentuple_968,& Cyc__gentuple_969,&
Cyc__gentuple_970,& Cyc__gentuple_971};struct Cyc_Typerep_Struct_struct Cyc_Absyn_fn_info_t_rep={
3,(struct _tagged_arr*)& Cyc__genname_973,sizeof(struct Cyc_Absyn_FnInfo),{(void*)((
struct _tuple5**)Cyc__genarr_972),(void*)((struct _tuple5**)Cyc__genarr_972),(void*)((
struct _tuple5**)Cyc__genarr_972 + 8)}};struct _tuple83{unsigned int f1;struct Cyc_Absyn_FnInfo
f2;};static struct _tuple6 Cyc__gentuple_974={offsetof(struct _tuple83,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_975={offsetof(struct _tuple83,f2),(
void*)& Cyc_Absyn_fn_info_t_rep};static struct _tuple6*Cyc__genarr_976[2]={& Cyc__gentuple_974,&
Cyc__gentuple_975};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_958={4,
sizeof(struct _tuple83),{(void*)((struct _tuple6**)Cyc__genarr_976),(void*)((
struct _tuple6**)Cyc__genarr_976),(void*)((struct _tuple6**)Cyc__genarr_976 + 2)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_954;static struct _tuple6 Cyc__gentuple_955={
offsetof(struct _tuple13,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_956={
offsetof(struct _tuple13,f2),(void*)& Cyc__genrep_286};static struct _tuple6*Cyc__genarr_957[
2]={& Cyc__gentuple_955,& Cyc__gentuple_956};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_954={4,sizeof(struct _tuple13),{(void*)((struct _tuple6**)Cyc__genarr_957),(
void*)((struct _tuple6**)Cyc__genarr_957),(void*)((struct _tuple6**)Cyc__genarr_957
+ 2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_950;struct _tuple84{
unsigned int f1;struct Cyc_Absyn_AggrInfo f2;};static struct _tuple6 Cyc__gentuple_951={
offsetof(struct _tuple84,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_952={
offsetof(struct _tuple84,f2),(void*)& Cyc_Absyn_aggr_info_t_rep};static struct
_tuple6*Cyc__genarr_953[2]={& Cyc__gentuple_951,& Cyc__gentuple_952};static struct
Cyc_Typerep_Tuple_struct Cyc__genrep_950={4,sizeof(struct _tuple84),{(void*)((
struct _tuple6**)Cyc__genarr_953),(void*)((struct _tuple6**)Cyc__genarr_953),(void*)((
struct _tuple6**)Cyc__genarr_953 + 2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_945;
static struct _tuple6 Cyc__gentuple_946={offsetof(struct _tuple64,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_947={offsetof(struct _tuple64,f2),(void*)& Cyc_Absyn_aggr_kind_t_rep};
static struct _tuple6 Cyc__gentuple_948={offsetof(struct _tuple64,f3),(void*)& Cyc__genrep_358};
static struct _tuple6*Cyc__genarr_949[3]={& Cyc__gentuple_946,& Cyc__gentuple_947,&
Cyc__gentuple_948};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_945={4,
sizeof(struct _tuple64),{(void*)((struct _tuple6**)Cyc__genarr_949),(void*)((
struct _tuple6**)Cyc__genarr_949),(void*)((struct _tuple6**)Cyc__genarr_949 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_940;struct _tuple85{unsigned int
f1;struct _tuple0*f2;struct Cyc_Absyn_Enumdecl*f3;};static struct _tuple6 Cyc__gentuple_941={
offsetof(struct _tuple85,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_942={
offsetof(struct _tuple85,f2),(void*)& Cyc__genrep_10};static struct _tuple6 Cyc__gentuple_943={
offsetof(struct _tuple85,f3),(void*)& Cyc__genrep_666};static struct _tuple6*Cyc__genarr_944[
3]={& Cyc__gentuple_941,& Cyc__gentuple_942,& Cyc__gentuple_943};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_940={4,sizeof(struct _tuple85),{(void*)((struct _tuple6**)Cyc__genarr_944),(
void*)((struct _tuple6**)Cyc__genarr_944),(void*)((struct _tuple6**)Cyc__genarr_944
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_119;static struct _tuple6
Cyc__gentuple_937={offsetof(struct _tuple13,f1),(void*)& Cyc__genrep_5};static
struct _tuple6 Cyc__gentuple_938={offsetof(struct _tuple13,f2),(void*)& Cyc__genrep_120};
static struct _tuple6*Cyc__genarr_939[2]={& Cyc__gentuple_937,& Cyc__gentuple_938};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_119={4,sizeof(struct _tuple13),{(
void*)((struct _tuple6**)Cyc__genarr_939),(void*)((struct _tuple6**)Cyc__genarr_939),(
void*)((struct _tuple6**)Cyc__genarr_939 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_110;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_112;extern
struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Typedefdecl_rep;static char
_tmp437[12]="Typedefdecl";static struct _tagged_arr Cyc__genname_1134=(struct
_tagged_arr){_tmp437,_tmp437,_tmp437 + 12};static char _tmp438[5]="name";static
struct _tuple5 Cyc__gentuple_1127={offsetof(struct Cyc_Absyn_Typedefdecl,name),(
struct _tagged_arr){_tmp438,_tmp438,_tmp438 + 5},(void*)& Cyc__genrep_10};static
char _tmp439[3]="tq";static struct _tuple5 Cyc__gentuple_1128={offsetof(struct Cyc_Absyn_Typedefdecl,tq),(
struct _tagged_arr){_tmp439,_tmp439,_tmp439 + 3},(void*)& Cyc__genrep_135};static
char _tmp43A[4]="tvs";static struct _tuple5 Cyc__gentuple_1129={offsetof(struct Cyc_Absyn_Typedefdecl,tvs),(
struct _tagged_arr){_tmp43A,_tmp43A,_tmp43A + 4},(void*)& Cyc__genrep_312};static
char _tmp43B[5]="kind";static struct _tuple5 Cyc__gentuple_1130={offsetof(struct Cyc_Absyn_Typedefdecl,kind),(
struct _tagged_arr){_tmp43B,_tmp43B,_tmp43B + 5},(void*)& Cyc__genrep_1091};static
char _tmp43C[5]="defn";static struct _tuple5 Cyc__gentuple_1131={offsetof(struct Cyc_Absyn_Typedefdecl,defn),(
struct _tagged_arr){_tmp43C,_tmp43C,_tmp43C + 5},(void*)& Cyc__genrep_92};static
char _tmp43D[5]="atts";static struct _tuple5 Cyc__gentuple_1132={offsetof(struct Cyc_Absyn_Typedefdecl,atts),(
struct _tagged_arr){_tmp43D,_tmp43D,_tmp43D + 5},(void*)& Cyc__genrep_43};static
struct _tuple5*Cyc__genarr_1133[6]={& Cyc__gentuple_1127,& Cyc__gentuple_1128,& Cyc__gentuple_1129,&
Cyc__gentuple_1130,& Cyc__gentuple_1131,& Cyc__gentuple_1132};struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_Typedefdecl_rep={3,(struct _tagged_arr*)& Cyc__genname_1134,
sizeof(struct Cyc_Absyn_Typedefdecl),{(void*)((struct _tuple5**)Cyc__genarr_1133),(
void*)((struct _tuple5**)Cyc__genarr_1133),(void*)((struct _tuple5**)Cyc__genarr_1133
+ 6)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_112={1,1,(void*)((void*)&
Cyc_struct_Absyn_Typedefdecl_rep)};struct _tuple86{unsigned int f1;struct _tuple0*
f2;struct Cyc_List_List*f3;struct Cyc_Absyn_Typedefdecl*f4;void**f5;};static struct
_tuple6 Cyc__gentuple_113={offsetof(struct _tuple86,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_114={offsetof(struct _tuple86,f2),(void*)& Cyc__genrep_10};
static struct _tuple6 Cyc__gentuple_115={offsetof(struct _tuple86,f3),(void*)& Cyc__genrep_102};
static struct _tuple6 Cyc__gentuple_116={offsetof(struct _tuple86,f4),(void*)& Cyc__genrep_112};
static struct _tuple6 Cyc__gentuple_117={offsetof(struct _tuple86,f5),(void*)& Cyc__genrep_111};
static struct _tuple6*Cyc__genarr_118[5]={& Cyc__gentuple_113,& Cyc__gentuple_114,&
Cyc__gentuple_115,& Cyc__gentuple_116,& Cyc__gentuple_117};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_110={4,sizeof(struct _tuple86),{(void*)((struct _tuple6**)Cyc__genarr_118),(
void*)((struct _tuple6**)Cyc__genarr_118),(void*)((struct _tuple6**)Cyc__genarr_118
+ 5)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_101;static struct _tuple6
Cyc__gentuple_107={offsetof(struct _tuple13,f1),(void*)& Cyc__genrep_5};static
struct _tuple6 Cyc__gentuple_108={offsetof(struct _tuple13,f2),(void*)& Cyc__genrep_102};
static struct _tuple6*Cyc__genarr_109[2]={& Cyc__gentuple_107,& Cyc__gentuple_108};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_101={4,sizeof(struct _tuple13),{(
void*)((struct _tuple6**)Cyc__genarr_109),(void*)((struct _tuple6**)Cyc__genarr_109),(
void*)((struct _tuple6**)Cyc__genarr_109 + 2)}};static char _tmp442[9]="VoidType";
static struct _tuple7 Cyc__gentuple_93={0,(struct _tagged_arr){_tmp442,_tmp442,
_tmp442 + 9}};static char _tmp443[10]="FloatType";static struct _tuple7 Cyc__gentuple_94={
1,(struct _tagged_arr){_tmp443,_tmp443,_tmp443 + 10}};static char _tmp444[8]="HeapRgn";
static struct _tuple7 Cyc__gentuple_95={2,(struct _tagged_arr){_tmp444,_tmp444,
_tmp444 + 8}};static struct _tuple7*Cyc__genarr_96[3]={& Cyc__gentuple_93,& Cyc__gentuple_94,&
Cyc__gentuple_95};static char _tmp445[5]="Evar";static struct _tuple5 Cyc__gentuple_1101={
0,(struct _tagged_arr){_tmp445,_tmp445,_tmp445 + 5},(void*)& Cyc__genrep_1086};
static char _tmp446[8]="VarType";static struct _tuple5 Cyc__gentuple_1102={1,(struct
_tagged_arr){_tmp446,_tmp446,_tmp446 + 8},(void*)& Cyc__genrep_1082};static char
_tmp447[11]="TunionType";static struct _tuple5 Cyc__gentuple_1103={2,(struct
_tagged_arr){_tmp447,_tmp447,_tmp447 + 11},(void*)& Cyc__genrep_1056};static char
_tmp448[16]="TunionFieldType";static struct _tuple5 Cyc__gentuple_1104={3,(struct
_tagged_arr){_tmp448,_tmp448,_tmp448 + 16},(void*)& Cyc__genrep_1030};static char
_tmp449[12]="PointerType";static struct _tuple5 Cyc__gentuple_1105={4,(struct
_tagged_arr){_tmp449,_tmp449,_tmp449 + 12},(void*)& Cyc__genrep_1011};static char
_tmp44A[8]="IntType";static struct _tuple5 Cyc__gentuple_1106={5,(struct _tagged_arr){
_tmp44A,_tmp44A,_tmp44A + 8},(void*)& Cyc__genrep_1000};static char _tmp44B[11]="DoubleType";
static struct _tuple5 Cyc__gentuple_1107={6,(struct _tagged_arr){_tmp44B,_tmp44B,
_tmp44B + 11},(void*)& Cyc__genrep_66};static char _tmp44C[10]="ArrayType";static
struct _tuple5 Cyc__gentuple_1108={7,(struct _tagged_arr){_tmp44C,_tmp44C,_tmp44C + 
10},(void*)& Cyc__genrep_977};static char _tmp44D[7]="FnType";static struct _tuple5
Cyc__gentuple_1109={8,(struct _tagged_arr){_tmp44D,_tmp44D,_tmp44D + 7},(void*)&
Cyc__genrep_958};static char _tmp44E[10]="TupleType";static struct _tuple5 Cyc__gentuple_1110={
9,(struct _tagged_arr){_tmp44E,_tmp44E,_tmp44E + 10},(void*)& Cyc__genrep_954};
static char _tmp44F[9]="AggrType";static struct _tuple5 Cyc__gentuple_1111={10,(
struct _tagged_arr){_tmp44F,_tmp44F,_tmp44F + 9},(void*)& Cyc__genrep_950};static
char _tmp450[13]="AnonAggrType";static struct _tuple5 Cyc__gentuple_1112={11,(struct
_tagged_arr){_tmp450,_tmp450,_tmp450 + 13},(void*)& Cyc__genrep_945};static char
_tmp451[9]="EnumType";static struct _tuple5 Cyc__gentuple_1113={12,(struct
_tagged_arr){_tmp451,_tmp451,_tmp451 + 9},(void*)& Cyc__genrep_940};static char
_tmp452[13]="AnonEnumType";static struct _tuple5 Cyc__gentuple_1114={13,(struct
_tagged_arr){_tmp452,_tmp452,_tmp452 + 13},(void*)& Cyc__genrep_119};static char
_tmp453[11]="SizeofType";static struct _tuple5 Cyc__gentuple_1115={14,(struct
_tagged_arr){_tmp453,_tmp453,_tmp453 + 11},(void*)& Cyc__genrep_97};static char
_tmp454[14]="RgnHandleType";static struct _tuple5 Cyc__gentuple_1116={15,(struct
_tagged_arr){_tmp454,_tmp454,_tmp454 + 14},(void*)& Cyc__genrep_97};static char
_tmp455[12]="TypedefType";static struct _tuple5 Cyc__gentuple_1117={16,(struct
_tagged_arr){_tmp455,_tmp455,_tmp455 + 12},(void*)& Cyc__genrep_110};static char
_tmp456[8]="TagType";static struct _tuple5 Cyc__gentuple_1118={17,(struct
_tagged_arr){_tmp456,_tmp456,_tmp456 + 8},(void*)& Cyc__genrep_97};static char
_tmp457[8]="TypeInt";static struct _tuple5 Cyc__gentuple_1119={18,(struct
_tagged_arr){_tmp457,_tmp457,_tmp457 + 8},(void*)& Cyc__genrep_66};static char
_tmp458[10]="AccessEff";static struct _tuple5 Cyc__gentuple_1120={19,(struct
_tagged_arr){_tmp458,_tmp458,_tmp458 + 10},(void*)& Cyc__genrep_97};static char
_tmp459[8]="JoinEff";static struct _tuple5 Cyc__gentuple_1121={20,(struct
_tagged_arr){_tmp459,_tmp459,_tmp459 + 8},(void*)& Cyc__genrep_101};static char
_tmp45A[8]="RgnsEff";static struct _tuple5 Cyc__gentuple_1122={21,(struct
_tagged_arr){_tmp45A,_tmp45A,_tmp45A + 8},(void*)& Cyc__genrep_97};static struct
_tuple5*Cyc__genarr_1123[22]={& Cyc__gentuple_1101,& Cyc__gentuple_1102,& Cyc__gentuple_1103,&
Cyc__gentuple_1104,& Cyc__gentuple_1105,& Cyc__gentuple_1106,& Cyc__gentuple_1107,&
Cyc__gentuple_1108,& Cyc__gentuple_1109,& Cyc__gentuple_1110,& Cyc__gentuple_1111,&
Cyc__gentuple_1112,& Cyc__gentuple_1113,& Cyc__gentuple_1114,& Cyc__gentuple_1115,&
Cyc__gentuple_1116,& Cyc__gentuple_1117,& Cyc__gentuple_1118,& Cyc__gentuple_1119,&
Cyc__gentuple_1120,& Cyc__gentuple_1121,& Cyc__gentuple_1122};static char _tmp45C[5]="Type";
struct Cyc_Typerep_TUnion_struct Cyc_Absyn_type_t_rep={5,(struct _tagged_arr){
_tmp45C,_tmp45C,_tmp45C + 5},{(void*)((struct _tuple7**)Cyc__genarr_96),(void*)((
struct _tuple7**)Cyc__genarr_96),(void*)((struct _tuple7**)Cyc__genarr_96 + 3)},{(
void*)((struct _tuple5**)Cyc__genarr_1123),(void*)((struct _tuple5**)Cyc__genarr_1123),(
void*)((struct _tuple5**)Cyc__genarr_1123 + 22)}};static char _tmp45D[8]="Vardecl";
static struct _tagged_arr Cyc__genname_155=(struct _tagged_arr){_tmp45D,_tmp45D,
_tmp45D + 8};static char _tmp45E[3]="sc";static struct _tuple5 Cyc__gentuple_146={
offsetof(struct Cyc_Absyn_Vardecl,sc),(struct _tagged_arr){_tmp45E,_tmp45E,_tmp45E
+ 3},(void*)& Cyc_Absyn_scope_t_rep};static char _tmp45F[5]="name";static struct
_tuple5 Cyc__gentuple_147={offsetof(struct Cyc_Absyn_Vardecl,name),(struct
_tagged_arr){_tmp45F,_tmp45F,_tmp45F + 5},(void*)& Cyc__genrep_10};static char
_tmp460[3]="tq";static struct _tuple5 Cyc__gentuple_148={offsetof(struct Cyc_Absyn_Vardecl,tq),(
struct _tagged_arr){_tmp460,_tmp460,_tmp460 + 3},(void*)& Cyc__genrep_135};static
char _tmp461[5]="type";static struct _tuple5 Cyc__gentuple_149={offsetof(struct Cyc_Absyn_Vardecl,type),(
struct _tagged_arr){_tmp461,_tmp461,_tmp461 + 5},(void*)((void*)& Cyc_Absyn_type_t_rep)};
static char _tmp462[12]="initializer";static struct _tuple5 Cyc__gentuple_150={
offsetof(struct Cyc_Absyn_Vardecl,initializer),(struct _tagged_arr){_tmp462,
_tmp462,_tmp462 + 12},(void*)& Cyc__genrep_122};static char _tmp463[4]="rgn";static
struct _tuple5 Cyc__gentuple_151={offsetof(struct Cyc_Absyn_Vardecl,rgn),(struct
_tagged_arr){_tmp463,_tmp463,_tmp463 + 4},(void*)& Cyc__genrep_92};static char
_tmp464[11]="attributes";static struct _tuple5 Cyc__gentuple_152={offsetof(struct
Cyc_Absyn_Vardecl,attributes),(struct _tagged_arr){_tmp464,_tmp464,_tmp464 + 11},(
void*)& Cyc__genrep_43};static char _tmp465[8]="escapes";static struct _tuple5 Cyc__gentuple_153={
offsetof(struct Cyc_Absyn_Vardecl,escapes),(struct _tagged_arr){_tmp465,_tmp465,
_tmp465 + 8},(void*)((void*)& Cyc__genrep_67)};static struct _tuple5*Cyc__genarr_154[
8]={& Cyc__gentuple_146,& Cyc__gentuple_147,& Cyc__gentuple_148,& Cyc__gentuple_149,&
Cyc__gentuple_150,& Cyc__gentuple_151,& Cyc__gentuple_152,& Cyc__gentuple_153};
struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Vardecl_rep={3,(struct
_tagged_arr*)& Cyc__genname_155,sizeof(struct Cyc_Absyn_Vardecl),{(void*)((struct
_tuple5**)Cyc__genarr_154),(void*)((struct _tuple5**)Cyc__genarr_154),(void*)((
struct _tuple5**)Cyc__genarr_154 + 8)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_134={
1,1,(void*)((void*)& Cyc_struct_Absyn_Vardecl_rep)};struct _tuple87{unsigned int f1;
struct Cyc_Absyn_Vardecl*f2;};static struct _tuple6 Cyc__gentuple_432={offsetof(
struct _tuple87,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_433={
offsetof(struct _tuple87,f2),(void*)& Cyc__genrep_134};static struct _tuple6*Cyc__genarr_434[
2]={& Cyc__gentuple_432,& Cyc__gentuple_433};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_431={4,sizeof(struct _tuple87),{(void*)((struct _tuple6**)Cyc__genarr_434),(
void*)((struct _tuple6**)Cyc__genarr_434),(void*)((struct _tuple6**)Cyc__genarr_434
+ 2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_1154;struct _tuple88{
unsigned int f1;struct Cyc_Absyn_Pat*f2;struct Cyc_Core_Opt*f3;struct Cyc_Absyn_Exp*
f4;};static struct _tuple6 Cyc__gentuple_1155={offsetof(struct _tuple88,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_1156={offsetof(struct _tuple88,f2),(
void*)& Cyc__genrep_232};static struct _tuple6 Cyc__gentuple_1157={offsetof(struct
_tuple88,f3),(void*)& Cyc__genrep_132};static struct _tuple6 Cyc__gentuple_1158={
offsetof(struct _tuple88,f4),(void*)& Cyc__genrep_126};static struct _tuple6*Cyc__genarr_1159[
4]={& Cyc__gentuple_1155,& Cyc__gentuple_1156,& Cyc__gentuple_1157,& Cyc__gentuple_1158};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_1154={4,sizeof(struct _tuple88),{(
void*)((struct _tuple6**)Cyc__genarr_1159),(void*)((struct _tuple6**)Cyc__genarr_1159),(
void*)((struct _tuple6**)Cyc__genarr_1159 + 4)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_1150;static struct _tuple6 Cyc__gentuple_1151={offsetof(struct _tuple13,f1),(
void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_1152={offsetof(struct
_tuple13,f2),(void*)& Cyc__genrep_133};static struct _tuple6*Cyc__genarr_1153[2]={&
Cyc__gentuple_1151,& Cyc__gentuple_1152};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_1150={
4,sizeof(struct _tuple13),{(void*)((struct _tuple6**)Cyc__genarr_1153),(void*)((
struct _tuple6**)Cyc__genarr_1153),(void*)((struct _tuple6**)Cyc__genarr_1153 + 2)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_1146;struct _tuple89{unsigned int
f1;struct Cyc_Absyn_Aggrdecl*f2;};static struct _tuple6 Cyc__gentuple_1147={
offsetof(struct _tuple89,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_1148={
offsetof(struct _tuple89,f2),(void*)((void*)& Cyc__genrep_356)};static struct
_tuple6*Cyc__genarr_1149[2]={& Cyc__gentuple_1147,& Cyc__gentuple_1148};static
struct Cyc_Typerep_Tuple_struct Cyc__genrep_1146={4,sizeof(struct _tuple89),{(void*)((
struct _tuple6**)Cyc__genarr_1149),(void*)((struct _tuple6**)Cyc__genarr_1149),(
void*)((struct _tuple6**)Cyc__genarr_1149 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_1142;struct _tuple90{unsigned int f1;struct Cyc_Absyn_Tuniondecl*f2;};
static struct _tuple6 Cyc__gentuple_1143={offsetof(struct _tuple90,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_1144={offsetof(struct _tuple90,f2),(void*)((void*)&
Cyc__genrep_302)};static struct _tuple6*Cyc__genarr_1145[2]={& Cyc__gentuple_1143,&
Cyc__gentuple_1144};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_1142={4,
sizeof(struct _tuple90),{(void*)((struct _tuple6**)Cyc__genarr_1145),(void*)((
struct _tuple6**)Cyc__genarr_1145),(void*)((struct _tuple6**)Cyc__genarr_1145 + 2)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_1138;struct _tuple91{unsigned int
f1;struct Cyc_Absyn_Enumdecl*f2;};static struct _tuple6 Cyc__gentuple_1139={
offsetof(struct _tuple91,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_1140={
offsetof(struct _tuple91,f2),(void*)& Cyc__genrep_256};static struct _tuple6*Cyc__genarr_1141[
2]={& Cyc__gentuple_1139,& Cyc__gentuple_1140};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_1138={4,sizeof(struct _tuple91),{(void*)((struct _tuple6**)Cyc__genarr_1141),(
void*)((struct _tuple6**)Cyc__genarr_1141),(void*)((struct _tuple6**)Cyc__genarr_1141
+ 2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_41;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_42;static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_42={1,1,(void*)((
void*)& Cyc_struct_Absyn_Typedefdecl_rep)};struct _tuple92{unsigned int f1;struct
Cyc_Absyn_Typedefdecl*f2;};static struct _tuple6 Cyc__gentuple_1135={offsetof(
struct _tuple92,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_1136={
offsetof(struct _tuple92,f2),(void*)& Cyc__genrep_42};static struct _tuple6*Cyc__genarr_1137[
2]={& Cyc__gentuple_1135,& Cyc__gentuple_1136};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_41={4,sizeof(struct _tuple92),{(void*)((struct _tuple6**)Cyc__genarr_1137),(
void*)((struct _tuple6**)Cyc__genarr_1137),(void*)((struct _tuple6**)Cyc__genarr_1137
+ 2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_36;struct _tuple93{
unsigned int f1;struct _tagged_arr*f2;struct Cyc_List_List*f3;};static struct _tuple6
Cyc__gentuple_37={offsetof(struct _tuple93,f1),(void*)& Cyc__genrep_5};static
struct _tuple6 Cyc__gentuple_38={offsetof(struct _tuple93,f2),(void*)& Cyc__genrep_12};
static struct _tuple6 Cyc__gentuple_39={offsetof(struct _tuple93,f3),(void*)& Cyc__genrep_0};
static struct _tuple6*Cyc__genarr_40[3]={& Cyc__gentuple_37,& Cyc__gentuple_38,& Cyc__gentuple_39};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_36={4,sizeof(struct _tuple93),{(
void*)((struct _tuple6**)Cyc__genarr_40),(void*)((struct _tuple6**)Cyc__genarr_40),(
void*)((struct _tuple6**)Cyc__genarr_40 + 3)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_9;static struct _tuple6 Cyc__gentuple_32={offsetof(struct _tuple54,f1),(
void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_33={offsetof(struct
_tuple54,f2),(void*)& Cyc__genrep_10};static struct _tuple6 Cyc__gentuple_34={
offsetof(struct _tuple54,f3),(void*)& Cyc__genrep_0};static struct _tuple6*Cyc__genarr_35[
3]={& Cyc__gentuple_32,& Cyc__gentuple_33,& Cyc__gentuple_34};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_9={4,sizeof(struct _tuple54),{(void*)((struct _tuple6**)Cyc__genarr_35),(
void*)((struct _tuple6**)Cyc__genarr_35),(void*)((struct _tuple6**)Cyc__genarr_35 + 
3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_4;static struct _tuple6 Cyc__gentuple_6={
offsetof(struct _tuple13,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_7={
offsetof(struct _tuple13,f2),(void*)& Cyc__genrep_0};static struct _tuple6*Cyc__genarr_8[
2]={& Cyc__gentuple_6,& Cyc__gentuple_7};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_4={
4,sizeof(struct _tuple13),{(void*)((struct _tuple6**)Cyc__genarr_8),(void*)((
struct _tuple6**)Cyc__genarr_8),(void*)((struct _tuple6**)Cyc__genarr_8 + 2)}};
static struct _tuple7*Cyc__genarr_3[0]={};static char _tmp473[6]="Var_d";static
struct _tuple5 Cyc__gentuple_1160={0,(struct _tagged_arr){_tmp473,_tmp473,_tmp473 + 
6},(void*)& Cyc__genrep_431};static char _tmp474[5]="Fn_d";static struct _tuple5 Cyc__gentuple_1161={
1,(struct _tagged_arr){_tmp474,_tmp474,_tmp474 + 5},(void*)& Cyc__genrep_130};
static char _tmp475[6]="Let_d";static struct _tuple5 Cyc__gentuple_1162={2,(struct
_tagged_arr){_tmp475,_tmp475,_tmp475 + 6},(void*)& Cyc__genrep_1154};static char
_tmp476[7]="Letv_d";static struct _tuple5 Cyc__gentuple_1163={3,(struct _tagged_arr){
_tmp476,_tmp476,_tmp476 + 7},(void*)& Cyc__genrep_1150};static char _tmp477[7]="Aggr_d";
static struct _tuple5 Cyc__gentuple_1164={4,(struct _tagged_arr){_tmp477,_tmp477,
_tmp477 + 7},(void*)& Cyc__genrep_1146};static char _tmp478[9]="Tunion_d";static
struct _tuple5 Cyc__gentuple_1165={5,(struct _tagged_arr){_tmp478,_tmp478,_tmp478 + 
9},(void*)& Cyc__genrep_1142};static char _tmp479[7]="Enum_d";static struct _tuple5
Cyc__gentuple_1166={6,(struct _tagged_arr){_tmp479,_tmp479,_tmp479 + 7},(void*)&
Cyc__genrep_1138};static char _tmp47A[10]="Typedef_d";static struct _tuple5 Cyc__gentuple_1167={
7,(struct _tagged_arr){_tmp47A,_tmp47A,_tmp47A + 10},(void*)& Cyc__genrep_41};
static char _tmp47B[12]="Namespace_d";static struct _tuple5 Cyc__gentuple_1168={8,(
struct _tagged_arr){_tmp47B,_tmp47B,_tmp47B + 12},(void*)& Cyc__genrep_36};static
char _tmp47C[8]="Using_d";static struct _tuple5 Cyc__gentuple_1169={9,(struct
_tagged_arr){_tmp47C,_tmp47C,_tmp47C + 8},(void*)& Cyc__genrep_9};static char
_tmp47D[10]="ExternC_d";static struct _tuple5 Cyc__gentuple_1170={10,(struct
_tagged_arr){_tmp47D,_tmp47D,_tmp47D + 10},(void*)& Cyc__genrep_4};static struct
_tuple5*Cyc__genarr_1171[11]={& Cyc__gentuple_1160,& Cyc__gentuple_1161,& Cyc__gentuple_1162,&
Cyc__gentuple_1163,& Cyc__gentuple_1164,& Cyc__gentuple_1165,& Cyc__gentuple_1166,&
Cyc__gentuple_1167,& Cyc__gentuple_1168,& Cyc__gentuple_1169,& Cyc__gentuple_1170};
static char _tmp47F[9]="Raw_decl";struct Cyc_Typerep_TUnion_struct Cyc_Absyn_raw_decl_t_rep={
5,(struct _tagged_arr){_tmp47F,_tmp47F,_tmp47F + 9},{(void*)((struct _tuple7**)Cyc__genarr_3),(
void*)((struct _tuple7**)Cyc__genarr_3),(void*)((struct _tuple7**)Cyc__genarr_3 + 0)},{(
void*)((struct _tuple5**)Cyc__genarr_1171),(void*)((struct _tuple5**)Cyc__genarr_1171),(
void*)((struct _tuple5**)Cyc__genarr_1171 + 11)}};static char _tmp480[5]="Decl";
static struct _tagged_arr Cyc__genname_1175=(struct _tagged_arr){_tmp480,_tmp480,
_tmp480 + 5};static char _tmp481[2]="r";static struct _tuple5 Cyc__gentuple_1172={
offsetof(struct Cyc_Absyn_Decl,r),(struct _tagged_arr){_tmp481,_tmp481,_tmp481 + 2},(
void*)& Cyc_Absyn_raw_decl_t_rep};static char _tmp482[4]="loc";static struct _tuple5
Cyc__gentuple_1173={offsetof(struct Cyc_Absyn_Decl,loc),(struct _tagged_arr){
_tmp482,_tmp482,_tmp482 + 4},(void*)& Cyc__genrep_2};static struct _tuple5*Cyc__genarr_1174[
2]={& Cyc__gentuple_1172,& Cyc__gentuple_1173};struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Decl_rep={
3,(struct _tagged_arr*)& Cyc__genname_1175,sizeof(struct Cyc_Absyn_Decl),{(void*)((
struct _tuple5**)Cyc__genarr_1174),(void*)((struct _tuple5**)Cyc__genarr_1174),(
void*)((struct _tuple5**)Cyc__genarr_1174 + 2)}};static struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_1={1,1,(void*)((void*)& Cyc_struct_Absyn_Decl_rep)};static char _tmp485[
5]="List";static struct _tagged_arr Cyc__genname_1179=(struct _tagged_arr){_tmp485,
_tmp485,_tmp485 + 5};static char _tmp486[3]="hd";static struct _tuple5 Cyc__gentuple_1176={
offsetof(struct Cyc_List_List,hd),(struct _tagged_arr){_tmp486,_tmp486,_tmp486 + 3},(
void*)& Cyc__genrep_1};static char _tmp487[3]="tl";static struct _tuple5 Cyc__gentuple_1177={
offsetof(struct Cyc_List_List,tl),(struct _tagged_arr){_tmp487,_tmp487,_tmp487 + 3},(
void*)& Cyc__genrep_0};static struct _tuple5*Cyc__genarr_1178[2]={& Cyc__gentuple_1176,&
Cyc__gentuple_1177};struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_decl_t46H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_1179,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_1178),(void*)((struct _tuple5**)Cyc__genarr_1178),(
void*)((struct _tuple5**)Cyc__genarr_1178 + 2)}};static struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_0={1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_decl_t46H2_rep)};
void*Cyc_decls_rep=(void*)& Cyc__genrep_0;
