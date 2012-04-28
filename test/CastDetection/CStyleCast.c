// RUN: %clang_cc1 %s -fsyntax-only -verify

const int i = 8;
int * ii = (int *)&i;
