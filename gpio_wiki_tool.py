# GPIO deocding and formatting tool for HH.org wiki
#
# bobofdooom@gmail.com
#

# Number of gpios, 120 on PXA27x
gpio_count = 120

# GPIO direction registers
GPDR = [0xC3EF001CL, 0xFF22AB53L, 0x9B1CFFFFL, 0x01F7E785L]

# GPIO alternate function registers (go 0_L, 0_U, 1_L, 1_U ...)
GAFR = [0x08000000L, 0xA51A8002L, 0x69989940L, 0x69989940L, 0x0AAAAAAAL, 0x08040C08L,
		0x010A950CL, 0x00001400L]

# GPIO level registers
GPLR = [0x051BA5FFL, 0x65FD5FBCL, 0x42FB2D93L, 0x0066138DL]

# GPIO rising edge detect
GRER = [0x08004800L, 0L, 0x04000000L, 0x00000800L]

# GPIO falling edge detect
GFER = [0x0000C400L, 0x00000080L, 0L, 0x00000800L]

# End of data

DIR = ['In', 'Out']


def get_bit(bit, reg):
	i = bit / 32
	o = bit % 32
	if reg[i] & (1L<<o): return 1
	return 0

def print_gpio(n):
	# decode
	direc = DIR[ get_bit(n, GPDR) ]
	level = str( get_bit(n, GPLR) )
	af = str( get_bit(n*2, GAFR) + (get_bit(n*2+1, GAFR)<<1) )
	ed = ''
	if get_bit(n, GRER): ed += 'RE '
	if get_bit(n, GFER): ed += 'FE '
	if not ed: ed = ' '
	
	comment = ' '	
	
	# print
	print "||'''" +  str(n) + "'''||" + direc + "||" + level + "||" + af + "||" +  ed + "||" + comment + "||"

def main():
	print "== GPIO configuration =="
	print
	print "||'''GPIO#'''||'''Direction'''||'''State'''||'''AFR'''||'''Interrupt'''||'''Function'''||"
	map(print_gpio, range(gpio_count))
	print """
'''Legend:'''

 * Direction: In - input, Out - output, In/Out - bidirectional
 * State: pin level, GPLR
 * AFR (Alternate Function Number): 0-3, GAFR
 * Interrupt (status): Rising Edge (RE), Falling Edge (FE)
 * Function - description"""
	

if __name__ == '__main__': main()
