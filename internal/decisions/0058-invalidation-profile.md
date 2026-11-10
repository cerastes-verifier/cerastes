# 0058 — `not_invalidating`, `--strict-owner`, marked fields (M4.5d2b2d)

- **Status:** draft (2026-11-10, M4.5d2b2d), written before code (invariant 6); to be revised after P77 and S42
  (`internal/il-inventory.md`, chapter M4.5d2b2d) and audit round 1. Owner decisions from M4.5d2b2c's planning,
  2026-11-10 (plan `.claude/plans/m4-5d2b2d-invalidation-profile.md`), each the recommendation:
  - **202:** `ref_to_uninit` on a pointer (parameter, result, variable) to a structure with an owning field is accepted,
    the marked site closing nothing (item 4). If the audit finds a hole, the rejections stay.
  - **203:** `not_invalidating` on a pointer to a `const` object is accepted.
  - In the session (2026-11-10): **218** "Allow, listed": N1's swap may change the Viper of the nine other existing
    programs whose bodiless contracts hold both asserts, outcomes unchanged (item 2, N1). **219** "Third flip" (it
    corrects 218's premise: the test-mutator's swap run found one outcome change outside the two flips):
    `owning_global_call_dangling_closure.c`, an untied bodiless contract with two sites, keeps one message per site
    (four `permission @ L27` become two, two expect lines turned into comments).
  - Standing: 175 (`owning` on a parameter stays a borrow), 177 (`--strict-owner`, an opt-in), 178
    (`cerastes::not_invalidating`, checked), 179 (no `no_dangling` in C), 192 (`ref_to_uninit` on a field here), 197 (the
    collision pins stay until this session revisits them), 198 (the split), 160 (a new test's expect line corrected
    after its first run, listed in status).
- **Milestone:** M4.5d2b2d.
- **Amends:** 0029 items 1 and 2; 0042 items 4 and 8; 0052 item 4; 0054 item 2; 0055 items 3 and 11.

## Context

**P4418R0's invalidation profile** (Stroustrup, "An invalidation profile", wording in P4417R0, both 2026-10-22) has three
parts that C can use and cerastes did not yet spell: `[[not_invalidating]]` on a pointer parameter (9.13.u, checked at
the definition, [profiles.inval.verify]); the owner rules p4 ("an allocation's or a call's owning result must
initialize or be assigned to an owner, or be deleted, in its full-expression") and p6 ("only an owner is deleted") of
[profiles.inval.owner]; and `std::no_dangling`, a suppression of the escape rules that C does not need (question 179).
Records 0029, 0047, 0052, 0054 and 0055 already give cerastes P4417's owners (`cerastes::owning` on variables, fields
and results, with path-precise `leak` and `invalid-free`), but two things are missing:

