#YARUP

##Yet Another Raduino Ubitx Program

----

This is a version of the Raduino Code designed to run on the Arduino *Nano
Every* Board (intead of the original Nano) and used in the uBitx v6 Radio HF
(<https://www.hfsignals.com/>).

The Arduino Every is a pin compatible version of the Arduino Nano but uses a
more powerful processor and has some differences from the original Nano.

No hardware changes (other than swapping the Nano board) are required to use
this with the uBitx v6 Radio.

It is designed to be built from the command line using arduino-cli (and a
Makefile wrapper) rather than the Arduino IDE.

This code has been derived from the standard uBitxv6 code (written by by Asshar
Farhan <https://github.com/afarhan/ubitxv6>).  Like Asshar's code it is released
under the same GPL v3 Licence.

----


It is not meant to supplant the original system and is largely a cut and paste
of Asshar's code (at least the initial version) only re-arranged to use with
arduino-cli and with enough changes to overcome the differences between the 2
processors (primarily to do with the rorary encoder system).

*There is no guarantee it will work for your purposes (or that it will work at
all!).  Use at your own risk.  I cannot guarantee it will not cause issues with
your radio.*

It is solely for experimental purposes - please feel free to take/use/modify for
your own purposes.

It may evolve over time.


Jon

