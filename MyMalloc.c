//
// CS252: MyMalloc Project
//
// The current implementation gets memory from the OS
// every time memory is requested and never frees memory.
//
// You will implement the allocator as indicated in the handout.
//
// Also you will need to add the necessary locking mechanisms to
// support multi-threaded programs.
//

#include "MyMalloc.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <pthread.h>


// This mutex must be held whenever modifying internal allocator state,
// or referencing state that is subject to change. The skeleton should
// take care of this automatically, unless you make modifications to the
// C interface (malloc(), calloc(), realloc(), and free()).

static pthread_mutex_t mutex;

// The size of block to get from the OS (2MB).

#define ARENA_SIZE ((size_t) 2097152)


// STATE VARIABLES

// Sum total size of heap

static size_t heap_size;

// Start of memory pool

static void *mem_start;

// Number of chunks requested from OS so far

static int num_chunks;

// Verbose mode enabled via environment variable
// (See initialize())

static int verbose;

// Keep track of the number of calls to each function

static int malloc_calls;
static int free_calls;
static int realloc_calls;
static int calloc_calls;

// The free list is a doubly-linked list, with a constant sentinel.

static object_header free_list_sentinel;
static object_header *free_list;


/*
 * Increments the number of calls to malloc().
 */

void increase_malloc_calls() {
  malloc_calls++;
} /* increase_malloc_calls() */

/*
 * Increase the number of calls to realloc().
 */

void increase_realloc_calls() {
  realloc_calls++;
} /* increase_realloc_calls() */

/*
 * Increase the number of calls to calloc().
 */

void increase_calloc_calls() {
  calloc_calls++;
} /* increase_calloc_calls() */

/*
 * Increase the number of calls to free().
 */

void increase_free_calls() {
  free_calls++;
} /* increase_free_calls() */

/*
 * externed version of at_exit_handler(), which can be passed to atexit().
 * See atexit(3).
 */

extern void at_exit_handler_in_c() {
  at_exit_handler();
} /* at_exit_handler_in_c() */

/*
 * Initialize the allocator by setting initial state
 * and making the first allocation.
 */

void initialize() {
  // Set this environment variable to the specified value
  // to disable verbose logging.

#define VERBOSE_ENV_VAR "MALLOCVERBOSE"
#define VERBOSE_DISABLE_STRING "NO"

  pthread_mutex_init(&mutex, NULL);

  // We default to verbose mode, but if it has been disabled in
  // the environment, disable it correctly.

  const char *env_verbose = getenv(VERBOSE_ENV_VAR);
  verbose = (!env_verbose || strcmp(env_verbose, VERBOSE_DISABLE_STRING));

  // Disable printf's buffer, so that it won't call malloc and make
  // debugging even more difficult

  setvbuf(stdout, NULL, _IONBF, 0);

  // Get initial memory block from OS

  void *new_block = get_memory_from_os(ARENA_SIZE +
                                       (2 * sizeof(object_header)) +
                                       (2 * sizeof(object_footer)));

  // In verbose mode register function to print statistics at exit

  atexit(at_exit_handler_in_c);

  // Establish memory locations for objects within the new block

  object_footer *start_fencepost = (object_footer *) new_block;
  object_header *current_header =
    (object_header *) ((char *) start_fencepost +
                              sizeof(object_footer));
  object_footer *current_footer =
    (object_footer *) ((char *) current_header +
                              ARENA_SIZE +
                              sizeof(object_header));
  object_header *end_fencepost =
    (object_header *) ((char *) current_footer +
                              sizeof(object_footer));

  // Establish fenceposts
  // We set fencepost size to 0 as an arbitrary value which would
  // be impossible as a value for a valid memory block

  start_fencepost->status = ALLOCATED;
  start_fencepost->object_size = 0;

  end_fencepost->status = ALLOCATED;
  end_fencepost->object_size = 0;
  end_fencepost->next = NULL;
  end_fencepost->prev = NULL;

  // Establish main free object

  current_header->status = UNALLOCATED;
  current_header->object_size = ARENA_SIZE +
                                sizeof(object_header) +
                                sizeof(object_footer);

  current_footer->status = UNALLOCATED;
  current_footer->object_size = current_header->object_size;

  // Initialize free list and add the new first object

  free_list = &free_list_sentinel;
  free_list->prev = current_header;
  free_list->next = current_header;
  current_header->next = free_list;
  current_header->prev = free_list;

  // Mark sentinel as such. Do not coalesce the sentinel.

  free_list->status = SENTINEL;
  free_list->object_size = 0;

  // Set start of memory pool

  mem_start = (char *) current_header;
} /* initialize() */

