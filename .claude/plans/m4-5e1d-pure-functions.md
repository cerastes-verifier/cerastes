# Plan: M4.5e1d — Pure functions and the exact sum (record 0062)

**Approved by the owner on 2026-11-24**, at a session that only planned. It supersedes section 6 of
`.claude/plans/m4-5e1-arrays.md` ("plan mode again after P81 and S46"): **e1d does not plan again.** Its probes
and spikes run first (section 5), each with a decision table that says what is built for each outcome; an outcome
outside its table stops the session and asks the owner.

**Written for this ensemble** (owner, 2026-11-24, as for e1b and e1c): an Opus orchestrator; three Sonnet
implementers from `.claude/agents/` (`edg-backend`, `il-analyst`, `test-mutator`); the Opus `soundness-auditor`; a
Fable advisor the orchestrator consults. Section 2 assigns every task; the orchestrator does not re-divide the
work. **The session starts outside plan mode**, from the opening prompt in `internal/handoff.md` §2, and reads the
records this plan names (0061, 0060, 0013, 0011 and the ones section 6 amends), not all of `internal/decisions/`.

Branch `m4.5e1d-pure-functions`, worktree `.claude/worktrees/m4.5e1d`, `build/m4.5e1d/`, record
`internal/decisions/0062-pure-functions.md` (the next free number: if M4.5e1f has run first, the one after its
record, and "0062" below reads as that number). The plan holds whether M4.5e1f lands before or after: e1f changes
the rows `call_atoms` writes for a range atom, which this session does not touch.

## 1. Context

**Why.** M4.5e1c gave C its quantifier by inlining the body of a `pure` function at the `forall`. No `pure` body
has a meaning of its own: a call of a `pure` function is rejected in every annotation (`call_in_annotation`), a
contract written on one is neither read nor rejected, and nothing can name a value computed from an array. M4's
acceptance asks for the exact array sum "by a recursive `pure` function" (question 227). e1d gives `pure`
functions their meaning, their calls and their recursion.

**Owner decisions.**
- **226** a `forall` inlines its function's body; a call of a `pure` function anywhere else waits for e1d.
- **227** the exact sum is in M4.5e1: e1d takes `pure` bodies, their calls and their recursion from M4.5f1/f2;
  M4.5f1 keeps predicates, `fold`/`unfold` and pure functions over predicates.
- **252** a contract on a `pure` function is rejected where a `forall` names the function; e1d gives such
  contracts their meaning.
- **At this planning (2026-11-24):**
  - e1d is planned in full, with decision tables in place of the second planning.
  - **Which `pure` functions are read:** one that an annotation or another read `pure` body calls, or that
    carries a `requires`, `ensures` or `decreases`. One that only a `cerastes_forall` names, or that nothing uses,
    is not read, as today.
  - **A `pure` function without a body in a verified file of the unit cannot be called:** rejected (a bodiless
    Viper function's `ensures` would be an axiom the user writes); M6's contract manifest lifts it.
  - **One session**, with split points and an ordered cut list; the exact sum is protected from cuts. The
    owner chose it twice: first on an estimate of 2,000–3,100 lines, then again (**271**) on the design check's
    3,050–4,500, knowing that the first split point is likely to be reached.
  - **272** nothing more is left out to hold the size: a `cerastes_forall` in a `pure` function's contract,
    arguments that read memory, and calls in a `pure` function's own `requires` and `ensures` are in scope, each a
    pre-agreed cut (section 7).
  - Questions 256–263 of M4.5e1c stand as built.
  - **264** Viper's own check of a `pure` function reports its wrong `ensures` and its termination, mapped to C
    lines. **265** a `forall` naming a function with a contract stays rejected. **266** calls under a binder, a
    `forall` in a called body, mutual recursion and calls in measures are not in e1d. **267** a caller learns an
    application's range only from the function's `ensures` (a false alarm otherwise; M4.8 reviews).
- **Standing:** 69 (no EDG work), 77 (one context), 160 (a new test's expect line may be corrected after its first
  run, each listed in status), 176 (the owner handles CI); `check.sh` once, at the end, on the x86-64 box; no test
  runs while a fix is queued; no Claude trailer; commits stay local.

**Scope.**
- **In:** a read `pure` function as a Viper function with its contract (`requires` with value clauses,
  `reads(*p)` and `reads_range(p, n)`; `ensures`; `decreases`); the check of its definition (the obligations of
  its body and contract, its `ensures`, its termination); self-recursion; **applications** (calls of a `pure`
  function) in an ordinary function's `requires` and `ensures`, in a loop's `CERASTES_INVARIANT`, in a
  `CERASTES_CHECK`, and in a read `pure` function's body and contract, each with explicit rows for the callee's
  `requires`; `f__wf` for contracts that hold an application; calls and returns; the exact sum with its loop; the
  shape rules and the static-assertion cross-check in read `pure` bodies; the lint; the tutorial section "The
  exact sum".
- **Not here** (rejected, each pinned, the message naming where it is lifted):

| Construct | Lifted by |
|---|---|
| a `predicate` (body, call, `fold`, `unfold`); a pure function over a predicate | M4.5f1 |
| a call of a `pure` function with no body in a verified file of the unit (owner, 2026-11-24) | M6 (the contract manifest) |
| a call of a `pure` function from code (`spec_call_in_code`, the front end's, unchanged) | never (specification) |
| a `pure` member function, template or template instance; a `constexpr`/`consteval` `pure` function that is read | M7, M5 |
| mutual recursion between `pure` functions; a `pure` function that reaches itself through a `requires` or an `ensures` (its own or another function's): the only cycle accepted is a function calling itself in its own body | M4.5f2 |
| a call under a quantifier's binder (inside the function a `cerastes_forall` names); a `cerastes_forall` naming a function that has a contract (question 252 at the `forall` site) | M4.5e2 |
| a `cerastes_forall` inside the body of a read `pure` function (a named quantified property) | M4.5f1 |
| a call in a function's or a loop's `decreases`; in the count or guard of an atom, in an element's index, as a trailing argument of `cerastes_forall` (the positions that must read no memory: a call counts as a read there); under `cerastes::old` (`cerastes_old` of a call does not compile: the macro takes an address) | M4.5f1 (measures), M4.5e2 (the rest) |
| a read `pure` function whose body names `cerastes_old` (a function has one state; e1c allows the body only under a `forall` in an `ensures`) | never as a call |
| a read `pure` function that returns a pointer, a structure or `void`, or takes a structure by value | M4.5f1 |
| in a `pure` function's `requires`: `owns`, `owns_range`, `owns_block`, `live`, an atom under a guard or `||`; any atom in its `ensures`; markers on its parameters (0042 item 4); `restrict` | M4.5f1 |
| in a read `pure` body: a global or `static` local (0041 item 12), structure members, array members, a local declaration, more than the one `return` | M4.5f1, e1e |
| a pointer argument that a callee's atom names and that is not a bare pointer parameter (contract) or a pointer variable that is not address-taken or a named local array (loop invariant, `check`): 0061 item 3's rule | M4.5e2 |

**Size** (`src/` and `include/` insertions without the VIR self-test cases; from the planning's design check
against `lower.cpp`, calibrated on what e1c landed: 3,514 against an estimate of 1,500–2,200): Part A 350–500,
Part B 1,700–2,400 (`encode.cpp` 600–900, `lower.cpp` 700–900, `check.cpp` 250–400), Part C 500–800, Part D
500–800; **3,050–4,500** in all, **plus about 300–500** for the three constructs question 272 keeps in scope
(they were outside the design check): **3,350–5,000**. Beside it the lint: 900–1,300 lines with 700–1,000 in its
self-test (e1c: 1,080 and 950). This is more than one e1c; the split points of section 7 are expected to be
used.

## 2. The ensemble

| Role (model) | Owns | Never touches |
|---|---|---|
| **Orchestrator** (Opus) | Steps 0–1; record 0062; the vprdiff verdict per part; user pages in `doc/` and the tutorial section's prose; status, handoff, `plan.md`'s Done row; every commit; briefing and resuming the agents | `src/`, `tests/`, `scripts/` (it delegates) |
| **edg-backend** (Sonnet) | `src/backend/**`, `src/driver/**`, `include/cerastes.h`; **in Part A only** `src/vir/**` and `src/viper/**`; the build system, the baseline, and **the joined build of each part** with `scripts/regen_vpr.py` and `scripts/vprdiff.py` on it; `internal/encoding.md`, the developer guide's paragraph | `tests/`, `scripts/lint_viper.py`, `src/vir` and `src/viper` after Part A |
| **il-analyst-1** (Sonnet) | P81 and S46; the chapter `## M4.5e1d` of `internal/il-inventory.md`; the fixtures `tests/viper-smoke/memory/s46_*.vpr`. It ends when the chapter is committed | `src/` |
| **il-analyst-2** (Sonnet, fresh at Part B) | `src/vir/**` (`vir.h`, `vir.cpp`, `check.cpp`, `lower.cpp`, `lowered.*`), `src/viper/**`, the next free self-test case file (`src/viper/selftest_cases33.inc` unless M4.5e1f took it), for Part B | `src/backend`, `tests/` |
| **il-analyst-3** (Sonnet, fresh at Part C) | the same files, for Parts C and D. Briefed with this plan, record 0062, the S46 chapter and Part B's commit | `src/backend`, `tests/` |
| **test-mutator** (Sonnet) | `tests/**` (tests, twins, pins, exactly the Flips), `tests/mutants-equivalent.txt`, the compiler matrix, `scripts/run_tests.py`, `scripts/lint_viper.py`, `scripts/rejections.py` and their self-tests, the example files under `doc/modules/ROOT/examples/` | `src/` |
| **soundness-auditor** (Opus, read-only) | four rounds and one re-audit (section 7), each on a snapshot commit in `.claude/worktrees/m4.5e1d-audit`, each before its part's commit; resumed between rounds | everything (never edits) |
| **Advisor** (Fable) | consulted by the orchestrator at the checkpoints below | — |

- **Five Sonnet agents over the session, at most three alive at once:** edg-backend, the current il-analyst,
  test-mutator. edg-backend is resumed between tasks (SendMessage); an il-analyst is replaced, not resumed, at the
  two hand-overs (`lower.cpp` is 21,675 lines); the test-mutator is replaced by a fresh one from its state file
  (`build/m4.5e1d/test-mutator-state.md`, which it keeps current) when the orchestrator and the advisor judge its
  context low (e1c did so at about 940k tokens). This is the plan, not a deviation.
- **Builds:** every agent that edits `src/` builds its own change through the lock (`lockf
  build/m4.5e1d/build.lock cmake --build build/cmake`). Nothing builds during a directory-wide run.
- **The runner** (`scripts/run_tests.py`; it locks `build/vpr`) has one holder at a time: the test-mutator for
  R3; il-analyst-1 from R3's end until Commit 1 (S46's `.vpr` fixtures, one file per run); the test-mutator from
  Commit 1 on. Everyone else uses `build/cerastes verify <file> --keep-vpr`.
- **The advisor** is the `advisor` tool; if the orchestrator has none, an Agent call with `model: fable`, briefed
  with the question and the paths to read. Step 0 checks that one of the two answers.
- **`lower.cpp`, `vir.h`, `vir.cpp`, `check.cpp`, `lowered.*`, `emit.cpp` have one holder at a time:** edg-backend
  in Part A, il-analyst-2 in Part B, il-analyst-3 in Parts C and D, with a commit between holders. An agent that
  needs a change in another's file asks the orchestrator, who passes it on.
- **Advisor checkpoints:** (1) the record draft with P81's and S46's outcomes mapped to section 5's tables,
  before audit round 1; (2) any outcome outside a table; (3) the split points at Commit 2 and at Commit 3;
  (4) any cut; (5) before the box run.
- **Every agent** (the orchestrator puts this in each briefing):
  - reads `CLAUDE.md`, this plan, record 0062 and the files it is assigned; edits only its own files;
  - any question stops its work and goes to the orchestrator; **an owner question goes to the owner, and the work
    it blocks waits for the answer** (owner, 2026-11-24);
  - writes each report to `build/m4.5e1d/<role>-report-<n>.md` as well as returning it (the soundness-auditor
    cannot write: the orchestrator cuts each snapshot commit and the audit worktree, copies the binaries built
    from exactly that source, and saves each audit report to `build/m4.5e1d/auditor-report-<n>.md` itself);
  - follows the build and runner rules above;
  - a test generator lives in `build/m4.5e1d/` and is never re-run over the tree;
  - tool code in the owner's style (`scripts/format.sh`; brace initialization, `U`, one declarator per
    declaration, no comment on a brace line); no Claude trailer.
- **A reader lands with what it reads** (P77): nothing is registered or declared in this session (`pure` and its
  macro exist); the encoder's acceptance of a call lands in the part whose lowering gives it rows.
- **Fail closed between parts:** what a later part lifts is rejected with a diagnostic and a staging pin, written
  and deleted inside the session; in `src/vir` the gate is `check.cpp`'s (it refuses an `Apply` in a contract and
  in a loop invariant until il-analyst-3 lifts each with its rows).
