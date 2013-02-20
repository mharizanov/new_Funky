Funky v2
=========

The Funky v2 is a miniature Arduino Leonardo clone with RFM12B module, intended for low power, battery operated remote node. Due to the miniature size, only few carefully selected pins are available as a side header, none the less that is more than sufficient to interface several sensors at a time.

Having gained experience with my Funky v1 project, I decided to create a new version. The project is a success, yet the areas I wanted to optimize are

The Attiny84 MCU that was used in the Funky v1 has limited RAM and sketch memory
The Attiny84 requires an ISP programmer for sketch uploading
The Attiny84 required a modified Arduino core
Many of the available Arduino libraries are not compatible with Attiny84
So the Funky v2 was created, see my posts here , here and here

Board size is mere 22×22.8mm (0.87″x0.89″) in size (same as Funky v1)
The MCU used in an Atmega32U4, the same MCU that Arduino Leonardo uses. This makes the Funky v2 a minuature Arduino Leonardo compatible board with RFM12b module
The Funky v2 has the Caterina bootloader, so programming is done with Arduino IDE via micro USB
Uses external 8Mhz crystal (can switch to internal RC clock in software as described here)
Operates on 3.3V

Read more http://harizanov.com/wiki/wiki-home/funky-sensor-v2/

Licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
