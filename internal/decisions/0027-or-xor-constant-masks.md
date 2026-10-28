# 0027 — `|` and `^` with a constant mask (M4.5a)

- **Status:** accepted for M4.5a (2026-10-27), finalized with S23, S23b and M3. Owner decisions:
  - **Handoff §8 question 52** (2026-10-27): "Do it now since we know it needs handled." The recommendation had been to
    wait for a port; the owner chose to extend 0026 part B to `|` and `^` now.
  - **M4.5a planning** (2026-10-27): the work is folded into M4.5a, as question 47 was folded into M4.4d.
  - **Expectation changes (pre-approved):** the Viper of programs that call `c_[u]or_N` or `c_[u]xor_N` with a constant
    mask operand may change, measured and listed (M3). No verdict or error line may change, and nothing is retired.

  Facts marked (M*n*) and (S*n*) are measurements and Silicon spikes, recorded with their evidence in
  `internal/il-inventory.md`, "M4.5a".
- **Milestone:** M4.5a (2026-10-27).

## Context

0023 item 2 encodes `&`, `|` and `^` as total prelude functions `c_[u]<op>_N`, exact for every operand, of which Z3
decides little beyond closed values, ranges and bounds by a constant mask. 0026 part B wrote `a & m` with a constant
mask m as a remainder or a cleared quotient, which Z3 decides for every operand, and left every `|` and `^` on the
prelude (0026 item 7). A claim about `x | 0xFF` or `x ^ 0xFF` over a variable therefore still runs to the time limit
(exit 3), never to a verdict. Character code (M4.6: case bits, `c | 0x20`, `c ^ 0x20`) and the M4.8 ports will meet
them. The same two masks as 0026 part B give exact forms for `|` and `^`.

## Decision

