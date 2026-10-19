# Handoff after the M2 re-look items and M3 (2026-10-16 to 2026-10-19)

The next session reads this file after `CLAUDE.md`, its own milestone in `plan.md`, `internal/status.md` and
`internal/decisions/`. `status.md` has the formal entry. This file adds what a cold start would otherwise have to
rediscover. The M2 handoff (with the owner's answers) is `git show e8830f9:internal/handoff.md`; the M1.5 one is
`git show c0b7ef2:internal/handoff.md`.

## 1. State in one paragraph

M3 is done on `main` (see §8 for acceptance): `build/cerastes verify` verifies non-template C23 and C++20
functions over integers and `bool`, with total correctness (termination measures, decision 0013), through VIR →
Viper → Silicon → Z3, and reports errors at C/C++ lines in eleven categories. The M2 re-look items are closed
(§3). The EDG fork carries a reviewed chain — bug 0020 (lambdas naming template parameters), series commits L, M,
N, P — on temporary branches, audited and box-checked; merging it and moving the pin is the next session's first step (§4). Nothing is
pushed: the owner pushes the fork first, then cerastes `main`.

## 2. Owner steps, in this order

1. **Push the fork's `cerastes`** (`git -C ~/Documents/compiler push origin cerastes`). It is ahead of GitHub (K
   `23f55e3628`) by the bug-campaign merges and the upstream merge `0bcd9bf28a` (the pin). The 0020/L/M/N/P chain is
   not merged into it yet (next session, §4); push again after that.
2. **Push cerastes `main`.** `check.sh` reports `NOT YET PUSHED` until step 1.
3. **Branches for upstream PRs** (owner files them): `0020` (`c539de4101`, one commit on upstream `307002eae6`); the
   earlier ones from the bug campaign (status.md 2026-10-15). Bug 0021 is recorded, open, not to be fixed.
4. **Your uncommitted bug-file edits** (0006 moved to `upstreamed/`, staged; edits to 0005, 0007 and the bug README)
   were left exactly as they were; the session committed only its own files (§9 question 1).
5. **Remove agent evidence when done reviewing:** the fork's `.tmp/agent-0020/`, `.tmp/agent-L/`, `.tmp/agent-M/`,
   `.tmp/agent-N/`, `.tmp/agent-P/`, `.tmp/agent-final/` (and the older `.tmp/agent-k/`, `.tmp/agent-m2/`,
   `.tmp/agent-bugs/`); the worktrees `build/wt-0020`, `wt-0020c`, `wt-L`, `wt-M`, `wt-N`, `wt-P` once merged.

## 3. The M2 re-look items (handoff §4 and §10 of M2) — what happened

| Item | Outcome |
|---|---|
| Q4 language modes | kept; plan.md M7 lists GNU emulation (to be added) and pre-C23 C (refused by design) |
| Q5 lambdas naming `T` | EDG bug 0020 + series L (function templates) + M (member templates of class templates); 3 audit rounds |
| Q6 no Claude co-author | CLAUDE.md, agent files, memory; existing commits left as they are (owner) |
| Q7 brand kit | the two loose files replaced by the kit's (`doc/ui/README.md` records the hashes) |
| Q11 K with G and I | Opus review: nothing blocking; one gap (in-class member-template specializations in an uninstantiated class template) closed on the cerastes side (`in_class_specialization_contract`) and pinned in the fork (series N) |
| §4.4 oracle lost | `scripts/walker_oracle.py` committed with self-tests; a `check.sh` step (131 goldens, ~7 s) |
| §4.8 constexpr spec functions | owner: keep `constexpr` allowed; 20 traceable compile-time uses rejected from the IL; the 5 no-trace ones by fork commit P (`RAF_NO_CONSTANT_EVALUATION`), effective when pinned and registered |
| §4.8 static locals, `&S::ok` | `spec_call_in_code` |
| §4.8 header tampering | widened after the driver audit: `RECORD_MACROS_IN_IL=1`, `annotation_macro_changed` for every protected and predefined name, `<cerastes.h>` fails closed (`#error`), drivers refuse `-U` and reserved/cerastes `-D` |
| §4.8 header in a namespace | `cerastes_header_in_namespace` |
| §4.3 `old` in spec bodies | `old_outside_ensures` |
| §4.8 shadow-check misses | still documented (0006 item 10), for M6 |

