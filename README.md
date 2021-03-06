# Realtime Metronome

NTU MA4830 Realtime Software **Major** Project AY20/21
* Generate a precise, uninterrupted, and regular beat
* Accept changes to beat rates in beats/minute via the keyboard in **real-time**
* Load and save metronome's setting into a configuration file


## Table of Contents

   1. [Getting started](#1-getting-started)
   2. [Prerequisites](#2-prerequisites)
   3. [Installing](#3-installing)
   4. [Run](#4-run)

## 1. Getting started

Welcome to **Realtime Metronome** project! There are just a few steps to get you started with developing!

## 2. Prerequisites

1. Compute
    * Any computer
2. Software package
    * gcc >= 7.5.0 (for Ubuntu 18.04 LTS)

## 3. Installing

To install gcc in Ubuntu 18.04 LTS, run
```bash
sudo apt-get update && sudo apt-get install -y gcc libc6-dev --no-install-recommends
```
## 4. Run

* To compile the program

    * On Ubuntu 18.04 LTS
        ```bash
        gcc -o main main.c -lrt -lpthread
        ```
    * On QNX 6.XX
        ```bash
        cc -o main main.c
        ```

* To run the executable
    ```bash
    ./main
    ```

* To run the executable with loading & saving the metronome's setting into a configuration file
    ```bash
    ./main --config config.txt
    ```
