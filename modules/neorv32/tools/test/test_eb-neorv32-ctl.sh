#!/bin/bash
# Tests for eb-neorv32-ctl
# Covers changes introduced in the PR:
#   - New options: -c FILE, -e, -p
#   - f_upload_program examine/verify flow (exit codes 8, 9)
#   - f_compare_program function (exit codes 13, 14, 15)
#   - Duplicate device argument detection (exit code 12)
#   - Option -c requires argument (exit code 16)

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SCRIPT="$SCRIPT_DIR/../eb-neorv32-ctl"
PASS=0
FAIL=0

# ── helpers ────────────────────────────────────────────────────────────────────

pass() { echo "PASS: $1"; ((PASS++)); }
fail() { echo "FAIL: $1"; ((FAIL++)); }

assert_exit() {
  local description="$1"
  local expected_code="$2"
  shift 2
  "$@"
  local actual_code=$?
  if [ "$actual_code" -eq "$expected_code" ]; then
    pass "$description (exit $expected_code)"
  else
    fail "$description – expected exit $expected_code, got $actual_code"
  fi
}

assert_file_exists() {
  local description="$1"
  local filepath="$2"
  if [ -f "$filepath" ]; then
    pass "$description"
  else
    fail "$description – file '$filepath' not found"
  fi
}

assert_file_absent() {
  local description="$1"
  local filepath="$2"
  if [ ! -f "$filepath" ]; then
    pass "$description"
  else
    fail "$description – file '$filepath' should not exist"
  fi
}

# ── test fixture setup ─────────────────────────────────────────────────────────

setup() {
  TMPDIR=$(mktemp -d)
  MOCK_BIN="$TMPDIR/bin"
  mkdir -p "$MOCK_BIN"

  # Mock eb-find: always succeeds, returns a dummy RAM/reset address
  cat > "$MOCK_BIN/eb-find" << 'EOF'
#!/bin/bash
echo "0x40000000"
exit 0
EOF

  # Mock eb-write: always succeeds
  cat > "$MOCK_BIN/eb-write" << 'EOF'
#!/bin/bash
exit 0
EOF

  # Mock eb-put: always succeeds (can be overridden per test)
  cat > "$MOCK_BIN/eb-put" << 'EOF'
#!/bin/bash
exit 0
EOF

  # Default mock eb-get: writes identical content to the source program
  # Reads the SIZE from the URL argument (device addr/size) and the output file from last arg
  cat > "$MOCK_BIN/eb-get" << 'EOF'
#!/bin/bash
# Args: -l -q DEVICE ADDR/SIZE OUTFILE
OUTFILE="${@: -1}"
# Find the source program: stored by test harness in MOCK_PROGRAM env var
if [ -n "${MOCK_PROGRAM:-}" ] && [ -f "$MOCK_PROGRAM" ]; then
  cp "$MOCK_PROGRAM" "$OUTFILE"
else
  echo "stub" > "$OUTFILE"
fi
exit 0
EOF

  chmod +x "$MOCK_BIN"/eb-find "$MOCK_BIN"/eb-write "$MOCK_BIN"/eb-put "$MOCK_BIN"/eb-get

  # Create a small binary program file for testing
  PROGRAM_FILE="$TMPDIR/test_program.bin"
  printf '\x00\x01\x02\x03\x04\x05\x06\x07' > "$PROGRAM_FILE"

  export PATH="$MOCK_BIN:$PATH"
  export MOCK_PROGRAM="$PROGRAM_FILE"
}

teardown() {
  rm -rf "$TMPDIR"
}

# Run a test: call setup, run the body function, call teardown
run_test() {
  local name="$1"
  local body="$2"
  setup
  (
    cd "$TMPDIR"
    eval "$body"
  )
  teardown
}

# ── argument parsing tests (new in this PR) ────────────────────────────────────

test_option_c_requires_argument() {
  run_test "option -c requires argument" '
    assert_exit "-c without argument exits 16" 16 \
      bash "$SCRIPT" /dev/ttyUSB0 -c
  '
}

test_option_c_rejects_flag_as_argument() {
  run_test "option -c rejects flag as argument" '
    assert_exit "-c followed by flag exits 16" 16 \
      bash "$SCRIPT" /dev/ttyUSB0 -c -v
  '
}

