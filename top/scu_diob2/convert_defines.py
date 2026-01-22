#!/usr/bin/env python3
import argparse
import re
import os

def verilog_to_vhdl(verilog_file: str, vhdl_file: str, package_name: str):
    """
    Convert numeric `define`s from Verilog header to a VHDL package.

    Parameters:
        verilog_file  - input .vh file
        vhdl_file     - output _pkg.vhd file
        package_name  - VHDL package name
    """
    if not os.path.exists(verilog_file):
        raise FileNotFoundError(f"File '{verilog_file}' not found")

    # Regex to match numeric defines
    define_re = re.compile(r"^\s*`define\s+(\w+)\s+([0-9]+)\s*$")
    defines = []

    with open(verilog_file, "r") as f:
        for line in f:
            m = define_re.match(line)
            if m:
                defines.append(m.groups())

    with open(vhdl_file, "w") as f:
        f.write(f"""-- Auto-generated VHDL package from {verilog_file}
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package {package_name} is
""")
        for name, value in defines:
            f.write(f"\tconstant {name} : integer := {value};\n")

        f.write("\n\t-- Note: macros need manual translation (e.g., `slice`)\n")
        f.write(f"end package {package_name};\n\n")
        f.write(f"package body {package_name} is\nend package body {package_name};\n")

    print(f"Translated {len(defines)} numeric defines from '{verilog_file}' to '{vhdl_file}'.")


def main():
    parser = argparse.ArgumentParser(description="Convert Verilog numeric defines to a VHDL package")
    parser.add_argument("basename", help="Base file name (without extension); used for input .vh and output _pkg.vhd and package name)")

    args = parser.parse_args()

    base = args.basename
    verilog_file = base + ".vh"
    vhdl_file    = base + "_pkg.vhd"
    package_name = base + "_pkg"

    verilog_to_vhdl(verilog_file, vhdl_file, package_name)


if __name__ == "__main__":
    main()
