#!/usr/bin/env python
#
# This script takes an executable binary image (such as a zImage) and spits out
# a bunch of commands for the "Sandal" recovery console on the LD which loads 
# the image into RAM and then executes it.
#
# eg. ./bin2sandal -d quickload.bin > /dev/ttyUSB0
#
# The -d option introduces a short delay to prevent the Sandal console from being
# overwhelmed.
#
# Author: Alex Osborne <alex at-sign hackndev dot com>
# Created: Jul 2007

LOAD_ADDR=0xa0000000

import sys
import struct
import time

if len(sys.argv) > 1 and sys.argv[1] in ['-h', '--help']:
	print 'Usage: %s [-d] [input binary file] [output sandal commands file]' % sys.argv[0]
	sys.exit(1)

inf = sys.stdin
outf = sys.stdout
delay = False

if len(sys.argv) > 1 and sys.argv[1] == '-d':
	delay = True
	sys.argv.pop(1)

if len(sys.argv) > 1:
	inf = file(sys.argv[1], 'rb')

if len(sys.argv) > 2:
	outf = file(sys.argv[2], 'wb')

def put(s):
	for c in s:
		outf.write(c)
		outf.flush()
		time.sleep(0.001)

# setup gpios
put('w 0x40e0000c=0xc3ef001c\r\n') # GPDR0
put('w 0x40e00010=0xff22ab93\r\n') # GPDR1
put('w 0x40e00014=0x9b1cffff\r\n') # GPDR2
put('w 0x40e0010c=0x01f7e785\r\n') # GPDR3

put('w 0x40e00054=0x08000000\r\n') # GAFR0_L
put('w 0x40e00058=0xa51a8002\r\n') # GAFR0_U
#put('w 0x40e0005c=0x69989940\r\n') # GAFR1_L
#put('w 0x40e00060=0x69989940\r\n') # GAFR1_U
put('w 0x40e00064=0x0aaaaaaa\r\n') # GAFR2_L
put('w 0x40e00068=0x08040c08\r\n') # GAFR2_U
put('w 0x40e0006c=0x010a950c\r\n') # GAFR3_L
put('w 0x40e00070=0x00001400\r\n') # GAFR3_U

put('w 0x40e00018=0x051ba5ff\r\n') # GPSR0
put('w 0x40e0001c=0x65fd5fbc\r\n') # GPSR1
put('w 0x40e00020=0x42fb2d93\r\n') # GPSR2
put('w 0x40e00118=0x0066138d\r\n') # GPSR3

put('\r\n')

address = LOAD_ADDR

while 1:
	data = inf.read(4)
	if not data: break

	data += '\0' * (4 - len(data)) # pad with zeros to make full dword

	value, = struct.unpack('<I', data)

	put('w 0x%x=0x%x\r\n' % (address, value))

	address += 4

	if delay:
		outf.flush()
		time.sleep(0.02)

put('x 0x%x\r\n' % LOAD_ADDR)
