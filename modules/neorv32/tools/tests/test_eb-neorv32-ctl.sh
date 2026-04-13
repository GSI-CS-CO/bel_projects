#!/bin/bash
# Unit tests for eb-neorv32-ctl
#
# Tests cover the functionality introduced or changed in this PR:
#   - New -c option (compare file with RAM content)
#   - New -e option (examine/verify RAM after programming)
#   - New -p option (preserve readback file)
#   - Duplicate device argument detection (exit 12)
#   - -c option argument validation (exit 16)
#   - f_compare_program: file match/mismatch, preserve/cleanup behavior
#   - f_upload_program with -e: verify success/failure, preserve/cleanup behavior
#   - Removal of set -e (script continues past non-fatal errors)

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SCRIPT_UNDER_TEST="$(cd "$SCRIPT_DIR/.." && pwd)/eb-neorv32-ctl"

# ---------------------------------------------------------------------------
# Test infrastructure
# ---------------------------------------------------------------------------
_pass=0
_fail=0
_total=0

pass() {
  local name="$1"
  _pass=$(( _pass + 1 ))
  _total=$(( _total + 1 ))
  echo "PASS: $name"
}

fail() {
  local name="$1"
  local detail="${2:-}"
  _fail=$(( _fail + 1 ))
  _total=$(( _total + 1 ))
  echo "FAIL: $name${detail:+ -- $detail}"
}

assert_exit() {
  local name="$1"
  local expected="$2"
  local actual="$3"
  if [ "$actual" -eq "$expected" ]; then
    pass "$name"
  else
    fail "$name" "expected exit $expected, got $actual"
  fi
}

assert_contains() {
  local name="$1"
  local pattern="$2"
  local text="$3"
  # Use -- to prevent grep from treating the pattern as an option flag
  if echo "$text" | grep -qF -- "$pattern"; then
    pass "$name"
  else
    fail "$name" "pattern '$pattern' not found in output"
  fi
}

assert_file_exists() {
  local name="$1"
  local filepath="$2"
  if [ -f "$filepath" ]; then
    pass "$name"
  else
    fail "$name" "file '$filepath' does not exist"
  fi
}

assert_file_absent() {
  local name="$1"
  local filepath="$2"
  if [ ! -f "$filepath" ]; then
    pass "$name"
  else
    fail "$name" "file '$filepath' should not exist but does"
  fi
}

# ---------------------------------------------------------------------------
# Setup: create a temporary directory with mock external tools.
# Each test runs eb-neorv32-ctl as a subprocess with PATH overridden so that
# the mock binaries are found before any real eb-* tools.
# ---------------------------------------------------------------------------
TMPDIR_ROOT=$(mktemp -d)
MOCK_BIN="$TMPDIR_ROOT/bin"
mkdir -p "$MOCK_BIN"

# eb-find mock: always succeed and return a fixed address
cat > "$MOCK_BIN/eb-find" << 'EOF'
#!/bin/bash
echo "0x10000000"
exit 0
EOF

# eb-write mock: always succeed
cat > "$MOCK_BIN/eb-write" << 'EOF'
#!/bin/bash
exit 0
EOF

# eb-put mock: always succeed
cat > "$MOCK_BIN/eb-put" << 'EOF'
#!/bin/bash
exit 0
EOF

# eb-get mock: simulate readback from device.
# Set EB_GET_FAIL=1 to simulate a device read failure.
# Set EB_GET_SOURCE=<path> to have the mock copy that file as the readback
# (simulating a successful readback that matches the source).
# Default (no env vars): writes 4 zero bytes — content will differ from
# any real binary file created by make_bin().
cat > "$MOCK_BIN/eb-get" << 'EOF'
#!/bin/bash
if [ "${EB_GET_FAIL:-0}" = "1" ]; then
  exit 1
fi
# The last positional argument is the output file
outfile="${@: -1}"
if [ -n "${EB_GET_SOURCE:-}" ]; then
  cp -- "$EB_GET_SOURCE" "$outfile"
else
  # Write 4 zero bytes — mismatches any file produced by make_bin()
  printf '\x00\x00\x00\x00' > "$outfile"
fi
exit 0
EOF

