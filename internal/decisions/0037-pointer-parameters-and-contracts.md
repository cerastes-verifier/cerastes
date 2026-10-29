# 0037 — Pointer parameters, results and contracts: permissions across calls (M4.5b1b)

- **Status:** accepted for M4.5b1b (2026-10-28), written before code (invariant 6) and revised after audit round 1
  (Opus, on the design: two blocking items, both in item 7's framing, and seven should-fix items, all taken), after
  S27 (one form of item 7), and after audit round 2 (on the code: three blocking items, `reads` pairs at entry, a pointer
  `?:` in a guard and `restrict` aliasing through the contract; two should-fix items; all taken).
  Owner decisions (M4.5b1b planning, 2026-10-28; plan `.claude/plans/m4-5b1b-pointer-params.md`):
  - **No cuts, no time cap:** pointer results and `&param` stay in this session, as handoff §4.4 planned.
  - **The framing of contracts is decided per path by Silicon**, not by a syntactic rejection (item 7). Of the two
    realizations offered, the planning's design check found one that needs no new driver reason: an explicit check in
    the contract's well-formedness method, with Silicon's own check kept as a backstop.
  - **A missing permission at a call or at a return is category `permission`** (not `precondition`/`postcondition`).
  - **`restrict`** (handoff question 64): accepted on a parameter whose contract owns its pointee, rejected everywhere
    else (item 2).
- **Milestone:** M4.5b1b (2026-10-28).
- **Implements:** record 0028 items 1 (`&param`), 2 (`WF`), 3 (`pointer-comparison`), 3a (liveness across calls), 4, 7,
  8, 9, 11 for parameters and results; record 0029 items 3 and 5 (`owns`, `reads`, `live`, `old` in both spellings).
- **Amends:** 0008 (the header's C forms and `live`), 0011 items 9–10 (`pointer-comparison`; `permission` at calls and
  returns), 0012 item 12 (pointer parameters and results accepted), 0028 items 4, 8 and 11, 0029 items 3 and 5, 0032
  items 2, 4, 6, 7 and 10.

## Context

Record 0032 (M4.5b1a) let a function hold pointers to its own address-taken locals and rejected everything that needs
a contract. This record lets permissions cross calls: a pointer parameter or result, the spec functions that describe
what a function owns or reads, the ghost amounts that carry fractions, and the checks that keep each call and each
return's failure an explicit, path-precise error in source terms (0028 item 11). Facts: P53 (the C spelling's IL), P58
(pointer parameters, results, `&param`, the spec functions in both modes) and S27 (Silicon's well-formedness of
contracts, call sites, ghost amounts), in `internal/il-inventory.md`, "M4.5b1b".

## Decision

1. **Scope.**

   | | b1b (this record) | b1c |
   |---|---|---|
   | Objects | pointer parameters (`restrict` with `owns`), pointer results, `&param` | `T**`; jumps out of a cell's scope; cells in loops |
   | Contracts | `WF`; `owns(*p)`, `reads(*p)`, `live(p)`, `old(*p)`, `*p` in both spellings; ghost amounts; framing per path | `[[cerastes::owning]]` (`p`, `r`), `owns_block`, atoms on `*result` |
   | Checks | calls, returns, entry separation, case splits | sequencing on objects (`unsequenced`), 0028 item 13's placement |
   | Categories | `pointer-comparison`; `permission` at calls and returns | `unsequenced` |

   Everything outside b1b stays rejected with a diagnostic naming the session that lifts it (item 15).

