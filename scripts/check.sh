#!/usr/bin/env bash
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Build, lint and test everything.  Must be green at the end of every session.
# Requires a prior scripts/bootstrap.sh.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

step() { printf '\n==== %s\n' "$*"; }

step "edg tree"
python3 scripts/edg_tree.py check

step "build"
scripts/bootstrap.sh --build-only

step "lint"
for script in scripts/*.sh; do
  bash -n "$script"
done
command -v shellcheck >/dev/null || { echo "check.sh: shellcheck missing; run scripts/bootstrap.sh" >&2; exit 1; }
shellcheck scripts/*.sh
python3 -m py_compile scripts/*.py
python3 scripts/run_tests.py --check-headers
python3 scripts/check_doc_examples.py

step "edg configuration"
scripts/check_edg_config.sh

step "harness self-test"
# The self-tests that need the real front end must fail, not skip, here.
export CERASTES_REQUIRE_E2E=1
python3 scripts/test_run_tests.py

step "tests"
python3 scripts/run_tests.py

step "docs"
scripts/build_docs.sh
python3 scripts/check_doc_examples.py --html build/doc

python3 scripts/edg_tree.py describe --verify-build
printf '\ncheck.sh: all green\n'
