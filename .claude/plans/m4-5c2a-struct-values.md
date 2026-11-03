# Plan: M4.5c2a — struct values and struct storage (record 0044); M4.5c2b split off

Approved by the owner on 2026-11-02 (planning in plan mode; questions 95–100 answered, each with the recommendation: 95 M4.5c2 split, values and storage first; 96 compound literals as C objects and C++ values; 97 by-value copies read every member; 98 the flips; 99 written struct atoms to M4.5c2b; 100 struct `?:`, comma and assignment values to M8).

Planned on 2026-11-02 in plan mode. The owner answered questions 95–100 (below), each with the recommendation.
- **Branch:** `m4.5c2a-struct-values`.
- **Worktree:** `.claude/worktrees/m4.5c2a`.
- **Baseline:** taken in the detached worktree `.claude/worktrees/m4.5c2a-base`, never in the main checkout.
- **Saved at step 1** as `.claude/plans/m4-5c2a-struct-values.md`.

## Context

**State.**
- `origin/main` is `c4957b8`, through M4.5b2b (PR #23 merged).
- M4.5c1 is pushed as `m4.5c1-struct-objects` (`cbdcce5`). Its PR #24 is open, with CI pending.
- The local `main` `eec198b` is a local merge of it.
- The worktrees `.claude/worktrees/m4.5b2b-base`, `m4.5c1` and `m4.5c1-base` stay; removing them is the owner's step.
- The EDG pin is unchanged (`f0ed5525af`), and this session needs no fork work.

**What is rejected today** (record 0043 items 8 and 15), each naming M4.5c2:
- **By-value struct parameters and results:** `vtype`, `encode.cpp:511-523`; backstops `check.cpp:336-346`, `:651`.
- **A struct used as an rvalue:** `:1836-1840`.
- **`f().x` and members of temporaries:** `:2874-2879`, `:3724-3732`.
- **Compound literals:**
  - as values, `:2640-2642`;
  - their addresses, `:3781-3784`;
  - `S s = (S){…}`, `:4563-4581`.
- **C++ temporaries `S s = S{…}` and `S()`:** `:4590-4593`.
- **Struct globals:** `:1969-1977`.
- **`static` struct locals:** `:2299-2305`.
- **`g.x`:** `place_address`, `:3649-3658`.
- **A function's `decreases(p->x)`:** `:3704-3707`. Lowering already reads member paths in measures through `contract_read`.

**Size.** All of M4.5c2 is about 3,500 changed source lines, more than M4.5c1's 3,200, so it is split (question 95):
- **M4.5c2a** (this session, about 2,000 lines and about 100 tests): struct values and struct storage.
- **M4.5c2b** (next): places reached through a heap read, written struct atoms in loop invariants, and the `restrict` re-check.

**What M4.5c2a lands:**
- by-value struct parameters and results (a struct value as the tuple of its leaves);
- `f().x`, `S s = f();`, `t = f();`, `g(f())`, `return f();`;
- C++ `S{…}` and `S()` as values;
- compound literals: values in both modes, and `&(T){…}` in C as an unnamed object of its block;
- struct globals and `static` struct locals;
- a function's `decreases(p->x)` and `decreases(g.x)`;
- `restrict` closed beside by-value structs with pointer leaves;
- the ports of VeriFast's `tests/call_in_struct_init.c` and `examples/struct_assignment.c`.

## Owner decisions (2026-11-02, all the recommendations)

- **95, the split:** c2a is parts (1) values and (2) storage, as listed above. c2b is part (3):
  - `owns(**pp)`, `reads(**pp)`, `live(*pp)`;
  - reads of `**pp` and `*gp`, `owns(*gp)`;
  - `p->q->x` and `*s->q`;
  - an atom on `*q` where `q` is a cell;
  - atoms and reads through a pointer member of a by-value parameter (`owns(*s.q)`, `*s.q`, `*r.q`);
  - the `restrict` re-check of handoff §3.

  M4.5d2 then needs c2b, because owning fields point through a heap read.
- **96, compound literals:**
  - **In C:** values, and `&(T){…}` as a hidden local of the enclosing block. It is allocated at block entry, stored where it is evaluated (each evaluation stores it again), and its scope ends with the block (C11 6.5.2.5p5, 6.2.4p6).
  - **In C++:** values only; `&(S){…}` is rejected.
  - **Rejected in both modes:** C23 storage-class literals and file-scope literals.
- **97, by-value copies:** passing a struct by value and `return s;` are copies (question 92).
  - Every member is read: its permission, its flag, and for a pointer member its liveness.
  - So a missing member is `uninitialized` at the call or at the return.
  - No flag formals are added.
- **98, flips:** pre-approved as listed under Flips, with 99's change. Nothing else may change.
- **99, written struct atoms in loop invariants:** they move to **M4.5c2b**, beside atoms on cell pointers, so the written-atom functions are rewritten once. `reject_struct_atom_in_invariant.c` is re-messaged naming M4.5c2b, with its `expect:` line kept.
- **100, struct `?:`, comma and assignment values:** examples are `t = c ? s : u;`, `t = (f(), s);` and `(t = f()).x`. They stay rejected (`unsupported_expression`), naming **M8**, and are listed as over-strict, like c1's copy used as a value.

**Standing:**
- 77: a session fits one context.
- 69: the EDG merge waits for the box session.
- 78: `const` objects are M4.6's, so a `const struct S s` parameter stays rejected.
- 80: `static` locals take implicit atoms.
- 84: initial values follow linkage.
- 90–94: record 0043.

## Step 1 (first, before any code): consolidate the plan documents

The first commit is markdown only.

1. **Worktree:** `git worktree add .claude/worktrees/m4.5c2a -b m4.5c2a-struct-values m4.5c1-struct-objects`.
   - It branches from `cbdcce5`, c1's pushed tip, so its PR stacks on PR #24.
   - Clone the submodules with `--reference` to the main checkout's stores (memory `background-worktree-setup`).
   - Then run `scripts/bootstrap.sh`.
2. **Save this plan** as `.claude/plans/m4-5c2a-struct-values.md`, with an "Approved" line and the Inputs below.
3. **`internal/handoff.md`.** The header gets one pointer: `git show cbdcce5:internal/handoff.md`.
   - **§1–2 owner steps:**
     1. Merge PR #24 after CI.
     2. Fast-forward `main`.
     3. Remove `.claude/worktrees/m4.5b2b-base`, `m4.5c1` and `m4.5c1-base`.
     4. At the end: c2a's PR.
   - **§3:**
     - The "M4.5c2" notes split. c2a's notes go into this plan's Inputs.
     - c2b keeps: the heap-read places; the note "a `T**` callee cannot use `*pp`'s pointer"; the `restrict` re-check; `p->q->x`; written struct atoms (question 99); `owns(*s.q)`.
     - M8 gains question 100's shapes.
   - **§4:**
     - Record 0044 is this session's, and 0045 the next free one.
     - §4.2: the c2 paragraph becomes c2a (a pointer to this plan) and c2b (its scope).
     - d2 needs c2b.
   - **§8:** questions 95–100 are answered; none is open.
4. **`plan.md`:**
   - the M4.5 table's c2 row becomes two rows, c2a and c2b;
   - d2's needs become "M4.5c2b, M4.5d1";
   - f1's needs become "M4.5c2b, M4.5d2";
   - the header's "the last" becomes M4.5c2a.
5. **Commit 1 (planning):** local, with no Claude trailer.

## Inputs from the handoff (moved into the plan at step 1)

- **§3 "M4.5c2" [c1], consumed here:**
  - by-value parameters and results, and `f().x`;
  - struct globals and `static` struct locals, with `program_start` rows per leaf. A struct-pointer global already has its `WF_S` row (audit round 2 S2).
  - compound literals, and `S s = S{…}` / `S()`;
  - `decreases(p->x)`.
- **The lint gap** "a `Perm` local as a non-first target of a multi-target call is not seen" (standing, b1c1). It is closed here, since multi-target calls now appear.
- **The d2 note** "`alloc_<t>` does not ensure `block_of(r) != GB_g()`". It is the same family as hazard H8 below: a completeness gap, never an assumed fact.
- **The standing one-type shortcuts of `overlap_pairs` [c1 audit 2 N2]** apply to struct globals as roots too.
- **Process rules (§6):**
  - test generators live in `build/m4.5c2a/` and are never rerun after a hand edit;
  - twins whose failed read flows into an `ensures` get an `ensures` that holds for any value;
  - single-file runs while working;
  - builds serialized: `lockf build/m4.5c2a/build.lock cmake --build build/cmake`.

## Setup (in parallel with step 1)

- **Baseline:** in `.claude/worktrees/m4.5c2a-base`, detached at `cbdcce5`, in the background:
  1. `bootstrap.sh`;
  2. `run_tests.py -j 8`;
  3. the VIR self-test;
  4. copy the results into the session worktree's `build/m4.5c2a-baseline-{vpr,vir}`.
- **Probes run against the session worktree's first build** (before any code change), which equals the baseline.

## Record 0044 — struct values and struct storage

The record is written before code (invariant 6), while P67, P68, S34 and the P69 probe run. Audit round 1 runs on it in
the background.

1. **Scope table:** c2a | c2b | d2 | e1 | M4.6 | M7 | M8.

   **Amends:**
   - 0043 items 1, 5–9 and 15;
   - 0041 items 2, 4, 5 and 7 (struct globals and `static` struct locals);
   - 0037 items 2 (`restrict`) and 3 (result splits per pointer leaf);
   - 0039 items 2–4 (struct-valued calls and stores);
   - 0012 item 2 (a struct parameter's entry value).

2. **Struct values in VIR.** A struct value is a tuple that exists only between a few producers and consumers. Lowering
   expands it into its leaves, in `struct_leaves` order, depth first. Struct objects keep c1's encoding.
   - **New VIR expressions:**
     - `Field(a, k)`: member `k` of a struct value. Fields chain like `MemberAddr`.
     - `Aggregate(type, args)`: one argument per leaf.
   - **Producers:**
     - `Call`;
     - `Aggregate`;
     - a struct-typed `Field`;
     - `Formal` and `Result`, in contracts only and only under a `Field`. A `Formal` may also sit inside `Old`.
   - **Consumers:**
     - a call argument;
     - `Return`;
     - a struct `Store` whose value is a `Call` or a `Field` chain over a `Call`;
     - an `ExprStmt` of a `Call`;
     - `Field`.
   - `check.cpp` enforces exactly this list at `:336-346, :651, :675-676, :689, :860-861, :898-899, :1168`. Every other
     position keeps its error.
   - A lowering guard makes any struct `LV` that reaches another consumer an internal error (exit 3).

3. **By-value parameters.**
   - **Formals:** each leaf is a formal named with the `_m<k>` suffix per path element (`p_s__0_m0`, `p_s__0_m1_m0`, as
     `ghost_name` already names leaves). The leaves take the parameter's place, in today's order, before the ghost amounts.
   - **`requires`:** each integer leaf's range, and `WF` or `WF_S` for each pointer leaf. `f__wf` takes the same formals.
   - **Callee entry:** the parameter's copy is a struct block, like every struct local (0043 item 4).
     - It is allocated by `alloc_struct_<n>_<tag>()`.
     - Each leaf is written from its formal through a `Ref` temporary, with its flag set (`lint_flags` unchanged).
     - It is exited at every return and at the fall-through (`param_cells`).
   - **Contracts:**
     - `s.x` is the formal, which holds the entry value (0012 item 2), in `requires`, `ensures` and `decreases`.
     - `old(s.x)` is the same value.
     - A whole struct (`old(s)`, `s` as an operand) is rejected.
   - **Still rejected:**
     - `const struct S s` (a `const` object, M4.6);
     - markers and `owning` on a by-value struct (`unsupported_annotation`);
     - `*s.q`, `owns(*s.q)`, `s.q->x` (M4.5c2b).

4. **By-value results.**
   - **Results:** the method returns one result per leaf (`res_m0`, …), each with its range or `WF_S` as an `ensures`.
   - **`return`:**
     - `return s;` reads every leaf of `s` (question 97: permission, flag, and a pointer leaf's liveness). The leaves are
       assigned to the results before the scope exits and the postconditions run.
     - `return f();`, `return (S){…};` and C++'s `return S{…};` / `return {…};` assign the value's leaves.
   - **Contracts:** `r.x`, `r.t.y` and `r.q` read result leaves in an `ensures`.
   - **Still rejected:**
     - `old(r.x)` (the result has no entry value);
     - atoms on result members (M4.5d2);
     - `*r.q` (M4.5c2b).
   - **Callers:** a call of a struct-returning function is a **multi-target call** `t_m0, t_m1 := f(…)`.
     - `CallItem`, `LStmt` and `LMethod` gain targets; `target` stays for one result.
     - `emit.cpp:148` prints the list.
     - `result_splits` (0037 item 3) runs per pointer leaf.

5. **Calls with struct values.**
   - **Arguments:** a struct argument passes its leaves.
     - A source object (`s`, `*p`, `p->t`, `s.t`, `g`) is read leaf by leaf, as a copy source (0043 item 7, question 97).
     - The base is checked for null once.
     - A base with side effects or calls is rejected: it would be evaluated once per leaf.
   - **Results used:** `S s = f();`, `t = f();`, `*p = f();`, `g(f())`, `f().x`, `f().t.y`, and a discarded `f();`.
   - **Stores:** a struct `Store` writes every destination leaf, after the value's temporaries exist.
     - The base's null check comes first.
     - Each leaf gets its permission check at `write`, with `WF` for a pointer leaf.
     - Its flag is set.
   - **Sequencing:**
     - a struct call is one fixed point with N temporaries (0021, 0039);
     - `Field` inherits its dependencies;
     - the leaf reads of an `Aggregate` are unsequenced siblings, keyed per leaf;
     - a struct `Store` writes every destination leaf's key;
     - `writes_in` (loop havoc, `lower.cpp:4372`) counts a struct `Store` as a write of every leaf field.
   - **Rejected:**
     - **M8** (question 100), each listed as over-strict:
       - struct `?:`, comma and assignment values;
       - a struct store in a `for` step or in a `?:` arm;
     - whole-struct equality in a contract: C has no `==` on structures, and VeriFast's `result == (struct foo){…}`
       becomes `r.x == … && r.y == …`.

6. **C++ temporaries.**
   - `S s = S{…}` and `S s = S()` are the braced list and `{}`. P65 shows them as an aggregate with
     `explicit_cast_applied`.
   - `S{…}` and `S()` are also values wherever item 5 takes one.
   - `S s(1, 2)` stays M7.
   - **C's assignment from a literal.** In `t = (S){…}` and `t = S{…}` (C23 6.5.16p3), an element's side effects are
     unsequenced with the store. In C, an element with a side effect therefore needs a destination that no element
     names and whose address is not taken; otherwise the assignment is rejected like the compilers (P68 decides the
     shape).

7. **Compound literals** (question 96).
   - **As values:** an `Aggregate` in both modes, built by `collect_init_elems` and zero leaves, with c1's C rule on
     elements with effects. `(int){5}` used as a value is its converted element.
   - **`&(T){…}` and `&(T){…}.m` in C:** a hidden address-taken local `cl_<n>`.
     - Its `Decl` without an initializer is emitted before the enclosing statement, in C's block for it. C23 6.8.4p3 and
       6.8.5p5: an `if`/`switch` condition is wrapped as `{Decl; If}`, and a non-compound substatement uses
       `cell_block`.
     - So it is allocated at block entry, may be jumped over (0019), and gets its scope exits (0038 item 7).
     - The expression becomes `Comma(StoreE(leaf_i, e_i)…, AddrOf(cl))`, zero leaves included.
     - Each hidden object is initialized at most once per lifetime and is never read before.
   - **Rejected:**
     - a literal in a loop condition, a `for` step or a `do` condition: it is evaluated again without re-entering its
       block (M8, listed);
     - `return &(T){…}` (R14, `return_local_address`);
     - `&(S){…}` in C++;
     - C23 storage-class literals;
     - file-scope literals;
     - array literals (M4.5e1).
   - **Hazard H3:** a scope wider than C's would accept dangling uses. Two `lifetime` twins pin it: a use after the `if`,
     and a use after the loop iteration.

8. **Struct globals** (amends 0041).
   - **Layout:** a struct global's block `GB_g()` holds the slot `loc(GB_g(), 0)` with its member blocks (0043 item 3).
   - **`vir::Global`:** gains one `GlobalLeaf` per leaf, `{Value | Null | Addr, value, init, target}`.
   - **The global facts** (every method's first `requires`, and `program_start`'s first `ensures`) gain, for each struct
     global, `struct_lens(loc(GB_g(), 0))`: the member lengths and slot facts. The prelude has no `len >= 0`, so
     `WF(&g.x)` needs them (0041 item 4's argument). They appear only in units with a struct global, so no existing
     program changes.
   - **`program_start`:** one row per struct global.
     - For every leaf: `acc(L.v_T) && acc(L.d_T) && L.d_T && <value>`, where the value is a literal, `null`,
       `loc(GB_h(), 0)`, or for an unknown value its range, `WF` or `WF_S`.
     - Then exactly one `acc(lvloc(GB_g()).lv)`.
   - **Initial values:** by linkage (question 84), per leaf. Initializers:
     - are read by `collect_init_elems`, constant elements only, with missing leaves zero or null;
     - are checked per leaf in the prologue (`named_const_check`, named `g.x`);
     - a pointer leaf may be null or `&h` (with 0041's escape rule);
     - `&h.x` stays rejected (M8, listed);
     - a C++ dynamic initializer is rejected, as 0041 rejects it.
   - **Code:**
     - `place_address` roots a global at `GlobalAddr`;
     - copies to and from `g`, `h(g)` and `g = f()` work through item 5;
     - messages and `LoopWrites` use `member_root`;
     - a held global (in `main`) gives its leaves to `loop_places`/`key_cells`;
     - `main`'s case splits use `split_candidates`.
   - **Contracts:** `owns(g)`, `reads(g)`, `owns(g.t)`, `owns(g.x)`, `g.x`, `old(g.x)` and `decreases(g.x)`, with 0043
     item 9's overlap rule per root.
   - **Hazard H8:** the axioms cannot prove `member(…GB_g…) != GB_h()`, and no rule assumes it (the "two globals"
     shortcuts only skip checks). If S34 (d) shows Silicon cannot separate such cells from its chunks, the record adds
     `parent_of(GB_h()) == null` to the facts, only in units with a struct global. Otherwise it records the gap as a
     completeness note.
9. **`static` struct locals.** The rejection is lifted. The implicit `owns` of question 80 (`add_static_local_atoms`)
   becomes a struct atom that expands per leaf. Its initializer runs once, before `main`, as item 8.
10. **Function measures.** `decreases(p->x)` (a member leaf of a pointer parameter, through 0039 item 8's machinery,
    which `contract_read` already makes path-aware), `decreases(g.x)` and `decreases(s.x)` (a plain formal). The termination
    and framing rows are unchanged.
11. **`restrict`** (amends 0037 item 2 and 0043 item 10). A by-value struct parameter whose leaves can hold a pointer
    from which a `T*` can be formed is now one of the places beside which `restrict T*` is rejected. These are an `int *`
    leaf, or a `struct T *` leaf whose `T` has an `int` leaf; the test is `may_hold_pointer_to` on the struct itself. The
    check also re-runs after the implicit atoms of `static` locals are added (they are added after today's check).
12. **The invariant heap-read gap (P69).**
    - **Today:** only `**pp` is rejected in a loop invariant or measure, and only by its syntax (`encode.cpp:3827`).
      `p->q->x`, `*p->q` and `*gp` reach `inv_deref` chains unpinned, and c2a makes `*s.q` reachable too.
    - **The probe:** for each of these shapes, in an invariant and in a measure, a positive, a value twin
      (`invariant-preserved` or `check`) and an unheld-target twin (`permission`).
    - **If every verdict is right and nothing exits 3:** they are accepted and pinned, extending 0040 item 5's N2
      argument (a read through a loaded pointer is framed by the automatic places).
    - **Otherwise:** they are rejected by a semantic rule ("a dereference whose address passes through a load"), naming
      M4.5c2b.
    - A wrong verdict today is an existing soundness hole: stop and ask.
13. **Lint** (`lint_viper.py`).
    - **`program_start`'s struct row** is pinned like `struct_allocation_problem`:
      - canonical chains rooted at `loc(GB_g(), 0)`;
      - each leaf once, and no leaf a prefix of another chain;
      - exactly one `lv`;
      - the value grammar per leaf;
      - an `Addr` to a struct global only from a `ptr_struct_*` leaf.
    - **The facts row** is exactly the scalar facts, plus the lengths of the row's member blocks, plus its slot pairs.
    - **`lint_perm_locals`** sees every target of a multi-target call.
14. **Categories:** none new. Messages name leaves in source terms:
    - "passes `s`, whose member `s.y` may not have a value";
    - "returns `s`, whose member …";
    - `r.x`;
    - `g.t.y`.
15. **Still rejected,** each pinned:
    - **M4.5c2b:**
      - the heap-read places;
      - written struct atoms in invariants (question 99);
      - `*s.q`, `owns(*s.q)`, `s.q->x`, `*r.q`.
    - **M8**, listed as over-strict:
      - struct `?:`, comma and assignment values (question 100);
      - struct stores in a `for` step or a `?:` arm;
      - literals in loop conditions;
      - `&h.x` as a global initializer.
    - **M4.5d2:** atoms on result members.
    - **M4.6:** `const` by-value structs.
    - **M7:** `S s(1, 2)`.
    - **Item 7's literal rejections.**
16. **Facts relied on:** P67, P68, P69, S34.
17. **Tests and documentation** (below).
18. **Flips** (below).

## Team (disjoint files; three Sonnet agents and one Opus reviewer from `.claude/agents/`)

| Agent | Owns |
|---|---|
| **il-analyst** (Sonnet) | **First, P67 (both modes):**<br>- a by-value parameter's `s.x` in body and contracts, and `cerastes_old(s.x)`;<br>- the arguments `g(s)`, `g(*p)`, `g(f())`, `g((S){…})`, C++ `g(S{…})` (temp-init or constructor wrappers);<br>- `f().x`, `f().t.y`, `S s = f()`, `t = f()`, `return f()`, `return s`, `return {1, 2}`, `f();`, `(void)f()`, and when `return_dynamic_init` appears;<br>- compound literals: `enk_temp_init` fields, C++ `(S){…}`/`&(S){…}`, C23 storage-class literals, file scope;<br>- global and `static` initializers: `ck_address` leaves, `{0}`, designators, `&g.x`, C++ `S g = S{…}`, dynamic initialization, `initk_function_local`.<br>**Then P69**, the invariant heap-read gap, end to end against the first build.<br>**Then S34**, test-mutator's fixtures through Silicon with vacuity runs.<br>**Then:** `src/vir/lower.cpp`, `src/vir/lowered.h` and `src/viper/` (items 2–5, 7–10: leaf formals and results, `Env`/`LV` leaves, entry copies, multi-target calls and emission, struct `Store`, `ret()`, `program_start` and facts, loop places of struct globals, `LoopWrites`, splits), and the `.vpr`/`.map` comparisons against the baseline. |
| **edg-backend** (Sonnet) | **First:** `vir.h`, `vir.cpp`, `check.cpp`:<br>- `Field`, `Aggregate`, struct `Param`/`ret`, `GlobalLeaf`;<br>- the consumer and producer positions of item 2;<br>- `restrict_hazard` over by-value structs;<br>- committed with no behaviour change, baseline compared.<br>**Then:** `encode.cpp`:<br>- `vtype`'s parameter/result flag and `struct_value()`;<br>- arguments, returns, `copy_struct`'s call and literal sources, `f().x`;<br>- C++ temporaries, compound literals and hidden locals;<br>- global and `static` initializers per leaf, `place_address` for globals;<br>- contract places on struct globals, by-value fields and result fields, `decreases` of members;<br>- the `restrict` re-run;<br>- the rejections of item 15 and the re-messaged c2b pins;<br>- `diagnostics.def`. |
| **test-mutator** (Sonnet) | **First:** S34's fixtures in `tests/viper-smoke/memory/s34_*`, each with a `_fail` and a `vacuity_fail` variant:<br>- (a) a multi-result method and `t1, t2 := f()` with `WF_S` and a pointer-leaf split;<br>- (b) the by-value entry copy, `return s`, and an `ensures` over the entry formal after the body changes `s.x`;<br>- (c) struct global facts and row, `owns(g)`, `&g.x` to a `WF`-only callee from a function holding nothing of `g`;<br>- (d) a scalar and a struct global in `main`'s loop (H8);<br>- (e) a `static` struct local through a recursive caller;<br>- (f) a hidden literal: allocation, `StoreE` inside an argument, the scope exit per iteration.<br>**And P68**, the compiler matrix (clang 21, GCC 16, both modes, three tiers):<br>- sequencing of struct arguments and struct-valued calls;<br>- `t = (S){…, x++}` shapes;<br>- `-Wmissing-field-initializers` on literals and globals;<br>- C++ compound-literal warnings and errors;<br>- `return &(int){5}`.<br>**Then:**<br>- the tests and twins below, written from the record before the code runs;<br>- the two ports;<br>- the flips;<br>- the VIR self-test `selftest_cases15.inc` with `VIR_CASES_MIN`;<br>- `lint_viper.py` (item 13) with its self-tests;<br>- the doc example files. |
| **soundness-auditor** (Opus, read-only) | **Round 1:** record 0044.<br>**Round 2:** the code and the tests, especially:<br>- leaf formals and results with their ranges and `WF_S`;<br>- the entry copy and `return s`;<br>- multi-target calls and per-leaf splits;<br>- a struct `Store`'s order;<br>- `program_start`'s rows and the facts lint (vacuity);<br>- H8;<br>- compound-literal scoping (H3);<br>- the `restrict` rule;<br>- P69's outcome.<br>**One re-audit** of the blocking fixes. Should-fix items not taken go to handoff §3. |
| **Orchestrator** | - step 1;<br>- record 0044 and its amendments;<br>- `doc/`;<br>- `internal/encoding.md` (an M4.5c2a paragraph after the c1 paragraph near `:462`, and §8's lists);<br>- `il-inventory.md` (P67, P68, P69, S34);<br>- integration, serialized builds, the baseline;<br>- status and handoff. |

Any question from an agent, or of mine, stops that work and goes to the owner.

## Tests (`tests/memory/`; `.c` in both modes unless marked; each positive has a twin failing for one reason at one line)

| Positive | Twin (expected) |
|---|---|
| `int get(struct S s)` with `requires(s.x > 0)`, `ensures(r, r == s.x)` | a wrong `ensures` → `postcondition`; the caller passes `s` with `s.y` unset → `uninitialized` at the call (question 97) |
| the callee changes `s.x`; `ensures(r, r == s.x + 1)` reads the entry value | `ensures(r, r == s.x)` → `postcondition` |
| `struct S make(int a)` returning `{a, 2}`, `ensures(r, r.x == a && r.y == 2)`; `struct S s = make(1); check(s.x == 1);` | `check(s.y == 1)` → `check`; `return s;` with `s.y` unset → `uninitialized` at the return |
| `f().x`, `g(f())`, `t = f();`, `*p = f();` with `owns(*p)`, `f();` discarded | a wrong value → `check`; `*p = f()` without an atom → `permission` |
| a nested result `r.t.y` | a wrong `ensures` → `postcondition` |
| a result with a pointer leaf, `ensures(r, r.p == q)`, and the caller dereferences it with `owns(*q)` | the caller dereferences with no relation → `permission` |
| passing `*p` by value with `reads(*p)` | without an atom → `permission`; a dangling pointer member → `lifetime` at the call |
| port `verifast/call_in_struct_init.c` (`result.x` → `r.x`) | `check(p.y == 1)` → `check` |
| port `verifast/struct_assignment.c` `test1`–`test6` (copies; a by-value parameter `requires(f.x < f.y)`; literal arguments and returns; `result == (struct foo){40, 50}` → `r.x == 40 && r.y == 50`; `test6`'s out-of-order designators reordered, noted in the header; predicates left out, M4.5f1) | `test4((struct foo){30, 20})` → `precondition` |
| C++ `S s = S{1, 2}`, `S s = S()`, `g(S{1, 2})`, `return S{};` | a wrong value → `check` |
| literal values `t = (struct S){1, 2};`, `g((struct S){.y = 2})`, `return (struct S){…};` | a wrong value → `check` |
| C: `int *p = &(int){5}; *p += 1; check(*p == 6);`, `f(&(struct S){1, 2})` with `owns(*p)`, a literal in a loop body and in an `if` condition | a use after the `if` → `lifetime`; a use after the iteration → `lifetime` (H3) |
| a `static` struct global with `owns(origin)`, `origin.x += 1`, `main` checks the zero start | touching `origin.y` without an atom → `permission`; C++ external linkage → `check` (unknown start) |
| a struct global `{1, &h}` (C, both linkages) | a wrong initial value → `check` |
| `owns(g.x)` only; `old(g.x)`; recursion `decreases(g.x)` | writing `g.y` → `permission`; a measure that does not decrease → `termination` |
| a `static` struct local counter | `next() + next()` → `unsequenced` |
| a struct global held in `main`'s loop with an invariant | a missing clause → `invariant-preserved` or `check` |
| `decreases(p->x)` recursion with `owns(*p)` | not decreasing → `termination`; unframed → `permission` |
| `f(int *restrict p, struct T t)` with `T` scalar-only | rejected: `f(int *restrict p, struct S s)` with an `int *` leaf |
| sequencing: `get(s) + (s.y = 1)` (P68 decides), `h(f(), f())` with `f` touching a global | `unsequenced` (or rejected like the compilers) |
| P69's shapes, if accepted: `p->q->x`, `*p->q`, `*gp` in an invariant and a measure | value → `invariant-preserved`/`check`; unheld → `permission` |
| — | **rejected:** struct `?:`/comma/assignment values (M8); `old(s)`; `old(r.x)`; `*s.q`/`owns(*s.q)` (c2b); an atom on `r.x` (d2); `const struct S s` (M4.6); a literal in a loop condition and a `for` step (M8); `(static int){5}`; a file-scope literal; C++ `&(S){…}` (two new C++ pins for the C-only flips); `return &(int){5}` (R14); `&h.x` as a global initializer; a C++ struct global with a dynamic initializer; a whole-struct comparison in a contract; a base with side effects passed by value |

## Flips (questions 98 and 99, pre-approved; nothing else may change)

- **Retired for positives of the same shape** (each with a new verified test in its place):
  - `reject_struct_by_value_param.c`, `reject_struct_by_value_result.c`;
  - `reject_struct_temporary_init.cpp`, `reject_struct_init_compound_literal.cpp`;
  - `reject_struct_static_local.c`, `reject_struct_decreases_member.c`;
  - `global_struct.c`, whose only `expect:` line (`@ L13`) goes;
  - `reject_struct_compound_literal.c` and `reject_compound_literal_address.c`. These run in both modes and their
    positives are C-only, so each also gets a new C++ rejection pin.
- **Re-messaged naming M4.5c2b,** with every `expect:` line kept and their `Pins:` comments updated:
  - `reject_owns_double_deref.c`, `reject_read_double_deref.c`, `reject_read_double_deref_invariant.c`;
  - `reject_atom_on_cell_pointer.c`, `global_pointer_global_deref_atom.c`;
  - `reject_struct_member_chain_contract.c`;
  - `reject_struct_atom_in_invariant.c` (question 99).

**Viper against the baseline:**
- every existing `.vpr` and `.map` is byte-identical, and the rest are added files;
- no program accepted today has these constructs, and the struct global facts appear only in units with a struct global;
- the VIR self-test shows only added cases.

## Docs (invariant 8; every example is a test)

- **Memory and ownership:**
  - "Structures": passing and returning by value (the members read, as for copies), `f().x`, compound literals and
    their lifetime, C++ temporaries;
  - "Global variables" and "`static` local variables": structures, and initial values per member;
  - the not-yet list (`memory.adoc:588-617`) loses c2a's items and keeps c2b's and M8's.
- **Annotations:** `s.x` of a by-value parameter, `r.x` of a struct result, `decreases(p->x)`, `g.x` places.
- **Error messages:** `uninitialized` at a call or return naming the member; `lifetime` of a compound literal.
- **Verifying a program:** the rejected list (`verifying.adoc:~852-878`).
- **`internal/encoding.md`:** the M4.5c2a paragraph.

## Order of work

1. **Step 1, the worktree and the baseline.** In parallel:
   - the record draft, with audit round 1 in the background;
   - P67 and P69 (il-analyst);
   - the S34 fixtures and P68 (test-mutator);
   - the VIR block (edg-backend).
2. **S34 through Silicon; the VIR changes.** No behaviour change; the VIR self-test and a sample regeneration are
   byte-identical. **Commit 2.**
3. **By-value parameters and results, and the measures (items 2–6, 10, 11):** lowering, encoder, tests, the
   multi-target lint, cases15 part 1. **Commit 3.**
4. **Struct globals and `static` struct locals (items 8–9):** lowering, encoder, the `program_start`/facts lint and its
   self-tests, tests. **Commit 4.**
5. **Compound literals (item 7) and P69's outcome (item 12).** **Commit 5.**
6. **Audit round 2.** First the full regeneration against the baseline, then audit round 2, the fixes and one re-audit.
7. **Close-out:** docs, `encoding.md`, `il-inventory.md`; `check.sh` (full, at most twice); status and handoff; the
   last commit.

**The pre-agreed cut**, if context runs short after commit 4: the hidden-local `&(T){…}` moves to c2b, and literals stay
values only. It changes two flips into re-messages, so ask the owner first. The `restrict` fix is never cut.

## Stop and ask

- Any question from an agent.
- An existing test's outcome changing outside the flips, or any existing `.vpr`/`.map` changing.
- P67: a by-value argument's leaves not visible in the IL, a C++ temporary shown as a constructor call, or a literal
  whose elements are lost.
- S34: a vacuity run that passes, Silicon failing first with its own reason (exit 3), or H8 needing the facts.
- P69: a wrong verdict today (an existing soundness hole).
- Exit 3 on a correct program, or a new test over 60 s.
- Context running short after commit 4 (the cut above).

## Acceptance (commands run at the end)

- `python3 scripts/run_tests.py tests/memory tests/scalar tests/viper-smoke/memory doc/modules/ROOT/examples/memory
  doc/modules/ROOT/examples/error-messages doc/modules/ROOT/examples/verifying doc/modules/ROOT/examples/annotations`:
  all pass.
- **Against the baseline:** every `.vpr`/`.map` in `build/vpr` that exists in `build/m4.5c2a-baseline-vpr` is
  byte-identical, and the VIR self-test shows only added cases.
- `python3 scripts/test_run_tests.py`, `python3 scripts/test_lint_viper.py` and `python3 scripts/run_tests.py
  --check-headers`: green.
- `scripts/build_docs.sh` and `python3 scripts/check_doc_examples.py --html build/doc`: green.
- `scripts/check.sh` (full, `CERASTES_CHECK_BASE` unset): green. There is no Linux run in the session; CI covers it, and
  any x86-64 run goes to `matt@clang4.cpp.al`.
- **Status entry:** record 0044, the flips, the audit outcome, the plan file, and the first step of M4.5c2b (planned
  from handoff §4.2 and §3 "M4.5c2b"). Handoff §3 gets what was not taken, and §8 any new question.
- **Commits:** planning; VIR; by-value; storage; literals; status and handoff. All local, on `m4.5c2a-struct-values`,
  with no Claude trailer, and never pushed. The handoff gives the owner the push and PR steps, after PR #24.
