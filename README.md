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

   


