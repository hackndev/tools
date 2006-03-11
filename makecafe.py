# Generate an acecafe header suitable for the LD ROM partition
#
import getopt, os, sys
from struct import pack
from sys import stdout, stderr

HEADER = 0xacecafe0
DEF_OFFSET = 134080    # in sectors (1 sector = 512 bytes)
DEF_LENGTH = 20479778  # in bytes
BLOCKSIZE = 512	       # for cancatenate option

def usage():
	stderr.write('Usage: %s [-o offset] [-l length] [romfile]\n' % sys.argv[0])
	stderr.write('Generate a LD rom partition header.\n\n')
	stderr.write(' -o [offset]     image offset in sectors (defaults to %d)\n' % DEF_OFFSET)
	stderr.write(' -l [length]     image length in bytes (defaults to size of given filename)\n')
	stderr.write(' -c              concatenate header with rom image\n')


def main():
	try:
		opts, args = getopt.getopt(sys.argv[1:], 'o:l:c')
	except getopt.GetoptError:
		usage()
		sys.exit(2)
		
	offset = DEF_OFFSET
	length = DEF_LENGTH
	concat = 0

	if args:
		filename = args[0]
		length = os.path.getsize(filename)

	for o, a in opts:
		if o == '-o':
			offset = int(a)
		elif o == '-l':
			length = int(a)
		elif o == '-c':
			concat = 1
	
	
	# first sector
	# image offset stuff
	stdout.write( pack('<IIII', HEADER, 4, 0x10000, 0) )
	stdout.write( pack('<IIII', 0, offset, 0xb000, 0) )
	stdout.write('\0' * 0x1e0)

	# second sector
	# image length
	stdout.write( pack('<IIII', length, 0, 0, 0) )
	stdout.write('\0' * 0x1f0)

	if concat and filename:
		f = file(filename, 'rb')
		while 1:
			data = f.read(BLOCKSIZE)
			if data == '': break
			stdout.write(data)
		del data

if __name__ == '__main__': main()

