# Cache Template Attacks
This repository contains several tools to perform Cache Template Attacks.

Cache Template Attacks are a new generic attack technique, allowing to profile and exploit cache-based information leakage of any program automatically, without prior knowledge of specific software versions or even specific system information.

The underlying cache attack used in this repository is Flush+Reload as presented by Yarom and Falkner in "[FLUSH+RELOAD: a High Resolution, Low Noise, L3 Cache Side-Channel Attack](https://www.usenix.org/conference/usenixsecurity14/technical-sessions/presentation/yarom)" (2014).

The "[Cache Template Attacks](https://www.usenix.org/conference/usenixsecurity15/technical-sessions/presentation/gruss)" paper by Gruss, Spreitzer and Mangard was published at USENIX Security 2015.

## One note before starting

The programs should work on x86-64 Intel CPUs independent of the operating system (as long as you can compile it). Different OS specific version of the tools are provided in subfolders.

**Warning:** This code is provided as-is. You are responsible for protecting yourself, your property and data, and others from any risks caused by this code. This code may not detect vulnerabilities of your applications. This code is only for testing purposes. Use it only on test systems which contain no sensitive data.

## Getting started: Calibration
Before starting the Cache Template Attack you have to find the cache hit/miss threshold of your system.

Use the calibration tool for this purpose:
```
cd calibration
make
./calibration 10
```
This program should print a histogram for cache hits and cache misses. The number specifies the bucket size for the histogram.

## Profiling
In this example we perform some steps by hand to illustrate what happens in the background.
Therefore, we will first find the address range to attack.
```
cat /proc/`pidof gedit`/maps | grep libgedit | grep "r-x"
7fe0674cf000-7fe067511000 r-xp 0001d000 103:04 2768721                   /usr/lib/x86_64-linux-gnu/gedit/libgedit-44.so
```
We do not care about the virtual addresses, but only the size of the address range and the offset in the file (which is 00000000 in this example). This is also the reason why we don't have to think about address space layout randomization.

**Note:** On Windows you can use the tool vmmap to find the same information.

This line can directly be passed to the profiling tool in the following step. We will create a Cache Template in this step.

During the profiling you need to perform or simulate a huge number of key presses. The profiling tool gives you 2 seconds of time to switch windows, e.g., to an already opened gedit window.

On Linux, run the following lines:
```
cd profiling
make
./spy 155 100 7fe0674cf000-7fe067511000 r-xp 0001d000 103:04 2768721                   /usr/lib/x86_64-linux-gnu/gedit/libgedit-44.so
```

Choose a cache line with a nice number of cache hits and pass it to the generic exploitation spy tool:
```
cd exploitation
make
./spy 155 /usr/lib/x86_64-linux-gnu/gedit/libgedit-44.so 0x22980
```
Now this tool prints a message exactly when a user presses a key (in gedit).
This spy tool can also be used on Windows just like that.

**Note:** Cache Template Attacks can be fully automated, but this requires the event to be automated as well.

## OpenSSL AES T-Table attack
This example requires a self-compiled OpenSSL library to enable it's T-Table-based AES implementation.
Place libcrypto.so in the same folder and make sure the program actually uses it as a shared library.
Then run
```
cd profiling_aes_example
make
./spy
```
The T-Table is easily locatable in the log file as there are only 64 addresses which are frequently accessed, but not always accessed.
Subsequently, you can monitor addresses from the profile to derive information about secret keys.

In the exploitation phase the spy tool has to trigger encryptions itself with an unknown key and can then trivially determine the upper 4 bits of each key byte after about 64 encryptions.

Of course, we know that OpenSSL does not use a T-Table-based AES implementation anymore. But you can use this tool to profile any (possibly closed-source) binary to find whether it contains a crypto algorithm which leaks key dependent information through the cache. Just trigger the algorithm execution with fixed keys 


That's it, now it's up to you to find out which of your software leaks data and how it could be exploited. I hope it helps you closing these leaks.
