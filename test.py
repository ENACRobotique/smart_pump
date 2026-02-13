#!/usr/bin/env python3
import serial
import time

DATA_PING = b'\xFF\xFF\x01\x02\x01\xFB'
DATA_READ = b'\xFF\xFF\x01\x04\x02\x2B\x01\xCC'
DATA_WRITE = b'\xFF\xFF\xFE\x04\x03\x03\x01\xF6'

DATAS = [(DATA_PING, 6), (DATA_READ, 7), (DATA_WRITE, 6)]

if __name__ == "__main__":
    ser = serial.Serial("/dev/ttyUSB0", 115200, timeout=2)
    while True:
        for wdata, rlen in DATAS:
            ser.read_all()
            ser.write(wdata)
            try:
                ser.read(len(wdata))
                rdata = ser.read(rlen)
                print(rdata)
            except:
                print("nope")

            time.sleep(2)
    
