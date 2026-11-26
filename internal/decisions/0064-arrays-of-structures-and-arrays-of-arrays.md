# 0064 — Arrays of structures and arrays of arrays (M4.5e1e2)

- **Status:** accepted for the items built so far: **item 15 (Part D) by the continuation's second session
  (2026-11-26: the leftovers of a structure with an array member, for array members of scalars; two audit
  rounds and a short read, below);** those of the continuation's first session (2026-11-26:
  Parts R, C1 and C2, named below: items 5, 16 and 19 as amended, items 7 to 9, and the range, global and
  member parts of items 10 and 12, with item 17's rules through Part C2; the full check green at its Commit 3
  `46f34fb`), and those its first session built (2026-11-25, M4.5e1e2: items 2 to 6 and 10 to 12
  as far as automatic local arrays of structures in code go, item 16, item 17's first rules, and item 19 in its
  first version, which the continuation withdraws); **the rest is decided and is built by the continuation,
  from a plan of its own** (`.claude/plans/m4-5e1e2b-arrays-of-structures.md`, written on 2026-11-25 at a
  session that only planned, at the owner's request, in place of question 310's "no second planning"; its parts
  in order: **Part R**, item 19 rewritten — every `restrict` parameter rejected — with item 5's rule for every
  element address and item 16's lone zero as P84 (a) decides; **Part C1**, items 7 and 8 and the range parts of
  10; **Part C2**, item 9 and the global and member parts of 10 and 12; **Part D**, item 15; **Part E**, item 13
  and depths 2 and 3; **Part F**, item 14). **Amended on 2026-11-26, before Part R's code** (invariant 6), from
  section 6 of that plan: this header; items 1, 5, 7, 12, 13, 16 and 19; the table of facts (P84); the
  consequences. **The continuation's first session built Part R (its Commit 1) and Part C1 (its Commit 2:
  items 7 and 8 and the range parts of 10 for a pointer parameter's range over structures, with round R's
  fixes) and Part C2 (its Commit 3: item 9, the global and member parts of items 7, 10 and 12, with round
  C1's code fixes). The continuation's second session built Part D (its Commit 4: item 15, with the fixes
  of audit rounds D1 and D2) and closed at the plan's split point; Parts E and F (items 13 and 14, with
  item 12's clause for Part E's structure values) stay "decided, built by the next session".** Written before code
  (invariant 6) from section 7 of the plan
  (`.claude/plans/m4-5e1e2-arrays-of-structures.md`, written at a session that only planned and executed without
  re-planning). Revised by P83's and S48's decision tables (the plan's section 6; `internal/il-inventory.md`,
  chapter M4.5e1e2; the fixtures `tests/viper-smoke/memory/s48_*.vpr`; the table at the end) and by audit round 1
  (a fresh Fable auditor on Commit 1 `571fe59`: blocking 1, a wrong program that verifies on the base, C's
  `restrict` beside a global the body names, not of this record: question 326; blocking 2, a walker missing
  from Part A's table, item 4; should-fix 3 and 4, how the pairing rows are written, item 7; 5, item 16's C
  clause; 6, spike evidence to complete; 7, the name `<E>`, and 8, the scope exit's pin, item 6; 9, a second
  walker, items 4 and 11; 10, stale text; notes 13–15 in items 7, 11 and 12;
  `build/m4.5e1e2/auditor-report-1.md`), and by audit round 2 (a fresh Fable auditor on the joined snapshot of
  Parts A and B, `83f447c`: Parts A and B held, a verdict per row of Part A's table; blocking 1 and 2, two more
  `restrict` forms that verified, both on the base, item 19's clause (iv) and pair rule; should-fix 3 to 6 taken,
  7 and 8 to the handoff; `build/m4.5e1e2/auditor-report-2.md`), with a short read of the fixes and the pages
  (`auditor-report-2b.md`).
  Owner decisions:
  - At M4.5e1e2's planning (2026-11-25): **310** one complete plan run under the 12-hour rule, in the order arrays
    of structures (code, then contracts and storage), the leftovers, arrays of arrays, quantifiers; the parts a
    session does not begin run from the same plan in a second session, and nothing is cut to hold a session short;
    **311** the full check ran green on M4.5e1e1; **312** question 304's rejection is built here (item 16);
    **304** (inline, 2026-11-25): "Follow the rejection like the compiler warning."
  - Defaults the plan builds on, for the owner's review (each fail closed; `internal/handoff.md` §8): **313** an
    array or a range whose element structure has an `owning` or marked member is rejected (M4.8); **314** no leaf
    path deeper than two index steps, depth 3 only if S48 (m) verifies it under 30 s a program (it did: depth 3 is admitted); **315** a pointer
    to an array is a parameter's type only (M8 elsewhere); **316** `restrict` backed by a range over structures is
    rejected (M4.8); **317** a whole-structure copy of a structure whose member is an array of structures is
    written leaf by leaf under the by-value limit of 7 and rejected beyond it (M4.5f3); **318** a store to `v[i].x`
    in a loop reaches every place of its field (a listed false alarm, M4.8); **319** the tutorial section "Arrays
    in aggregates" is written by the session that lands Part C and extended by the one that lands Part E.
  - From M4.5e1e's planning and M4.5e1e1: 290–300 and 301–309 as built (record 0063); 292–294 and 306 are lifted
    here (item 15); 298 puts the tutorial section here.
  - Standing: 69 (no EDG work), 77 (one context, under about 12 h), 160 (a new test's expect line corrected after
    its first run, listed in status), 176 (the owner handles CI), 277 (a twin with several real errors lists every
    one), 285 and 299 (a test slow on the box only because of what it must refute goes to `tests/long/` with a
    short twin).
  - Questions of the first session (`internal/handoff.md` §8): four were the owner's and were **answered at the
    planning of the continuation (2026-11-25, the question dialog):** **326** (`restrict`, unsound on `main` and
    with one form open on the first session's branch) "Reject restrict (Recommended)": every `restrict`
    parameter is rejected with a diagnostic that says to drop the keyword, the acceptance rules of records
    0037, 0040, 0043, 0044, 0045, 0047, 0052, 0054, 0060 and this record's first item 19 are withdrawn and
    their code removed, exactly 17 test files change outcome (8 positives become rejection pins, their 9 error
    twins are retired), and a model of `restrict` is an M8 entry (item 19); **320** (the parameter spelling `T
    (*a)[M]`) "Accept it in Part E (Recommended)" (item 13); **322, second half** (a truth test or null
    comparison of an element's address on an array of scalars) "Reject them too (Recommended)" (item 5);
    **325** (a lone zero that is not the literal `0`) "Probe, reject if the IL can tell" (item 16; P84 (a)).
    Settled with the advisor in the first session: **321** and **327** (item 16 is the measured rule of both
    compilers, designated lists included), **322** (the address tests of item 5 on arrays of aggregates),
    **323** (the depth-2 copy is built; its time is recorded, item 12), **324** (25 fixtures over the spike's
    cap), **328** (the cost of several arrays of structures in one function, item 6), **329** (the first item
    19's clause (iv) and pair rule, withdrawn with it), **330** (one self-test case corrected after the first
    run of the full check).
  - Defaults the continuation's plan builds on, for the owner's review (each fail closed or a matter of
    process; `internal/handoff.md` §8): **331** Part C is two parts, C1 (ranges over structures in contracts)
    and C2 (static storage and members), each with a commit, an audit round and a split point; **332** the
    `restrict` rejection covers a declaration without a body as well as a definition (item 19; question 338
    says where: at its first call, and one the unit never calls is not reported); **333** a
    structure value stored into an element (`v[i] = f();`) stays rejected until M4.8 (item 12); **334** the 33
    pins that already reject `restrict` stay, with only their `Pins:` comments changed; **335** `&p[0]` in a
    truth test or null comparison is rejected with the other indexes, for scalars and structures alike (item
    5); **336** `scripts/box_lane.sh` refuses to run without its two lane variables; **337** the harness's
    twin rules shape the 17 files' flips: the 8 re-expected files move to `tests/memory/negative/`, a new
    positive with the keyword dropped and its error twin stand for the rule, and the 14 pins whose `twin:`
    line named one of the 8 name the new positive.
  - Questions of the continuation's sessions (`internal/handoff.md` §8), each settled with the advisor: **338**
    (where a `restrict` declaration without a body is reported, item 19), **339** (the lone zero that is not a
    literal: the spellings the IL tells, item 16), **340** (a staging pin's row that Part C1's lift removes one
    part early), **341** (a function without a body is rejected if ANY of its declarations has the qualifier,
    item 19), **342** (item 5's rule is over-strict on a family no compiler flags, listed) and **344** (the
    contract reads `g[e].x` and `p->v[e].x` are built in Part C2, item 7); **343** (a structure pointer formed
    outside what the function's atoms cover, passed on or returned: an exit 3, item 7) went to the owner,
    **who answered it on 2026-11-26, inline in the handoff: "Follow alternative plan since it sounds like
    the correct fix."** The alternative: the exit 3 stays listed until M4.5e2, the pointer-arithmetic
    session, which gives pointers into arrays their bounds rows; no row is built in M4.5e1e2. The owner let
    338 to 342 and 344 to 347, and the earlier defaults and rulings, stand ("Follow recommendation").
  - Questions of the continuation's second session (2026-11-26, Part D; `internal/handoff.md` §8), each
    settled with the advisor: **348** (item 15 and the plan's Part D table were written before Part C2
    and speak of an array member of scalars: a structure whose array member has AGGREGATE elements stays
    rejected at Part D's six positions, item 15's last clause; fail closed, for the owner's review);
    **349** (a `must_init` filler that misses an element is `postcondition`, item 15); **350** (0052 item 2a
    stands: two rows the plan listed as flips stay rejected, item 15); **351** and **354** (`calloc`'s zero
    values one clause per element, and the cap of 24 values that followed from it: for the owner, item 15);
    **352** and **353** (Part E's: a `const` row parameter stays rejected, item 13; whole copies of Part E's
    structure types cell by cell, item 12); **355** (written atoms at `write`, item 15); **356** (test
    mechanics: the owning-member reassignment's false alarm is pinned by its correct form); **357** (a lint
    edit that would have admitted record 0045 item 7's partner-sum amount on an array part reached through
    a heap read was DENIED by the permission system; two tests of that shape are set aside under
    `internal/pending/m4.5e1e2c/` for the owner; item 17).
  - **Audit round D1** (a fresh Fable auditor on `b193add01de6`: Part D's encoder half and the lowering's
    heap, marker, partnerless and heap-read columns, with a heap block's array part as a loop place;
    `build/m4.5e1e2c/auditor-report-D1.md`; about 100 probes): no wrong program verifies and no blocking
    finding; every stop for what was not yet built held. Should-fix, all taken: 1, four tests failing at
    that snapshot (parts not yet built); 2, this record said the written atom was built before it was; 3,
    no C-level test of the loop place; 4, a lint relaxation that hid an obligation (item 17); 5, three of
    question 348's backstops without a case. Notes in item 15; one exit 3 not of this record (a recursive
    function with a heap-read atom and no tie; the same on the base build). It read `check.cpp`'s diff in
    full, `encode.cpp`'s changed lines and, in `lower.cpp`, the loop place, its gate, `block_cells` and half
    of `block_conj`; `block_rows` and its sinks, the marked, output and heap-read parts of `atom`,
    `call_atoms` and `atom_returns` and `contract_read`'s block loop it judged by probes only.
  - **Audit round D2** (a fresh Fable auditor on the final source `fc484b966ee6`;
    `build/m4.5e1e2c/auditor-report-D2.md`; 79 probes): no wrong program verified in any probe with a
    verdict, no blocking finding, no crash. It read every added line of `lower.cpp` in its diff (the loop
    carriers, written atoms), the `check.cpp`, `encode.cpp` and lint diffs in full. Should-fix: 2, a
    memory-reading guard on a written atom's array part was lowered to a constant (fail closed; now
    rejected, item 15); 3, a written `owns(c->a)` on a scalar member of a structure with an aggregate array
    member was accepted (now rejected, item 15); 4, this record's and the page's sentence about flags
    across a loop (corrected); 5, the refused written forms printed the checker's generic tag (the encoder
    rejects them itself now); 6, rules without a C-level test (15 files added). It also read the amount
    code behind question 357's shape: a method without a body does carry the partner-sum amount, its
    well-formedness method carries the obligation that makes it the partner's own formal, and no caller
    with a verdict gained permission (eight probes; six timed out). Not read: the self-test cases beyond
    names, `atom_returns` beyond the array-part rows.
  - **The short read D2b** (a fresh Fable auditor on `55f83710ea48`, which is Commit 4's `src/`;
    `build/m4.5e1e2c/auditor-report-D2b.md`): both diffs since round D2's snapshot read in full (the fixes
    in `src/`; the lint rules written after that snapshot): no blocking finding; every form the fixes mean
    to reject is rejected but one family that is accepted by design of the shared predicate (a guard that
    reads an address-taken local by name: item 15 says what is built); the lint's kept-flag and amount
    refusals are unchanged (the denied relaxation did not come back); two places where the new lint rules
    admitted too much were tightened afterwards (item 17).
  - **What no auditor read** (Part D): the self-test cases beyond their names; `atom_returns` beyond the
    array-part rows; the functions round D1 judged by probes only (above); the two lint tightenings made
    after the short read, the lint self-test's diff and its fixtures. The auditors ran tests through
    `cerastes verify` in C23 for the most part; probes without a verdict are not checks.
  - **Audit round R** (a fresh Fable auditor on the joined snapshot of Part R, `c263b457938f`; `build/m4.5e1e2b/
    auditor-report-R.md`): no wrong program verifies and no blocking finding; should-fix 1, a called function
    without a body whose declarations disagree on `restrict` is accepted (item 19, question 341); 2, item 5's
    rule misses a folded constant address as the condition of an `if` in C (item 5); 3, question 339's rule had
    no page statement and one call site without a test row (item 16); notes 4 to 8 in items 5, 16 and 19. The
    record and the pages were made true of that build before Commit 1; the code and test fixes land with the
    continuation's Commit 2 and are read again by the audit round of Part C1.
  - **Audit round C1** (a fresh Fable auditor on the joined snapshot of Part C1, `0513b158e4e2`;
    `build/m4.5e1e2b/auditor-report-C1.md`; about 575 runs): no wrong program verifies and no blocking finding;
    round R's fixes hold. Should-fix 1, the sequencing rows of a call that holds a range over structures have
    no other check behind them and had no C-level test (items 7 and 11: pinned with Commit 2); 2, a `pure`
    application beside a range over structures is framed as an interval of scalar cells (`apply_frame`; fail
    closed, an exit 3 beside the right diagnostics); 3, a quantifier beside such a range is accepted with a
    body outside 0061 item 11 (the lint refuses its Viper); 4, a range over a structure with a
    pointer-to-structure member is accepted and right, but untested, and the lint refuses its Viper; 5,
    `f__wf`'s ghosts per leaf and slot had no must-fail test (pinned with Commit 2); 6, the lint's scope-exit
    rule is switched off by any `assert false` in the method; 7, an unlisted cost (item 7). Items 2, 3, 4 and
    6 are code and lint fixes that landed with Commit 3 (Part C2; items 7 and 17 say how). Audit round C2
    reran the 31 probes of the code fixes (2 and 3: the outcomes hold, no exit 3); it did not check the lint
    rules of 4 and 6.
  - **Audit round C2** (a fresh Fable auditor on the snapshot of Part C2, `a044aff629b8`;
    `build/m4.5e1e2b/auditor-report-C2.md`; 387 probes): no wrong program verifies and no blocking finding.
    It rested on its probes and on the functions it names: most of the `lower.cpp` and `encode.cpp` diff, the
    consumers of `leaf_arrays` and the lint diff it did not read. Seven of its probes got no verdict and are
    not checked.
    Should-fix 1, an exit 3 on accepted programs: the length quantifier of a member array of structures
    reused the binder `k` under another quantifier (item 3, "A length quantifier under another quantifier":
    fixed with Commit 3); 2, the lint at that snapshot refused Part C2's rows (fixed by the lint round that
    followed the snapshot); 3, seven Part C2 rules without a C-level test (written with Commit 3); 4, two
    rejections this record did not list (question 346: items 7 and 12); notes 5 to 8: a list of about a
    thousand structure elements (item 9), costs (item 12), three messages not in source terms (item 9), three
    page statements (corrected).
  - **Audit round C2b** (a fresh Fable auditor, a short second read on the final snapshot `588967a05edf`,
    which is Commit 3's `src/`; `build/m4.5e1e2b/auditor-report-C2b.md`; 78 runs): no wrong program verifies.
    It read the `src/` diff BETWEEN the two snapshots (the clause split of the global facts' length
    quantifiers, made after round C2's snapshot: the same conjunction at its three callers, nothing dropped
    or doubled; the binder fix of item 3: structural and right, `k2` unreachable today and without a case),
    the consumers of `leaf_arrays` and `exempt_owned_fields` by their branches, and part of the lint diff;
    it probed the thirteen shapes the binder fix newly accepts and, in C++, the by-value element of item 12.
    Its findings: 1, a verification-time regression between the two snapshots (question 347, item 12); 2 and
    3, `fp(gt.v[1])` is accepted in C++ where this record said rejected, and right in twenty probes
    (question 346, item 12); 4, the lint still refuses four classes of accepted shapes that no test holds;
    5 and 6, notes. Seven of its probes got no verdict and are not checked.
  - **What no auditor read:** the body of Part C2's diff against Commit 2 in `lower.cpp` and `encode.cpp`
    beyond the functions the two reports name (both rounds rested on probes there: 387 and 69), the
    self-test cases, round C1's lint rules for its findings 4 and 6, the lint lines written after round
    C2b's snapshot (the nested-quantifier rule and three placements), and mutants of the sequencing rows
    (the il-analyst's replay stands alone).
- **Milestone:** M4.5e1e2.
- **Amends:** 0012 item 11 (the lint); 0024 item 6 (array types of aggregates); 0028 items 2 and 16 (as built); 0043
items 6, 7, 9 and 11; 0044 items 2–5 and 8; 0045 items 1 and 12; 0047 items 2, 4, 6 and 8; 0052 item 2; 0055
items 2, 3 and 5; 0058 item 4; 0059 items 1–4, 6, 8 and 9; 0060 items 1, 3–5, 8, 9, 11–13 and 15; 0061 items 1, 3
and 6; 0062 item 4; 0063 items 1, 2, 4–10 and 13–15; and, their `restrict` clauses withdrawn by item 19: 0037
item 2, 0040 item 3, 0043 item 10, 0044 item 11, 0045 item 13, 0047 item 4, 0052 item 6 (d), 0054 item 2 and 0060
item 14.

## Context

Record 0063 (M4.5e1e1) gave arrays of scalars every storage: globals, `static` locals, structure members, compound
literals, structure values. An array whose element is a structure or an array is still rejected with a diagnostic
that names M4.5e1e2, and six rules for a structure with an array member were left to this record (questions
292–294 and 306, partnerless atoms, written atoms). Record 0028 items 2 and 16 fixed the layout in M4.5a (element
`i` of an array is the slot `loc(B, i)`; a structure element's member `k` is the block `member(loc(B, i), k)`; row
`i` of `int m[2][3]` is `member(loc(B, i), 0)`), and M4.5e1e1's spike S47 (x) verified its two unknown shapes on
the box: one quantifier per leaf over an array of structures (9.2–9.9 s) and a permission over two bound variables
for a matrix (8.7–8.9 s).

Facts: P83 (the IL of arrays of structures and of arrays, their initializers, array-of-rows parameters, members,
globals, quantifier bodies and arguments, and the compiler matrix M) and S48 (Silicon on the shapes of items 3–15),
in `internal/il-inventory.md`, chapter M4.5e1e2; the table at the end maps each outcome to the branch of the
plan's decision table it took.

## Decision

**Terms.** A **scalar** is `bool`, an integer type, an enumeration or a pointer to an object type. A **verified
structure** is 0043 item 2's, with the array members 0063 gave it. An **aggregate element** is an array element
that is a verified structure or an array. A **leaf path** runs from an object's first slot to one scalar cell by
**index steps** (`[i]`, with a count) and **member steps** (`.k`); between two index steps stands the member
step 0 that 0028 item 16 gives a row. Its **depth** is its number of index steps: 0063 accepted depth 1 with the
index step last (`s.a[i]`, `g[i]`); this record accepts index steps anywhere, to depth 3 (S48 (m) admitted depth 3;
the parts are written for depth 2, and depth 3 takes one more bound variable and one more bound row).

**No existing program's Viper or map changes in this session.** Every item adds rows only for a program that
holds a construct rejected today; where an existing function is touched, the text it writes for a program
without the new constructs is the same, and `vprdiff` shows it after every part. **Accepted forms that become rejections:**
item 16's measured elision rule (the owner's answer to question 304, as measured: question 321), in C++ and, for
nested lists, in C. Step 0 found no existing test or doc example whose outcome it changes; one that does at Part
B's joined proof goes to the owner before it is touched (the plan's Flips G). **In the continuation the same
holds with one listed exception** (the continuation plan's section 3, rule 1, and its Flips R): the 17 test files
the owner approved with question 326 lose their Viper (8 become rejections, 9 are retired); and two more
accepted forms become rejections, item 19's `restrict` parameters and item 5's address tests on arrays of
scalars, and item 16's lone zero that is not a literal (question 339), none of which any other existing test or
doc example holds.

1. **Scope.**
   - **In:**
     - **(S) Arrays of structures.** In code (the plan's Part B): an automatic local `struct S v[N]`; `v[i].x`, `v[i].t.x`,
       `p[i].x` on any `struct S *`, `&v[i]`, `&v[i].x`, the decay of `v`; an element as a structure value
       (`t = v[i]`, `v[i] = t`, `v[i] = v[j]`, `f(v[i])`, `struct S t = v[i]`); braced initializers with a nested
       list per element and C's zero fill; scope exits; loops; sequencing; `sizeof(v)` (an element structure that
       itself has an array member is depth 2 and waits for Part E). In contracts and storage
       (Part C): `owns_range(p, n)` and `reads_range(p, n)` with `p` a pointer parameter to a verified structure;
       element reads `p[e].x` in `requires`, `ensures`, `decreases` and under `old`; calls, returns, `f__wf`; loops
       over a parameter's range; `owning(n)` on such a parameter; P4222's markers over such a range; global and
       `static` local arrays of structures with their initial values, `owns(g)` and `reads(g)`; an array member
       whose elements are structures (`t.v[i].x`, `owns(*p)`, `owns(p->v)`), in every storage of its structure.
     - **(H) The leftovers of a structure with an array member** (Part D): heap storage (`malloc(sizeof(struct
       S))`, `calloc`, `free`, `owns_block`; question 292); an `owning` or marked member beside an array member
       (293); `ref_to_uninit` and `must_init` on a pointer to such a structure (294); contract places through a
       heap read (`owns(*p->q)`, `p->q->a[i]`; 306); an `ensures` atom with no `requires` partner; a written atom
       in a loop invariant on such a place.
     - **(A) Arrays of arrays** (Part E): `T m[N][M]` as a local, a global, a `static` local and a member;
       `m[i][j]`, `&m[i][j]`, a row `m[i]` used as a pointer; `sizeof(m)`, `sizeof(m[0])`, `sizeof(int[2][3])`,
       typedefs of such types; nested braced initializers; **array-of-rows parameters** in the three spellings
       `int a[][3]`, `int a[2][3]` and `int (*a)[3]` (question 320, item 13) with `owns_range(a, n)` over rows, `a[i][j]`, and `m`
       passed to one; nested loops; and the other depth-2 path, an array of structures whose member is an array
       (`v[i].a[j]`, `p[i].a[j]`).
     - **(Q) Quantifiers and `pure` applications** (Part F): under a binder, a member of a structure element
       (`p[i].x`, `v[i].t.x`); as the pointer argument of `cerastes_forall` and of a `pure` application, the decay
       of an array member (`p->a`, `s.a`) and a named global array in a contract.
     - **(I) Question 304's rejection, as measured** (item 16; Part B).
     - **The pages, and the tutorial section "Arrays in aggregates"** (question 298).
   - **Not here** (rejected, each pinned, the message naming where it is lifted):

   | Construct | Lifted by |
   |---|---|
   | an array or a range whose element structure has an `owning` or marked member; an array of owning pointers; `owning`, `ref_to_uninit` or `uninit` on an array of aggregates; a structure value stored into an element (`v[i] = f();`, `v[i] = (struct P){1, 2};`: question 333, item 12) | M4.8 |
   | at item 15's six positions, a structure whose array member has aggregate elements (`struct T { struct P v[2]; int n; }`, a matrix member): heap storage and `owns_block`, an `owning` or marked member beside that member, `ref_to_uninit` or `must_init` on a pointer to it, a place reached through a heap read, an `ensures` atom with no `requires` partner, a written atom in a loop invariant (question 348) | M4.5e2 (the owner may place it elsewhere) |
   | `malloc(n * sizeof(struct S))`, `calloc(n, sizeof(struct S))`, `realloc`; a range on part of an array (`owns_range(&v[1], n)`, `owns_range(g, n)`, `owns_range(p->v, n)`, a row `m[i]` as a range's base in a contract); an atom on one element (`owns(p[i])`, `owns(v[i].x)`); `&p[i]`, `&p[i].x` and `&m[i][j]` in a contract; a range whose base is not a pointer parameter; a range atom in a loop invariant; `p + i` | M4.5e2 |
   | under a binder: a second index (`p[i].a[j]`, `m[i][j]`), a row `m[r]` or `&v[1]` as the pointer argument; a memory-reading argument (`p->q->a`) | M4.5e2 |
   | a `forall` inside a `forall`; the positions 0061 item 1 leaves to M4.8 | M4.8 |
   | a leaf path of depth 4 or more (`int d[2][2][2][2]`, a matrix of structures whose member is a matrix); depth 3 (`int c[2][3][4]`, an array of structures whose member is a matrix) is admitted by S48 (m) and built in Part E | M8 |
   | a pointer to an array anywhere but a parameter (`int (*p)[3]` as a local, a member, a result; `&m[i]`, `&a`; a cast to one); `sizeof(char(*)[4])` | M8 |
   | a structure value that holds more than 7 array-element leaves, passed or returned by value (0063 item 13; a copy has no limit); an array as a value | M4.5f3 |
   | a read `pure` function over a structure or a global (0062 item 1) | M4.5f1 |
   | `const` arrays in any storage; string-initialized arrays | M4.6 |
   | array designators, VLAs, zero-length and flexible members, `[static N]`, `thread_local` and block-scope `extern` arrays, C++ array literals, `_Alignof` of an expression | M8 |

2. **Which arrays** (amends 0059 item 2, 0063 item 2). The element is a scalar that 0059 item 2 accepts, a
   verified structure, or an array of these; **no leaf path is deeper than 3** (S48 (m): `int c[2][3][4]` with a permission over three bound variables, nested loops and three bound rows verified in 10.2–10.6 s a program, under the plan's 30 s line, so depth 3 is admitted; depth 4 or more is rejected, `unsupported_type`, M8). The
   `const` rule is tested first (M4.6), then: an element structure with an `owning` or marked member at any
   depth is rejected (`unsupported_type`, M4.8). **Every bound is read as written** (0063 item 2's rule, each
   dimension on its own: a prologue constant for a global, a member and a file-scope typedef, a constant of its
   function otherwise; "the bound of `m`", "the second bound of `m`"). `owning`, `ref_to_uninit` and `uninit` on
   an array of aggregates are rejected (`unsupported_annotation`, M4.8; `uninit` stays accepted on a local array
   of scalars, 0059 item 11, so under `--strict-init` a local array of aggregates needs an initializer: listed).
3. **Layout** (0028 items 2 and 16, as decided in M4.5a).
   - Element `i` of the array block `B` is the slot `loc(B, i)`. For a structure element, its member `k` is the
     block `member(loc(B, i), k)`, exactly as 0043 item 3 and 0063 item 3 lay out a structure at any slot. For an
     array element (a row), the row is the block `member(loc(B, i), 0)` of the row's length.
   - **The length facts of a range of slots:** `forall k: Int :: { loc(B, k) } lo <= k && k < hi ==>` the
     conjunction, for the element at `loc(B, k)`, of what `struct_lens` states of one structure slot (`len(member(X,
     j)) == 1`, or the count for an array member; a nested structure's own facts; 0043 item 8's slot facts), or
     `len(member(loc(B, k), 0)) == M` for a row. They are stated by an allocation (`alloc_arr_<E>`), by
     `program_start` and the global facts, by `struct_lens` for an array member of aggregates, and by the first
     line of a range atom over structures or rows. They hold in 0028 item 2's model (`len` is free; a member
     block is a fresh term) and are hypotheses discharged by an allocation, by `program_start` or by a caller,
     never axioms.
   - **A length quantifier under another quantifier** (written on 2026-11-26 before its code; audit round C2,
     finding 1). The slot facts of a structure with a member that is an array of structures hold a quantifier
     of their own (`forall k: Int :: { loc(member(X, j), k) } 0 <= k && k < N ==>` the element's facts), and
     those slot facts may themselves stand under a quantifier whose binder occurs in `X`: the well-formedness
     row of a pointer leaf under a range's binder (`reads_range(pp, n)` over `struct T **`; a range over a
     structure with a `struct T *` member), an array of such pointers as a global, a `static` local, a member
     or a local a loop carries, and a type that reaches itself through a pointer (`struct E { struct TT *up;
     }; struct TT { struct E v[2]; };`). **Rule:** the binder of a length quantifier is a name that is not
     free in its slot expression `X`: `k` where `X` does not mention `k` (every program before this rule,
     whose Viper is unchanged), else the first of `k1`, `k2`, … that `X` does not mention; inside the
     quantifier's body and trigger the outer binder keeps its name and its meaning (it occurs only inside
     `X`), and the new binder occurs only as the index of `loc(member(X, j), ·)` and in its bounds. Part C2's
     first build wrote `k` at every level, so the inner quantifier captured the outer binder inside `X`;
     Silicon refused the unit ("duplicate identifier"), an exit 3 for every function of the file and never a
     proof, but nothing other than that check stood behind it. The global facts' own quantifier
     (`global_facts`) had the same fixed name and cannot nest today (an element structure with an array
     member is rejected). **As built** (Commit 3): one helper, `length_binder(X)`, decides the name by a
     structural walk of `X` for a free variable of that name; `struct_lens`, the global facts and
     `alloc_struct_<T>`'s member-array block share it, and every caller of `struct_lens` (the block and
     slot facts, the shape rows, the call and return rows) has the rule through it. No program emitted
     before the fix changed. The thirteen shapes the audit found verify (5 to 14 s), each reports an added
     false check, and a wrong claim per family fails. No C program can yet USE the nested fact (it would
     need an atom through `pp[0]`, a place reached through a heap read, which contracts reject), so at C
     level the row is tested as present and consistent, and its content by nine self-test cases
     (`e1e2c2c_binder_*`).
   - Every member block, at any depth, has its root block's liveness cell (`member_lv`): one liveness cell per
     root, as before.
