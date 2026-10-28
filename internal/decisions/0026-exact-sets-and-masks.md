# 0026 — Exact sets for preconditions and commas; constant masks as remainders (M4.4d)

- **Status:** accepted for M4.4d (2026-10-27), revised after audit round 1 (Opus: no blocking item; every should-fix
  item taken), finalized with M1, M2 and S22, and revised after audit round 2 (no blocking item; its three should-fix
  items taken: the complement form, the `c_pow2` bound, the docs' type of a mask). Owner decisions:
  - **Exact sets** (old handoff §11 question 27, 2026-10-23): "Do it now since we know it is correct." The M4.4
    planning found that exact sets also reject shapes accepted today (`h(f(x), y) + 100 / g(z)`), so the owner chose two
    steps: exact sets first, today's (sound) sets if the exact family is not a chain, a rejection only if both fail.
  - **Constant masks** (handoff §8 question 47, 2026-10-27): the `bit_mask.c [c23]` hang (handoff §3, "From S4") is
    fixed in this session by candidate 1, a mask by a constant written as a remainder.
  - **Expectation changes (pre-approved):** `tests/scalar/negative/reject_unsequenced_split_requires.c` is retired. The
    Viper of programs whose sets or masks change may change, measured and listed (M1, M2); no other verdict or error line
    may.
  - **The M1 stop (2026-10-27):** the VIR case `hook_drop_flag_access` (0025 item 1's guard) stopped firing, since its
    program now takes step 1. A self-test hook `force_base_sets` makes every full expression take step 2; the case sets
    it and keeps its program and message (item 3).

  Facts marked (M*n*) and (S*n*) are measurements and Silicon spikes, recorded with their evidence in
  `internal/il-inventory.md`, "M4.4d".
- **Milestone:** M4.4d (2026-10-27).

## Context

0021 item 3 places every item of a full expression right after the fixed points (calls, and branches that hold a call)
of its *set*, the fixed points C must have completed before it evaluates the item, and requires the sets of the checks
to form a chain. Two kinds of set are smaller than that (0021 item 10, "Smaller sets are safe"): a call's precondition
has the set of the argument values its clauses read, and the value of a comma does not carry its left operand's floor
(0020 E6). An item with a smaller set is asserted earlier than C may evaluate it, which can only report an error on a
path where C would not reach it. Those false alarms hit C idioms: `h((validate(y), y))`, where `validate` ends the
program unless `y > 0` and `h` requires a positive argument, reports `precondition`; `100 / (validate(y), y)` reports
`division-by-zero`; `(nr(0), 1) && 100 / d`, with `nr` never returning, reports `division-by-zero`. And a split contract
is rejected: `take(f(x), h(y))` with `requires(1 <= a)` and `requires(2 <= b)` gives each clause the set of one call.

Separately, 0023 item 2 encodes every `&` as a prelude function that unfolds the bits of its operands. For a mask that
reduces a value (`v & 4294967295u`), whether Z3 answers the range check of the result depends on what the same prover
process checked before; Silicon's parallel verifiers make that order vary, and `tests/scalar/bit_mask.c [c23]` hangs
intermittently (CI run `40434946397`; with one verifier, deterministically).

## Decision

### Part A: exact sets

1. **The exact sets.** Two sets change; no other does.
   - **A precondition's set is PRE(c).** Each `requires` clause of a callee is an obligation at the call (0012 item 6)
     whose set is PRE(c): the comma floor and everything the arguments use or contain. That is the set the explicit
     termination check already has (0021 item 4).
   - **A comma's value carries its floor.** The value of `a, b` has the set of `b`'s value joined with the floor in
     force while `b` was lowered: the enclosing floor and everything `a` contains (0020 E6). Every item that uses the
     value — a check, a guard, a write, a branch condition, an argument — inherits it.
   - **Why exact.** C completes the evaluation of the function designator and the arguments before the call (C17
     6.5.2.2p10; C++ [intro.execution]/11), so every fixed point of PRE(c) is complete when the precondition is checked.
     The value of `,` exists only after its left operand is complete (C17 6.5.17; C++ [expr.comma]), and everything in
     the enclosing floor completed before the comma began. Neither set holds a fixed point C may evaluate after the item: a sibling of the call or of the comma
     is not in either (`(abort(), 0) + 100 / d` still checks the division first, 0020 E6). The other sets stay as they
     were, never larger than what C completes and sometimes smaller (a snapshot's set holds no guard, an argument's
     conversion only that argument's calls): "exact" names the two sets this record makes complete, not every set.
2. **Two steps, for each full expression.** Each full expression, and each branch's own full expression (0021 item 3,
   "Branches with a call"), decides on its own:
   - **Step 1:** the family F (0025 item 2: the sets of the checks and of the call-free branches, D(B) for each branch
     with a call, PRE(c) for each call in the caller's recursion cycle) is built from the exact sets. If it is a chain,
     the exact sets are used for everything: π, the placement of every item (writes and snapshots included), and the
     fail-closed checks.
   - **Step 2:** otherwise F is built from today's sets (the *base* sets: a precondition's set is the argument values its
     clauses read and the floor; a comma's value carries no floor). If it is a chain, the base sets are used, and the
     emitted Viper is exactly today's.
   - **Otherwise** the expression is rejected (`unsequenced_calls`), with step 2's conflict, its witnesses and today's
     message (0021 item 9, 0025 item 2).
   - **Why it is sound.** Each step's sets are never larger than C's (item 1 for step 1; 0021 item 10 for step 2), and
     each step places every item right after exactly its set, so either choice is sound; which one is used only decides
     how many false alarms remain. Step 2 exists because exact sets can conflict where the base sets do not:
     `h(f(x), y) + 100 / g(z)`, where `h`'s `requires` reads only `y`, has the exact sets PRE(h) = {f} and {g}, which are
     not a chain; its base sets ({} and {g}) are. Exact sets can also form a chain where the base sets do not
     (`take(f(x), h(y))` above: both clauses have {f, h}).
3. **The mechanism: two sets in one pass.** Every set lowering computes has two halves, exact and base, joined half by
   half; only the two places of item 1 give them different values. `flush` builds F from the exact halves and, if it is
   not a chain, from the base halves, and then projects the whole full expression onto the half it chose, so π, the
   placement, `check_fixed_order`, `check_variable_order`, the conflict report and the self-test hooks (0011 item 13)
   are unchanged code running on one family. Every base half is contained in its exact half, and `flush` checks it
   (an internal error otherwise, exit 3; a self-test hook breaks it so the check fires). A second hook,
   `force_base_sets`, skips step 1, so that the guards that matter only for step 2 (0025 item 1's access record: in step
   1 the comma's value already orders the write after the flag check) stay reachable, and so that the whole suite's
   Viper under step 2 can be compared with the previous version's. The projection copies every
   field of an item except its set whole, so a field added later cannot be lost in it (audit round 1). (Lowering twice
   with a
   rollback was rejected: it would have to restore temporaries, the prelude list, errors, rejections, the assert count
   and the inner flushes, and the work doubles at each nesting level of branches.)
4. **What the checks cannot see.** A set computed too large (the unsound direction) is invisible to the fail-closed
   checks, as before (M4.3b note 4). Pins guard it: tests that would verify if a set were too large —
   `nr(0) + h((g(y), y))` must fail `precondition` (PRE(h) is {g}, not `nr`), `nr(0) + 100 / (g(y), y)` must fail
   `division-by-zero` (the comma's value carries {g}, not `nr`), and `(nr(0), 1) + 100 / d` must fail
   `division-by-zero` (the floor does not leak to a sibling). Audit round 1 added one per code path: a nested call
   (`h2(nr(0), h((g(y), y)))`, `precondition`: PRE(h) holds only what h's own arguments created), inside a branch
   (`a && (nr(0) + h((g(y), y)) > 0)`, the inner full expression's sets), a comma's value as a guard next to a sibling
   (`nr(0) + ((g(y), 1) && 100 / d > 0)`, `division-by-zero`), and a write of a comma's value
   (`nr(0) + ((x = (g(y), y)), 100 / x)`, `division-by-zero`).
5. **The emitted Viper.** Where step 1 is used and some item's two halves differ, the item moves later: a precondition
   follows the calls in its call's arguments (also in programs accepted before M4.3b: `f(a, g(b))` with a `requires` on
   `a` only now checks it after `g`), and an item that uses a comma's value follows the comma's left operand; π may
   change with them, and with π whether an explicit termination check is printed (0021 item 4). A full expression whose
   two halves agree for every item keeps its Viper, and so does every expression that takes step 2. M1 measured it: 12
   of 1822 `.vpr` changed (six tests in both modes: π in `(one(x), 1) + two(y)`, and the write of
   `x = (sink(…), 2)` after `sink`), each in a file whose trace shows a full expression with differing families, and
   no existing test moves a precondition or takes step 2 to be accepted.
6. **Accepted now:** `h((validate(y), y))`, `100 / (validate(y), y)`, `(nr(0), 1) && 100 / d` and
   `(nonzero(d), 1) && 100 / d` (each verifies; the C tests use `nonzero`, which returns only when `d != 0`, so the
   operand after the comma is reachable and mutation can reach it; the never-returning form is pinned by the VIR cases
   `exact_comma_value_guard` and `_returns`), and a
   callee with one `requires` clause per argument called with an unsequenced call in each (`take(f(x), h(y))`).
   **Still accepted, by step 2:** `h(f(x), y) + 100 / g(z)`; there a false alarm can remain (the documented price of
   the fallback). **Still rejected:** `g(f(x), h(y))` with `short` parameters (each argument's conversion is a check on
   its own call, in both steps), `100 / f(x) + 100 / g(y)`, and every other conflict of 0021 item 3 and 0025 item 2.

### Part B: constant masks as remainders

7. **The rule** (amends 0023 item 2). In `a & m`, with the constant on either side (and so `a &= m`, which 0023 item 3
   lowers through the same operator), when the operand `m`, *as lowering emits it*, is a closed expression of integer
   literals (sums, differences, products, negations, `c_pow2` of a literal from 0 to 128, the prelude's table:
   `INT_MIN` arrives as `(-2147483647) - 1`,
   `UINT_MAX` as `(2147483647 * 2) + 1`, `~15` as `(-1) - 15`), its value is computed exactly, and when that value M is a
   mask of the computed type (width N, 32, 64 or 128):
   - `m = 2^k − 1` (`0 ≤ k ≤ N` for an unsigned type, `0 ≤ k ≤ N − 1` for a signed one) is written `a % 2^k`;
   - `m = ~(2^k − 1)`, that is `−2^k` for a signed type (`0 ≤ k ≤ N − 1`) and `2^N − 2^k` for an unsigned one
     (`0 ≤ k ≤ N`), is written `2^k * (a \ 2^k)`, the same value as `a − a % 2^k` (SMT-LIB's `div` and `mod` by a
     positive divisor d satisfy `a = d * (a div d) + a mod d`), with `a` written once: audit round 2 found that
     `a − a % 2^k` doubles the text at each level of nested complement masks (`((x & ~3) & ~7) & …`: 284 KB at depth
     14).

   The value is the operand's own: each `~` and each conversion was already applied in its own node's type when the
   operand was lowered (`v & ~0u` with a 64-bit `v` has M = 2^32 − 1, the low 32 bits, as in C; `x & ~0xFFu` with a
   `long long x` has M = 2^32 − 256, not a mask), and cerastes never wraps a value (0022), so M is the value the prelude
   function would have read. The operand is still lowered as before, so the obligations it carries stay; the rule only
   replaces the call of `c_[u]and_N`. An operand that is not such a closed expression, or whose value is not a mask,
   keeps the function, which is exact for every value (0023 item 2). Every `|` and `^` keeps its function.

   `%` is Viper's: Silicon translates it to SMT-LIB's `mod` (`ExpressionTranslator.scala:150`,
   `TermToSMTLib2Converter.scala:206`), whose result for a positive divisor is the remainder in `[0, divisor)` (the
   Euclidean one; silver leaves `/` and `%` on negative operands to the back end), unlike C's `%`, which keeps
   `c_rem`/`c_urem`; Viper's `\` is SMT-LIB's `div`, the matching quotient. Lowering builds each only through a
   constructor that takes the exponent k and prints a power-of-two literal divisor (≥ 1), and `scripts/lint_viper.py`
   refuses any `%` or `\` outside the prelude functions that use them whose divisor is not such a literal; a
   divisor-zero failure cannot arise. The rule adds no obligation: the result is in the type's range, as
   before.
8. **Why it is exact.** Every operand is in its type's range (0010 item 2), and 0023 item 1 reads it as N-bit two's
   complement. Its low k bits, read as an unsigned number, are `a mod 2^k` (the Euclidean remainder, for negative `a`
   too), so `a & (2^k − 1) = a mod 2^k`; and `a & ~(2^k − 1)` clears those bits, `a − (a mod 2^k) = 2^k · (a div 2^k)`,
   which lies in the type's range. For a signed type k stops at N − 1: the signed value of the all-ones mask is −1, and `a & −1 = a`,
   which the complement form with k = 0 already gives. **S22** checks the rewrite against the prelude function by
   Silicon evaluation at N = 32, 64 and 128, signed and unsigned, for k ∈ {0, 1, 8, 16, N/2, N − 1, N} where allowed,
   at the range's bounds, at negative operands and at random ones; the near misses (`254`, `256`, `2^k`) and the
   cross-type cases above; Viper's `%` on a negative dividend; and, with Z3 directly, the identity for every operand
   of every (N, k, signedness). Those runs used the first complement form `a − a % 2^k`; for `2^k * (a \ 2^k)` S22
   was rerun (591 Silicon asserts at every N, k and signedness, negative operands included), and Z3 proved
   `a − a mod 2^k = 2^k · (a div 2^k)` for every k from 0 to 128 (Viper's `\` is SMT-LIB's `div`, floor division for
   a positive divisor).
9. **Decided now** (amends 0023 item 2's list of what Z3 does not decide): `v & 0xFFFFFFFF == v % 4294967296` for an
   unsigned 64-bit `v` (C's `%` agrees with the Euclidean remainder on non-negative operands), `x & ~15 == x − x % 16`
   for `x ≥ 0`, `x & 255` for every `x` (against `((x % 256) + 256) % 256` in C's terms), and the range checks of
   masked results (`(unsigned)(v & 4294967295u)`). The rest of the list (`(u | 4) & 4 == 4`, `x & x == x`, a variable
   mask, `x & -x`, …) stays undecided. **Only a mask that is a constant where it is written** counts: a mask that is a
   parameter becomes a constant only at a call (`f(v, 4294967295u)` with `requires((v & m) == v)`), and while the
   explicit check lowering writes at the call is rewritten, Silicon's own check of the callee's precondition there
   still reads `c_uand_64(p_v, p_m)`; such a claim may still run to the time limit (exit 3, never a verdict; audit
   round 2, a probe of 90 s). Writing the constant in the contract decides it.
10. **The emitted Viper** changes in exactly the programs that call a `c_[u]and_N` function with such a constant
    operand: 42 of the suite's `.vpr` (21 programs in both modes), each calling `c_and_`/`c_uand_` before; the 22 others
    that call those functions keep their Viper (their masks are not constants), and no verdict changes (M2). **The
    hang:** `bit_mask.c` and `bit_mask_post.c` [c23] give their expected outcome in every one of 52 runs with
    `--numberOfParallelVerifiers 1`, 32 of them with random seeds, in 1.4–1.6 s each (before: 10 of 10 hung).
    **S22:** 1182 closed comparisons with the prelude functions, the `%` probes and 902 bit-vector proofs (Z3, every
    operand of every N, k, signedness and form) agree. These were measured on the first complement form; after audit
    round 2 the complement lines changed and nothing else (34 lines in the 42 files), the 68 mask sources and 124 VIR
    mask programs passed again, 20 more one-verifier runs of the bit_mask tests passed, and 14 nested complement masks
    emit about 1 KB.

### Both parts

11. **No new category, rejection or message.** No `inhale`, `assume` or axiom is emitted (invariant 4).
12. **Tests and documentation.** Part A: must-verify and must-fail pairs for each accepted form of item 6 in both modes
    (the twins fail at their line for the stated reason), the pins of item 4, a case inside a branch, and VIR self-test
    cases for each set and for the fallback; `reject_unsequenced_split_requires.c` is retired and returns as a positive
    with its twin. Part B: positives for each mask form and k at its extremes (`INT_MIN`, `UINT_MAX` included), with
    twins that state a false claim Z3 refutes (never an undecided one), closed near misses, the cross-type pins of item
    7 (`v & ~0u` must fail `r == v`; `x & ~0xFFu` at a closed argument must fail), `sc &= 0xFF` on a `signed char`
    (must fail `conversion`: the conversion back is still checked) and `sc &= ~0x0F` (verifies), a loop that masks a
    signed variable, and VIR cases at each width and signedness. The existing pins that an implementation dropping the
    operand's obligations would break: `integer-semantics/bitwise_conversion.c` (`u & ~7`),
    `negative/bit_mask_negative_conversion.c`, `negative/bit_and_assign_flag_conversion.c`. Since the VIR cases verify
    under either encoding, a harness test checks the emitted text: the masks of `mask_remainder_word.c` and
    `mask_remainder_clear.c` are `%` and `\` forms with no `c_[u]and_N` call, and the near misses of `mask_near_miss.c`
    keep `c_and_32`. Pages: Verifying a
    program ("Calls in expressions"), Integer semantics (bitwise operators); doc examples `verifying/comma_order.c` and
    `integer-semantics/mask_remainder.c`.
13. **Facts relied on** (`internal/il-inventory.md`, "M4.4d"): (M1) the full suite with exact sets: expectation
    changes, the `.vpr` differences and the fallback population; the force-base run (every program's Viper under step 2
    equals the previous version's outside the masks); (M2) the programs that call `c_[u]and_N`, before and after the mask
    rule, and the bit_mask runs; (S22) the mask battery.
14. **Amends:**
    - 0012 item 6 (a precondition is placed by PRE(c), or by its base set in step 2);
    - 0020 E6 (a comma's value carries its floor in step 1);
    - 0021 item 3 (the sets, two steps), item 7 (the Viper of programs accepted before M4.3b changes where a
      precondition or a comma value moves) and item 10 ("Smaller sets are safe": now step 2 only);
    - 0023 item 2 (masks as remainders; the list of what Z3 does not decide);
    - 0025 item 2 (`x = (g(a), 2)`: the write now follows `g` in step 1);
    - `internal/encoding.md` §2 and §3.

15. **For M4.5b.** Step 1 places a write of a comma's value after the comma's left operand, C's order; step 2 keeps
    0025 item 2's placement, which may put a write before a call C makes first. 0025 item 2's caveat (callees do not see
    locals) applies to step 2 in particular when globals and address-taken locals arrive.

## Consequences

- The false alarms of 0021 item 10 are gone wherever the exact sets form a chain; where they do not, cerastes keeps
  today's sets, so no program accepted before becomes rejected.
- A callee may split its contract into one `requires` per argument.
- A mask by a constant `2^k − 1` or its complement, written as a constant, is decided for every operand, and the
  intermittent `bit_mask.c [c23]` hang is gone.

*2026-10-27, amended by 0027 (M4.5a): items 7–10 — the same mask recognition writes `|` and `^` too (0027 items 1–3); "every `|` and `^` keeps its function" now holds only for masks that are not of the two forms. Amended by 0028 (M4.5a, effective from M4.5b1): item 15 — writes and snapshots of a cell are never moved across a call whose contract reaches cells (0028 item 13).*
