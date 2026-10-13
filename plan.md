# plan.md — C/C++ verifier: EDG front end → Viper (Silicon) → Z3

Working name in this plan: **`cerastes`** (we used verifier and vcpp as a placeholder which you need to replace everywhere it is used)

One Claude Code session per milestone. Each session starts from `CLAUDE.md`, this file (its own milestone only), `docs/status.md`, and the decision records, and ends by leaving the repo green and `docs/status.md` updated so the next session can start cold.

---

## 0. Architecture

```
  C/C++ source + [[cerastes::...]] attributes + <cerastes.h>
        │
        ▼
  EDG front end
    --parse_templates, prototype IL kept, unrecognized/cerastes attributes kept
        │  in-process back end (IL walk)
        ▼
  VIR  — small internal verification IR (front-end agnostic)
        │
        ▼
  Viper text (.vpr) + position map (.vpr pos → C++ file:line)
        │
        ▼
  Silicon (symbolic execution) → Z3
        │
        ▼
  Errors mapped back to C++ source
```

VIR exists so the encoder does not depend on EDG's IL directly. If Gate G1 fails, a Clang dependent-AST front end can feed the same VIR.

### Repository layout

```
Cerastes/
  CLAUDE.md                 # always-loaded rules (template in Appendix A)
  plan.md                   # this file
  docs/
    status.md               # session handoff log (updated every session)
    decisions/NNNN-*.md     # one record per non-obvious decision
    il-inventory.md         # M1 output
    annotations.md          # user-facing annotation reference (M2+)
    encoding.md             # C/C++ → Viper encoding spec (grows each milestone)
    differential.md         # agreement table vs VeriFast (M4+)
    soundness/              # arguments the encoder relies on
  third_party/
    edg/                    # submodule: edgcpp/compiler, pinned to head commit
    silicon/                # submodule: viperproject/silicon (recursive), pinned
  include/cerastes.h        # spec functions + VF_* macros
  src/
    backend/                # EDG back end: IL walk → VIR
    vir/                    # VIR definitions + checks
    viper/                  # VIR → Viper text + position map
    driver/                 # CLI: run EDG, emit .vpr, call Silicon, map errors
  tests/
    viper-smoke/            # hand-written .vpr fixtures (M0)
    annotations/  scalar/  templates/  concepts/  memory/
    differential/verifast/  # original //@ versions from the VeriFast PRs
  scripts/
    bootstrap.sh  check.sh  run_tests.py  mutate.py  lint_viper.py
  .claude/agents/           # subagents (Appendix B)
```

### Test format (used from M0 on)

Every test file starts with expectation lines that `scripts/run_tests.py` checks:

```cpp
// expect: verified
// expect: error postcondition @ L14
// expect: rejected unsupported-union
// mode: c++20            (or c23)
```

**Must-fail twin rule:** every must-verify test that exercises an encoding rule has at least one twin that must fail for the right reason, at the right line.

---

## 1. Global invariants (copied into CLAUDE.md)

1. **Never edit an `expect:` line to make a test pass.** If a test looks wrong, stop and ask.
2. **Every encoding rule lands with at least one must-verify and one must-fail test.**
3. **Unsupported constructs are rejected with a diagnostic, never approximated.**
4. **No `inhale`/`assume` in emitted Viper outside tagged contexts.** Every one carries `// cerastes:inhale-ok <reason>`, and `scripts/lint_viper.py` fails the build otherwise. These are the points where an encoder becomes unsound.
5. **`third_party/edg` only ever pins an upstream `edgcpp/compiler` commit,** so anyone can build the tool without a fork. EDG changes are developed as small commits, each with a test, on a `cerastes` branch of a fork, and land upstream before the pin moves. Keep the CIR work on its own branch.
6. **Every non-obvious semantic choice gets a decision record** (integer conversions, parameter meaning in postconditions, memory model, …).
7. **User-facing errors point at C++ source,** never at `.vpr` lines.
8. **`scripts/check.sh` must be green at the end of every session.**

