# plan.md — C/C++ verifier: EDG front end → Viper (Silicon) → Z3

Working name in this plan: **`cerastes`** (we used verifier and vcpp as a placeholder which you need to replace everywhere it is used)

One Claude Code session per milestone, or per sub-milestone when a milestone does not fit in a few hours (≤ 4 h; owner, 2026-10-20): M4.1, M4.5a, M5a, …. Each session starts from `CLAUDE.md`, this file (its own milestone only), `internal/status.md`, and the decision records, and ends by leaving the repo green and `internal/status.md` updated so the next session can start cold.

**Milestone renumbering (owner, 2026-10-20).** Gate G2's VeriFast tests need pointers, references and classes,
which M3's scalar fragment cannot express, so the rest of C comes first and the later milestones move by one:

| Before 2026-10-20 | Now |
|---|---|
| — | **M4** The rest of C (new; sub-milestones M4.1–M4.8, pointers in M4.5a–f) |
| M4 Generic templates I · Gate G2 | **M5** (sessions M5a–M5g) |
| M5 Concept-requirement contracts | **M6** |
| M6 Memory model | its C parts → **M4.5**; its C++ parts → **M7** C++ memory and classes |
| M7 C and C++ inventory | **M8** |
| M8 Evaluation | **M9** |

Records, the handoff and code comments use the new numbers (updated 2026-10-20; each record notes it). The history
entries of `internal/status.md` keep the numbers they were written with.

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
  doc/                      # user documentation: Antora component, published at https://cerastes.dev/cerastes/doc/ (M1.5+)
    antora.yml  local-playbook.yml  package.json  ui/
    modules/ROOT/           # nav.adoc, pages/, examples/ (every example is a test), images/
  internal/                     # developer documentation (not published)
    status.md               # session handoff log (updated every session)
    decisions/NNNN-*.md     # one record per non-obvious decision
    il-inventory.md         # M1 output
    encoding.md             # C/C++ → Viper encoding spec (grows each milestone)
    differential.md         # agreement table vs VeriFast (M5+)
    soundness/              # arguments the encoder relies on
  third_party/
    edg/                    # submodule: edgcpp/compiler, pinned to head commit
    silicon/                # submodule: viperproject/silicon (recursive), pinned
  include/cerastes.h        # spec functions + CERASTES_* macros
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
    bootstrap.sh  check.sh  run_tests.py  mutate.py  lint_viper.py  build_docs.sh
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
5. **`third_party/edg` pins the owner's fork** (`mborland/compiler`, branch `cerastes`: upstream `main` plus the cerastes series; decision 0009, owner 2026-10-14, superseding "upstream only"). EDG changes are developed as small commits, each with a test, kept upstreamable, and offered upstream by the owner. Keep the CIR work on its own branch.
6. **Every non-obvious semantic choice gets a decision record** (integer conversions, parameter meaning in postconditions, memory model, …).
7. **User-facing errors point at C++ source,** never at `.vpr` lines.
8. **`scripts/check.sh` must be green at the end of every session.**
9. **User-visible behavior is documented in `doc/` in the milestone that adds it** (annotations, CLI options, diagnostics, supported and rejected constructs), and every code example in `doc/` is a test with `expect:` lines that `check.sh` runs.

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
- Pin every tool version and record the SHAs and `--version` outputs in `internal/decisions/0001-toolchain-pins.md`.
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
- Write `CLAUDE.md` (Appendix A), `.claude/agents/` (Appendix B) and `internal/status.md`.

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
- Run `--parse_templates --il_display` on each. Write `internal/il-inventory.md` as a table mapping every Clang query used by `GenericFunctionChecker`, `collectIntegralTypeParams` and the call-site code to its EDG IL field, with `il_display` excerpts as evidence.
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

## M1.5 — User documentation site (Antora)

