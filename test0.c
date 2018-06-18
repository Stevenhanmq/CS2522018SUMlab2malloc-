#include <stdlib.h>
#include <stdio.h>

#include "MyMalloc.h"

/*
 * Runs a simple test to allocate memory using your allocated.
 * Modify this file or use it as a base to create your own more complex tests.
 */
int main(int argc, char **argv) {
  printf("\n---- Running test0 ---\n");

  printf("Before any allocation\n");
  print_list();

  malloc(8);
  printf("After allocation\n");
  print_list();

  exit(0);
} /* main() */
