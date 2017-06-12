# llvm-lx
LLVM Targeted Loop Extractor

[![Build Status](https://travis-ci.org/sfu-arch/llvm-lx.svg?branch=master)](https://travis-ci.org/sfu-arch/llvm-lx)

## Usage

1. Compile the target workload into LLVM 4.0 bitcode with no optimizations and debug symbols embedded.   
2. Specify the target loop to extract using the file name and line number.   
3. The tool (`llvm-lx`) produces a `*.lx.bc` which has the target loop extracted into a function `__lx*`.   

## Example

1. `clang -c -g -emit-llvm test.c`    
Produces `test.bc` with debug info.

2. `llvm-lx -t=test.c:4 test.bc`
Prints `[llvm-lx] Found loop at test.c:4`   
Produces `test.lx.bc` which has the loop at line 4 extracted into function called `__lx_test_4`.

## Notes

1. Extract Multiple - Multiple loops may be extracted by specifying each on the commandline as comma separated locations. Eg. `llvm-lx -t=file1.c:4,file2.c6, file1.c:22`. However, when the loops are in close proximity the code extractor may crash.  
    a. Loop Visit Order - Loops are visited in reverse topological order, innermost to outer loop.   

2. [Whole Program Bitcode](https://github.com/travitch/whole-program-llvm) - Use this project to compile entire workloads to bitcode. Note that this tools requires debug info present in the bitcode.

3. Debugging - In case the loop you expect to find is missing, you can use the `-d` flag while running `llvm-lx` to dump the file name and line number of each loop it encounters into a file called `loop-locs.txt` 

4. Specifying Loop - Loop location is specified using the file name (basename) only. A fully qualified path will be stripped to use the basename only. Loops occurring on the same line and same filename within different source locations of the same project are not supported. 

5. Extracted Function Name - The extracted function name is constructed by appending to `__lx`, the filename (including extension) and line number where the `_` character is used as delimiter.    

6. Persistence of extracted function - The loop extracted as a function is marked as `noinline` so it should not be removed in subsequent stages of optimization. 
