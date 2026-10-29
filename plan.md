# plan.md — C/C++ verifier: EDG front end → Viper (Silicon) → Z3

Working name: **`cerastes`**.

One Claude Code session per milestone, or per sub-milestone when a milestone does not fit in a few hours (≤ 4 h; owner,
2026-10-20): M4.1, M4.5a, M5a, …. Each session starts from `CLAUDE.md`, this file (its own milestone only),
`internal/status.md`, `internal/handoff.md` and the decision records, and ends by leaving the repo green and
`internal/status.md` updated so the next session can start cold.

This file was trimmed to what is live at the end of M4.4c (owner, 2026-10-26): the plans of the finished milestones,
the superseded text and the appendices are in `git show 1fcc43b:plan.md`. The milestones were renumbered on 2026-10-20
(the rest of C became M4; generic templates M5, concepts M6, C++ memory M7, inventory M8, evaluation M9); history
entries keep the numbers they were written with.

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
  CLAUDE.md                 # always-loaded rules
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
  .claude/agents/           # subagents (soundness-auditor, test-mutator, edg-backend, il-analyst)
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

## Done

| Milestone | Dates | Records | Result |
|---|---|---|---|
| Before M0 | 2026-10 | — | license Apache-2.0 WITH LLVM-exception; VeriFast outcomes recorded or run in Docker (M5c) |
| M0 Bootstrap and toolchain | 2026-10-13 | 0001 | pinned toolchain, `bootstrap.sh`, `run_tests.py`, `check.sh` |
| M1 EDG IL inventory · **Gate G1** | 2026-10-13 | 0002, 0003 | **G1 PASS**: prototype IL has generic bodies, instance links, constraints, attributes; in-process back end |
| M1.5 User documentation (Antora) | 2026-10-14 | 0004 | https://cerastes.dev/cerastes/doc/, every example a test |
| M2 Annotation layer | 2026-10-14/15 | 0005–0009 | `[[cerastes::…]]` registered through the EDG fork (`mborland/compiler` `cerastes`), `<cerastes.h>` |
| M3 Scalar encoder and driver | 2026-10-16/20 | 0010–0013 | `cerastes verify` over integers and `bool`, both modes; termination; mutation and lint |
| S1 EDG upstream merge · S2 CI · S3 mutation triage | 2026-10-21 | 0016 | the fork merged with upstream; CI on GitHub's `xcode-27` runner; the mutation bucket reviewed |
| M4.1 Control flow | 2026-10-20 | 0014 | `break`, `continue`, `do`, `switch` |
| M4.2a Jumps and ends | 2026-10-21 | 0015 | outward-and-forward `goto`, `noreturn`, `abort`/`exit`/`_Exit`, `assert` |
| M4.2b1 Named constants · M4.2b2 Enumerations | 2026-10-21/22 | 0017, 0018 | constants and case values checked in a prologue; enumeration value ranges |
| M4.2c Locals without an initializer | 2026-10-22 | 0019 | category `uninitialized` (P4222R1) by a ghost flag |
| M4.3a Side effects · M4.3b Calls in expressions | 2026-10-22/23 | 0020, 0021 | sequencing rules, the dependency-chain rule for calls |
| M4.4a No wrapping | 2026-10-23 | 0022 | unsigned arithmetic checked like signed; internal errors exit 3 |
| M4.4b1 Bitwise operators · M4.4b2 Values the front end computes | 2026-10-23/26 | 0023, 0024 | exact `& \| ^ ~`; static assertions checked as written; wrapped character literals; types from constants rejected until M4.5e |
| M4.4c Writes outside the chain, `void` `?:`, `-Wall` shapes | 2026-10-26 | 0025 | writes leave the chain; `void` `?:` statements; warnings in tests; the in-function static-assertion cross-check; 13 `-Wall` shape rules |
| S4 CI tiers | 2026-10-27 | 0016 (item 14) | pull requests and pushes to `main` run a PR tier (Silicon for the changed tests, the doc examples and a fixed 1-in-8 sample; every other run); the full check every night at 07:00 UTC and on dispatch |
| M4.4d Exact sets, constant masks | 2026-10-27 | 0026 | a precondition and a comma's value get their exact sets, today's sets as the fallback (one `requires` per argument accepted); constant masks as remainders, the `bit_mask.c [c23]` hang fixed |
| M4.5a Memory-model design · `\|`/`^` masks | 2026-10-27 | 0027–0030 | the C memory model designed and audited (a pointer as a `Ref` over a `Block` domain with four axioms, member blocks, liveness, explicit checks; fixtures in `tests/viper-smoke/memory/`), `[[cerastes::owning]]`, P4222's markers, C's spec functions, vlibc; `\|`/`^` with constant masks decided; M4.4e moved to M8, S5 Linux CI first |
| S5 Linux CI | 2026-10-28 | 0031 | a second CI job, `check-linux`, on `ubuntu-26.04` x86-64 in both tiers (`scripts/ci-linux.sh`); `bootstrap.sh` for Linux (clang 21 over libc++ 21 and glibc 2.43, Ubuntu's GCC 16 snapshot, sbt and Z3 by checksum); glibc's `assert`; five Embedded C extensions off in `edg_config` so both front ends accept the same language |
| M4.5b1a The model, address-taken locals | 2026-10-28 | 0032–0036 | the `Block` domain and its four axioms with lint tags; address-taken locals as one-slot blocks (allocated at block entry, scope exit checked), `&`, `*`, NULL, `==`/`!=`, liveness inside one function; `null-dereference`, `permission`, `lifetime`; C's incompatible pointer types as errors; the Memory and ownership page |
| M4.5b1b Pointer parameters and contracts | 2026-10-28/29 | 0037 | pointer parameters and results, `&param`; `owns`/`reads`/`live`/`old(*p)` in C and C++ spellings with one ghost amount per atom; the framing of contracts decided per path; `permission` at calls and returns; `pointer-comparison`; `restrict` on owned C parameters (plan `.claude/plans/m4-5b1b-pointer-params.md`) |

The finished milestones' plans (goals, tasks, acceptance) are in the commit named above; `internal/status.md` has a
row per session.

## M4 — The rest of C

**Goal:** every C23 construct that VeriFast verifies in C is verified by cerastes, or deliberately deferred with a
decision record; non-template code, one translation unit. `.c` tests run in both `c23` and `c++20` where the code is
valid in both. Strictly C (owner, 2026-10-20): C++ references, `new`/`delete` and classes are M7.

**Sessions.** Each is planned at ≤ 4 h and lands its decision record (invariant 6), must-verify/must-fail pairs
(invariant 2) and its docs (invariant 9), with one Opus audit round per diff plus a re-audit of blocking fixes and
`check.sh` at most twice. `internal/handoff.md` holds each session's tasks, agents, acceptance and stop rules.
Finished sessions are in "Done" above; the remaining ones are below (the splits are recorded in the
history of `internal/status.md`).

| Session | Scope | Needs |
|---|---|---|
| **M4.5 Pointers** (sessions b1–f3, below) | the memory model for C (records 0028–0030) | M4.5a, S5 (done) |
| **M4.6 Chars and strings** | string literals as read-only arrays, `char` buffers, vlibc's `<string.h>` (`strlen`/`memcpy`/`memcmp`/`strcmp` contracts and verified implementations), `main(argc, argv)` | M4.5e2, M4.5f3 |
| **M4.7 Function pointers** | contracts on function types, calls through pointers ("the pointer satisfies the type") | M4.5b1b |
| **M4.8 C parity close-out** | the VeriFast C differential set, the M4 mutation run, the Memory and ownership page complete, M4's acceptance, the deferred list for M8 | all |

**M4.5 Pointers** (one 4.x for pointers, owner, 2026-10-20; the C parts of the former memory-model milestone; split
further in M4.5a, owner, 2026-10-27: "this can be expanded if you see fit"). M4.5a designed it (records 0028 memory
model, 0029 ownership annotations and C spec functions, 0030 vlibc); `internal/handoff.md` §4.4 has each session's
plan and the VeriFast ports it takes.

| Session | Scope | Needs |
|---|---|---|
| **M4.5b1b Pointer parameters, results and contracts** (done) | pointer parameters and results (`restrict` only with `owns`), `&param`; `WF`; `owns`/`reads`/`live`/`old(*p)` in both modes (C's `cerastes_` forms, 0029 item 3); ghost amounts per atom, case splits; the framing of contracts decided per path; call, return and entry permission checks; category `pointer-comparison` (plan `.claude/plans/m4-5b1b-pointer-params.md`) | M4.5b1a (done) |
| **M4.5b1c Ownership, sequencing, loops** | `[[cerastes::owning]]` on parameters and functions, `owns_block`; sequencing on objects (`unsequenced`) and the call placement rule; cells in loops (hybrid permission invariants); `T**`; jumps out of cell scopes; the aliasing trap; the ports (`swap`, `inc`, `address_of_local.c`); the page completed | M4.5b1b |
| **M4.5b2 Globals, statics, initialization through pointers** | mutable globals and `static` locals as blocks with frames; `main` owns the globals; 0025 item 2's placement revisited; 0019's flag per cell; `[[cerastes::uninit]]`, `ref_to_uninit`, `must_init` and `--strict-init` | M4.5b1c |
| **M4.5c Structs** | field cells, `.` and `->`, struct values (copies, initializers, by-value parameters and results), nested structs, pointers to fields, `owns(*s)`, non-recursive owning fields | M4.5b1c |
| **M4.5d1 vlibc** | the repository (the owner creates `cerastes-verifier/vlibc`), its layout, license and CI; the submodule pin; explicit and transparent includes; the trust rule; `abort`/`exit`/`_Exit` moved out of the encoder's table | M4.5a (any time) |
| **M4.5d2 `malloc` and `free`** | vlibc's contracts for `malloc`, `calloc`, `realloc`, `free`; heap blocks; the free checks; leaks (a record; `forperm`); owning locals and globals; `invalid-free`, `leak` | M4.5c, M4.5d1 |
| **M4.5e1 Arrays** | local arrays, `a[i]`, array parameters, `owns_range` and quantified permissions, `forall` in C (0029 item 4), sum and max; P38's braced initializers over 1000 elements; M4.4b2's types computed from constants | M4.5b1c |
| **M4.5e2 Pointer arithmetic** | `p + i`, `p - q`, comparisons within one object, one-past-the-end, `malloc(n * sizeof(T))`; `out-of-bounds`; relational comparisons and `p - q` under `pointer-comparison` (the category lands in M4.5b1b) | M4.5e1, M4.5d2 |
| **M4.5f1 Predicates and pure functions** | predicate and pure bodies encoded (0011 item 4 lifted), `fold`/`unfold`, non-recursive predicates | M4.5c, M4.5d2 |
| **M4.5f2 Recursive predicates** | recursive predicates; a linked list (push, pop, length); recursive owning fields | M4.5f1 |
| **M4.5f3 Lemmas and ghost sequences** | lemma functions and ghost sequences for content specifications (M4.6's string contracts); its record decides whether all of it is in M4 | M4.5f2 |

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

## Before M5: M5.0 Type traits (owner, 2026-10-26)

**Goal:** what cerastes cannot read in C++ constant expressions before generic templates and concepts depend on it.
M4.4b2 skips a static assertion whose condition calls a function (a `constexpr` one included), uses a type trait
(`std::is_integral_v<T>`, `__is_integral(T)`, `std::is_same_v`), names `std::numeric_limits<T>` or another entity of
a system header (old handoff §11 question 39: "Skip it for now, but we need to address type traits before we get into
concepts. Add before M5."). This session decides how such conditions are encoded or checked, so that M5's integral
`T` and M6's requirement contracts rest on a known reading of type traits. One session of ≤ 4 h, after M4.8 and before
M5a; its scope is planned at its own planning (handoff §4.6).

Also before M5 (owner, 2026-10-27, handoff §8 question 11): `mutate.py`'s classifier learns to read local declarations
(typed locals), with its own audit, in M5.0 or a session of its own (handoff §4.6).

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
- P4222R1's markers for what M4.5 leaves (member initialization; owner, 2026-10-22, old handoff §11 question 21).
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
  - **Modular arithmetic on request** (owner, 2026-10-23, M4.4 planning, old handoff §11 question 13): cerastes has no
    wrapping (M4.4a, record 0022) and no opt-in; a modular result is written as a reduction in a wider type (`%`, or a
    mask from M4.4b1). A VeriFast-style truncating marker is an entry, to revisit with the ports.
  - **`void` `?:` outside statement position** (owner, 2026-10-23, old handoff §11 question 30): M4.4c accepts `c ? v() : w();` as a
    statement; a `void` `?:` as the left operand of a comma whose value is used, and C++'s `return c ? v() : w();` in a
    `void` function, stay rejected (`unsupported_type`).
  - **Enumerations M4.2b2 leaves rejected** (decision 0018): enumerations from system headers, enumerations in blocks, classes and templates, a `bool` underlying type, enumerators outside `int`, ordering comparisons of unscoped enumeration values, compound assignment and `++`/`--` on enumerations, and in C an enumerator list that names an earlier enumerator whose initializer is not of type `int`.
  - **From M4.4c** (record 0025): `(x = f(a), x) + (y = g(b), y)` with locals declared without an initializer
    stays `unsequenced_calls` (owner: the snapshots' `uninitialized` checks stay in the chain); `RED == RED` is
    accepted although clang warns (the front end folds it without an expression); `else ;` and `do ; while (c);`
    are accepted (GCC `-Wextra` only).
  - **Platforms beyond macOS and x86-64 Linux** (owner, 2026-10-27, during M4.5a: "If MSVC is easy add it, if not
    defer"). MSVC is not easy: it needs a second EDG configuration besides 0002's Clang emulation, a Windows bootstrap
    without bash, a driver without `posix_spawn`, Windows builds of the JDK 17 and Z3 pins, and an MSVC `native-ok`
    matrix. arm64 Linux (`ubuntu-26.04-arm`) adds a second set of target facts (plain `char` and `wchar_t` are unsigned
    there). S5 covered `ubuntu-26.04` x86-64 only (record 0031; the owner moved it from 24.04).
  - **The warning sweep (was M4.4e;** owner, 2026-10-27, M4.5a planning: "warnings will impact much more than just
    what we have now"). Every default and `-Wall` warning of clang 21 and GCC 16 that can fire on the language
    cerastes accepts, measured in both modes and rejected like the compilers (one rule per family) or listed; record
    0025 item 6's "still accepted" shapes and handoff §3's M4.4c notes first. Its record takes the next free number
    when it runs (0027 went to M4.5a).
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

**The approved plan** (owner, 2026-10-28): its first execution step saves it in the repository as
`.claude/plans/<milestone>-<topic>.md`, the branch name with `.` written as `-` (`m4-5b1b-pointer-params.md`), committed
with the planning commit and named in the status entry and the handoff.

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
