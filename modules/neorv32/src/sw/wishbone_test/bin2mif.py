import sys

# Parameter
BIN_FILE = "program.bin"
MIF_FILE = "program.mif"
WORD_WIDTH = 32  # bits
ADDRESS_RADIX = "HEX"
DATA_RADIX = "HEX"

# Read BIN file
with open(BIN_FILE, "rb") as f:
    binary = f.read()

# Slice into 4 byte words
words = []
for i in range(0, len(binary), 4):
    chunk = binary[i:i+4]
    while len(chunk) < 4:
        chunk += b'\x00'  # padding
    word = int.from_bytes(chunk, byteorder='little')
    words.append(word)

depth = len(words)

# Write MIF file
with open(MIF_FILE, "w") as f:
    f.write(f"WIDTH={WORD_WIDTH};\n")
    f.write(f"DEPTH={depth};\n\n")
    f.write(f"ADDRESS_RADIX={ADDRESS_RADIX};\n")
    f.write(f"DATA_RADIX={DATA_RADIX};\n\n")
    f.write("CONTENT BEGIN\n")

    for addr, word in enumerate(words):
        f.write(f"  {addr:02X} : {word:08X};\n")

    f.write("END;\n")

print(f"Done: {MIF_FILE} (Words: {depth})")
