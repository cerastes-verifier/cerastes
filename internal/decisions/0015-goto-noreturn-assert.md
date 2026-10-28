# 0015 — Forward `goto`, functions that do not return, and `assert` (M4.2a)

- **Status:** accepted for M4.2a. Owner decisions (2026-10-21): M4.2 is split into M4.2a (this record), M4.2b (named
  constants and enumerations) and M4.2c (locals without an initializer); a new error category `noreturn`; a failing
  `assert(e)` is a `check` error; "verified" now means "returns satisfying its `ensures`, or ends the program through
  a call of a function that does not return" (amends 0013 item 6); the M3 rejection tests `reject_goto.c` and
  `reject_noreturn.c` are retired. Facts marked (P*n*) are IL probes and (S*n*) Silicon spikes, recorded with
  evidence in `internal/il-inventory.md`, "M4.2a"; code relies on them only once confirmed.
- **Milestone:** M4.2a (2026-10-21).

## Context

M3 rejected user `goto` and labels, `[[noreturn]]`/`_Noreturn` functions, and every call without a contract, which
includes `abort`, `exit` and the function `<assert.h>`'s `assert` calls (0012 item 12, 0011 items 5 and 7). M4.1
(0014) added structured jumps: every jump is to the end or the continue point of an enclosing construct, so it is
forward and never enters a loop, and `check_program` proves that from the nesting alone. Silicon accepts a forward
`goto` out of loops (0014, S10) and throws on a jump into a loop (S9c); a jump into an `if` branch is accepted, but no
spike has shown that Silicon keeps the facts of each path apart there (S9d has no must-fail twin). Results are total
correctness (0013).

## Decision

1. **Accepted:** a user `goto` whose target satisfies item 2, and user labels; functions declared `[[noreturn]]`,
   `[[gnu::noreturn]]`, `__attribute__((noreturn))`, `_Noreturn` or (C23) `[[_Noreturn]]` (P21: each sets EDG's
   "does not return" flag on the function type, which is what cerastes reads; `_Noreturn` leaves no attribute entry;
   in C++20 `[[_Noreturn]]` is an unrecognized attribute that sets nothing, and is rejected as such);
   calls of `abort`, `exit` and `_Exit` from the C library (item 5); `assert(e)` from `<assert.h>`/`<cassert>`
   (item 6). **Still rejected** (invariant 3), each with a diagnostic and a test: a `goto` that breaks item 2, a
   computed `goto *p` and the label address `&&L` (GNU; `stmk_assigned_goto`, P19), a
   `cerastes::ensures` on a function that does not return (`ensures_on_noreturn`), a `main` that does not return, a
   function that does not return by looping forever (a loop needs a measure, 0013 item 3: stricter than C 6.8.5p6),
   and the look-alikes `quick_exit`, `__builtin_trap`, `__builtin_unreachable`, C23 `unreachable()`,
   `std::terminate` (no contract; each stays an M8 entry).
2. **Forward `goto`: "outward and forward".** `goto L` is accepted only if the statement `L:` is a statement of a
   block that encloses the `goto`, comes after the `goto`'s ancestor in that block, and no declaration of that block
   lies between the two. "Block" is a VIR block: the encoder flattens the blocks EDG generates (C23 makes more of
   them than C++20) so that a declaration's VIR block contains at least its C scope (P20). Two guards keep that true
   whatever EDG generates (audit): a generated block that directly holds a user label is never flattened silently
   (rejected; this happens for a label that is the whole branch of an `if` without braces, `if (c) L: x = 1;`), and a generated block that is not flattened (one with attributes) and directly holds a declaration is
   rejected, since a jump past that declaration would not be seen. Consequences: a `goto` is never backward
   (so it creates no loop that Silicon would not check for invariants or termination), never enters a loop (S9c),
   never enters an `if` branch, a plain block or a switch body (so the dispatch of a switch, with its case-value
   obligations, runs on every path into a switch body, 0014 item 6), and never skips a declaration that is in scope
   at the label (whose object would be indeterminate there). This is stricter than C, which allows entering blocks;
   the idioms it keeps — error exits, a common cleanup label, leaving nested loops or a switch — are all outward. A
   `goto` may leave loops, switches and a `do … while (0)`; on that path nothing is checked, as for `break` (0014
   item 4): C does not evaluate the loop condition there. After the label, Silicon knows the facts of each incoming
   path separately (S11). The rejection is reported at the `goto`, naming the label and the rule it breaks. In C23 a
   jump past an initialized declaration is only EDG's warning #546 (suppressed in verify mode, 0014 item 11), so this
   rule is the only guard there; C++20 makes it a front-end error (P20). EDG's compiler-generated blocks (C23 wraps
   every `if`, loop and `switch` in one) never hold a declaration whose C scope goes past them, except a `for`
   loop's init-statement, which the encoder handles on its own (P20). GNU local labels (`__label__ L;`) leave no
   trace in the IL (P19); a `goto` names its label entity, so they change nothing that is verified.
