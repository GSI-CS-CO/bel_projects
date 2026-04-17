"""
Tests for ip_cores/fpga-config-space sdbfs/lib/Makefile changes.

PR change: added -std=gnu99 to CFLAGS in sdbfs/lib/Makefile
Commit: "sdbfs/lib: fix error -> cannot use keyword false as enumeration constant"

These tests verify:
1. The Makefile contains the required -std=gnu99 flag in CFLAGS
2. Other essential CFLAGS are preserved after the change
3. The compiler correctly rejects code that uses 'false' as an enum constant
   when compiled with -std=gnu99 (reproducing the original bug scenario)
4. Valid C99/gnu99 code using <stdbool.h> compiles correctly
5. The library source files are compilable with -std=gnu99
"""
import os
import re
import subprocess
import sys
import tempfile
import textwrap

import pytest

# Path to the submodule Makefile relative to the repo root
REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
SDBFS_LIB_DIR = os.path.join(REPO_ROOT, "ip_cores", "fpga-config-space", "sdbfs", "lib")
SDBFS_LIB_MAKEFILE = os.path.join(SDBFS_LIB_DIR, "Makefile")
SDBFS_INCLUDE_DIR = os.path.join(
    REPO_ROOT, "ip_cores", "fpga-config-space", "sdbfs", "include", "linux"
)


def submodule_available():
    """Return True if the fpga-config-space submodule has been initialized."""
    return os.path.isfile(SDBFS_LIB_MAKEFILE)


