#!/usr/bin/python
import serial   ##sudo apt-get install python-serial
import os
from time import time, sleep

print "Waiting for a IR command.."
ser = serial.Serial('/dev/ttyACM0')
ir = ser.readline()
ir = ir.split(' ')
print ir[2]
if ir[2].strip() == '2602743F':
  print "Got power off command; PID is"; print os.getpid()
  sleep (45)
  os.system("sudo halt -p");
else:
  print "got unrecognized code"

ser.close()

