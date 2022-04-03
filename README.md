# YARUP

## Yet Another Raduino Ubitx Program, using Nano Every

----

This is a version of the Raduino Code designed to run on the Arduino *Nano
Every* Board (intead of the original Nano) and used in the uBitx v6 Radio HF
(<https://www.hfsignals.com/>).

The Arduino Every is a pin compatible version of the Arduino Nano but uses a
more powerful processor and has some differences from the original Nano.

No hardware changes (other than swapping the Nano board and using a different
style USB cable) are required to use this with the uBitx v6 Radio.

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

# How to Build the Program

1. Install arduino-cli
	see https://arduino.github.io/arduino-cli/0.21/installation/

NB you will need to install the approriate cores for Nano Every.

```
arduino-cli core install arduino:megaavr
```

If you install using snap, you may need to do a couple of extra steps

or it will balk when you do a make upload:
```
snap connect arduino-cli:raw-usb
```
and
```
snap connect arduino-cli:serial-port
```


2. Clone the repository

3. do:
```
make compile
```
This will build the image ready to upload to the Arduino board.  Assuming no
problems in the compile you can install it on a Nano every Board.

# Installing

1. Make sure you have an Arduino Every Board.  NB When preparing the board,
the headers need to be installed the same as the standard Raduino Nano board
(ie with the pins "upside down" - the reset button will be effectively on the
underside of the board when it gets installed in the uBitx radio).

2. Connect the Nano Every to PC via USB cable.

3. Do:
```
make upload
```

4. Hopefully it should upload the image to the board.  The board should be a simple swap with the original board.

----

# Can it be used with the Arduino IDE?

Probably... you may need to shuffle bits around - I will look at this later and
see what is involved.  I find the Arduino IDE gets in my way, mostly because I
am a dinosaur who refuses to evolve :)