**Goal:** user documentation written in AsciiDoc in this repo, built with Antora, checked by `check.sh` and published at https://cerastes.dev/cerastes/doc/. From here on, every milestone documents what it makes user-visible (invariant 9). This milestone depends only on M0. Run it before M2, which writes the first reference page.

**Hosting** (decision record `0004-docs-site.md`)

> **Superseded by decision 0004 (owner, 2026-10-13):** there is no docs-only repo. The docs are this repository's own GitHub Pages site, deployed by its Actions workflow, at https://cerastes.dev/cerastes/doc/ (the org convention is `cerastes.dev/<repo>/doc/`). The text below is the original plan; its other mentions of the address have been updated.

- **Default:** a dedicated org repo `cerastes-verifier/doc` with GitHub Pages (source: GitHub Actions). The org site `cerastes-verifier.github.io` owns the custom domain `cerastes.dev`, and GitHub serves an org's project sites under that domain. A repo named `doc` therefore publishes at `https://cerastes.dev/doc/` with no proxy rules and no change to the website's deployment.
- That repo holds only the publishing playbook and workflow:
  - The playbook pulls content from this repo (`url: https://github.com/cerastes-verifier/cerastes`, `start_path: doc`) and sets `site.url: https://cerastes.dev/doc`.
  - The workflow runs on `repository_dispatch` (sent by this repo's CI when a push to `main` touches `doc/`), on `workflow_dispatch`, and nightly as a backstop.
- **Alternative to weigh in the record:** the website's own workflow builds Antora into `dist/doc/`. That is one deployment, but every docs change would redeploy the website, and the two repos' build tooling would be coupled.
- **Owner steps** (outward-facing, not done by Claude): create the repo, enable Pages, add the dispatch token as a secret in this repo, and add a "Documentation" link on cerastes.dev.

**Tasks**
- Create the Antora component in `doc/`, in the same shape as the owner's other Antora projects (for example Boost.Int128's `doc/`):
  - `antora.yml` with `name: ROOT` and `version: ~` until the first tagged release;
  - `modules/ROOT/{nav.adoc,pages/,examples/,images/}`;
  - `package.json` with exact pins for Antora 3.x and `@antora/lunr-extension`, plus a committed `package-lock.json`;
  - `local-playbook.yml`, which builds the working tree into `build/doc/`.
- Give the site a UI that matches cerastes.dev. Use the default Antora UI plus supplemental UI files in `doc/ui/` (logo, Ink/Sand palette, Archivo, favicon), taken from the brand kit and the website's `src/assets/`. Do not use the Boost UI bundle. Pin the UI bundle with a versioned URL or a vendored zip with a checksum, never a moving `latest`.
- Write the first pages, describing only what exists:
  - Overview: what cerastes is, the pipeline, and current status.
  - Installation: building from source with `scripts/bootstrap.sh` on macOS with Apple silicon.
  - No placeholder pages for future features.
- **Examples are tests.**
  - Files under `doc/modules/ROOT/examples/` carry `expect:` headers.
  - Pages include them through tagged regions, leaving the header out.
  - `run_tests.py` discovers them by default alongside `tests/`.
- Add `scripts/build_docs.sh`, which runs the local playbook with `--log-failure-level=warn` so that broken xrefs, includes or attributes fail the build.
  - `check.sh` gains a docs step.
  - `bootstrap.sh` installs Node (Homebrew) and runs `npm ci` in `doc/`.
  - Record the Node and Antora pins in 0001.
- Add a CI workflow in this repo that builds the docs on pull requests and, on pushes to `main` that touch `doc/`, sends the `repository_dispatch` to the doc repo.
- `internal/` stays developer-only and unpublished. The user-facing annotation reference goes in `doc/` (written in M2).

**Acceptance**
- `scripts/check.sh` builds the docs with zero Antora warnings and runs every doc example as a test.
- `curl -sI https://cerastes.dev/cerastes/doc/` returns 200, search works, and the site matches cerastes.dev's branding (screenshot referenced in the status entry).
- A docs-only commit to `main` appears at https://cerastes.dev/cerastes/doc/ with no manual step.