## 4. The EDG fork

- **Upstream merge:** `0bcd9bf28a` (upstream `307002eae6`) — pinned (`7aec17d`), box-checked.
- **The chain** (temp branches in `~/Documents/compiler`, worktrees under `build/`), final hashes: 0020
  `c539de4101`; trial merge `9068279c12`; L `3b97dcf3eb`; M `45954e8d6d`; N `2c423db33d`; P `b9519388ea`.
  - Box (x86-64, full `changes,regressions,imported,edg,cwg,stdlib`, both 64-bit configurations) at P
    `b9519388ea`: everything matches its expectations except `cwg1835` and `cwg2881`, the known baseline box
    mismatches. The Opus auditor approved the merge (2026-10-19).
  - **Not merged, not pinned yet:** the owner closed the session once the audit was done; fast-forwarding the
    fork's `cerastes` to `b9519388ea` and moving the pin is the next session's first step (`status.md`).
  - **Branch `0020` alone** (`c539de4101`, on upstream `main`) fails its own new test on Linux in both modes:
    `GH-lambda-template-param-qualified.sft.cpp` cases 1 (`--c++20 --gnu_version=999999`) and 2 (`--c++20 --clang
    --clang_version=210000`), `FAIL--OUTPUT MISMATCH`; the same test passes at the chain tip, and natively. Explain
    it (a recording made in the fork? a difference between the upstream and the fork tree?) before the owner files
    0020 upstream.
- **`edg_x86_64_cp` expectation files are overrides of `edg_x86_64`'s.** Compare a cp run with the overlay
  (`~/edg-cerastes-runs/cmp_cp.py` on the box). The "85 EDGcpfe + 2 lines of known cp differences" that the M2
  handoff and the K evidence reported were an artefact of comparing with the cp file alone; with the overlay the cp
  configuration matches exactly.
- **J leaked to ordinary attributes** (found while fixing L): J's fail-closed substitution rule and generic-lambda
  guard applied to every attribute, so Clang's `diagnose_if` got extra errors since M2. L restricts both to
  registered attributes; `GH-attr-subst-unregistered` pins it.