# cmp mock: compare two files byte-for-byte using diff.
# Matches real cmp semantics: exit 0 if identical, 1 if different.
cat > "$MOCK_BIN/cmp" << 'EOF'
#!/bin/bash
# Strip optional flags (e.g. -s, -l) and take the first two file args
file1=""
file2=""
for arg in "$@"; do
  case "$arg" in
    -*) continue ;;
    *)
      if [ -z "$file1" ]; then
        file1="$arg"
      elif [ -z "$file2" ]; then
        file2="$arg"
      fi
      ;;
  esac
done
diff "$file1" "$file2" > /dev/null 2>&1
EOF

chmod +x "$MOCK_BIN/eb-find" "$MOCK_BIN/eb-write" "$MOCK_BIN/eb-put" \
         "$MOCK_BIN/eb-get" "$MOCK_BIN/cmp"

# Helper: run the script under test with mock PATH and a dedicated working
# directory.  Stdout, stderr, and exit code are captured via global variables:
#   _stdout, _stderr, _rc
#
# Usage: run_script [env-vars...] -- [script-args...]
# For environment variables that need to be visible to mocks (like
# EB_GET_SOURCE), export them before calling this function or set them inline:
#   EB_GET_SOURCE=... run_script /dev/ttyUSB0 ...
#
_workdir=""
run_script() {
  _workdir=$(mktemp -d "$TMPDIR_ROOT/work_XXXXXX")
  local rc
  # We capture stdout/stderr separately using process substitution.
  # EB_GET_SOURCE and EB_GET_FAIL are inherited from the calling scope when
  # they are set via inline assignment before the function call.
  _stdout=$(cd "$_workdir" && PATH="$MOCK_BIN:$PATH" bash "$SCRIPT_UNDER_TEST" "$@" 2>"$_workdir/_stderr")
  rc=$?
  _stderr=$(cat "$_workdir/_stderr")
  _rc=$rc
}

cleanup() {
  rm -rf "$TMPDIR_ROOT"
}
trap cleanup EXIT

# ---------------------------------------------------------------------------
# Helper: create a small binary file with known non-zero content.
# ---------------------------------------------------------------------------
make_bin() {
  local path="$1"
  printf '\xDE\xAD\xBE\xEF' > "$path"
}

# ===========================================================================
# Tests for changed/new argument parsing
# ===========================================================================

# -c option: requires a file argument — passing -c as the last arg exits 16
test_c_option_missing_argument() {
  local name="test_c_option_missing_argument"
  run_script -c
  assert_exit "$name" 16 "$_rc"
}

# -c option: passing another flag immediately after -c exits 16
test_c_option_flag_as_argument() {
  local name="test_c_option_flag_as_argument"
  run_script -c -v
  assert_exit "$name" 16 "$_rc"
}

# Duplicate device argument: second positional arg exits 12
test_duplicate_device_argument() {
  local name="test_duplicate_device_argument"
  run_script /dev/ttyUSB0 /dev/ttyUSB1
  assert_exit "$name" 12 "$_rc"
}

# Duplicate device argument: error message mentions the duplicate
test_duplicate_device_error_message() {
  local name="test_duplicate_device_error_message"
  run_script /dev/ttyUSB0 /dev/ttyUSB1
  assert_contains "$name" "Too many input argument" "$_stderr"
}

# ===========================================================================
# Tests for f_compare_program (-c FILE)
# ===========================================================================

# -c with non-existent file exits 15
test_compare_nonexistent_file_exits_15() {
  local name="test_compare_nonexistent_file_exits_15"
  run_script /dev/ttyUSB0 -c /nonexistent/file.bin
  assert_exit "$name" 15 "$_rc"
}

# -c with non-existent file prints an error message
test_compare_nonexistent_file_error_message() {
  local name="test_compare_nonexistent_file_error_message"
  run_script /dev/ttyUSB0 -c /nonexistent/file.bin
  assert_contains "$name" "Cannot find file" "$_stderr"
}

# -c with eb-get failure exits 13
test_compare_ebget_failure_exits_13() {
  local name="test_compare_ebget_failure_exits_13"
  local prog="$TMPDIR_ROOT/prog_cmp_ebfail.bin"
  make_bin "$prog"
  EB_GET_FAIL=1 run_script /dev/ttyUSB0 -c "$prog"
  assert_exit "$name" 13 "$_rc"
}

# -c with matching readback content exits 0
test_compare_matching_content_exits_0() {
  local name="test_compare_matching_content_exits_0"
  local prog="$TMPDIR_ROOT/prog_cmp_match.bin"
  make_bin "$prog"
  # eb-get copies prog back so cmp succeeds
  EB_GET_SOURCE="$prog" run_script /dev/ttyUSB0 -c "$prog"
  assert_exit "$name" 0 "$_rc"
}

