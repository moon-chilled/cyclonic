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
#ifdef NO_CYC_NULL_CHECKS
#define _check_null(ptr) (ptr)
#else
#define _check_null(ptr) \
  ({ void*_check_null_temp = (void*)(ptr); \
     if (!_check_null_temp) _throw_null(); \
     _check_null_temp; })
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_known_subscript_null(ptr,bound,elt_sz,index) ({ \
  ((char *)ptr) + (elt_sz)*(index); })
#else
#define _check_known_subscript_null(ptr,bound,elt_sz,index) ({ \
  void*_cks_ptr = (void*)(ptr); \
  unsigned _cks_bound = (bound); \
  unsigned _cks_elt_sz = (elt_sz); \
  unsigned _cks_index = (index); \
  if (!_cks_ptr) _throw_null(); \
  if (!_cks_index >= _cks_bound) _throw_arraybounds(); \
  ((char *)cks_ptr) + cks_elt_sz*cks_index; })
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_known_subscript_notnull(bound,index) (index)
#else
#define _check_known_subscript_notnull(bound,index) ({ \
  unsigned _cksnn_bound = (bound); \
  unsigned _cksnn_index = (index); \
  if (_cksnn_index >= _cksnn_bound) _throw_arraybounds(); \
  _cksnn_index; })
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_unknown_subscript(arr,elt_sz,index) ({ \
  struct _tagged_arr _cus_arr = (arr); \
  unsigned _cus_elt_sz = (elt_sz); \
  unsigned _cus_index = (index); \
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index; \
  _cus_ans; })
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

#define _tag_arr(tcurr,elt_sz,num_elts) ({ \
  struct _tagged_arr _tag_arr_ans; \
  _tag_arr_ans.base = _tag_arr_ans.curr = (void*)(tcurr); \
  _tag_arr_ans.last_plus_one = _tag_arr_ans.base + (elt_sz) * (num_elts); \
  _tag_arr_ans; })

#define _init_tag_arr(arr_ptr,arr,elt_sz,num_elts) ({ \
  struct _tagged_arr *_itarr_ptr = (arr_ptr); \
  void* _itarr = (arr); \
  _itarr_ptr->base = _itarr_ptr->curr = _itarr; \
  _itarr_ptr->last_plus_one = ((char *)_itarr) + (elt_sz) * (num_elts); \
  _itarr_ptr; })

#ifdef NO_CYC_BOUNDS_CHECKS
#define _untag_arr(arr,elt_sz,num_elts) ((arr).curr)
#else
#define _untag_arr(arr,elt_sz,num_elts) ({ \
  struct _tagged_arr _arr = (arr); \
  unsigned char *_curr = _arr.curr; \
  if (_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one)\
    _throw_arraybounds(); \
  _curr; })
#endif

#define _get_arr_size(arr,elt_sz) \
  ({struct _tagged_arr _get_arr_size_temp = (arr); \
    unsigned char *_get_arr_size_curr=_get_arr_size_temp.curr; \
    unsigned char *_get_arr_size_last=_get_arr_size_temp.last_plus_one; \
    (_get_arr_size_curr < _get_arr_size_temp.base || \
     _get_arr_size_curr >= _get_arr_size_last) ? 0 : \
    ((_get_arr_size_last - _get_arr_size_curr) / (elt_sz));})

#define _tagged_arr_plus(arr,elt_sz,change) ({ \
  struct _tagged_arr _ans = (arr); \
  _ans.curr += ((int)(elt_sz))*(change); \
  _ans; })

#define _tagged_arr_inplace_plus(arr_ptr,elt_sz,change) ({ \
  struct _tagged_arr * _arr_ptr = (arr_ptr); \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  *_arr_ptr; })

#define _tagged_arr_inplace_plus_post(arr_ptr,elt_sz,change) ({ \
  struct _tagged_arr * _arr_ptr = (arr_ptr); \
  struct _tagged_arr _ans = *_arr_ptr; \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  _ans; })

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
 struct Cyc_Std__types_fd_set{int fds_bits[2];};struct Cyc_Core_Opt{void*v;};extern
