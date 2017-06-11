#include <stdio.h>

void foo(int argc) {
    for(int i = 0; i < argc; i++) {
        printf("chickens");
    }
}

int main(int argc, char *argv[]) {
    foo(argc);
    return 0;
}

// RUN: clang -c -g -emit-llvm %s -o %t.bc 
// RUN: llvm-lx -t=%s:4 %t.bc 