# -c with mismatching readback content exits 14
test_compare_mismatching_content_exits_14() {
  local name="test_compare_mismatching_content_exits_14"
  local prog="$TMPDIR_ROOT/prog_cmp_mismatch.bin"
  make_bin "$prog"
  # Default eb-get writes zeros — content differs from make_bin output
  run_script /dev/ttyUSB0 -c "$prog"
  assert_exit "$name" 14 "$_rc"
}

# -c mismatch prints an error message to stderr
test_compare_mismatch_error_message() {
  local name="test_compare_mismatch_error_message"
  local prog="$TMPDIR_ROOT/prog_cmp_errmsg.bin"
  make_bin "$prog"
  run_script /dev/ttyUSB0 -c "$prog"
  assert_contains "$name" "mismatch" "$_stderr"
}

# -c without -p: readback file is deleted after comparison (mismatch case)
test_compare_readback_deleted_without_preserve_mismatch() {
  local name="test_compare_readback_deleted_without_preserve_mismatch"
  local prog="$TMPDIR_ROOT/prog_cmp_del_mis.bin"
  make_bin "$prog"
  local workdir
  workdir=$(mktemp -d "$TMPDIR_ROOT/work_XXXXXX")
  (cd "$workdir" && PATH="$MOCK_BIN:$PATH" bash "$SCRIPT_UNDER_TEST" /dev/ttyUSB0 -c "$prog") || true
  assert_file_absent "$name" "$workdir/firmware-neorv32.bin"
}

# -c without -p: readback file is deleted after comparison (match case)
test_compare_readback_deleted_without_preserve_match() {
  local name="test_compare_readback_deleted_without_preserve_match"
  local prog="$TMPDIR_ROOT/prog_cmp_del_match.bin"
  make_bin "$prog"
  local workdir
  workdir=$(mktemp -d "$TMPDIR_ROOT/work_XXXXXX")
  (cd "$workdir" && EB_GET_SOURCE="$prog" PATH="$MOCK_BIN:$PATH" bash "$SCRIPT_UNDER_TEST" /dev/ttyUSB0 -c "$prog") || true
  assert_file_absent "$name" "$workdir/firmware-neorv32.bin"
}

# -c with -p: readback file is preserved after a successful comparison
test_compare_readback_preserved_with_p_flag_on_match() {
  local name="test_compare_readback_preserved_with_p_flag_on_match"
  local prog="$TMPDIR_ROOT/prog_cmp_pres_match.bin"
  make_bin "$prog"
  local workdir
  workdir=$(mktemp -d "$TMPDIR_ROOT/work_XXXXXX")
  (cd "$workdir" && EB_GET_SOURCE="$prog" PATH="$MOCK_BIN:$PATH" bash "$SCRIPT_UNDER_TEST" /dev/ttyUSB0 -c "$prog" -p) || true
  assert_file_exists "$name" "$workdir/firmware-neorv32.bin"
}

# -c with -p: readback file is preserved even when comparison fails (exit 14)
test_compare_readback_preserved_with_p_flag_on_mismatch() {
  local name="test_compare_readback_preserved_with_p_flag_on_mismatch"
  local prog="$TMPDIR_ROOT/prog_cmp_pres_mis.bin"
  make_bin "$prog"
  local workdir
  workdir=$(mktemp -d "$TMPDIR_ROOT/work_XXXXXX")
  # Default eb-get writes zeros — content will differ from make_bin output
  (cd "$workdir" && PATH="$MOCK_BIN:$PATH" bash "$SCRIPT_UNDER_TEST" /dev/ttyUSB0 -c "$prog" -p) || true
  assert_file_exists "$name" "$workdir/firmware-neorv32.bin"
}

# ===========================================================================
# Tests for f_upload_program with -e (examine/verify after upload)
# ===========================================================================

# -e without -f has no effect; the script exits 0 (no upload, no verify)
test_examine_without_f_no_error() {
  local name="test_examine_without_f_no_error"
  # Without -f the script enters f_ctl_mode; without -s/-t/-r it exits 0
  run_script /dev/ttyUSB0 -e
  assert_exit "$name" 0 "$_rc"
}

