#ifndef TCENV_H
#define TCENV_H

#include "core.h"
#include "list.h"
#include "set.h"
#include "dict.h"
#include "absyn.h"
#include "position.h"

namespace Tcenv {

using Core;
using List;
using Set;
using Dict;
using Absyn;
using Position;

// Used to tell what an ordinary identifer refers to 
extern enum Resolved {
  VarRes(binding_t); // includes unresolved variant
  StructRes(structdecl);
  EnumRes(enumdecl,enumfield);
  XenumRes(xenumdecl,enumfield);
};
typedef enum Resolved resolved_t;

// Global environments -- what's declared in a global scope 
extern struct Genv {
  Set<var>              namespaces;
  Dict<var,structdecl>  structdecls;
  Dict<var,enumdecl>    enumdecls;
  Dict<var,xenumdecl>   xenumdecls;
  Dict<var,typedefdecl> typedefs;
  Dict<var,resolved_t>  ordinaries;
  list<list<var>>       availables; // "using" namespaces
};
typedef struct Genv @genv;

// Local function environments
extern struct Fenv;
typedef struct Fenv @fenv; 

extern enum Jumpee { 
  NotAllowed_j;
  FnEnd_j;
  Stmt_j(stmt);
};
typedef enum Jumpee jumpee_t;

// Models the nesting of the RTCG constructs 
extern enum Frames<`a> {
  Outermost(`a);
  Frame(`a,enum Frames<`a>);
  Hidden(`a,enum Frames<`a>);
};
typedef enum Frames<`a> frames<`a>;

// Type environments 
extern struct Tenv {
  list<var>            ns; // current namespace
  Dict<list<var>,genv> ae; // absolute environment
  Opt_t<frames<fenv>>  le; // local environment, == null except in functions
};
typedef struct Tenv @tenv;

extern tenv set_in_loop(tenv te, stmt continue_dest);
extern tenv set_in_switch(tenv);
extern bool process_continue(tenv,stmt);
extern bool process_break(tenv,stmt);
extern void process_goto(tenv,stmt);
extern $(stmt, list<tvar>,list<typ>)* process_fallthru(tenv, stmt);

extern tenv set_fallthru(tenv te, 
			 $(list<tvar>,list<vardecl>) * pat_typ,
			 stmt body);
extern tenv clear_fallthru(tenv);
extern tenv set_next(tenv, jumpee_t);

extern tenv enter_ns(tenv, var);
extern genv genv_concat(genv, genv);

// lookup functions 
extern list<var>   resolve_namespace(tenv,segment,list<var>);
extern genv        lookup_namespace(tenv,segment,list<var>);
extern resolved_t  lookup_ordinary(tenv,segment,qvar);
extern structdecl  lookup_structdecl(tenv,segment,qvar);
extern enumdecl    lookup_enumdecl(tenv,segment,qvar);
extern Opt_t<xenumdecl> lookup_xenumdecl(tenv,segment,qvar);
extern typedefdecl lookup_typedefdecl(tenv,segment,qvar);
extern list<tvar>  lookup_type_vars(tenv);

extern tenv add_local_var(segment,tenv,vardecl);
extern tenv add_pat_var  (segment,tenv,vardecl);

extern tenv add_type_vars(segment,tenv,list<tvar>);

// what we synthesize when type-checking a statement or expression:
extern struct Synth;
typedef struct Synth @synth;
// whether the statement/expr may implicitly fall through to the next
extern bool synth_may_implicit_fallthru(synth);
// whether the statement/expr explicitly falls through to the next
extern bool synth_may_explicit_fallthru(synth);
// the type of the expression (only call on synth's generated by exps)
extern typ synth_typ(synth);
// given a synth, set the type to t -- IMPERATIVE!!!
extern synth synth_set_typ(synth s,typ t);

  // FIX: separate pass and include var id's
extern enum Unassigned;
typedef enum Unassigned unassigned_t;

extern unassigned_t merge_unassigned(unassigned_t, unassigned_t);

// make the unassigned set in the resulting tenv come from the 
// "fallthrough" edge of the synth
extern tenv layer_synth(tenv,synth);
// give back two environments -- one when the exp is true and one when false
extern $(tenv,tenv) bool_layer_synth(tenv,synth);
// synth we get for most expressions and atomic statements -- implicit 
// fallthru with no forward jump, and all currently unassigned variables as
// unassigned on the normal edge, empty set of unassigned variables on
// the forward jump edge.
extern synth standard_synth(tenv, typ);
// synth we get on error in expressions (type is wild)
extern synth wild_synth(tenv);
// synth we get for most statements -- standard_synth with void type
extern synth skip_synth(tenv te);
// synth we get upon return, continue, raise -- no fallthru or forward jump,
// no unassigned variables on either the normal or forward jump edge.
extern synth empty_synth(tenv te);
// synth we get upon a break, goto, or raise -- forward jump with no 
// fallthru, all currently unassigned variables on the forward jump edge, the
// empty set on the normal edge.
extern synth forward_jump_synth(tenv te);
// synth we get for sequencing.  We merge the normal and jump edges
// according to whether or not s1 may fallthru to s2.
extern synth seq_synth(synth s1, synth s2);
// synth we get upon join of two if statements or switch cases etc.
extern synth join_synth(synth s1, synth s2);
// after joining all of the switch cases' synths, we treat the 
// unassigned variables on the "forward jump" edge as unassigned
// on the outgoing normal control flow edge, because any case's break is
// a forward jump to the bottom of the switch statement.  
extern synth switch_bottom(synth s);
// after a loop, the synth is the join of the "false" part of the
// test expression, and the forward_jump part of the statement
extern synth loop_synth(synth e, synth s);
// an explicit fallthru -- similar to skip but ctrl indicates explicit fallthru
extern synth fallthru_synth(tenv te);
// add the set v to both edges of the synth
extern synth add_var_synth(Set<var> v, synth s);
// remove v from the fall-through edge of the synth (if any)
extern synth initialize_var_synth(synth s, var v);
// set of variables unassigned on fallthru edge
extern Set<var> maybe_unassigned(synth);
// set of variables unassigned on "true"/"false" branches respectively
extern $(Set<var>,Set<var>) maybe_unassigned_bool(synth);

extern Set<var> get_unassigned(tenv);
extern tenv     set_unassigned(tenv, Set<var>);
extern tenv     add_label(tenv, var, stmt);
extern bool     all_labels_resolved(tenv);
extern typ      return_typ(tenv);

extern tenv tc_init();
extern genv empty_genv();
extern fenv new_fenv(fndecl);

}
#endif
