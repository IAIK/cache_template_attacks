# Cache Template Attacks
This repository contains several tools to perform Cache Template Attacks.

Cache Template Attacks are a new generic attack technique, allowing to profile and exploit cache-based information leakage of any program automatically, without prior knowledge of specific software versions or even specific system information.

The underlying cache attack used in this repository is Flush+Reload as presented by Yarom and Falkner in "[FLUSH+RELOAD: a High Resolution, Low Noise, L3 Cache Side-Channel Attack](https://eprint.iacr.org/2013/448.pdf)" (2014).

# Getting started: Calibration
Before starting the Cache Template Attack you have to find the cache hit/miss threshold of your system.

Use the calibration tool for this purpose:
```
cd calibration
make
./calibration
```
This program should print a histogram for cache hits and cache misses. Based on the histogram it suggests a suitable threshold value (this value is also returned by the program).

# Getting started: Keypresses
It is helpful to start with well observable events like key strokes and an application which is known to process such events (for instance an editor).