/*
 * Allocate an object of size size. Ideally, we can allocate from the free list,
 * but if we don't have a free object large enough, go get more memory from the
 * OS. Return a pointer to the newly allocated memory.
 *
 * TODO: Add your code to allocate memory from the free list, instead of getting
 * it from the OS every time.
 */

void *allocate_object(size_t size) {
  // SIZE_PRECISION determines how to round.
  // By default, round up to nearest 8 bytes.
  // It must be a power of 2.
  // MINIMUM_SIZE is the minimum size that can be requested, not including
  // header and footer. Smaller requests are rounded up to this minimum.

#define SIZE_PRECISION (8)
#define MINIMUM_SIZE (8)

  if (size < MINIMUM_SIZE) {
    size = MINIMUM_SIZE;
  }

  if (size%8 != 0) {
    size = (size/8 + 1) * 8;
  }
  // Add the object_header/Footer to the size and round the total size
  // up to a multiple of 8 bytes for alignment.
  // Bitwise-and with ~(SIZE_PRECISION - 1) will set the last x bits to 0,
  // if SIZE_PRECISION = 2**x.

  size_t rounded_size = (size +
                         sizeof(object_header) +
                         sizeof(object_footer) +
                         (SIZE_PRECISION - 1)) & ~(SIZE_PRECISION - 1);

  // For now, naively get the memory from the OS every time.
  // (You need to change this.)
  
  object_header *tmp_header = free_list->next;
  if (free_list->next == free_list) {
    //check if the memory has been used all up
    //printf("i am here\n");
    void *new_block = get_memory_from_os(ARENA_SIZE +
					 (2 * sizeof(object_header)) +
					 (2 * sizeof(object_footer)));
    object_footer *start_fencepost = (object_footer *) new_block;
    object_header *current_header =
      (object_header *) ((char *) start_fencepost +
			 sizeof(object_footer));
    object_footer *current_footer =
      (object_footer *) ((char *) current_header +
			 ARENA_SIZE +
			 sizeof(object_header));
    object_header *end_fencepost =
      (object_header *) ((char *) current_footer +
			 sizeof(object_footer));
    start_fencepost->status = ALLOCATED;
    start_fencepost->object_size = 0;
    
    end_fencepost->status = ALLOCATED;
    end_fencepost->object_size = 0;
    end_fencepost->next = NULL;
    end_fencepost->prev = NULL;

    current_header->status = UNALLOCATED;
    current_header->object_size = ARENA_SIZE +
                                  sizeof(object_header) +
                                  sizeof(object_footer);
    
    current_footer->status = UNALLOCATED;
    current_footer->object_size = current_header->object_size;
    free_list->prev = current_header;
    free_list->next = current_header;
    current_header->next = free_list;
    current_header->prev = free_list;
  }
  tmp_header = free_list->next;
  while (tmp_header != free_list) {
    
    //decide which approach: split, not split and ask for new memory
    int blank_size = tmp_header->object_size - sizeof(object_footer)
                                        - sizeof(object_header);    
    if (tmp_header->object_size >=rounded_size + sizeof(object_header)
	                                       + sizeof(object_footer)
	                                       + MINIMUM_SIZE) {
      //            printf("here \n");
      //printf("if i dont see this 3 times\n");
      object_footer *new_footer =
	(object_footer *) ((char *) tmp_header + rounded_size
			   - sizeof(object_footer));
      //   int blank_size = tmp_header->object_size - sizeof(object_footer)
      //                                - sizeof(object_header);
      object_footer *old_footer =
	(object_footer *) ((char *) tmp_header
			          + sizeof(object_header)
	              		  + blank_size);
      old_footer->object_size = old_footer->object_size - rounded_size;
      //            printf("%d\n",(int)old_footer->object_size);
      tmp_header->status = ALLOCATED;
      tmp_header->object_size = rounded_size;
      new_footer->object_size = rounded_size;
      new_footer->status = ALLOCATED;
      object_header *new_header =
	(object_header *) ((char *) tmp_header + rounded_size);
      //printf("new header address: %p\n",&new_header);
      // printf("tmp header address: %p\n",&tmp_header);
      // printf("sentinel address: %p\n",&free_list);

      new_header->status = UNALLOCATED;
      //      printf("sentinel->next address: %p\n",&(free_list->next));
      new_header->object_size = old_footer->object_size;
      tmp_header->prev->next = new_header;
      new_header->next = tmp_header->next;
      new_header->prev = tmp_header->prev;
      new_header->next->prev = new_header;
      break;
    }
    else if (tmp_header->object_size >= rounded_size &&
	     tmp_header->object_size < rounded_size
	                              + sizeof(object_header)
	                              + sizeof(object_footer)
	                              + MINIMUM_SIZE) { /*situation of 
                                                          don't need split*/
      // printf("anybody see me ?????\n");
      size = tmp_header->object_size - sizeof(object_header)
                                     - sizeof(object_footer);
      object_footer *tmp_footer =
	(object_footer*)((char*)tmp_header +
                               size + sizeof(object_header));
      tmp_header->status = ALLOCATED;
      tmp_footer->status = ALLOCATED;
      tmp_header->next->prev = tmp_header->prev;
      tmp_header->prev->next = tmp_header->next;
      // printf("anybody see me finish?????\n");
      break;
    }
    else {      /* situation of need to look keep looking*/
      if (tmp_header->next == free_list) {

        //situation when there is not a block big enough

        void *new_block = get_memory_from_os(ARENA_SIZE +
                                         (2 * sizeof(object_header)) +
	                                 (2 * sizeof(object_footer)));
        object_footer *start_fencepost = (object_footer *) new_block;
        object_header *current_header =
          (object_header *) ((char *) start_fencepost +
			     sizeof(object_footer));
	object_footer *current_footer =
	  (object_footer *) ((char *) current_header +
			     ARENA_SIZE +
			     sizeof(object_header));
	object_header *end_fencepost =
	  (object_header *) ((char *) current_footer +
			     sizeof(object_footer));
	start_fencepost->status = ALLOCATED;
	start_fencepost->object_size = 0;
	
	end_fencepost->status = ALLOCATED;
	end_fencepost->object_size = 0;
	end_fencepost->next = NULL;
	end_fencepost->prev = NULL;
	
	current_header->status = UNALLOCATED;
	current_header->object_size = ARENA_SIZE +
	                              sizeof(object_header) +
	                              sizeof(object_footer);
	
	current_footer->status = UNALLOCATED;
	current_footer->object_size = current_header->object_size;
    	tmp_header->next->prev = current_header;
        current_header->next = tmp_header->next;
	tmp_header->next = current_header;
	current_header->prev = tmp_header;
      }
      tmp_header = tmp_header->next;
    }
  }
  

//object_header *new_object = (object_header *) new_block;
//  new_object->object_size = rounded_size;

  // Return a pointer to usable memory

  return (void *) (tmp_header + 1);
} /* allocate_object() */