4. **VIR** (amends 0063 item 4).
   - `Type::Array`'s element may be a `Struct` or an `Array`. `Ptr(Array)` is a type of a **parameter** only (item
     13), never of a local, a member, a result or a cell.
   - **Leaf paths.** `object_leaf_paths(type)` gives every leaf of an object type as its steps (member `k`; index,
     with its count) and its cell type, in declaration order, one path per leaf whatever its counts. For a type
     0063 accepted it is `struct_object_leaves`' list (a cell: member steps only; an array leaf: member steps,
     then one index step), and **every site that 0063 gave rows writes the same text for those types**. **The
     poison rule continues:** a walk of leaves that has not been given the general path reports an internal
     error on a path it does not handle (an index step that is not last; two index steps) — exit 3, never a cell
     read as something else — `check.cpp` refuses an array of aggregates in every position a part has not
     lifted, and the encoder rejects it there first with a diagnostic at the C line (invariants 3 and 7).
   - **The traps are closed before any gate is lifted.** The planning's survey found six places where lowering
     or the checker would treat an aggregate element wrongly without an error (a nested `ElemAddr` as a flat
     pointer; element keys that ignore an index; a range over structures with no field, so a callee's writes go
     unrecorded; walks that count an array by its element; nest walkers that skip elements; a structure element
     compared as one field), each masked only by a refusal upstream, and three positions only the encoder
     refuses. Part A makes each an internal error or gives it the general path, closes the three positions in
     `check.cpp`, and pins every guard with a self-test case (the first plan's section 8, Part A's table; the
     guards that still stand, and the part that lifts each, are in the continuation plan's sections 4 and 7).
     **Audit round 1 added to that table** (issues 2 and 9): `restrict_backed`'s `reaches` walker
     (`vir.cpp`), which decides whether another parameter's object can hold a cell of a `restrict`
     parameter's pointee and which read an array member of aggregates as one leaf of the structure's or
     row's type, so that such a member, or an array-of-rows parameter, never "reached" an `int *restrict p`
     (the unsound direction: 0043 item 10 then stops asking for the other parameter's atom) — it walks the
     leaf paths and takes a pointer to a row by its leaf cell, pinned by a rejection case, before Part C or
     Part E lifts a gate; the Sequencer's `struct_store_leaves` (`check.cpp`), item 11; and the walkers that
     Part A's guards on `site_fields` and `element_range` dominate today and that the part lifting each guard
     gives the general path: `entry_checks`, `result_splits`, `given_result_splits`, `ens_amount`,
     `exclusive_pair`, `wf_keep_facts`, `site_pointer_written`, `flag_follows`, `note_global_member_write`,
     `need_lb` (`lower.cpp`).
   - **Places.** `ElemAddr`'s base may be a pointer to a structure and the address of an array of aggregates.
     Its result for a structure element is that element's address (a `struct S *` value: `&v[i]`); for a row it
     is the row's address, accepted only as the base of another `ElemAddr` and as the row's decay.
     `MemberAddr` is accepted over an `ElemAddr`. `place_root` steps through an `ElemAddr`; places are spelled as
     written (`p[i].x`, `m[i][j]`, `v[i].a[j]`), never `(&p[i])->x`.
   - `Var::zero_filled` and `ArrayZero` hold for an array of aggregates (every leaf of every unwritten element);
     `ArrayCopy` is unchanged (an array leaf of scalars inside one structure, wherever that structure stands);
     the value view expands every index step (item 12).
   - `check.cpp` backstops each rule of items 2–16.
5. **Rows in code** (amends 0059 item 4, 0063 item 5).
   - **An element access** takes 0059 item 4's rows at each index step, in source order, then the cell's.
     `v[i].x`: the bound of `i` against `len(block_of(v))` (`out-of-bounds`: "reads `v[i].x`, whose index may be
     outside `v`"), then `permission` and, for a read, `uninitialized` on the leaf cell. `m[i][j]`: the first
     bound, then `j` against the row's length ("… whose second index may be outside `m[i]`"), then the cell's
     rows. A named array is a named base (no null or liveness row). `p[i].x` on a pointer takes e1a's rows for
     `p[i]` (`null-dereference`, `p`'s liveness, the bound) and then the cell's. **A member step adds no row,**
     but the slot it steps from must exist: `&v[i].x`, `v[i].x` and `m[i][j]` need `i` strictly inside, while
     `&v[i]` and `&m[i][j]` as values allow one past the end at their last index (0059 item 10).
   - **A row used as a pointer** (`m[i]` as an argument or an initializer) is the row's decay: the bound of `i`,
     strictly inside, then a pointer to the row's element 0, a valid `T *` by the length facts.
   - **The decay of a global array of aggregates as a constant** (P83 (a), extending 0063 item 5): in C the
     front end folds the decay into a `ck_address` constant in every rvalue position of a declaration's
     initializer, and in both modes when it is a whole call argument; never on either side of an assignment and
     never in an annotation. The constant has the decay's type (`int (*)[3]` for a matrix) and is read from its
     `expr`, the written `array_to_pointer`, as 0063 item 5 reads it.
   - **The bound local** (0063 item 5). A row over an array's slots in code is written over a Viper local bound
     to the array's first slot, with the leaf's receiver built from it: `loc(member(loc(block_of(t), k), j), 0)`
     for a cell leaf of an array of structures (a longer member chain for a nested structure), and over two
     bound variables for a depth-2 leaf, `loc(member(loc(block_of(t), k), j), l)`. Each permission is its own
     quantifier with its own receiver as the trigger (0028 item 2: one quantifier holding two members'
     permissions fails Silicon's injectivity check). A marked row still reads no heap (0059 item 7).
   - **Rejected like the compilers** (P83 (h)'s matrix): a constant index outside any dimension (EDG's #175,
     which fires on every such form measured; `&m[2][0]` and `&m[1][3]` draw no front-end error and take the
     path-precise rows above). **A truth test or a null comparison** (`if (e)`, `!e`, `e == NULL`, `e != NULL`,
     in code and in an annotation) **of a row or of an address formed inside an array of aggregates**,
     `array_address_null_test` (question 322; the rule is the matrix's cells, 376 shapes, both compilers, both
     modes: GCC warns `-Waddress` at `-Wall` on each, clang without a flag only on a whole array's name or
     decay, which the tag already covers, and on `&E[i].x`): a row `m[i]`; `&E[i]` whose element is a
     structure or a row, for `E` a named array, a member array or a pointer (`&v[i]`, `&t.v[i]`, `&p[i]` on a
     `struct P *p`, `&m[i][j]`); `&E[i].x`. GCC is silent on `&p[0]` alone (and on `p` and `&*p`); the rule
     rejects `&p[0]` with the other indexes (over-strict by that one cell, fail closed, listed). **The rule
     covers every address formed by an index** (question 322's second half, the owner, 2026-11-25: "Reject them
     too"; built in the continuation's Part R): `&E[i]` and `&E[i].x` for `E` a named array, an array member or
     a pointer, **whatever the element**, so also on an array of scalars — `if (&a[1])`, `&a[1] == NULL`, `&p[1]
     != NULL` on an `int *p`, `&s.a[i]`, `&p->a[i]` — which GCC flags too and the base accepted from M4.5e1a
     until Part R. It is one predicate, by the one tag `array_address_null_test`: `&p[0]` on a pointer is
     rejected with the other indexes for scalars and structures alike, over-strict by the one cell GCC is
     silent on (question 335, listed). Not unsoundness: a warning shape, rejected like the compilers; no
     existing test or doc example holds one (the first session's three passes; Part R's `rejections.py` and
     `vprdiff` runs check it again), and an existing file whose outcome changes goes to the owner before it is
     touched. **An address the front end folds to a constant is under the rule too** (audit round R,
     should-fix 2): a constant index into an array of static storage (`&ga[2]`, a `static` local array's
     element, `&gs.a[1]`) reaches the encoder as a `ck_address` constant whose kept `expr` is the written
     `&E[i]`, and the predicate reads through it. **As built** (the continuation's Commit 2; Commit 1's build
     did not read through the fold): in C every truth test whose OPERAND is the folded address is
     `array_address_null_test` — the condition of an `if`, `while`, `for` or `do`, an operand of `&&`, `||` or
     `?:`, a conversion to `bool` of the address itself (`if (&ga[2])` and `x && &ga[2]` verified before, on
     the base too; GCC flags them; they were sound, the address is never null and the proof said so). A test
     that the front end folds WHOLE to a constant (`!&ga[1]`, `&ga[1] != NULL`, `(bool)&ga[2]`) never shows
     the address as an operand and keeps its older rejection, `unsupported_expression` ("taking an address …
     in an expression the compiler folded to a constant"), and so does every such test in C++. Among the 254
     probe runs before and after and `rejections.py`'s record, no row that already rejected changed its tag
     or line. Audit round C1 (note 14) found one contract line, `!!&p[1]` in an `ensures`, that draws two rows
     (`array_address_null_test` and the older `unsupported_expression`, "the address of an element, `&p[e]`,
     in a contract") where the base drew the older one alone; whether the pair came with Part R or with these
     fixes was not determined (no lane held Commit 1's build then). Fail closed either way. **Over-strict beyond `&p[0]`, listed** (audit round R, note 4; question
     342): a test written through a macro (`assert(&a[1] != NULL)`, `ISNULL(&a[1])`, `TRUTH(&a[1])`), on which
     GCC and clang are silent in both modes, and `&a[1] == (int *)0` in C, which only g++ flags; the rule
     rejects them, the base accepted them for arrays of scalars, and no existing test or doc example holds
     one. Fail closed; the precise set (the front end does not record that a test came from a macro) is M8's
     warning sweep. **Not under this rule, accepted, and listed for M8** (note 5): the same tests on an
     address formed without an index (`&s.x == NULL`, `if (&ps->y)`, `if (&x)`), which both compilers flag.
6. **Local arrays of aggregates** (amends 0059 items 3 and 6, 0043 item 6, 0063 item 6).
   - **Allocation:** one trusted method per element type, `alloc_arr_<E>(n)`, in `alloc_arr_<t>`'s shape: the block's length and liveness facts, item 3's length facts, and
     per leaf path full permission to the value field and to the flag field (each its own quantifier) with every
     flag false. `zero_arr_<E>` is `zero_arr_<t>` per leaf (it takes and returns the two quantified permissions of
     every leaf path and ensures every leaf zero, `false` or `null` and every flag set; **it states no length
     facts**: the caller holds them from the allocation). Each (receiver, field) pair is given once. A nested
     structure member's slot facts (0043 item 4) stand under the length-facts quantifier, rooted at the slot
     `loc(block_of(r), k)`. **The name `<E>` identifies the element type exactly** (audit round 1, issue 7):
     for a structure it is the structure's numbered suffix, as in `alloc_struct_<n>_<tag>` (two scopes can
     hold one tag); for a row it spells the row's cell type and every bound (`alloc_arr_row_int_3`, and for a
     row of rows every bound in order), so that two row types never share one trusted method, which would
     state one `len` for both. **Every way out exhales exactly what the allocation gave** (audit round 1,
     issue 8: through `*q` or `q->x` a use after the scope is reported as `permission`, 0032 item 6, so a leaf
     path missed at a scope exit would be silent): the set of (receiver, field) quantifiers a scope exit
     checks and exhales equals the allocation's, pinned by the lint or a self-test case with Part B.
   - **Cost, a listed limit** (question 328; fail closed: a timeout is exit 3). Every leaf path of every
     local array of aggregates is a pair of quantified permission chunks of its field, and every access and
     every scope-exit row on that field is separated from each of them, so the cost grows with the number of
     such arrays in one function: measured alone on the box in C, a function holding 1, 2, 3, 4 and 5 arrays
     `struct P a[2]` of two `int` leaves, each with an initializer and one check, verifies in 7, 11, 17, 36
     and 65 s; a function with five mixed arrays (three of three elements, a nested structure, five
     multi-conjunct checks: 11 leaves, 22 chunks, 88 quantified scope-exit rows) took 735 s, past Silicon's
     300 s cap; five `int a[2]` arrays take 14 s, as on the base. No case split is emitted for such a
     function (audit round 2, note 12). Up to about three small arrays of structures in one function verify
     in well under a minute; more is a timeout risk. M4.8's cost items take it (with questions 66 and 189).
   - **Scope exit:** per leaf path the two marked rows over the bound local (`lifetime`: "the address of an
     element of `v` may still be in use when its lifetime ends"), then the `exhale`s.
   - **Initializers:** each element takes a nested braced list in its own rules (0043 item 6 for a structure,
     0059 for a row); unwritten elements and leaves are zero (`zero_arr_<E>` first, then the written leaves'
     stores); in C every element of the whole list is evaluated before the first store; elision of an element's
     braces is rejected like the compilers except `{}` and C's `{0}`, and **C++'s `{0}` follows item 16**; an
     element that reads, writes or takes the address of the array being initialized is rejected.
7. **Contracts** (amends 0060 items 1 and 3–5, 0063 item 7).
   - **A range atom over structures or rows.** `owns_range(p, n)` and `reads_range(p, n)` with `p` a pointer
     parameter to a verified structure (or to a row, item 13): 0060 item 4's first line with item 3's length
     facts over `[offset_of(p), offset_of(p) + n)`; then, **per leaf path of the element,** 0060 item 4's two
     permissions, flags and type range, each a quantifier over the absolute slot with the leaf's receiver as its
     trigger (`{ loc(member(loc(block_of(p), k), j), 0) }`; two bound variables for a depth-2 leaf); one liveness
     fraction. Partners, the guard outside each quantifier and the ghost amount are 0060 items 3 and 4's.
   - **Element reads** `p[e].x`, `p[e].t.x`, `p[e].a[e2]` and `g[e].x` in `requires`, `ensures`, a function's
     `decreases` and under `old`, the indexes reading no memory. A read is framed by a range atom when **its
     element slot** lies in the range (0060 item 5's disjunct on `&p[e]`; the member steps then name a leaf the
     atom holds), and by a structure atom whose place is that slot. **As built in Part C2** (question 344,
     settled with the advisor: the lowering's contract-read path for a structure element extended to these
     bases at small cost, and 0063 item 7 has the same reads for arrays of scalars): `g[e].x` of a global or
     `static` array of structures, framed by `owns(g)` or `reads(g)` (the whole-array range atom), and a
     member array's element reached by pure members from a parameter, a global or, in an `ensures`, the
     result (`p->v[e].x`, `gt.v[e].x`, `r->v[e].x`), framed by the structure atom's array part or by
     `owns(p->v)`. Still rejected: a second index (`p[e].a[j]`, Part E), a base reached through a load (Part
     D), an atom on one element (`owns(g[1].x)`), `&g[e]` in a contract (M4.5e2), and **an element of a
     structure VALUE** (`t.v[1].y` of a by-value parameter, `f().v[i]`; item 12). `owns(gt.v)` and
     `reads(gt.v)`, the member array of a GLOBAL structure alone, are accepted as `owns(p->v)` is (a write of
     `gt.n` under them is `permission`). **Listed limit, fail closed** (audit round C2, finding 4; question
     346, settled with the advisor; pinned): under `cerastes_old`, `g[e].x`, `p->v[e].x` and `gt.n` are
     accepted, and `cerastes_old(gt.v[e].x)`, an element of a member array of a global structure, is
     rejected, although the same read outside `old` is accepted.
   - `owns(g)` of a global array of aggregates is the range atom over the whole array (as 0063 built it for
     scalars); `owns(*p)` of a structure with an array member of aggregates takes that member's leaf paths with
     its other leaves; `owns(p->v)` is that member alone.
   - **Overlap is by slots** (0043 item 9 and 0063 item 7, the soundness core). A leaf path of a site is a set
     of slots; every rule that pairs two sites (the exclusion of `owns` and `reads`, the amount sums at calls and
     returns, `f__wf`'s distinctness and equal-ghost facts, framing disjuncts, loop places) pairs a leaf path
     with a cell, an array leaf or another leaf path of the same field by those slots. So `f(v, 3, &v[1].x)`
     with `owns_range(p, n) && owns(*q)` fails at the call (`permission`), and so does `f(v, &v[1])` with
     `owns_range(p, 3) && owns(*q)` over structures.
   - **The pairing rows** (S48 (c) decided between the two forms; audit round 1, issues 3 and 4, then
     revised how the chosen one is written). **Form P is built for every pairing: a pure row over slot terms,
     written per pair of leaf paths of one field, with the receiver terms in view.** For each leaf path `L` of
     one site and each cell, array leaf or leaf path `M` of the other site that has `L`'s cell field, where
     either site is an `owns`:
     - **A leaf path against a cell `q`** (a cell atom's place, or a leaf of a structure atom): the element
       index is recovered from `q` by one `parent_of(block_of(·))` per member step of `L`, innermost first —
       for a leaf one member step below the slot, `X = offset_of(parent_of(block_of(q)))`; for `t.a` of
       `struct S { int x; struct T { int a; int b; } t; }`, `X =
       offset_of(parent_of(block_of(parent_of(block_of(q)))))` — and the row is `!(g && lo <= X && X < hi && q
       == R_L(X))`, with `R_L(X)` the leaf's receiver written out over the range's bound local (`loc(member(loc(
       block_of(t), X), j), 0)`; `loc(member(loc(member(loc(block_of(t), X), 1), 0), 0), 0)` for `t.a`). A
       structure atom is paired leaf by leaf (each of its leaves is such a cell), whatever its structure type:
       a `struct T` atom on `&v[1].t` meets the range through `t.a` and `t.b`. A single `X =
       offset_of(parent_of(block_of(q)))` for every leaf (the draft's wording and the S48 (c) fixtures') lets
       an alias through a nested leaf pass the row and fail Silicon's own precondition check, an exit 3: fail
       closed, never a proof, but not the `permission` error this item promises.
     - **Two ranges** `(p, n)` and `(q, m)`, per pair of leaf paths of one field (the element structures may
       differ: a range over `struct T` at `&v[1].t` meets a range over `struct S` through `t.a`): the
       **slot row**, with each range's first leaf receiver compared against the other's receiver at the
       recovered index — for two ranges of one element type and their first leaf, `!((0 < n) && (0 < m) &&
       ((offset_of(p) <= offset_of(q) && offset_of(q) < offset_of(p) + n && loc(member(q, 0), 0) ==
       loc(member(loc(block_of(p), offset_of(q)), 0), 0)) || (offset_of(q) <= offset_of(p) && offset_of(p) <
       offset_of(q) + m && loc(member(p, 0), 0) == loc(member(loc(block_of(q), offset_of(p)), 0), 0))))` —
       (**as built**, one choice wider than this wording: two leaves of ONE structure whose paths differ in
       length, `x` against `t.a`, get the general two-way membership row rather than no row, since this
       item argues that two leaf paths cannot meet only for paths of equal length; a pair of ranges over
       different element structures gets that general row too, `!((0 < n && first_p meets q's range) || (0
       < m && first_q meets p's range))`)
       **never the block row** `!(block_of(q) == block_of(p) && the intervals meet)` that 0060 item 8 writes
       for scalar ranges and the S48 (c) fixtures wrote: Silicon does not decide it where a function forwards
       two of its own ranges over structures (no leaf receiver of a first slot is in view; a false
       `permission` on a correct program), while the slot row verifies there, is not vacuous, passes S48 (c)'s
       positive and fails its twin once (the audit's `fwd2_*` and `*_slotrow` probes).
     - **Form S** (the sum form of 0060 items 8 and 9, one marked `acc` row holding both sites' permissions)
       is not built for a pairing in which either site is an `owns`; two `reads` sites that may meet keep the
       sum form of 0060 items 8 and 9 and 0063 item 7, which checks their amounts and separates nothing.
     - Never a bare block comparison, never `perm` under a quantifier. A row Silicon cannot decide is a false
       `permission` or an exit 3, never a proof: behind every pairing row **of a call, a return and a loop**
       stands the call's own precondition, the postcondition or the invariant (the audit's nine `fp_*`
       probes: with no row at all, each alias fails Silicon's precondition check; audit round C1 repeated it
       on Part C1's build). **That is not true of the sequencing rows, which the same membership tests
       write** (item 11; audit round C1, finding 1): nothing stands behind those.
     - **A site that is a nested slot** may be paired by its slot instead of leaf by leaf (the two are
       equivalent, and lowering writes one of them per site kind): a structure atom on a nested structure of
       the element (`struct T` on `&v[k].t`) by one row on the atom's slot, `!(g && lo <= X && X < hi && q ==
       loc(member(loc(block_of(t), X), 1), 0))` with `X = offset_of(parent_of(block_of(q)))`; a range over
       the nested structure type at such a slot by the block-and-interval row over the member block,
       `!(block_of(q) == member(loc(block_of(t), X), 1) && offset_of(q) <= 0 && 0 < offset_of(q) + m)` (the
       member block has length 1, so the interval test on its slot 0 is what the receiver comparison comes
       to).
     - **The spike's evidence for these forms** (audit round 1, issues 3, 4 and 6; 25 fixtures added after the
       audit, `s48_c2_*`, `s48_z_*`, `s48_v_*`, each passing alone on the box): the slot row verifies in a
       function that forwards two of its own ranges and in its caller with disjoint halves of one array, and
       the overlapping twin fails once at the row; the three nested pairings over `struct S { int x; struct T
       { int a; int b; } t; }` (a cell `&v[k].t.a`, a `struct T` atom on `&v[k].t`, a range over `struct T`
       at `&v[k].t`) verify with the other site in element 3 of the same array, outside the range, and each
       twin with the other site in element 1 fails exactly once at its row, not at Silicon's precondition.
       **Form P as written here passes every pairing; Form S is not needed for any.** Not covered by a
       fixture: a `reads` site against an `owns` in the nested pairings, a cell held by a fraction and a
       forwarded structure atom of the element's own type (the audit's probes `fwd_row`, `fwd3_row` decide
       both). The nested twins are the slow ones (48–85 s at load 3–11, the positive 43.5 s): Part C's tests
       of them are timed under the test budget's rule 3.
     - **Listed limit, fail closed: an exit 3** (audit round 1, note 13, called it a false alarm; question 343,
       answered by the owner on 2026-11-26: it stays listed until M4.5e2, which gives pointers into arrays
       their bounds rows; nothing is built for it in M4.5e1e2). A structure pointer formed by indexing a pointer base outside the slots that the
       function's atoms show to hold structures cannot be shown `WF_S` (the length facts of a range cover its
       slots only; `owns(*p)` covers `p`'s own slot): `&p[n]` one past a range, `&p[i]` with `i` only known to
       be in `[0, n]`, and `&p[1]` under `owns(*p)`. Forming it is accepted. **Passing it to a function with no
       atom on that parameter** (directly or through a local copy) **and returning it end in an internal error**
       (exit 3: "Silicon's own check of the precondition of `g`"; "the pointer a function returns is
       well-formed"), not in a diagnostic: nothing is reported verified, but the explicit row that 0028 item
       11 asks for is missing. Passing it to a callee that owns or reads the pointee is an ordinary
       `permission` error, storing it into a cell a false `lifetime`, and inside the range (`&p[0]` with `1 <=
       n`) everything verifies. To a callee with only `live(q)` on that parameter it is two false
       `permission` errors ("needs `&p[n]` to point to a live object"; "returns without owning `p[0..n)`"),
       and `f(&p[n], 0)` to a range callee one (audit round C1, note 9, which found no use of such a pointer
       that verifies something false: a false `check` after forming it is reported, and `&p[n] == &p[0]`
       is not provable). **It predates Part C1:** `g(&p[1])` under `owns(*p)` gives the same exit 3 on
       the continuation's Commit 1 build, since the first session's Part B accepted `&p[i]` on a pointer to a
       structure. The fix is one explicit `WF_S` row (`permission`) for a structure-pointer argument and
       result; written for every such value it changes the Viper of existing programs, which is the owner's
       to approve, and written only for functions that hold a range it would leave the older form as it is.
       So nothing is built here (the owner's answer to question 343: M4.5e2); the harness cannot pin an exit 3, and the planned pin
       `range_struct_past_end_false_alarm.c` did not land. Probes: `build/m4.5e1e2b/q343/`.
     - **Listed, from Part C1's lowering:** a `must_init` filler over a range of structures that fills WITH A
       LOOP fails its flag rows at the return (the loop's automatic conjuncts do not say which flags it set;
       a quantified invariant over a member is item 14's, Part F): a false alarm, pinned by a self-test case;
       a `ref_to_uninit` loop verifies. In a contract and in a loop annotation a read through a pointer
       member of an element (`*p[0].q`, `p[0].q[0]`, `p[0].q->x`) is refused (M4.5e2, with the other
       memory-reading places). In a loop annotation `p[e].x` is accepted for the parameter's own copy in a
       function whose `requires` holds a range on it; after `p = q;` the read is framed by `q`'s place or
       fails `permission`, never by `p`'s entry range (0060 item 11). A caller that holds one element fewer
       gets two `permission` errors at the call, the shape row and the first leaf's marked row, as 0060 item
       8 has it for a forwarder (a local array one element short: one row, which names no member).
     - **Fixed with the continuation's Commit 3** (audit round C1, findings 2 to 4; each failed closed at
       Commit 2): (finding 2) `apply_frame` skips every site whose pointee is a structure (a range over
       structures, a structure atom, `owns(g)` of a global array of structures), so a `pure` application
       beside one is framed by scalar sites only: an unframed application is one `permission` error with no
       exit 3 after it (the older structure-atom form gave that exit 3 on the base), and no existing
       program's Viper changed. (finding 3) A read under a binder is no longer framed by a range over
       structures or by the array part of a structure atom (`contract_read`, and the loop-invariant path):
       framed by a scalar range it verifies, framed only by the structure site it is `permission`, and the
       quantifier's body holds no computed `loc` term over the binder. Members under the binder are Part F.
       (finding 4) A range over a structure with a pointer-to-structure member (`struct N { struct N *next;
       int v; }`) and a scalar range over `struct P **` have self-test cases and a test pair
       (`tests/memory/range_struct_pointer_member.c`), and the lint admits exactly the well-formedness row
       the lowering writes for such a leaf. Pinned beside them: `owns_block` beside a range over structures
       is rejected (`negative/range_struct_reject_owns_block.c`); a `pure` application and a `forall` beside
       each storage (`range_struct_pure_beside.c`, `range_struct_forall_beside.c`, each with an unframed and
       a wrong-value twin).
     - **Listed false alarm** (question 345, settled with the advisor; M4.8, with 0063 item 7's): `f__wf`
       does not tie the entry and exit ghosts of a member of an element of a MEMBER array of structures
       reached through a pointer. With `struct T { struct P v[2]; int n; }`, `requires reads(*p) &&
       p->v[0].x != 0` and `ensures reads(*p) && (r == 0 || r == 10 / p->v[0].x)` report `division-by-zero`
       at the `ensures`, under `reads(p->v)` too; a contract that states the guard in the clause that
       divides verifies. A scalar member (`p->n`), a nested structure's member (`p->a.x`), a parameter's
       range (`p[e].x`) and a global array (`g[e].x`) are tied. Only weaker, since ghosts are keyed with the
       state: 0063 item 7 lists the same for a member array of scalars (its N1). Pinned by
       `tests/memory/negative/struct_member_array_struct_wf_tie_false_alarm.c`; the ghosts themselves (one
       per leaf path and slot) by `struct_member_array_struct_wf.c` and its `_wf_division` twin, whose
       cross-state row waits for the tie.
     - **Cost, listed** (audit round C1, finding 7; fail closed: a timeout is exit 3; M4.8's cost items): a
       function that holds two `reads_range` atoms over ONE structure type and has a loop is slow. Under a
       loaded box, a loop that only reads took 81 s, and its twin whose loop writes through one of the
       ranges, which owes a `permission` error, got no verdict in 560 s. The lowering's own case of that
       shape took 78 to 112 s before `range_row_amount` used the slot-index test for a leaf against itself.
     - **Not of this record, listed** (audit round C1, note 12): a contract that owns one object twice and
       says so (`owns_range(p, n) && owns_range(q, n) && p == q`, also the cell and scalar-range forms, which
       verify on the base) verifies vacuously; no caller can meet it, and no entry row reports it for
       ranges (0060 item 7 asserts no separation for ranges).
   - **Amounts** (0060 item 6): a `reads_range`'s amount is the minimum over each leaf path's first slot, the
     liveness cell and the caller's own read amounts of those fields (question 238's listed limit applies).
   - **Rejected** (`unsupported_annotation`): item 1's M4.5e2 rows; two atoms on one parameter that overlap
     (`owns_range(p, n) && owns(*p)`). (`restrict` beside any atom is rejected by item 19.)
8. **Calls and returns** (amends 0060 items 8 and 9, 0063 item 8). A range atom's rows are per leaf path: the
   shape row once (with the length facts), then e1b's marked rows (value, then flag) and its flag and range rows
   for each leaf, over the bound argument; the messages in source terms ("calls `f`, which needs `p[0..n)`: the
   member `x` of every element"). An array of aggregates passed to a range parameter is bound to a local first.
9. **`program_start`, initial values, `static` locals** (amends 0063 item 9, 0044 item 8). A global array of
   aggregates has, per leaf path, 0063 item 9's row group (the two quantified permissions, the flags, the
   values, once the liveness cell) and item 3's length facts in the global facts. **The values, known:** one
   conjunct per leaf of each element the list writes (the written constant, or zero for a leaf the element's
   list leaves out), and per leaf path one quantifier for the elements the list does not reach. **Unknown** (by
   0041 item 3's linkage rule and 0063 item 9's two escape directions, per leaf): the type range as one
   quantifier per leaf. Every element constant is read through `constant_value` (P38; P83 (f): the loss of `expr`
   is keyed to the outermost list, so a list of 1,002 elements or more loses it for every leaf below it, whatever
   the nesting, and such a leaf under an implicit conversion is rejected by `constant_value`'s own rule). A `static` local array of
   aggregates is such a global with its implicit atom. **As built in Part C2** for arrays of structures:
   `Global::elems` holds one entry per element up to the last element the list writes, each with one
   `GlobalLeaf` per leaf of the element (an omitted leaf zero or null), and `rest` (zero or unknown); a
   structure global's member that is an array of structures is that leaf's `elems` and `rest`; both escape
   directions set every entry and the rest to unknown; `struct P *pg = g;` is `loc(GB_g(), 0)`. An array of
   structures declared through a typedef or as a type operand stays rejected (Part E, with the computed
   types of item 13). **Listed** (audit round C2, notes 5 and 7; each fail closed): an initializer list of
   about a thousand two-leaf elements with no converted leaf (1,003 measured; 300 verify in 29 s) is
   accepted by the encoder and gets no verdict, because Silicon's own stack overflows on `program_start`'s
   2,017 `ensures` clauses: exit 3, "Silicon did not produce a verdict" (with a converted leaf the list is
   rejected at the declaration by P38's limit, as designed). Three messages are not yet in source terms: a
   place in a global structure's member array prints as `(&gt.v[0])[1].x`; "passes `gt.v[0].y` and a range
   to `gq`"; a write under `reads` says "the object it points to may have ended".
10. **Loops** (amends 0059 item 8, 0060 item 11, 0063 item 10). An array of aggregates the function holds is,
    **per leaf path,** an array place (a local, a global in `main`) or a range place (a parameter's range, a
    held global elsewhere, a held structure place's member), with 0059 item 8's entry rows, conjuncts and
    back-edge rows over the bound local; one liveness conjunct per root (S47's F5). **Reach** follows the rules
    in force: a store by name to a leaf of a global array the function holds reaches that leaf path only (0044
    item 8); a store to a member of an element of a local array, any store through a pointer of the leaf's
    field, and a call whose `requires` owns a cell of that field reach every place of the field (0043 item 11).
    **Listed false alarm:** a loop that writes `v[i].x` cannot keep `v[k].y` by the automatic conjunct when
    both are of one field; a written quantified invariant keeps it (Part F), and the precise rule is M4.8's.
    **As built in Part C2:** a member that is an array of structures, once its first slot is bound to a
    local, is a range over structures with a literal count, so its leaves are the places Part C1 gave a
    parameter's range; a loop that writes `t.v[i].x` loses `v[k].y` and the structure's own `int` cells
    alike (the same listed false alarm). A loop invariant or measure may read `g[e].x`, `t.v[e].x` and
    `p->v[e].x`, framed only by places the loop holds: in a function that holds no atom on `g`, and after
    `p = q;`, the read is `permission`.
11. **Sequencing** (0039 item 3, 0059 item 9, 0063 item 11). Object keys gain the index steps: `v[0].x` and
    `v[1].x`, and `v[i].x` and `v[i].y`, are different objects; two unsequenced accesses to the same leaf path
    of one array, one a write, with indexes that are not all constants, get the path-precise `unsequenced` row;
    the same leaf of a NAMED array named twice with a write is `unsequenced_modification`; through a pointer
    (`p[i].x = p[i].x++`) there is no static key, as for scalar `p[i]` since 0059, and the access takes the
    path-precise `unsequenced` row.
    **An element as a whole** (audit round 1, note 15, and issue 9): a whole-element read or write (`t =
    v[i]`, `f(v[i])`, `v[i] = t`) reads or writes every leaf path of that element, so it conflicts with a write
    to a leaf of an element of the same array unless both indexes are constants that differ (`f(v[i], v[j].x++)`
    takes the path-precise row); and a structure store's leaves (`struct_store_leaves`) carry the index steps
    of an array member of aggregates, so a write to `t.v[0].x` inside a value stored to `t` is the static
    conflict it is for a scalar member.
    **A call that holds a range over structures** (Part C1) is a memory call on every leaf path of the
    range: the path-precise `unsequenced` rows against another access or another call in the same full
    expression are written per leaf with item 7's membership tests (a cell against a leaf's slots; two
    ranges by the slot row or the two-way membership row). **Nothing stands behind these rows** (audit round
    C1, finding 1): Viper has no evaluation order to check, so with a row left out `wr(v, 2) + wr(&v[1], 2)`
    verifies with one order's result (the auditor made the rows `assert true` in the kept Viper and Silicon
    verified it). As built the rows are right (23 probes in both modes: every overlap is `unsequenced`, the
    disjoint control passes), and they are pinned at C level by `tests/memory/range_struct_seq.c` and
    `negative/range_struct_seq_unsequenced.c` and by six self-test cases (`e1e2c1_seq_*`); no lint rule
    inspects them.
12. **Structure values and elements** (amends 0043 item 7, 0044 items 2–5, 0063 item 13). An element `v[i]` or
    `p[i]` is a structure place: a copy's source and destination, a by-value argument, an initializer's source,
    with 0043 item 7's and 0063 item 13's rows, after the index's own rows. **The source's rows come before any guard** (0063 item 13: the source's permission and every source flag,
    `uninitialized`, then the destination's permission; audit round 1, note 14: the S48 (g) fixture skipped the
    source's flag rows in the `i == j` arm, which lowering does not). Cell leaves are copied by leaf
    stores, which need no guard (`v[i] = v[i]` rewrites each value). **For an element structure's array leaves
    (Part E), `copy_arr_<t>`'s guard `d != s` is written where the two places can be one object:** a place
    reached through a pointer (0063 item 13) and two elements of one array (`v[i] = v[j]`); **never between two
    named objects or between an element and another named object**, where Silicon would leave the guard's
    branch open and skip the copy (S47's F2). A structure whose
    member is an array of structures is a value by its value view: every leaf of every element is one Viper
    value, and `kMaxByValueArrayElements` (7) counts those leaves; a whole-structure copy of it is leaf stores
    and `copy_arr_<t>` calls per element — **rejected beyond 7 such leaves until M4.5f3** (no trusted
    `copy_arr_<E>` is added here). An array is never a value. **As built in Part C2:** the value view gives
    each leaf of each element as one leaf (`path` through the array member to the leaf, `elem` the index,
    in element order), a by-value parameter or result one Viper value per such leaf
    (`<p>_m0_m0_e1`-style names), and a whole-structure copy reads every leaf of every element into
    temporaries before the first store; 7 leaves are accepted and 8 rejected at the parameter, the result
    and the copy. **A contract cannot read an element of such a structure VALUE** (`t.v[1].y` of a by-value
    parameter, `f().v[i]`): rejected (`unsupported_annotation`; M4.5f3, with array values), where 0063 item
    13 reads `s.a[i]` of a by-value parameter for an array of scalars. **Cost, listed** (fail closed: a
    timeout is exit 3; M4.8's cost items, with item 6's): each structure local that holds a member array of
    structures adds about 45 s to its function's proof on the loaded box (one such local 15 to 23 s, two
    59 to 61 s, six in one `main` no verdict in 300 s); a call of a function with `reads(gt)` of a global
    structure with such a member, followed by one `check` of five of its leaves, got no verdict in 560 s
    (with one leaf: 9 s), and so did one call with three or four atoms over such a structure, a range and a
    global (audit round C2, note 6). **Listed limit, fail closed** (audit round C2, finding 4; question
    346; pinned): an element of a member array of a GLOBAL structure passed by value (`fp(gt.v[1])`) is
    rejected IN C, with a message that speaks of "a pointer read from memory" although none is read;
    `struct P e = gt.v[1];`, `fp(t.v[1])` of a local and `fp(g[1])` of a global array are accepted. **In
    C++ the same call is accepted** (found by the joined proof, probed by audit round C2b): the C front end
    folds the element's address into an address constant, which `object_value` refuses, and the C++ front
    end keeps the subscript. The accepted path reads each leaf of the element under the caller's
    permission, with the bounds, permission and flag rows before the call (the right value verifies, a
    wrong element or leaf fails, a symbolic index is checked, a function without `gt`'s atom is
    `permission`, a call beside it that may change the element is `unsequenced`: twenty probes), and has a
    C++ test pair. **Cost that this session's own change added, listed** (audit round C2b, finding 1;
    question 347, settled with the advisor; fail closed): giving the length facts of a global their own
    clauses, which the lint's shape asks for, moved the quantifier of a structure global's member array out
    of the middle of the facts' conjunction, and the prover then no longer finishes one measured program: a
    function other than `main` with `requires reads(gt) && gt.v[1].x == 3` that calls a `reads(gt)`
    function and checks that leaf took 17 s on the build before the split and gets no verdict in 200 s
    after it (the same call from `main` took 9 s before and 13 s after). The two-call chain `main -> mid -> rd` over
    such a global got no verdict in 200 s before the split either, so the family was fragile already; the
    split is kept because Commit 3's `src/` is the snapshot the second read took, and no committed build
    accepted these programs before. The alternative for a later session is the fused form with a lint rule
    for it. **A structure value stored into an element**
    (`v[i] = f();`, `v[i] = (struct P){1, 2};`) is not a copy between objects and stays rejected, with a
    message of its own, until M4.8 (audit round 2, issue 5; question 333, a default of the continuation's
    plan: 0044 item 5's rules for a structure `Store` are not extended to an element here); `struct P t =
    f(); v[i] = t;` is the way to write it. **S48 (g):** the guard between two elements of one
    array is decided by Silicon in both directions and is needed (without it the copy's `requires` fails once);
    the copy `v[i] = v[j]` of `struct W { int a[4]; int n; }` elements verified in 34.5 s and 35.2 s on the
    box, over the spike's 30 s line: it is built (question 323), and Part E's tests of it are timed under the
    test budget's rule 3.
    **Decided for Part E, not yet built** (question 353, settled with the advisor on 2026-11-26 from the
    encoder draft that was written ahead of Part E; the draft is unproved and unaudited). A whole-structure
    copy, a by-value parameter or result and a compound literal of a structure with a MATRIX member, or
    with a member array of structures whose elements have an array leaf, is written cell by cell: every
    cell is read into a temporary before the first store (so a self-copy needs no guard), no `ArrayCopy` of
    a nested leaf is written, and **the limit of 7 counts cells** of such members (`struct T { struct W
    v[2]; }` with `W { int a[4]; int n; }` is 10 cells and rejected as a whole copy; M4.5f3), pinned at 7
    and 8 for the copy, the parameter, the result and the literal; whether a scalar array member beside
    the matrix counts is stated by the session that builds it, from the code. The element copy `v[i] =
    v[j]` of an element with a scalar array leaf keeps this item's guarded `ArrayCopy`. **The encoder's
    order of cells and the lowering's value view must be ONE enumeration** (depth first, members in
    declaration order, every array's elements in index order, a matrix row major): at the base
    `vir.cpp`'s value view leaves a matrix member unexpanded and gives an element structure's array leaf
    two cells with one (path, element) key, and the draft's encoder walks its own list; until there is one
    definition, `check.cpp` refuses a structure value of depth 2 or more. Tested by a structure passed by
    value with a distinct value per cell, each read back, and a swapped twin.
13. **Arrays of arrays** (amends 0060 item 15, 0063 item 14). A row is an array object in its own block; a
    matrix's leaf path has two index steps, and so has an array member of an element structure (`v[i].a[j]`).
    - **Array-of-rows parameters.** `T a[][M]`, `T a[N][M]` and `T (*a)[M]` declare a parameter of type pointer
      to a row. **`T (*a)[M]` is accepted as `T a[][M]` and `T a[N][M]` are** (question 320, the owner,
      2026-11-25: "Accept it in Part E"): the three spellings share one adjusted type (P83 (d)), and the pin
      `tests/memory/negative/array_reject_pointer_to_array.c`, which rejected exactly that parameter (0059
      item 1), is retired by Part E for a positive of the same program with an error twin, with a new pin for
      a pointer to an array as a local. The first session's interim rule (a parameter accepted only when its
      written type is an array declarator) is withdrawn. **A spelling through a typedef** (`typedef int
      row[3];` with `row *a`, `row a[]`, `row a[2]`) **is accepted as `int (*a)[3]` is** (P84 (b), its first
      branch): the three give one adjusted pointer over one `tk_typeref` (`row`) whose type is the row's
      `tk_array` entry, with the bound field for field that of the spelling without a typedef (a literal, a
      product, an enumerator; both modes; an alias declared with `using` alike). The reader peels that one
      typeref and reads the bound as written, under 0063 item 14's typedef rule (a typedef declared in a
      verified file, its bound checked where it is written; one from a system header stays rejected). A
      reader that did not peel would reject, never misread. A qualified row (`const row *a`) reaches the
      encoder as a fresh array of `const int` with no typeref above it, the shape of `const int (*)[N]`,
      and follows the rule that spelling has. **That rule is rejection** (question 352, settled with the
      advisor on 2026-11-26): `const int a[][3]`, `const int (*a)[3]` and `const row *a` are rejected as
      `const` arrays (item 2 tests the `const` rule first; M4.6), stricter than one dimension, where `const
      int *p` is accepted; a pin row for each spelling comes with Part E. A pointer to an array as a local, a member, a result or under `&` stays M8's (item 1).
      Such a parameter is accepted with `owns_range(a, n)` or `reads_range(a, n)` over `n` rows (item 7), `a[i][j]`
      in code and in contracts, and a row `a[i]` used as a pointer. The outer bound of `T a[N][M]` is checked as
      written and is no promise (0060 item 15); the inner bound is part of the type. A matrix passed to such a
      parameter is its decay, bound to a local. `sizeof(a)` of such a parameter is rejected
      (`sizeof_array_argument`). P83 (d): the three spellings share one adjusted type, pointer to the row's
      array type with the row's bound and its `expr`; the written type is the parameter type's `declared_type`
      (so the spelling can be told), `[]` has no bound constant, and a call with a matrix of another inner bound
      is the front end's error in both modes (in C its #167, which the driver makes an error).
    - **Computed types** (0063 item 14, extended): a typedef, `typeof`, `decltype`, and the type operand of
      `sizeof` and `_Alignof` may name an array of aggregates when every bound in it is read as written.
14. **Quantifiers and applications** (amends 0061 items 1, 3 and 6, 0062 item 4).
    - **Under the binder** a read may step through members of the indexed element: `p[i ± c].x`, `v[i].t.x`.
      Its read class is the pair (argument, member steps), its trigger that leaf's receiver (`{
      loc(member(loc(block_of(X), q), j), 0) }`), and 0061 items 6–10's framing, bound and flag rows are written
      per class against that leaf's range or array place.
    - **The pointer argument** of `cerastes_forall` and of a `pure` application may be the decay of an array
      member of scalars (`p->a` and `s.a`; in a contract, of a structure the contract owns or reads through a
      pointer parameter) and a named global array in a contract (P83 (g): each is `array_to_pointer` over a
      field or a variable, with no load; a global is not folded in an annotation; a `pure` application's
      argument is under a compiler-generated qualification cast, which is peeled as 0060 item 3 peels the
      top-level generated conversion). It reads no memory (an address is formed, not
      loaded); `p->q->a` stays rejected (M4.5e2). The class's block is the member's or the global's block term
      in a contract line, and its bound local in code.
15. **The leftovers of a structure with an array member** (amends 0063 items 1 and 7). Each site below takes
    the object view (a cell leaf as before, an array leaf with 0063's quantified lines), and the audit round of
    Part D gives a verdict per site of the continuation plan's section 7 table.
    - **Heap storage** (question 292; amends 0047 items 2, 4, 6 and 8). `malloc(sizeof(struct S))`,
      `calloc(1, sizeof(struct S))` and `free` take a structure with array members: the typed view's `ensures`
      and `owns_block` state the member lengths and hold each array leaf by its quantified permissions and
      flags (`calloc`: every element zero and set; `malloc`: the flags unknown, as 0047 item 8); `free`'s rows
      are per leaf (`invalid-free`); the leak check reads the block's token as before. **`calloc`'s zero
      values, as built** (question 351, settled with the advisor; fail closed): the view's `ensures` holds
      one value clause per ELEMENT of an array leaf (`r->a[0] == 0`, …, `r->a[N-1] == 0`, each index a
      literal, each once), since VIR has no quantifier over a member array's elements outside a `pure`
      function's `forall`; each clause is a contract read framed by the view's own block site, and the flags
      are set by the block site's quantified line. **The cap** (question 354, settled with the advisor
      the same day, after the first run: `calloc` of a structure with `int a[64]` got no verdict, Silicon's
      own stack overflowing in the view's `__wf` method, a JVM stack depth and not a time; fail closed):
      `calloc` of a structure that has an array leaf is rejected at the call (`unsupported_expression`) when
      its view would write more than **24 value clauses**, counting one per cell leaf and one per element
      of every array leaf, by-value members included (`int a[23]; int n;` is 24 and accepted, `int a[24];
      int n;` is 25 and rejected). Measured through the driver on the box, both modes agreeing: an array
      alone verifies up to 50 elements (78 s) and gets no verdict from 52; `a[24]` with 24 cells verifies
      (57 s), `a[32]` twice does not; the cap is about half the largest passing count, since the limit is
      a stack depth that another platform may reach earlier. A structure with no array leaf has no cap
      (unchanged: 100 cells verify, slowly), and `malloc` and `free` have none (no value clause). The
      diagnostic says to allocate with `malloc` and write the elements, and says what that costs today: a
      read of an element that only a LOOP wrote is reported as `uninitialized` (the listed false alarm of
      the next clause; measured), so a structure over the cap can be filled by a loop but its elements read
      back only after straight-line writes or, from Part F, under a written quantified invariant. **For
      the owner** (it sits inside question 292's scope): the alternatives are a different framing of the
      per-element reads in `f__wf`, or the quantified zero line per array leaf (new trusted text;
      M4.5e2's, with `calloc(n, sizeof(T))`).
    - **A heap block's array part in a loop** (amends 0047 item 10; written on 2026-11-26 with the code,
      before Part D's final joined proof). The array leaf of a block the loop holds through a pointer is a
      place of the loop beside the block's cell places: bound to a local before the loop, with ghost
      amounts taken from its first slot (value and flag), marked entry rows, guarded permission conjuncts,
      the flags kept, the type range, the values kept where the loop cannot write the field or the amount
      is below `write`, and marked back-edge rows written before the cells' rows (so a loop that gives the
      block away is reported at the array's row). A structure with no cell leaf gets a value-less place
      for its liveness and token. **The place exists only in a program where a token-carrying block of
      such a structure can exist** (the gate `array_block_possible`): some function of the unit has an
      `owns_block` site (a typed view's `ensures` is one), the implicit atom of an `owning` parameter, or
      an `owning` local, global or member, whose pointee is the structure or holds it by value at any
      depth. A block with a token enters a function only through a call of a function whose `ensures` has a
      block site, through the function's own contract, or through an owner it holds, and before Part D no
      such site was accepted for a structure with an array member; so every program accepted before keeps
      its Viper byte for byte (the first build wrote the place for every pointer to such a structure and
      changed three existing self-test cases; the gate restored them). Where the gate is false a missing
      place is a false `permission`, never a proof. **Listed false alarms:** after a loop that fills the
      elements of a `malloc`ed structure, the array part's flags are the loop entry's, so a read of an
      element after the loop is `uninitialized` (as for a local array; a written quantified invariant is
      Part F's); a loop over the elements of a structure that a callee gave by a partnerless `owns(*r)` is
      `permission` (no place for a given structure in a function that is not a heap function; the same
      for a structure without an array member, older than this record).
    - **An `owning` or marked member beside an array member** (293; amends 0055 items 2, 3 and 5, 0058 item 4).
      The structure is accepted. An array leaf of scalars is never an owner (arrays of owning pointers are
      M4.8's), so every owner site walks the cell leaves and passes over an array leaf; the closure of an atom
      holds the array leaf's lines beside its owners' blocks.
    - **Markers on a pointer to such a structure** (294; amends 0042, 0058 item 4). `ref_to_uninit`: the
      `requires` site claims no flag of an array leaf either, and the kept-flag row is one quantifier per array
      leaf (`old(flag) ==> flag`, where 0058 item 4 keeps it for a cell). `must_init`: every element's flag is
      set at the return; a filler that leaves an element unwritten is `postcondition` at its closing brace,
      the category 0042 item 5 gives `must_init`'s flag row ("returns without initializing …"), and a
      caller's read of an element nobody wrote is `uninitialized` (question 349, settled with the advisor:
      this item's first wording said `uninitialized` for the filler).
    - **Contract places through a heap read** (306; amends 0045 item 1). A load-rooted place whose structure has
      an array member takes 0045's rows with the array leaf's lines keyed by the loaded pointer; `p->q->a[e]`
      is an element read through that place.
    - **An `ensures` atom with no `requires` partner** (amends 0052 item 2) on a structure with an array member:
      covered or given per leaf, an array leaf by its quantified lines and, where 0052 gives an output amount,
      one amount for the site. **0052 item 2a stands unchanged** (question 350): in a function without a body
      such an atom is accepted only on the result or a place reached through it, never on a formal (`void
      give(struct S *p) ensures(owns(p->a));` stays rejected: a second write chunk on cells the caller
      already holds would make its state impossible and everything after the call would verify). The
      continuation plan's Flips A (D) listed that row of `struct_array_reject_annotations.c` (L37) as
      retired; it is not, and neither is a row that an older rule still rejects once Part D's gate is open
      (L39, a marked parameter with no atom: 0042 item 4).
    - **A written atom in a loop invariant** on such a structure place (amends 0045 item 12, 0040 item 5): the
      array leaf's conjuncts are a range place's. (This item's first wording gave it the first slot's
      amount, 0060 item 6, as the fixture S48 (k) verified it, with `Perm` locals taken from the first slot
      before the loop; the amount as built is `write`, below.) **As built** (the continuation's second session, Part D's last step; written here before the
      final joined proof): `owns(*p)` and `owns(p->a)` written in a loop invariant, `p` a pointer variable
      whose address is not taken and, for the member form, one member step; one entry per array leaf beside
      the cells' entries (never a cell entry holding an array type: that walk crashed behind `check.cpp`'s
      refusal on the base, exit 139, and is now an explicit stop with its cases), with the NOTAUTO guards,
      the marked entry and back-edge rows, the range place's conjuncts, the pair rows against the other
      written entries and the splits; a structure with no cell leaf gets an entry that carries the basic row
      and the liveness. **The amount is `write`, not the first slot's ghost amount of the S48 (k) fixture:**
      the fixture's `Perm` local would let an `owns` invariant stand over a fraction at the loop's entry;
      with `write` the entry row asks for the whole array (stricter than this item's first wording, never
      weaker). **Still refused, each rejected by the encoder at the atom's line with its own text**
      (`unsupported_annotation`; M4.5e2; `check.cpp` repeats each): a written `reads(*p)` and `reads(p->a)`
      (S48 (k) did not cover a written `reads` atom, and none was put through Silicon); a written atom whose
      pointer is read through memory, is a global pointer, an address-taken local or a by-value nest
      (`owns(*r->q)`, `owns(w->s.a)`); **a written atom on such a structure whose guard reads memory**
      (`r->z > 0 ? owns(*c) : true`: audit round D2, finding 2, found that the array part's guard was lowered
      to a constant there, fail closed, a false `permission` or more demanded than written; a guard that
      reads no memory, `p == NULL || owns(*p)`, is accepted). **What "reads memory" means as built** (the
      short read after round D2, finding 1): a dereference, a member or element read through a pointer, a
      global, a `static` local, an atom or an application; **a guard that reads an address-taken LOCAL by
      its name** (`int x = 1; int *px = &x;` … `x > 0 ? owns(*c) : true`) is not counted, is accepted, and
      is lowered as a real read of the local's cell, framed by the loop's own place for it, not as a
      constant (four probes under Silicon behave: a guard the body flips to false loses the part and the
      writes fail; a guard kept true keeps it). The shared predicate `vir::reads_heap` decides all three
      layers and was left as it is (changing it reaches other rules of other records and wants its own
      read); the lint refuses the Viper of such a program (no test holds one; handoff §3); and a written atom on the structure or on ANY
      array member of a structure that has an aggregate-element array member (question 348; a written atom
      on one of its scalar cells, `owns(p->n)`, is accepted as it was before this record: audit round D2,
      finding 3). At the snapshot the two audit rounds read, the first two were refused by `check.cpp` alone
      (the generic `unsupported_expression`), and the last two forms were accepted or mis-lowered as said;
      the fixes landed with the commit and were read again (the short read D2b, in this record's header). **Listed false alarms** (fail closed): a loop
      invariant that READS an element of a moved written atom's array, or of a `calloc`ed block the loop
      holds (`p->a[0]` in the invariant), is `permission` ("this loop invariant reads `p->a[0]`, which this
      loop does not hold").
    - **Owners in loops** (amends 0055 item 7, 0054 items 5 and 6; built with Part D's loop column). The
      carriers of an owner's block across a loop (the written `owns_block` sites, the full-owner carriers of
      an owning local, global and member, and a marked closure's kept-flag row) take the block's array parts
      with the cells: full-permission quantified conjuncts per array leaf, marked entry and back-edge rows
      over a bound local, and for a marked closure a per-part snapshot of the flags; a site whose pointer
      is read through memory names its array parts by a ghost pointer, taken again at the back edge when
      the loop may write the pointer (a loop that frees and reallocates an owning global's block). **Flags
      across the loop** (corrected after audit round D2, finding 4): a written site keeps its block's flags
      through a snapshot taken at the loop's start, as a checked invariant, so a read after a loop in which
      the owner held its block throughout verifies; the full-owner carrier of an owner the loop does not
      name states no flags, and a block the loop itself gives the owner (an owner that may be null when the
      loop starts; a block freed and allocated again) has none to keep: a read of such a block's element or
      cell after the loop is `uninitialized` until it is written again (a listed false alarm that the
      structure without an array member has too; confirmed on the base build). **Listed false alarm:** a loop that reassigns an owning
      MEMBER's block (`free(r->q); r->q = calloc(…)`) reports `leak` and `permission` rows where a correct
      program has none (it predates this record for a structure without an array member: the same six rows
      on the base build). **Also listed, each fail closed** (audit rounds D1 and D2): a marked member's block
      freed and allocated again in a loop (eight rows); two owning globals swapped in a loop (`leak` at the
      store); a callee with a load-rooted atom, or over an owning member, called in a loop (`permission`:
      "needs to own `*r->q`"); under a written `owns(c->a)` alone, with the pointer moved by the loop, the
      address of an element passed to a callee is `lifetime` (the member atom carries no liveness; element
      reads and writes are accepted); `owns(r->a)` on the result of a function without a body is rejected
      with 0052 item 2a's message although 2a allows a place reached through the result (over-strict); the
      member atom is printed `&p->a` in some messages where the source says `p->a`. **No verdict in the
      time limit (exit 3, never a proof), measured by audit round D2:** two guarded written atoms whose
      guard the loop flips; two owning or two marked members swapped in a loop; a caller of a bodiless
      function with two `reads` atoms through two loaded pointers, in five of the shapes tried.
    - **Array members of scalars only** (question 348, settled with the advisor on 2026-11-26, before Part
      D's code; fail closed). This item, the continuation plan's Part D table and the spike's rows (h), (i),
      (k) and (l) were written before Part C2 and are about a member that is an array of SCALARS (`struct SA
      { int n; int a[4]; }`). Since Part C2 a structure may hold a member that is an array of structures, and
      from Part E one that is an array of arrays; every gate of Part D tests "has an array member" whatever
      the element, so lifting it as written would admit them at sites whose Viper no fixture has verified,
      one of them a TRUSTED contract (the typed view of `malloc` and `calloc` holding one quantifier per leaf
      of every element under a token). **Rule:** at each of this item's six positions a structure with an
      array member whose element is a structure or an array, at any depth of its by-value nest, stays
      rejected: `malloc`, `calloc`, `free` and `owns_block` of it; an `owning` or `ref_to_uninit` member
      beside such a member; `ref_to_uninit` or `must_init` on a pointer to it, and `owning` on one; a place
      of it reached through a heap read, `p->q->v[e].x` included; an `ensures` atom on it with no `requires`
      partner; a written atom in a loop invariant on it or on any array member of it (a written atom on one
      of its scalar cells is accepted, as before this record). The encoder rejects each at the C line with a
      message that names this item; `check.cpp` refuses each (the backstop); and **lowering stops with an
      internal error for an aggregate element at each of these sites that would otherwise take the general
      leaf path Part C2 built** (the array parts of `atom`, `call_atoms` and `atom_returns` under a marker,
      for an output site and for a load-rooted site; `contract_read`'s structure-element branch for a
      load-rooted base; the block's array parts), each pinned by a self-test case. Everything Part C2
      accepts for such a structure is unchanged (`owns(*p)`, `owns(p->v)`, locals, globals, values, the
      automatic loop places). The alternative, the general leaf path at these six positions now, needs
      fixtures for four shape families first (per-leaf quantifiers inside `owns_block` and consumed by
      `free`, with the typed view over them; marked per-leaf rows and the kept-flag quantifier per leaf
      path; a written atom and a partnerless amount per leaf of an element structure) and was not what the
      plan sized; it is listed for M4.5e2, which builds `malloc(n * sizeof(struct S))` and with it the same
      rows under a token, unless the owner places it elsewhere.
16. **Brace elision under `{0}`: the measured rule** (the owner's answer to question 304, "Follow the rejection
    like the compiler warning"; question 321, the advisor's ruling by the plan's P83 (h) row, "the measured set
    is the rule"; amends 0043 item 6, 0063 item 6). The planning worded the rejection as "C++'s `{0}` over a
    first member that is a structure or an array, in an aggregate of two or more members"; the matrix M
    (clang 21.1.8 and GCC 16.0.1 on the box, 668 shapes, both modes, three tiers; the Mac's compilers agree)
    measured a wider set, and the rule is that set, 0020 item 5's union of the first two tiers over both
    compilers:
    - **In C++** (clang++ `-Wmissing-braces` at `-Wall`; GCC's C++ never warns): a braced list is rejected when
      some subaggregate whose braces it elides has, as its parent, **an array (of any length, one element
      included) or a structure with two or more members**. An elided subaggregate that is the only member of
      its structure never makes the list rejected, at any depth. Storage and position (a local, a global, a
      compound literal, a member, an element) do not matter, and neither does the value (`{1}` is flagged
      where `{0}` is). So C++'s `struct P v[2] = {0};`, `int m[2][3] = {0};`, `{0}` over a structure of two
      members whose first is a structure or an array, and `{0}` over a one-member structure that holds a
      matrix, an array of structures or a structure beginning with an array (the inner elision's parent is an
      array or has two members) are rejected; `{0}` over `struct T { struct P p; }` and over `struct T { int a[4]; }` is accepted, as before, and so
      is `{0}` over a chain of only members that ends in an array (`struct B1 { struct A1 x; }` with `struct A1
      { int a[4]; }`), which the base rejected by the same elided recursion as C's defect below and on which
      both compilers are silent at every tier: the one C++ form this item newly accepts.
    - **In C** (GCC `-Wmissing-braces` at `-Wall`): **a braced list nested inside the object's initializer
      whose element elides a subaggregate's braces is rejected** (`struct W { T t; int k; } w = {{0}};` with
      `T` an aggregate whose first member is one; `T v[2] = {{0}}` likewise; `{{{0}}}` and `T t = {{0}}` over
      a structure that begins with a structure beginning with an array): GCC flags any elision except where
      the whole initializer is the lone `{0}`. **The lone `{0}` as the whole initializer stays accepted at any
      depth** (C's universal zero initializer: a local, a global, a compound literal `(T){0}`, an array `T
      v[2] = {0}`, a matrix), and **C's lone `{0}` over a structure whose first member is a structure that
      begins with an array is accepted** (both compilers are silent): the base rejects it in both modes with
      a message that says C accepts it (`collect_init_elems`' `read_list` under the elided recursion; Step
      0's R2), a defect this item fixes for C. **The lone zero written otherwise than as an integer literal**
      (audit round 1, issue 5; question 325, the owner, 2026-11-25: "Probe, reject if the IL can tell") —
      `{(0)}`, `{'\0'}`, an enumerator, `{1 - 1}`, `{false}`, `{NULL}`, `{nullptr}`, `{-0}`, `{+0}`, `{(int)0}`
      — which the base accepts by its value and on which clang's C warns `-Wmissing-braces` at `-Wall`
      wherever its C++ predicate applies (GCC is silent; the matrix M held only the literal; clang's C is
      silent on `0`, `0u`, `0L`, `0x0`, `00`, `0b0`, `0ULL` and on a macro that expands to such a literal):
      **the probe P84 (a) took its middle branch, and the rule is built for the spellings the IL tells**
      (question 339, settled with the advisor by that row's default; the continuation's Part R). **The
      literal test:** peel every compiler-generated cast from the lone element; it is a literal zero when the
      innermost constant keeps no `expr`, carries `is_simple_zero` (`0`) or `non_arithmetic` (`0x0`, `00`,
      `0b0`, and the suffixed zeros `0u`, `0L`, `0ULL`, which sit under a generated cast), is not a named
      constant's definition (an enumerator) and is not of type `bool`. **The rule, in C only:** where the
      whole initializer is an undesignated lone zero that elides braces and this item's C++ predicate applies
      (an elided subaggregate whose parent is an array or a structure of two or more members), a zero that
      fails the literal test is rejected (`unsupported_expression`, pinned): `{'\0'}`, an enumerator, `{1 -
      1}`, `{false}`, `{NULL}`, `{nullptr}`, `{-0}`, `{+0}`, `{(int)0}`. **Not told by the IL, still accepted
      by their value, and listed for M8's warning sweep:** `{(0)}`, `{((0))}`, `{(0u)}`, a macro whose
      body is `(0)` and `{__extension__ 0}` (audit round R, note 6) — no field of a constant or an expression node records a parenthesis, and the source
      columns that differ are layout, not an IL field (the owner's condition). Outside the rule, unchanged
      and listed for M8 with them: a designated element (`{.p = '\0'}`; `{.p = 0}` is accepted as before,
      both compilers silent in C) and a zero inside a nested list, which the probe did not run (audit round R
      measured the designated cells: clang's C flags `{.p = '\0'}` and `{.t = '\0'}` and is silent on `{.p =
      0}`; they stay accepted). **The rule's three call sites** are an elided element of an array, a nested
      structure in a parent of two or more members, and an array member in such a parent; the first two are
      pinned at Commit 1 and the third, with a global, a `static` local and a compound literal, by Commit 2
      (audit round R, should-fix 3: the behaviour matches clang's C on every cell measured, 65 in all). Not unsoundness: a warning shape, rejected like the compilers. No existing test
      or doc example may change outcome by it; one that does goes to the owner before it is touched.
    - **Designated lists** (96 shapes, each designated form beside its plain control, then 62 forms with
      every warning either compiler prints without a flag or at `-Wall` recorded by name). A designator does
      not change `-Wmissing-braces`: a **braced** designated value whose `0` elides (`struct W w = {.t =
      {0}};` with `struct T { struct P p; int z; }`) is the nested list of the C clause and is rejected in C,
      and in C++ where the predicate flags it (not for an only member: `{.t = {0}}` over `struct T1 { struct
      P p; }` is accepted in C++ and rejected in C). An **unbraced** designated member that is a structure or
      an array (`{.t = 0}`, `{.p = 0}`, `{.a = 1}`): in C a zero is silent in both compilers and stays
      accepted, a lone-zero chain that ends in an array member included (the base rejected that one by the
      elided recursion), and a non-zero value is rejected (GCC flags it; the base accepted it); **in C++ it is
      rejected whatever its parent and its value** (`unsupported_expression`): clang++ warns
      `-Wc99-designator` on every such form without a flag ("brace elision for designated initializer is a
      C99 extension"), and `-Wmissing-braces` too at `-Wall` where the parent has two or more members; g++ is
      silent. The base accepted the C++ forms (`{.p = 0}` since M4.5c1): a changed outcome for programs
      accepted before this record, by 0020 item 5's union rule and the existing rule for that warning (0059
      item 3 rejects C++'s array designators for it); no existing test or doc example holds one (question
      327, for the owner's review).
    - `{}` elides nothing and is accepted in both modes. A list with every level's braces written is accepted.
      A non-zero list that elides braces stays rejected in both modes as 0043 item 6 and 0063 item 6 have it
      (stricter than clang++ for an only member; unchanged).
    - **What changes for programs accepted before this record** (Step 0's addenda: 60 runs of the frozen base
      over every flagged form the base can express, as a local, a global and a compound literal, which agree):
      in C++, `{0}` over a first member that is a nested structure in an aggregate of two or more members
      (question 304), and `struct W w = {{0}};` where the inner `0` elides a nested structure in a member of
      two or more members (clang++ flags it, the base accepts it); in C, the forms GCC flags and the base
      accepts: `struct W { T t; int k; } w = {{0}};` with `T`'s first member a structure or an array, and
      `{{{0}}}` or `T t = {{0}}` over a structure that begins with a structure beginning with an array. The
      base already rejects C++'s forms over an array member (question 303) and, in both modes, `{{0}}` over a
      structure beginning with an array. **No existing file changes outcome:** `{{0}}` and `{{{0}}}` occur in
      no file under `tests/` or `doc/modules/ROOT/examples/`, and a compile pass of all 5,344 test and example
      sources with `-Wall -Wmissing-braces` (both compilers, both modes) flags seven files, every one already
      a rejection pin for the flagged mode or flagged only by GCC's C++ with the explicit flag (not in the
      rule). **An existing file whose outcome changes at Part B's joined proof goes to the owner before it is
      touched** (the plan's Flips G). The rule goes beyond question 304's wording and is logged for the
      owner's review (question 321).
    - The rule is in `collect_init_elems`, which locals, globals and values share. A C positive in the tests
      avoids every row GCC flags (the `native-ok` policy).
17. **The lint** (amends 0012 item 11, 0060 item 16, 0061 item 11, 0063 item 15).
    - **The receiver grammar** gains index steps: a quantified row's receiver is a chain of `loc(<block>, <bound
      variable or 0>)` and `member(<ref>, <literal>)` steps from `block_of(<bound local>)` in code and from a
      block term of 0063 item 15's grammar where a line has no local, with one bound variable per index step,
      each ranging over literal or `offset_of` bounds as today; the trigger is the receiver.
    - The length facts of item 3 are pinned where they are stated; `alloc_arr_<E>` and `zero_arr_<E>` by their
      clause sets, each (receiver, field) once; `program_start`'s row group per leaf path; a user quantifier's
      class trigger with member steps (item 14).
    - Each rule with a refusing twin in `scripts/test_lint_viper.py`.
    - **As built through Part C1** (the continuation's Commit 2): leaf rows in a range atom's `requires`
      and `ensures` and in a call's rows; the length-facts row of a range, and its tie to every leaf row;
      the `reads_range` amount as a minimum over each leaf of the first slot; a scope exit's rows tied to
      its allocation's (receiver, field) set (the first session's audit 2, issue 7). **Fixed with Commit 3**
      (audit round C1, findings 6 and 4): the scope-exit rule reads the rows before every `goto l_end` and
      before the end label, and an `assert false` in the method hides nothing (at Commit 2 it switched the
      rule off, so a method that dropped an array's exit whole was not refused); the well-formedness row of
      a pointer-to-structure leaf is admitted as the lowering writes it. **As built through Part C2**
      (Commit 3): the length facts of a global array of structures and of a structure global's member array
      are clauses of their own after the global facts' clause, each followed by its row group in
      `program_start`; the well-formedness row of a pointer to a structure with a member array of
      structures, in a `requires`, an `ensures`, an `assert` and an `f__wf` prior; `struct P *pg = g;` as a
      pointer to a structure global; the `Perm`-local minimum over a member array's leaf amounts. **Limits
      of these rules, listed** (each admits a weaker assumption only): the pointer-to-structure row may
      state fewer member lengths than the pointee has; with one pointer field any element shape is admitted
      for `pg`. **Not inspected by any rule, by design** (audit round C1, note 8): the
      pairing rows of a call, the `? :` terms of a marked amount, a cell's sum row and the contract-read
      framing disjunction, each of which Silicon checks again (a wrong one is a false alarm or an exit 3:
      a marked row consumes and gives nothing, and the amount a callee receives is bound by Silicon's own
      exhale of its precondition); and the sequencing rows and `f__wf`'s ghost names, which Silicon does
      NOT check again and which tests pin instead (item 11; `tests/memory/range_struct_wf.c` and
      `negative/range_struct_wf_division.c` for the ghosts: one per leaf path and slot, a `reads_range`'s
      post value tied to its pre value and an `owns_range`'s not).
    - **As built through Part D** (the continuation's Commit 4; the test-mutators' reports 2 to 12 of that
      session say what each rule does not check): the typed views of `malloc`, `calloc` and `free` for a
      structure with array members, rebuilt and compared clause by clause (an array leaf's three quantified
      rows over its own block at the count of its `len` fact; `calloc`'s zero values as one contract read
      per element, literal indexes, each once; several array leaves; arrays of `bool`, enumerations and
      pointers); `owns_block` clauses with array quantifiers, also nested under a guarded pointer leaf; a
      block term rooted at one pointer load in a `requires` or an `ensures`; a heap block's array part as a
      loop place (the amount a snapshot of its own first slot and field; the flag and value antecedents);
      the owner and written-site carriers (full-permission rows at the count of a length fact of the same
      method, their entry rows under the binding guard, the `wea_` flag snapshots); written atoms in a loop
      invariant (rows tied to an invariant of the same method by block, count, field and guard; no amount;
      the values-kept conjunct; never over a block reached through a heap read; the ghost pointer's two
      assignments tied to the site's own cell, pointer and guard). **Narrowed after audit round D1,
      finding 4:** an `assert` row's guard may read the heap only by the canonical null test of a canonical
      load. **Not admitted, by a decision that is the owner's** (question 357): a quantified row whose
      amount is 0045 item 7's partner sum (nested conditionals over `Perm` formals), which a contract with
      two atoms of one kind through two loaded pointers gives an array part; an edit that would have
      admitted it was denied by the permission system, the refusal stands unchanged, and the two tests of
      that shape are under `internal/pending/m4.5e1e2c/`. **Accepted shapes the lint refuses, none held by
      a test:** that one; two owning globals of such structures in one loop; an owning global of a
      structure holding such a structure by value; a written atom under a guard that reads an
      address-taken local. **No backstop:** a written atom's guard lowered to a constant would pass the
      lint (the checker's refusal and the lowering's internal error stand there instead).
18. **Categories and tags:** none new. **Facts:** P83, S48.
19. **`restrict`: every `restrict` parameter is rejected** (question 326, the owner, 2026-11-25: "Reject
    restrict"; this item was rewritten on 2026-11-26, before the code of the continuation's Part R; amends 0037
    item 2, 0040 item 3, 0043 item 10, 0044 item 11, 0045 item 13, 0047 item 4, 0052 item 6 (d), 0054 item 2
    and 0060 item 14). **Not of this record's arrays: a hole in the base,** found by audit round 1 of the first
    session. The first version of this item, which that session built, is `git show
    f313023:internal/decisions/0064-arrays-of-structures-and-arrays-of-arrays.md`, with the programs that
    verified wrongly.
    - **The rule.** A pointer parameter qualified `restrict` (spelled `restrict`, `__restrict`, `__restrict__`,
      `int p[restrict]` or through a typedef) is rejected, `unsupported_annotation`, at the function's head (the
      position the earlier `restrict` rejections used), once per function, naming every such parameter: on a
      definition, and on a declaration without a body (question 332). **Where each is reported** (question
      338, settled with the advisor; the facts are edg-backend's report 3 of the continuation, 20 single-file
      runs): a definition, called or not, a `static` one included, at its own head line. **A declaration
      without a body where the encoder first reads its signature:** at the first call of it in the unit (one
      row, at that call; a second call, in the same function or another, adds none), as every rejection of a
      bodiless routine's signature is placed; or at the declaration itself when its signature is read early
      whether or not it is called (a structure result or a pointer-to-structure parameter, a parameter marker,
      `not_invalidating`, a block atom in its contract). **A bodiless `restrict` prototype that the unit never
      calls and that none of those reads early draws no diagnostic:** it is neither verified nor called, so no
      result rests on it (fail closed). An eager rejection of every such prototype is not built: it would
      reject a unit for a declaration it only includes and never uses (a header's, M4.6). **Which declaration
      counts for a function without a body** (audit round R, should-fix 1; question 341, settled with the
      advisor, fail closed): it is rejected when ANY of its declarations in the unit gives a parameter the
      qualifier, in C and in C++, reported as above. The front end's merged type of two declarations that
      differ in a parameter's top-level qualifiers carries none, so the encoder reads each declaration's own
      written type. **Built with the continuation's Commit 2** (`declared_restrict_params`; an `int
      p[restrict]` on a second declaration is read too; pins `restrict_reject_redeclared.c` and
      `restrict_reject_redeclared_cpp.cpp`). In C++ the rejection is `vtype`'s `unsupported_type`, at the head
      of the function's first declaration, whichever declaration has the keyword. Commit 1's build read only
      the merged type and accepted a called function without a body declared once with `restrict` and once
      without it (in either order, or with a differing `const`), and in C++ one declared plain first and
      with `__restrict` second: not unsound (no body is verified, the contract is trusted either way, and C
      gives a prototype's `restrict` no meaning), and it verified on the base too. **The qualifier is
      read where it is read today:** from the definition's own parameters when the unit holds the definition, from the declaration
      otherwise. So `restrict` written only on a prototype of a function that the unit defines without it,
      before or after the definition, stays accepted, as it is today: C attaches the promise to the block of the definition (6.7.3.1), and
      that definition makes none. The message says what to do: "cerastes does not model `restrict`; dropping
      the keyword does not change what a program with defined behaviour does, so drop it to verify the
      function". `restrict` on a local, a global, a member, a pointee or a result, and C++'s `__restrict`,
      stay `unsupported_type` as before (0032 item 2, 0037 item 15). A `pure` function's and a quantifier
      function's `restrict` parameter keep their own rejections (0043 item 10, 0061 item 3); a `pure` function
      that has no contract and that nothing reads is not encoded at all, so its `restrict` parameter draws
      nothing (audit round R, note 7h). **The one form of the keyword still accepted** (note 7d; also on the
      base; listed for M8 with the model): the type of a cast or of a compound literal, `(int *restrict)p` and
      `*(int *restrict){p} = 1;`. No declared identifier designates such an object, so C23 6.7.3.1 attaches no
      promise to it by the auditor's reading, and one program run under both compilers at `-O0` and `-O2`
      agrees with the model; accepted, not shown wrong. **What the proof is of** (note 7b): the program
      without the keyword. A unit that hides the keyword from cerastes only (`#define restrict` under
      `__CERASTES__`) verifies as if it were dropped, and says nothing of the program compiled with it.
    - **Why.** `restrict` promises that an object modified through the parameter is reached only through
      pointers based on it (C23 6.7.3.1), and a compiler may reorder accesses on that promise. Permissions
      belong to cells, not to provenance, so owning `*p` does not keep the promise: the function may reach the
      same cell by another name. Records 0037, 0043, 0044, 0052, 0060 and this item's first version accepted
      `restrict` under conditions that enumerate the routes to another name; three audit reads in one day each
      found a route the rule then in force missed, and one form still verified when the first session closed:
      `int g; int f(int *restrict p, int *q, int n)` with `requires(owns_range(p, n) && owns_range(q, n) &&
      owns(g) && p == &g && q == &g)`, body `p[0] = 1; q[0] = 2; return p[0];`, called as `f(&g, &g, 0)`,
      verified `r == 2` where `gcc -O2` and `clang -O2` return 1 (two ranges of the same count 0 beside a
      global the contract holds and the body never names). An enumeration is not a proof. Removing the keyword
      removes only undefined behaviour, so the rejection costs a user one word.
    - **What it withdraws.** The acceptance of 0037 item 2, 0040 item 3 (`owning` as the `owns` conjunct), 0043
      item 10, 0044 item 11, 0045 item 13, 0047 item 4 and 0052 item 6 (d) (block sites), 0054 item 2, 0060
      item 14, and this item's first version: its clauses (i) to (iv), the exemption and the pair rule. With
      them goes the `reaches` walker of `restrict_backed` that item 4 names: once no `restrict` parameter is
      accepted, no later part has a `restrict` rule to keep in step.
    - **Where.** The encoder rejects in `function_of`; `check.cpp` refuses any parameter that carries the flag
      (the backstop, with a self-test case); `vir.cpp`'s `restrict` functions go. `lower.cpp` never read the
      flag: an accepted `restrict` added nothing to the Viper, so removing the acceptance changes no other
      program's Viper.
    - **What changes for programs accepted before.** Every function that verified with a `restrict` parameter
      is rejected until the keyword is dropped. **The owner approved exactly 17 test files** (invariant 1; the
      continuation plan's Flips R): 8 positives become rejection pins, one row per function with a `restrict`
      parameter at its head line (`block_restrict.c`, `owning_restrict.c`, `range_restrict.c`,
      `restrict_body_owns_global.c`, `restrict_body_pair_ok.c`, `restrict_owned.c`, `struct_global_restrict.c`,
      `struct_value_restrict.c`), and their 9 error twins are retired. The 33 pins that already reject on a
      `restrict` line and `forall_reject_f_restrict.c` keep their `expect:` rows: the new rejection has the tag
      and the position the old ones had; one whose tag or line moves goes to the owner before it is touched.
      The harness's twin rules (question 337) move the 8 into `tests/memory/negative/` and give the rule a new
      positive, `tests/memory/restrict_dropped.c` (the same functions with the keyword dropped, as the message
      tells a user to write them), with an error twin; the 14 pins whose `twin:` line named one of the 8 name
      it instead. No doc example, no vlibc header and not `cerastes.h` uses `restrict`.
    - **Tests:** the positive and its twin above; `restrict_reject_declaration.c` (a prototype with a
      `restrict` parameter and a contract, no body, called: rejected; and the form that stays accepted, a
      `restrict` prototype before a definition without the keyword), with a row for each spelling the rule
      names (`__restrict`, `__restrict__`, `int p[restrict]`, a typedef'd restricted pointer, a plain prototype
      before a `restrict` definition, a second parameter alone, an uncalled `static` definition, a bodiless
      declaration called twice, and one read early at its declaration): each draws its row on the build (the
      same report; no spelling verifies); three hand-built VIR rejection cases for
      `check.cpp`'s backstop (a `restrict` parameter with `owns`; with a range; on a function with no body),
      which replace the 60 `restrict` cases of the first version and of records 0037 and 0060 (22 accepted, 38
      rejected) and the reachability check `check_restrict_reach`.
    - **Later.** A model of `restrict` (every access based on `p` inside `p`'s atom, every other access outside
      it, with a soundness argument in `internal/soundness/`) is an M8 entry. M4.6 decides what a library
      declaration with `restrict` parameters (`memcpy`) needs when vlibc's `<string.h>` arrives.


## Facts and the branches taken

Each probe and spike of the plan's section 6, its outcome, the branch of its decision table, and the item it
revised. (To be filled from P83 and S48.)

| Row | Outcome | Branch taken | Item revised |
|---|---|---|---|
| P83 (a) arrays of structures in code | the variable's type is e1a's `tk_array` entry over the structure; `v[i].x` is `dot_field(subscript(array_to_pointer(var v), i), x)` and `p[i].x` the same over `var p` with no `array_to_pointer`; copies are `eok_assign` or `dik_expression`, never a call; a by-value argument is the subscript; `sizeof(v) / sizeof(v[0])` folds to a constant that keeps its `expr`. In C the decay of a global array is folded into a `ck_address` constant in every rvalue position of a declaration's initializer and as a whole call argument (both modes), never on either side of an assignment or in an annotation | Expected (refined: where the decay is folded) | 5 |
| P83 (b) initializers of arrays of aggregates | one nested `ck_aggregate` per written element, trailing elements absent; elision is told only by the inner aggregate lacking `explicit_braces_on_aggregate`, and `{0}` over an array of structures has the shape of an elided `{3}`; an array designator is a `ck_designator`; an excess element is the front end's error in both modes | Expected | 6, 16 |
| P83 (c) arrays of arrays | a `tk_array` entry over a `tk_array` entry, each bound with its own `expr` in every position (in C++ every non-local bound of both dimensions under a generated cast); `m[i][j]` is `subscript(array_to_pointer(subscript(array_to_pointer(var m), i)), j)`; a row's use is `array_to_pointer` over the inner subscript | Expected | 2, 13 |
| P83 (d) array-of-rows parameters | `int a[][3]`, `int a[2][3]` and `int (*a)[3]` share one adjusted type, pointer to the row's `tk_array` entry with its bound's `expr`; the written type is the parameter type's `declared_type` (so the spelling can be told), and `[]` has no bound constant; `sizeof(a)` is the pointer's size; a wrong inner bound at a call is an error in both modes (in C EDG's #167, which the driver already makes an error) | Expected (cut 5 not taken) | 13 |
| P83 (e) members and nesting | the field's type is (a)'s or (c)'s entry and the accesses compose; the C++ class tests of 0043 item 2 pass for a structure with a member that is an array of structures, a matrix member and an element structure with an array member (two trivial `operator=` each) | Expected | 2, 15 |
| P83 (f) globals | `initk_static` over (b)'s shapes; a `static` local through `local-static-variable-init`; pointer leaves are `ck_address`; P38's loss of `expr` is keyed to the outermost list (1,001 elements keep every `expr`, 1,002 lose it for every leaf below; `unsigned char big[2][1002]` keeps all) | Expected (refined: P38 by the outermost list) | 9 |
| P83 (g) quantifier bodies and arguments | a body's read is (a)'s shape over the parameter; each `cerastes_forall` argument is `array_to_pointer` over a field or a variable, with no load, and a global is not folded in an annotation; a `pure` application's argument is under a compiler-generated qualification cast, which `application` peels | Expected (refined: the generated cast) | 14 |
| P83 (h) the compiler matrix M (Step 0) | clang 21.1.8 and GCC 16.0.1 on the box, both modes, three tiers, 668 shapes, 8,016 compiles (the Mac's compilers agree on all but 8 cells, none of `-Wmissing-braces` or `-Wsequence-point`): every positive shape compiles with `-Wall -Wextra -Werror`; EDG's #175 fires on every constant-index form but `&m[2][0]` and `&m[1][3]`; `sizeof(a)` of a parameter warns for the `[]` spellings and not for `int (*a)[3]`; **question 304's measured set is wider than the planned wording** (clang++: an elided subaggregate whose parent is an array or a structure of two or more members; GCC's C: any elision but the lone whole-initializer `{0}`); **one family with no rule:** GCC `-Waddress` at `-Wall` on a truth test or null comparison of a row or an element address | the rule; "If not" for question 304 (the measured set is the rule, question 321) and for the new family (question 322) | 5, 16 |
| S48 (a) a local array of structures | `alloc_arr_P(3)` in item 6's shape (the length-facts quantifier, one quantifier per leaf and permission), `v[i].x` read and written for a symbolic `i`, `&v[i]` passed to an `owns(*p)` callee with its cell lines taken out of the quantified chunks and given back, the scope exit per leaf: verified; the twins one error each (the bound row; the first marked scope-exit row); 4 fixtures, 7.1–7.7 s | Expected | 3, 5, 6 |
| S48 (b) a range over structures | `owns_range` in a callee that writes `p[i].x` with the caller reading it back; `reads_range` at a ghost amount with a forwarded `(p, n)` call; a return's rows: verified; the twins one error each (a write under `reads_range`; a caller holding one element fewer, at the first marked row); 5 fixtures, 6.9–13.1 s | Expected | 7, 8 |
| S48 (c) the pairing rows | each of the spike's three pairings (a range of structures beside a cell one member step below the slot, beside a structure atom of the element's type, two halves of one fresh array) in both forms: every positive verifies and every alias twin is exactly one error (Form P: `assertion.false` at the pure row; Form S: `insufficient.permission` at the first marked sum row); no alias twin verifies in either form; 8 fixtures, 16.8–25.2 s. **Audit round 1 found the pairings too few:** the fixtures' block row for two ranges is not decided where a function forwards two of its own ranges (a false `permission`), and their single index inversion lets an alias through a nested leaf, a structure atom of another type or a range of another element structure pass the row (Silicon's own precondition check then fails: exit 3, never a proof). The slot row and the per-leaf-path rows of item 7 decide every such pairing in the audit's probes (`build/m4.5e1e2/audit1/`), and the fixtures added after the audit pass them (8 `s48_c2_*` files; the alias twins one error each at the row) | Form P for every pairing, written per pair of leaf paths (or by a nested site's slot) with the slot row for two ranges (item 7); Form S not needed | 7 |
| S48 (d) loops over an array of structures | a local array and a parameter's range, each leaf its own place with the entry marked rows and the conjuncts, one liveness conjunct per root; a loop writing `p[i].x` under a quantified invariant: verified; the twins one `invariant-preserved` each (the user's conjunct; the values-kept conjunct of leaf `y` with a store through an `int *` in the body, so that conjunct must be absent there); 4 fixtures, 9.6–15.9 s | Expected | 10 |
| S48 (e) a global array of structures | `program_start` with item 9's row group per leaf (`struct S g[3] = {{1, 2}, {3}}`: one conjunct per leaf of each written element, one quantifier per leaf for the rest), `main`'s loop, an `owns(g)` callee (which needs the global facts among its `requires`, as 0041 has them): verified; the twins one error each; 4 fixtures, 6.6–9.5 s | Expected (cut 3 not taken) | 9 |
| S48 (f) an array member of structures | `alloc_struct_T` for `struct T { struct P v[2]; int n; }` with the nested length facts and one quantifier per leaf of `v` and permission over `loc(member(loc(member(r, 0), k), j), 0)`; `t.v[i].x` as a local and under `owns(*p)`: verified; `f(&t, &t.v[1])` one error at the Form P slot row; 3 fixtures, 9.1–10.6 s | Expected (cut 4 not taken) | 3, 7 |
| S48 (g) depth 2 | an array-of-rows parameter with `owns_range(a, n)` over two bound variables and a caller passing `m` (8.6 s; `m[0][3]` one error at the second bound); a row `m[1]` bound to a local and passed to a one-variable `owns_range(q, 3)` callee (8.6 s: Silicon takes the one-variable quantifier out of the two-variable chunk and gives it back; the keeping callee one error); `v[i].a[j]` over `struct W { int a[4]; int n; } v[2]`; the copy `v[i] = v[j]` under the guard `d != s`, which Silicon decides in both directions (the unguarded twin one `precondition` error at the copy): verified, **34.5 s and 35.2 s at load about 2, over the row's 30 s line** | Expected for the row call, the parameter and `v[i].a[j]` (cut 5 not taken); the copy's time to the advisor (question 323) | 5, 12, 13 |
| S48 (n) cost | an array of structures with four leaves and a loop over each 58.9 s (load 0.6); S45 (k)'s scale program at K = 16 holding one array of structures 52.7 s (load 1.3); a global array of 1,000 structures with 16 listed 8.5 s | recorded (under the 60 s line, two of them closely) | — |
| S48 (h) heap storage | the typed view of `malloc(sizeof(struct S))` for a structure with an array member in the block-term form (the token, the member lengths, the array leaf as three quantified lines, flags unknown), `p->a[i]` written and read, `free`'s rows with the array leaf's after the cell's: verified; a second `free` fails alone at the invalid-free token row, a leak alone at the `forperm` row; the vacuity twin one error; 4 fixtures, 7.3–7.9 s | Expected (the first form tried passed) | 15 (heap) |
| S48 (i) markers | a `ref_to_uninit` pointer to a structure with an array member (no flag line in the `requires` atom for either leaf; one kept-flag quantifier per array leaf in the `ensures` atom) and a `must_init` filler (a `forall k` flag row per array leaf at the return), each with a caller: verified; a caller reading an unwritten element fails alone at its flag row, a filler that misses one element alone at the array leaf's flags row; 3 fixtures, 8.4–9.6 s | Expected (the first form tried passed) | 15 (markers) |
| S48 (j) a read class with member steps | a user quantifier over `p[q].x` in a `requires`, an `ensures` and a fill loop's invariant with `f__wf`'s variant M over the leaf, and one over a member array in the block-term form (`{ loc(member(p, 1), q) }`), each with a caller: verified; the twin one `invariant-preserved`; the vacuity twin one error per method; 3 fixtures, 11.4–12.1 s. The trigger is the leaf's receiver; no other form was needed | Expected (cut 1 not taken) | 14 |
| S48 (k) a written atom | `owns(*p)` written in a loop invariant for a structure with an array member, the leaf's amount its first slot's: verified; a callee that keeps the array leaf fails alone at the back edge's first array row; 2 fixtures, 9.2–9.8 s. Not covered: a written `reads(*p)` atom | Expected (built, not left rejected) | 15 (written atoms) |
| S48 (l) a partnerless atom | `ensures(r, owns(*r))` of a structure with an array member, bodiless and through a pass-through with a body, given its output amount, with a caller: verified; the twin one error; 2 fixtures, 7.7–8.4 s. Not covered: a covered atom, a fresh object from the function's own body | Expected (built, not left rejected) | 15 (partnerless) |
| S48 (m) depth 3 | `int c[2][3][4]` with a permission over three bound variables, three nested loops and three bound rows: verified in 10.2 s; `c[1][2][4]` fails alone at the third bound; the vacuity twin one error; 3 fixtures, 9.7–10.7 s at load 4–6, no load dependence seen | depth 3 admitted (every program under 30 s) | 2 |
| S48 after audit round 1 | 25 fixtures on top of the cap (question 324): the pairing rows above (8); the trusted `zero_arr_<E>` per leaf path of a nested structure and over two bound variables with `alloc_arr_row_int_3(n)` (6: satisfiable, and it fixes the values); a vacuity twin for rows (c), (d), (g), (i), (k), (l) and (n) (11: every `assert false` reached reports, none is vacuous). 24 passed at the first run; one vacuity twin was split because Silicon stops a path at its first failing assert | evidence completed (audit round 1, issue 6) | 6, 7 |
| Step 0 R1–R3 | R1: every pin of the plan's section 10 has the outcome it lists (30 Flips D rows unchanged; no extra or absent row); one pin contradicts the plan's scope (`array_reject_pointer_to_array.c` is a parameter: question 320). R2: 174 probes, 340 runs: nothing of item 1's "In" list verifies before this record; C's `{0}` over a nested structure that begins with an array is rejected on the base in both modes (the suspected defect, confirmed); 12 staging pins (38 rows) added. R3: 2,569 rejection runs recorded (2,593 with the pins) | the plan's rule; question 320 to the owner | 13, 16 |
| P84 (a) the lone zero's spelling (the continuation's Step 0; question 325) | In an initializer where `{0}` elides braces the leaf is a `ck_integer` constant under compiler-generated casts to the member's type. A literal zero's innermost constant keeps no `expr` and carries `is_simple_zero` (`0`) or `non_arithmetic` (`0x0`, `00`, `0b0`; the suffixed zeros under a generated cast); `is_simple_zero` alone is not the literal flag (`+0`, `(int)0`, a `const` variable and an enumerator carry it with a kept `expr` or a named-constant flag). Told by a field: `'\0'` (C23: no flag, no `expr`, no `orig_type`), an enumerator, `1 - 1`, `false` (by the inner constant's type), `-0`, `+0`, `(int)0`, `NULL`, `nullptr`. **Not told:** `(0)`, `((0))`, `(0u)`, a macro whose body is `(0)` (no field records a parenthesis; only positions differ). A macro that expands to `0` is the literal. The same classes in six positions (global and local, an array of structures, a matrix, a nested structure), in compound literals, and under every leaf type tried; the Mac's and the box's front ends agree | the middle branch ("it tells some"): the told spellings are rejected in C under item 16's predicate, the `(0)` family listed for M8 (question 339, the advisor) | 16 |
| P84 (b) typedef'd row parameters (the continuation's Step 0) | `typedef int row[3];` with `row *a`, `row a[]` and `row a[2]`: one adjusted `tk_pointer` over a `tk_typeref` (`row`) whose `typeref_type` is the row's `tk_array`; the `declared_type` tells the spelling (the pointer; a `tk_array` with no bound constant; a `tk_array` of 2); the row's bound is field for field that of `int (*a)[3]`, `int a[][3]` and `int a[2][3]` (a literal: no `expr` in C23, a generated `unsigned long` cast in C++20; `2 * 3` keeps `eok_multiply`; an enumerator its named constant); `using` aliases the same; a qualified row is a fresh `tk_array` of `const int` with no typeref; the Mac's and the box's front ends agree on every field | first branch: the typedef'd spellings are accepted in Part E, the reader peeling one `tk_typeref` | 13 |

## Consequences

- C programs that keep arrays of structures and arrays of arrays in automatic, global and `static` storage and in
  structures verify, with ranges over structures in contracts, element reads in contracts, loops that carry them
  per leaf path, and array-of-rows parameters; a structure with an array member can live on the heap, hold an
  `owning` or marked member, and be reached through a heap read in a contract.
- Every array of aggregates costs a quantified permission per leaf path and field in its allocation, scope exit,
  atoms and loops, and a depth-2 leaf a permission over two bound variables.
- Ranges on part of an array of aggregates, atoms on one element, `malloc(n * sizeof(struct S))` and a second
  index under a binder wait for M4.5e2; element structures with `owning` or marked members for M4.8; pointers to
  arrays outside parameters and depth 4 or more for M8.
- Programs accepted before this record keep their Viper and maps byte for byte; the forms item 16 lists (in
  C++, and for nested lists in C) become rejections, and no existing test or doc example holds one.
- **Programs that verified with a `restrict` parameter are rejected until the keyword is dropped** (item 19;
  the continuation's Part R): 8 tests, which become rejection pins with their 9 error twins retired, and no doc
  example. Wrong programs that `restrict` let verify (on `main` every form the first session's audits found; on
  that session's branch one) no longer verify.
- **The address tests of item 5 on arrays of scalars are rejected** (question 322; Part R): a truth test or a
  null comparison of `&a[i]`, `&p[i]`, `&s.a[i]` or `&p->a[i]`; no existing test or doc example holds one, by
  the first session's three passes.
- **A session that closes at a split point** accepts this record for the items built; the others stay "decided,
  built by the continuation".
