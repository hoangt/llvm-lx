#include <stdio.h>

int main(int argc, char *argv[]) {
    for(int i = 0; i < argc; i++) {
        printf("chickens");
    }
    return 0;
}

// RUN: clang -c -g -emit-llvm %s -o %t.bc 
// RUN: llvm-lx -t=%s:5 %t.bc 
