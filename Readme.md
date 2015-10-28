# HRM-CCPU
## An Assembler and Runtime for Human Resource Machine programs, written in C++

An attempt at running programs written in HRM's assembly language in C++. This version is an early version with many of the planned features mising. Currently, this program can read, assemble, and execute the programs. In the future, this program will be able to export the assembled binary (in a intermediate bytecode I made to improve runtime performance), cound the number of execution cycles, use a seperate Inbox input system, compile to C (for generating native executables), and support a simple binary storage type (so removing overflow/underflow/mixed-type checks).

**This is a very early version**. Many important features such as a `--help` menu haven't been implemented yet. Please follow the instructions carefully.

### Usage

I've provided a Makefile that should work on Unix-like systems (ex: Mac OS X and Linux). Running `make` will build the executable in the same directory as the source code.

Usage: `./hrm-ccpu <file>` will run the code in the file. If file is `--` or no file is provided, it will read from standard in.

Error handling is not finished yet, so a failure at any point will just exit the script with an error code `1`.

The initial memory state and inbox are handled via a comment in the program source. Start the comment with `--#` to begin the data. The first number is the number of avaiable memory locations avaiable to the program. Each pair of numbers after indicate the memory index and value at the index before the program starts. Next, is a `:`, and then the values that come from the Inbox, starting with the first to be read. Finally, the comment ends with `#--`. This entire comment needs to be on a single line. An example for problem 20 is `--# 10 9 0 : 9 4 1 7 7 0 0 8 4 2 #--`

### HRM-SVG

Included is another project that can export the comments and labels made in the game. It requires zlib to build and run. It can be build by running `make hrm-svg` or `make all`. When run, it will produce an SVG image of the comment or label. It's in a roughly written stage currently, and will likely be merged into HRM-CCPU.

### Related Projects

 * [hrm-cpu](https://github.com/nrkn/hrm-cpu) - A similar project to this one, but written in Javascript.
 * [hrm-hrm-image-decoder](https://github.com/nrkn/hrm-image-decoder) - A javascript program similar to HRM-SVG, but with raw path exporting instead.
 * [hrmsandbox](https://github.com/sixlettervariables/hrmsandbox) - Another interpreter in Javacript, but includes a web interface and a more flexible interpreter.
 * [hrm-level-data](https://github.com/atesgoral/hrm-level-data) - Has metadata for each level in the game. Useful for generating the initial state comments.
 * [hrm-solutions](https://github.com/atesgoral/hrm-solutions) - A collection of solutions for each level in the game.

### Licence

Licenced under The MIT Licence (MIT)
