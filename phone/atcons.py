#!/usr/bin/env python
#
# Experimental AT command console for the Treo 650 GSM
#

import serial, sys, threading, termios, tty

import os,sys
import sys
import termios

FILTER=''.join([(len(repr(chr(x)))==3) and chr(x) or '.' for x in range(256)])

def dump(src, length=8):
    N=0; result=''
    while src:
       s,src = src[:length],src[length:]
       hexa = ' '.join(["%02X"%ord(x) for x in s])
       s = s.translate(FILTER)
       result += "%04X   %-*s   %s\n" % (N, length*3, hexa, s)
       N+=length
    return result

def getchar():
	'''
	Equivale al comando getchar() di C
	http://www.bigbold.com/snippets/posts/show/3084
	'''

	fd = sys.stdin.fileno()
	
	if os.isatty(fd):
		
		old = termios.tcgetattr(fd)
		new = termios.tcgetattr(fd)
		new[3] = new[3] & ~termios.ICANON & ~termios.ECHO
		new[6] [termios.VMIN] = 1
		new[6] [termios.VTIME] = 0
		
		try:
			termios.tcsetattr(fd, termios.TCSANOW, new)
			termios.tcsendbreak(fd,0)
			ch = os.read(fd,7)

		finally:
			termios.tcsetattr(fd, termios.TCSAFLUSH, old)
	else:
		ch = os.read(fd,7)
	
	return(ch)


readbuf = ''

def read_thread(ser):
	global readbuf
	while 1:
		c = ser.read()
		readbuf += c
		if c != '\r': 
			sys.stdout.write(c)
			sys.stdout.flush()

def main():
	if len(sys.argv) > 1:
		port = sys.argv[1]
	else:
		port = '/dev/ttyS0'
	ser = serial.Serial(port, 460800) #460800)
	try:
		ser.setRTS(1)
	except IOError, e:
		print 'Unable set RTS high:', e
	t = threading.Thread(target=read_thread, args=(ser,))
	t.setDaemon(True)
	t.start()

	while 1:
		c = getchar()
		if c == '\x01': print dump(readbuf); continue
		if c == '\x7f': c = chr(8) # send BS not DEL
		if c == '\n': ser.write('\r')
		ser.write(c)

if __name__ == '__main__': main()