/*
 * Free an object. ptr is a pointer to the usable block of memory in
 * the object. If possible, coalesce the object, then add to the free list.
 *
 * TODO: Add your code to free the object
 */

void free_object(void *ptr) {

  //first find the right location where the address of the freed object
  //should be at,
  //and then find out that if it should merge with adjacent blocks

  object_header *tmp_header = (object_header*)((char *) ptr
      - sizeof(object_header));
  object_header *iter_header = free_list;
  while (iter_header->next < tmp_header ) {
    iter_header = iter_header->next;
    printf("here we have a loop");
  }
  object_header *old_header = iter_header->next;
  iter_header->next = tmp_header;
  tmp_header->prev = iter_header;
  tmp_header->next = old_header;
  old_header->prev = tmp_header;
  object_footer *tmp_footer = (object_footer*)((char*)tmp_header
					       + tmp_header->object_size
					       - sizeof(object_footer));
  tmp_header->status = UNALLOCATED;
  tmp_footer->status = UNALLOCATED;
  object_header *next_header = (object_header*)((char *)tmp_header
					       + tmp_header->object_size);
  
  object_footer *next_footer = (object_footer*)((char *)next_header
					       + next_header->object_size
					       - sizeof(object_footer));
  
  object_footer *prev_footer = (object_footer*)((char *)tmp_header
					       - sizeof(object_footer));
  
  object_header *prev_header = (object_header*)((char *)tmp_header
					       - prev_footer->object_size);
  if(next_header->status == UNALLOCATED){
    printf("next_header->status == UNALLOCATED\n");  
  }
  if(prev_header->status == UNALLOCATED){
    printf("prev_header->status == UNALLOCATED\n");
  }
  if(next_footer->object_size != 0){
    printf("next_footer->object_size != 0\n");
  }
  if(prev_footer->object_size != 0){
    printf("prev_footer->object_size != 0\n");
  }

  
  if (next_header->status == UNALLOCATED
      && prev_header->status == UNALLOCATED
      && next_footer->object_size != 0
      && prev_footer->object_size != 0) {       // merge both
    printf("entered 1\n");
    prev_header->object_size += tmp_header->object_size
                              + next_header->object_size;
    next_footer->object_size = prev_header->object_size;
    prev_header->next = next_header->next;
    prev_header->next->prev = prev_header;
  }
  else if (
	   (next_header->status == UNALLOCATED
	    && prev_header->status == ALLOCATED)||
	   (next_header->status == UNALLOCATED
	    && prev_header->status == UNALLOCATED
	    && next_footer->object_size != 0)
	   ) {    // merge right
    printf("entered 2\n");
    tmp_header->object_size += next_header->object_size;
    next_footer->object_size =  tmp_header->object_size;
    tmp_header->next = next_header->next;
    tmp_header->next->prev = tmp_header;
  }
  else if ((next_header->status == UNALLOCATED
           && prev_header->status == UNALLOCATED)||
	   (next_header->status == UNALLOCATED
	    && prev_header->status == UNALLOCATED
	    && prev_footer->object_size != 0)) {  // merge left
    printf("entered 3\n");
    prev_header->object_size += tmp_header->object_size;
    tmp_footer->object_size = prev_header->object_size;
    prev_header->next = tmp_header->next;
    prev_header->next->prev = prev_header;  
  }
  else {
     printf("entered 4\n");// don't merge
  }
  return;
  
} /* free_object() */

