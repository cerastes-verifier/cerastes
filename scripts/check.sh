#!/usr/bin/env bash
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Build, lint and test everything.  Must be green at the end of every session.
# Requires a prior scripts/bootstrap.sh.
#
# CERASTES_CHECK_BASE unset or empty (the default, and what every session ends with): the full
# check.  CI's pull requests and pushes set it to the commit they are compared with (decision 0016
# item 14): the PR tier, in which both test runs below make Silicon runs only for the tests changed
# since that commit, the doc examples and a fixed sample; the nightly CI run is the full check.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

step() { printf '\n==== %s\n' "$*"; }

pr_base="${CERASTES_CHECK_BASE:-}"
if [[ -n "$pr_base" ]]; then
  printf 'check.sh: the PR tier, against %s (decision 0016 item 14)\n' "$pr_base"
fi

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
# the mutants of the must-verify tests generate, and the equivalence list is current
# (the full mutation run, with the verifier, is an acceptance step)
python3 scripts/mutate.py --validate

step "edg configuration"
scripts/check_edg_config.sh
scripts/check_edg_options.sh

step "harness self-test"
# The self-tests that need the real front end must fail, not skip, here.
export CERASTES_REQUIRE_E2E=1
python3 scripts/test_run_tests.py
python3 scripts/test_walker_oracle.py
python3 scripts/test_lint_viper.py
python3 scripts/test_mutate.py

step "tests"
python3 scripts/run_tests.py --pr-base="$pr_base"

step "viper lint"
# the Viper that the tests above left in build/vpr (invariant 4)
python3 scripts/lint_viper.py

step "vir self-test"
# VIR's own cases (typing rejections and lowering cases), through Silicon and the lint;
# a directory of its own, emptied first: build/vpr belongs to the harness
# The case count is pinned twice: the tool must state at least VIR_CASES_MIN cases (a smaller
# set must not pass), and the harness must pass exactly the number it stated (a case lost on
# the way, or a file that is not run, must not pass either).
VIR_CASES_MIN=1317
rm -rf build/vir-selftest
mkdir -p build/vir-selftest
vir_out="$(build/cerastes-vir-selftest build/vir-selftest)"
printf '%s\n' "$vir_out"
vir_cases="$(printf '%s\n' "$vir_out" | sed -n 's/^cerastes-vir-selftest: \([0-9][0-9]*\) lowering cases written to .*, 0 problems$/\1/p')"
if [[ ! "$vir_cases" =~ ^[0-9]+$ ]] || (( vir_cases < VIR_CASES_MIN )); then
  echo "check.sh: the VIR self-test stated ${vir_cases:-no number of} cases; at least $VIR_CASES_MIN are expected" >&2
  exit 1
fi
python3 scripts/run_tests.py --pr-base="$pr_base" --expect-passed "$vir_cases" build/vir-selftest
python3 scripts/lint_viper.py build/vir-selftest/*.vpr

step "walker oracle"
# every walker golden agrees with an independent print of the IL (--il_display)
python3 scripts/walker_oracle.py

step "docs"
scripts/build_docs.sh
python3 scripts/check_doc_examples.py --html build/doc

python3 scripts/edg_tree.py describe --verify-build
if [[ -n "$pr_base" ]]; then
  printf '\ncheck.sh: green in the PR tier (not the full check)\n'
else
  printf '\ncheck.sh: all green\n'
fi
