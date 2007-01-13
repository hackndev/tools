#!/usr/bin/env python
#
# Requires pyserial (see http://pyserial.sf.net/)
#
import serial, readline, atexit, sys

# Register addresses
GPLR = [0xA8E00000, 0xA8E00004, 0xA8E00008, 0xA8E00100]
GPDR = [0xA8E0000C, 0xA8E00010, 0xA8E00014, 0xA8E0010C]
GPSR = [0xA8E00018, 0xA8E0001C, 0xA8E00020, 0xA8E00118]
GPCR = [0xA8E00024, 0xA8E00028, 0xA8E0002c, 0xA8E00124]
GRER = [0xA8E00030, 0xA8E00034, 0xA8E00038, 0xA8E00130]
GFER = [0xA8E0003C, 0xA8E00040, 0xA8E00044, 0xA8E0013C]
GEDR = [0xA8E00048, 0xA8E0004C, 0xA8E00050, 0xA8E00148]
GAFR = [0xA8E00054, 0xA8E00058, 0xA8E0005c, 0xA8E00060,
	0xA8E00064, 0xA8E00068, 0xA8E0006c, 0xA8E00070]

def openhist():
	import os
	histfile = os.path.join(os.environ["HOME"], ".treodebughist")
	try:
		readline.read_history_file(histfile)
	except IOError:
		pass
	atexit.register(readline.write_history_file, histfile)
	del os, histfile


def dumpmem(start, count):
	f = file('treodump.bin','wb')
	ser.write('mb %x %x\r\n' % (start, count))
	written = 0
	while written < count:
		line = ser.readline().strip()
		print line
		if len(line) < 9 or line[8] != ':': continue
		bits = line.split()
		bin = ''.join([chr(int(x,16)) for x in bits[1:9] + bits[10:18]])
		written += len(bin)
		f.write(bin)
	
	f.close()
		

def gpiotable():
	try:
		import gpio_wiki_tool
	except LibraryError:
		print 'Unable to load gpio_wiki_tool.py'
		return
	
	def readreg(reg):
		ser.write('mw %x 1\r\n' % reg)
		for line in ser.readlines():
			print line.strip()
			if len(line) >= 9 and line[8] == ':':
				code = '-'
				i = 1
				while code == '-':
					code = line.split()[i]
					i+= 1
				return int(code, 16)
				break
	
	gpio_wiki_tool.GPLR = map(readreg, GPLR)
	gpio_wiki_tool.GPDR = map(readreg, GPDR)
	gpio_wiki_tool.GRER = map(readreg, GRER)
	gpio_wiki_tool.GFER = map(readreg, GFER)
	gpio_wiki_tool.GAFR = map(readreg, GAFR)
	gpio_wiki_tool.main()

def loadimg(fn):
	import struct
	try:
		f = file(fn, 'rb')
	except IOError, e:
		print 'Unable to open', fn
		print e
		return
	
	i = 0xa1000000
	while 1:
		buf = f.read(4)
		if buf == '': break
		while len(buf) < 4:
			buf += '\0'

		dat, = struct.unpack('<I', buf)
		ser.write('mw %x 1 %x\r\n' % (i, dat))
		if i % 0x100 == 0:
			lines = ser.readlines()
			print ''.join(lines).replace('\r','')
		i+=4

def docmd(ser, buf):
	words = buf.split()
	if buf == 'exit': sys.exit(0)
	if words[0] == 'dump': return dumpmem(int(words[1],16), int(words[2],16))
	if words[0] == 'gpiotable': return gpiotable()
	if words[0] == 'loadimg': return loadimg(' '.join(words[1:]))

	ser.write(buf + '\r\n')

def main():
	global ser
	openhist()
	ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=0.01)
	try:
		while 1:
			lines = ser.readlines()
			print ''.join(lines).replace('\r','')
			buf = raw_input()
			docmd(ser, buf)
	finally:
		ser.close()
	
	

	

if __name__ == '__main__': main()