---

## 2. Before M0 (not a Claude Code session)

- [x] Choose a license. Apache-2.0 WITH LLVM-exception matches EDG
- [x] Decide how VeriFast results are obtained for differential tests:
  - **(a)** build `mborland/verifast@generic2` locally, or if that does not work
  - **(b)** record its outcomes on the six template tests once, as static expectations.

---

## M0 — Bootstrap and toolchain

**Goal:** a reproducible toolchain, the repo skeleton and a working test harness, with no verification logic yet.

**Tasks**
- Initialize the repo and add the submodules: EDG (upstream `edgcpp/compiler`, pinned to its head commit), and Silicon cloned recursively, since it carries `silver` as a submodule.
- Pin every tool version and record the SHAs and `--version` outputs in `docs/decisions/0001-toolchain-pins.md`.
- Write `scripts/bootstrap.sh` so it is idempotent on macOS:
  - Homebrew: `cmake ninja python openjdk sbt z3`.
  - Build EDG with the stock `macos-arm-clang-debug` preset.
  - Build Silicon with `sbt assembly` to get `target/scala-*/silicon.jar`.
  - Export `Z3_EXE`.
- Write `scripts/run_tests.py`:
  - Parses the `expect:` headers.
  - Runs `.vpr` fixtures directly through Silicon for now.
  - Exits non-zero on any mismatch, including "failed for the wrong reason or at the wrong line".
- Add `tests/viper-smoke/ok.vpr` (must verify) and `fail.vpr` (must fail with a known error at a known line).
- Write `scripts/check.sh` to run build, lint and tests.
- Write `CLAUDE.md` (Appendix A), `.claude/agents/` (Appendix B) and `docs/status.md`.

**Acceptance**
- On a clean clone, `scripts/bootstrap.sh && scripts/check.sh` is green.
- `fail.vpr` is reported with the expected error kind and line.
- The EDG front-end binary processes a hello-world `.cpp` without errors.

**Out of scope:** any EDG modification and any encoder code.

---

## M1 — EDG IL inventory and integration spike · **Gate G1**

**Goal:** prove EDG's prototype IL carries what the generic proof needs, and fix how the back end hooks in.

**Tasks**
- Add a `cerastes` EDG configuration (a defines block or a preset) that sets:
  - `PROTOTYPE_INSTANTIATIONS_IN_IL 1` (implies `ALL_TEMPLATE_INFO_IN_IL`)
  - `RECORD_UNRECOGNIZED_ATTRIBUTES TRUE`
  - `EXTRA_SOURCE_POSITIONS_IN_IL TRUE`
  - `GENERATE_SOURCE_SEQUENCE_LISTS`, only if needed.
- Copy the six tests from `mborland/verifast@generic2:tests/cxx/templates/` into `tests/differential/verifast/` unchanged: `generic_body`, `integral`, `integral_fallback`, `integral_per_instantiation`, `integral_wrong_spec`, `function_template_general_contract`. Make annotation-free copies for IL inspection.
- Run `--parse_templates --il_display` on each. Write `docs/il-inventory.md` as a table mapping every Clang query used by `GenericFunctionChecker`, `collectIntegralTypeParams` and the call-site code to its EDG IL field, with `il_display` excerpts as evidence.
- **Required checks** (each one is pass or fail, with evidence):
  1. **`inc`:** what `type_kind` the `v + 1` operation has (`tk_template_param` vs `tk_unknown`), and whether the `(T)` cast is its own node.
  2. **Never-instantiated templates:** a template with no instantiation still has a prototype routine with a full body.
  3. **Instance routines:** they carry `assoc_template` and `template_arg_list`, with deduced arguments.
  4. **`std::integral` is reachable** from all four constraint locations: type-constraint, requires-clause, trailing requires, abbreviated `auto`.
  5. **Unrecognized attributes survive:** an unrecognized `[[cerastes::x(...)]]` appears in the IL with namespace, name, raw tokens and positions on:
     - a function declaration,
     - a loop statement,
     - a null statement,
     - each of those inside a template's prototype instantiation.
