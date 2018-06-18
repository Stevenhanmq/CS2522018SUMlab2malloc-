#ifndef MYMALLOC_H
#define MYMALLOC_H

#include <unistd.h>

enum allocation_status {
  UNALLOCATED,
  ALLOCATED,
  SENTINEL
};

struct object_header_struct {
  // Size of the object including header and footer

  size_t object_size;

  // Allocation/sentinel status

  enum allocation_status status;

  // Free list pointers

  struct object_header_struct *next;
  struct object_header_struct *prev;
};
typedef struct object_header_struct object_header;

struct object_footer_struct{
  // Size of the object including header and footer

  size_t object_size;

  // Allocation/sentinel status

  enum allocation_status status;
};
typedef struct object_footer_struct object_footer;

// Direct gcc to run this function before main()

void initialize() __attribute__ ((constructor));

void *allocate_object(size_t size);

void free_object(void *ptr);

size_t object_size(void *ptr);

void at_exit_handler();

void print_stats();

void print_list();

void *get_memory_from_os(size_t size);

void print_list();

#endif // MYMALLOC_H
