# 0005 — Building cerastes against an EDG tree other than the pin

- **Status:** accepted (owner, 2026-10-14: "Topic branch + override").
  Superseded the same day by decision 0009 in two places:
  - item 6 (topic branch): the pin moves to fork commits, so M2 lands on
    `main`. The override remains for uncommitted EDG work, and `check.sh`
    on `main` still refuses it;
  - the pin guard of item 5: the URL is now the fork's, and the gitlink
    must be contained in `origin/cerastes` or `fork-local/cerastes`. It no
    longer has to be an ancestor of upstream `origin/main`.
- **Milestone:** M2 (2026-10-14)

## Context

M2 needs EDG to register the `[[cerastes::…]]` attributes, so that their
arguments are parsed and type-checked as C++ expressions and contracts
written after a parameter list survive redeclaration (`internal/upstream/
attribute-registration.md`). That is an EDG source change. Decision 0003
says how such changes are made:
- they live on the `cerastes` branch of the owner's fork
  (https://github.com/mborland/compiler), in a local clone such as
  `~/Documents/compiler`;
- `third_party/edg` only ever points at upstream commits, and no commit of
  this repository records a fork pointer (item 2);
- cerastes code that needs an unmerged EDG change waits on a topic branch
  until the pin moves to an upstream commit that contains it (item 8);
- `check.sh` on `main` always runs against the upstream pin (item 8).

On 2026-10-14 the owner decided (answers to the M1.5 handoff and to this
session's questions):
- the EDG work is a **local commit series** on the fork's `cerastes` branch,
  one commit per phase of the proposal, which the owner pushes and then
  discusses with the EDG maintainers;
- M2's cerastes code lives on a **local topic branch, `m2-annotations`**, and
  is built against the fork through an **override**; `main` keeps the
  upstream pin.

Decision 0003 item 8 had suggested a different mechanism: add the fork as a
remote inside `third_party/edg`, check the fork commit out there, and restore
the pin before `check.sh`. That leaves the submodule in a state that is easy
to commit by accident, needs a full EDG rebuild on every switch, and is not
enforced by anything: no script checked that `third_party/edg` is clean and at
its gitlink, although 0003 item 1 says `check.sh` relies on it.

Facts that shape the mechanism (checked 2026-10-14):
- `cmake/EdgFrontEnd.cmake` hard-coded `third_party/edg/src` and
  `build/edg/src/includes` with plain `set()`, and `CMakeLists.txt` fixed the
  output directory to `build/`.
- `err_codes.h` and `err_data.h` are generated from the EDG tree's own
  `src/error_msg.txt`. A fork that adds diagnostics therefore needs its
  generated headers from a stock build of the fork, not of the pin.
- `build/edg`'s CMake cache is bound to `third_party/edg`
  (`CMAKE_HOME_DIRECTORY`), so a stock build of another tree needs its own
  build directory.
- `scripts/cerastes-fe.sh` and `scripts/cpfe.sh` source `build/env.sh`, which
  overwrites `CERASTES_FE` and `CPFE`; `run_tests.py` lets inherited variables
  win over `env.sh`. Either way, a run can silently use a binary built from
  the wrong tree.

## Decision

1. **The override.** The environment variable `CERASTES_EDG_SRC` names the
   root of an EDG checkout (the directory that contains `src/`).
   - Unset or empty: the active tree is `third_party/edg` (kind `upstream`).
   - Set: the active tree is its real path (kind `override`). It must be an
     absolute path, exist, contain `src/il_def.h`, be the root of a git work
     tree, and not be, or be inside, `third_party/edg`.
   - It is only ever an environment variable: never read from a file, never
     set in CI, never recorded in a commit.
2. **Separate build directories per kind**, so that switching never mixes
   trees and never forces the upstream build to start over:

   | Kind | Stock EDG build | cerastes configuration | Front end |
   |---|---|---|---|
   | upstream | `build/edg` | `build/cmake` | `build/cerastes-fe` |
   | override | `build/override/edg` | `build/override/cmake` | `build/override/cerastes-fe` |

   CMake takes the tree, its generated headers and the output directory as
   cache variables (`CERASTES_EDG_SRC`, `CERASTES_EDG_GEN`,
   `CERASTES_OUTPUT_DIR`). A configure-time check refuses generated headers
   that come from a stock build of a different tree.
3. **A stamp per build** (`<stock build>/edg-tree.stamp`, written by
   `scripts/edg_tree.py stamp` after a successful build) records the tree's
   real path, HEAD, branch, and a hash of its uncommitted state (tracked
   changes and untracked, non-ignored files).
   - A different tree path wipes that kind's build directories.
   - Otherwise bootstrap always re-runs the cerastes configure step and an
     incremental `ninja` of both builds. A different HEAD or uncommitted
     state is picked up by ninja's dependency tracking.
   - The stamp also records a hash of the cerastes configuration
     (`cmake/EdgFrontEnd.cmake` and `src/backend/edg_il.h`). Then
     `describe --verify-build` rejects a binary built under an older
     `edg_config` (CP1 audit).
4. **`build/env.sh` names its tree** (`CERASTES_EDG_KIND`,
   `CERASTES_EDG_SRC_ACTIVE`, `CERASTES_EDG_GEN`). `cerastes-fe.sh`,
   `cpfe.sh` and `run_tests.py` stop with exit status 2 when the
   `CERASTES_EDG_SRC` of the current shell selects a different tree than the
   one `env.sh` was written for ("run scripts/bootstrap.sh").
   `check_edg_config.sh` takes the binary from `env.sh`.
5. **Guards in `check.sh`.** Its first step is `scripts/edg_tree.py check`,
   and its last step `scripts/edg_tree.py describe --verify-build`. `check`
   fails when:
   - `third_party/edg` is not clean (`git status --porcelain
     --untracked-files=all`), is not at the commit its gitlink records in the
     index, or its URL (in `.gitmodules` or the submodule's `origin`) is not
     `https://github.com/edgcpp/compiler.git` (this implements 0003 item 1);
   - the gitlink is not an ancestor of the submodule's `origin/main` (0003
     item 2: never a fork commit). A stale `origin/main` fails with "fetch
     origin" (CP1 audit);
   - `CERASTES_EDG_SRC` is set and HEAD is not a named branch other than
     `main`. That covers `main` itself and a detached HEAD (0003 item 8:
     `check.sh` on `main` runs against the upstream pin);
   - the override fails the checks of item 1.

   `check.sh` prints which tree it used, for example
   `EDG tree: upstream third_party/edg @ 034385ba (gitlink, clean)` or
   `EDG tree: OVERRIDE ~/Documents/compiler @ <sha> (branch cerastes, N
   commits on top of the pin, M modified, U untracked)`. `describe
   --verify-build` fails if the binary under test was built from a different
   tree state than the active one.
6. **Branches.** Infrastructure that works with either tree (this record,
   the override, harness extensions, decision records) lands on `main`. Code
   and tests that need an unmerged EDG change live on a topic branch
   (for M2, `m2-annotations`), which is rebased onto `main` and merged only
   after the pin has moved to an upstream commit that contains the EDG
   change. User docs for that code are therefore published after the merge.
7. **Amends 0003 item 8.** Fork commits are never checked out inside
   `third_party/edg`; use `CERASTES_EDG_SRC` instead. `scripts/bootstrap.sh`
   still leaves a submodule that is not at its gitlink alone, with a warning,
   but `check.sh` now fails on it.

## Consequences

- The topic branch is green only with the override set; without it, the M2
  tests fail loudly (unknown attributes are errors once the namespace is
  registered), never silently.
- A handoff that leaves work on a topic branch names the fork commit it was
  tested against, in prose. That is not a fork pointer (0003 item 2).
- Switching between `main` and the topic branch costs no full rebuild: each
  kind keeps its own build directories.
- The upstream build's behaviour is unchanged when the variable is unset,
  apart from the two new guards and the report line.
