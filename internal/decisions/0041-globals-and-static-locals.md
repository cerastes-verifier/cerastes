# 0041 — Mutable globals and `static` locals (M4.5b2a)

- **Status:** draft for M4.5b2a (2026-10-30), written before code (invariant 6); revised after P63 (hazards H1–H8: items
  2, 3, 5, 6, 11), audit round 2 on the code (no blocking item; SF1–SF4 taken: calls holding globals frame loops, item 10;
  `static` locals and globals in `pure`/`predicate` bodies rejected, item 12; the lint refuses a call of the entry, item 4;
  C's trust assumptions, item 3; `build/m4.5b2a/audit2.md`) and audit round 1 (Opus, on the design: two blocking items, B1 an internal C++ global written before
  `main` through an escaped pointer, B2 address comparisons the C++ front end folds; ten should-fix items and thirteen
  notes; all taken except N9, by design; `build/m4.5b2a/audit1.md`) and S31 (items 4–12 hold; F1 for loops in item 8). Owner decisions (M4.5b2a planning, 2026-10-30; plan `.claude/plans/m4-5b2a-globals.md`; handoff
  questions 78–84):
  - **80:** a `static` local's permission reaches its function by an **implicit `owns`** in the function's contract and
    in the contract of every function that reaches it in the call graph; `main` gets it from `program_start` (chosen
    over a function-private permission inhaled at entry with recursive cycles rejected, and over keeping `static` locals
    rejected).
  - **81:** the flips of this session (item 14).
  - **84:** initial values by linkage (item 3).
  - Earlier answers this record carries out: 78 (`&K` of a `const` object stays rejected, naming M4.6).
  - Planning's split (sizing per question 77): P4222's markers and `--strict-init` (0029 item 2) are M4.5b2b's, record
    0042 (questions 82, 83).
- **Milestone:** M4.5b2a (2026-10-30).
- **Implements:** record 0028 item 5 (globals as `unique` blocks, `program_start`, frames through contracts) and its open
  choice for `static` locals; 0028 item 13 and 0025 item 2's placement for globals.
- **Amends:** 0017 items 1–2 and 4 (mutable globals beside named constants), 0028 items 3a and 5, 0032 item 6 (cells by
  name), 0037 items 5, 7, 8, 10 and 15 (the places an atom may name; sites keyed by place), 0038 item 3 (global places in
  loops), 0039 items 3–4 (keys of globals).

## Context

Since M3, a mutable global is rejected at its declaration (`unsupported_declaration`, 0012 item 12) and a `static` local
likewise; 0017 made `const`/`constexpr` globals named constants, folded to EDG's value and checked in every method's
prologue. Record 0028 item 5 designed the rest: each mutable global and `static` local is one block named by a `unique`
0-ary domain function, `main` receives every global from a trusted `program_start`, and a function reads or writes a
global only through its contract (`owns(g)`, `reads(g)`), so a callee that does not name it cannot change it. It left
open how a `static` local's permission reaches its function, which callers cannot name. The fixtures
`tests/viper-smoke/memory/e2_global_{addr,frame}{,_fail}.vpr` pin the encoding (S24, S26).

Records 0032–0040 built cells, contracts, loops, sequencing and pointers to pointers around parameters and address-taken
locals: every atom site is keyed by a formal parameter's index, and the sequencing keys and the loops' places know only
locals and parameters. Facts: P63 (the IL of globals and `static` locals) and S31 (Silicon), in `internal/il-inventory.md`.

## Decision

1. **Scope.**

   | | b2a (this record) | b2b (0042) | M4.5c | M4.5d2 | M4.6 | M8 |
   |---|---|---|---|---|---|---|
   | Objects | mutable globals and `static` locals of scalar and pointer types | — | struct and union globals; places through a pointer global (`owns(*gp)`) | `owning` globals | `const` objects as cells (`&K`); string literals | several translation units (`extern` without a definition) |
   | Contracts | `owns(g)`, `reads(g)`, reads of `g`, `old(g)`, `decreases(g)` | P4222's markers on parameters | `owns(**pp)`, `live(*pp)` | `owns_block` | `const` places | — |
   | `main` | `program_start`; initial values by linkage | `--strict-init` | — | `main`'s leaks | `main(argc, argv)` | — |

