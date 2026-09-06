import sys
from pathlib import Path


def hex_to_bin(input_file, output_file):
    # Read the hex file
    with open(input_file, "r") as file:
        hex_data = file.read()

    try:
        # Convert hex text into raw bytes
        binary_data = bytes.fromhex(hex_data)
    except ValueError as e:
        print(f"[!] Invalid hex data: {e}")
        return

    # Write the bytes to the binary file
    with open(output_file, "wb") as file:
        file.write(binary_data)

    print(f"[+] Converted {input_file} -> {output_file}")
    print(f"[+] Wrote {len(binary_data)} bytes")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: python {Path(sys.argv[0]).name} <input.hex> <output.bin>")
        sys.exit(1)

    hex_to_bin(sys.argv[1], sys.argv[2])