#!/usr/bin/env python2.7

#import Gnuplot
import serial
import sys

ser = serial.Serial("/dev/test",9600)

#from starmap_arduino import serial_dic 
#print serial_dic.keys()

#serial_dic = {}
write_file = open('ft.txt','w')


n = 0
while True:
	n += 1
	read_line = ser.readline()
	print read_line.strip()
	read_list = read_line.split(', ')
	print len(read_list)
	write_file.write(read_line+'\n')

write.file.close()
	