3. **VIR and lowering.** `Goto(label id)` and `Label(id, name)` are new statements; ids come from the program's jump
   ids. `check_program` is authoritative: each `Goto` names a `Label` of the same function that is a later statement
   of a block enclosing the `Goto` with no `Decl` of that block between them; label ids are unique; the encoder's own
   checks only give the user a diagnostic at the C/C++ line. Lowering emits `goto l_user_<name>__N` and, only when a
   `goto` targets it, `label l_user_<name>__N`; an unreferenced label emits nothing. Labels and gotos carry no checks
   and no map entries (0014 item 9). Attributes on a label are held to the allow-list (0011 item 7); EDG keeps them on
   the label entity, not on the label statement (P19), so the encoder checks the entity's.
   `scripts/lint_viper.py` checks the same rule on the emitted Viper, at token level, as defence in depth: every
   `goto` (the dispatch of a switch, `break`, `continue`, `goto l_end` and user gotos alike) must jump to a later
   label whose enclosing braces also enclose the `goto`; this subsumes 0014 item 10's backward rule and excludes a
   jump into a loop body or an `if` branch.
4. **Functions that do not return.**
   - **Meaning.** Returning from such a function is undefined behaviour (C23 `_Noreturn`/`[[noreturn]]`,
     C++ [dcl.attr.noreturn]). A
     function with a body in a verified file is verified never to return: each `return` and its closing brace get
     `assert false` with the new category `noreturn` (a non-`void` one falling off its end is `noreturn`, not
     `missing-return`), in place of the postcondition asserts; its Viper method gets `ensures false`, which these
     asserts imply (Silicon's own postcondition check stays a backstop, mapped to exit 3, 0011 item 11). VIR marks the
     function (`Function::noreturn`); `false` is not put among the user's `ensures`, which report `postcondition`.
   - **Calls** are ordinary calls (0012 item 6): after `f(…)`, Viper's call rule gives the callee's `ensures false`,
     so the rest of the path is unreachable. Nothing is inhaled or assumed.
   - **Spellings with an effect only.** A `noreturn` attribute entry is accepted only on a routine whose type has the
     flag set; anywhere else it is rejected (EDG keeps a misplaced one as an unrecognized attribute, so the diagnostic
     is `unrecognized_attribute`, or EDG errors itself, #1835).
   - **Declared here, defined elsewhere.** A user function declared `[[noreturn]]` in a verified file and defined in
     another unit — its first declaration in a verified file, with a source position (not a builtin), and the flag
     written there: no `noreturn` attribute entry of it in a file that is not verified, and no file that cannot be
     placed (code audit: a user's plain `void quick_exit(int);` before `<stdlib.h>` takes the flag from the SDK's
     `__dead2`, which is not the user's claim) — is used through its declaration: a bodyless method with its `requires` and `ensures false`, tagged
     `// cerastes:trusted-contract declared noreturn here, defined in another unit` (the single-unit assumption,
     0011 item 6). `[[noreturn]]` counts as a contract for 0011 item 5, so such a function may be called without a
     `requires` or `ensures`. Like any contract a user writes on a declaration, it is the user's claim: a function the C
     library also declares (`_Noreturn int rand(void);` before `<stdlib.h>`) is trusted to end the path, as a user's
     `ensures(false)` on it would be (0011 item 6); the tag's "defined in another unit" is then inexact, as it already
     is for such contracts (the IL keeps only the first declaration's position, P21). A scan of the system headers for
     the name was tried and dropped (it misses `(name)(…)` declarations and names built by macros); the sound check,
     EDG's source sequence lists, is a configuration change for a later session (handoff). Builtins have no source
     position and are never trusted. As for every trusted contract (0012 item 6), a trusted callee that throws (C++)
     is outside its contract.
   - **Rejected:** a `cerastes::ensures` on such a function (it never returns, so it would never be checked:
     `ensures_on_noreturn`), and a `main` declared not to return. `requires` and `decreases` are allowed.
   - **Termination** (0013) is unchanged: the function is in the call graph like any other; a loop needs a measure, so
     a function that never returns by looping forever is rejected.
   - **EDG's warning #1305** (a function declared not to return may return) is suppressed in verify mode (amends 0011
     item 7 and 0010 item 12's list): cerastes reports every reachable `return` and closing brace itself, with a
     category and a line (EDG puts #1305 at the same places, P21); an unreachable one is no problem.
   - **What "verified" means** (owner, 2026-10-21; amends 0013 item 6): a verified function, on every input that
     satisfies its `requires`, has no undefined behaviour and either returns, satisfying its `ensures`, or ends the
     program by calling a function that does not return. This is VeriFast's contract for `abort` and `exit`. In
     particular `exit(1)` in `main` ends the program without `main`'s `ensures` being checked; the Verifying page says
     so. `exit` runs the functions registered with `atexit` and, in C++, the destructors of static objects; in a
     single unit with no function pointers and no classes none of them is user code of this unit, and code of other
     units is under the single-unit assumption (0011 item 6, 0013 item 5). The success line is unchanged.
5. **Trusted C library functions** (a new, built-in table in the encoder; M4.5a decides whether it moves to a header
   when contracts need permissions; M4.5d adds `malloc`/`free`). An entry applies to a function declared only in
   system files (0011 item 4), with C linkage, the exact signature and EDG's "does not return" flag (P21); anything
   else with the same name is an ordinary function. The IL keeps only the first declaration's position (P21), so
   "declared only in system files" is checked on the first declaration; a later user redeclaration of `exit` cannot add
   a contract (must-match, 0006 item 6) and changes nothing. A rule keyed to the flag alone would also trust `quick_exit`, the
   builtins `__builtin_trap`/`__builtin_unreachable`, `std::terminate` and `__assert_rtn`, which all carry it (P21):
   the table is a closed list of names. Entries:
   `void abort(void)`, `void exit(int)`, `void _Exit(int)`: `requires true`, `ensures false`, `decreases _` (0013
   item 5), tagged `// cerastes:trusted-contract libc: <name> never returns (C23 7.24.4.<n>)`. `std::exit` and
   `std::abort` are the same routine entities as `::exit` and `::abort`; all three are declared in the SDK's
   `usr/include` with C linkage in both modes (P21). `ensures false` is the strongest trusted assumption, but like
   every trusted contract it is marked by its tag (0011 item 6), which the lint requires on every bodyless method; a
   closed list of reasons was considered and dropped (audit): a user's own `ensures(false)` on a declaration is a
   trusted contract too (the Verifying page's `trusted_never_returns.cpp`), and a text match is easy to get around.
6. **`assert`.** The macOS SDK defines `assert(e)` as
   `(__builtin_expect(!(e), 0) ? __assert_rtn(__func__, __ASSERT_FILE_NAME, __LINE__, #e) : (void)0)` (the
   `__GNUC__` branch of `assert.h`, which both modes take, P22), and as
   `((void)0)` under `NDEBUG`. `__assert_rtn` takes pointers and string literals (M4.5, M4.6), so it cannot get an
   ordinary contract. The encoder recognises exactly the expansion's shape (P22) as an expression statement: a `void`
   conditional (`eok_question`) whose condition is a call of the builtin `__builtin_expect` (in C++ under a `bool`
   conversion) whose second argument is the constant 0 and whose first argument is a conversion to `long` of `!`
   applied to `e`; whose second operand is a call of the system function `__assert_rtn`; and whose third operand is a
   cast of 0 to `void`. The two zeros must be plain constants with no original expression (0010 item 12: a folded
   `(void)(INT_MAX + 1)` does not match). Only the `long` conversion of the `!` and the `bool` conversion of the call
   belong to the shape: every node below the `!`, a cast the user wrote included, is part of `e`
   (`assert((unsigned char)x)` with `x == 256` fails). It encodes `e` — the operand of the macro's
   `!`, which is the user's text with the user's positions — as code (its obligations, calls by 0012 item 6, C's
   truth values by 0010 item 6), then asserts that `e` holds, with category `check` and a message that names
   `assert`: in VIR a `Check` marked as an assert, whose expression `check_program` types as code (calls allowed and
   hoisted, as in an `if` condition) rather than as an annotation. An `assert` in a `for` loop's increment is rejected
   (a loop's step holds no checks). Every node of the expansion is positioned at the macro invocation (P22), so obligations and the check are
   reported at the line of the `assert`. The arguments of `__assert_rtn` (`__func__`, the file name, the line, the text) are never encoded: the
   call is unreachable once the assert holds. Any other use of `__builtin_expect` or `__assert_rtn` is rejected.
   Under `NDEBUG` the statement is `(void)0`, so nothing is checked — C's meaning. A result holds for the
   configuration verified: with asserts on, what follows an `assert(p(x))` may rely on `p(x)`; an `NDEBUG` build of
   the same code is a different program (the Verifying page says so). A user macro with the same
   expansion means the same thing. glibc's expansion (`__assert_fail`) is not recognised yet (the CI session, if it
   adds Linux).
7. **Categories.** New: `noreturn` — "a function declared not to return may return", reported at the reachable
   `return` or closing brace (0011 item 10). `check` also covers a failing `assert`.
8. **IL facts relied on** (`internal/il-inventory.md`, "M4.2a", all confirmed): (P19) a user `goto` is a
   `stmk_goto` without `compiler_generated` naming its `a_label`; the label is a `stmk_label` covering only `L:`, and
   the labeled statement is its next sibling; gotos out of loops and switches generate nothing; label attributes are
   on the label entity; an unreferenced label is warning #177; (P20) the scopes above; (P21) the noreturn spellings
   and the C library declarations; (P22) the `assert` expansion in both modes, and `(void)0` under `NDEBUG`.
9. **Spikes** (Silicon `v.26.08-release`, `build/m4.2a/spikes/`, all confirmed): (S11) a `goto` out of nested loops,
   out of a dispatch inside a loop body to a later label of the body (a twin breaking an invariant on that path is
   `invariant.not.preserved` at the clause, one skipping the step fails termination at the measure), and out of
   straight-line `DoOnce` code verify, and a fact true only on the normal path fails after the label; (S12) a bodyless
   `ensures false` method with `decreases _` called inside a measured loop and from a measured method verifies and
   makes what follows unreachable; a bodied `ensures false` method whose returns are `assert false` verifies when
   each path ends in such a call, and with a reachable return Silicon reports only the explicit `assert false` (not
   the postcondition); (S13) an assert's obligations come first; after a call that does not return, every assert
   holds.
10. **Tooling.** `lint_viper.py`: item 3's outward-and-forward rule. `mutate.py`: `delete-stmt` deletes a `goto L;`;
    deleting an `assert(…);` statement whose condition has no call, increment, assignment or comma only removes a check,
    so such a surviving mutant is in the safe-direction bucket; one with a call is not (re-audit: a callee's `ensures`
    can hold only because the callee ends the program otherwise, so deleting the call can make the program wrong).
11. **Amends** 0011 items 5, 6, 7 and 10; 0012 items 3, 6, 9 and 12; 0013 items 5 and 6; 0014 items 1, 2, 3, 9 and 11
    (#546 stays suppressed: cerastes now rejects every jump past a declaration, `goto` included, item 2); and
    `internal/encoding.md` §2, §3, §7 and §8.

## Consequences

- Error exits, cleanup labels, `[[noreturn]]` helpers, `abort`/`exit` and `assert` verify; the user-visible rules are
  on the Verifying a program, Annotations and Error messages pages.
- Jumps stay forward and structured, so their soundness still reduces to Silicon's forward `goto` (S10, S11) and
  `check_program`'s nesting rules.
- The trusted libc table is the first cerastes-supplied contract; each entry is tagged in the emitted Viper.

*2026-10-22, amended by 0019 (M4.2c): item 2 — a `goto` may jump past a declaration without an initializer (the variable has no value at the label until it is assigned, `uninitialized`); one past a declaration with an initializer stays rejected.*

*2026-10-23, amended by 0021 (M4.3b): item 6 — an `assert` condition may hold calls in guarded operands and unsequenced calls, under 0021's dependency-chain rule; side effects stay refused in it.*

*2026-10-27, amended by 0030 (M4.5a, effective from M4.5d1): item 5 — the trusted table moves to vlibc's `<stdlib.h>`: a C library function's contract comes from a vlibc declaration, and `noreturn` from vlibc, not the SDK's attributes (0030 item 4).*
