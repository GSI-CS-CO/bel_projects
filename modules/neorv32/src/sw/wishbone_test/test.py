import re

def extract_hex_codes(disassembly_file):
    with open(disassembly_file, 'r') as f:
        lines = f.readlines()

    hex_codes = []
    for line in lines:
        match = re.search(r'^\s*[0-9a-f]+:\s*([0-9a-f]{8})\s', line)
        if match:
            hex_codes.append(match.group(1))

    return hex_codes

def hex_to_mif(hex_codes, output_file, width, depth):
    with open(output_file, 'w') as outfile:
        outfile.write(f"WIDTH={width};\n")
        outfile.write(f"DEPTH={depth};\n")
        outfile.write("ADDRESS_RADIX=UNS;\n")
        outfile.write("DATA_RADIX=HEX;\n")
        outfile.write("CONTENT BEGIN\n")

        for i, code in enumerate(hex_codes):
            outfile.write(f"    {i} : {code};\n")

        outfile.write("END;\n")

# Parameter
disassembly_file = 'start.dump'
output_file = 'output.mif'
width = 32  # Bit-Breite eines Befehls
depth = 256  # Anzahl der Adressen (angepasst an die Größe des Programms)

hex_codes = extract_hex_codes(disassembly_file)
hex_to_mif(hex_codes, output_file, width, depth)
