
# Introduction

this is the start of a Depo to implement several basic I/O functions with fast 
GPIO access of the VideoCore


Work based on this:
https://github.com/hermanhermitage/videocoreiv/tree/master/blinker01


This is a nice tool to dump the GPIO Status:
http://www.susa.net/wordpress/2012/07/raspberry-pi-gpfsel-gpio-and-pads-status-viewer/


Connection Setup
The SWD Data line is bi directional. SPI HW can drive/read this line for
the bulk of Data. Switching forth and back is done with bitbang.

For the start we use 
Pin 38 - BCM 20 - MOSI - SWD Data, ALT Function 4
Pin 39 - Ground
Pin 40 - BCM 21 - SP1_SCLK  - SWD CLK, ALT Function 4


  FSEL20 (GPFSEL2[00-02]) = 0 (GPIO In - [Low])
  FSEL21 (GPFSEL2[03-05]) = 0 (GPIO In - [Low])

