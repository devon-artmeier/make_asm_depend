# Assembly Dependency Maker

[![Build Status](https://github.com/devon-artmeier/make-asm-dependencies/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/devon-artmeier/make-asm-dependencies/actions/workflows/cmake-multi-platform.yml)

Tool that makes dependency files from Assembly files.

## Usage

    make-asm-dependencies -o [output] [object file] <-i [search path]> <-r> [input file]
    
        -o [output] [object file] - Output file and object file
        <-i [search path>         - Add search path
        <-r>                      - Use relative path finding
        [input file]              - Input file