**Out of scope:**
- pages for unimplemented features;
- versioned docs (added at the first tagged release);
- API docs for the tool's internal code.

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
- `CERASTES_REQUIRES(...)`, `CERASTES_ENSURES(...)`, etc. expand to the attribute only when the driver defines `__CERASTES__`, and to nothing otherwise. (Named `VF_*` until M2; the owner chose `CERASTES_*` on 2026-10-14, decision 0008.)

**Tasks**
- Register the attributes in `attribute.c`'s descriptor table under the `cerastes` namespace, one commit per attribute on the `cerastes` branch. Write application functions for binder injection and deferred parsing.
  > **Superseded by decision 0007 (2026-10-14):** EDG gets a general registration mechanism (no `cerastes` string in EDG; 0003 item 7), developed as a local commit series on the fork's `cerastes` branch, one commit per phase. cerastes registers its table from its own driver. Since decision 0009 (same day) the submodule pins the fork, so M2 landed on `main` (the topic branch `m2-annotations` was dropped).
- Diagnostics:
  - binder shadows a parameter,
  - wrong placement,
  - ill-typed predicate,
  - `cerastes::` attribute on an unsupported entity.
- Write the user-facing annotation reference as `doc/modules/ROOT/pages/annotations.adoc`, plus a page on building annotated code with ordinary compilers through `<cerastes.h>`. Every example is an included test (see M1.5).

