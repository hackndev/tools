#!/usr/bin/env python
#
# Quick and dirty script for sending an image to the 'quickload' serial
# bootloader.
#
# eg. ./ql.py < zImage > /dev/ttyUSB0
#
# Author: Alex Osborne <alex at-sign hackndev dot com>
# Created: July 2007

import sys
import struct
import time

image = sys.stdin.read()

# anything before a 'G' will be echoed by bootloader
sys.stdout.write('ql.py: uploading image...\r\n')

# notify bootloader to begin listening
sys.stdout.write('G')

# output the size of the image
sys.stdout.write(struct.pack('<I', len(image)))

# write out image
i = 0
for c in image:
	sys.stdout.write(c)
	sys.stdout.flush()

	# print progress percentage
	i += 1
	if (i % 1000) == 0:
		sys.stderr.write('%f\n' % (i / float(len(image)) * 100))
		sys.stderr.flush()
