avrdude -v -v  -patmega32u4 -cusbtiny -e -Ulock:w:0x3F:m -Uefuse:w:0xce:m -Uhfuse:w:0xd8:m -Ulfuse:w:0xff:m 
avrdude -v -v -patmega32u4 -cusbtiny -Uflash:w:Caterina-lilypadusb.hex:i -Ulock:w:0x2F:m 