- **Integration decision** (`0002-backend-integration.md`): an in-process EDG back end, modeled on how your CIR generator hooks in, versus writing an IL file and reading it with `il_read`. Default: in-process.
- Add a minimal back end that walks the IL and prints each routine (normal and prototype) with its parameter types. Add a golden-file test for it.

**Gate G1**
- **Checks 1–5 all pass:** proceed to M2.
- **Check 1 or 2 fails** (no usable generic body): stop. Write `0003-frontend-fallback.md` evaluating a Clang dependent-AST front end feeding the same VIR, and discuss before continuing.
- **Check 5 fails:** proceed. M2 registers the attributes explicitly instead of relying on the unrecognized path.

**Acceptance:** `il-inventory.md` is complete with evidence for every row, and the walker golden test passes in `check.sh`.

---

## M2 — Annotation layer

**Goal:** EDG parses and type-checks `[[cerastes::...]]` contracts as C++ expressions, and `<cerastes.h>` makes the same source build cleanly with ordinary compilers.

**v0 attribute set**

| Attribute | Where | Argument | Notes |
|---|---|---|---|
| `requires(p)` | after parameter list | `Xb` | parameters in scope; deferred check like Clang-mode `enable_if` |
| `ensures(p)` / `ensures(r, p)` | after parameter list | binder + `Xb` | arity decides; reject a binder that shadows a parameter; parameters denote **entry** values (decision record) |
| `invariant(p)` | loop statement | `Xb` | |
| `check(p)` | null statement | `Xb` | **not** `assert`, which `<cassert>` macro-expands |
| `fold(e)` / `unfold(e)` | null statement | `X` | predicate instance |
| `predicate` / `pure` | function declaration | none | marks spec functions written as ordinary C++ |

There is deliberately no user-facing `assume`.

**`<cerastes.h>`**
- Spec functions in `namespace cerastes`: `owns`, `reads`, `old`, `forall` (lambda-bound), `owns_range`.
- Decision: are spec functions declared-only or trivially defined? Either way, `predicate`/`pure` functions must never be emitted or called at runtime.
- `VF_REQUIRES(...)`, `VF_ENSURES(...)`, etc. expand to the attribute only when the driver defines `__CERASTES__`, and to nothing otherwise.

**Tasks**
- Register the attributes in `attribute.c`'s descriptor table under the `cerastes` namespace, one commit per attribute on the `cerastes` branch. Write application functions for binder injection and deferred parsing.
- Diagnostics:
  - binder shadows a parameter,
  - wrong placement,
  - ill-typed predicate,
  - `cerastes::` attribute on an unsupported entity.
- Write `docs/annotations.md`: the user-facing reference, with examples.

**Acceptance**
- `tests/annotations/` positive cases: `il_display` golden output shows each attribute attached to the right entity with a typed expression tree, and inside templates the expressions are dependent.
- Negative cases produce the specific EDG diagnostic.
- The same files compile with Apple clang (and GCC if installed) using `-std=c++20 -Wall -Wextra -Werror`, without `__CERASTES__`, and with zero warnings.

**Out of scope:** verification.

---

## M3 — Scalar encoder and driver (non-template code)

**Goal:** end-to-end verification of non-template functions over integer types and `bool`, in both C++20 and C23 modes.

**Scope**
- Locals; integer arithmetic with per-type range checks; integer promotions and usual arithmetic conversions for builtin types; division and remainder by zero; shift-amount checks.
- Comparisons and logical operators.
- `if`/`else`, `while`/`for` with `invariant`.
- `return`.
- Calls to annotated functions, verified modularly against their contracts.
- `check`.
- Decision records:
  - Signed overflow is a verification error.
  - Unsigned arithmetic is modular.
  - Narrowing conversions to signed types: error by default, or modular per C++20? Record which.

