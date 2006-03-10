# Quick script to extract raw data out of pdb files.
#
# bobofdoom@gmail.com
#

import sys, struct

def main():
	if len(sys.argv) != 3:
		print 'Usage: %s <pdb file> <out file>' % sys.argv[0]
		return
		
	fn = sys.argv[1]
	f = file(fn, 'rb')
	fo = file(sys.argv[2], 'wb')
	f.seek(76)
	
	num_rec = struct.unpack('>H', f.read(2))[0]
	print num_rec
	toc = []

	for i in range(num_rec):
		toc.append(struct.unpack('>II', f.read(8))[0])
		print toc[-1]

	for i in range(num_rec):
		f.seek(toc[i] + 4)
		blksize = struct.unpack('>I', f.read(4))[0]
		print blksize
		fo.write( f.read(blksize) )		
	
	f.close()
	fo.close()
	
	
if __name__ == '__main__': main()