char Cyc_Core_Invalid_argument[21];struct Cyc_Core_Invalid_argument_struct{char*
tag;struct _tagged_arr f1;};extern char Cyc_Core_Failure[12];struct Cyc_Core_Failure_struct{
char*tag;struct _tagged_arr f1;};extern char Cyc_Core_Impossible[15];struct Cyc_Core_Impossible_struct{
char*tag;struct _tagged_arr f1;};extern char Cyc_Core_Not_found[14];extern char Cyc_Core_Unreachable[
16];struct Cyc_Core_Unreachable_struct{char*tag;struct _tagged_arr f1;};char*
string_to_Cstring(struct _tagged_arr);char*underlying_Cstring(struct _tagged_arr);
struct _tagged_arr Cstring_to_string(char*);struct _tagged_arr
wrap_Cstring_as_string(char*,unsigned int);struct _tagged_arr ntCsl_to_ntsl(char**);
struct Cyc_Std_dirent{int d_ino;int d_off;unsigned short d_reclen;char d_type;char
d_name[256];};void*Cyc_Std_opendir(struct _tagged_arr name);void*opendir(char*name);
void*Cyc_Std_opendir(struct _tagged_arr name){return opendir(string_to_Cstring(name));}
struct Cyc_Std_flock{short l_type;int l_start;short l_whence;int l_len;int l_pid;};
struct Cyc_Std_Flock_struct{int tag;struct Cyc_Std_flock*f1;};struct Cyc_Std_Long_struct{
int tag;int f1;};int Cyc_Std_fcntl(int fd,int cmd,struct _tagged_arr);int Cyc_Std_open(
struct _tagged_arr,int,struct _tagged_arr);int Cyc_Std_creat(struct _tagged_arr,int);
int fcntl(int fd,int cmd);int fcntl_with_arg(int fd,int cmd,int arg);int fcntl_with_lock(
int fd,int cmd,struct Cyc_Std_flock*lock);int creat(char*,int);int open_without_mode(
char*,int);int open_with_mode(char*,int,int);int Cyc_Std_fcntl(int fd,int cmd,struct
_tagged_arr argv){if(_get_arr_size(argv,sizeof(void*))== 0)return fcntl(fd,cmd);
else{if(_get_arr_size(argv,sizeof(void*))!= 1)(int)_throw((void*)({struct Cyc_Core_Failure_struct*
_tmp0=_cycalloc(sizeof(*_tmp0));_tmp0[0]=({struct Cyc_Core_Failure_struct _tmp1;
_tmp1.tag=Cyc_Core_Failure;_tmp1.f1=_tag_arr("fcntl: too many args",sizeof(char),
21);_tmp1;});_tmp0;}));else{void*_tmp2=*((void**)_check_unknown_subscript(argv,
sizeof(void*),0));int _tmp3;struct Cyc_Std_flock*_tmp4;_LL1: if(*((int*)_tmp2)!= 1)
goto _LL3;_tmp3=((struct Cyc_Std_Long_struct*)_tmp2)->f1;_LL2: return fcntl_with_arg(
fd,cmd,_tmp3);_LL3: if(*((int*)_tmp2)!= 0)goto _LL0;_tmp4=((struct Cyc_Std_Flock_struct*)
_tmp2)->f1;_LL4: return fcntl_with_lock(fd,cmd,(struct Cyc_Std_flock*)_tmp4);_LL0:;}}}
int Cyc_Std_creat(struct _tagged_arr s,int m){return creat(string_to_Cstring(s),m);}
int Cyc_Std_open(struct _tagged_arr s,int i,struct _tagged_arr ms){if(_get_arr_size(ms,
sizeof(int))>= 1)return open_with_mode(string_to_Cstring(s),i,*((int*)
_check_unknown_subscript(ms,sizeof(int),0)));else{return open_without_mode(
string_to_Cstring(s),i);}}struct Cyc_Std_timeval{int tv_sec;int tv_usec;};struct Cyc_Std_timezone{
int tz_minuteswest;int tz_dsttime;};struct Cyc_Std_itimerval{struct Cyc_Std_timeval
it_interval;struct Cyc_Std_timeval it_value;};typedef struct{unsigned int __val[64];}
Cyc_Std___sigset_t;typedef Cyc_Std___sigset_t Cyc_Std_sigset_t;struct Cyc_Std_timespec{
int tv_sec;int tv_nsec;};struct Cyc_Std_timeval;struct Cyc_Std_sockaddr_in;struct Cyc_Std_sockaddr{
unsigned short sa_family;char sa_data[14];};struct Cyc_Std_SA_sockaddr_in_struct{
int tag;struct Cyc_Std_sockaddr_in*f1;};struct Cyc_Std_SA_sockaddr_struct{int tag;
struct Cyc_Std_sockaddr*f1;};struct Cyc_Std_SA_socklenptr_struct{int tag;
unsigned int*f1;};struct Cyc_Std_SA_socklen_struct{int tag;unsigned int f1;};int Cyc_Std_accept(
int fd,struct _tagged_arr);int Cyc_Std_bind(int fd,struct _tagged_arr);int Cyc_Std_connect(
int fd,struct _tagged_arr);int Cyc_Std_getpeername(int fd,struct _tagged_arr);int Cyc_Std_getsockname(
int fd,struct _tagged_arr);int Cyc_Std_send(int fd,struct _tagged_arr buf,unsigned int
n,int flags);int Cyc_Std_recv(int fd,struct _tagged_arr buf,unsigned int n,int flags);
int Cyc_Std_sendto(int fd,struct _tagged_arr buf,unsigned int n,int flags,struct
_tagged_arr);int Cyc_Std_recvfrom(int fd,struct _tagged_arr buf,unsigned int n,int
flags,struct _tagged_arr);struct Cyc_Std_SO_int_struct{int tag;int*f1;};struct Cyc_Std_SO_timeval_struct{
int tag;struct Cyc_Std_timeval*f1;};struct Cyc_Std_SO_socklenptr_struct{int tag;
unsigned int*f1;};struct Cyc_Std_SO_socklen_struct{int tag;unsigned int f1;};int Cyc_Std_getsockopt(
int fd,int level,int optname,struct _tagged_arr);int Cyc_Std_setsockopt(int fd,int
level,int optname,struct _tagged_arr);struct Cyc_Std_in_addr{unsigned int s_addr;};
struct Cyc_Std_sockaddr_in{unsigned short sin_family;unsigned short sin_port;struct
Cyc_Std_in_addr sin_addr;char sin_zero[8];};struct Cyc_Std_servent{struct
_tagged_arr s_name;struct _tagged_arr s_aliases;unsigned short s_port;struct
_tagged_arr s_proto;};struct Cyc_Std_hostent{struct _tagged_arr h_name;struct
_tagged_arr h_aliases;int h_addrtype;int h_length;struct _tagged_arr h_addr_list;};
struct Cyc_Std_protoent{struct _tagged_arr p_name;struct _tagged_arr p_aliases;int
p_proto;};struct Cyc_Std_servent*Cyc_Std_getservbyname(struct _tagged_arr name,
struct _tagged_arr proto);struct Cyc_Std_hostent*Cyc_Std_gethostbyname(struct
_tagged_arr name);struct Cyc_Std_protoent*Cyc_Std_getprotobyname(struct _tagged_arr
name);void Cyc_Std_herror(struct _tagged_arr);struct Cyc_Cnetdb_Cservent{char*
s_name;char**s_aliases;unsigned short s_port;char*s_proto;};struct Cyc_Cnetdb_Chostent{
char*h_name;char**h_aliases;short h_addrtype;short h_length;struct Cyc_Std_in_addr**
h_addr_list;};struct Cyc_Cnetdb_Cprotoent{char*p_name;char**p_aliases;int p_proto;
};struct Cyc_Cnetdb_Cservent*getservbyname(char*name,char*proto);struct Cyc_Cnetdb_Chostent*
gethostbyname(char*name);struct Cyc_Cnetdb_Cprotoent*getprotobyname(char*name);
void herror(char*);struct _tagged_arr pntlp_toCyc(struct Cyc_Std_in_addr**);struct
Cyc_Std_servent*Cyc_Std_getservbyname(struct _tagged_arr name,struct _tagged_arr
proto){struct Cyc_Cnetdb_Cservent*src=getservbyname(string_to_Cstring(name),
string_to_Cstring(proto));return(unsigned int)src?({struct Cyc_Std_servent*_tmp5=
_cycalloc(sizeof(*_tmp5));_tmp5->s_name=Cstring_to_string(src->s_name);_tmp5->s_aliases=
ntCsl_to_ntsl(src->s_aliases);_tmp5->s_port=src->s_port;_tmp5->s_proto=
Cstring_to_string(src->s_proto);_tmp5;}): 0;}struct Cyc_Std_hostent*Cyc_Std_gethostbyname(
struct _tagged_arr name){struct Cyc_Cnetdb_Chostent*src=gethostbyname(
string_to_Cstring(name));return(unsigned int)src?({struct Cyc_Std_hostent*_tmp6=
_cycalloc(sizeof(*_tmp6));_tmp6->h_name=Cstring_to_string(src->h_name);_tmp6->h_aliases=
ntCsl_to_ntsl(src->h_aliases);_tmp6->h_addrtype=(int)src->h_addrtype;_tmp6->h_length=(
int)src->h_length;_tmp6->h_addr_list=pntlp_toCyc(src->h_addr_list);_tmp6;}): 0;}
struct Cyc_Std_protoent*Cyc_Std_getprotobyname(struct _tagged_arr name){struct Cyc_Cnetdb_Cprotoent*
src=getprotobyname(string_to_Cstring(name));return(unsigned int)src?({struct Cyc_Std_protoent*
_tmp7=_cycalloc(sizeof(*_tmp7));_tmp7->p_name=Cstring_to_string(src->p_name);
_tmp7->p_aliases=ntCsl_to_ntsl(src->p_aliases);_tmp7->p_proto=src->p_proto;_tmp7;}):
0;}void Cyc_Std_herror(struct _tagged_arr s){herror(string_to_Cstring(s));}char Cyc_Std_sockaddr_in[
16]="\000\000\000\000sockaddr_in";struct Cyc_Std_sockaddr_in_struct{char*tag;
struct Cyc_Std_sockaddr_in f1;};int Cyc_Std_inet_aton(struct _tagged_arr cp,struct Cyc_Std_in_addr*
inp);struct _tagged_arr Cyc_Std_inet_ntoa(struct Cyc_Std_in_addr);unsigned int Cyc_Std_inet_addr(
struct _tagged_arr addr);int inet_aton(char*cp,struct Cyc_Std_in_addr*inp);char*
inet_ntoa(struct Cyc_Std_in_addr);unsigned int inet_addr(char*);int Cyc_Std_inet_aton(
struct _tagged_arr cp,struct Cyc_Std_in_addr*inp){return inet_aton(string_to_Cstring(
cp),inp);}struct _tagged_arr Cyc_Std_inet_ntoa(struct Cyc_Std_in_addr x){return
wrap_Cstring_as_string(inet_ntoa(x),- 1);}unsigned int Cyc_Std_inet_addr(struct
_tagged_arr addr){return inet_addr(string_to_Cstring(addr));}void(*Cyc_Std_signal(
int sig,void(*func)(int)))(int);void Cyc_Std__SIG_DFL(int);void Cyc_Std__SIG_IGN(
int);void Cyc_Std__SIG_ERR(int);void(*signal_func(int sig,void(*func)(int)))(int);
void Cyc_Std__SIG_DFL(int n){;}void Cyc_Std__SIG_IGN(int n){;}void Cyc_Std__SIG_ERR(
int n){;}void(*Cyc_Std_signal(int sig,void(*func)(int)))(int){return signal_func(
sig,func);}struct Cyc_Std_tm{int tm_sec;int tm_min;int tm_hour;int tm_mday;int tm_mon;
int tm_year;int tm_wday;int tm_yday;int tm_isdst;};struct _tagged_arr Cyc_Std_asctime(
const struct Cyc_Std_tm*timeptr);struct _tagged_arr Cyc_Std_ctime(const int*timep);
unsigned int Cyc_Std_strftime(struct _tagged_arr s,unsigned int maxsize,struct
_tagged_arr fmt,const struct Cyc_Std_tm*t);struct _tagged_arr Cyc_Std_asctime_r(const
struct Cyc_Std_tm*,struct _tagged_arr);struct _tagged_arr Cyc_Std_ctime_r(const int*,
struct _tagged_arr);struct Cyc_Std_stat_t{short st_dev;unsigned int st_ino;int
st_mode;unsigned short st_nlink;unsigned short st_uid;unsigned short st_gid;short
st_rdev;int st_size;int st_atime;int st_spare1;int st_mtime;int st_spare2;int st_ctime;
int st_spare3;int st_blksize;int st_blocks;int st_spare4[2];};int Cyc_Std_stat(struct
_tagged_arr filename,struct Cyc_Std_stat_t*buf);int Cyc_Std_lstat(struct _tagged_arr
filename,struct Cyc_Std_stat_t*buf);int Cyc_Std_mkdir(struct _tagged_arr pathname,
int mode);int Cyc_Std_chmod(struct _tagged_arr path,int mode);int stat(char*filename,
struct Cyc_Std_stat_t*buf);int lstat(char*filename,struct Cyc_Std_stat_t*buf);int
mkdir(char*pathname,int mode);int chmod(char*pathname,int mode);int Cyc_Std_stat(
struct _tagged_arr filename,struct Cyc_Std_stat_t*buf){return stat(string_to_Cstring(
filename),buf);}int Cyc_Std_lstat(struct _tagged_arr filename,struct Cyc_Std_stat_t*
buf){return lstat(string_to_Cstring(filename),buf);}int Cyc_Std_mkdir(struct
_tagged_arr pathname,int mode){return mkdir(string_to_Cstring(pathname),mode);}int
Cyc_Std_chmod(struct _tagged_arr pathname,int mode){return chmod(string_to_Cstring(
pathname),mode);}struct Cyc_Cstdio___abstractFILE;struct Cyc_Std___cycFILE;struct
Cyc_Std___cycFILE*Cyc_Std_fromCfile(struct Cyc_Cstdio___abstractFILE*cf);int Cyc_Std_remove(
struct _tagged_arr);int Cyc_Std_rename(struct _tagged_arr,struct _tagged_arr);int Cyc_Std_fclose(
struct Cyc_Std___cycFILE*);int Cyc_Std_fflush(struct Cyc_Std___cycFILE*);struct Cyc_Std___cycFILE*
Cyc_Std_fopen(struct _tagged_arr __filename,struct _tagged_arr __modes);struct Cyc_Std___cycFILE*
Cyc_Std_freopen(struct _tagged_arr,struct _tagged_arr,struct Cyc_Std___cycFILE*);
void Cyc_Std_setbuf(struct Cyc_Std___cycFILE*__stream,struct _tagged_arr __buf);int
Cyc_Std_setvbuf(struct Cyc_Std___cycFILE*__stream,struct _tagged_arr __buf,int
__modes,unsigned int __n);int Cyc_Std_fgetc(struct Cyc_Std___cycFILE*__stream);int
Cyc_Std_getc(struct Cyc_Std___cycFILE*__stream);struct _tagged_arr Cyc_Std_fgets(
struct _tagged_arr __s,int __n,struct Cyc_Std___cycFILE*__stream);int Cyc_Std_fputc(
int __c,struct Cyc_Std___cycFILE*__stream);int Cyc_Std_putc(int __c,struct Cyc_Std___cycFILE*
__stream);int Cyc_Std_fputs(struct _tagged_arr __s,struct Cyc_Std___cycFILE*__stream);
int Cyc_Std_puts(struct _tagged_arr __s);int Cyc_Std_ungetc(int __c,struct Cyc_Std___cycFILE*
__stream);unsigned int Cyc_Std_fread(struct _tagged_arr __ptr,unsigned int __size,
unsigned int __n,struct Cyc_Std___cycFILE*__stream);unsigned int Cyc_Std_fwrite(
struct _tagged_arr __ptr,unsigned int __size,unsigned int __n,struct Cyc_Std___cycFILE*
__s);int Cyc_Std_fseek(struct Cyc_Std___cycFILE*__stream,int __off,int __whence);int
Cyc_Std_ftell(struct Cyc_Std___cycFILE*__stream);void Cyc_Std_rewind(struct Cyc_Std___cycFILE*
__stream);int Cyc_Std_fgetpos(struct Cyc_Std___cycFILE*__stream,int*__pos);int Cyc_Std_fsetpos(
struct Cyc_Std___cycFILE*__stream,int*__pos);void Cyc_Std_clearerr(struct Cyc_Std___cycFILE*
__stream);int Cyc_Std_feof(struct Cyc_Std___cycFILE*__stream);int Cyc_Std_ferror(
struct Cyc_Std___cycFILE*__stream);void Cyc_Std_perror(struct _tagged_arr __s);
struct Cyc_Std___cycFILE*Cyc_Std_fdopen(int __fd,struct _tagged_arr __modes);int Cyc_Std_fileno(
struct Cyc_Std___cycFILE*__stream);int Cyc_Std_getw(struct Cyc_Std___cycFILE*
__stream);int Cyc_Std_putw(int __w,struct Cyc_Std___cycFILE*__stream);void Cyc_Std_setbuffer(
struct Cyc_Std___cycFILE*__stream,struct _tagged_arr __buf,unsigned int __size);void
Cyc_Std_setlinebuf(struct Cyc_Std___cycFILE*__stream);struct Cyc_Std___cycFILE*Cyc_Std_popen(
struct _tagged_arr command,struct _tagged_arr type);int Cyc_Std_pclose(struct Cyc_Std___cycFILE*
stream);extern char Cyc_Std_FileCloseError[19];extern char Cyc_Std_FileOpenError[18];
struct Cyc_Std_FileOpenError_struct{char*tag;struct _tagged_arr f1;};struct Cyc_Std___cycFILE*
Cyc_Std_file_open(struct _tagged_arr fname,struct _tagged_arr mode);void Cyc_Std_file_close(
struct Cyc_Std___cycFILE*);struct Cyc_Std_String_pa_struct{int tag;struct
_tagged_arr f1;};struct Cyc_Std_Int_pa_struct{int tag;unsigned int f1;};struct Cyc_Std_Double_pa_struct{
int tag;double f1;};struct Cyc_Std_ShortPtr_pa_struct{int tag;short*f1;};struct Cyc_Std_IntPtr_pa_struct{
int tag;unsigned int*f1;};struct _tagged_arr Cyc_Std_aprintf(struct _tagged_arr fmt,
struct _tagged_arr);struct Cyc_Std_ShortPtr_sa_struct{int tag;short*f1;};struct Cyc_Std_UShortPtr_sa_struct{
int tag;unsigned short*f1;};struct Cyc_Std_IntPtr_sa_struct{int tag;int*f1;};struct
Cyc_Std_UIntPtr_sa_struct{int tag;unsigned int*f1;};struct Cyc_Std_StringPtr_sa_struct{
int tag;struct _tagged_arr f1;};struct Cyc_Std_DoublePtr_sa_struct{int tag;double*f1;
};struct Cyc_Std_FloatPtr_sa_struct{int tag;float*f1;};struct Cyc_List_List{void*hd;
struct Cyc_List_List*tl;};extern char Cyc_List_List_mismatch[18];extern char Cyc_List_Nth[
8];unsigned int Cyc_Std_strlen(struct _tagged_arr s);struct _tagged_arr Cyc_Std_strconcat(
struct _tagged_arr,struct _tagged_arr);struct _tagged_arr Cyc_Std_strcpy(struct
_tagged_arr dest,struct _tagged_arr src);struct Cyc_Cstdio___abstractFILE;int remove(
char*);int rename(char*,char*);int fclose(struct Cyc_Cstdio___abstractFILE*);int
fflush(struct Cyc_Cstdio___abstractFILE*);struct Cyc_Cstdio___abstractFILE*fopen(
char*__filename,char*__modes);struct Cyc_Cstdio___abstractFILE*freopen(char*
__filename,char*__modes,struct Cyc_Cstdio___abstractFILE*__stream);struct Cyc_Cstdio___abstractFILE*
fdopen(int __fd,char*__modes);int setvbuf(struct Cyc_Cstdio___abstractFILE*__stream,
char*__buf,int __modes,unsigned int __n);int fgetc(struct Cyc_Cstdio___abstractFILE*
__stream);int getc(struct Cyc_Cstdio___abstractFILE*__stream);int fputc(int __c,
struct Cyc_Cstdio___abstractFILE*__stream);int putc(int __c,struct Cyc_Cstdio___abstractFILE*
__stream);int getw(struct Cyc_Cstdio___abstractFILE*__stream);int putw(int __w,
struct Cyc_Cstdio___abstractFILE*__stream);char*fgets(char*__s,int __n,struct Cyc_Cstdio___abstractFILE*
__stream);int fputs(char*__s,struct Cyc_Cstdio___abstractFILE*__stream);int puts(
char*__s);int ungetc(int __c,struct Cyc_Cstdio___abstractFILE*__stream);
unsigned int fread(char*__ptr,unsigned int __size,unsigned int __n,struct Cyc_Cstdio___abstractFILE*
__stream);unsigned int fwrite(char*__ptr,unsigned int __size,unsigned int __n,
struct Cyc_Cstdio___abstractFILE*__s);int fseek(struct Cyc_Cstdio___abstractFILE*
__stream,int __off,int __whence);int ftell(struct Cyc_Cstdio___abstractFILE*__stream);
void rewind(struct Cyc_Cstdio___abstractFILE*__stream);int fgetpos(struct Cyc_Cstdio___abstractFILE*
__stream,int*__pos);int fsetpos(struct Cyc_Cstdio___abstractFILE*__stream,int*
__pos);void clearerr(struct Cyc_Cstdio___abstractFILE*__stream);int feof(struct Cyc_Cstdio___abstractFILE*
__stream);int ferror(struct Cyc_Cstdio___abstractFILE*__stream);void perror(char*
__s);int fileno(struct Cyc_Cstdio___abstractFILE*__stream);struct Cyc_Cstdio___abstractFILE*
popen(char*,char*);int pclose(struct Cyc_Cstdio___abstractFILE*__stream);struct Cyc_Std___cycFILE{
struct Cyc_Cstdio___abstractFILE*file;};struct Cyc_Std___cycFILE*Cyc_Std_fromCfile(
struct Cyc_Cstdio___abstractFILE*cf){return(unsigned int)cf?({struct Cyc_Std___cycFILE*
_tmp8=_cycalloc(sizeof(*_tmp8));_tmp8->file=cf;_tmp8;}): 0;}int Cyc_Std_remove(
struct _tagged_arr filename){return remove(string_to_Cstring(filename));}int Cyc_Std_rename(
struct _tagged_arr old_filename,struct _tagged_arr new_filename){return rename(
string_to_Cstring(old_filename),string_to_Cstring(new_filename));}int Cyc_Std_fclose(
struct Cyc_Std___cycFILE*f){int r=fclose((struct Cyc_Cstdio___abstractFILE*)
_check_null(f->file));if(r == 0)f->file=0;return r;}int Cyc_Std_fflush(struct Cyc_Std___cycFILE*
f){return(unsigned int)f?fflush(f->file): fflush(0);}struct Cyc_Std___cycFILE*Cyc_Std_freopen(
struct _tagged_arr x,struct _tagged_arr y,struct Cyc_Std___cycFILE*f){struct Cyc_Cstdio___abstractFILE*
cf=freopen(string_to_Cstring(x),string_to_Cstring(y),(struct Cyc_Cstdio___abstractFILE*)
_check_null(f->file));return(unsigned int)cf?({struct Cyc_Std___cycFILE*_tmp9=
_cycalloc(sizeof(*_tmp9));_tmp9->file=cf;_tmp9;}): 0;}void Cyc_Std_setbuf(struct
Cyc_Std___cycFILE*f,struct _tagged_arr buf){Cyc_Std_setvbuf(f,buf,(unsigned int)
buf.curr?0: 2,1024);}void Cyc_Std_setbuffer(struct Cyc_Std___cycFILE*f,struct
_tagged_arr buf,unsigned int size){Cyc_Std_setvbuf(f,buf,(unsigned int)buf.curr?0:
2,size);}void Cyc_Std_setlinebuf(struct Cyc_Std___cycFILE*f){Cyc_Std_setvbuf(f,
_tag_arr(0,0,0),1,0);}int Cyc_Std_setvbuf(struct Cyc_Std___cycFILE*f,struct
_tagged_arr buf,int mode,unsigned int size){if(_get_arr_size(buf,sizeof(char))< 
size)(int)_throw((void*)({struct Cyc_Core_Failure_struct*_tmpA=_cycalloc(sizeof(*
_tmpA));_tmpA[0]=({struct Cyc_Core_Failure_struct _tmpB;_tmpB.tag=Cyc_Core_Failure;
_tmpB.f1=_tag_arr("setvbuf: buffer insufficient",sizeof(char),29);_tmpB;});_tmpA;}));
return setvbuf((struct Cyc_Cstdio___abstractFILE*)_check_null(f->file),
underlying_Cstring((struct _tagged_arr)buf),mode,size);}int Cyc_Std_fgetc(struct
Cyc_Std___cycFILE*f){return fgetc((struct Cyc_Cstdio___abstractFILE*)_check_null(f->file));}
struct _tagged_arr Cyc_Std_fgets(struct _tagged_arr s,int n,struct Cyc_Std___cycFILE*f){
char*result;char*buffer=underlying_Cstring((struct _tagged_arr)s);unsigned int len=
_get_arr_size(s,sizeof(char));n=(int)(len < n?len:(unsigned int)n);result=fgets(
buffer,n,(struct Cyc_Cstdio___abstractFILE*)_check_null(f->file));if(result == 0)
return _tag_arr(0,0,0);else{return s;}}int Cyc_Std_fputc(int i,struct Cyc_Std___cycFILE*
f){return fputc(i,(struct Cyc_Cstdio___abstractFILE*)_check_null(f->file));}int Cyc_Std_fputs(
struct _tagged_arr s,struct Cyc_Std___cycFILE*f){return fputs(string_to_Cstring(s),(
struct Cyc_Cstdio___abstractFILE*)_check_null(f->file));}int Cyc_Std_getc(struct
Cyc_Std___cycFILE*f){return getc((struct Cyc_Cstdio___abstractFILE*)_check_null(f->file));}
int Cyc_Std_putc(int i,struct Cyc_Std___cycFILE*f){return putc(i,(struct Cyc_Cstdio___abstractFILE*)
_check_null(f->file));}int Cyc_Std_puts(struct _tagged_arr s){return puts(
string_to_Cstring(s));}int Cyc_Std_ungetc(int i,struct Cyc_Std___cycFILE*f){return
ungetc(i,(struct Cyc_Cstdio___abstractFILE*)_check_null(f->file));}unsigned int
Cyc_Std_fread(struct _tagged_arr ptr,unsigned int size,unsigned int nmemb,struct Cyc_Std___cycFILE*
f){if(size * nmemb > _get_arr_size(ptr,sizeof(char)))(int)_throw((void*)({struct
Cyc_Core_Failure_struct*_tmpC=_cycalloc(sizeof(*_tmpC));_tmpC[0]=({struct Cyc_Core_Failure_struct
_tmpD;_tmpD.tag=Cyc_Core_Failure;_tmpD.f1=_tag_arr("fread: buffer insufficient",
sizeof(char),27);_tmpD;});_tmpC;}));return fread(underlying_Cstring((struct
_tagged_arr)ptr),size,nmemb,(struct Cyc_Cstdio___abstractFILE*)_check_null(f->file));}
unsigned int Cyc_Std_fwrite(struct _tagged_arr ptr,unsigned int size,unsigned int
nmemb,struct Cyc_Std___cycFILE*f){if(size * nmemb > _get_arr_size(ptr,sizeof(char)))(
int)_throw((void*)({struct Cyc_Core_Failure_struct*_tmpE=_cycalloc(sizeof(*_tmpE));
_tmpE[0]=({struct Cyc_Core_Failure_struct _tmpF;_tmpF.tag=Cyc_Core_Failure;_tmpF.f1=
_tag_arr("fwrite: buffer insufficient",sizeof(char),28);_tmpF;});_tmpE;}));
return fwrite(underlying_Cstring(ptr),size,nmemb,(struct Cyc_Cstdio___abstractFILE*)
_check_null(f->file));}int Cyc_Std_fgetpos(struct Cyc_Std___cycFILE*f,int*x){
return fgetpos((struct Cyc_Cstdio___abstractFILE*)_check_null(f->file),x);}int Cyc_Std_fseek(
struct Cyc_Std___cycFILE*f,int offset,int whence){return fseek((struct Cyc_Cstdio___abstractFILE*)
_check_null(f->file),offset,whence);}int Cyc_Std_fsetpos(struct Cyc_Std___cycFILE*
f,int*x){return fsetpos((struct Cyc_Cstdio___abstractFILE*)_check_null(f->file),x);}
int Cyc_Std_ftell(struct Cyc_Std___cycFILE*f){return ftell((struct Cyc_Cstdio___abstractFILE*)
_check_null(f->file));}void Cyc_Std_rewind(struct Cyc_Std___cycFILE*f){rewind((
struct Cyc_Cstdio___abstractFILE*)_check_null(f->file));}void Cyc_Std_clearerr(
struct Cyc_Std___cycFILE*f){clearerr((struct Cyc_Cstdio___abstractFILE*)
_check_null(f->file));}int Cyc_Std_feof(struct Cyc_Std___cycFILE*f){return feof((
struct Cyc_Cstdio___abstractFILE*)_check_null(f->file));}int Cyc_Std_ferror(struct
Cyc_Std___cycFILE*f){return ferror((struct Cyc_Cstdio___abstractFILE*)_check_null(
f->file));}void Cyc_Std_perror(struct _tagged_arr s){perror(string_to_Cstring(s));}
struct Cyc_Std___cycFILE*Cyc_Std_fopen(struct _tagged_arr name,struct _tagged_arr
type){struct Cyc_Cstdio___abstractFILE*cf=fopen(string_to_Cstring(name),
string_to_Cstring(type));return(unsigned int)cf?({struct Cyc_Std___cycFILE*_tmp10=
_cycalloc(sizeof(*_tmp10));_tmp10->file=cf;_tmp10;}): 0;}struct Cyc_Std___cycFILE*
Cyc_Std_fdopen(int i,struct _tagged_arr s){struct Cyc_Cstdio___abstractFILE*cf=
fdopen(i,string_to_Cstring(s));return(unsigned int)cf?({struct Cyc_Std___cycFILE*
_tmp11=_cycalloc(sizeof(*_tmp11));_tmp11->file=cf;_tmp11;}): 0;}int Cyc_Std_fileno(
struct Cyc_Std___cycFILE*f){return fileno((struct Cyc_Cstdio___abstractFILE*)
_check_null(f->file));}int Cyc_Std_getw(struct Cyc_Std___cycFILE*f){return getw((
struct Cyc_Cstdio___abstractFILE*)_check_null(f->file));}int Cyc_Std_pclose(struct
Cyc_Std___cycFILE*f){return pclose((struct Cyc_Cstdio___abstractFILE*)_check_null(
f->file));}struct Cyc_Std___cycFILE*Cyc_Std_popen(struct _tagged_arr s,struct
_tagged_arr m){struct Cyc_Cstdio___abstractFILE*cf=popen(string_to_Cstring(s),
string_to_Cstring(m));return(unsigned int)cf?({struct Cyc_Std___cycFILE*_tmp12=
_cycalloc(sizeof(*_tmp12));_tmp12->file=cf;_tmp12;}): 0;}int Cyc_Std_putw(int i,
struct Cyc_Std___cycFILE*f){return putw(i,(struct Cyc_Cstdio___abstractFILE*)
_check_null(f->file));}char Cyc_Std_FileCloseError[19]="\000\000\000\000FileCloseError";
char Cyc_Std_FileOpenError[18]="\000\000\000\000FileOpenError";struct Cyc_Std___cycFILE*
Cyc_Std_file_open(struct _tagged_arr fname,struct _tagged_arr mode){struct Cyc_Std___cycFILE*
f=Cyc_Std_fopen(fname,mode);if(f == 0){struct _tagged_arr fn=({unsigned int _tmp15=
_get_arr_size(fname,sizeof(char));char*_tmp16=(char*)_cycalloc_atomic(
_check_times(sizeof(char),_tmp15));struct _tagged_arr _tmp18=_tag_arr(_tmp16,
sizeof(char),_get_arr_size(fname,sizeof(char)));{unsigned int _tmp17=_tmp15;
unsigned int i;for(i=0;i < _tmp17;i ++){_tmp16[i]=((const char*)fname.curr)[(int)i];}}
_tmp18;});(int)_throw((void*)({struct Cyc_Std_FileOpenError_struct*_tmp13=
_cycalloc(sizeof(*_tmp13));_tmp13[0]=({struct Cyc_Std_FileOpenError_struct _tmp14;
_tmp14.tag=Cyc_Std_FileOpenError;_tmp14.f1=fn;_tmp14;});_tmp13;}));}return(
struct Cyc_Std___cycFILE*)_check_null(f);}void Cyc_Std_file_close(struct Cyc_Std___cycFILE*
f){if(Cyc_Std_fclose(f)!= 0)(int)_throw((void*)Cyc_Std_FileCloseError);}extern
char Cyc_Array_Array_mismatch[19];struct Cyc_Std__Div{int quot;int rem;};struct Cyc_Std__Ldiv{
int quot;int rem;};double Cyc_Std_atof(struct _tagged_arr);int Cyc_Std_atoi(struct
_tagged_arr);int Cyc_Std_atol(struct _tagged_arr);struct _tagged_arr Cyc_Std_getenv(
struct _tagged_arr);double Cyc_Std_strtod(struct _tagged_arr n,struct _tagged_arr*end);
int Cyc_Std_strtol(struct _tagged_arr n,struct _tagged_arr*end,int base);unsigned int
Cyc_Std_strtoul(struct _tagged_arr n,struct _tagged_arr*end,int base);unsigned int
Cyc_Std_mstrtoul(struct _tagged_arr n,struct _tagged_arr*endptr,int base);void Cyc_Std_qsort(
struct _tagged_arr tab,unsigned int nmemb,unsigned int szmemb,int(*compar)(const void*,
const void*));int Cyc_Std_system(struct _tagged_arr);void Cyc_Std_free(struct
_tagged_arr);double atof(char*);int atoi(char*);int atol(char*);char*getenv(char*);
int putenv(char*);double strtod(char*,char**);int strtol(char*,char**,int);
unsigned int strtoul(char*,char**,int);void qsort(void*base,unsigned int nmemb,
unsigned int size,int(*compar)(const void*,const void*));int system(char*);double Cyc_Std_atof(
struct _tagged_arr _nptr){return atof(string_to_Cstring(_nptr));}int Cyc_Std_atoi(
struct _tagged_arr _nptr){return atoi(string_to_Cstring(_nptr));}int Cyc_Std_atol(
struct _tagged_arr _nptr){return atol(string_to_Cstring(_nptr));}struct _tagged_arr
Cyc_Std_getenv(struct _tagged_arr name){return Cstring_to_string(getenv(
string_to_Cstring(name)));}int Cyc_Std_putenv(struct _tagged_arr s){return putenv(
string_to_Cstring(s));}static void Cyc_Std_check_valid_cstring(struct _tagged_arr s){
if(s.curr == ((struct _tagged_arr)_tag_arr(0,0,0)).curr)(int)_throw((void*)({
struct Cyc_Core_Invalid_argument_struct*_tmp19=_cycalloc(sizeof(*_tmp19));_tmp19[
0]=({struct Cyc_Core_Invalid_argument_struct _tmp1A;_tmp1A.tag=Cyc_Core_Invalid_argument;
_tmp1A.f1=_tag_arr("strtox NULL pointer",sizeof(char),20);_tmp1A;});_tmp19;}));{
int found_zero=0;{int i=(int)(_get_arr_size(s,sizeof(char))- 1);for(0;i >= 0;i --){
if(((const char*)s.curr)[i]== '\000'){found_zero=1;break;}}}if(!found_zero)(int)
_throw((void*)({struct Cyc_Core_Invalid_argument_struct*_tmp1B=_cycalloc(sizeof(*
_tmp1B));_tmp1B[0]=({struct Cyc_Core_Invalid_argument_struct _tmp1C;_tmp1C.tag=Cyc_Core_Invalid_argument;
_tmp1C.f1=_tag_arr("strtox: not a C string",sizeof(char),23);_tmp1C;});_tmp1B;}));}}
double Cyc_Std_strtod(struct _tagged_arr nptr,struct _tagged_arr*endptr){Cyc_Std_check_valid_cstring(
nptr);{char*c=underlying_Cstring(nptr);char*e=endptr == 0?0: c;double d=strtod(c,(
char**)& e);if(endptr != 0){int n=(int)((unsigned int)e - (unsigned int)c);*endptr=
_tagged_arr_plus(nptr,sizeof(char),n);}return d;}}int Cyc_Std_strtol(struct
_tagged_arr n,struct _tagged_arr*endptr,int base){Cyc_Std_check_valid_cstring(n);{
char*c=underlying_Cstring(n);char*e=endptr == 0?0: c;int r=strtol(c,(char**)& e,base);
if(endptr != 0){int m=(int)((unsigned int)e - (unsigned int)c);*endptr=
_tagged_arr_plus(n,sizeof(char),m);}return r;}}unsigned int Cyc_Std_strtoul(struct
_tagged_arr n,struct _tagged_arr*endptr,int base){Cyc_Std_check_valid_cstring(n);{
char*c=underlying_Cstring(n);char*e=endptr == 0?0: c;unsigned int r=strtoul(c,(char**)&
e,base);if(endptr != 0){int m=(int)((unsigned int)e - (unsigned int)c);*endptr=
_tagged_arr_plus(n,sizeof(char),m);}return r;}}unsigned int Cyc_Std_mstrtoul(
struct _tagged_arr n,struct _tagged_arr*endptr,int base){Cyc_Std_check_valid_cstring((
struct _tagged_arr)n);{char*c=underlying_Cstring((struct _tagged_arr)n);char*e=
endptr == 0?0: c;unsigned int r=strtoul(c,(char**)& e,base);if(endptr != 0){int m=(int)((
unsigned int)e - (unsigned int)c);*endptr=_tagged_arr_plus(n,sizeof(char),m);}
return r;}}void Cyc_Std_qsort(struct _tagged_arr tab,unsigned int nmemb,unsigned int
szmemb,int(*compar)(const void*,const void*)){if(tab.curr == (_tag_arr(0,0,0)).curr?
1: _get_arr_size(tab,sizeof(void))< nmemb)(int)_throw((void*)({struct Cyc_Core_Invalid_argument_struct*
_tmp1D=_cycalloc(sizeof(*_tmp1D));_tmp1D[0]=({struct Cyc_Core_Invalid_argument_struct
_tmp1E;_tmp1E.tag=Cyc_Core_Invalid_argument;_tmp1E.f1=_tag_arr("Std::qsort",
sizeof(char),11);_tmp1E;});_tmp1D;}));qsort((void*)_check_null(_untag_arr(tab,
sizeof(void),1)),nmemb,(unsigned int)szmemb,(int(*)(const void*,const void*))
compar);}int Cyc_Std_system(struct _tagged_arr cmd){return system(string_to_Cstring(
cmd));}void Cyc_Std_free(struct _tagged_arr ptr){;}int accept(int,const struct Cyc_Std_sockaddr*,
unsigned int*);int accept_in(int,const struct Cyc_Std_sockaddr_in*,unsigned int*);
int bind(int,const struct Cyc_Std_sockaddr*,unsigned int);int bind_in(int,const
struct Cyc_Std_sockaddr_in*,unsigned int);int connect(int,const struct Cyc_Std_sockaddr*,
unsigned int);int connect_in(int,const struct Cyc_Std_sockaddr_in*,unsigned int);
int getpeername(int,const struct Cyc_Std_sockaddr*,unsigned int*);int getpeername_in(
int,const struct Cyc_Std_sockaddr_in*,unsigned int*);int getsockname(int,const
struct Cyc_Std_sockaddr*,unsigned int*);int getsockname_in(int,const struct Cyc_Std_sockaddr_in*,
unsigned int*);int recvfrom(int,struct _tagged_arr,unsigned int,int,const struct Cyc_Std_sockaddr*,
unsigned int*);int recvfrom_in(int,struct _tagged_arr,unsigned int,int,const struct
Cyc_Std_sockaddr_in*,unsigned int*);int sendto(int,struct _tagged_arr,unsigned int,
int,const struct Cyc_Std_sockaddr*,unsigned int);int sendto_in(int,struct
_tagged_arr,unsigned int,int,const struct Cyc_Std_sockaddr_in*,unsigned int);int
send_wrapped(int,struct _tagged_arr,unsigned int,int);int recv_wrapped(int,struct
_tagged_arr,unsigned int,int);int getsockopt_int(int,int,int,int*,unsigned int*);
int setsockopt_int(int,int,int,const int*,unsigned int);int getsockopt_timeval(int,
int,int,struct Cyc_Std_timeval*,unsigned int*);int setsockopt_timeval(int,int,int,
const struct Cyc_Std_timeval*,unsigned int);char Cyc_Std_SocketError[16]="\000\000\000\000SocketError";
struct Cyc_Std_SocketError_struct{char*tag;struct _tagged_arr f1;};static struct
_tagged_arr Cyc_Std_sopts2string(struct _tagged_arr args){struct _tagged_arr res=
_tag_arr(({char*_tmp20=_cycalloc_atomic(sizeof(char)* 1);_tmp20[0]='\000';_tmp20;}),
sizeof(char),1);{int i=0;for(0;i < _get_arr_size(args,sizeof(void*));i ++){void*
_tmp1F=((void**)args.curr)[i];_LL6: if(*((int*)_tmp1F)!= 0)goto _LL8;_LL7: res=Cyc_Std_strconcat((
struct _tagged_arr)res,_tag_arr("|SO_int",sizeof(char),8));goto _LL5;_LL8: if(*((
int*)_tmp1F)!= 1)goto _LLA;_LL9: res=Cyc_Std_strconcat((struct _tagged_arr)res,
_tag_arr("|SO_timeval",sizeof(char),12));goto _LL5;_LLA: if(*((int*)_tmp1F)!= 2)
goto _LLC;_LLB: res=Cyc_Std_strconcat((struct _tagged_arr)res,_tag_arr("|SO_socklenptr",
sizeof(char),15));goto _LL5;_LLC: if(*((int*)_tmp1F)!= 3)goto _LL5;_LLD: res=Cyc_Std_strconcat((
struct _tagged_arr)res,_tag_arr("|SO_socklen",sizeof(char),12));goto _LL5;_LL5:;}}
return res;}struct _tuple0{void*f1;void*f2;};int Cyc_Std_accept(int fd,struct
_tagged_arr ap){if(_get_arr_size(ap,sizeof(void*))!= 2)(int)_throw((void*)({
struct Cyc_Std_SocketError_struct*_tmp21=_cycalloc(sizeof(*_tmp21));_tmp21[0]=({
struct Cyc_Std_SocketError_struct _tmp22;_tmp22.tag=Cyc_Std_SocketError;_tmp22.f1=
_tag_arr("accept---need 2 args",sizeof(char),21);_tmp22;});_tmp21;}));{struct
_tuple0 _tmp24=({struct _tuple0 _tmp23;_tmp23.f1=*((void**)_check_unknown_subscript(
ap,sizeof(void*),0));_tmp23.f2=*((void**)_check_unknown_subscript(ap,sizeof(void*),
1));_tmp23;});void*_tmp25;struct Cyc_Std_sockaddr_in*_tmp26;void*_tmp27;
unsigned int*_tmp28;void*_tmp29;struct Cyc_Std_sockaddr*_tmp2A;void*_tmp2B;
unsigned int*_tmp2C;_LLF: _tmp25=_tmp24.f1;if(*((int*)_tmp25)!= 0)goto _LL11;
_tmp26=((struct Cyc_Std_SA_sockaddr_in_struct*)_tmp25)->f1;_tmp27=_tmp24.f2;if(*((
int*)_tmp27)!= 2)goto _LL11;_tmp28=((struct Cyc_Std_SA_socklenptr_struct*)_tmp27)->f1;
_LL10: return accept_in(fd,(const struct Cyc_Std_sockaddr_in*)_tmp26,_tmp28);_LL11:
_tmp29=_tmp24.f1;if(*((int*)_tmp29)!= 1)goto _LL13;_tmp2A=((struct Cyc_Std_SA_sockaddr_struct*)
_tmp29)->f1;_tmp2B=_tmp24.f2;if(*((int*)_tmp2B)!= 2)goto _LL13;_tmp2C=((struct Cyc_Std_SA_socklenptr_struct*)
_tmp2B)->f1;_LL12: return accept(fd,(const struct Cyc_Std_sockaddr*)_tmp2A,_tmp2C);
_LL13:;_LL14:(int)_throw((void*)({struct Cyc_Std_SocketError_struct*_tmp2D=
_cycalloc(sizeof(*_tmp2D));_tmp2D[0]=({struct Cyc_Std_SocketError_struct _tmp2E;
_tmp2E.tag=Cyc_Std_SocketError;_tmp2E.f1=_tag_arr("accept---bad args",sizeof(
char),18);_tmp2E;});_tmp2D;}));_LLE:;}}int Cyc_Std_bind(int fd,struct _tagged_arr ap){
if(_get_arr_size(ap,sizeof(void*))!= 2)(int)_throw((void*)({struct Cyc_Std_SocketError_struct*
_tmp2F=_cycalloc(sizeof(*_tmp2F));_tmp2F[0]=({struct Cyc_Std_SocketError_struct
_tmp30;_tmp30.tag=Cyc_Std_SocketError;_tmp30.f1=_tag_arr("bind---need 2 args",
sizeof(char),19);_tmp30;});_tmp2F;}));{struct _tuple0 _tmp32=({struct _tuple0 _tmp31;
_tmp31.f1=*((void**)_check_unknown_subscript(ap,sizeof(void*),0));_tmp31.f2=*((
void**)_check_unknown_subscript(ap,sizeof(void*),1));_tmp31;});void*_tmp33;
struct Cyc_Std_sockaddr_in*_tmp34;void*_tmp35;unsigned int _tmp36;void*_tmp37;
struct Cyc_Std_sockaddr*_tmp38;void*_tmp39;unsigned int _tmp3A;_LL16: _tmp33=_tmp32.f1;
if(*((int*)_tmp33)!= 0)goto _LL18;_tmp34=((struct Cyc_Std_SA_sockaddr_in_struct*)
_tmp33)->f1;_tmp35=_tmp32.f2;if(*((int*)_tmp35)!= 3)goto _LL18;_tmp36=((struct Cyc_Std_SA_socklen_struct*)
_tmp35)->f1;_LL17: return bind_in(fd,(const struct Cyc_Std_sockaddr_in*)_tmp34,
_tmp36);_LL18: _tmp37=_tmp32.f1;if(*((int*)_tmp37)!= 1)goto _LL1A;_tmp38=((struct
Cyc_Std_SA_sockaddr_struct*)_tmp37)->f1;_tmp39=_tmp32.f2;if(*((int*)_tmp39)!= 3)
goto _LL1A;_tmp3A=((struct Cyc_Std_SA_socklen_struct*)_tmp39)->f1;_LL19: return bind(
fd,(const struct Cyc_Std_sockaddr*)_tmp38,_tmp3A);_LL1A:;_LL1B:(int)_throw((void*)({
struct Cyc_Std_SocketError_struct*_tmp3B=_cycalloc(sizeof(*_tmp3B));_tmp3B[0]=({
struct Cyc_Std_SocketError_struct _tmp3C;_tmp3C.tag=Cyc_Std_SocketError;_tmp3C.f1=
_tag_arr("bind---bad args",sizeof(char),16);_tmp3C;});_tmp3B;}));_LL15:;}}int Cyc_Std_connect(
int fd,struct _tagged_arr ap){if(_get_arr_size(ap,sizeof(void*))!= 2)(int)_throw((
void*)({struct Cyc_Std_SocketError_struct*_tmp3D=_cycalloc(sizeof(*_tmp3D));
_tmp3D[0]=({struct Cyc_Std_SocketError_struct _tmp3E;_tmp3E.tag=Cyc_Std_SocketError;
_tmp3E.f1=_tag_arr("connect---need 2 args",sizeof(char),22);_tmp3E;});_tmp3D;}));{
struct _tuple0 _tmp40=({struct _tuple0 _tmp3F;_tmp3F.f1=*((void**)
_check_unknown_subscript(ap,sizeof(void*),0));_tmp3F.f2=*((void**)
_check_unknown_subscript(ap,sizeof(void*),1));_tmp3F;});void*_tmp41;struct Cyc_Std_sockaddr_in*
_tmp42;void*_tmp43;unsigned int _tmp44;void*_tmp45;struct Cyc_Std_sockaddr*_tmp46;
void*_tmp47;unsigned int _tmp48;_LL1D: _tmp41=_tmp40.f1;if(*((int*)_tmp41)!= 0)
goto _LL1F;_tmp42=((struct Cyc_Std_SA_sockaddr_in_struct*)_tmp41)->f1;_tmp43=
_tmp40.f2;if(*((int*)_tmp43)!= 3)goto _LL1F;_tmp44=((struct Cyc_Std_SA_socklen_struct*)
_tmp43)->f1;_LL1E: return connect_in(fd,(const struct Cyc_Std_sockaddr_in*)_tmp42,
_tmp44);_LL1F: _tmp45=_tmp40.f1;if(*((int*)_tmp45)!= 1)goto _LL21;_tmp46=((struct
Cyc_Std_SA_sockaddr_struct*)_tmp45)->f1;_tmp47=_tmp40.f2;if(*((int*)_tmp47)!= 3)
goto _LL21;_tmp48=((struct Cyc_Std_SA_socklen_struct*)_tmp47)->f1;_LL20: return
connect(fd,(const struct Cyc_Std_sockaddr*)_tmp46,_tmp48);_LL21:;_LL22:(int)_throw((
void*)({struct Cyc_Std_SocketError_struct*_tmp49=_cycalloc(sizeof(*_tmp49));
_tmp49[0]=({struct Cyc_Std_SocketError_struct _tmp4A;_tmp4A.tag=Cyc_Std_SocketError;
_tmp4A.f1=_tag_arr("connect---bad args",sizeof(char),19);_tmp4A;});_tmp49;}));
_LL1C:;}}int Cyc_Std_getpeername(int fd,struct _tagged_arr ap){if(_get_arr_size(ap,
sizeof(void*))!= 2)(int)_throw((void*)({struct Cyc_Std_SocketError_struct*_tmp4B=
_cycalloc(sizeof(*_tmp4B));_tmp4B[0]=({struct Cyc_Std_SocketError_struct _tmp4C;
_tmp4C.tag=Cyc_Std_SocketError;_tmp4C.f1=_tag_arr("getpeername---need 2 args",
sizeof(char),26);_tmp4C;});_tmp4B;}));{struct _tuple0 _tmp4E=({struct _tuple0 _tmp4D;
_tmp4D.f1=*((void**)_check_unknown_subscript(ap,sizeof(void*),0));_tmp4D.f2=*((
void**)_check_unknown_subscript(ap,sizeof(void*),1));_tmp4D;});void*_tmp4F;
struct Cyc_Std_sockaddr_in*_tmp50;void*_tmp51;unsigned int*_tmp52;void*_tmp53;
struct Cyc_Std_sockaddr*_tmp54;void*_tmp55;unsigned int*_tmp56;_LL24: _tmp4F=
_tmp4E.f1;if(*((int*)_tmp4F)!= 0)goto _LL26;_tmp50=((struct Cyc_Std_SA_sockaddr_in_struct*)
_tmp4F)->f1;_tmp51=_tmp4E.f2;if(*((int*)_tmp51)!= 2)goto _LL26;_tmp52=((struct Cyc_Std_SA_socklenptr_struct*)
_tmp51)->f1;_LL25: return getpeername_in(fd,(const struct Cyc_Std_sockaddr_in*)
_tmp50,_tmp52);_LL26: _tmp53=_tmp4E.f1;if(*((int*)_tmp53)!= 1)goto _LL28;_tmp54=((
struct Cyc_Std_SA_sockaddr_struct*)_tmp53)->f1;_tmp55=_tmp4E.f2;if(*((int*)_tmp55)
!= 2)goto _LL28;_tmp56=((struct Cyc_Std_SA_socklenptr_struct*)_tmp55)->f1;_LL27:
return getpeername(fd,(const struct Cyc_Std_sockaddr*)_tmp54,_tmp56);_LL28:;_LL29:(
int)_throw((void*)({struct Cyc_Std_SocketError_struct*_tmp57=_cycalloc(sizeof(*
_tmp57));_tmp57[0]=({struct Cyc_Std_SocketError_struct _tmp58;_tmp58.tag=Cyc_Std_SocketError;
_tmp58.f1=_tag_arr("getpeername---bad args",sizeof(char),23);_tmp58;});_tmp57;}));
_LL23:;}}int Cyc_Std_getsockname(int fd,struct _tagged_arr ap){if(_get_arr_size(ap,
sizeof(void*))!= 2)(int)_throw((void*)({struct Cyc_Std_SocketError_struct*_tmp59=
_cycalloc(sizeof(*_tmp59));_tmp59[0]=({struct Cyc_Std_SocketError_struct _tmp5A;
_tmp5A.tag=Cyc_Std_SocketError;_tmp5A.f1=_tag_arr("getsockname---need 2 args",
sizeof(char),26);_tmp5A;});_tmp59;}));{struct _tuple0 _tmp5C=({struct _tuple0 _tmp5B;
_tmp5B.f1=*((void**)_check_unknown_subscript(ap,sizeof(void*),0));_tmp5B.f2=*((
void**)_check_unknown_subscript(ap,sizeof(void*),1));_tmp5B;});void*_tmp5D;
struct Cyc_Std_sockaddr_in*_tmp5E;void*_tmp5F;unsigned int*_tmp60;void*_tmp61;
struct Cyc_Std_sockaddr*_tmp62;void*_tmp63;unsigned int*_tmp64;_LL2B: _tmp5D=
_tmp5C.f1;if(*((int*)_tmp5D)!= 0)goto _LL2D;_tmp5E=((struct Cyc_Std_SA_sockaddr_in_struct*)
_tmp5D)->f1;_tmp5F=_tmp5C.f2;if(*((int*)_tmp5F)!= 2)goto _LL2D;_tmp60=((struct Cyc_Std_SA_socklenptr_struct*)
_tmp5F)->f1;_LL2C: return getsockname_in(fd,(const struct Cyc_Std_sockaddr_in*)
_tmp5E,_tmp60);_LL2D: _tmp61=_tmp5C.f1;if(*((int*)_tmp61)!= 1)goto _LL2F;_tmp62=((
struct Cyc_Std_SA_sockaddr_struct*)_tmp61)->f1;_tmp63=_tmp5C.f2;if(*((int*)_tmp63)
!= 2)goto _LL2F;_tmp64=((struct Cyc_Std_SA_socklenptr_struct*)_tmp63)->f1;_LL2E:
return getsockname(fd,(const struct Cyc_Std_sockaddr*)_tmp62,_tmp64);_LL2F:;_LL30:(
int)_throw((void*)({struct Cyc_Std_SocketError_struct*_tmp65=_cycalloc(sizeof(*
_tmp65));_tmp65[0]=({struct Cyc_Std_SocketError_struct _tmp66;_tmp66.tag=Cyc_Std_SocketError;
_tmp66.f1=_tag_arr("getsockname---bad args",sizeof(char),23);_tmp66;});_tmp65;}));
_LL2A:;}}int Cyc_Std_recvfrom(int fd,struct _tagged_arr buf,unsigned int n,int flags,
struct _tagged_arr ap){if(_get_arr_size(ap,sizeof(void*))!= 2)(int)_throw((void*)({
struct Cyc_Std_SocketError_struct*_tmp67=_cycalloc(sizeof(*_tmp67));_tmp67[0]=({
struct Cyc_Std_SocketError_struct _tmp68;_tmp68.tag=Cyc_Std_SocketError;_tmp68.f1=
_tag_arr("recvfrom---need 2 args",sizeof(char),23);_tmp68;});_tmp67;}));{struct
_tuple0 _tmp6A=({struct _tuple0 _tmp69;_tmp69.f1=*((void**)_check_unknown_subscript(
ap,sizeof(void*),0));_tmp69.f2=*((void**)_check_unknown_subscript(ap,sizeof(void*),
1));_tmp69;});void*_tmp6B;struct Cyc_Std_sockaddr_in*_tmp6C;void*_tmp6D;
unsigned int*_tmp6E;void*_tmp6F;struct Cyc_Std_sockaddr*_tmp70;void*_tmp71;
unsigned int*_tmp72;_LL32: _tmp6B=_tmp6A.f1;if(*((int*)_tmp6B)!= 0)goto _LL34;
_tmp6C=((struct Cyc_Std_SA_sockaddr_in_struct*)_tmp6B)->f1;_tmp6D=_tmp6A.f2;if(*((
int*)_tmp6D)!= 2)goto _LL34;_tmp6E=((struct Cyc_Std_SA_socklenptr_struct*)_tmp6D)->f1;
_LL33: return recvfrom_in(fd,buf,n,flags,(const struct Cyc_Std_sockaddr_in*)_tmp6C,
_tmp6E);_LL34: _tmp6F=_tmp6A.f1;if(*((int*)_tmp6F)!= 1)goto _LL36;_tmp70=((struct
Cyc_Std_SA_sockaddr_struct*)_tmp6F)->f1;_tmp71=_tmp6A.f2;if(*((int*)_tmp71)!= 2)
goto _LL36;_tmp72=((struct Cyc_Std_SA_socklenptr_struct*)_tmp71)->f1;_LL35: return
recvfrom(fd,buf,n,flags,(const struct Cyc_Std_sockaddr*)_tmp70,_tmp72);_LL36:;
_LL37:(int)_throw((void*)({struct Cyc_Std_SocketError_struct*_tmp73=_cycalloc(
sizeof(*_tmp73));_tmp73[0]=({struct Cyc_Std_SocketError_struct _tmp74;_tmp74.tag=
Cyc_Std_SocketError;_tmp74.f1=_tag_arr("recvfrom---bad args",sizeof(char),20);
_tmp74;});_tmp73;}));_LL31:;}}int Cyc_Std_sendto(int fd,struct _tagged_arr buf,
unsigned int n,int flags,struct _tagged_arr ap){if(_get_arr_size(ap,sizeof(void*))!= 
2)(int)_throw((void*)({struct Cyc_Std_SocketError_struct*_tmp75=_cycalloc(sizeof(*
_tmp75));_tmp75[0]=({struct Cyc_Std_SocketError_struct _tmp76;_tmp76.tag=Cyc_Std_SocketError;
_tmp76.f1=_tag_arr("sendto---need 2 args",sizeof(char),21);_tmp76;});_tmp75;}));{
struct _tuple0 _tmp78=({struct _tuple0 _tmp77;_tmp77.f1=*((void**)
_check_unknown_subscript(ap,sizeof(void*),0));_tmp77.f2=*((void**)
_check_unknown_subscript(ap,sizeof(void*),1));_tmp77;});void*_tmp79;struct Cyc_Std_sockaddr_in*
_tmp7A;void*_tmp7B;unsigned int _tmp7C;void*_tmp7D;struct Cyc_Std_sockaddr*_tmp7E;
void*_tmp7F;unsigned int _tmp80;_LL39: _tmp79=_tmp78.f1;if(*((int*)_tmp79)!= 0)
goto _LL3B;_tmp7A=((struct Cyc_Std_SA_sockaddr_in_struct*)_tmp79)->f1;_tmp7B=
_tmp78.f2;if(*((int*)_tmp7B)!= 3)goto _LL3B;_tmp7C=((struct Cyc_Std_SA_socklen_struct*)
_tmp7B)->f1;_LL3A: return sendto_in(fd,buf,n,flags,(const struct Cyc_Std_sockaddr_in*)
_tmp7A,_tmp7C);_LL3B: _tmp7D=_tmp78.f1;if(*((int*)_tmp7D)!= 1)goto _LL3D;_tmp7E=((
struct Cyc_Std_SA_sockaddr_struct*)_tmp7D)->f1;_tmp7F=_tmp78.f2;if(*((int*)_tmp7F)
!= 3)goto _LL3D;_tmp80=((struct Cyc_Std_SA_socklen_struct*)_tmp7F)->f1;_LL3C:
return sendto(fd,buf,n,flags,(const struct Cyc_Std_sockaddr*)_tmp7E,_tmp80);_LL3D:;
_LL3E:(int)_throw((void*)({struct Cyc_Std_SocketError_struct*_tmp81=_cycalloc(
sizeof(*_tmp81));_tmp81[0]=({struct Cyc_Std_SocketError_struct _tmp82;_tmp82.tag=
Cyc_Std_SocketError;_tmp82.f1=_tag_arr("sendto---bad args",sizeof(char),18);
_tmp82;});_tmp81;}));_LL38:;}}int Cyc_Std_send(int fd,struct _tagged_arr buf,
unsigned int n,int flags){return send_wrapped(fd,buf,n,flags);}int Cyc_Std_recv(int
fd,struct _tagged_arr buf,unsigned int n,int flags){return recv_wrapped(fd,buf,n,
flags);}int Cyc_Std_getsockopt(int fd,int level,int optname,struct _tagged_arr ap){if(
_get_arr_size(ap,sizeof(void*))!= 2)(int)_throw((void*)({struct Cyc_Std_SocketError_struct*
_tmp83=_cycalloc(sizeof(*_tmp83));_tmp83[0]=({struct Cyc_Std_SocketError_struct
_tmp84;_tmp84.tag=Cyc_Std_SocketError;_tmp84.f1=_tag_arr("getsockopt---need 2 args",
sizeof(char),25);_tmp84;});_tmp83;}));{struct _tuple0 _tmp86=({struct _tuple0 _tmp85;
_tmp85.f1=*((void**)_check_unknown_subscript(ap,sizeof(void*),0));_tmp85.f2=*((
void**)_check_unknown_subscript(ap,sizeof(void*),1));_tmp85;});void*_tmp87;int*
_tmp88;void*_tmp89;unsigned int*_tmp8A;void*_tmp8B;int*_tmp8C;void*_tmp8D;int*
_tmp8E;void*_tmp8F;struct Cyc_Std_timeval*_tmp90;void*_tmp91;unsigned int*_tmp92;
void*_tmp93;struct Cyc_Std_timeval*_tmp94;void*_tmp95;int*_tmp96;_LL40: _tmp87=
_tmp86.f1;if(*((int*)_tmp87)!= 0)goto _LL42;_tmp88=((struct Cyc_Std_SO_int_struct*)
_tmp87)->f1;_tmp89=_tmp86.f2;if(*((int*)_tmp89)!= 2)goto _LL42;_tmp8A=((struct Cyc_Std_SO_socklenptr_struct*)
_tmp89)->f1;_LL41: return getsockopt_int(fd,level,optname,_tmp88,_tmp8A);_LL42:
_tmp8B=_tmp86.f1;if(*((int*)_tmp8B)!= 0)goto _LL44;_tmp8C=((struct Cyc_Std_SO_int_struct*)
_tmp8B)->f1;_tmp8D=_tmp86.f2;if(*((int*)_tmp8D)!= 0)goto _LL44;_tmp8E=((struct Cyc_Std_SO_int_struct*)
_tmp8D)->f1;_LL43: return getsockopt_int(fd,level,optname,_tmp8C,(unsigned int*)
_tmp8E);_LL44: _tmp8F=_tmp86.f1;if(*((int*)_tmp8F)!= 1)goto _LL46;_tmp90=((struct
Cyc_Std_SO_timeval_struct*)_tmp8F)->f1;_tmp91=_tmp86.f2;if(*((int*)_tmp91)!= 2)
goto _LL46;_tmp92=((struct Cyc_Std_SO_socklenptr_struct*)_tmp91)->f1;_LL45: return
getsockopt_timeval(fd,level,optname,_tmp90,_tmp92);_LL46: _tmp93=_tmp86.f1;if(*((
int*)_tmp93)!= 1)goto _LL48;_tmp94=((struct Cyc_Std_SO_timeval_struct*)_tmp93)->f1;
_tmp95=_tmp86.f2;if(*((int*)_tmp95)!= 0)goto _LL48;_tmp96=((struct Cyc_Std_SO_int_struct*)
_tmp95)->f1;_LL47: return getsockopt_timeval(fd,level,optname,_tmp94,(unsigned int*)
_tmp96);_LL48:;_LL49:(int)_throw((void*)({struct Cyc_Std_SocketError_struct*
_tmp97=_cycalloc(sizeof(*_tmp97));_tmp97[0]=({struct Cyc_Std_SocketError_struct
_tmp98;_tmp98.tag=Cyc_Std_SocketError;_tmp98.f1=(struct _tagged_arr)({struct Cyc_Std_String_pa_struct
_tmp9A;_tmp9A.tag=0;_tmp9A.f1=(struct _tagged_arr)Cyc_Std_sopts2string(ap);{void*
_tmp99[1]={& _tmp9A};Cyc_Std_aprintf(_tag_arr("getsockopt---bad args %s",sizeof(
char),25),_tag_arr(_tmp99,sizeof(void*),1));}});_tmp98;});_tmp97;}));_LL3F:;}}
int Cyc_Std_setsockopt(int fd,int level,int optname,struct _tagged_arr ap){if(
_get_arr_size(ap,sizeof(void*))!= 2)(int)_throw((void*)({struct Cyc_Std_SocketError_struct*
_tmp9B=_cycalloc(sizeof(*_tmp9B));_tmp9B[0]=({struct Cyc_Std_SocketError_struct
_tmp9C;_tmp9C.tag=Cyc_Std_SocketError;_tmp9C.f1=_tag_arr("setsockopt---need 2 args",
sizeof(char),25);_tmp9C;});_tmp9B;}));{struct _tuple0 _tmp9E=({struct _tuple0 _tmp9D;
_tmp9D.f1=*((void**)_check_unknown_subscript(ap,sizeof(void*),0));_tmp9D.f2=*((
void**)_check_unknown_subscript(ap,sizeof(void*),1));_tmp9D;});void*_tmp9F;int*
_tmpA0;void*_tmpA1;unsigned int _tmpA2;void*_tmpA3;struct Cyc_Std_timeval*_tmpA4;
void*_tmpA5;unsigned int _tmpA6;_LL4B: _tmp9F=_tmp9E.f1;if(*((int*)_tmp9F)!= 0)
goto _LL4D;_tmpA0=((struct Cyc_Std_SO_int_struct*)_tmp9F)->f1;_tmpA1=_tmp9E.f2;if(*((
int*)_tmpA1)!= 3)goto _LL4D;_tmpA2=((struct Cyc_Std_SO_socklen_struct*)_tmpA1)->f1;
_LL4C: return setsockopt_int(fd,level,optname,(const int*)_tmpA0,_tmpA2);_LL4D:
_tmpA3=_tmp9E.f1;if(*((int*)_tmpA3)!= 1)goto _LL4F;_tmpA4=((struct Cyc_Std_SO_timeval_struct*)
_tmpA3)->f1;_tmpA5=_tmp9E.f2;if(*((int*)_tmpA5)!= 3)goto _LL4F;_tmpA6=((struct Cyc_Std_SO_socklen_struct*)
_tmpA5)->f1;_LL4E: return setsockopt_timeval(fd,level,optname,(const struct Cyc_Std_timeval*)
_tmpA4,_tmpA6);_LL4F:;_LL50:(int)_throw((void*)({struct Cyc_Std_SocketError_struct*
_tmpA7=_cycalloc(sizeof(*_tmpA7));_tmpA7[0]=({struct Cyc_Std_SocketError_struct
_tmpA8;_tmpA8.tag=Cyc_Std_SocketError;_tmpA8.f1=(struct _tagged_arr)({struct Cyc_Std_String_pa_struct
_tmpAA;_tmpAA.tag=0;_tmpAA.f1=(struct _tagged_arr)Cyc_Std_sopts2string(ap);{void*
_tmpA9[1]={& _tmpAA};Cyc_Std_aprintf(_tag_arr("getsockopt---bad args %s",sizeof(
char),25),_tag_arr(_tmpA9,sizeof(void*),1));}});_tmpA8;});_tmpA7;}));_LL4A:;}}
char*asctime(const struct Cyc_Std_tm*timeptr);char*ctime(const int*timep);
unsigned int strftime(char*s,unsigned int maxsize,char*fmt,const struct Cyc_Std_tm*t);
struct _tagged_arr Cyc_Std_asctime(const struct Cyc_Std_tm*timeptr){return
wrap_Cstring_as_string(asctime(timeptr),- 1);}struct _tagged_arr Cyc_Std_ctime(
const int*timep){return wrap_Cstring_as_string(ctime(timep),- 1);}unsigned int Cyc_Std_strftime(
struct _tagged_arr s,unsigned int maxsize,struct _tagged_arr fmt,const struct Cyc_Std_tm*
t){unsigned int m=_get_arr_size(s,sizeof(char))< maxsize?_get_arr_size(s,sizeof(
char)): maxsize;return strftime(underlying_Cstring(s),m,underlying_Cstring(fmt),t);}
struct _tagged_arr Cyc_Std_asctime_r(const struct Cyc_Std_tm*t,struct _tagged_arr s){
struct _tagged_arr _tmpAB=wrap_Cstring_as_string(asctime(t),- 1);if(Cyc_Std_strlen((
struct _tagged_arr)_tmpAB)+ 1 > _get_arr_size(s,sizeof(char)))(int)_throw((void*)({
struct Cyc_Core_Invalid_argument_struct*_tmpAC=_cycalloc(sizeof(*_tmpAC));_tmpAC[
0]=({struct Cyc_Core_Invalid_argument_struct _tmpAD;_tmpAD.tag=Cyc_Core_Invalid_argument;
_tmpAD.f1=_tag_arr("Time::asctime_r: string too small",sizeof(char),34);_tmpAD;});
_tmpAC;}));else{Cyc_Std_strcpy(s,(struct _tagged_arr)_tmpAB);return s;}}struct
_tagged_arr Cyc_Std_ctime_r(const int*t,struct _tagged_arr s){struct _tagged_arr
_tmpAE=wrap_Cstring_as_string(ctime(t),- 1);if(Cyc_Std_strlen((struct _tagged_arr)
_tmpAE)+ 1 > _get_arr_size(s,sizeof(char)))(int)_throw((void*)({struct Cyc_Core_Invalid_argument_struct*
_tmpAF=_cycalloc(sizeof(*_tmpAF));_tmpAF[0]=({struct Cyc_Core_Invalid_argument_struct
_tmpB0;_tmpB0.tag=Cyc_Core_Invalid_argument;_tmpB0.f1=_tag_arr("Time::ctime_r: string too small",
sizeof(char),32);_tmpB0;});_tmpAF;}));else{Cyc_Std_strcpy(s,(struct _tagged_arr)
_tmpAE);return s;}}struct Cyc_Std_option{struct _tagged_arr name;int has_arg;int*flag;
int val;};int Cyc_Std_access(struct _tagged_arr,int);int Cyc_Std_chdir(struct
_tagged_arr);int Cyc_Std_chown(struct _tagged_arr,unsigned short,unsigned short);
struct _tagged_arr Cyc_Std_getcwd(struct _tagged_arr buf,unsigned int size);int Cyc_Std_execl(
struct _tagged_arr path,struct _tagged_arr arg0,struct _tagged_arr argv);int Cyc_Std_execlp(
struct _tagged_arr file,struct _tagged_arr arg0,struct _tagged_arr argv);int Cyc_Std_execve(
struct _tagged_arr filename,struct _tagged_arr argv,struct _tagged_arr envp);int Cyc_Std_link(
struct _tagged_arr,struct _tagged_arr);int Cyc_Std_read(int fd,struct _tagged_arr buf,
unsigned int count);int Cyc_Std_rmdir(struct _tagged_arr);int Cyc_Std_symlink(struct
_tagged_arr,struct _tagged_arr);int Cyc_Std_truncate(struct _tagged_arr,int);int Cyc_Std_write(
int fd,struct _tagged_arr buf,unsigned int count);int Cyc_Std_unlink(struct
_tagged_arr pathname);int Cyc_Std_gethostname(struct _tagged_arr,unsigned int);int
Cyc_Std_chroot(struct _tagged_arr);struct _tagged_arr Cyc_Std_getpass(struct
_tagged_arr prompt);int access(char*,int);int chdir(char*);int chown(char*,
unsigned short,unsigned short);char*getcwd(char*buf,unsigned int size);int execv(
char*path,char**argv);int execvp(char*file,char**argv);int execve(char*path,char**
argv,char**envp);int link(char*path1,char*path2);int read(int fd,char*buf,
unsigned int count);int rmdir(char*);int symlink(char*path1,char*path2);int truncate(
char*,int);int write(int fd,char*buf,unsigned int count);int unlink(char*pathname);
int gethostname(char*,unsigned int);int chroot(char*);char*getpass(char*);int Cyc_Std_access(
struct _tagged_arr path,int mode){return access(string_to_Cstring(path),mode);}int
Cyc_Std_chdir(struct _tagged_arr path){return chdir(string_to_Cstring(path));}int
Cyc_Std_chown(struct _tagged_arr path,unsigned short owner,unsigned short group){
return chown(string_to_Cstring(path),owner,group);}struct _tagged_arr Cyc_Std_getcwd(
struct _tagged_arr buf,unsigned int size){if(!((unsigned int)buf.curr)?1:
_get_arr_size(buf,sizeof(char))< size)(int)_throw((void*)({struct Cyc_Core_Failure_struct*
_tmpB1=_cycalloc(sizeof(*_tmpB1));_tmpB1[0]=({struct Cyc_Core_Failure_struct
_tmpB2;_tmpB2.tag=Cyc_Core_Failure;_tmpB2.f1=_tag_arr("getcwd: invalid buf argument",
sizeof(char),29);_tmpB2;});_tmpB1;}));{char*response=getcwd((char*)_check_null(
_untag_arr(buf,sizeof(char),0)),size);return(unsigned int)response?buf: _tag_arr(
0,0,0);}}int Cyc_Std_execl(struct _tagged_arr path,struct _tagged_arr arg0,struct
_tagged_arr argv){if((*((struct _tagged_arr*)_check_unknown_subscript(argv,sizeof(
struct _tagged_arr),(int)(_get_arr_size(argv,sizeof(struct _tagged_arr))- 1)))).curr
!= ((struct _tagged_arr)_tag_arr(0,0,0)).curr)(int)_throw((void*)({struct Cyc_Core_Failure_struct*
_tmpB3=_cycalloc(sizeof(*_tmpB3));_tmpB3[0]=({struct Cyc_Core_Failure_struct
_tmpB4;_tmpB4.tag=Cyc_Core_Failure;_tmpB4.f1=_tag_arr("execl: arg list must be NULL-terminated",
sizeof(char),40);_tmpB4;});_tmpB3;}));{struct _tagged_arr newargs=({unsigned int
_tmpB5=1 + _get_arr_size(argv,sizeof(struct _tagged_arr));char**_tmpB6=(char**)
_cycalloc(_check_times(sizeof(char*),_tmpB5));struct _tagged_arr _tmpB8=_tag_arr(
_tmpB6,sizeof(char*),1 + _get_arr_size(argv,sizeof(struct _tagged_arr)));{
unsigned int _tmpB7=_tmpB5;unsigned int i;for(i=0;i < _tmpB7;i ++){_tmpB6[i]=0;}}
_tmpB8;});*((char**)_check_unknown_subscript(newargs,sizeof(char*),0))=
string_to_Cstring(arg0);{int i=0;for(0;i < _get_arr_size(argv,sizeof(struct
_tagged_arr));i ++){*((char**)_check_unknown_subscript(newargs,sizeof(char*),i + 1))=
string_to_Cstring(((struct _tagged_arr*)argv.curr)[i]);}}return execv(
string_to_Cstring(path),(char**)_check_null(_untag_arr(newargs,sizeof(char*),1)));}}
int Cyc_Std_execlp(struct _tagged_arr path,struct _tagged_arr arg0,struct _tagged_arr
argv){if((*((struct _tagged_arr*)_check_unknown_subscript(argv,sizeof(struct
_tagged_arr),(int)(_get_arr_size(argv,sizeof(struct _tagged_arr))- 1)))).curr != ((
struct _tagged_arr)_tag_arr(0,0,0)).curr)(int)_throw((void*)({struct Cyc_Core_Failure_struct*
_tmpB9=_cycalloc(sizeof(*_tmpB9));_tmpB9[0]=({struct Cyc_Core_Failure_struct
_tmpBA;_tmpBA.tag=Cyc_Core_Failure;_tmpBA.f1=_tag_arr("execl: arg list must be NULL-terminated",
sizeof(char),40);_tmpBA;});_tmpB9;}));{struct _tagged_arr newargs=({unsigned int
_tmpBB=1 + _get_arr_size(argv,sizeof(struct _tagged_arr));char**_tmpBC=(char**)
_cycalloc(_check_times(sizeof(char*),_tmpBB));struct _tagged_arr _tmpBE=_tag_arr(
_tmpBC,sizeof(char*),1 + _get_arr_size(argv,sizeof(struct _tagged_arr)));{
unsigned int _tmpBD=_tmpBB;unsigned int i;for(i=0;i < _tmpBD;i ++){_tmpBC[i]=0;}}
_tmpBE;});*((char**)_check_unknown_subscript(newargs,sizeof(char*),0))=
string_to_Cstring(arg0);{int i=0;for(0;i < _get_arr_size(argv,sizeof(struct
_tagged_arr));i ++){*((char**)_check_unknown_subscript(newargs,sizeof(char*),i + 1))=
string_to_Cstring(((struct _tagged_arr*)argv.curr)[i]);}}return execvp(
string_to_Cstring(path),(char**)_check_null(_untag_arr(newargs,sizeof(char*),1)));}}
int Cyc_Std_execve(struct _tagged_arr filename,struct _tagged_arr argv,struct
_tagged_arr envp){if((*((struct _tagged_arr*)_check_unknown_subscript(argv,sizeof(
struct _tagged_arr),(int)(_get_arr_size(argv,sizeof(struct _tagged_arr))- 1)))).curr
!= (_tag_arr(0,0,0)).curr)(int)_throw((void*)({struct Cyc_Core_Failure_struct*
_tmpBF=_cycalloc(sizeof(*_tmpBF));_tmpBF[0]=({struct Cyc_Core_Failure_struct
_tmpC0;_tmpC0.tag=Cyc_Core_Failure;_tmpC0.f1=_tag_arr("execve: arg list must be NULL-terminated",
sizeof(char),41);_tmpC0;});_tmpBF;}));{struct _tagged_arr newargs=({unsigned int
_tmpC5=_get_arr_size(argv,sizeof(struct _tagged_arr));char**_tmpC6=(char**)
_cycalloc(_check_times(sizeof(char*),_tmpC5));struct _tagged_arr _tmpC8=_tag_arr(
_tmpC6,sizeof(char*),_get_arr_size(argv,sizeof(struct _tagged_arr)));{
unsigned int _tmpC7=_tmpC5;unsigned int i;for(i=0;i < _tmpC7;i ++){_tmpC6[i]=0;}}
_tmpC8;});{int i=0;for(0;i < _get_arr_size(argv,sizeof(struct _tagged_arr));i ++){((
char**)newargs.curr)[i]=string_to_Cstring((struct _tagged_arr)((struct _tagged_arr*)
argv.curr)[i]);}}{struct _tagged_arr newenvp=({unsigned int _tmpC1=_get_arr_size(
envp,sizeof(struct _tagged_arr));char**_tmpC2=(char**)_cycalloc(_check_times(
sizeof(char*),_tmpC1));struct _tagged_arr _tmpC4=_tag_arr(_tmpC2,sizeof(char*),
_get_arr_size(envp,sizeof(struct _tagged_arr)));{unsigned int _tmpC3=_tmpC1;
unsigned int i;for(i=0;i < _tmpC3;i ++){_tmpC2[i]=0;}}_tmpC4;});{int i=0;for(0;i < 
_get_arr_size(envp,sizeof(struct _tagged_arr));i ++){((char**)newenvp.curr)[i]=
string_to_Cstring((struct _tagged_arr)((struct _tagged_arr*)envp.curr)[i]);}}
return execve(string_to_Cstring(filename),(char**)_check_null(_untag_arr(newargs,
sizeof(char*),1)),(char**)_check_null(_untag_arr(newenvp,sizeof(char*),1)));}}}
int Cyc_Std_link(struct _tagged_arr path1,struct _tagged_arr path2){return link(
string_to_Cstring(path1),string_to_Cstring(path2));}int Cyc_Std_read(int fd,struct
_tagged_arr buf,unsigned int count){if(count > _get_arr_size(buf,sizeof(char)))(int)
_throw((void*)({struct Cyc_Core_Failure_struct*_tmpC9=_cycalloc(sizeof(*_tmpC9));
_tmpC9[0]=({struct Cyc_Core_Failure_struct _tmpCA;_tmpCA.tag=Cyc_Core_Failure;
_tmpCA.f1=_tag_arr("read: called with count > buf.size",sizeof(char),35);_tmpCA;});
_tmpC9;}));return read(fd,underlying_Cstring((struct _tagged_arr)buf),count);}int
Cyc_Std_rmdir(struct _tagged_arr path){return rmdir(string_to_Cstring(path));}int
Cyc_Std_symlink(struct _tagged_arr path1,struct _tagged_arr path2){return symlink(
string_to_Cstring(path1),string_to_Cstring(path2));}int Cyc_Std_truncate(struct
_tagged_arr path,int length){return truncate(string_to_Cstring(path),length);}int
Cyc_Std_write(int fd,struct _tagged_arr buf,unsigned int count){if(count > 
_get_arr_size(buf,sizeof(char)))(int)_throw((void*)({struct Cyc_Core_Failure_struct*
_tmpCB=_cycalloc(sizeof(*_tmpCB));_tmpCB[0]=({struct Cyc_Core_Failure_struct
_tmpCC;_tmpCC.tag=Cyc_Core_Failure;_tmpCC.f1=_tag_arr("write: called with count > buf.size",
sizeof(char),36);_tmpCC;});_tmpCB;}));return write(fd,underlying_Cstring(buf),
count);}int Cyc_Std_unlink(struct _tagged_arr pathname){return unlink(
string_to_Cstring(pathname));}int Cyc_Std_gethostname(struct _tagged_arr buf,
unsigned int count){if(count > _get_arr_size(buf,sizeof(char)))(int)_throw((void*)({
struct Cyc_Core_Failure_struct*_tmpCD=_cycalloc(sizeof(*_tmpCD));_tmpCD[0]=({
struct Cyc_Core_Failure_struct _tmpCE;_tmpCE.tag=Cyc_Core_Failure;_tmpCE.f1=
_tag_arr("gethostname: called with count > buf.size",sizeof(char),42);_tmpCE;});
_tmpCD;}));return gethostname(underlying_Cstring((struct _tagged_arr)buf),count);}
int Cyc_Std_chroot(struct _tagged_arr pathname){return chroot(string_to_Cstring(
pathname));}struct _tagged_arr Cyc_Std_getpass(struct _tagged_arr prompt){return
wrap_Cstring_as_string(getpass(string_to_Cstring(prompt)),- 1);}struct Cyc_utimbuf{
int actime;int modtime;};int utime(char*filename,struct Cyc_utimbuf*buf);int Cyc_Std_utime(
struct _tagged_arr filename,struct Cyc_utimbuf*buf){return utime(string_to_Cstring(
filename),buf);}