**Tasks**
- Design VIR (`src/vir/`) and document it in `encoding.md`.
- Write the Viper emitter with a position map; every emitted assertion carries its source position.
- Driver: `cerastes verify file.cpp [-I…] [--std=c++20|c23] [--keep-vpr]`. It maps Silicon's errors to `file:line: error: <category>: <source-level message>`.
- `scripts/lint_viper.py` enforces invariant 4.
- `scripts/mutate.py`: flip relational operators, ±1 constants, drop a conjunct, swap `&&`/`||`, delete a statement. Each mutant must fail verification or appear in a reviewed `tests/mutants-equivalent.txt`.

**Acceptance**
- `tests/scalar/` has at least 30 must-verify/must-fail pairs, run in both modes where the code is valid C.
- Mutation run: zero unexplained surviving mutants.
- Lint clean.
- Errors report C++ lines for every failure category.

---

## M4 — Generic templates I: reproduce #1034/#1041 · **Gate G2**

**Goal:** parity with the VeriFast PRs. A template is verified once, and the result is used at every eligible call site.

**Tasks**
- **Eligibility checker on prototype IL**, mirroring `GenericFunctionChecker`'s rules. It fails closed: `tk_unknown` operations, dependent calls, member access, dependent casts other than to `T`, non-type/template-template/variadic parameters, and member templates all fall back.
- **Contracts are checked too.** They're now visible as dependent IL, so the eligibility decision is made up front, not by try-then-fallback.
- **Opaque `T`:** a Viper domain type; values of `T` are domain values.
- **Integral `T`:**
  - `T_MIN`/`T_MAX` with `127 <= T_MAX <= 2^128-1` and `-T_MAX-1 <= T_MIN <= 0`.
  - `promoted(T)`, with `T_MAX <= promoted_T_MAX`, `INT_MAX <= promoted_T_MAX` and `promoted_T_MIN <= T_MIN`.
  - Literals: `0..127` as `T`, `0..32767` as `promoted(T)`.
  - Bitwise operators rejected; `bool` excluded. Keep the axioms linear.
- **Call sites:** instance routine → `assoc_template`. Use the generic proof when every integral argument is a builtin integer type other than `bool`; otherwise verify the instance with the M3 encoder.
- **Fallback:** if the generic proof fails, verify per instantiation for the types actually instantiated, and report which templates fell back.
- Write `docs/soundness/prototype-correspondence.md`: for every construct the checker accepts, the prototype parse and every real instantiation's parse agree. This is the lemma Clang gave for free.

**Gate G2:** the six tests, ported to `[[cerastes::]]` syntax, plus their mutants reproduce VeriFast's outcomes, recorded in `docs/differential.md`. That includes `integral_fallback`, where the proof holds for `int` but not `signed char` and only `int` is instantiated. Any disagreement is either explained in a decision record or blocks M5.

---

## M5 — Concept-requirement contracts and per-instantiation obligations (thesis core)

**Goal:** contracts attach to concept requirements; a generic body is verified once against those contracts only; each instantiation discharges an obligation that the overload actually selected satisfies the requirement's contract.

**Step 1 — syntax decision** (`0005-requirement-contracts.md`). Spike each option against EDG before choosing:
- **(a)** Attributes on compound requirements inside the requires-expression, if EDG's grammar accepts and records them.
- **(b)** Requirement-spec declarations: `[[cerastes::requirement_of(Addable)]] T add_spec(T a, T b) [[cerastes::ensures(r, r == a + b)]];`
- **(c)** One attribute on the concept declaration listing requirement contracts.

Criteria, in order: readable to C++ programmers; unambiguous matching from requirement to body expression; no grammar extensions to EDG beyond attribute registration.

