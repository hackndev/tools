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

address = LOAD_ADDR

while 1:
	data = inf.read(4)
	if not data: break

	data += '\0' * (4 - len(data)) # pad with zeros to make full dword

	value, = struct.unpack('<I', data)

	outf.write('w 0x%x=0x%x\r\n' % (address, value))

	address += 4

	if delay:
		outf.flush()
		time.sleep(0.02)

outf.write('x 0x%x\r\n' % LOAD_ADDR)