2. **Pointer parameters.** A parameter may have any pointer type 0032 item 2 accepts for a local. It is a `Ref` formal
   with `requires WF(p)` (0028 item 2), and its local copy is a `Ref` Viper local (a cell only through `&param`, item 4).
   **`restrict`** on the pointer itself (`int *restrict p`) is accepted only when the function's `requires` holds
   `owns(*p)` as a top-level conjunct or as `p == NULL || owns(*p)`, its `ensures` returns the same atom (item 6), and
   **every other pointer parameter of the same pointee type** has its own `owns` or `reads` atom in the `requires`,
   unconditional or after `q == NULL ||`. That is sound: every access in the callee needs permission; full permission to
   a cell excludes every other holder; a caller cannot lend two full permissions, or a full permission and a fraction,
   of one cell (item 8's checks); and the other same-typed parameters, holding atoms of their own, are proved distinct
   from `p` by the permissions (or the `requires` is unsatisfiable and no caller meets it). Audit round 2, B3:
   permissions belong to cells, not to pointers' provenance, so without the last condition a contract that lets `q ==
   p` lets `q` use `p`'s permission (`void f(int *restrict p, int *q) requires(owns(*p) && p == q)` writing `*p` and
   `*q` verified). Parameters of another pointee type never reach `p`'s cell (0028 item 1). So an access to `*p`'s
   object through a pointer not based on `p` cannot verify, which is what C23 6.7.3.1 forbids. `restrict` stays rejected
   on a local, on a result, on a parameter with only `reads` or `live`, beside a same-typed pointer parameter without an
   atom of its own, and on a pointee (`int *restrict *`). It is C only (`__restrict` in C is the same qualifier and is
   treated alike): EDG, clang++ and g++ refuse `restrict` in C++20 (P58; `__restrict` there is rejected), and the encoder reads the qualifier
   from the parameter's variable type, since C++ strips top-level qualifiers from the parameter type.

3. **Pointer results.** A function may return any pointer type 0032 item 2 accepts. Its method has `ensures
   WF(result)`. A contract may relate the result to the parameters and to NULL (`ensures(result == p || result ==
   NULL)`); an atom on `*result` is rejected until owning results (b1c, M4.5d2). Silicon does not branch on a
   disjunction it inhales, so a caller that dereferences `r` known only as `r == p || r == q` would read through an
   ambiguous receiver and fail with Silicon's own reason (found end to end, exit 3): after a call whose result is a
   pointer, lowering writes the no-op case split `if (r == X) { }` for each cell in scope and each pointer parameter
   with an atom of the same field (0028 item 8's device), and at entry the same for each pointer parameter without an
   atom of its own against those with one. The returned value is a use: it gets
   the liveness check of 0032 item 6 at the `return`, after its value is computed and before the scope exits (0032 item
   8), so `return &x;` passes there; the caller's first use of a result whose object has ended fails `lifetime`.

4. **`&param`** (0028 items 1, 4; amends 0032 item 4). A parameter whose IL variable is `address_taken` is copied at
   entry into a fresh one-slot block (`alloc_<t>()`, then a store of the formal's value, the flag set), and the block's
   scope exit is the function body's (fall-through and every `return`, 0032 item 8). Contracts keep reading the formal,
   the parameter's entry value (0006 item 5). The VIR checker's refusal of an address-taken parameter copy is lifted for
   this shape.

5. **The spec functions in contracts** (0029 items 3 and 5; amends 0008).
   - **Atoms:** `owns(*p)`, `reads(*p)` and `live(p)`, where `p` names a pointer parameter (in an `ensures`, its entry
     value). **Values:** `*p`, `old(*p)` (the cell's value on entry) and `old(p)` of a pointer parameter (which is `p`).
   - **Spellings:** C++ `cerastes::owns`, `cerastes::reads`, `cerastes::live` (new) and `cerastes::old`; in C and in C++,
     `bool cerastes_owns(...)`, `bool cerastes_reads(...)`, `bool cerastes_live(...)` (C23's variadic declaration without
     a named parameter; `_Bool` in C, `bool` and `noexcept` inside `extern "C"` in C++, so both modes name the same
     functions) and the macro `cerastes_old(e)` = `(*(__typeof__(e)*)cerastes_old_(&(e)))` over `const void*
     cerastes_old_(const void*)` (P53; `__typeof__`, since clang++ and g++ refuse plain `typeof` in C++20 while every
     compiler and EDG accept `__typeof__` in both modes, P58). `<cerastes.h>` declares them in both modes, and C++'s
     `template <class T> bool live(const T* pointer) noexcept;` in namespace `cerastes`; like every spec function they
     are never defined. The encoder recognizes them by entity (the C names at file
     scope, the C++ names in namespace `cerastes`), before 0006's `call_in_annotation`; in code they are
     `spec_call_in_code`. It reads the place under the compiler-generated promotion casts and the `old` macro's address
     forms (P53).
   - **`live(p)` implies `p != NULL`**, like `owns(*p)`: a function that accepts NULL writes `p == NULL || live(p)`.
   - **Positions.** An atom's guards are lowered as contract expressions (a pointer `?:` there is a Viper `ite`, never
     a branch; audit round 2, B2). An atom may appear as a top-level conjunct of a clause; as the right operand of `||` whose left
     operand reads no heap (lowered `!left ==> atom`: Viper refuses `acc` under `||`); or as an arm of `?:` whose
     condition reads no heap. Anywhere else (under `!`, inside `==`, as the left operand of `||`, as an argument) it is
     rejected (`unsupported_annotation`).
     Positions compose: the right operand of `||` and an arm of `?:` may hold a conjunction of atoms and reads (`p ==
     NULL || (owns(*p) && *p > 0)`). Two atoms of one kind on one place in the same clause list are rejected (`owns(*p)
     && owns(*p)` would make the `requires` infeasible; two `reads(*p)` would make the partner ambiguous), and so are
     `owns(*p)` and `reads(*p)` on one parameter in one clause list (audit round 2: `acc(p.v) && acc(p.v, a)` can never
     hold, and the body would verify vacuously).
   - **Still rejected:** `owns_range`, `owns_block`, `forall` over a pointer (M4.5e1, M4.5d2); places other than
     `*param` (`*local`, a global, `*result`, a member: b2, c, b1c); `*result` as a value (b1c, with owning results);
     `&x` and `&param` in a contract (0032 item 4's rule, kept); `&p` of a pointer parameter (`T**`, b1c); a heap read
     in a `decreases` (audit round 1: 0021 item 4's explicit check evaluates the caller's measure in the current heap,
     which is not the callee's entry value; b1c or later specifies `old` and framing for measures).
   - `cerastes_old(x)` of a by-value parameter does not make it address-taken (P53, P58): it stays a Viper local.

6. **Viper meaning and ghost amounts** (0028 items 7, 8, 3a, 14; amends 0028 item 8).
   - `owns(*p)` = `p != null && WF(p) && offset_of(p) < len(block_of(p)) && acc(p.v_T) && acc(p.d_T) && p.d_T &&
     lo_T <= p.v_T <= hi_T && acc(lvloc(block_of(p)).lv, a_k)` (the range for an integer or enumeration cell; none for
     `bool`). `reads(*p)` is the same with `acc(p.v_T, a_k) && acc(p.d_T, a_k)`. `live(p)` = `p != null && WF(p) &&
     acc(lvloc(block_of(p)).lv, a_k)`.
   - **One ghost `Perm` formal per atom** of the `requires`, `a_k`, with `requires none < a_k && a_k < write` written
     before every user clause. An `ensures` atom uses the amount of the `requires` atom with the same kind and the same
     place (the same parameter); an `ensures` atom with no such partner is rejected (b1c's owning results add the other
     direction).
   - **The caller's amounts:** `a_k := (g_k ? m_k : write) / (N + 1)`, where `g_k` is the atom's guard over the
     arguments, `m_k` is `perm(L_k)` (the liveness cell) for `owns` and `live` and `min(perm(V_k), perm(D_k), perm(L_k))`
     (the value, flag and liveness cells) for `reads`, and `N` is the number of atoms. The sum of what the callee takes from one
     cell is then below what the caller holds even when two atoms name the same cell (`f(p, p)`) and when the caller holds
     only a fraction (0028 item 8). A guard that is false gives `write / (N + 1)`, a positive amount the callee never takes.
   - **Return check of an amount:** `g_i ==> perm(X_i) >= a_i + Σ_{j≠i} (g_j && X_j == X_i ? a_j : none)` for each place
     `X_i` an `ensures` atom returns with a fraction (the liveness cell of every atom, the value and flag cells of
     `reads`); `owns`'s value and flag cells are checked `== write`.
   - The fixture `s25_a_reads.vpr` (one shared `rd`) stays as a verified alternative; `e2_swap.vpr` has the chosen
     per-atom form.

7. **The framing of contracts, decided per path** (owner). A contract may read `*p` only where an earlier atom gives
   permission to that cell on that path; Viper requires it (a method's contract must be self-framing), and cerastes
   reports a violation in source terms.
   - **In `f__wf`** (the contract's well-formedness method, `internal/encoding.md` §3): every heap read of the contract
     becomes a ghost value formal of `f__wf`, keyed by place and state, with its type's range as a `requires`: the
     **pre-state** value for reads in the `requires` and for `old(*p)`, the **post-state** value for plain reads in the
     `ensures`. `f__wf` reads no heap. For each heap read it asserts `prior && guard ==> OR_i (g_i && q_i == p)` over
     the atoms of the same field that precede the read in evaluation order (the `requires` frame reads in the `requires`
     and `old` reads; earlier `ensures` conjuncts frame plain `ensures` reads), each with its own guard; category
     `permission`, "the contract of `f` reads `*p` without owning it". `prior` (the earlier clauses, as today) also holds
     `g_i && g_j ==> q_i != q_j` for two `owns` atoms of one field **in the same clause list** (both in the `requires`,
     or both in the `ensures`), added after the second of them in evaluation order, and `q_i == q_j ==> h_i == h_j` for
     two ghost values of one state, and that the post-state value of a `reads` place returned by its `ensures` partner
     equals its pre-state value (no one can write a cell while a fraction of it is held, from entry to return; Silicon
     merges the returned fraction with the caller's on the same value), so `f__wf` is not stricter than Silicon. (Audit round 1, B2: the fact over a
     `requires` atom and its `ensures` partner, `owns(*p)` twice, would be `p != p`, a false `prior` under which every
     later assertion of `f__wf` passes.) In contract mode a dereference has no flag obligation and a
     pointer use no liveness check (there is no heap in `f__wf`); a comparison keeps item 12's obligation.
   - **In the method's own contract** each heap read `p.v_T` (and `old(p.v_T)`) is guarded **by itself**, never the
     conjunct that holds it (audit round 1, B1: guarding the whole conjunct `p != NULL ? *p == 1 : result == 0` by `p !=
     null` drops its `result == 0` part, and the function verifies against a postcondition it breaks): the read becomes
     `(D_1 ? p.v_T : (D_2 ? p.v_T : … : 0))`, a chain over the disjuncts of its framing formula `F = OR_i D_i` (`D_i` =
     `g_i && q_i == p`), with `false` for a `bool` cell, and plain `p.v_T` when `F` is `true` (the common case of an
     unconditional atom on the same place). S27 P6: Silicon refuses a read under the disjunction itself (`(r == p || r ==
     q) ==> r.v > 0` is not well-formed) and accepts one disjunct per branch. Each `D_i` reads no heap (guards read no
     heap; places are parameters). `f__wf` proves `F` wherever C evaluates the read, so there the chain is the read and
     the clause means what it says; where `f__wf` fails, the error is reported there and the guarded clause is still
     well-formed, so Silicon reports nothing else. Silicon's own well-formedness check stays an `internal` row: a report
     there is a cerastes bug (exit 3).
   - No new driver reason; the callers need no read checks beyond item 8's.

8. **Calls** (0028 items 8, 11; amends 0011 item 10's `precondition` row). Before the call, in order, each one an
   explicit, atomic assert:
   - for each callee atom, under its guard: `p != null`, `WF(p)`, `offset_of(p) < len(block_of(p))` (`owns`, `reads`),
     the permissions with the flag and the range (`perm == write` for `owns`, `> none` for `reads`, the liveness cell `>
     none`), category `permission`, "calls `f`, which needs to own `*p`, without owning it" (`reads`: "… to read …";
     `live`: "… needs `p` to point to a live object …"; a null argument where the atom needs non-null: the same row);
   - pairwise separation for two `owns` atoms, or an `owns` and a `reads` atom, of one field: `p != q` under both guards
     (`permission`: "passes `*p` to `f` twice, once to own it"), so `swap(&x, &x)` fails here;
   - the `lvloc` case splits (0028 item 8) for every pair of atoms, and the case split on the cells (`if (p == q) { }`)
     for every pair of `reads` atoms, and for each pointer argument `a` of a callee atom the case split `if (a == q) { }`
     against each `reads` parameter `q` of the caller of the same field (audit round 2, B1: a call that borrows one of
     two `reads` places of the caller otherwise adds up chunks that may alias, and Silicon's own precondition check
     fails first; the re-audit found that splitting every pair at entry instead multiplies Silicon's paths, six `reads`
     parameters timing out, so the split is made only at the calls that need it), then the amounts (item 6), then for each cell an atom takes a fraction of, the sum
     check `perm(X) >= a_k + Σ_{j≠k} (g_j && X_j == X ? a_j : none)` (`permission`), as `e2_swap.vpr` and
     `s25_a_reads.vpr` do (audit round 1, S3: without them Silicon's own precondition check would fail first);
   - the value `requires` (`precondition`, as today);
   - the call, with the ghost amounts as arguments. The callee's `ensures` come back through Viper's call (no `inhale`).
   These are items of the call created before its value preconditions, all under the call's guard. A pointer argument is
   a use (0032 item 6).
   **Placement until b1c** (fail closed; 0028 item 13 is b1c's): a full expression that holds a call of a function
   with an atom in its contract has no other access to a cell except inside that call's arguments and as the statement's
   top-level store target, and no second such call. The read of a compound assignment's target is an access (`x +=
   f(&x)`, `*p += f(p)`). Otherwise `unsupported_expression`, naming b1c.

9. **Entry.** After the ghost bounds: the `lvloc` case splits for every pair of liveness atoms of the `requires`, and
   `assert g_a && g_b ==> a != b` for each pair of `owns` atoms of one field, under both atoms' guards (`internal`:
   provable from the `requires`; S26 round 2 showed Silicon needs it before a failing return check; audit round 1, S1:
   without the guards `requires(c ? owns(*a) : live(a)) requires(owns(*b))` would exit 3).

10. **Returns** (0028 item 11; amends 0011 item 10's `postcondition` row). At each `return`, after its value and the
    scope exits (0032 item 8), and at a `void` function's closing brace: first the pairwise separation of the `ensures`'
    `owns`/`owns` and `owns`/`reads` atoms of one field under both guards (`permission`; audit round 1, S2: two `== write`
    checks pass one by one on the path where the places are equal, and Silicon's own postcondition check would fail
    first); then each `ensures` atom, one by one, under its guard (`p != null`, the permissions `== write` or `>=` the sums
    of item 6, the flag and the range for `owns`), category `permission`, "returns without owning `*p`, which its contract
    promises" (`reads`: "… without the permission to read `*p` it borrowed"); `WF(p)`, `offset_of(p) <
    len(block_of(p))` and, for a pointer result, `WF(result)` as `internal` asserts (provable: the parameters' are
    entry facts that nothing changes, and a returned pointer is a parameter, null, a cell's address or a `?:` of them;
    audit round 1, S7); then the value `ensures` (`postcondition`, as today). Silicon's own postcondition check stays an
    `internal` row.

11. **Cells by name** (amends 0032 item 6). The checks of a read or a write of an address-taken local by name are now
    category `permission` ("reads `x`, whose object a callee kept"), not `internal`: a callee can now keep a cell
    (item 13), so `keep(&x); return x;` is a user's error. Only the rows' category and message change in b1a's tests.

12. **Comparisons** (0028 item 3; amends 0032 item 7). 0032's one-past-the-end row after `==`/`!=` becomes the category
    `pointer-comparison` ("compares `p` and `q`, which may point into different objects, one of them one past its end"),
    now reachable: `WF` allows a parameter at `offset_of(p) == len(block_of(p))`. It is checked wherever the code
    compares (a comparison with a null constant never is). Two `owns` or `reads` atoms exclude one past the end
    (`offset_of(p) < len`), so comparing owned pointers verifies; `live` alone does not. **In a contract**, `==` and
    `!=` of pointers mean identity in the model (the same object and offset), with no check (found end to end: a
    result is only `WF`, so `ensures(r, r == p || r == q)` would always be reported): a contract states which object a
    pointer designates, the model's separation is by identity, and every comparison the code performs, whose C result
    is what a program can observe, keeps its check.

13. **A callee that keeps a cell.** A callee whose `requires` has `owns(*p)` and whose `ensures` does not return it keeps
    the cell: its caller's scope exit fails `lifetime` (0032 item 8's missing twin), and a later read of the cell fails
    `permission` (item 11).

14. **The lint** (`lint_viper.py`; amends 0032 item 11): a method's ghost `Perm` formals each have exactly `requires
    none < a && a < write`, before the user's clauses; `wildcard` is refused; a case-split line `if (…) { }` is
    accepted; `inhale` and `assume` stay refused outside tagged lines.

15. **Rejected until b1c or later** (fail closed; each pinned by a test the status entry lists):

    | Construct | Diagnostic | Lifted by |
    |---|---|---|
    | an atom on `*result` | `unsupported_annotation` (new: "%s is not supported by \"cerastes verify\" (decision 0037)") | b1c, M4.5d2 |
    | `restrict` without `owns`, or beside a same-typed pointer parameter without an atom (item 2) | `unsupported_annotation` ("a `restrict` parameter `p` whose contract does not own `*p` in its `requires` and its `ensures`", "… beside `q`, whose contract does not own or read `*q`") | M8 |
    | `restrict` on a result, a local or a pointee; `__restrict` in C++ | `unsupported_type` | M8 |
    | `owns(*p)` and `reads(*p)` in one clause list | `unsupported_annotation` | — |
    | an atom in any other position (item 5) | `unsupported_annotation` | — (Viper) |
    | `owns`/`reads` of a place other than `*param`; `owns_range`, `owns_block` over a pointer | `unsupported_annotation` | b2, c, d2, e1 |
    | an `ensures` atom with no `requires` partner; two atoms of one kind on one place | `unsupported_annotation` | b1c; — |
    | `*result` as a value; `&x`, `&param` in a contract | `unsupported_expression` | b1c; — |
    | a heap read in a `decreases` | `unsupported_expression` | b1c or later |
    | a memory call beside another cell access, or two in one full expression (item 8) | `unsupported_expression` | b1c |

16. **Categories** (amends 0011 item 10, in its five copies): `pointer-comparison`; `permission` gains its call, return,
    framing and by-name messages. "Seventeen" categories.

## Consequences

- Functions take and return pointers to scalars; `swap`, `inc` and a `reads` function verify, with their callers.
- A missing permission is reported at the call, at the return or in the contract, never as Silicon's own error.
- The contract's framing is checked per path; an unframed read is a `permission` error at the contract.
- `restrict` parameters are accepted where ownership already proves what `restrict` promises.
- **Cost of aliasing** (owner, 2026-10-28, after the re-audit of audit round 2): Silicon case-splits on which of a
  function's `reads`/`live` pointer parameters of one pointee type may alias, so its time grows exponentially with
  their number. Measured on a quiet machine (one call, c23): four `reads` parameters 5.7 s, five 22 s, five `live` 5.2 s,
  four with two sequential calls 13 s; six exceed Silicon's 300-second limit (exit 3: never a wrong verdict). The encoding is kept and the limit documented (Memory and ownership page); it is revisited when a port needs
  more (M4.8), for example with Silicon's joins. Tests stay at four or fewer.
- Completeness cost (audit round 1): returning or comparing a pointer parameter is a use, so the contract must give
  `live(p)` (or an `owns`/`reads` of `*p`): `int *id(int *p) { return p; }` without it fails `lifetime`.
- Ghost amounts appear in every method whose contract has an atom; programs without pointers keep their Viper.