**Encoding**
- Each requirement becomes a bodyless Viper method carrying its contract.
- Body expressions that correspond to a requirement become calls to that method. Matching a dependent body expression to its requirement is a core research problem. Write the matching rule into `encoding.md` and test it adversarially.
- **Obligation per instantiation:**
  - Read the selected overload from the instance IL: a builtin operation, a free function, an ADL-found function, or a hidden friend.
  - Emit a Viper method checking `req.pre ⇒ ovl.pre` and `ovl.post ⇒ req.post`.
  - Builtin operations use M3 semantics.

**Tests (`tests/concepts/`)**
- A user type whose overload satisfies the requirement contract (must verify).
- An overload with a weaker postcondition (must fail at the instantiation site, reporting both locations).
- An overload whose selection depends on scope or ADL, where the obligation must be checked against the overload actually chosen.
- A hidden friend, overloads differing only in constraints, and a requirement satisfied by a builtin operator.

**Acceptance:** all pairs pass; the mutation run is clean; the error messages name the requirement, the selected overload and the instantiation site.

---

## M6 — Memory model

**Goal:** ownership and heap reasoning with `owns`/`reads`/`old` and predicates written as C++ functions.

**Scope**
- One Viper reference per addressable cell, with a typed value field (decision record).
- Structs.
- Address-taken locals moved to the heap in the encoding.
- `new`/`delete`.
- References encoded as pointers.
- Predicates and pure functions from `[[cerastes::predicate]]`/`[[cerastes::pure]]` C++ functions, with `fold`/`unfold`.
- Arrays via a `loc(a, i)` domain plus quantified permissions, exposed as `owns_range`.
- `forall` lowered from lambdas.

**Rejected with diagnostics (for now):** unions, `reinterpret_cast`, character-type aliasing, pointer arithmetic outside arrays, virtual dispatch, exceptions, concurrency.

**Self-framing diagnostics** use source terms, e.g. "reads `c->val` without owning it".

**Acceptance:** pass/fail pairs for swap, struct field update, linked list (push/pop/length with a recursive predicate), array sum/max with a quantifier, and an aliasing trap that must fail. Run differential checks against VeriFast where the fragments overlap.

---

## M7 - C and C++ Inventory

**Goal:** create a comprehensive inventory by C and C++ language standards of features that need to be added in future work.

- **Comparison:** We must be at least at feature parity with Verifast on both C and C++
- **Metrics:**
  - Using the ISO standard drafts and CppReference inventory the language features and then annotate which we support and which we do not. Store this in an MD file for future milestones
  - Assess the relative difficulty of unimplemented features. E.g. which require compiler changes, which require viper changes, or which we can handle locally in future sessions

**Acceptance:** complete records of features that are and are not implemented with annotated difficulty sorted by language standard.

## M8 — Evaluation and dissertation artifacts

**Goal:** numbers and reproducible artifacts for the prelim.

- **Corpus:** generic integer code from real libraries. Candidates are your own Boost libraries' templates; pick functions, don't claim coverage.
- **Metrics:**
  - templates verified once vs per instantiation,
  - instantiations covered per generic proof,
  - verification time,
  - annotation overhead (spec lines / code lines),
  - mutation score,
  - agreement with VeriFast.
- **Artifacts:** `scripts/eval.sh` regenerates every table from a clean clone; results go in `docs/eval/`.
- **Out of scope for Claude Code:** usability-study design and IRB materials.

---

## Gates and fallbacks

| Gate | Where | Pass | Fail |
|---|---|---|---|
| G1 | end of M1 | prototype IL has generic bodies, instance links, constraints, attributes | Clang dependent-AST front end into the same VIR (decision 0003) |
| G2 | end of M4 | parity with #1034/#1041 on the six tests + mutants | fix or explain before M5; never weaken expectations |