# -f with -e and matching readback: exits 0, CPU is started (no -k)
test_examine_matching_exits_0() {
  local name="test_examine_matching_exits_0"
  local prog="$TMPDIR_ROOT/prog_exam_match.bin"
  make_bin "$prog"
  EB_GET_SOURCE="$prog" run_script /dev/ttyUSB0 -f "$prog" -e
  assert_exit "$name" 0 "$_rc"
}

# -f with -e and mismatching readback: exits 9
test_examine_mismatching_exits_9() {
  local name="test_examine_mismatching_exits_9"
  local prog="$TMPDIR_ROOT/prog_exam_mismatch.bin"
  make_bin "$prog"
  # Default eb-get writes zeros — cmp will fail
  run_script /dev/ttyUSB0 -f "$prog" -e
  assert_exit "$name" 9 "$_rc"
}

# -f with -e mismatch: error message contains "Verification failed"
test_examine_mismatch_error_message() {
  local name="test_examine_mismatch_error_message"
  local prog="$TMPDIR_ROOT/prog_exam_errmsg.bin"
  make_bin "$prog"
  run_script /dev/ttyUSB0 -f "$prog" -e
  assert_contains "$name" "Verification failed" "$_stderr"
}

# -f with -e and eb-get failure: exits 8
test_examine_ebget_failure_exits_8() {
  local name="test_examine_ebget_failure_exits_8"
  local prog="$TMPDIR_ROOT/prog_exam_ebfail.bin"
  make_bin "$prog"
  EB_GET_FAIL=1 run_script /dev/ttyUSB0 -f "$prog" -e
  assert_exit "$name" 8 "$_rc"
}

# -f with -e without -p: readback file deleted after successful verify
test_examine_readback_deleted_without_preserve_match() {
  local name="test_examine_readback_deleted_without_preserve_match"
  local prog="$TMPDIR_ROOT/prog_exam_del_match.bin"
  make_bin "$prog"
  local workdir
  workdir=$(mktemp -d "$TMPDIR_ROOT/work_XXXXXX")
  (cd "$workdir" && EB_GET_SOURCE="$prog" PATH="$MOCK_BIN:$PATH" bash "$SCRIPT_UNDER_TEST" /dev/ttyUSB0 -f "$prog" -e) || true
  assert_file_absent "$name" "$workdir/firmware-neorv32.bin"
}

# -f with -e without -p: readback file deleted even after mismatch
test_examine_readback_deleted_without_preserve_mismatch() {
  local name="test_examine_readback_deleted_without_preserve_mismatch"
  local prog="$TMPDIR_ROOT/prog_exam_del_mis.bin"
  make_bin "$prog"
  local workdir
  workdir=$(mktemp -d "$TMPDIR_ROOT/work_XXXXXX")
  (cd "$workdir" && PATH="$MOCK_BIN:$PATH" bash "$SCRIPT_UNDER_TEST" /dev/ttyUSB0 -f "$prog" -e) || true
  assert_file_absent "$name" "$workdir/firmware-neorv32.bin"
}

# -f with -e and -p: readback file preserved after successful verify
test_examine_readback_preserved_with_p_flag_on_match() {
  local name="test_examine_readback_preserved_with_p_flag_on_match"
  local prog="$TMPDIR_ROOT/prog_exam_pres_match.bin"
  make_bin "$prog"
  local workdir
  workdir=$(mktemp -d "$TMPDIR_ROOT/work_XXXXXX")
  (cd "$workdir" && EB_GET_SOURCE="$prog" PATH="$MOCK_BIN:$PATH" bash "$SCRIPT_UNDER_TEST" /dev/ttyUSB0 -f "$prog" -e -p) || true
  assert_file_exists "$name" "$workdir/firmware-neorv32.bin"
}

# -f with -e and -p: readback file preserved even on mismatch (exit 9)
test_examine_readback_preserved_with_p_flag_on_mismatch() {
  local name="test_examine_readback_preserved_with_p_flag_on_mismatch"
  local prog="$TMPDIR_ROOT/prog_exam_pres_mis.bin"
  make_bin "$prog"
  local workdir
  workdir=$(mktemp -d "$TMPDIR_ROOT/work_XXXXXX")
  (cd "$workdir" && PATH="$MOCK_BIN:$PATH" bash "$SCRIPT_UNDER_TEST" /dev/ttyUSB0 -f "$prog" -e -p) || true
  assert_file_exists "$name" "$workdir/firmware-neorv32.bin"
}

