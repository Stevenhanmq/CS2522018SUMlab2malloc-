#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

#include "MyMalloc.h"

#define ALLOCATIONS 10
#define SBRK_SIZE 4096

/*
 * Allocate multiple OS chunks, calling sbrk halfway through
 */

int main(int argc, char **argv) {
  // 2 megabytes, an entire OS chunk

  size_t size = 2097152;
  char *ptrs[ALLOCATIONS];

  int i;
  for (i = 0; i < ALLOCATIONS; i++) {
      ptrs[i] = (char*) malloc(size);
      print_list();

      // Write to every part of the memory

      memset(ptrs[i], 'a', size);
      assert(ptrs[i][size - 1] == 'a');
      if (i == ALLOCATIONS / 2) {
          sbrk(SBRK_SIZE);
      }
  }

  for(i = 0; i < ALLOCATIONS; i++){
      free(ptrs[i]);
      print_list();
  }
} /* main() */