test_option_c_accepts_filename() {
  # With a valid filename the script proceeds past parsing; eb-find is mocked
  # so it will fail later (no device) but arg parsing succeeds past exit 16.
  run_test "option -c accepts a filename (no parse error)" '
    result=$(bash "$SCRIPT" /dev/ttyUSB0 -c "$PROGRAM_FILE" 2>&1)
    code=$?
    if [ "$code" -ne 16 ]; then
      pass "-c with valid filename does not exit 16"
    else
      fail "-c with valid filename should not exit 16, got exit 16"
    fi
  '
}

test_option_e_sets_examine_flag() {
  # When -e is passed without -f, the script still runs but examine path
  # is not triggered. We verify parsing doesn't error on -e alone.
  run_test "-e flag is accepted by argument parser" '
    result=$(bash "$SCRIPT" /dev/ttyUSB0 -e 2>&1)
    code=$?
    if [ "$code" -ne 11 ]; then
      pass "-e flag accepted (no 'Unknown argument' error)"
    else
      fail "-e flag should be accepted, got exit 11 (unknown arg)"
    fi
  '
}

test_option_p_sets_preserve_flag() {
  run_test "-p flag is accepted by argument parser" '
    result=$(bash "$SCRIPT" /dev/ttyUSB0 -p 2>&1)
    code=$?
    if [ "$code" -ne 11 ]; then
      pass "-p flag accepted (no 'Unknown argument' error)"
    else
      fail "-p flag should be accepted, got exit 11 (unknown arg)"
    fi
  '
}

test_duplicate_device_argument_exits_12() {
  run_test "second positional device argument exits 12" '
    assert_exit "two device args exits 12" 12 \
      bash "$SCRIPT" /dev/ttyUSB0 /dev/ttyUSB1
  '
}

test_duplicate_device_with_flags_exits_12() {
  run_test "two positional args with flags between them exits 12" '
    assert_exit "two device args with -v between exits 12" 12 \
      bash "$SCRIPT" /dev/ttyUSB0 -v /dev/ttyUSB1
  '
}

# ── f_upload_program examine flow (new in this PR) ─────────────────────────────

test_upload_with_examine_success_exits_0() {
  run_test "upload with -e, matching content exits 0" '
    # eb-get returns identical content (default mock behaviour)
    assert_exit "upload + examine success exits 0" 0 \
      bash "$SCRIPT" /dev/ttyUSB0 -f "$PROGRAM_FILE" -e
  '
}

test_upload_with_examine_starts_cpu() {
  run_test "upload with -e, matching content calls f_start_neorv32" '
    # Record whether eb-write was called with the start value (0x1)
    cat > "$MOCK_BIN/eb-write" << '"'"'EOF'"'"'
#!/bin/bash
echo "eb-write $@" >> /tmp/eb_write_calls_$$
exit 0
EOF
    CALLS_FILE="/tmp/eb_write_calls_$$"
    bash "$SCRIPT" /dev/ttyUSB0 -v -f "$PROGRAM_FILE" -e 2>&1
    if grep -q "0x1" "$CALLS_FILE" 2>/dev/null; then
      pass "CPU start (0x1) was issued after successful examine"
    else
      fail "CPU start (0x1) was not issued after successful examine"
    fi
    rm -f "$CALLS_FILE"
  '
}

test_upload_with_examine_keep_reset_no_start() {
  run_test "upload with -e -k: CPU is NOT started" '
    cat > "$MOCK_BIN/eb-write" << '"'"'EOF'"'"'
#!/bin/bash
echo "eb-write $@" >> /tmp/eb_write_calls_kr_$$
exit 0
EOF
    CALLS_FILE="/tmp/eb_write_calls_kr_$$"
    bash "$SCRIPT" /dev/ttyUSB0 -f "$PROGRAM_FILE" -e -k 2>&1
    if grep -q "0x1" "$CALLS_FILE" 2>/dev/null; then
      fail "CPU should NOT be started when -k is used"
    else
      pass "CPU NOT started when -k flag is set"
    fi
    rm -f "$CALLS_FILE"
  '
}

