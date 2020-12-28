## Universal Test for Linux
**Info:** Test program for the Universal Library for Linux. The UL for Linux binary name is libuldaq.

**Author:** Measurement Computing DV

## About
Tests the API (Application Programming Interface) for interacting with MCC data acquisition and control hardware. 

### Prerequisites:
---------------
Requires the **uldaq** package.

Download and install the latest version of **uldaq** per instructions on the mccdaq/uldaq GitHub page.

Also requires git, the development package for libusb, and Qt runtime libraries. 
The following describes how these prerequisites can be installed.
  
  - Debian-based Linux distributions such as Ubuntu 18.04, Raspbian
  
```sh
     $ sudo apt-get update
     $ sudo apt-get install git
     $ sudo apt install qtbase5-dev
     $ sudo apt install qt5-default
```
May also need:
```sh
     $ sudo apt install mesa-common-dev
     $ sudo apt-get install build-essential libgl1-mesa-dev
```

  - Arch-based Linux distributions such as Manjaro
  
```sh
     $ sudo pacman -S qt5-base
```
  - Mac OS
       >Requires xCode and Homebrew. If you installed the **uldaq** above, these should already be installed. Proceed to the Qt installation below.
```sh
     $ brew install qt5
```

### Build Instructions
---------------------

1. Clone the universaltest repository onto your linux system:
 
```sh
  $ cd ~
  $ git clone https://github.com/MccDv/universaltest.git
```
  
2. Run the following commands to build the universaltest program:

```sh
  $ cd universaltest
  $ qmake
  $ make
```


