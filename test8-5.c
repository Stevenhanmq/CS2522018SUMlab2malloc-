#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "MyMalloc.h"

#define ALLOCATIONS 1000
#define SEED 123456
#define MAX_SIZE 4096

/*
 * Generate an array of unique random numbers in linear time.
 * Implementation found at https://stackoverflow.com/a/1608254
 */

void getRandomVector(int* list, size_t size) {
  int i;
  for (i = 0; i < size; i++) {
    list[i] = i;
  }

  for (i = 0; i < size; i++) {
    int j = i + rand() % (size - i);
    int temp = list[i];
    list[i] = list[j];
    list[j] = temp;
  }
} /* getRandomVector() */

/*
 * Allocate lots of pseudo-random sized blocks of memory, write to them, then free them at random
 */

int main(int argc, char **argv) {
  // Set the random seed

  srand(SEED);

  char *ptrs[ALLOCATIONS];

  int i;
  for (i = 0; i < ALLOCATIONS; i++) {
    // Get a random size, no greater than MAX_SIZE

    size_t alloc_size = rand() % MAX_SIZE;

    // Allocate some memory and store in the pointer array

    ptrs[i] = (char *) malloc(alloc_size * sizeof(char));

    // Write to every part of allocated memory.

    memset(ptrs[i], 255, alloc_size);
  }

  print_list();

  int vector[ALLOCATIONS];
  getRandomVector(vector, ALLOCATIONS);

  // Free all the allocated memory

  for (i = 0; i < ALLOCATIONS; i++) {
    free(ptrs[vector[i]]);
  }

  print_list();
} /* main() */