- **Box:** cerastes's own tree `~/edg-cerastes`, wrapper `~/edg-cerastes.sh`, runner `~/edg-cerastes-runs/run.sh
  <label> <commit> <suites>` (memory `x86-box-edg-suites`). The box sleeps; ask the owner to wake it.
- **The owner uses the fork clone** (it was on `acknowledg-forge`, then `0006`, during this session): never check
  out in the main tree; fork work happens in worktrees on temp branches; `cerastes` moves only by fast-forward or a
  `--no-ff` merge.

## 5. Things to look at again (M3)

1. **Every audit round found real holes,** including blocking ones, in records, harness, driver, encoder and every
   EDG commit (0020 needed three rounds). Keep: implement → Opus audit with probes against the binaries → fix →
   re-audit; box suites before any pin move.
2. **Known gaps, documented:**
   - termination: a recursion cycle through a function defined in another unit is not detected (0013 item 5);
   - the `predicate`/`pure` marker must be on a function's first declaration — not checkable from the IL (needs
     an EDG registration feature); a marker only on a later redeclaration leaves earlier compile-time uses open;
   - the 5 no-trace compile-time uses of constexpr spec functions until fork commit P is pinned and cerastes
     registers `RAF_NO_CONSTANT_EVALUATION` for `predicate`/`pure` (`register_attributes.cpp`), and then
     `tests/il/m3/r4a_constant_contexts.cpp` gains 5 expect lines;
   - attributes in positions where EDG records nothing (0011 item 7) are invisible but have no effect;
   - EDG bug 0021 (std-mode fold of a lambda call in a constant template argument), packs named in a lambda, a
     nested deduction of the same template (fails closed), a lambda depending on `T` only through `decltype` of a
     parameter (rejected; valid C++).
3. **M3 rejects a lot** (0012 item 12, 0011 item 7): the M7 inventory starts from that list.
4. **Run time:** `check.sh` takes about 14 minutes on a quiet machine (the harness self-test ~3 min, the tests
   ~4 min, the VIR self-test ~2.5 min); under the agents' load it took 30. The full mutation run (~1300 mutants)
   is an acceptance step, not in `check.sh`.
5. **Concurrent harness runs** race on `build/vpr`: now a lock (`build/vpr.lock`) makes the second run fail at once.

## 6. For M4

- Generic proofs read contracts from the root template; contracts on templates can now use lambdas naming `T`
  (after the pin moves to the chain).
- VIR's `IntType` is interned by (bits, signedness); M4's opaque/integral `T` needs a new type form.
- Templates in verified files are rejected today (`unsupported_declaration`); M4 lifts that.
- The M1/M2 notes (handoff §6–7 of M2) still hold.

## 7. Friction (process and tooling)

- **Background shells:** the owner found 14 left running (duplicate and stale waiters of the subagents). Rule now in
  memory and every brief: one waiter per job, stop stale ones at once, list shells before reporting.
- **Agent slips, all reported by the agents themselves:** a `git reset --hard` in a fork worktree lost uncommitted
  work (redone); `sed -i` used three times despite the rule; probe files written into the cerastes root; a waiter
  waiting on another waiter.
- **Ownership changed mid-session** to keep M3 moving while edg-backend did the EDG chain: il-analyst wrote
  `src/vir`, `src/viper`, the encoder and the driver; test-mutator wrote the positive tests as well as the twins,
  the docs, and the cerastes-side R4/R2 checks. The auditor's independence was the cross-check.
- **No interactive `git add -p`:** commits are grouped by file; `check.sh` was verified green at the tip.
- **The usage limit** stopped two agents mid-task for two days; resuming with SendMessage worked.

## 8. M3 acceptance (commands and results)

| # | Command | Result |
|---|---|---|
| 1 | `scripts/check.sh` | green, 821 s (self-tests 931 + 68 + 107 + 117; tests 2043 passed, 0 failed, 1 skipped; lint 412 `.vpr`; VIR self-test 155/155; oracle 131 goldens; docs 0 problems) |
| 2 | `python3 scripts/run_tests.py --count-error-twins` | 82 (every positive has an `error` twin in each mode) |
| 3 | `python3 scripts/mutate.py tests/scalar -j 8` (test-mutator's full run) | 892 killed, 260 safe-direction, 125 reviewed equivalents, 0 unexplained, 19 stillborn |
| 4 | `tests/mutants-equivalent.txt` review | Opus auditor: all 125 accepted (2026-10-19) |
| 5 | `build/cerastes verify doc/modules/ROOT/examples/verifying/midpoint.cpp --keep-vpr=…` | exit 0; `.vpr` + `.map`; `lint_viper.py` clean |
| 6 | `build/cerastes verify doc/modules/ROOT/examples/verifying/midpoint_overflow.cpp` | exit 1; `…midpoint_overflow.cpp:10: error: overflow: signed `int` addition may overflow` |
| 7 | `scripts/build_docs.sh && python3 scripts/check_doc_examples.py --html build/doc` | 0 problems (in `check.sh`) |
| 8 | audits | records ×2, harness ×3, VIR, driver, encoder, delta with termination, equivalents: every blocking finding fixed; tooling should-fix items in `status.md` |

## 9. Questions for the owner

Each has a recommended answer. Answer inline, as before.

1. **Your bug-file edits** (0006 → `upstreamed/`, 0005, 0007, README) are uncommitted in the tree. *Recommended:
   commit them yourself, or tell the next session to include them with its first records commit.*
2. **The "marker on the first declaration" rule** needs an EDG registration feature (refuse an accumulating
   attribute on a later declaration). *Recommended: a small series commit in the M4 or M6 session, when spec
   functions get their meaning.*
3. **Cross-unit termination** (0013 item 5) stays a stated limit until the contract manifest exists. *Recommended:
   schedule the manifest (handoff §6 of M2) with M5 or M6.*
4. **CI** was deferred to its own session (owner, 2026-10-16). *Recommended: next, before M4; it needs both pushes.*
