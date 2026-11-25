# 0062 — Pure functions (M4.5e1d)

- **Status:** accepted (2026-11-25, M4.5e1d; drafted 2026-11-24), written before code (invariant 6) from the plan's section 6
  (`.claude/plans/m4-5e1d-pure-functions.md`, approved 2026-11-24); to be revised only by P81's and S46's decision
  tables (the plan's section 5; `internal/il-inventory.md`, chapter M4.5e1d; fixtures
  `tests/viper-smoke/memory/s46_*.vpr`), by Step 0's runs and by audit round 1 (Opus, on Commit 1 `92294b7`: every
  S46 fixture reproduced; two blocking items, both taken: B1 the lint admitted fact (ii) without a callee's quantified
  clause and refused the record's form, items 8 and 12; B2 an application nested in `cerastes::old`'s operand
  compiles in C++, items 1 and 4; six should-fix items, all taken: S1 the keep fact's guard read from the callee's
  atoms, S2 fact (i)'s range tied to a result tag, S3 every read function's `__wf`, S4 trusted contracts read
  functions, S5 the check's range and shape rows, S6 a discriminating no-`decreases` fixture; notes N1 (item 11's
  argument traced), N2, N9 taken; `build/m4.5e1d/auditor-report-1.md`), by audit round 2 (Opus, on the snapshot
  `80b2954` of Parts A and B: no blocking item; five should-fix items, all taken: cut 2's source tests, the
  definition-unknown pin (question 282), value checks in seven positives, a pure body's own message for an unframed
  read, a read through a loaded pointer rejected; `build/m4.5e1d/auditor-report-2.md`) and by audit round 3 (Opus, on
  the snapshot `62f79b5` of Part C: no blocking item; four should-fix items, all taken: the keep fact over a cell atom
  admitted by the lint with a test, an application over a range of pointer elements in a `check` or a loop invariant
  rejected, the pointer-argument conversion peeled one qualifier level only, a source pin for an application in an
  atom's guard; `build/m4.5e1d/auditor-report-3.md`) and by audit round 4 (Opus, on the snapshot `7f1a494` of Part D
  and the pages; probes on the x86-64 box: no blocking item, so it is also the plan's re-audit; should-fix items R4-S1
  to R4-S4 taken: a contract's check knows an application's result through the callee's `ensures` and the result
  type's range (fact (i)), a pure function's `requires` calls only functions that read no memory, the pages list the
  rejections of round 3's R3-S2 and R3-S3 and item 7's marked-range false alarm, and `internal/encoding.md` states the
  loop form's guard as built; R4-S5 a listed limit, fail closed: two of the auditor's wrong probes, a `must_init` filler whose invariant also applies a function over the range it is still filling, time out at 300 s on the box (exit 3) instead of reporting `uninitialized`; the prefix-only twin verifies in 36 s; item 7 and the handoff's questions; `build/m4.5e1d/auditor-report-4.md`). Owner decisions:
  - At M4.5e1's planning (2026-11-11): **226** a `forall` inlines its function's body; a call of a `pure` function
    anywhere else waits for e1d. **227** the exact sum is in M4.5e1: e1d takes `pure` bodies, their calls and their
    recursion from M4.5f1/f2; M4.5f1 keeps predicates, `fold`/`unfold` and pure functions over predicates.
  - At M4.5e1c's planning (2026-11-23): **252** a contract on a `pure` function is rejected where a `forall` names the
    function; e1d gives such contracts their meaning.
  - At M4.5e1d's planning (2026-11-24), each the recommendation except 271 and 272: **264** a wrong `ensures` on a
    `pure` function and a recursive call whose measure does not decrease are reported from Viper's own check of the
    function, mapped to C lines (`postcondition` at the function's `return`, `termination` at the recursive call); every
    other obligation is an explicit row (items 9–11). **265** a `cerastes_forall` that names a function with a contract
    stays rejected (question 252 at the `forall`); M4.5e2 lifts it with calls under a binder. **266** not in e1d, each
    rejected and pinned: a call under a quantifier's binder (M4.5e2), a `cerastes_forall` inside a called `pure` body
    (M4.5f1), mutual recursion between `pure` functions (M4.5f2), a call in a `decreases` (M4.5f1). **267** outside
    `f__wf` a caller learns the range of an application's result only from the function's `ensures` (a false alarm
    otherwise, fail closed; M4.8 reviews). **268** which `pure` functions are read: "Called, or has a contract" (item
    2). **269** a `pure` function with no body in the unit that an annotation calls: "Rejected" (item 3; M6's contract
    manifest lifts it). **270** one session, split points. **271** (asked again on the design check's 3,050–4,500
    lines; the recommendation was two sessions planned now): "One session, split points". **272** (which of three
    constructs to leave out to hold the size; the recommendation was all three): "None: keep all three in" — a
    `cerastes_forall` in a `pure` function's contract, arguments that read memory, and calls in a `pure` function's own
    contract are in scope, as the plan's pre-agreed cuts 2, 3 and 4. Questions 256–263 of M4.5e1c stand as built.
  - In this session (2026-11-24), each the recommendation: **273** (raised by the test-mutator: item 12's name rule
    read literally refused 145 of the lint's 843 self-tests, whose fixtures use made-up function names) "H'": a
    top-level function that is neither a prelude function nor a `pf_` function may apply only prelude functions, so it
    can neither recurse nor reach a `pf_` function (item 12); **274** the `<id>` of `pf_<name>__<id>` is digits only,
    as a method's `f_<name>__<n>`; **275** (S46 (b), an outcome no table named: Silicon's axiomatisation warning on a
    read or an application in a branch the function's `requires` make dead, beside "verified" too) "Accept for
    `pf_`": the driver and `run_tests.py` accept exactly that warning line when it names a `pf_` function (item 11);
    **276** (S46 (c): the plan's literal discriminator (3), `ensures r == sum_to(a, n) + 1`, verifies without the keep
    fact, since fact (ii) already bounds the sum; with `+ 9223372036854775000` it fails once without (iii) and passes
    with it, and (4) is as expected; the table's letter was cut 1) "Keep (iii)": fact (iii) stays (item 8), and the
    adapted program is its discriminator and Part C's keep test; **277** (S46 (d): the twin `s == sum_to(a, i + 1)`
    gives five Silicon errors where the plan expected two, four with an `ensures` that holds for any value, and a
    framing-only twin gives three, so no one-error framing twin exists under item 7's guard) "Expect all": each such
    twin's expect lines list every error the driver prints (question 254's rule), and item 7 says why; **278** (S46
    (f): a count past a local array in a `check` gave two errors in the planned order, the bound row and the callee's
    pure line; with the pure lines first, one) "Pure lines first": in a `check` the callee's pure lines are asserted
    before the bound and the marked rows (item 7), as e1b's shape row comes first at a call (0060 item 8); **280**
    (test-mechanics, raised by test-mutator-2) two rejection pins of a function rule call from a `check`, which Part B
    accepts, not from an ordinary `requires`; **281** "Count read pure functions": a read `pure` function counts as one
    function verified (item 9; amends 0011 item 9); **282** "Keep fix + add pin": item 8's listed false alarm (a
    callee known only through its `ensures`) has a pinned negative test beside the positive that gives the callee an
    `ensures`; **283** "Rows, as elsewhere": in a loop invariant a callee atom's pure lines are explicit `precondition`
    rows, as in contracts and checks (an application outside its callee's `requires` whose interval passes the framing
    would otherwise take its default silently, an approximation); **284** "Accept, check form": an application in a
    `do … while (0)` loop's invariant (0014 item 3: straight-line code) takes the `check` form of item 7, its fact
    asserted as that invariant's `invariant-entry`. **279**
    (the test-mutator's lint found two facts of item 8 that `f__wf` cannot assume safely, outside the plan: fact
    (i) for an enumeration result, fact (ii) for a callee whose `requires` holds an application) "Accept, M4.8": both
    are listed limits, fail closed (item 8). **285** (plan section 11's 120 s rule: on the x86-64 box, where the owner
    now has every test run (2026-11-24), the false facts of `pure_sum_caller_write_between.c` and
    `pure_check_local_false_fact.c` take 226–356 s, one Z3 refutation over the sum of a 3- or 4-element array: 34 s
    for 2 elements, 207 s for 3, 251 s for 4; the recommendation was two-element programs) "Move to tests/long/": both
    move unchanged to `tests/long/memory/negative/` (record 0057), and `pure_sum_caller.c` keeps a default-tier twin
    (question 207's rule) in `pure_sum_caller_write_between_short.c`, the same program over 2 elements; **286**
    "Fix now in e1d": the driver and `run_tests.py` pass Silicon `--numberOfParallelVerifiers` min(hardware threads,
    16); Silicon's default, one prover per hardware thread, cost every run on the 172-thread box about 33 s of startup
    (a trivial method: 37 s by default, 8 s with 16), and on a 16-thread Mac the cap changes nothing (amends 0011
    item 11).
  - Standing: 69 (no EDG work), 77 (one context), 160 (a new test's expect line may be corrected after its first run,
    each listed in status), 176 (the owner handles CI).
- **Milestone:** M4.5e1d.
- **Amends:** 0006 items 8 and 10; 0010 item 11; 0011 items 4, 7, 9 and 10's messages; 0012 item 11; 0013 items 2, 4, 5
  and 8; 0024 item 2 and 0025 item 5 (the exemption of a spec routine's region); 0037 item 7; 0041 item 12; 0042 item 4
  and 0044 item 4 (their messages); 0061 items 1, 3, 4, 7 and 11; 0011 item 11 (the prover cap, question 286).

## Context

Record 0061 (M4.5e1c) gave C its quantifier by inlining the body of a `pure` function at the `forall`. No `pure` body
has a meaning of its own: a call of a `pure` function is rejected in every annotation (`call_in_annotation`), a
contract written on one is neither read nor rejected (`Encoder::run` skips every spec routine before `function_of`),
and nothing can name a value computed from an array. M4's acceptance asks for the exact array sum "by a recursive
`pure` function" (question 227). This record gives `pure` functions their meaning as Viper functions, their contracts,
the check of their definitions, their recursion and their calls (**applications**) in contracts, loop invariants,
checks and other `pure` functions, with every obligation an explicit row in source terms (0028 item 11). Facts: P81 (the
IL of a call of a `pure` function in each position, a `pure` body, contracts on a `pure` function) and S46 (Silicon on
the shapes of this record), in `internal/il-inventory.md`, chapter M4.5e1d; the table at the end maps each outcome to
the branch of the plan's decision table it took.

## Decision

1. **Scope.** In: a read `pure` function (item 2) as a Viper function with its contract (`requires` with value
   clauses, `reads(*p)` and `reads_range(p, n)`; `ensures`; `decreases`); the check of its definition (the obligations
   of its body and contract, its `ensures`, its termination); self-recursion; **applications** (calls of a `pure`
   function) in an ordinary function's `requires` and `ensures`, in a loop's `CERASTES_INVARIANT`, in a
   `CERASTES_CHECK`, and in a read `pure` function's body and contract, each with explicit rows for the callee's
   `requires`; `f__wf` for contracts that hold an application; calls and returns; the exact sum with its loop; the
   shape rules and the static-assertion cross-check in read `pure` bodies; the lint; the tutorial section "The exact
   sum". Everything else stays rejected, each with a diagnostic naming where it is lifted and a pin:

   | Construct | Lifted by |
   |---|---|
   | a `predicate` (body, call, `fold`, `unfold`); a pure function over a predicate | M4.5f1 |
   | a call of a `pure` function with no body in a verified file of the unit (owner, 2026-11-24, question 269) | M6 (the contract manifest) |
   | a call of a `pure` function from code (`spec_call_in_code`, the front end's, unchanged) | never (specification) |
   | a `pure` member function, template or template instance; a `constexpr`/`consteval` `pure` function that is read | M7, M5 |
   | mutual recursion between `pure` functions; a `pure` function that reaches itself through a `requires` or an `ensures` (its own or another function's): the only cycle accepted is a function calling itself in its own body | M4.5f2 |
   | a call under a quantifier's binder (inside the function a `cerastes_forall` names); a `cerastes_forall` naming a function that has a contract (question 252 at the `forall` site) | M4.5e2 |
   | a `cerastes_forall` inside the body of a read `pure` function (a named quantified property) | M4.5f1 |
   | a call in a function's or a loop's `decreases`; in the count or guard of an atom, in an element's index, as a trailing argument of `cerastes_forall` (the positions that must read no memory: a call counts as a read there); under `old` (the direct forms do not compile, P81 (f): `cerastes_old` takes an address, `cerastes::old` refuses an rvalue argument, each pinned as a front-end negative; an application nested in `cerastes::old`'s operand compiles in C++, `cerastes::old(*(g(n) ? p : q))`, `cerastes::old(a[g(n)])`, audit round 1 B2, and the encoder rejects it, pinned in C++, with `check.cpp`'s backstop behind it) | M4.5f1 (measures), M4.5e2 (the rest) |
   | a read `pure` function whose body names `cerastes_old` (a function has one state; e1c allows the body only under a `forall` in an `ensures`) | never as a call |
   | a read `pure` function that returns a pointer, a structure or `void`, or takes a structure by value | M4.5f1 |
   | in a `pure` function's `requires`: `owns`, `owns_range`, `owns_block`, `live`, an atom under a guard or `\|\|`; any atom in its `ensures`; markers on its parameters (0042 item 4); `restrict` | M4.5f1 |
   | in a read `pure` body: a global or `static` local (0041 item 12), structure members, array members, a local declaration, more than the one `return` | M4.5f1, e1e |
   | a pointer argument that a callee's atom names and that is not a bare pointer parameter (contract) or a pointer variable that is not address-taken or a named local array (loop invariant, `check`): 0061 item 3's rule; in a loop invariant or a `check`, an application whose callee reads a range of pointer elements (audit round 3, S2: question 259's rule for pointer elements under a binder) | M4.5e2 |

2. **Which `pure` functions are read** (owner, 2026-11-24, question 268). A `pure` function of the unit is **read** when
   an annotation of a function whose contract is lowered calls it (a verified function's, or a trusted one's: a
   bodiless or vlibc contract; audit round 1, S4), when a read `pure` function's body or contract calls it, or when it
   carries a `requires`, an `ensures` or a `decreases`. A read function is checked once, whether or not anything calls
   it. Every other `pure` function is as before: one that a `cerastes_forall` names is inlined there (0061), one that
   nothing uses is not read (0011 item 4), and the checks `Encoder::run` makes on every spec routine (markers, structure
   values, globals) stay and run first: **a function they reject is not read further**, so it keeps its one diagnostic
   (`reject_owning_global_pure_read.cpp`, the only such function with a contract today).

3. **What a read `pure` function is.**
   - **The function:** defined in a verified file of the unit (a call of one that is not is rejected,
     `unsupported_annotation`, M6); not a `predicate`, a member function, a template, an instance, a lambda, variadic,
     `constexpr` or `consteval`; `check_signature`'s rules; the rejections an ordinary function gets before
     `function_of` (weak, the bodiless `inline` rule). Its result is `bool`, an integer type or an enumeration; every
     parameter is a `bool`, an integer, an enumeration, or a pointer to one of those or to a pointer (0061 item 3's
     types). A function that is not such a function: **`pure_function`** (new tag; its message says which rule), at the
     call, or at the function when only its contract makes it read.
   - **The body** is exactly one `return E;` (an extra block is read through, as e1c's reader does). `E` holds what an
     annotation expression may hold; element reads `p[e]` and `*p` through the function's pointer parameters; calls of
     read `pure` functions, itself included. Refused in `E`, each with the tag the construct has in an annotation and a
     pin: an atom, a `cerastes_forall`, `cerastes_old` (a function has one state), an assignment, `++`, `--`, a comma, a
     global or `static` local, `&` of a parameter, a call of anything that is not a `pure` function. The returned value
     is converted to the result type (in C a `bool` result is `E != 0`, P80 (c)); **that conversion has its own
     obligation**.
   - **The shape rules and the static assertions reach it:** R1–R13 run in `E` as in any expression (it is encoded
     through `expr()`); the region of a read `pure` function is no longer exempt from `check_block_static_assertions`.
     **A block-scope `static_assert` in a read `pure` body is checked as in code** (P81 (e), (e2), (e3)): it is a
     `stmk_decl` statement before the `return`, of the same shape as in an ordinary body, and the reader passes each of
     its entities to `encode_block_static_assertion` (which takes the entity alone, records it as seen and makes it a
     local constant of the function, checked by the prologue of the function's `__wf` method), with the function's own
     context (not a contract's or an annotation's). The reader admits a `stmk_decl` only when every entity in it is a
     static assertion; any other entity (a local variable, with or without an initializer) is the local declaration of
     item 1's table. An assertion the reader does not pass is still reported by 0024 item 2's cross-check, now that the
     region is not exempt. The region of an unread `pure` function and of a `predicate` stays exempt
     (`static_assert_pure_body.c` is unchanged).
   - **Its contract.** `requires`: value clauses, and the atoms `cerastes_reads(*p)` and `cerastes_reads_range(p, n)` on
     its own pointer parameters, each a top-level conjunct with no guard, its count reading no memory; an unmarked atom
     promises initialized elements, as on any function (0042). **A value clause of the `requires` reads no memory**
     (`*p > 0` is rejected, `unsupported_annotation`, M4.5f1: the clause is lowered at every application over the
     arguments, which the heap-free idiom does only for a clause that reads none); the one clause that may read memory
     is a `cerastes_forall` (cut 2; item 7 says how it is lowered at an application). `ensures(r, …)`: value clauses over
     the parameters, the result and reads the `requires` atoms frame; no atom (the function takes nothing and returns
     nothing: it reads); no `old`. `decreases`: as 0013, reading no memory and calling nothing. A `cerastes_forall` may
     stand in its `requires` and `ensures` (S46 (h); cut 2), as in any contract, and so may a call of another read `pure`
     function (cut 4), as long as no function reaches itself through a contract (item 10). The rest of item 1's table is
     rejected.

4. **Applications.** A call `g(e_1, …, e_m)` of a read `pure` function is an **application**. It may stand in an
   ordinary function's `requires` and `ensures`, in a loop invariant, in a `check`, and in a read `pure` function's
   body, `requires` and `ensures`; anywhere an expression of that annotation may stand, under `!`, `||` and `?:` too (it
   is a value, not a quantifier). Every other position is rejected (item 1's table): `unsupported_annotation`.
   `call_in_annotation` stays for a callee that is not a `pure` function; its text becomes "only `cerastes::old` and
   `pure` functions can be called here".
   - **An application counts as a read of memory** (`reads_heap`), whatever its callee reads: where a rule asks that an
     expression read no memory (an atom's count or guard, an element's index, a `forall`'s trailing argument) it is
     rejected, and **`old` over an application is rejected** (today `old` wraps only what `reads_heap` sees, so an
     application under it would silently be read in the post-state). P81 (f): the direct forms do not compile — the
     front end refuses `cerastes_old(g(…))` in both modes (the macro takes an address: "expression must be an lvalue or
     a function designator") and `cerastes::old(g(…))` in C++ (an rvalue bound to `old`'s reference parameter, an
     `enk_temp_init`, as P80 (b) found: "this version cannot check this part of an annotation"), each pinned as a
     front-end negative. **An application nested in `cerastes::old`'s operand compiles in C++** (audit round 1, B2:
     `cerastes::old(*(g(n) ? p : q))`, `cerastes::old(a[g(n)])`; in C the operand of `cerastes_old` is ordinary code,
     where the front end refuses the call): the encoder rejects an application anywhere under `old`
     (`unsupported_annotation`, M4.5e2; pinned in C++ with both shapes from Part C, when an application in an `ensures`
     is lifted), and `check.cpp`'s refusal of an `Apply` under `Old` is the backstop behind it (an internal error,
     pinned by a VIR rejection case).
   - **Arguments** are expressions of the site, each evaluated once, at the site, in the site's state, with its own
     obligations (0010 item 11's; a read's framing, bound and flag) as rows of their own before the application's, in
     evaluation order. An argument may read memory (cut 3).
   - **A pointer argument that an atom of the callee names** is, in a contract or a `pure` function, a bare pointer
     parameter; in a loop invariant or a `check`, a pointer variable that is not address-taken or a named local array
     (0061 item 3's rule, for the same reason: the rows' trigger terms name a variable). P81 (d): an argument has the
     shape the same expression has as a call argument in code; `int *` for `const int *` and `int` for `long long` are
     one generated conversion (only the one-level qualification `T *` to `const T *` is read through for a pointer;
     C++'s multi-level `int **` to `const int *const *` is rejected as 0040 item 2 rejects it elsewhere: audit round 3,
     S3), and a named local array for a `const T *` parameter is a generated qualification cast
     over the generated decay (two nodes), read as e1c reads it (`forall_check_local.c`).
   - **Meaning:** the value of `g`'s `E` with its parameters replaced by the argument values, read in the site's state.

5. **VIR.** `Function::pure` and `Function::pure_body` (the encoded `E`, over `Formal`s, a pre-state contract
   expression); a read `pure` function has no statement body and is neither a verified nor a trusted method: each place
   that reads `body == nullptr` as "trusted" gets the `pure` case (`check.cpp`'s trusted-reason rule, lowering's
   `function()` and `run()`, `verified()`). `Expr::Kind::Apply` (the callee, the encoded arguments, the site's spelling
   as `c_text`); `Builder::apply`; `reads_heap` is true for it. `Kind::Call` stays the code call. `check.cpp` backstops
   items 3 and 4: positions, a callee that is `pure` with a body, arity and types against the callee's parameters, no
   `Apply` under a `Bound`'s `Forall`, in a measure, under `Old`; no `Old`, atom, `Forall` or write in a `pure_body`; the
   atoms a `pure` function's `requires` may hold; **the graph of applications over bodies and contracts has no cycle but
   a function's application of itself in its own body** (lowering has the same backstop over the emitted functions, as
   `check_cycles` has for methods).

6. **The Viper function** (lowering; a new `LFunction` in `LProgram::functions`; the name `pf_<name>__<id>`).
   - **Formals:** the parameters, as a method's, with no ghost amount.
   - **`requires`:** the parameters' type ranges and each pointer parameter's shape line (`p == null || (p ==
     loc(block_of(p), offset_of(p)) && 0 <= offset_of(p) && offset_of(p) <= len(block_of(p)))`); then the `requires`
     clauses in order (`push_clauses`, which may repeat a line), a value clause as a method's contract has it, an atom
     in the **minimal shape** (S46 (a), form **Min**, `s46_a_min.vpr`): for a range atom its pure line split into one
     `requires` per conjunct (`p != null`, the shape line, `0 <= n`, `offset_of(p) + n <= len(block_of(p))`), then
     `forall k: Int :: { loc(block_of(p), k) } in-range ==> acc(loc(block_of(p), k).v_<t>, wildcard)` and the
     element-range quantifier; for a cell atom the same lines for one slot, unquantified (`acc(p.v_<t>, wildcard)` and
     the element's range; as built in Part B, the lint admits it). **No `acc` of the flag field `d_<t>`
     or of the liveness cell `lv`, and no flag line:** every `acc` line widens the snapshot the function's value depends
     on, and the flags and the liveness are explicit rows at every application (item 7). (Form **Full**, a method's lines
     for the atom with `wildcard` for the amount, is the fallback of S46 (a).) Each line has its `internal` map row
     (`not.wellformed`).
   - **`ensures`:** the user's clauses, one line each, `result` for the result name. **No line states the result's type
     range** (Silicon would have to prove it, and a body that overflows would fail it beside the `overflow` row; callers
     learn a bound from the function's `ensures`, and `__wf` methods assume the range, item 8).
   - **`decreases`:** the user's measure, or an empty `decreases` for a function that does not call itself. Never
     `decreases _`, never no line (S46 (a): with no `decreases` line Silicon checks nothing and a recursive function
     that never decreases verifies; with an empty one on a recursive function Silicon reports consistency errors, an exit
     3, so `missing_decreases` and the lint are the guards).
   - **The prelude functions** print an empty `decreases` line **in a program that holds a `pf_` function, and only
     there** (S46 (a): a `pf_` function with a `decreases` line that applies a prelude function without one gets
     `termination.condition.false` at the application; none of them is recursive, `c_pow2` being a table), so no
     existing program's Viper changes. Measured over all 41 prelude functions the emitter prints: each, applied from a
     `pf_` function, verifies with the empty line and reports `termination.condition.false` without it.
   - **The body:** `E` in the function's own contract form: a read is its `ite` chain over the function's `requires`
     sites (0037 item 7, 0060 item 5), an application is guarded by `G` (item 7), every application of the function
     itself begins a line of its own (the map is keyed by line: two on one line could not carry two positions).
   - **One state.** The function's `ensures` and body are lowered as its `requires` are, in the entry state with every
     `requires` site in view (the way a measure is lowered today); the contract's lowering context writes `wildcard` for
     an amount and leaves out the liveness `acc` (a flag on the context, not a new contract mode).

7. **Rows at an application, by mode** (0028 item 11: every obligation is an explicit row in source terms; the callee's
   Viper precondition is only the `internal` backstop). **The framing rows of the callee's atoms come first, then the
   atoms' pure lines and the value clauses of its `requires`**, each with the arguments substituted (a failed framing,
   assumed, then ends the path of a row that depends on it: one error); **in a `check` the pure lines come first**
   (question 278, below):
   - **The guard `G`** of an application is the conjunction of (i) for each atom of the callee, the disjunction over the
     enclosing contract's unmarked sites of that cell field of "the site's guard, the same block, and the callee's
     interval inside the site's" (a cell atom is an interval of one slot; **one site must hold the whole interval**: a
     listed limit; a marked site frames no application, as 0061 item 7 has it for quantified reads; and a loop that fills a
     `must_init` range while its invariant applies a function over the part not yet filled may time out, exit 3,
     instead of `uninitialized`: audit round 4's R4-S5, fail closed), and (ii) every
     other line of the callee's Viper precondition that reads no memory, at the arguments: the parameters' ranges, each
     atom's pure line, each value clause. The callee's element-range line follows from (i): a site that frames an
     application promises it. **One helper computes `G`'s conjuncts for the guard and for the rows**, so the guard can
     never be stronger than what the rows assert (as built: one list of `requires` lines per function, `pf_lines`, is
     the function's own Viper `requires`, the guard `G` at an application and fact (ii)'s `V`). **The rows'
     categories** (as built): a framing row `permission`; an atom's pure line and a callee's value clause
     `precondition`, with a note at the callee's clause; the parameters' ranges and pointer shapes `internal` (the
     arguments' own obligations come first, so they hold). Where several rows of one application fail at one line
     (a count past an array fails four pure lines and a value clause), the first reports and is assumed afterwards.
   - **A `cerastes_forall` clause of the callee's `requires`** (cut 2) is, at an application, **the quantifier it would
     be if it were written at the site with the arguments in place of the parameters**: lowered by 0061 in the site's
     mode (a contract's quantifier with its `ite` chains; in `f__wf` over the enclosing method's `hvq` ghosts; in a loop
     invariant and a `check` with 0061 items 9 and 10's rows), asserted there as one `precondition` row, and standing as
     one more conjunct of `G` where the application is guarded. Its trailing arguments must then be what 0061 item 3
     admits (an application that passes an argument that reads memory to such a parameter is rejected, M4.5e2). In fact
     (ii) of item 8, `V` holds the clause as `f__wf` lowers it at the arguments. S46 (h) gates all of it.
   - **A method's contract (`CMode::Method`), a `pure` function's body and contract:** the application is `(G ?
     pf_g(args) : d)`, `d` the type's default, so that Viper's own check of the contract or the function can never fail;
     the rows are the `__wf` method's (item 8): per atom one `permission` row ("the contract of `f` calls `g`, which
     reads `a[0..n)`, without owning them"), per value clause one `precondition` row ("the contract of `f`
     calls `g`, whose `requires` may not hold: `0 <= n`", with a note at the callee's clause).
   - **A call and a return** (`CMode::Call`, `CMode::Ret`): the clause's `precondition` or `postcondition` rows as today
     (one per clause, per conjunct where `push_clauses` splits): the clause is lowered again over the arguments, so the
     application inside takes the form the method's contract gives it, read in the caller's state after `call_atoms`'
     rows (it writes no quantifier there, so no base needs binding). Callers learn an `ensures` from the method's
     postcondition, as today.
   - **A loop invariant (`CMode::Inv`):** before the user's clause, in evaluation order, the framing of each callee atom:
     **its interval lies inside one place the loop holds** (a range place or an array place; no quantifier), asserted at
     entry and at the back edge as a read's framing is (`permission`, "this loop invariant calls `g`, which reads `a[0..i)`, which this loop does not hold"); then one `obligation_clause` invariant line per obligation: a named
     array's bound (`out-of-bounds`), the flags over the interval in 0061's `q0` form, guarded by the framing
     (`uninitialized`), each value clause of the callee (`precondition`); the application in the clause is guarded by the
     same inclusion and value clauses, as `inv_elem` guards a read. A loop whose invariant holds an application of a
     function with an atom carries its places (`expr_touches`). **An unframed application is more than one error**
     (S46 (d), question 277): the framing `assert` fails at entry and at the back edge, and where the guarded
     application's default breaks the clause (`s == 0` while `s` grows) the clause also fails (`invariant-preserved`);
     an application that reads one element too many and states the wrong value (`s == sum_to(a, i + 1)`) adds
     `invariant-entry`, and where the method's `ensures` rests on the invariant, the `postcondition`. Each is real; the
     tests expect every error the driver prints (question 254's rule). Two arrays with a pair of places need the
     entry case split `if (lvloc(block_of(b)) == lvloc(block_of(a))) { }` before the snapshots, as 0060 item 11 has
     it (S46 (d)'s copy loop).
   - **A `check` (`CMode::None`), and a `do … while (0)` loop's invariant (question 284):** **first the callee's parameter ranges and pointer shapes at the arguments** (audit
     round 1, S5: S46 (f) writes them as rows; without them the native `application.precondition` would fail, an exit
     3), then per callee atom **its pure lines** (`p != null`, the shape, `0 <= n`,
     `offset_of(p) + n <= len(block_of(p))`; `precondition`, question 278: in the planned order a count past a local
     array was two errors, the bound row and the pure line, and with the pure lines first one), then the rows of 0061
     item 10 over its interval, in the binder form the lint admits (the null row and the liveness row for a pointer
     base, the bound, the marked rows of `v_T` and `d_T`, the flag row), then the element-range row (`internal`), then
     one `precondition` assert per value clause, then the fact with the bare application (a failed row is assumed
     afterwards, so the native check is silent; S46 (f)).
   - **What establishes each line of the callee's precondition, by mode** (a line with no entry would be an unmapped
     native error, exit 3):

     | Mode | pure lines, value clauses | `acc` of `v_T` | element range |
     |---|---|---|---|
     | a `pure` function's own body | `G` | its own `requires` | its own `requires` |
     | a method's contract | `G` | the framing site (unmarked) | the site's range line |
     | a call, a return | `G` over the arguments | `call_atoms`' and the return's marked rows | their range rows |
     | a loop invariant | `G` over the places, the atoms' pure lines and value clauses also as `precondition` rows (question 283) | the place's conjuncts | the place's range conjunct |
     | a `check` | the `precondition` rows, the parameters' ranges and pointer shapes among them (audit round 1, S5) | the marked rows | the `internal` row above |

8. **`f__wf`** (amends 0037 item 7 and 0061 item 7; variant M stays: the method is heap-free and applies no function).
   An application is a ghost function of the method's domain, `hva_<m>__<g>_<pre|post>(args)` (`<m>` the `__wf`
   method's name without `__wf`, `<g>` the callee's full Viper name: `hva_f_kern__11__pf_sum_to__1_post`, and in a
   `pure` function's own check `hva_pf_wrap__3__pf_small__2_pre`; a function's definition check is the method
   `pf_<name>__<id>__wf`), one parameter per C parameter (`Int`, `Bool`, or `Ref` for a pointer) and the callee's
   result type; the domain keeps its tag and holds no
   axiom (a method with an application and no quantified read has the domain too). Assumed as `requires` of the `__wf`
   method, each ground at one application the contract writes:
   - **(i) the result's type range** (a `bool` none), unguarded: the definition check of `g` (item 9) shows that no
     operation of its body leaves its type, so the value is one of its result type wherever the application is defined,
     and the fact is satisfiable everywhere;
   - **(ii) the callee's `ensures`** at the arguments, **under `V`, the callee's value `requires` and pure lines at the
     arguments** (a quantified clause of the callee, cut 2, as item 7 lowers it over the ghosts), when that `ensures`
     clause reads no memory and holds no quantifier and no application (otherwise it is left out: a false alarm, never a
     proof). **The guard is what makes it sound:** a fact is a `requires` of the method and holds on every path, and an
     `ensures` at arguments outside the callee's `requires` may be false (`sum_to`'s bounds at `n - 1` where `n == 0`),
     which would make every row vacuous (S46 (c) (2u)). Under `V` it is a theorem: where `V` holds a heap satisfying the
     callee's full precondition exists, Silicon proved the `ensures` there, and the clause reads no memory. For an
     application of the function inside its own body this is the induction hypothesis, sound because the function's
     termination is checked (item 10);
   - **(iii) the keep fact** (cut 1): for a post-state application whose arguments read no memory, whose every pointer
     argument that a callee atom names is a bare variable (as built: otherwise the fact is left out, a false alarm), and
     whose every callee interval lies inside a `reads` or `reads_range` atom of the method that has an `ensures` partner
     (a cell atom is an interval of one slot: audit round 3, S1), under both atoms'
     guards and the inclusion, `hva_…_post(args) == hva_…_pre(args)` (the function's value depends on its arguments and
     on the cells its atoms name, and a `reads` place is not written between entry and return: 0061 item 7 (d)'s
     argument).
   - **Narrowings, each fail closed** (a fact left out only adds a false alarm; owner, question 279: listed limits,
     M4.8): fact (i) is written for an integer
     result only (a `bool` has none), **not for an enumeration result** (its range is 0018's value range, which the lint cannot
     tell from an unguarded fact (ii)); fact (ii) is not written for a callee whose `requires` holds an application
     (its guard `V` would hold one, which `f__wf` cannot compare), nor for an `ensures` clause that reads memory, holds
     a quantifier or holds an application.
   - Two applications of one function in one state with equal arguments are equal by the ghost function itself.
   - Rows: `prior ==> conjunct` for each conjunct of `G`, in the categories of item 7. A failed row is assumed
     afterwards, as every `f__wf` row is. **Listed limits** (false alarms, fail closed): the definition of `g` is not
     known in `f__wf` (an obligation that needs it, such as `sum_to(a, 2) - a[0] - a[1]`'s overflow under a bound on the
     elements, fails); an `ensures` of `g` that reads memory is not assumed.

9. **The definition check of a read `pure` function.**
   - **Every read function gets its `__wf` method**, `pf_<name>__<id>__wf`, whether or not it has a contract (audit
     round 1, S3: today `run()` makes one only for a function with a clause; the body is always a clause here, and the
     lint requires the method: without it neither fact (i) nor the argument below has a basis).
   - **A read function counts as one function verified** (amends 0011 item 9; owner, question 281): the success line's
     `n` counts each read `pure` function besides the verified bodies, so a file whose only functions are read `pure`
     functions verifies and is not "nothing to verify".
   - **Its `__wf` method** (`wf_method`, extended): the `requires` clauses; then **the body as one more clause of the
     entry state**: every obligation of `E` (0010 item 11's, each read's framing by the function's own atoms, each
     application's rows by item 7 with the ghosts of item 8, the conversion at the `return`), reported **in the
     function, at the operation**; then the `ensures` clauses' own obligations, read in the same state (a `pure` function
     has one state: no post ghost, no partner row, no keep fact); then the measure's (0013 item 7).
   - **Silicon's own check of the Viper function** is the proof of its `ensures` and of its termination. These two are
     reported to the user from it (item 11); nothing else of that check can fail in a program whose `__wf` rows hold,
     and anything else it reports is an internal error (exit 3).
   - **Why the function's value is the C value** (audit round 1 checks this argument): by induction on the measure. The
     Viper body computes in unbounded integers; the `__wf` rows show, under the `requires` and with every inner
     application's `ensures`, that no operation of `E` overflows, divides by zero, converts out of range or reads what it
     may not; so the two values agree, given that they agree for every inner application, whose measure is smaller.

10. **Termination** (amends 0013 items 2, 4, 5 and 8). The encoder records the calls among read `pure` functions, from
    bodies and from contracts (`edges_` holds code calls only). A function that calls itself in its body needs a
    `decreases` (`missing_decreases`, its message for a `pure` function); **every other cycle is rejected**: two or more
    functions, or a function reached again through a `requires` or an `ensures` (`unsupported_annotation`, M4.5f2: a
    cycle through a contract would escape the termination check, and the facts of item 8 would rest on themselves).
    Silicon's termination plugin checks the Viper function against its measure; a failure is `termination`, at the
    recursive call, with a note at the measure ("the measure of `g` may not decrease at this call, or may not be bounded
    below by zero"). An ordinary function's recursion cycle (0013 item 4) is unchanged: an application is not a call of a
    method.

11. **The two native rows.** On each `ensures` line of a Viper function: `postcondition.violated` → `postcondition`, at
    the function's `return` (as built: the position of the returned expression; the measure's map entry is there
    too), with a note at the clause ("`g` may not satisfy its `ensures`"). On each line of its body
    that begins with an application of the function itself: `termination.failed` → `termination` (item 10). Both through
    the existing reasons (`assertion.false`; the three termination reasons); `report.cpp` and the map's format do not
    change. Every other row on a function is `internal`.
    - **One accepted warning** (amends 0011 item 9; owner, question 275). When a memory read or an application in a
      `pf_` function's body or `ensures` stands on a branch that no path satisfying the function's `requires` reaches,
      Silicon prints, before its verdict, `Could not resolve expression <e> (<file>@<l>.<c>--<l>.<c>) during the
      axiomatisation of function <f>. This typically happens if the expression is on an infeasible path, i.e. is dead
      code. The unresolved expression will be replaced by a fresh symbol, i.e. an arbitrary value.` It does so beside
      "verified" for a correct function (`requires n >= 0`, body `n < 0 ? a[0] : 0`) and beside the `__wf` row of a
      wrong one (an application whose guard the `requires` make false). The driver (`silicon_output.cpp`) and
      `run_tests.py` accept exactly that line, **only before the verdict line** (where Silicon prints it; after the
      verdict it is a stray line as before, and after an error entry it joins that entry's text as any continuation line
      does, in both readers: audit round 1, N2), when `<f>` is a `pf_<name>__<id>` function, whatever the
      verdict; any other warning, and this one for any other function, stays exit 3 (both readers share one rule and
      the harness's agreement tests run both). **Why it is sound** (S46 (b), M5, from the
      pinned Silicon's source; audit round 1 checks it): the line is printed only when no explored path evaluated the
      expression (`HeapAccessReplacingExpressionTranslator.getOrFail`); an arm is left unexplored only when an SMT
      check proves its path condition unsat (`Brancher`; an `unknown` or a timeout explores it) or the path is
      unreachable; the fresh symbol replaces the expression only in the body's translation inside the definitional
      axiom `pre(args) ==> f(args) == body(args)`, which Silicon emits only for a function that verified; so for every
      argument satisfying the `requires` the axiom's value never depends on it. The same holds on an `ensures` line
      (measured, M4): an expression of the postcondition that no path satisfying the `requires` evaluates is replaced
      in the postcondition's translation, which is guarded by the same precondition. For a body expression Silicon
      prints nothing when the function's own check already failed (the postcondition's axiom is computed before that
      check, so an `ensures` expression may still print it: harmless, since the line is accepted whatever the verdict).
      **Audit round 1 traced the rest** (N1): the default function encoding adds no auxiliary axiom; the `requires`
      expressions enter no axiom (each axiom's antecedent is the uninterpreted `f%pre`, assumed only after the callee's
      precondition is consumed); recording happens only at evaluation depth 0, and depth rises only while consuming a
      callee's precondition or unfolding a predicate (e1d has none), so a body or contract expression is always recorded
      at depth 0; snapshots are keyed by (AST node, context) with structural equality, so a dead occurrence equal to a
      live one resolves to the live snapshot and prints nothing.

12. **The lint** (amends 0012 item 11, 0061 item 11).
    - A top-level `function` is a prelude function (by name, its body under today's prelude rules, and an empty
      `decreases` line admitted where the file holds a `pf_` function and only there), `pf_<name>__<id>` (`<id>` digits, question 274), or a function
      that applies only prelude functions (question 273, "H'": it can neither recurse nor reach a `pf_` function, so
      every cycle goes through `pf_` functions, whose rules follow; the emitter writes no such function, and the lint's
      self-test fixtures use them). A `pf_` function: bodied; one `decreases` line, with a measure or empty, never `_`
      or `*`; `requires` lines in exactly the shapes of item 6 (the
      form S46 (a) built; `wildcard` is admitted there and on 0061 item 10's marked row, nowhere else); `ensures` lines
      hold no `acc`, `perm`, `old`, `inhale`; the body holds no `acc`, `perm`, `old`, `forperm`, `inhale`, `assume`.
    - `pf_` applications: in a `pf_` function, a method's `requires` and `ensures`, an `invariant`, an `assert`; **never
      in a `__wf` method**, never in a `decreases` line, never in the bounds, base, amount or guard positions
      `_heap_free` protects: **`HEAP_READ_RE` counts a `pf_` application as a heap read** (handoff §3, e1a round 2 item
      6).
    - The ghost domain of a `__wf` method may hold `hva_<method>__<callee>_<pre|post>` functions of `Int`, `Bool` and
      `Ref` parameters; the assumed facts over them have exactly the shapes (i)–(iii) of item 8, the `ensures` fact
      under `V` and the keep fact under its guards (each unguarded form is a must-fail self-test, as 0061's unguarded
      keep fact is).
    - Map rows of a function: each `requires` line `not.wellformed` (`internal`); each `ensures` line
      `postcondition.violated` (a user row) and `not.wellformed` (`internal`); its `decreases` line with a measure keeps
      the `termination.failed` entry today's rule asks for (at the function's `return`; Silicon does not report there,
      S46 (a)); each body line that begins with an application of the function itself needs the `termination.failed`
      entry of that call, and no other body line may have an entry. Every `pf_` function has a `decreases` line (today's
      rule reads methods and loops only).
    - A user quantifier of 0061's shape is admitted in a `pf_` function's `requires` and `ensures` (today's rules read
      methods only) and, for a callee whose `requires` holds one (cut 2), as a conjunct of an application's guard and,
      over the ghosts, in the antecedent of fact (ii).
    - **What the lint checks of each fact** (audit round 1): fact (ii)'s guard `V` is exactly the callee's value
      `requires`, pure lines **and its quantified clauses (cut 2) over the method's `hvq` ghosts at the arguments**, as
      a set of conjuncts (B1: a `V` without the quantified clause is the unsound form S46 (c) (2u) shows, and is a
      must-fail self-test; `audit1-scratch/cx1_vnoq_unsound.vpr` is its evidence); the keep fact's guard is the
      conjunction of the method's `reads`/`reads_range` atom guard, its `ensures` partner's guard, the same block and
      **the inclusion of every interval of the callee's atoms at the arguments in that atom's interval**, read from the
      callee's own atoms (S1); fact (i)'s range is the callee's C result type's, which the `pf_` function's signature
      line states as a tag `// cerastes:result <lo> <hi>` that the lint checks against a standard C integer range (S2;
      the lint cannot see the C type itself, so the tag and the fact are checked for agreement); every `pf_` function
      has its `pf_<name>__<id>__wf` method (S3); the prelude functions apply no function but prelude functions, and
      their application graph has no cycle (N9: H' rests on it).
    - Each rule with twins.

13. **Categories:** none new. **Tags:** `pure_function`. **Facts:** P81, S46.

## Facts and the branches taken

Each probe and spike of the plan's section 5, its outcome (`internal/il-inventory.md`, chapter M4.5e1d), the branch of
its decision table, and the item it revised. An outcome outside its table stops the session and asks the owner.

| Row | Outcome | Branch taken | Item revised |
|---|---|---|---|
| P81 (a) a call in each annotation position | an `eok_call` over `enk_routine g` in a `requires`, an `ensures` (the result name an `enk_param_ref`), an invariant and a `check`, each argument as written; contract parameters `enk_param_ref`, loop and `check` names `enk_variable`s; C23 and C++20 trees identical but for `int`/`bool` comparison types | Expected | 4 |
| P81 (b) a call inside a `pure` body | the same `eok_call`, also in a `?:` arm and an operand of `+`; a self-call's callee is the routine being read; parameters are `enk_variable`s mapped by `param_num` through the scope's `parameters` chain; the front end raises nothing. Side fact: the IL's `called` flag is false for a function only annotations name and true once another `pure` body calls it, so the read set is computed by walking the annotations, never from that flag | Expected | 2, 3 |
| P81 (c) contracts on a `pure` function | `requires`, `ensures(r, …)` and `decreases` on the routine's `attributes` chain as on any function, on a definition, a prototype or both (one chain); a mismatch is the front end's "does not match the attributes of an earlier declaration" | Expected | 3 |
| P81 (d) argument shapes | `int *` for `const int *` and `int` for `long long` one generated `eok_cast` each; `a[i]`, `*p` as written; a named local array for `const int *` in an invariant and a `check` a generated cast over a generated `eok_array_to_pointer` (two nodes), the shape a code call has and the one e1c's `forall_check_local.c` already reads; both modes identical | Expected (the shape code has; the two-node decay as e1c reads it) | 4 |
| P81 (e) `static_assert`, a local declaration, `constexpr` in a `pure` body | a block-scope `static_assert` is a `stmk_decl` **statement** before the `return`, whose entity is an `iek_static_assertion` with its condition folded (the Expected column's "not a statement" does not hold); a local variable is `stmk_decl`, `stmk_init`, `stmk_return`; `constexpr` gives `is_declared_constexpr` in C++20 (the body present); in C23 a `constexpr` function does not parse | the reader sees every declaration, so the "If not" (a declaration it cannot see) does not arise. Two further probes: (e2) the same `stmk_decl` shape in an ordinary body, both modes, where `declaration()` passes each `iek_static_assertion` entity to `encode_block_static_assertion` (which takes the entity alone); (e3) `int y;` is a lone `stmk_decl` too, told apart by its entity's kind (`variable`). So item 3 takes its "pass it" branch: the assertion is checked as in code | 3 |
| P81 (f) `old` over a call | the front end refuses `cerastes_old(g(…))` in both modes and `cerastes::old(g(…))` in C++ (`enk_temp_init`, as P80 (b)); the Expected column's "C++: accepted" does not hold | "If not": recorded; no mode compiles, so the pin is a front-end negative, and `check.cpp`'s backstop is pinned by a VIR rejection case | 1, 4 |
| S46 (a) the function and Silicon's own checks | 14 fixtures `s46_a_*`, 2.2–3.2 s each (load 4.5–6.6): `pf_sum_to` verifies in form **Min** and in form Full; vacuity one error each; a wrong `ensures` bound one `postcondition.violated` at the `ensures` line; `pf_sum_to(a, n)` as the self-application one `termination.failed` (`tuple.false`) at the application's line; `decreases n - 5` one (`tuple.false`, not bounded) there; with a real `ensures` the self-application twin gives three (two `postcondition.violated`, one per conjunct, and the termination one: recorded for the count); **no `decreases` line on the recursive function: verified** (Silicon checks nothing: the evidence for the lint rule); **an empty `decreases` on the recursive function: reported, as two Silicon consistency errors** ("function `decreasing` is required but not declared", "`bounded` … not defined"), an exit 3 in the driver; a non-recursive function with an empty `decreases` that applies `c_and_32` one `termination.condition.false` at that application, one that applies another `pf_` function verifies, and with an empty `decreases` on `c_and_32` both verify; a function applying itself in its own `ensures` does not finish in 60 s (recorded only; rejected by item 10 and the lint) | **Min**; the two native rows as item 11; the prelude functions' empty `decreases` where a `pf_` function is in the program (recorded, no stop); the empty-`decreases` backstop holds (reported, fail closed) | 6, 10, 11, 12 |
| S46 (b) the guard | 11 fixtures `s46_b_*` (the two body twins entered after question 275), 2.4–2.7 s: `kern`'s `ensures r == (G ? pf_sum_to(a, n) : 0)` with its heap-free `__wf` (the ghost `hva_kern__pf_sum_to_post`, facts (i) and (ii)) verifies; `pf_sum_to(a, n + 1)` in the contract one error, the `__wf` framing row, no native error; a callee value `requires` (`n <= 1000`) the contract does not give one error, its row, no native error, and the same inside a function's body one error in the function's `__wf`; the default is not observable (`check(r == 0)` after the bodiless method fails once, `check(r == pf_sum_to(a, n))` verifies). **Outside the table:** `pf_sum_to(a, n + 1)` in a function's BODY under its (unsatisfiable) `G` gives its one `__wf` row and no native error, and Silicon also prints an axiomatisation warning ("Could not resolve expression … replaced by a fresh symbol") before its verdict, which the driver and the harness refused (exit 3). Measured (M1–M5): the warning also stands beside "verified" for a correct function whose own branch is dead under its `requires`, for an application and for a plain read, and for an unsatisfiable `requires`; the bare form S gives two native errors and no warning; it stands on an `ensures` line too; Silicon prints it only for an expression no explored path evaluated, an arm being unexplored only when an SMT check proves it unsat, and suppresses it when the function's own check failed | form **G** (item 7); the warning asked: question 275, accepted for `pf_` functions (item 11) | 7, 11 |
| S46 (c) `__wf` ghosts | 16 fixtures `s46_c_*`, 1.9–3.0 s, with item 12's names: (1) the overflow row of `sum_to(a, n - 1) + a[n - 1]` passes with (ii) and fails once without the function's `ensures`; (2) `ensures r == sum_to(a, n) + 1` passes with (ii), fails once without; (2u) (ii) without `V` lets a wrong base case verify, under `V` it fails alone; (3) the plan's literal program verifies **also without (iii)** (fact (ii) bounds the sum), the adapted `+ 9223372036854775000` passes with (iii) and fails once without (the `+` overflow row); (4) the unguarded keep fact with an `owns_range` alias under a false guard verifies a wrong contract (the defect, kept as evidence), under `(c != 0) && inclusion` it fails alone; (5) vacuity one error each | (1), (2), (2u), (4), (5) Expected; (3) by the table's letter cut 1, asked: question 276, kept with the adapted discriminator | 8 |
| S46 (d) the loop (protected) | 19 fixtures `s46_d_*` (the multi-error twins entered after question 277), 2.4–5.3 s: the read-only `sum` loop with `invariant s == (G ? pf_sum_to(a, i) : 0)` verifies **unaided** (no written step) in Min and in Full; a loop that also copies `a[i]` into an owned `b` verifies (with 0060 item 11's `lvloc` case split); a local array of 4 in one function verifies; one error each for the invariant `+ 1` (`invariant-entry`), the body adding `a[i] + 1` (`invariant-preserved`), a write of `a[0]` inside the summed prefix (`invariant-preserved`), no bound in the function's `ensures` (the `s + a[i]` overflow; two errors with the function's `__wf` in the file, as the table says); **`s == sum_to(a, i + 1)` five errors** (entry framing, `invariant-entry`, `invariant-preserved`, back-edge framing, the return's postcondition; four with an any-value `ensures`), and a framing-only twin three (both framing asserts and `invariant-preserved`). Recorded: `ensures r == 10` over `{1, 2, 3, 4}` is not proved, and `check(sum_to(a, 2) == a[0] + a[1])` needs the written step `check(sum_to(a, 1) == a[0])` (one unfolding per element) | **unaided**; the multi-error twins asked: question 277, "Expect all" | 7 |
| S46 (e) calls and returns | 10 fixtures `s46_e_*`: a caller of `sum` on a local array, `x = sum(a, 4)` with `call_atoms`' rows and then `check(x == sum_to(a, 4))` with item 7's `check` rows, verifies (2.6 s); a write of `a[1]` between the call and the `check` one error at the fact (27.1–29.5 s: the slowest S46 program, under its row's and section 11's limits); a callee with `requires(sum_to(a, n) < 100)` and a caller that gives it verify, the `precondition` row after `call_atoms`' rows over the call's arguments; a caller that cannot prove the bound one error at that row, no native error; `sum_to(a, 4)` read before and after a call that takes and returns `reads_range(a, 4)`, and before and after a write to another array, keeps its value in Min and in Full (no discriminator between the forms) | Expected | 7 |
| S46 (f) `check` | 11 fixtures `s46_f_*`: on a local array, an `owns_range` and a `reads_range` parameter the rows of item 7 verify (2.1–2.3 s; the marked rows accept a read fraction); one unwritten element one error, the flag row (the element-range row does not fail beside it); `sum_to(a, 0 - 1)` one error, the `0 <= n` row; a false fact one error; a range a callee kept one error, `insufficient.permission` at the marked `v_int` row; the native `application.precondition` silent in every program. **A count past the array (`sum_to(a, 5)` over 4) two errors**, the bound row and the callee's pure line `offset_of(a) + 5 <= len(block_of(a))`; with the callee's pure lines asserted first, one (the pure line), every other (f) outcome unchanged (the negative count now at the pure line `0 <= n`) | Expected; the count-past twin asked: question 278, "Pure lines first" | 7 |
| S46 (h) a quantified bound | 8 fixtures `s46_h_*`: `int sum_to` with `requires(n <= 1000 && reads_range(a, n) && cerastes_forall(small_at, a, n))` and `ensures(-1000 * n <= r && r <= 1000 * n)`: the function, its `__wf` (the scalar tie, the quantified clause over the ghosts, fact (ii) under a `V` holding the clause) and the loop with the clause as an obligation line and `invariant s == (G && Q ? pf_sum_to(a, i) : 0)` verify (2.6–3.1 s); without the quantified `requires` one error, the body's overflow row in the function's `__wf`; an enclosing contract that applies the function without giving the clause one error, its `Q` row, no native error | Expected: cut 2 not taken | 3, 7, 8 |
| S46 (k) cost | (d)'s two programs 2.9 s and 2.8 s; a function applied in two invariants of one loop 3.1 s; three functions in one contract 2.9 s; S45 (k)'s scale program at K = 16 with a local array and one application in a `check` 13.7–15.0 s (load 4–6) | recorded (all under 60 s) | — |
| Step 0 R1 the pins today | on the frozen base build, both modes: `forall_reject_f_{requires,ensures,decreases}.c` `unsupported_annotation @ L16`; `forall_reject_e_call.c`, `reject_call_in_annotation.c`, `reject_atom_in_check.c` `call_in_annotation`; `forall_reject_namespace_check.cpp` `unsupported_annotation`; the `pure`/`predicate` rejections of `tests/memory/negative/` as their expect lines; `static_assert_pure_body.c` verified; `array_loop_bool_post.c` `postcondition @ L23` | the baseline of Flips B | — |
| Step 0 R2 today's diagnostics | a call of a `pure` function in a `requires`, an `ensures`, an invariant, a `check`, a function's and a loop's `decreases`, an atom's count, a `forall`'s trailing argument, inside `E`, and of a bodiless and a `constexpr` `pure` function: each `call_in_annotation` at the call; a recursive `pure` function, one with a contract and one with a wrong `ensures` that nothing calls: verified (not read; question 252's premise). `git grep e1d` over `src/`: one message (`encode.cpp:9737`). Four staging pins added, `tests/scalar/negative/pure_call_reject_{requires,ensures,invariant,check}.c` (`call_in_annotation` at the call, both modes) | the baseline of Flips A | — |
| Step 0 R3 the rejections | `scripts/rejections.py` on the frozen build: 2,477 runs (scalar 1,020, memory 1,126, vlibc 70 `rejected`; 261 front-end errors), none failing | the baseline of the plan's section 9 | — |
| Step 0 M the compiler matrix | Apple clang 21 and GCC 16, both modes, three tiers, 108 cells: a recursive `CERASTES_PURE` function with `requires`, `ensures(r, …)` and `decreases`, with and without a prior prototype, the `long long` sum with `-2147483648LL * n`, a `bool` `pure` function a `forall` names, applications in contracts and checks: clean; the only warning is `-Wunused-parameter` at `-Wall -Wextra` for a parameter used only in a contract (an ordinary function; `(void)n;` is clean) | the plan's rule: tests write every parameter used (an ordinary function with `(void)n;`; a read `pure` function, whose body is one `return`, names every parameter in `E` or marks it `[[maybe_unused]]`); nothing new rejected | 3 |

## Consequences

- A `pure` function that an annotation calls, or that carries a contract, is a Viper function whose definition is
  checked once: its body's operations, its `ensures` and its termination.
- Contracts, loop invariants and checks can name a value computed from an array; the exact sum of an array verifies
  with a loop whose invariant applies a recursive `pure` function.
- Every obligation at an application is an explicit row in source terms; Viper's own checks of an application never
  fail in a program whose rows hold.
- Programs accepted before e1d keep their Viper and maps byte for byte.

*2026-11-25, amended by 0063 (M4.5e1e1): item 4 — a named global array is an argument of a `pure` application in a loop invariant and a `check` (0063 item 1).*
