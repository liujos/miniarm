## About

miniarm contains an ARM CPU emulator and an ARM assembler for educational purposes.

## Supported Instructions
- All data processing instructions
- Branch with and without link
- Single word/byte data transfers
- Multiplication 

## How to Build

1. Clone the repo
```sh
git clone https://github.com/liujos/miniarm.git
```
2. Build the CPU emulator
```sh
cd miniarm/cpu/build
make
```

3. Build the assembler
```sh
cd ../../assembler
pip install -e .
```

## Usage

1. Set up the assembler 

```python
from armasm.assemble import AssemblyParser
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('input') # ARM assembly file
parser.add_argument('-o', '--output') # binary file
args = parser.parse_args()

ap = AssemblyParser()
ap.assemble(args.input, args.output)
```

2. Pass the ARM assembly file to assemble.py

```sh
python3 assemble.py prog.s -o prog.bin
```

3. Run the binary through the CPU emulator

```sh
./cpu prog.bin
```

In the build directory you will find the a copy of assemble.py and a test program that increments r1 from 0 to 100


## TODO

- [ ] Simulate pipelining with multithreading
- [ ] Implement a debug mode with inspiration from GDB
- [ ] Write the CPU emulator in Verilog
- [ ] Have the assembler output an ELF file
- [ ] Write a linker 
   

## Acknowledgements

- [ARM Instruction Set](https://iitd-plos.github.io/col718/ref/arm-instructionset.pdf)
- Computer Architecture: A Quantitative Approach 5th Edition