- **Session tools are committed** (question 193): anything Step 0 or a spike scripts goes to `scripts/` with a
  self-test, or its summary to `internal/il-inventory.md`.

## 3. Survey facts (tree at `9ab9a1e`; read, not run; names, with lines where they help)

- **Nothing needs the EDG fork and nothing new is registered.** `pure` is registered `{"pure", "", "r",
  RAF_ENTITY_ATTACHED | RAF_NO_CONSTANT_EVALUATION}` (`register_attributes.cpp:94`); `CERASTES_PURE` is
  `[[cerastes::pure]] inline` (`include/cerastes.h:63`; `inline` alone for other compilers).
- **The front end allows what e1d needs.** `annotation_checks.cpp`: a spec body may name any spec function
  (`body_on_expr` 2038: with `spec_caller` every node is exempt); a call of a `pure` function inside an annotation
  is exempt from `spec_call_in_code` (`mark_attribute_arguments` 2282); no rule on recursion; `contract_on_predicate`
  (1929) is for predicates only; `check_old_in_spec_body` (658) reports `cerastes::old` in a spec body. The
  refusal of a call in an annotation is the encoder's alone.
- **The encoder** (`src/backend/encode.cpp`, 22,986 lines):
  - `Encoder::run` (22503) skips every spec routine at 22605–22614, before the weak/`constexpr` rejections
    (22615–22626), the bodiless static/inline rejection (22627), `check_signature` (22672), `function_of` (22681,
    22697) and `encode_body`; it runs only `reject_markers_in_spec_function` (7330),
    `reject_struct_value_in_spec_signature` (7305) and `reject_globals_in_spec_body` (7415). So R1–R13 and
    `encode_block_static_assertion` never see a
    `pure` body, and `check_block_static_assertions` (22723) exempts a spec routine's region (22748–22753).
  - `call()` (11178): `spec_fn(callee)` (8443; a user `pure` function is `SpecFn::none`); under `cx_.contract ||
    cx_.annotation` a call that is no atom and no `forall` is `call_in_annotation` at 11257 (`diagnostics.def:141`,
    "a call to \"%s\" is not allowed in an annotation: only \"cerastes::old\" can be called here"); inside the
    function of a quantifier the same tag at 10256 (`first_forall_effect` 9876). The four entry points:
    `encode_contract` (4887; `cx_.contract`, `cx_.annot`; a function's `decreases` through `encode_measure` 4962),
    `encode_loop_clauses` (15712), `annotation_statement` (15673; `cx_.check_clause`), `owning_count` (3793).
  - e1c's reader: `struct ForallFn` (9646), `read_forall_function` (9681–9829: the rules on `f`, its parameters by
    position from `body_scope(r)`, up to eight single-statement blocks peeled, one `stmk_return`), `ForallSite`
    (9312) in `Ctx::forall` (325) with `variable()` consulting `forall_parameter` (9397) first, `forall_site`
    (10061), `forall_body_ok` (9833). The only string in `src/` that names e1d is 9737 ("contracts on a `pure`
    function come with M4.5e1d").
  - Contracts into VIR: `function_of` (4613: `check_signature`, `prog_.add_function`, params, `encode_contract`),
    `routine_has_contract` (15495; "a `decreases` alone is not one"), `callee_for_call` (15610).
  - Termination: `edges_` (333) is filled only from code calls (11283–11286); `require_measures_in_cycles`
    (22765) walks `to_verify_` and raises `missing_decreases` (22811). Nothing models calls among `pure`
    functions.
- **VIR** (`vir.h` 1,371 lines; `check.cpp` 5,199): `Expr::Kind::Call` (326) is the code call that lowering hoists
  as a statement; `Forall`, `Bound`, `ForallArg` (436–448). `Function` (767–842) has no `pure` flag and no body
  expression; `verified()` is `body != nullptr`. `check.cpp` `call()` (4377) refuses a `Call` in a `check`, an
  invariant or a measure and in a contract; `mark_forall_positions` (1974), `forall_node` (2323).
- **The lowered program** (`lowered.h`): `LProgram` has `methods` only (285); the prelude functions are raw text
  (`prelude.cpp:236–343`, `emit.cpp:431`), each with a body, at most one `ensures`, **no `requires` and no
  `decreases`**, and never a map row; an application is `LExpr::Kind::Fn` (`low::fn`, no name check);
  `LGhostFn {name, result}` (254) is a ghost function of one `Int` slot; `LStmt` has no `inhale`. `Check
  {viper_id, meta, expected_reason}` (184) is one map row.
- **`f__wf`** (`wf_method`, `lower.cpp` 20926–21572; made for every function with a `requires`, `ensures` or
  `decreases`, 883–889): the `requires` clauses, then the `ensures`, each lowered under `set_ctx(f, CMode::Wf, ens,
  idx)` with its obligations asserted as `prior && guard ==> cond` (20953) and `prior` extended; the `decreases`
  obligations under `requires_only` (21173); assumed facts as `requires` lines of the method (`facts_` 21207, the
  four `hvq` facts 21346–21494); the ghost domain `Hvq_<method>` (21360) with functions
  `hvq_<method>__<formal>_<pre|post>` (`hvq_name` 7360). `contract_read`'s `Wf` branch (7792, 8015–8082): a
  ghost and one framing obligation
  (`permission`). `check_cycles` (1515) requires every `__wf` method to make no call.
- **Contract modes:** `enum class CMode { None, Method, Wf, Call, Ret, Inv }` (4678); range atoms by
  `range_atom_value` (6508) and `range_quant` (3841): the pure line `p != null && WF(p) && 0 <= n && offset_of(p) +
  n <= len(block_of(p))`, then one quantifier per fact over the absolute slot (`acc` of `v_<t>` and `d_<t>` at
  the ghost amount `amt_<k>`, the flag, the element range), then `acc(lvloc(block_of(p)).lv, amt_<k>)`; clauses
  split per conjunct by `push_clauses` (20470). `forall_expr` (7381) lowers a quantifier in every mode.
- **Hook points for an application** (the planning's design check; read, not run):
  - obligations need no threading: every mode writes them with `obl(full, guard, …)`; `wf_method` adds `prior`
    (20955), a loop adds it (17402–17408), and `Method`, `Call` and `Ret` lower into a scratch that is dropped
    (20647, 10614, 14243), so one set of rows appears in `__wf`, loops and checks and vanishes elsewhere;
  - a call site re-lowers the callee's clause over `cenv.formals` (`set_ctx(callee, CMode::Call, false, ci)`,
    10527–10533, 10611–10620), so an application nested in a clause is lowered afresh with its arguments;
  - a callee's `requires` at the arguments: the heap-free idiom of `site_guard` (6266–6275) and `range_places`
    (14600–14605): `cmode_ = CMode::Wf` over an `Env` whose formals are the arguments; a range atom gives
    `range_shape` (6522–6526);
  - a range site's framing disjunct is built in `contract_read` from `site_ref`, the site's count and
    `site_guard` (7917–7931; the site filter 7888–7916; marked sites excluded 7920–7924); `in_range` (3851) is
    point membership: **interval inclusion needs a new helper**;
  - the function's own contract cannot be plain `CMode::Method`: `range_atom_value` (6528) and `atom` (6733) name
    `amt_k` and write the liveness `acc` (6577). A flag saved in `CtxSave` (5733) fits; a new `CMode` value would
    need every `cmode_ ==` test audited. Its `ensures` and body are lowered as the measure is, `set_ctx(f, …,
    false, requires_.size())` (20709): with `ens` false every read is the entry state's and `first_site` (5757)
    sees every `requires` site;
  - **the body as one more clause of `wf_method` is small** (about 30–40 lines, modelled on the measure loop
    21174–21194, asserted under `requires_only`); `wf_ghosts_` (21603; formals 21510–21513) and `wf_hvq_` (7040;
    `m.ghost_fns` 21376; the domain is set only when it is non-empty, 21360) are where application ghosts go;
    `emit.cpp:343` hardcodes a ghost function's `(s: Int)`;
  - loops: `InvRead` (1731), `inv_framing_any` (4228) with its asserts at entry (17513) and at the back edge
    (17762), `frame_flag_line` (7623), `RangePlace` (14499) and `ArrayPlace` (14487), `expr_touches` (14333);
    checks: `fq_check_rows` (7723–7786), whose marked rows the lint admits only over a `q<j>` binder with the
    bound variable's range in the antecedent (`wildcard_row_problem`, `lint_viper.py:3731–3753`);
  - **a function with `body == nullptr` is a trusted method today:** `check.cpp:847` (an error without a
    `trusted_reason`), `function()` (`lower.cpp` 20693–20698: a bodiless method with `decreases _`), `run()`
    (879–889: no `wf_method` without a clause), `verified()` (`vir.h:804`); `src/driver/selftest.cpp:603` builds
    an `LProgram` by hand;
  - **`reads_heap` (`vir.cpp:1325`) sees only `Deref` and `Atom`**, and `lower_node` sets `cold_` only when it
    is true (9172): an `Old` over an application would be read in the post-state unless `reads_heap` is true for
    an application. The rules that ask "reads no memory" (`guard_ok` `vir.cpp:1223`; range counts `check.cpp:1370`
    and guards 1082; element indices 3709; `forall` arguments 2369 and `encode.cpp:10225`) then refuse an
    application by themselves.
- **Termination today** (0013, 0021): every method and loop prints a `decreases` line (`emit.cpp:288–296,
  384–402`); Silicon's plugin checks them (no `import`; the lint refuses one); a method's measure line carries
  `Check {"termination.failed", …}` (20700–20724); `compute_cycles` (1469) and `check_cycles` see only `Call`
  statements. **`tests/viper-smoke/termination_kinds.vpr` pins the function form:** `function f_not_decreasing(n:
  Int): Int decreases n { f_not_decreasing(n) }` gives `termination:tuple.false` **twice at the application's
  line** (not at the `decreases` line), and `f_ok` with `requires n > 0` and `decreases n` verifies.
- **Errors** (`src/driver/report.cpp` `map_errors` 118–226): a Silicon error is matched by `(vpr line, id)`; no
  row, or a row of category `internal`, is exit 3; a row's reason must be `assertion.false` (the three
  termination reasons for `termination.failed`; a marked row its `expected_reason`, and `parse_position_map`
  accepts only `assert.failed[insufficient.permission]`). Silicon runs with `--numberOfErrorsToReport 0`
  (`silicon.cpp:129`) and assumes a failed assertion afterwards. The driver prints one error for rows that share a
  line, a category and a message.
