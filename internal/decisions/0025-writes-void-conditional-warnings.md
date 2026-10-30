# 0025 — Writes outside the chain, `void` `?:` statements, warnings in tests, the other `-Wall` shapes (M4.4c)

- **Status:** accepted for M4.4c (2026-10-26). Owner decisions (handoff §11 questions 28, 30, 31, 37 and 40, and the
  M4.4c planning, 2026-10-26):
  - **Question 28:** "Do it now since we know it is correct." Only writes and snapshots leave 0021's family F; checks and
    call-free branches stay. A write goes right after the member of its set that comes last in π, with ties broken by
    creation order.
  - **Question 30:** "Do it now since we know it is correct." A `void` `?:` in statement position is C's `if`.
  - **Question 31:** a `verified` or `error` test may list the front-end warnings it expects.
  - **Question 37:** the other `-Wall` shapes are rejected like the compilers (0020 item 5's union), with one rejection test
    per shape; the `-Wextra`-only `-Wtype-limits` shapes stay accepted. At the planning: `x ? RED : NA` too.
  - **Question 40:** "follow the recommendation": the static-assertion cross-check covers function bodies.
  - **During the session (2026-10-26):** the shapes audit round 1 found outside question 37's list (clang's
    `-Wconstant-logical-operand`, `-Wtautological-constant-out-of-range-compare`, `-Wtautological-overlap-compare`,
    `-Wempty-body`, and `-Wdangling-else`) are added to M4.4c (item 6, R9–R13); the further ones audit round 2 found are
    listed, and a new session **M4.4e** sweeps every default and `-Wall` warning of clang and GCC.
  - **Planning (2026-10-26):** one session, with question 37 last and a cut order; the planning review's **finding A**
    (item 1) is fixed first; a flagged snapshot's `uninitialized` check stays in F; the Viper of programs with a write or
    snapshot of a call result may change where measured and listed, but no verdict or error line may.
  - **Expectation changes (pre-approved):** `tests/scalar/negative/reject_unsequenced_assignments.c`,
    `reject_void_conditional_arms.c` and the VIR case `calls_rej_two_writes` are retired; the `unsupported_type` line of
    the four `reject_assert_shape_*.c` tests is removed as measured; the harness self-test
    `test_a_warning_line_does_not_mix_with_other_outcomes` becomes a positive test.

  Facts marked (P*n*) are probes, recorded with their evidence in `internal/il-inventory.md`, "M4.4c" (P46–P48).

## Context

0021 placed every item of a full expression — check, write, snapshot, branch — by its set of fixed points, and required
the sets to form a chain. Writes and snapshots cannot fail, so they need no place in the chain; only their order with
the other accesses to the same local matters, and that order follows from their sets. 0020 made a `?:` with `void` arms
unsupported because VIR has no `void` value, although in statement position it is exactly an `if`. The harness refused
every front-end warning in a verified test, so accepted shapes that print EDG's #187 could not be pinned. The `-Wall`
shapes 0020 and 0023 did not cover were measured in M4.4b1 (P41 group 11) and accepted. Planning found an ordering hole
for `uninitialized` checks (item 1).

## Decision

