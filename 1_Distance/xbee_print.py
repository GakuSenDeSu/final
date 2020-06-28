import time
import serial
import matplotlib.pyplot as plt
import numpy as np

serdev = '/dev/ttyUSB0'
s = serial.Serial(serdev, 9600)
while(1):
    line=s.readline() # Read an echo string from K66F terminated with '\n' (RPC)
    print(line)
    time.sleep(1)