1. **The rule** (amends 0023 item 2 and 0026 item 7). In `x | m` and `x ^ m`, with the mask on either side (and so
   `x |= m` and `x ^= m`, which 0023 item 3 lowers through the same operators), when the operand m, as lowering emits it,
   is a closed expression whose value M is a mask of the computed type (width N = 32, 64 or 128), recognised exactly as
   0026 item 7 recognises one for `&` (the same evaluation, the same two forms, the same ranges of k). Let
   H = `x \ 2^k` and L = `x % 2^k` (Viper's floor quotient and non-negative remainder, 0026 item 7):

   | Shape | M | Written as |
   |---|---|---|
   | `x \| (2^k − 1)` | 2^k − 1 | `2^k * H + (2^k − 1)` |
   | `x ^ (2^k − 1)` | 2^k − 1 | `2^k * H + ((2^k − 1) − L)` |
   | `x \| ~(2^k − 1)`, signed | −2^k | `L − 2^k` |
   | `x \| ~(2^k − 1)`, unsigned | 2^N − 2^k | `L + (2^N − 2^k)` |
   | `x ^ ~(2^k − 1)`, signed | −2^k | `(L − 2^k) − 2^k * H` |
   | `x ^ ~(2^k − 1)`, unsigned | 2^N − 2^k | `(L + (2^N − 2^k)) − 2^k * H` |

   k ranges as in 0026 item 7: `0 ≤ k ≤ N` unsigned and `0 ≤ k ≤ N − 1` signed for 2^k − 1; `0 ≤ k ≤ N − 1` signed and
   `0 ≤ k ≤ N` unsigned for the complement. Where a value is both forms (the unsigned all-ones mask, k = N or k = 0),
   the first row that matches is used; both give the same value. The operand x is lowered as before, so its
   obligations stay; the rule only replaces the call. No obligation is added: the result is in the type's range.
   Every other `|` and `^` keeps its prelude function, and so does `&` outside 0026's forms.
2. **Why it is exact.** Every operand is in its type's range (0010 item 2), read as N-bit two's complement (0023 item
   1). Write x = 2^k · H + L with 0 ≤ L < 2^k: L is the low k bits of x, and H, read in N − k bits of two's
   complement (signed) or unsigned, is the rest.
   - `x | (2^k − 1)` sets the low k bits: 2^k · H + (2^k − 1).
   - `x ^ (2^k − 1)` flips them: 2^k · H + (2^k − 1 − L).
   - `x | ~(2^k − 1)` keeps the low bits and sets every higher one: as a signed value the high part is −1, giving
     L − 2^k; unsigned it is 2^(N−k) − 1, giving L + 2^N − 2^k.
   - `x ^ ~(2^k − 1)` keeps the low bits and flips every higher one: the flipped high part is −H − 1 (signed) or
     2^(N−k) − 1 − H (unsigned), giving L − 2^k − 2^k · H and L + 2^N − 2^k − 2^k · H.

   Each result lies in the type's range. **S23** checks every form against the prelude function by Silicon evaluation
   at N = 32, 64 and 128, signed and unsigned, for k ∈ {0, 1, 8, N − 1, N} where allowed, at the range's bounds, at
   negative operands and at random ones, and with Z3 directly for every operand of every (N, k, signedness, form).
3. **x written once.** The `|` forms name x once (through `\` or `%`), inline. The `^` forms need H and L, so each is a
   prelude function with a body and a range postcondition, emitted only when used (as `c_[u]xor_N` is), one per form,
   signedness, width and k: `c_[u]xor_mask_N_k(a)` for `x ^ (2^k − 1)` and `c_[u]xor_cmask_N_k(a)` for
   `x ^ ~(2^k − 1)`, whose body is the form of item 1 with the parameter `a` for x and whose postcondition is
   `in_range(a) ==> in_range(result)` (guarded by the type's range, so it holds for every `Int` and needs no
   `requires`, hence no new failure reason at a call). x is then named once, as the call's argument. **Why not inline:**
   S23 first used a Viper `let` to bind x (Viper refuses a `let` that shadows an enclosing name, so the names had to be
   unique per function); it is exact, but a nested chain makes Z3 prove the range of the whole term at once, and
   `((((x ^ 255) ^ 15) ^ 3) ^ 1)` repeated to depth 8 had no result in 200 s (M3's measurement). With the functions, Silicon
   proves each postcondition once and uses it at every call: chains of depth 8, 16 and 32 verify in about 2 s, and
   identities through the bodies (an eight-mask chain that cancels, `x ^ 255 == x − 2·(x % 256) + 255`) are proved, a
   wrong twin refuted (S23b). `%` and `\` are still built only with power-of-two literal divisors, so
   `scripts/lint_viper.py`'s rule holds unchanged. A closed operand that is itself such a call is not evaluated by the
   mask recogniser: the outer operator keeps its prelude function, which is exact (no approximation, only less
   decided).
4. **Decided now** (amends 0023 item 2's list and 0026 item 9): claims over a variable about `x | 255`, `x ^ 255`,
   `v | 0xFFFFFFFF` for a 64-bit `v`, `x | ~15`, `u ^ ~0xFFu` in `u`'s own type, and identities such as
   `(x ^ 255) ^ 255 == x` and `(x | 255) == x - x % 256 + 255` for `x ≥ 0`. Still undecided: `(u | 4) & 4 == 4`
   (4 is not a mask), the swap by XOR of two variables, `x & x == x`, a variable mask, and the rest of 0023's list.
   As in 0026 item 9, only a mask that is a constant where it is written counts.
   **Nested `^` chains** (M3 and the threshold runs, serial, `--timeout 120`; `build/m4.5a/xor-threshold.md`): a
   **range** claim through a chain is decided at every depth measured (1 to 32, `int` and `unsigned`, 1–12 s). An
   **identity** `r == x` is decided for `unsigned` at every depth measured (2 to 8, both orders, both modes, 3–9 s); for
   `int`, when the masks cancel in adjacent pairs (`((((x ^ 255) ^ 255) ^ 15) ^ 15) …`) to depth 6 in both modes
   (depth 8 in C only), and when they alternate (`(((x ^ 255) ^ 15) ^ 255) ^ 15`) to depth 3 in both modes; the round
   trip `(x ^ 255) ^ 255 == x` everywhere. Beyond that a signed identity may run to the time limit and end with exit 3,
   never a verdict, as 0023's undecided shapes do; false claims are refuted in 2–5 s at every depth measured. The cause
   is Z3's handling of the identity together with the method's automatic range postcondition, and the shape of the
   postcondition: with the range `ensures` removed by hand the worst cases verify in 2–15 s, and C's
   `((res == x) ? 1 : 0) != 0` against C++'s bare `res == x` alone moves a case between 3 s and a timeout. The Integer
   semantics page says so.
5. **The emitted Viper** changes in exactly the programs that call `c_[u]or_N` or `c_[u]xor_N` with such a constant
   operand, and no verdict changes (M3).
6. **No new category, rejection or message.** No `inhale`, `assume` or axiom is emitted (invariant 4).
7. **Tests and documentation.** Positives for each form and k at its extremes (`| 0`, `^ INT_MAX`, `| UINT_MAX`,
   `^ INT_MIN`), with the mask on either side and through `|=` and `^=`, each with a twin stating a false claim Z3
   refutes (never an undecided one; a twin's failure alone cannot show that, since Silicon reports "might not hold"
   when Z3 gives up too, so S23 and S23b, which ran the same claims to a refutation, are the evidence); closed near misses (`x | 254`, `x ^ 256`) that keep the prelude; a nested `^`
   chain whose emitted size is pinned; a harness check that the emitted `|` masks are `%`/`\` forms and the `^`
   masks calls of `c_[u]xor_mask`/`cmask` functions, with no `c_[u]or_N`/`c_[u]xor_N` call; VIR cases at each width and signedness. The page: Integer semantics (bitwise
   operators), with the doc example `integer-semantics/mask_or_xor.c`. Mutation of the positives goes to M4.8's run
   under handoff §8 question 49's rule.
8. **Facts relied on** (`internal/il-inventory.md`, "M4.5a"): (S23) the forms against the prelude functions; (S23b) the
   `^` forms as prelude functions, nested chains; (M3) the `.vpr` diff against the baseline.

## Consequences

- Character code that sets, clears or flips bits by constant masks (M4.6) is decided.
- The prelude gains, on demand, one small function per `^` mask form used, besides `c_[u]xor_N`.

*2026-10-28, amended (M4.5b1a follow-up, the owner's "fix or remove the single consistently failing CI test", PR #14):
item 4 — the thresholds were measured one claim per prover. Several mask-chain methods verified one after another on
one prover can make a later query run to the time limit (prover history): the seven functions of
`mask_or_xor_round_trip.c` and its twins verified in 1–2 s each alone and in 2 s together at Silicon's default
parallelism on a 16-core machine, but with one verifier (`--numberOfParallelVerifiers 1`) `..._chain_high_part_post.c`
and `..._chain_value_off_by_one_post.c` ran to the limit every time, and with two or three verifiers (GitHub's macOS
runner has three cores) they did so on some runs; dropping any one of several functions removed the hang. Three of
these twins timed out on CI in C23, one per run. The family now has one chain per file (`mask_or_xor_round_trip.c` with
the three depth-2 round trips, `mask_or_xor_chain.c`, `mask_or_xor_chain_value.c`, `mask_or_xor_chain4.c`,
`mask_or_xor_chain8_range.c`, each twin with only its own function, its other lines comments, no expectation changed):
every file verifies or fails as expected in 1–3 s with one, two or three verifiers in both modes. The claims of item 4
stand, each for a function verified on its own; the Integer semantics page says that several such chains in one file
may take longer.*

