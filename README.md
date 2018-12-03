## Universal Test for Linux
**Info:** Test program for the Universal Library for Linux. The UL for Linux binary name is libuldaq.

**Author:** Measurement Computing DV

## About
Tests the API (Application Programming Interface) for interacting with MCC data acquisition and control hardware. 

### Prerequisites:
---------------
Requires the **uldaq** package.

Also requires git, the development package for libusb, and Qt runtime libraries. 
The following describes how these prerequisites can be installed.
  
  - Debian-based Linux distributions such as Ubuntu 18.04, Raspbian
  
```sh
     $ sudo apt-get update
     $ sudo apt-get install git
     $ sudo apt install qtbase5-dev
     $ sudo apt install qt5-default
```

### Build Instructions
---------------------

1. Download and install the latest version of **uldaq** per instructions on the mccdaq/uldaq GitHub page.

2. Clone the universallibrary repository onto your linux system:
 
```sh
  $ cd ~
  $ git clone https://github.com/MccDv/universaltest.git
```
  
3. Run the following commands to build the universaltest program:

```sh
  $ cd universaltest
  $ qmake
  $ make
```


