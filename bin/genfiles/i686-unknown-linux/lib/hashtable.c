 extern void exit( int); extern void* abort(); struct Cyc_Core_Opt{ void* v; } ;
extern unsigned char Cyc_Core_InvalidArg[ 15u]; struct Cyc_Core_InvalidArg_struct{
unsigned char* tag; struct _tagged_arr f1; } ; extern unsigned char Cyc_Core_Failure[
12u]; struct Cyc_Core_Failure_struct{ unsigned char* tag; struct _tagged_arr f1;
} ; extern unsigned char Cyc_Core_Impossible[ 15u]; struct Cyc_Core_Impossible_struct{
unsigned char* tag; struct _tagged_arr f1; } ; extern unsigned char Cyc_Core_Not_found[
14u]; extern unsigned char Cyc_Core_Unreachable[ 16u]; struct Cyc_Core_Unreachable_struct{
unsigned char* tag; struct _tagged_arr f1; } ; extern unsigned char*
string_to_Cstring( struct _tagged_arr); extern unsigned char* underlying_Cstring(
struct _tagged_arr); extern struct _tagged_arr Cstring_to_string( unsigned char*);
extern struct _tagged_arr wrap_Cstring_as_string( unsigned char*, unsigned int);
extern struct _tagged_arr ntCsl_to_ntsl( unsigned char**); extern int system(
unsigned char*); struct Cyc_List_List{ void* hd; struct Cyc_List_List* tl; } ;
extern int Cyc_List_length( struct Cyc_List_List* x); extern unsigned char Cyc_List_List_empty[
15u]; extern unsigned char Cyc_List_List_mismatch[ 18u]; extern unsigned char
Cyc_List_Nth[ 8u]; extern void* Cyc_List_assoc_cmp( int(* compare)( void*, void*),
struct Cyc_List_List* l, void* x); struct Cyc_Hashtable_Table; extern struct Cyc_Hashtable_Table*
Cyc_Hashtable_create( int sz, int(* cmp)( void*, void*), int(* hash)( void*));
extern void Cyc_Hashtable_insert( struct Cyc_Hashtable_Table* t, void* key, void*
val); extern void* Cyc_Hashtable_lookup( struct Cyc_Hashtable_Table* t, void*
key); extern void Cyc_Hashtable_resize( struct Cyc_Hashtable_Table* t); extern
void Cyc_Hashtable_remove( struct Cyc_Hashtable_Table* t, void* key); extern int
Cyc_Hashtable_hash_string( struct _tagged_arr s); extern int Cyc_Hashtable_hash_stringptr(
struct _tagged_arr* s); extern void Cyc_Hashtable_iter( void(* f)( void*, void*),
struct Cyc_Hashtable_Table* t); extern void Cyc_Hashtable_print_table_map(
struct Cyc_Hashtable_Table* t, void(* prn_key)( void*), void(* prn_val)( void*));
struct Cyc_std___sFILE; extern unsigned char Cyc_std_FileCloseError[ 19u];
extern unsigned char Cyc_std_FileOpenError[ 18u]; struct Cyc_std_FileOpenError_struct{
unsigned char* tag; struct _tagged_arr f1; } ; static const int Cyc_std_String_pa=
0; struct Cyc_std_String_pa_struct{ int tag; struct _tagged_arr f1; } ; static
const int Cyc_std_Int_pa= 1; struct Cyc_std_Int_pa_struct{ int tag; unsigned int
f1; } ; static const int Cyc_std_Double_pa= 2; struct Cyc_std_Double_pa_struct{
int tag; double f1; } ; static const int Cyc_std_ShortPtr_pa= 3; struct Cyc_std_ShortPtr_pa_struct{
int tag; short* f1; } ; static const int Cyc_std_IntPtr_pa= 4; struct Cyc_std_IntPtr_pa_struct{
int tag; unsigned int* f1; } ; extern int Cyc_std_printf( struct _tagged_arr fmt,
struct _tagged_arr); static const int Cyc_std_ShortPtr_sa= 0; struct Cyc_std_ShortPtr_sa_struct{
int tag; short* f1; } ; static const int Cyc_std_UShortPtr_sa= 1; struct Cyc_std_UShortPtr_sa_struct{
int tag; unsigned short* f1; } ; static const int Cyc_std_IntPtr_sa= 2; struct
Cyc_std_IntPtr_sa_struct{ int tag; int* f1; } ; static const int Cyc_std_UIntPtr_sa=
3; struct Cyc_std_UIntPtr_sa_struct{ int tag; unsigned int* f1; } ; static const
int Cyc_std_StringPtr_sa= 4; struct Cyc_std_StringPtr_sa_struct{ int tag; struct
_tagged_arr f1; } ; static const int Cyc_std_DoublePtr_sa= 5; struct Cyc_std_DoublePtr_sa_struct{
int tag; double* f1; } ; static const int Cyc_std_FloatPtr_sa= 6; struct Cyc_std_FloatPtr_sa_struct{
int tag; float* f1; } ; struct Cyc_Hashtable_Table{ int(* cmp)( void*, void*);
int(* hash)( void*); int max_len; struct _tagged_arr tab; } ; struct Cyc_Hashtable_Table*
Cyc_Hashtable_create( int sz, int(* cmp)( void*, void*), int(* hash)( void*)){
struct Cyc_List_List* mt= 0; return({ struct Cyc_Hashtable_Table* _temp0=(
struct Cyc_Hashtable_Table*) GC_malloc( sizeof( struct Cyc_Hashtable_Table));
_temp0->cmp= cmp; _temp0->hash= hash; _temp0->max_len= 3; _temp0->tab=({
unsigned int _temp1=( unsigned int) sz; struct Cyc_List_List** _temp2=( struct
Cyc_List_List**) GC_malloc( sizeof( struct Cyc_List_List*) *  _temp1); struct
_tagged_arr _temp4= _tag_arr( _temp2, sizeof( struct Cyc_List_List*),(
unsigned int) sz);{ unsigned int _temp3= _temp1; unsigned int i; for( i= 0; i < 
_temp3; i ++){ _temp2[ i]= mt;}}; _temp4;}); _temp0;});} struct _tuple0{ void*
f1; void* f2; } ; void Cyc_Hashtable_insert( struct Cyc_Hashtable_Table* t, void*
key, void* val){ struct _tagged_arr tab= t->tab; int bucket=( int)((* t->hash)(
key) %  _get_arr_size( tab, sizeof( struct Cyc_List_List*)));*(( struct Cyc_List_List**)
_check_unknown_subscript( tab, sizeof( struct Cyc_List_List*), bucket))=({
struct Cyc_List_List* _temp5=( struct Cyc_List_List*) GC_malloc( sizeof( struct
Cyc_List_List)); _temp5->hd=( void*)({ struct _tuple0* _temp6=( struct _tuple0*)
GC_malloc( sizeof( struct _tuple0)); _temp6->f1= key; _temp6->f2= val; _temp6;});
_temp5->tl=*(( struct Cyc_List_List**) _check_unknown_subscript( tab, sizeof(
struct Cyc_List_List*), bucket)); _temp5;}); if((( int(*)( struct Cyc_List_List*
x)) Cyc_List_length)(*(( struct Cyc_List_List**) _check_unknown_subscript( tab,
sizeof( struct Cyc_List_List*), bucket))) >  t->max_len){ Cyc_Hashtable_resize(
t);}} void* Cyc_Hashtable_lookup( struct Cyc_Hashtable_Table* t, void* key){
struct _tagged_arr tab= t->tab; struct Cyc_List_List* l=*(( struct Cyc_List_List**)
_check_unknown_subscript( tab, sizeof( struct Cyc_List_List*),( int)((* t->hash)(
key) %  _get_arr_size( tab, sizeof( struct Cyc_List_List*))))); return Cyc_List_assoc_cmp(
t->cmp, l, key);} void Cyc_Hashtable_remove( struct Cyc_Hashtable_Table* t, void*
key){ struct _tagged_arr tab= t->tab; int(* cmp)( void*, void*)= t->cmp; int
bucket=( int)((* t->hash)( key) %  _get_arr_size( tab, sizeof( struct Cyc_List_List*)));
struct Cyc_List_List* l=*(( struct Cyc_List_List**) _check_unknown_subscript(
tab, sizeof( struct Cyc_List_List*), bucket)); if( l ==  0){ return;} if((* cmp)(
key,((( struct _tuple0*)(( struct Cyc_List_List*) _check_null( l))->hd)[
_check_known_subscript_notnull( 1u, 0)]).f1) ==  0){*(( struct Cyc_List_List**)
_check_unknown_subscript( tab, sizeof( struct Cyc_List_List*), bucket))=((
struct Cyc_List_List*) _check_null( l))->tl; return;}{ struct Cyc_List_List*
prev= l; l=(( struct Cyc_List_List*) _check_null( l))->tl; for( 0;(( struct Cyc_List_List*)
_check_null( l))->tl !=  0;( prev= l, l=(( struct Cyc_List_List*) _check_null( l))->tl)){
if((* cmp)( key,((( struct _tuple0*)(( struct Cyc_List_List*) _check_null( l))->hd)[
_check_known_subscript_notnull( 1u, 0)]).f1) ==  0){(( struct Cyc_List_List*)
_check_null( prev))->tl=(( struct Cyc_List_List*) _check_null( l))->tl; return;}}}}
int Cyc_Hashtable_hash_string( struct _tagged_arr s){ int ans= 0; int sz=( int)
_get_arr_size( s, sizeof( unsigned char)); int shift= 0;{ int i= 0; for( 0; i < 
sz; ++ i){ ans= ans ^ *(( const unsigned char*) _check_unknown_subscript( s,
sizeof( unsigned char), i)) <<  shift; shift += 8; if( shift ==  32){ shift= 0;}}}
return ans;} int Cyc_Hashtable_hash_stringptr( struct _tagged_arr* s){ return
Cyc_Hashtable_hash_string(* s);} void Cyc_Hashtable_insert_bucket( struct
_tagged_arr tab, int(* hash)( void*), struct Cyc_List_List* elems){ if( elems == 
0){ return;} Cyc_Hashtable_insert_bucket( tab, hash,(( struct Cyc_List_List*)
_check_null( elems))->tl);{ void* key=((( struct _tuple0*)(( struct Cyc_List_List*)
_check_null( elems))->hd)[ _check_known_subscript_notnull( 1u, 0)]).f1; void*
val=((( struct _tuple0*)(( struct Cyc_List_List*) _check_null( elems))->hd)[
_check_known_subscript_notnull( 1u, 0)]).f2; int nidx=( int)((* hash)( key) % 
_get_arr_size( tab, sizeof( struct Cyc_List_List*)));*(( struct Cyc_List_List**)
_check_unknown_subscript( tab, sizeof( struct Cyc_List_List*), nidx))=( struct
Cyc_List_List*)({ struct Cyc_List_List* _temp7=( struct Cyc_List_List*)
GC_malloc( sizeof( struct Cyc_List_List)); _temp7->hd=( void*)(( struct _tuple0*)({
struct _tuple0* _temp8=( struct _tuple0*) GC_malloc( sizeof( struct _tuple0));
_temp8->f1= key; _temp8->f2= val; _temp8;})); _temp7->tl=*(( struct Cyc_List_List**)
_check_unknown_subscript( tab, sizeof( struct Cyc_List_List*), nidx)); _temp7;});}}
void Cyc_Hashtable_resize( struct Cyc_Hashtable_Table* t){ struct _tagged_arr
odata= t->tab; int osize=( int) _get_arr_size( odata, sizeof( struct Cyc_List_List*));
int nsize= 2 *  osize +  1; struct Cyc_List_List* mt= 0; struct _tagged_arr
ndata=({ unsigned int _temp9=( unsigned int) nsize; struct Cyc_List_List**
_temp10=( struct Cyc_List_List**) GC_malloc( sizeof( struct Cyc_List_List*) * 
_temp9); struct _tagged_arr _temp12= _tag_arr( _temp10, sizeof( struct Cyc_List_List*),(
unsigned int) nsize);{ unsigned int _temp11= _temp9; unsigned int i; for( i= 0;
i <  _temp11; i ++){ _temp10[ i]= mt;}}; _temp12;});{ int i= 0; for( 0; i < 
osize; i ++){ Cyc_Hashtable_insert_bucket( ndata, t->hash,*(( struct Cyc_List_List**)
_check_unknown_subscript( odata, sizeof( struct Cyc_List_List*), i)));}} t->tab=
ndata; t->max_len= 2 *  t->max_len;} void Cyc_Hashtable_iter( void(* f)( void*,
void*), struct Cyc_Hashtable_Table* t){ struct _tagged_arr odata= t->tab; int
osize=( int) _get_arr_size( odata, sizeof( struct Cyc_List_List*)); int i= 0;
for( 0; i <  osize; i ++){ struct Cyc_List_List* iter=*(( struct Cyc_List_List**)
_check_unknown_subscript( odata, sizeof( struct Cyc_List_List*), i)); for( 0;
iter !=  0; iter=(( struct Cyc_List_List*) _check_null( iter))->tl){ f((((
struct _tuple0*)(( struct Cyc_List_List*) _check_null( iter))->hd)[
_check_known_subscript_notnull( 1u, 0)]).f1,((( struct _tuple0*)(( struct Cyc_List_List*)
_check_null( iter))->hd)[ _check_known_subscript_notnull( 1u, 0)]).f2);}}} void
Cyc_Hashtable_print_table_map( struct Cyc_Hashtable_Table* t, void(* prn_key)(
void*), void(* prn_val)( void*)){ struct _tagged_arr odata= t->tab; int osize=(
int) _get_arr_size( odata, sizeof( struct Cyc_List_List*)); int i= 0; for( 0; i
<  osize; i ++){({ struct Cyc_std_Int_pa_struct _temp14; _temp14.tag= Cyc_std_Int_pa;
_temp14.f1=( int)(( unsigned int) i);{ void* _temp13[ 1u]={& _temp14}; Cyc_std_printf(
_tag_arr("%d: ", sizeof( unsigned char), 5u), _tag_arr( _temp13, sizeof( void*),
1u));}});{ struct Cyc_List_List* iter=*(( struct Cyc_List_List**)
_check_unknown_subscript( odata, sizeof( struct Cyc_List_List*), i)); for( 0;
iter !=  0; iter=(( struct Cyc_List_List*) _check_null( iter))->tl){({ void*
_temp15[ 0u]={}; Cyc_std_printf( _tag_arr("(", sizeof( unsigned char), 2u),
_tag_arr( _temp15, sizeof( void*), 0u));}); prn_key(((( struct _tuple0*)((
struct Cyc_List_List*) _check_null( iter))->hd)[ _check_known_subscript_notnull(
1u, 0)]).f1);({ void* _temp16[ 0u]={}; Cyc_std_printf( _tag_arr(",", sizeof(
unsigned char), 2u), _tag_arr( _temp16, sizeof( void*), 0u));}); prn_val((((
struct _tuple0*)(( struct Cyc_List_List*) _check_null( iter))->hd)[
_check_known_subscript_notnull( 1u, 0)]).f2);({ void* _temp17[ 0u]={}; Cyc_std_printf(
_tag_arr(") ", sizeof( unsigned char), 3u), _tag_arr( _temp17, sizeof( void*), 0u));});}}({
void* _temp18[ 0u]={}; Cyc_std_printf( _tag_arr("\n", sizeof( unsigned char), 2u),
_tag_arr( _temp18, sizeof( void*), 0u));});}}
