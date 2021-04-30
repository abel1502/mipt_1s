# SoftLang

This is my final project of the first semester: it's a compiler from my own
high-level language (Abel Source Format, .asf) to my own assembler (Abel Assembler
Format, .aaf). When paired with the corresponding assembler and soft processor
(provided in SoftProc in this repository), it can be used to create and execute programs.

## Key features
 - Multi-type support (double as dbl, int as int4, long long as int8)
 - Arithmetic operations
 - Local variables with scopes
 - Functions with recursion
 - Loops and conditionals
 - Comparisons
 - Type casts
 - Various assignments
 - Intrinsic functions for other features

## Examples
In `./programs/` are several example programs: a Fibonacci numbers generator
and a square equation solver. A formal grammar is given in `./grammar.txt`
(The notation isn't anything pre-existent, but is pretty intuitive). 