- **The lint** (`scripts/lint_viper.py`, 5,055 lines): a bodiless top-level `function` is refused (4564); a
  bodied one passes with no rule on its name; **no rule asks a function for a `decreases` line**
  (`lint_termination_clauses` 4711 reads methods and loops); a function's `requires`, `ensures` and `decreases`
  lines need map entries (`CHECK_LINE_RE` 209; `KIND_IDS` 226: `requires` → `not.wellformed`, `ensures` →
  `not.wellformed`, `postcondition.violated`); `MAP_ERROR_IDS` excludes `application.precondition` (192);
  `wildcard` is admitted only on 0061 item 10's marked row (1292, 3824–3828); a `forall` in a `function`'s
  `requires` is "outside the shapes" (4499 tests `lineno in method_of`; `classify_quantified_lines` 4369 reads
  methods only); a `decreases` line with a measure needs a `termination.failed` entry of its own (4600–4606),
  and an entry on a body line is not refused (4665–4675); ghost domains by `lint_ghost_domain` (4213) with
  `GHOST_PARAM_RE` (3578: `Int` or `Block` parameters), the name rule `hvq_<method>__` (4265) and
  `wf_fact_verdict` (3971: exactly four fact shapes); `HEAP_READ_RE` (3340) reads `.field`, `perm`, `acc`,
  `old`, `forperm`, `forall` — **an application `sum_to(a, n)` counts as heap-free** in the fifteen places
  `_heap_free` is used.
- **S45 (d) (7), the only existing evidence** (`tests/viper-smoke/memory/s45_d_{h,m}_sum*.vpr`): `function
  sum_to(a: Ref, n: Int): Int requires 0 <= n requires forall k: Int :: { loc(block_of(a), k) } … ==>
  acc(loc(block_of(a), k).v_int, wildcard) { (n == 0) ? 0 : (sum_to(a, n - 1) + loc(block_of(a), offset_of(a) + n -
  1).v_int) }` verifies in a bodiless method's `ensures r == sum_to(a, n)`; under variant M the `__wf` row is the
  interval inclusion over a ghost `gs_post(block, offset, count)`. **The twin `sum_to(a, n + 1)` gives two errors
  under M: the `__wf` row and Viper's own check at the bare application in the contract** (finding (6)). The
  fixture's function has **no `decreases`, no `ensures`, no element-range line and no guard**, and **no fixture
  applies a function in a loop invariant**: whether Silicon keeps `s == sum_to(a, i)` across an iteration
  unaided is not known (the function peels the last element and the loop grows at the end, so one unfolding
  should match; the risk is the equality of the two snapshots over a quantified precondition). `e2_array_sum.vpr`
  (S24) holds no function: it bounds the sum. `tests/viper-smoke/fail_kinds.vpr` pins `application.precondition`
  with `assertion.false` at an application; no fixture shows a function's failed postcondition or an ill-formed
  function body.
- **Tests and pins** (`tests/README.md` has the grammar, the 21 categories and the tag-coverage rule):
  - no test holds a recursive `pure` function; no verify-path test calls a `pure` function in a `requires`,
    `ensures`, invariant, `check` or measure (nothing pins `call_in_annotation` for a `pure` callee);
  - pins that name e1d: `tests/memory/negative/forall_reject_f_{requires,ensures,decreases}.c`
    (`unsupported_annotation @ L16`; the only contracts on `pure` functions in the verify path, each named by a
    `forall`) and `forall_reject_e_call.c` (`call_in_annotation @ L19`, a call inside `E`);
  - the twelve `pure`/`predicate` rejections of `tests/memory/negative/` (`reject_marker_pure.cpp`,
    `reject_owning_pure.cpp`, `reject_not_invalidating_pure.cpp`, `reject_uninit_pure_body.cpp`,
    `reject_struct_value_pure.c`, `global_pure_write.cpp`, `static_local_pure.cpp`,
    `reject_owning_global_pure_read.cpp`, the predicate counterparts): each stays rejected;
  - `tests/scalar/static_assert_pure_body.c` (`twice` is neither called nor contracted) with its entry
    `tests/mutants-equivalent.txt:661`; `tests/memory/negative/array_loop_bool_post.c` (the exact count);
    `tests/scalar/negative/reject_call_in_annotation.c` (a non-`pure` callee);
  - front-end tests only (`tests/annotations/**`): `pure` functions with contracts, calls in contracts
    (`template_predicate_pure.cpp`, `constexpr_spec_functions.cpp`), `constexpr` `pure` functions: **none is on
    the verify path; no front-end expectation and no golden may change.**
