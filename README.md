# nanocom

**Serial port Tool**

This program is a simple tool to debug serial port communications.

Author: Pedro Zorzenon Neto - www.pzn.com.br

License: GNU GPL

Usage: run the program without arguments to see usage help

This is an old project, made in 2004 and versioned with CVS :-) Since it
is useful for me and maybe useful for others, then I dedided to import
it to GitHub

The program name was inspired by "minicom", and since it is too much
smaller, the name "nanocom" seemed to be nice ;-)

Current usage modes are "noecho", "rxonly", "cfgonly", "rxhex", "rxhext".
Check the two useful modes "rxhex" and "rxhext" that open serial port in
read-only mode (you can view the RX bytes, and can send TX bytes from
outsize nanocom, with "echo something > /dev/ttyS0" for example).

### **rxhex** mode example (shows received bytes in hex and char, similar to
"hexdump -C", updates screen in realtime, does not need to receive all
16 bytes to show on screen)

```
00000000  ab eb ff 28 00 1b 42 00  00 1b 42 c1 2a 71 0a ab  |���(..B...B�*q.�|
00000010  eb 28 00 1b 42 00 00 1b  42 ea 2a 5a 0a           |�(..B...B�*Z.   |
```

### **rxhext** mode example (shows timestamp and received byte in hex,
timestamp is in seconds from previous byte):

```
0.000000 ab
0.000064 eb
0.000022 ff
0.000031 28
0.000013 00
0.000012 1b
0.000012 42
```

## INSTALL

Compile/install procedure:
 - make
 - make install

## Notes

 - Use at your own risk. There are no warranties.

## SUPPORT ME

 - https://www.buymeacoffee.com/pzn77
