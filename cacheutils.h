#ifndef CACHEUTILS_H
#define CACHEUTILS_H

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>

#ifndef HIDEMINMAX
#define MAX(X,Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X,Y) (((X) < (Y)) ? (X) : (Y))
#endif

uint64_t rdtsc_nofence() {
  uint64_t a, d;
  asm volatile ("rdtsc" : "=a" (a), "=d" (d));
  a = (d<<32) | a;
  return a;
}

uint64_t rdtsc() {
  uint64_t a, d;
  asm volatile ("mfence");
  asm volatile ("rdtsc" : "=a" (a), "=d" (d));
  a = (d<<32) | a;
  asm volatile ("mfence");
  return a;
}

void maccess(void* p)
{
  asm volatile ("movq (%0), %%rax\n"
    :
    : "c" (p)
    : "rax");
}

void flush(void* p) {
    asm volatile ("clflush 0(%0)\n"
      :
      : "c" (p)
      : "rax");
}

void prefetch(void* p)
{
  asm volatile ("prefetcht1 %0" : : "m" (p));
}

void longnop()
{
  asm volatile ("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
}

typedef struct map_handle_s {
  int fd;
  size_t range;
  void* mapping;
} map_handle_t;

void* map_file(const char* filename, map_handle_t** handle) {
  if (filename == NULL || handle == NULL) {
    return NULL;
  }

  *handle = calloc(1, sizeof(map_handle_t));
  if (*handle == NULL) {
    return NULL;
  }

  (*handle)->fd = open(filename, O_RDONLY);
  if ((*handle)->fd == -1) {
    return NULL;
  }

  struct stat filestat;
  if (fstat((*handle)->fd, &filestat) == -1) {
    close((*handle)->fd);
    return NULL;
  }

  (*handle)->range = filestat.st_size;

  (*handle)->mapping = mmap(0, (*handle)->range, PROT_READ, MAP_SHARED, (*handle)->fd, 0);

  return (*handle)->mapping;
}

void unmap_file(map_handle_t* handle) {
  if (handle == NULL) {
    return;
  }

  munmap(handle->mapping, handle->range);
  close(handle->fd);

  free(handle);
}

#endif

