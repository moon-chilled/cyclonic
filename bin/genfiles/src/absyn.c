 struct Cyc_timespec{ unsigned int tv_sec; int tv_nsec; } ; struct Cyc_itimerspec{
struct Cyc_timespec it_interval; struct Cyc_timespec it_value; } ; extern void
exit( int); extern void* abort(); struct Cyc_Core_Opt{ void* v; } ; extern
unsigned char Cyc_Core_InvalidArg[ 15u]; struct Cyc_Core_InvalidArg_struct{
unsigned char* tag; struct _tagged_arr f1; } ; extern unsigned char Cyc_Core_Failure[
12u]; struct Cyc_Core_Failure_struct{ unsigned char* tag; struct _tagged_arr f1;
} ; extern unsigned char Cyc_Core_Impossible[ 15u]; struct Cyc_Core_Impossible_struct{
unsigned char* tag; struct _tagged_arr f1; } ; extern unsigned char Cyc_Core_Not_found[
14u]; extern unsigned char Cyc_Core_Unreachable[ 16u]; struct Cyc_Core_Unreachable_struct{
unsigned char* tag; struct _tagged_arr f1; } ; extern unsigned char*
string_to_Cstring( struct _tagged_arr); extern unsigned char* underlying_Cstring(
struct _tagged_arr); extern struct _tagged_arr Cstring_to_string( unsigned char*);
extern struct _tagged_arr wrap_Cstring_as_string( unsigned char*, int); extern
struct _tagged_arr ntCsl_to_ntsl( unsigned char**); extern int system(
unsigned char*); extern int* __errno(); struct Cyc_List_List{ void* hd; struct
Cyc_List_List* tl; } ; extern struct Cyc_List_List* Cyc_List_list( struct
_tagged_arr); extern unsigned char Cyc_List_List_empty[ 15u]; extern struct Cyc_List_List*
Cyc_List_map( void*(* f)( void*), struct Cyc_List_List* x); extern unsigned char
Cyc_List_List_mismatch[ 18u]; extern struct Cyc_List_List* Cyc_List_imp_rev(
struct Cyc_List_List* x); extern unsigned char Cyc_List_Nth[ 8u]; extern int Cyc_List_list_cmp(
int(* cmp)( void*, void*), struct Cyc_List_List* l1, struct Cyc_List_List* l2);
struct Cyc_Lineno_Pos{ struct _tagged_arr logical_file; struct _tagged_arr line;
int line_no; int col; } ; extern unsigned char Cyc_Position_Exit[ 9u]; struct
Cyc_Position_Segment; static const int Cyc_Position_Lex= 0; static const int Cyc_Position_Parse=
1; static const int Cyc_Position_Elab= 2; struct Cyc_Position_Error{ struct
_tagged_arr source; struct Cyc_Position_Segment* seg; void* kind; struct
_tagged_arr desc; } ; extern unsigned char Cyc_Position_Nocontext[ 14u]; struct
_tuple0{ void* f1; struct _tagged_arr* f2; } ; struct Cyc_Absyn_Tvar; struct Cyc_Absyn_Tqual;
struct Cyc_Absyn_Conref; struct Cyc_Absyn_PtrInfo; struct Cyc_Absyn_VarargInfo;
struct Cyc_Absyn_FnInfo; struct Cyc_Absyn_TunionInfo; struct Cyc_Absyn_TunionFieldInfo;
struct Cyc_Absyn_VarargCallInfo; struct Cyc_Absyn_Exp; struct Cyc_Absyn_Stmt;
struct Cyc_Absyn_Pat; struct Cyc_Absyn_Switch_clause; struct Cyc_Absyn_SwitchC_clause;
struct Cyc_Absyn_Fndecl; struct Cyc_Absyn_Structdecl; struct Cyc_Absyn_Uniondecl;
struct Cyc_Absyn_Tuniondecl; struct Cyc_Absyn_Tunionfield; struct Cyc_Absyn_Enumfield;
struct Cyc_Absyn_Enumdecl; struct Cyc_Absyn_Typedefdecl; struct Cyc_Absyn_Vardecl;
struct Cyc_Absyn_Decl; struct Cyc_Absyn_Structfield; static const int Cyc_Absyn_Loc_n=
0; static const int Cyc_Absyn_Rel_n= 0; struct Cyc_Absyn_Rel_n_struct{ int tag;
struct Cyc_List_List* f1; } ; static const int Cyc_Absyn_Abs_n= 1; struct Cyc_Absyn_Abs_n_struct{
int tag; struct Cyc_List_List* f1; } ; static const int Cyc_Absyn_Static= 0;
static const int Cyc_Absyn_Abstract= 1; static const int Cyc_Absyn_Public= 2;
static const int Cyc_Absyn_Extern= 3; static const int Cyc_Absyn_ExternC= 4;
struct Cyc_Absyn_Tqual{ int q_const: 1; int q_volatile: 1; int q_restrict: 1; }
; static const int Cyc_Absyn_B1= 0; static const int Cyc_Absyn_B2= 1; static
const int Cyc_Absyn_B4= 2; static const int Cyc_Absyn_B8= 3; static const int
Cyc_Absyn_AnyKind= 0; static const int Cyc_Absyn_MemKind= 1; static const int
Cyc_Absyn_BoxKind= 2; static const int Cyc_Absyn_RgnKind= 3; static const int
Cyc_Absyn_EffKind= 4; static const int Cyc_Absyn_Signed= 0; static const int Cyc_Absyn_Unsigned=
1; struct Cyc_Absyn_Conref{ void* v; } ; static const int Cyc_Absyn_Eq_constr= 0;
struct Cyc_Absyn_Eq_constr_struct{ int tag; void* f1; } ; static const int Cyc_Absyn_Forward_constr=
1; struct Cyc_Absyn_Forward_constr_struct{ int tag; struct Cyc_Absyn_Conref* f1;
} ; static const int Cyc_Absyn_No_constr= 0; struct Cyc_Absyn_Tvar{ struct
_tagged_arr* name; int* identity; struct Cyc_Absyn_Conref* kind; } ; static
const int Cyc_Absyn_Unknown_b= 0; static const int Cyc_Absyn_Upper_b= 0; struct
Cyc_Absyn_Upper_b_struct{ int tag; struct Cyc_Absyn_Exp* f1; } ; struct Cyc_Absyn_PtrInfo{
void* elt_typ; void* rgn_typ; struct Cyc_Absyn_Conref* nullable; struct Cyc_Absyn_Tqual
tq; struct Cyc_Absyn_Conref* bounds; } ; struct Cyc_Absyn_VarargInfo{ struct Cyc_Core_Opt*
name; struct Cyc_Absyn_Tqual tq; void* type; void* rgn; int inject; } ; struct
Cyc_Absyn_FnInfo{ struct Cyc_List_List* tvars; struct Cyc_Core_Opt* effect; void*
ret_typ; struct Cyc_List_List* args; int c_varargs; struct Cyc_Absyn_VarargInfo*
cyc_varargs; struct Cyc_List_List* rgn_po; struct Cyc_List_List* attributes; } ;
struct Cyc_Absyn_UnknownTunionInfo{ struct _tuple0* name; int is_xtunion; } ;
static const int Cyc_Absyn_UnknownTunion= 0; struct Cyc_Absyn_UnknownTunion_struct{
int tag; struct Cyc_Absyn_UnknownTunionInfo f1; } ; static const int Cyc_Absyn_KnownTunion=
1; struct Cyc_Absyn_KnownTunion_struct{ int tag; struct Cyc_Absyn_Tuniondecl* f1;
} ; struct Cyc_Absyn_TunionInfo{ void* tunion_info; struct Cyc_List_List* targs;
void* rgn; } ; struct Cyc_Absyn_UnknownTunionFieldInfo{ struct _tuple0*
tunion_name; struct _tuple0* field_name; int is_xtunion; } ; static const int
Cyc_Absyn_UnknownTunionfield= 0; struct Cyc_Absyn_UnknownTunionfield_struct{ int
tag; struct Cyc_Absyn_UnknownTunionFieldInfo f1; } ; static const int Cyc_Absyn_KnownTunionfield=
1; struct Cyc_Absyn_KnownTunionfield_struct{ int tag; struct Cyc_Absyn_Tuniondecl*
f1; struct Cyc_Absyn_Tunionfield* f2; } ; struct Cyc_Absyn_TunionFieldInfo{ void*
field_info; struct Cyc_List_List* targs; } ; static const int Cyc_Absyn_VoidType=
0; static const int Cyc_Absyn_Evar= 0; struct Cyc_Absyn_Evar_struct{ int tag;
struct Cyc_Core_Opt* f1; struct Cyc_Core_Opt* f2; int f3; struct Cyc_Core_Opt*
f4; } ; static const int Cyc_Absyn_VarType= 1; struct Cyc_Absyn_VarType_struct{
int tag; struct Cyc_Absyn_Tvar* f1; } ; static const int Cyc_Absyn_TunionType= 2;
struct Cyc_Absyn_TunionType_struct{ int tag; struct Cyc_Absyn_TunionInfo f1; } ;
static const int Cyc_Absyn_TunionFieldType= 3; struct Cyc_Absyn_TunionFieldType_struct{
int tag; struct Cyc_Absyn_TunionFieldInfo f1; } ; static const int Cyc_Absyn_PointerType=
4; struct Cyc_Absyn_PointerType_struct{ int tag; struct Cyc_Absyn_PtrInfo f1; }
; static const int Cyc_Absyn_IntType= 5; struct Cyc_Absyn_IntType_struct{ int
tag; void* f1; void* f2; } ; static const int Cyc_Absyn_FloatType= 1; static
const int Cyc_Absyn_DoubleType= 2; static const int Cyc_Absyn_ArrayType= 6;
struct Cyc_Absyn_ArrayType_struct{ int tag; void* f1; struct Cyc_Absyn_Tqual f2;
struct Cyc_Absyn_Exp* f3; } ; static const int Cyc_Absyn_FnType= 7; struct Cyc_Absyn_FnType_struct{
int tag; struct Cyc_Absyn_FnInfo f1; } ; static const int Cyc_Absyn_TupleType= 8;
struct Cyc_Absyn_TupleType_struct{ int tag; struct Cyc_List_List* f1; } ; static
const int Cyc_Absyn_StructType= 9; struct Cyc_Absyn_StructType_struct{ int tag;
struct _tuple0* f1; struct Cyc_List_List* f2; struct Cyc_Absyn_Structdecl** f3;
} ; static const int Cyc_Absyn_UnionType= 10; struct Cyc_Absyn_UnionType_struct{
int tag; struct _tuple0* f1; struct Cyc_List_List* f2; struct Cyc_Absyn_Uniondecl**
f3; } ; static const int Cyc_Absyn_AnonStructType= 11; struct Cyc_Absyn_AnonStructType_struct{
int tag; struct Cyc_List_List* f1; } ; static const int Cyc_Absyn_AnonUnionType=
12; struct Cyc_Absyn_AnonUnionType_struct{ int tag; struct Cyc_List_List* f1; }
; static const int Cyc_Absyn_EnumType= 13; struct Cyc_Absyn_EnumType_struct{ int
tag; struct _tuple0* f1; struct Cyc_Absyn_Enumdecl* f2; } ; static const int Cyc_Absyn_RgnHandleType=
14; struct Cyc_Absyn_RgnHandleType_struct{ int tag; void* f1; } ; static const
int Cyc_Absyn_TypedefType= 15; struct Cyc_Absyn_TypedefType_struct{ int tag;
struct _tuple0* f1; struct Cyc_List_List* f2; struct Cyc_Core_Opt* f3; } ;
static const int Cyc_Absyn_HeapRgn= 3; static const int Cyc_Absyn_AccessEff= 16;
struct Cyc_Absyn_AccessEff_struct{ int tag; void* f1; } ; static const int Cyc_Absyn_JoinEff=
17; struct Cyc_Absyn_JoinEff_struct{ int tag; struct Cyc_List_List* f1; } ;
static const int Cyc_Absyn_NoTypes= 0; struct Cyc_Absyn_NoTypes_struct{ int tag;
struct Cyc_List_List* f1; struct Cyc_Position_Segment* f2; } ; static const int
Cyc_Absyn_WithTypes= 1; struct Cyc_Absyn_WithTypes_struct{ int tag; struct Cyc_List_List*
f1; int f2; struct Cyc_Absyn_VarargInfo* f3; struct Cyc_Core_Opt* f4; struct Cyc_List_List*
f5; } ; static const int Cyc_Absyn_NonNullable_ps= 0; struct Cyc_Absyn_NonNullable_ps_struct{
int tag; struct Cyc_Absyn_Exp* f1; } ; static const int Cyc_Absyn_Nullable_ps= 1;
struct Cyc_Absyn_Nullable_ps_struct{ int tag; struct Cyc_Absyn_Exp* f1; } ;
static const int Cyc_Absyn_TaggedArray_ps= 0; static const int Cyc_Absyn_Regparm_att=
0; struct Cyc_Absyn_Regparm_att_struct{ int tag; int f1; } ; static const int
Cyc_Absyn_Stdcall_att= 0; static const int Cyc_Absyn_Cdecl_att= 1; static const
int Cyc_Absyn_Fastcall_att= 2; static const int Cyc_Absyn_Noreturn_att= 3;
static const int Cyc_Absyn_Const_att= 4; static const int Cyc_Absyn_Aligned_att=
1; struct Cyc_Absyn_Aligned_att_struct{ int tag; int f1; } ; static const int
Cyc_Absyn_Packed_att= 5; static const int Cyc_Absyn_Section_att= 2; struct Cyc_Absyn_Section_att_struct{
int tag; struct _tagged_arr f1; } ; static const int Cyc_Absyn_Nocommon_att= 6;
static const int Cyc_Absyn_Shared_att= 7; static const int Cyc_Absyn_Unused_att=
8; static const int Cyc_Absyn_Weak_att= 9; static const int Cyc_Absyn_Dllimport_att=
10; static const int Cyc_Absyn_Dllexport_att= 11; static const int Cyc_Absyn_No_instrument_function_att=
12; static const int Cyc_Absyn_Constructor_att= 13; static const int Cyc_Absyn_Destructor_att=
14; static const int Cyc_Absyn_No_check_memory_usage_att= 15; static const int
Cyc_Absyn_Carray_mod= 0; static const int Cyc_Absyn_ConstArray_mod= 0; struct
Cyc_Absyn_ConstArray_mod_struct{ int tag; struct Cyc_Absyn_Exp* f1; } ; static
const int Cyc_Absyn_Pointer_mod= 1; struct Cyc_Absyn_Pointer_mod_struct{ int tag;
void* f1; void* f2; struct Cyc_Absyn_Tqual f3; } ; static const int Cyc_Absyn_Function_mod=
2; struct Cyc_Absyn_Function_mod_struct{ int tag; void* f1; } ; static const int
Cyc_Absyn_TypeParams_mod= 3; struct Cyc_Absyn_TypeParams_mod_struct{ int tag;
struct Cyc_List_List* f1; struct Cyc_Position_Segment* f2; int f3; } ; static
const int Cyc_Absyn_Attributes_mod= 4; struct Cyc_Absyn_Attributes_mod_struct{
int tag; struct Cyc_Position_Segment* f1; struct Cyc_List_List* f2; } ; static
const int Cyc_Absyn_Char_c= 0; struct Cyc_Absyn_Char_c_struct{ int tag; void* f1;
unsigned char f2; } ; static const int Cyc_Absyn_Short_c= 1; struct Cyc_Absyn_Short_c_struct{
int tag; void* f1; short f2; } ; static const int Cyc_Absyn_Int_c= 2; struct Cyc_Absyn_Int_c_struct{
int tag; void* f1; int f2; } ; static const int Cyc_Absyn_LongLong_c= 3; struct
Cyc_Absyn_LongLong_c_struct{ int tag; void* f1; long long f2; } ; static const
int Cyc_Absyn_Float_c= 4; struct Cyc_Absyn_Float_c_struct{ int tag; struct
_tagged_arr f1; } ; static const int Cyc_Absyn_String_c= 5; struct Cyc_Absyn_String_c_struct{
int tag; struct _tagged_arr f1; } ; static const int Cyc_Absyn_Null_c= 0; static
const int Cyc_Absyn_Plus= 0; static const int Cyc_Absyn_Times= 1; static const
int Cyc_Absyn_Minus= 2; static const int Cyc_Absyn_Div= 3; static const int Cyc_Absyn_Mod=
4; static const int Cyc_Absyn_Eq= 5; static const int Cyc_Absyn_Neq= 6; static
const int Cyc_Absyn_Gt= 7; static const int Cyc_Absyn_Lt= 8; static const int
Cyc_Absyn_Gte= 9; static const int Cyc_Absyn_Lte= 10; static const int Cyc_Absyn_Not=
11; static const int Cyc_Absyn_Bitnot= 12; static const int Cyc_Absyn_Bitand= 13;
static const int Cyc_Absyn_Bitor= 14; static const int Cyc_Absyn_Bitxor= 15;
static const int Cyc_Absyn_Bitlshift= 16; static const int Cyc_Absyn_Bitlrshift=
17; static const int Cyc_Absyn_Bitarshift= 18; static const int Cyc_Absyn_Size=
19; static const int Cyc_Absyn_Printf= 20; static const int Cyc_Absyn_Fprintf=
21; static const int Cyc_Absyn_Xprintf= 22; static const int Cyc_Absyn_Scanf= 23;
static const int Cyc_Absyn_Fscanf= 24; static const int Cyc_Absyn_Sscanf= 25;
static const int Cyc_Absyn_PreInc= 0; static const int Cyc_Absyn_PostInc= 1;
static const int Cyc_Absyn_PreDec= 2; static const int Cyc_Absyn_PostDec= 3;
struct Cyc_Absyn_VarargCallInfo{ int num_varargs; struct Cyc_List_List*
injectors; struct Cyc_Absyn_VarargInfo* vai; } ; static const int Cyc_Absyn_Const_e=
0; struct Cyc_Absyn_Const_e_struct{ int tag; void* f1; } ; static const int Cyc_Absyn_Var_e=
1; struct Cyc_Absyn_Var_e_struct{ int tag; struct _tuple0* f1; void* f2; } ;
static const int Cyc_Absyn_UnknownId_e= 2; struct Cyc_Absyn_UnknownId_e_struct{
int tag; struct _tuple0* f1; } ; static const int Cyc_Absyn_Primop_e= 3; struct
Cyc_Absyn_Primop_e_struct{ int tag; void* f1; struct Cyc_List_List* f2; } ;
static const int Cyc_Absyn_AssignOp_e= 4; struct Cyc_Absyn_AssignOp_e_struct{
int tag; struct Cyc_Absyn_Exp* f1; struct Cyc_Core_Opt* f2; struct Cyc_Absyn_Exp*
f3; } ; static const int Cyc_Absyn_Increment_e= 5; struct Cyc_Absyn_Increment_e_struct{
int tag; struct Cyc_Absyn_Exp* f1; void* f2; } ; static const int Cyc_Absyn_Conditional_e=
6; struct Cyc_Absyn_Conditional_e_struct{ int tag; struct Cyc_Absyn_Exp* f1;
struct Cyc_Absyn_Exp* f2; struct Cyc_Absyn_Exp* f3; } ; static const int Cyc_Absyn_SeqExp_e=
7; struct Cyc_Absyn_SeqExp_e_struct{ int tag; struct Cyc_Absyn_Exp* f1; struct
Cyc_Absyn_Exp* f2; } ; static const int Cyc_Absyn_UnknownCall_e= 8; struct Cyc_Absyn_UnknownCall_e_struct{
int tag; struct Cyc_Absyn_Exp* f1; struct Cyc_List_List* f2; } ; static const
int Cyc_Absyn_FnCall_e= 9; struct Cyc_Absyn_FnCall_e_struct{ int tag; struct Cyc_Absyn_Exp*
f1; struct Cyc_List_List* f2; struct Cyc_Absyn_VarargCallInfo* f3; } ; static
const int Cyc_Absyn_Throw_e= 10; struct Cyc_Absyn_Throw_e_struct{ int tag;
struct Cyc_Absyn_Exp* f1; } ; static const int Cyc_Absyn_NoInstantiate_e= 11;
struct Cyc_Absyn_NoInstantiate_e_struct{ int tag; struct Cyc_Absyn_Exp* f1; } ;
static const int Cyc_Absyn_Instantiate_e= 12; struct Cyc_Absyn_Instantiate_e_struct{
int tag; struct Cyc_Absyn_Exp* f1; struct Cyc_List_List* f2; } ; static const
int Cyc_Absyn_Cast_e= 13; struct Cyc_Absyn_Cast_e_struct{ int tag; void* f1;
struct Cyc_Absyn_Exp* f2; } ; static const int Cyc_Absyn_Address_e= 14; struct
Cyc_Absyn_Address_e_struct{ int tag; struct Cyc_Absyn_Exp* f1; } ; static const
int Cyc_Absyn_New_e= 15; struct Cyc_Absyn_New_e_struct{ int tag; struct Cyc_Absyn_Exp*
f1; struct Cyc_Absyn_Exp* f2; } ; static const int Cyc_Absyn_Sizeoftyp_e= 16;
struct Cyc_Absyn_Sizeoftyp_e_struct{ int tag; void* f1; } ; static const int Cyc_Absyn_Sizeofexp_e=
17; struct Cyc_Absyn_Sizeofexp_e_struct{ int tag; struct Cyc_Absyn_Exp* f1; } ;
static const int Cyc_Absyn_Offsetof_e= 18; struct Cyc_Absyn_Offsetof_e_struct{
int tag; void* f1; struct _tagged_arr* f2; } ; static const int Cyc_Absyn_Deref_e=
19; struct Cyc_Absyn_Deref_e_struct{ int tag; struct Cyc_Absyn_Exp* f1; } ;
static const int Cyc_Absyn_StructMember_e= 20; struct Cyc_Absyn_StructMember_e_struct{
int tag; struct Cyc_Absyn_Exp* f1; struct _tagged_arr* f2; } ; static const int
Cyc_Absyn_StructArrow_e= 21; struct Cyc_Absyn_StructArrow_e_struct{ int tag;
struct Cyc_Absyn_Exp* f1; struct _tagged_arr* f2; } ; static const int Cyc_Absyn_Subscript_e=
22; struct Cyc_Absyn_Subscript_e_struct{ int tag; struct Cyc_Absyn_Exp* f1;
struct Cyc_Absyn_Exp* f2; } ; static const int Cyc_Absyn_Tuple_e= 23; struct Cyc_Absyn_Tuple_e_struct{
int tag; struct Cyc_List_List* f1; } ; static const int Cyc_Absyn_CompoundLit_e=
24; struct _tuple1{ struct Cyc_Core_Opt* f1; struct Cyc_Absyn_Tqual f2; void* f3;
} ; struct Cyc_Absyn_CompoundLit_e_struct{ int tag; struct _tuple1* f1; struct
Cyc_List_List* f2; } ; static const int Cyc_Absyn_Array_e= 25; struct Cyc_Absyn_Array_e_struct{
int tag; struct Cyc_List_List* f1; } ; static const int Cyc_Absyn_Comprehension_e=
26; struct Cyc_Absyn_Comprehension_e_struct{ int tag; struct Cyc_Absyn_Vardecl*
f1; struct Cyc_Absyn_Exp* f2; struct Cyc_Absyn_Exp* f3; } ; static const int Cyc_Absyn_Struct_e=
27; struct Cyc_Absyn_Struct_e_struct{ int tag; struct _tuple0* f1; struct Cyc_Core_Opt*
f2; struct Cyc_List_List* f3; struct Cyc_Absyn_Structdecl* f4; } ; static const
int Cyc_Absyn_AnonStruct_e= 28; struct Cyc_Absyn_AnonStruct_e_struct{ int tag;
void* f1; struct Cyc_List_List* f2; } ; static const int Cyc_Absyn_Tunion_e= 29;
struct Cyc_Absyn_Tunion_e_struct{ int tag; struct Cyc_Core_Opt* f1; struct Cyc_Core_Opt*
f2; struct Cyc_List_List* f3; struct Cyc_Absyn_Tuniondecl* f4; struct Cyc_Absyn_Tunionfield*
f5; } ; static const int Cyc_Absyn_Enum_e= 30; struct Cyc_Absyn_Enum_e_struct{
int tag; struct _tuple0* f1; struct Cyc_Absyn_Enumdecl* f2; struct Cyc_Absyn_Enumfield*
f3; } ; static const int Cyc_Absyn_Malloc_e= 31; struct Cyc_Absyn_Malloc_e_struct{
int tag; struct Cyc_Absyn_Exp* f1; void* f2; } ; static const int Cyc_Absyn_UnresolvedMem_e=
32; struct Cyc_Absyn_UnresolvedMem_e_struct{ int tag; struct Cyc_Core_Opt* f1;
struct Cyc_List_List* f2; } ; static const int Cyc_Absyn_StmtExp_e= 33; struct
Cyc_Absyn_StmtExp_e_struct{ int tag; struct Cyc_Absyn_Stmt* f1; } ; static const
int Cyc_Absyn_Codegen_e= 34; struct Cyc_Absyn_Codegen_e_struct{ int tag; struct
Cyc_Absyn_Fndecl* f1; } ; static const int Cyc_Absyn_Fill_e= 35; struct Cyc_Absyn_Fill_e_struct{
int tag; struct Cyc_Absyn_Exp* f1; } ; struct Cyc_Absyn_Exp{ struct Cyc_Core_Opt*
topt; void* r; struct Cyc_Position_Segment* loc; } ; static const int Cyc_Absyn_Skip_s=
0; static const int Cyc_Absyn_Exp_s= 0; struct Cyc_Absyn_Exp_s_struct{ int tag;
struct Cyc_Absyn_Exp* f1; } ; static const int Cyc_Absyn_Seq_s= 1; struct Cyc_Absyn_Seq_s_struct{
int tag; struct Cyc_Absyn_Stmt* f1; struct Cyc_Absyn_Stmt* f2; } ; static const
int Cyc_Absyn_Return_s= 2; struct Cyc_Absyn_Return_s_struct{ int tag; struct Cyc_Absyn_Exp*
f1; } ; static const int Cyc_Absyn_IfThenElse_s= 3; struct Cyc_Absyn_IfThenElse_s_struct{
int tag; struct Cyc_Absyn_Exp* f1; struct Cyc_Absyn_Stmt* f2; struct Cyc_Absyn_Stmt*
f3; } ; static const int Cyc_Absyn_While_s= 4; struct _tuple2{ struct Cyc_Absyn_Exp*
f1; struct Cyc_Absyn_Stmt* f2; } ; struct Cyc_Absyn_While_s_struct{ int tag;
struct _tuple2 f1; struct Cyc_Absyn_Stmt* f2; } ; static const int Cyc_Absyn_Break_s=
5; struct Cyc_Absyn_Break_s_struct{ int tag; struct Cyc_Absyn_Stmt* f1; } ;
static const int Cyc_Absyn_Continue_s= 6; struct Cyc_Absyn_Continue_s_struct{
int tag; struct Cyc_Absyn_Stmt* f1; } ; static const int Cyc_Absyn_Goto_s= 7;
struct Cyc_Absyn_Goto_s_struct{ int tag; struct _tagged_arr* f1; struct Cyc_Absyn_Stmt*
f2; } ; static const int Cyc_Absyn_For_s= 8; struct Cyc_Absyn_For_s_struct{ int
tag; struct Cyc_Absyn_Exp* f1; struct _tuple2 f2; struct _tuple2 f3; struct Cyc_Absyn_Stmt*
f4; } ; static const int Cyc_Absyn_Switch_s= 9; struct Cyc_Absyn_Switch_s_struct{
int tag; struct Cyc_Absyn_Exp* f1; struct Cyc_List_List* f2; } ; static const
int Cyc_Absyn_SwitchC_s= 10; struct Cyc_Absyn_SwitchC_s_struct{ int tag; struct
Cyc_Absyn_Exp* f1; struct Cyc_List_List* f2; } ; static const int Cyc_Absyn_Fallthru_s=
11; struct Cyc_Absyn_Fallthru_s_struct{ int tag; struct Cyc_List_List* f1;
struct Cyc_Absyn_Switch_clause** f2; } ; static const int Cyc_Absyn_Decl_s= 12;
struct Cyc_Absyn_Decl_s_struct{ int tag; struct Cyc_Absyn_Decl* f1; struct Cyc_Absyn_Stmt*
f2; } ; static const int Cyc_Absyn_Cut_s= 13; struct Cyc_Absyn_Cut_s_struct{ int
tag; struct Cyc_Absyn_Stmt* f1; } ; static const int Cyc_Absyn_Splice_s= 14;
struct Cyc_Absyn_Splice_s_struct{ int tag; struct Cyc_Absyn_Stmt* f1; } ; static
const int Cyc_Absyn_Label_s= 15; struct Cyc_Absyn_Label_s_struct{ int tag;
struct _tagged_arr* f1; struct Cyc_Absyn_Stmt* f2; } ; static const int Cyc_Absyn_Do_s=
16; struct Cyc_Absyn_Do_s_struct{ int tag; struct Cyc_Absyn_Stmt* f1; struct
_tuple2 f2; } ; static const int Cyc_Absyn_TryCatch_s= 17; struct Cyc_Absyn_TryCatch_s_struct{
int tag; struct Cyc_Absyn_Stmt* f1; struct Cyc_List_List* f2; } ; static const
int Cyc_Absyn_Region_s= 18; struct Cyc_Absyn_Region_s_struct{ int tag; struct
Cyc_Absyn_Tvar* f1; struct Cyc_Absyn_Vardecl* f2; struct Cyc_Absyn_Stmt* f3; } ;
struct Cyc_Absyn_Stmt{ void* r; struct Cyc_Position_Segment* loc; struct Cyc_List_List*
non_local_preds; int try_depth; void* annot; } ; static const int Cyc_Absyn_Wild_p=
0; static const int Cyc_Absyn_Var_p= 0; struct Cyc_Absyn_Var_p_struct{ int tag;
struct Cyc_Absyn_Vardecl* f1; } ; static const int Cyc_Absyn_Null_p= 1; static
const int Cyc_Absyn_Int_p= 1; struct Cyc_Absyn_Int_p_struct{ int tag; void* f1;
int f2; } ; static const int Cyc_Absyn_Char_p= 2; struct Cyc_Absyn_Char_p_struct{
int tag; unsigned char f1; } ; static const int Cyc_Absyn_Float_p= 3; struct Cyc_Absyn_Float_p_struct{
int tag; struct _tagged_arr f1; } ; static const int Cyc_Absyn_Tuple_p= 4;
struct Cyc_Absyn_Tuple_p_struct{ int tag; struct Cyc_List_List* f1; } ; static
const int Cyc_Absyn_Pointer_p= 5; struct Cyc_Absyn_Pointer_p_struct{ int tag;
struct Cyc_Absyn_Pat* f1; } ; static const int Cyc_Absyn_Reference_p= 6; struct
Cyc_Absyn_Reference_p_struct{ int tag; struct Cyc_Absyn_Vardecl* f1; } ; static
const int Cyc_Absyn_Struct_p= 7; struct Cyc_Absyn_Struct_p_struct{ int tag;
struct Cyc_Absyn_Structdecl* f1; struct Cyc_Core_Opt* f2; struct Cyc_List_List*
f3; struct Cyc_List_List* f4; } ; static const int Cyc_Absyn_Tunion_p= 8; struct
Cyc_Absyn_Tunion_p_struct{ int tag; struct Cyc_Absyn_Tuniondecl* f1; struct Cyc_Absyn_Tunionfield*
f2; struct Cyc_List_List* f3; struct Cyc_List_List* f4; } ; static const int Cyc_Absyn_Enum_p=
9; struct Cyc_Absyn_Enum_p_struct{ int tag; struct Cyc_Absyn_Enumdecl* f1;
struct Cyc_Absyn_Enumfield* f2; } ; static const int Cyc_Absyn_UnknownId_p= 10;
struct Cyc_Absyn_UnknownId_p_struct{ int tag; struct _tuple0* f1; } ; static
const int Cyc_Absyn_UnknownCall_p= 11; struct Cyc_Absyn_UnknownCall_p_struct{
int tag; struct _tuple0* f1; struct Cyc_List_List* f2; struct Cyc_List_List* f3;
} ; static const int Cyc_Absyn_UnknownFields_p= 12; struct Cyc_Absyn_UnknownFields_p_struct{
int tag; struct _tuple0* f1; struct Cyc_List_List* f2; struct Cyc_List_List* f3;
} ; struct Cyc_Absyn_Pat{ void* r; struct Cyc_Core_Opt* topt; struct Cyc_Position_Segment*
loc; } ; struct Cyc_Absyn_Switch_clause{ struct Cyc_Absyn_Pat* pattern; struct
Cyc_Core_Opt* pat_vars; struct Cyc_Absyn_Exp* where_clause; struct Cyc_Absyn_Stmt*
body; struct Cyc_Position_Segment* loc; } ; struct Cyc_Absyn_SwitchC_clause{
struct Cyc_Absyn_Exp* cnst_exp; struct Cyc_Absyn_Stmt* body; struct Cyc_Position_Segment*
loc; } ; static const int Cyc_Absyn_Unresolved_b= 0; static const int Cyc_Absyn_Global_b=
0; struct Cyc_Absyn_Global_b_struct{ int tag; struct Cyc_Absyn_Vardecl* f1; } ;
static const int Cyc_Absyn_Funname_b= 1; struct Cyc_Absyn_Funname_b_struct{ int
tag; struct Cyc_Absyn_Fndecl* f1; } ; static const int Cyc_Absyn_Param_b= 2;
struct Cyc_Absyn_Param_b_struct{ int tag; struct Cyc_Absyn_Vardecl* f1; } ;
static const int Cyc_Absyn_Local_b= 3; struct Cyc_Absyn_Local_b_struct{ int tag;
struct Cyc_Absyn_Vardecl* f1; } ; static const int Cyc_Absyn_Pat_b= 4; struct
Cyc_Absyn_Pat_b_struct{ int tag; struct Cyc_Absyn_Vardecl* f1; } ; struct Cyc_Absyn_Vardecl{
void* sc; struct _tuple0* name; struct Cyc_Absyn_Tqual tq; void* type; struct
Cyc_Absyn_Exp* initializer; struct Cyc_Core_Opt* rgn; struct Cyc_List_List*
attributes; } ; struct Cyc_Absyn_Fndecl{ void* sc; int is_inline; struct _tuple0*
name; struct Cyc_List_List* tvs; struct Cyc_Core_Opt* effect; void* ret_type;
struct Cyc_List_List* args; int c_varargs; struct Cyc_Absyn_VarargInfo*
cyc_varargs; struct Cyc_List_List* rgn_po; struct Cyc_Absyn_Stmt* body; struct
Cyc_Core_Opt* cached_typ; struct Cyc_Core_Opt* param_vardecls; struct Cyc_List_List*
attributes; } ; struct Cyc_Absyn_Structfield{ struct _tagged_arr* name; struct
Cyc_Absyn_Tqual tq; void* type; struct Cyc_Core_Opt* width; struct Cyc_List_List*
attributes; } ; struct Cyc_Absyn_Structdecl{ void* sc; struct Cyc_Core_Opt* name;
struct Cyc_List_List* tvs; struct Cyc_Core_Opt* fields; struct Cyc_List_List*
attributes; } ; struct Cyc_Absyn_Uniondecl{ void* sc; struct Cyc_Core_Opt* name;
struct Cyc_List_List* tvs; struct Cyc_Core_Opt* fields; struct Cyc_List_List*
attributes; } ; struct Cyc_Absyn_Tunionfield{ struct _tuple0* name; struct Cyc_List_List*
tvs; struct Cyc_List_List* typs; struct Cyc_Position_Segment* loc; void* sc; } ;
struct Cyc_Absyn_Tuniondecl{ void* sc; struct _tuple0* name; struct Cyc_List_List*
tvs; struct Cyc_Core_Opt* fields; int is_xtunion; } ; struct Cyc_Absyn_Enumfield{
struct _tuple0* name; struct Cyc_Absyn_Exp* tag; struct Cyc_Position_Segment*
loc; } ; struct Cyc_Absyn_Enumdecl{ void* sc; struct _tuple0* name; struct Cyc_Core_Opt*
fields; } ; struct Cyc_Absyn_Typedefdecl{ struct _tuple0* name; struct Cyc_List_List*
tvs; void* defn; } ; static const int Cyc_Absyn_Var_d= 0; struct Cyc_Absyn_Var_d_struct{
int tag; struct Cyc_Absyn_Vardecl* f1; } ; static const int Cyc_Absyn_Fn_d= 1;
struct Cyc_Absyn_Fn_d_struct{ int tag; struct Cyc_Absyn_Fndecl* f1; } ; static
const int Cyc_Absyn_Let_d= 2; struct Cyc_Absyn_Let_d_struct{ int tag; struct Cyc_Absyn_Pat*
f1; struct Cyc_Core_Opt* f2; struct Cyc_Core_Opt* f3; struct Cyc_Absyn_Exp* f4;
int f5; } ; static const int Cyc_Absyn_Letv_d= 3; struct Cyc_Absyn_Letv_d_struct{
int tag; struct Cyc_List_List* f1; } ; static const int Cyc_Absyn_Struct_d= 4;
struct Cyc_Absyn_Struct_d_struct{ int tag; struct Cyc_Absyn_Structdecl* f1; } ;
static const int Cyc_Absyn_Union_d= 5; struct Cyc_Absyn_Union_d_struct{ int tag;
struct Cyc_Absyn_Uniondecl* f1; } ; static const int Cyc_Absyn_Tunion_d= 6;
struct Cyc_Absyn_Tunion_d_struct{ int tag; struct Cyc_Absyn_Tuniondecl* f1; } ;
static const int Cyc_Absyn_Enum_d= 7; struct Cyc_Absyn_Enum_d_struct{ int tag;
struct Cyc_Absyn_Enumdecl* f1; } ; static const int Cyc_Absyn_Typedef_d= 8;
struct Cyc_Absyn_Typedef_d_struct{ int tag; struct Cyc_Absyn_Typedefdecl* f1; }
; static const int Cyc_Absyn_Namespace_d= 9; struct Cyc_Absyn_Namespace_d_struct{
int tag; struct _tagged_arr* f1; struct Cyc_List_List* f2; } ; static const int
Cyc_Absyn_Using_d= 10; struct Cyc_Absyn_Using_d_struct{ int tag; struct _tuple0*
f1; struct Cyc_List_List* f2; } ; static const int Cyc_Absyn_ExternC_d= 11;
struct Cyc_Absyn_ExternC_d_struct{ int tag; struct Cyc_List_List* f1; } ; struct
Cyc_Absyn_Decl{ void* r; struct Cyc_Position_Segment* loc; } ; static const int
Cyc_Absyn_ArrayElement= 0; struct Cyc_Absyn_ArrayElement_struct{ int tag; struct
Cyc_Absyn_Exp* f1; } ; static const int Cyc_Absyn_FieldName= 1; struct Cyc_Absyn_FieldName_struct{
int tag; struct _tagged_arr* f1; } ; unsigned char Cyc_Absyn_EmptyAnnot[ 15u]="\000\000\000\000EmptyAnnot";
extern int Cyc_Absyn_qvar_cmp( struct _tuple0*, struct _tuple0*); extern int Cyc_Absyn_varlist_cmp(
struct Cyc_List_List*, struct Cyc_List_List*); extern int Cyc_Absyn_tvar_cmp(
struct Cyc_Absyn_Tvar*, struct Cyc_Absyn_Tvar*); extern struct Cyc_Absyn_Tqual
Cyc_Absyn_const_tqual(); extern struct Cyc_Absyn_Tqual Cyc_Absyn_combine_tqual(
struct Cyc_Absyn_Tqual x, struct Cyc_Absyn_Tqual y); extern struct Cyc_Absyn_Tqual
Cyc_Absyn_empty_tqual(); extern struct Cyc_Absyn_Conref* Cyc_Absyn_new_conref(
void* x); extern struct Cyc_Absyn_Conref* Cyc_Absyn_empty_conref(); extern
struct Cyc_Absyn_Conref* Cyc_Absyn_compress_conref( struct Cyc_Absyn_Conref* x);
extern void* Cyc_Absyn_conref_val( struct Cyc_Absyn_Conref* x); extern void* Cyc_Absyn_conref_def(
void*, struct Cyc_Absyn_Conref* x); extern void* Cyc_Absyn_new_evar( struct Cyc_Core_Opt*
k, struct Cyc_Core_Opt* tenv); extern void* Cyc_Absyn_wildtyp( struct Cyc_Core_Opt*);
extern void* Cyc_Absyn_uchar_t; extern void* Cyc_Absyn_ushort_t; extern void*
Cyc_Absyn_uint_t; extern void* Cyc_Absyn_ulong_t; extern void* Cyc_Absyn_ulonglong_t;
extern void* Cyc_Absyn_schar_t; extern void* Cyc_Absyn_sshort_t; extern void*
Cyc_Absyn_sint_t; extern void* Cyc_Absyn_slong_t; extern void* Cyc_Absyn_slonglong_t;
extern void* Cyc_Absyn_float_t; extern void* Cyc_Absyn_double_t; extern struct
_tuple0* Cyc_Absyn_exn_name; extern struct Cyc_Absyn_Tuniondecl* Cyc_Absyn_exn_tud;
extern struct _tuple0* Cyc_Absyn_null_pointer_exn_name; extern struct _tuple0*
Cyc_Absyn_match_exn_name; extern struct Cyc_Absyn_Tunionfield* Cyc_Absyn_null_pointer_exn_tuf;
extern struct Cyc_Absyn_Tunionfield* Cyc_Absyn_match_exn_tuf; extern void* Cyc_Absyn_exn_typ;
extern void* Cyc_Absyn_string_typ( void* rgn); extern void* Cyc_Absyn_const_string_typ(
void* rgn); extern void* Cyc_Absyn_file_typ(); extern struct Cyc_Absyn_Exp* Cyc_Absyn_exp_unsigned_one;
extern void* Cyc_Absyn_bounds_one; extern void* Cyc_Absyn_starb_typ( void* t,
void* rgn, struct Cyc_Absyn_Tqual tq, void* b); extern void* Cyc_Absyn_atb_typ(
void* t, void* rgn, struct Cyc_Absyn_Tqual tq, void* b); extern void* Cyc_Absyn_star_typ(
void* t, void* rgn, struct Cyc_Absyn_Tqual tq); extern void* Cyc_Absyn_at_typ(
void* t, void* rgn, struct Cyc_Absyn_Tqual tq); extern void* Cyc_Absyn_cstar_typ(
void* t, struct Cyc_Absyn_Tqual tq); extern void* Cyc_Absyn_tagged_typ( void* t,
void* rgn, struct Cyc_Absyn_Tqual tq); extern void* Cyc_Absyn_void_star_typ();
extern struct Cyc_Core_Opt* Cyc_Absyn_void_star_typ_opt(); extern void* Cyc_Absyn_strct(
struct _tagged_arr* name); extern void* Cyc_Absyn_strctq( struct _tuple0* name);
extern void* Cyc_Absyn_unionq_typ( struct _tuple0* name); extern void* Cyc_Absyn_union_typ(
struct _tagged_arr* name); extern struct Cyc_Absyn_Exp* Cyc_Absyn_new_exp( void*,
struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp* Cyc_Absyn_New_exp(
struct Cyc_Absyn_Exp* rgn_handle, struct Cyc_Absyn_Exp*, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_copy_exp( struct Cyc_Absyn_Exp*); extern
struct Cyc_Absyn_Exp* Cyc_Absyn_const_exp( void*, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_null_exp( struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_bool_exp( int, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_true_exp( struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_false_exp( struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_int_exp( void*, int, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_signed_int_exp( int, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_uint_exp( unsigned int, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_char_exp( unsigned char c, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_float_exp( struct _tagged_arr f, struct
Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp* Cyc_Absyn_string_exp(
struct _tagged_arr s, struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp*
Cyc_Absyn_var_exp( struct _tuple0*, struct Cyc_Position_Segment*); extern struct
Cyc_Absyn_Exp* Cyc_Absyn_varb_exp( struct _tuple0*, void*, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_unknownid_exp( struct _tuple0*, struct
Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp* Cyc_Absyn_primop_exp( void*,
struct Cyc_List_List* es, struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp*
Cyc_Absyn_prim1_exp( void*, struct Cyc_Absyn_Exp*, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_prim2_exp( void*, struct Cyc_Absyn_Exp*,
struct Cyc_Absyn_Exp*, struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp*
Cyc_Absyn_add_exp( struct Cyc_Absyn_Exp*, struct Cyc_Absyn_Exp*, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_subtract_exp( struct Cyc_Absyn_Exp*,
struct Cyc_Absyn_Exp*, struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp*
Cyc_Absyn_times_exp( struct Cyc_Absyn_Exp*, struct Cyc_Absyn_Exp*, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_divide_exp( struct Cyc_Absyn_Exp*, struct
Cyc_Absyn_Exp*, struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp* Cyc_Absyn_eq_exp(
struct Cyc_Absyn_Exp*, struct Cyc_Absyn_Exp*, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_neq_exp( struct Cyc_Absyn_Exp*, struct
Cyc_Absyn_Exp*, struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp* Cyc_Absyn_gt_exp(
struct Cyc_Absyn_Exp*, struct Cyc_Absyn_Exp*, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_lt_exp( struct Cyc_Absyn_Exp*, struct Cyc_Absyn_Exp*,
struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp* Cyc_Absyn_gte_exp(
struct Cyc_Absyn_Exp*, struct Cyc_Absyn_Exp*, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_lte_exp( struct Cyc_Absyn_Exp*, struct
Cyc_Absyn_Exp*, struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp* Cyc_Absyn_assignop_exp(
struct Cyc_Absyn_Exp*, struct Cyc_Core_Opt*, struct Cyc_Absyn_Exp*, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_assign_exp( struct Cyc_Absyn_Exp*, struct
Cyc_Absyn_Exp*, struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp* Cyc_Absyn_increment_exp(
struct Cyc_Absyn_Exp*, void*, struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp*
Cyc_Absyn_post_inc_exp( struct Cyc_Absyn_Exp*, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_post_dec_exp( struct Cyc_Absyn_Exp*,
struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp* Cyc_Absyn_pre_inc_exp(
struct Cyc_Absyn_Exp*, struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp*
Cyc_Absyn_pre_dec_exp( struct Cyc_Absyn_Exp*, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_conditional_exp( struct Cyc_Absyn_Exp*,
struct Cyc_Absyn_Exp*, struct Cyc_Absyn_Exp*, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_and_exp( struct Cyc_Absyn_Exp*, struct
Cyc_Absyn_Exp*, struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp* Cyc_Absyn_or_exp(
struct Cyc_Absyn_Exp*, struct Cyc_Absyn_Exp*, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_seq_exp( struct Cyc_Absyn_Exp*, struct
Cyc_Absyn_Exp*, struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp* Cyc_Absyn_unknowncall_exp(
struct Cyc_Absyn_Exp*, struct Cyc_List_List*, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_fncall_exp( struct Cyc_Absyn_Exp*, struct
Cyc_List_List*, struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp* Cyc_Absyn_throw_exp(
struct Cyc_Absyn_Exp*, struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp*
Cyc_Absyn_noinstantiate_exp( struct Cyc_Absyn_Exp*, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_instantiate_exp( struct Cyc_Absyn_Exp*,
struct Cyc_List_List*, struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp*
Cyc_Absyn_cast_exp( void*, struct Cyc_Absyn_Exp*, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_address_exp( struct Cyc_Absyn_Exp*,
struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp* Cyc_Absyn_sizeoftyp_exp(
void* t, struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp* Cyc_Absyn_sizeofexp_exp(
struct Cyc_Absyn_Exp* e, struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp*
Cyc_Absyn_offsetof_exp( void*, struct _tagged_arr*, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_deref_exp( struct Cyc_Absyn_Exp*, struct
Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp* Cyc_Absyn_structmember_exp(
struct Cyc_Absyn_Exp*, struct _tagged_arr*, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_structarrow_exp( struct Cyc_Absyn_Exp*,
struct _tagged_arr*, struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp*
Cyc_Absyn_subscript_exp( struct Cyc_Absyn_Exp*, struct Cyc_Absyn_Exp*, struct
Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp* Cyc_Absyn_tuple_exp( struct
Cyc_List_List*, struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp* Cyc_Absyn_stmt_exp(
struct Cyc_Absyn_Stmt*, struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp*
Cyc_Absyn_null_pointer_exn_exp( struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp*
Cyc_Absyn_match_exn_exp( struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Exp*
Cyc_Absyn_array_exp( struct Cyc_List_List*, struct Cyc_Position_Segment*);
extern struct Cyc_Absyn_Exp* Cyc_Absyn_unresolvedmem_exp( struct Cyc_Core_Opt*,
struct Cyc_List_List*, struct Cyc_Position_Segment*); extern struct Cyc_Absyn_Stmt*
Cyc_Absyn_new_stmt( void* s, struct Cyc_Position_Segment* loc); extern struct
Cyc_Absyn_Stmt* Cyc_Absyn_skip_stmt( struct Cyc_Position_Segment* loc); extern
struct Cyc_Absyn_Stmt* Cyc_Absyn_exp_stmt( struct Cyc_Absyn_Exp* e, struct Cyc_Position_Segment*
loc); extern struct Cyc_Absyn_Stmt* Cyc_Absyn_seq_stmt( struct Cyc_Absyn_Stmt*
s1, struct Cyc_Absyn_Stmt* s2, struct Cyc_Position_Segment* loc); extern struct
Cyc_Absyn_Stmt* Cyc_Absyn_seq_stmts( struct Cyc_List_List*, struct Cyc_Position_Segment*
loc); extern struct Cyc_Absyn_Stmt* Cyc_Absyn_return_stmt( struct Cyc_Absyn_Exp*
e, struct Cyc_Position_Segment* loc); extern struct Cyc_Absyn_Stmt* Cyc_Absyn_ifthenelse_stmt(
struct Cyc_Absyn_Exp* e, struct Cyc_Absyn_Stmt* s1, struct Cyc_Absyn_Stmt* s2,
struct Cyc_Position_Segment* loc); extern struct Cyc_Absyn_Stmt* Cyc_Absyn_while_stmt(
struct Cyc_Absyn_Exp* e, struct Cyc_Absyn_Stmt* s, struct Cyc_Position_Segment*
loc); extern struct Cyc_Absyn_Stmt* Cyc_Absyn_break_stmt( struct Cyc_Position_Segment*
loc); extern struct Cyc_Absyn_Stmt* Cyc_Absyn_continue_stmt( struct Cyc_Position_Segment*
loc); extern struct Cyc_Absyn_Stmt* Cyc_Absyn_for_stmt( struct Cyc_Absyn_Exp* e1,
struct Cyc_Absyn_Exp* e2, struct Cyc_Absyn_Exp* e3, struct Cyc_Absyn_Stmt* s,
struct Cyc_Position_Segment* loc); extern struct Cyc_Absyn_Stmt* Cyc_Absyn_switch_stmt(
struct Cyc_Absyn_Exp* e, struct Cyc_List_List*, struct Cyc_Position_Segment* loc);
extern struct Cyc_Absyn_Stmt* Cyc_Absyn_fallthru_stmt( struct Cyc_List_List* el,
struct Cyc_Position_Segment* loc); extern struct Cyc_Absyn_Stmt* Cyc_Absyn_decl_stmt(
struct Cyc_Absyn_Decl* d, struct Cyc_Absyn_Stmt* s, struct Cyc_Position_Segment*
loc); extern struct Cyc_Absyn_Stmt* Cyc_Absyn_declare_stmt( struct _tuple0*,
void*, struct Cyc_Absyn_Exp* init, struct Cyc_Absyn_Stmt*, struct Cyc_Position_Segment*
loc); extern struct Cyc_Absyn_Stmt* Cyc_Absyn_cut_stmt( struct Cyc_Absyn_Stmt* s,
struct Cyc_Position_Segment* loc); extern struct Cyc_Absyn_Stmt* Cyc_Absyn_splice_stmt(
struct Cyc_Absyn_Stmt* s, struct Cyc_Position_Segment* loc); extern struct Cyc_Absyn_Stmt*
Cyc_Absyn_label_stmt( struct _tagged_arr* v, struct Cyc_Absyn_Stmt* s, struct
Cyc_Position_Segment* loc); extern struct Cyc_Absyn_Stmt* Cyc_Absyn_do_stmt(
struct Cyc_Absyn_Stmt* s, struct Cyc_Absyn_Exp* e, struct Cyc_Position_Segment*
loc); extern struct Cyc_Absyn_Stmt* Cyc_Absyn_trycatch_stmt( struct Cyc_Absyn_Stmt*
s, struct Cyc_List_List* scs, struct Cyc_Position_Segment* loc); extern struct
Cyc_Absyn_Stmt* Cyc_Absyn_goto_stmt( struct _tagged_arr* lab, struct Cyc_Position_Segment*
loc); extern struct Cyc_Absyn_Stmt* Cyc_Absyn_assign_stmt( struct Cyc_Absyn_Exp*
e1, struct Cyc_Absyn_Exp* e2, struct Cyc_Position_Segment* loc); extern struct
Cyc_Absyn_Pat* Cyc_Absyn_new_pat( void* p, struct Cyc_Position_Segment* s);
extern struct Cyc_Absyn_Decl* Cyc_Absyn_new_decl( void* r, struct Cyc_Position_Segment*
loc); extern struct Cyc_Absyn_Decl* Cyc_Absyn_let_decl( struct Cyc_Absyn_Pat* p,
struct Cyc_Core_Opt* t_opt, struct Cyc_Absyn_Exp* e, struct Cyc_Position_Segment*
loc); extern struct Cyc_Absyn_Decl* Cyc_Absyn_letv_decl( struct Cyc_List_List*,
struct Cyc_Position_Segment* loc); extern struct Cyc_Absyn_Vardecl* Cyc_Absyn_new_vardecl(
struct _tuple0* x, void* t, struct Cyc_Absyn_Exp* init); extern struct Cyc_Absyn_Vardecl*
Cyc_Absyn_static_vardecl( struct _tuple0* x, void* t, struct Cyc_Absyn_Exp* init);
extern struct Cyc_Absyn_Decl* Cyc_Absyn_struct_decl( void* s, struct Cyc_Core_Opt*
n, struct Cyc_List_List* ts, struct Cyc_Core_Opt* fs, struct Cyc_List_List* atts,
struct Cyc_Position_Segment* loc); extern struct Cyc_Absyn_Decl* Cyc_Absyn_union_decl(
void* s, struct Cyc_Core_Opt* n, struct Cyc_List_List* ts, struct Cyc_Core_Opt*
fs, struct Cyc_List_List* atts, struct Cyc_Position_Segment* loc); extern struct
Cyc_Absyn_Decl* Cyc_Absyn_tunion_decl( void* s, struct _tuple0* n, struct Cyc_List_List*
ts, struct Cyc_Core_Opt* fs, int is_xtunion, struct Cyc_Position_Segment* loc);
extern int Cyc_Absyn_is_format_prim( void* p); extern void* Cyc_Absyn_function_typ(
struct Cyc_List_List* tvs, struct Cyc_Core_Opt* eff_typ, void* ret_typ, struct
Cyc_List_List* args, int c_varargs, struct Cyc_Absyn_VarargInfo* cyc_varargs,
struct Cyc_List_List* rgn_po, struct Cyc_List_List*); extern void* Cyc_Absyn_pointer_expand(
void*); extern int Cyc_Absyn_is_lvalue( struct Cyc_Absyn_Exp*); extern struct
Cyc_Absyn_Structfield* Cyc_Absyn_lookup_field( struct Cyc_List_List*, struct
_tagged_arr*); extern struct Cyc_Absyn_Structfield* Cyc_Absyn_lookup_struct_field(
struct Cyc_Absyn_Structdecl*, struct _tagged_arr*); extern struct Cyc_Absyn_Structfield*
Cyc_Absyn_lookup_union_field( struct Cyc_Absyn_Uniondecl*, struct _tagged_arr*);
struct _tuple3{ struct Cyc_Absyn_Tqual f1; void* f2; } ; extern struct _tuple3*
Cyc_Absyn_lookup_tuple_field( struct Cyc_List_List*, int); extern struct
_tagged_arr Cyc_Absyn_attribute2string( void*); extern int Cyc_Absyn_fntype_att(
void* a); struct Cyc_Stdio___sFILE; extern unsigned char Cyc_Stdio_FileCloseError[
19u]; extern unsigned char Cyc_Stdio_FileOpenError[ 18u]; struct Cyc_Stdio_FileOpenError_struct{
unsigned char* tag; struct _tagged_arr f1; } ; extern int Cyc_String_zstrptrcmp(
struct _tagged_arr*, struct _tagged_arr*); struct Cyc_Set_Set; extern
unsigned char Cyc_Set_Absent[ 11u]; struct Cyc_Dict_Dict; extern unsigned char
Cyc_Dict_Present[ 12u]; extern unsigned char Cyc_Dict_Absent[ 11u]; static const
int Cyc_Tcenv_VarRes= 0; struct Cyc_Tcenv_VarRes_struct{ int tag; void* f1; } ;
static const int Cyc_Tcenv_StructRes= 1; struct Cyc_Tcenv_StructRes_struct{ int
tag; struct Cyc_Absyn_Structdecl* f1; } ; static const int Cyc_Tcenv_TunionRes=
2; struct Cyc_Tcenv_TunionRes_struct{ int tag; struct Cyc_Absyn_Tuniondecl* f1;
struct Cyc_Absyn_Tunionfield* f2; } ; static const int Cyc_Tcenv_EnumRes= 3;
struct Cyc_Tcenv_EnumRes_struct{ int tag; struct Cyc_Absyn_Enumdecl* f1; struct
Cyc_Absyn_Enumfield* f2; } ; struct Cyc_Tcenv_Genv{ struct Cyc_Set_Set*
namespaces; struct Cyc_Dict_Dict* structdecls; struct Cyc_Dict_Dict* uniondecls;
struct Cyc_Dict_Dict* tuniondecls; struct Cyc_Dict_Dict* enumdecls; struct Cyc_Dict_Dict*
typedefs; struct Cyc_Dict_Dict* ordinaries; struct Cyc_List_List* availables; }
; struct Cyc_Tcenv_Fenv; static const int Cyc_Tcenv_NotLoop_j= 0; static const
int Cyc_Tcenv_CaseEnd_j= 1; static const int Cyc_Tcenv_FnEnd_j= 2; static const
int Cyc_Tcenv_Stmt_j= 0; struct Cyc_Tcenv_Stmt_j_struct{ int tag; struct Cyc_Absyn_Stmt*
f1; } ; static const int Cyc_Tcenv_Outermost= 0; struct Cyc_Tcenv_Outermost_struct{
int tag; void* f1; } ; static const int Cyc_Tcenv_Frame= 1; struct Cyc_Tcenv_Frame_struct{
int tag; void* f1; void* f2; } ; static const int Cyc_Tcenv_Hidden= 2; struct
Cyc_Tcenv_Hidden_struct{ int tag; void* f1; void* f2; } ; struct Cyc_Tcenv_Tenv{
struct Cyc_List_List* ns; struct Cyc_Dict_Dict* ae; struct Cyc_Core_Opt* le; } ;
extern unsigned char Cyc_Tcutil_TypeErr[ 12u]; extern void* Cyc_Tcutil_impos(
struct _tagged_arr); extern void* Cyc_Tcutil_compress( void* t); static int Cyc_Absyn_zstrlist_cmp(
struct Cyc_List_List* ss1, struct Cyc_List_List* ss2){ return(( int(*)( int(*
cmp)( struct _tagged_arr*, struct _tagged_arr*), struct Cyc_List_List* l1,
struct Cyc_List_List* l2)) Cyc_List_list_cmp)( Cyc_String_zstrptrcmp, ss1, ss2);}
int Cyc_Absyn_varlist_cmp( struct Cyc_List_List* vs1, struct Cyc_List_List* vs2){
return Cyc_Absyn_zstrlist_cmp( vs1, vs2);} struct _tuple4{ void* f1; void* f2; }
; int Cyc_Absyn_qvar_cmp( struct _tuple0* q1, struct _tuple0* q2){ void* _temp0=(*
q1).f1; goto _LL1; _LL1: { void* _temp2=(* q2).f1; goto _LL3; _LL3:{ struct
_tuple4 _temp5=({ struct _tuple4 _temp4; _temp4.f1= _temp0; _temp4.f2= _temp2;
_temp4;}); void* _temp21; void* _temp23; void* _temp25; struct Cyc_List_List*
_temp27; void* _temp29; struct Cyc_List_List* _temp31; void* _temp33; struct Cyc_List_List*
_temp35; void* _temp37; struct Cyc_List_List* _temp39; void* _temp41; void*
_temp43; void* _temp45; void* _temp47; void* _temp49; void* _temp51; struct Cyc_List_List*
_temp53; void* _temp55; struct Cyc_List_List* _temp57; void* _temp59; _LL7:
_LL24: _temp23= _temp5.f1; if( _temp23 ==( void*) Cyc_Absyn_Loc_n){ goto _LL22;}
else{ goto _LL9;} _LL22: _temp21= _temp5.f2; if( _temp21 ==( void*) Cyc_Absyn_Loc_n){
goto _LL8;} else{ goto _LL9;} _LL9: _LL30: _temp29= _temp5.f1; if(( unsigned int)
_temp29 > 1u?*(( int*) _temp29) == Cyc_Absyn_Rel_n: 0){ _LL32: _temp31=(( struct
Cyc_Absyn_Rel_n_struct*) _temp29)->f1; goto _LL26;} else{ goto _LL11;} _LL26:
_temp25= _temp5.f2; if(( unsigned int) _temp25 > 1u?*(( int*) _temp25) == Cyc_Absyn_Rel_n:
0){ _LL28: _temp27=(( struct Cyc_Absyn_Rel_n_struct*) _temp25)->f1; goto _LL10;}
else{ goto _LL11;} _LL11: _LL38: _temp37= _temp5.f1; if(( unsigned int) _temp37
> 1u?*(( int*) _temp37) == Cyc_Absyn_Abs_n: 0){ _LL40: _temp39=(( struct Cyc_Absyn_Abs_n_struct*)
_temp37)->f1; goto _LL34;} else{ goto _LL13;} _LL34: _temp33= _temp5.f2; if((
unsigned int) _temp33 > 1u?*(( int*) _temp33) == Cyc_Absyn_Abs_n: 0){ _LL36:
_temp35=(( struct Cyc_Absyn_Abs_n_struct*) _temp33)->f1; goto _LL12;} else{ goto
_LL13;} _LL13: _LL44: _temp43= _temp5.f1; if( _temp43 ==( void*) Cyc_Absyn_Loc_n){
goto _LL42;} else{ goto _LL15;} _LL42: _temp41= _temp5.f2; goto _LL14; _LL15:
_LL48: _temp47= _temp5.f1; goto _LL46; _LL46: _temp45= _temp5.f2; if( _temp45 ==(
void*) Cyc_Absyn_Loc_n){ goto _LL16;} else{ goto _LL17;} _LL17: _LL52: _temp51=
_temp5.f1; if(( unsigned int) _temp51 > 1u?*(( int*) _temp51) == Cyc_Absyn_Rel_n:
0){ _LL54: _temp53=(( struct Cyc_Absyn_Rel_n_struct*) _temp51)->f1; goto _LL50;}
else{ goto _LL19;} _LL50: _temp49= _temp5.f2; goto _LL18; _LL19: _LL60: _temp59=
_temp5.f1; goto _LL56; _LL56: _temp55= _temp5.f2; if(( unsigned int) _temp55 > 1u?*((
int*) _temp55) == Cyc_Absyn_Rel_n: 0){ _LL58: _temp57=(( struct Cyc_Absyn_Rel_n_struct*)
_temp55)->f1; goto _LL20;} else{ goto _LL6;} _LL8: goto _LL6; _LL10: _temp39=
_temp31; _temp35= _temp27; goto _LL12; _LL12: { int i= Cyc_Absyn_zstrlist_cmp(
_temp39, _temp35); if( i != 0){ return i;} goto _LL6;} _LL14: return - 1; _LL16:
return 1; _LL18: return - 1; _LL20: return 1; _LL6:;} return Cyc_String_zstrptrcmp((*
q1).f2,(* q2).f2);}} int Cyc_Absyn_tvar_cmp( struct Cyc_Absyn_Tvar* tv1, struct
Cyc_Absyn_Tvar* tv2){ int i= Cyc_String_zstrptrcmp( tv1->name, tv2->name); if( i
!= 0){ return i;} if( tv1->identity == tv2->identity){ return 0;} if( tv1->identity
!= 0? tv2->identity != 0: 0){ return*(( int*) _check_null( tv1->identity)) -*((
int*) _check_null( tv2->identity));} else{ if( tv1->identity == 0){ return - 1;}
else{ return 1;}}} static int Cyc_Absyn_new_type_counter= 0; void* Cyc_Absyn_new_evar(
struct Cyc_Core_Opt* k, struct Cyc_Core_Opt* env){ return( void*)({ struct Cyc_Absyn_Evar_struct*
_temp61=( struct Cyc_Absyn_Evar_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Evar_struct));
_temp61[ 0]=({ struct Cyc_Absyn_Evar_struct _temp62; _temp62.tag= Cyc_Absyn_Evar;
_temp62.f1= k; _temp62.f2= 0; _temp62.f3= Cyc_Absyn_new_type_counter ++; _temp62.f4=
env; _temp62;}); _temp61;});} static struct Cyc_Core_Opt Cyc_Absyn_mk={( void*)((
void*) 1u)}; void* Cyc_Absyn_wildtyp( struct Cyc_Core_Opt* tenv){ return Cyc_Absyn_new_evar((
struct Cyc_Core_Opt*)& Cyc_Absyn_mk, tenv);} struct Cyc_Absyn_Tqual Cyc_Absyn_combine_tqual(
struct Cyc_Absyn_Tqual x, struct Cyc_Absyn_Tqual y){ return({ struct Cyc_Absyn_Tqual
_temp63; _temp63.q_const= x.q_const? 1: y.q_const; _temp63.q_volatile= x.q_volatile?
1: y.q_volatile; _temp63.q_restrict= x.q_restrict? 1: y.q_restrict; _temp63;});}
struct Cyc_Absyn_Tqual Cyc_Absyn_empty_tqual(){ return({ struct Cyc_Absyn_Tqual
_temp64; _temp64.q_const= 0; _temp64.q_volatile= 0; _temp64.q_restrict= 0;
_temp64;});} struct Cyc_Absyn_Tqual Cyc_Absyn_const_tqual(){ return({ struct Cyc_Absyn_Tqual
_temp65; _temp65.q_const= 1; _temp65.q_volatile= 0; _temp65.q_restrict= 0;
_temp65;});} int Cyc_Absyn_is_format_prim( void* p){ void* _temp66= p; _LL68:
if( _temp66 ==( void*) Cyc_Absyn_Printf){ goto _LL69;} else{ goto _LL70;} _LL70:
if( _temp66 ==( void*) Cyc_Absyn_Fprintf){ goto _LL71;} else{ goto _LL72;} _LL72:
if( _temp66 ==( void*) Cyc_Absyn_Xprintf){ goto _LL73;} else{ goto _LL74;} _LL74:
if( _temp66 ==( void*) Cyc_Absyn_Scanf){ goto _LL75;} else{ goto _LL76;} _LL76:
if( _temp66 ==( void*) Cyc_Absyn_Fscanf){ goto _LL77;} else{ goto _LL78;} _LL78:
if( _temp66 ==( void*) Cyc_Absyn_Sscanf){ goto _LL79;} else{ goto _LL80;} _LL80:
goto _LL81; _LL69: return 1; _LL71: return 1; _LL73: return 1; _LL75: return 1;
_LL77: return 1; _LL79: return 1; _LL81: return 0; _LL67:;} struct Cyc_Absyn_Conref*
Cyc_Absyn_new_conref( void* x){ return({ struct Cyc_Absyn_Conref* _temp82=(
struct Cyc_Absyn_Conref*) GC_malloc( sizeof( struct Cyc_Absyn_Conref)); _temp82->v=(
void*)(( void*)({ struct Cyc_Absyn_Eq_constr_struct* _temp83=( struct Cyc_Absyn_Eq_constr_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_Eq_constr_struct)); _temp83[ 0]=({ struct
Cyc_Absyn_Eq_constr_struct _temp84; _temp84.tag= Cyc_Absyn_Eq_constr; _temp84.f1=(
void*) x; _temp84;}); _temp83;})); _temp82;});} struct Cyc_Absyn_Conref* Cyc_Absyn_empty_conref(){
return({ struct Cyc_Absyn_Conref* _temp85=( struct Cyc_Absyn_Conref*) GC_malloc(
sizeof( struct Cyc_Absyn_Conref)); _temp85->v=( void*)(( void*) Cyc_Absyn_No_constr);
_temp85;});} static struct Cyc_Absyn_Eq_constr_struct Cyc_Absyn_true_constraint={
0u,( void*) 1}; static struct Cyc_Absyn_Eq_constr_struct Cyc_Absyn_false_constraint={
0u,( void*) 0}; struct Cyc_Absyn_Conref Cyc_Absyn_true_conref_v={( void*)(( void*)&
Cyc_Absyn_true_constraint)}; struct Cyc_Absyn_Conref Cyc_Absyn_false_conref_v={(
void*)(( void*)& Cyc_Absyn_false_constraint)}; struct Cyc_Absyn_Conref* Cyc_Absyn_true_conref=&
Cyc_Absyn_true_conref_v; struct Cyc_Absyn_Conref* Cyc_Absyn_false_conref=& Cyc_Absyn_false_conref_v;
struct Cyc_Absyn_Conref* Cyc_Absyn_compress_conref( struct Cyc_Absyn_Conref* x){
void* _temp88=( void*) x->v; void* _temp96; struct Cyc_Absyn_Conref* _temp98;
struct Cyc_Absyn_Conref** _temp100; _LL90: if( _temp88 ==( void*) Cyc_Absyn_No_constr){
goto _LL91;} else{ goto _LL92;} _LL92: if(( unsigned int) _temp88 > 1u?*(( int*)
_temp88) == Cyc_Absyn_Eq_constr: 0){ _LL97: _temp96=( void*)(( struct Cyc_Absyn_Eq_constr_struct*)
_temp88)->f1; goto _LL93;} else{ goto _LL94;} _LL94: if(( unsigned int) _temp88
> 1u?*(( int*) _temp88) == Cyc_Absyn_Forward_constr: 0){ _LL99: _temp98=((
struct Cyc_Absyn_Forward_constr_struct*) _temp88)->f1; _temp100=&(( struct Cyc_Absyn_Forward_constr_struct*)
_temp88)->f1; goto _LL95;} else{ goto _LL89;} _LL91: return x; _LL93: return x;
_LL95: { struct Cyc_Absyn_Conref* _temp101=(( struct Cyc_Absyn_Conref*(*)(
struct Cyc_Absyn_Conref* x)) Cyc_Absyn_compress_conref)(* _temp100); goto _LL102;
_LL102:* _temp100= _temp101; return _temp101;} _LL89:;} void* Cyc_Absyn_conref_val(
struct Cyc_Absyn_Conref* x){ void* _temp103=( void*)((( struct Cyc_Absyn_Conref*(*)(
struct Cyc_Absyn_Conref* x)) Cyc_Absyn_compress_conref)( x))->v; void* _temp109;
_LL105: if(( unsigned int) _temp103 > 1u?*(( int*) _temp103) == Cyc_Absyn_Eq_constr:
0){ _LL110: _temp109=( void*)(( struct Cyc_Absyn_Eq_constr_struct*) _temp103)->f1;
goto _LL106;} else{ goto _LL107;} _LL107: goto _LL108; _LL106: return _temp109;
_LL108: return(( void*(*)( struct _tagged_arr)) Cyc_Tcutil_impos)( _tag_arr("conref_val",
sizeof( unsigned char), 11u)); _LL104:;} void* Cyc_Absyn_conref_def( void* y,
struct Cyc_Absyn_Conref* x){ void* _temp111=( void*)((( struct Cyc_Absyn_Conref*(*)(
struct Cyc_Absyn_Conref* x)) Cyc_Absyn_compress_conref)( x))->v; void* _temp117;
_LL113: if(( unsigned int) _temp111 > 1u?*(( int*) _temp111) == Cyc_Absyn_Eq_constr:
0){ _LL118: _temp117=( void*)(( struct Cyc_Absyn_Eq_constr_struct*) _temp111)->f1;
goto _LL114;} else{ goto _LL115;} _LL115: goto _LL116; _LL114: return _temp117;
_LL116: return y; _LL112:;} static struct Cyc_Absyn_IntType_struct Cyc_Absyn_uchar_tt={
5u,( void*)(( void*) 1u),( void*)(( void*) 0u)}; static struct Cyc_Absyn_IntType_struct
Cyc_Absyn_ushort_tt={ 5u,( void*)(( void*) 1u),( void*)(( void*) 1u)}; static
struct Cyc_Absyn_IntType_struct Cyc_Absyn_uint_tt={ 5u,( void*)(( void*) 1u),(
void*)(( void*) 2u)}; static struct Cyc_Absyn_IntType_struct Cyc_Absyn_ulong_tt={
5u,( void*)(( void*) 1u),( void*)(( void*) 2u)}; static struct Cyc_Absyn_IntType_struct
Cyc_Absyn_ulonglong_tt={ 5u,( void*)(( void*) 1u),( void*)(( void*) 3u)}; void*
Cyc_Absyn_uchar_t=( void*)& Cyc_Absyn_uchar_tt; void* Cyc_Absyn_ushort_t=( void*)&
Cyc_Absyn_ushort_tt; void* Cyc_Absyn_uint_t=( void*)& Cyc_Absyn_uint_tt; void*
Cyc_Absyn_ulong_t=( void*)& Cyc_Absyn_ulong_tt; void* Cyc_Absyn_ulonglong_t=(
void*)& Cyc_Absyn_ulonglong_tt; static struct Cyc_Absyn_IntType_struct Cyc_Absyn_schar_tt={
5u,( void*)(( void*) 0u),( void*)(( void*) 0u)}; static struct Cyc_Absyn_IntType_struct
Cyc_Absyn_sshort_tt={ 5u,( void*)(( void*) 0u),( void*)(( void*) 1u)}; static
struct Cyc_Absyn_IntType_struct Cyc_Absyn_sint_tt={ 5u,( void*)(( void*) 0u),(
void*)(( void*) 2u)}; static struct Cyc_Absyn_IntType_struct Cyc_Absyn_slong_tt={
5u,( void*)(( void*) 0u),( void*)(( void*) 2u)}; static struct Cyc_Absyn_IntType_struct
Cyc_Absyn_slonglong_tt={ 5u,( void*)(( void*) 0u),( void*)(( void*) 3u)}; void*
Cyc_Absyn_schar_t=( void*)& Cyc_Absyn_schar_tt; void* Cyc_Absyn_sshort_t=( void*)&
Cyc_Absyn_sshort_tt; void* Cyc_Absyn_sint_t=( void*)& Cyc_Absyn_sint_tt; void*
Cyc_Absyn_slong_t=( void*)& Cyc_Absyn_slong_tt; void* Cyc_Absyn_slonglong_t=(
void*)& Cyc_Absyn_slonglong_tt; void* Cyc_Absyn_float_t=( void*) 1u; void* Cyc_Absyn_double_t=(
void*) 2u; static struct Cyc_Absyn_Abs_n_struct Cyc_Absyn_abs_null={ 1u, 0};
static unsigned char _temp130[ 4u]="exn"; static struct _tagged_arr Cyc_Absyn_exn_str={
_temp130, _temp130, _temp130 + 4u}; static struct _tuple0 Cyc_Absyn_exn_name_v={(
void*)& Cyc_Absyn_abs_null,& Cyc_Absyn_exn_str}; struct _tuple0* Cyc_Absyn_exn_name=&
Cyc_Absyn_exn_name_v; static unsigned char _temp131[ 15u]="Null_Exception";
static struct _tagged_arr Cyc_Absyn_null_pointer_exn_str={ _temp131, _temp131,
_temp131 + 15u}; static struct _tuple0 Cyc_Absyn_null_pointer_exn_name_v={( void*)&
Cyc_Absyn_abs_null,& Cyc_Absyn_null_pointer_exn_str}; static unsigned char
_temp132[ 16u]="Match_Exception"; static struct _tagged_arr Cyc_Absyn_match_exn_str={
_temp132, _temp132, _temp132 + 16u}; static struct _tuple0 Cyc_Absyn_match_exn_name_v={(
void*)& Cyc_Absyn_abs_null,& Cyc_Absyn_match_exn_str}; struct _tuple0* Cyc_Absyn_null_pointer_exn_name=&
Cyc_Absyn_null_pointer_exn_name_v; struct _tuple0* Cyc_Absyn_match_exn_name=&
Cyc_Absyn_match_exn_name_v; static struct Cyc_Absyn_Tunionfield Cyc_Absyn_null_pointer_exn_tuf_v={&
Cyc_Absyn_null_pointer_exn_name_v, 0, 0, 0,( void*)(( void*) 3u)}; static struct
Cyc_Absyn_Tunionfield Cyc_Absyn_match_exn_tuf_v={& Cyc_Absyn_match_exn_name_v, 0,
0, 0,( void*)(( void*) 3u)}; struct Cyc_Absyn_Tunionfield* Cyc_Absyn_null_pointer_exn_tuf=&
Cyc_Absyn_null_pointer_exn_tuf_v; struct Cyc_Absyn_Tunionfield* Cyc_Absyn_match_exn_tuf=&
Cyc_Absyn_match_exn_tuf_v; static struct Cyc_List_List Cyc_Absyn_exn_l0={( void*)&
Cyc_Absyn_null_pointer_exn_tuf_v, 0}; static struct Cyc_List_List Cyc_Absyn_exn_l1={(
void*)& Cyc_Absyn_match_exn_tuf_v,( struct Cyc_List_List*)& Cyc_Absyn_exn_l0};
static struct Cyc_Core_Opt Cyc_Absyn_exn_ol={( void*)(( struct Cyc_List_List*)&
Cyc_Absyn_exn_l1)}; static struct Cyc_Absyn_Tuniondecl Cyc_Absyn_exn_tud_v={(
void*)(( void*) 3u),& Cyc_Absyn_exn_name_v, 0,( struct Cyc_Core_Opt*)& Cyc_Absyn_exn_ol,
1}; struct Cyc_Absyn_Tuniondecl* Cyc_Absyn_exn_tud=& Cyc_Absyn_exn_tud_v; static
struct Cyc_Absyn_KnownTunion_struct Cyc_Absyn_exn_tinfou={ 1u,& Cyc_Absyn_exn_tud_v};
static struct Cyc_Absyn_TunionType_struct Cyc_Absyn_exn_typ_tt={ 2u,{( void*)((
void*)& Cyc_Absyn_exn_tinfou), 0,( void*)(( void*) 3u)}}; void* Cyc_Absyn_exn_typ=(
void*)& Cyc_Absyn_exn_typ_tt; static struct Cyc_Core_Opt* Cyc_Absyn_string_t_opt=
0; void* Cyc_Absyn_string_typ( void* rgn){ if( rgn !=( void*) Cyc_Absyn_HeapRgn){
return Cyc_Absyn_starb_typ( Cyc_Absyn_uchar_t, rgn, Cyc_Absyn_empty_tqual(),(
void*) Cyc_Absyn_Unknown_b);} else{ if( Cyc_Absyn_string_t_opt == 0){ void* t=
Cyc_Absyn_starb_typ( Cyc_Absyn_uchar_t,( void*) Cyc_Absyn_HeapRgn, Cyc_Absyn_empty_tqual(),(
void*) Cyc_Absyn_Unknown_b); Cyc_Absyn_string_t_opt=({ struct Cyc_Core_Opt*
_temp135=( struct Cyc_Core_Opt*) GC_malloc( sizeof( struct Cyc_Core_Opt));
_temp135->v=( void*) t; _temp135;});} return( void*)(( struct Cyc_Core_Opt*)
_check_null( Cyc_Absyn_string_t_opt))->v;}} static struct Cyc_Core_Opt* Cyc_Absyn_const_string_t_opt=
0; void* Cyc_Absyn_const_string_typ( void* rgn){ if( rgn !=( void*) Cyc_Absyn_HeapRgn){
return Cyc_Absyn_starb_typ( Cyc_Absyn_uchar_t, rgn, Cyc_Absyn_const_tqual(),(
void*) Cyc_Absyn_Unknown_b);} else{ if( Cyc_Absyn_const_string_t_opt == 0){ void*
t= Cyc_Absyn_starb_typ( Cyc_Absyn_uchar_t,( void*) Cyc_Absyn_HeapRgn, Cyc_Absyn_const_tqual(),(
void*) Cyc_Absyn_Unknown_b); Cyc_Absyn_const_string_t_opt=({ struct Cyc_Core_Opt*
_temp136=( struct Cyc_Core_Opt*) GC_malloc( sizeof( struct Cyc_Core_Opt));
_temp136->v=( void*) t; _temp136;});} return( void*)(( struct Cyc_Core_Opt*)
_check_null( Cyc_Absyn_const_string_t_opt))->v;}} static struct Cyc_Absyn_Int_c_struct
Cyc_Absyn_one_intc={ 2u,( void*)(( void*) 1u), 1}; static struct Cyc_Absyn_Const_e_struct
Cyc_Absyn_one_b_raw={ 0u,( void*)(( void*)& Cyc_Absyn_one_intc)}; struct Cyc_Absyn_Exp
Cyc_Absyn_exp_unsigned_one_v={ 0,( void*)(( void*)& Cyc_Absyn_one_b_raw), 0};
struct Cyc_Absyn_Exp* Cyc_Absyn_exp_unsigned_one=& Cyc_Absyn_exp_unsigned_one_v;
static struct Cyc_Absyn_Upper_b_struct Cyc_Absyn_one_bt={ 0u,& Cyc_Absyn_exp_unsigned_one_v};
void* Cyc_Absyn_bounds_one=( void*)& Cyc_Absyn_one_bt; void* Cyc_Absyn_starb_typ(
void* t, void* r, struct Cyc_Absyn_Tqual tq, void* b){ return( void*)({ struct
Cyc_Absyn_PointerType_struct* _temp140=( struct Cyc_Absyn_PointerType_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_PointerType_struct)); _temp140[ 0]=({ struct
Cyc_Absyn_PointerType_struct _temp141; _temp141.tag= Cyc_Absyn_PointerType;
_temp141.f1=({ struct Cyc_Absyn_PtrInfo _temp142; _temp142.elt_typ=( void*) t;
_temp142.rgn_typ=( void*) r; _temp142.nullable= Cyc_Absyn_true_conref; _temp142.tq=
tq; _temp142.bounds=(( struct Cyc_Absyn_Conref*(*)( void* x)) Cyc_Absyn_new_conref)(
b); _temp142;}); _temp141;}); _temp140;});} void* Cyc_Absyn_atb_typ( void* t,
void* r, struct Cyc_Absyn_Tqual tq, void* b){ return( void*)({ struct Cyc_Absyn_PointerType_struct*
_temp143=( struct Cyc_Absyn_PointerType_struct*) GC_malloc( sizeof( struct Cyc_Absyn_PointerType_struct));
_temp143[ 0]=({ struct Cyc_Absyn_PointerType_struct _temp144; _temp144.tag= Cyc_Absyn_PointerType;
_temp144.f1=({ struct Cyc_Absyn_PtrInfo _temp145; _temp145.elt_typ=( void*) t;
_temp145.rgn_typ=( void*) r; _temp145.nullable= Cyc_Absyn_false_conref; _temp145.tq=
tq; _temp145.bounds=(( struct Cyc_Absyn_Conref*(*)( void* x)) Cyc_Absyn_new_conref)(
b); _temp145;}); _temp144;}); _temp143;});} void* Cyc_Absyn_star_typ( void* t,
void* r, struct Cyc_Absyn_Tqual tq){ return Cyc_Absyn_starb_typ( t, r, tq, Cyc_Absyn_bounds_one);}
void* Cyc_Absyn_cstar_typ( void* t, struct Cyc_Absyn_Tqual tq){ return Cyc_Absyn_starb_typ(
t,( void*) Cyc_Absyn_HeapRgn, tq, Cyc_Absyn_bounds_one);} void* Cyc_Absyn_at_typ(
void* t, void* r, struct Cyc_Absyn_Tqual tq){ return Cyc_Absyn_atb_typ( t, r, tq,
Cyc_Absyn_bounds_one);} void* Cyc_Absyn_tagged_typ( void* t, void* r, struct Cyc_Absyn_Tqual
tq){ return( void*)({ struct Cyc_Absyn_PointerType_struct* _temp146=( struct Cyc_Absyn_PointerType_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_PointerType_struct)); _temp146[ 0]=({ struct
Cyc_Absyn_PointerType_struct _temp147; _temp147.tag= Cyc_Absyn_PointerType;
_temp147.f1=({ struct Cyc_Absyn_PtrInfo _temp148; _temp148.elt_typ=( void*) t;
_temp148.rgn_typ=( void*) r; _temp148.nullable= Cyc_Absyn_true_conref; _temp148.tq=
tq; _temp148.bounds=(( struct Cyc_Absyn_Conref*(*)( void* x)) Cyc_Absyn_new_conref)((
void*) Cyc_Absyn_Unknown_b); _temp148;}); _temp147;}); _temp146;});} static
struct Cyc_Core_Opt* Cyc_Absyn_file_t_opt= 0; static unsigned char _temp149[ 8u]="__sFILE";
static struct _tagged_arr Cyc_Absyn_sf_str={ _temp149, _temp149, _temp149 + 8u};
static struct _tagged_arr* Cyc_Absyn_sf=& Cyc_Absyn_sf_str; static unsigned char
_temp150[ 6u]="Stdio"; static struct _tagged_arr Cyc_Absyn_st_str={ _temp150,
_temp150, _temp150 + 6u}; static struct _tagged_arr* Cyc_Absyn_st=& Cyc_Absyn_st_str;
void* Cyc_Absyn_file_typ(){ if( Cyc_Absyn_file_t_opt == 0){ struct _tuple0*
file_t_name=({ struct _tuple0* _temp157=( struct _tuple0*) GC_malloc( sizeof(
struct _tuple0)); _temp157->f1=( void*)({ struct Cyc_Absyn_Abs_n_struct*
_temp158=( struct Cyc_Absyn_Abs_n_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Abs_n_struct));
_temp158[ 0]=({ struct Cyc_Absyn_Abs_n_struct _temp159; _temp159.tag= Cyc_Absyn_Abs_n;
_temp159.f1=({ struct Cyc_List_List*(* _temp160)( struct _tagged_arr)=( struct
Cyc_List_List*(*)( struct _tagged_arr)) Cyc_List_list; struct _tagged_arr*
_temp163= Cyc_Absyn_st; struct _tagged_arr* _temp161[ 1u]={ _temp163}; struct
_tagged_arr _temp162={( void*) _temp161,( void*) _temp161,( void*)( _temp161 + 1u)};
_temp160( _temp162);}); _temp159;}); _temp158;}); _temp157->f2= Cyc_Absyn_sf;
_temp157;}); struct Cyc_Absyn_Structdecl* sd=({ struct Cyc_Absyn_Structdecl*
_temp155=( struct Cyc_Absyn_Structdecl*) GC_malloc( sizeof( struct Cyc_Absyn_Structdecl));
_temp155->sc=( void*)(( void*) Cyc_Absyn_Abstract); _temp155->name=({ struct Cyc_Core_Opt*
_temp156=( struct Cyc_Core_Opt*) GC_malloc( sizeof( struct Cyc_Core_Opt));
_temp156->v=( void*) file_t_name; _temp156;}); _temp155->tvs= 0; _temp155->fields=
0; _temp155->attributes= 0; _temp155;}); void* file_struct_typ=( void*)({ struct
Cyc_Absyn_StructType_struct* _temp152=( struct Cyc_Absyn_StructType_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_StructType_struct)); _temp152[ 0]=({ struct
Cyc_Absyn_StructType_struct _temp153; _temp153.tag= Cyc_Absyn_StructType;
_temp153.f1=( struct _tuple0*) file_t_name; _temp153.f2= 0; _temp153.f3=({
struct Cyc_Absyn_Structdecl** _temp154=( struct Cyc_Absyn_Structdecl**)
GC_malloc( sizeof( struct Cyc_Absyn_Structdecl*) * 1); _temp154[ 0]= sd;
_temp154;}); _temp153;}); _temp152;}); Cyc_Absyn_file_t_opt=({ struct Cyc_Core_Opt*
_temp151=( struct Cyc_Core_Opt*) GC_malloc( sizeof( struct Cyc_Core_Opt));
_temp151->v=( void*) Cyc_Absyn_at_typ( file_struct_typ,( void*) Cyc_Absyn_HeapRgn,
Cyc_Absyn_empty_tqual()); _temp151;});} return( void*)(( struct Cyc_Core_Opt*)
_check_null( Cyc_Absyn_file_t_opt))->v;} static struct Cyc_Core_Opt* Cyc_Absyn_void_star_t_opt=
0; void* Cyc_Absyn_void_star_typ(){ if( Cyc_Absyn_void_star_t_opt == 0){ Cyc_Absyn_void_star_t_opt=({
struct Cyc_Core_Opt* _temp164=( struct Cyc_Core_Opt*) GC_malloc( sizeof( struct
Cyc_Core_Opt)); _temp164->v=( void*) Cyc_Absyn_star_typ(( void*) Cyc_Absyn_VoidType,(
void*) Cyc_Absyn_HeapRgn, Cyc_Absyn_empty_tqual()); _temp164;});} return( void*)((
struct Cyc_Core_Opt*) _check_null( Cyc_Absyn_void_star_t_opt))->v;} struct Cyc_Core_Opt*
Cyc_Absyn_void_star_typ_opt(){ if( Cyc_Absyn_void_star_t_opt == 0){ Cyc_Absyn_void_star_t_opt=({
struct Cyc_Core_Opt* _temp165=( struct Cyc_Core_Opt*) GC_malloc( sizeof( struct
Cyc_Core_Opt)); _temp165->v=( void*) Cyc_Absyn_star_typ(( void*) Cyc_Absyn_VoidType,(
void*) Cyc_Absyn_HeapRgn, Cyc_Absyn_empty_tqual()); _temp165;});} return Cyc_Absyn_void_star_t_opt;}
void* Cyc_Absyn_strct( struct _tagged_arr* name){ return( void*)({ struct Cyc_Absyn_StructType_struct*
_temp166=( struct Cyc_Absyn_StructType_struct*) GC_malloc( sizeof( struct Cyc_Absyn_StructType_struct));
_temp166[ 0]=({ struct Cyc_Absyn_StructType_struct _temp167; _temp167.tag= Cyc_Absyn_StructType;
_temp167.f1=({ struct _tuple0* _temp168=( struct _tuple0*) GC_malloc( sizeof(
struct _tuple0) * 1); _temp168[ 0]=({ struct _tuple0 _temp169; _temp169.f1=(
void*)({ struct Cyc_Absyn_Rel_n_struct* _temp170=( struct Cyc_Absyn_Rel_n_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_Rel_n_struct)); _temp170[ 0]=({ struct Cyc_Absyn_Rel_n_struct
_temp171; _temp171.tag= Cyc_Absyn_Rel_n; _temp171.f1= 0; _temp171;}); _temp170;});
_temp169.f2= name; _temp169;}); _temp168;}); _temp167.f2= 0; _temp167.f3= 0;
_temp167;}); _temp166;});} void* Cyc_Absyn_union_typ( struct _tagged_arr* name){
return( void*)({ struct Cyc_Absyn_UnionType_struct* _temp172=( struct Cyc_Absyn_UnionType_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_UnionType_struct)); _temp172[ 0]=({ struct
Cyc_Absyn_UnionType_struct _temp173; _temp173.tag= Cyc_Absyn_UnionType; _temp173.f1=({
struct _tuple0* _temp174=( struct _tuple0*) GC_malloc( sizeof( struct _tuple0) *
1); _temp174[ 0]=({ struct _tuple0 _temp175; _temp175.f1=( void*)({ struct Cyc_Absyn_Rel_n_struct*
_temp176=( struct Cyc_Absyn_Rel_n_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Rel_n_struct));
_temp176[ 0]=({ struct Cyc_Absyn_Rel_n_struct _temp177; _temp177.tag= Cyc_Absyn_Rel_n;
_temp177.f1= 0; _temp177;}); _temp176;}); _temp175.f2= name; _temp175;});
_temp174;}); _temp173.f2= 0; _temp173.f3= 0; _temp173;}); _temp172;});} void*
Cyc_Absyn_strctq( struct _tuple0* name){ return( void*)({ struct Cyc_Absyn_StructType_struct*
_temp178=( struct Cyc_Absyn_StructType_struct*) GC_malloc( sizeof( struct Cyc_Absyn_StructType_struct));
_temp178[ 0]=({ struct Cyc_Absyn_StructType_struct _temp179; _temp179.tag= Cyc_Absyn_StructType;
_temp179.f1=( struct _tuple0*) name; _temp179.f2= 0; _temp179.f3= 0; _temp179;});
_temp178;});} void* Cyc_Absyn_unionq_typ( struct _tuple0* name){ return( void*)({
struct Cyc_Absyn_UnionType_struct* _temp180=( struct Cyc_Absyn_UnionType_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_UnionType_struct)); _temp180[ 0]=({ struct
Cyc_Absyn_UnionType_struct _temp181; _temp181.tag= Cyc_Absyn_UnionType; _temp181.f1=(
struct _tuple0*) name; _temp181.f2= 0; _temp181.f3= 0; _temp181;}); _temp180;});}
struct Cyc_Absyn_Exp* Cyc_Absyn_new_exp( void* r, struct Cyc_Position_Segment*
loc){ return({ struct Cyc_Absyn_Exp* _temp182=( struct Cyc_Absyn_Exp*) GC_malloc(
sizeof( struct Cyc_Absyn_Exp)); _temp182->topt= 0; _temp182->r=( void*) r;
_temp182->loc= loc; _temp182;});} struct Cyc_Absyn_Exp* Cyc_Absyn_New_exp(
struct Cyc_Absyn_Exp* rgn_handle, struct Cyc_Absyn_Exp* e, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_new_exp(( void*)({ struct Cyc_Absyn_New_e_struct*
_temp183=( struct Cyc_Absyn_New_e_struct*) GC_malloc( sizeof( struct Cyc_Absyn_New_e_struct));
_temp183[ 0]=({ struct Cyc_Absyn_New_e_struct _temp184; _temp184.tag= Cyc_Absyn_New_e;
_temp184.f1= rgn_handle; _temp184.f2= e; _temp184;}); _temp183;}), loc);} struct
Cyc_Absyn_Exp* Cyc_Absyn_copy_exp( struct Cyc_Absyn_Exp* e){ return({ struct Cyc_Absyn_Exp*
_temp185=( struct Cyc_Absyn_Exp*) GC_malloc( sizeof( struct Cyc_Absyn_Exp));
_temp185->topt= e->topt; _temp185->r=( void*)(( void*) e->r); _temp185->loc= e->loc;
_temp185;});} struct Cyc_Absyn_Exp* Cyc_Absyn_const_exp( void* c, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_new_exp(( void*)({ struct Cyc_Absyn_Const_e_struct*
_temp186=( struct Cyc_Absyn_Const_e_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Const_e_struct));
_temp186[ 0]=({ struct Cyc_Absyn_Const_e_struct _temp187; _temp187.tag= Cyc_Absyn_Const_e;
_temp187.f1=( void*) c; _temp187;}); _temp186;}), loc);} struct Cyc_Absyn_Exp*
Cyc_Absyn_null_exp( struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_exp((
void*)({ struct Cyc_Absyn_Const_e_struct* _temp188=( struct Cyc_Absyn_Const_e_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_Const_e_struct)); _temp188[ 0]=({ struct Cyc_Absyn_Const_e_struct
_temp189; _temp189.tag= Cyc_Absyn_Const_e; _temp189.f1=( void*)(( void*) Cyc_Absyn_Null_c);
_temp189;}); _temp188;}), loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_int_exp( void*
s, int i, struct Cyc_Position_Segment* seg){ return Cyc_Absyn_const_exp(( void*)({
struct Cyc_Absyn_Int_c_struct* _temp190=( struct Cyc_Absyn_Int_c_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_Int_c_struct)); _temp190[ 0]=({ struct Cyc_Absyn_Int_c_struct
_temp191; _temp191.tag= Cyc_Absyn_Int_c; _temp191.f1=( void*) s; _temp191.f2= i;
_temp191;}); _temp190;}), seg);} struct Cyc_Absyn_Exp* Cyc_Absyn_signed_int_exp(
int i, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_int_exp(( void*) Cyc_Absyn_Signed,
i, loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_uint_exp( unsigned int i, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_int_exp(( void*) Cyc_Absyn_Unsigned,( int) i, loc);}
struct Cyc_Absyn_Exp* Cyc_Absyn_bool_exp( int b, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_signed_int_exp( b? 1: 0, loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_true_exp(
struct Cyc_Position_Segment* loc){ return Cyc_Absyn_bool_exp( 1, loc);} struct
Cyc_Absyn_Exp* Cyc_Absyn_false_exp( struct Cyc_Position_Segment* loc){ return
Cyc_Absyn_bool_exp( 0, loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_char_exp(
unsigned char c, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_const_exp((
void*)({ struct Cyc_Absyn_Char_c_struct* _temp192=( struct Cyc_Absyn_Char_c_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_Char_c_struct)); _temp192[ 0]=({ struct Cyc_Absyn_Char_c_struct
_temp193; _temp193.tag= Cyc_Absyn_Char_c; _temp193.f1=( void*)(( void*) Cyc_Absyn_Unsigned);
_temp193.f2= c; _temp193;}); _temp192;}), loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_float_exp(
struct _tagged_arr f, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_const_exp((
void*)({ struct Cyc_Absyn_Float_c_struct* _temp194=( struct Cyc_Absyn_Float_c_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_Float_c_struct)); _temp194[ 0]=({ struct Cyc_Absyn_Float_c_struct
_temp195; _temp195.tag= Cyc_Absyn_Float_c; _temp195.f1= f; _temp195;}); _temp194;}),
loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_string_exp( struct _tagged_arr s, struct
Cyc_Position_Segment* loc){ return Cyc_Absyn_const_exp(( void*)({ struct Cyc_Absyn_String_c_struct*
_temp196=( struct Cyc_Absyn_String_c_struct*) GC_malloc( sizeof( struct Cyc_Absyn_String_c_struct));
_temp196[ 0]=({ struct Cyc_Absyn_String_c_struct _temp197; _temp197.tag= Cyc_Absyn_String_c;
_temp197.f1= s; _temp197;}); _temp196;}), loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_var_exp(
struct _tuple0* q, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_exp((
void*)({ struct Cyc_Absyn_Var_e_struct* _temp198=( struct Cyc_Absyn_Var_e_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_Var_e_struct)); _temp198[ 0]=({ struct Cyc_Absyn_Var_e_struct
_temp199; _temp199.tag= Cyc_Absyn_Var_e; _temp199.f1= q; _temp199.f2=( void*)((
void*) Cyc_Absyn_Unresolved_b); _temp199;}); _temp198;}), loc);} struct Cyc_Absyn_Exp*
Cyc_Absyn_varb_exp( struct _tuple0* q, void* b, struct Cyc_Position_Segment* loc){
return Cyc_Absyn_new_exp(( void*)({ struct Cyc_Absyn_Var_e_struct* _temp200=(
struct Cyc_Absyn_Var_e_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Var_e_struct));
_temp200[ 0]=({ struct Cyc_Absyn_Var_e_struct _temp201; _temp201.tag= Cyc_Absyn_Var_e;
_temp201.f1= q; _temp201.f2=( void*) b; _temp201;}); _temp200;}), loc);} struct
Cyc_Absyn_Exp* Cyc_Absyn_unknownid_exp( struct _tuple0* q, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_new_exp(( void*)({ struct Cyc_Absyn_UnknownId_e_struct*
_temp202=( struct Cyc_Absyn_UnknownId_e_struct*) GC_malloc( sizeof( struct Cyc_Absyn_UnknownId_e_struct));
_temp202[ 0]=({ struct Cyc_Absyn_UnknownId_e_struct _temp203; _temp203.tag= Cyc_Absyn_UnknownId_e;
_temp203.f1= q; _temp203;}); _temp202;}), loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_primop_exp(
void* p, struct Cyc_List_List* es, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_exp((
void*)({ struct Cyc_Absyn_Primop_e_struct* _temp204=( struct Cyc_Absyn_Primop_e_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_Primop_e_struct)); _temp204[ 0]=({ struct
Cyc_Absyn_Primop_e_struct _temp205; _temp205.tag= Cyc_Absyn_Primop_e; _temp205.f1=(
void*) p; _temp205.f2= es; _temp205;}); _temp204;}), loc);} struct Cyc_Absyn_Exp*
Cyc_Absyn_prim1_exp( void* p, struct Cyc_Absyn_Exp* e, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_primop_exp( p,({ struct Cyc_List_List* _temp206=( struct
Cyc_List_List*) GC_malloc( sizeof( struct Cyc_List_List)); _temp206->hd=( void*)
e; _temp206->tl= 0; _temp206;}), loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_prim2_exp(
void* p, struct Cyc_Absyn_Exp* e1, struct Cyc_Absyn_Exp* e2, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_primop_exp( p,({ struct Cyc_List_List* _temp207=( struct
Cyc_List_List*) GC_malloc( sizeof( struct Cyc_List_List)); _temp207->hd=( void*)
e1; _temp207->tl=({ struct Cyc_List_List* _temp208=( struct Cyc_List_List*)
GC_malloc( sizeof( struct Cyc_List_List)); _temp208->hd=( void*) e2; _temp208->tl=
0; _temp208;}); _temp207;}), loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_add_exp(
struct Cyc_Absyn_Exp* e1, struct Cyc_Absyn_Exp* e2, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_prim2_exp(( void*) Cyc_Absyn_Plus, e1, e2, loc);} struct
Cyc_Absyn_Exp* Cyc_Absyn_subtract_exp( struct Cyc_Absyn_Exp* e1, struct Cyc_Absyn_Exp*
e2, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_prim2_exp(( void*) Cyc_Absyn_Minus,
e1, e2, loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_times_exp( struct Cyc_Absyn_Exp*
e1, struct Cyc_Absyn_Exp* e2, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_prim2_exp((
void*) Cyc_Absyn_Times, e1, e2, loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_divide_exp(
struct Cyc_Absyn_Exp* e1, struct Cyc_Absyn_Exp* e2, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_prim2_exp(( void*) Cyc_Absyn_Div, e1, e2, loc);} struct
Cyc_Absyn_Exp* Cyc_Absyn_eq_exp( struct Cyc_Absyn_Exp* e1, struct Cyc_Absyn_Exp*
e2, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_prim2_exp(( void*) Cyc_Absyn_Eq,
e1, e2, loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_neq_exp( struct Cyc_Absyn_Exp* e1,
struct Cyc_Absyn_Exp* e2, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_prim2_exp((
void*) Cyc_Absyn_Neq, e1, e2, loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_gt_exp(
struct Cyc_Absyn_Exp* e1, struct Cyc_Absyn_Exp* e2, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_prim2_exp(( void*) Cyc_Absyn_Gt, e1, e2, loc);} struct
Cyc_Absyn_Exp* Cyc_Absyn_lt_exp( struct Cyc_Absyn_Exp* e1, struct Cyc_Absyn_Exp*
e2, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_prim2_exp(( void*) Cyc_Absyn_Lt,
e1, e2, loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_gte_exp( struct Cyc_Absyn_Exp* e1,
struct Cyc_Absyn_Exp* e2, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_prim2_exp((
void*) Cyc_Absyn_Gte, e1, e2, loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_lte_exp(
struct Cyc_Absyn_Exp* e1, struct Cyc_Absyn_Exp* e2, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_prim2_exp(( void*) Cyc_Absyn_Lte, e1, e2, loc);} struct
Cyc_Absyn_Exp* Cyc_Absyn_assignop_exp( struct Cyc_Absyn_Exp* e1, struct Cyc_Core_Opt*
popt, struct Cyc_Absyn_Exp* e2, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_exp((
void*)({ struct Cyc_Absyn_AssignOp_e_struct* _temp209=( struct Cyc_Absyn_AssignOp_e_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_AssignOp_e_struct)); _temp209[ 0]=({ struct
Cyc_Absyn_AssignOp_e_struct _temp210; _temp210.tag= Cyc_Absyn_AssignOp_e;
_temp210.f1= e1; _temp210.f2= popt; _temp210.f3= e2; _temp210;}); _temp209;}),
loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_assign_exp( struct Cyc_Absyn_Exp* e1,
struct Cyc_Absyn_Exp* e2, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_assignop_exp(
e1, 0, e2, loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_increment_exp( struct Cyc_Absyn_Exp*
e, void* i, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_exp(( void*)({
struct Cyc_Absyn_Increment_e_struct* _temp211=( struct Cyc_Absyn_Increment_e_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_Increment_e_struct)); _temp211[ 0]=({ struct
Cyc_Absyn_Increment_e_struct _temp212; _temp212.tag= Cyc_Absyn_Increment_e;
_temp212.f1= e; _temp212.f2=( void*) i; _temp212;}); _temp211;}), loc);} struct
Cyc_Absyn_Exp* Cyc_Absyn_post_inc_exp( struct Cyc_Absyn_Exp* e, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_increment_exp( e,( void*) Cyc_Absyn_PostInc, loc);}
struct Cyc_Absyn_Exp* Cyc_Absyn_pre_inc_exp( struct Cyc_Absyn_Exp* e, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_increment_exp( e,( void*) Cyc_Absyn_PreInc, loc);} struct
Cyc_Absyn_Exp* Cyc_Absyn_pre_dec_exp( struct Cyc_Absyn_Exp* e, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_increment_exp( e,( void*) Cyc_Absyn_PreDec, loc);} struct
Cyc_Absyn_Exp* Cyc_Absyn_post_dec_exp( struct Cyc_Absyn_Exp* e, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_increment_exp( e,( void*) Cyc_Absyn_PostDec, loc);}
struct Cyc_Absyn_Exp* Cyc_Absyn_conditional_exp( struct Cyc_Absyn_Exp* e1,
struct Cyc_Absyn_Exp* e2, struct Cyc_Absyn_Exp* e3, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_new_exp(( void*)({ struct Cyc_Absyn_Conditional_e_struct*
_temp213=( struct Cyc_Absyn_Conditional_e_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Conditional_e_struct));
_temp213[ 0]=({ struct Cyc_Absyn_Conditional_e_struct _temp214; _temp214.tag=
Cyc_Absyn_Conditional_e; _temp214.f1= e1; _temp214.f2= e2; _temp214.f3= e3;
_temp214;}); _temp213;}), loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_and_exp( struct
Cyc_Absyn_Exp* e1, struct Cyc_Absyn_Exp* e2, struct Cyc_Position_Segment* loc){
return Cyc_Absyn_conditional_exp( e1, e2, Cyc_Absyn_false_exp( loc), loc);}
struct Cyc_Absyn_Exp* Cyc_Absyn_or_exp( struct Cyc_Absyn_Exp* e1, struct Cyc_Absyn_Exp*
e2, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_conditional_exp( e1, Cyc_Absyn_true_exp(
loc), e2, loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_seq_exp( struct Cyc_Absyn_Exp*
e1, struct Cyc_Absyn_Exp* e2, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_exp((
void*)({ struct Cyc_Absyn_SeqExp_e_struct* _temp215=( struct Cyc_Absyn_SeqExp_e_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_SeqExp_e_struct)); _temp215[ 0]=({ struct
Cyc_Absyn_SeqExp_e_struct _temp216; _temp216.tag= Cyc_Absyn_SeqExp_e; _temp216.f1=
e1; _temp216.f2= e2; _temp216;}); _temp215;}), loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_unknowncall_exp(
struct Cyc_Absyn_Exp* e, struct Cyc_List_List* es, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_new_exp(( void*)({ struct Cyc_Absyn_UnknownCall_e_struct*
_temp217=( struct Cyc_Absyn_UnknownCall_e_struct*) GC_malloc( sizeof( struct Cyc_Absyn_UnknownCall_e_struct));
_temp217[ 0]=({ struct Cyc_Absyn_UnknownCall_e_struct _temp218; _temp218.tag=
Cyc_Absyn_UnknownCall_e; _temp218.f1= e; _temp218.f2= es; _temp218;}); _temp217;}),
loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_fncall_exp( struct Cyc_Absyn_Exp* e,
struct Cyc_List_List* es, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_exp((
void*)({ struct Cyc_Absyn_FnCall_e_struct* _temp219=( struct Cyc_Absyn_FnCall_e_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_FnCall_e_struct)); _temp219[ 0]=({ struct
Cyc_Absyn_FnCall_e_struct _temp220; _temp220.tag= Cyc_Absyn_FnCall_e; _temp220.f1=
e; _temp220.f2= es; _temp220.f3= 0; _temp220;}); _temp219;}), loc);} struct Cyc_Absyn_Exp*
Cyc_Absyn_noinstantiate_exp( struct Cyc_Absyn_Exp* e, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_new_exp(( void*)({ struct Cyc_Absyn_NoInstantiate_e_struct*
_temp221=( struct Cyc_Absyn_NoInstantiate_e_struct*) GC_malloc( sizeof( struct
Cyc_Absyn_NoInstantiate_e_struct)); _temp221[ 0]=({ struct Cyc_Absyn_NoInstantiate_e_struct
_temp222; _temp222.tag= Cyc_Absyn_NoInstantiate_e; _temp222.f1= e; _temp222;});
_temp221;}), loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_instantiate_exp( struct Cyc_Absyn_Exp*
e, struct Cyc_List_List* ts, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_exp((
void*)({ struct Cyc_Absyn_Instantiate_e_struct* _temp223=( struct Cyc_Absyn_Instantiate_e_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_Instantiate_e_struct)); _temp223[ 0]=({
struct Cyc_Absyn_Instantiate_e_struct _temp224; _temp224.tag= Cyc_Absyn_Instantiate_e;
_temp224.f1= e; _temp224.f2= ts; _temp224;}); _temp223;}), loc);} struct Cyc_Absyn_Exp*
Cyc_Absyn_cast_exp( void* t, struct Cyc_Absyn_Exp* e, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_new_exp(( void*)({ struct Cyc_Absyn_Cast_e_struct*
_temp225=( struct Cyc_Absyn_Cast_e_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Cast_e_struct));
_temp225[ 0]=({ struct Cyc_Absyn_Cast_e_struct _temp226; _temp226.tag= Cyc_Absyn_Cast_e;
_temp226.f1=( void*) t; _temp226.f2= e; _temp226;}); _temp225;}), loc);} struct
Cyc_Absyn_Exp* Cyc_Absyn_throw_exp( struct Cyc_Absyn_Exp* e, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_new_exp(( void*)({ struct Cyc_Absyn_Throw_e_struct*
_temp227=( struct Cyc_Absyn_Throw_e_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Throw_e_struct));
_temp227[ 0]=({ struct Cyc_Absyn_Throw_e_struct _temp228; _temp228.tag= Cyc_Absyn_Throw_e;
_temp228.f1= e; _temp228;}); _temp227;}), loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_address_exp(
struct Cyc_Absyn_Exp* e, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_exp((
void*)({ struct Cyc_Absyn_Address_e_struct* _temp229=( struct Cyc_Absyn_Address_e_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_Address_e_struct)); _temp229[ 0]=({ struct
Cyc_Absyn_Address_e_struct _temp230; _temp230.tag= Cyc_Absyn_Address_e; _temp230.f1=
e; _temp230;}); _temp229;}), loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_sizeoftyp_exp(
void* t, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_exp(( void*)({
struct Cyc_Absyn_Sizeoftyp_e_struct* _temp231=( struct Cyc_Absyn_Sizeoftyp_e_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_Sizeoftyp_e_struct)); _temp231[ 0]=({ struct
Cyc_Absyn_Sizeoftyp_e_struct _temp232; _temp232.tag= Cyc_Absyn_Sizeoftyp_e;
_temp232.f1=( void*) t; _temp232;}); _temp231;}), loc);} struct Cyc_Absyn_Exp*
Cyc_Absyn_sizeofexp_exp( struct Cyc_Absyn_Exp* e, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_new_exp(( void*)({ struct Cyc_Absyn_Sizeofexp_e_struct*
_temp233=( struct Cyc_Absyn_Sizeofexp_e_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Sizeofexp_e_struct));
_temp233[ 0]=({ struct Cyc_Absyn_Sizeofexp_e_struct _temp234; _temp234.tag= Cyc_Absyn_Sizeofexp_e;
_temp234.f1= e; _temp234;}); _temp233;}), loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_offsetof_exp(
void* t, struct _tagged_arr* n, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_exp((
void*)({ struct Cyc_Absyn_Offsetof_e_struct* _temp235=( struct Cyc_Absyn_Offsetof_e_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_Offsetof_e_struct)); _temp235[ 0]=({ struct
Cyc_Absyn_Offsetof_e_struct _temp236; _temp236.tag= Cyc_Absyn_Offsetof_e;
_temp236.f1=( void*) t; _temp236.f2= n; _temp236;}); _temp235;}), loc);} struct
Cyc_Absyn_Exp* Cyc_Absyn_deref_exp( struct Cyc_Absyn_Exp* e, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_new_exp(( void*)({ struct Cyc_Absyn_Deref_e_struct*
_temp237=( struct Cyc_Absyn_Deref_e_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Deref_e_struct));
_temp237[ 0]=({ struct Cyc_Absyn_Deref_e_struct _temp238; _temp238.tag= Cyc_Absyn_Deref_e;
_temp238.f1= e; _temp238;}); _temp237;}), loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_structmember_exp(
struct Cyc_Absyn_Exp* e, struct _tagged_arr* n, struct Cyc_Position_Segment* loc){
return Cyc_Absyn_new_exp(( void*)({ struct Cyc_Absyn_StructMember_e_struct*
_temp239=( struct Cyc_Absyn_StructMember_e_struct*) GC_malloc( sizeof( struct
Cyc_Absyn_StructMember_e_struct)); _temp239[ 0]=({ struct Cyc_Absyn_StructMember_e_struct
_temp240; _temp240.tag= Cyc_Absyn_StructMember_e; _temp240.f1= e; _temp240.f2= n;
_temp240;}); _temp239;}), loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_structarrow_exp(
struct Cyc_Absyn_Exp* e, struct _tagged_arr* n, struct Cyc_Position_Segment* loc){
return Cyc_Absyn_new_exp(( void*)({ struct Cyc_Absyn_StructArrow_e_struct*
_temp241=( struct Cyc_Absyn_StructArrow_e_struct*) GC_malloc( sizeof( struct Cyc_Absyn_StructArrow_e_struct));
_temp241[ 0]=({ struct Cyc_Absyn_StructArrow_e_struct _temp242; _temp242.tag=
Cyc_Absyn_StructArrow_e; _temp242.f1= e; _temp242.f2= n; _temp242;}); _temp241;}),
loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_arrow_exp( struct Cyc_Absyn_Exp* e,
struct _tagged_arr* n, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_deref_exp(
Cyc_Absyn_structmember_exp( e, n, loc), loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_subscript_exp(
struct Cyc_Absyn_Exp* e1, struct Cyc_Absyn_Exp* e2, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_new_exp(( void*)({ struct Cyc_Absyn_Subscript_e_struct*
_temp243=( struct Cyc_Absyn_Subscript_e_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Subscript_e_struct));
_temp243[ 0]=({ struct Cyc_Absyn_Subscript_e_struct _temp244; _temp244.tag= Cyc_Absyn_Subscript_e;
_temp244.f1= e1; _temp244.f2= e2; _temp244;}); _temp243;}), loc);} struct Cyc_Absyn_Exp*
Cyc_Absyn_tuple_exp( struct Cyc_List_List* es, struct Cyc_Position_Segment* loc){
return Cyc_Absyn_new_exp(( void*)({ struct Cyc_Absyn_Tuple_e_struct* _temp245=(
struct Cyc_Absyn_Tuple_e_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Tuple_e_struct));
_temp245[ 0]=({ struct Cyc_Absyn_Tuple_e_struct _temp246; _temp246.tag= Cyc_Absyn_Tuple_e;
_temp246.f1= es; _temp246;}); _temp245;}), loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_stmt_exp(
struct Cyc_Absyn_Stmt* s, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_exp((
void*)({ struct Cyc_Absyn_StmtExp_e_struct* _temp247=( struct Cyc_Absyn_StmtExp_e_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_StmtExp_e_struct)); _temp247[ 0]=({ struct
Cyc_Absyn_StmtExp_e_struct _temp248; _temp248.tag= Cyc_Absyn_StmtExp_e; _temp248.f1=
s; _temp248;}); _temp247;}), loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_null_pointer_exn_exp(
struct Cyc_Position_Segment* loc){ return Cyc_Absyn_var_exp(& Cyc_Absyn_null_pointer_exn_name_v,
loc);} struct Cyc_Absyn_Exp* Cyc_Absyn_match_exn_exp( struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_var_exp(& Cyc_Absyn_match_exn_name_v, loc);} struct
_tuple5{ struct Cyc_List_List* f1; struct Cyc_Absyn_Exp* f2; } ; struct Cyc_Absyn_Exp*
Cyc_Absyn_array_exp( struct Cyc_List_List* es, struct Cyc_Position_Segment* loc){
struct Cyc_List_List* _temp249= 0; goto _LL250; _LL250: for( 0; es != 0; es=((
struct Cyc_List_List*) _check_null( es))->tl){ _temp249=({ struct Cyc_List_List*
_temp251=( struct Cyc_List_List*) GC_malloc( sizeof( struct Cyc_List_List));
_temp251->hd=( void*)({ struct _tuple5* _temp252=( struct _tuple5*) GC_malloc(
sizeof( struct _tuple5)); _temp252->f1= 0; _temp252->f2=( struct Cyc_Absyn_Exp*)((
struct Cyc_List_List*) _check_null( es))->hd; _temp252;}); _temp251->tl=
_temp249; _temp251;});} _temp249=(( struct Cyc_List_List*(*)( struct Cyc_List_List*
x)) Cyc_List_imp_rev)( _temp249); return Cyc_Absyn_new_exp(( void*)({ struct Cyc_Absyn_Array_e_struct*
_temp253=( struct Cyc_Absyn_Array_e_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Array_e_struct));
_temp253[ 0]=({ struct Cyc_Absyn_Array_e_struct _temp254; _temp254.tag= Cyc_Absyn_Array_e;
_temp254.f1= _temp249; _temp254;}); _temp253;}), loc);} struct Cyc_Absyn_Exp*
Cyc_Absyn_unresolvedmem_exp( struct Cyc_Core_Opt* n, struct Cyc_List_List* dles,
struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_exp(( void*)({ struct
Cyc_Absyn_UnresolvedMem_e_struct* _temp255=( struct Cyc_Absyn_UnresolvedMem_e_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_UnresolvedMem_e_struct)); _temp255[ 0]=({
struct Cyc_Absyn_UnresolvedMem_e_struct _temp256; _temp256.tag= Cyc_Absyn_UnresolvedMem_e;
_temp256.f1= n; _temp256.f2= dles; _temp256;}); _temp255;}), loc);} struct Cyc_Absyn_Stmt*
Cyc_Absyn_new_stmt( void* s, struct Cyc_Position_Segment* loc){ return({ struct
Cyc_Absyn_Stmt* _temp257=( struct Cyc_Absyn_Stmt*) GC_malloc( sizeof( struct Cyc_Absyn_Stmt));
_temp257->r=( void*) s; _temp257->loc= loc; _temp257->non_local_preds= 0;
_temp257->try_depth= 0; _temp257->annot=( void*)(( void*) Cyc_Absyn_EmptyAnnot);
_temp257;});} struct Cyc_Absyn_Stmt* Cyc_Absyn_skip_stmt( struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_new_stmt(( void*) Cyc_Absyn_Skip_s, loc);} struct Cyc_Absyn_Stmt*
Cyc_Absyn_exp_stmt( struct Cyc_Absyn_Exp* e, struct Cyc_Position_Segment* loc){
return Cyc_Absyn_new_stmt(( void*)({ struct Cyc_Absyn_Exp_s_struct* _temp258=(
struct Cyc_Absyn_Exp_s_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Exp_s_struct));
_temp258[ 0]=({ struct Cyc_Absyn_Exp_s_struct _temp259; _temp259.tag= Cyc_Absyn_Exp_s;
_temp259.f1= e; _temp259;}); _temp258;}), loc);} struct Cyc_Absyn_Stmt* Cyc_Absyn_seq_stmts(
struct Cyc_List_List* ss, struct Cyc_Position_Segment* loc){ if( ss == 0){
return Cyc_Absyn_skip_stmt( loc);} else{ if((( struct Cyc_List_List*)
_check_null( ss))->tl == 0){ return( struct Cyc_Absyn_Stmt*)(( struct Cyc_List_List*)
_check_null( ss))->hd;} else{ return Cyc_Absyn_seq_stmt(( struct Cyc_Absyn_Stmt*)((
struct Cyc_List_List*) _check_null( ss))->hd, Cyc_Absyn_seq_stmts((( struct Cyc_List_List*)
_check_null( ss))->tl, loc), loc);}}} struct Cyc_Absyn_Stmt* Cyc_Absyn_return_stmt(
struct Cyc_Absyn_Exp* e, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_stmt((
void*)({ struct Cyc_Absyn_Return_s_struct* _temp260=( struct Cyc_Absyn_Return_s_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_Return_s_struct)); _temp260[ 0]=({ struct
Cyc_Absyn_Return_s_struct _temp261; _temp261.tag= Cyc_Absyn_Return_s; _temp261.f1=
e; _temp261;}); _temp260;}), loc);} struct Cyc_Absyn_Stmt* Cyc_Absyn_ifthenelse_stmt(
struct Cyc_Absyn_Exp* e, struct Cyc_Absyn_Stmt* s1, struct Cyc_Absyn_Stmt* s2,
struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_stmt(( void*)({ struct
Cyc_Absyn_IfThenElse_s_struct* _temp262=( struct Cyc_Absyn_IfThenElse_s_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_IfThenElse_s_struct)); _temp262[ 0]=({
struct Cyc_Absyn_IfThenElse_s_struct _temp263; _temp263.tag= Cyc_Absyn_IfThenElse_s;
_temp263.f1= e; _temp263.f2= s1; _temp263.f3= s2; _temp263;}); _temp262;}), loc);}
struct Cyc_Absyn_Stmt* Cyc_Absyn_while_stmt( struct Cyc_Absyn_Exp* e, struct Cyc_Absyn_Stmt*
s, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_stmt(( void*)({
struct Cyc_Absyn_While_s_struct* _temp264=( struct Cyc_Absyn_While_s_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_While_s_struct)); _temp264[ 0]=({ struct Cyc_Absyn_While_s_struct
_temp265; _temp265.tag= Cyc_Absyn_While_s; _temp265.f1=({ struct _tuple2
_temp266; _temp266.f1= e; _temp266.f2= Cyc_Absyn_skip_stmt( e->loc); _temp266;});
_temp265.f2= s; _temp265;}); _temp264;}), loc);} struct Cyc_Absyn_Stmt* Cyc_Absyn_break_stmt(
struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_stmt(( void*)({ struct
Cyc_Absyn_Break_s_struct* _temp267=( struct Cyc_Absyn_Break_s_struct*) GC_malloc(
sizeof( struct Cyc_Absyn_Break_s_struct)); _temp267[ 0]=({ struct Cyc_Absyn_Break_s_struct
_temp268; _temp268.tag= Cyc_Absyn_Break_s; _temp268.f1= 0; _temp268;}); _temp267;}),
loc);} struct Cyc_Absyn_Stmt* Cyc_Absyn_continue_stmt( struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_new_stmt(( void*)({ struct Cyc_Absyn_Continue_s_struct*
_temp269=( struct Cyc_Absyn_Continue_s_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Continue_s_struct));
_temp269[ 0]=({ struct Cyc_Absyn_Continue_s_struct _temp270; _temp270.tag= Cyc_Absyn_Continue_s;
_temp270.f1= 0; _temp270;}); _temp269;}), loc);} struct Cyc_Absyn_Stmt* Cyc_Absyn_for_stmt(
struct Cyc_Absyn_Exp* e1, struct Cyc_Absyn_Exp* e2, struct Cyc_Absyn_Exp* e3,
struct Cyc_Absyn_Stmt* s, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_stmt((
void*)({ struct Cyc_Absyn_For_s_struct* _temp271=( struct Cyc_Absyn_For_s_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_For_s_struct)); _temp271[ 0]=({ struct Cyc_Absyn_For_s_struct
_temp272; _temp272.tag= Cyc_Absyn_For_s; _temp272.f1= e1; _temp272.f2=({ struct
_tuple2 _temp273; _temp273.f1= e2; _temp273.f2= Cyc_Absyn_skip_stmt( e3->loc);
_temp273;}); _temp272.f3=({ struct _tuple2 _temp274; _temp274.f1= e3; _temp274.f2=
Cyc_Absyn_skip_stmt( e3->loc); _temp274;}); _temp272.f4= s; _temp272;});
_temp271;}), loc);} struct Cyc_Absyn_Stmt* Cyc_Absyn_switch_stmt( struct Cyc_Absyn_Exp*
e, struct Cyc_List_List* scs, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_stmt((
void*)({ struct Cyc_Absyn_Switch_s_struct* _temp275=( struct Cyc_Absyn_Switch_s_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_Switch_s_struct)); _temp275[ 0]=({ struct
Cyc_Absyn_Switch_s_struct _temp276; _temp276.tag= Cyc_Absyn_Switch_s; _temp276.f1=
e; _temp276.f2= scs; _temp276;}); _temp275;}), loc);} struct Cyc_Absyn_Stmt* Cyc_Absyn_seq_stmt(
struct Cyc_Absyn_Stmt* s1, struct Cyc_Absyn_Stmt* s2, struct Cyc_Position_Segment*
loc){ void* _temp277=( void*) s1->r; _LL279: if( _temp277 ==( void*) Cyc_Absyn_Skip_s){
goto _LL280;} else{ goto _LL281;} _LL281: goto _LL282; _LL280: return s2; _LL282:
return Cyc_Absyn_new_stmt(( void*)({ struct Cyc_Absyn_Seq_s_struct* _temp283=(
struct Cyc_Absyn_Seq_s_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Seq_s_struct));
_temp283[ 0]=({ struct Cyc_Absyn_Seq_s_struct _temp284; _temp284.tag= Cyc_Absyn_Seq_s;
_temp284.f1= s1; _temp284.f2= s2; _temp284;}); _temp283;}), loc); _LL278:;}
struct Cyc_Absyn_Stmt* Cyc_Absyn_fallthru_stmt( struct Cyc_List_List* el, struct
Cyc_Position_Segment* loc){ return Cyc_Absyn_new_stmt(( void*)({ struct Cyc_Absyn_Fallthru_s_struct*
_temp285=( struct Cyc_Absyn_Fallthru_s_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Fallthru_s_struct));
_temp285[ 0]=({ struct Cyc_Absyn_Fallthru_s_struct _temp286; _temp286.tag= Cyc_Absyn_Fallthru_s;
_temp286.f1= el; _temp286.f2= 0; _temp286;}); _temp285;}), loc);} struct Cyc_Absyn_Stmt*
Cyc_Absyn_decl_stmt( struct Cyc_Absyn_Decl* d, struct Cyc_Absyn_Stmt* s, struct
Cyc_Position_Segment* loc){ return Cyc_Absyn_new_stmt(( void*)({ struct Cyc_Absyn_Decl_s_struct*
_temp287=( struct Cyc_Absyn_Decl_s_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Decl_s_struct));
_temp287[ 0]=({ struct Cyc_Absyn_Decl_s_struct _temp288; _temp288.tag= Cyc_Absyn_Decl_s;
_temp288.f1= d; _temp288.f2= s; _temp288;}); _temp287;}), loc);} struct Cyc_Absyn_Stmt*
Cyc_Absyn_declare_stmt( struct _tuple0* x, void* t, struct Cyc_Absyn_Exp* init,
struct Cyc_Absyn_Stmt* s, struct Cyc_Position_Segment* loc){ struct Cyc_Absyn_Decl*
d= Cyc_Absyn_new_decl(( void*)({ struct Cyc_Absyn_Var_d_struct* _temp291=(
struct Cyc_Absyn_Var_d_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Var_d_struct));
_temp291[ 0]=({ struct Cyc_Absyn_Var_d_struct _temp292; _temp292.tag= Cyc_Absyn_Var_d;
_temp292.f1= Cyc_Absyn_new_vardecl( x, t, init); _temp292;}); _temp291;}), loc);
return Cyc_Absyn_new_stmt(( void*)({ struct Cyc_Absyn_Decl_s_struct* _temp289=(
struct Cyc_Absyn_Decl_s_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Decl_s_struct));
_temp289[ 0]=({ struct Cyc_Absyn_Decl_s_struct _temp290; _temp290.tag= Cyc_Absyn_Decl_s;
_temp290.f1= d; _temp290.f2= s; _temp290;}); _temp289;}), loc);} struct Cyc_Absyn_Stmt*
Cyc_Absyn_cut_stmt( struct Cyc_Absyn_Stmt* s, struct Cyc_Position_Segment* loc){
return Cyc_Absyn_new_stmt(( void*)({ struct Cyc_Absyn_Cut_s_struct* _temp293=(
struct Cyc_Absyn_Cut_s_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Cut_s_struct));
_temp293[ 0]=({ struct Cyc_Absyn_Cut_s_struct _temp294; _temp294.tag= Cyc_Absyn_Cut_s;
_temp294.f1= s; _temp294;}); _temp293;}), loc);} struct Cyc_Absyn_Stmt* Cyc_Absyn_splice_stmt(
struct Cyc_Absyn_Stmt* s, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_stmt((
void*)({ struct Cyc_Absyn_Splice_s_struct* _temp295=( struct Cyc_Absyn_Splice_s_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_Splice_s_struct)); _temp295[ 0]=({ struct
Cyc_Absyn_Splice_s_struct _temp296; _temp296.tag= Cyc_Absyn_Splice_s; _temp296.f1=
s; _temp296;}); _temp295;}), loc);} struct Cyc_Absyn_Stmt* Cyc_Absyn_label_stmt(
struct _tagged_arr* v, struct Cyc_Absyn_Stmt* s, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_new_stmt(( void*)({ struct Cyc_Absyn_Label_s_struct*
_temp297=( struct Cyc_Absyn_Label_s_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Label_s_struct));
_temp297[ 0]=({ struct Cyc_Absyn_Label_s_struct _temp298; _temp298.tag= Cyc_Absyn_Label_s;
_temp298.f1= v; _temp298.f2= s; _temp298;}); _temp297;}), loc);} struct Cyc_Absyn_Stmt*
Cyc_Absyn_do_stmt( struct Cyc_Absyn_Stmt* s, struct Cyc_Absyn_Exp* e, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_new_stmt(( void*)({ struct Cyc_Absyn_Do_s_struct*
_temp299=( struct Cyc_Absyn_Do_s_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Do_s_struct));
_temp299[ 0]=({ struct Cyc_Absyn_Do_s_struct _temp300; _temp300.tag= Cyc_Absyn_Do_s;
_temp300.f1= s; _temp300.f2=({ struct _tuple2 _temp301; _temp301.f1= e; _temp301.f2=
Cyc_Absyn_skip_stmt( e->loc); _temp301;}); _temp300;}); _temp299;}), loc);}
struct Cyc_Absyn_Stmt* Cyc_Absyn_trycatch_stmt( struct Cyc_Absyn_Stmt* s, struct
Cyc_List_List* scs, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_stmt((
void*)({ struct Cyc_Absyn_TryCatch_s_struct* _temp302=( struct Cyc_Absyn_TryCatch_s_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_TryCatch_s_struct)); _temp302[ 0]=({ struct
Cyc_Absyn_TryCatch_s_struct _temp303; _temp303.tag= Cyc_Absyn_TryCatch_s;
_temp303.f1= s; _temp303.f2= scs; _temp303;}); _temp302;}), loc);} struct Cyc_Absyn_Stmt*
Cyc_Absyn_goto_stmt( struct _tagged_arr* lab, struct Cyc_Position_Segment* loc){
return Cyc_Absyn_new_stmt(( void*)({ struct Cyc_Absyn_Goto_s_struct* _temp304=(
struct Cyc_Absyn_Goto_s_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Goto_s_struct));
_temp304[ 0]=({ struct Cyc_Absyn_Goto_s_struct _temp305; _temp305.tag= Cyc_Absyn_Goto_s;
_temp305.f1= lab; _temp305.f2= 0; _temp305;}); _temp304;}), loc);} struct Cyc_Absyn_Stmt*
Cyc_Absyn_assign_stmt( struct Cyc_Absyn_Exp* e1, struct Cyc_Absyn_Exp* e2,
struct Cyc_Position_Segment* loc){ return Cyc_Absyn_exp_stmt( Cyc_Absyn_assign_exp(
e1, e2, loc), loc);} struct Cyc_Absyn_Pat* Cyc_Absyn_new_pat( void* p, struct
Cyc_Position_Segment* s){ return({ struct Cyc_Absyn_Pat* _temp306=( struct Cyc_Absyn_Pat*)
GC_malloc( sizeof( struct Cyc_Absyn_Pat)); _temp306->r=( void*) p; _temp306->topt=
0; _temp306->loc= s; _temp306;});} struct Cyc_Absyn_Decl* Cyc_Absyn_new_decl(
void* r, struct Cyc_Position_Segment* loc){ return({ struct Cyc_Absyn_Decl*
_temp307=( struct Cyc_Absyn_Decl*) GC_malloc( sizeof( struct Cyc_Absyn_Decl));
_temp307->r=( void*) r; _temp307->loc= loc; _temp307;});} struct Cyc_Absyn_Decl*
Cyc_Absyn_let_decl( struct Cyc_Absyn_Pat* p, struct Cyc_Core_Opt* t_opt, struct
Cyc_Absyn_Exp* e, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_decl((
void*)({ struct Cyc_Absyn_Let_d_struct* _temp308=( struct Cyc_Absyn_Let_d_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_Let_d_struct)); _temp308[ 0]=({ struct Cyc_Absyn_Let_d_struct
_temp309; _temp309.tag= Cyc_Absyn_Let_d; _temp309.f1= p; _temp309.f2= 0;
_temp309.f3= t_opt; _temp309.f4= e; _temp309.f5= 0; _temp309;}); _temp308;}),
loc);} struct Cyc_Absyn_Decl* Cyc_Absyn_letv_decl( struct Cyc_List_List* vds,
struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_decl(( void*)({ struct
Cyc_Absyn_Letv_d_struct* _temp310=( struct Cyc_Absyn_Letv_d_struct*) GC_malloc(
sizeof( struct Cyc_Absyn_Letv_d_struct)); _temp310[ 0]=({ struct Cyc_Absyn_Letv_d_struct
_temp311; _temp311.tag= Cyc_Absyn_Letv_d; _temp311.f1= vds; _temp311;});
_temp310;}), loc);} struct Cyc_Absyn_Vardecl* Cyc_Absyn_new_vardecl( struct
_tuple0* x, void* t, struct Cyc_Absyn_Exp* init){ return({ struct Cyc_Absyn_Vardecl*
_temp312=( struct Cyc_Absyn_Vardecl*) GC_malloc( sizeof( struct Cyc_Absyn_Vardecl));
_temp312->sc=( void*)(( void*) Cyc_Absyn_Public); _temp312->name= x; _temp312->tq=
Cyc_Absyn_empty_tqual(); _temp312->type=( void*) t; _temp312->initializer= init;
_temp312->rgn= 0; _temp312->attributes= 0; _temp312;});} struct Cyc_Absyn_Vardecl*
Cyc_Absyn_static_vardecl( struct _tuple0* x, void* t, struct Cyc_Absyn_Exp* init){
return({ struct Cyc_Absyn_Vardecl* _temp313=( struct Cyc_Absyn_Vardecl*)
GC_malloc( sizeof( struct Cyc_Absyn_Vardecl)); _temp313->sc=( void*)(( void*)
Cyc_Absyn_Static); _temp313->name= x; _temp313->tq= Cyc_Absyn_empty_tqual();
_temp313->type=( void*) t; _temp313->initializer= init; _temp313->rgn= 0;
_temp313->attributes= 0; _temp313;});} struct Cyc_Absyn_Decl* Cyc_Absyn_struct_decl(
void* s, struct Cyc_Core_Opt* n, struct Cyc_List_List* ts, struct Cyc_Core_Opt*
fs, struct Cyc_List_List* atts, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_decl((
void*)({ struct Cyc_Absyn_Struct_d_struct* _temp314=( struct Cyc_Absyn_Struct_d_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_Struct_d_struct)); _temp314[ 0]=({ struct
Cyc_Absyn_Struct_d_struct _temp315; _temp315.tag= Cyc_Absyn_Struct_d; _temp315.f1=({
struct Cyc_Absyn_Structdecl* _temp316=( struct Cyc_Absyn_Structdecl*) GC_malloc(
sizeof( struct Cyc_Absyn_Structdecl)); _temp316->sc=( void*) s; _temp316->name=
n; _temp316->tvs= ts; _temp316->fields= fs; _temp316->attributes= atts; _temp316;});
_temp315;}); _temp314;}), loc);} struct Cyc_Absyn_Decl* Cyc_Absyn_union_decl(
void* s, struct Cyc_Core_Opt* n, struct Cyc_List_List* ts, struct Cyc_Core_Opt*
fs, struct Cyc_List_List* atts, struct Cyc_Position_Segment* loc){ return Cyc_Absyn_new_decl((
void*)({ struct Cyc_Absyn_Union_d_struct* _temp317=( struct Cyc_Absyn_Union_d_struct*)
GC_malloc( sizeof( struct Cyc_Absyn_Union_d_struct)); _temp317[ 0]=({ struct Cyc_Absyn_Union_d_struct
_temp318; _temp318.tag= Cyc_Absyn_Union_d; _temp318.f1=({ struct Cyc_Absyn_Uniondecl*
_temp319=( struct Cyc_Absyn_Uniondecl*) GC_malloc( sizeof( struct Cyc_Absyn_Uniondecl));
_temp319->sc=( void*) s; _temp319->name= n; _temp319->tvs= ts; _temp319->fields=
fs; _temp319->attributes= atts; _temp319;}); _temp318;}); _temp317;}), loc);}
struct Cyc_Absyn_Decl* Cyc_Absyn_tunion_decl( void* s, struct _tuple0* n, struct
Cyc_List_List* ts, struct Cyc_Core_Opt* fs, int is_xtunion, struct Cyc_Position_Segment*
loc){ return Cyc_Absyn_new_decl(( void*)({ struct Cyc_Absyn_Tunion_d_struct*
_temp320=( struct Cyc_Absyn_Tunion_d_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Tunion_d_struct));
_temp320[ 0]=({ struct Cyc_Absyn_Tunion_d_struct _temp321; _temp321.tag= Cyc_Absyn_Tunion_d;
_temp321.f1=({ struct Cyc_Absyn_Tuniondecl* _temp322=( struct Cyc_Absyn_Tuniondecl*)
GC_malloc( sizeof( struct Cyc_Absyn_Tuniondecl)); _temp322->sc=( void*) s;
_temp322->name= n; _temp322->tvs= ts; _temp322->fields= fs; _temp322->is_xtunion=
is_xtunion; _temp322;}); _temp321;}); _temp320;}), loc);} static struct _tuple1*
Cyc_Absyn_expand_arg( struct _tuple1* a){ return({ struct _tuple1* _temp323=(
struct _tuple1*) GC_malloc( sizeof( struct _tuple1)); _temp323->f1=(* a).f1;
_temp323->f2=(* a).f2; _temp323->f3= Cyc_Absyn_pointer_expand((* a).f3);
_temp323;});} void* Cyc_Absyn_function_typ( struct Cyc_List_List* tvs, struct
Cyc_Core_Opt* eff_typ, void* ret_typ, struct Cyc_List_List* args, int c_varargs,
struct Cyc_Absyn_VarargInfo* cyc_varargs, struct Cyc_List_List* rgn_po, struct
Cyc_List_List* atts){ return( void*)({ struct Cyc_Absyn_FnType_struct* _temp324=(
struct Cyc_Absyn_FnType_struct*) GC_malloc( sizeof( struct Cyc_Absyn_FnType_struct));
_temp324[ 0]=({ struct Cyc_Absyn_FnType_struct _temp325; _temp325.tag= Cyc_Absyn_FnType;
_temp325.f1=({ struct Cyc_Absyn_FnInfo _temp326; _temp326.tvars= tvs; _temp326.ret_typ=(
void*) Cyc_Absyn_pointer_expand( ret_typ); _temp326.effect= eff_typ; _temp326.args=((
struct Cyc_List_List*(*)( struct _tuple1*(* f)( struct _tuple1*), struct Cyc_List_List*
x)) Cyc_List_map)( Cyc_Absyn_expand_arg, args); _temp326.c_varargs= c_varargs;
_temp326.cyc_varargs= cyc_varargs; _temp326.rgn_po= rgn_po; _temp326.attributes=
atts; _temp326;}); _temp325;}); _temp324;});} void* Cyc_Absyn_pointer_expand(
void* t){ void* _temp327= t; struct Cyc_Absyn_FnInfo _temp333; _LL329: if((
unsigned int) _temp327 > 4u?*(( int*) _temp327) == Cyc_Absyn_FnType: 0){ _LL334:
_temp333=(( struct Cyc_Absyn_FnType_struct*) _temp327)->f1; goto _LL330;} else{
goto _LL331;} _LL331: goto _LL332; _LL330: return Cyc_Absyn_at_typ( t,( void*)
Cyc_Absyn_HeapRgn, Cyc_Absyn_empty_tqual()); _LL332: return t; _LL328:;} int Cyc_Absyn_is_lvalue(
struct Cyc_Absyn_Exp* e){ void* _temp335=( void*) e->r; void* _temp359; struct
Cyc_Absyn_Fndecl* _temp361; struct _tuple0* _temp363; void* _temp365; struct Cyc_Absyn_Vardecl*
_temp367; struct _tuple0* _temp369; void* _temp371; struct Cyc_Absyn_Vardecl*
_temp373; struct _tuple0* _temp375; void* _temp377; struct _tuple0* _temp379;
struct Cyc_Absyn_Exp* _temp381; struct Cyc_Absyn_Exp* _temp383; struct
_tagged_arr* _temp385; struct Cyc_Absyn_Exp* _temp387; struct _tagged_arr*
_temp389; struct Cyc_Absyn_Exp* _temp391; struct Cyc_Absyn_Exp* _temp393; struct
Cyc_List_List* _temp395; struct Cyc_Absyn_Exp* _temp397; struct Cyc_Absyn_Exp*
_temp399; _LL337: if(*(( int*) _temp335) == Cyc_Absyn_Var_e){ _LL364: _temp363=((
struct Cyc_Absyn_Var_e_struct*) _temp335)->f1; goto _LL360; _LL360: _temp359=(
void*)(( struct Cyc_Absyn_Var_e_struct*) _temp335)->f2; if(( unsigned int)
_temp359 > 1u?*(( int*) _temp359) == Cyc_Absyn_Funname_b: 0){ _LL362: _temp361=((
struct Cyc_Absyn_Funname_b_struct*) _temp359)->f1; goto _LL338;} else{ goto
_LL339;}} else{ goto _LL339;} _LL339: if(*(( int*) _temp335) == Cyc_Absyn_Var_e){
_LL370: _temp369=(( struct Cyc_Absyn_Var_e_struct*) _temp335)->f1; goto _LL366;
_LL366: _temp365=( void*)(( struct Cyc_Absyn_Var_e_struct*) _temp335)->f2; if((
unsigned int) _temp365 > 1u?*(( int*) _temp365) == Cyc_Absyn_Global_b: 0){
_LL368: _temp367=(( struct Cyc_Absyn_Global_b_struct*) _temp365)->f1; goto
_LL340;} else{ goto _LL341;}} else{ goto _LL341;} _LL341: if(*(( int*) _temp335)
== Cyc_Absyn_Var_e){ _LL376: _temp375=(( struct Cyc_Absyn_Var_e_struct*)
_temp335)->f1; goto _LL372; _LL372: _temp371=( void*)(( struct Cyc_Absyn_Var_e_struct*)
_temp335)->f2; if(( unsigned int) _temp371 > 1u?*(( int*) _temp371) == Cyc_Absyn_Local_b:
0){ _LL374: _temp373=(( struct Cyc_Absyn_Local_b_struct*) _temp371)->f1; goto
_LL342;} else{ goto _LL343;}} else{ goto _LL343;} _LL343: if(*(( int*) _temp335)
== Cyc_Absyn_Var_e){ _LL380: _temp379=(( struct Cyc_Absyn_Var_e_struct*)
_temp335)->f1; goto _LL378; _LL378: _temp377=( void*)(( struct Cyc_Absyn_Var_e_struct*)
_temp335)->f2; goto _LL344;} else{ goto _LL345;} _LL345: if(*(( int*) _temp335)
== Cyc_Absyn_Subscript_e){ _LL384: _temp383=(( struct Cyc_Absyn_Subscript_e_struct*)
_temp335)->f1; goto _LL382; _LL382: _temp381=(( struct Cyc_Absyn_Subscript_e_struct*)
_temp335)->f2; goto _LL346;} else{ goto _LL347;} _LL347: if(*(( int*) _temp335)
== Cyc_Absyn_StructMember_e){ _LL388: _temp387=(( struct Cyc_Absyn_StructMember_e_struct*)
_temp335)->f1; goto _LL386; _LL386: _temp385=(( struct Cyc_Absyn_StructMember_e_struct*)
_temp335)->f2; goto _LL348;} else{ goto _LL349;} _LL349: if(*(( int*) _temp335)
== Cyc_Absyn_StructArrow_e){ _LL392: _temp391=(( struct Cyc_Absyn_StructArrow_e_struct*)
_temp335)->f1; goto _LL390; _LL390: _temp389=(( struct Cyc_Absyn_StructArrow_e_struct*)
_temp335)->f2; goto _LL350;} else{ goto _LL351;} _LL351: if(*(( int*) _temp335)
== Cyc_Absyn_Deref_e){ _LL394: _temp393=(( struct Cyc_Absyn_Deref_e_struct*)
_temp335)->f1; goto _LL352;} else{ goto _LL353;} _LL353: if(*(( int*) _temp335)
== Cyc_Absyn_Instantiate_e){ _LL398: _temp397=(( struct Cyc_Absyn_Instantiate_e_struct*)
_temp335)->f1; goto _LL396; _LL396: _temp395=(( struct Cyc_Absyn_Instantiate_e_struct*)
_temp335)->f2; goto _LL354;} else{ goto _LL355;} _LL355: if(*(( int*) _temp335)
== Cyc_Absyn_NoInstantiate_e){ _LL400: _temp399=(( struct Cyc_Absyn_NoInstantiate_e_struct*)
_temp335)->f1; goto _LL356;} else{ goto _LL357;} _LL357: goto _LL358; _LL338:
return 0; _LL340: _temp373= _temp367; goto _LL342; _LL342: { void* _temp401= Cyc_Tcutil_compress((
void*) _temp373->type); struct Cyc_Absyn_Exp* _temp407; struct Cyc_Absyn_Tqual
_temp409; void* _temp411; _LL403: if(( unsigned int) _temp401 > 4u?*(( int*)
_temp401) == Cyc_Absyn_ArrayType: 0){ _LL412: _temp411=( void*)(( struct Cyc_Absyn_ArrayType_struct*)
_temp401)->f1; goto _LL410; _LL410: _temp409=(( struct Cyc_Absyn_ArrayType_struct*)
_temp401)->f2; goto _LL408; _LL408: _temp407=(( struct Cyc_Absyn_ArrayType_struct*)
_temp401)->f3; goto _LL404;} else{ goto _LL405;} _LL405: goto _LL406; _LL404:
return 0; _LL406: return 1; _LL402:;} _LL344: return 1; _LL346: return 1; _LL348:
return Cyc_Absyn_is_lvalue( _temp387); _LL350: return 1; _LL352: return 1;
_LL354: return Cyc_Absyn_is_lvalue( _temp397); _LL356: return Cyc_Absyn_is_lvalue(
_temp399); _LL358: return 0; _LL336:;} struct Cyc_Absyn_Structfield* Cyc_Absyn_lookup_field(
struct Cyc_List_List* fields, struct _tagged_arr* v){{ struct Cyc_List_List* fs=
fields; for( 0; fs != 0; fs=(( struct Cyc_List_List*) _check_null( fs))->tl){
struct Cyc_Absyn_Structfield* _temp413=( struct Cyc_Absyn_Structfield*)(( struct
Cyc_List_List*) _check_null( fs))->hd; goto _LL414; _LL414: if( Cyc_String_zstrptrcmp(
_temp413->name, v) == 0){ return( struct Cyc_Absyn_Structfield*) _temp413;}}}
return 0;} struct Cyc_Absyn_Structfield* Cyc_Absyn_lookup_struct_field( struct
Cyc_Absyn_Structdecl* sd, struct _tagged_arr* v){ return sd->fields == 0? 0: Cyc_Absyn_lookup_field((
struct Cyc_List_List*)(( struct Cyc_Core_Opt*) _check_null( sd->fields))->v, v);}
struct Cyc_Absyn_Structfield* Cyc_Absyn_lookup_union_field( struct Cyc_Absyn_Uniondecl*
ud, struct _tagged_arr* v){ return ud->fields == 0? 0: Cyc_Absyn_lookup_field((
struct Cyc_List_List*)(( struct Cyc_Core_Opt*) _check_null( ud->fields))->v, v);}
struct _tuple3* Cyc_Absyn_lookup_tuple_field( struct Cyc_List_List* ts, int i){
for( 0; i != 0; -- i){ if( ts == 0){ return 0;} ts=(( struct Cyc_List_List*)
_check_null( ts))->tl;} if( ts == 0){ return 0;} return( struct _tuple3*)((
struct _tuple3*)(( struct Cyc_List_List*) _check_null( ts))->hd);} struct
_tagged_arr Cyc_Absyn_attribute2string( void* a){ void* _temp415= a; int
_temp455; int _temp457; struct _tagged_arr _temp459; _LL417: if(( unsigned int)
_temp415 > 16u?*(( int*) _temp415) == Cyc_Absyn_Regparm_att: 0){ _LL456:
_temp455=(( struct Cyc_Absyn_Regparm_att_struct*) _temp415)->f1; goto _LL418;}
else{ goto _LL419;} _LL419: if( _temp415 ==( void*) Cyc_Absyn_Stdcall_att){ goto
_LL420;} else{ goto _LL421;} _LL421: if( _temp415 ==( void*) Cyc_Absyn_Cdecl_att){
goto _LL422;} else{ goto _LL423;} _LL423: if( _temp415 ==( void*) Cyc_Absyn_Fastcall_att){
goto _LL424;} else{ goto _LL425;} _LL425: if( _temp415 ==( void*) Cyc_Absyn_Noreturn_att){
goto _LL426;} else{ goto _LL427;} _LL427: if( _temp415 ==( void*) Cyc_Absyn_Const_att){
goto _LL428;} else{ goto _LL429;} _LL429: if(( unsigned int) _temp415 > 16u?*((
int*) _temp415) == Cyc_Absyn_Aligned_att: 0){ _LL458: _temp457=(( struct Cyc_Absyn_Aligned_att_struct*)
_temp415)->f1; goto _LL430;} else{ goto _LL431;} _LL431: if( _temp415 ==( void*)
Cyc_Absyn_Packed_att){ goto _LL432;} else{ goto _LL433;} _LL433: if((
unsigned int) _temp415 > 16u?*(( int*) _temp415) == Cyc_Absyn_Section_att: 0){
_LL460: _temp459=(( struct Cyc_Absyn_Section_att_struct*) _temp415)->f1; goto
_LL434;} else{ goto _LL435;} _LL435: if( _temp415 ==( void*) Cyc_Absyn_Nocommon_att){
goto _LL436;} else{ goto _LL437;} _LL437: if( _temp415 ==( void*) Cyc_Absyn_Shared_att){
goto _LL438;} else{ goto _LL439;} _LL439: if( _temp415 ==( void*) Cyc_Absyn_Unused_att){
goto _LL440;} else{ goto _LL441;} _LL441: if( _temp415 ==( void*) Cyc_Absyn_Weak_att){
goto _LL442;} else{ goto _LL443;} _LL443: if( _temp415 ==( void*) Cyc_Absyn_Dllimport_att){
goto _LL444;} else{ goto _LL445;} _LL445: if( _temp415 ==( void*) Cyc_Absyn_Dllexport_att){
goto _LL446;} else{ goto _LL447;} _LL447: if( _temp415 ==( void*) Cyc_Absyn_No_instrument_function_att){
goto _LL448;} else{ goto _LL449;} _LL449: if( _temp415 ==( void*) Cyc_Absyn_Constructor_att){
goto _LL450;} else{ goto _LL451;} _LL451: if( _temp415 ==( void*) Cyc_Absyn_Destructor_att){
goto _LL452;} else{ goto _LL453;} _LL453: if( _temp415 ==( void*) Cyc_Absyn_No_check_memory_usage_att){
goto _LL454;} else{ goto _LL416;} _LL418: return( struct _tagged_arr) xprintf("regparm(%d)",
_temp455); _LL420: return _tag_arr("stdcall", sizeof( unsigned char), 8u);
_LL422: return _tag_arr("cdecl", sizeof( unsigned char), 6u); _LL424: return
_tag_arr("fastcall", sizeof( unsigned char), 9u); _LL426: return _tag_arr("noreturn",
sizeof( unsigned char), 9u); _LL428: return _tag_arr("const", sizeof(
unsigned char), 6u); _LL430: if( _temp457 == - 1){ return _tag_arr("aligned",
sizeof( unsigned char), 8u);} else{ return( struct _tagged_arr) xprintf("aligned(%d)",
_temp457);} _LL432: return _tag_arr("packed", sizeof( unsigned char), 7u);
_LL434: return( struct _tagged_arr)({ struct _tagged_arr _temp461= _temp459;
xprintf("section(\"%.*s\")", _get_arr_size( _temp461, 1u), _temp461.curr);});
_LL436: return _tag_arr("nocommon", sizeof( unsigned char), 9u); _LL438: return
_tag_arr("shared", sizeof( unsigned char), 7u); _LL440: return _tag_arr("unused",
sizeof( unsigned char), 7u); _LL442: return _tag_arr("weak", sizeof(
unsigned char), 5u); _LL444: return _tag_arr("dllimport", sizeof( unsigned char),
10u); _LL446: return _tag_arr("dllexport", sizeof( unsigned char), 10u); _LL448:
return _tag_arr("no_instrument_function", sizeof( unsigned char), 23u); _LL450:
return _tag_arr("constructor", sizeof( unsigned char), 12u); _LL452: return
_tag_arr("destructor", sizeof( unsigned char), 11u); _LL454: return _tag_arr("no_check_memory_usage",
sizeof( unsigned char), 22u); _LL416:;} int Cyc_Absyn_fntype_att( void* a){ void*
_temp462= a; int _temp476; _LL464: if(( unsigned int) _temp462 > 16u?*(( int*)
_temp462) == Cyc_Absyn_Regparm_att: 0){ _LL477: _temp476=(( struct Cyc_Absyn_Regparm_att_struct*)
_temp462)->f1; goto _LL465;} else{ goto _LL466;} _LL466: if( _temp462 ==( void*)
Cyc_Absyn_Stdcall_att){ goto _LL467;} else{ goto _LL468;} _LL468: if( _temp462
==( void*) Cyc_Absyn_Cdecl_att){ goto _LL469;} else{ goto _LL470;} _LL470: if(
_temp462 ==( void*) Cyc_Absyn_Noreturn_att){ goto _LL471;} else{ goto _LL472;}
_LL472: if( _temp462 ==( void*) Cyc_Absyn_Const_att){ goto _LL473;} else{ goto
_LL474;} _LL474: goto _LL475; _LL465: goto _LL467; _LL467: goto _LL469; _LL469:
goto _LL471; _LL471: goto _LL473; _LL473: return 1; _LL475: return 0; _LL463:;}