test_upload_with_examine_eb_get_failure_exits_8() {
  run_test "upload with -e, eb-get fails exits 8" '
    cat > "$MOCK_BIN/eb-get" << '"'"'EOF'"'"'
#!/bin/bash
exit 1
EOF
    assert_exit "eb-get failure during examine exits 8" 8 \
      bash "$SCRIPT" /dev/ttyUSB0 -f "$PROGRAM_FILE" -e
  '
}

test_upload_with_examine_content_mismatch_exits_9() {
  run_test "upload with -e, RAM content differs from program exits 9" '
    # eb-get writes different content
    cat > "$MOCK_BIN/eb-get" << '"'"'EOF'"'"'
#!/bin/bash
OUTFILE="${@: -1}"
printf '"'"'\xFF\xFE\xFD\xFC'"'"' > "$OUTFILE"
exit 0
EOF
    assert_exit "content mismatch during examine exits 9" 9 \
      bash "$SCRIPT" /dev/ttyUSB0 -f "$PROGRAM_FILE" -e
  '
}

test_upload_with_examine_readback_deleted_on_success() {
  run_test "upload with -e: readback file removed after success (no -p)" '
    bash "$SCRIPT" /dev/ttyUSB0 -f "$PROGRAM_FILE" -e 2>&1
    assert_file_absent "readback file deleted after successful examine" \
      "firmware-neorv32.bin"
  '
}

test_upload_with_examine_preserve_keeps_readback() {
  run_test "upload with -e -p: readback file is kept" '
    bash "$SCRIPT" /dev/ttyUSB0 -f "$PROGRAM_FILE" -e -p 2>&1
    assert_file_exists "readback file preserved with -p after examine" \
      "firmware-neorv32.bin"
  '
}

test_upload_with_examine_mismatch_readback_deleted() {
  run_test "upload with -e mismatch: readback file removed (no -p)" '
    cat > "$MOCK_BIN/eb-get" << '"'"'EOF'"'"'
#!/bin/bash
OUTFILE="${@: -1}"
printf '"'"'\xFF\xFE'"'"' > "$OUTFILE"
exit 0
EOF
    bash "$SCRIPT" /dev/ttyUSB0 -f "$PROGRAM_FILE" -e 2>&1 || true
    assert_file_absent "readback file deleted after mismatch (no -p)" \
      "firmware-neorv32.bin"
  '
}

test_upload_without_examine_no_start_called() {
  # After this PR, when -e is NOT set, f_start_neorv32 is never called
  # (the entire examine block is skipped). Previously -k controlled this.
  run_test "upload without -e: f_start_neorv32 is NOT called" '
    cat > "$MOCK_BIN/eb-write" << '"'"'EOF'"'"'
#!/bin/bash
echo "eb-write $@" >> /tmp/eb_write_no_examine_$$
exit 0
EOF
    CALLS_FILE="/tmp/eb_write_no_examine_$$"
    bash "$SCRIPT" /dev/ttyUSB0 -f "$PROGRAM_FILE" 2>&1
    if grep -q "0x1" "$CALLS_FILE" 2>/dev/null; then
      fail "CPU should NOT be started when -e is omitted"
    else
      pass "CPU not started when -e flag is absent"
    fi
    rm -f "$CALLS_FILE"
  '
}

# ── f_compare_program tests (new function in this PR) ──────────────────────────

test_compare_file_not_found_exits_15() {
  run_test "f_compare_program: nonexistent file exits 15" '
    assert_exit "compare with missing file exits 15" 15 \
      bash "$SCRIPT" /dev/ttyUSB0 -c /tmp/nonexistent_file_$$_xyz.bin
  '
}

test_compare_eb_get_failure_exits_13() {
  run_test "f_compare_program: eb-get failure exits 13" '
    cat > "$MOCK_BIN/eb-get" << '"'"'EOF'"'"'
#!/bin/bash
exit 1
EOF
    assert_exit "eb-get failure during compare exits 13" 13 \
      bash "$SCRIPT" /dev/ttyUSB0 -c "$PROGRAM_FILE"
  '
}

test_compare_content_mismatch_exits_14() {
  run_test "f_compare_program: RAM mismatch exits 14" '
    cat > "$MOCK_BIN/eb-get" << '"'"'EOF'"'"'
#!/bin/bash
OUTFILE="${@: -1}"
printf '"'"'\xFF\xFE\xFD'"'"' > "$OUTFILE"
exit 0
EOF
    assert_exit "RAM mismatch during compare exits 14" 14 \
      bash "$SCRIPT" /dev/ttyUSB0 -c "$PROGRAM_FILE"
  '
}

