#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "MyMalloc.h"

// Do not set allocations to higher than 255, since we write unique values to each block

#define ALLOCATIONS 255
#define SEED 123456
#define BLOCK_SIZE 2048

/*
 * Allocate many blocks of memory, write to them, and check if anything was overwritten
 */

int main(int argc, char **argv) {
  char *ptrs[ALLOCATIONS];

  int i;
  for (i = 0; i < ALLOCATIONS; i++) {
    // Allocate some memory and store in the pointer array
    // Note: This test uses blocks of fixed sizes

    ptrs[i] = (char *) malloc(BLOCK_SIZE * sizeof(char));

    // Write to every part of allocated memory.

    memset(ptrs[i], i, BLOCK_SIZE);
  }

  print_list();

  // Verify all data

  for (i = 0; i < ALLOCATIONS; i++) {
    int j;
    for (j = 0; j < BLOCK_SIZE; j++) {
      assert(ptrs[i][j] == (char) i);
    }
  }

  // Free all the allocated memory

  for (i = 0; i < ALLOCATIONS; i++) {
    free(ptrs[i]);
    int j;
    for (j = 0; j < ALLOCATIONS; j++) {
      int k;
      for (k = 0; k < BLOCK_SIZE; k++) {
        assert(ptrs[j][k] == (char) j);
      }
    }
  }

  print_list();
} /* main() */