# -f without -e: no readback is performed; CPU starts normally (exit 0).
# Verifies that the changed flow (keep_reset/start logic moved inside -e block)
# does not break normal programming without verification.
test_upload_without_examine_exits_0() {
  local name="test_upload_without_examine_exits_0"
  local prog="$TMPDIR_ROOT/prog_noe.bin"
  make_bin "$prog"
  run_script /dev/ttyUSB0 -f "$prog"
  assert_exit "$name" 0 "$_rc"
}

# -f without -e and with -k: CPU stays in reset; exits 0
test_upload_without_examine_keep_reset_exits_0() {
  local name="test_upload_without_examine_keep_reset_exits_0"
  local prog="$TMPDIR_ROOT/prog_noe_k.bin"
  make_bin "$prog"
  run_script /dev/ttyUSB0 -f "$prog" -k
  assert_exit "$name" 0 "$_rc"
}

# -f with -e and -k: CPU remains in reset after a successful verify (exit 0)
test_examine_matching_with_keep_reset_exits_0() {
  local name="test_examine_matching_with_keep_reset_exits_0"
  local prog="$TMPDIR_ROOT/prog_exam_k.bin"
  make_bin "$prog"
  EB_GET_SOURCE="$prog" run_script /dev/ttyUSB0 -f "$prog" -e -k
  assert_exit "$name" 0 "$_rc"
}

# ===========================================================================
# Tests for help output containing new options
# ===========================================================================

# -h: exits 0
test_help_flag_exits_0() {
  local name="test_help_flag_exits_0"
  run_script -h
  assert_exit "$name" 0 "$_rc"
}

# -h output includes the new -c option
test_help_contains_c_option() {
  local name="test_help_contains_c_option"
  run_script -h
  assert_contains "$name" "-c FILE" "$_stdout"
}

# -h output includes the new -e option
test_help_contains_e_option() {
  local name="test_help_contains_e_option"
  run_script -h
  assert_contains "$name" "Verify the RAM content after programming" "$_stdout"
}

# -h output includes the new -p option
test_help_contains_p_option() {
  local name="test_help_contains_p_option"
  run_script -h
  assert_contains "$name" "Preserve the downloaded RAM content" "$_stdout"
}

# ===========================================================================
# Additional / boundary / regression tests
# ===========================================================================

# Unknown argument exits 11 (pre-existing behaviour, regression check)
test_unknown_argument_exits_11() {
  local name="test_unknown_argument_exits_11"
  run_script --bad-option
  assert_exit "$name" 11 "$_rc"
}

# -c and -p together are accepted without a parse error
test_c_and_p_options_accepted() {
  local name="test_c_and_p_options_accepted"
  local prog="$TMPDIR_ROOT/prog_cp.bin"
  make_bin "$prog"
  local workdir
  workdir=$(mktemp -d "$TMPDIR_ROOT/work_XXXXXX")
  (cd "$workdir" && EB_GET_SOURCE="$prog" PATH="$MOCK_BIN:$PATH" bash "$SCRIPT_UNDER_TEST" /dev/ttyUSB0 -c "$prog" -p)
  local rc=$?
  # parse error (exit 16) would indicate options were not accepted; 0 is success
  if [ "$rc" -ne 16 ]; then
    pass "$name"
  else
    fail "$name" "option parsing rejected -c and -p together (exit 16)"
  fi
}

# -c with matching content and -v flag: output contains success indicator
test_compare_match_success_message_with_verbose() {
  local name="test_compare_match_success_message_with_verbose"
  local prog="$TMPDIR_ROOT/prog_succ.bin"
  make_bin "$prog"
  local workdir
  workdir=$(mktemp -d "$TMPDIR_ROOT/work_XXXXXX")
  local out
  out=$(cd "$workdir" && EB_GET_SOURCE="$prog" PATH="$MOCK_BIN:$PATH" bash "$SCRIPT_UNDER_TEST" /dev/ttyUSB0 -v -c "$prog" -p 2>/dev/null)
  if echo "$out" | grep -qF -- "[SUCCESS]"; then
    pass "$name"
  else
    fail "$name" "success message not found in verbose output"
  fi
}

# -f with -e and matching content and -v: success message includes "verified"
test_examine_match_success_message_with_verbose() {
  local name="test_examine_match_success_message_with_verbose"
  local prog="$TMPDIR_ROOT/prog_exam_succ.bin"
  make_bin "$prog"
  local workdir
  workdir=$(mktemp -d "$TMPDIR_ROOT/work_XXXXXX")
  local out
  out=$(cd "$workdir" && EB_GET_SOURCE="$prog" PATH="$MOCK_BIN:$PATH" bash "$SCRIPT_UNDER_TEST" /dev/ttyUSB0 -v -f "$prog" -e -p 2>/dev/null)
  if echo "$out" | grep -qF -- "verified"; then
    pass "$name"
  else
    fail "$name" "verification success message not found in verbose output"
  fi
}