2. **Which globals.** A *mutable global* is a namespace-scope (file-scope) variable whose defining declaration is in a
   verified file (0011 item 4) and which is not a named constant (0017 item 1: neither `const` nor `constexpr`). It is
   accepted when:
   - its type is `bool`, a builtin integer type, an enumeration 0018 accepts, or a pointer at any depth to one of these
     (0032 item 2, 0040 item 2);
   - it is not `volatile` or `_Atomic` (0032 item 2), not `thread_local`/`_Thread_local`, not weak (0017 item 1), and
     carries only allow-listed attributes (0011 item 7);
   - it is defined in the unit (P63 A1: `declared_type != NULL`; EDG keeps one variable whose `decl_position` is the
     defining declaration): a definition, a C tentative definition (zero-initialized, C23 6.9.2; `initk_none`, as C++'s
     `int z;`), or an `extern` declaration with a later definition in the unit. An `extern` declaration with no
     definition in the unit is rejected (`unsupported_declaration`, "a global variable defined in another translation
     unit (M8)");
   - its initializer, if any, is a constant (item 5): in C++, a global whose initialization is dynamic (`int g = f();`,
     `initk_dynamic`) is rejected, and so is one whose constant initializer the front end folded from a call (a
     `constexpr` call, P63 H7: 0017's calls-rejected rule, fail closed).

   Rejected, each with its own message: an array (M4.5e1), a struct or union (M4.5c, M8), a `void*` or a pointer to a
   struct, union or function (0032 item 2), an initializer that is the address of a `const` object (`int *gp = &K;`,
   M4.6, question 78), a string literal (M4.6), or any other address constant than `&h` of an accepted mutable global at
   offset 0 (`&h + 1`, `&f`, a label), a non-zero integer constant of pointer type ("an integer used as a pointer"), a
   `restrict`-qualified pointer global (0032 item 2), a global that also has a declaration in a system file (the
   runtime may touch it: `environ`, `optind`; audit N6), and a C++ variable template (M5). A class's static data member
   is M7's. Among `static` locals (item 12), `static const` and `static constexpr` locals are rejected naming M4.6
   (`const` objects, question 78; their uses fold), and so is a C++ `static` local whose initializer names a local
   constant (audit N7: it breaks the prologue's closedness); a block-scope `extern` declaration stays rejected
   (`encode.cpp`'s local-declaration branch, unchanged).

3. **Initial values by linkage** (question 84). `program_start` states a global's value only where the unit can know it:
   - **internal linkage** (`static`, C++'s unnamed namespace): its initializer's value, in both languages;
   - **external linkage in C, with an initializer:** its initializer's value. Nothing runs before `main` in C, and a
     second definition with an initializer in another unit is a link error (or undefined, C23 6.9p5);
   - **external linkage in C, with no initializer** (a tentative definition, `int g;`): an **unknown value** (audit S1).
     Apple clang 21 passes `-fcommon` by default, so another unit's `int g = 5;` silently merges with it, which is why
     0017 item 2 rejects `const int z;`; a mutable global is accepted with its value unknown instead;
   - **external linkage in C++:** an **unknown value** (initialized, within its type's range, `WF` for a pointer), since
     another unit's dynamic initializer may write it before `main` ([basic.start.dynamic]). `main` must not rely on its
     initial value; its flag is set.

   Linkage is `source_corresp.name_linkage` (`nlk_internal` for `static` and C++'s unnamed namespace), and the language
   is the unit's (`il_header.source_language`), not the linkage's label: an `extern "C" int g = 1;` in a C++ unit is
   external in C++, so unknown (P63 A1). A `static` local has no linkage and gets its initializer's value.

   **Escape through an initializer** (audit B1). In C++, a global whose address is the initial value of a global whose
   value is unknown is itself unknown, transitively (`static int g = 0; int *gp = &g;`: another unit's initializer may
   write `*gp` before `main`). A `static` local's address cannot appear in a namespace-scope initializer.

   **Trust assumptions** (audit S10; stated on the Verifying page's single-unit section): (a) code that runs before
   `main` (another C++ unit's dynamic initializers) calls no function of this unit and touches its globals only as item
   3 allows; in C nothing runs before `main` (a `constructor` attribute is not on the allow-list); (b) a bodyless
   function's contract (another unit's function) states every global of this unit it changes, including through a call
   back into this unit, as it states every cell (0028 item 9); (c) the program is valid: one definition of each object;
   (d) in C too (audit round 2, SF4), no other file runs code before `main` (a GNU `constructor` function elsewhere
   could write an external global this unit takes as known) and no other file calls `main` (it would enter `main` in a
   state `program_start` does not describe); this unit's own `constructor`/`destructor` attributes are rejected, and its
   own calls of `main` too.

4. **The encoding** (0028 items 1, 2, 5).
   - **VIR.** A global is not a program-level `Var` (`Var::id` is unique per function only; `var_names`, `key_of_cell` and
     the Sequencer rely on it). VIR gains `vir::Global` (an id, the C name, the type, the initial value or "unknown", the
     declaration's position, the owning function for a `static` local) in `Program`, and one expression kind,
     `GlobalAddr(g)`, the address `&g`. A read of `g` is `Deref(GlobalAddr(g))`, a write `Store`/`StoreE` through it, an
     atom `Atom(a = GlobalAddr(g))`. `Program::entry` is the global-namespace `main`: named `main`, not a class member,
     its parent scope the file's (`sck_file`; P63 A7). C++'s `ns::main` is an ordinary function there (today's `is_main`
     also matches it for its rejections, which stay: over-strict, harmless).
   - **The prelude.** For each global and `static` local, `unique function GB_<name>__<id>(): Block` inside `domain
     Block`; its cell is `loc(GB_g(), 0)`, with the value field and flag of 0028 item 1 (`v_int`, `d_int`, `v_ptr_int`, …).
     A unit without globals prints the prelude unchanged, byte for byte.
   - **`program_start()`**, a trusted bodyless method (`// cerastes:trusted-contract <reason>`), whose first `ensures` row
     is the global facts below (audit S2), followed, for each global and `static` local `g` in declaration order, by one
     `ensures` row positioned at its declaration:
     `acc(loc(GB_g(),0).v_T) && acc(loc(GB_g(),0).d_T) && loc(GB_g(),0).d_T && <value> && acc(lvloc(GB_g()).lv)`, where
     `<value>` is `loc(GB_g(),0).v_T == c` for a known value (an integer, `true`/`false`, `null`, or `loc(GB_h(),0)` for
     `&h`), and for an unknown one its range, or `WF(loc(GB_g(),0).v_ptr_T)` for a pointer.
   - **The global facts.** Every method of a unit with globals, its `__wf` method included and `main`'s excepted, takes
     as its first `requires` (before the ghost amounts' `none < a` rows, audit N12) the pure facts `len(GB_g()) == 1 &&
     lvloc(GB_g()) == loc(GB_g(), -2)` for each global and `static` local; `main` gets them from `program_start`, which
     ensures them, and every caller has them from its own `requires`. They are true
     in 0028 item 2's model (a `GB_` constant is an allocated block of one slot whose liveness cell is `loc(b, -2)`), so
     they are hypotheses discharged by callers, never axioms. Without them `&g` passed to a callee's `WF` precondition
     fails Silicon's own check (exit 3), and 0032's one-past-the-end check gives false alarms on `&g`.
   - **`main`.** `Program::entry`'s method calls `program_start()` as its first statement, before anything else.
     A `requires` on `main` (`contract_on_main`) and a call of `main` stay rejected (0028 item 5); an `ensures` on `main`
     is checked at its returns as before (0012 item 9). So `program_start` is
     reached once per program. `program_start` is emitted in every unit with a global, and only `main` calls it: in a unit without `main` it is a trusted method nobody calls, and the globals reach functions only through contracts.
   - **The lint** (`lint_viper.py`): `unique function GB_<ident>(): Block` is admitted only inside `domain Block`, with
     no other `unique` declaration anywhere; `program_start`'s contract is pinned row by row, as `lint_allocations` pins
     `alloc_<t>`: the facts row, then exactly one row per declared `GB_` function and no other (a duplicate row would
     hold two full permissions and make `main` vacuous, audit S3), each of the form above with a value that is a
     literal, `null`, or `loc(GB_h(),0)` of a declared `GB_h` of the matching pointer field, or a range or `WF` row for
     an unknown value; the global facts are pinned as the first pure `requires` row of every other method; the entry
     has no `requires`, calls `program_start()` once as its first statement, and is called by no method (audit round 2,
     SF3).
   - The fixtures `tests/viper-smoke/memory/e2_global_*.vpr` (a separate `domain Globals`; `len` as a clause of `owns`)
     are superseded by S31's `s31_*` fixtures, which pin this shape; they stay as S24's evidence (audit N5).

5. **Initializers** (amends 0017 items 2 and 4).
   - An initializer of `bool`, integer or enumeration type is read and checked like a named constant's: in every
     method's prologue, by its original expression with its obligations at their positions, then the `internal` guard
     that the value as written equals EDG's (0017 item 4); `program_start` states EDG's value. They are kept in a list of
     their own beside the named constants (a named constant is folded at its uses; a mutable global never is). 0017 item
     9's `perturb_folded` hook moves these guards' values too.
   - A pointer initializer is read by a new reader (P63 A2, H3–H5): a `ck_address` constant with `address.kind ==
     abk_variable`, `address.offset == 0` and no `subobject_path`, whose variable is an accepted mutable global or
     `static` local (`&h`, declared before or after), and whose type is a pointer to the variable's own type, is
     `GlobalAddr(h)`. The front end folds casts into such constants (`(char *)&h`, `(int *)&k` for a `const int k`, H3),
     so a constant whose pointee type differs from the variable's is a pointer conversion under 0040 item 2's rule
     (rejected unless it only adds qualifiers at the pointee's top level). `&h + 1` (offset 4) and `&h - 0` (a
     `subobject_path`) are rejected (H5); a string literal is `abk_constant` (M4.6). A null pointer constant is a
     `ck_integer` 0 of pointer type (C's `0`/`NULL`, C++'s `NULL` with `null_keyword`, `nullptr`; not `is_simple_zero`,
     H4) and is `null`; no initializer is `null` (static storage).
   - A `static` local's initializer is not on the variable when its `init_kind` is `initk_function_local`: it is the
     `local-static-variable-init` entry of its scope's `local_static_variable_inits` (P63 H6), read the same way
     (`initk_static`); in C++ a `static` local initialized dynamically (`initk_dynamic`) is rejected. A `static` local is
     told by `enclosing_routine != NULL && storage_class == sc_static` (P63 A6); two of one name in nested blocks are
     two blocks.
   - An automatic local initialized with `&g` (`int *q = &g;`) is `dik_constant` with a `ck_address` (P63 H8), a shape
     the local-initializer reader has not met (`&x` of a local is `dik_expression`): it reads the constant as above.

6. **In code** (amends 0032 item 6).
   - `g` is a cell: a read and a write have 0032 item 6's and 0037 item 11's checks (permission, flag, range), with the
     place spelled `g`; without the permission the error is `permission` ("reads `g` without owning it; the function
     needs `reads(g)` or `owns(g)` in its contract").
   - `&g` is `GlobalAddr(g)`: no allocation, no scope exit (a global's block never ends). `*&g` is `g`. The IL has two
     shapes for it (P63 H1): `eok_address_of` over the variable (a `return` value, `*&g`, an operand of `+`, an
     assignment target, C++'s `?:` arms) and a `ck_address` constant read as in item 5 (an assignment source, a call
     argument, a comparison operand, C's `?:` arms; its `expr` keeps the original tree). In C, a read of an `int *const p
     = &g;` is folded to the constant `&g` (`expr` the variable `p`, H2): read as `GlobalAddr(g)`, the same value.
   - **Folded address arithmetic and comparisons** (audit B2). The C++ front end folds `&g == &h` and `&g != &h`, and also
     `&g + 1 == &h` (to `false`, though C 6.5.9p6 lets a one-past-the-end pointer equal the next object) and `(&g + 1) -
     &g`. A constant whose original expression (`expr`) holds an address is never read by EDG's value: it is encoded
     from its original expression, as `constant_value` already does, so 0037 item 12's checks apply (and `&g + 1`, pointer
     arithmetic, is rejected until M4.5e2), or it is rejected.
   - **Liveness** (amends 0028 item 3a). In a unit with globals, the liveness check of a pointer use gains, for each
     global and `static` local, a disjunct on its liveness cell (audit N2: the cell, not the block, so M4.5c's member
     blocks of a global are covered by `member_lv`):
     `p != null ==> (perm(lvloc(block_of(p)).lv) > none || lvloc(block_of(p)) == lvloc(GB_g1()) || …)`. Sound: no
     operation consumes a `GB_` block's liveness cell (no scope exit; `free` of a global has no token, 0028 item 6), and
     no allocation can return a block whose liveness cell is a global's in a function that holds nothing of it. It lets a
     function compare `&g` with a pointer, or test a pointer global's value, without holding the target. If S31 shows the
     disjunction too slow, the fallback is to skip the check only for an operand that is `GlobalAddr` itself.
     (`alloc_<t>` does not ensure `block_of(r) != GB_g()`, so `&x != &g` in a function that holds nothing of `g` is not
     proved: a completeness gap, audit N3.)
   - A callee's `live(p)` still takes a fraction of the liveness cell: passing `&g` to a `live` parameter needs a held
     fraction (`owns(g)`, `reads(g)`, or `program_start` in `main`).

7. **Contracts** (amends 0037 items 5, 7 and 15).
   - `owns(g)` and `reads(g)` in both spellings (`cerastes_owns(g)`, `cerastes::owns(g)`) name the global's cell, with
     0028 item 7's meaning: full or a fraction of the value and flag cells, the flag set, the value in its range (`WF` for
     a pointer global), a fraction of its liveness cell (one ghost amount per atom, 0037 item 6).
   - A contract may read `g` where an earlier atom of its clause list frames it (0037 item 7); `old(g)` in an `ensures`
     is its entry value; `decreases(g)` is accepted where `decreases(*p)` is (0039 item 8).
   - `owns(*gp)`/`reads(*gp)` through a pointer global, and reads of `*gp`, are places reached through a heap read:
     rejected, naming M4.5c (question 74). `live(&g)` is rejected: liveness of a global is never in doubt (item 6), and a
     function that passes `&g` on writes `reads(g)`.
   - `&g` as a value (audit S8) is allowed in a contract, in both IL shapes (item 6): `p == &g`, `ensures(r, r == &g)`,
     `gp == &h` after an atom that frames `gp`. It is a constant `Ref` with no heap read; comparisons are identity
     (0037 item 12). A `static` local's address never appears in a contract (its name is not in scope there).
   - Two atoms of one kind on one global in one clause list (`owns(g) && owns(g)`) are rejected, as 0037 item 5 rejects
     them on one `*p` (audit S4: two full permissions would make the body vacuous); `owns(g)` beside `reads(g)` is
     checked per path by `__wf`'s overlap assert, as `owns(*p)` beside `reads(*p)` is (0039 item 7): `permission` at the
     contract, at the later atom.
   - `owns(p)` of a parameter's own cell stays rejected, now naming no session (a parameter's copy is the callee's;
     `reject_owns_of_param_cell.c`).

8. **Calls and returns** (amends 0037 items 8 and 10).
   - An atom site is identified by a key (amends 0037's sites "by parameter index"): a formal's index, the result, or a
     global (keys from `1 << 20`). A global site's receiver is `loc(GB_g(),0)`; it has no argument.
   - At a call, the caller's explicit checks for a global atom of the callee's `requires` are 0037 item 8's (permission
     of value and flag, the flag, the range or `WF`, a fraction of liveness), category `permission`, with "calls `f`,
     which needs to own `g`, without owning it"; its ghost amount is chosen as for a `*p` atom. Separation: a global
     atom and a `*p` atom of one field are checked distinct (`p != loc(GB_g(),0)`), so `f(&g)` with `requires(owns(*p)
     && owns(g))` is `permission` (the aliasing trap). Two atoms on different globals need no check and no case split
     (`unique` blocks differ).
   - At each `return`, the `ensures` atoms on globals are checked as 0037 item 10's, keyed by site; `__wf`'s ghosts are
     keyed by site.
   - Atoms are paired (an `ensures` atom with its `requires` partner, 0039 item 7) by site key, not by parameter index.
   - A global atom beside a `*p` atom of its field is treated as two parameter atoms are (audit N4): at entry, 0037 item
     9's separation assert and `__wf`'s distinctness and value-equality facts; at calls, the case splits between
     `reads` atoms that may name one cell (0037 item 8); in loops, the case splits and guards (0038 item 3); in framing,
     the disjuncts with `q = loc(GB_g(),0)` (0037 item 7).

9. **Frames.** A callee whose contract does not name `g` gets no permission to it, so the caller keeps its value across
   the call (0028 item 9); a callee with `owns(g)` and no `ensures` about `g`'s value havocs it.

10. **Loops** (amends 0038 item 3). Every global and `static` local the function holds is an automatic place of each
    loop, like a parameter's place: in a function, the globals of its `requires` atoms, written and implicit (item 12;
    audit S6: otherwise a call that reaches a `static` local inside a loop fails `permission`, and a missed havoc is exit
    3); in `main`, every global and `static` local of the unit (`program_start`). The implicit atoms are added before
    anything reads the contracts to classify loops and memory calls. A loop whose body calls a function with any
    `requires` atom (a global's included) touches memory, so its places are framed (audit round 2, SF1). Its value is kept unless a write in the loop can reach it: a write to
    it by name, a write through a pointer of its field, or a call that owns a cell of its field (0038's havoc rule). A
    written `owns(g)`/`reads(g)` in `CERASTES_INVARIANT` is rejected (`unsupported_annotation`: "a global the function
    holds is in every loop's invariant already").

11. **Sequencing** (0028 items 12–13; amends 0039 items 3–4; 0025 item 2 revisited).
    - Each global is an object of its own in the Sequencer (a key range for globals, decoded to the name), and `*&g` is
      `g`. Same-object shapes are rejected like the compilers (`unsequenced_modification`; P63 A8, the union rule of 0020
      item 5: `g++ + g`, `g + g++`, `(g = 1) + g`, `g = g++`, `g = (g = 1)`, `g += g++` warn in both compilers and modes,
      `*&g + g++` and `(*&g = 1) + g` in GCC's C mode); without the key, lowering's same-key skip would let them through.
    - A global beside a pointer access (`g++ + *p`, `*p = g++`, `g++ + *gp`; no compiler warns, P63 A8) is checked
      path-precise (`p != &g`, `unsequenced`).
    - **Placement** (0025 item 2): a callee whose contract has an atom on a global is a memory call, with no pointer
      argument needed; writes and snapshots of a global's cell never move across it, and an access the call is not
      ordered with must name a different place: `g + f()` with `f` owning `g`, and `g++ + f()` with `f` reading `g`,
      fail `unsequenced` (the places are equal). C calls the first only unspecified; cerastes reports it, as 0028 item 13
      does for cells. Viper locals keep today's placement (0021 item 2).
    - C++17 sequences an assignment's right operand before its left (`g = f()` is fine in both languages; `g += f()` is
      sequenced in C++ only). 0039 keeps C's rule in both modes, by design (handoff §3, M4.5b1c2's N4); audit N9 asked for
      C++ positives, not taken.

12. **`static` locals** (question 80; 0028 item 5's open choice).
    - A `static` local of an accepted type is a block like a global (`GB_<function>__<name>__<id>`), with its
      initializer's value in `program_start` (item 3).
    - **The declaration emits nothing** (audit S7): no store, no allocation, no scope exit; a `static` local is not a cell
      of its block's scope (0038 item 3's scopes). Its initializer runs once, before `main` (`program_start`), so
      `static int c = 0; c++; return c;` does not always return 1.
    - **The implicit atom.** Its function `f` gets `owns(s)` appended to its `requires` and its `ensures`, after the
      written atoms' numbering (its ghost amount numbered after them), and so does every function that reaches `f`
      through the encoder's call edges (reverse reachability; recursion needs nothing more). The atoms of one function
      are a set (audit S4: a recursive `f` reaches itself, and `owns(s)` twice would make its body vacuous). Only
      `Program::entry` is skipped (it holds `s` from `program_start`; C++'s `ns::main` is not skipped, audit N1); a
      `noreturn` function gets no `ensures` atom. Messages name "the `static` local `s` of `f`". The atom never appears in
      the user's text and frames nothing a contract can write (the name is not in scope there). The propagation fails
      closed (audit N1): a missed edge leaves a caller without the permission, which is `permission` at its call, never a
      proof; a caller in another unit (bodyless) cannot be given the atom, which only loses the two-call report, since the
      value is unknown on entry anyway.
    - **Consequences.** The value is unknown on each entry to `f`. Every caller of `f` holds `s` across its own body, so
      `f() + f()` and `h() + k()` with both reaching `f` are two memory calls on one cell: `unsequenced` (0028 item 13).
      Within `f`, `&s` is `GlobalAddr(s)`; `return &s;` is accepted (R14 does not fire on static storage), but the caller
      knows only `WF(r)` (it has no atom on `*r` or `r == &s` before M4.5d2), so its uses of the result are `lifetime`
      false alarms (audit S5; documented).
    - **Rejected:** a C++ dynamic initializer, `thread_local`, a `static` local in a `pure` or `predicate` function, and a
      `pure` or `predicate` body that reads or writes a mutable global or `static` local (their bodies are not encoded
      before M4.5f1, which must then decide; audit round 2, SF2).

13. **Categories:** none new. `permission` gains the global messages, `unsequenced` the global and implicit-atom ones.

14. **Flips** (question 81, pre-approved). Retired, each replaced by a positive of the same shape with a must-fail twin:
    `tests/scalar/negative/reject_global.c`, `tests/scalar/negative/reject_static_local.c`,
    `tests/memory/negative/reject_pointer_global.c`. Kept rejected with their `expect:` lines, only the message and
    `Pins:` changed: `reject_address_of_const_global.c` and `reject_case_const_address_taken.cpp` (M4.6),
    `reject_owns_of_param_cell.c` (no session).

15. **Still rejected:** P4222's markers (0042); places through a pointer global (M4.5c); `owning` on globals (M4.5d2);
    `&K` of a `const` object (M4.6); arrays and structs as globals (M4.5e1, M4.5c); `extern` without a definition and
    several translation units (M8); `volatile`, `_Atomic` and thread-local globals (M8).

16. **Facts relied on** (`internal/il-inventory.md`, "M4.5b2a"): (P63) the IL of globals, their linkage and initializers,
    `&g` in code and contracts, `static` locals, `main`, and the compiler matrix of same-object shapes on globals; (S31)
    `program_start` with `unique` globals and the global facts (vacuity), global atoms at calls, returns and `__wf`,
    implicit `static` atoms through recursion, global places in loops, and the liveness disjunct's cost.

17. **Tests and documentation.** Must-verify and must-fail pairs for every accepted form in both modes where valid
    (invariant 2), the ports `global_points_to_syntax.c` and `globals.c`'s scalar part, the Memory and ownership page's
    "Global variables" and "`static` local variables", the Error messages page's new messages, the Verifying page's
    rejected list.

## Consequences

- Programs without globals keep their Viper byte for byte; the site keys are a refactor with no behaviour change.
- `main` starts from the unit's initial state (VeriFast's `open_module` without the ghost); other functions see globals
  only through their contracts, so a function that touches a global without naming it is reported, not trusted.
- A `static` local costs every transitive caller an implicit atom (an amount, about ten rows per call); its value is
  never known on entry, and two unordered calls that reach it are reported.
- In C++, `main` cannot rely on the initial value of a global with external linkage, or of one whose address such a
  global holds; in C, of a tentative definition with external linkage. The trust assumptions of item 3 are stated where
  the single-unit assumption already is.
- For later sessions (audit N8, N10): M4.5f1 encodes `pure` bodies, which must then handle globals; M4.7 keeps `&main`
  rejected, so `program_start` stays reached once.
