Funkyv2/bootloaders
------------------------------------------------------------

This directory contains Funky v2 Bootloader:

caterina:
-----------------
This is the most current version of the bootloader. This bootloader uses the AVR109 avrdude sketch upload protocol.

This is a slight variation of what ships on Leonardo boards. The USB VID and PIDs are modified. And, in addition, lines 126 and 130 of Caterina.c were modified to speed up reset time. For more info on this change, check out this pull request on Arduino's github page: https://github.com/arduino/Arduino/pull/93.
	