# Generate an acecafe header suitable for the LD ROM partition
#
import getopt, os, sys
from struct import pack
from sys import stdout, stderr

HEADER = 0xacecafe0
DEF_OFFSET = 134080    # in sectors (1 sector = 512 bytes)
DEF_LENGTH = 20479778  # in bytes

def usage():
	stderr.write('Usage: %s [-o offset] [-l length] [filename]\n' % sys.argv[0])
	stderr.write('Generate a LD rom partition header.\n\n')
	stderr.write(' -o [offset]     image offset in sectors (defaults to %d)\n' % DEF_OFFSET)
	stderr.write(' -l [length]     image length in bytes (defaults to size of given filename)\n')


def main():
	try:
		opts, args = getopt.getopt(sys.argv[1:], 'o:l:')
	except getopt.GetoptError:
		usage()
		sys.exit(2)
		
	offset = DEF_OFFSET
	length = DEF_LENGTH

	if args:
		length = os.path.getsize(args[0])

	for o, a in opts:
		if o == '-o':
			offset = int(a)
		elif o == '-l':
			length = int(a)
	
	
	# first sector
	# image offset stuff
	stdout.write( pack('<IIII', HEADER, 4, 0x10000, 0) )
	stdout.write( pack('<IIII', 0, offset, 0xb000, 0) )
	stdout.write('\0' * 0x1e0)

	# second sector
	# image length
	stdout.write( pack('<IIII', length, 0, 0, 0) )
	stdout.write('\0' * 0x1f0)

if __name__ == '__main__': main()