1. **The flag read is an access of the local** (finding A; amends 0019 item 13's lowering and 0020 E2–E3; P47).
   A snapshot of a local declared without an initializer carries an `uninitialized` check of the local's flag, with the
   set of the read: the snapshot's own set, its guard's and the comma floor's. The local's access record
   (`last_access`) took only the snapshot's own set. So a later write of the same local could be placed before the
   check: a comma's value drops its left operand's calls, so the write may have an empty set while the check follows a
   call. In `int x; int y = (x = (g(f(a) > 0 && x > 0), 2));` the flag was then set before the check read it, and the
   program verified, although C reads `x` uninitialized when `f(a) > 0`. Now the access record takes the check's set,
   so every later write of the local follows the check, and `check_variable_order` counts a flag read as a read of its
   local. The access record now holds a set no larger than C's (0021 item 3: a set is never larger): the guard's calls
   complete before the read, which is sequenced before the write. Verdicts change only for programs that were wrongly
   verified; the Viper of a correct program whose flagged local is written after a guarded read in the same full
   expression may change (no test had the shape). A self-test hook `drop_flag_access` undoes the record, and the
   access-order check fires (exit 3). (P47: three shapes verified in both
   modes on the M4.4b2 binary, the comma above, a write in the right operand of `&&` after a discarded guarded read,
   and an arm of a `?:` under a discarded cast; the hole needs a guard whose set holds a call and a comma that drops it. No test
   had the shape.)
2. **Writes and snapshots leave F** (question 28; amends 0021 items 3, 9 and 10, and 0020 E2).
   - **F** holds the sets of the checks (obligations, the explicit termination checks) and of the call-free branches,
     D(B) for each branch B with a call, and PRE(c) for every call in the caller's recursion cycle. The chain test and
     π are 0021's.
   - **Placement.** A write or a snapshot w goes right after the member of its set S(w) that comes last in π, or before
     every fixed point when S(w) is empty. Items placed after the same fixed point keep creation order.
   - **Why it is sound.** A write or a snapshot need not sit where C performs it; it must only keep its order with the
     other accesses to its local, since nothing else can observe it. Inside a full expression the variable `v_x` and its
     flag `d_x` are touched only by the accesses that `last_write` and `last_access` record: snapshots, writes, branches
     that read or write `x`, and (item 1) flag reads. A callee cannot touch them (0021 item 2: no globals or pointers
     before M4.5), 0020 item 4 rejects unsequenced conflicting accesses, and `check_variable_order` keeps each local's
     accesses in creation order, which is C's order for them. A write cannot fail, so placing it earlier or later hides no
     check; a check that uses the stored value reads the value's own temporaries (0020 E4), not the variable. The checks
     stay in F, so 0021 item 10's argument for them is unchanged. (A write may thus be emitted before a call C performs
     first, as in `x = (g(a), 2)`, whose comma value drops g: no state the call can see differs.) This rests on callees not
     seeing locals; M4.5b (globals and address-taken locals as cells) must revisit it, together with item 1's kinds of
     access.
   - **Fail closed.** After emission, lowering checks the prefix rule for every check and call-free branch, as before;
     for every write and snapshot, that every member of its set is emitted before it, and that every later item whose set
     (or PRE, for a fixed point) contains its set is emitted after it; and the access order of each local
     (`check_variable_order`, with the flag reads of item 1). Each check reads the placement emission used. Hooks reach
     each half (exit 3): `perturb_write_slot` moves the first write with a non-empty set before every fixed point,
     `delay_write_slot` moves a write that a later item depends on to the last slot, and `drop_flag_access` (item 1).
   - **Reports.** `origin_of` and the conflict report count only the items of F, so `unsequenced_calls` names checks
     and branches, never a write.
   - **What stays rejected.** A call-free branch is an item of F, and it may hold a write:
     `(f(a) > 0 && (x = 1)) + 100 / g(b)` is still `unsequenced_calls`. So is `int x, y; … (x = f(a), x) + (y = g(b), y)`:
     the snapshots of locals declared without an initializer carry `uninitialized` checks, which stay in F, although
     they cannot fail after the writes (owner, planning: one rule; an M8 entry).
   - **Accepted now:** `(x = f(a)) + (y = g(b))`, `f(b) + (x = g(a))`, `(x = f(a), x) + (y = g(b), y)` with initialized
     locals.
   - **The message** of `unsequenced_calls` says "something this expression checks, or evaluates on one path only,
     depends on only one of them" (it said "checks or assigns").
   - **Viper.** π may differ for programs accepted since M4.3b whose order was forced by a write's or a snapshot's set
     (`f(b) + (x = g(a))` was emitted g, f; now f, g), and with π, whether an explicit termination check is printed.
     Programs accepted before M4.3b keep their Viper (0021 item 7), except for item 1's shape.
3. **A `void` `?:` in statement position is an `if`** (question 30; amends 0020 item 1, 0021 item 1's sentence on
   `void` arms, and 0014 item 2).
   - **Where.** An expression statement, a discarded operand of a comma (which the encoder already splits into
     statements), a `for` step and a `for` init clause (an expression statement), whose value is discarded, after `(void)`
     casts are removed. The `?:` is matched after the `assert` shape (0015 item 6), so an `assert` stays a `check`; an
     `assert` arm in a `for` step stays rejected (a step holds no checks, 0015 item 6). A `void` `?:` counts as having an
     effect for 0020 item 5's comma rule, as a `(void)` cast does (`x++, c ? (void)0 : (void)0;` is accepted: no
     compiler warns).
   - **How.** `c ? a : b;` is encoded as `if (c) { a; } else { b; }`: the condition is encoded as the condition of a
     value `?:` (the `?:` detectors of 0023 item 5, not `assignment_as_condition`, which the compilers do not apply to a
     `?:` condition); each arm is an expression statement in its own block, so `(void)0` is an empty statement, an arm
     that is itself a `void` `?:` nests, and an `assert` arm is a `check` on its own path. C sequences the condition
     before either arm, so the `if` is exact: the condition's checks are unconditional, each arm's only on its path.
   - **A `for` step** may be an `if` whose two arms are steps (VIR `check_program`; a one-armed `if` stays refused, as
     the encoder always builds both).
   - **Still rejected:** a `void` `?:` as the left operand of a comma whose value is used (`unsupported_type`: VIR has
     no `void` value), and C++ `return c ? v() : w();` in a `void` function (`unsupported_statement`, as any return with
     a value in such a function). Both are M8 entries.
   - The four `reject_assert_shape_*.c` near misses of `assert` lose their `unsupported_type` line (their `(void)0` arm
     is now an empty statement); their `call_without_contract` stays.