- **Docs today:** `annotations.adoc` 9–19 (the IMPORTANT box: every call in an annotation is rejected), 776–783
  ("a pure function's contract is not read yet"), 918–939 (`forall`'s function has no contract), 1075–1079,
  1309–1316; `verifying.adoc` 108–111, 259–276, 784–786, 1088–1089, 1299; `error-messages.adoc` 345–367
  (`termination`); `tutorial/arrays.adoc` (sections at 9, 39, 61, 105, "What is next" at 138);
  `tutorial/loops.adoc` already has an executable `int sum_to(int n)` (the tutorial's `pure` function takes
  another name).
- **Self-tests:** the latest case file is `selftest_cases32.inc`; `VIR_CASES_MIN=2202` (`scripts/check.sh:104`).

## 4. Step 0 and Step 1

1. **Orchestrator.** `git fetch`; branch from `origin/main` when it holds this plan's commit (otherwise from
   local `main`, and say so in status); `git worktree add .claude/worktrees/m4.5e1d -b m4.5e1d-pure-functions
   <base>`; submodules with `--reference` (edg, silicon, then silver); `scripts/bootstrap.sh`. `npm ls
   @antora/logger` in `doc/` shows one hoisted copy (**nested copies: stop and ask**). CI recorded only (`gh run
   list --workflow ci.yml --limit 5`). Whether M4.5e1f has landed (the record's number). One question to the
   advisor, to check that it answers.
2. **edg-backend.** The baseline before any change to `src/`: `scripts/regen_vpr.py` and the VIR self-test into
   `build/m4.5e1d/baseline/`; frozen binaries in `build/m4.5e1d/cerastes-base/`.
3. **test-mutator, on the frozen build** (`build/m4.5e1d/s0/`, both modes):
   - **R1** today's outcome of the pins of section 3 (unchanged by this session unless section 9 lists them).
   - **R2** today's diagnostic for every construct of section 1's scope and table, one small program each: a call
     of a `pure` function in a `requires`, an `ensures`, an invariant, a `check`, a function's and a loop's
     `decreases`, an atom's count, a `forall`'s trailing argument and inside `E`; a recursive `pure` function
     nothing calls and one with a contract nothing calls (expected: verified, not read; question 252's premise);
     a call of a bodiless `pure` function; a `constexpr` `pure` function. **The test-mutator adds one pin per
     position the session lifts** (`pure_call_reject_{requires,ensures,invariant,check}.c`,
     `call_in_annotation` at the call), retired by the part that lifts it (Flips A). `grep -n 'e1d' src/` (the
     survey found one message).
   - **R3** `scripts/rejections.py build/m4.5e1d/s0/rejected-base.txt`. After each part the same run may differ
     only by section 9.
   - **M** the compiler matrix (clang and GCC, both modes, no flag / `-Wall` / `-Wall -Wextra`) for the new
     source shapes: a recursive `CERASTES_PURE` function that only annotations name; one with `CERASTES_REQUIRES`,
     `CERASTES_ENSURES(r, …)` and `CERASTES_DECREASES` after its parameter list, with and without a prior
     prototype; `long long` bounds such as `-2147483648LL * n`; a parameter used only in the contract. **Rule
     (not a decision):** every positive compiles with `-Wall -Wextra -Werror`; a shape that warns without a flag
     or at `-Wall` is written the other way in the tests and noted in the record; nothing new is rejected from M.
   - R3 is the only `run_tests.py` run until il-analyst-1's P81 has finished; S46's runs start after R3.

## 5. P81 and S46 (il-analyst-1, before any encoder code; results in `internal/il-inventory.md`, `## M4.5e1d`)

**P81 (IL, both modes, `build/m4.5e1d/p81/`; every claim with its `--il_display` or `--cerastes_walk` excerpt).**

| Probe | Expected | If not |
|---|---|---|
| (a) a call `g(e_1, …)` of a `pure` function in a `requires`, an `ensures` (the result name as an argument), an invariant, a `check`; C23 and C++20 | an `eok_call` whose callee is `enk_routine g` (`direct_callee`), each argument the written expression under at most one compiler-generated conversion; parameters of the enclosing function as in any annotation of that kind (`enk_param_ref` in a contract, variables in a loop clause and a `check`) | another callee shape in one mode (a constant address, a reference binding of an argument that no single peel rule reads): **stop and ask** |
| (b) a call inside a `pure` body: of another `pure` function, of itself, in an arm of `?:` and in an operand of `+` | the same call node; the callee of a self-call is the routine being read; parameters are `enk_variable`s that map to positions (P80 (c)); the front end raises nothing | a diagnostic from the front end: **stop and ask** (no front-end expectation may change) |
| (c) `requires`, `ensures(r, …)` and `decreases` written on a `pure` function, on its definition, and on a prototype followed by the definition | `contracts_of(r)` gives them as on any function (`encode_contract` reads them unchanged; the result name resolves); a mismatch between two declarations is the front end's `annotation_mismatch` | attributes land elsewhere for a `pure` routine: one reading rule in `encode_contract`; anything that needs the front end: **stop and ask** |
| (d) argument shapes: `int *` for `const int *`; an `int` for a `long long` parameter; `a[i]` and `*p` (reads) as arguments; a named local array (its decay) in an invariant and a `check`; the result name | each is the shape the same expression has as a call argument in code (P78 (c), P79) under one generated conversion | a shape code does not have: rejected with a pin (`unsupported_annotation`, M4.5e2), recorded; no stop |
| (e) a `pure` body with a block-scope `static_assert` beside its `return`; with a local declaration; `constexpr` on the function | the static assertion is a declaration of the body's scope and not a statement (the one `return` is still the only statement); a local declaration is a second statement or a scope entry the reader can see | the reader cannot see a declaration it must refuse: **stop and ask** |
| (f) the front end today on a call of a `pure` function under `cerastes::old(…)` (C++) and `cerastes_old(…)` (C) | C: a compile error from the macro (the address of an rvalue); C++: accepted by the front end, so the encoder refuses it | recorded; the encoder's refusal is pinned in the mode where the program compiles |

**S46 (hand-written Viper in the shapes of section 6; fixtures `tests/viper-smoke/memory/s46_*.vpr`; every row
expanded as lowering will write it, from the frozen build's kept Viper of e1b's and e1c's tests; each positive
with failing twins; one file at a time through `scripts/run_tests.py`; times on a quiet machine).** A twin must
fail with **exactly one** error unless the table says otherwise. Every fixture method ends with an `assert false`
twin (vacuity). `pf_g` is the Viper function of the `pure` function `g`; `G` is the guard of an application
(item 7).

| Spike | Programs | Built when it passes | If it does not |
|---|---|---|---|
| **(a) the function and Silicon's own checks** | `pf_sum_to(a: Ref, n: Int): Int` in item 6's shape, **in two forms of the atom's lines: Min** (the pure line, `acc(v_T, wildcard)` under the quantifier, the element range) **and Full** (a method's lines for the atom, `d_T` and `lv` included, `wildcard` for every amount): the parameter ranges, `0 <= n`, the user's `ensures` (the two bounds), `decreases n`, the body with its guarded read and guarded self-application. Twins, each recording Silicon's id, reason, line and count: a wrong `ensures` bound; `sum_to(a, n)` for the self-application (not decreasing); the measure `n - 5` (not bounded); **no `decreases` line on the recursive function (expected: Silicon reports nothing — the evidence for the lint rule)**; **an empty `decreases` on the recursive function (expected: reported — the backstop if `missing_decreases` were ever bypassed, 0013 item 4)**; a non-recursive function with an empty `decreases` that applies a prelude function (`c_and_32`, which has no `decreases`) and another `pf_` function; a function that applies itself in its own `ensures` (recorded only: the encoder rejects it). **Every twin of a body row gives the function no `ensures`, or one that holds for any value** (where a `__wf` row fails, the guarded body takes its default on that path, and a real `ensures` may fail beside it); one twin with a real `ensures` is recorded for the count | items 6, 10 and 11: **Min**, when (a), (d) and (e) pass with it (Full where a positive fails with Min and passes with Full, and the chapter says which line made the difference); the two native rows (`postcondition.violated` on an `ensures` line; `termination.failed` on the line of a self-application), every other native id on a function unmapped | `wildcard` under the quantifier refused in a function's precondition, or both forms failing a positive: **stop and ask**. A wrong `ensures` reported with another id, or on another line: the row follows what Silicon reports (recorded; item 11 revised). Two errors with one id on one line: accepted (the driver prints one). An application of a prelude function refused by the termination check (expected from memory: `termination.condition.false`, as 0013 found for methods): the prelude functions print an empty `decreases` line **in a program that holds a `pf_` function, and only there** (none is recursive: `c_pow2` is a table), so no existing program's Viper changes; recorded, no stop. Termination reported at the `decreases` line instead of the application: one row there, at the function's `return` (item 10 revised). An empty `decreases` on a recursive function **not** reported: **stop and ask** (the backstop is gone) |
| **(b) the guard** | a bodiless method `ensures r == (G ? pf_sum_to(a, n) : 0)` with its `__wf` (the ghost application of (c)); the function's own body with `G` around its self-application. Twins: `sum_to(a, n + 1)` in the contract (S45's twin: expected **one** error, the `__wf` row, and no native error); a callee value-`requires` the contract does not give (`n <= 1000`: one `precondition` row); the same two inside a function's body, the function with no `ensures` (see (a)); **the default is not observable**: a caller of the bodiless method with `check(r == 0)` fails, and with `check(r == pf_sum_to(a, n))` verifies | item 7's form **G** in a method's contract and in a function's body and contract | a correct program fails, a twin gives a native error, or any program takes over 30 s: **stop and ask**, with the alternative measured for the question (form **S**: the bare application, and a native well-formedness error on that declaration dropped when its `__wf` method reported an error, exit 3 otherwise — a change to the driver, the map and the lint) |
| **(c) `__wf` ghosts** | the function's own `__wf` and a method's: an application is `hva_<m>__<g>_<st>(args)` in the method's ghost domain. The rows of one application in order: the framing rows first, then the pure lines and the value clauses (for `sum_to(a, n + 1)` the failed framing, assumed, ends the path: one error). Facts as `requires` of `__wf`: (i) the result type's range; (ii) the callee's `ensures` at the arguments **under `V`, the callee's value `requires` and pure lines at the arguments** (for a self-application, the function's own `ensures`: the induction hypothesis); (iii) the keep fact, post equals pre, under the guards of a `reads_range` atom and its `ensures` partner that include every interval of the callee. Discriminators: (1) `sum_to(a, n - 1) + a[n - 1]`'s `overflow` row passes with (ii) and fails once without the function's `ensures` (the twin: no bound written); (2) a method's `ensures(r == sum_to(a, n) + 1)` passes with (ii); **(2u) the unguarded form of (ii) is unsound evidence**: a fact is a `requires` of the method and holds on every path, and `sum_to`'s `ensures` at `(a, n - 1)` is false where `n == 0`, so every row is vacuous there (a wrong body passes; kept as `s46_c_unsound_evidence_ensures_*`; under `V` the same body fails alone); (3) `requires(sum_to(a, n) < 100) ensures(reads_range(a, n) && r == sum_to(a, n) + 1)` passes with (iii) and fails once without it; **(4) the unguarded form of (iii) is unsound evidence**: with an `owns_range` alias under a false guard a wrong contract passes (kept as `s46_c_unsound_evidence_keep_*`, as S45 kept its own), and under the guards the same contract fails alone; (5) vacuity | item 8 with facts (i)–(iii) | (1), (2) or (2u) not as expected: **stop and ask** (protected). (3) or (4) not as expected: **cut 1 without asking** (no keep fact; a pinned false alarm, M4.8) |
| **(d) the loop (protected)** | `sum` over `reads_range(a, n)`: item 7's rows in order (the framing `assert` at entry and at the back edge, **the callee's interval inside one place the loop holds**, with no quantifier; then the flag line over the interval in 0061's `q0` form, guarded by the framing; then the line of the callee's value `requires`), then `invariant s == (G ? pf_sum_to(a, i) : 0)`, `G` the same inclusion and value clauses; the body `s := s + a[i]; i := i + 1` with e1a's rows; **the loop's exit to the return's `postcondition` row** `res == (G' ? pf_sum_to(a, n) : 0)`, `G'` built from the `ensures` sites. The same over a local array of 4 in one function. Twins: the invariant `s == sum_to(a, i) + 1` (one error at entry); the invariant `s == sum_to(a, i + 1)` (**two errors expected at entry**, the framing and the invariant's value, as 0061 item 9 has it for a read: question 254's rule); the body adds `a[i] + 1` (`invariant-preserved`); no bound in the function's `ensures` (`overflow` at `s + a[i]`, **and the function's own `overflow` row where its `__wf` is in the fixture: both expected**); **the body writes `a[0]` inside the summed prefix (`invariant-preserved`: the function's value changed)**; a loop that writes `b[i]` of another array keeps the invariant (verifies). **Each positive in form Min and in form Full.** Recorded: `check(sum_to(a, 2) == a[0] + a[1])` with and without the intermediate `check(sum_to(a, 1) == a[0])` (how many written steps a value over known elements needs: the Memory page states it as a limit, and lemmas are M4.5f3's) | items 7 and 9: **unaided**, when the read-only loop and the other-array loop verify and every twin fails once, each within 30 s, in the form (a) builds | the loop verifies only with `check(sum_to(a, i + 1) == sum_to(a, i) + a[i])` written in its body: **stop and ask**, with that result (the choices: the user writes the `check` and the tutorial shows it; or lowering writes that step itself). Not with the `check` either: **stop and ask** |
| **(e) calls and returns** | a caller of `sum`: `x = sum(a, 4); check(x == sum_to(a, 4))` (the method's postcondition, read in the caller's state); a callee with `requires(sum_to(a, n) < 100)`: the `precondition` row after `call_atoms`' rows, the base in a local (S44 F-b1). Twins: the caller writes an element between the call and the `check` (`check`); a caller that cannot prove the bound (`precondition`). In form Min and in form Full: `sum_to(a, n)` read before and after a call that takes and returns `reads_range(a, n)`, and before and after a write to another array (the value is kept: this is the discriminator between the two forms; a failure in both is recorded as a listed limit, not a stop) | item 7's call and return rows | a correct program fails: **stop and ask** |
| **(f) `check`** | on a local array, an `owns_range` and a `reads_range` parameter: 0061 item 10's rows over the callee's interval, written over a `q0` binder with `i = q0 - offset_of(X)` and `0 <= i && i < m` (the form the lint's marked-row rule admits; `X` a variable, another base bound to a local first): null, liveness, bound, the marked rows of `v_T` and `d_T`, the flag row; **then the element-range row** (`internal`: every stored element is within its type; nothing else establishes the callee's range line in code), then the callee's value `requires` (`precondition`), then the fact with the bare application. Twins: a range a callee kept (`permission`); one unwritten element (`uninitialized`: the flag row fails and is assumed, and the element-range row must not then fail beside it — if it does, the row is written `flag ==> range` before the flag row, recorded); a count past a local array (`out-of-bounds`); `n` negative (`precondition`); a false fact (`check`); and the native precondition silent in each | item 7's `check` rows | a native `application.precondition` beside a failed row: the application in a `check` takes form G too (recorded; no stop). A correct program fails: **stop and ask** |
| **(h) a quantified bound** | `int sum_to` with `requires(… && cerastes_forall(small_at, a, n))` (elements within ±1000, `n <= 1000`) and `ensures(-1000 * n <= r && r <= 1000 * n)`: the function (Silicon proves the `ensures` by induction), its `__wf` (the `hvq` clause instantiated at the body's read through 0061's scalar tie; **fact (ii) for the self-application under a `V` that holds the quantified clause over the ghosts at `(a, n - 1)`**), the loop of (d) with the quantified invariant beside it, the application's guard holding the clause as a conjunct (`G && Q ? pf_sum_to(a, i) : 0`). Twins: no quantified `requires` on the function (`overflow`, in the function); **an enclosing contract that applies the function and does not give the quantified clause: one `precondition` row in its `__wf` and no native error** | item 3's quantifier in a `pure` function's contract and item 7's rule for it at an application | a positive failing, a twin with a native error or a second error, or any program over 30 s: **cut 2 without asking** (a `cerastes_forall` in a `pure` function's contract is rejected and pinned, M4.5f1; the `long long` sum stands) |
| **(k) cost** | (d)'s two programs; a function applied in two invariants of one loop; three functions in one contract; S43 (h)'s scale program at K = 16 with one application | recorded | **over 60 s: stop and ask** |

**Stop rules for all of S46:** a second Silicon error after a marked row; a native error on a function other than
the two of (a) in a positive; `qp.not.injective`; an outcome no table names.

## 6. Record 0062 — pure functions (the orchestrator writes it before code from this section; revised only by section 5's tables and audit round 1)

Amends 0006 items 8 and 10; 0010 item 11; 0011 items 4, 7, 9 and 10's messages; 0012 item 11; 0013 items 2, 4, 5
and 8; 0024 item 2 and 0025 item 5 (the exemption of a spec routine's region); 0037 item 7; 0041 item 12; 0042
item 4 and 0044 item 4 (their messages); 0061 items 1, 3, 4, 7 and 11.

1. **Scope:** section 1, with its table.
2. **Which `pure` functions are read** (owner, 2026-11-24). A `pure` function of the unit is **read** when an
   annotation of a verified function calls it, when a read `pure` function's body or contract calls it, or when
   it carries a `requires`, an `ensures` or a `decreases`. A read function is checked once, whether or not anything
   calls it. Every other `pure` function is as before: one that a `cerastes_forall` names is inlined there (0061),
   one that nothing uses is not read (0011 item 4), and the checks `Encoder::run` makes on every spec routine
   (markers, structure values, globals) stay and run first: **a function they reject is not read further**, so it
   keeps its one diagnostic (`reject_owning_global_pure_read.cpp`, the only such function with a contract today).
3. **What a read `pure` function is.**
   - **The function:** defined in a verified file of the unit (a call of one that is not is rejected,
     `unsupported_annotation`, M6); not a `predicate`, a member function, a template, an instance, a lambda,
     variadic, `constexpr` or `consteval`; `check_signature`'s rules; the rejections an ordinary function gets
     before `function_of` (weak, the bodiless `inline` rule). Its result is `bool`, an integer type or an
     enumeration; every parameter is a `bool`, an integer, an enumeration, or a pointer to one of those or to a
     pointer (0061 item 3's types). A function that is not such a function: **`pure_function`** (new tag; its
     message says which rule), at the call, or at the function when only its contract makes it read.
   - **The body** is exactly one `return E;` (an extra block is read through, as e1c's reader does). `E` holds
     what an annotation expression may hold; element reads `p[e]` and `*p` through the function's pointer
     parameters; calls of read `pure` functions, itself included. Refused in `E`, each with the tag the construct
     has in an annotation and a pin: an atom, a `cerastes_forall`, `cerastes_old` (a function has one state), an
     assignment, `++`, `--`, a comma, a global or `static` local, `&` of a parameter, a call of anything that is
     not a `pure` function. The returned value is converted to the result type (in C a `bool` result is `E != 0`,
     P80 (c)); **that conversion has its own obligation**.
   - **The shape rules and the static assertions reach it:** R1–R13 run in `E` as in any expression (it is
     encoded through `expr()`); the region of a read `pure` function is no longer exempt from
     `check_block_static_assertions`, and a block-scope `static_assert` in it is rejected (`unsupported_declaration`,
     0024 item 2's message) unless P81 (e) shows the reader can pass it to `encode_block_static_assertion`. The
     region of an unread `pure` function and of a `predicate` stays exempt (`static_assert_pure_body.c` is
     unchanged).
   - **Its contract.** `requires`: value clauses, and the atoms `cerastes_reads(*p)` and `cerastes_reads_range(p,
     n)` on its own pointer parameters, each a top-level conjunct with no guard, its count reading no memory; an
     unmarked atom promises initialized elements, as on any function (0042). **A value clause of the `requires`
     reads no memory** (`*p > 0` is rejected, `unsupported_annotation`, M4.5f1: the clause is lowered at every
     application over the arguments, which the heap-free idiom does only for a clause that reads none); the one
     clause that may read memory is a `cerastes_forall` (cut 2; item 7 says how it is lowered at an application).
     `ensures(r, …)`: value clauses over the parameters, the result and reads the `requires` atoms frame; no atom
     (the function takes nothing and returns nothing: it reads); no `old`. `decreases`: as 0013, reading no
     memory and calling nothing. A
     `cerastes_forall` may stand in its `requires` and `ensures` (S46 (h); cut 2), as in any contract, and so may
     a call of another read `pure` function (cut 4), as long as no function reaches itself through a contract
     (item 10). The rest of section 1's table is rejected.
4. **Applications.** A call `g(e_1, …, e_m)` of a read `pure` function is an **application**. It may stand in an
   ordinary function's `requires` and `ensures`, in a loop invariant, in a `check`, and in a read `pure`
   function's body, `requires` and `ensures`; anywhere an expression of that annotation may stand, under `!`,
   `||` and `?:` too (it is a value, not a quantifier). Every other position is rejected (section 1's table):
   `unsupported_annotation`. `call_in_annotation` stays for a callee that is not a `pure` function; its text
   becomes "only `cerastes::old` and `pure` functions can be called here".
   - **An application counts as a read of memory** (`reads_heap`), whatever its callee reads: where a rule asks
     that an expression read no memory (an atom's count or guard, an element's index, a `forall`'s trailing
     argument) it is rejected, and **`old` over an application is rejected** (today `old` wraps only what
     `reads_heap` sees, so an application under it would silently be read in the post-state; pinned in C++,
     where it compiles).
   - **Arguments** are expressions of the site, each evaluated once, at the site, in the site's state, with its
     own obligations (0010 item 11's; a read's framing, bound and flag) as rows of their own before the
     application's, in evaluation order. An argument may read memory (cut 3).
   - **A pointer argument that an atom of the callee names** is, in a contract or a `pure` function, a bare
     pointer parameter; in a loop invariant or a `check`, a pointer variable that is not address-taken or a named
     local array (0061 item 3's rule, for the same reason: the rows' trigger terms name a variable).
   - **Meaning:** the value of `g`'s `E` with its parameters replaced by the argument values, read in the site's
     state.
5. **VIR.** `Function::pure` and `Function::pure_body` (the encoded `E`, over `Formal`s, a pre-state contract
   expression); a read `pure` function has no statement body and is neither a verified nor a trusted method:
   each place that reads `body == nullptr` as "trusted" gets the `pure` case (`check.cpp:847`, lowering's
   `function()` and `run()`, `verified()`). `Expr::Kind::Apply` (the callee, the encoded arguments, the site's
   spelling as `c_text`); `Builder::apply`; `reads_heap` is true for it. `Kind::Call` stays the code call.
   `check.cpp` backstops items 3 and 4: positions, a callee that is `pure` with a body, arity and types against
   the callee's parameters, no `Apply` under a `Bound`'s `Forall`, in a measure, under `Old`; no `Old`, atom,
   `Forall` or write in a `pure_body`; the atoms a `pure` function's `requires` may hold; **the graph of
   applications over bodies and contracts has no cycle but a function's application of itself in its own body**
   (lowering has the same backstop over the emitted functions, as `check_cycles` has for methods).
6. **The Viper function** (lowering; a new `LFunction` in `LProgram::functions`; the name `pf_<name>__<id>`).
   - **Formals:** the parameters, as a method's, with no ghost amount.
   - **`requires`:** the parameters' type ranges; then the `requires` clauses in order (`push_clauses`), a value
     clause as a method's contract has it, an atom in the **minimal shape** (S46 (a), form **Min**): for a range
     atom its pure line, `forall k: Int :: { loc(block_of(p), k) } in-range ==> acc(loc(block_of(p), k).v_<t>,
     wildcard)` and the element-range quantifier; for a cell atom the same three lines for one slot. **No `acc`
     of the flag field `d_<t>` or of the liveness cell `lv`, and no flag line:** every `acc` line widens the
     snapshot the function's value depends on, and the flags and the liveness are explicit rows at every
     application (item 7). (Form **Full**, a method's lines for the atom with `wildcard` for the amount, is the
     fallback of S46 (a).) Each line has its `internal` map row (`not.wellformed`).
   - **`ensures`:** the user's clauses, one line each, `result` for the result name. **No line states the
     result's type range** (Silicon would have to prove it, and a body that overflows would fail it beside the
     `overflow` row; callers learn a bound from the function's `ensures`, and `__wf` methods assume the range,
     item 8).
   - **`decreases`:** the user's measure, or an empty `decreases` for a function that does not call itself.
     Never `decreases _`, never no line.
   - **The body:** `E` in the function's own contract form: a read is its `ite` chain over the function's
     `requires` sites (0037 item 7, 0060 item 5), an application is guarded by `G` (item 7), every application of
     the function itself begins a line of its own (the map is keyed by line: two on one line could not carry two
     positions).
   - **One state.** The function's `ensures` and body are lowered as its `requires` are, in the entry state with
     every `requires` site in view (the way a measure is lowered today); the contract's lowering context writes
     `wildcard` for an amount and leaves out the liveness `acc` (a flag on the context, not a new contract mode).
7. **Rows at an application, by mode** (0028 item 11: every obligation is an explicit row in source terms; the
   callee's Viper precondition is only the `internal` backstop). **The framing rows of the callee's atoms come
   first, then the atoms' pure lines and the value clauses of its `requires`**, each with the arguments
   substituted (a failed framing, assumed, then ends the path of a row that depends on it: one error):
   - **The guard `G`** of an application is the conjunction of (i) for each atom of the callee, the disjunction
     over the enclosing contract's unmarked sites of that cell field of "the site's guard, the same block, and
     the callee's interval inside the site's" (a cell atom is an interval of one slot; **one site must hold the
     whole interval**: a listed limit; a marked site frames no application, as 0061 item 7 has it for quantified
     reads), and (ii) every other line of the callee's Viper precondition that reads no memory, at the arguments:
     the parameters' ranges, each atom's pure line, each value clause. The callee's element-range line follows
     from (i): a site that frames an application promises it. **One helper computes `G`'s conjuncts for the guard
     and for the rows**, so the guard can never be stronger than what the rows assert.
   - **A `cerastes_forall` clause of the callee's `requires`** (cut 2) is, at an application, **the quantifier it
     would be if it were written at the site with the arguments in place of the parameters**: lowered by 0061 in
     the site's mode (a contract's quantifier with its `ite` chains; in `f__wf` over the enclosing method's `hvq`
     ghosts; in a loop invariant and a `check` with 0061 items 9 and 10's rows), asserted there as one
     `precondition` row, and standing as one more conjunct of `G` where the application is guarded. Its trailing
     arguments must then be what 0061 item 3 admits (an application that passes an argument that reads memory to
     such a parameter is rejected, M4.5e2). In fact (ii) of item 8, `V` holds the clause as `f__wf` lowers it at
     the arguments. S46 (h) gates all of it.
   - **A method's contract (`CMode::Method`), a `pure` function's body and contract:** the application is `(G ?
     pf_g(args) : d)`, `d` the type's default, so that Viper's own check of the contract or the function can
     never fail; the rows are the `__wf` method's (item 8): per atom one `permission` row ("the contract of `f`
     calls `g`, which reads `a[0]` to `a[n - 1]`, without owning them"), per value clause one `precondition` row
     ("the contract of `f` calls `g`, whose `requires` may not hold: `0 <= n`", with a note at the callee's
     clause).
   - **A call and a return** (`CMode::Call`, `CMode::Ret`): the clause's `precondition` or `postcondition` rows
     as today (one per clause, per conjunct where `push_clauses` splits): the clause is lowered again over the
     arguments, so the application inside takes the form the method's contract gives it, read in the caller's
     state after `call_atoms`' rows (it writes no quantifier there, so no base needs binding). Callers learn an
     `ensures` from the method's postcondition, as today.
   - **A loop invariant (`CMode::Inv`):** before the user's clause, in evaluation order, the framing of each
     callee atom: **its interval lies inside one place the loop holds** (a range place or an array place; no
     quantifier), asserted at entry and at the back edge as a read's framing is (`permission`, "this loop
     invariant calls `g`, which reads `a[0]` to `a[i - 1]`, which this loop does not hold"); then one
     `obligation_clause` invariant line per obligation: a named array's bound (`out-of-bounds`), the flags over
     the interval in 0061's `q0` form, guarded by the framing (`uninitialized`), each value clause of the callee
     (`precondition`); the application in the clause is guarded by the same inclusion and value clauses, as
     `inv_elem` guards a read. A loop whose invariant holds an application of a function with an atom carries its
     places (`expr_touches`).
   - **A `check` (`CMode::None`):** per callee atom the rows of 0061 item 10 over its interval, in the binder
     form the lint admits (the null row and the liveness row for a pointer base, the bound, the marked rows of
     `v_T` and `d_T`, the flag row), then the element-range row (`internal`), then one `precondition` assert per
     value clause, then the fact with the bare application (a failed row is assumed afterwards, so the native
     check is silent; S46 (f)).
   - **What establishes each line of the callee's precondition, by mode** (a line with no entry would be an
     unmapped native error, exit 3):

     | Mode | pure lines, value clauses | `acc` of `v_T` | element range |
     |---|---|---|---|
     | a `pure` function's own body | `G` | its own `requires` | its own `requires` |
     | a method's contract | `G` | the framing site (unmarked) | the site's range line |
     | a call, a return | `G` over the arguments | `call_atoms`' and the return's marked rows | their range rows |
     | a loop invariant | `G` over the places | the place's conjuncts | the place's range conjunct |
     | a `check` | the `precondition` rows | the marked rows | the `internal` row above |
8. **`f__wf`** (amends 0037 item 7 and 0061 item 7; variant M stays: the method is heap-free and applies no
   function). An application is a ghost function of the method's domain, `hva_<m>__<g>_<pre|post>(args)`, one
   parameter per C parameter (`Int`, `Bool`, or `Ref` for a pointer) and the callee's result type; the domain
   keeps its tag and holds no axiom (a method with an application and no quantified read has the domain too).
   Assumed as `requires` of the `__wf` method, each ground at one application the contract writes:
   - **(i) the result's type range** (a `bool` none), unguarded: the definition check of `g` (item 9) shows that
     no operation of its body leaves its type, so the value is one of its result type wherever the application
     is defined, and the fact is satisfiable everywhere;
   - **(ii) the callee's `ensures`** at the arguments, **under `V`, the callee's value `requires` and pure lines
     at the arguments** (a quantified clause of the callee, cut 2, as item 7 lowers it over the ghosts), when that
     `ensures` clause reads no memory and holds no quantifier and no application (otherwise
     it is left out: a false alarm, never a proof). **The guard is what makes it sound:** a fact is a `requires`
     of the method and holds on every path, and an `ensures` at arguments outside the callee's `requires` may be
     false (`sum_to`'s bounds at `n - 1` where `n == 0`), which would make every row vacuous (S46 (c) (2u)). Under
     `V` it is a theorem: where `V` holds a heap satisfying the callee's full precondition exists, Silicon proved
     the `ensures` there, and the clause reads no memory. For an application of the function inside its own body
     this is the induction hypothesis, sound because the function's termination is checked (item 10);
   - **(iii) the keep fact** (cut 1): for a post-state application whose arguments read no memory and whose
     every callee interval lies inside a `reads` or `reads_range` atom of the method that has an `ensures`
     partner, under both atoms' guards and the inclusion, `hva_…_post(args) == hva_…_pre(args)` (the function's
     value depends on its arguments and on the cells its atoms name, and a `reads` place is not written between
     entry and return: 0061 item 7 (d)'s argument).
   - Two applications of one function in one state with equal arguments are equal by the ghost function itself.
   - Rows: `prior ==> conjunct` for each conjunct of `G`, in the categories of item 7. A failed row is assumed
     afterwards, as every `f__wf` row is. **Listed limits** (false alarms, fail closed): the definition of `g` is
     not known in `f__wf` (an obligation that needs it, such as `sum_to(a, 2) - a[0] - a[1]`'s overflow under a
     bound on the elements, fails); an `ensures` of `g` that reads memory is not assumed.
9. **The definition check of a read `pure` function.**
   - **Its `__wf` method** (`wf_method`, extended): the `requires` clauses; then **the body as one more clause of
     the entry state**: every obligation of `E` (0010 item 11's, each read's framing by the function's own atoms,
     each application's rows by item 7 with the ghosts of item 8, the conversion at the `return`), reported **in
     the function, at the operation**; then the `ensures` clauses' own obligations, read in the same state (a
     `pure` function has one state: no post ghost, no partner row, no keep fact); then the measure's (0013 item
     7).
   - **Silicon's own check of the Viper function** is the proof of its `ensures` and of its termination. These
     two are reported to the user from it (item 11); nothing else of that check can fail in a program whose
     `__wf` rows hold, and anything else it reports is an internal error (exit 3).
   - **Why the function's value is the C value** (the record argues it; audit round 1 checks it): by induction on
     the measure. The Viper body computes in unbounded integers; the `__wf` rows show, under the `requires` and
     with every inner application's `ensures`, that no operation of `E` overflows, divides by zero, converts out
     of range or reads what it may not; so the two values agree, given that they agree for every inner
     application, whose measure is smaller.
10. **Termination** (amends 0013 items 2, 4, 5 and 8). The encoder records the calls among read `pure` functions,
    from bodies and from contracts (`edges_` holds code calls only). A function that calls itself in its body
    needs a `decreases` (`missing_decreases`, its message for a `pure` function); **every other cycle is
    rejected**: two or more functions, or a function reached again through a `requires` or an `ensures`
    (`unsupported_annotation`, M4.5f2: a cycle through a contract would escape the termination check, and the
    facts of item 8 would rest on themselves). Silicon's termination plugin checks the Viper function against
    its measure; a failure is `termination`, at the recursive call, with a note at the measure ("the measure of
    `g` may not decrease at this call, or may not be bounded below by zero"). An ordinary function's recursion
    cycle (0013 item 4) is unchanged: an application is not a call of a method.
11. **The two native rows.** On each `ensures` line of a Viper function: `postcondition.violated` →
    `postcondition`, at the function's `return`, with a note at the clause ("`g` may not satisfy its `ensures`").
    On each line of its body that begins with an application of the function itself: `termination.failed` →
    `termination` (item 10). Both through the existing reasons (`assertion.false`; the three termination
    reasons); `report.cpp` and the map's format do not change. Every other row on a function is `internal`.
12. **The lint** (amends 0012 item 11, 0061 item 11).
    - A top-level `function` is a prelude function (by name, with its fixed text) or `pf_<name>__<id>`: bodied;
      one `decreases` line, with a measure or empty, never `_` or `*`; `requires` lines in exactly the shapes of
      item 6 (the form S46 (a) built; `wildcard` is admitted there and on 0061 item 10's marked row, nowhere
      else); `ensures` lines hold no `acc`, `perm`, `old`, `inhale`; the body holds no `acc`, `perm`, `old`,
      `forperm`, `inhale`, `assume`.
    - `pf_` applications: in a `pf_` function, a method's `requires` and `ensures`, an `invariant`, an `assert`;
      **never in a `__wf` method**, never in a `decreases` line, never in the bounds, base, amount or guard
      positions `_heap_free` protects: **`HEAP_READ_RE` counts a `pf_` application as a heap read** (handoff §3,
      e1a round 2 item 6).
    - The ghost domain of a `__wf` method may hold `hva_<method>__<callee>_<pre|post>` functions of `Int`, `Bool`
      and `Ref` parameters; the assumed facts over them have exactly the shapes (i)–(iii) of item 8, the
      `ensures` fact under `V` and the keep fact under its guards (each unguarded form is a must-fail self-test,
      as 0061's unguarded keep fact is).
    - Map rows of a function: each `requires` line `not.wellformed` (`internal`); each `ensures` line
      `postcondition.violated` (a user row) and `not.wellformed` (`internal`); its `decreases` line with a
      measure keeps the `termination.failed` entry today's rule asks for (at the function's `return`; Silicon
      does not report there, S46 (a)); each body line that begins with an application of the function itself
      needs the `termination.failed` entry of that call, and no other body line may have an entry. Every
      `pf_` function has a `decreases` line (today's rule reads methods and loops only).
    - A user quantifier of 0061's shape is admitted in a `pf_` function's `requires` and `ensures` (today's
      rules read methods only) and, for a callee whose `requires` holds one (cut 2), as a conjunct of an
      application's guard and, over the ghosts, in the antecedent of fact (ii).
    - Each rule with twins.
13. **Categories:** none new. **Tags:** `pure_function`. **Facts:** P81, S46.

## 7. Parts, owners, order

| Part | Content | edg-backend | il-analyst | test-mutator |
|---|---|---|---|---|
| **A** plumbing, behaviour-neutral | VIR and lowered-program plumbing; nothing accepted yet | `Function::pure`, `pure_body`; `Expr::Kind::Apply` and `Builder::apply`; every kind switch (`same_pure_expr`, `reads_heap` true, `guard_ok`, `expr_touches`, the sequencing walkers, the printers; `check.cpp`: refused everywhere, and the `pure` case of its `body == nullptr` rule); `LFunction`, `LProgram::functions`, `LGhostFn` with typed parameters; `emit.cpp`'s function printer with map rows; `src/driver/selftest.cpp`'s hand-built program | il-analyst-1: P81, S46 | Step 0 (R1–R3, M, R2's pins); the lint rules of item 12 against S46's fixtures, with twins |
| **B** the function, and its use in a `check` | items 2, 3, 5, 6, 9, 10, 11; items 4, 7 and 8 for a `pure` function's own body and contract and for a `check` | the set of read functions; the reader generalized from `read_forall_function` (any scalar result, a contract allowed, the body encoded over `Formal`s); `function_of` and `encode_contract` for a read `pure` function, with item 3's contract rules; the calls among `pure` functions and `missing_decreases`; `Apply` in `call()` inside a `pure` body or contract and under `cx_.check_clause`; the static-assertion region rule; `pure_function` in `diagnostics.def` and the new text of `call_in_annotation` | **il-analyst-2**: `check.cpp`'s rules (item 5, the application graph); lowering's dispatch for a `pure` function (`run()`, `function()`: neither a verified nor a trusted method); the Viper function (item 6) with its two native rows; `wf_method` for a `pure` function (item 9) with the `hva` ghosts and facts (i), (ii); the interval-inclusion helper and the one helper for `G`; `Apply` in `CMode::None` (the `check` rows); the backstop over the emitted functions' graph; VIR cases in the session's case file | Part B's tests; R2's `check` pin retired |
| **C** applications in contracts | items 4, 7 and 8 for an ordinary function's `requires` and `ensures` | `Apply` in `call()` under `cx_.contract` for an ordinary function; the argument rule for contracts | **il-analyst-3**: `Apply` in `CMode::Method` (the guarded form), `CMode::Wf` (post-state ghosts, fact (iii)), `CMode::Call` and `CMode::Ret` (the clause lowered again over the arguments) | Part C's tests; R2's `requires` and `ensures` pins retired |
| **D** applications in loop invariants; the exact sum | items 4 and 7 for a loop invariant | `Apply` in `call()` under `cx_.loop_invariant`; the argument rule for locals and arrays | il-analyst-3: `Apply` in `CMode::Inv` (the framing, the obligation lines, the guard), `expr_touches` | Part D's tests, the exact sum, the doc and tutorial examples; R2's invariant pin retired |

**Staging pins:** R2's four pins are the staging pins. After Part B a call of a `pure` function in an ordinary
function's `requires` or `ensures` and in a loop invariant is still rejected (`call_in_annotation`), so
`pure_call_reject_{requires,ensures}.c` stand until Part C and `pure_call_reject_invariant.c` until Part D;
`pure_call_reject_check.c` is retired by Part B.

**Order of work.**
1. Step 0 and Step 1. Then, in parallel: il-analyst-1 on P81 and S46; edg-backend on Part A; the test-mutator on
   R1–R3 and M, then the lint rules; the orchestrator on record 0062's draft.
2. **Commit 1:** record 0062 (draft, revised by section 5's tables), the `## M4.5e1d` chapter, the S46 fixtures,
   R2's pins, the lint rules. Advisor checkpoint 1, then audit round 1 on it. il-analyst-1 ends.
3. **Part A** may be built before round 1 returns (it accepts nothing). It is proved neutral (`vprdiff`: every
   `.vpr` and `.map` identical) and joins Part B's commit.
4. **Part B starts only when audit round 1 has returned and the record holds its blocking fixes** (invariant
   6). edg-backend and il-analyst-2 work in parallel against Part A's VIR; the test-mutator writes the tests from
   the record, expectations pre-registered. Audit round 2 on the snapshot. **Commit 2** (Parts A and B). **Split
   point 1.**
5. **Part C:** il-analyst-3 on the lowering; edg-backend on the wiring. Audit round 3 on the snapshot.
   **Commit 3.** **Split point 2.**
6. **Part D:** il-analyst-3, then edg-backend's part. Audit round 4 on the snapshot, with the pages. **Commit 4.**
7. The doc and tutorial examples (test-mutator); the user pages and the tutorial section (orchestrator);
   `internal/encoding.md` and the developer guide (edg-backend); the re-audit of blocking fixes; the checks of
   section 11; record 0062 accepted; status, handoff, `plan.md`'s Done row. **A session that ends at a split
   point still does this step for what it built** (invariant 8: the pages for what landed, the re-audit, the
   acceptance of section 11 with its one box run, the handoff naming the parts left).

**Proofs, per part.**
- **Each half alone:** the il-analyst proves its half by the new VIR cases through `cerastes-vir-selftest` and
  Silicon, which need no encoder; edg-backend proves its half by its build and by R3's run of the rejections
  against section 9. Both run `scripts/format.sh`.
- **The joined proof**, once both halves have reported: edg-backend makes the joined build, runs
  `scripts/regen_vpr.py` into a directory of its own and `scripts/vprdiff.py` against the baseline (expected in
  every part: no existing program's `.vpr` or `.map` differs); then, with builds held, the test-mutator runs the
  part's tests once. A failure goes through the orchestrator to the half that owns the failing file; the joined
  proof is repeated once, after the last fix.
- The audit snapshot is taken from the tree the joined proof passed on.

**Audits** (one resumed Opus reviewer; a snapshot commit and a copy of the binaries built from exactly it):
- **Round 1, the record with P81 and S46:** reproduce the gates; item 9's induction (can a function that does
  not terminate, or whose body is ill-defined, still give Silicon an axiom that a verified program uses; can a
  function's `ensures` be used in its own proof other than through a smaller application; **a cycle through a
  contract**); the facts (i)–(iii) over the ghosts (the state of each ghost; **fact (ii) outside the callee's
  `requires`**, S46 (c) (2u); the keep fact's guards; a fact about an application whose rows failed); **the
  guard: can the default value be observed in a verified program**, and can `G` be stronger than the rows; the
  `wildcard` precondition (can a site with no permission apply the function); **`old` over an application, and
  an application where a rule asks for "reads no memory"**; can another native error hide behind the two rows
  of item 11; the precondition table of item 7 (a line no mode establishes); a `pure` function that is read by
  the contract rule but that the record lets escape a check.
- **Round 2, Parts A and B:** a function read that item 3 refuses (bodiless, a cycle, `constexpr`, a body with
  `cerastes_old`, a global, a marker); a body construct read but not checked (a shape rule, a static assertion,
  the conversion at the `return`, a side effect); an application whose callee `requires` has no row; a
  self-call in a nested expression that `missing_decreases` misses; an argument read twice or in the wrong
  state; the two native rows' positions; the lint (a function with no `decreases` line, `decreases _`, a bodiless
  function, `wildcard` elsewhere, an application counted heap-free, a `pf_` application in a `__wf` method);
  Part A's neutrality; every other function's `f__wf` unchanged.
- **Round 3, Part C:** each conjunct of `G` in `CMode::Method` against its `__wf` row; post-state and pre-state
  ghosts; fact (ii) at arguments that read memory; fact (iii) against S46 (c) (4); the `precondition` row against
  the callee's clause; a trusted (bodiless) contract that applies a function and lets `assert false` verify
  after a call; a contract without an application keeps its method and `__wf` byte for byte.
- **Round 4, Part D (with the pages):** the invariant's rows at entry and at the back edge; **a loop that writes
  inside the callee's range and keeps `s == g(a, i)`**; a loop that should carry the callee's places and does
  not; an element a callee kept passing the framing; the pages' statements about what is checked and trusted.
- **One re-audit** of blocking fixes. Should-fix items not taken go to handoff §3.

**Pre-agreed cuts, in this order** (owner, 2026-11-24, question 272: each is in scope and is cut only by its
spike's table, by an audit finding, or at a split point; each leaves its pin, re-pointed; the tutorial and the
pages say what landed): (1) the keep fact (iii) in `f__wf` (M4.8); (2) a `cerastes_forall` in a `pure`
function's contract, with the `int` sum (M4.5f1); (3) arguments that read memory (M4.5e2); (4) applications in
a `pure` function's own `requires` and `ensures` (M4.5f1); (5) an application in an ordinary function's
`requires` (the `precondition` rows at calls; M4.5e2). **Not cut without asking:** the Viper function with its
contract and its definition check, self-recursion with `decreases`, applications in a `check`, in an `ensures`
and in a loop invariant, the `long long` exact sum with its twins, and the tutorial section.

**Split points** (owner, 2026-11-24, question 271: one session, and the first of these is expected to be
reached). At Commit 2 and Commit 3 the orchestrator reports `git diff --numstat <base> -- src/ include/` without
the self-tests against section 1's sizes, the hours used against the twelve-hour cap (question 77) and what is
still owed. **Stop and ask** if Parts A and B passed 3,200 lines, if Parts A–C passed 4,200 (the tops of
section 1's estimates, question 272's constructs included), if the remaining
parts with their audits, the docs and the box run do not fit the cap, or if the orchestrator and the advisor
judge that its context will not carry the next part. The owner then decides between continuing, a cut, and a
session of its own for the rest (M4.5e1d2: R2's staging pins make the tree at either commit a closed state,
green on its own; that session would start from this plan's Parts C and D with no second planning, and Commit
2's or Commit 3's docs say what has landed).

## 8. Tests (`tests/memory/pure_*.c`, scalar-only ones in `tests/scalar/pure_*.c`, negatives in the area's `negative/`; `.c` in `c23, c++20`; each positive has a twin failing for one reason at one line)

- **Part B.** A non-recursive function without a contract (`is_small`) used in a `check` (twin: a false fact,
  `check`). A function with a value `requires` (twin: a `check` that applies it outside the `requires`,
  `precondition`, with the note). A wrong `ensures` (`postcondition`, at the function's `return`). An overflow in
  a body (`overflow`, at the operation, in the function) and the same body under a `requires` that excludes it.
  A narrowing at the `return` (`conversion`). A scalar recursion (`tri(n)`, the sum 0 + … + n) with
  `decreases(n)` and a bound `ensures`; twins: the self-call with `n` (`termination`), a measure that may be
  negative (`termination`), no `decreases` (`rejected missing_decreases`). A function over `reads(*p)` and one
  over `reads_range(a, n)` applied in a `check` on a local array, an `owns_range` and a `reads_range` parameter;
  one twin per row of S46 (f). A function that calls another in its body (its `precondition` twin), with an
  argument that reads memory (`is_small(a[n - 1])`, unless cut 3), and one that calls another in its `requires`
  (unless cut 4). A read in a body that the function's atoms do not frame (`permission`, in the function).
  The cycles: two functions calling each other, and a function reached again through its own `ensures` (both
  rejected). `cerastes::old` over a call in a `.cpp` (rejected); a call in an atom's count and in an element's
  index (rejected).
  A function with a contract that nothing calls (it is checked: a twin with a wrong `ensures`). A function that
  a `forall` names and a `check` also calls. A shape rule in a read body (R5 or R9) and a `static_assert` in one.
  One pin per rejection of items 3, 4 and 10 and of section 1's table; the new tag; both modes. **A twin whose
  failing row is inside a `pure` body gives that function no `ensures`, or one that holds for any value** (the
  guarded body takes its default where the row fails, and a real `ensures` may then fail beside it: S46 (a)).
- **Part C.** `ensures(r == twice(x))` with a body that proves it (twin: a wrong body, `postcondition`). A
  callee with `requires(sum_to(a, n) < 100)` and its caller (twin: `precondition`). A bodiless (trusted)
  function with `ensures(r == sum_to(a, n))` and a caller that uses it; **the default is not observable** (the
  caller's `check(r == 0)` fails). An application the contract does not frame (`permission`, one error; S45's
  twin). A callee value `requires` the contract does not give (`precondition`, one error). A later clause resting
  on the callee's `ensures`: `sum_to(a, n) + 1` verifies; the one-error twin uses a callee whose own body needs
  no bound (`clamp100(x) + 1` with and without `clamp100`'s `ensures`: `overflow` in the contract alone), since
  `sum_to` without its bound also fails its own `overflow` row. The keep fact
  (unless cut 1), with its twin over `owns_range`. An `ensures` application after a write (the body writes
  `a[0]`, then computes the sum: verifies; computing it before the write: `postcondition`). An argument that
  reads memory (unless cut 3).
- **Part D.** `pure_sum.c`, the exact sum: `CERASTES_PURE long long sum_to(const int *a, int n)` with
  `requires(0 <= n && cerastes_reads_range(a, n))`, `ensures(r, -2147483648LL * n <= r && r <= 2147483647LL *
  n)`, `decreases(n)` and `return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];`, and `long long sum(const int *a,
  int n)` with the invariant `0 <= i && i <= n && s == sum_to(a, i)` and `ensures(r, cerastes_reads_range(a, n)
  && r == sum_to(a, n))`. Twins: the invariant `s == sum_to(a, i) + 1` (`invariant-entry`); the invariant with
  `i + 1` (two errors at entry, `permission` and `invariant-entry`: both expected, S46 (d)); the body adds `a[i]
  + 1` (`invariant-preserved`); the function without its bound (two errors, both expected: `overflow` in the
  function and `overflow` at `s + a[i]`); **a loop that writes `a[0]` inside the summed prefix**
  (`invariant-preserved`); a loop that also fills another array (verifies). The sum over a local array
  in `main`. `pure_sum_int.c`, the `int` form with a quantified bound (unless cut 2). A caller of `sum`. An
  application the loop does not hold (`permission`, at the invariant); an unwritten element under the
  application (`uninitialized`). The exact count over a `bool` array (`pure_count.c`, the sibling of
  `array_loop_bool_post.c`), as far as S46 (d)'s recorded row allows (with the intermediate `check`s it needs,
  or not written if it needs a lemma: then noted for M4.5f3).
- **VIR self-test:** the session's cases in the next free case file; `VIR_CASES_MIN` raised.
- **Byte identity:** `scripts/vprdiff.py` against the baseline after each part: no existing program's `.vpr` or
  `.map` changes in this session.

## 9. Flips (pre-approved with this plan; any other changed outcome stops the session)

- **A. Retired for a positive of the same program, with a twin:** only the pins R2 adds in Step 0
  (`pure_call_reject_{requires,ensures,invariant,check}.c`), each by the part that lifts its position; a pin
  whose position a cut leaves rejected stays, re-pointed.
- **B. Stay as written; only the message or the `Pins:` comment changes:** `forall_reject_f_{requires,ensures,
  decreases}.c` (a `forall` naming a function with a contract stays rejected: M4.5e2; question 265);
  `forall_reject_e_call.c` (a call under the binder: M4.5e2); `reject_call_in_annotation.c` (the tag's new
  text); the `pure` rejections of `tests/memory/negative/` whose messages say "M4.5f1 encodes the bodies"
  (`reject_struct_value_pure.c`, `global_pure_write.cpp`, `static_local_pure.cpp` and their source messages);
  `array_loop_bool_post.c` (its comment names its sibling, if written); `static_assert_pure_body.c` and
  `tests/mutants-equivalent.txt:661` (their words "not encoded before M4.5f" become "not read: nothing calls
  it"; the program, the expectation and the entry's mutant are unchanged — **if the mutant no longer survives,
  stop and ask**).
- **C. Lint self-tests** that pin "this `function` or this `wildcard` is refused" for a shape item 12 now
  admits: the test-mutator reports which before editing, and status lists them.
- **D. Front-end tests, goldens and existing Viper:** none (`include/cerastes.h` changes in comments only:
  **if a golden changes, stop and ask**; the `tests/annotations/**` programs with contracts and calls on `pure`
  functions are not on the verify path). The new text of `call_in_annotation` changes no expectation: at
  `9ab9a1e` its words stand only in `diagnostics.def`, and the four tests that name the tag
  (`reject_call_in_annotation.c`, `reject_atom_in_check.c`, `forall_reject_e_call.c`,
  `forall_reject_namespace_check.cpp`) name the tag alone.

## 10. Docs (invariant 8; every example is a test)

- **`annotations.adoc`:** the IMPORTANT box (9–19: calls of `pure` functions are checked now; predicates, `fold`
  and `unfold` are not); "Predicates, pure functions" (759–783) rewritten for `pure`: which functions `cerastes
  verify` reads, what the body and the contract may hold, where a call may stand, what is checked (the body's
  operations, the `ensures`, termination) and reported where; the `forall` section's sentences on contracts
  (918–939: still rejected there, and why); `decreases` (685–740: a recursive `pure` function); 1075–1079 (the
  calls an annotation may make); 1309–1316.
- **`memory.adoc`:** "Arrays" gains "A value computed from an array" (a `pure` function over `reads_range`, the
  exact sum, what a loop must hold to call it, what `cerastes verify` does not compute by itself); the not-yet
  lists (728–741, 1780–1856) gain section 1's table rows a user can meet.
- **`verifying.adoc`:** 108–111 and 784–786 (read `pure` bodies are verified; which are read); 259–276
  (recursion: a `pure` function's measure); 1088–1089 (static assertions); 1299 (the not-supported list).
  **`error-messages.adoc`:** `precondition`, `postcondition`, `termination`, `permission` and `overflow` gain the
  forms a `pure` function gives them (an application's row with its note; an error reported in the function).
  **`overview.adoc`** where it lists what is verified.
- **The tutorial** (0049 item 4): `pages/tutorial/arrays.adoc` gains "The exact sum" before "What is next" (the
  function, its contract and measure, the loop and its invariant, the off-by-one variant, what the bound in the
  `ensures` is for); examples `examples/tutorial/arrays_sum.c` and `arrays_sum_offbyone.c` ("Derived from"), the
  `pure` function named so that it does not read as `loops.adoc`'s executable `sum_to`; `console` blocks copied
  from runs; "What is next" and `tutorial/index.adoc` updated.
- **`include/cerastes.h`**'s comments (`CERASTES_PURE`); **`tests/README.md`** (the tag; the native rows of a
  function); **`internal/encoding.md`** §2 (`Apply`, `pure_body`), §3 "Pure functions (M4.5e1d)", §4 (the
  function's shape and names), §5 (the two rows), §6, §8; the developer guide where it names `f__wf` and the
  prelude; one amendment line in each amended record.

## 11. Stop and ask; acceptance

**Stop and ask:** any agent question; any changed outcome outside section 9; any existing program's Viper or map
changing; a P81 or S46 outcome outside section 5's tables; a cut beyond section 7's; a split point's threshold;
an exit 3 or a time over 120 s not fixed within its part. **While the owner is away, the session waits at a
question** (owner, 2026-11-24: "If you or any of the implementers have questions stop and ask me"); M4.5e1c's
overnight rule, building the recommended answer and logging it, applies only if the owner gives it again for this
session.

**Acceptance.**
- Before the box run, on the final build: `scripts/regen_vpr.py`, `scripts/lint_viper.py` over its output and
  over a fresh VIR self-test directory, the session's VIR cases through Silicon, `scripts/rejections.py` against
  section 9, `scripts/vprdiff.py build/m4.5e1d/baseline/vpr <regen>` with no existing program differing. Advisor
  checkpoint 5.
- `CERASTES_LONG=1 scripts/check.sh` **once**, after the last code change, on the x86-64 box (the branch as a
  git bundle into `~/cerastes-s8/p-ubuntu-26.04`, `~/cerastes-s8/run.sh update` then `check`): green.
- `test_lint_viper.py`, `test_run_tests.py` (the tag-coverage test names the new tag), `run_tests.py
  --check-headers`, `scripts/format.sh --check`, `scripts/build_docs.sh`, `check_doc_examples.py` (source and
  `--html`).
- The status entry (sizes measured, deviations, corrections under question 160, each cut taken) and the handoff
  with its Questions section and what e1d leaves (section 13). Commits stay local.

## 12. Inputs (moved here from `internal/handoff.md` §3 and `.claude/plans/m4-5e1-arrays.md` sections 6 and 9 at this planning)

- **From e1c, as built** (record 0061): the reader of a `pure` body (`read_forall_function`, `forall_site`: one
  `return E;`, a map from the function's parameter variables to `Bound` and `ForallArg`), which e1d generalizes
  to a map onto `Formal`s; `f__wf` is variant M (heap-free; ghost functions `hvq_<method>__<formal>_<state>` of
  the slot with four assumed facts, item 7); S45 (d)'s row (7) (section 3); a `pure` function's own contract is
  rejected where a `forall` names it and not read elsewhere (question 252: item 2 decides it).
- **From e1a** (round 2, item 6): `lint_viper.py`'s `HEAP_READ_RE` treats a marked row's `B`, `lo`, `hi` and
  amount as heap-free when they hold no `.`; a heap-reading application would pass (item 12).
- **From M4.5f1's notes:** a `requires`, `ensures` or `decreases` on a `pure` function is neither encoded nor
  rejected today (`Encoder::run` skips every spec routine before `function_of`); `pure` bodies reject globals and
  `static` locals (0041 item 12; they still do); R1–R13 and the static-assertion cross-check do not reach `pure`
  bodies (item 3 says what reaches a read one), and `static_assert_pure_body.c`'s listed entry was expected to go
  stale (it does not: `twice` stays unread; Flips B).
- **From the umbrella's sketch (section 6 of `git show 9ab9a1e:.claude/plans/m4-5e1-arrays.md`), and what this
  plan changed:** the definition check is the function's own `__wf` method, not a method beside it; Silicon
  proves the `ensures` and the termination, and those two errors are reported from its check; an application in a
  contract is guarded (S45's twin gave two errors without the guard); the protected sum returns `long long` and
  needs no quantified bound, the `int` form with `cerastes_forall(small_at, a, n)` is the second test (cut 2);
  mutual recursion and a `pure` function returning a pointer or a structure stay rejected, as sketched.
- **Costs:** S45 (k): at most 4.1 s for e1c's combinations, the K = 16 scale program 7.3 s (14.6 s with an array
  and a quantified callee); S45 (d) (7): the bodiless contract with `sum_to` 1.8 s.
- **Lessons that bind here:** an allow-list is never computed by `grep` before a build (this session expects no
  existing Viper to change); the audit worktree needs no bootstrap (a source-only `git worktree add --detach`
  plus copies of the three binaries built from exactly that source); proofs made while another agent builds are
  provisional; a twin that runs further than its positive needs an `ensures` that holds at the wider state; a
  conjunction in one invariant clause is two errors (N2); after a failed check Silicon goes on with the failing
  value unconstrained; before an owner question about a lowering change, run the end-to-end vprdiff and put
  every changed VIR case through Silicon; a new `.cpp` under `src/` changes `object_identity.py`'s TU counts.

## 13. What e1d leaves, and the questions it opens

- **For handoff §3 at the session's end:** M4.5e2 takes applications under a binder, a `forall` naming a
  contracted function, `old` of an application, pointer-argument forms, and cuts 3 and 5 if taken; M4.5f1 takes
  predicates, a `cerastes_forall` in a `pure` body, calls in measures, the atoms a `pure` function's contract
  refuses, and cuts 2 and 4 if taken; M4.5f2 takes mutual recursion and cycles through contracts; M4.8 takes
  item 8's listed limits, question 267's missing range fact and cut 1 if taken; M6 takes bodiless `pure`
  functions; M4.5f3 takes whatever S46 (d) shows needs a lemma.
- **Answered by the owner at this planning (2026-11-24), each the recommendation except 271 and 272; record
  0062's header quotes them:**
  - **264** A wrong `ensures` on a `pure` function and a measure that does not decrease are reported from
    Viper's own check of the function, mapped to C lines (`postcondition` at the function's `return`,
    `termination` at the recursive call); every other obligation is an explicit row (items 9–11).
  - **265** A `cerastes_forall` that names a function with a contract stays rejected (question 252 at the
    `forall`); M4.5e2 lifts it with calls under a binder (section 1's table; Flips B).
  - **266** Not in e1d, each rejected and pinned: a call under a quantifier's binder (M4.5e2), a
    `cerastes_forall` inside a called `pure` body (M4.5f1), mutual recursion between `pure` functions (M4.5f2), a
    call in a `decreases` (M4.5f1).
  - **267** Outside `f__wf`, the result of an application is known only through the function's `ensures`: no
    automatic "the result fits its type" fact (item 6); a false alarm, fail closed, that M4.8 reviews.
  - **268** which `pure` functions are read (called, or carrying a contract: item 2); **269** a `pure` function
    with no body in the unit cannot be called (item 3; M6); **270** one session with split points (section 7);
    and M4.5e1c's 256–263 stand as built.
  - **271** (asked again when the design check put the size at 3,050–4,500 lines; the recommendation was two
    sessions planned now): "One session, split points". **272** (which of three constructs to leave out to hold
    the size; the recommendation was all three): "None: keep all three in" — a `cerastes_forall` in a `pure`
    function's contract, arguments that read memory, and calls in a `pure` function's own contract are in scope,
    as cuts 2, 3 and 4 (section 7).
- **Open questions: none.** A question that arises in the session stops the work it blocks and goes to the
  owner (section 11); the next question is 273.
