avrdude -C"C:\Users\Martin\Desktop\My stuff\IDEs\arduino-1.0.1\hardware/tools/avr/etc/avrdude.conf" -v -v -v -v -patmega32u4 -cusbtiny -e -Ulock:w:0x3F:m -Uefuse:w:0xCB:m -Uhfuse:w:0xD8:m -Ulfuse:w:0xFD:m 
avrdude -C"C:\Users\Martin\Desktop\My stuff\IDEs\arduino-1.0.1\hardware/tools/avr/etc/avrdude.conf" -v -v -v -v -patmega32u4 -cusbtiny -Uflash:w:Caterina-lilypadusb.hex:i -Ulock:w:0x2F:m 