**Acceptance**
- `tests/annotations/` positive cases: `il_display` golden output shows each attribute attached to the right entity with a typed expression tree, and inside templates the expressions are dependent. *(As built: the goldens are the walker's `--cerastes_walk` listing, which prints each attribute with its typed tree; il_display is the evidence each golden is checked against.)*
- Negative cases produce the specific EDG diagnostic.
- The same files compile with Apple clang (and GCC if installed) using `-std=c++20 -Wall -Wextra -Werror`, without `__CERASTES__`, and with zero warnings.
- The annotation reference is published at https://cerastes.dev/cerastes/doc/, and each of its examples runs in `check.sh`.

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
- Termination (owner, 2026-10-16): `decreases` measures on loops and recursive functions; results are total correctness (decision 0013).
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
- User docs:
  - "Verifying a program": `cerastes verify`, its options, exit codes and `--keep-vpr`.
  - "Error messages": one section per category, each with a must-fail example, what the error means and how to fix it.
  - "Integer semantics": overflow, modular unsigned arithmetic, narrowing, division and shifts, summarizing the decision records in user terms.
  - Installation gains a walkthrough of a first verification.

**Acceptance**
- `tests/scalar/` has at least 30 must-verify/must-fail pairs, run in both modes where the code is valid C.
- Mutation run: zero unexplained surviving mutants.
- Lint clean.
- Errors report C++ lines for every failure category.
- Every error category the driver can emit has a section with a must-fail example on the error-message page.

---

## Between M4.2a and M4.2b: three sessions (owner, 2026-10-21)

Owner, 2026-10-16 and 2026-10-20: CI for the repository is its own session. It was planned after M4.1 and before
the mutation triage session; the owner ran M4.2a first (2026-10-21). On 2026-10-21 the owner split the CI session's
box work from CI itself: the sessions run **EDG upstream merge (S1) → CI (S2) → mutation triage (S3) → M4.2b**,
each planned at ≤ 4 h with at most three Sonnet agents and one read-only Opus auditor. Details, facts found while
planning and stop rules: `internal/handoff.md` §4.1, §4.2 and §5.

### EDG upstream merge session (S1, box)

The two items the M3 close-out left, both needing the x86-64 box: merge upstream EDG `main` (`dedc820c91` on
2026-10-21, 9 commits after `307002eae6`) into the fork's `cerastes` with the full box suites before the pin moves
(decision 0009), and explain why branch `0020` alone fails its own test on Linux (most likely a stale incremental
build on the box; confirmed by a clean-build run). The emitted Viper of every test must stay byte-identical.

### CI session (S2)

A decision record (0016), then `.github/workflows/ci.yml` running the full `scripts/check.sh` on GitHub's
`xcode-27` runner (macOS 27, Xcode 27.0, SDK 27.0: the owner's toolchain) on pull requests, pushes to `main` and
`workflow_dispatch`. The owner pushes the branch at checkpoints; the session watches the runs read-only. If the
expectations drift on the runner, the session stops and asks; x64 Docker (which needs glibc's `assert` shape in the
encoder) stays the recorded fallback.

### Mutation triage session (S3)

The M3 close-out's tightened safe-direction classifier moved 74 mutants of `tests/scalar` out of the bucket, and
M4.1 and M4.2a left 70 and 185 survivors: each survivor is killed by strengthening its own file's contract (in
place, owner 2026-10-21: never an edited expect line, a must-fail twin per new clause) or listed in
`tests/mutants-equivalent.txt` with a reason, reviewed by the Opus auditor, together with the close-out audit A's
should-fix items and `mutate.py` reporting as it goes. Until then M3's "0 unexplained survivors" holds for the old
bucket only.

---

## M4 — The rest of C

**Goal:** every C23 construct that VeriFast verifies in C is verified by cerastes, or deliberately deferred with a
decision record; non-template code, one translation unit. `.c` tests run in both `c23` and `c++20` where the code is
valid in both. Strictly C (owner, 2026-10-20): C++ references, `new`/`delete` and classes are M7.

**Sessions.** Each is planned at ≤ 4 h and lands its decision record (invariant 6), must-verify/must-fail pairs
(invariant 2) and its docs (invariant 9), with one Opus audit round per diff plus a re-audit of blocking fixes and
`check.sh` at most twice. `internal/handoff.md` holds each session's tasks, agents, acceptance and stop rules.
M4.2 was split into M4.2a–c (owner, 2026-10-21): its six constructs did not fit one session; M4.2b was split again
into M4.2b1 and M4.2b2 (owner, 2026-10-21, M4.2b planning). Between M4.2a and M4.2b1 run the EDG upstream merge, CI
and mutation triage sessions (above).

| Session | Scope | Needs |
|---|---|---|
| **M4.1 Control flow** | `break`, `continue`, `do … while` (`do … while (0)` needs no measure, owner), `switch` (case/default/fallthrough, `[[fallthrough]]`), `for(;;)` left by `break` | — |
| **M4.2a Jumps and ends** | forward `goto` and labels ("outward and forward": never backward, never into a block, branch, switch body or loop, never past a declaration); `_Noreturn`/`[[noreturn]]` (a new category `noreturn`) with trusted `abort`/`exit`/`_Exit`; `<assert.h>` `assert` checked like `check` | M4.1 |
| **M4.2b1 Named constants and case values** | `const`/`constexpr` globals (the initializer encoded, EDG's folding not trusted; checked in a prologue of every method, owner); `case k`/`case N` with case-value obligations checked path-independently; a `do … while` condition that names a variable is a loop | M4.2a |
| **M4.2b2 Enumerations** | enumeration types (a value range per type, every conversion into it checked) and enumerators (one meaning in both modes); `case RED`; enum-typed constants | M4.2b1 |
| **M4.2c Locals without an initializer** | a read of an indeterminate value is an error: the category `uninitialized` (P4222), checked on each path by a ghost flag (owner, 2026-10-22; record 0019); jumps past such a declaration; with handoff §11 questions 17–19 (EDG #2656 fixed on fork branch `0022`, pinned at `f0ed5525af`; ordering of one enumeration, `-Wswitch`'s rules) | M4.2a |
| **M4.3 Expressions and sequencing** | the comma operator, assignments as values, `x++` inside expressions, calls in guarded operands and loop conditions, unsequenced calls (C's sequencing rules) | — |
| **M4.4 Bitwise operators** | `& \| ^ ~` and `&= \|= ^=` on signed and unsigned types (C23 two's complement); a Silicon/Z3 spike first (bounded `Int` definitions vs SMT bit-vector interpretation) | — |
| **M4.5 Pointers** (sessions a–f, below) | the memory model for C | M4.1–M4.3 |
| **M4.6 Chars and strings** | string literals as read-only arrays, `char` buffers, `strlen`/`memcpy`/`memcmp`/`strcmp` contracts, `main(argc, argv)` | M4.5e |
| **M4.7 Function pointers** | contracts on function types, calls through pointers ("the pointer satisfies the type") | M4.5b |
| **M4.8 C parity close-out** | the VeriFast C differential set, the M4 mutation run, the Memory and ownership page complete, M4's acceptance, the deferred list for M8 | all |

**M4.5 Pointers** (one 4.x for pointers, owner, 2026-10-20; the C parts of the former memory-model milestone):
- **M4.5a Design** (no encoder code): the memory-model record (cells and typed value fields; a pointer as a block
  plus an offset, as VerCors does, or one reference per cell; NULL; provenance; address-taken locals), the **C spec
  syntax** record (`owns`, `reads`, `old`, `owns_range` and `forall` are C++ only today, decision 0008 item 6, and
  `forall` needs lambdas, which C lacks), the new error categories (permission, null, bounds, double free, …),
  Silicon spikes, and whether an EDG fork commit is needed (then a box session comes before M4.5b).
- **M4.5b Pointers to scalars and globals:** `&x` (address-taken locals as cells), `*p`, NULL, pointer parameters
  with `owns`/`reads`/`old(*p)`, pointer equality, the aliasing trap, mutable globals and `static` locals as cells
  with frames; ports of VeriFast's `swap.c`, `address_of_local.c`, `globals.c`.
- **M4.5c Structs:** field cells, `.` and `->`, struct values (copies, initializers, by-value parameters and
  results), nested structs, pointers to fields; ports of `account.c`, `deposit.c`.
- **M4.5d malloc and free:** trusted libc contracts (`malloc`, `calloc`, `realloc`, `free`; `abort`/`exit` came in
  M4.2a),
  allocation blocks, ownership on free, leaks (a record); ports of `malloc_block.c`, `cell.c`.
- **M4.5e Arrays and pointer arithmetic:** local arrays, `a[i]`, `p + i`, `p - q`, comparisons within one object,
  `owns_range` and quantified permissions, `forall` in C; ports of `carrays.c`, `insertion_sort.c`, sum and max.
- **M4.5f Predicates and pure functions:** predicate and pure bodies encoded, `fold`/`unfold`, recursive
  predicates; a linked list (push, pop, length), `stack.c`, `reverse`.

**Reference: VerCors** (owner, 2026-10-20). VerCors (University of Twente) verifies C through Viper, and its pointer
encoding is the reference for M4.5 (summary in `internal/handoff.md`). Ideas only: VerCors is MPL-2.0 and its
pointer prelude rests on axioms; cerastes copies no files, and any axiom it adopts gets its own decision record and
lint tag (invariant 4).

**Deferred beyond M4** (entries of M8's inventory, each with its reason): unions (effective types), floating point,
`volatile`/`_Atomic`/`_BitInt`/vector types, variadic functions and `printf`, several translation units (the contract
manifest, M6), concurrency.

**Docs:** each session extends "Verifying a program", "Annotations" and "Error messages" (one section per new error
category); from M4.5, a "Memory and ownership" page: `owns`/`reads`/`old` in C, structs, `malloc`/`free`, arrays and
`owns_range`, `forall`, predicates and pure functions, `fold`/`unfold`, and the rejected constructs.

**Acceptance (checked in M4.8):** pass/fail pairs for every construct, including swap, a struct field update, a
linked list (push/pop/length with a recursive predicate), array sum/max with a quantifier, and an aliasing trap that
must fail; the ported VeriFast C examples and their outcomes in `internal/differential.md`; the mutation run over the
M4 tests with zero unexplained survivors; lint clean; every error category documented with a must-fail example;
self-framing diagnostics in source terms (for example "reads `c->val` without owning it").

---

## M5 — Generic templates I: reproduce #1034/#1041 · **Gate G2** (was M4)

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
- Write `internal/soundness/prototype-correspondence.md`: for every construct the checker accepts, the prototype parse and every real instantiation's parse agree. This is the lemma Clang gave for free.
- **The `predicate`/`pure` marker on the first declaration** (owner, 2026-10-20, M3 handoff question 2): a small fork series commit giving EDG's attribute registration a way to refuse an accumulating attribute on a later declaration; cerastes registers it for `predicate` and `pure`.
- User docs: a "Generic templates" page covering:
  - when a template is verified once;
  - the eligibility rules in user terms;
  - what falls back to per-instantiation verification, and how that is reported;
  - the model of an integral `T`.

**Sessions** (owner, 2026-10-20; designed in the M4.1 session, details in `internal/handoff.md`). The critical path
is M5a → M5c → M5d → M5e → M5f → M5g; M5b needs the box and can run any time before M5g, best just before it.

| Session | Scope |
|---|---|
| **M5a Eligibility** | the fail-closed checker on prototype IL; a verdict per template (eligible; each parameter opaque or integral; the first offending construct); a `cerastes-fe` listing (`--cerastes_generic`) with golden tests (`// golden-options:`); verification results unchanged |
| **M5b EDG (box)** | fork commits: the first-declaration rule for `predicate`/`pure`; EDG diagnosing the error node series L leaves for a `decltype(p)` lambda parameter; the full box suites before the pin moves |
| **M5c Generic infrastructure and opaque `T`** | a generic Viper method per eligible template, verified even when never instantiated; an opaque `T` as an empty Viper domain (a tagged lint rule); VeriFast in Docker and the recorded outcomes of the six originals |
| **M5d Integral `T`** | the limits as ghost formal arguments with VeriFast's linear constraints as `requires` (no axiom, no assume); every symbolic operation checked as signed with symbolic bounds |
| **M5e Instances, call sites, fallback** | callers call each instance's method; one Silicon run holds the generic method and the instances; the driver decides per template and reports fallbacks as notes |
| **M5f Specializations, recursion, headers** | explicit specializations, the call graph keyed by instance, templates in user headers, `internal/soundness/prototype-correspondence.md` |
| **M5g Gate G2** | the ports with line maps, VeriFast on originals and mutants, `internal/differential.md` |

**Gate G2:** the six tests, ported to `[[cerastes::]]` syntax, plus their mutants reproduce VeriFast's outcomes, recorded in `internal/differential.md`. That includes `integral_fallback`, where the proof holds for `int` but not `signed char` and only `int` is instantiated. Any disagreement is either explained in a decision record or blocks M6. **Deferred to M7** (owner, 2026-10-20): `generic_body`'s `assign` and `same_ref` (references) and all of `function_template_general_contract` (constructors and destructors); they are ported and compared there. Pointers (`pick(1, p)`) and bitwise operators (`low_bit`) come from M4 and are in G2.

---

## M6 — Concept-requirement contracts and per-instantiation obligations (thesis core) (was M5)

**Goal:** contracts attach to concept requirements; a generic body is verified once against those contracts only; each instantiation discharges an obligation that the overload actually selected satisfies the requirement's contract.

**Step 1 — syntax decision** (`NNNN-requirement-contracts.md`, the next free decision number when M6 starts; 0005–0008 were taken by M2). Spike each option against EDG before choosing:
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

**Contract manifest** (owner, 2026-10-20, M3 handoff question 3): the manifest of the contracts a unit relies on (M2 handoff §6), which cross-unit termination needs (0013 item 5). Schedule it here; if it does not fit M6, move it to M7 and say why in the handoff.

**Tests (`tests/concepts/`)**
- A user type whose overload satisfies the requirement contract (must verify).
- An overload with a weaker postcondition (must fail at the instantiation site, reporting both locations).
- An overload whose selection depends on scope or ADL, where the obligation must be checked against the overload actually chosen.
- A hidden friend, overloads differing only in constraints, and a requirement satisfied by a builtin operator.

**Docs:** a "Concept requirement contracts" page with the syntax chosen in that record and how obligation failures are reported.

**Acceptance:** all pairs pass; the mutation run is clean; the error messages name the requirement, the selected overload and the instantiation site.

---

## M7 — C++ memory and classes (was M6, "Memory model"; its C parts moved to M4.5)

**Goal:** C++'s own memory constructs on top of M4.5's memory model: references, `new`/`delete` and classes.

**Scope**
- References encoded as pointers.
- `new`/`delete` (and `new[]`/`delete[]`).
- Classes: data members as M4.5c's struct cells, member functions, `this`, constructors and destructors with contracts (owner, 2026-10-20: C++ memory stays after the templates and concepts milestones).
- Predicates and pure functions written as C++ member functions; `forall` lowered from lambdas.
- This can take inspiration from VerCors, which implements these features for Viper.

**Rejected with diagnostics (for now):** `reinterpret_cast`, character-type aliasing, virtual dispatch, exceptions, concurrency.

**Gate G2's deferred parts** (owner, 2026-10-20): `generic_body`'s `assign` and `same_ref`, and `function_template_general_contract`, ported and compared with VeriFast, recorded in `internal/differential.md`.

**Docs:** the "Memory and ownership" page gains references, `new`/`delete`, classes and the C++ forms of the spec functions; the list of rejected constructs with their diagnostics.

**Acceptance:** pass/fail pairs for references, `new`/`delete`, a class with constructor and destructor contracts, and a member predicate; the deferred G2 tests agree with VeriFast or the disagreement is explained in a decision record.

---

## M8 — C and C++ Inventory (was M7)

**Goal:** create a comprehensive inventory by C and C++ language standards of features that need to be added in future work.

- **Comparison:** We must be at least at feature parity with Verifast on both C and C++
- **Metrics:**
  - Using the ISO standard drafts and CppReference inventory the language features and then annotate which we support and which we do not. Store this in an MD file for future milestones
  - Assess the relative difficulty of unimplemented features. E.g. which require compiler changes, which require viper changes, or which we can handle locally in future sessions
- **Known entries** (recorded as they are decided, so none is lost):
  - **Language modes** (owner, 2026-10-16, M2 handoff question 4). GNU emulation is refused today and will need to be added: it needs EDG work, since `extern "C"` redeclarations in two namespaces escape must-match there. C before C23 is refused by design (C23 is the supported C).
  - **Constructs M3 rejects** (decision 0012 lists them) and **what M4 defers** (M4's "Deferred beyond M4"): each one is an entry, with the milestone expected to lift it.
  - **C23 `constexpr` objects** (owner, 2026-10-22, M4.2b1 handoff question 15): they do not parse in this EDG configuration (P23: warning #77, errors #65/#20), so `case N:` with a C `constexpr` cannot be written; check upstream EDG's C23 status at the next upstream merge.
  - **Enumerations M4.2b2 leaves rejected** (decision 0018): enumerations from system headers, enumerations in blocks, classes and templates, a `bool` underlying type, enumerators outside `int`, ordering comparisons of unscoped enumeration values, compound assignment and `++`/`--` on enumerations, and in C an enumerator list that names an earlier enumerator whose initializer is not of type `int`.
- **User docs:** publish a "Language support" page in `doc/` from the inventory. It shows what is supported and unsupported, by standard. The difficulty assessment stays in the planning file.

**Acceptance:** complete records of features that are and are not implemented with annotated difficulty sorted by language standard.

## M9 — Evaluation and dissertation artifacts (was M8)

**Goal:** numbers and reproducible artifacts for the prelim.

- **Corpus:** generic integer code from real libraries. Candidates are your own Boost libraries' templates; pick functions, don't claim coverage.
- **Metrics:**
  - templates verified once vs per instantiation,
  - instantiations covered per generic proof,
  - verification time,
  - annotation overhead (spec lines / code lines),
  - mutation score,
  - agreement with VeriFast.
- **Artifacts:** `scripts/eval.sh` regenerates every table from a clean clone; results go in `internal/eval/`.
- **Out of scope for Claude Code:** usability-study design and IRB materials.

---

## Gates and fallbacks

| Gate | Where | Pass | Fail |
|---|---|---|---|
| G1 | end of M1 | prototype IL has generic bodies, instance links, constraints, attributes | Clang dependent-AST front end into the same VIR (decision 0003) |
| G2 | end of M5 (M5g) | parity with #1034/#1041 on the six tests + mutants (parts needing C++ memory deferred to M7) | fix or explain before M6; never weaken expectations |

**Non-goals for this plan:** CIR (stays on its own branch; possible later use for instantiation-side checks), further VeriFast upstream work (the PRs stand as the pilot result), and concurrency.

---

## Session protocol

**Opening prompt** (paste at the start of each milestone session):

```
Read CLAUDE.md, the "M<k>" section of plan.md, internal/status.md, and all of
internal/decisions/. Use plan mode: propose a task breakdown for M<k>, list the
exact acceptance commands you will run, and name any decision records you
expect to write. Do not modify files until I approve the plan.
```

**During the session**
- Delegate must-fail twins and mutants to the `test-mutator` subagent.
- Before any commit that touches `src/viper/` or `src/backend/`, run the `soundness-auditor` subagent on the diff.
- If a milestone runs long, split it with a handoff entry rather than carrying a huge context forward.

**Closing prompt:**

```
Run scripts/check.sh. Then update internal/status.md with: what is done, what is
not, surprises, any decision records added, and the exact first step for the
next session. Commit with a message referencing M<k>.
```

---

## Appendix A — CLAUDE.md template (keep under ~150 lines)

```markdown
# cerastes — C/C++ verifier (EDG → Viper/Silicon → Z3)

Plan: @plan.md   Status: @internal/status.md   User docs: doc/ (Antora → https://cerastes.dev/cerastes/doc/)

## Commands
- Bootstrap: scripts/bootstrap.sh
- Everything: scripts/check.sh   (must be green before ending a session)
- Tests only: python3 scripts/run_tests.py [dir]
- Mutants: python3 scripts/mutate.py <dir>
- Verify one file: build/cerastes verify <file> --keep-vpr
- User docs: scripts/build_docs.sh   (builds doc/ into build/doc; fails on warnings)

## Invariants (non-negotiable)
1. Never edit an `expect:` line to make a test pass; stop and ask instead.
2. Every encoding rule: ≥1 must-verify and ≥1 must-fail test.
3. Unsupported constructs are rejected with a diagnostic, never approximated.
4. No inhale/assume in emitted Viper without `// cerastes:inhale-ok <reason>`.
5. third_party/edg pins the owner's fork, branch `cerastes` (decision 0009); EDG edits are small commits with tests on that branch, kept upstreamable.
6. Non-obvious semantics → internal/decisions/NNNN-*.md before code.
7. Errors shown to users point at C++ source lines.
8. User-visible behavior lands with its page in doc/; every doc example is a test.

## Layout
src/backend (EDG IL → VIR) · src/vir · src/viper (VIR → .vpr + pos map) ·
src/driver · include/cerastes.h · tests/<area>/ · doc/ (user, Antora) ·
internal/ (developer)

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