- **A borrower cannot say it leaves the structure alone.** A caller that holds `int *t = p->q;` across `touch(p)`,
  where `p` points to a structure whose owning field `q` holds a block, loses `t`: the closure of 0055 item 3 gives the
  block of `p->q` back, but nothing says it is the same block, so `*t` fails after the call (`lifetime`, S42). A bodiless
  borrower must write `p->q == old(p->q)` (0054 item 2's tie), per level of a chain.
- **P4222R3's `ref_to_uninit` stops at fields.** A structure whose owning buffer was allocated and not yet written
  cannot be handed to a filler (0055 item 3, "the closure is plain"), and `ref_to_uninit` on a pointer to a structure
  with an owning field is rejected (0055 item 11, question 197).

`--strict-init` (0042 item 8, 0052 item 4) is the model for an opt-in that only rejects more: P4417's p4 and p6 are
static rules whose dynamic counterparts (`leak`, `invalid-free`) cerastes already checks path-precisely in every mode.

## Decision

1. **Scope.** `cerastes::not_invalidating` (item 2), `--strict-owner` (item 3), `ref_to_uninit` on fields and on
   pointers to structures with owning fields (item 4), and the pins (item 5). Every other shape fails closed (item 8).
   C++'s parts (references, member functions, `std::no_dangling`) are M7's.

2. **`cerastes::not_invalidating`** (question 178; P4417R0 9.13.u and [profiles.inval.verify]).
   - **Registration:** `{"not_invalidating", "", "p", RAF_NO_FLAGS}`, as `must_init`; macro
     `CERASTES_NOT_INVALIDATING` in `<cerastes.h>` (a user definition is `annotation_macro_changed`). The registration,
     the name in `annotation_il.h`'s `kNames`, `marker_name` and `read_marker`'s branch land in one commit: P77 found that
     a registered name missing from `kNames` is classified `Annot::none` and silently ignored (on the probe build a
     marked parameter verified with no complaint). `marker_name` and `read_marker` were first made exhaustive, with an
     internal error for an unknown kind (`55bd461`, behaviour-neutral), since they filed any kind that is not `owning`
     or `must_init` as `ref_to_uninit`. VIR carries it as
     `vir::Param::not_invalidating`, separate from `Param::init`: it is not an initialization marker.
   - **Accepted** on a pointer parameter to an object type, in either position, a pointer to a `const` object included
     (question 203: 0042 item 4's `const` rejection is an initialization rule and does not apply), beside
     `cerastes::owning` on the same parameter (the common case: an owning borrow whose closures the marker ties).
     Declarations must agree (`annotation_mismatch`, naming the attribute and both declarations), read as 0042 item 4
     reads the markers (the union over declarations, told apart by position).
   - **Rejected** (`unsupported_annotation`, one pin each): a non-pointer parameter; `void *`; a pointer to a function
     (M4.7); the marker twice; beside `ref_to_uninit` or `must_init` on the same parameter, by an explicit check (a
     pointee that may hold no value has no pointers to keep; the implicit clause is built as VIR after the contract is
     read, so `reject_marked_read` would never see its `old`); a partnerless `ensures` atom or block site rooted at the
     marked parameter ("given", 0052 item 2, 0054 item 2: a body could never return a fresh block at a place whose
     pointer it must keep, so the rejection names the cause before Silicon would; the marker returns what it takes and
     gives nothing new through `p`); an atom on the marked parameter of a function declared not to return (it has no
     `ensures`, as 0042 item 4 says of `must_init`). On a function it is EDG's #1835 (pinned; a member function is
     M7's).
   - **Never ignored** (0042 item 4's contexts, the same rules): on a parameter of a `pure` or `predicate` function
     (M4.5f1) and on a user redeclaration of a function first declared in a system header it is rejected
     (`unsupported_annotation`); a declaration nothing calls is checked on its own (the rules of this item), as a marker
     must mean something wherever it is written; a typedef'd function type's marker counts for the functions it
     declares. A marker inside a function-pointer type's own parameter list reaches no `vir::Param`, as for the other
     markers (0042 item 4): a used pointer to a function is `unsupported_type` (M4.7), and an unused typedef carrying one
     is inert (it declares no function; listed in item 8). `is_param_marker`, which
     also drives vlibc's trust (0047 item 3) and three readers of function-level attributes that `not_invalidating`
     cannot reach (target `p` only), is unchanged; a second predicate counts it in the three callers that enforce these
     contexts (`first_param_marker`, the specification-body and system-redeclaration rejection, `carries_marker`). vlibc
     writes none, and a `not_invalidating` in a library file is rejected explicitly (counted by `library_markers` it
     would be trusted and read by nothing); no file under `tests/` reaches it, so a `LibraryFixtureTests` case of
     `scripts/test_run_tests.py` pins it with a fixture header in `scripts/testdata/library_fixture/include`.
   - **No atom rooted at the parameter:** accepted. The check below holds vacuously: the callee holds no permission
     through `p`, so it can neither free nor replace anything `p` reaches; a block it takes through another parameter
     that aliases `p` is accounted by that parameter's own atoms at the caller. A positive pins it.
   - **The check on the contract,** with or without a body, after `finish_atoms` (so closures of owning fields, 0055 item
     3, are counted), for each marked `p`:
     - every `requires` atom rooted at `p` (`atom_place(…).root`, any `via` and `path`) has an `ensures` atom whose
       partner it is (the atom is returned, not consumed);
     - every `requires` block site rooted at `p` has an `ensures` site on the same place key (0054 item 2), its
       partner;
     - otherwise `unsupported_annotation` at the parameter: "`p` is `cerastes::not_invalidating`, but the contract of
       `f` takes `owns_block(p->q)` without returning it";
     - **taken implies returned on every path.** The encoder checks only that a partner exists; lowering decides the
       rows. For each such pair whose guards are both absent, or the same null check (`atom_guard_shape`) over no heap
       read, the rule is syntactic. Any other pair gets a new `f__wf` row `prior ==> (g_req ==> g_ens)` (`permission`:
       "the contract of `f` takes `*p` on a path where it does not return it, but `p` is
       `cerastes::not_invalidating`"), asserted where the implicit clause below holds, so that a guard reading a loaded
       pointer sees it (two guards that read the heap are read in different states, so "the same expression" proves
       nothing for them). Today's row is the converse (returned implies taken).
   - **The implicit `ensures`,** built after that check: for each `ensures` atom or block site rooted at `p` whose
     place goes through a load (closures included), each loaded pointer `E` on its path satisfies `E == old(E)`, outermost
     first, under the guards of the atom that frames its load, **never under the site's own null test**: a closure
     `p->q == NULL || owns_block(p->q)` under `p == NULL || owns(*p)` gives `p == NULL || p->q == old(p->q)`, and a chain
     `a->b == old(a->b) && (a->b == NULL || a->b->x == old(a->b->x))` (the second level under the first's null test, its
     predecessor's). Tied under its own null test (`p->q != NULL ==> p->q == old(p->q)`, the draft's wording) the clause
     would hold vacuously for a body that frees `p->q` and nulls it, and the taken-implies-returned row could not be
     proved from it (edg-backend's survey). S42 compares both: under the draft's wording a callee that frees and nulls
     `p->q` verifies, the caller still loses `t`, and the new row is unprovable (`s42_2_A_*`, `s42_3a_A_*`,
     `s42_3b_A_*`, `s42_A_does_not_prove_taken_implies_returned_fail`); under this wording every shape holds, its twins
     fail at the tie (`s42_2_B_*`), and for a borrowed pair the two guard rows already force the same nullness on entry
     and return, so the unguarded tie asks nothing more (`s42_B_rows_force_nullness_ok`). **Placed as the closure is** (0055 item 3's
     `close_source`, which splices `atom && closure` into the atom's own clause, so the closure inherits the atom's
     guards by position and no guard is copied): each equality is spliced right after the `ensures` atom that frames
     `E`'s load (the permission atom with `E`'s cell's root and `via` whose path is a prefix of it), `atom && E ==
     old(E)`, built from `clone_place` and `old`, before that atom's closure. It is framed in the return state by that
     atom and in the entry state by its `requires` partner. S42: placed so, `f__wf`'s rows verify unchanged
     (`s42_3a_B_tie_ok`, `s42_3b_B_chain_ok`); as a later separate clause the tie fails (`s42_3a_later_clause_fail`);
     after the closure, a marked closure's `old(flag)` is ill-formed (`s42_4_tie_after_closure_wf_fail`).
     - **With a body** it is checked at each return with its clause as a `postcondition`. The clause that received an
       equality carries its own message: "the postcondition of `f` may not hold (with `p->q == old(p->q)`, which
       `cerastes::not_invalidating` on `p` adds)".
     - **Without one** it discharges 0054 item 2's tie for the sites rooted at `p`: the equality is in the site's own
       clause, so `f__wf`'s tie, asserted under the clauses before and the site's clause, holds by construction and
       stays as a backstop. A bodiless borrow of a structure with an owning field no longer writes `p->q ==
       old(p->q)`. A site rooted at another parameter or a global in the same contract keeps its tie.
     - It names pointers, never values: the marker promises nothing about `p->n`.
   - **What the caller gains:** a pointer taken before the call into what `p` reaches (`int *t = p->q;`) is still valid
     after it, because the block of `p->q` that comes back is the block `t` points into.
   - **N1** (d2b2b's audit): in `wf_method` the tie assert of a bodiless site moves above its partner-guard assert, so an
     untied bodiless borrow gets one message (the tie, which names the fix; Silicon then assumes the tie, from which
     the partner-guard row follows). The swap reorders the two asserts in every bodiless contract that has both (eleven
     existing programs; question 218); outcomes change only where both rows fail today: the two flips and
     `owning_global_call_dangling_closure.c` (question 219). Where the tie holds and only the partner-guard row fails
     (`owns_block_load_bodiless_pathpartner.c`, `owns_block_global_bodiless_pathpartner.c`) the error stays, so that row
     stays pinned. One VIR self-test case holds both rows (`owning_global_leaf_guarded_ensures`, verified): its two
     asserts swap and it still verifies, within question 218's terms (the orchestrator's first count missed it, and
     question 219 said no case was affected; status corrects that).

3. **`--strict-owner`** (question 177; P4417R0 [profiles.inval.owner] p4 and p6). The pattern is `--strict-init`'s,
   end to end: a driver option through `VerifyConfig`, a rule applied by the encoder to verified bodies, a diagnostic
   tag per rule, and the option table and its own section on the Verifying page.
   - **p4.** A call whose contract gives its result's block (an `owning` function, `malloc`, `calloc`, vlibc's
     allocations, a user function whose `ensures` has a block site at the result) must be: the initializer of, or the
     value assigned to, an owner (an `owning` local, global, `static` local or field, `p->q = make()` included, and an
     owning field's leaf in a struct initializer or compound literal, `struct S s = { .q = malloc(n) };`); the
     operand of a `return` in a function that gives its result's block; or the argument of a parameter whose contract
     takes the block (`free(make())`). Anything else is rejected (`owner_result_unowned`: "the result of `make` owns a
     block and must go to an owner (`--strict-owner`)").
   - **p6.** The argument of a parameter whose contract takes a block and does not return it (`free`, `vlibc_free`, a
     user's destructor) must be: an owner by name (`v`, `g`, `s.q`, `p->q`, `(*pp)->q`); a parameter of the enclosing
     function whose own contract takes its block (P4417's owner parameter, question 175); or a p4 call. Anything else
     is rejected (`owner_release_unowned`: "`free` releases `t`, which is not an owner (`--strict-owner`)").
   - **Two encoder-side facts are new:** "this function gives its result's block" (the `result_sites` test as a
     function) and "this parameter's contract consumes its block" (the encoder's twin of lowering's `block_consumes`).
     Heap routines leave `call()` early, so the rules are also applied in `heap_call` and `allocation_view`.
   - **How it is read.** The receiving context marks the value's node before it is encoded (casts peeled); a call
     whose contract gives its result's block is accepted only at a marked node. An `owning` (borrow) parameter is not
     an owner for p6, an address-taken owning local is. Messages end "(--strict-owner, decision 0058 item 3)", as
     `--strict-init`'s name their record.
   - **What the static rule cannot see** (as 0042 item 8 lists its own). **Accepted although wrong**, each still
     reported by the default mode's path-precise rows: an owner by name that is already empty or dangling
     (`invalid-free`); a release on some paths only (`if (c) free(v);`, `leak`); an assignment to an owner that still
     holds a block (`leak`); a member released through two aliases of its structure (`invalid-free`). **Rejected although
     harmless:** `t = v; free(t);`; `free(c ? a : a)` (with two owners, `free(c ? a : b)` leaks the other, which the
     default mode reports); `free(*pp)`, `pp == &v` included; a result stored in a non-owning local and released in the
     same function; a call under `?:` or a comma as the receiver's value (only casts are peeled); the owning leaf of a
     call's struct result (`free(mk().data)`). A by-value struct parameter is a named owner (`free(s.data)` is
     accepted), and `(void)make();` is rejected, rightly: the default mode reports its `leak`. Both lists are on the
     Verifying page, one test per entry.
   - Without the option every program means the same; the option only rejects more. Both strict options may be given
     together (`// verify-options: --strict-init --strict-owner`), and the harness allow-list holds both.

4. **Marked fields** (questions 192, 197, 202).
   - **Registration:** `ref_to_uninit` becomes `"r|v|p|d"`, in the commit that adds its reader in `struct_info` and
     `vir::StructMember::ref_to_uninit`. `must_init` on a field stays EDG's #1835 (pinned).
   - **Accepted** on a pointer field to an object type; **rejected** as `owning` on a field is (0055 item 2): a
     non-pointer, `void *`, a pointer to a function, a pointer to `const` (nothing could initialize it, 0042 item 4), the
     marker twice, a bit-field, a union member.
   - **Default mode, non-owning field:** the marker documents intent and changes no Viper (as 0052 item 4 for a local).
   - **Owning and marked field:** its closure clause is a marked block (0047 item 8's mode, as an owning and marked
     global's deep atom in 0054 item 2): a `requires` site claims no flags, a borrowed `ensures` site keeps them
     (`old(flag) ==> flag`, 0042 item 5's `internal` row), a given site claims none. This lifts 0055 item 3's "the
     closure is plain" for marked fields: a structure whose buffer is allocated and not yet written can be passed to a
     borrower, which cannot read the buffer before writing it (`uninitialized`). An unmarked owning field keeps the
     plain closure.
   - **`--strict-init`:** a store to a marked field follows 0052 item 4's rule for a marked local (it may receive a null
     pointer, a `ref_to_uninit` call's result, `&x` of an `uninit` local, or another marked local, parameter or field),
     and a marked field is accepted as the argument of a `ref_to_uninit` parameter. The same store to an unmarked field
     stays rejected (`reject_strict_init_malloc_member.c`, re-messaged). Every store form is covered: assignments
     through `assignment_parts`, and each marked leaf of a struct initializer or compound literal, checked per leaf
     before its element is evaluated (`init_element_value`'s callers); a global's static initializer gives a marked field
     a null pointer (accepted) or the address of an object of static storage, which always holds a value (rejected). A
     struct copy or a by-value pass moves a marked field into the same marked field of the same type, which the rule
     accepts (a sentence and a test).
   - **A marked pointer to a structure with an owning field** (question 202): accepted, and the marked site closes
     nothing on either side, as 0055 item 3 already says of `malloc`'s view. A callee that leaves a block in the field is
     reported by its own end row (`leak`); a caller whose structure held a block in the field before the call keeps it
     and its end row reports it if the callee overwrote the field; an initializer is `must_init`. The price (the sound
     direction): after such a call the caller does not know the field's value, so a field that may be non-null is
     reported at the caller's end even when the callee set it to `NULL` (`leak`), unless the contract says so (an
     `ensures` naming the field's value) or the callee is `must_init`. The three rejections
     (`reject_marked_owning_pointees` for a parameter and a result, `read_variable_markers` for a variable) go, after
     S42 and the audit. If the audit finds a hole, they stay and their pins are re-filed under this record.

5. **Pins:** `not_invalidating` on a function (EDG's #1835); `no_dangling` (an unknown name in the closed namespace:
   `unrecognized_attribute`); `must_init` on a field (#1835).

6. **The lint** (`lint_viper.py`): the new `f__wf` row; marked closure sites (their flag rows). **Categories:** none
   new. **Diagnostic tags:** two new (`owner_result_unowned`, `owner_release_unowned`), each named by a test.

7. **The `[[owner]]` table** of 0055 item 8 (and `memory.adoc`'s `[#p4417-owners]`) gains `[[not_invalidating]]` and the
   p4 and p6 rows.

8. **Still rejected** (fail closed): `not_invalidating` on a function or member function (M7); `no_dangling`; `must_init`
   on a field; `ref_to_uninit` on a field that is not a pointer to an object; `not_invalidating` beside an initialization
   marker, with a partnerless `ensures` site rooted at its parameter, on a function declared not to return, or in a
   library file; the contexts of item 2's "Never ignored". **Inert, not rejected:** a `not_invalidating` in an unused
   function-pointer typedef, as 0042 item 4 says of the other markers (the typedef declares no function; a pointer to a
   function that uses it is `unsupported_type` until M4.7).

9. **Facts relied on:** P77 (the IL of the new positions) and S42 (hand-written Viper for items 2 and 4).

10. **Tests and documentation.** Must-verify and must-fail pairs for every accepted form in both modes where valid; one
    pin per rejection; `annotations.adoc`, `verifying.adoc` (`[#strict-owner]`), `memory.adoc`, `error-messages.adoc`,
    the tutorial's heap chapter, `tests/README.md`, `internal/encoding.md`.

## Consequences

To be written when the record is accepted.
