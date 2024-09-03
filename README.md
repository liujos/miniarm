## miniarm

miniarm is a CPU emulator of an ARM processor and allows the user to execute ARM binaries on a different platform. An ARM assembler is also included.  

## Supported Instructions
- All data processing instructions
- Branch with and without link
- Single word/byte data transfers
- Multiplication 

## How to Build

To build assembler:
    cd miniarm
    pip install -e assembler

To build emulator:
    cd cpu/build
    make

   


