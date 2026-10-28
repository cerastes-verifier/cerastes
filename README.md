# cerastes

A deductive verifier for C and C++ that verifies template code generically.
Contracts are written as `[[cerastes::...]]` attributes. The EDG front end
parses them, and the program is encoded into Viper and verified with Silicon
and Z3. Errors are reported against the C/C++ source.

Status: toolchain, test harness and EDG front-end integration (milestones M0
to M1.5); nothing is verified yet. See `plan.md` for the roadmap and
`internal/status.md` for where things stand.

User documentation: https://cerastes.dev/cerastes/doc/ (source in `doc/`;
build it locally with `scripts/build_docs.sh`).

## Getting started (macOS on Apple silicon)

```sh
git clone https://github.com/cerastes-verifier/cerastes
cd cerastes
scripts/bootstrap.sh   # Homebrew prerequisites, submodules, EDG + Silicon builds, pinned Z3
scripts/check.sh       # build, lint, harness self-test, tests
```

Bootstrap needs Homebrew and the Xcode Command Line Tools. It is idempotent.
The tool versions it pins are listed in `internal/decisions/0001-toolchain-pins.md`.
x86-64 Linux (Ubuntu 26.04) is supported too: the packages to install first are
on the Installation page of the user docs.

## License

Apache License v2.0 with LLVM Exceptions; see `LICENSE.txt`. The submodules
under `third_party/` carry their own licenses.
