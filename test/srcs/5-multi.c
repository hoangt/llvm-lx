#include <stdio.h>

int main(int argc, char *argv[]) {
    for(int i = 0; i < argc; i++) {
        printf("chickens");
    }
    for(int j = 0; j < argc; j++) {
        printf("turkeys");
    }
    return 0;
}

// RUN: clang -c -g -emit-llvm %s -o %t.bc 
// RUN: llvm-lx -t=%s:4,%s:7 %t.bc 