**Non-goals for this plan:** CIR (stays on its own branch; possible later use for instantiation-side checks), further VeriFast upstream work (the PRs stand as the pilot result), and concurrency.

---

## Session protocol

**Opening prompt** (paste at the start of each milestone session):

```
Read CLAUDE.md, the "M<k>" section of plan.md, docs/status.md, and all of
docs/decisions/. Use plan mode: propose a task breakdown for M<k>, list the
exact acceptance commands you will run, and name any decision records you
expect to write. Do not modify files until I approve the plan.
```

**During the session**
- Delegate must-fail twins and mutants to the `test-mutator` subagent.
- Before any commit that touches `src/viper/` or `src/backend/`, run the `soundness-auditor` subagent on the diff.
- If a milestone runs long, split it with a handoff entry rather than carrying a huge context forward.

**Closing prompt:**

```
Run scripts/check.sh. Then update docs/status.md with: what is done, what is
not, surprises, any decision records added, and the exact first step for the
next session. Commit with a message referencing M<k>.
```

---

## Appendix A — CLAUDE.md template (keep under ~150 lines)

```markdown
# cerastes — C/C++ verifier (EDG → Viper/Silicon → Z3)

Plan: @plan.md   Status: @docs/status.md   Annotations: @docs/annotations.md

## Commands
- Bootstrap: scripts/bootstrap.sh
- Everything: scripts/check.sh   (must be green before ending a session)
- Tests only: python3 scripts/run_tests.py [dir]
- Mutants: python3 scripts/mutate.py <dir>
- Verify one file: build/cerastes verify <file> --keep-vpr

## Invariants (non-negotiable)
1. Never edit an `expect:` line to make a test pass; stop and ask instead.
2. Every encoding rule: ≥1 must-verify and ≥1 must-fail test.
3. Unsupported constructs are rejected with a diagnostic, never approximated.
4. No inhale/assume in emitted Viper without `// cerastes:inhale-ok <reason>`.
5. third_party/edg pins upstream edgcpp/compiler only; EDG edits are small commits with tests on a `cerastes` fork branch, upstreamed before the pin moves.
6. Non-obvious semantics → docs/decisions/NNNN-*.md before code.
7. Errors shown to users point at C++ source lines.

## Layout
src/backend (EDG IL → VIR) · src/vir · src/viper (VIR → .vpr + pos map) ·
src/driver · include/cerastes.h · tests/<area>/ · docs/

## Conventions
- C++20 for tool code; EDG code follows EDG's own style in third_party/edg.
- Test files start with `// expect:` lines (see plan.md §0).
- Annotation names: never `assert` (macro collision); use `check`.
```

## Appendix B — Subagents (`.claude/agents/`)

`.claude/agents/soundness-auditor.md`

```markdown
---
name: soundness-auditor
description: Reviews diffs to src/backend, src/vir, or src/viper for unsoundness before commit. Use proactively on any encoder change.
tools: Read, Grep, Glob, Bash
---
You audit a C/C++ → Viper encoder for soundness. For the given diff, report:
- any inhale/assume/axiom added without a `cerastes:inhale-ok` tag and justification;
- C++ operations encoded without the checks C++ semantics require
  (overflow, division by zero, shift range, conversions, permissions);
- constructs silently accepted that should be rejected;
- encoding rules added without a must-fail test.
Output a numbered issue list with file:line and severity. Do not edit files.
```

`.claude/agents/test-mutator.md`

```markdown
---
name: test-mutator
description: Writes must-fail twins and mutation tests for new verifier test cases.
tools: Read, Grep, Glob, Write, Bash
---
Given must-verify tests, write twins that must fail for one specific reason each
(weakened precondition, strengthened postcondition, off-by-one, dropped
invariant conjunct, overflow at the boundary, wrong overload selected).
Each twin's `// expect: error <category> @ L<line>` must name the exact failing
line. Never modify existing tests' expect lines.
```
