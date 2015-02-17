@echo off
echo This will overwrite existing .HEX! Press any key..
pause
avrdude -v -v -v -v -patmega32u4 -cusbtiny -P usb -U flash:r:current_firmware.hex:i