/*
 * Return the size of the object pointed by ptr. We assume that ptr points * usable memory in a valid obejct.
 */

size_t object_size(void *ptr) {
  // ptr will point at the end of the header, so subtract the size of the
  // header to get the start of the header.

  object_header *object =
    (object_header *) ((char *) ptr - sizeof(object_header));

  return object->object_size;
} /* object_size() */

/*
 * Print statistics on heap size and
 * how many times each function has been called.
 */

void print_stats() {
  printf("\n-------------------\n");

  printf("HeapSize:\t%zd bytes\n", heap_size);
  printf("# mallocs:\t%d\n", malloc_calls);
  printf("# reallocs:\t%d\n", realloc_calls);
  printf("# callocs:\t%d\n", calloc_calls);
  printf("# frees:\t%d\n", free_calls);

  printf("\n-------------------\n");
} /* print_stats() */

/*
 * Print a representation of the current free list.
 * For each object in the free list, show the offset (distance in memory from
 * the start of the memory pool, mem_start) and the size of the object.
 */

void print_list() {
  pthread_mutex_lock(&mutex);

  printf("FreeList: ");

  object_header *ptr = free_list->next;

  while (ptr != free_list) {
    long offset = (long) ptr - (long) mem_start;
    printf("[offset:%ld,size:%zd]", offset, ptr->object_size);
    ptr = ptr->next;
    if (ptr != free_list) {
      printf("->");
    }
  }
  printf("\n");

  pthread_mutex_unlock(&mutex);
} /* print_list() */

