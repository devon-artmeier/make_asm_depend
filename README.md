# Assembly Dependency Maker

[![Build Status](https://github.com/devon-artmeier/make_asm_depend/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/devon-artmeier/make_asm_depend/actions/workflows/cmake-multi-platform.yml)

This is a tool that makes dependency files from Assembly files for use in Makefiles.

## Detects

* include
* incbin (Psy-Q, vasm)
* binclude (AS Macro Assembler)
* incdir (vasm)

## Usage

    make_asm_depend -o [output] [object file] <-i [search path]> <-r> [input file]
    
        -o [output] [object file] - Output file and object file
        <-i [search path>         - Add search path
        <-r>                      - Use relative path finding
        [input file]              - Input file