4. **Front-end warnings in `verified` and `error` tests** (question 31; harness only, `tests/README.md`). A test whose
   outcome is `verified` or `error` may list `frontend-warning <tag> @ L<n>` lines; the warnings EDG prints must then
   match them exactly (a multiset of tag and line, as for `rejected` tests). A test without such lines still fails on
   any warning. A `.vpr` test takes none. The warnings such a test may list are a closed list in the harness (today only
   EDG's #187, `assign_where_compare_meant`; verify mode already suppresses the warnings that mark a changed value, 0011
   item 7); a warning printed in one mode only goes in a test restricted to that mode. Pinned by `discarded_and_assign_warn.c` (`a && (y = 2);`, EDG's #187
   `assign_where_compare_meant`, silent in all four compilers, P48) and its `error` twin.
5. **The static-assertion cross-check inside functions** (question 40; amends 0024 item 2 and 0011 item 13). EDG's walk
   of each routine's memory region lists the block-scope static assertions it holds, whatever the statement structure
   (nested blocks, loops, switch bodies, dead code). After the bodies are encoded, and only when nothing has been
   rejected (a rejected statement's sub-statements may never be walked, so no rejected test gains a line), each
   assertion in a verified file must have reached the encoder (P49: with the hook below, every assertion of a probe in
   a nested block, a loop, a `switch`, both branches of an `if`, dead code after `return`, code a `goto` jumps over and
   a C23 `for` init-declaration is reported at its own line): an assertion in the region of a `predicate` or `pure`
   routine is exempt (not checked until M4.5f), and any other is rejected with 0024's "a static assertion this version
   does not check". A new hook `drop_block_static_assertion` makes the encoder ignore block assertions, and the check
   fires (exit 2).
6. **The other `-Wall` shapes** (question 37; amends 0020 item 5 and 0023 items 4–5; P48). Each rule rejects, in both
   modes, what some compiler warns about by default or under `-Wall` in some mode (0020 item 5's union; P48 found no
   shape that only `-Wextra` warns about). The rules read the IL as 0023 item 5 does (through compiler-generated casts,
   `is_parenthesized` on the innermost written operation, folded constants' `expr`), report at the operator, and apply
   wherever the operator is encoded: bodies, contracts and annotations, the prologue's initializers, case values,
   enumerator initializers and encoded static assertions. As in 0023 item 5, the operands of `sizeof`, `_Alignof` and
   `typeof` and the conditions of skipped static assertions (0024 item 3) are not scanned. A **truth value** is 0023
   item 7's.
   - **R8, two enumerations** (`unsupported_expression`, 0023 item 4's wording): `+ - * / %` whose operands are values
     of two different enumerations (`RED + NA`), and a `?:` whose arms are (`x ? RED : NA`). In C a value of an
     enumeration includes, as in 0023 item 7, a `?:` of enumerators of one enumeration and a comma ending in an
     enumerator (`RED + (x ? NA : NB)`). clang++ and g++ warn by
     default in C++20 (`-Wdeprecated-enum-enum-conversion`, `-Wdeprecated-enum-compare-conditional`); the C compilers are
     silent, and the rule holds in both modes, as 0023 item 4's `RED | NA` does. `RED + GREEN`, `(int)RED + NA` and
     `x ? RED : GREEN` stay as they were.
   - **R1, `logical_operand_parentheses`** (all four): an unparenthesized `&&` as an operand of `||` (`a && b || c`,
     `a || b && c`). `(a && b) || c`, `a && b && c` are accepted.
   - **R2, `shift_operand_parentheses`** (all four): an unparenthesized binary `+` or `-` as an operand of `<<` or `>>`
     (`a + b << c`, `a << b - c`, `a + b >> c`), signed or unsigned. `(a + b) << c`, `a * b << c`, `-a << b`,
     `a << b << c` and the compound `x <<= a + b` are accepted (silent in all four).
   - **R3, `logical_not_comparison`** (all four; `!x == 0` clang only): an unparenthesized `!` as the left operand of a
     comparison whose right operand is not boolean in GCC's sense, which differs from 0023 item 7's truth value (audit
     round 1): a `bool` node, a comparison, `!`, `&&`, `||`, a `&`, `|` or `^` of two such, or a unary `+` of one is
     boolean; a `?:` of truth values, a comma ending in one and an assignment of one are not (GCC's C mode warns). So
     `!x == y`, `!x < y`, `!x == 1`, `!x == 0`, `!x == (c ? (a > b) : (a < b))`, `!x == (y = 1, a > b)` and
     `!x == (y = (a > b))` are rejected; `x == !y`, `!x == !y`, `!x == (y > 0)`, `!x == b` with a `bool` `b`,
     `!x == (p & q)`, `!x == +(a > b)` and `(!x) == y` are accepted.
   - **R4, `chained_comparison`** (relational chains: all four, an error in clang; the others GCC): an operand of a
     comparison that is an unparenthesized comparison (`a < b < c`, `a == b == c`, `a < b == 1`, `a == b < c`).
     `(a < b) < c`, `(a == b) == c` and `(a < b) == (c < d)` are accepted.
   - **R5, `self_comparison`** (all four; the identical-expression form GCC only): any comparison whose operands, after
     compiler-generated conversions, are the same variable, parameter, enumerator or named constant (`x == x`,
     `x >= x`, `(x) == x`, `K == K`: clang by default), or, for GCC, the same expression without calls,
     side effects or explicit casts that contains a variable or parameter (`x + 1 == x + 1`, `-x == -x`). In a contract
     the operands are parameter references, compared by parameter (P46b). An explicit cast is a barrier, and so is an
     expression of constants only (`(int)x == x`, `(long)x == (long)x` and `1 + 2 == 1 + 2` are accepted: all four are
     silent). **Macros:** all four are silent when the comparison itself is
     written inside a macro (`SAME(x)`, `ID(x == x)`, `assert(x == x)`), and GCC's C mode warns when only an operand
     comes from one (`ID(x) == x`, `X == x`): the comparison is exempt when its operator comes from a macro expansion
     (its original position differs from its position, P46b: the IL tells a macro body and a macro argument apart, and
     both exempt), except in annotations (`requires`, `ensures`, `invariant`, `decreases`, `check`), where the compilers see nothing
     and `CERASTES_REQUIRES(x == x)` is rejected like `[[cerastes::requires(x == x)]]`.
   - **R6, `truth_value_compare`** (all four for a truth value; clang only for `& | ^` of two truth values, a unary `+`
     of one and an assignment of one; GCC's C mode only through an explicit integer cast): a comparison of a truth value
     with an integer constant (an enumerator included) whose result is the same whether the truth value is 0 or 1
     (`(a > b) == 2`, `(a > b) >= 0`, `(a > b) < 2`, `p == 2` with a `bool` `p`, `(p ^ q) == 2`, `+(a > b) == 2`,
     `(y = (a > b)) == 2`, `(int)(a > b) == 2`). `(a > b) == 1`, `(a > b) >= 1`, `(a > b) != 0` and `(a > b) == p` are accepted.
   - **R7, `int_in_truth_context`** (signed `<<`: all four; `*`, and a `?:` with a constant arm other than 0 or 1: GCC,
     `c ? 2 : 3` all four; enumerators: GCC's C++ and clang): in a truth context — the condition of `if`, `while`, `do`,
     `for` and `?:`, an operand of `!`, `&&` and `||`, a conversion to `bool`, the conditions of `requires`, `ensures`,
     `invariant`, `check`, `assert` and an encoded static assertion, and item 3's condition — a `<<` whose result is signed (also
     as either arm of a `?:` or a comma's right operand: clang's C++ mode warns in every context), a `*` (also as a
     comma's right operand; as the arms of a `?:` only when both arms are, and only in a conversion to `bool`: GCC's C
     mode), a `?:` with an integer-constant arm whose value is neither 0 nor 1, or an enumerator whose value is neither 0
     nor 1, looking through parentheses. Unsigned `<<`, `>>`, `a << b != 0`, `c ? 1 : 0`, `if (2)`, an
     enumeration-typed variable and `if (c ? a * b : x)` are accepted. Listed as narrower than GCC: an arm that is a
     comma ending in a constant (`c ? (y = 1, 2) : 0`) is not looked into.
   - **R9, `constant_logical_operand`** (clang by default; for 0 and 1 its C++ mode only, P50): the right operand of
     `&&` or `||` is an integer constant that is not a truth value (`x && 4`, `x || 2`, `x && K`, `x && 'a'`, and
     `x && 1`), unless the left operand is a `bool` value (`b && 4`). A constant left operand (`4 && x`), a truth value
     on the right (`x && !4`, `x && 2 > 1`) and `x && true` are accepted, and so, in code, is an `&&` or `||` that comes
     from a macro expansion (`assert(x && 4)`, `#define AND4(x) ((x) && 4)`: clang is silent; audit round 2); in an
     annotation the rule applies whatever the macros, as R5's does. A 0 or 1 right operand that comes from a macro
     (`#define ONE 1`, `x && ONE`) is rejected although all four compilers are silent: broader, listed for M4.4e.
   - **R10, `constant_out_of_range_compare`** (clang by default): a comparison of a literal or an enumerator with an
     operand whose type, before the front end's promotions, cannot hold its value (`uc == 300`, `uc == -1`, `sc > 200`,
     `us < 70000`, `x < 3000000000LL`; for an enumeration, its value range: `e == 7` for 0..3). A constant in range never
     fires, even when the result is constant (`uc > 255`, `uc >= 0`: GCC's `-Wextra` `-Wtype-limits`), nor does a named
     constant (`c == DEF` with `const enum color DEF = (enum color)4;`: all four compilers are silent; but clang's C++ mode
     warns on `uc == DEF` with `const int DEF = 300;`, which is accepted: narrower, listed for M4.4e). The rule runs only
     when the enumeration and sign rules have not already rejected the comparison, and in code only, as R11 (an
     out-of-range comparison in a contract is a wrong contract, which verification reports; `check.sh` found three error
     tests with one, `char_literal_u8_to_schar.c`, `enum_const_conversion.c` and `uchar_promote_post.c`). A constant
     expression of literals (`uc == 256 + 44`, clang warns) is accepted: narrower, listed for M4.4e.
   - **R11, `overlapping_comparisons`** (clang `-Wall`): `&&` (`||`) whose two operands compare the same variable or
     parameter with literals or enumerators so that the result is always false (always true): `x > 5 && x < 3`,
     `x == 1 && x == 2`, `x == 1 || x != 1`, `x <= 5 || x >= 5`. A named constant (`x > K && x < 3`) and an operator
     from a macro expansion (`assert(x > 5 && x < 3)`) are accepted (clang is silent; audit round 2). An operand from a
     macro under a written operator (`#define LIM 5`, `x > LIM && x < 3`; clang is silent) is rejected: broader, listed
     for M4.4e. In code only, not in annotations: a precondition that
     cannot hold, or a postcondition that always does, is a contract (possibly a wrong one), which no compiler sees, and
     three tests use one on purpose. `x >= 5 && x <= 5`, `x == 1 || x == 2` and `z && x > 5 && x < 3` (the comparisons
     are not one operator's operands) are accepted.
   - **R12, `dangling_else`** (clang by default, GCC `-Wall`; P50b): an `if` without `else` whose body, written without
     braces (through loop bodies and labels), is an `if` with an `else`: `if (a) if (b) x = 1; else x = 2;`, at the
     `else`. Braces anywhere between them, an outer `else`, an `else if` chain, a `switch` or a `do` between them are
     accepted.
   - **R13, `empty_body`** (clang by default; P50b): a null statement as the body of an `if` without `else` or of a
     `switch` (`if (x > 0);`, `switch (x);`), at the `;`. clang warns only when the `;` is on the line of the condition's
     `)`, which the IL does not record, so a `;` on the next line is rejected too, and so is a body that is an empty
     macro invocation followed by `;` (`if (x > 0) LOG("positive");` with `LOG` expanding to nothing; clang is silent):
     broader, listed. A `;` that itself comes from a macro expansion is accepted (clang is silent). Loops are not
     covered: clang warns about `while (x--);` and `for (…);` only when the next statement is on the same line or
     indented deeper (audit round 2; P50's probe had the next statement on the same line), which a layout rule would
     need. `if (x) ; else y;`, `else ;`, `do ; while (c);` (GCC `-Wextra` only) and an empty block `{}` are accepted.
   - **Still accepted, listed:** default and `-Wall` warnings found by audit round 2 that no rule covers yet (owner,
     2026-10-26: listed now; a later session, **M4.4e**, sweeps every default and `-Wall` warning of clang and GCC
     against the language cerastes accepts): clang's `-Wparentheses-equality` (`if ((x == y))`), `-Wswitch-bool`
     (`switch (a > b)`), `-Wfor-loop-analysis` (`for (…; i < n; i++) { i++; }`), `-Wmisleading-indentation` (clang and
     GCC), GCC's `-Wmultistatement-macros`, and clang's `-Wempty-body` on loops (R13); GCC's sign-compare on `u == -1`
     (`-Wall` in C++; the owner's answer to handoff §11 question 35 keeps the path-precise `conversion` check); GCC's `-Wtype-limits` (`-Wextra` only): `u >= 0`, `u < 0`; `RED == RED` and
     `RED + 1 == RED + 1` (clang warns by default), which the front end folds to the constant 1 without keeping the
     expression, so the IL holds no comparison to detect (an M8 entry).
7. **Facts relied on** (`internal/il-inventory.md`, "M4.4c"): (P46) the IL of a `void` `?:` in both modes; (P47) finding
   A on the M4.4b2 binary; (P48) the compiler matrix of item 6's R1–R8 (clang 21, GCC 16, both modes, no flags,
   `-Wall`, `-Wall -Wextra`) and the Q30 and Q31 shapes; (P49) the region walk of item 5; (P50) the compiler matrix of
   R9–R13; (P50b) the IL of written and generated blocks, `else` positions and null statements.
8. **Tests and documentation.** For item 1, must-fail pins of the three P47 shapes and a verified twin (the local set
   before the guarded read); for item 2, must-verify and must-fail pairs for the accepted shapes, rejection tests for what
   stays rejected, and the soundness pin `(x = nr(k)) + 100 / zero(y)` (must fail `division-by-zero`; rejected before
   this record, it verifies if the write's placement hides the check), in both modes; the same for item 3 (statement,
   discarded comma, `for` step, nested, `assert` arm); item 4's pin and its harness self-tests; for item 5, a verified
   test with assertions in nested statements and its error twins, a pin of the `pure` exemption and a harness test of
   the hook; one rejection test per shape of item 6 and a verified test of its accepted neighbours; VIR self-test cases
   for items 1–3 and the three hooks. Pages: Verifying a program (side effects and calls, `void` `?:`, the warning shapes,
   static assertions, the not-supported list), Integer semantics (enumerations).

## Amends

- 0011 item 13 (hooks: `drop_block_static_assertion` in the driver; `perturb_write_slot`, `delay_write_slot` and
  `drop_flag_access` in the VIR self-test);
- 0014 item 2 (a `for` step may be an `if` of steps);
- 0019 item 13 and 0020 E2–E3 (item 1);
- 0020 items 1 and 5 (items 3 and 6);
- 0021 items 1 (the `void` arms), 3, 9 and 10 (item 2);
- 0023 items 4 and 5 ("not rejected, listed": the truth-value comparisons are now R6);
- 0024 item 2 (item 5);
- `internal/encoding.md` §3; `tests/README.md` (item 4).

## Consequences

- Assignments of call results may share an expression; checks and branches still may not depend on different
  unsequenced calls.
- `c ? v(x) : w(y);` and its forms in commas and `for` steps verify.
- The `-Wall` shapes are rejected in every mode, even where only one compiler in one mode warns.
- The emitted Viper of programs with a write or snapshot of a call result may change; their verdicts do not.

*2026-10-27, amended by 0026 (M4.4d): item 2 — with the exact sets (0026 step 1) a comma's value carries its floor, so the write of `x = (g(a), 2)` now follows `g`; the remark that it may be emitted before `g` describes step 2.*

*2026-10-27, amended by 0028 (M4.5a, effective from M4.5b1): item 2 — writes and snapshots of a cell are never moved across a call whose contract reaches cells; Viper locals keep this placement (0028 item 13).*

*2026-10-29, amended by 0039 (M4.5b1c2): item 2 — writes and snapshots of cells are not placed by this item: a cell write follows every fixed point of its operands in both halves of its set, and a cell snapshot its guard's set, so neither crosses a memory call C orders with it (0039 items 2 and 4). Item 6 — R14, `return_local_address`: `return &x;` of an automatic local or a parameter, through parentheses, casts, either arm of `?:` and the right operand of `,` (clang `-Wreturn-stack-address`, GCC `-Wreturn-local-addr`; P61).*