/*
 * Use sbrk() to get the memory from the OS. See sbrk(2).
 */

void *get_memory_from_os(size_t size) {
  heap_size += size;

  void *new_block = sbrk(size);

  num_chunks++;

  return new_block;
} /* get_memory_from_os() */

/*
 * Run when the program exists, and prints final statistics about the allocator.
 */

void at_exit_handler() {
  if (verbose) {
    print_stats();
  }
} /* at_exit_handler() */



//
// C interface
//


/*
 * Allocates size bytes of memory and returns the pointer to the
 * newly-allocated memory. See malloc(3).
 */

extern void *malloc(size_t size) {
  pthread_mutex_lock(&mutex);
  increase_malloc_calls();

  void *memory = allocate_object(size);

  pthread_mutex_unlock(&mutex);

  return memory;
} /* malloc() */

/*
 * Frees a block of memory allocated by malloc(), calloc(), or realloc().
 * See malloc(3).
 */

extern void free(void *ptr) {
  pthread_mutex_lock(&mutex);
  increase_free_calls();

  if (ptr != NULL) {
    free_object(ptr);
  }

  pthread_mutex_unlock(&mutex);
} /* free() */

/*
 * Resizes the block of memory at ptr, which was allocated by
 * malloc(), realloc(), or calloc(), and returns a pointer to
 * the new resized block. See malloc(3).
 */

extern void *realloc(void *ptr, size_t size) {
  pthread_mutex_lock(&mutex);
  increase_realloc_calls();

  void *new_ptr = allocate_object(size);

  pthread_mutex_unlock(&mutex);

  // Copy old object only if ptr is non-null

  if (ptr != NULL) {
    // Copy everything from the old ptr.
    // We don't need to hold the mutex here because it is undefined behavior
    // (a double free) for the calling program to free() or realloc() this
    // memory once realloc() has already been called.

    size_t size_to_copy =  object_size(ptr);
    if (size_to_copy > size) {
      // If we are shrinking, don't write past the end of the new block

      size_to_copy = size;
    }

    memcpy(new_ptr, ptr, size_to_copy);

    pthread_mutex_lock(&mutex);
    free_object(ptr);
    pthread_mutex_unlock(&mutex);
  }

  return new_ptr;
} /* realloc() */

/*
 * Allocates contiguous memory large enough to fit num_elems elements
 * of size elem_size. Initialize the memory to 0. Return a pointer to
 * the beginning of the newly-allocated memory. See malloc(3).
 */

extern void *calloc(size_t num_elems, size_t elem_size) {
  pthread_mutex_lock(&mutex);
  increase_calloc_calls();

  // Find total size needed

  size_t size = num_elems * elem_size;

  void *ptr = allocate_object(size);

  pthread_mutex_unlock(&mutex);

  if (ptr) {
    // No error, so initialize chunk with 0s

    memset(ptr, 0, size);
  }

  return ptr;
} /* calloc() */
