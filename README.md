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

The following code allows you to specify the input ARM assembly file and the name of the output file.

```python
from armasm.assemble import AssemblyParser
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('input')
parser.add_argument('-o', '--output')
args = parser.parse_args()

ap = AssemblyParser()
ap.assemble(args.input, args.output)
```

Given that the file containing the code above is assemble.py

```sh
python3 assemble.py prog.s -o prog.bin
```

This outputs an ARM binary named prog.bin. Then we can run the binary through the emulator as follows

```sh
./cpu prog.bin
```


## TODO

- [ ] Simulate pipelining with multithreading
- [ ] Implement a debug mode with inspiration from GDB
- [ ] Write the CPU emulator in Verilog
- [ ] Have the assembler output an ELF file
- [ ] Write a linker 
   

## Acknowledgements

- [ARM Instruction Set](https://iitd-plos.github.io/col718/ref/arm-instructionset.pdf)
- Computer Architecture: A Quantitative Approach 5th Edition