test_compare_matching_content_exits_0() {
  run_test "f_compare_program: matching RAM content exits 0" '
    assert_exit "matching RAM content exits 0" 0 \
      bash "$SCRIPT" /dev/ttyUSB0 -c "$PROGRAM_FILE"
  '
}

test_compare_readback_deleted_on_success() {
  run_test "f_compare_program: readback file removed after match (no -p)" '
    bash "$SCRIPT" /dev/ttyUSB0 -c "$PROGRAM_FILE" 2>&1
    assert_file_absent "readback file deleted after successful compare" \
      "firmware-neorv32.bin"
  '
}

test_compare_preserve_keeps_readback() {
  run_test "f_compare_program: -p keeps readback file after match" '
    bash "$SCRIPT" /dev/ttyUSB0 -c "$PROGRAM_FILE" -p 2>&1
    assert_file_exists "readback file preserved with -p after compare" \
      "firmware-neorv32.bin"
  '
}

test_compare_mismatch_readback_deleted() {
  run_test "f_compare_program: readback file removed after mismatch (no -p)" '
    cat > "$MOCK_BIN/eb-get" << '"'"'EOF'"'"'
#!/bin/bash
OUTFILE="${@: -1}"
printf '"'"'\xAA\xBB'"'"' > "$OUTFILE"
exit 0
EOF
    bash "$SCRIPT" /dev/ttyUSB0 -c "$PROGRAM_FILE" 2>&1 || true
    assert_file_absent "readback file deleted after mismatch (no -p)" \
      "firmware-neorv32.bin"
  '
}

test_compare_mismatch_preserve_keeps_readback() {
  run_test "f_compare_program: -p keeps readback file even after mismatch" '
    cat > "$MOCK_BIN/eb-get" << '"'"'EOF'"'"'
#!/bin/bash
OUTFILE="${@: -1}"
printf '"'"'\xAA\xBB'"'"' > "$OUTFILE"
exit 0
EOF
    bash "$SCRIPT" /dev/ttyUSB0 -c "$PROGRAM_FILE" -p 2>&1 || true
    assert_file_exists "readback file preserved with -p after mismatch" \
      "firmware-neorv32.bin"
  '
}

# ── idle-init IDLE_MS constant test ────────────────────────────────────────────

test_idle_ms_is_2500() {
  local main_c="$SCRIPT_DIR/../../src/sw/idle-init/main.c"
  if grep -qE '#define[[:space:]]+IDLE_MS[[:space:]]+2500' "$main_c"; then
    pass "IDLE_MS is defined as 2500 in main.c"
    ((PASS++))
  else
    fail "IDLE_MS is NOT 2500 in main.c (expected value from PR change)"
    ((FAIL++))
  fi
}

# ── run all tests ──────────────────────────────────────────────────────────────

echo "========================================"
echo "Tests for eb-neorv32-ctl and idle-init"
echo "========================================"

test_option_c_requires_argument
test_option_c_rejects_flag_as_argument
test_option_c_accepts_filename
test_option_e_sets_examine_flag
test_option_p_sets_preserve_flag
test_duplicate_device_argument_exits_12
test_duplicate_device_with_flags_exits_12
test_upload_with_examine_success_exits_0
test_upload_with_examine_starts_cpu
test_upload_with_examine_keep_reset_no_start
test_upload_with_examine_eb_get_failure_exits_8
test_upload_with_examine_content_mismatch_exits_9
test_upload_with_examine_readback_deleted_on_success
test_upload_with_examine_preserve_keeps_readback
test_upload_with_examine_mismatch_readback_deleted
test_upload_without_examine_no_start_called
test_compare_file_not_found_exits_15
test_compare_eb_get_failure_exits_13
test_compare_content_mismatch_exits_14
test_compare_matching_content_exits_0
test_compare_readback_deleted_on_success
test_compare_preserve_keeps_readback
test_compare_mismatch_readback_deleted
test_compare_mismatch_preserve_keeps_readback
test_idle_ms_is_2500

echo "========================================"
echo "Results: $PASS passed, $FAIL failed"
echo "========================================"

[ "$FAIL" -eq 0 ]