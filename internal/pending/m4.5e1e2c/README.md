# Pending: two tests set aside for the owner's decision (M4.5e1e2's continuation, second session, 2026-11-26)

`struct_array_chain_wf_two_pointers.c.txt` and `struct_array_chain_wf_division_two_pointers.c.txt` are two
test files as a test-mutator wrote them for `tests/memory/` and `tests/memory/negative/` (their first names:
`struct_array_chain_wf.c`, `negative/struct_array_chain_wf_division.c`). They are kept here unchanged, with
`.txt` appended so that no tool reads them as tests. Handoff §8, question 357, has the account.

**What they are.** The pin pair for the ghosts of a contract's well-formedness method (`f__wf`) when the
contract reads an element of an array member through a pointer read from memory (`p->q->a[e]`), in the form
with TWO loaded pointers (`reads(*p->q) && reads(*p->q2)`), whose `other_root` row tells the two loaded
structures apart. On the session's final build both give their registered results (the positive verifies;
the twin reports its five `division-by-zero` rows).

**Why they are not in the suite.** With two atoms of one kind through two loaded pointers, the `ensures` rows
and the return rows of the array part carry record 0045 item 7's partner-sum amount (nested conditionals over
the method's `Perm` formals, selected by comparing loaded pointers with their `old` values).
`scripts/lint_viper.py` refuses that amount on a quantified row (and reads its `old(` as a kept-flag row over
a block reached through a heap read): 96 problems over the four emitted files, so `scripts/check.sh` would be
red with these two files in `tests/`. A test-mutator tried to edit the lint to admit the shape; **the
permission system denied that edit ("Undermining Oversight"), the agent stopped, and nobody made the edit in
its place.** A reduced pair with ONE loaded pointer (no partner sum; its Viper passes the lint unchanged) is
in the suite under the two original names; the `other_root` row therefore has no C-level pin.

**What the auditor found** (audit round D2, item 12, which read the amount code): a method without a body does
carry this amount in its `ensures`, and its callers inhale it; the contract's well-formedness method carries
the obligation (`q_post == q_partner_pre` per site) that makes the amount the partner's own formal; in eight
probes with a verdict no caller ended with more permission than it gave; six more probes timed out and are
not checked; the rest of `atom_returns` was not read.

**The rule that was proposed and not applied** (the test-mutator's words, from its report 8):

> **What I would write and was not allowed to apply:** a function `partner_amount_ok(text, formals)`: a top-level sum of terms, each `none`, a `Perm` formal of the method, or a conditional `c ? T : T` of such terms,
>   the condition free of `perm`, `acc`, quantifiers and labelled `old`, reading no heap except a pointer load `.v_ptr_struct_<n>_<t>` (also under `old(...)`); used (i) in `block_row_problem`: an `ensures` acc row over a
>   load-rooted block may carry such an amount, and the kept-flag refusal is narrowed to a consequent that really is `old(F.d_t) ==> F.d_t`; (ii) in `shaped` and `marked_row_shape(code, formals)`: a marked `assert` row (a check, never an
>   inhale) may carry such an amount. Twins for each (the amount with a leaf that is not a formal, a `perm(...)` or a scalar heap read in a condition, a quotient, an `ensures` row of a block reached through no load,
>   a `requires` row with the sum, the kept flag still refused over a load) and a pass over the chain_wf pair. **The permission system denied the edit that makes this change** (the classifier reads it as
>   loosening the lint); I stopped and did not apply it by another route. **Soundness note for whoever decides:** the marked `assert` rows are checks (they cannot add permission); the `ensures` rows with the sum stand in methods WITH a body (every flagged line, above), so they are exhaled at the end; they would be INHALED by callers if the same shape stood in a bodiless contract, where an amount above the callee's own `requires` amounts would be unsound; each leaf is one of the
>   method's `Perm` formals, but the lint does not check that the chosen formal is the one the `requires` gave that object (no rule reads the choice). The rule as I would write it does not restrict to methods with a body (the emitted corpus has no bodiless instance); the auditor should judge whether it must.

**For the owner:** approve a lint rule for this amount (a session then writes it with its refusing twins,
restores the two files under their first names and drops the reduced pair or keeps it beside them), or keep
the shape listed as one the lint refuses.
