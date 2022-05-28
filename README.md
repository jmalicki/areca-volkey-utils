The Areca RAID arrays, such as the [8050T3](https://www.areca.com.tw/products/thunderbolt-8050T3.html)
series, support encrypted arrays.

However, to do this, one of the steps requires generating a key using either Cli64, the Areca commandline
client, that only works for PCIE card controllers in the computer, or the Areca API, which requires you to
write your own code to use it, so I did.

This requires your Areca RAID controller be hooked up to ethernet.

## Prerequisites
* boost
* curl
* Linux (should be very easy to adapt the Makefile to other targets supported by the library,
i.e. MacOS, FreeBSD, Solaris, and Windows.)

## Building
Just type make.

This will download the API library files and build.

## How to use
1. In the Areca Web UI, create a new volume with full-volume encryption, and choose the key type you want.
1. Make sure you select initialization (preferably foreground), as this tool doesn't seem to work until
  initialization is complete (feel free to experiment if you want to work on this, I couldn't make it happen).
1. Once initialization of the RAID volume has completed, run this tool, giving the IP and admin password:
  ```areca-volkey --ip 192.168.0.1 --password 0000 --outfile arecaKey.bin```
1. The file created upon success is your key, for all practical purposes a one time pad.  Guard it well and make
sure you have ways of recovering it before you begin using the array.
1. In the Areca UI, upload the key.  This will be required every time the array powers off.