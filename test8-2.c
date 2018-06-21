#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "MyMalloc.h"

/*
 * Allocates an entire OS chunk
 */

int main(int argc, char **argv) {
  // 2 megabytes, an entire OS chunk

  size_t size = 2097152;
  char *mem = (char*) malloc(size);
  print_list();

  // Write to every part of the memory

  memset(mem, 'a', size);
  assert(mem[size - 1] == 'a');
  free(mem);
  print_list();
} /* main() */
