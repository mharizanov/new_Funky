#!/usr/bin/python
import serial   ##sudo apt-get install python-serial
from time import time, sleep

print "Waiting for a IR sequence.."
ser = serial.Serial('/dev/ttyACM0')
while (1):
        ir = ser.readline()
        print ir

 