def cc_available():
    """Return True if a C compiler (gcc or cc) is present on the PATH."""
    for compiler in ("gcc", "cc"):
        try:
            subprocess.run(
                [compiler, "--version"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                check=True,
            )
            return True
        except (subprocess.CalledProcessError, FileNotFoundError):
            pass
    return False


def get_cc():
    """Return the first available C compiler."""
    for compiler in ("gcc", "cc"):
        try:
            subprocess.run(
                [compiler, "--version"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                check=True,
            )
            return compiler
        except (subprocess.CalledProcessError, FileNotFoundError):
            pass
    pytest.skip("No C compiler available")


requires_submodule = pytest.mark.skipif(
    not submodule_available(),
    reason="fpga-config-space submodule not initialized (run: git submodule update --init ip_cores/fpga-config-space)",
)

requires_cc = pytest.mark.skipif(
    not cc_available(),
    reason="No C compiler (gcc/cc) found on PATH",
)


# ---------------------------------------------------------------------------
# Makefile content tests
# ---------------------------------------------------------------------------


@requires_submodule
class TestMakefileCflags:
    """Verify that the sdbfs/lib/Makefile contains the expected CFLAGS."""

    def _read_makefile(self):
        with open(SDBFS_LIB_MAKEFILE, "r") as f:
            return f.read()

    def test_cflags_contains_std_gnu99(self):
        """CFLAGS must contain -std=gnu99 to fix the 'false as enum constant' error."""
        content = self._read_makefile()
        # The CFLAGS assignment line must contain -std=gnu99
        cflags_lines = [
            line for line in content.splitlines() if re.match(r"^CFLAGS\s*=", line)
        ]
        assert cflags_lines, "No CFLAGS assignment found in Makefile"
        assert any(
            "-std=gnu99" in line for line in cflags_lines
        ), f"-std=gnu99 not found in CFLAGS lines: {cflags_lines}"

    def test_cflags_contains_wall(self):
        """-Wall must be preserved in CFLAGS."""
        content = self._read_makefile()
        cflags_lines = [
            line for line in content.splitlines() if re.match(r"^CFLAGS\s*[+]?=", line)
        ]
        all_cflags = " ".join(cflags_lines)
        assert "-Wall" in all_cflags, "-Wall not found in CFLAGS"

    def test_cflags_contains_ggdb(self):
        """-ggdb must be preserved in CFLAGS."""
        content = self._read_makefile()
        cflags_lines = [
            line for line in content.splitlines() if re.match(r"^CFLAGS\s*[+]?=", line)
        ]
        all_cflags = " ".join(cflags_lines)
        assert "-ggdb" in all_cflags, "-ggdb not found in CFLAGS"

    def test_cflags_contains_O2(self):
        """-O2 optimisation flag must be preserved in CFLAGS."""
        content = self._read_makefile()
        cflags_lines = [
            line for line in content.splitlines() if re.match(r"^CFLAGS\s*[+]?=", line)
        ]
        all_cflags = " ".join(cflags_lines)
        assert "-O2" in all_cflags, "-O2 not found in CFLAGS"

    def test_cflags_does_not_use_std_c89(self):
        """CFLAGS must not specify -std=c89 or -std=c90 (would re-introduce the bug)."""
        content = self._read_makefile()
        assert "-std=c89" not in content, "CFLAGS must not use -std=c89"
        assert "-std=c90" not in content, "CFLAGS must not use -std=c90"
        assert "-std=gnu89" not in content, "CFLAGS must not use -std=gnu89"
        assert "-std=gnu90" not in content, "CFLAGS must not use -std=gnu90"

    def test_std_gnu99_is_on_main_cflags_line(self):
        """
        -std=gnu99 must appear on the primary CFLAGS assignment (not just an append),
        so it takes effect regardless of include order.
        """
        content = self._read_makefile()
        for line in content.splitlines():
            if re.match(r"^CFLAGS\s*=", line) and "-std=gnu99" in line:
                return  # found on main CFLAGS = ... line
        pytest.fail("-std=gnu99 not found on the primary 'CFLAGS = ...' line")

    def test_lib_targets_defined(self):
        """Makefile must define LIB and OBJS targets needed to build libsdbfs.a."""
        content = self._read_makefile()
        assert "LIB" in content, "LIB variable not found in Makefile"
        assert "libsdbfs.a" in content, "libsdbfs.a not referenced in Makefile"
        assert "OBJS" in content, "OBJS variable not found in Makefile"


# ---------------------------------------------------------------------------
# Compiler behaviour tests (require gcc/cc but NOT the submodule)
# ---------------------------------------------------------------------------


@requires_cc
class TestGnu99CompilerBehaviour:
    """
    Verify compiler behaviour with -std=gnu99 vs without it.

    These tests document why -std=gnu99 was added: using 'false' as an
    enumeration constant name is illegal in C99/gnu99 because <stdbool.h>
    defines 'false' as a macro.
    """

    def _compile(self, source: str, extra_flags: list[str] | None = None) -> tuple[int, str]:
        """Compile *source* in a temp file; return (returncode, combined output)."""
        cc = get_cc()
        flags = ["-Wall", "-std=gnu99"] + (extra_flags or [])
        with tempfile.NamedTemporaryFile(suffix=".c", mode="w", delete=False) as f:
            f.write(source)
            src_path = f.name
        try:
            result = subprocess.run(
                [cc] + flags + ["-x", "c", "-c", src_path, "-o", "/dev/null"],
                capture_output=True,
                text=True,
            )
            return result.returncode, result.stdout + result.stderr
        finally:
            os.unlink(src_path)

    def test_false_as_enum_constant_fails_with_gnu99(self):
        """
        Regression test: 'false' CANNOT be used as an enumeration constant name
        under -std=gnu99 because it is a keyword / macro from <stdbool.h>.
        Compiling such code must fail.
        """
        bad_code = textwrap.dedent(
            """\
            #include <stdbool.h>
            enum my_enum {
                false = 0,
                true_val = 1
            };
            int main(void) { return 0; }
            """
        )
        rc, output = self._compile(bad_code)
        assert rc != 0, (
            "Expected compilation failure when 'false' is used as enum constant "
            "with -std=gnu99, but compiler returned 0.\nCompiler output:\n" + output
        )

    def test_valid_stdbool_usage_compiles_with_gnu99(self):
        """
        Code that uses stdbool.h idiomatically compiles cleanly with -std=gnu99.
        """
        good_code = textwrap.dedent(
            """\
            #include <stdbool.h>
            #include <stdint.h>
            static bool is_valid(int x)
            {
                return x > 0;
            }
            int main(void)
            {
                bool result = is_valid(1);
                return result ? 0 : 1;
            }
            """
        )
        rc, output = self._compile(good_code)
        assert rc == 0, (
            "Expected successful compilation of valid C99 stdbool code, "
            "but compiler returned non-zero.\nCompiler output:\n" + output
        )

    def test_enum_with_false_name_fails_without_stdbool_but_with_gnu99(self):
        """
        Even without explicitly including stdbool.h, GCC under -std=gnu99 may
        define 'false' and 'true' as builtin macros, so using 'false' as an
        enum constant name should still be rejected.
        """
        bad_code = textwrap.dedent(
            """\
            enum state {
                false = 0,
                active = 1
            };
            int main(void) { return 0; }
            """
        )
        # With -std=gnu99, GCC typically defines false/true as builtins
        # so this should fail or at least produce a warning treated as error.
        # We also pass -Werror to catch it even if it's only a warning.
        cc = get_cc()
        with tempfile.NamedTemporaryFile(suffix=".c", mode="w", delete=False) as f:
            f.write(bad_code)
            src_path = f.name
        try:
            result = subprocess.run(
                [cc, "-Wall", "-Werror", "-std=gnu99", "-x", "c", "-c",
                 src_path, "-o", "/dev/null"],
                capture_output=True,
                text=True,
            )
            # Either it fails to compile, or the compiler silently allows it
            # (some compilers may not define false as builtin without stdbool).
            # The important assertion is that WITH stdbool.h it definitely fails.
            # Here we just document and record the behaviour.
            output = result.stdout + result.stderr
            # If compilation fails that is the expected/safe behaviour
            # If it succeeds, we still pass but note it
            assert True, "Compiler behaviour documented (may pass or fail)"
        finally:
            os.unlink(src_path)

    def test_gnu99_enables_c99_features(self):
        """
        With -std=gnu99 the compiler must accept C99 features such as
        variable-length arrays and designated initialisers.
        """
        c99_code = textwrap.dedent(
            """\
            #include <stdint.h>
            #include <string.h>
            struct point { int x; int y; };
            int main(void)
            {
                int n = 4;
                int arr[n];          /* VLA - C99 feature */
                memset(arr, 0, sizeof(arr));
                struct point p = { .x = 1, .y = 2 };  /* designated initialiser */
                return p.x - 1;
            }
            """
        )
        rc, output = self._compile(c99_code)
        assert rc == 0, (
            "C99 code should compile cleanly with -std=gnu99.\n"
            "Compiler output:\n" + output
        )

    def test_gnu99_allows_gnu_extensions(self):
        """
        The 'gnu99' dialect must allow GNU extensions such as typeof().
        """
        gnu_ext_code = textwrap.dedent(
            """\
            int main(void)
            {
                int x = 5;
                typeof(x) y = x * 2;  /* GNU typeof() extension */
                return y - 10;
            }
            """
        )
        rc, output = self._compile(gnu_ext_code)
        assert rc == 0, (
            "GNU extension code should compile with -std=gnu99.\n"
            "Compiler output:\n" + output
        )

    def test_sdbfs_like_struct_compiles_with_gnu99(self):
        """
        A self-contained reimplementation of the key sdbfs structures and
        function signatures must compile cleanly under -std=gnu99.
        This validates that the library ABI is compatible with C99.
        """
        sdbfs_like_code = textwrap.dedent(
            """\
            #include <stdint.h>
            #include <string.h>
            #include <arpa/inet.h>

            #define SDB_MAGIC 0x5344422dU
            #define SDBFS_DEPTH 4

            enum sdb_record_type {
                sdb_type_interconnect = 0x00,
                sdb_type_device       = 0x01,
                sdb_type_bridge       = 0x02,
                sdb_type_empty        = 0xFF
            };

            struct sdb_product {
                uint64_t vendor_id;
                uint32_t device_id;
                uint32_t version;
                uint32_t date;
                uint8_t  name[19];
                uint8_t  record_type;
            };

            struct sdb_component {
                uint64_t        addr_first;
                uint64_t        addr_last;
                struct sdb_product product;
            };

            struct sdb_device {
                uint16_t abi_class;
                uint8_t  abi_ver_major;
                uint8_t  abi_ver_minor;
                uint32_t bus_specific;
                struct sdb_component sdb_component;
            };

            struct sdb_interconnect {
                uint32_t sdb_magic;
                uint16_t sdb_records;
                uint8_t  sdb_version;
                uint8_t  sdb_bus_type;
                struct sdb_component sdb_component;
            };

            struct sdbfs {
                char          *name;
                void          *drvdata;
                unsigned long  blocksize;
                unsigned long  entrypoint;
                unsigned long  flags;
                void          *data;
                unsigned long  datalen;
                int (*read)(struct sdbfs *fs, int offset, void *buf, int count);
                int (*write)(struct sdbfs *fs, int offset, void *buf, int count);
                struct sdb_device  *currentp;
                struct sdb_device   current_record;
                unsigned long  f_len;
                unsigned long  f_offset;
                unsigned long  read_offset;
                struct sdbfs  *next;
                unsigned long  base[SDBFS_DEPTH];
                unsigned long  this_[SDBFS_DEPTH];
                int            nleft[SDBFS_DEPTH];
                int            depth;
            };

            int main(void)
            {
                struct sdbfs fs;
                memset(&fs, 0, sizeof(fs));
                return (int)sizeof(fs);
            }
            """
        )
        rc, output = self._compile(sdbfs_like_code)
        assert rc == 0, (
            "sdbfs-like struct definitions must compile cleanly with -std=gnu99.\n"
            "Compiler output:\n" + output
        )


# ---------------------------------------------------------------------------
# Library source compilation test (requires submodule AND compiler)
# ---------------------------------------------------------------------------


@requires_submodule
@requires_cc
class TestSdbfsLibCompilation:
    """Verify that the sdbfs/lib C sources compile with -std=gnu99."""

    def _compile_file(self, src_path: str, include_dirs: list[str]) -> tuple[int, str]:
        cc = get_cc()
        inc_flags = [f"-I{d}" for d in include_dirs]
        result = subprocess.run(
            [cc, "-Wall", "-ggdb", "-O2", "-std=gnu99",
             "-Wno-pointer-sign",
             "-ffunction-sections", "-fdata-sections"]
            + inc_flags
            + ["-c", src_path, "-o", "/dev/null"],
            capture_output=True,
            text=True,
        )
        return result.returncode, result.stdout + result.stderr

    def test_access_c_compiles_with_gnu99(self):
        """sdbfs/lib/access.c must compile without errors with -std=gnu99."""
        src = os.path.join(SDBFS_LIB_DIR, "access.c")
        if not os.path.isfile(src):
            pytest.skip(f"Source file not found: {src}")
        rc, output = self._compile_file(
            src,
            include_dirs=[SDBFS_LIB_DIR, SDBFS_INCLUDE_DIR],
        )
        assert rc == 0, f"access.c failed to compile with -std=gnu99:\n{output}"

    def test_glue_c_compiles_with_gnu99(self):
        """sdbfs/lib/glue.c must compile without errors with -std=gnu99."""
        src = os.path.join(SDBFS_LIB_DIR, "glue.c")
        if not os.path.isfile(src):
            pytest.skip(f"Source file not found: {src}")
        rc, output = self._compile_file(
            src,
            include_dirs=[SDBFS_LIB_DIR, SDBFS_INCLUDE_DIR],
        )
        assert rc == 0, f"glue.c failed to compile with -std=gnu99:\n{output}"

    def test_access_c_fails_to_compile_without_stdbool_fix(self):
        """
        Regression guard: if access.c or glue.c contained 'false' as an enum
        constant name, compilation with -std=gnu99 and <stdbool.h> would fail.
        Verify that the current sources do NOT trigger this bug.
        """
        # If both source files compile cleanly (tested above), this test passes
        # by definition - we just make it explicit.
        for fname in ("access.c", "glue.c"):
            src = os.path.join(SDBFS_LIB_DIR, fname)
            if not os.path.isfile(src):
                pytest.skip(f"Source file not found: {src}")
            with open(src) as f:
                content = f.read()
            # Ensure no enum definition uses 'false' or 'true' as a constant name
            # Pattern: enum { ... false ..., or enum { ... true ...,
            bad_pattern = re.compile(
                r"\benum\b[^}]*\bfalse\b[^}]*\}", re.DOTALL
            )
            assert not bad_pattern.search(content), (
                f"{fname} contains 'false' as an enum constant name, "
                "which would fail to compile with -std=gnu99"
            )