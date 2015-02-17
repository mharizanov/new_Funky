@avrdude -c usbtiny -p m328p -e -u -U lock:w:0x3f:m -U efuse:w:0x05:m -U hfuse:w:0xDE:m -U lfuse:w:0xE2:m
@avrdude -c usbtiny -p m328p -U flash:w:RF12_Demo_atmega328.cpp.hex -U lock:w:0x0f:m
pause