# Boundary: -c with an empty file (0 bytes) exits 0 (files are identical after
# reading 0 bytes back, as cmp of two empty files returns 0).
test_compare_empty_file_exits_0() {
  local name="test_compare_empty_file_exits_0"
  local prog="$TMPDIR_ROOT/prog_empty.bin"
  : > "$prog"  # create empty file
  local workdir
  workdir=$(mktemp -d "$TMPDIR_ROOT/work_XXXXXX")
  # EB_GET_SOURCE copies the empty file; cmp of two empty files -> exit 0
  (cd "$workdir" && EB_GET_SOURCE="$prog" PATH="$MOCK_BIN:$PATH" bash "$SCRIPT_UNDER_TEST" /dev/ttyUSB0 -c "$prog")
  local rc=$?
  assert_exit "$name" 0 "$rc"
}

# ===========================================================================
# Test for IDLE_MS constant in main.c (changed from 1000 to 2500 in this PR)
# ===========================================================================

test_idle_ms_constant_is_2500() {
  local name="test_idle_ms_constant_is_2500"
  local main_c="$SCRIPT_DIR/../../src/sw/idle-init/main.c"
  if [ ! -f "$main_c" ]; then
    fail "$name" "main.c not found at expected path: $main_c"
    return
  fi
  if grep -qE '^#define[[:space:]]+IDLE_MS[[:space:]]+2500([[:space:]]|$)' "$main_c"; then
    pass "$name"
  else
    fail "$name" "IDLE_MS is not set to 2500 in main.c (old value was 1000)"
  fi
}

# Regression: old IDLE_MS value (1000) must NOT be present
test_idle_ms_old_value_absent() {
  local name="test_idle_ms_old_value_absent"
  local main_c="$SCRIPT_DIR/../../src/sw/idle-init/main.c"
  if [ ! -f "$main_c" ]; then
    fail "$name" "main.c not found at expected path: $main_c"
    return
  fi
  if grep -qE '^#define[[:space:]]+IDLE_MS[[:space:]]+1000([[:space:]]|$)' "$main_c"; then
    fail "$name" "IDLE_MS still has the old value 1000 in main.c"
  else
    pass "$name"
  fi
}

# ===========================================================================
# Run all tests
# ===========================================================================
echo "Running tests for eb-neorv32-ctl and related changes..."
echo "-----------------------------------------------------------"

test_c_option_missing_argument
test_c_option_flag_as_argument
test_duplicate_device_argument
test_duplicate_device_error_message
test_compare_nonexistent_file_exits_15
test_compare_nonexistent_file_error_message
test_compare_ebget_failure_exits_13
test_compare_matching_content_exits_0
test_compare_mismatching_content_exits_14
test_compare_mismatch_error_message
test_compare_readback_deleted_without_preserve_mismatch
test_compare_readback_deleted_without_preserve_match
test_compare_readback_preserved_with_p_flag_on_match
test_compare_readback_preserved_with_p_flag_on_mismatch
test_examine_without_f_no_error
test_examine_matching_exits_0
test_examine_mismatching_exits_9
test_examine_mismatch_error_message
test_examine_ebget_failure_exits_8
test_examine_readback_deleted_without_preserve_match
test_examine_readback_deleted_without_preserve_mismatch
test_examine_readback_preserved_with_p_flag_on_match
test_examine_readback_preserved_with_p_flag_on_mismatch
test_upload_without_examine_exits_0
test_upload_without_examine_keep_reset_exits_0
test_examine_matching_with_keep_reset_exits_0
test_help_flag_exits_0
test_help_contains_c_option
test_help_contains_e_option
test_help_contains_p_option
test_unknown_argument_exits_11
test_c_and_p_options_accepted
test_compare_match_success_message_with_verbose
test_examine_match_success_message_with_verbose
test_compare_empty_file_exits_0
test_idle_ms_constant_is_2500
test_idle_ms_old_value_absent

echo "-----------------------------------------------------------"
echo "Results: $_pass passed, $_fail failed out of $_total tests."

if [ "$_fail" -gt 0 ]; then
  exit 1
fi
exit 0